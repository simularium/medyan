
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

void PolakRibiere::minimize(ForceFieldManager &FFM, double GRADTOL){

    
    //system size
    int n = BeadDB::instance()->size();
    int nd = 3 * n;
    if (nd == 0) return;
    
	double curEnergy = FFM.computeEnergy(0.0);
    double prevEnergy;
    
	FFM.computeForces();

    //compute first gradient
    double gSquare = gradSquare();
    
	int numIter = 0;
	do {
		numIter++;
		double lambda, beta, newGradSquare, conjSquare;
		vector<double> newGrad;
        
        //find lambda by line search, move beads
        lambda = quadraticLineSearch(FFM);
        moveBeads(lambda);
        
        //compute new forces
        FFM.computeForcesAux();
        
        //compute direction
        newGradSquare = gradAuxSquare();
        conjSquare = gradDotProduct();

        //choose beta, safeguard for blowups
		if (numIter % (5 * nd) == 0) beta = 0;
		else {
            if(gSquare == 0) beta = 0;
            else beta = max(0.0, (newGradSquare - conjSquare)/ gSquare);
        }
        
        //shift gradient
        if(conjSquare < 0) shiftGradient(0.0);
        else shiftGradient(beta);
        
		prevEnergy = curEnergy;
		curEnergy = FFM.computeEnergy(0.0);
        
		gSquare = newGradSquare;
	}
	while (gSquare > GRADTOL);
}
