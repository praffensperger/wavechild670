import sys

import wdfgenerator
import transformermodel

def GenerateTransformerCoupledInputCircuit():
	g = wdfgenerator.Generator('TransformerCoupledInputCircuit')
	g.HeaderCommentBlock("""Circuit schematic:
	                                Primary  Secondary
	                                    Np  :   Ns
	 ----Rin-------Lp---Rp--------------B       B------Rs----Ls------------
	 |         |            |   |       B       B               |         |
	Vin     RinTerm         Rc  Lm      B TXFMR B               Cw      Rload
	 |         |            |   |       B       B               |         |
	 -----------------------------------B       B--------------------------
	B = winding symbol
	""")
	Rload = wdfgenerator.GeneratorWDFResistor('Rload', g)
	inputSource = wdfgenerator.GeneratorWDFResistiveSource('inputSource', g)
	RinputTermination = wdfgenerator.GeneratorWDFResistor('RinputTermination', g)
	seriesConn = wdfgenerator.GeneratorWDFSeriesAdapter('seriesConn', g)
	parallelConn = wdfgenerator.GeneratorWDFSeriesAdapter('parallelConn', g)

	Cw, parts = transformermodel.GeneratorNonIdealTransformerModel(parallelConn, Rload, g)

	parallelConn.ConnectChild(RinputTermination)
	seriesConn.ConnectChild(parallelConn)
	seriesConn.ConnectChild(inputSource)

	g.Input('vin')
	g.ForwardCalculation('inputSourceE = vin')
	g.ForwardCalculation('//Get Bs')
	a = seriesConn.GetB()
	b = '-(' + a + ')' #short circuit rule
	g.ForwardCalculation('b = ' + b)
	g.ForwardCalculation('//Set As')
	seriesConn.SetA('b')
	g.Output(Cw.GetVoltage())
	return g.GetCode()

def GenerateTubeStageCircuit():
	g = wdfgenerator.Generator('TubeStageCircuit')
	g.HeaderCommentBlock("""Circuit schematic: (B = winding symbol)
	         VplateSource
	          |
	          Rplate                      Primary  Secondary
	          |                            Np  :   Ns
	          --------Lp---Rp--------------B       B------Rs----Ls------------------------
	                           |   |       B       B               |         |           |
	                           Rc  Lm      B TXFMR B               Cw      Routput   Rsidechain
	                           |   |       B       B               |         |           |
	          -----------------------------B       B--------------------------------------
	          |
	Vgate --Tube
	          |
	        -----------------
	        |               |
	        Rcathode        Ccathode
	        |               |
	       VcathodeSource   cathodeCapacitorConn
	""")
	
	#Parts
	cathodeSource = wdfgenerator.GeneratorWDFResistiveSource('VcathodeBias', g)
	Ccathode = wdfgenerator.GeneratorWDFCapacitor('Ccathode', g)
	
	plateSource = wdfgenerator.GeneratorWDFResistiveSource('Vplate', g)
	
	Routput = wdfgenerator.GeneratorWDFResistor('Routput', g)
	Rsidechain = wdfgenerator.GeneratorWDFResistor('Rsidechain', g)
	
	#Wiring
	tubeSeriesConn1 = wdfgenerator.GeneratorWDFSeriesAdapter('tubeSeriesConn1', g)
	tubeSeriesConn2 = wdfgenerator.GeneratorWDFSeriesAdapter('tubeSeriesConn2', g)
	outputParallelConn = wdfgenerator.GeneratorWDFParallelAdapter('outputParallelConn', g)
	cathodeParallelConn = wdfgenerator.GeneratorWDFParallelAdapter('cathodeParallelConn', g)
	cathodeCapSeriesConn = wdfgenerator.GeneratorWDFSeriesAdapter('cathodeCapSeriesConn', g)
	
	#Connections
	outputParallelConn.ConnectChild(Routput)
	outputParallelConn.ConnectChild(Rsidechain)
	Cw, parts = transformermodel.GeneratorNonIdealTransformerModel(tubeSeriesConn1, outputParallelConn, g)
	
	cathodeCapacitorConn = wdfgenerator.GeneratorWDFExternal('cathodeCapacitorConn', g, 'cathodeCapacitorConn->setA(cathodeCapacitorConna)', 'cathodeCapacitorConn->getB()')
	cathodeCapSeriesConn.ConnectChild(Ccathode)
	cathodeCapSeriesConn.ConnectChild(cathodeCapacitorConn)
	
	cathodeParallelConn.ConnectChild(cathodeSource)
	#cathodeParallelConn.ConnectChild(Ccathode)
	cathodeParallelConn.ConnectChild(cathodeCapSeriesConn)	
	
	tubeSeriesConn1.ConnectChild(plateSource)
	
	tubeSeriesConn2.ConnectChild(tubeSeriesConn1)
	tubeSeriesConn2.ConnectChild(cathodeParallelConn)
	#tube = wdfgenerator.GeneratorWDFExternal('tube', g, 'tube->setA(tubeA, tubeR)', 'tube->getB()')
	#tube.Connect(tubeSeriesConn2)
	g.ConstructorItem('tube', 'tube(tube_)', 'WDFTubeInterface', parameter='tube_', reference=True)		
	g.RValue('cathodeCapacitorConn', 'cathodeCapacitorConn = cathodeCapacitorConn_', parameter='cathodeCapacitorConn_', type='BidirectionalUnitDelayInterface*')
	g.RValue('tubeR', 'tubeR = tubeSeriesConn2_3R')	
	g.RCheck('LOG_INFO(" ")')
	g.RCheck('LOG_INFO("outputParallelConn_3Gamma1=" << outputParallelConn_3Gamma1)')
	g.RCheck('LOG_INFO("cathodeCapSeriesConn_3Gamma1=" << cathodeCapSeriesConn_3Gamma1)')
	g.RCheck('LOG_INFO("cathodeParallelConn_3Gamma1=" << cathodeParallelConn_3Gamma1)')
	g.RCheck('LOG_INFO("tubeSeriesConn1_3Gamma1=" << tubeSeriesConn1_3Gamma1)')
	g.RCheck('LOG_INFO("tubeSeriesConn2_3Gamma1=" << tubeSeriesConn2_3Gamma1)')
	g.RCheck('LOG_INFO(" ")')
	g.RCheck('LOG_INFO("primarySeriesConn2_3Gamma1=" << primarySeriesConn2_3Gamma1)')
	g.RCheck('LOG_INFO("secondarySeriesConn2_3Gamma1=" << secondarySeriesConn2_3Gamma1)')
	g.RCheck('LOG_INFO("primaryInputSeriesConn_3Gamma1=" << primaryInputSeriesConn_3Gamma1)')
	g.RCheck('LOG_INFO("secondaryOutputParallelConn_3Gamma1=" << secondaryOutputParallelConn_3Gamma1)')
	g.RCheck('LOG_INFO("secondarySeriesConn1_3Gamma1=" << secondarySeriesConn1_3Gamma1)')
	g.RCheck('LOG_INFO("primaryParallelConn2_3Gamma1=" << primaryParallelConn2_3Gamma1)')
	g.RCheck('LOG_INFO("primaryParallelConn1_3Gamma1=" << primaryParallelConn1_3Gamma1)')		

	g.StateVariable('Vcathode')
	#g.Destructor('\t\tdelete tube;\n')
	g.Input('vgate')
	g.ForwardCalculation('//Get Bs')	
	a = tubeSeriesConn2.GetB()
	g.ForwardCalculation('//Call tube model')	
	g.ForwardCalculation('b = tube.getB(' + a + ', tubeR, vgate, Vcathode)')	
	g.AddBareCode('SCOPE("Vak", -(tubeSeriesConn2_3b3 + b));')
	g.AddBareCode('SCOPE("VplateE", VplateE);')	
	g.ForwardCalculation('//Set As')	
	tubeSeriesConn2.SetA('b')
	g.ForwardCalculation('Vcathode = ' + Ccathode.GetVoltage())
	g.AddBareCode('SCOPE("Vcathode", Vcathode);')
	g.AddBareCode('SCOPE("Va", -(tubeSeriesConn2_3b3 + b) + Vcathode);')

	g.Output(Cw.GetVoltage())

	return g.GetCode()

