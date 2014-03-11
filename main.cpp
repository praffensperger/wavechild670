/************************************************************************************
* 
* Wavechild670 v0.1 
* 
* main.cpp
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



/*
** Modified 2012 by Peter Raffensperger
** Copyright (C) 2001-2009 Erik de Castro Lopo <erikd@mega-nerd.com>
**
**
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in
**       the documentation and/or other materials provided with the
**       distribution.
**     * Neither the author nor the names of any contributors may be used
**       to endorse or promote products derived from this software without
**       specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
** TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
** OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
** WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
** OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
** ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include	<stdio.h>

/* Include this header file to use functions from libsndfile. */
#include	<sndfile.h>

/*    This will be the length of the buffer used to hold.frames while
**    we process them.
*/
#define		BUFFER_LEN	1024

/* libsndfile can handle more than 6 channels but we'll restrict it to 6. */
#define		MAX_CHANNELS	6

#include "Misc.h"
#include "wavechild670.h"
#include "getopt_pp.h"
#include "scope.h"

void TestVariableMuAmplifier(){
	cout << "Testing the variable mu amplifier..." << endl;
	LOG_WARNING("No oversampling!");
	
	Real sampleRate = 44100.0;
	
	Real testFrequency = 1000.0;
	Real testDuration = 1.0;
	Real warmUpTime = 1.0;
	uint testNumSamples = (uint) (testDuration * sampleRate);

	Real *buffer = new Real[testNumSamples];
	
	uint graphDisplayNSamples = 0.01*((uint) sampleRate);
	GScope().setup(graphDisplayNSamples, sampleRate);

	cout << "Buffer length = " << testNumSamples << endl;
	cout << "Halfway point = " << (testNumSamples/2) << endl;

	Real inputAmplitude = BasicDSP::ConvertdBmToRMSVoltage(0.0)*sqrt(2.0);
	BasicDSP::FillWithSineWave(buffer, testNumSamples, 1, inputAmplitude, testFrequency, sampleRate);		
	
	VariableMuAmplifier amp(sampleRate);
	
	Real inputGain = BasicDSP::CalculateRMS(buffer+(testNumSamples/2), testNumSamples/2, 1);
	
	SCOPE_PROBE("Vgate", 1);
	SCOPE_PROBE("Vout", 1);
	SCOPE_PROBE("Vcathode", 2);
	SCOPE_PROBE("Vak", 2);
	SCOPE_PROBE("VakModel", 2);
	SCOPE_PROBE("VplateE", 2);
	SCOPE_PROBE("Va", 2);	
	
	//Warm up
	for (uint i = 0; i < ((uint) warmUpTime*sampleRate); ++i){
		amp.advanceAndGetOutputVoltage(0.0, 0.0);
	}

	SCOPE_RESET();
	
	//Process
	for (uint i = 0; i < testNumSamples; ++i){
		SCOPE("Vin", buffer[i]);
		Real vout = amp.advanceAndGetOutputVoltage(buffer[i], 0.0);
		buffer[i] = vout;
		SCOPE("Vout", vout);
	}
	Real outputGain = BasicDSP::CalculateRMS(buffer+(testNumSamples/2), testNumSamples/2, 1);
	
	cout << "============================" << endl;
	cout << "Input amplitude = " << inputAmplitude << endl;
	Real inputAmplitudeM = BasicDSP::ConvertRMSVoltageTodBm(inputGain);
	Real outputAmplitudeM = BasicDSP::ConvertRMSVoltageTodBm(outputGain);
	cout << "Measured input  amplitude = " << inputGain << " = " << inputAmplitudeM << "dBm" << endl;
	cout << "Measured output amplitude = " << outputGain << " = " << outputAmplitudeM << "dBm" << endl;
	cout << "Measured gain             = " << outputAmplitudeM - inputAmplitudeM << "dB" << endl;
	stringstream title;
	title << "Variable mu amplifier ";
	title << "Measured gain = " << outputAmplitudeM - inputAmplitudeM << "dB";
	GScope().showGraph(title.str(), 3, "Vin", "Vgate", "Vout");
	GScope().showGraph(title.str(), 4, "Vcathode", "Vak", "VplateE", "VakModel");
	GScope().showGraph(title.str(), 1, "Va");
	
	delete[] buffer;
	
	exit(0);	
}


