
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

#ifndef M3SYM_BoundarySurface_h
#define M3SYM_BoundarySurface_h

#include <vector>

#include "common.h"

#include "BoundaryElement.h"

//FORWARD DECLARATIONS
class SubSystem;

/// A boundary shape that holds [BoundaryElements](@ref BoundaryElement).
/*!
 *  The BoundarySurface class is a basic boundary shape that is owned and
 *  controlled by the Boundary that created it. It holds a vector of
 *  [BoundaryElements](@ref BoundaryElement) as well as any other geometrical 
 *  information needed for the given implementation.
 */
class BoundarySurface {
    
protected:
    SubSystem* _subSystem; ///< SubSystem ptr
    
    /// Vector of boundary elements that make up this surface
    vector<unique_ptr<BoundaryElement>> _boundaryElements;
    
    short _nDim; ///< Dimensionality of surface
    
public:
    ///Constructor, does nothing
    BoundarySurface(SubSystem* s, int nDim) : _subSystem(s), _nDim(nDim) {};
    /// Destructor
    ~BoundarySurface() {};

    /// Get boundary elements
    const vector<unique_ptr<BoundaryElement>>& boundaryElements() {
        return _boundaryElements;
    }
    
};

#endif
