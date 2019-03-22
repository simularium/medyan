
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

#include "LinkerStretchingHarmonic.h"
#include "LinkerStretchingHarmonicCUDA.h"
#include "LinkerStretching.h"
#include "Linker.h"

#include "Bead.h"

#include "MathFunctions.h"
#ifdef CUDAACCL
#include <cuda.h>
#include <cuda_runtime.h>
#include "nvToolsExt.h"
#endif

using namespace mathfunc;
#ifdef CUDAACCL
void LinkerStretchingHarmonic::deallocate(){
//    if(!(CUDAcommon::getCUDAvars().conservestreams))
//        CUDAcommon::handleerror(cudaStreamDestroy(stream));
    CUDAcommon::handleerror(cudaFree(gU_i));
    CUDAcommon::handleerror(cudaFree(gU_sum));
    CUDAcommon::handleerror(cudaFree(gFF));
    CUDAcommon::handleerror(cudaFree(ginteraction));
}
void LinkerStretchingHarmonic::optimalblocksnthreads( int nint, cudaStream_t stream_pass){
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
        numint = nint;
        cudaOccupancyMaxPotentialBlockSizeVariableSMem(&minGridSize, &blockSize,
                                                       LinkerStretchingHarmonicenergy, blockToSmem, 0);
        blocksnthreadse.push_back((nint + blockSize - 1) / blockSize);
        blocksnthreadse.push_back(blockSize);
        blockSize = 0;

        cudaOccupancyMaxPotentialBlockSizeVariableSMem(&minGridSize, &blockSize,
//                                                       LinkerStretchingHarmonicenergyz, blockToSmemez, 0);
                                                       LinkerStretchingHarmonicenergyz, blockToSmemZero, 0);
        blocksnthreadsez.push_back((nint + blockSize - 1) / blockSize);
        blocksnthreadsez.push_back(blockSize);
        blockSize = 0;

        cudaOccupancyMaxPotentialBlockSizeVariableSMem(&minGridSize, &blockSize,
//                                                       LinkerStretchingHarmonicforces, blockToSmem, 0);
                                                       LinkerStretchingHarmonicforces, blockToSmemZero, 0);
        blocksnthreadsf.push_back((nint + blockSize - 1) / blockSize);
        blocksnthreadsf.push_back(blockSize);
        //get addition vars
        bntaddvec2.clear();
        bntaddvec2 = getaddred2bnt(nint);
        CUDAcommon::handleerror(cudaMalloc((void **) &gU_i, bntaddvec2.at(0)*sizeof(floatingpoint)));
        CUDAcommon::handleerror(cudaMemsetAsync(gU_i, 0, bntaddvec2.at(0) * sizeof
                                                                                    (floatingpoint), stream));
        CUDAcommon::handleerror(cudaMalloc((void **) &gU_sum, sizeof(floatingpoint)));
        char a[] = "LinkerFF";
        char b[] = "Linker Stretching Harmonic";
        CUDAcommon::handleerror(cudaMalloc((void **) &gFF, 100 * sizeof(char)));
        CUDAcommon::handleerror(cudaMalloc((void **) &ginteraction, 100 * sizeof(char)));
        CUDAcommon::handleerror(cudaMemcpyAsync(gFF, a, 100 * sizeof(char),
                                            cudaMemcpyHostToDevice, stream));
        CUDAcommon::handleerror(cudaMemcpyAsync(ginteraction, b, 100 * sizeof(char),
                                            cudaMemcpyHostToDevice, stream));

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
floatingpoint* LinkerStretchingHarmonic::energy(floatingpoint *coord, floatingpoint *f, int *beadSet,
                                         floatingpoint *kstr, floatingpoint *eql, floatingpoint *pos1, floatingpoint *pos2,
                                         int *params) {
//    if(blocksnthreadse[1]>0) {
//
//        LinkerStretchingHarmonicenergy<<<blocksnthreadse[0], blocksnthreadse[1], (12 * blocksnthreadse[1]) * sizeof
//                (floatingpoint), stream>>>
//                          (coord, f, beadSet, kstr, eql, pos1, pos2, params, gU_i, CUDAcommon::getCUDAvars().gculpritID,
//                                  CUDAcommon::getCUDAvars().gculpritFF,
//                                  CUDAcommon::getCUDAvars().gculpritinteraction, gFF, ginteraction);
//        CUDAcommon::handleerror( cudaGetLastError() ,"LinkerStretchingHarmonicenergy", "LinkerStretchingHarmonic.cu");
//        auto cvars = CUDAcommon::getCUDAvars();
//        cvars.streamvec.push_back(&stream);
//        CUDAcommon::cudavars = cvars;
//        floatingpoint* gpu_Utot = CUDAcommon::getCUDAvars().gpu_energy;
//        addvector<<<1,1,0,stream>>>(gU_i,params, gU_sum, gpu_Utot);
//        CUDAcommon::handleerror( cudaGetLastError() ,"LinkerStretchingHarmonicenergy", "LinkerStretchingHarmonic.cu");
//        return gU_sum;}
//    else
//        return NULL;
}


floatingpoint* LinkerStretchingHarmonic::energy(floatingpoint *coord, floatingpoint *f, int *beadSet,
                                         floatingpoint *kstr, floatingpoint *eql, floatingpoint *pos1, floatingpoint *pos2, floatingpoint *z,
                                         int *params) {
//    if(blocksnthreadse[1]>0) {

//        LinkerStretchingHarmonicenergy<<<blocksnthreadse[0], blocksnthreadse[1], (12 * blocksnthreadse[1]) * sizeof
//                (floatingpoint), stream>>>
//                          (coord, f, beadSet, kstr, eql, pos1, pos2, params, gU_i, z, CUDAcommon::getCUDAvars()
//                                  .gculpritID,
//                                  CUDAcommon::getCUDAvars().gculpritFF,
//                                  CUDAcommon::getCUDAvars().gculpritinteraction, gFF, ginteraction);
//        CUDAcommon::handleerror( cudaGetLastError() ,"LinkerStretchingHarmonicenergy", "LinkerStretchingHarmonic.cu");
//    }

    if(blocksnthreadsez[1]>0) {
        auto boolvarvec = CUDAcommon::cudavars.backtrackbools;
//        LinkerStretchingHarmonicenergyz << < blocksnthreadsez[0], blocksnthreadsez[1], (24 * blocksnthreadsez[1]) *
        LinkerStretchingHarmonicenergyz << < blocksnthreadsez[0], blocksnthreadsez[1], (0) *
                                             sizeof(floatingpoint), stream>> >
                                            (coord, f, beadSet, kstr, eql, pos1, pos2,
                                                    params, gU_i, CUDAcommon::cudavars.gpu_energyvec,  z ,
                                             CUDAcommon::getCUDAvars().gculpritID,
                                             CUDAcommon::getCUDAvars().gculpritFF,
                                             CUDAcommon::getCUDAvars().gculpritinteraction, gFF, ginteraction, boolvarvec.at(0),
                                                    boolvarvec.at(1));
    }
    if(blocksnthreadse[1]<=0 && blocksnthreadsez[1]<=0)
        return NULL;
    else{
        auto cvars = CUDAcommon::getCUDAvars();
        cvars.streamvec.push_back(&stream);
        CUDAcommon::cudavars = cvars;
#ifdef CUDA_INDIVIDUAL_ESUM
        floatingpoint* gpu_Utot = CUDAcommon::getCUDAvars().gpu_energy;
        resetfloatingpointvariableCUDA<<<1,1,0,stream>>>(gU_sum);
        addvectorred2<<<bntaddvec2.at(2),bntaddvec2.at(3), bntaddvec2.at(3) * sizeof(floatingpoint),stream>>>(gU_i,
                params, gU_sum, gpu_Utot);
#endif
        CUDAcommon::handleerror( cudaGetLastError() ,"LinkerStretchingHarmonicenergy", "LinkerStretchingHarmonic.cu");
        return gU_sum;
    }
}
void LinkerStretchingHarmonic::forces(floatingpoint *coord, floatingpoint *f, int *beadSet,
                                      floatingpoint *kstr, floatingpoint *eql, floatingpoint *pos1, floatingpoint
                                      *pos2, int *params, floatingpoint *Lstretchforce ) {
    if (blocksnthreadsf[1] > 0) {
//        LinkerStretchingHarmonicforces << < blocksnthreadsf[0], blocksnthreadsf[1], (12 *
        LinkerStretchingHarmonicforces << < blocksnthreadsf[0], blocksnthreadsf[1], (0 *
                                       blocksnthreadsf[1]) * sizeof(floatingpoint), stream >> >
                                    (coord, f, beadSet, kstr, eql, pos1, pos2, params, Lstretchforce);
        auto cvars = CUDAcommon::getCUDAvars();
        cvars.streamvec.push_back(&stream);
        CUDAcommon::cudavars = cvars;
        CUDAcommon::handleerror(cudaGetLastError(),"LinkerStretchingHarmonicforces", "LinkerStretchingHarmonic.cu");
    }
}
void LinkerStretchingHarmonic::checkforculprit() {
    CUDAcommon::printculprit("LinkerStretching","LinkerStretchingHarmonic");
    Linker* l;
    l = Linker::getLinkers()[CUDAcommon::getCUDAvars().culpritID[0]];
    cout<<"Printing culprit Filament information."<<endl;
    l->printSelf();
    exit(EXIT_FAILURE);
}

#endif
    floatingpoint LinkerStretchingHarmonic::energy(floatingpoint *coord, totalforcefloatingpoint *f, int *beadSet,
                                            floatingpoint *kstr, floatingpoint *eql, floatingpoint *pos1, floatingpoint *pos2) {

        int n = LinkerStretching<LinkerStretchingHarmonic>::n;
        int nint = Linker::getLinkers().size();

        floatingpoint *coord1, *coord2, *coord3, *coord4, dist, U_i;
        floatingpoint *v1 = new floatingpoint[3];
        floatingpoint *v2 = new floatingpoint[3];

        floatingpoint U = 0.0;

        for(int i = 0; i < nint; i += 1) {

            coord1 = &coord[3 * beadSet[n * i]];
            coord2 = &coord[3 * beadSet[n * i + 1]];
            coord3 = &coord[3 * beadSet[n * i + 2]];
            coord4 = &coord[3 * beadSet[n * i + 3]];

            midPointCoordinate(v1, coord1, coord2, pos1[i]);
            midPointCoordinate(v2, coord3, coord4, pos2[i]);

            dist = twoPointDistance(v1, v2) - eql[i];
            U_i = 0.5 * kstr[i] * dist * dist;

            if(fabs(U_i) == numeric_limits<floatingpoint>::infinity()
               || U_i != U_i || U_i < -1.0) {

                //set culprit and return
                LinkerInteractions::_linkerCulprit = Linker::getLinkers()[i];

                return -1;
            }

            U += U_i;
        }
        delete [] v1;
        delete [] v2;

        return U;
    }

    floatingpoint LinkerStretchingHarmonic::energy(floatingpoint *coord, totalforcefloatingpoint * f, int *beadSet,
                                            floatingpoint *kstr, floatingpoint *eql, floatingpoint *pos1, floatingpoint *pos2, floatingpoint d){

        int n = LinkerStretching<LinkerStretchingHarmonic>::n;
        int nint = Linker::getLinkers().size();

        floatingpoint *coord1, *coord2, *coord3, *coord4, dist;
        totalforcefloatingpoint  *f1, *f2, *f3, *f4;
        floatingpoint *v1 = new floatingpoint[3];
        floatingpoint *v2 = new floatingpoint[3];

        floatingpoint U = 0.0;

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

            U += 0.5 * kstr[i] * dist * dist;
        }
        delete [] v1;
        delete [] v2;

        return U;

    }
    void LinkerStretchingHarmonic::forces(floatingpoint *coord, totalforcefloatingpoint *f, int *beadSet,
                                          floatingpoint *kstr, floatingpoint *eql, floatingpoint *pos1, floatingpoint
                                          *pos2, floatingpoint *stretchforce){


        int n = LinkerStretching<LinkerStretchingHarmonic>::n;
        int nint = Linker::getLinkers().size();

        floatingpoint *coord1, *coord2, *coord3, *coord4, dist, invL;
        floatingpoint *v1 = new floatingpoint[3];
        floatingpoint *v2 = new floatingpoint[3];

        totalforcefloatingpoint f0, *f1, *f2, *f3, *f4;

        for(int i = 0; i < nint; i += 1) {

            coord1 = &coord[3 * beadSet[n * i]];
            coord2 = &coord[3 * beadSet[n * i + 1]];
            coord3 = &coord[3 * beadSet[n * i + 2]];
            coord4 = &coord[3 * beadSet[n * i + 3]];
/*            std::cout<<"Linker beadset "<<beadSet[n * i]<<" "<<beadSet[n * i +1]<<" "
                    ""<<beadSet[n * i +2]<<" " <<beadSet[n * i +3]<<endl;*/

            midPointCoordinate(v1, coord1, coord2, pos1[i]);
            midPointCoordinate(v2, coord3, coord4, pos2[i]);

            dist = twoPointDistance(v1, v2) ;
            invL = 1 / dist;

            f0 = kstr[i] * ( dist - eql[i] ) * invL;

            f1 = &f[3 * beadSet[n * i]];
            f2 = &f[3 * beadSet[n * i + 1]];
            f3 = &f[3 * beadSet[n * i + 2]];
            f4 = &f[3 * beadSet[n * i + 3]];

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

            //assign stretch force
            stretchforce[i] = f0/invL;
//        std::cout<<"L "<<i<< " "<<f1[0]<<" "<<f1[1]<<" "<<f1[2]<<" "<<f2[0]<<" "<<f2[1]<<" "
//                ""<<f2[2]<<" "<<f3[0]<<" "<<f3[1]<<" "<<f3[2]<<" "<<f4[0]<<" "<<f4[1]<<" "<<f4[2]<<endl;
//            std::cout<<"L "<<i<<" "<<kstr[i]<<" "<<dist<<" "<<eql[i]<<" "<<invL<<" "
//                    ""<<pos1[i]<<" "<<pos2[i]<<endl;
        }
        delete [] v1;
        delete [] v2;
    }

