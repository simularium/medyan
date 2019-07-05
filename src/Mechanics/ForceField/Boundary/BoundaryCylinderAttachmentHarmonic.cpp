
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

#include "BoundaryCylinderAttachmentHarmonic.h"
#include "BoundaryCylinderAttachment.h"

#include "Bead.h"

#include "MathFunctions.h"

using namespace mathfunc;

floatingpoint BoundaryCylinderAttachmentHarmonic::energy(floatingpoint *coord, int *beadSet,
                                                  floatingpoint *kattr, floatingpoint *pins) {
    

    int n = BoundaryCylinderAttachment<BoundaryCylinderAttachmentHarmonic>::n;
    int nint = Bead::getPinnedBeads().size();

    floatingpoint *coord1, *pin1, distsq, U_i;
    floatingpoint U = 0;

    for(int i = 0; i < nint; i += 1) {

        coord1 = &coord[3 * beadSet[n * i]];
        pin1 = &pins[3 * beadSet[n * i]];

        distsq = twoPointDistancesquared(coord1, pin1);
        U_i = 0.5 * kattr[i] * distsq;

        if(fabs(U_i) == numeric_limits<floatingpoint>::infinity()
           || U_i != U_i || U_i < -1.0) {

            //set culprit and return
            BoundaryInteractions::_otherCulprit = Bead::getPinnedBeads()[i];

            return -1;
        }

        U += U_i;
    }
    return U;
}

floatingpoint BoundaryCylinderAttachmentHarmonic::energy(floatingpoint *coord, floatingpoint *f, int *beadSet,
                                                  floatingpoint *kattr, floatingpoint *pins, floatingpoint d) {


    int n = BoundaryCylinderAttachment<BoundaryCylinderAttachmentHarmonic>::n;
    int nint = Bead::getPinnedBeads().size();

    floatingpoint *coord1, *pin1, *zero, dist, U_i;
    floatingpoint *force1;
    floatingpoint U = 0;
    zero = new floatingpoint[3]; zero[0] = 0; zero[1] = 0; zero[2] = 0;

    for(int i = 0; i < nint; i += 1) {

        coord1 = &coord[3 * beadSet[n * i]];
        force1 = &f[3 * beadSet[n * i]];

        pin1 = &pins[3 * beadSet[n * i]];

        dist = twoPointDistanceStretched(coord1, force1, pin1, zero, d);
        U_i = 0.5 * kattr[i] * dist * dist;

        if(fabs(U_i) == numeric_limits<floatingpoint>::infinity()
           || U_i != U_i || U_i < -1.0) {

            //set culprit and return
            BoundaryInteractions::_otherCulprit = Bead::getPinnedBeads()[i];

            return -1;
        }

        U += U_i;
    }
    delete zero;
    return U;
}

void BoundaryCylinderAttachmentHarmonic::forces(floatingpoint *coord, floatingpoint *f, int *beadSet,
                                                floatingpoint *kattr, floatingpoint *pins) {
    
    int n = BoundaryCylinderAttachment<BoundaryCylinderAttachmentHarmonic>::n;
    int nint = Bead::getPinnedBeads().size();

    floatingpoint *coord1, *pin1, *dir, dist, f0;
    floatingpoint *force1;
    dir = new floatingpoint[3];

    for(int i = 0; i < nint; i += 1) {

        coord1 = &coord[3 * beadSet[n * i]];
        force1 = &f[3 * beadSet[n * i]];
        pin1 = &pins[3 * beadSet[n * i]];

        dist = twoPointDistance(coord1, pin1);
        if(areEqual(dist, 0.0)) return;

        twoPointDirection(dir, coord1, pin1);
        f0 = kattr[i] * dist;

        force1[0] = f0 * dir[0];
        force1[1] = f0 * dir[1];
        force1[2] = f0 * dir[2];
    }
    delete dir;
}
