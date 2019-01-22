/*

Adaptive mesh algorithm

Implementation inspired by
"An Adaptive Mesh Algorithm for Evolving Surfaces: Simulations of Drop Breakup and Coalescence" (2001)
by Vittorio Cristini, Jerzy Blawzdziewicz and Michael Loewenberg,
"About surface remeshing" (2000) by Pascal J. Frey,
"Geometric surface mesh optimization" (1998) by Pascal J. Frey and Houman Borouchaki

Performs mesh relaxation and topological transformation to improve mesh size
quality and shape quality, while maintaining the geometrical accuracy.

The algorithm was not introduced explicity in the article, and we'll formalize it as follows

Init: Find per-element quantities for all elements.
Loop
    Global relaxation
    Update per-element quantities and local averaged quantities
    For all places that does not satisfy criteria 2, 3
        Local topological transformation
        Local relaxation
        Update affected per-element quantities and local averaged quantites
    End
Until all criteria are met

*/

#ifndef MEDYAN_AdaptiveMesh_hpp
#define MEDYAN_AdaptiveMesh_hpp

#include <algorithm> // max min
#include <cstdint>
#include <limits>
#include <vector>

#include "MathFunctions.h"

#include "Structure/SurfaceMesh/AdaptiveMeshAttribute.hpp"
#include "Structure/SurfaceMesh/Membrane.hpp"

enum class TriangleQualityCriteria {
    RadiusRatio     // Circumradius / (2 * Inradius), [1, inf)
};
template< TriangleQualityCriteria > struct TriangleQuality;
template<> struct TriangleQuality< TriangleQualityCriteria::RadiusRatio > {
    static constexpr double best = 1.0;
    static constexpr double worst = std::numeric_limits<double>::infinity();
    static constexpr bool better(double q1, double q2) { return q1 < q2; }
    static constexpr auto betterOne(double q1, double q2) { return better(q1, q2) ? q1 : q2; }
    static constexpr bool worse(double q1, double q2) { return q1 > q2; }
    static constexpr auto worseOne(double q1, double q2) { return worse(q1, q2) ? q1 : q2; }
    static constexpr auto improvement(double q0, double q1) { return q0 / q1; }

    template< typename VecType >
    auto operator()(const VecType& v0, const VecType& v1, const VecType& v2) const {
        using namespace mathfunc;
        const auto d0 = distance(v1, v2);
        const auto d1 = distance(v2, v0);
        const auto d2 = distance(v0, v1);
        const auto p = 0.5 * (d0 + d1 + d2);
        return d0 * d1 * d2 / (8 * (p - d0) * (p - d1) * (p - d2));
    }
}

