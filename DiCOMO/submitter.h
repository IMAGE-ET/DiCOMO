//
//  submitter.h
//  DiCOMO
//
//  Created by Maximilian Zangs on 24.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#ifndef __DiCOMO__submitter__
#define __DiCOMO__submitter__

//  This is a class that allows the utilisation of the terminal to pass arguments
//  to the program that allow the execution of specific scenarios.
//  For help simply execute the code and pass "-h" for HELP

#include "simulation.h"
#include "irishData.h"

using namespace std;

enum setter {
    Error           = -1,
    SourceVoltage   = 0,
    SinkVoltage     = 1,
    Phases          = 2,
    IrishDataSetup  = 3,
    StartHouse      = 4,
    FeederLength    = 5,
    Sample          = 6,
    OutputFile      = 7,
    PowerFactor     = 8,
};

class Submitter {
protected:
    Simulation *_simulation;
    IrishData *_irishData;
    
    // Some variables can be applied immediately, so they need to be stored
    // until the simulation starts
    bool _verbose;
    int _startHouse;
    int _feederLenth;
    int _sample;
    double _powerFactor;
    string _outputFilePath;
    
public:
    Submitter(bool verbose = false);
    ~Submitter();
    
    // All arguments passed are interpreted here
    void setValues(int argc, const char * argv[]);
    
protected:
    // Lists help
    void help(const char *argv = NULL);
    
    // Lists about
    void about();

    // Executes the simulation
    void run();
};

#endif /* defined(__DiCOMO__submitter__) */