void ComputeStaticGainCurve(Wavechild670Parameters& params, Real sampleRate, uint numGainPoints=10, Real minGain=-50, Real maxGain=10, bool quiet=false){
	cout << "Calculating static gain curve..." << endl;
	LOG_WARNING("No oversampling!");
	
	Real testFrequency = 1000.0;
	Real testDuration = 1.0;
	Real compressorWarmUpTime = 1.0;
	uint testNumSamples = (uint) (testDuration * sampleRate);
	uint numChannels = 2;
	Real *testGainsIndBm = new Real[numGainPoints];
	for (uint i = 0; i < numGainPoints; ++i) {
		testGainsIndBm[i] = ((Real) i) / ((Real) numGainPoints - 1) * (maxGain - minGain) + minGain;
	}
	uint bufferLength = testNumSamples*numChannels;
	Real *buffer = new Real[bufferLength];

	params.hardClipOutput = false;
	
	uint graphDisplayNSamples = 0.01*((uint) sampleRate);
	GScope().setup(graphDisplayNSamples, sampleRate);

	cout << "Buffer length = " << bufferLength << endl;
	cout << "Halfway point = " << (testNumSamples*numChannels/2) << endl;
	cout << "START MACHINE READABLE" << endl;
	for (uint gainIndex = 0; gainIndex < numGainPoints; ++gainIndex){
		Real inputAmplitude = BasicDSP::ConvertdBmToRMSVoltage(testGainsIndBm[gainIndex])*sqrt(2.0);
		for (uint channelIndex = 0; channelIndex < numChannels; ++channelIndex){
			//uint channelIndex = 0;
			BasicDSP::FillWithSineWave(buffer+channelIndex, testNumSamples, numChannels, inputAmplitude, testFrequency, sampleRate);		
			//channelIndex = 1;
			//BasicDSP::FillWithCosineWave(buffer+channelIndex, testNumSamples, numChannels, inputAmplitude, testFrequency, sampleRate);		
		}
		Real inputGainLeft = BasicDSP::CalculateRMS(buffer+(testNumSamples*numChannels/2), testNumSamples/2, numChannels);
		Wavechild670 compressor(sampleRate, params);
		compressor.warmUp(compressorWarmUpTime);
    	compressor.process(buffer, buffer, bufferLength);	
		Real outputGainLeft = BasicDSP::CalculateRMS(buffer+(testNumSamples*numChannels/2), testNumSamples/2, numChannels);
		Real inputAmplitudeM = BasicDSP::ConvertRMSVoltageTodBm(inputGainLeft);
		Real outputAmplitudeM = BasicDSP::ConvertRMSVoltageTodBm(outputGainLeft);

		if (quiet){
			cout << inputAmplitudeM << ", " << outputAmplitudeM << endl;
		}
		else{
			cout << "============================" << endl;
			cout << gainIndex << " of " << numGainPoints << endl;
			cout << "Test gain = " << testGainsIndBm[gainIndex] << "dBm" << endl;
			cout << "Input amplitude = " << inputAmplitude << endl;
			cout << "Measured input  amplitude = " << inputGainLeft << " = " << inputAmplitudeM << "dBm" << endl;
			cout << "Measured output amplitude = " << outputGainLeft << " = " << outputAmplitudeM << "dBm" << endl;
			cout << "Measured gain             = " << outputAmplitudeM - inputAmplitudeM << "dB" << endl;
		}
		
	}
	
	delete[] buffer;
	delete[] testGainsIndBm;
}

