
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

#include "SysParams.h"
#include "Util/Io/Log.hpp"

bool SysParams::RUNSTATE=true;
bool SysParams::INITIALIZEDSTATUS=false;
bool SysParams::DURINGCHEMISTRY=false;

int SysParams::exvolcounter[3] = {0,0,0};
long long SysParams::exvolcounterz[3] = {0,0,0};
#ifdef NLSTENCILLIST
short SysParams::numcylcylNL = 0;
#endif
vector<float> SysParams::MUBBareRate ={};
vector<float> SysParams::LUBBareRate ={};
vector<float> SysParams::BUBBareRate ={};
bool SysParams::checkChemParameters(ChemistryData& chem) {
    
    if(CParams.numFilaments < 1) {
        cout << "Must specify at least one type of filament. Exiting." << endl;
        exit(EXIT_FAILURE);
    }
    
    for(auto filType = 0; filType < CParams.numFilaments; filType++) {
    
        if(chem.speciesFilament[filType].size() == 0) {
            
            cout << "At least one filament species is required for each filament type. Exiting."
            <<endl;
            
            return false;
        }

        if(chem.speciesPlusEnd[filType].size() == 0) {
            
            cout << "At least one plus end species is required for each filament type. Exiting."
            <<endl;
    
            return false;
        }

        if(chem.speciesMinusEnd[filType].size() == 0) {
            
            cout << "At least one minus end species is required for each filament type. Exiting."
            <<endl;
            
            return false;
        }

        
        if(chem.speciesBound[filType].size() == 0) {
            
            cout << "At least one Bound species is required for each filament type. "
                    "Exiting."
            <<endl;
            
            return false;
        }

        
        //check if binding sites are valid
        if(chem.B_BINDING_INDEX[filType] == "" && chem.speciesBrancher[filType].size() != 0) {
            cout << "A brancher binding site must be set for every filament type. Exiting."
                 << endl;
            return false;
        }
        
        if(chem.L_BINDING_INDEX[filType] == "" && chem.speciesLinker[filType].size() != 0) {
            cout << "A linker binding site must be set for every filament type. Exiting."
            << endl;
            return false;
        }
        
        if(chem.M_BINDING_INDEX[filType] == "" && chem.speciesMotor[filType].size() != 0) {
            cout << "A motor binding site must be set for every filament type. Exiting."
            << endl;
            return false;
        }
    }

    //count all first
    short totalNumMotors = sum(CParams.numMotorSpecies);
    
    //additional motor params
    if(totalNumMotors != CParams.motorNumHeadsMin.size()) {
        
        cout << "Number of minimum motor heads in chemistry input does not "
        << "match the number of motor species. Check these parameters. Exiting."
        <<endl;
        
        return false;
    }
    if(totalNumMotors != CParams.motorNumHeadsMax.size()) {
        
        cout << "Number of maximum motor heads in chemistry input does not "
        << "match the number of motor species. Check these parameters. Exiting."
        <<endl;
        return false;
    }
    if(totalNumMotors != CParams.motorStepSize.size()) {
        
        cout << "Number of motor step sizes in chemistry input does not "
        << "match the number of motor species. Check these parameters. Exiting."
        <<endl;
        return false;
    }

    return true;
}

void SysParams::addChemParameters(ChemistryData& chem){
    for(auto filType = 0; filType < CParams.numFilaments; filType++) {
        CParams.numFilamentSpecies.push_back(chem.speciesFilament[filType].size());
            
        CParams.numPlusEndSpecies.push_back(chem.speciesPlusEnd[filType].size());
            
        CParams.numMinusEndSpecies.push_back(chem.speciesMinusEnd[filType].size());
            
        CParams.numBoundSpecies.push_back(chem.speciesBound[filType].size());
        
        CParams.numLinkerSpecies.push_back(chem.speciesLinker[filType].size());
        
        CParams.numMotorSpecies.push_back(chem.speciesMotor[filType].size());
        
        CParams.numBrancherSpecies.push_back(chem.speciesBrancher[filType].size());
    }
    
}

