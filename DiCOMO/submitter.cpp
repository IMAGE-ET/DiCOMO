//
//  submitter.cpp
//  DiCOMO
//
//  Created by Maximilian Zangs on 24.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#include "submitter.h"

Submitter::Submitter(bool verbose) {
    // Generates new instance of simulation
    _simulation = new Simulation(verbose);
    
    _verbose = verbose;
    
    _startHouse = 0;
    _feederLenth = 0;
    _sample = 0;
    _powerFactor = 1.0;
}

Submitter::~Submitter() {
    // Delete simulation
    if (_simulation)
        _simulation->~Simulation();
    _simulation = NULL;
}

void Submitter::setValues(int argc, const char * argv[]) {
    int settingCounter = Error;
    
    // Runs through each passed variable and tries to interpret it
    for (int i = 0; i < argc; i++) {
        // Command begin (i.e. "-h" or "-d")
        if (argv[i][0] == '-') {
            
            // Ensure a whole command has been passed
            if (sizeof( argv[i] ) == sizeof(char)) {
                cout << "ERROR : Can not interpret <" << argv[i] << ">" << endl;
                settingCounter = Error;
                break;
            }
            
            switch (argv[i][1]) {
                case 'a':
                    about();
                    break;
                
                case 'd':
                    // Next the sample counter (delay) will be set up
                    settingCounter = Sample;
                    break;
                
                case 'f':
                    // Next the power factor will be set up
                    settingCounter = PowerFactor;
                    break;
                    
                case 'h':
                    if (argc > 2) {
                        // If the help was intended for a previous flag
                        // then display this flag's information
                        help(argv[i-1]);
                    } else {
                        // Otherwise simply
                        help();
                    }
                    break;
                    
                case 'i':
                    // Next the irish data source path will be set up
                    settingCounter = IrishDataSetup;
                    break;
                    
                case 'l':
                    // Next the feeder lenth is passed
                    settingCounter = FeederLength;
                    break;
                    
                case 'o':
                    // Next the output file path will be stored
                    settingCounter = OutputFile;
                    break;
                    
                case 'p':
                    // Setup of phases
                    settingCounter = Phases;
                    break;
                    
                case 'r':
                    run();
                    break;
                    
                case 's':
                    // Next the which house will be started with will be set up
                    settingCounter = StartHouse;
                    break;
                    
                case 'v':
                    // Setting voltages
                    
                    // Ensuring the length of the passed parameter is 2
                    if (!argv[i][2]) {
                        
                        // Make sure we don't interfer with a help call...
                        if (argc > i-1 && argv[i+1][1] == 'h')
                            break;
                        
                        cout << "ERROR : Can not set voltage with <" << argv[i] << ">" << endl;
                        settingCounter = Error;
                        break;
                    }
                    
                    switch (argv[i][2]) {
                        case 'c':
                            settingCounter = SourceVoltage;
                            break;
                        case 's':
                            settingCounter = SinkVoltage;
                            break;
                        default:
                            cout << "ERROR : Can not understand voltage <" << argv[i] << ">" << endl;
                            settingCounter = Error;
                            break;
                    }
                    break;
                    
                default:
                    settingCounter = Error;
                    break;
            }
            
        } else {
            
            // Ensure that the requested command can be interpreted
            // and that the value passed is of adequate length
            if (settingCounter != Error && argv[i]) {
                switch (settingCounter) {
                        
                    case SourceVoltage:
                        _simulation->setSource(atof(argv[i]));
                        if (_verbose)
                            cout << setw(30) << "Source set to: " << argv[i] << endl;
                        break;
                        
                    case SinkVoltage:
                        _simulation->setSink(atof(argv[i]));
                        if (_verbose)
                            cout << setw(30) << "Sink set to: " << argv[i] << endl;
                        break;

                    case IrishDataSetup:
                        _irishData = new IrishData(argv[i]);
                        _irishData->loadData();
                        if (_verbose)
                            cout << setw(30) << "Irish Data loaded from: " << argv[i] << endl;
                        break;
                        
                    case Phases:
                        _simulation->setPhases(atoi(argv[i]));
                        if (_verbose)
                            cout << setw(30) << "Number of Phases set to: " << argv[i] << endl;
                        break;
                        
                    case StartHouse:
                        _startHouse = atoi(argv[i]);
                        if (_verbose)
                            cout << setw(30) << "First House set to: " << argv[i] << endl;
                        break;
                        
                    case FeederLength:
                        _feederLenth = atoi(argv[i]);
                        if (_verbose)
                            cout << setw(30) << "Feeder length set to: " << argv[i] << endl;
                        break;
                        
                    case Sample:
                        _sample = atoi(argv[i]);
                        if (_verbose)
                            cout << setw(30) << "Sample set to: " << argv[i] << endl;
                        break;
                    
                    case PowerFactor:
                        _powerFactor = atof(argv[i]);
                        if (_verbose)
                            cout << setw(30) << "Power factor set to: " << argv[i] << endl;
                        break;
                        
                    case OutputFile:
                        _outputFilePath = argv[i];
                        if (_verbose)
                            cout << setw(30) << "Output path set to: " << _outputFilePath << endl;
                        break;
                        
                    default:
                        break;
                }
            }
            
        }
        
    }
    
}

