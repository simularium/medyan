
//------------------------------------------------------------------
//  **M3SYM** - Simulation Package for the Mechanochemical
//              Dynamics of Active Networks, 3rd Generation
//
//  Copyright (2014) Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the Papoian lab page for more information:
//  http://papoian.chem.umd.edu/
//------------------------------------------------------------------

#ifndef M3SYM_CGMethod_h
#define M3SYM_CGMethod_h

#include <cmath>

#include "common.h"

//FORWARD DECLARATIONS
class ForceFieldManager;

/// For performing a conjugate gradient minimization method

/*!
 *  CGMethod is an abstract class that contains methods for conjugate gradient 
 *  minimization. It has functions for various line search techniques, including golden 
 *  section, backtracking, and quadratic line search. This base class also contains 
 *  parameters for tolerance criterion and other helpful parameters.
 */
class CGMethod {

protected:
    
    //@{
    /// Lambda parameter for use in linear search methods
    const double LAMBDAMAX = 1.0;  ///< Max lambda that can be returned,
                                   ///< used in all methods
    const double MAXDIST = 0.01;   ///< Max distance parameter,
                                   ///< used only in backtracking line search
    //@}
    
    //@{
    /// Parameter used in backtracking line search
    const double LAMBDAREDUCE = 0.5;   ///< Lambda reduction parameter for backtracking
    const double BACKTRACKSLOPE = 0.1; ///< Backtrack slope parameter
    //@}
    
    //@{
    /// Parameter used in quadratic line search
    const double QUADRATICTOL = 0.1;
    const double EPS_QUAD = 1e-28;
    //@}
    
    //@{
    /// Parameter used in golden section
    const double PHI = (1 + sqrt(5)) / 2;
    const double R = 0.61803399;
    const double C = 1 - R;
    //@}
    
    const double LSENERGYTOL = 1e-8; ///< Line search energy tolerance for all
                                     ///< linesearch methods
    
    //@{
    /// For use in minimization
    double allFDotF();
    double allFADotFA();
    double allFDotFA();
    
    void moveBeads(double d);
    void moveBeadsAux(double d);
    void shiftGradient(double d);
    //@}
    
    //@{
    /// Linear search method
    double goldenSection(ForceFieldManager &FFM);
    double binarySearch(ForceFieldManager& FFM);
    
    ///@note - the most robust linesearch method, but slow at times.
    double backtrackingLineSearch(ForceFieldManager& FFM);
    
    
    ///@note - this is somewhat unstable for most systems. Backtracking
    ///        line search will be the most robust method.
    double quadraticLineSearch(ForceFieldManager& FFM);
    //@}
    
    void printForces();
    
public:
    
    virtual ~CGMethod() {};
    
    /// Minimize the system
    virtual void minimize(ForceFieldManager &FFM, double GRADTOL) = 0;
};


#endif
