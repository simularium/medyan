
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

#ifndef MEDYAN_CGPolakRibiereMethod_h
#define MEDYAN_CGPolakRibiereMethod_h

#include <cmath>
#include <numeric>
#include <algorithm>

#include "common.h"

#include "CGMethod.h"

/// The Polak-Ribiere method for conjugate gradient minimization
class PolakRibiere : public CGMethod
{
public:
    
    virtual void minimize(ForceFieldManager &FFM, double GRADTOL,
                          double MAXDIST, double LAMBDAMAX, bool steplimit);
protected:
    cudaStream_t stream_shiftsafe, stream_dotcopy;
    cudaEvent_t  event_safe, event_dot;
};
#endif

