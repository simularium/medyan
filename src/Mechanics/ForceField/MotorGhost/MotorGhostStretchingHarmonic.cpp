
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v3.2.1
//
//  Copyright (2015-2018)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#include "MotorGhostStretchingHarmonic.h"
#include "MotorGhostStretching.h"
#include "MotorGhost.h"
#include "MotorGhostStretchingHarmonicCUDA.h"
#include "Bead.h"
#include "MathFunctions.h"
#include "common.h"
#ifdef CUDAACCL
#include <cuda.h>
#include <cuda_runtime.h>
#include "nvToolsExt.h"
#endif

using namespace mathfunc;
#ifdef CUDAACCL
void MotorGhostStretchingHarmonic::deallocate(){
//    if(!(CUDAcommon::getCUDAvars().conservestreams))
//        CUDAcommon::handleerror(cudaStreamDestroy(stream));
    CUDAcommon::handleerror(cudaFree(gU_i));
    CUDAcommon::handleerror(cudaFree(gU_sum));
    CUDAcommon::handleerror(cudaFree(gFF));
    CUDAcommon::handleerror(cudaFree(ginteraction));
}
void MotorGhostStretchingHarmonic::checkforculprit() {
    CUDAcommon::printculprit("MotorGhostStretching","MotorGhostStretchingHarmonic");
    MotorGhost* m;
    m = MotorGhost::getMotorGhosts()[CUDAcommon::getCUDAvars().culpritID[0]];
    cout<<"Printing culprit Motor information."<<endl;
    m->printSelf();
    exit(EXIT_FAILURE);
}
void MotorGhostStretchingHarmonic::optimalblocksnthreads( int nint, cudaStream_t
stream_pass){
//    //CUDA stream create
//    if(stream == NULL || !(CUDAcommon::getCUDAvars().conservestreams))
//        CUDAcommon::handleerror(cudaStreamCreate(&stream));
    stream = stream_pass;
    blocksnthreadse.clear();
    blocksnthreadsez.clear();
    blocksnthreadsf.clear();
    int blockSize;   // The launch configurator returned block size
    int minGridSize; // The minimum grid size needed to achieve the
    // maximum occupancy for a full device launch
//    int gridSize;    // The actual grid size needed, based on input size
//    unaryfn::argument_type blksize;
//    unaryfn::result_type result;
//    unaryfn ufn;
    if(nint>0) {
        cudaOccupancyMaxPotentialBlockSizeVariableSMem(&minGridSize, &blockSize,
                                                       MotorGhostStretchingHarmonicenergy, blockToSmem, 0);
//    std::cout<<(nint +blockSize -1) / blockSize<<" "<<blockSize<<endl;
//
//    cudaOccupancyMaxPotentialBlockSize( &minGridSize, &blockSize,
//                                        CUDAExclVolRepulsionenergy, 0, 0);
        blocksnthreadse.push_back((nint + blockSize - 1) / blockSize);
        blocksnthreadse.push_back(blockSize);
//    std::cout<<(nint +blockSize -1) / blockSize<<" "<<blockSize<<endl;
        blockSize = 0;

        cudaOccupancyMaxPotentialBlockSizeVariableSMem(&minGridSize, &blockSize,
//                                                       MotorGhostStretchingHarmonicenergyz, blockToSmemez, 0);
                                                       MotorGhostStretchingHarmonicenergyz, blockToSmemZero, 0);
        blocksnthreadsez.push_back((nint + blockSize - 1) / blockSize);
        blocksnthreadsez.push_back(blockSize);
        blockSize = 0;

        cudaOccupancyMaxPotentialBlockSizeVariableSMem(&minGridSize, &blockSize,
                                                       MotorGhostStretchingHarmonicforces, blockToSmemZero, 0);
//                                                       MotorGhostStretchingHarmonicforces, blockToSmem, 0);
        blocksnthreadsf.push_back((nint + blockSize - 1) / blockSize);
        blocksnthreadsf.push_back(blockSize);
        //get addition vars
        bntaddvec2.clear();
        bntaddvec2 = getaddred2bnt(nint);
        CUDAcommon::handleerror(cudaMalloc((void **) &gU_i, bntaddvec2.at(0)*sizeof(double)));
        CUDAcommon::handleerror(cudaMemsetAsync(gU_i, 0, bntaddvec2.at(0) * sizeof
                                                                                    (double), stream));
	    CUDAcommon::handleerror(cudaMalloc((void **) &gU_sum, sizeof(double)));
        char a[] = "MotorGhostFF";
        char b[] = "MotorGhost Stretching Harmonic";
        CUDAcommon::handleerror(cudaMalloc((void **) &gFF, 100 * sizeof(char)));
        CUDAcommon::handleerror(cudaMalloc((void **) &ginteraction, 100 * sizeof(char)));
        CUDAcommon::handleerror(cudaMemcpy(gFF, a, 100 * sizeof(char), cudaMemcpyHostToDevice));
        CUDAcommon::handleerror(cudaMemcpy(ginteraction, b, 100 * sizeof(char), cudaMemcpyHostToDevice));

    }
    else{
        blocksnthreadse.push_back(0);
        blocksnthreadse.push_back(0);
        blocksnthreadsez.push_back(0);
        blocksnthreadsez.push_back(0);
        blocksnthreadsf.push_back(0);
        blocksnthreadsf.push_back(0);
    }

}
double* MotorGhostStretchingHarmonic::energy(double *coord, double *f, int *beadSet,
                                            double *kstr, double *eql, double *pos1, double *pos2,
                                            int *params) {
//    if(blocksnthreadse[1]>0) {
//
//    MotorGhostStretchingHarmonicenergy<<<blocksnthreadse[0], blocksnthreadse[1], (12 * blocksnthreadse[1]) * sizeof
//                                                                                                                  (double), stream>>>
//            (coord, f, beadSet, kstr, eql, pos1, pos2, params, gU_i, CUDAcommon::getCUDAvars().gculpritID,
//            CUDAcommon::getCUDAvars().gculpritFF,
//            CUDAcommon::getCUDAvars().gculpritinteraction, gFF, ginteraction);
//        auto cvars = CUDAcommon::getCUDAvars();
//        cvars.streamvec.push_back(&stream);
//        CUDAcommon::cudavars = cvars;
////        cudaEventRecord(event, stream);

//                CUDAcommon::handleerror( cudaGetLastError(), "MotorGhostStretchingHarmonicenergy",
//                                         "MotorGhostStretchingHarmonic.cu");

//        double* gpu_Utot = CUDAcommon::getCUDAvars().gpu_energy;
//        addvector<<<1,1,0,stream>>>(gU_i,params, gU_sum, gpu_Utot);

//        CUDAcommon::handleerror( cudaGetLastError() , "MotorGhostStretchingHarmonicenergy",
//                                 "MotorGhostStretchingHarmonic.cu");

//
//    return gU_sum;}
//    else
//        return NULL;
}


