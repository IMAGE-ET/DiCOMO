//
//  resistor.cpp
//  DiCOMO
//
//  Created by Maximilian Zangs on 09.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#include "resistor.h"

Resistor::Resistor(complex<double> vcc, complex<double> vss) : Element(vcc, vss) {
    // Defines element as Resistor
    _elementType = RESISTOR;
    
    // Setup impedance parameter
    _resistorState.name = IMPEDANCE;
    _resistorState.value = complex<double>(INFINITY, 0.0);
        
    
    // Setup left port with all parameters
    _leftPortCurrent.name = CURRENT;
    _leftPortCurrent.value = complex<double>(0.0, 0.0);
    _leftPortCurrent.isGiven = false;
    _leftPortCurrent.isSet = false;
    
    _leftPortVoltage.name = VOLTAGE;
    _leftPortVoltage.value = complex<double>(0.0, 0.0);
    _leftPortVoltage.isGiven = false;
    _leftPortVoltage.isSet = false;
    
    _leftPort.name = PORT_L;
    _leftPort.ptrElementThatOwnsPort = this;
    _leftPort.isConnected = false;
    _leftPort.portParameters.push_back(&_leftPortCurrent);
    _leftPort.portParameters.push_back(&_leftPortVoltage);
    
    
    // Setup right port with all parameters
    _rightPortCurrent.name = CURRENT;
    _rightPortCurrent.value = complex<double>(0.0, 0.0);
    _rightPortCurrent.isGiven = false;
    _rightPortCurrent.isSet = false;
    
    _rightPortVoltage.name = VOLTAGE;
    _rightPortVoltage.value = complex<double>(0.0, 0.0);
    _rightPortVoltage.isGiven = false;
    _rightPortVoltage.isSet = false;
    
    _rightPort.name = PORT_R;
    _rightPort.ptrElementThatOwnsPort = this;
    _rightPort.isConnected = false;
    _rightPort.portParameters.push_back(&_rightPortCurrent);
    _rightPort.portParameters.push_back(&_rightPortVoltage);
    
    
    // Add resistorState to the elemnetState vector
    _elementStates.push_back(&_resistorState);
    
    // Add left port to elementPort vector
    _elementPorts.push_back(&_leftPort);
    
    // Add right port to elementPort vector
    _elementPorts.push_back(&_rightPort);
}

Resistor::~Resistor() {
    // Maybe break connections when deleted
    // Implement later...
}

void Resistor::setImpedance(complex<double> impedance) {
    // Sets the impedance for the resistor
    _resistorState.value = impedance;
}

complex<double> Resistor::getImpedance() {
    return _resistorState.value;
}

complex<double> Resistor::getImpedanceInDirectionOf(string mine) {
    // Before continuing, check if this element is an open circuit
    if (getImpedance().real() == INFINITY)
        return INFINITY;
    
    // Define the impedance that will be returned
    complex<double> impedance = complex<double>(0.0, 0.0);
    
    // Get all neighboring elements
    vector<Element *> elements = getConnectedElements(mine);
    
    // Check if any of the elements have an open circuit i.e. infinity impedance
    vector<Element *>::iterator anElement;
    for (anElement = elements.begin();
         anElement != elements.end();
         anElement++) {
        
        // Checks if the element is a resistor and treats it like one
        if (Resistor *aResistor = dynamic_cast<Resistor *>(*anElement)) {
            if (aResistor->getImpedance().real() == INFINITY) {
                
                // If the resistor has infinity impedance, remove it
                elements.erase(anElement);
                anElement = elements.begin();
                
                // And if this element is no longer connected to other elements
                if (elements.empty()) {
                    // then the impedance behind the port is infinity
                    return complex<double>(INFINITY, 0.0);
                }
                
            }
        }
    }
    
    // If this element is still connected to other elements, then compute the
    // impedance beyond the port
    
    if (elements.size() > 1) {
        // For multiple neighbors execute the parallel connections function
        impedance += getParallelImpedances(elements, mine);
    } else {
        // For a single neihbor connection execute series connection function
        impedance += getSeriesImpedances(elements.back(), mine);
    }
    
    // Buffer impedance beyond resistor for faster execution
    _impedanceBeyondResistor = impedance;
    
    // Then add its own impedance to the total
    impedance += getImpedance();
    
    // And return the result
    return impedance;
}

