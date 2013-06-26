//
//  backbone.h
//  DiCOMO
//
//  Created by Maximilian Zangs on 09.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#ifndef __DiCOMO__backbone__
#define __DiCOMO__backbone__

#include <iostream>

// Stream manipulator
#include <iomanip>

#include <string>
#include <set>
#include <vector>
#include <complex>
#include <algorithm>

// Used for writing to file
#include <fstream>
#include <sstream>

// To access directory
#include <dirent.h>
#include <sys/stat.h>

/* Used for on screen output */
#define GREETING        "Project Program    -    SE4RP11    -    Maximilian J Zangs"
#define BYE             "Project Program    -    Yey I'm all done time for a coffee"

#define SPACER          "----------------------------------------------------------"
#define START_SPACER    "/--------------------------------------------------------\\"
#define END_SPACER      "\\--------------------------------------------------------/"

/* Constants needed to access state names */
#define PORT_L          "left"
#define PORT_R          "right"
#define NOT_CONNECTED   "not_connected"
#define VCC             "Vcc"
#define VSS             "Vss"
#define VOLTAGE         "voltage"
#define CURRENT         "current"
#define IMPEDANCE       "impedance"

#define RESISTOR        "resistor"
#define CONSUMER        "consumer"
#define STORAGE         "storage"
#define POWER           "watt"

using namespace std;

// Forward class definition
class Element;

// A struct to keep all information about numerical parameters
struct state {
    // Name of parameter
    string name;
    
    // Its numerical value
    complex<double> value;
    
    // Has it been set
    bool isSet;
    
    // Is value given i.e. fixed
    bool isGiven;
};

// A struct that contains all information about a port
struct port {
    // Name of port
    string name;
    
    // Element that owns port
    Element *ptrElementThatOwnsPort;
    
    // All port's parameters stored in state vetcor
    vector<state *> portParameters;
    
    // Vector of connected ports
    vector<port *> neighbourPorts;
    
    // Whether this port is connected at all
    bool isConnected;
};

/* Vector sort & comparison functions for complex numbers */
bool compareComplexAbs (complex<double> i,complex<double> j);
bool compareComplexReal (complex<double> i,complex<double> j);
bool compareComplexImag (complex<double> i,complex<double> j);

#endif /* defined(__DiCOMO__backbone__) */
