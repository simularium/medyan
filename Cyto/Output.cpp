//
//  Output.cpp
//  Cyto
//
//  Created by James Komianos on 9/16/14.
//  Copyright (c) 2014 University of Maryland. All rights reserved.
//

#include "Output.h"
#include "MathFunctions.h"

using namespace mathfunc;

///Print basic information about filaments
void Output::printBasicSnapshot(int step) {
    
    _outputFile.precision(10);
    
    //print first line (step number, time, number of filaments, linkers, motors)
    _outputFile << step << " " << tau() << " " << FilamentDB::instance(FilamentDBKey())->size() <<
    " " << LinkerDB::instance(LinkerDBKey())->size() << " " << MotorGhostDB::instance(MotorGhostDBKey())->size() << endl;
    
    for(auto &filament : *FilamentDB::instance(FilamentDBKey())) {

        ///print first line(Filament ID, length, left_delta, right_delta
        _outputFile << "F " << filament->getID() << " " << filament->getCylinderVector().size() + 1
            << " " << filament->getDeltaMinusEnd() << " " << filament->getDeltaPlusEnd() << endl;

        ///print coordinates
        for (auto cylinder : filament->getCylinderVector()){
            
            auto x = cylinder->getFirstBead()->coordinate;
            _outputFile<<x[0]<<" "<<x[1]<<" "<<x[2]<<" ";
            
        }
        ///print last bead coord
        auto x = filament->getCylinderVector().back()->getSecondBead()->coordinate;
        _outputFile<<x[0]<<" "<<x[1]<<" "<<x[2];
        
        _outputFile << endl;
        
        ///Reset deltas for this filament
        filament->resetDeltaPlusEnd();
        filament->resetDeltaMinusEnd();
    }
    
    
    for(auto &linker : *LinkerDB::instance(LinkerDBKey())) {
        
        ///print first line(Filament ID, length, left_delta, right_delta
        _outputFile << "L " << linker->getLinkerID()<< " " << linker->getLinkerType() << endl;
        
        ///print coordinates
        auto x = MidPointCoordinate(linker->getFirstCylinder()->getFirstBead()->coordinate,
                                    linker->getFirstCylinder()->getSecondBead()->coordinate,
                                    linker->getFirstPosition());
        _outputFile<<x[0]<<" "<<x[1]<<" "<<x[2] << " ";
        
        x = MidPointCoordinate(linker->getSecondCylinder()->getFirstBead()->coordinate,
                                    linker->getSecondCylinder()->getSecondBead()->coordinate,
                                    linker->getSecondPosition());
        _outputFile<<x[0]<<" "<<x[1]<<" "<<x[2];
        
        _outputFile << endl;
    }

    for(auto &motor : *MotorGhostDB::instance(MotorGhostDBKey())) {
        
        ///print first line(Filament ID, length, left_delta, right_delta
        _outputFile << "M " << motor->getMotorID() << " " << motor->getMotorType() << endl;
        
        ///print coordinates
        auto x = MidPointCoordinate(motor->getFirstCylinder()->getFirstBead()->coordinate,
                                    motor->getFirstCylinder()->getSecondBead()->coordinate,
                                    motor->getFirstPosition());
        _outputFile<<x[0]<<" "<<x[1]<<" "<<x[2] << " ";
        
        x = MidPointCoordinate(motor->getSecondCylinder()->getFirstBead()->coordinate,
                               motor->getSecondCylinder()->getSecondBead()->coordinate,
                               motor->getSecondPosition());
        _outputFile<<x[0]<<" "<<x[1]<<" "<<x[2];
        
        _outputFile << endl;
    }

    
    _outputFile <<endl;
}


void Output::printSnapshot(int step) {
    
    _outputFile.precision(10);
    
    //print first line (step number, time, number of filaments
    _outputFile << step << " " << tau() << " " << FilamentDB::instance(FilamentDBKey())->size() << endl;
    
    for(auto &filament : *FilamentDB::instance(FilamentDBKey())) {
        
        ///print first line(Filament ID, length, index of first bead, index of last bead
        _outputFile << filament->getID() << " " << filament->getCylinderVector().size() + 1
        << " " << filament->getCylinderVector().front()->getFirstBead()->getPositionFilament()
        << " " << filament->getCylinderVector().back()->getSecondBead()->getPositionFilament() << endl;
        
        ///print coordinates
        for (auto cylinder : filament->getCylinderVector()){
            
            auto x = cylinder->getFirstBead()->coordinate;
            _outputFile<<x[0]<<" "<<x[1]<<" "<<x[2]<<" ";
            
        }
        auto x = filament->getCylinderVector().back()->getSecondBead()->coordinate;
        _outputFile<<x[0]<<" "<<x[1]<<" "<<x[2];
        
        _outputFile << endl;
    }
    _outputFile <<endl;
    
}


void Output::printBirthTimes(int step) {
    
    _outputFile.precision(10);
    
    //print first line (step number, time, number of filaments
    _outputFile << step << " " << tau() << " " << FilamentDB::instance(FilamentDBKey())->size() << endl;
    
    for(auto &filament : *FilamentDB::instance(FilamentDBKey())) {
        
        ///print first line(Filament ID, length, index of first bead, index of last bead
        _outputFile << filament->getID() << " " << filament->getCylinderVector().size() + 1
        << " " << filament->getCylinderVector().front()->getFirstBead()->getPositionFilament()
        << " " << filament->getCylinderVector().back()->getSecondBead()->getPositionFilament() << endl;
        
        ///print coordinates
        for (auto cylinder : filament->getCylinderVector()){
            
            auto b = cylinder->getFirstBead();
            _outputFile<< b->getBirthTime() << " ";
            
        }
        ///last bead
        _outputFile<< filament->getCylinderVector().back()->getSecondBead()->getBirthTime();
        _outputFile << endl;
    }
    _outputFile <<endl;
}

void Output::printForces(int step) {
    
    _outputFile.precision(10);
    
    //print first line (step number, time, number of filaments
    _outputFile << step << " " << tau() << " " << FilamentDB::instance(FilamentDBKey())->size() << endl;
    
    for(auto &filament : *FilamentDB::instance(FilamentDBKey())) {
        
        ///print first line(Filament ID, length, index of first bead, index of last bead
        _outputFile << filament->getID() << " " << filament->getCylinderVector().size() + 1
        << " " << filament->getCylinderVector().front()->getFirstBead()->getPositionFilament()
        << " " << filament->getCylinderVector().back()->getSecondBead()->getPositionFilament() << endl;
        
        ///print coordinates
        for (auto cylinder : filament->getCylinderVector()){
            
            auto x = cylinder->getFirstBead()->force;
            _outputFile<<x[0]<<" "<<x[1]<<" "<<x[2]<<" ";
            
        }
        auto x = filament->getCylinderVector().back()->getSecondBead()->force;
        _outputFile<<x[0]<<" "<<x[1]<<" "<<x[2];
        _outputFile << endl;
    }
    _outputFile <<endl;
}

void Output::printStresses(int step) {}

