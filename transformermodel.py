import math

import numpy
import matplotlib.pyplot as plt

import wavedigitalfilters
import wdfgenerator

def GenerateNonIdealTransformerModelCode():
	g = wdfgenerator.Generator('WDFNonIdealTransformer')
	
	Rload = wdfgenerator.GeneratorWDFResistor('Rload', g)
	Source = wdfgenerator.GeneratorWDFResistiveSource('Source', g)
	RsourceTermination = wdfgenerator.GeneratorWDFResistor('RsourceTermination', g)
	seriesConn = wdfgenerator.GeneratorWDFSeriesAdapter('seriesConn', g)
	parallelConn = wdfgenerator.GeneratorWDFSeriesAdapter('parallelConn', g)
	
	Lp = wdfgenerator.GeneratorWDFInductor('Lp', g)
	Rp = wdfgenerator.GeneratorWDFResistor('Rp', g)
	Lm = wdfgenerator.GeneratorWDFInductor('Lm', g)
	Rc = wdfgenerator.GeneratorWDFResistor('Rc', g)
	Ls = wdfgenerator.GeneratorWDFInductor('Ls', g)
	Rs = wdfgenerator.GeneratorWDFResistor('Rs', g)
	Cw = wdfgenerator.GeneratorWDFCapacitor('Cw', g)

	transformer = wdfgenerator.GeneratorWDFIdealTransformer('transformer', g)
	
	#Adaptors
	primaryInputSeriesConn = wdfgenerator.GeneratorWDFSeriesAdapter('primaryInputSeriesConn', g)
	primarySeriesConn2 = wdfgenerator.GeneratorWDFSeriesAdapter('primarySeriesConn2', g)
	primaryParallelConn1 = wdfgenerator.GeneratorWDFParallelAdapter('primaryParallelConn1', g)
	primaryParallelConn2 = wdfgenerator.GeneratorWDFParallelAdapter('primaryParallelConn2', g)
	secondarySeriesConn1 = wdfgenerator.GeneratorWDFSeriesAdapter('secondarySeriesConn1', g)
	secondarySeriesConn2 = wdfgenerator.GeneratorWDFSeriesAdapter('secondarySeriesConn2', g)
	secondaryOutputParallelConn = wdfgenerator.GeneratorWDFParallelAdapter('secondaryOutputParallelConn', g)

	#print seriesConn.childrensPorts[0].R
	#print seriesConn.childrensPorts[1].R

	secondaryOutputParallelConn.ConnectChild(Rload)
	secondaryOutputParallelConn.ConnectChild(Cw)
	secondarySeriesConn1.ConnectChild(secondaryOutputParallelConn)
	secondarySeriesConn2.ConnectChild(Rs)
	secondarySeriesConn2.ConnectChild(Ls)
	secondarySeriesConn1.ConnectChild(secondarySeriesConn2)
	transformer.ConnectChild(secondarySeriesConn1)
	primaryParallelConn1.ConnectChild(transformer)
	
	primarySeriesConn2.ConnectChild(Lp)
	primarySeriesConn2.ConnectChild(Rp)
	primaryParallelConn2.ConnectChild(Lm)
	primaryParallelConn2.ConnectChild(Rc)
	primaryParallelConn1.ConnectChild(primaryParallelConn2)
	
	primaryInputSeriesConn.ConnectChild(primarySeriesConn2)
	primaryInputSeriesConn.ConnectChild(primaryParallelConn1)
	
	parallelConn.ConnectChild(primaryInputSeriesConn)
	parallelConn.ConnectChild(RsourceTermination)
	seriesConn.ConnectChild(parallelConn)
	seriesConn.ConnectChild(Source)

	g.Input('vin')
	g.ForwardCalculation('SourceE = vin')
	g.ForwardCalculation('//Get Bs')
	a = seriesConn.GetB()
	b = '-(' + a + ')' #short circuit rule
	g.ForwardCalculation('b = ' + b)
	g.ForwardCalculation('//Set As')
	seriesConn.SetA('b')
	g.Output(Cw.GetVoltage())

	start = """#include "wdf_cpp/Misc.h"\n\n"""
	end = """
	
int main(){
	/*
	Lp = 4e-3
	Rp = 10.0
	Rc = 5e3
	Lm = 20.0
	Rs = 50.0
	Ls = 1e-3
	Cw = 210e-12
	NpOverNs = 2.0/9.0
	
	Rl = 100e3
	Rsrc = 600
	RsrcTerm = 360
	
	Fs = 44100.0 * 2
	
	dt = 1.0 / Fs
	t = numpy.arange(0.0, 0.10, dt)
	f0 = 1000.0
	A = 1.0
	#vins = A*numpy.sin(2.0*math.pi*f0*t)
	vins = numpy.zeros_like(t)
	vins[0] = 1.0	
	*/
	Real sampleRate = 44100.0 * 2; 
	Real R_Rload = 100e3;
	Real R_Source = 600;
	Real E_Source = 0.0;
	Real R_RsourceTermination = 360.0;
	Real L_Lp = 4e-3;
	Real R_Rp = 10.0;
	Real L_Lm = 20.0;
	Real R_Rc = 5e3;
	Real L_Ls = 1e-3;
	Real R_Rs = 50.0;
	Real C_Cw = 210e-12;
	Real NpOverNs = 2.0/9.0;
	""" + g.circuitName + """ rc(sampleRate, R_Rload, R_Source, E_Source, R_RsourceTermination, L_Lp, R_Rp, L_Lm, R_Rc, L_Ls, R_Rs, C_Cw, NpOverNs);	
	uint numTimeSteps = 20;
	cout << rc.advance(1.0) << endl;	
	for (uint i = 0; i < numTimeSteps; ++i){
		cout << rc.advance(0.0) << endl;
	}
}

"""
	middle =  g.GetCode()
	#print middle
	f = open('test2.cpp', 'w')
	f.write(start + middle + end)
	f.close()


