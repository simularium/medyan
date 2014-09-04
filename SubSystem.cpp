//
//  MSubSystem.cpp
//  CytoMech
//
//  Created by Konstantin Popov on 4/15/14.
//  Copyright (c) 2014 Konstantin Popov. All rights reserved.
//

#include "SubSystem.h"

using namespace std;


//void SubSystem::AddNewFilament(vector<double> v) { _pfilamentVector.push_back( _pfdb->CreateFilament(this, v));}


// Interfaces for creation of a Filament (vector with coordinates of beginig and end for the filament, and in what SubSystem (i) to create ):


void SubSystem::AddNewFilaments(vector<vector<vector<double> > > v)
{
    
    for (auto it: v){
        
        _pfdb->CreateFilament(_pS, this, it);  //Call Filament constructor
    }
}
/// Add many linkers:
void SubSystem::AddNewLinkers(std::vector<std::vector<Cylinder* > > v, double stretchConst){
    for (auto it: v){
        _pldb->CreateLinker(this, it[0], it[1], stretchConst); //Call Linkers constructor
    }
}
/// Add a linker
void SubSystem::AddNewLinker(Cylinder* pc1, Cylinder* pc2, double stretchConst){
    
    _pldb->CreateLinker(this, pc1, pc2, stretchConst);  //Call Linker constructor
}
/// Add many linkers:
void SubSystem::AddNewMotorGhosts(std::vector<std::vector<Cylinder* > > v, double k, double position1, double position2){
    for (auto it: v){
        _pmgdb->CreateMotorGhost(this, it[0], it[1], k, position1, position2);
    }
}
/// Add a gost motor:
void SubSystem::AddNewMotorGhost(Cylinder* pc1, Cylinder* pc2, double k, double position1, double position2){
    
    _pmgdb->CreateMotorGhost(this, pc1, pc2, k, position1, position2);
    
}

// compute energy: iterate over all filaments, motors, lincers etc and call compute energy. add it up ant return.
double SubSystem::CopmuteEnergy(double d)
{
    double totEnergy = 0;
    
    if (d == 0.0){
        
        double filamentEnergy = 0;
        double motorEnergy = 0;
        double linkerEnergy = 0;
        
        for (auto it: (*_pfdb) ) {filamentEnergy += it->CopmuteEnergy();}
        
        for (auto it: (*_pmgdb) ) {motorEnergy += it->CopmuteEnergy();}
        
        for (auto it: (*_pldb) ) {linkerEnergy += it->CopmuteEnergy();}
        
        return totEnergy = filamentEnergy + linkerEnergy + motorEnergy;
    }
    
    
    else {
        
        double filamentEnergy = 0;
        double motorEnergy = 0;
        double linkerEnergy = 0;
        
        for (auto it: (*_pfdb) ) {filamentEnergy += it->CopmuteEnergy(d);}
        
        for (auto it: (*_pmgdb) ) {motorEnergy += it->CopmuteEnergy(d);}
        
        for (auto it: (*_pldb) ) {linkerEnergy += it->CopmuteEnergy(d);}
        
        return totEnergy = filamentEnergy;
    }
}

// End Energy calculation

// Force Calculation:


void SubSystem::ResetForces(BeadDB&  list){
    for(auto it: list) {
        it->force.assign (3, 0); //Set force to zero;
    }
    
}

void SubSystem::ResetForcesAux(BeadDB&  list){
    for(auto it: list) {
        it->forceAux.assign (3, 0); //Set forceAux to zero;
        
    }
    
}

double SubSystem::getSubSystemEnergy() {return _energy;}