template< typename Mesh, TriangleQualityCriteria c > class EdgeFlipManager {
public:
    using TriangleQualityType = TriangleQuality< c >;

private:
    size_t _minDegree;
    size_t _maxDegree;
    double _minDotNormal; // To assess whether triangles are coplanar.

public:

    // Constructor
    // Parameters
    //   - minDegree: minimum number of neighbors of any vertex
    //   - maxDegree: maximum number of neighbors of any vertex
    //   - minDotNormal: minimum dot product required between neighboring triangles. Range (-1, 1)
    EdgeFlipManager(size_t minDegree, size_t maxDegree, double minDotNormal) :
        _minDegree(minDegree), _maxDegree(maxDegree), _minDotNormal(minDotNormal) {}

    // Returns whether the edge is flipped.
    // Requires
    //   - vertex degrees
    //   - triangle unit normal and quality
    bool tryFlip(Mesh& mesh, size_t ei) const {
        using namespace mathfunc;

        const size_t hei = mesh.getEdges()[ei].halfEdgeIndex;
        const size_t hei_o = mesh.opposite(hei);
        const size_t hei_n = mesh.next(hei);
        const size_t hei_p = mesh.prev(hei);
        const size_t hei_on = mesh.next(hei_o);
        const size_t hei_op = mesh.prev(hei_o);

        const size_t vi0 = mesh.target(hei);
        const size_t vi1 = mesh.target(hei_n);
        const size_t vi2 = mesh.target(hei_o);
        const size_t vi3 = mesh.target(hei_on);
        // Currently the edge connects v0 and v2.
        // If the edge flips, the connection would be between v1 and v3.

        const size_t ti0 = mesh.triangle(hei);
        const size_t ti1 = mesh.triangle(hei_o);

        // Check if topo constraint is satisfied.
        if(
            mesh.degree(vi0) <= _minDegree ||
            mesh.degree(vi2) <= _minDegree ||
            mesh.degree(vi1) >= _maxDegree ||
            mesh.degree(vi3) >= _maxDegree
        ) return false;

        // Check if the current triangles are coplanar.
        if(dot(
            mesh.getTriangleAttribute(ti0).gTriangle.unitNormal,
            mesh.getTriangleAttribute(ti1).gTriangle.unitNormal
        ) < _minDotNormal) return false;

        // Check if the target triangles are coplanar.
        const auto c0 = vector2Vec<3, double>(mesh.getVertexAttribute(vi0).getCoordinate());
        const auto c1 = vector2Vec<3, double>(mesh.getVertexAttribute(vi1).getCoordinate());
        const auto c2 = vector2Vec<3, double>(mesh.getVertexAttribute(vi2).getCoordinate());
        const auto c3 = vector2Vec<3, double>(mesh.getVertexAttribute(vi3).getCoordinate());
        const auto un013 = normalizedVector(cross(c1 - c0, c3 - c0));
        const auto un231 = normalizedVector(cross(c3 - c2, c1 - c2));
        if(dot(un013, un231) < _minDotNormal) return false;

        // Check whether triangle quality can be improved.
        const auto qBefore = TriangleQualityType::betterOne(
            mesh.getTriangleAttribute(ti0).aTriangle.quality,
            mesh.getTriangleAttribute(ti1).aTriangle.quality
        );
        const auto q013 = TriangleQualityType{}(c0, c1, c3);
        const auto q231 = TriangleQualityType{}(c2, c3, c1);
        const auto qAfter = TriangleQualityType::betterOne(q013, q231);
        if( !TriangleQualityType::better(qAfter, qBefore) ) return false;

        // All checks complete. Do the flip.
        typename Mesh::EdgeFlip{}(mesh, ei);

        // TODO: set new triangle attributes
        // TODO: set new edge length (?)
        return true;
    }
};

enum class EdgeSplitVertexInsertionMethod {
    MidPoint
};
template< EdgeSplitVertexInsertionMethod > struct EdgeSplitVertexInsertion;
template<> struct EdgeSplitVertexInsertion< EdgeSplitVertexInsertionMethod::MidPoint > {
    size_t v0, v1;
    template< typename Mesh >
    auto coordinate(const Mesh& mesh, size_t v) const {
        const auto& c0 = mesh.getVertexAttribute(v0).getCoordinate();
        const auto& c1 = mesh.getVertexAttribute(v1).getCoordinate();
        return mathfunc::midPointCoordinate(c0, c1, 0.5);
    }
};

template<
    typename Mesh,
    TriangleQualityCriteria c,
    EdgeSplitVertexInsertionMethod m
