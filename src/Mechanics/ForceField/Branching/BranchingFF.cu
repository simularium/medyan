
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

#include "BranchingFF.h"

#include "BranchingStretching.h"
#include "BranchingStretchingHarmonic.h"

#include "BranchingBending.h"
#include "BranchingBendingCosine.h"

#include "BranchingDihedral.h"
#include "BranchingDihedralCosine.h"

#include "BranchingPosition.h"
#include "BranchingPositionCosine.h"

#include "BranchingPoint.h"
#include "Bead.h"
BranchingFF::BranchingFF(string& stretching, string& bending,
                         string& dihedral, string& position)
{
    if(stretching == "HARMONIC")
        _branchingInteractionVector.emplace_back(
                new BranchingStretching<BranchingStretchingHarmonic>());
    else if(stretching == "") {}
    else {
        cout << "Branching stretching FF not recognized. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    if(bending == "COSINE")
        _branchingInteractionVector.emplace_back(
                new BranchingBending<BranchingBendingCosine>());
    else if(bending == "") {}
    else {
        cout << "Branching bending FF not recognized. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    if(dihedral == "COSINE")
        _branchingInteractionVector.emplace_back(
                new BranchingDihedral<BranchingDihedralCosine>());
    else if(dihedral == "") {}
    else {
        cout << "Branching dihedral FF not recognized. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

    if(position == "COSINE")
        _branchingInteractionVector.emplace_back(new
                                                         BranchingPosition<BranchingPositionCosine>());
    else if(position == "") {}
    else {
        cout << "Branching position FF not recognized. Exiting." << endl;
        exit(EXIT_FAILURE);
    }

}

void BranchingFF::vectorize() {
    //Reset stretching forces to 0.
    for(auto b:BranchingPoint::getBranchingPoints()){
        //Using += to ensure that the stretching forces are additive.
        b->getMBranchingPoint()->stretchForce = 0.0;
    }


    for (auto &interaction : _branchingInteractionVector)
        interaction->vectorize();
}

void BranchingFF::cleanup() {

    for (auto &interaction : _branchingInteractionVector)
        interaction->deallocate();
}


void BranchingFF::whoIsCulprit() {

    cout << endl;

    cout << "Culprit interaction = " << _culpritInteraction->getName() << endl;

    cout << "Printing the culprit branching point..." << endl;
    _culpritInteraction->_branchingCulprit->printSelf();

    cout << endl;
}


totalenergyfloatingpoint BranchingFF::computeEnergy(floatingpoint *coord, totalforcefloatingpoint *f, floatingpoint d) {

    totalenergyfloatingpoint U = 0.0;
    totalenergyfloatingpoint U_i=0.0;
    for (auto &interaction : _branchingInteractionVector) {
#ifdef SERIAL_CUDACROSSCHECK
        CUDAcommon::handleerror(cudaDeviceSynchronize(),"ForceField", "ForceField");
//        std::cout<<interaction->getName()<<endl;
#endif
        U_i = interaction->computeEnergy(coord, f, d);
//        CUDAcommon::handleerror(cudaDeviceSynchronize(),"BranchingFF","BranchingFF");
//        std::cout<<interaction->getName()<<" "<<U_i<<endl;

        if(U_i <= -1) {
            //set culprit and return
            _culpritInteraction = interaction.get();
            return -1;
        }
        else U += U_i;

    }
    return U;
}

void BranchingFF::computeForces(floatingpoint *coord, totalforcefloatingpoint *f) {

    for (auto &interaction : _branchingInteractionVector)
        interaction->computeForces(coord, f);
}
