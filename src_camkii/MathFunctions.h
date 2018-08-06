
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

#ifndef MEDYAN_MathFunctions_h
#define MEDYAN_MathFunctions_h

#include <cmath>
#include <vector>

#include "common.h"

/// @namespace mathfunc is used for the mathematics module for the entire codebase
/// mathfunc includes functions to calculate distances, products, and midpoints

namespace mathfunc {
    
    /// Normalize a vector
    inline void normalize(vector<double>& v) {
        
        double norm = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        
        v[0] /= norm; v[1] /= norm; v[2] /= norm;
    }
    
    /// Return normalized vector
    inline vector<double> normalizedVector(const vector<double>& v) {
        
        double norm = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        
        vector<double> v1;
        
        v1.push_back(v[0]/norm); v1.push_back(v[1]/norm); v1.push_back(v[2]/norm);
        
        return v1;
    }
    
    /// Get the magnitude of a vector
    inline double magnitude(const vector<double>& v) {
        
        return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    }
    
    /// Compute distance between two points with coordinates: (x1,y1,z1) and (x2,y2,z3)
    inline double twoPointDistance(const vector<double>& v1, const vector<double>& v2) {
        
        return sqrt((v2[0]-v1[0])*(v2[0]-v1[0]) +
                    (v2[1]-v1[1])*(v2[1]-v1[1]) +
                    (v2[2]-v1[2])*(v2[2]-v1[2]));
    }
    
    /// Compute distance between two points with coordinates
    /// (x1 -d*p1x,y1-d*p1y,z1-d*p1z) and (x2-d*p2x,y2-d*p2y,z2-d*p2z)
    inline double twoPointDistanceStretched(const vector<double>& v1,
                                            const vector<double>& p1,
                                            const vector<double>& v2,
                                            const vector<double>& p2, double d){
        
        return sqrt(((v2[0] + d*p2[0])-(v1[0] + d*p1[0])) *
                    ((v2[0] + d*p2[0])-(v1[0] + d*p1[0])) +
                    ((v2[1] + d*p2[1])-(v1[1] + d*p1[1])) *
                    ((v2[1] + d*p2[1])-(v1[1] + d*p1[1])) +
                    ((v2[2] + d*p2[2])-(v1[2] + d*p1[2])) *
                    ((v2[2] + d*p2[2])-(v1[2] + d*p1[2])));
    }
    
    /// Calculates a normal to a line starting at (x1,y1,z1) and ending at (x2,y2,z2)
    inline vector<double> twoPointDirection(const vector<double>& v1,
                                            const vector<double>& v2) {
        vector<double> tau (3, 0);
        double invD = 1/twoPointDistance(v1, v2);
        tau[0] = invD * ( v2[0] - v1[0] );
        tau[1] = invD * ( v2[1] - v1[1] );
        tau[2] = invD * ( v2[2] - v1[2] );
        return tau;
    }
    
    /// Scalar product of two vectors v1(x,y,z) and v2(x,y,z)
    inline double dotProduct(const vector<double>& v1, const vector<double>& v2) {
        return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
    }
    
    /// Scalar product of two vectors with coordinates: (x2-x1,y2-y1,z2-z1) and
    /// (x4-x3,y4-y3,z4-z3)
    inline double scalarProduct(const vector<double>& v1, const vector<double>& v2,
                                const vector<double>& v3, const vector<double>& v4) {
        
        return ((v2[0] - v1[0])*(v4[0] - v3[0]) +
                (v2[1] - v1[1])*(v4[1] - v3[1]) +
                (v2[2] - v1[2])*(v4[2] - v3[2]));
    }
    
    /// Scalar product of two vectors with coordinates: (x2-x1,y2-y1,z2-z1) and
    /// (x4-x3,y4-y3,z4-z3) but with x+d*p coordinates
    inline double scalarProductStretched(const vector<double>& v1,
                                         const vector<double>& p1,
                                         const vector<double>& v2,
                                         const vector<double>& p2,
                                         const vector<double>& v3,
                                         const vector<double>& p3,
                                         const vector<double>& v4,
                                         const vector<double>& p4,
                                         double d){
        
        double xx = ((v2[0] + d*p2[0]) - (v1[0] + d*p1[0]))*
                    ((v4[0] + d*p4[0]) - (v3[0] + d*p3[0]));
        double yy = ((v2[1] + d*p2[1]) - (v1[1] + d*p1[1]))*
                    ((v4[1] + d*p4[1]) - (v3[1] + d*p3[1]));
        double zz = ((v2[2] + d*p2[2]) - (v1[2] + d*p1[2]))*
                    ((v4[2] + d*p4[2]) - (v3[2] + d*p3[2]));
        return xx + yy + zz;
        
    }
    