def GeneratorNonIdealTransformerModel(parent, child, g, reverse=False): #parent = primary, child = secondary
	Lp = wdfgenerator.GeneratorWDFInductor('Lp', g)
	Rp = wdfgenerator.GeneratorWDFResistor('Rp', g)
	Lm = wdfgenerator.GeneratorWDFInductor('Lm', g)
	Rc = wdfgenerator.GeneratorWDFResistor('Rc', g)
	Ls = wdfgenerator.GeneratorWDFInductor('Ls', g)
	Rs = wdfgenerator.GeneratorWDFResistor('Rs', g)
	Cw = wdfgenerator.GeneratorWDFCapacitor('Cw', g)
	transformer = wdfgenerator.GeneratorWDFIdealTransformer('transformer', g)
	
	#Adaptors
	primaryInputSeriesConn = wdfgenerator.GeneratorWDFSeriesAdapter('primaryInputSeriesConn', g)
	primarySeriesConn2 = wdfgenerator.GeneratorWDFSeriesAdapter('primarySeriesConn2', g)
	primaryParallelConn1 = wdfgenerator.GeneratorWDFParallelAdapter('primaryParallelConn1', g)
	primaryParallelConn2 = wdfgenerator.GeneratorWDFParallelAdapter('primaryParallelConn2', g)
	secondarySeriesConn1 = wdfgenerator.GeneratorWDFSeriesAdapter('secondarySeriesConn1', g)
	secondarySeriesConn2 = wdfgenerator.GeneratorWDFSeriesAdapter('secondarySeriesConn2', g)
	secondaryOutputParallelConn = wdfgenerator.GeneratorWDFParallelAdapter('secondaryOutputParallelConn', g)

	#print seriesConn.childrensPorts[0].R
	#print seriesConn.childrensPorts[1].R

	secondaryOutputParallelConn.ConnectChild(child)
	secondaryOutputParallelConn.ConnectChild(Cw)
	secondarySeriesConn1.ConnectChild(secondaryOutputParallelConn)
	secondarySeriesConn2.ConnectChild(Rs)
	secondarySeriesConn2.ConnectChild(Ls)
	secondarySeriesConn1.ConnectChild(secondarySeriesConn2)
	transformer.ConnectChild(secondarySeriesConn1)
	primaryParallelConn1.ConnectChild(transformer)
	
	primarySeriesConn2.ConnectChild(Lp)
	primarySeriesConn2.ConnectChild(Rp)
	primaryParallelConn2.ConnectChild(Lm)
	primaryParallelConn2.ConnectChild(Rc)
	primaryParallelConn1.ConnectChild(primaryParallelConn2)
	
	primaryInputSeriesConn.ConnectChild(primarySeriesConn2)
	primaryInputSeriesConn.ConnectChild(primaryParallelConn1)
	
	parent.ConnectChild(primaryInputSeriesConn)
	
	return Cw, (Lp, Rp, Lm, Rc, Ls, Rs, Cw, transformer, primaryInputSeriesConn, primarySeriesConn2, primaryParallelConn1, primaryParallelConn2, secondarySeriesConn1, secondarySeriesConn2, secondaryOutputParallelConn)

