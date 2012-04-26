//
//  Species.cpp
//  CytoSim-Experimenting
//
//  Created by Garegin Papoian on 4/21/12.
//  Copyright (c) 2012 University of Maryland. All rights reserved.
//

#include <iostream>

#include "Species.h"

std::vector<std::string> SpeciesType::_vec_type_names = {"Unknown", "Bulk", "Diffusing", "Poly", "PolyA", "PolyM", "ProxyA"};

void Species::printSelf() const {
    std::cout << "Species: " << _type.get().getName() + "[" + _type.get().getTypeAsString() + "]" << ", copy_num=" << (int)this->_n << ", ptr=" << (void*)this << std::endl;
    std::cout << "This species is involved in the following Reactions: \nAs a reactant:\n";
    for(auto &r : _reactions[0])
        std::cout << r << ",";
    std::cout << "\nAs a product:\n";
    for(auto &r : _reactions[1])
        std::cout << r << ",";
    std::cout << "\n";
}

