
//------------------------------------------------------------------
//  **M3SYM** - Simulation Package for the Mechanochemical
//              Dynamics of Active Networks, 3rd Generation
//
//  Copyright (2014) Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the Papoian lab page for more information:
//  http://papoian.chem.umd.edu/
//------------------------------------------------------------------

#ifndef M3SYM_ForceFieldManager_h
#define M3SYM_ForceFieldManager_h

#include <vector>

#include "common.h"

#include "ForceField.h"
#include "Bead.h"

/// A class to store and iterate over all [ForceFields](@ref ForceField).
/*!
 *  The ForceFieldManager is used to store all [ForceFields](@ref ForceField) 
 *  initialized by the system, as well as iterate over these potentials and calculate 
 *  total forces and energies. This class contains functions for the said calculations.
 */
class ForceFieldManager {
    
public:
     vector<ForceField*> _forceFields; ///< All forcefields in the system
    
    /// Compute the energy using all available force fields
    /// @return Returns infinity if there was a problem with a ForceField
    /// energy calculation, such that beads will not be moved to this
    /// problematic configuration.
    /// @param print - prints detailed info about energies
    double computeEnergy(double d, bool verbose = false);
    
    /// Compute the forces of all force fields 
    void computeForces();
    /// Compute the forcesAux of all force fields
    void computeForcesAux();
    /// Compute forcesAuxP of all force fields
    void computeForcesAuxP();
    
    /// Reset the forces of all objects
    void resetForces();
    /// Reset the forcesAux of all objects
    void resetForcesAux();
};

#endif
