#ifndef SCOPE_H
#define SCOPE_H

#include "Misc.h"
#include <stdarg.h>
#include "basicdsp.h"

#include "gnuplot_i.h"

//Control use of the global scope object. Disable use of the global scope for best performance, but it may be useful in debugging.
#define USE_SCOPE

#ifdef USE_SCOPE
#define SCOPE(channel, value) GScope()[channel](value)
#define SCOPE_PROBE(name, numChannels) GScope().addProbe(name, numChannels)
#define SCOPE_RESET() LOG_INFO("GScope reset!"); GScope().reset()
#else
#define SCOPE(channel, value)
#define SCOPE_RESET()
#define SCOPE_PROBE(name, numChannels)
#endif

#define SCOPE_DEFAULT_BUFFER_SIZE 1024
#define SCOPE_DEFAULT_SAMPLERATE 44100.0

gnuplot_ctrl* MultiLinePlot(double* x, vector<double*> ys, uint numSamples, vector<string> labels, string title);

class Probe {
public:
	Probe(string name_="", uint bufferSize_=SCOPE_DEFAULT_BUFFER_SIZE, uint numChannels_=1) : 
	name(name_), bufferSize(bufferSize_), buffer(0), nextSampleIndex(0), numChannels(numChannels_) { }
	
	virtual ~Probe() { Confirm(buffer == 0); }
	
	virtual void setup() { 
		buffer = new Real[bufferSize*numChannels]; 
		nextSampleIndex = 0;
		for (uint i = 0; i < bufferSize*numChannels; ++i){
			buffer[i] = 0.0;
		}
	}
	
	virtual void cleanup() { 
		//cout << "Cleaning up " << name << "!" << endl;
		delete[] buffer; 
		buffer = 0;
	}

	virtual void reset(){
		cleanup();
		setup();
	}

	void operator() (const Real sample) {
		saveSample(sample);
	}

	void operator() (const Real* buf, uint numSamples) {
		//cout << name << " Snatching buffer of length " << numSamples << " nextSampleIndex = " << nextSampleIndex << endl;
		for (uint i = 0; i < numSamples; ++i){
			saveSample(buf[i]);
		}
	}

	virtual void saveSample(const Real sample){
		Assert(buffer);
		buffer[nextSampleIndex++] = sample;
		if (nextSampleIndex >= bufferSize*numChannels){
			nextSampleIndex = 0;
		}
	}

	virtual Real* getBufferAlignedOneChannel(uint channel) const{
		/*for (uint i = 0; i < bufferSize*numChannels; ++i){
			cout << "i=" << i << " " << name << "=" << buffer[i] << endl;
		}
		Real *bufOneChannel = BasicDSP::Deinterleave(buffer+channel, bufferSize, numChannels);
		for (uint i = 0; i < bufferSize; ++i){
			cout << "i=" << i << " " << name << "Deinterleaved" << channel << "=" << bufOneChannel[i] << endl;
		}
		return bufOneChannel;*/
		Real *bufAligned = getBufferAligned();
		Real *bufAlignedOneChannel = BasicDSP::Deinterleave(bufAligned+channel, bufferSize, numChannels);
		//cout << "Deleting bufAligned" << endl;
		delete[] bufAligned;
		return bufAlignedOneChannel;
		
	}
	
	virtual Real* getBufferAligned() const{
		Real *bufAligned = new Real[bufferSize*numChannels];
		for (uint i = nextSampleIndex; i < bufferSize*numChannels; ++i){
			//cout << "i=" << i << " i - nextSampleIndex=" << i - nextSampleIndex << endl;
			bufAligned[i - nextSampleIndex] = buffer[i];
		}		
		for (uint i = 0; i < nextSampleIndex; ++i){
			//cout << "i=" << i << " i + nextSampleIndex=" << i + nextSampleIndex << endl;
			bufAligned[i + bufferSize*numChannels - nextSampleIndex] = buffer[i];
		}		
		return bufAligned;
	}

	virtual uint getNumChannels() const{
		return numChannels;
	}
	
protected:
	uint bufferSize;
	uint nextSampleIndex;
	
	Real *buffer;
	string name;
	uint numChannels;
};

typedef map<string, Probe> ProbeMap;

class Scope {
public:
	Scope(uint bufferSize_=SCOPE_DEFAULT_BUFFER_SIZE, Real sampleRate_=SCOPE_DEFAULT_SAMPLERATE): 
	bufferSize(bufferSize_), sampleRate(sampleRate_){
		
	}
	
	virtual void setup(uint bufferSize_, Real sampleRate_){
		sampleRate = sampleRate_;
		bufferSize = bufferSize_;
	}
	
	virtual ~Scope() {
		for (uint i = 0; i < graphs.size(); ++i){
			gnuplot_close(graphs[i]);
		}
		ProbeMap::const_iterator end = probes.end(); 
		for (ProbeMap::iterator it = probes.begin(); it != end; ++it) {
			it->second.cleanup();
		}		
	}
	
	virtual void reset(){
		ProbeMap::const_iterator end = probes.end(); 
		for (ProbeMap::iterator it = probes.begin(); it != end; ++it) {
			it->second.reset();
		}				
	}
	
	virtual void addProbe(string name, uint numChannels=1){
		probes[name] = Probe(name, bufferSize, numChannels);
		probes[name].setup();
	}
	
	Probe& operator[](const string key) {
		ProbeMap::const_iterator end = probes.end(); 
    	if (probes.find(key) == end){
			addProbe(key);
    	}
    	return probes[key];
	}
	
	virtual void saveSample(string probeName, Real sample){
		probes[probeName].saveSample(sample);
	}

	virtual void showGraph(string title, uint numTraces, ...){
		va_list args;
		vector<string> probeNameRestrictions;

		va_start(args, numTraces); 
		for (uint i = 0; i < numTraces; ++i) {
			string probeName = va_arg(args, const char*);
			probeNameRestrictions.push_back(string(probeName));
		}
		va_end(args);
		
		Real *x = new Real[bufferSize];
		Real dt = 1.0 / sampleRate;
		for (uint i = 0; i < bufferSize; ++i){
			x[i] = ((Real) i) * dt;
		}
		//cout << "Max x value " << x[bufferSize-1] << endl;
		vector<string> channelNames;
		vector<Real*> ys;
		ProbeMap::const_iterator end = probes.end(); 
		for (ProbeMap::const_iterator it = probes.begin(); it != end; ++it) {
			for (uint i = 0; i < probeNameRestrictions.size(); ++i){
				if (probeNameRestrictions[i] == it->first){
					if(it->second.getNumChannels() > 1){
						for (uint channelIndex = 0; channelIndex < it->second.getNumChannels(); ++channelIndex){
							stringstream name;
							name << it->first << channelIndex;
							channelNames.push_back(name.str());
							ys.push_back(it->second.getBufferAlignedOneChannel(channelIndex));
						}
					}
					else{
						channelNames.push_back(it->first);
						ys.push_back(it->second.getBufferAligned());
					}
				}
			}
		}		
		
		gnuplot_ctrl* g = MultiLinePlot(x, ys, bufferSize, channelNames, title);
		//cout << "Deleting x" << endl;
		delete[] x;
		for (uint i = 0; i < ys.size(); ++i){
			//cout << "Deleting ys" << endl;
			delete[] ys[i];
		}
		graphs.push_back(g);
	}
	
protected:
	Real sampleRate;
	vector<gnuplot_ctrl*> graphs;
	ProbeMap probes;
	
	uint bufferSize;
};

Scope& GScope();

#endif