void Submitter::help(const char *argv) {
    cout << "HELP : This will explain everything! Hopefully..." << endl;
    cout << endl;
    if (!argv) {
        cout << "When executing the program, multiple values can be passed" << endl;
        cout << "as arguments. These arguments are passed on launch and are" << endl;
        cout << "simply appended to the binary launch command. E.g." << endl;
        cout << endl;
        cout << " ./DiCOMO -h     This command passed the argument '-h' and" << endl;
        cout << "                 allowed to display this help text." << endl;
        cout << endl;
        cout << "Some arguments require not only to be flagged, but need a" << endl;
        cout << "variable to be passed, too. This is done by sending the" << endl;
        cout << "argument flag first and then the value. E.g." << endl;
        cout << endl;
        cout << " ./DiCOMO -p 3   This command tels the program that '-p'" << endl;
        cout << "                 with the value '3' is to be sent as" << endl;
        cout << "                 launch arguments." << endl;
        cout << endl;
        cout << "IMPORTANT!" << endl;
        cout << "When passing strings (i.e. paths) to the program that may" << endl;
        cout << "contain spaces, then please make sure they are passed in" << endl;
        cout << "two inverted commas. E.g." << endl;
        cout << endl;
        cout << " ./DiCOMO -o \"/path with spaces/out\"" << endl;
        cout << endl;
        cout << "A cumulative list of all variables (in prefered order) is" << endl;
        cout << "given here. For more information regarding the specific" << endl;
        cout << "launch argument, simply call it with its help '-h' flag." << endl;
        cout << endl;
        cout << " -t                   verbose" << endl;
        cout << " -a                   about" << endl;
        cout << " -h                   help" << endl;
        cout << " -i    <path>         irish data" << endl;
        cout << " -s    <+ve num>      start house" << endl;
        cout << " -d    <+ve num>      sample dalay" << endl;
        cout << " -o    <path>         output data" << endl;
        cout << " -p    <+ve num>      phases" << endl;
        cout << " -v<n> <+ve num>      voltages" << endl;
        cout << " -l    <+ve num>      length of feeder(s)" << endl;
        cout << " -r                   run" << endl;
        return;
    }
    
    switch (argv[1]) {
        case 't':
            cout << "-t" << endl;
            cout << endl;
            cout << "This command sets the verbose flag and allows the output" << endl;
            cout << "all commands on screen. Very handy when debugging or not" << endl;
            cout << "saving the output to an output file. Also gives insight" << endl;
            cout << "into the current status of execution." << endl;
            cout << endl;
            cout << "Simply prepend this argument in front of all other flags." << endl;
            cout << "It does not require any values or so to be passed." << endl;
            break;
            
        case 'a':
            cout << "-a" << endl;
            cout << endl;
            cout << "The program will display a quick about message when this" << endl;
            cout << "flag is passed. This message is simply for intellectual" << endl;
            cout << "property and warranty purposes and has no functionality" << endl;
            cout << "for the actual simulation execution." << endl;
            
        case 'h':
            cout << "-h" << endl;
            cout << endl;
            cout << "This is the help flah that displays the help messages." << endl;
            break;
            
        case 'i':
            cout << "-i    <path>" << endl;
            cout << endl;
            cout << "This command allows you to set up the Irish data set. It" << endl;
            cout << "consists of 542 anonymous households where each supplies" << endl;
            cout << "a 22 week long power profile sampled at 30 minute inter-" << endl;
            cout << "vals, thus containing 7392 samples." << endl;
            cout << "This numbers are also the maximum values that can be set" << endl;
            cout << "for the 'start house' and 'sample delay' values. If they" << endl;
            cout << "are not set, then they are defaulted to ZERO and ZERO" << endl;
            cout << "respectively." << endl;
            cout << endl;
            cout << "With this command the dataset is loaded. When passing the" << endl;
            cout << "path string, ensure that it is encapsulated in inverted" << endl;
            cout << "commas like so: \"data folder/data file.txt\"." << endl;
            break;
            
        case 's':
            cout << "-s    <+ve num>" << endl;
            cout << endl;
            cout << "Once the irish dataset has been set up with the '-i' flag" << endl;
            cout << "then this allows to set the starting house. Depending on" << endl;
            cout << "the length of the feeder and the number of phases applied" << endl;
            cout << "determine how many consecutive household power values are" << endl;
            cout << "extracted. The value passed here must be greater or equal" << endl;
            cout << "to zero and determines the start from where the power" << endl;
            cout << "values are extractec. A sample use would be:" << endl;
            cout << endl;
            cout << " ./DiCOMO -s 3  To set the starting house to house number" << endl;
            cout << "                three." << endl;
            cout << endl;
            break;
            
        case 'd':
            cout << "-d    <+ve num>" << endl;
            cout << endl;
            cout << "This sets up the sample delay which chooses which sample" << endl;
            cout << "will be extracted form the data and inserted into the" << endl;
            cout << "simulation. The Irish data set is 7392 samples long which" << endl;
            cout << "makes the maximum value 7392. The minimum value passed" << endl;
            cout << "must therefore be zero. This is also the default value." << endl;
            cout << "A sample implementation would be:" << endl;
            cout << endl;
            cout << " ./DiCOMO -d 3  To set the third power value to be extra-" << endl;
            cout << "                cted from the data set." << endl;
            cout << endl;
            break;
            
        case 'o':
            cout << "-o    <path>" << endl;
            cout << endl;
            cout << "The output data is stored at the path in the file name" << endl;
            cout << "passed in the path variable. If any spaces are passed" << endl;
            cout << "then the path must be encapsulated in inverted commas" << endl;
            cout << "like so: \"path to folder/output\". It is also important" << endl;
            cout << "not to append a suffix to the output file since this is" << endl;
            cout << "done automatically." << endl;
            cout << endl;
            break;
            
        case 'p':
            cout << "-p    <+ve num>" << endl;
            cout << endl;
            cout << "This sets up the number of phases that is applied to the" << endl;
            cout << "simulation. The minimum value must be one and the maximum" << endl;
            cout << "value is unspecified." << endl;
            cout << endl;
            break;
        
        case 'v':
            cout << "-v<n> <+ve num>" << endl;
            cout << endl;
            cout << "The source and sink (ground) voltages are set up via this" << endl;
            cout << "command. The source Vcc is set up by passing 'c' as the" << endl;
            cout << "'n' parameter and the value for this is passed as the num" << endl;
            cout << "value which may be a decimal value, too. E.g." << endl;
            cout << endl;
            cout << " ./DiCOMO -vc 240.0    To set up the supply voltage" << endl;
            cout << endl;
            cout << "The same applies to sink/ground Vss where 's' is passed as" << endl;
            cout << "rge 'n' parameter. E.g." << endl;
            cout << endl;
            cout << " ./DiCOMO -vs 0.0      To set up the ground voltage" << endl;
            cout << endl;
            cout << "The voltages are atomatically rotated for different pases" << endl;
            cout << endl;
            break;
            
        case 'l':
            cout << "-l    <+ve num>" << endl;
            cout << endl;
            cout << "The length of the feeder/s is set up with this command." << endl;
            cout << "When multiple phases are selected, then the length times" << endl;
            cout << "the number of phases determines how many houses are" << endl;
            cout << "extracetd from the Irish data set." << endl;
            cout << endl;
            break;
            
        case 'r':
            cout << "-r" << endl;
            cout << endl;
            cout << "Runs the simulation and should be passed last." << endl;
            cout << endl;
            break;
            
        default:
            break;
    }
    
}

