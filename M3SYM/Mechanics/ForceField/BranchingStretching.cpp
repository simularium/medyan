
//------------------------------------------------------------------
//  **M3SYM** - Simulation Package for the Mechanochemical
//              Dynamics of Active Networks, 3rd Generation
//
//  Copyright (2014) Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the Papoian lab page for installation and documentation:
//  http://papoian.chem.umd.edu/
//------------------------------------------------------------------

#include "BranchingStretching.h"

#include "BranchingStretchingHarmonic.h"

#include "BranchingPoint.h"
#include "Cylinder.h"
#include "Bead.h"

template <class BStretchingInteractionType>
double BranchingStretching<BStretchingInteractionType>::computeEnergy(BranchingPoint* b, double d) {
    
    Bead* b1 = b->getFirstCylinder()->getFirstBead();
    Bead* b2 = b->getFirstCylinder()->getSecondBead();
    Bead* b3 = b->getSecondCylinder()->getFirstBead();

    double kStretch = b->getMBranchingPoint()->getStretchingConstant();
    double eqLength = b->getMBranchingPoint()->getEqLength();
    double position = b->getPosition();
    
    if (d == 0.0)
        return _FFType.energy(b1, b2, b3, position, kStretch, eqLength);
    else
        return _FFType.energy(b1, b2, b3, position, kStretch, eqLength, d);
    
}

template <class BStretchingInteractionType>
void BranchingStretching<BStretchingInteractionType>::computeForces(BranchingPoint* b) {
    
    Bead* b1 = b->getFirstCylinder()->getFirstBead();
    Bead* b2 = b->getFirstCylinder()->getSecondBead();
    Bead* b3 = b->getSecondCylinder()->getFirstBead();
    
    double kStretch = b->getMBranchingPoint()->getStretchingConstant();
    double eqLength = b->getMBranchingPoint()->getEqLength();
    double position = b->getPosition();
    
    _FFType.forces(b1, b2, b3, position, kStretch, eqLength);
    
}


template <class BStretchingInteractionType>
void BranchingStretching<BStretchingInteractionType>::computeForcesAux(BranchingPoint* b) {
    
    Bead* b1 = b->getFirstCylinder()->getFirstBead();
    Bead* b2 = b->getFirstCylinder()->getSecondBead();
    Bead* b3 = b->getSecondCylinder()->getFirstBead();
    
    double kStretch = b->getMBranchingPoint()->getStretchingConstant();
    double eqLength = b->getMBranchingPoint()->getEqLength();
    double position = b->getPosition();
    
    _FFType.forcesAux(b1, b2, b3, position, kStretch, eqLength);
    
}


///Template specializations
template double BranchingStretching<BranchingStretchingHarmonic>::computeEnergy(BranchingPoint* b, double d);
template void  BranchingStretching<BranchingStretchingHarmonic>::computeForces(BranchingPoint* b);
template void  BranchingStretching<BranchingStretchingHarmonic>::computeForcesAux(BranchingPoint* b);
