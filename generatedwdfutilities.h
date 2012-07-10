#ifndef GENERATEDWDFUTILITIES_H
#define GENERATEDWDFUTILITIES_H

#include "Misc.h"
#include "tubemodel.h"

class BidirectionalUnitDelay;

class BidirectionalUnitDelayInterface {
public:
	friend class BidirectionalUnitDelay;
	void setA(Real a_){ a = a_; }
	Real getB() { return b;}
protected:
	Real a;
	Real b;
};

class BidirectionalUnitDelay {
public:
	BidirectionalUnitDelayInterface* getInterface(uint index){
		if (index == 0){
			return &interface0;
		}
		return &interface1;		
	}
	void advance(){
		interface0.b = interface1.a;
		interface1.b = interface0.a;
	}
protected:
	BidirectionalUnitDelayInterface interface0;
	BidirectionalUnitDelayInterface interface1;
};


#endif