int main (int argc, char** argv) {

	string inputFilename = "input.wav";
	string outputFilename = "output.wav";
	
	Real inputLevelA = 1.0;
	Real ACThresholdA = 0.5;
	uint timeConstantSelectA = 2;
	Real DCThresholdA = 0.1;
	
	Real inputLevelB = 1.0;
	Real ACThresholdB = 0.5;
	uint timeConstantSelectB = 2;
	Real DCThresholdB = 0.1;

	Real inputLevelJoint = 1.0;
	Real ACThresholdJoint = 0.5;
	uint timeConstantSelectJoint = 2;
	Real DCThresholdJoint = 0.1;
	
	bool sidechainLink = false;
	bool isMidSide = false;
	bool useFeedbackTopology = true;
	
	bool computeStaticGainCurve = false;
	bool computeStaticGainCurveQuiet = false;
	uint numGainPoints = 10;
	Real minGain = -50.0; 
	Real maxGain = 10.0;
	
	Real outputGain = 1.0;
	bool hardClipOutput = true;

	Real sampleRateOverride = 44100.0;	

	GetOpt::GetOpt_pp ops(argc, argv);
	ops >> GetOpt::Option('i', "inputfilename", inputFilename);
	ops >> GetOpt::Option('o', "outputfilename", outputFilename);
	
	ops >> GetOpt::Option('s', "sampleRate", sampleRateOverride);

	ops >> GetOpt::Option('x', "inputLevel", inputLevelJoint);
	ops >> GetOpt::Option('x', "ACThreshold", ACThresholdJoint);
	ops >> GetOpt::Option('x', "timeConstantSelect", timeConstantSelectJoint);
	ops >> GetOpt::Option('x', "DCThreshold", DCThresholdJoint);

	inputLevelA = inputLevelJoint;
	inputLevelB = inputLevelJoint;
	ACThresholdA = ACThresholdJoint;
	ACThresholdB = ACThresholdJoint;
	timeConstantSelectA = timeConstantSelectJoint;
	timeConstantSelectB = timeConstantSelectJoint;
	DCThresholdA = DCThresholdJoint;
	DCThresholdB = DCThresholdJoint;
	
	ops >> GetOpt::Option('x', "inputLevelA", inputLevelA);
	ops >> GetOpt::Option('x', "ACThresholdA", ACThresholdA);
	ops >> GetOpt::Option('x', "timeConstantSelectA", timeConstantSelectA);
	ops >> GetOpt::Option('x', "DCThresholdA", DCThresholdA);
	
	ops >> GetOpt::Option('x', "inputLevelB", inputLevelB);
	ops >> GetOpt::Option('x', "ACThresholdB", ACThresholdB);
	ops >> GetOpt::Option('x', "timeConstantSelectB", timeConstantSelectB);
	ops >> GetOpt::Option('x', "DCThresholdB", DCThresholdB);

	ops >> GetOpt::OptionPresent('x', "sidechainLink", sidechainLink);
	ops >> GetOpt::OptionPresent('x', "isMidSide", isMidSide);
	ops >> GetOpt::Option('x', "outputGain", outputGain);
	//ops >> GetOpt::OptionPresent('x', "useFeedbackTopology", useFeedbackTopology);
	
	ops >> GetOpt::OptionPresent('c', "computeStaticGainCurve", computeStaticGainCurve);
	ops >> GetOpt::OptionPresent('q', "computeStaticGainCurveQuiet", computeStaticGainCurveQuiet);
	ops >> GetOpt::Option('x', "numGainPoints", numGainPoints);
	ops >> GetOpt::Option('x', "minGain", minGain);
	ops >> GetOpt::Option('x', "maxGain", maxGain);	
	
	cout << "Processing audio with Wavechild670!" << endl;	
	cout << "inputFilename=" << inputFilename << endl; 
	cout << "outputFilename=" << outputFilename << endl; 
		
	cout << "inputLevelA=" << inputLevelA << endl; 
	cout << "ACThresholdA=" << ACThresholdA << endl; 
	cout << "timeConstantSelectA=" << timeConstantSelectA << endl; 
	cout << "DCThresholdA=" << DCThresholdA << endl; 
		
	cout << "inputLevelB=" << inputLevelB << endl; 
	cout << "ACThresholdB=" << ACThresholdB << endl; 
	cout << "timeConstantSelectB=" << timeConstantSelectB << endl; 
	cout << "DCThresholdB=" << DCThresholdB << endl; 
		
	cout << "sidechainLink=" << sidechainLink << endl; 
	cout << "isMidSide=" << isMidSide << endl; 
	cout << "useFeedbackTopology=" << useFeedbackTopology << endl; 
	
	cout << "sampleRateOverride=" << sampleRateOverride << endl; 
	cout << "outputGain=" << outputGain << endl; 	

	Wavechild670Parameters params(inputLevelA, ACThresholdA, timeConstantSelectA, DCThresholdA, 
									inputLevelB, ACThresholdB, timeConstantSelectB, DCThresholdB, 
									sidechainLink, isMidSide, useFeedbackTopology, outputGain,
									hardClipOutput);
	
	if (computeStaticGainCurve){
		ComputeStaticGainCurve(params, sampleRateOverride, numGainPoints, minGain, maxGain, computeStaticGainCurveQuiet);
		exit(0);
	}
	
	/* This is a buffer of double precision floating point values
    ** which will hold our data while we process it.
    */
    static double data [BUFFER_LEN] ;

    /* A SNDFILE is very much like a FILE in the Standard C library. The
    ** sf_open function return an SNDFILE* pointer when they sucessfully
	** open the specified file.
    */
    SNDFILE      *infile, *outfile ;

    /* A pointer to an SF_INFO stutct is passed to sf_open.
    ** On read, the library fills this struct with information about the file.
    ** On write, the struct must be filled in before calling sf_open.
    */
    SF_INFO		sfinfo ;
    int			readcount ;

    /* Here's where we open the input file. We pass sf_open the file name and
    ** a pointer to an SF_INFO struct.
    ** On successful open, sf_open returns a SNDFILE* pointer which is used
    ** for all subsequent operations on that file.
    ** If an error occurs during sf_open, the function returns a NULL pointer.
	**
	** If you are trying to open a raw headerless file you will need to set the
	** format and channels fields of sfinfo before calling sf_open(). For
	** instance to open a raw 16 bit stereo PCM file you would need the following
	** two lines:
	**
	**		sfinfo.format   = SF_FORMAT_RAW | SF_FORMAT_PCM_16 ;
	**		sfinfo.channels = 2 ;
    */
    if (! (infile = sf_open (inputFilename.c_str(), SFM_READ, &sfinfo)))
    {   /* Open failed so print an error message. */
        printf ("Not able to open input file %s.\n", inputFilename.c_str()) ;
        /* Print the error message from libsndfile. */
        puts (sf_strerror (NULL)) ;
        return  1 ;
        } ;

    if (sfinfo.channels > MAX_CHANNELS)
    {   printf ("Not able to process more than %d channels\n", MAX_CHANNELS) ;
        return  1 ;
        } ;
    /* Open the output file. */
    if (! (outfile = sf_open (outputFilename.c_str(), SFM_WRITE, &sfinfo)))
    {   printf ("Not able to open output file %s.\n", outputFilename.c_str()) ;
        puts (sf_strerror (NULL)) ;
        return  1 ;
        } ;

	Wavechild670 compressor(sampleRateOverride, params);
	compressor.warmUp();

    /* While there are.frames in the input file, read them, process
    ** them and write them to the output file.
    */
    
	time_t starttime1 = time (NULL);
    
    Assert(sfinfo.channels == 2);
    while ((readcount = sf_read_double (infile, data, BUFFER_LEN)))
    {   
        //cout << "Processing " << readcount << " frames." << endl;
    	compressor.process (data, data, readcount) ;
        sf_write_double (outfile, data, readcount) ;
        } ;

	time_t stoptime1 = time (NULL);
	cout << "time taken: " << stoptime1 - starttime1 << endl;


    /* Close input and output files. */
    sf_close (infile) ;
    sf_close (outfile) ;

    return 0 ;
} /* main */
