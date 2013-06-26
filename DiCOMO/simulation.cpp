//
//  simulation.cpp
//  DiCOMO
//
//  Created by Maximilian Zangs on 10.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#include "simulation.h"

Simulation::Simulation(bool verbose) {
    cout << START_SPACER << endl << GREETING << endl << SPACER << endl;
    
    // Default single phase simulation setup
    
    _maxPhases = 3;
    _minPhases = 1;
    setPhases(1);
    
    _verbose = verbose;
}

Simulation::~Simulation() {
    // Clean up simulation
    
    cout << SPACER << endl << BYE << endl << END_SPACER << endl;
}

void Simulation::setPhases(int phases) {
    // The minimum number of phases must be 1, maximum is undefined yet set to 3
    if (phases < _minPhases || phases > _maxPhases) {
        if (_feederImpedances.size() >= _minPhases
            && _feederImpedances.size() <= _maxPhases)
            _phases = (int) _feederImpedances.size();
        return;
    }
    
    _phases = phases;

    // Reset source and sink voltages
    setSink();
    setSource();
    
    // Match the number columns in impedance matrix with nuber of phases
    _feederImpedances.clear();
    while (_feederImpedances.size() < _phases) {
        vector< complex<double> > phaseImpedance;
        _feederImpedances.push_back(phaseImpedance);
    }
    
    // Match the number columns in power matrix with nuber of phases
    _powers.clear();
    while (_powers.size() < _phases) {
        vector< complex<double> > phasePower;
        _powers.push_back(phasePower);
    }
    
    // Also clear return line and connection order vector
    _returnImpedances.clear();
    _connectionOrder.clear();
}

int Simulation::getPhases() {
    if (_phases == 0)
        setPhases(1);

    return _phases;
}

bool Simulation::phaseOK(int phase) {
    // Ensure only relevant data is stored
    if (   phase < _minPhases
        || phase > _maxPhases
        || phase > _phases
        || phase > _feederImpedances.size()
        || phase > _powers.size()) {
        
        cout << "WARNING : Phase <" << phase << "> not supported, use <" << _phases << "> or lower." << endl;
        return false;
    }
    
    return true;
}

void Simulation::setSource(complex<double> vcc) {
    _vcc = vcc;
}

void Simulation::setSink(complex<double> vss) {
    _vss = vss;
}

void Simulation::addFeederImpedanceForPhase(complex<double> impedance, int phase) {
    // Ensure only relevant data is stored
    if (!phaseOK(phase)) return;
    
    // Add impedance to given feeder phase
    _feederImpedances[phase-1].push_back(impedance);
}

void Simulation::addReturnImpedance(complex<double> impedance) {
    // Add impedance to return line
    _returnImpedances.push_back(impedance);
}

void Simulation::addPowerToPhase(complex<double> power, int phase) {
    // Ensure only relevant data is stored
    if (!phaseOK(phase)) return;
    
    // Add power to given phase
    _powers[phase-1].push_back(power);
    
    // Add power to connection order vector
    _connectionOrder.push_back(phase);
}

void Simulation::addPowerToPhase(double power, double powerFactor, int phase, bool isInductive) {
    // Ensure only relevant data is stored
    if (!phaseOK(phase)) return;

    // Check if power factor is valid
    if (powerFactor < 0 || powerFactor > 1.0) {
        // Power factor must lie between 0 and 1
        cout << "ERROR : Invalid power factor of <" << powerFactor << "> when setting power." << endl;
        exit(-1);
    }
    
    // And compute true and reactive power for complex number power
    double truePower = power * powerFactor;
    double reactivePower = (isInductive
                            ? sqrt(power*power - truePower*truePower)
                            : -sqrt(power*power - truePower*truePower));
    
    addPowerToPhase(complex<double>(truePower, reactivePower), phase);
}