> class EdgeSplitManager {
public:
    using EdgeFlipManagerType = EdgeFlipManager< Mesh, c >;
    using EdgeSplitVertexInsertionType = EdgeSplitVertexInsertion< m >;

private:
    size_t _maxDegree;

public:
    // Returns whether a new vertex is inserted.
    // Requires
    //   - Vertex degree
    bool trySplit(Mesh& mesh, size_t ei, const EdgeFlipManagerType& efm) const {
        using namespace mathfunc;

        const size_t hei = mesh.getEdges()[ei].halfEdgeIndex;
        const size_t hei_o = mesh.opposite(hei);
        const size_t hei_n = mesh.next(hei);
        const size_t hei_p = mesh.prev(hei);
        const size_t hei_on = mesh.next(hei_o);
        const size_t hei_op = mesh.prev(hei_o);

        const size_t vi0 = mesh.target(hei);
        const size_t vi1 = mesh.target(hei_n);
        const size_t vi2 = mesh.target(hei_o);
        const size_t vi3 = mesh.target(hei_on);

        const size_t ei0 = mesh.edge(hei_n); // v0 - v1
        const size_t ei1 = mesh.edge(hei_p); // v1 - v2
        const size_t ei2 = mesh.edge(hei_on); // v2 - v3
        const size_t ei3 = mesh.edge(hei_op); // v3 - v1

        // Check topology constraints
        // A new vertex with degree 4 will always be introduced
        if(
            mesh.degree(vi1) >= _maxDegree ||
            mesh.degree(vi3) >= _maxDegree
        ) return false;

        // All checks passed. Do the splitting.
        typename Mesh::VertexInsertionOnEdge< EdgeSplitVertexInsertionType > {}(mesh, ei);

        // TODO: set new triangle attributes
        // TODO: update edge preferred lengths
        // TODO: update edge lengths (?)

        // Propose edge flipping on surrounding quad edges
        efm.tryFlip(mesh, ei0);
        efm.tryFilp(mesh, ei1);
        efm.tryFlip(mesh, ei2);
        efm.tryFlip(mesh, ei3);

        return true;

    }
};

template< typename Mesh, TriangleQualityCriteria c > class EdgeCollapseManager {
public:
    using TriangleQualityType = TriangleQuality< c >;

private:
    size_t _minDegree;
    size_t _maxDegree;
    double _minQualityImprovement; // If smaller than 1, then some degradation is allowed.

public:
    // Returns whether the edge is collapsed
    // Requires
    //   - Triangle quality
    bool tryCollapse(Mesh& mesh, size_t ei) const {
        using namespace mathfunc;

        const size_t hei = mesh.getEdges()[ei].halfEdgeIndex;
        const size_t hei_o = mesh.opposite(hei);
        const size_t hei_n = mesh.next(hei);
        const size_t hei_p = mesh.prev(hei);
        const size_t hei_on = mesh.next(hei_o);
        const size_t hei_op = mesh.prev(hei_o);

        const size_t vi0 = mesh.target(hei);
        const size_t vi1 = mesh.target(hei_n);
        const size_t vi2 = mesh.target(hei_o);
        const size_t vi3 = mesh.target(hei_on);
        // Currently the edge connects v0 and v2.
        // If the edge collapses, v0 and v2 would become one point.

        const size_t ti0 = mesh.triangle(hei);
        const size_t ti1 = mesh.triangle(hei_o);

        // Check topology constraints
        if(
            mesh.degree(vi0) + mesh.degree(vi2) - 4 > _maxDegree ||
            mesh.degree(vi0) + mesh.degree(vi2) - 4 < _minDegree ||
            mesh.degree(vi1) <= _minDegree ||
            mesh.degree(vi3) <= _minDegree
        ) return false;

        // Future: maybe also geometric constraints (gap, smoothness, etc)

        // Check triangle quality constraints
        const auto c0 = vector2Vec<3, double>(mesh.getVertexAttribute(vi0).getCoordinate());
        const auto c2 = vector2Vec<3, double>(mesh.getVertexAttribute(vi2).getCoordinate());

        // Calculate previous triangle qualities around a vertex
        // if v0 is removed
        double q0Before = TriangleQualityType::worst;
        double q0After = TriangleQualityType::worst;
        mesh.forEachHalfEdgeTargetingVertex(vi0, [&](size_t hei) {
            const size_t ti = mesh.triangle(hei);
            q0Before = TriangleQualityType::betterOne(
                mesh.getTriangleAttribute(ti).aTriangle.quality,
                q0Before
            );
            if(ti != ti0 && ti != ti1) {
                const size_t vn = mesh.target(mesh.next(hei));
                const size_t vp = mesh.target(mesh.prev(hei));
                const auto cn = vector2Vec<3, double>(mesh.getVertexAttribute(vn).getCoordinate());
                const auto cp = vector2Vec<3, double>(mesh.getVertexAttribute(vp).getCoordinate());
                q0After = TriangleQualityType::betterOne(
                    TriangleQualityType{}(cp, c2, cn),
                    q0After
                );
            }
        });
        const auto imp0 = TriangleQualityType::improvement(q0Before, q0After);

        // if v2 is removed
        double q2Before = TriangleQualityType::worst;
        double q2After = TriangleQualityType::worst;
        mesh.forEachHalfEdgeTargetingVertex(vi2, [&](size_t hei) {
            const size_t ti = mesh.triangle(hei);
            q2Before = TriangleQualityType::betterOne(
                mesh.getTriangleAttribute(ti).aTriangle.quality,
                q2Before
            );
            if(ti != ti0 && ti != ti1) {
                const size_t vn = mesh.target(mesh.next(hei));
                const size_t vp = mesh.target(mesh.prev(hei));
                const auto cn = vector2Vec<3, double>(mesh.getVertexAttribute(vn).getCoordinate());
                const auto cp = vector2Vec<3, double>(mesh.getVertexAttribute(vp).getCoordinate());
                q2After = TriangleQualityType::betterOne(
                    TriangleQualityType{}(cp, c0, cn),
                    q2After
                );
            }
        });
        const auto imp2 = TriangleQualityType::improvement(q2Before, q2After);

        if(imp0 < _minQualityImprovement && imp2 < _minQualityImprovement) return false;

        if(imp0 > imp2) {
            // Remove v0, collapse onto v2
            typename Mesh::EdgeCollapse {}(mesh, hei_o);
        } else {
            // Remove v2, collapse onto v0
            typename Mesh::EdgeCollapse {}(mesh, hei);
        }

        // TODO: update triangle normals and qualities
        // Do not update edge preferred lengths
        // TODO: update edge lengths?

        return true;
    }
};

