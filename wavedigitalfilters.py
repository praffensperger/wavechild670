##########################################################################################
# 
# Wavechild670 v0.1 
# 
# wavedigitalfilters.py
# 
# By Peter Raffensperger 11 March 2014
# 
# Reference:
# Toward a Wave Digital Filter Model of the Fairchild 670 Limiter, Raffensperger, P. A., (2012). 
# Proc. of the 15th International Conference on Digital Audio Effects (DAFx-12), 
# York, UK, September 17-21, 2012.
# 
# Note:
# Fairchild (R) a registered trademark of Avid Technology, Inc., which is in no way associated or 
# affiliated with the author.
# 
# License:
# Wavechild670 is licensed under the GNU GPL v2 license. If you use this
# software in an academic context, we would appreciate it if you referenced the original
# paper.
# 
##########################################################################################
"""
Wave digital filters
Peter Raffensperger
2012

Rules: 
- Two components are connected by feeding one's a into the other's b and vice versa
- Two components that are connected must have the same value of R
- The connected components form a directed binary connection tree. Start computation of the 'b's at 
  the leaves and then work to the trunk. Then compute the other way to calculate the 'a's.
- If you have a non-linear element, you have to put it at the trunk of the tree.

References:
A. Fettweis, "Wave Digital Filters: Theory and Practice,"
Proc. of the IEEE, Vol. 74, No. 2, pp. 270-327, 1986.

"""

import math


class WDFPort():
	def __init__(self, R):
		self.a = 0.0
		self.b = 0.0
		self.R = R
		self.mate = None
	def SetA(self, a):
		self.a = a
	def GetB(self):
		return self.b
	def GetR(self):
		return self.R
	def Connect(self, other):
		assert(other.GetR() == self.R)
		self.mate = other
	def GetVoltage(self):
		"""
		a = v + Ri
		b = v - Ri
		v = a + b
		"""
		return -(self.a + self.b)


class WDFResistor(WDFPort):
	"""
	V = RI
	b = 0
	"""
	def __init__(self, R):
		WDFPort.__init__(self, R)

class WDFCapacitor(WDFPort):
	"""
	V = IR/phi
	b = a*z^-1
	
	R = 1/(2*C*fs)
	"""
	def __init__(self, C, sampleRate):
		R = 1.0 / (2.0*C*sampleRate)
		WDFPort.__init__(self, R)
	def GetB(self):
		self.b = self.a
		return self.b
		
class WDFInductor(WDFPort):
	"""
	V = phi*IR
	b = -a*z^-1
	
	R = 2*L*fs
	"""
	def __init__(self, L, sampleRate):
		R = 2.0*L*sampleRate
		WDFPort.__init__(self, R)

	def GetB(self):
		self.b = -1.0*self.a
		return self.b
		
class WDFResistiveSource(WDFPort): #Voltage source
	"""
	V = E + IR
	b = E
	
	"""
	def __init__(self, E, R):
		WDFPort.__init__(self, R)
		self.b = E
	def SetE(self, E):
		self.b = E

class WDFIdealVoltageSource(WDFPort): #Voltage source
	"""
	b = 2e - a 
	"""
	def __init__(self, e, R): #Value of R doesn't matter
		WDFPort.__init__(self, R)
		self.e = e
	def SetE(self, e):
		self.e = e
	def GetB(self):
		a = self.mate.b #I hope the timing of this works right!
		return 2.0*self.e - a

# class WDFResistiveCurrentSource(WDFPort):
# 	"""
# 	source current = e / R
# 	b = a - 2e
# 	
# 	"""
# 	def __init__(self, sourceCurrent, R):
# 		WDFPort.__init__(self, R)
# 		self.b = E
# 	def SetSourceCurrent(self, E):
# 		self.b = E
		
class WDFOpenCircuit(WDFPort):
	"""
	b = a
	"""
	def __init__(self, R):
		WDFPort.__init__(self, R)
	def GetB(self):
		import warnings
		warnings.warn("This might muck up the passage of time for L's and C's")
		return self.mate.GetB()