double* MotorGhostStretchingHarmonic::energy(double *coord, double *f, int *beadSet,
                                            double *kstr, double *eql, double *pos1, double *pos2, double *z,
                                            int *params) {
//    if(blocksnthreadse[1]>0) {
//        MotorGhostStretchingHarmonicenergy<<<blocksnthreadse[0], blocksnthreadse[1], (12 * blocksnthreadse[1]) * sizeof
//                (double), stream>>>
//                          (coord, f, beadSet, kstr, eql, pos1, pos2, params, gU_i, z,
//                                  CUDAcommon::getCUDAvars().gculpritID,
//                                  CUDAcommon::getCUDAvars().gculpritFF,
//                                  CUDAcommon::getCUDAvars().gculpritinteraction, gFF, ginteraction);
//        CUDAcommon::handleerror( cudaGetLastError(), "MotorGhostStretchingHarmonicenergy",
//                                 "MotorGhostStretchingHarmonic.cu");
//    }

    if(blocksnthreadsez[1]>0) {
        auto boolvarvec = CUDAcommon::cudavars.backtrackbools;
//        MotorGhostStretchingHarmonicenergyz << < blocksnthreadsez[0], blocksnthreadsez[1], (24 * blocksnthreadsez[1]) *
        MotorGhostStretchingHarmonicenergyz << < blocksnthreadsez[0], blocksnthreadsez[1], (0) *
                sizeof(double), stream>> > (coord, f, beadSet, kstr, eql, pos1, pos2, params, gU_i, CUDAcommon::cudavars.gpu_energyvec, z,
                 CUDAcommon::getCUDAvars().gculpritID,
                 CUDAcommon::getCUDAvars().gculpritFF,
                 CUDAcommon::getCUDAvars().gculpritinteraction, gFF, ginteraction, boolvarvec.at(0),
                boolvarvec.at(1) );
    }
    if(blocksnthreadse[1]<=0 && blocksnthreadsez[1]<=0)
        return NULL;
    else{
        auto cvars = CUDAcommon::getCUDAvars();
        cvars.streamvec.push_back(&stream);
        CUDAcommon::cudavars = cvars;
#ifdef CUDA_INDIVIDUAL_ESUM
        double* gpu_Utot = CUDAcommon::getCUDAvars().gpu_energy;
        resetdoublevariableCUDA<<<1,1,0,stream>>>(gU_sum);
        addvectorred2<<<bntaddvec2.at(2),bntaddvec2.at(3), bntaddvec2.at(3) * sizeof(double),stream>>>(gU_i,
                params, gU_sum, gpu_Utot);
#endif
        CUDAcommon::handleerror( cudaGetLastError() , "MotorGhostStretchingHarmonicenergy",
                                 "MotorGhostStretchingHarmonic.cu");
        return gU_sum;
    }
}

