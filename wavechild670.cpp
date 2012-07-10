#include "Wavechild670.h"

const Real Wavechild670::levelTimeConstantCircuitComponentValues[6][6] = {
	/* C1,    C2,   C3,   R1,   R2,    R3 */
	{ 2e-6, 8e-6, 20e-6, 51.9e3, 10e9, 10e9 },
	{ 2e-6, 8e-6, 20e-6, 149.9e3, 10e9, 10e9 },
	{ 4e-6, 8e-6, 20e-6, 220e3, 10e9, 10e9 },
	{ 8e-6, 8e-6, 20e-6, 220e3, 10e9, 10e9 },
	{ 4e-6, 8e-6, 20e-6, 220e3, 100e3, 10e9 },
	{ 2e-6, 8e-6, 20e-6, 220e3, 100e3, 100e3 }};