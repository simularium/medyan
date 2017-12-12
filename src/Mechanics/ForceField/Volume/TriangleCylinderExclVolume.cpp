
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v3.1
//
//  Copyright (2017-2018)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#include "TriangleCylinderExclVolume.h"

#include "TriangleCylinderBeadExclVolRepulsion.h"

#include "Triangle.h"
#include "Vertex.h"
#include "Cylinder.h"
#include "Bead.h"

template <class TriangleCylinderExclVolumeInteractionType>
double TriangleCylinderExclVolume<TriangleCylinderExclVolumeInteractionType>::computeEnergy(double d) {
    
    double U = 0;
    double U_i;

    for(auto t: Triangle::getTriangles()) {
        
        for(auto &c : _neighborList->getNeighbors(t)) {
                        
            Bead* b = c->getFirstBead();
            double kExVol = t->getMTriangle()->getExVolConst();
            
            if (d == 0.0)
                U_i = _FFType.energy(t, b, kExVol);
            else
                U_i = _FFType.energy(t, b, kExVol, d);
            
            if(fabs(U_i) == numeric_limits<double>::infinity()
               || U_i != U_i || U_i < -1.0) {
                
                //set culprits and exit
                _triangleCulprit = t;
                _cylinderCulprit = c;
                
                return -1;
            }
            else
                U += U_i;
        }
    }
    
    return U;
}

template <class TriangleCylinderExclVolumeInteractionType>
void TriangleCylinderExclVolume<TriangleCylinderExclVolumeInteractionType>::computeForces() {

    for(auto t: Triangle::getTriangles()) {
        
        for(auto &c: _neighborList->getNeighbors(t)) {
            
            Bead* b = c->getFirstBead();
            double kExVol = t->getMTriangle()->getExVolConst();
            
            _FFType.forces(t, b, kExVol);
        }
    }
}


template <class TriangleCylinderExclVolumeInteractionType>
void TriangleCylinderExclVolume<TriangleCylinderExclVolumeInteractionType>::computeForcesAux() {

    for(auto t: Triangle::getTriangles()) {
        
        for(auto &c: _neighborList->getNeighbors(t)) {
            
            Bead* b = c->getFirstBead();
            double kExVol = t->getMTriangle()->getExVolConst();
            
            _FFType.forcesAux(t, b, kExVol);
        }
    }
}

///Template specializations
template double TriangleCylinderExclVolume<TriangleCylinderBeadExclVolRepulsion>::computeEnergy(double d);
template void TriangleCylinderExclVolume<TriangleCylinderBeadExclVolRepulsion>::computeForces();
template void TriangleCylinderExclVolume<TriangleCylinderBeadExclVolRepulsion>::computeForcesAux();

