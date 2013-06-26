//
//  simulation.h
//  DiCOMO
//
//  Created by Maximilian Zangs on 10.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#ifndef __DiCOMO__simulation__
#define __DiCOMO__simulation__

//  The simulation class provides a C++ framework that utilises the element
//  based architecture for execution of simulations. With its public functions
//  one is capable of generating a vast number of single- and multi-phase
//  distribution feeders and supply them with power values for simulation.

#include "storage.h"

class Simulation {
protected:
    // This matrix contains the number of phases per column where each column
    // contains a column vector with the feeder impedance in each row
    vector< vector< complex<double> > > _feederImpedances;
    
    // Stores the number of phases. Although this can be read from the feeder-
    // Impedance matrix, it is still needed if no impedances have been added to
    // the matrix
    int _phases;
    int _maxPhases;
    int _minPhases;
    
    // Source(s) and sink voltages
    // For multiple phases, sources are rotated by 2π/phases
    complex<double> _vcc;
    complex<double> _vss;
    
    // This column vector contains each return line impedance and should be of
    // length equal to the concatinated feederImpedance
    // NOTE: To connect join two phases togethes imply set resistance inbetween
    //       to zero
    vector< complex<double> > _returnImpedances;
    // List in which the 
    vector<int> _connectionOrder;
    
    // Each households power consumption for the specific simulation is stored
    // in this matrix similar to the feederImpedance matrix
    vector< vector< complex<double> > > _powers;
    
    // The entire circuit will be stored in this vector
    vector<Element *> _circuit;
    
    bool _verbose;
    
public:
    Simulation(bool verbose = false);
    ~Simulation();

    // Sets and gets the number of phases
    void setPhases(int phases);
    int getPhases();
    // Allows quick check whether the given phase value is supported
    bool phaseOK(int phase);
    
    void setSource(complex<double> vcc = complex<double> (240.0, 0.0));
    void setSink(complex<double> vss = complex<double> (0.0, 0.0));
    
    void addFeederImpedanceForPhase(complex<double> impedance, int phase = 1);
    void addReturnImpedance(complex<double> impedance);
    
    // Two ways in which house powers can be addded to the power matrix
    // The order the powers are added is directly translated into the order in
    // which they are connected along the feeder
    void addPowerToPhase(complex<double> power, int phase = 1);
    void addPowerToPhase(double power, double powerFactor, int phase = 1, bool isInductive = true);
    
    // Starts the simulation
    void start();
    
    // Saves the feeder with all voltagses in an external CSV file
    // as path pass: "out" so store the output in the current directory
    void saveFeeders(string path, bool saveComplex = false);
    
    void saveSubstation(string path, bool saveComplex = false);
protected:
    
};

#endif /* defined(__DiCOMO__simulation__) */
