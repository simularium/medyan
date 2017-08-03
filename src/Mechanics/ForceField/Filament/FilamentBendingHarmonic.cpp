
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

#include "FilamentBendingHarmonic.h"
#include "FilamentBending.h"

#include "Filament.h"
#include "Cylinder.h"
#include "Bead.h"

#include "MathFunctions.h"

using namespace mathfunc;

double FilamentBendingHarmonic::energy(double *coord, double *f, int *beadSet,
                                       double *kbend, double *eqt){

    int n = FilamentBending<FilamentBendingHarmonic>::n;
    int nint = n * (Bead::getBeads().size() - 2 * Filament::getFilaments().size());
    
    double *coord1, *coord2, *coord3, dist, U_i, L1, L2, L1L2, l1l2;
    
    double U = 0;
    
    for(int i = 0; i < nint; i += 1) {
        
        coord1 = &coord[3 * beadSet[n * i]];
        coord2 = &coord[3 * beadSet[n * i + 1]];
        coord3 = &coord[3 * beadSet[n * i + 2]];
        
        L1 = sqrt(scalarProduct(coord1, coord2,
                                coord1, coord2));
        L2 = sqrt(scalarProduct(coord2, coord3,
                                coord2, coord3));
        
        L1L2 = L1*L2;
        l1l2 = scalarProduct(coord1, coord2,
                             coord2, coord3);
        
        U_i = kbend[i] * ( 1 - l1l2 / L1L2 );
        
        if(fabs(U_i) == numeric_limits<double>::infinity()
           || U_i != U_i || U_i < -1.0) {
            
            //set culprit and return
            FilamentInteractions::_filamentCulprit = Filament::getFilaments()[i];
            
            return -1;
        }
        
        U += U_i;
    }
    
    return U;
}

double FilamentBendingHarmonic::energy(double *coord, double *f, int *beadSet,
                                       double *kbend, double *eqt, double d ){
    
    int n = FilamentBending<FilamentBendingHarmonic>::n;
    int nint = n * (Bead::getBeads().size() - 2 * Filament::getFilaments().size());
    
    double *coord1, *coord2, *coord3, *force1, *force2, *force3, dist, U_i, L1, L2, L1L2, l1l2;
    
    double U = 0;
    
    for(int i = 0; i < nint; i += 1) {
        
        coord1 = &coord[3 * beadSet[n * i]];
        coord2 = &coord[3 * beadSet[n * i + 1]];
        coord3 = &coord[3 * beadSet[n * i + 2]];
        
        force1 = &f[3 * beadSet[n * i]];
        force2 = &f[3 * beadSet[n * i + 1]];
        force3 = &f[3 * beadSet[n * i + 2]];
        
        
        L1 = sqrt(scalarProductStretched(coord1, force1, coord2, force2,
                                         coord1, force1, coord2, force2, d));
        L2 = sqrt(scalarProductStretched(coord2, force2, coord3, force3,
                                         coord2, force2, coord3, force3, d));
        
        L1L2 = L1*L2;
        l1l2 = scalarProductStretched(coord1, force1, coord2, force2,
                                      coord2, force2, coord3, force3, d);
        
        U_i = kbend[i] * ( 1 - l1l2 / L1L2);
        
        if(fabs(U_i) == numeric_limits<double>::infinity()
           || U_i != U_i || U_i < -1.0) {
            
            //set culprit and return
            FilamentInteractions::_filamentCulprit = Filament::getFilaments()[i];
            
            return -1;
        }
        
        U += U_i;
    }
    
    return U;
}

void FilamentBendingHarmonic::forces(double *coord, double *f, int *beadSet,
                                     double *kbend, double *eqt){
    
    int n = FilamentBending<FilamentBendingHarmonic>::n;
    int nint = n * (Bead::getBeads().size() - 2 * Filament::getFilaments().size());
    
    double *coord1, *coord2, *coord3, *force1, *force2, *force3, dist,
           L1, L2, l1l2, invL1, invL2, A,B,C, k;
    
    for(int i = 0; i < nint; i += 1) {
        
        coord1 = &coord[3 * beadSet[n * i]];
        coord2 = &coord[3 * beadSet[n * i + 1]];
        coord3 = &coord[3 * beadSet[n * i + 2]];
        
        force1 = &f[3 * beadSet[n * i]];
        force2 = &f[3 * beadSet[n * i + 1]];
        force3 = &f[3 * beadSet[n * i + 2]];
        
        L1 = sqrt(scalarProduct(coord1, coord2,
                                coord1, coord2));
        L2 = sqrt(scalarProduct(coord2, coord3,
                                coord2, coord3));
        
        l1l2 = scalarProduct(coord1, coord2,
                             coord2, coord3);
        
        invL1 = 1/L1;
        invL2 = 1/L2;
        A = invL1*invL2;
        B = l1l2*invL1*A*A*L2;
        C = l1l2*invL2*A*A*L1;
        
        k = kbend[i];
        
        //force on i-1, f = k*(-A*l2 + B*l1):
        force1[0] +=  k * ((-coord3[0] + coord2[0])*A +
                           (coord2[0] - coord1[0])*B );
        force1[1] +=  k * ((-coord3[1] + coord2[1])*A +
                           (coord2[1] - coord1[1])*B );
        force1[2] +=  k * ((-coord3[2] + coord2[2])*A +
                           (coord2[2] - coord1[2])*B );
        
        
        //force on i, f = k*(A*(l1-l2) - B*l1 + C*l2):
        force2[0] +=  k *( (coord3[0] - 2*coord2[0] + coord1[0])*A -
                          (coord2[0] - coord1[0])*B +
                          (coord3[0] - coord2[0])*C );
        
        force2[1] +=  k *( (coord3[1] - 2*coord2[1] + coord1[1])*A -
                          (coord2[1] - coord1[1])*B +
                          (coord3[1] - coord2[1])*C );
        
        force2[2] +=  k *( (coord3[2] - 2*coord2[2] + coord1[2])*A -
                          (coord2[2] - coord1[2])*B +
                          (coord3[2] - coord2[2])*C );
        
        //force on i-1, f = k*(A*l - B*l2):
        force3[0] +=  k *( (coord2[0] - coord1[0])*A -
                          (coord3[0] - coord2[0])*C );
        
        force3[1] +=  k *( (coord2[1] - coord1[1])*A -
                          (coord3[1] - coord2[1])*C );
        
        force3[2] +=  k *( (coord2[2] - coord1[2])*A -
                          (coord3[2] - coord2[2])*C );
        
    }
}
