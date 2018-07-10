
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v3.1
//
//  Copyright (2015-2016)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#ifndef MEDYAN_Rand_h
#define MEDYAN_Rand_h

#include <stdio.h>
#include <random>

#include "common.h"

/// A random number generator class.
class Rand {
    
private:
    static uniform_int_distribution<int> _int_distr;
    
public:
    static mt19937 _eng;
#ifdef DEBUGCONSTANTSEED
    static long counter;
    static long Dcounter;
    static long Ncounter;
#endif
    ///Get a random double between low and high
    static inline double randDouble(double low, double high) {
#ifdef DEBUGCONSTANTSEED
        counter++;
        Dcounter++;
#endif
        return ((float)_int_distr(_eng) / numeric_limits<int>::max()) * (high - low) + low;
    }
    ///Get a random integer between low and high
    static inline int randInteger(int low, int high) {
        int y =_int_distr(_eng);
        int x = low + (y % (high - low + 1));
#ifdef DEBUGCONSTANTSEED
        counter++;
//        std::cout<<"RandomInteger "<<x<<" "<<y<<" "<<high<<" "<<low<<" "<<counter<<" "<<Dcounter<<" "<<Ncounter<<endl;
#endif
        return x;
    }
};


#endif