enum class RelaxationType {
    GlobalElastic // E = (l - l_0)^2 / (2 l_0), with const elastic modulus 1.0
};
template< RelaxationType > struct RelaxationForceField;
template<> struct RelaxationForceField< RelaxationType::GlobalElastic > {

    // The function requires the vertex unit normal information
    template< typename Mesh, typename VecType >
    void computeForces(std::vector<VecType>& forces, const Mesh& mesh, const std::vector<VecType>& coords) {
        // The size of forces should be the same as the size of coords.
        // Resizing of forces should be done by the caller.
        const size_t numVertices = coords.size();
        for(size_t i = 0; i < numVertices; ++i) {
            VecType f {};
            mesh.forEachHalfEdgeTargetingVertex(i, [&](size_t hei) {
                const auto l0 = mesh.getEdgeAttribute(mesh.edge(hei)).aEdge.eqLength;
                const auto r = coords[mesh.target(mesh.opposite(hei))] - coords[i];
                const auto mag = mathfunc::magnitude(r);
                f += r * (1.0 / l0 - 1.0 / mag);
            });

            const auto& un = mesh.getVertexAttribute(v).aVertex.unitNormal;
            f -= un * mathfunc::dot(un, f); // Remove normal component

            forces[i] = f;
        }
    }
};

// For the purpose of global relaxation, we create a coordinate list and do work on them.
template<
    typename Mesh,
    RelaxationType r,
    TriangleQualityCriteria c