bool SysParams::checkMechParameters(MechParams::MechanicsFFType& mech) {
    
    //check ff and associated parameters for consistency
    
    //FILAMENT
    if(mech.FStretchingType != "" &&
       MParams.FStretchingK.size() != CParams.numFilaments) {
        cout << "Must set a filament stretching constant for all filaments. Exiting." << endl;
        return false;
    }
    if(mech.FBendingType != "" &&
       MParams.FBendingK.size() != CParams.numFilaments) {
        cout << "Must set a filament bending constant for all filaments. Exiting." << endl;
        return false;
    }
    if(mech.FTwistingType != "" &&
       MParams.FTwistingK.size() != CParams.numFilaments) {
        cout << "Must set a filament twisting constant for all filaments. Exiting." << endl;
        return false;
    }
    
    //LINKER
    short totalNumLinkers = sum(CParams.numLinkerSpecies);
    
    if(mech.LStretchingType != "" &&
       MParams.LStretchingK.size() != totalNumLinkers && totalNumLinkers > 0) {
        cout << "Number of linker stretching constants does not match the number of"
             << " linker species in system. Exiting." << endl;
        return false;
    }
    if(mech.LBendingType != "" &&
       MParams.LBendingK.size() != totalNumLinkers && totalNumLinkers > 0) {
        cout << "Number of linker bending constants does not match the number of"
             << " linker species in system. Exiting." << endl;
        return false;
    }
    if(mech.LBendingType != "" &&
       MParams.LBendingTheta.size() != totalNumLinkers && totalNumLinkers > 0) {
        cout << "Number of linker bending angles does not match the number of"
             << " linker species in system. Exiting." << endl;
        return false;
    }
    if(mech.LTwistingType != "" &&
       MParams.LTwistingK.size() != totalNumLinkers && totalNumLinkers > 0) {
        cout << "Number of linker twisting constants does not match the number of"
        << " linker species in system. Exiting." << endl;
        return false;
    }
    if(mech.LTwistingType != "" &&
       MParams.LTwistingPhi.size() != totalNumLinkers && totalNumLinkers > 0) {
        cout << "Number of linker twisting angles does not match the number of"
             << " linker species in system. Exiting." << endl;
        return false;
    }
    
    //MOTOR
    short totalNumMotors = sum(CParams.numMotorSpecies);
    
    if(mech.MStretchingType != "" &&
       MParams.MStretchingK.size() != totalNumMotors && totalNumMotors > 0) {
        cout << "Number of motor stretching constants does not match the number of"
             << " motor species in system. Exiting." << endl;
        return false;
    }
    if(mech.MBendingType != "" &&
       MParams.MBendingK.size() != totalNumMotors && totalNumMotors > 0) {
        cout << "Number of motor bending constants does not match the number of"
             << " motor species in system. Exiting." << endl;
        return false;
    }
    if(mech.MBendingType != "" &&
       MParams.MBendingTheta.size() != totalNumMotors && totalNumMotors > 0) {
        cout << "Number of motor bending angles does not match the number of"
             << " motor species in system. Exiting." << endl;
        return false;
    }
    if(mech.MTwistingType != "" &&
       MParams.MTwistingK.size() != totalNumMotors && totalNumMotors > 0) {
        cout << "Number of motor twisting constants does not match the number of"
             << " motor species in system. Exiting." << endl;
        return false;
    }
    if(mech.MTwistingType != "" &&
       MParams.MTwistingPhi.size() != totalNumMotors && totalNumMotors > 0) {
        cout << "Number of motor twisting angles does not match the number of"
             << " motor species in system. Exiting." << endl;
        return false;
    }

    
    //BRANCHINGPOINT
    short totalNumBranchers = sum(CParams.numBrancherSpecies);
    

    if(mech.BrStretchingType != "" &&
       MParams.BrStretchingK.size() != totalNumBranchers && totalNumBranchers > 0) {
        cout << "Number of branching point stretching constants does not match the number of"
             << " brancher species in system. Exiting." << endl;
        return false;
    }
    if(mech.BrStretchingType != "" &&
       MParams.BrStretchingL.size() != totalNumBranchers && totalNumBranchers > 0) {
        cout << "Number of branching point stretching length does not match the number of"
             << " brancher species in system. Exiting." << endl;
        return false;
    }
    if(mech.BrBendingType != "" &&
       MParams.BrBendingK.size() != totalNumBranchers && totalNumBranchers > 0) {
        cout << "Number of branching point bending constants does not match the number of"
             << " brancher species in system. Exiting." << endl;
        return false;
    }
    if(mech.BrBendingType != "" &&
       MParams.BrBendingTheta.size() != totalNumBranchers && totalNumBranchers > 0) {
        cout << "Number of branching point bending angles does not match the number of"
             << " brancher species in system. Exiting." << endl;
        return false;
    }
    if(mech.BrDihedralType != "" &&
       MParams.BrDihedralK.size() != totalNumBranchers && totalNumBranchers > 0) {
        cout << "Number of branching point dihedral constants does not match the number of"
             << " brancher species in system. Exiting." << endl;
        return false;
    }
    if(mech.BrPositionType != "" &&
       MParams.BrPositionK.size() != totalNumBranchers && totalNumBranchers > 0) {
        cout << "Number of branching point position constants does not match the number of"
             << " brancher species in system. Exiting." << endl;
        return false;
    }
    
    //VOLUME
    if(mech.VolumeFFType != "" &&
       MParams.VolumeK.size() != CParams.numFilaments) {
        cout << "Must set a cylinder volume force constant for every filament type. Exiting." << endl;
        return false;
    }
    if(mech.VolumeFFType != "" && areEqual(MParams.VolumeCutoff, 0.0)) {
        cout << "Must set a cylinder volume cutoff for mechanical equilibration. Exiting." << endl;
        return false;
    }
    if(mech.triangleBeadVolumeFFType != "" && MParams.triangleBeadVolume.cutoff == 0.0) {
        LOG(ERROR) << "The membrane-bead volume cutoff for load force is not set.";
        return false;
    }
    if(mech.triangleBeadVolumeFFType != "" && MParams.triangleBeadVolume.cutoffMech == 0.0) {
        LOG(ERROR) << "The membrane-bead volume cutoff for mechanical equilibration is not set.";
        return false;
    }
    
    //Boundary
    if(mech.BoundaryFFType != "" && areEqual(BParams.BoundaryK, 0.0)) {
        cout << "Must set a boundary force constant. Exiting." << endl;
        return false;
    }
    if(mech.BoundaryFFType != "" && areEqual(BParams.BScreenLength, 0.0)) {
        cout << "Must set a boundary screen length. Exiting." << endl;
        return false;
    }
    if(mech.BoundaryFFType != "" && areEqual(BParams.BoundaryCutoff, 0.0)) {
        cout << "Must set a boundary cutoff for mechanical equilibration. Exiting." << endl;
        return false;
    }
    
    //Bubbles
    if(mech.BubbleFFType != "" &&
      (MParams.BubbleK.size() != MParams.numBubbleTypes ||
       MParams.BubbleRadius.size() != MParams.numBubbleTypes ||
       MParams.BubbleScreenLength.size() != MParams.numBubbleTypes)) {
        cout << "Must set all bubble mechanical constants for every bubble type. Exiting." << endl;
        return false;
    }
    if(mech.BubbleFFType != "" && areEqual(MParams.BubbleCutoff, 0.0)) {
        cout << "Must set a bubble cutoff for mechanical equilibration. Exiting." << endl;
        return false;
    }
    
    ///Cylinder and monomer lengths specified
    if(GParams.cylinderSize.size() != CParams.numFilaments) {
        
        cout << "Must specify a cylinder size for every type of filament. Exiting." << endl;
        return false;
    }
    if(GParams.monomerSize.size() != CParams.numFilaments) {
        
        cout << "Must specify a monomer size for every type of filament. Exiting." << endl;
        return false;
    }
    
    return true;
}

