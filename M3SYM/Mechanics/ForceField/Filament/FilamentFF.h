
//------------------------------------------------------------------
//  **M3SYM** - Simulation Package for the Mechanochemical
//              Dynamics of Active Networks, 3rd Generation
//
//  Copyright (2014) Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the Papoian lab page for more information:
//  http://papoian.chem.umd.edu/
//------------------------------------------------------------------

#ifndef M3SYM_FilamentFF_h
#define M3SYM_FilamentFF_h

#include <vector>

#include "common.h"

#include "ForceField.h"

//FORWARD DECLARATIONS
class FilamentInteractions;
class Filament;

/// An implementation of the ForceField class that calculates Filament interactions.
class FilamentFF : public ForceField {
 
private:
    vector<unique_ptr<FilamentInteractions>>
    _filamentInteractionVector; ///< Vector of initialized filament interactions
    
    FilamentInteractions* _culpritInteraction; ///< Culprit in case of error
public:
    /// Constructor, intializes stretching, bending, and twisting forces
    FilamentFF(string& stretching, string& bending, string& twisting);
    
    virtual string getName() {return "Filament";}
    virtual void whoIsCulprit();
    
    virtual double computeEnergy(double d);
    virtual void computeForces();
    virtual void computeForcesAux();
    
    virtual vector<NeighborList*> getNeighborLists() {return vector<NeighborList*>{};}
};

#endif
