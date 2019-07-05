
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

#ifndef MEDYAN_LinkerStretchingHarmonic_h
#define MEDYAN_LinkerStretchingHarmonic_h

#include "common.h"

//FORWARD DECLARATIONS
class Bead;

/// A harmonic potential used by the LinkerStretching template.
class LinkerStretchingHarmonic {
    
public:
    floatingpoint energy(floatingpoint *coord, floatingpoint *f, int *beadSet,
                  floatingpoint *kstr, floatingpoint *eql, floatingpoint *pos1, floatingpoint *pos2);
    
    floatingpoint energy(floatingpoint *coord, floatingpoint * f, int *beadSet,
                  floatingpoint *kstr, floatingpoint *eql, floatingpoint *pos1, floatingpoint *pos2, floatingpoint d);
    
    void forces(floatingpoint *coord, floatingpoint *f, int *beadSet,
                floatingpoint *kstr, floatingpoint *eql, floatingpoint *pos1, floatingpoint *pos2, floatingpoint
                *stretchforce);
#ifdef CUDAACCL
    void optimalblocksnthreads(int nint, cudaStream_t stream);

    floatingpoint* energy(floatingpoint *coord, floatingpoint *f, int *beadSet, floatingpoint *kstr, floatingpoint *eql,
                   floatingpoint *pos1, floatingpoint *pos2, int *params);

    floatingpoint* energy(floatingpoint *coord, floatingpoint *f, int *beadSet, floatingpoint *kstr, floatingpoint *eql, floatingpoint *pos1, floatingpoint *pos2,
                   floatingpoint *z, int *params);

    void forces(floatingpoint *coord, floatingpoint *f, int *beadSet, floatingpoint *kstr, floatingpoint *eql, floatingpoint *pos1, floatingpoint *pos2, int
    *params, floatingpoint *Lstretchforce);
    void deallocate();
    vector<int> blocksnthreadse;
    vector<int> blocksnthreadsez;
    vector<int> blocksnthreadsf;
    vector<int> bntaddvec2;
    static void checkforculprit();
    floatingpoint *gU_i;
    floatingpoint *gU_sum;
    char *gFF, *ginteraction;
    cudaStream_t stream = NULL;
    int numint;
#endif
#ifdef CROSSCHECK
    floatingpoint energy(Bead*, Bead*, Bead*, Bead*,
                  floatingpoint position1, floatingpoint position2,
                  floatingpoint kStretch, floatingpoint eqLength);
    floatingpoint energy(Bead*, Bead*, Bead*, Bead*,
                  floatingpoint position1, floatingpoint position2,
                  floatingpoint kStretch, floatingpoint eqLength, floatingpoint d);
    
    floatingpoint forces(Bead*, Bead*, Bead*, Bead*,
                  floatingpoint position1, floatingpoint position2,
                  floatingpoint kStretch, floatingpoint eqLength);
    floatingpoint forcesAux(Bead*, Bead*, Bead*, Bead*,
                     floatingpoint position1, floatingpoint position2,
                     floatingpoint kStretch, floatingpoint eqLength);
#endif
};

#endif
