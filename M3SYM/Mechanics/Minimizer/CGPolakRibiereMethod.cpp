
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

#include "CGPolakRibiereMethod.h"

#include "ForceFieldManager.h"
#include "Output.h"

void PolakRibiere::minimize(ForceFieldManager &FFM, double GRADTOL,
                                                    double MAXDIST,
                                                    double LAMBDAMAX){
    
    //system size
    int N = Bead::numBeads();
    
	double curEnergy = FFM.computeEnergy(0.0);
    
	FFM.computeForces();
    startMinimization();

    //compute first gradient
    double curGrad = CGMethod::allFDotF();
//    
//    cout << "Starting minimization" << endl;
//    cout << "Energy = " << curEnergy << endl;
//    cout << "MaxF = " << maxF() << endl;
    
	int numIter = 0;
    while (/* Iteration criterion */  numIter < N &&
           /* Gradient tolerance  */  maxF() > GRADTOL) {

		numIter++;
		double lambda, beta, newGrad, prevGrad;
        
        //find lambda by line search, move beads
        lambda = _safeMode ? safeBacktrackingLineSearch(FFM, MAXDIST, LAMBDAMAX)
                           : backtrackingLineSearch(FFM, MAXDIST, LAMBDAMAX);
        
        moveBeads(lambda); setBeads();
        
        //compute new forces
        FFM.computeForcesAux();
        
        //compute direction
        newGrad = CGMethod::allFADotFA();
        prevGrad = CGMethod::allFADotFAP();
        
        //Polak-Ribieri update
        beta = max(0.0, (newGrad - prevGrad) / curGrad);
        
        //update prev forces
        FFM.computeForcesAuxP();
        
        //shift gradient
        shiftGradient(beta);
        
        //direction reset if not downhill or no progress made
        if(CGMethod::allFDotFA() <= 0 || areSame(curGrad, newGrad)) {
            
            cout << "Safe mode enabled" << endl;
            
            shiftGradient(0.0);
            _safeMode = true;
        }
        
        curEnergy = FFM.computeEnergy(0.0);
        curGrad = newGrad;
    }    
    cout << "Ending minimization" << endl;
    cout << "Numiter = " << numIter << endl;
    cout << "NBeads = " << N << endl;
    cout << "Energy = " << curEnergy << endl;
    cout << "MaxF = " << maxF() << endl;
}
