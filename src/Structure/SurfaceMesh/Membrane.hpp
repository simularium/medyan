#ifndef MEDYAN_Membrane_hpp
#define MEDYAN_Membrane_hpp

#include <array>
#include <limits> // numeric_limits
#include <stdexcept> // logic_error
#include <vector>
#include <memory>

#include "Database.h"
#include "Trackable.h"
#include "Composite.h"

#include "MathFunctions.h"
#include "MMembrane.h"
#include "Structure/SubSystem.h"
#include "Structure/SurfaceMesh/MembraneMeshAttribute.hpp"
#include "Structure/SurfaceMesh/SurfaceMesh.hpp"

/******************************************************************************
Topologically, the membrane is represented by a 2d surface with 2 sides (which
means no Klein bottles are allowed!). The surface is constructed by
interconnected vertices, edges and triangles.

The Membrane class is a manager for constructing the mesh and computing the
geometry. It contains a meshwork instance that's responsible for adding and
removing vertices, edges (halfedges) and triangles to/from the SubSystem.
However, the ownership of all elements is in this Membrane class through
inheriting Composite.
******************************************************************************/
class Membrane: public Composite, public Trackable {
public:
    using coordinate_type = MembraneMeshAttribute::coordinate_type;
    using MembraneMeshAttributeType = MembraneMeshAttribute< SurfaceTriangularMesh >;
    using MeshType = SurfaceTriangularMesh< MembraneMeshAttributeType >;

private:

    MeshType _mesh;

    unique_ptr<MMembrane> _mMembrane; // pointer to mechanical membrane object

    short _memType; // Membrane type

    SubSystem* _subSystem; // SubSystem pointer

    static Database<Membrane*> _membranes; // Collection in SubSystem
    int _id; // Unique integer id of this membrane

public:

    // Constructors
    // This constructor creates a membrane according to vertex and neighbor data
    Membrane(
        SubSystem* s,
        short membraneType,
        const std::vector< coordinate_type >& vertexCoordinateList,
        const std::vector< std::array< size_t, 3 > >& triangleVertexIndexList
    );

    /// Get vector of triangles/edges/vertices that this membrane contains.
    const auto& getMesh() const { return _mesh; }
    auto&       getMesh()       { return _mesh; }

    // Get Id
    int getId()const { return _id; }
    
    // SubSystem management, inherited from Trackable
    virtual void addToSubSystem()override { _membranes.addElement(this); }
    virtual void removeFromSubSystem()override { _membranes.removeElement(this); }
    
    /// Get all instances of this class from the SubSystem
    static const vector<Membrane*>& getMembranes() {
        return _membranes.getElements();
    }
    /// Get the number of membranes in this system
    static int numMembranes() {
        return _membranes.countElements();
    }

    //@{
    /// Implements Component
    // Get type
    int getType()override { return _memType; }
    // Print self information
    virtual void printSelf()const override;
    //@}

    /**************************************************************************
    Geometric
    **************************************************************************/
    template< bool stretched = false > void updateGeometryValue() {
        MembraneMeshAttribute::updateGeometryValue<stretched>(_mesh);
    }
    void updateGeometryValueWithDerivative() {
        MembraneMeshAttribute::updateGeometryValueWithDerivative(_mesh);
    }

    /**
     * Use pseudo normal signed distance field method to get the signed distance to a point.
     * If the point is outside, the result is positive and vice versa.
     * Throws an exception if the membrane is not closed.
     * The function will search through the whole meshwork, so it might not be efficient.
     */
    double signedDistance(const mathfunc::Vec3& p) const {
        if(!isClosed()) throw std::logic_error("Membrane is not closed while trying to find signed distance field.");
        return MembraneMeshAttribute::signedDistance(_mesh, p);
    }
    /**
     * Use signed distance or other methods to judge whether a point is inside membrane.
     * Throws an exception if the membrane is not closed.
     */
    bool contains(const mathfunc::Vec3& p) const {
        if(!isClosed()) throw std::logic_error("Membrane is not closed while trying to find signed distance field.");
        return MembraneMeshAttribute::contains(_mesh, p);
    }

    // Function to monitor the quality of the meshwork
    double meshworkQuality()const; // Must be used after updating the geometry
                                   // Returns a value between 0 and 1,
                                   // 1 being best and 0 being worst.

    /**************************************************************************
    Topological
    **************************************************************************/
    bool updateClosedness() { return _mesh.updateClosedness(); }
    bool isClosed() const { return _mesh.isClosed(); }

    /**************************************************************************
    Mechanics
    **************************************************************************/
    // Get mech membrane
    MMembrane* getMMembrane() { return _mMembrane.get(); }


};





#endif