complex<double> Resistor::getParallelImpedances(vector<Element *> elements, string mine) {
    // If many elements are connected do Rt = ∏Rn / ∑Rn
    complex<double> numerator   = complex<double>(1.0, 0.0);
    complex<double> denominator = complex<double>(0.0, 0.0);
    
    vector<Element *>::iterator anElement;
    for (anElement = elements.begin();
         anElement != elements.end();
         anElement++) {
        
        // Make sure that the current element is handled like a resistor
        if (Resistor *aResistor = dynamic_cast<Resistor *>(*anElement)) {
            
            // Extract the impedance of the resistor and all its circuitry beyond
            complex<double> impedance = aResistor->getImpedanceInDirectionOf(mine);

            numerator   *= impedance;
            denominator += impedance;
        }
    }
    
    // Before returning check for special cases
    
    // If the numerator is an open circuit, then there is no point in returning
    // an imaginary part
    if (numerator.real() == INFINITY)
        return complex<double>(INFINITY, 0.0);
    
    // If the denominator is zero, then the complex-number division may not
    // yield the open circuit i.e. infinity
    if (denominator.real() == 0 && denominator.imag() == 0)
        return complex<double>(INFINITY, 0.0);
    
    return numerator / denominator;
}

complex<double> Resistor::getSeriesImpedances(Element *element, string mine) {
    // Make sure that the current element is handled like a resistor
    if (Resistor *aResistor = dynamic_cast<Resistor *>(element)) {
        
        // Return the impedance of the resistor and all its circuitry beyond
        return aResistor->getImpedanceInDirectionOf(mine);
    }
    
    // If the device was not a resistor, return an open circuit to avoid error
    return complex<double>(INFINITY, 0.0);
}