void Simulation::start() {
    
#pragma mark CHECKING EVERYTHING IS FINE
    cout << "CHECKING EVERYTHING IS FINE" << endl << endl;
    
    // Check whether the branches match
    int totalNumberOfFeederImpedances = 0;
    int totalNummerOfPowers = 0;

    // Adds all feeder and power values to check whether there is an equal
    // number of both
    for (int i = 0; i < _phases; i++) {
        
        totalNumberOfFeederImpedances += (unsigned int) _feederImpedances[i].size();
        totalNummerOfPowers += (unsigned int) _powers[i].size();
        
        int totalNumberOfConnectionsOnPhase = 0;
        vector<int>::iterator connection;
        for (connection = _connectionOrder.begin();
             connection != _connectionOrder.end();
             connection++) {
            if (*connection == i+1) {
                totalNumberOfConnectionsOnPhase++;
            }
        }
        
        // If the length of the feeder is not equal to the number of consumers
        // connected to it, then call an error and abort the simulation
        if (   _feederImpedances[i].size() != _powers[i].size()
            || totalNumberOfConnectionsOnPhase != _powers[i].size()
            || totalNumberOfConnectionsOnPhase != _feederImpedances[i].size()) {
            cout << "ERROR : feeder segments, consumers and connection # do not match for phase <" << i+1 << ">" << endl;
            cout << "F           = " << _feederImpedances[i].size() << endl;
            cout << "P           = " << _powers[i].size() << endl;
            cout << "Connections = " << totalNumberOfConnectionsOnPhase << endl;
            return;
        }
        
        if (_verbose) cout << "Phase :" << setw(48) << i+1 << " OK" << endl;
    }
    
    // Check if the return line is long enough to connect to all consumers and
    // also all feeder line segments. Furthermore ensure the connection order is
    // equal of same length, too. Otherwise assembly would be impossible.
    if (   _returnImpedances.size() != totalNummerOfPowers
        || _returnImpedances.size() != totalNumberOfFeederImpedances
        || _returnImpedances.size() != _connectionOrder.size()) {
        cout << "ERROR : Return line segments do not match the feeder and power profiles." << endl;
        cout << "F           = " << totalNumberOfFeederImpedances << endl;
        cout << "R           = " << _returnImpedances.size() << endl;
        cout << "P           = " << totalNummerOfPowers << endl;
        cout << "Connections = " << _connectionOrder.size() << endl;
        return;
    }
    if (_verbose) cout << "Components match :" << setw(40) << " OK" << endl;
    
    
    // Start connecting
#pragma mark ASSEMBLING CIRCUIT
    if (_verbose) cout << endl << SPACER << endl;
    cout << "ASSEMBLING CIRCUIT" << endl << endl;
    
    // Empty the current circuit and generate a vector through which the
    // algorithm can enter the computation
    _circuit.clear();
    vector<Element *> entryElements;
    
    // Connect entire return line first since it is phase independent and
    // a continuous connection
    for (int i = 0; i < _returnImpedances.size(); i++) {
        // Compute the rotated voltage source for the current phase
        double angle = (double)(2 * M_PI / _phases);
        double offset = (double)tan(_vcc.imag() / _vcc.real());
        if (offset == 0.0 && _vcc.real() < 0)
            offset = M_PI;
        
        double real = abs(_vcc) * cos(angle * (_connectionOrder[i]-1) + offset);
        double imag = abs(_vcc) * sin(angle * (_connectionOrder[i]-1) + offset);
        
        complex<double> phaseVcc = complex<double> (real, imag);
        
        // Set up new impedance
        Resistor *r = new Resistor(phaseVcc, _vss);
        r->setImpedance(_returnImpedances[i]);

        if (_circuit.size() > 0) {
            // If it is not the vss conection, then connect to last element
            r->connectTo(_circuit.back(), PORT_R, PORT_L);
        } else {
            // Else connect to _vss and flag given
            if (_verbose) {
                
                cout << "Grounding return line element :" << setw(27) << r->elementName() << endl;
                cout << " > " << setw(10) << abs(_vss) << " V";
                cout << " @ " << setw (3) << ( (abs(_vss) != 0)
                                              ? atan(_vss.imag()/_vss.real())*180/M_PI
                                              : 0 ) << "°" << endl;
            }
            
            r->setPortParameter(PORT_R, VOLTAGE, _vss);
            r->fixPortParameter(PORT_R, VOLTAGE, true);
            
            // Set entrypoint for computation
            entryElements.push_back( r );
        }
        // Insert into circuit
        _circuit.push_back( r );
    }
    
    // Connect one phase at a time to the return line
    for (int currentPhase = 0; currentPhase < _phases; currentPhase++) {
        int connectionsPerPhase = 0;
        int lastPhaseConnection = 0;
        
        for (int elementsCounter = 0;
             elementsCounter < _connectionOrder.size();
             elementsCounter++) {
            
            if (_connectionOrder[elementsCounter] == currentPhase+1) {
                // Compute the rotated voltage source for the current phase
                double angle = (double)(2 * M_PI / _phases);
                double offset = (double)tan(_vcc.imag() / _vcc.real());
                if (offset == 0.0 && _vcc.real() < 0)
                    offset = M_PI;
                
                double real = abs(_vcc) * cos(angle * currentPhase + offset);
                double imag = abs(_vcc) * sin(angle * currentPhase + offset);
                
                complex<double> phaseVcc = complex<double> (real, imag);
                
                // Create consumer
                Consumer *c = new Consumer(phaseVcc, _vss);
                // Set its power consumption
                c->setPower(_powers[currentPhase][connectionsPerPhase]);
                // Connect to "down" the return line
                c->connectTo(_circuit[elementsCounter], PORT_R, PORT_L);
                // If not connecting the last element...
                if (elementsCounter < _connectionOrder.size()-1) {
                    // ...connect to "up" the return line
                    c->connectTo(_circuit[elementsCounter+1], PORT_R, PORT_R);
                }
                // Insert into circuit
                _circuit.push_back(c);
                
                // Create feeder
                Resistor *f = new Resistor(phaseVcc, _vss);
                // Set its impedance
                f->setImpedance(_feederImpedances[currentPhase][connectionsPerPhase]);
                // Connect to Consumer (last inserted into circuit)
                f->connectTo(_circuit.back(), PORT_R, PORT_L);
                // If not the first feeder segment of this phase...
                if (connectionsPerPhase > 0) {
                    // ...connect "up" the feeder line i.e to the last feeder line
                    // segment which was stored two elements earlier
                    f->connectTo(_circuit[_circuit.size()-2], PORT_L, PORT_R);
                } else {
                    if (_verbose) {
                        cout << "Connecting feeder line element to source:" << setw(17) << f->elementName() << endl;
                        cout << " > " << setw(10) << abs(_vcc) << " V";
                        cout << " @ " << setw(3) << angle*currentPhase*180/M_PI << "°";
                        cout << " on phase " << setw(3) << currentPhase << endl;
                    }
                    f->setPortParameter(PORT_L, VOLTAGE, phaseVcc);
                    f->fixPortParameter(PORT_L, VOLTAGE, true);
                    
                    // Set entrypoint for computation
                    entryElements.push_back( f );
                }
                // Insert into circuit
                _circuit.push_back(f);
                
                lastPhaseConnection = elementsCounter;
                // Increase phase connection counter
                connectionsPerPhase++;
            }
            
        }
    }
    
    // Start execution
#pragma makr STARTING EVALUATION
    if (_verbose) cout << endl << SPACER << endl;
    cout << "STARTING EVALUATION" << endl << endl;
    
    // Reverse order, so that feeder lines are evaluated first and common
    // return line last
//    reverse(entryElements.begin(), entryElements.end());
    
    if (_verbose) cout << "Executing " << _returnImpedances.size()*3 << " iterations" << endl << endl;
    
    vector<Element *> computationBuffer;
    clock_t startTime = clock();
    
    // Times executions by 3 since each "branch" contains 3 elements:
    // > feeder, consumer/storage, return
    for (int execution = 0; execution < _returnImpedances.size()*3; execution++) {
        computationBuffer = entryElements;
        
        if (_verbose) {

            cout << "Computing :    " << setw(3) << (int)(execution / (_returnImpedances.size()*3.0) * 100.0) << "%";
            clock_t nowTime = clock();
            cout << "         |           Time:" << setw(10) << fixed << setprecision(2) << (((float)nowTime - (float)startTime) / 1000000.0F ) * 1000 << " ms" << endl;
        }
        
        do {
            // Getting the last element of the buffer vector as new interrogator
            Element *interrogator = computationBuffer.back();
            // Removing this element from the buffer
            computationBuffer.pop_back();
            
            // Interrogating the circuit to get the new state and next
            // interrogators as a small vector
            vector<Element *>nextInterrogators = interrogator->getNewState();
            // This vector is then appended to the previous vector
            computationBuffer.insert(computationBuffer.end(),
                                     nextInterrogators.begin(),
                                     nextInterrogators.end());
            
        } while (!computationBuffer.empty());
    }
    
    cout << "Computing :    100%";
    clock_t nowTime = clock();
    cout << "         |           Time:" << setw(10) << fixed << setprecision(2) << (((float)nowTime - (float)startTime) / 1000000.0F ) * 1000 << " ms" << endl << endl;
    
    
    // Show results
    if (_verbose) {
        cout << endl << SPACER << endl;
        cout << "RESULTS" << endl << endl;
        
        for (int i = 0; i < _circuit.size(); i++) {
            cout << setw(19) << _circuit[i]->elementName();
            cout << "         _____" << endl;
            cout << setw(19) << _circuit[i]->getPortParameter(PORT_L, VOLTAGE) << " V";
            cout << "   ___|     |___";
            cout << setw(19) << _circuit[i]->getPortParameter(PORT_R, VOLTAGE) << " V" << endl;
            cout << setw(19) << _circuit[i]->getPortParameter(PORT_L, CURRENT) << " A";
            cout << "      |_____|   ";
            cout << setw(19) << _circuit[i]->getPortParameter(PORT_R, CURRENT) << " A" << endl << endl;
        }
    }
}

