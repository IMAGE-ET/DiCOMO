//
//  element.cpp
//  DiCOMO
//
//  Created by Maximilian Zangs on 09.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#include "element.h"

long numberOfElements = 0;

#pragma mark PUBLIC

Element::Element(complex<double> vcc, complex<double> vss) {
    // Assign element index number
    _elementIndex = numberOfElements;
    
    // Assign source (vcc) and sink (vss) to element
    _vcc = vcc;
    _vss = vss;
    
    // Increase global element count by one
    // Since element has now been generated successfully
    numberOfElements++;
}

Element::~Element() {
    // Decrease number of global elements
    // !!! One CANNOT guarante that new elements have new name after this
    numberOfElements--;
}

string Element::elementName() {
    // Creates a string that starts with the element's type
    // And appends the element's index
    stringstream elementNameStream;
    
    // This has to be outputted to a stream...
    elementNameStream << _elementType << "_" << _elementIndex;
    
    // ... before it is appended to the name string
    return elementNameStream.str();
}

int Element::connectTo(Element *neighbour, string mine, string his) {
    // Number of connections that was established
    int connections = 0;
    
    // Get my (this element) and his (the neighbour's) port
    port *myPort = this->getPort(mine);
    port *hisPort = neighbour->getPort(his);
    
    bool isMyPortConnected = false;
    bool isHisPortConnected = false;
    
    // Check if the two ports are connected at all
    // Beause if they are they may be already connected to another
    if (myPort->isConnected && hisPort->isConnected) {
        vector<port *>::iterator aPort;
        
        // Check myPorts neighbours for hisPort
        for (aPort = myPort->neighbourPorts.begin();
             aPort != myPort->neighbourPorts.end();
             aPort++) {
            if (isEqual(*aPort, hisPort)) {
                isMyPortConnected = true;
                break;
            }
        }
        
        // Check hisPorts neighbors for myPort
        for (aPort = hisPort->neighbourPorts.begin();
             aPort != hisPort->neighbourPorts.end();
             aPort++) {
            if (isEqual(*aPort, myPort)) {
                isHisPortConnected = true;
                break;
            }
        }
        
    }
    
    // If myPort is not connected to him then...
    if (!isMyPortConnected) {
        // ...connect
        myPort->neighbourPorts.push_back(hisPort);
        
        // ...and set connection flag to true
        myPort->isConnected = true;
        
        connections++;
    }
    
    // If hisPort is not connected to me then...
    if (!isHisPortConnected) {
        // ...connect
        hisPort->neighbourPorts.push_back(myPort);
        
        // ...and set connection flag to true
        hisPort->isConnected = true;
        
        connections++;
    }
    
    return connections;
}

void Element::setPortParameter(string mine, string parameter, complex<double> value) {
    // Get parameter state that will be set
    state *aState = getState(mine, parameter);
    
    // If parameter exists i.e. not NULL and is not fixed i.e. given
    if (aState && !aState->isGiven) {
        aState->value = value;
        aState->isSet = true;
        
        if (parameter == VOLTAGE) {
            // If updateing the voltage, ensure that all neighbouring ports
            // are also updated to have the same potential
            
            vector<port *>::iterator neighbourPort;
            for (neighbourPort = getPort(mine)->neighbourPorts.begin();
                 neighbourPort != getPort(mine)->neighbourPorts.end();
                 neighbourPort++) {
                
                // Search in the neighbour's port's state to set voltage
                vector<state *>::iterator neighbourState;
                for (neighbourState = (*neighbourPort)->portParameters.begin();
                     neighbourState != (*neighbourPort)->portParameters.end();
                     neighbourState++) {
                    if ((*neighbourState)->name == VOLTAGE) {
                        (*neighbourState)->value = value;
                        (*neighbourState)->isSet = true;
                    }
                }
                
            }
            
            // Then clear the current state flag since this element has now been
            // interrogated and needn't keep this flag
            aState = getState(mine, CURRENT);
            if (aState && !aState->isGiven)
                aState->isSet = false;
            
        } else if (parameter == CURRENT) {
            // If updateing the current, then ensure that the voltage flag
            // is reset
            aState = getState(mine, VOLTAGE);
            if (aState && !aState->isGiven)
                aState->isSet = false;
        }
    }
}

complex<double> Element::getPortParameter(string mine, string parameter) {
    // Get parameter state that will be returned
    state *aState = getState(mine, parameter);
    
    // If a state has been found i.e. not NULL
    if (aState) {
        // Return
        return aState->value;
    }
    
    // Return NULL if no parameter was found
    return NULL;
}

void Element::fixPortParameter(string mine, string parameter, bool given) {
    // Get the parameter state
    state *aState = getState(mine, parameter);
    
    // Fix state if it exists i.e. not NULL
    if (aState)
        aState->isGiven = given;
}

bool Element::isPortParameterFixed(string mine, string parameter) {
    // Get the parameter state
    state *aState = getState(mine, parameter);
    
    // Return ifGiven if state has been found
    if (aState)
        return aState->isGiven;
    
    return NULL;
}

#pragma mark PROTECTED

bool Element::isEqual(port *p1, port *p2) {
    // Define equal paramter
    bool isEqual = false;
    
    // Check for name and owner
    bool nameIsEqual = (p1->name == p2->name);
    bool ownerIsEqial = (p1->ptrElementThatOwnsPort->elementName()
                         == p2->ptrElementThatOwnsPort->elementName());
    
    // Ckeck if both cases are true
    isEqual = (nameIsEqual && ownerIsEqial);
    
    // Return equal parameter
    return isEqual;
}

port *Element::getPort(string mine) {
    // Use iterator to jump approproate memory size when incremented
    vector<port *>::iterator aPort;
    
    // Seach through prot vector to find the correct port
    for (aPort = _elementPorts.begin();
         aPort != _elementPorts.end();
         aPort++) {
        if ((*aPort)->name == mine)
            return *aPort;
    }
    
    // Return NULL-pointer if no other port has been found
    return NULL;
}

state *Element::getState(string mine, string parameter) {
    port *aPort = getPort(mine);
    
    vector<state *>::iterator aState;
    
    // Search states for correct parameter
    for (aState = aPort->portParameters.begin();
         aState != aPort->portParameters.end();
         aState++) {
        if ((*aState)->name == parameter)
            return *aState;
    }
    
    // Return NULL if no parameter has been found
    return NULL;
}

bool Element::isPortParameterSet(string mine, string parameter) {
    // Get parameter state that will be returned
    state *aState = getState(mine, parameter);
    
    // Return isSet flag if a state has been found i.e. not NULL
    if (aState)
        return aState->isSet;
    
    // Return NULL if no parameter was found
    return NULL;
}

vector<Element *> Element::getConnectedElements(string mine) {
    // Get the port from which the neighbors are to be extracted
    port *aPort = getPort(mine);
    
    // Create empty vector that will contain element pointers
    vector<Element *> neighbours;
    
    // From each neighbour's port, add its owning element to the list
    vector<port *>::iterator neighbourPorts;
    for (neighbourPorts = aPort->neighbourPorts.begin();
         neighbourPorts != aPort->neighbourPorts.end();
         neighbourPorts++) {
        port *aNeighborPort = *neighbourPorts;
        neighbours.push_back(aNeighborPort->ptrElementThatOwnsPort);
    }
    
    return neighbours;
}
