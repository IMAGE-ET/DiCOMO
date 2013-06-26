//
//  backbone.cpp
//  DiCOMO
//
//  Created by Maximilian Zangs on 09.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#include "backbone.h"

bool compareComplexAbs (complex<double> i,complex<double> j) {
    return (abs(i) < abs(j));
}

bool compareComplexReal (complex<double> i,complex<double> j) {
    return (i.real() < j.real());
}

bool compareComplexImag (complex<double> i,complex<double> j) {
    return (i.imag() < j.imag());
}
