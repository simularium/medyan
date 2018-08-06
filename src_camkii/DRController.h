
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v3.1
//
//  Copyright (2015-2016)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#ifndef MEDYAN_DRController_h
#define MEDYAN_DRController_h

#include "common.h"

//FORWARD DECLARATIONS
struct DynamicRateType;

/// Used to initialize the dynamic rate components of a simulation.

/*!
 *  The DRController intializes all dynamic rate objects in the system, 
 *  as specified in the input file. The controller's intiailize function will 
 *  set [Cylinders](@ref Cylinder), [Linkers](@Linker) and [MotorGhosts](@MotorGhost)
 *  with corresponding static DynamicRateChanger objects.
 */
class DRController {
    
public:
    ///Intialize all elements with corresponding DynamicRateChanger objects.
    void initialize(DynamicRateType& drTypes);
};


#endif