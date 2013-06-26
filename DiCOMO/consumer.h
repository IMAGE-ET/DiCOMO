//
//  consumer.h
//  DiCOMO
//
//  Created by Maximilian Zangs on 09.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#ifndef __DiCOMO__consumer__
#define __DiCOMO__consumer__

#include "resistor.h"

using namespace std;

class Consumer : public Resistor {
protected:
    // The consumer class encapsulates an element that updates its port values
    // dependent on the 
    state _consumerState;
    
public:
    Consumer(complex<double> vcc, complex<double> vss);
    ~Consumer();
    
    void setPower(complex<double> power);
    void setPower(double power, double powerFactor, bool isInductive = true);
    complex<double> getPower();
protected:
    
    // Consumer's implementation of impedance acquiring function.
    // Unlike Resistor, Consumer implements the circuit-splitting feature
    // as described in SE4RP11 Report & Paper
    virtual complex<double> getImpedanceInDirectionOf(string mine);

public:
    // Consumer's implementation of updating function.
    // Unlike Resistor, Consumer implements the circuit-splitting feature
    // as described in SE4RP11 Report & Paper and also updates the port voltages
    // iteratively
    virtual vector<Element *>getNewState();
};

#endif /* defined(__DiCOMO__consumer__) */
