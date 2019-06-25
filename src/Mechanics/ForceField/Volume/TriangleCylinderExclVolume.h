
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

#ifndef MEDYAN_TriangleCylinderExclVolume_h
#define MEDYAN_TriangleCylinderExclVolume_h

#include <memory> // unique_ptr

#include "Mechanics/ForceField/Volume/TriangleCylinderVolumeInteractions.h"
#include "NeighborListImpl.h"

#include "SysParams.h"

//FORWARD DECLARATIONS
class Triangle;
class Cylinder;
class Bead;

/// Represents an excuded volume interaction between a triangle and a cylinder (bead).
template <class TriangleCylinderExclVolumeInteractionType>
class TriangleCylinderExclVolume : public TriangleCylinderVolumeInteractions {
    
private:
    TriangleCylinderExclVolumeInteractionType _FFType;
    std::unique_ptr<TriangleCylinderNL> _neighborList;  ///< Neighbor list of cylinders

public:
    ///Constructor
    TriangleCylinderExclVolume() {
        _neighborList = std::make_unique<TriangleCylinderNL>(SysParams::Mechanics().MemCylinderVolumeCutoff);
    }

    virtual floatingpoint computeEnergy(const floatingpoint* coord, bool stretched) override;
    virtual void computeForces(const floatingpoint* coord, floatingpoint* force) override;

    virtual void computeLoadForces() override;

    /// Get the neighbor list for this interaction
    virtual NeighborList* getNeighborList() override { return _neighborList.get(); }
    
    virtual const string getName() {return "Triangle Cylinder Excluded Volume";}
};

#endif
