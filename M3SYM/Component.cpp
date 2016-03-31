
//------------------------------------------------------------------
//  **M3SYM** - Simulation Package for the Mechanochemical
//              Dynamics of Active Networks, 3rd Generation
//
//  Copyright (2015)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#include "Component.h"

#include "Composite.h"
#include "Visitor.h"

Composite* Component::getRoot() {
    if(isRoot())
        return (Composite*)(this);
    else
        return getParent()->getRoot();
}

bool Component::apply (Visitor &v) {
    return v.visit(this);
}

