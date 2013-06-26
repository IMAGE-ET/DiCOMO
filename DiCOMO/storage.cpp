//
//  storage.cpp
//  DiCOMO
//
//  Created by Maximilian Zangs on 10.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#include "storage.h"

Storage::Storage(complex<double> vcc, complex<double> vss) : Consumer(vcc, vss) {
    _elementType = STORAGE;
}

Storage::~Storage() {
    
}