    /// Scalar product of two vectors with coordinates: v1[z,y,z] + d*p1[x,y,z] and
    /// v2[x,y,z] + d*p2[x,y,z]
    inline double dotProductStretched(const vector<double>& v1,
                                      const vector<double>& p1,
                                      const vector<double>& v2,
                                      const vector<double>& p2,
                                      double d){
        
        double xx = (v1[0] + d*p1[0]) * (v2[0] + d*p2[0]);
        double yy = (v1[1] + d*p1[1]) * (v2[1] + d*p2[1]);
        double zz = (v1[2] + d*p1[2]) * (v2[2] + d*p2[2]);
        return xx + yy + zz;
        
    }
    
    
    /// Vector product of two vectors with coordinates: (x2-x1,y2-y1,z2-z1) and
    /// (x4-x3,y4-y3,z4-z3). Returns a 3d vector.
    inline vector<double> vectorProduct(const vector<double>& v1,
                                        const vector<double>& v2,
                                        const vector<double>& v3,
                                        const vector<double>& v4) {
        vector<double> v;
        
        double vx = (v2[1]-v1[1])*(v4[2]-v3[2]) - (v2[2]-v1[2])*(v4[1]-v3[1]);
        double vy = (v2[2]-v1[2])*(v4[0]-v3[0]) - (v2[0]-v1[0])*(v4[2]-v3[2]);
        double vz = (v2[0]-v1[0])*(v4[1]-v3[1]) - (v2[1]-v1[1])*(v4[0]-v3[0]);
        
        v.push_back(vx);
        v.push_back(vy);
        v.push_back(vz);
        
        return v;
    };
    
    
    /// Vector product of two vectors with coordinates: (x2-x1,y2-y1,z2-z1) and
    /// (x4-x3,y4-y3,z4-z3), but with v -> v+d*p. Returns a 3d vector.
    inline vector<double> vectorProductStretched (const vector<double>& v1,
                                                  const vector<double>& p1,
                                                  const vector<double>& v2,
                                                  const vector<double>& p2,
                                                  const vector<double>& v3,
                                                  const vector<double>& p3,
                                                  const vector<double>& v4,
                                                  const vector<double>& p4,
                                                  double d){
        vector<double> v;
        
        double vx =
        ((v2[1]+d*p2[1])-(v1[1]+d*p1[1]))*((v4[2]+d*p4[2])-(v3[2]+d*p3[2]))
         - ((v2[2]+d*p2[2])-(v1[2]+d*p1[2]))*((v4[1]+d*p4[1])-(v3[1]+d*p3[1]));
        
        double vy =
        ((v2[2]+d*p2[2])-(v1[2]+d*p1[2]))*((v4[0]+d*p4[0])-(v3[0]+d*p3[0]))
        - ((v2[0]+d*p2[0])-(v1[0]+d*p1[0]))*((v4[2]+d*p4[2])-(v3[2]+d*p3[2]));
        
        double vz =
        ((v2[0]+d*p2[0])-(v1[0]+d*p1[0]))*((v4[1]+d*p4[1])-(v3[1]+d*p3[1]))
        - ((v2[1]+d*p2[1])-(v1[1]+d*p1[1]))*((v4[0]+d*p4[0])-(v3[0]+d*p3[0]));
        
        v.push_back(vx);
        v.push_back(vy);
        v.push_back(vz);
        
        return v;
        
        
    };
    
    /// Vector product of two vectors v1[x,y,z] and v2[x,y,z]. Returns a 3d vector.
    inline vector<double> crossProduct(const vector<double>& v1,
                                       const vector<double>& v2) {
        vector<double> v;
        
        double vx = v1[1]*v2[2] - v1[2]*v2[1];
        double vy = v1[2]*v2[0] - v1[0]*v2[2];
        double vz = v1[0]*v2[1] - v1[1]*v2[0];
        
        v.push_back(vx);
        v.push_back(vy);
        v.push_back(vz);
        
        return v;
    };
    
