
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

#ifndef MEDYAN_MotorGhostStretching_h
#define MEDYAN_MotorGhostStretching_h

#include "common.h"

#include "MotorGhostInteractions.h"

//FORWARD DECLARATIONS
class MotorGhost;

/// Represents a MotorGhost stretching interaction.
template <class MStretchingInteractionType>
class MotorGhostStretching : public MotorGhostInteractions {
    
private:
    MStretchingInteractionType _FFType;

public:
    virtual double computeEnergy(double d);
    virtual void computeForces();
    virtual void computeForcesAux();
    
    virtual const string getName() {return "Motor Stretching";}
};

#endif