void Simulation::saveFeeders(string path, bool saveComplex) {
#pragma mark SAVING FEEDER
    if (_verbose) cout << endl << SPACER << endl;
    cout << "SAVING FEEDER" << endl << endl;

    // Check if the circuit exists and halt if not
    if (_circuit.empty()) {
        cout << "ERROR : No circuit has been set up." << endl;
        return;
    }
    
    // Defines file type
    stringstream pathStream;
    pathStream << path << ".csv";
    path = pathStream.str();
    
    // Generate output stream
    ofstream output(path.c_str(), ios::binary);
    if (output.is_open()) {
        
        vector<Element *> storageElements;
        
        if (saveComplex) {
            // If complex values are to be stored. That includes real and
            // imaginary
            
            // Mate title row
            output << "Name,Re(V_l),Im(V_l),Re(V_r),Im(V_r),Re(I),Im(I),Re(Z),Im(Z),Re(S),Im(S)" << endl;
        
            // Analysie each element in the circuit and only write the port values
            // of consumers and storage devices so that they may be interpreted
            for (int i = 0; i < _circuit.size(); i++) {
                
                // Check if the element at hand is a consumer thus is located
                // along the feeder invetween phase and return
                if (Consumer *consumer = dynamic_cast<Consumer *>(_circuit[i])) {
                    
                    // Then check if one deals with storage since this information
                    // will be stored in a separate row
                    if (Storage *storage = dynamic_cast<Storage *>(consumer)) {
                        // Save storage for later
                        storageElements.push_back(storage);
                    } else {
                        // Write data to file
                        
                        // Outputting element name
                        output << consumer->elementName() << ",";
                        
                        // Getting voltage
                        complex<double> voltageL = consumer->getPortParameter(PORT_L, VOLTAGE);
                        complex<double> voltageR = consumer->getPortParameter(PORT_R, VOLTAGE);
                        
                        // Outputting voltage
                        output << voltageL.real() << ",";
                        output << voltageL.imag() << ",";
                        output << voltageR.real() << ",";
                        output << voltageR.imag() << ",";
                        
                        // Getting current
                        complex<double> current = consumer->getPortParameter(PORT_L, CURRENT);
                        
                        // Outputting current
                        output << current.real() << ",";
                        output << current.imag() << ",";
                        
                        // Computing impedance
                        complex<double> impedance;
                        if (abs(current) == 0)
                            impedance = INFINITY;
                        else
                            impedance = (voltageL-voltageR) / current;
                        
                        // Outputting impedance
                        output << impedance.real() << ",";
                        output << impedance.imag() << ",";
                        
                        // Computing power
                        complex<double> power;
                        if (abs(current) == 0)
                            power = 0;
                        else
                            power = (voltageL-voltageR) * current;
                        
                        // Outputting power
                        output << power.real() << ",";
                        output << power.imag() << endl;
                    }
                }
            }
        } else {
            // If not complex, only absolute values are needed
            
            // Mate title row
            output << "Name,V_l,V_r,I,Z,S" << endl;
            
            // Analysie each element in the circuit and only write the port values
            // of consumers and storage devices so that they may be interpreted
            for (int i = 0; i < _circuit.size(); i++) {
                
                // Check if the element at hand is a consumer thus is located
                // along the feeder invetween phase and return
                if (Consumer *consumer = dynamic_cast<Consumer *>(_circuit[i])) {
                    
                    // Then check if one deals with storage since this information
                    // will be stored in a separate row
                    if (Storage *storage = dynamic_cast<Storage *>(consumer)) {
                        // Save storage for later
                        storageElements.push_back(storage);
                    } else {
                        // Write data to file
                        
                        // Outputting element name
                        output << consumer->elementName() << ",";
                        
                        // Getting voltage
                        double voltageL = abs(consumer->getPortParameter(PORT_L, VOLTAGE));
                        double voltageR = abs(consumer->getPortParameter(PORT_R, VOLTAGE));
                        
                        // Outputting voltage
                        output << voltageL << ",";
                        output << voltageR << ",";
                        
                        // Getting current
                        double current = abs(consumer->getPortParameter(PORT_L, CURRENT));
                        
                        // Outputting current
                        output << current << ",";
                        
                        // Computing impedance
                        double impedance;
                        if (current == 0)
                            impedance = INFINITY;
                        else
                            impedance = (voltageL-voltageR) / current;
                        
                        // Outputting impedance
                        output << impedance << ",";
                        
                        // Computing power
                        double power;
                        if (current == 0)
                            power = 0;
                        else
                            power = (voltageL-voltageR) * current;
                        
                        // Outputting power
                        output << power << endl;
                    }
                }
            }
        }
        
        
        // Close the output stream before returning
        output.close();
        if (_verbose) {
            cout << " File successfully written to:" << endl;
            cout << path << endl;
        }
    } else {
        cout << "ERROR : Could not generate <" << path << ">." << endl;
        return;
    }
    
    return;
}

