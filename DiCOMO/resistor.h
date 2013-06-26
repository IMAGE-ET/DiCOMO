//
//  resistor.h
//  DiCOMO
//
//  Created by Maximilian Zangs on 09.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#ifndef __DiCOMO__resistor__
#define __DiCOMO__resistor__

#include "element.h"

using namespace std;

class Resistor : public Element {
protected:
    // State that stores the impedance
    state _resistorState;
    
    // All parameters for the left port
    state _leftPortVoltage;
    state _leftPortCurrent;
    port _leftPort;
    
    // All parameters for the right port
    state _rightPortVoltage;
    state _rightPortCurrent;
    port _rightPort;
    
    // Variable that buffers the impedance from currently interrogated
    // neighboring elements. Implemented to speed up algorithm propagation
    complex<double> _impedanceBeyondResistor;
    
public:
    Resistor(complex<double> vcc, complex<double> vss);
    ~Resistor();
    
    // Functions that set and the impedance for this resistor
    void setImpedance(complex<double> impedance);
    complex<double> getImpedance();
    
protected:
    // Function that returns the impedance in the direction of a port
    // i.e. the impedance of the adjacent circuit
    // Function is virtual since it may be overwritten by inheriting classes
    virtual complex<double> getImpedanceInDirectionOf(string mine);
    
    // These two functions are used in the "getImpedanceBehind(...)" function
    // and are defined separately since they call themselves recursively
    complex<double> getParallelImpedances(vector<Element *>elements, string mine);
    complex<double> getSeriesImpedances(Element *element, string mine);
    
public:
    // The Resistor implementation of getNewState
    virtual vector<Element *>getNewState();
};

#endif /* defined(__DiCOMO__resistor__) */
