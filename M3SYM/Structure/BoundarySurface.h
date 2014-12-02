
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

#ifndef M3SYM_BoundarySurface_h
#define M3SYM_BoundarySurface_h

#include <vector>

#include "common.h"

#include "BoundaryElementDB.h"

/// BoundarySurface class is a boundary shape that holds BoundaryElements
/*!
 *  The BoundarySurface class is a basic boundary shape that is owned and
 *  controlled by the [Boundary](@ref Boundary] that created it. It holds a vector of
 *  [BoundaryElements](@ref BoundaryElement) as well as any other geometrical information needed
 *  for the given implementation.
 */

class BoundarySurface {
    
protected:
    /// Vector of boundary elements that make up this surface
    vector<unique_ptr<BoundaryElement>> _boundaryElements;
    short _nDim; ///< Dimensionality of surface
    
public:
    ///Constructor, does nothing
    BoundarySurface(int nDim) : _nDim(nDim) {};
    /// Destructor, removes boundary elements from DB
    ~BoundarySurface() {
        //loop through boundary elements, remove from DB
        for (auto &b : _boundaryElements) BoundaryElementDB::instance()->removeBoundaryElement(b.get());
    };

    /// Get boundary elements
    const vector<unique_ptr<BoundaryElement>>& boundaryElements() {return _boundaryElements;}
    
};

#endif