> class GlobalRelaxationManager {
public:
    using RelaxationForceFieldType = RelaxationForceField< r >;
    using EdgeFlipManagerType = EdgeFlipManager< Mesh, c >;

private:
    double _epsilon2; // Square of force tolerance
    double _dt; // Step size for Runge Kutta method
    size_t _maxIterRelocation;
    size_t _maxIterRelaxation; // each iteration: (relocation + edge flipping)

    // Utility for max force magnitude squared
    template< typename VecType > static auto _maxMag2(std::vector<VecType>& v) {
        double res = 0.0;
        for(const auto& i : v) {
            double mag2 = mathfunc::magnitude2(i);
            if(mag2 > res) res = mag2;
        }
        return res;
    }

    // Relocates vertices using 2nd order Runge Kutta method
    // Returns whether the final forces are below threshold.
    template< typename VecType > bool _vertexRelocation(
        std::vector<VecType>& coords,
        std::vector<VecType>& forces,
        std::vector<VecType>& coordsHalfway,
        std::vector<VecType>& forcesHalfway,
        const Mesh& mesh
    ) const {
        const size_t numVertices = coords.size();

        RelaxationForceFieldType().computeForces(forces, mesh, coords);
        auto maxMag2F = _maxMag2(forces);

        size_t iter = 0;
        while(maxMag2F >= _epsilon2 && iter < _maxIterRelocation) {
            ++iter;

            // Test move halfway
            for(size_t i = 0; i < numVertices; ++i)
                coordsHalfway[i] = coords[i] + (0.5 * dt) * forces[i];

            // Force at halfway
            RelaxationForceFieldType().computeForces(forcesHalfway, mesh, coordsHalfway);

            // Real move
            for(size_t i = 0; i < numVertices; ++i)
                coords[i] += forcesHalfway[i] * dt;

            // Compute new forces
            RelaxationForceFieldType().computeForces(forces, mesh, coords);
            maxMag2F = _maxMag2(forces);
        }

        if(maxMag2F >= _epsilon2) return false;
        else return true;
    }

    // Returns whether at least 1 edge is flipped
    bool _edgeFlipping(Mesh& mesh, const EdgeFlipManagerType& efm) const {
        // Edge flipping does not change edge id or total number of edges
        // Also the preferred length does not need to be changed
        bool res = false;
        const size_t numEdges = mesh.getEdges().size();
        for(size_t i = 0; i < numEdges; ++i) {
            if(efm.tryFlip(mesh, i)) res = true;
        }
        return res;
    }

public:
    // Returns whether relaxation is complete.
    // Requires:
    //   - Normals on vertices (not updated during relaxation)
    //   - Preferred lengths of edges (not updated during relaxation)
    bool relax(Mesh& mesh, const EdgeFlipManagerType& efm) const {
        // Initialization
        const size_t numVertices = mesh.getVertices().size();
        std::vector< mathfunc::Vec3 > coords(numVertices);
        for(size_t i = 0; i < numVertices; ++i) {
            coords[i] = vector2Vec<3, double>(mesh.getVertexAttribute(i).vertex->getCoordinate());
        }

        // Aux variables
        std::vector< mathfunc::Vec3 > forces(numVertices);
        std::vector< mathfunc::Vec3 > coordsHalfway(numVertices);
        std::vector< mathfunc::Vec3 > forcesHalfway(numVertices);

        // Main loop
        bool needRelocation = true;
        bool needFlipping = true;
        size_t iter = 0;
        while( (needRelocation || needFlipping) && iter < _maxIterRelaxation) {
            ++iter;
            needRelocation = !_vertexRelocation(coords, forces, coordsHalfway, forcesHalfway, mesh);
            needFlipping = _edgeFlipping(mesh, efm);
        }

        // Reassign coordinates
        for(size_t i = 0; i < numVertices; ++i) {
            mesh.getVertexAttribute(i).vertex->getCoordinate() = vec2Vector(coords[i]);
        }

        if(needRelocation || needFlipping) return false;
        else return true;
    }
};

