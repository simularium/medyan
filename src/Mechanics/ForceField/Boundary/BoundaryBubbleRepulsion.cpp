
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

#include "BoundaryBubbleRepulsion.h"

#include "BoundaryBubbleRepulsionExp.h"
#include "BoundaryElement.h"

#include "Bubble.h"
#include "Bead.h"

template <class BRepulsionInteractionType>
double BoundaryBubbleRepulsion<BRepulsionInteractionType>::computeEnergy(bool stretched) {
    
    double U = 0.0;
    double U_i=0.0;
    
    for (auto be: BoundaryElement::getBoundaryElements()) {
        
        for(auto &bb : _neighborList->getNeighbors(be)) {
            
            double kRep = be->getRepulsionConst();
            double screenLength = be->getScreeningLength();
            double radius = bb->getRadius();
            
            Bead* bd = bb->getBead();
            
            U_i =  _FFType.energy(
                bd, be->distance(stretched ? bd->getCoordinate<true>() : bd->getCoordinate<false>()), radius, kRep, screenLength);
            
            if(fabs(U_i) == numeric_limits<double>::infinity()
               || U_i != U_i || U_i < -1.0) {
                
                //set culprits and return
                _otherCulprit = bb;
                _boundaryElementCulprit = be;
                
                return -1;
            }
            else
                U += U_i;
        }
    }
    
    return U;
}

template <class BRepulsionInteractionType>
void BoundaryBubbleRepulsion<BRepulsionInteractionType>::computeForces() {
    
    for (auto be: BoundaryElement::getBoundaryElements()) {
        
        for(auto &bb : _neighborList->getNeighbors(be)) {
            
            double kRep = be->getRepulsionConst();
            double screenLength = be->getScreeningLength();
            double radius = bb->getRadius();
            
            Bead* bd = bb->getBead();
            
            auto normal = be->normal(bd->coordinate);
            _FFType.forces(bd, be->distance(bd->coordinate),
                           radius, normal, kRep, screenLength);
            
        }
    }
}


template <class BRepulsionInteractionType>
void BoundaryBubbleRepulsion<BRepulsionInteractionType>::computeForcesAux() {
    
    for (auto be: BoundaryElement::getBoundaryElements()) {
        
        for(auto &bb : _neighborList->getNeighbors(be)) {
            
            double kRep = be->getRepulsionConst();
            double screenLength = be->getScreeningLength();
            double radius = bb->getRadius();
            
            Bead* bd = bb->getBead();
            
            auto normal = be->normal(bd->coordinate);
            _FFType.forcesAux(bd, be->distance(bd->coordinate),
                              radius, normal, kRep, screenLength);
            
        }
    }
}

///Template specializations
template double BoundaryBubbleRepulsion<BoundaryBubbleRepulsionExp>::computeEnergy(double d);
template void BoundaryBubbleRepulsion<BoundaryBubbleRepulsionExp>::computeForces();
template void BoundaryBubbleRepulsion<BoundaryBubbleRepulsionExp>::computeForcesAux();
