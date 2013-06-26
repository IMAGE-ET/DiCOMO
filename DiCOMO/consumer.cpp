//
//  consumer.cpp
//  DiCOMO
//
//  Created by Maximilian Zangs on 09.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#include "consumer.h"

Consumer::Consumer(complex<double> vcc, complex<double> vss) : Resistor(vcc, vss) {
    // Sets up element type
    _elementType = CONSUMER;
    
    // Default power consumption to zero
    _consumerState.name = POWER;
    _consumerState.value = complex<double>(0.0, 0.0);
    
    setImpedance(complex<double>(INFINITY, 0.0));
    
}

Consumer::~Consumer() {
    
}

void Consumer::setPower(complex<double> power) {
    // Assign power
    _consumerState.value = power;
}

void Consumer::setPower(double power, double powerFactor, bool isInductive) {
    // Check if power factor is valid
    if (powerFactor < 0 || powerFactor > 1.0) {
        // Power factor must lie between 0 and 1
        cout << "ERROR : Invalid power factor of <" << powerFactor << "> for element <" << elementName() << ">" << endl;
        exit(-1);
    }
    
    // And compute true and reactive power for complex number power
    double truePower = power * powerFactor;
    double reactivePower = (isInductive
                            ? sqrt(power*power - truePower*truePower)
                            : -sqrt(power*power - truePower*truePower));
    setPower(complex<double>(truePower, reactivePower));
}

complex<double> Consumer::getPower() {
    // Get power
    return _consumerState.value;
}

complex<double> Consumer::getImpedanceInDirectionOf(string mine) {
    
    // If the apparent power consumption is zero then return an open circuit
    if (getPower().real() == 0.0 && getPower().imag() == 0.0)
        return complex<double> (INFINITY, 0.0);
    
    // Check if there is current flowing
    if (getPortParameter(PORT_L, CURRENT).real() == 0.0
        && getPortParameter(PORT_L, CURRENT).imag() == 0.0)
        return complex<double> (INFINITY, 0.0);
    
    if (mine == PORT_R) {
        return (getPortParameter(PORT_L, VOLTAGE) - _vss)
        / getPortParameter(PORT_L, CURRENT);
    } else {
        return (_vcc - getPortParameter(PORT_R, VOLTAGE))
        / getPortParameter(PORT_L, CURRENT);
    }
}

vector<Element *> Consumer::getNewState() {
    vector<Element *> nextElements;
    
    bool isLeftVoltageFixed = isPortParameterFixed(PORT_L, VOLTAGE);
    bool isRightVoltageFixed = isPortParameterFixed(PORT_R, VOLTAGE);
    
    bool isLeftVoltageSet = isPortParameterSet(PORT_L, VOLTAGE);
    bool isRightVoltageSet = isPortParameterSet(PORT_R, VOLTAGE);
    
    // If either:
    //      left voltages has been fixed    and     right voltage is fixed
    // OR   left voltage is fixed           and     right voltage has been set
    // OR   left voltage has been set       and     right voltage is fixed
    // OR   left voltage has been set       and     right voltage has been set
    // THEN execute R = V^2 / P and I = V / R
    //
    // This was the code only executes if both sides of the element have been
    // evaluated, ensuring they correlate before updating
    if ((isLeftVoltageFixed && isRightVoltageFixed)
        || (isLeftVoltageFixed && isRightVoltageSet)
        || (isLeftVoltageSet && isRightVoltageFixed)
        || (isLeftVoltageSet && isRightVoltageSet)) {
        
        complex<double> newImpedance = complex<double> (INFINITY, 0.0);
        complex<double> current = complex<double> (0.0, 0.0);
        
        // Make sure there is power flowing, else leave as open circuit
        if (getPower().real() != 0.0 || getPower().imag() != 0.0) {
            // Compute "absolute impedance" and then rotate to meet the power factor
            
            complex<double>voltageL = getPortParameter(PORT_L, VOLTAGE);
            complex<double>voltageR = getPortParameter(PORT_R, VOLTAGE);
            
            newImpedance = abs( pow(voltageL - voltageR , 2) / getPower());
            
#warning No impedance rotation to meet non unity power factor implemented
            
            current = (getPortParameter(PORT_L, VOLTAGE)
                       - getPortParameter(PORT_R, VOLTAGE)) / newImpedance;
        }
        
        setImpedance(newImpedance);
        
        setPortParameter(PORT_L, CURRENT, current);
        setPortParameter(PORT_R, CURRENT, -current);
        
    }
    
    return nextElements;
}