enum class SizeMeasureCriteria {
    Curvature
};
template< SizeMeasureCriteria > struct VertexSizeMeasure;
template<> struct VertexSizeMeasure< SizeMeasureCriteria::Curvature > {
    double resolution; // size = res * min_radius_curvature
    double upperLimit; // maximum size

    // Requires
    //   - Vertex unit normal
    template< typename Mesh > auto vertexMaxSize(Mesh& mesh, size_t vi) const {
        double minRadiusCurvature = std::numeric_limits<double>::infinity();
        const auto& un = mesh.getVertexAttribute(vi).aVertex.unitNormal;
        const auto ci = mathfunc::vector2Vec<3, double>(mesh.getVertexAttribute(vi).vertex->getCoordinate());
        mesh.forEachHalfEdgeTargetingVertex(vi, [&](size_t hei) {
            const auto r = mathfunc::vector2Vec<3, double>(mesh.getVertexAttribute(mesh.target(mesh.opposite(hei))).vertex->getCoordinate()) - ci;
            minRadiusCurvature = std::min(
                std::abs(0.5 * mathfunc::magnitude2(r) / mathfunc::dot(un, r)),
                minRadiusCurvature
            );
        });
        
        return std::min(resolution * minRadiusCurvature, upperLimit);
    }
};

template< SizeMeasureCriteria... > struct VertexSizeMeasureCombined;
template< SizeMeasureCriteria c, SizeMeasureCriteria... cs >
struct VertexSizeMeasureCombined< c, cs... > {
    template< typename Mesh >
    static auto vertexMaxSize(Mesh& mesh, size_t vi, const VertexSizeMeasure<c>& vsm, const VertexSizeMeasure<cs>&... vsms) const {
        return std::min(vsm.vertexMaxSize(mesh, vi), VertexSizeMeasureCombined<cs...>::vertexMaxSize(mesh, vi, vsms...));
    }
};
template< SizeMeasureCriteria c >
struct VertexSizeMeasureCombined< c > {
    template< typename Mesh >
    static auto vertexMaxSize(Mesh& mesh, size_t vi, const VertexSizeMeasure<c>& vsm) const {
        return vsm.vertexMaxSize(mesh, vi);
    }
};

template< typename Mesh > class SizeMeasureManager {
private:

    double _curvRes; // resolution used in radius curvature
    double _maxSize; // Hard upper bound of size
    size_t _diffuseIter; // Diffusion iterations used in gradation control

    template< SizeMeasureCriteria... cs >
    auto _vertexMaxSize(Mesh& mesh, size_t vi, const VertexSizeMeasure<cs>&... vsms) const {
        return VertexSizeMeasureCombined<cs...>::vertexMaxSize(mesh, vi, vsms...);
    }
    template< SizeMeasureCriteria... cs >
    void _updateVertexMaxSize(Mesh& mesh, const VertexSizeMeasure<cs>&... vsms) const {
        const size_t numVertices = mesh.getVertices().size();
        for(size_t i = 0; i < numVertices; ++i) {
            mesh.getVertexAttribute(i).aVertex.maxSize = _vertexMaxSize(mesh, i, vsms...);
        }
    }

    void _diffuseSize(Mesh& mesh) const {
        const size_t numVertices = mesh.getVertices().size();

        // Initialize with max size
        for(size_t i = 0; i < numVertices; ++i) {
            auto& av = mesh.getVertexAttribute(i).aVertex;
            av.size = av.maxSize;
        }

        // Diffuse, with D * Delta t = 0.5, and uniformly weighted Laplace operator
        // l_new = l_old / 2 - (sum of neighbor l_old) / (2 * numNeighbors)
        for(size_t iter = 0; iter < _diffuseIter; ++iter) {
            for(size_t i = 0; i < numVertices; ++i) {
                auto& av = mesh.getVertexAttribute(i).aVertex;
                const size_t deg = mesh.degree(i);
    
                double sumSizeNeighbor = 0.0;
                mesh.forEachHalfEdgeTargetingVertex(i, [&](size_t hei) {
                    sumSizeNeighbor += mesh.getVertexAttribute(mesh.target(mesh.opposite(hei))).aVertex.size;
                });

                av.sizeAux = std::min(
                    0.5 * av.size - 0.5 * sumSizeNeighbor / deg,
                    av.maxSize
                ); // capped by maxSize
            }
            for(size_t i = 0; i < numVertices; ++i) {
                auto& av = mesh.getVertexAttribute(i).aVertex;
                av.size = av.sizeAux;
            }
        }
    }

    void _updateEdgeEqLength(Mesh& mesh) const {
        const size_t numEdges = mesh.getEdges().size();
        for(size_t i = 0; i < numEdges; ++i) {
            auto& l0 = mesh.getEdgeAttribute(i).aEdge.eqLength;
            l0 = 0.0;
            mesh.forEachHalfEdgeInEdge(i, [&](size_t hei) {
                l0 += 0.5 * mesh.getVertexAttribute(mesh.target(hei)).aVertex.size;
            });
        }
    }

public:

    // Requires
    //   - Unit normal on each vertex
    void computeSizeMeasure(Mesh& mesh) const {
        VertexSizeMeasure< SizeMeasureCriteria::Curvature > vsmCurv {_curvRes, _maxSize};

        // Compute size on each vertex
        _updateVertexMaxSize(mesh, vsmCurv);

        // Diffuse size on vertices
        _diffuseSize(mesh);

        // Compute preferred length of edges
        _updateEdgeEqLength(mesh);
    }
};

