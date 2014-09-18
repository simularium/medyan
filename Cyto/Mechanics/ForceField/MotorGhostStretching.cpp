//
//  MotorGhostStretching.cpp
//  Cyto
//
//  Created by Konstantin Popov on 9/3/14.
//  Copyright (c) 2014 University of Maryland. All rights reserved.
//

#include "MotorGhostStretching.h"
#include "MotorGhostStretchingHarmonic.h"
#include "MotorGhostDB.h"
#include "Cylinder.h"
#include "Bead.h"

template <class MStretchingInteractionType>
double MotorGhostStretching<MStretchingInteractionType>::ComputeEnergy(MotorGhost* pm, double d)
{
    Bead* pb1 = pm->GetFirstCylinder()->getMCylinder()->GetFirstBead();
    Bead* pb2 = pm->GetFirstCylinder()->getMCylinder()->GetSecondBead();
    Bead* pb3 = pm->GetSecondCylinder()->getMCylinder()->GetFirstBead();
    Bead* pb4 = pm->GetSecondCylinder()->getMCylinder()->GetSecondBead();
    double kStretch = pm->GetStretchingConstant();
    double L = pm->GetEqLength();
    
    double pos1 = pm->GetFirstPosition();
    double pos2 = pm->GetSecondPosition();
    
    if (d == 0.0)
        return _FFType.Energy(pb1, pb2, pb3, pb4, pos1, pos2, kStretch, L);
    else
        return _FFType.Energy(pb1, pb2, pb3, pb4, pos1, pos2, kStretch, L, d);   ///This type of function needed for conjugated gradient minimisation only;
    
}

template <class MStretchingInteractionType>
void MotorGhostStretching<MStretchingInteractionType>::ComputeForces(MotorGhost* pm)
{
    Bead* pb1 = pm->GetFirstCylinder()->getMCylinder()->GetFirstBead();
    Bead* pb2 = pm->GetFirstCylinder()->getMCylinder()->GetSecondBead();
    Bead* pb3 = pm->GetSecondCylinder()->getMCylinder()->GetFirstBead();
    Bead* pb4 = pm->GetSecondCylinder()->getMCylinder()->GetSecondBead();
    double kStretch = pm->GetStretchingConstant();
    double L = pm->GetEqLength();
    
    double pos1 = pm->GetFirstPosition();
    double pos2 = pm->GetSecondPosition();
    
    _FFType.Forces(pb1, pb2, pb3, pb4, pos1, pos2, kStretch, L);
    
}


template <class MStretchingInteractionType>
void MotorGhostStretching<MStretchingInteractionType>::ComputeForcesAux(MotorGhost* pm) /// Needed for Conjugated Gradient minimization;
{
    Bead* pb1 = pm->GetFirstCylinder()->getMCylinder()->GetFirstBead();
    Bead* pb2 = pm->GetFirstCylinder()->getMCylinder()->GetSecondBead();
    Bead* pb3 = pm->GetSecondCylinder()->getMCylinder()->GetFirstBead();
    Bead* pb4 = pm->GetSecondCylinder()->getMCylinder()->GetSecondBead();
    double kStretch = pm->GetStretchingConstant();
    double L = pm->GetEqLength();
    
    double pos1 = pm->GetFirstPosition();
    double pos2 = pm->GetSecondPosition();
    
    _FFType.ForcesAux(pb1, pb2, pb3, pb4, pos1, pos2, kStretch, L);
    
}

///Template specializations
template double MotorGhostStretching<MotorGhostStretchingHarmonic>::ComputeEnergy(MotorGhost* pm, double d);
template void  MotorGhostStretching<MotorGhostStretchingHarmonic>::ComputeForces(MotorGhost* pm);
template void  MotorGhostStretching<MotorGhostStretchingHarmonic>::ComputeForcesAux(MotorGhost* pm);

