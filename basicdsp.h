/************************************************************************************
Wavechild670 v0.1 

basicdsp.h

By Peter Raffensperger 10 July 2012

Reference:
Toward a Wave Digital Filter Model of the Fairchild 670 Limiter, Raffensperger, P. A., (2012). 
Proc. of the 15th International Conference on Digital Audio Effects (DAFx-12), 
York, UK, September 17-21, 2012.

Note:
Fairchild (R) a registered trademark of Avid Technology, Inc., which is in no way associated or 
affiliated with the author.

License:
Wavechild670 is licensed under the GNU GPL v2 license. If you use this
software in an academic context, we would appreciate it if you referenced the original
paper.
************************************************************************************/

#ifndef BASICDSP_H
#define BASICDSP_H

#include "Misc.h"

namespace BasicDSP{

Real clip(Real x, Real minVal, Real maxVal); //constrains x to [minVal, maxVal]
Real clipWithWarning(Real x, Real minVal, Real maxVal); //constrains x to [minVal, maxVal]


struct stereoSample {
	Real l;
	Real r;
};

Real* Deinterleave(Real *inputBuf, uint numSamples, uint stepSize);

Real GetPeak(Real *inputBuf, uint numSamples, uint stepSize);
Real CalculateRMS(Real *inputBuf, uint numSamples, uint stepSize);

Real ConvertRMSVoltageTodBm(Real voltage, Real resistance=600.0);

Real ConvertdBmToRMSVoltage(Real dBm, Real resistance=600.0);

void FillWithSineWave(Real* output, uint numSamples, uint stepSize, Real amplitude, Real frequency, Real sampleRate);
void FillWithCosineWave(Real* output, uint numSamples, uint stepSize, Real amplitude, Real frequency, Real sampleRate);
class WindowFunctions {
public:
	/**
	 * From http://en.wikipedia.org/wiki/Window_function
	 * 'The "raised cosine" with these particular coefficients was proposed by Richard W. Hamming. 
	 * The window is optimized to minimize the maximum (nearest) side lobe, giving it a height of 
	 * about one-fifth that of the Hann window, a raised cosine with simpler coefficients
	 * @param the number of points from the window 
	 * @return w(i) = 0.54 - 0.46*cos( (2*pi*i) / (length-1) ) 
	 */
	static void getHammingWindow(uint length, Real* output){
		Assert(output);
		for (uint i = 0; i < length; ++i){
			output[i] = 0.54 - 0.46 * cos( (2.0 * M_PI * ((Real) i)) / ((Real) (length - 1)) );
		}
	}

private:
	WindowFunctions() {} //Holder class
};

}

#endif