
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v3.0
//
//  Copyright (2015)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#ifndef MEDYAN_LinkerStretching_h
#define MEDYAN_LinkerStretching_h

#include "common.h"

#include "LinkerInteractions.h"

//FORWARD DECLARATIONS
class Linker;

/// Represents a Linker stretching interaction
template <class LStretchingInteractionType>
class LinkerStretching : public LinkerInteractions {
    
private:
    LStretchingInteractionType _FFType;
    
public:
    virtual double computeEnergy(double d);
    virtual void computeForces();
    virtual void computeForcesAux();
    
    virtual const string getName() {return "Linker Stretching";}
};

#endif