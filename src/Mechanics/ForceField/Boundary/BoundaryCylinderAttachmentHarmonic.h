
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

#ifndef MEDYAN_BoundaryCylinderAttachmentHarmonic_h
#define MEDYAN_BoundaryCylinderAttachmentHarmonic_h

#include <vector>
#include <cmath>

#include "common.h"

//FORWARD DECLARATIONS
class Bead;

/// A harmonic attractive potential used by the BoundaryCylinderAttachment template.
class BoundaryCylinderAttachmentHarmonic {
    
public:
    double energy(Bead*, double);
    double energy(Bead*, double, double);
    
    void forces(Bead*, double);
    void forcesAux(Bead*, double);
};

#endif