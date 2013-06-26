//
//  main.cpp
//  DiCOMO
//
//  Created by Maximilian Zangs on 09.06.13.
//  Copyright (c) 2013 Maximilian J. Zangs. All rights reserved.
//

#include "submitter.h"

int main(int argc, const char * argv[])
{

    // insert code here...
    Submitter *sumbitter = new Submitter(true);
    sumbitter->setValues(argc, argv);
    return 0;
}