class WDFShortCircuit(WDFPort):
	"""
	b = -a
	"""
	def __init__(self, R):
		WDFPort.__init__(self, R)
	def GetB(self):
		import warnings
		warnings.warn("This might muck up the passage of time for L's and C's")
		return -self.mate.GetB()
		
class WDFTwoPort(WDFPort):
	"""
	Signal flow is from the parent to the child
	"""
	def __init__(self, Rparent, Rchild):
		WDFPort.__init__(self, Rparent)
		self.childPort = WDFPort(Rchild)
	def Connect(self, parent):
		WDFPort.Connect(self, parent)
	def ConnectChild(self, child):
		self.childPort.Connect(child)
	def SetA(self, parentA):
		self.a = parentA
		self.ComputeChildB() 
		self.childPort.mate.SetA(self.childPort.b)
	def GetB(self): #Gets parent side B
		self.childPort.a = self.childPort.mate.GetB()
		self.ComputeParentB()
		return self.b
	def ComputeChildB(self):
		self.childPort.b = self.a #Pass through
	def ComputeParentB(self):
		self.b = self.secondaryPort.a #Pass through

class WDFBidirectionalUnitDelay(): #Use for breaking delay free loops and things
	def __init__(self, R):
		self.ports = [WDFPort(R), WDFPort(R)]
	def GetPort(self, portIndex):
		return self.ports[portIndex]
	def Advance(self):
		for p, q in zip([0, 1], [1, 0]):
			self.ports[p].b = self.ports[q].a

class WDFIdealTransformer(WDFTwoPort):
	"""
	Primary = Parent
	Secondary = Child
	"""
	def __init__(self, NpOverNs):
		self.n = 1.0 / NpOverNs		
		WDFTwoPort.__init__(self, 1.0, 1.0)
	def ConnectChild(self, child):
		Rs = child.R
		self.R = Rs/(self.n**2)
		#
		self.childPort.R = Rs
		WDFTwoPort.ConnectChild(self, child)
		#self.childPort.Connect(child)
	def ComputeChildB(self):
		self.childPort.b = self.a*self.n
	def ComputeParentB(self):
		self.b = self.childPort.a/self.n

class WDFInterconnect(WDFPort):
	"""
	Port 3 is reflection free
	"""
	def __init__(self):
		self.children = []
		self.childrensPorts = [WDFPort(1.0), WDFPort(1.0)]
		self.gamma1 = None
		WDFPort.__init__(self, 1.0)
	def ConnectChild(self, child):
		childIndex = len(self.children)
		assert(childIndex <= 1)
		self.children.append(child)
		self.childrensPorts[childIndex].R = child.GetR()
		self.childrensPorts[childIndex].Connect(child)
	
	def SetA(self, parentsA):
		self.a = parentsA
		for childIndex in [0, 1]:
			#self.ports[childIndex].b = self.ComputeB1Or2(childIndex) #The theoretically proper way
			#self.children[childIndex].SetA(self.ports[childIndex].GetB())
			self.children[childIndex].SetA(self.ComputeB1Or2(childIndex)) #Just cut to the chase
		
	def GetB(self):
		for childIndex in [0, 1]:
			self.childrensPorts[childIndex].SetA(self.children[childIndex].GetB())
		self.b = self.ComputeB3()
		return self.b
		
	def GetAs(self):
		a1 = self.childrensPorts[0].a
		a2 = self.childrensPorts[1].a
		a3 = self.a
		return a1, a2, a3
		
	def ComputeB3(self):
		return 0.0
	def ComputeB1Or2(self, childIndex):
		return 0.0

	
	