void MotorGhostStretchingHarmonic::forces(double *coord, double *f, int *beadSet,
                                          double *kstr, double *eql, double *pos1, double
                                          *pos2, int *params, double *Mstretchforce){
    if(blocksnthreadsf[1]>0) {

        //TODO  since the number of threads needed is constant through out the minimization, consider storing the pointer.
//        CUDAcommon::handleerror(cudaMalloc((void **) &gU_i, 36 * blocksnthreads[0] * blocksnthreads[1] * sizeof
//                                                                                                                (double)));


//        double F_c[3*Bead::getBeads().size()];
//        double C_c[3*Bead::getBeads().size()];
//        //TODO remove this later need not copy forces back to CPU.
//        CUDAcommon::handleerror(cudaMemcpy(F_c, f, 3 * Bead::getBeads().size() *sizeof(double),
//                                           cudaMemcpyDeviceToHost));
//        CUDAcommon::handleerror(cudaMemcpy(C_c, coord, 3 * Bead::getBeads().size() *sizeof(double),
//                                           cudaMemcpyDeviceToHost));
//        for(int iter=0;iter<Bead::getBeads().size();iter++) {
//            std::cout << C_c[3 * iter] << " " << C_c[3 * iter + 1] << " " << C_c[3 * iter + 2]<<" "<<F_c[3 * iter] <<
//            " " << F_c[3 * iter + 1] << " " << F_c[3 * iter + 2] <<endl;
//        }
//
//        std::cout<<"check ends "<<blocksnthreads[0]<<" "<<blocksnthreads[1]<<endl;

//        MotorGhostStretchingHarmonicforces << < blocksnthreadsf[0], blocksnthreadsf[1], (12 *
        MotorGhostStretchingHarmonicforces << < blocksnthreadsf[0], blocksnthreadsf[1], (0 *
        blocksnthreadsf[1]) * sizeof (double), stream >> > (coord, f, beadSet, kstr, eql,
                pos1, pos2, params, Mstretchforce);
        auto cvars = CUDAcommon::getCUDAvars();
        cvars.streamvec.push_back(&stream);
        CUDAcommon::cudavars = cvars;
        //CUDAcommon::handleerror(cudaDeviceSynchronize());
        CUDAcommon::handleerror(cudaGetLastError(), "MotorGhostStretchingHarmonicforces",
                                "MotorGhostStretchingHarmonic.cu");
    }
//    CUDAcommon::handleerror(cudaEventRecord( stop, 0));
//    CUDAcommon::handleerror(cudaEventSynchronize(stop));
//    float elapsedtime;
//    CUDAcommon::handleerror(cudaEventElapsedTime(&elapsedtime, start, stop));
//    CUDAvars cvars=CUDAcommon::getCUDAvars();
//    cvars.Ccforce += elapsedtime;
//    std::cout<<"C CFM "<<elapsedtime<<endl;
//    CUDAcommon::cudavars=cvars;
//    CUDAcommon::handleerror(cudaEventDestroy(start));
//    CUDAcommon::handleerror(cudaEventDestroy(stop));
}