void Simulation::saveSubstation(string path, bool saveComplex) {
#pragma mark SAVING SUBSTATION
    if (_verbose) cout << endl << SPACER << endl;
    cout << "SAVING SUBSTATION" << endl << endl;
    
    // Check if the circuit exists and halt if not
    if (_circuit.empty()) {
        cout << "ERROR : No circuit has been set up." << endl;
        return;
    }
    
    // Defines file type
    stringstream pathStream;
    pathStream << path << "s.csv";
    path = pathStream.str();
    
    // Generate output stream
    ofstream output(path.c_str(), ios::binary);
    if (output.is_open()) {
        
        vector<Element *> storageElements;
        
        if (saveComplex) {
            // If complex values are to be stored. That includes real and
            // imaginary
            
            // Mate title row
            output << "Name,Re(V_l),Im(V_l),Re(V_r),Im(V_r),Re(I),Im(I),Re(Z),Im(Z),Re(S),Im(S)" << endl;
            
            // Analysie each element in the circuit and only write the port values
            // of consumers and storage devices so that they may be interpreted
            for (int i = 0; i < _circuit.size(); i++) {
                
                // Check if the element at hand is a consumer thus is located
                // along the feeder invetween phase and return
                if (Resistor *resistor = dynamic_cast<Resistor *>(_circuit[i])) {
                    // Write data to file
                    
                    // Only extract the lines connected to sources
                    if (!resistor->isPortParameterFixed(PORT_L, VOLTAGE) &&
                        !resistor->isPortParameterFixed(PORT_R, VOLTAGE)) {
                        continue;
                    }
                    
                    // Outputting element name
                    output << resistor->elementName() << ",";
                    
                    // Getting voltage
                    complex<double> voltageL = resistor->getPortParameter(PORT_L, VOLTAGE);
                    complex<double> voltageR = resistor->getPortParameter(PORT_R, VOLTAGE);
                    
                    // Outputting voltage
                    output << voltageL.real() << ",";
                    output << voltageL.imag() << ",";
                    output << voltageR.real() << ",";
                    output << voltageR.imag() << ",";
                    
                    // Getting current
                    complex<double> current = resistor->getPortParameter(PORT_L, CURRENT);
                    
                    // Outputting current
                    output << current.real() << ",";
                    output << current.imag() << ",";
                    
                    // Computing impedance
                    complex<double> impedance;
                    if (abs(current) == 0)
                        impedance = INFINITY;
                    else
                        impedance = (voltageL-voltageR) / current;
                    
                    // Outputting impedance
                    output << impedance.real() << ",";
                    output << impedance.imag() << ",";
                    
                    // Computing power
                    complex<double> power;
                    if (abs(current) == 0)
                        power = 0;
                    else
                        power = (voltageL-voltageR) * current;
                    
                    // Outputting power
                    output << power.real() << ",";
                    output << power.imag() << endl;
                }
            }
        } else {
            // If not complex, only absolute values are needed
            
            // Mate title row
            output << "Name,V_l,V_r,I,Z,S" << endl;
            
            // Analysie each element in the circuit and only write the port values
            // of consumers and storage devices so that they may be interpreted
            for (int i = 0; i < _circuit.size(); i++) {
                
                // Check if the element at hand is a consumer thus is located
                // along the feeder invetween phase and return
                if (Resistor *resistor = dynamic_cast<Resistor *>(_circuit[i])) {
                    // Write data to file
                    
                    // Only extract the lines connected to sources
                    if (!resistor->isPortParameterFixed(PORT_L, VOLTAGE) &&
                        !resistor->isPortParameterFixed(PORT_R, VOLTAGE)) {
                        continue;
                    }
                    
                    // Outputting element name
                    output << resistor->elementName() << ",";
                    
                    // Getting voltage
                    double voltageL = abs(resistor->getPortParameter(PORT_L, VOLTAGE));
                    double voltageR = abs(resistor->getPortParameter(PORT_R, VOLTAGE));
                    
                    // Outputting voltage
                    output << voltageL << ",";
                    output << voltageR << ",";
                    
                    // Getting current
                    double current = abs(resistor->getPortParameter(PORT_L, CURRENT));
                    
                    // Outputting current
                    output << current << ",";
                    
                    // Computing impedance
                    double impedance;
                    if (current == 0)
                        impedance = INFINITY;
                    else
                        impedance = (voltageL-voltageR) / current;
                    
                    // Outputting impedance
                    output << impedance << ",";
                    
                    // Computing power
                    double power;
                    if (current == 0)
                        power = 0;
                    else
                        power = (voltageL-voltageR) * current;
                    
                    // Outputting power
                    output << power << endl;
                }
            }
        }
        
        
        // Close the output stream before returning
        output.close();
        if (_verbose) {
            cout << " File successfully written to:" << endl;
            cout << path << endl;
        }
    } else {
        cout << "ERROR : Could not generate <" << path << ">." << endl;
        return;
    }
    
    return;
}