vector<Element *> Resistor::getNewState() {
    vector<Element *> nextElements;
    
    bool isLeftPortVoltageFixed = isPortParameterFixed(PORT_L, VOLTAGE);
    bool isRightPortVoltageFixed = isPortParameterFixed(PORT_R, VOLTAGE);
    
    complex<double> impedance = getImpedance();
    complex<double> current = complex<double> (0.0, 0.0);
    
    if (isLeftPortVoltageFixed && isRightPortVoltageFixed) {
        // If both ports' voltages have been given, then simply use I=V/R
        // But only if the circuit is not an open circuit
        if (impedance.real() != INFINITY) {
            current = (getPortParameter(PORT_L, VOLTAGE)
                       - getPortParameter(PORT_R, VOLTAGE)) / impedance;
        }
        
        // There is no element to update next
        
    } else if (isLeftPortVoltageFixed) {
        // If only the left voltage has been given, then compute
        // towards the right. Thus get the total impedance begind the right port
        complex<double> totalImpedance = getImpedanceInDirectionOf(PORT_R);
        
        // Here use the difference between the current potential and ground
        // with the total impedance to compute the total current flowing
        if (totalImpedance.real() != INFINITY) {
            current = (getPortParameter(PORT_L, VOLTAGE)
                       - _vss) / totalImpedance;
        }
        // Also compute the new port voltage due to current through component
        complex<double> newPortVoltage = getPortParameter(PORT_L, VOLTAGE)
        - current * impedance;
        
        // Appyl new voltage
        setPortParameter(PORT_R, VOLTAGE, newPortVoltage);
        
        // Set all connected elements as elements that will be updated next
        nextElements = getConnectedElements(PORT_R);
        
    } else if (isRightPortVoltageFixed) {
        // If only teh right voltage has been given, then compute
        // towards the left. Thus get the total impedance begind the left port
        complex<double> totalImpedance = getImpedanceInDirectionOf(PORT_L);
        
        // here use the difference between the current potential and ground
        // with the total impedance to compute the total current flowing
        if (totalImpedance.real() != INFINITY) {
            current = (_vcc
                       - getPortParameter(PORT_R, VOLTAGE)) / totalImpedance;
        }
        // Also compute the new port voltage due to current through component
        complex<double> newPortVoltage = getPortParameter(PORT_R, VOLTAGE)
        + current * impedance;
        
        // Apply new voltage
        setPortParameter(PORT_L, VOLTAGE, newPortVoltage);
        
        // Set all connected elements as elements that will be updated next
        nextElements = getConnectedElements(PORT_L);
        
    } else {
        // No port voltage has been given, hence check the isSet flags
        
        bool isLeftPortVoltageSet = isPortParameterSet(PORT_L, VOLTAGE);
        bool isRightPortVoltageSet = isPortParameterSet(PORT_R, VOLTAGE);
        
        if (isLeftPortVoltageSet && isRightPortVoltageSet) {
            // If both voltages have been set, then an error occured
            cout << "ERROR : For element <" << elementName() << "> both port voltages have been set" << endl;
            
            // And exit simulation with error code
            exit(-1);
            
        } else if (isLeftPortVoltageSet) {
            // If only the left voltage has been set, then compute towards
            // the right. Thus get the total impedance begind the right port
            complex<double> totalImpedance;
            if (_impedanceBeyondResistor.real() != 0.0
                && _impedanceBeyondResistor.imag() != 0.0) {
                // If the buffered impedance contains a supposingly valid
                // impedance, then use it to avoid unnecessary computation
                totalImpedance = impedance + _impedanceBeyondResistor;
                
                // Then empty buffer since it has been used
                _impedanceBeyondResistor = complex<double> (0.0, 0.0);
            } else {
                // Yet if the impedance computed seems invalid, then compute it
                // again to ensure a correct value
                totalImpedance = getImpedanceInDirectionOf(PORT_R);
            }
            
            // Here use the difference between the current potential and ground
            // with the total impedance to compute the total current flowing
            if (totalImpedance.real() != INFINITY) {
                current = (getPortParameter(PORT_L, VOLTAGE)
                           - _vss) / totalImpedance;
            }
            // Also compute the new port voltage due to current through component
            complex<double> newPortVoltage = getPortParameter(PORT_L, VOLTAGE)
            - current * impedance;
            
            // Appyl new voltage
            setPortParameter(PORT_R, VOLTAGE, newPortVoltage);
            
            // Set all connected elements as elements that will be updated next
            nextElements = getConnectedElements(PORT_R);
            
        } else if (isRightPortVoltageSet) {
            // If only teh right voltage has been given, then compute
            // towards the left. Thus get the total impedance begind the left port
            complex<double> totalImpedance;
            if (_impedanceBeyondResistor.real() != 0.0
                && _impedanceBeyondResistor.imag() != 0.0) {
                // If the buffered impedance contains a supposingly valid
                // impedance, then use it to avoid unnecessary computation
                totalImpedance = impedance + _impedanceBeyondResistor;
                
                // Then empty buffer since it has been used
                _impedanceBeyondResistor = complex<double> (0.0, 0.0);
            } else {
                // Yet if the impedance computed seems invalid, then compute it
                // again to ensure a correct value
                totalImpedance = getImpedanceInDirectionOf(PORT_L);
            }
            
            // here use the difference between the current potential and ground
            // with the total impedance to compute the total current flowing
            if (totalImpedance.real() != INFINITY) {
                current = (_vcc
                           - getPortParameter(PORT_R, VOLTAGE)) / totalImpedance;
            }
            // Also compute the new port voltage due to current through component
            complex<double> newPortVoltage = getPortParameter(PORT_R, VOLTAGE)
            + current * impedance;
            
            // Apply new voltage
            setPortParameter(PORT_L, VOLTAGE, newPortVoltage);
            
            // Set all connected elements as elements that will be updated next
            nextElements = getConnectedElements(PORT_L);
            
        } else {
            // If voltage has not been given nor set then return another error
            cout << "ERROR : For element <" << elementName() << "> not having been updated" << endl;
        }
    }
    
    // Apply computed current to the ports
    setPortParameter(PORT_L, CURRENT, current);
    setPortParameter(PORT_R, CURRENT, -current);
    
    return nextElements;
}
