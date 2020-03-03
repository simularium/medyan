
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

#ifndef MEDYAN_MTOCBending_h
#define MEDYAN_MTOCBending_h

#include <vector>

#include "common.h"

#include "BubbleInteractions.h"

#include "SysParams.h"

//FORWARD DECLARATIONS
class MTOC;
class Bead;

/// Represents an attachment potential of a MTOC.
template <class MTOCInteractionType>
class MTOCBending : public BubbleInteractions {
    
private:
    MTOCInteractionType _FFType;
    
    int *beadSet;
    ///Array describing the constants in calculation
    floatingpoint *kbend;
    floatingpoint *pos1;
    floatingpoint *pos2;
    
    
public:
    
    ///Array describing indexed set of interactions
    ///For MTOC, this is a 2-bead potential + fixed MTOC bead
    const static int n = 2;
    
    virtual void vectorize();
    virtual void deallocate();
    
    virtual floatingpoint computeEnergy(floatingpoint *coord, bool stretched) override;
    virtual void computeForces(floatingpoint *coord, floatingpoint *f);
    //virtual void computeForcesAux(floatingpoint *coord, floatingpoint *f);
    
    virtual void computeLoadForces() {return;}
    
    /// Get the neighbor list for this interaction
    virtual NeighborList* getNeighborList() {return nullptr;}
    
    virtual const string getName() {return "MTOC Attachment";}
};

#endif