class NonIdealTransformerModel(wavedigitalfilters.WDFTwoPort):
	def __init__(self, Lp, Rp, Rc, Lm, Rs, Ls, NpOverNs, sampleRate, Cw=None):
		#Components
		self.Lp = wavedigitalfilters.WDFInductor(Lp, sampleRate)
		self.Rp = wavedigitalfilters.WDFResistor(Rp)
		self.Lm = wavedigitalfilters.WDFInductor(Lm, sampleRate)
		self.Rc = wavedigitalfilters.WDFResistor(Rc)
		self.Ls = wavedigitalfilters.WDFInductor(Ls, sampleRate)
		self.Rs = wavedigitalfilters.WDFResistor(Rs)
		if Cw is not None:
			self.Cw = wavedigitalfilters.WDFCapacitor(Cw, sampleRate)
			self.hasOutputCapacitance = True
		else:
			self.hasOutputCapacitance = False
		self.transformer = wavedigitalfilters.WDFIdealTransformer(NpOverNs)
		
		#Adaptors
		self.primaryInputSeriesConn = wavedigitalfilters.WDFSeriesAdapter()
		self.primarySeriesConn2 = wavedigitalfilters.WDFSeriesAdapter()
		self.primaryParallelConn1 = wavedigitalfilters.WDFParallelAdapter()
		self.primaryParallelConn2 = wavedigitalfilters.WDFParallelAdapter()
		self.secondarySeriesConn1 = wavedigitalfilters.WDFSeriesAdapter()
		self.secondarySeriesConn2 = wavedigitalfilters.WDFSeriesAdapter()
		if Cw is not None:
			self.secondaryOutputParallelConn = wavedigitalfilters.WDFParallelAdapter()
		
	def GetR(self):
		return self.primaryInputSeriesConn.GetR()
	
	def Connect(self, parent):
		self.primaryInputSeriesConn.Connect(parent)
		
	def ConnectChild(self, child):
		#Connections
		#Make connections roughly in reverse order of signal flow in order to get the right propagation of the R values
		if self.hasOutputCapacitance:
			self.secondaryOutputParallelConn.ConnectChild(child)
			self.secondaryOutputParallelConn.ConnectChild(self.Cw)
			self.secondarySeriesConn1.ConnectChild(self.secondaryOutputParallelConn)
		else:
			self.secondarySeriesConn1.ConnectChild(child)
		self.secondarySeriesConn2.ConnectChild(self.Rs)
		self.secondarySeriesConn2.ConnectChild(self.Ls)
		self.secondarySeriesConn1.ConnectChild(self.secondarySeriesConn2)
		self.transformer.ConnectChild(self.secondarySeriesConn1)
		self.primaryParallelConn1.ConnectChild(self.transformer)
	
		self.primarySeriesConn2.ConnectChild(self.Lp)
		self.primarySeriesConn2.ConnectChild(self.Rp)
		self.primaryParallelConn2.ConnectChild(self.Lm)
		self.primaryParallelConn2.ConnectChild(self.Rc)
		self.primaryParallelConn1.ConnectChild(self.primaryParallelConn2)
	
		self.primaryInputSeriesConn.ConnectChild(self.primarySeriesConn2)
		self.primaryInputSeriesConn.ConnectChild(self.primaryParallelConn1)

		#print "TX R", self.transformer.R
		#print "S conn 0 R", self.primaryParallelConn1.childrensPorts[0].R
		#print "S conn 1 R", self.primaryParallelConn1.childrensPorts[1].R
			
	def SetA(self, parentsA):
		self.primaryInputSeriesConn.SetA(parentsA)
	
	def GetB(self): #Gets parent side B
		return self.primaryInputSeriesConn.GetB()

