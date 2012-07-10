//
//  Species.cpp
//  CytoSim-Experimenting
//
//  Created by Garegin Papoian on 4/21/12.
//  Copyright (c) 2012 University of Maryland. All rights reserved.
//

#include <iostream>

#include "Species.h"
#include "Reaction.h"
#include "Signaling.h"

namespace chem {

    SpeciesNamesDB* SpeciesNamesDB::_instance = 0;

    SpeciesNamesDB* SpeciesNamesDB::Instance() {
        if(_instance==0)
            _instance = new SpeciesNamesDB;
        return _instance;
    }

    
    
//enum class SType : unsigned char {
//    None = 0, ///< Undefined Species; should only be used privately during construction; 
//    Bulk, ///< Species that have no spatial association (i.e. are "well-mixed") 
//    Diffusing, ///< Species that diffuse between cytosolic compartments 
//    Filament, ///< Species that comprise filaments (such as F-Actin)
//    Walking, ///< Species that can walk ("convectively") on filaments (like Myosin X)
//    Motors, ///< Species that are bound to filaments and generate forces (like Myosin II)
//    Membrane ///< Species that diffuse within a membrane 
//    };
        
//std::vector<std::string> vec_type_name = {"None", "Bulk", "Diffusing", "Filament", "Walking", "Motors", "Membrane"};

//std::string getTypeAsString (SType T) {
//    return vec_type_name[static_cast<int>(T)];
//}
    
} // end of namespace chem

/// Print self into an iostream
std::ostream& operator<<(std::ostream& os, const chem::Species& s){
    os << s.getFullName() << "[" << s.getN() << "]";
    return os;
}
