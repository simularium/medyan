
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

#include "TriangleCylinderVolumeFF.h"

#include "TriangleCylinderExclVolume.h"
#include "TriangleCylinderBeadExclVolRepulsion.h"

#include "Triangle.h"
#include "Cylinder.h"

TriangleCylinderVolumeFF::TriangleCylinderVolumeFF (string& type) {
    if (type == "REPULSION")
        _triangleCylinderVolInteractionVector.emplace_back(
        new TriangleCylinderExclVolume <TriangleCylinderBeadExclVolRepulsion>());
    else if(type == "") {}
    else {
        cout << "Volume FF not recognized. Exiting." << endl;
        exit(EXIT_FAILURE);
    }
}

void TriangleCylinderVolumeFF::whoIsCulprit() {
    
    cout << endl;
    
    cout << "Culprit interaction = " << _culpritInteraction->getName() << endl;
    
    cout << "Printing the culprit triangle and cylinder..." << endl;
    _culpritInteraction->_triangleCulprit->printSelf();
    _culpritInteraction->_cylinderCulprit->printSelf();
    
    cout << endl;
}

double TriangleCylinderVolumeFF::computeEnergy(double d) {
    
    double U= 0;
    double U_i;
    
    for (auto &interaction : _triangleCylinderVolInteractionVector) {
        
        U_i = interaction->computeEnergy(d);
                
        if(U_i <= -1) {
            //set culprit and return
            _culpritInteraction = interaction.get();
            return -1;
        }
        else U += U_i;
        
    }
    return U;
}

void TriangleCylinderVolumeFF::computeForces() {
    
    for (auto &interaction : _triangleCylinderVolInteractionVector)
        interaction->computeForces();
}

void TriangleCylinderVolumeFF::computeForcesAux() {
    
    for (auto &interaction : _triangleCylinderVolInteractionVector)
        interaction->computeForcesAux();
}

void TriangleCylinderVolumeFF::computeLoadForces() {
    for(auto& interaction: _triangleCylinderVolInteractionVector)
        interaction->computeLoadForces();
}

vector<NeighborList*> TriangleCylinderVolumeFF::getNeighborLists() {
    
    vector<NeighborList*> neighborLists;
    
    for(auto &interaction : _triangleCylinderVolInteractionVector)
        neighborLists.push_back(interaction->getNeighborList());
    
    return neighborLists;
}