def GenerateLevelTimeConstantCircuit():
	g = wdfgenerator.Generator('LevelTimeConstantCircuit')
	g.HeaderCommentBlock("""Circuit schematic:
	 --------------------------
	 |       |    |     |     |
	 |       |    |     R2    R3
	Iin      R1   C1    |     |
	 |       |    |     C2    C3
	 |       |    |     |     |
	 --------------------------
	""")

	R1 = wdfgenerator.GeneratorWDFResistor('R1', g)
	C1 = wdfgenerator.GeneratorWDFCapacitor('C1', g)
	R2 = wdfgenerator.GeneratorWDFResistor('R2', g)
	C2 = wdfgenerator.GeneratorWDFCapacitor('C2', g)
	R3 = wdfgenerator.GeneratorWDFResistor('R3', g)
	C3 = wdfgenerator.GeneratorWDFCapacitor('C3', g)

	#Wires
	parallelConnInput = wdfgenerator.GeneratorWDFParallelAdapter('parallelConnInput', g)
	parallelConn1 = wdfgenerator.GeneratorWDFParallelAdapter('parallelConn1', g)
	parallelConn23 = wdfgenerator.GeneratorWDFParallelAdapter('parallelConn23', g)
	serialConn2 = wdfgenerator.GeneratorWDFSeriesAdapter('serialConn2', g)
	serialConn3 = wdfgenerator.GeneratorWDFSeriesAdapter('serialConn3', g)		

	serialConn2.ConnectChild(R2)
	serialConn2.ConnectChild(C2)
	serialConn3.ConnectChild(R3)
	serialConn3.ConnectChild(C3)
	parallelConn23.ConnectChild(serialConn2)
	parallelConn23.ConnectChild(serialConn3)
	parallelConn1.ConnectChild(R1)
	parallelConn1.ConnectChild(C1)
	parallelConnInput.ConnectChild(parallelConn1)
	parallelConnInput.ConnectChild(parallelConn23)

	g.RValue('Rsource', 'Rsource = ' + parallelConnInput.GetR())

	g.Input('Iin')
	a = parallelConnInput.GetB()
	g.ForwardCalculation('//Current source law')
	g.ForwardCalculation('e = Iin * Rsource')
	g.ForwardCalculation('b = (' + a + ') - 2.0*e')
	parallelConnInput.SetA('b')
	g.Output(C1.GetVoltage())
	return g.GetCode()
	
if __name__ == '__main__':
	outputFile = 'wdfcircuits.h'
	f = open(outputFile, 'w')
	preamble = """//Autogenerated wave digital filter circuits
//Peter Raffensperger
#ifndef WDFCIRCUITS_H
#define WDFCIRCUITS_H
#include "generatedwdfutilities.h"
#include "scope.h"

"""
	postamble = """
#endif

"""

	f.write(preamble)
	f.write(GenerateTubeStageCircuit())
	f.write(GenerateTransformerCoupledInputCircuit())
	f.write(GenerateLevelTimeConstantCircuit())
	f.write(postamble)