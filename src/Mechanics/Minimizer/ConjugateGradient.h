
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v3.0
//
//  Copyright (2015)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#ifndef MEDYAN_ConjugateGradient_h
#define MEDYAN_ConjugateGradient_h

#include "common.h"

#include "CGFletcherRievesMethod.h"
#include "CGPolakRibiereMethod.h"
#include "CGSteepestDescent.h"

//FORWARD DECLARATIONS
class ForceFieldManager;

/// An implementation of [Minimzer](@ref Minimizer).
template <class CGType>
class ConjugateGradient : public Minimizer {
    
private:
    CGType _CGType;  ///< Implementation of a CG method
    
    double _GRADTOL;   ///< Gradient tolerance used
    double _MAXDIST;   ///< Max distance used to move
    double _LAMBDAMAX; ///< Maximum lambda that can be returned
    
public:
    /// Constructor sets gradient tolerance parameter
    ConjugateGradient(double gradientTolerance,
                      double maxDistance,
                      double lambdaMax)
    
        : _GRADTOL(gradientTolerance),
          _MAXDIST(maxDistance),
          _LAMBDAMAX(lambdaMax) {}
    
    
    /// This function will minimize the system until the following criterion are met:
    /// 1) Largest force in the network < GRADTOL
    /// 3) Number of iterations exceeds 5N, unless in initial minimization
    void equlibrate(ForceFieldManager &FFM, bool steplimit) {
        _CGType.minimize(FFM, _GRADTOL, _MAXDIST, _LAMBDAMAX, steplimit);
    }
};

#endif