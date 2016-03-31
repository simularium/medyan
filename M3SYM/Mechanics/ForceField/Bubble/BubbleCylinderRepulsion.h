
//------------------------------------------------------------------
//  **M3SYM** - Simulation Package for the Mechanochemical
//              Dynamics of Active Networks, 3rd Generation
//
//  Copyright (2015)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#ifndef M3SYM_BubbleCylinderRepulsion_h
#define M3SYM_BubbleCylinderRepulsion_h

#include <vector>

#include "common.h"

#include "BubbleInteractions.h"
#include "NeighborListImpl.h"

#include "SysParams.h"

//FORWARD DECLARATIONS
class Bead;

/// Represents a repulsive interaction between a Bubble and Cylinder.
template <class BRepulsionInteractionType>
class BubbleCylinderRepulsion : public BubbleInteractions {
    
private:
    BRepulsionInteractionType _FFType;
    BubbleCylinderNL* _neighborList; ///<Neighbor list of Bubble-Cylinder
public:
    
    /// Constructor
    BubbleCylinderRepulsion() {
        _neighborList = new BubbleCylinderNL(SysParams::Mechanics().BubbleCutoff);
    }
    
    virtual double computeEnergy(double d);
    
    virtual void computeForces();
    virtual void computeForcesAux();
    
    /// Get the neighbor list for this interaction
    virtual NeighborList* getNeighborList() {return _neighborList;}
    
    virtual const string getName() {return "Bubble-Cylinder Repulsion";}
};

#endif
