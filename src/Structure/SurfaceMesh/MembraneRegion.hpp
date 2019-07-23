#ifndef MEDYAN_Structure_SurfaceMesh_MembraneRegion_Hpp
#define MEDYAN_Structure_SurfaceMesh_MembraneRegion_Hpp

#include <array>
#include <vector>
#include <memory>

#include "MathFunctions.h"
#include "Structure/Boundary.h"
#include "Structure/SurfaceMesh/MembraneHierarchy.hpp"

/******************************************************************************
 * 
 * Membrane Region is a class for management of regions enclosed by membranes
 * and boundaries.
 * 
 * Note: before building the membrane region, all the related membranes must
 * have the updated geometry.
 * 
******************************************************************************/

template< typename MemType >
class MembraneRegion {
public:
    using HierarchyType = MembraneHierarchy< MemType >;

private:
    Boundary* _boundary = nullptr; ///< The boundary of the playground
    std::vector<HierarchyType*> _hierOut; ///< Outer limit. A point is inside the region if it is in at least 1 of the membranes.
    std::vector<HierarchyType*> _hierIn;  ///< Inner limit. A point is inside the region if it is not in any of the membranes.

    /// Constructor only for internal use
    MembraneRegion() {}

public:
    /// Construct the region with one membrane
    MembraneRegion(HierarchyType* hier, bool excludeChildren=false)
        : _hierOut({hier})
    {
        if(excludeChildren) {
            const auto n = hier->numberOfChildren();
            _hierIn.reserve(n);
            for(size_t idx = 0; idx < n; ++idx) {
                _hierIn.push_back( static_cast< HierarchyType* >(hier->children(idx)) );
            }
        }
    }

    /// Construct the region with the boundary
    MembraneRegion(Boundary* b): _boundary(b) {}
    MembraneRegion(Boundary* b, HierarchyType* parentOfExcluded)
        : _boundary(b)
    {
        const auto n = parentOfExcluded->numberOfChildren();
        _hierIn.reserve(n);
        for(size_t idx = 0; idx < n; ++idx) {
            _hierIn.push_back( static_cast< HierarchyType* >(parentOfExcluded->children(idx)) );
        }
    }

    /// Is point inside region
    template< typename VecType, std::enable_if_t< VecType::vec_size == 3 >* = nullptr >
    bool contains(const VecType& point) const {
        /**************************************************************************
        This function checks whether a certain point is in the region. If a point
        is in the region, it must satisfy all of the following.
            - within _boundary if specified
            - in any of the membranes in _hierOut
            - not in any of the membranes in _hierIn
        **************************************************************************/
        if(_boundary) {
            auto p = mathfunc::vec2Vector(point);
            if(!_boundary->within(p)) return false;
        }

        if(!_hierOut.empty()) {
            bool hierOutGood = false;
            for(auto eachHier: _hierOut) {
                if(eachHier->getMembrane()->contains(point)) {
                    hierOutGood = true;
                    break;
                }
            }
            if(!hierOutGood) return false;
        }

        for(auto eachHier: _hierIn) {
            if(eachHier->getMembrane()->contains(point)) return false;
        }

        return true;
    }

    /**************************************************************************
    Getters and Setters
    **************************************************************************/
    Boundary* getBoundary()const { return _boundary; }
    const std::vector<HierarchyType*>& getHierOut()const { return _hierOut; }
    const std::vector<HierarchyType*>& getHierIn ()const { return _hierIn;  }

    /**************************************************************************
    Factory functions
    **************************************************************************/
    /// Create region with hier's children as outer limit
    static std::unique_ptr<MembraneRegion> makeByChildren(const HierarchyType& hier, bool excludeChildren=false) {
        std::unique_ptr< MembraneRegion > mr(new MembraneRegion());

        for(const auto& it: hier.children()) {
            auto eachHier = static_cast< HierarchyType* >(it.get());
            if(eachHier->getMembrane()->isClosed()) {
                mr->_hierOut.push_back(eachHier);

                if(excludeChildren) {
                    size_t n = eachHier->numberOfChildren();
                    for(size_t idx = 0; idx < n; ++idx) {
                        auto childHier = static_cast< HierarchyType* >(eachHier->children(idx));
                        if(childHier->getMembrane()->isClosed())
                            mr->_hierIn.push_back( childHier );
                    }
                }
            }
        }

        return mr;
    }
};

#endif