template< typename Mesh >
class MeshAdapter {
public:
    static constexpr auto relaxationType = RelaxationType::GlobalElastic;
    static constexpr auto triangleQualityCriteria = TriangleQualityCriteria::RadiusRatio;
    static constexpr auto edgeSplitVertexInsertionMethod = EdgeSplitVertexInsertionMethod::MidPoint;

private:
    SizeMeasureManager< Mesh > _sizeMeasureManager;
    GlobalRelaxationManager< Mesh, relaxationType, triangleQualityCriteria > _globalRelaxationManager;

    EdgeFlipManager< Mesh, triangleQualityCriteria > _edgeFlipManager;
    EdgeSplitManager< Mesh, triangleQualityCriteria, edgeSplitVertexInsertionMethod > _edgeSplitManager;
    EdgeCollapseManager< Mesh, triangleQualityCriteria > _edgeCollapseManager;

    void _computeTriangleNormals(Mesh& mesh) const {
        const size_t numTriangles = mesh.getTriangles().size();

        for(size_t ti = 0; ti < numTriangles; ++ti) {
            const size_t hei = triangles[ti].halfEdgeIndex;
            const size_t vi0 = mesh.target(hei);
            const size_t vi1 = mesh.target(mesh.next(hei));
            const size_t vi2 = mesh.target(mesh.prev(hei));
            const auto& c0 = vertices[vi0].attr.vertex->coordinate;
            const auto& c1 = vertices[vi1].attr.vertex->coordinate;
            const auto& c2 = vertices[vi2].attr.vertex->coordinate;
            auto& tag = mesh.getTriangleAttribute(ti).gTriangle;

            const auto vp = mathfunc::vectorProduct(c0, c1, c0, c2);

            // unit normal
            tag.unitNormal = mathfunc::vector2Vec<3, double>(mathfunc::normalizedVector(vp));
        }
    }

