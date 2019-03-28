
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v3.2.1
//
//  Copyright (2015-2018)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#ifndef MEDYAN_BoundaryBubbleRepulsionExp_h
#define MEDYAN_BoundaryBubbleRepulsionExp_h

#include <vector>
#include <cmath>

#include "common.h"

//FORWARD DECLARATIONS
class Bead;
class BoundaryElement;

/// A exponential repulsive potential used by the BoundaryBubbleRepulsion template.
class BoundaryBubbleRepulsionExp {
    
public:
    double energy(double *coord, int *beadSet,
                  double *krep, double *slen, int *nneighbors);
    
    double energy(double *coord, double *f, int *beadSet,
                  double *krep, double *slen, int *nnneighbors, double d);
    
    void forces(double *coord, double *f, int *beadSet,
                double *krep, double *slen, int *nneighbors);

};

#endif
