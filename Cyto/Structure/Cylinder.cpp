//
//  Cylinder.cpp
//  Cyto
//
//  Created by James Komianos on 7/31/14.
//  Copyright (c) 2014 University of Maryland. All rights reserved.
//

#include "Cylinder.h"

Cylinder::Cylinder(Filament* pf, Bead* firstBead, Compartment* c, bool extensionFront, bool extensionBack) {
    
    _mCylinder = std::unique_ptr<MCylinder>(new MCylinder(pf, firstBead));
    _cCylinder = std::unique_ptr<CCylinder>(
        ChemInitializer::createCCylinder(ChemInitializerCylinderKey() , pf, c, extensionFront, extensionBack));
    
    _mCylinder->setCylinder(this);
    _cCylinder->setCylinder(this);
    setFilament(pf);
}


bool Cylinder::IfLast(){
    if (_ifLast) return true;
    
    return false;
}

void Cylinder::SetLast(bool b){ _ifLast = b;}