class WDFParallelAdapter(WDFInterconnect):
	"""
	Port 3 is reflection free
	"""
	def ConnectChild(self, child):
		WDFInterconnect.ConnectChild(self, child)
		if len(self.children) == 2:
			G1 = 1.0 / self.childrensPorts[0].GetR() 
			G2 = 1.0 / self.childrensPorts[1].GetR()
			G3 = G1 + G2
			self.R = 1.0 / G3
			self.gamma1 = G1 / G3


	def ComputeB1Or2(self, childIndex):
		a1, a2, a3 = self.GetAs()
		if childIndex == 0:
			b1 = a3 + a2 - a1 - self.gamma1*(a2 - a1)
			return b1
		else:
			b2 = a3 - self.gamma1*(a2 - a1)
			return b2

	def ComputeB3(self):
		a1, a2, a3 = self.GetAs()
		b3 = a2 - self.gamma1*(a2 - a1)
		return b3

class WDFSeriesAdapter(WDFInterconnect):
	"""
	Port 3 is reflection free
	"""
	def ConnectChild(self, child):
		WDFInterconnect.ConnectChild(self, child)
		if len(self.children) == 2:
			R1 = self.childrensPorts[0].GetR() 
			R2 = self.childrensPorts[1].GetR()
			R3 = R1 + R2
			self.R = R3
			self.gamma1 = R1 / R3
			#print self.gamma1

	def ComputeB1Or2(self, childIndex):
		a1, a2, a3 = self.GetAs()
		if childIndex == 0:
			b1 = a1 - self.gamma1*(a1 + a2 + a3)
			return b1
		else:
			b2 = -1.0*(a1 + a3 - self.gamma1*(a1 + a2 + a3))
			return b2
		
	def ComputeB3(self):
		a1, a2, a3 = self.GetAs()		
		b3 = -1.0*(a1 + a2)
		return b3
		
def RCHandDSP(numTimeSteps):
	sampleRate = 1000.0
	R = 1000.0
	C = 10e-6
	R2 = 1.0 / (2.0*C*sampleRate)
	R1 = R - R2
	R3 = R1 + R2
	
	e = 1.0
	
	y1 = R1 / R3
	#print y1
	b1 = 0.0
	b1Last = 0.0
	for t in range(numTimeSteps):
		b1 = (2.0*e*(1.0 - y1) + y1*b1Last) / (2.0 - y1)
		output = -2.0*e + b1 + b1Last
		a1 = 2*e - b1
		a2 = -1.0*b1Last
		b1s = a1 + -y1*(a1 + a2)
		vc = -1.0*b1-b1Last
		print a1, a2, b1s, b1, output, vc, vc + output
		b1Last = b1

def RCWDF(numTimeSteps):
	sampleRate = 1000.0
	#V = WDFResistiveSource(10, 100)
	R = WDFResistor(1000)
	C = WDFCapacitor(10e-6, sampleRate)
	Conn = WDFSeriesAdapter()
	Conn.ConnectChild(R)
	Conn.ConnectChild(C)
	
	for t in range(numTimeSteps):
		e = 1.0
		a = Conn.GetB()
		b = 2*e - a
		Conn.SetA(b)
		print a, b, R.GetVoltage()


def RCWDFParallel():
	sampleRate = 10000.0
	#V = WDFResistiveSource(10, 100)
	R = WDFResistor(10000)
	C = WDFCapacitor(10e-9, sampleRate)
	Conn = WDFParallelAdapter()
	Conn.ConnectChild(R)
	Conn.ConnectChild(C)
	print Conn.R
	
	for t in range(100):
		e = 0.5
		a = Conn.GetB()
		b = e
		Conn.SetA(b)
		print a, b, R.GetVoltage()
		
if __name__ == '__main__':
	import time
	numTimeSteps = 20
	for fn, msg in (RCHandDSP, "Hand"), (RCWDF, "WDF"):
		print msg
		start = time.time()
		fn(numTimeSteps)
		end = time.time()
		print "Time take =", end - start

	#RCWDFParallel()
	#import sys
	#raise sys.exit(0)