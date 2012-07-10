#include "tubemodel.h"

const Real TriodeRemoteCutoff6386::fa = -0.1961135820501605;
const Real TriodeRemoteCutoff6386::aa = 3.980508168e-08;
const Real TriodeRemoteCutoff6386::ab = 2.3830020303;
const Real TriodeRemoteCutoff6386::e = -1.8;
const Real TriodeRemoteCutoff6386::d = 0.5;
const Real TriodeRemoteCutoff6386::c = 10.0;
const Real TriodeRemoteCutoff6386::g = 5.0;
const Real TriodeRemoteCutoff6386::h = 0.5;	

Real WDFTubeInterface::iterateNewtonRaphson(Real x, Real dx){
	/*
	x(n+1) = x(n) - Fn(x)/Fn'(x)
	
	Fn'(x) = dFn(x)/dx = (Fn(x + dx) - Fn(x))/(dx) #Finite difference approximation

	x(n+1) = x(n) - dx*Fn(x)/(Fn(x+dx) - Fn(x))
	*/
	Real F = evaluateImplicitEquation(x);
	Real xNew = x - dx*F/(evaluateImplicitEquation(x + dx) - F);
	Assert(isfinite(xNew));
	return xNew;
}

