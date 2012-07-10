#ifndef TUBEMODEL_H
#define TUBEMODEL_H

#include "Misc.h"
#include "scope.h"

class TriodeModel {
public:
	TriodeModel() {}
	virtual ~TriodeModel() {}
	virtual Real getIa(Real Vgk, Real Vak) { 
		LOG_ERROR("Using an undefined triode model! FAIL!");
		return 0.0; 
	}
	virtual Real getIg(Real Vgk, Real Vak) { 
		LOG_ERROR("Using an undefined triode model! FAIL!");
		return 0.0; 
	}

	virtual TriodeModel* clone() const { return new TriodeModel(*this); }
private:
	TriodeModel(const TriodeModel& other) { }
};

class TriodeRemoteCutoff6386 : public TriodeModel {
public:
	TriodeRemoteCutoff6386() : VgkLast(1.0), numeratorLast(0.0) { }
	virtual ~TriodeRemoteCutoff6386() { }
	
	virtual Real getIa(Real Vgk, Real Vak){
		if (Vak < 0.0) {
			Vak = 0.0;
		}
		if (Vgk > 0.0) {
			Vgk = 0.0;
		}
		Real p1 = 3.981e-8;
		Real p2 = 2.383;
		Real p3 = 0.5;
		Real p4 = 0.1;
		Real p5 = 1.8;
		Real p6 = 0.5;
		Real p7 = -0.03922;
		Real p8 = 0.2;
		Real iakAlt = p1*pow(Vak, p2) / (pow((p3-p4*Vgk), p5)*(p6+exp(p7*Vak-p8*Vgk)));
		return iakAlt;
		
	}
	virtual TriodeModel* clone() const { return new TriodeRemoteCutoff6386(*this); }	

private:
	TriodeRemoteCutoff6386(const TriodeRemoteCutoff6386& other) {
		LOG_INFO("Copied TriodeRemoteCutoff6386");
		VgkLast = other.VgkLast; 
		numeratorLast = other.numeratorLast; 
	}

protected:

	static inline Real getA(Real Vak){
		return aa*pow(Vak, ab);
	}
	static inline Real getF(Real Vak){
		return fa*Vak;
	}			
	
protected:
	Real VgkLast;
	Real numeratorLast;
	
	static const Real fa;
	static const Real aa;
	static const Real ab;
	static const Real e;
	static const Real d;
	static const Real c;
	static const Real g;
	static const Real h;	
};

class WDFTubeInterface {
public:
	WDFTubeInterface() { model = NULL; }
	WDFTubeInterface(TriodeModel *model_, Real numParallelInstances_=3.0) : model(model_), 
	numParallelInstances(numParallelInstances_) {
		a = 0.0;
		Vgk = 0.0;
		Iak = 0.0;
		VakGuess = 100.0;
	}
	~WDFTubeInterface() { delete model; }
	
	WDFTubeInterface(WDFTubeInterface& other) {
		LOG_INFO("Copied WDFTubeInterface");
		model = other.model->clone();
		numParallelInstances = other.numParallelInstances;
		a = other.a;
		Vgk = other.Vgk;
		Iak = other.Iak;
		VakGuess = other.VakGuess;
	}
	
	Real getB(Real a_, Real r0_, Real Vgate, Real Vk){
		Assert(model);
		/*
		Reference:
		"Wave Digital Simulation of a Vacuum-Tube Amplifier"
		By M. Karjalainen and J. Pakarinen, ICASSP 2006

		Vak + R0*f(Vgk, Vak) - a = 0 	#[Karjalainen and Pakarinen, eq 7]
		b = Vak - Ro*f(Vgk, Vak)		#[Karjalainen and Pakarinen, eq 8]
		*/
		
		r0 = r0_;
		
		a = a_;
		
		Vgk = Vgate - Vk;

		Real Vak = VakGuess;
		uint iteration = 0;
		Real err = 1e6;
		Iak = 0.0;
		
		LOG_SAMPLE2("Vak=" << Vak << " Vgk=" << Vgk << " a=" << a << " ");
		
		while (fabs(err)/fabs(Vak) > 1e-9){
			VakGuess = iterateNewtonRaphson(Vak);
			err = Vak - VakGuess;
			Vak = VakGuess;

			LOG_INNER_LOOP("Vak=" << Vak << " err=" << err << " Iak=" << Iak);
			if (iteration > 100){
				LOG_ERROR("Convergence failure!");
				break;
			}
			++iteration;
		}
		Real b = Vak - r0*Iak;
		/*
		a = v + Ri
		b = v - Ri
		v = a + b
		*/
		SCOPE("VakModel", Vak);
		LOG_SAMPLE2("Vgk" <<  Vgk << " Vak=" << Vak << " Iak=" << Iak);
		return b;
	}

protected:
	Real evaluateImplicitEquation(Real Vak){
		Assert(!isnan(Vak));
		Assert(!isnan(Vgk));		
		Iak = model->getIa(Vgk, Vak) * numParallelInstances;
		LOG_INNER_LOOP("Eval: " << "Vgk=" << Vgk << " Vak=" << Vak << " Iak=" << Iak << " r0=" << r0 << "; ");
		Assert(!isnan(Iak));
		LOG_INNER_LOOP("a=" << a << " diff=" << Vak + r0*Iak - a);
		return Vak + r0*Iak - a;
	}
	Real iterateNewtonRaphson(Real x, Real dxFactor=1e-6);

	Real numParallelInstances;

	Real r0;
	Real a;
	Real Vgk;
	Real Iak;
	Real VakGuess;
	TriodeModel *model;
};

#endif