    /// Vector product of two vectors v1[x,y,z] and v2[x,y,z]. Returns a 3d vector.
    inline vector<double> crossProductStretched(const vector<double>& v1,
                                                const vector<double>& p1,
                                                const vector<double>& v2,
                                                const vector<double>& p2,
                                                double d) {
        vector<double> v;
        
        double vx = (v1[1]+d*p1[1])*(v2[2]+d*p2[2]) - (v1[2]+d*p1[2])*(v2[1]+d*p2[1]);
        double vy = (v1[2]+d*p1[2])*(v2[0]+d*p2[0]) - (v1[0]+d*p1[0])*(v2[2]+d*p2[2]);
        double vz = (v1[0]+d*p1[0])*(v2[1]+d*p2[1]) - (v1[1]+d*p1[1])*(v2[0]+d*p2[0]);
        
        v.push_back(vx);
        v.push_back(vy);
        v.push_back(vz);
        
        return v;
    };
    
    /// Projection of a new point based on a given direction and starting point
    inline vector<double> nextPointProjection(const vector<double>& coordinate,
                                              double d, const vector<double>& tau) {
        vector<double> v;
        v.push_back(coordinate[0] + d * tau[0]);
        v.push_back(coordinate[1] + d * tau[1]);
        v.push_back(coordinate[2] + d * tau[2]);
        return v;
    }
    
    /// Returns coordinates of a point v located on a line between v1 and v2.
    /// |v-v1|/|v2-v1| = alpha.
    inline vector<double> midPointCoordinate(const vector<double>& v1,
                                             const vector<double>& v2, double alpha) {
        vector<double> v;
        v.push_back(v1[0]*(1.0 - alpha) + alpha*v2[0]);
        v.push_back(v1[1]*(1.0 - alpha) + alpha*v2[1]);
        v.push_back(v1[2]*(1.0 - alpha) + alpha*v2[2]);
        return v;
    }
    
    
    /// Returns coordinates of a point v located on a line between v1 and v2.
    /// |v-v1|/|v2-v| = alpha, but with x-d*p coordinates
    inline vector<double> midPointCoordinateStretched(const vector<double>& v1,
                                                      const vector<double>& p1,
                                                      const vector<double>& v2,
                                                      const vector<double>& p2,
                                                      double alpha , double d) {
        
        vector<double> v;
        v.push_back((v1[0] + d*p1[0])*(1.0 - alpha) + alpha*(v2[0] + d*p2[0]));
        v.push_back((v1[1] + d*p1[1])*(1.0 - alpha) + alpha*(v2[1] + d*p2[1]));
        v.push_back((v1[2] + d*p1[2])*(1.0 - alpha) + alpha*(v2[2] + d*p2[2]));
        return v;
    }
    
    /// Returns true if two vectors (p1->p2 and p3->p4) are parallel
    inline bool areParallel(const vector<double>& p1, const vector<double>& p2,
                            const vector<double>& p3, const vector<double>& p4) {
        
        auto v1 = {p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]};
        auto v2 = {p4[0] - p3[0], p4[1] - p3[1], p4[2] - p3[2]};

        return areEqual(magnitude(crossProduct(v1,v2)), 0.0);
    }
    
    /// Returns true if two vectors (p1->p2 and p3->p4) are in the same plane
    inline bool areInPlane(const vector<double>& p1, const vector<double>& p2,
                           const vector<double>& p3, const vector<double>& p4) {
        
        auto v1 = {p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]};
        auto v2 = {p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2]};
        auto v3 = {p4[0] - p1[0], p4[1] - p1[1], p4[2] - p1[2]};
        
        auto cp = crossProduct(v1, v2);
        
        return areEqual(dotProduct(v3, cp), 0.0);
    }
    
    /// Function to move bead out of plane by specified amount
    vector<double> movePointOutOfPlane(const vector<double>& p1,
                                       const vector<double>& p2,
                                       const vector<double>& p3,
                                       const vector<double>& p4,
                                       int i, double d);
    
    
    /// Function to create a initial camkiiing point and direction, given an
    /// initial normal vector and point.
    /// @param l - the distance of the camkii from the original point
    /// @param m - the size of the camkii projection
    /// @param theta - the angle of camkiiing
    /// @return a vector describing the initial camkiiing direction and point
    tuple<vector<double>, vector<double>> camkiiProjection(const vector<double>& n,
                                                           const vector<double>& p,
                                                           double l, double m, double theta);
    
    
    }

#endif