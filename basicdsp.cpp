/************************************************************************************
* 
* Wavechild670 v0.1 
* 
* basicdsp.cpp
* 
* By Peter Raffensperger 11 March 2014
* 
* Reference:
* Toward a Wave Digital Filter Model of the Fairchild 670 Limiter, Raffensperger, P. A., (2012). 
* Proc. of the 15th International Conference on Digital Audio Effects (DAFx-12), 
* York, UK, September 17-21, 2012.
* 
* Note:
* Fairchild (R) a registered trademark of Avid Technology, Inc., which is in no way associated or 
* affiliated with the author.
* 
* License:
* Wavechild670 is licensed under the GNU GPL v2 license. If you use this
* software in an academic context, we would appreciate it if you referenced the original
* paper.
* 
************************************************************************************/



#include "basicdsp.h"

namespace BasicDSP {

Real clip(Real x, Real minVal, Real maxVal) { //constrains x to [minVal, maxVal]
	if (x < minVal) {
		return minVal;
	}
	if (x > maxVal) {
		return maxVal;
	}
	return x;
}

Real clipWithWarning(Real x, Real minVal, Real maxVal) { //constrains x to [minVal, maxVal]
	if (x < minVal) {
		LOG_WARNING("Clip! " << x << " < " << minVal);
		return minVal;
	}
	if (x > maxVal) {
		LOG_WARNING("Clip! " << x << " > " << maxVal);
		return maxVal;
	}
	return x;
}

Real GetPeak(Real *inputBuf, uint numSamples, uint stepSize){
	Real peak = 0.0;
	for (uint i = 0; i < numSamples*stepSize; i += stepSize){
		Real input = inputBuf[i];
		if (fabs(input) > fabs(peak)){
			peak = input;
		}
	}
	return peak;
}

Real* Deinterleave(Real *inputBuf, uint numSamples, uint stepSize){
	Real *newBuf = new Real[numSamples];
	for (uint i = 0; i < numSamples; i += 1){
		newBuf[i] = inputBuf[i*stepSize];
	}
	return newBuf;
}

Real CalculateRMS(Real *inputBuf, uint numSamples, uint stepSize){
	Real sumOfSquares = 0.0;
	for (uint i = 0; i < numSamples*stepSize; i += stepSize){
		Real input = inputBuf[i];
		sumOfSquares += input*input;
	}
	return sqrt(sumOfSquares/((Real) numSamples));
}

Real ConvertRMSVoltageTodBm(Real voltage, Real resistance){
	Real power = voltage*voltage/resistance;
	return 10.0*log10(power) + 30.0;
}

Real ConvertdBmToRMSVoltage(Real dBm, Real resistance){
	//cout << "dBm=" << dBm << endl;
	//cout << "dBm/10 - 3=" << dBm/10.0 - 3.0 << endl;
	Real x = pow(10.0, dBm/10.0 - 3.0);
	//cout << "x=" << x << endl;
	return sqrt(x*resistance);
}

void FillWithSineWave(Real* output, uint numSamples, uint stepSize, Real amplitude, Real frequency, Real sampleRate){
	for (uint i = 0; i < numSamples; i += 1){
		Real t = ((Real) i) / sampleRate;
		output[i*stepSize] = amplitude*sin(2.0*M_PI*frequency*t);
	}
}

void FillWithCosineWave(Real* output, uint numSamples, uint stepSize, Real amplitude, Real frequency, Real sampleRate){
	for (uint i = 0; i < numSamples; i += 1){
		Real t = ((Real) i) / sampleRate;
		output[i*stepSize] = amplitude*cos(2.0*M_PI*frequency*t);
	}
}

}