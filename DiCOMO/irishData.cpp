//
//  irishData.cpp
//  DiCOMO
//
//  Created by Maximilian Zangs on 13.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#include "irishData.h"

IrishData::IrishData(string path, bool inColumns, unsigned char cols, unsigned char rows, double scale) {
    // Check if the file exists
    _fileExists = false;
    
    ifstream input(path.c_str(), ios::binary);
    if (input.is_open()) {
        input.close();
        
        // Assign all variables since the file exists
        _pathToData = path;
        _profilesInColumn = inColumns;
        _ignoireCols = cols;
        _ignoireRows = rows;
        _scale = scale;
        _fileExists = true;
        
    } else {
        
        // Flag error since file can not be opened
        cout << "ERROR : Can not open file at:" << endl;
        cout << path << endl;
        return;
    }
    return;
}

IrishData::~IrishData() {
    // Destruct everything
}

void IrishData::loadData() {
    // Load the data from the input file
    // Make sure the file exists
    if (!_fileExists) {
        cout << "ERROR : File could not be read" << endl;
        return;
    }
    
    // The imput file
    ifstream input(_pathToData.c_str(), ios::binary);
    
    if (input.is_open()) {
        
        // Counts the rows that were jumped over to ignore them
        unsigned char rows = 0;
        
        while (input.good()) {
            // Buffer that will store the power value read from the datasheet
            vector<double> profile;
            
            // A line will be be read in at a time
            string line;
            
            // Counts the columns that will be ignored.
            // Needs to reset for each line
            unsigned char cols = 0;
            
            getline(input, line);
            line.push_back('\n');
            
            
            if (rows < _ignoireRows) {
                // If not enough rows have been ignored, do so and increment
                // counter by 1
                rows++;
            } else {
                // Whenever data is read (no comma or tab) then this value is
                // stored in this buffer
                string buffer;
                
                // Start reading through the read line
                for (string::iterator byte = line.begin();
                     byte !=line.end();
                     byte++) {
                    
                    switch (*byte) {
                        case '\t':
                        case '\n':
                        case ',':
                            if (cols < _ignoireCols) {
                                // If not enough columns have been ignored, do
                                // so and increlemt counter by 1
                                cols++;
                            } else {
                                // Append profile data into profile buffer
                                profile.push_back(atof(buffer.c_str()) * _scale);
                                
                                // And clear the buffer
                                buffer = "";
                                buffer.clear();
                            }
                            break;
                        default:
                            if (cols >= _ignoireCols) {
                                // Appends current value to the buffer
                                buffer.push_back(*byte);
                            }
                            break;
                    }
                }
            }
            
            // One data row has been read and needs to be stored into the power
            // profile matrix
            if (_profilesInColumn) {
                
                // Check if matrix is empty
                if (_powerProfiles.empty()) {
                    // If it is, then insert the 
                    for (int i = 0; i < profile.size(); i++) {
                        vector<double> emptyVector;
                        _powerProfiles.push_back(emptyVector);
                    }
                }
                
                // Then append each power value to the individual houses
                for (int i = 0; i < profile.size(); i++) {
                    _powerProfiles[i].push_back(profile[i]);
                }
                
            } else {
                // If each row contains a house's power profile,
                // simply insert it into the vector
                _powerProfiles.push_back(profile);
            }
        }
        
        input.close();
    }
}

dataSize IrishData::getDataSize() {
    dataSize size;
    
    // Make sure the matrix is not empty
    if (_powerProfiles.empty()) {
        cout << "ERROR : No data has been read in yet." << endl;
        return size;
    }
    
    // Get sizes from matrix and return the values
    size.houses = _powerProfiles.size();
    size.samples = _powerProfiles[0].size();
    
    return size;
}

double IrishData::getSampleForHouse(int delay, int house) {
    // Ensure the requested samples and houses lie within the data range
    dataSize maximumSize = getDataSize();
    if (delay >= maximumSize.samples || house >= maximumSize.houses) {
        cout << "ERROR : Data is out of the matrix bounds." << endl;
        return 0;
    }
    
    return _powerProfiles[house][delay];
}

void IrishData::applyProfilesToSim(Simulation *simulation, int startHouse, int houseCount, int delay, double powerFactor, int phases) {
    // Ensure the requested samples and houses lie within the data range
    dataSize maximumSize = getDataSize();
    if (   delay >= maximumSize.samples
        || startHouse >= maximumSize.houses
        || startHouse+houseCount >= maximumSize.houses) {
        cout << "ERROR : Data is out of the matrix bounds." << endl;
        return;
    }
    
    for (int i = 0; i < houseCount; i++) {
        simulation->addPowerToPhase(_powerProfiles[startHouse+i][delay], powerFactor, (i%phases)+1);
    }
}

