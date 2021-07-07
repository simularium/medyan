
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

#ifndef MEDYAN_CylinderVolumeFF_h
#define MEDYAN_CylinderVolumeFF_h

#include <vector>
#include "HybridNeighborListImpl.h"

#include "common.h"

#include "ForceField.h"

//FORWARD DECLARATIONS
class CylinderVolumeInteractions;
class Cylinder;

/// An implementation of the ForceField class that calculates
/// Cylinder volume interactions.
class CylinderVolumeFF : public ForceField {

private:
    
protected:
    CylinderVolumeInteractions* _culpritInteraction; ///< Culprit in case of error

public:
    
    vector <unique_ptr<CylinderVolumeInteractions>>
    _cylinderVolInteractionVector;  ///< Vector of initialized volume interactions
    
    /// Initialize the volume forcefields
    CylinderVolumeFF(string& interaction);

    virtual void vectorize();
    virtual void cleanup();

    virtual string getName() {return "Excluded Volume";}
    virtual void whoIsCulprit();

    virtual floatingpoint computeEnergy(floatingpoint *coord, bool stretched = false) override;
    virtual void computeForces(floatingpoint *coord, floatingpoint *f);

    virtual void computeLoadForces() {return;}

    virtual vector<NeighborList*> getNeighborLists();
#if defined(HYBRID_NLSTENCILLIST) || defined(SIMDBINDINGSEARCH)
    //setter to store HybridneighborList pointer
    void setHNeighborLists(HybridCylinderCylinderNL* Hnl);
#endif

    virtual vector<string> getinteractionnames();
};

#endif