#endif
double MotorGhostStretchingHarmonic::energy(double *coord, double *f, int *beadSet,
                                            double *kstr, double *eql, double *pos1, double *pos2) {

    int n = MotorGhostStretching<MotorGhostStretchingHarmonic>::n;
    int nint = MotorGhost::getMotorGhosts().size();

    double *coord1, *coord2, *coord3, *coord4, dist, U_i;
    double *v1 = new double[3];
    double *v2 = new double[3];

    double U = 0.0;


    for(int i = 0; i < nint; i += 1) {

        coord1 = &coord[3 * beadSet[n * i]];
        coord2 = &coord[3 * beadSet[n * i + 1]];
        coord3 = &coord[3 * beadSet[n * i + 2]];
        coord4 = &coord[3 * beadSet[n * i + 3]];

        midPointCoordinate(v1, coord1, coord2, pos1[i]);
        midPointCoordinate(v2, coord3, coord4, pos2[i]);

        dist = twoPointDistance(v1, v2) - eql[i];
        U_i = 0.5 * kstr[i] * dist * dist;

        if(fabs(U_i) == numeric_limits<double>::infinity()
           || U_i != U_i || U_i < -1.0) {

            //set culprit and return
            MotorGhostInteractions::_motorCulprit = MotorGhost::getMotorGhosts()[i];

            return -1;
        }

        U += U_i;
//        std::cout<<U_i<<endl;
    }
//    std::cout<<"MS Total energy serial "<< U <<endl;
    delete [] v1;
    delete [] v2;

    return U;
}

double MotorGhostStretchingHarmonic::energy(double *coord, double * f, int *beadSet,
                                            double *kstr, double *eql, double *pos1, double *pos2, double d){

    int n = MotorGhostStretching<MotorGhostStretchingHarmonic>::n;
    int nint = MotorGhost::getMotorGhosts().size();

    double *coord1, *coord2, *coord3, *coord4, *f1, *f2, *f3, *f4, dist, U_i;
    double *v1 = new double[3];
    double *v2 = new double[3];

    double U = 0.0;

    for(int i = 0; i < nint; i += 1) {

        coord1 = &coord[3 * beadSet[n * i]];
        coord2 = &coord[3 * beadSet[n * i + 1]];
        coord3 = &coord[3 * beadSet[n * i + 2]];
        coord4 = &coord[3 * beadSet[n * i + 3]];

        f1 = &f[3 * beadSet[n * i]];
        f2 = &f[3 * beadSet[n * i + 1]];
        f3 = &f[3 * beadSet[n * i + 2]];
        f4 = &f[3 * beadSet[n * i + 3]];

        midPointCoordinateStretched(v1, coord1, f1, coord2, f2, pos1[i], d);
        midPointCoordinateStretched(v2, coord3, f3, coord4, f4, pos2[i], d);

        dist = twoPointDistance(v1,  v2) - eql[i];
        U_i = 0.5 * kstr[i] * dist * dist;

//        std::cout<<v1[0]<<" "<<v1[1]<<" "<<v1[2]<<" "<<v2[0]<<" "<<v2[1]<<" "<<v2[2]<<" "<<coord1[0]<<" "
//                ""<<coord1[1]<<" "<<coord1[2]<<" "<<coord3[0]<<" "<<coord3[1]<<" "<<coord3[2]<<" "<<f1[0]<<" "
//                ""<<f1[1]<<" "<<f1[2]<<" "<<f3[0]<<" "<<f3[1]<<" "<<f3[2]<<" "<<coord2[0]<<" "<<coord2[1]<<" "
//                ""<<coord2[2]<<" "<<coord4[0]<<" "<<coord4[1]<<" "<<coord4[2]<<" "<<f2[0]<<" "<<f2[1]<<" "<<f2[2]<<" "
//                ""<<f4[0]<<" "<<f4[1]<<" "<<f4[2]<<" "<<pos1[i]<<" "<<pos2[i]<<" "<<d<<" "<<U_i<<endl;
        if(fabs(U_i) == numeric_limits<double>::infinity()
           || U_i != U_i || U_i < -1.0) {

            //set culprit and return
            MotorGhostInteractions::_motorCulprit = MotorGhost::getMotorGhosts()[i];

            return -1;
        }

        U += U_i;
    }

//    std::cout<<"MS Total energy serial "<< U <<endl;
    delete [] v1;
    delete [] v2;

    return U;

}

