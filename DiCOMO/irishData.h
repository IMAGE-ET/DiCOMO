//
//  irishData.h
//  DiCOMO
//
//  Created by Maximilian Zangs on 13.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#ifndef __DiCOMO__irishData__
#define __DiCOMO__irishData__

#include "simulation.h"

struct dataSize {
    size_t houses;
    size_t samples;
};

class IrishData {
    // The path to the data
    string _pathToData;
    // Whether the power profiles for each house are stored in a column
    // for the 22-weeks set this is <true>
    bool _profilesInColumn;
    // How many columns to ignore
    // for the 22-weeks set this is 2
    unsigned char _ignoireCols;
    // How many rows to ignore
    // for the 22-weeks set this is 3
    unsigned char _ignoireRows;
    // Value that translates the read data into 1W for the Irish dataset
    // ALWAYS 2000 becasue: Each sample is in kWh and recorded over 0.5h.
    double _scale;
    
    bool _fileExists;
    
    // Matrix where each row contains a vector of each house's power profile
    vector< vector<double> > _powerProfiles;
    
public:
    IrishData(string path, bool inColumns = true, unsigned char cols = 2, unsigned char rows = 3, double scale = 2000);
    ~IrishData();
    
    // Method that extracts the data from the file and stores it in a matrix
    void loadData();
    // Returns size of the matrix to ensure only valid houses and power profiles
    // can be extracted
    dataSize getDataSize();
    // Function that returns the power profile for a specific house at a
    // specific sample delay
    double getSampleForHouse(int delay, int house);
    
    // Functions that apply the power profiles to a "DiCOMO" simulation
    void applyProfilesToSim(Simulation *simulation, int startHouse, int houseCount, int delay, double powerFactor = 1.0, int phases = 1);
};


#endif /* defined(__DiCOMO__irishData__) */
