//
//  storage.h
//  DiCOMO
//
//  Created by Maximilian Zangs on 10.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#ifndef __DiCOMO__storage__
#define __DiCOMO__storage__

#include "consumer.h"

class Storage : public Consumer {
protected:
    
public:
    Storage(complex<double> vcc, complex<double> vss);
    ~Storage();
    
protected:
    
public:
    
};

#endif /* defined(__DiCOMO__storage__) */