void MotorGhostStretchingHarmonic::forces(double *coord, double *f, int *beadSet,
                                          double *kstr, double *eql, double *pos1, double *pos2
                                            , double *stretchforce){

    int n = MotorGhostStretching<MotorGhostStretchingHarmonic>::n;
    int nint = MotorGhost::getMotorGhosts().size();

    double *coord1, *coord2, *coord3, *coord4, dist, invL;
    double *v1 = new double[3];
    double *v2 = new double[3];

    double f0, *f1, *f2, *f3, *f4;

    for(int i = 0; i < nint; i += 1) {

        coord1 = &coord[3 * beadSet[n * i]];
        coord2 = &coord[3 * beadSet[n * i + 1]];
        coord3 = &coord[3 * beadSet[n * i + 2]];
        coord4 = &coord[3 * beadSet[n * i + 3]];

        midPointCoordinate(v1, coord1, coord2, pos1[i]);
        midPointCoordinate(v2, coord3, coord4, pos2[i]);


        dist = twoPointDistance(v1, v2) ;
        invL = 1 / dist;

        f0 = kstr[i] * ( dist - eql[i] ) * invL;

        f1 = &f[3 * beadSet[n * i]];
        f2 = &f[3 * beadSet[n * i + 1]];
        f3 = &f[3 * beadSet[n * i + 2]];
        f4 = &f[3 * beadSet[n * i + 3]];

//        std::cout<<coord1[0]<<" "<<coord1[1]<<" "<<coord1[2]<<" "<<coord2[0]<<" "<<coord2[1]<<" "<<coord2[2]<<" "
//                ""<<coord3[0]<<" "<<coord3[1]<<" "<<coord3[2]<<" "<<coord4[0]<<" "<<coord4[1]<<" "<<coord4[2]<<" ";
//        std::cout<<-f0 * ( v1[0] - v2[0] ) * (1 - pos1[i])<<" "
//                 <<-f0 * ( v1[1] - v2[1] ) * (1 - pos1[i])<<" "
//                 <<-f0 * ( v1[2] - v2[2] ) * (1 - pos1[i])<<" "
//                 <<-f0 * ( v1[0] - v2[0] ) * (pos1[i])<<" "
//                 <<-f0 * ( v1[1] - v2[1] ) * (pos1[i])<<" "
//                 <<-f0 * ( v1[2] - v2[2] ) * (pos1[i])<<" "
//                 <<f0 * ( v1[0] - v2[0] ) * (1 - pos2[i])<<" "
//                 <<f0 * ( v1[1] - v2[1] ) * (1 - pos2[i])<<" "
//                 <<f0 * ( v1[2] - v2[2] ) * (1 - pos2[i])<<" "
//                 <<f0 * ( v1[0] - v2[0] ) * (pos2[i])<<" "
//                 <<f0 * ( v1[1] - v2[1] ) * (pos2[i])<<" "
//                 <<f0 * ( v1[2] - v2[2] ) * (pos2[i])<<" ";
//        std::cout<<f1[0]<<" "<<f1[1]<<" "<<f1[2]<<" "<<f2[0]<<" "<<f2[1]<<" "<<f2[2]<<" "<<f3[0]<<" "
//                ""<<f3[1]<<" "<<f3[2]<<" "<<f4[0]<<" "<<f4[1]<<" "<<f4[2]<<endl;

        //force on i
        f1[0] +=   -f0 * ( v1[0] - v2[0] ) * (1 - pos1[i]);
        f1[1] +=   -f0 * ( v1[1] - v2[1] ) * (1 - pos1[i]);
        f1[2] +=   -f0 * ( v1[2] - v2[2] ) * (1 - pos1[i]);

        // force i+1
        f2[0] +=   -f0 * ( v1[0] - v2[0] ) * (pos1[i]);
        f2[1] +=   -f0 * ( v1[1] - v2[1] ) * (pos1[i]);
        f2[2] +=   -f0 * ( v1[2] - v2[2] ) * (pos1[i]);

        //force on j
        f3[0] +=   f0 * ( v1[0] - v2[0] ) * (1 - pos2[i]);
        f3[1] +=   f0 * ( v1[1] - v2[1] ) * (1 - pos2[i]);
        f3[2] +=   f0 * ( v1[2] - v2[2] ) * (1 - pos2[i]);

        // force j+1
        f4[0] +=   f0 * ( v1[0] - v2[0] ) * (pos2[i]);
        f4[1] +=   f0 * ( v1[1] - v2[1] ) * (pos2[i]);
        f4[2] +=   f0 * ( v1[2] - v2[2] ) * (pos2[i]);
        //asign stretching force
        stretchforce[i] = f0/invL;
//        MotorGhost::getMotorGhosts()[i]->getMMotorGhost()->stretchForce = f0;
    }
    delete [] v1;
    delete [] v2;

}