    void _computeAngles(Mesh& mesh) const {
        const size_t numHalfEdges = mesh.getHalfEdges().size();

        for(size_t hei = 0; hei < numHalfEdges; ++hei) {
            // The angle is (v0, v1, v2)
            const size_t vi0 = mesh.target(mesh.prev(hei));
            const size_t vi1 = mesh.target(hei);
            const size_t vi2 = mesh.target(mesh.next(hei));
            const auto& c0 = vertices[vi0].attr.vertex->coordinate;
            const auto& c1 = vertices[vi1].attr.vertex->coordinate;
            const auto& c2 = vertices[vi2].attr.vertex->coordinate;
            auto& heag = mesh.getHalfEdgeAttribute(hei).gHalfEdge;

            const auto vp = mathfunc::vectorProduct(c1, c0, c1, c2);
            const auto sp = mathfunc::scalarProduct(c1, c0, c1, c2);
            const auto ct = heag.cotTheta = sp / mathfunc::magnitude(vp);
            heag.theta = M_PI_2 - std::atan(ct);
        }
    }

    // Requires
    //   - Unit normals in triangles (geometric)
    //   - Angles in halfedges (geometric)
    void _computeVertexNormals(Mesh& mesh) const {
        const size_t numVertices = mesh.getVertices().size();

        // Using pseudo normal (weighted by angles)
        for(size_t vi = 0; vi < numVertices; ++vi) {
            auto& vaa = mesh.getVertexAttribute(vi).aVertex;

            // clearing
            vaa.unitNormal = {0.0, 0.0, 0.0};

            mesh.forEachHalfEdgeTargetingVertex(vi, [&](size_t hei) {
                const size_t ti0 = mesh.triangle(hei);
                const auto theta = mesh.getHalfEdgeAttribute(hei).gHalfEdge.theta;
                vaa.unitNormal += theta * mesh.getTriangleAttribute(ti0).gTriangle.unitNormal;
            });

            mathfunc::normalize(vaa.unitNormal);
        }

    }

    void _computeSizeMeasures(Mesh& mesh) const {
        _computeTriangleNormals(mesh);
        _computeAngles(mesh);
        _computeVertexNormals(mesh);
        _sizeMeasureManager.computeSizeMeasure(mesh);
    }
public:
    MeshAdapter() {}

    void adapt(Mesh& mesh) const {
        using namespace mathfunc;

        init();

        _computeSizeMeasures();

        while(true) {
            bool sizeMeasureSatisfied = true;

            size_t countTopoModified;
            do {
                countTopoModified = 0;
                for(size_t ei = 0; ei < mesh.getEdges().size(); /* No increment here */) {
                    const size_t hei0 = mesh.getEdges()[ei].halfEdgeIndex;
                    const size_t v0 = mesh.target(hei0);
                    const size_t v1 = mesh.target(mesh.opposite(hei0));

                    const auto c0 = vector2Vec<3, double>(mesh.getVertexAttribute(v0).getCoordinate());
                    const auto c1 = vector2Vec<3, double>(mesh.getVertexAttribute(v1).getCoordinate());
                    const double length2 = distance2(c0, c1);

                    const double eqLength = mesh.getEdgeAttribute(ei).aEdge.eqLength;
                    const double eqLength2 = eqLength * eqLength;

                    if(length2 >= 2 * eqLength2) { // Too long
                        sizeMeasureSatisfied = false;
                        if(_edgeSplitManager.trySplit(mesh, ei, _edgeFlipManager))
                            // Edge splitting happened. Will check edge ei again next round
                            ++countTopoModified;
                        else
                            ++ei;
                    } else if(2 * length2 <= eqLength2) { // Too short
                        sizeMeasureSatisfied = false;
                        if(_edgeCollapseManager.tryCollapse(mesh, ei))
                            // Edge collapsing happened. The edge at ei will be different next round
                            ++countTopoModified;
                        else
                            ++ei;
                    } else { // Check passed
                        ++ei;
                    }
                }
            } while(countTopoModified); // If any topology was modified, will loop through all edges again.

            if(sizeMeasureSatisfied) break;

            _globalRelaxationManager.relax(mesh, _edgeFlipManager);

            _computeSizeMeasures();
        } // End loop TopoModifying-Relaxation
    }

};


#endif
