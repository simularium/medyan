
//------------------------------------------------------------------
//  **M3SYM** - Simulation Package for the Mechanochemical
//              Dynamics of Active Networks, 3rd Generation
//
//  Copyright (2014) Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the Papoian lab page for installation and documentation:
//  http://papoian.chem.umd.edu/
//------------------------------------------------------------------

#include "MotorGhostDB.h"

int MotorGhostDB::_currentMotorID = 0;
MotorGhostDB* MotorGhostDB::_instance = 0;

MotorGhostDB* MotorGhostDB::instance() {
    if(_instance==0)
        _instance = new MotorGhostDB;
    return _instance;
}