bool SysParams::checkGeoParameters() {
    
    //Check that grid and compartmentSize match nDim
    if((GParams.nDim == 3 &&
        GParams.NX != 0 && GParams.NY != 0 && GParams.NZ !=0 &&
        GParams.compartmentSizeX != 0 &&
        GParams.compartmentSizeY != 0 &&
        GParams.compartmentSizeZ != 0)){
    }
    else {
        cout << "Grid parameters are invalid. Exiting." << endl;
        return false;
    }
    return true;
}

bool SysParams::checkDyRateParameters(DyRateParams::DynamicRateType& dy) {
    
    //check types match number of species
    if(dy.dFPolymerizationType.size() != CParams.numFilaments &&
       !dy.dFPolymerizationType.empty()) {
        cout << "Number of filament dynamic rate polymerization forms must" <<
                " match the number of filaments. Exiting." << endl;
        return false;
    }
    
    if(dy.dLUnbindingType.size() != sum(CParams.numLinkerSpecies) &&
       !dy.dLUnbindingType.empty() && sum(CParams.numLinkerSpecies) > 0){
        cout << "Number of linker dynamic rate unbinding forms must" <<
                " match the number of species. Exiting." << endl;
        return false;
    }
    
    if(dy.dMUnbindingType.size() != sum(CParams.numMotorSpecies) &&
       !dy.dMUnbindingType.empty() && sum(CParams.numMotorSpecies) > 0) {
        cout << "Number of motor dynamic rate unbinding forms must" <<
                " match the number of species. Exiting." << endl;
        return false;
    }
    if(dy.dMWalkingType.size() != sum(CParams.numMotorSpecies) &&
       !dy.dMWalkingType.empty() && sum(CParams.numMotorSpecies) > 0) {
        cout << "Number of motor dynamic rate walking forms must" <<
                " match the number of species. Exiting." << endl;
        return false;
    }
    
    //now check parameters
    if(dy.dFPolymerizationType.size() != SysParams::DynamicRates().
                                         dFilPolymerizationCharLength.size()) {
        cout << "Must set a dynamic rate polymerization length for all filaments. Exiting." << endl;
        return false;
    }
    
    auto numCharLengths = 0;
    auto numAmps = 0;
    
    for(auto &changer : dy.dLUnbindingType) {
        
        if(changer == "CATCHSLIP") {
            numCharLengths += 2;
            numAmps += 2;
        }
        else if(changer == "SLIP") {
            numCharLengths += 1;
        }
        
    }
    if(numCharLengths != SysParams::DynamicRates().
                         dLinkerUnbindingCharLength.size()) {
        cout << "Number of characteristic lengths specified for chosen "
             << "linker unbinding dynamic rate forms is not accurate. Exiting."
        << endl;
        return false;
    }
    
    if(numAmps != SysParams::DynamicRates().
                  dLinkerUnbindingAmplitude.size()) {
        
        
        cout << "Number of amplitudes specified for chosen "
             << "linker unbinding dynamic rate forms is not accurate. Exiting."
        << endl;
        return false;
    }
    
    auto numCharForces = 0;
    
    for(auto &changer : dy.dMUnbindingType) {
        
        if(changer == "LOWDUTYCATCHSLIP") {
            numCharForces += 2;
        }
        else if(changer == "LOWDUTYCATCH") {
            numCharForces += 1;
        }
        else if(changer == "HIGHDUTYCATCH") {
            numCharForces += 1;
        }
        
    }
    if(numCharForces != SysParams::DynamicRates().
                        dMotorUnbindingCharForce.size()) {
        cout << "Number of characteristic forces specified for chosen "
             << "motor unbinding dynamic rate forms is not accurate. Exiting."
        << endl;
        return false;
    }
    
    if(dy.dMWalkingType.size() != SysParams::DynamicRates().
                                  dMotorWalkingCharForce.size()) {
        cout << "Number of characteristic forces specified for chosen "
             << "motor walking dynamic rate forms is not accurate. Exiting."
        << endl;
        return false;
    }
    
    return true;
}

MechParams   SysParams::MParams;
ChemParams   SysParams::CParams;
GeoParams    SysParams::GParams;
BoundParams  SysParams::BParams;
DyRateParams SysParams::DRParams;
#ifdef TRACKDIDNOTMINIMIZE
MinimizationParams SysParams::MinParams;
#endif
SpecialParams SysParams::SParams;
SysParams::SimulParams  SysParams::simulParams_;

