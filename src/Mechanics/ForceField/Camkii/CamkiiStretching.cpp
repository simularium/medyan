#ifdef CAMKII
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

#include "CamkiiStretching.h"

#include "CamkiiStretchingHarmonic.h"
#include "Filament.h"
#include "Cylinder.h"
#include "Camkii.h"

template <class FStretchingInteractionType>
double CamkiiStretching<FStretchingInteractionType>::computeEnergy(double d) {
    
    double U = 0;
    double U_i;
    
    for (auto f: Camkii::getCamkiis()) {
        
        U_i = 0;
        
        if (d == 0.0){
            for(auto it : f->getCylinders()){
                
                Bead* b1 = it->getFirstBead();
                Bead* b2 = it->getSecondBead();
                double kStretch = it->getMCylinder()->getStretchingConst();
                double eqLength = it->getMCylinder()->getEqLength();
                
                U_i += _FFType.energy(b1, b2, kStretch, eqLength);
            }
        }
        else {
            for(auto it : f->getCylinders()){
                Bead* b1 = it->getFirstBead();
                Bead* b2 = it->getSecondBead();
                double kStretch =it->getMCylinder()->getStretchingConst();
                double eqLength = it->getMCylinder()->getEqLength();
                
                U_i += _FFType.energy(b1, b2, kStretch, eqLength, d);
            }
        }
        
        if(fabs(U_i) == numeric_limits<double>::infinity()
           || U_i != U_i || U_i < -1.0) {
            
            //set culprit and return
            _camkiiCulprit = f;
            
            return -1;
        }
        else
            U += U_i;
    }
    
    return U;
}

template <class FStretchingInteractionType>
void CamkiiStretching<FStretchingInteractionType>::computeForces() {
    
    for (auto f: Camkii::getCamkiis()) {
    
        for(auto it : f->getCylinders()){
            
            Bead* b1 = it->getFirstBead();
            Bead* b2 = it->getSecondBead();
            double kStretch =it->getMCylinder()->getStretchingConst();
            double eqLength = it->getMCylinder()->getEqLength();
           
            _FFType.forces(b1, b2, kStretch, eqLength);
        }
    }
}


template <class FStretchingInteractionType>
void CamkiiStretching<FStretchingInteractionType>::computeForcesAux() {
    
    for (auto f: Camkii::getCamkiis()) {
        
        for(auto it : f->getCylinders()){
            
            Bead* b1 = it->getFirstBead();
            Bead* b2 = it->getSecondBead();
            double kStretch =it->getMCylinder()->getStretchingConst();
            double eqLength = it->getMCylinder()->getEqLength();
            
            _FFType.forcesAux(b1, b2, kStretch, eqLength);
        }
    }
}

///Template specializations
template double CamkiiStretching<CamkiiStretchingHarmonic>::computeEnergy(double d);
template void CamkiiStretching<CamkiiStretchingHarmonic>::computeForces();
template void CamkiiStretching<CamkiiStretchingHarmonic>::computeForcesAux();
#endif