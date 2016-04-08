
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

#ifndef MEDYAN_BubbleFF_h
#define MEDYAN_BubbleFF_h

#include <vector>

#include "common.h"

#include "ForceField.h"

//FORWARD DECLARATIONS
class BubbleInteractions;
class Bead;

/// An implementation of the ForceField class that calculates Bubble
/// repulsion and attraction to [Beads](@ref Bead) in the system.
class BubbleFF : public ForceField {
    
private:
    vector<unique_ptr<BubbleInteractions>>
    _bubbleInteractionVector; ///< Vector of initialized bubble interactions
    
    /// The culprit in the case of an error
    BubbleInteractions* _culpritInteraction;
public:
    /// Initialize the forcefields
    BubbleFF(string type, string mtoc);
    
    virtual string getName() {return "Bubble";}
    virtual void whoIsCulprit();
    
    virtual double computeEnergy(double d);
    virtual void computeForces();
    virtual void computeForcesAux();
    
    virtual vector<NeighborList*> getNeighborLists();
};

#endif