def PlotSpectrum(x, sampleRate, showPhase=False, limitAxes=True):
	x = numpy.fft.fft(x)
	f = numpy.fft.fftfreq(len(x))
	f = numpy.abs(f)*sampleRate
	ll = math.ceil(len(x)/2)-100
	h = f[:ll]
	w = x[:ll]	

	wdB = 10.0*numpy.log10(numpy.abs(w))
	peakGain = wdB.max()
	inband = wdB > peakGain - 1.0
	print "Fmin @ -1dB", f[inband.nonzero()[0][0]]
	print "Fmax @ -1dB", f[inband.nonzero()[0][-1]]
	
	print "Av =", numpy.abs(w).max(), "=", peakGain, 'db'
	x40HzIndex = (numpy.abs(f - 40.0)).argmin()
	print "gain at 40Hz:", wdB[x40HzIndex]
	
	
	fig = plt.figure()
	plt.title('Frequency response')
	ax1 = fig.add_subplot(111)
	
	plt.semilogx(h, wdB, 'b')
	plt.ylabel('Amplitude (dB)', color='b')
	plt.xlabel('Frequency (Hz)')
	plt.grid()
	if limitAxes:
		plt.axis([10.0, 25e3, 0.0, 10.0])
	plt.legend()
	
	if showPhase:
		ax2 = ax1.twinx()
		angles = numpy.angle(w)#numpy.unwrap(numpy.angle(w))
		plt.plot(h, angles, 'g')
		plt.ylabel('Angle (radians)', color='g')
	plt.show()

def NiceTX1():
	Lp = 4e-3
	Rp = 10.0
	Rc = 5e3
	Lm = 5.7
	Rs = 50.0
	Ls = 1e-3
	Cw = 210e-12
	NpOverNs = 1.0/9.0
	
	Rl = 1000e3
	Rsrc = 600

def TX17to1forSidechain():
	Lp = 2e-3
	Rp = 10.0
	Rc = 50e3
	Lm = 5.7
	Rs = 50.0
	Ls = 1e-3
	Cw = 10e-12
	NpOverNs = 1.0/17.0
	
	Rl = 73e3
	Rsrc = 600

		
if __name__ == '__main__':
	GenerateNonIdealTransformerModelCode()
	import sys
	raise sys.exit(0)

	Lp = 4e-3
	Rp = 10.0
	Rc = 5e3
	Lm = 20.0
	Rs = 50.0
	Ls = 1e-3
	Cw = 210e-12
	NpOverNs = 2.0/9.0
	
	Rl = 100e3
	Rsrc = 600
	RsrcTerm = 360
	
	Fs = 44100.0 * 2
	
	dt = 1.0 / Fs
	t = numpy.arange(0.0, 0.10, dt)
	f0 = 1000.0
	A = 1.0
	#vins = A*numpy.sin(2.0*math.pi*f0*t)
	vins = numpy.zeros_like(t)
	vins[0] = 1.0

	Rload = wavedigitalfilters.WDFResistor(Rl)
	Source = wavedigitalfilters.WDFResistiveSource(0.0, Rsrc)
	RsourceTermination = wavedigitalfilters.WDFResistor(RsrcTerm)
	seriesConn = wavedigitalfilters.WDFSeriesAdapter()
	parallelConn = wavedigitalfilters.WDFSeriesAdapter()
	tx = NonIdealTransformerModel(Lp, Rp, Rc, Lm, Rs, Ls, NpOverNs, Fs, Cw=Cw)
	#tx = wavedigitalfilters.WDFIdealTransformer(NpOverNs)
	tx.ConnectChild(Rload)
	parallelConn.ConnectChild(tx)
	parallelConn.ConnectChild(RsourceTermination)
	seriesConn.ConnectChild(parallelConn)
	seriesConn.ConnectChild(Source)

	#print seriesConn.childrensPorts[0].R
	#print seriesConn.childrensPorts[1].R

	vouts = []
	for vin in vins:
		Source.SetE(vin)
		a = seriesConn.GetB()
		b = - a #short circuit rule
		seriesConn.SetA(b)
		vout = Rload.GetVoltage()
		#print vin, vout#, tx.Cw.b, tx.Cw.a
		vouts.append(vout) 
		print vout
	vouts = numpy.array(vouts)
	
	print "Turns ratio gain:", 1.0 / NpOverNs, '=', 10*math.log10(1.0 / NpOverNs), 'dB'
	PlotSpectrum(vouts, Fs, limitAxes=True)
	
	
	plt.figure()
	plt.plot(t, vins)
	plt.plot(t, vouts)
	plt.show()
	b = raw_input()
