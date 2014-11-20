//
//  LinkerDB.cpp
//  Cyto
//
//  Created by James Komianos on 7/30/14.
//  Copyright (c) 2014 University of Maryland. All rights reserved.
//

#include "LinkerDB.h"

int LinkerDB::_currentLinkerID = 0;
LinkerDB* LinkerDB::_instance = 0;

LinkerDB* LinkerDB::instance() {
    if(_instance==0)
        _instance = new LinkerDB;
    return _instance;
}