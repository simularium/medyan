
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v3.2.1
//
//  Copyright (2015-2018)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#include "BoundaryElement.h"

void BoundaryElement::printSelf() const {
    
    cout << endl;
    
    cout << "Boundary element: ptr = " << this << endl;
    cout << "Coordinates = " << _coords[0] << ", " << _coords[1] << ", " << _coords[2] << endl;
    
    cout << endl;
}
