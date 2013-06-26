//
//  element.h
//  DiCOMO
//
//  Created by Maximilian Zangs on 09.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#ifndef __DiCOMO__element__
#define __DiCOMO__element__

#include "backbone.h"

using namespace std;

class Element {
    
// Make values available to friend classes -> allow inheritance
protected:
    // Voltage source (vcc) and sink (vss) that this element is connected to
    complex<double> _vcc;
    complex<double> _vss;

    // Defines the element type
    string _elementType;
    
    // Stores element index number
    long _elementIndex;
    
    // Vector of element ports
    vector<port *> _elementPorts;
    
    // Vector of element states
    vector<state *> _elementStates;
    
    
public:
    // Constructor
    Element(complex<double> vcc, complex<double> vss);
    
    // Destructor
    ~Element();

    // Returns the element name
    string elementName();
    
    // Connects two element's ports
    int connectTo(Element *neighbour, string mine, string his);
    
    // Set and get a certain parameter
    void setPortParameter(string mine, string parameter, complex<double> value);
    complex<double> getPortParameter(string mine, string parameter);
    
    // Fix a parameter too keep it constant (i.e. vcc & vss)
    void fixPortParameter(string mine, string parameter, bool given);
    bool isPortParameterFixed(string mine, string parameter);
    
// Further protected class functions
protected:
    
    // Checks if ports are equal
    bool isEqual(port *p1, port *p2);
    
    // Returns element port
    port *getPort(string mine);
    
    // Returns element port parameter
    state *getState(string mine, string name);
    
    // Flag that tells if port parameter has been set
    bool isPortParameterSet(string mine, string parameter);
    
    // Return all elements that are connected to a port
    vector<Element *> getConnectedElements(string mine);
    
public:
    // A function that must be implemented in all inheriting classes
    // Will contain the algorithm, whilst the protocol is defined in Element
    virtual vector<Element *> getNewState() = 0;
};

#endif /* defined(__DiCOMO__element__) */
