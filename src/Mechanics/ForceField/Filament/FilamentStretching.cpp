
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

#include "FilamentStretching.h"

#include "FilamentStretchingHarmonic.h"
#include "Filament.h"
#include "Cylinder.h"

#include "Bead.h"

template <class FStretchingInteractionType>
double FilamentStretching<FStretchingInteractionType>::computeEnergy(double d) {
    
    double U = 0.0;
    double U_i=0.0;
    
    for (auto f: Filament::getFilaments()) {
        
        U_i = 0;
        if (d == 0.0){
            for(auto it : f->getCylinderVector()){
                
                Bead* b1 = it->getFirstBead();
                Bead* b2 = it->getSecondBead();
                double kStretch = it->getMCylinder()->getStretchingConst();
                double eqLength = it->getMCylinder()->getEqLength();
                U_i += _FFType.energy(b1, b2, kStretch, eqLength);
            }
        }
        else {
            for(auto it : f->getCylinderVector()){
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
            _filamentCulprit = f;
            
            return -1;
        }
        else
            U += U_i;
    }
    
    return U;
}

template <class FStretchingInteractionType>
void FilamentStretching<FStretchingInteractionType>::computeForces() {
    int nint = 0;
    
    for (auto f: Filament::getFilaments()) {
    
        for(auto it : f->getCylinderVector()){
            
            Bead* b1 = it->getFirstBead();
            Bead* b2 = it->getSecondBead();
            double kStretch =it->getMCylinder()->getStretchingConst();
            double eqLength = it->getMCylinder()->getEqLength();
           
            _FFType.forces(b1, b2, kStretch, eqLength);
            nint++;
        }
    }
    double maxF = 0;
    
    //calc max force
    for(auto b: Bead::getBeads())
        maxF = max(maxF, sqrt(b->FDotF()));
    std::cout<<"maxF "<<getName()<<" "<<maxF<<" nint "<<nint<<endl;
}


template <class FStretchingInteractionType>
void FilamentStretching<FStretchingInteractionType>::computeForcesAux() {
    
    for (auto f: Filament::getFilaments()) {
        
        for(auto it : f->getCylinderVector()){
            
            Bead* b1 = it->getFirstBead();
            Bead* b2 = it->getSecondBead();
            double kStretch =it->getMCylinder()->getStretchingConst();
            double eqLength = it->getMCylinder()->getEqLength();
            
            _FFType.forcesAux(b1, b2, kStretch, eqLength);
        }
    }
    double maxF = 0;
    
    //calc max force
    for(auto b: Bead::getBeads())
        maxF = max(maxF, sqrt(b->FADotFA()));
    std::cout<<"maxF "<<getName()<<" "<<maxF<<endl;
}

///Template specializations
template double FilamentStretching<FilamentStretchingHarmonic>::computeEnergy(double d);
template void FilamentStretching<FilamentStretchingHarmonic>::computeForces();
template void FilamentStretching<FilamentStretchingHarmonic>::computeForcesAux();
