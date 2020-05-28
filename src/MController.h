
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v4.0
//
//  Copyright (2015-2018)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#ifndef MEDYAN_MController_h
#define MEDYAN_MController_h

#include <vector>

#include "common.h"

#include "Minimizer.h"
#include "ForceFieldManager.h"

#include "Parser.h"
#include "Structure/SubSystem.h"
#include "Util/Io/Log.hpp"

//FORWARD DECLARATIONS
class SubSystem;

/// Used to initialize, control, and run the mechanical components of a simulation

/*!
 *  MechanicalController is a class used by the SubSystem to initialize [ForceFields]
 *  (@ref ForceField), given an initial selection of which force fields should be 
 *  included. It can compute forces and energies over all force fields using the
 *  ForceFieldManager, as well as run energy [Minimization] (@ref Minimizer) algorithms.
 */
class MController {
    
private:
    ForceFieldManager _FFManager;  ///< Container and methods for all force
                                   ///< fields in system
    vector<Minimizer*> _minimizerAlgorithms; ///< Vector with algorithms
                                             ///< for system minimization
    
    SubSystem* _subSystem; ///< A pointer to the subsystem

    /// Initialize the force-fields used in the simualtion
    void initializeFF (MechParams::MechanicsFFType& forceFields);
    
    /// Initialize the minimization algorithms used in the simulation
    void initializeMinAlgorithms (MechParams::MechanicsAlgorithm& Minimizers);

public:
    /// Constructor which sets a subsystem pointer
    MController(SubSystem* s) {
        _subSystem = s;

        if(_subSystem->getCylinderLoadForceFunc()) {
            LOG(WARNING) << "The cylinder load force function has already been set.";
        }
        else {
            _subSystem->setCylinderLoadForceFunc([this](Cylinder* c, ForceFieldTypes::LoadForceEnd end) {
                _FFManager.computeLoadForce(c, end);
            });
        }
    }
    ~MController() {
        if(_subSystem->getCylinderLoadForceFunc()) {
            _subSystem->setCylinderLoadForceFunc(nullptr);
        }
        else {
            LOG(WARNING) << "The cylinder load force function has already been deleted.";
        }
    }
    
    /// Initialze the force fields and minimizers used
    void initialize(MechParams::MechanicsFFType forceFields, MechParams::MechanicsAlgorithm Minimizers) {
        initializeFF(forceFields);
        initializeMinAlgorithms(Minimizers);
    }

    /// Run a minimization on the system using the chosen algorithm
    auto run(bool steplimit = true) { return _minimizerAlgorithms[0]->equlibrate(_FFManager, steplimit); }

    
    ForceFieldManager* getForceFieldManager(){
        return &_FFManager;
    }

};

#endif