void Submitter::about() {
    cout << endl;
    cout << "           .:'       |  This piece of software has been" << endl;
    cout << "       __ :'__       |  developed, coded and tested by" << endl;
    cout << "    .'`  `-'  ``.    |    Maximilian J. Zangs." << endl;
    cout << "   :  YES     .-'    |  " << endl;
    cout << "   :   I     :       |           .:DISCLAIMER:." << endl;
    cout << "    :   KNOW  `-;    |  No guarantee of functionality or" << endl;
    cout << "     `.__.-.__.'     |  any other warranty when executed" << endl;
    cout << endl;
}

void Submitter::run() {
    // Error checking before execution
    
    if (!_simulation) {
        cout << "ERROR : Simulation not correctly set up" << endl;
        return;
    }
    
    if (!_irishData) {
        cout << "ERROR : The irish data power profile must be used for this type of simulaton" << endl;
        return;
    }
    
    if (_feederLenth == 0) {
        cout << "ERROR : Feeder of length ZERO selected (not set up properly)" << endl;
        return;
    }
    
    _feederLenth = _feederLenth * _simulation->getPhases();
    
    // Applying data from irish data
//    _irishData->applyProfilesToSim(_simulation, _startHouse, _feederLenth, _sample, _powerFactor, _simulation->getPhases());

    for (int i = 0; i < _feederLenth; i++)
        _simulation->addFeederImpedanceForPhase(complex<double>(0.01*_simulation->getPhases(), 0.0), (i%_simulation->getPhases())+1);
    for (int i = 0; i < _feederLenth; i++)
        _simulation->addReturnImpedance(complex<double>(0.01, 0.0));

    for (int i = 0; i < _feederLenth; i++)
//        dicomo->addPowerToPhase((double)(rand()%350) + 150.0, (double)(rand()%21)/100.0 + 0.8, (i%numberOfPhases)+1);
        _simulation->addPowerToPhase(900, 1.0, (i%_simulation->getPhases())+1);
    
    _simulation->start();
    _simulation->saveFeeders(_outputFilePath, true);
    _simulation->saveSubstation(_outputFilePath, true);

    _simulation->~Simulation();
    _simulation = NULL;
    
}
