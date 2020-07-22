
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v4.0
//
//  Copyright (2015-2018)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#ifndef MEDYAN_MTOCAttachmentHarmonic_h
#define MEDYAN_MTOCAttachmentHarmonic_h

#include "common.h"

//FORWARD DECLARATIONS
class Bead;

/// A harmonic potential used by the MTOCAttachment template.
class MTOCAttachmentHarmonic {
    
public:

    floatingpoint energy(
        floatingpoint *coord,
        int *beadSet,
        std::size_t beadStartIndex,
        std::size_t bubbleStartIndex,
        floatingpoint *kstr,
        floatingpoint* radiusvec
    ) const;
    void forces(floatingpoint *coord, floatingpoint *f, int *beadSet,
                floatingpoint *kstr, floatingpoint* radiusvec);

    
    //void forcesAux(Bead*, Bead*, double, double);
};

#endif
