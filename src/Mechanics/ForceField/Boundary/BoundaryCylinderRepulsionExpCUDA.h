//
// Created by aravind on 11/21/17.
//

#ifndef CUDA_VEC_BOUNDARYCYLINDERREPULSIONEXPCUDA_H
#define CUDA_VEC_BOUNDARYCYLINDERREPULSIONEXPCUDA_H
#ifdef CUDAACCL
#include "MathFunctions.h"
#include "SysParams.h"
#include <limits>
#include <assert.h>
#include <cuda.h>
#include <cuda_runtime.h>
using namespace mathfunc;
__global__ void BoundaryCylinderRepulsionExpenergy(floatingpoint* coord, floatingpoint* f, int* beadSet, floatingpoint* krep, floatingpoint* slen,
                                                   int* nintvec, floatingpoint* beListplane,
                                                   int* params, floatingpoint* U_i,
                                                   floatingpoint *z, int* culpritID, char*
                                                   culpritFF, char* culpritinteraction, char*
                                                   FF, char* interaction){
    if(z[0] == 0.0) {
        extern __shared__ floatingpoint s[];
        floatingpoint *c1 = s;
        int nint = params[1];
        floatingpoint R, r;
        int n = params[0];
        const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;
        floatingpoint plane[4];
        if (thread_idx < nint) {
            U_i[thread_idx] = 0.0;
            for (auto i = 0; i < 3; i++) {
                c1[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx] + i];
            }
            //get the plane equation.
            if (thread_idx < nintvec[0]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[i];
            } else if (thread_idx >= nintvec[0] && thread_idx < nintvec[1]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[4 + i];
            } else if (thread_idx >= nintvec[1] && thread_idx < nintvec[2]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[8 + i];
            } else if (thread_idx >= nintvec[2] && thread_idx < nintvec[3]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[12 + i];
            } else if (thread_idx >= nintvec[3] && thread_idx < nintvec[4]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[16 + i];
            } else if (thread_idx >= nintvec[4] && thread_idx < nintvec[5]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[20 + i];
            }
            //get distance from plane
            r = getdistancefromplane(c1, plane, 3 * threadIdx.x);
            R = -r / slen[thread_idx];
            U_i[thread_idx] = krep[thread_idx] * exp(R);

            if (fabs(U_i[thread_idx]) == __longlong_as_floatingpoint(0x7ff0000000000000) //infinity
                || U_i[thread_idx] != U_i[thread_idx] || U_i[thread_idx] < -1.0) {
                //set culprit and exit
                U_i[thread_idx] = -1.0;
                culpritID[0] = beadSet[n * thread_idx];//set Cylinder info.
                //set boundary element information
                if (thread_idx < nintvec[0]) culpritID[1] = 0;
                else if (thread_idx >= nintvec[0] || thread_idx < nintvec[1]) culpritID[1] = 1;
                else if (thread_idx >= nintvec[1] || thread_idx < nintvec[2]) culpritID[1] = 2;
                else if (thread_idx >= nintvec[2] || thread_idx < nintvec[3]) culpritID[1] = 3;
                else if (thread_idx >= nintvec[3] || thread_idx < nintvec[4]) culpritID[1] = 4;
                else if (thread_idx >= nintvec[4] || thread_idx < nintvec[5]) culpritID[1] = 5;
                int j = 0;
                while (FF[j] != 0) {
                    culpritFF[j] = FF[j];
                    j++;
                }
                j = 0;
                while (interaction[j] != 0) {
                    culpritinteraction[j] = interaction[j];
                    j++;
                }

                printf("Coord %d %f %f %f\n", thread_idx, c1[3*threadIdx.x],c1[3*threadIdx.x +1],c1[3*threadIdx.x +2]);
                printf("Plane %d %f %f %f %f\n", thread_idx, plane[0], plane[1], plane[2], plane[3]);
                printf("%d %f %f %f\n", thread_idx, r, R, U_i[thread_idx]);
                assert(0);
                __syncthreads();
            }
        }
    }
}

__global__ void BoundaryCylinderRepulsionExpenergyz(floatingpoint* coord, floatingpoint* f, int* beadSet, floatingpoint* krep, floatingpoint* slen,
                                                   int* nintvec, floatingpoint* beListplane,
                                                    int* params, floatingpoint* U_i, floatingpoint *U_vec,
                                                   floatingpoint *z, int* culpritID, char* culpritFF, char* culpritinteraction,
                                                   char* FF, char* interaction, bool*
                                                    conv_state1, bool* conv_state2){
    if(conv_state1[0]||conv_state2[0]) return;
    if(z[0] == 0.0) {
//        extern __shared__ floatingpoint s[];
//        floatingpoint *c1 = s;
        floatingpoint *c1;
        int nint = params[1];
        floatingpoint R, r;
        int n = params[0];
        const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;
        floatingpoint plane[4];
        int offset = max(params[2] - 1,0);
        if (thread_idx < nint) {
//            if(thread_idx == 0){
//                printf("Offset %d \n", offset);
//            }
            U_i[thread_idx] = 0.0;
//            for (auto i = 0; i < 3; i++) {
//                c1[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx] + i];
//            }
            c1 = &coord[3 * beadSet[n * thread_idx]];
            //get the plane equation.
            if (thread_idx < nintvec[0]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[i];
            } else if (thread_idx >= nintvec[0] && thread_idx < nintvec[1]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[4 + i];
            } else if (thread_idx >= nintvec[1] && thread_idx < nintvec[2]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[8 + i];
            } else if (thread_idx >= nintvec[2] && thread_idx < nintvec[3]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[12 + i];
            } else if (thread_idx >= nintvec[3] && thread_idx < nintvec[4]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[16 + i];
            } else if (thread_idx >= nintvec[4] && thread_idx < nintvec[5]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[20 + i];
            }
            //get distance from plane
//            r = getdistancefromplane(c1, plane, 3 * threadIdx.x);
            r = getdistancefromplane(c1, plane,0);
            R = -r / slen[thread_idx];
            U_i[thread_idx] = krep[thread_idx] * exp(R);
            U_vec[offset + thread_idx] = krep[thread_idx] * exp(R);
            if (fabs(U_i[thread_idx]) == __longlong_as_floatingpoint(0x7ff0000000000000) //infinity
                || U_i[thread_idx] != U_i[thread_idx] || U_i[thread_idx] < -1.0) {
                //set culprit and exit
                U_i[thread_idx] = -1.0;
                culpritID[0] = beadSet[n * thread_idx];//set Cylinder info.
                //set boundary element information
                if (thread_idx < nintvec[0]) culpritID[1] = 0;
                else if (thread_idx >= nintvec[0] || thread_idx < nintvec[1]) culpritID[1] = 1;
                else if (thread_idx >= nintvec[1] || thread_idx < nintvec[2]) culpritID[1] = 2;
                else if (thread_idx >= nintvec[2] || thread_idx < nintvec[3]) culpritID[1] = 3;
                else if (thread_idx >= nintvec[3] || thread_idx < nintvec[4]) culpritID[1] = 4;
                else if (thread_idx >= nintvec[4] || thread_idx < nintvec[5]) culpritID[1] = 5;
                int j = 0;
                while (FF[j] != 0) {
                    culpritFF[j] = FF[j];
                    j++;
                }
                j = 0;
                while (interaction[j] != 0) {
                    culpritinteraction[j] = interaction[j];
                    j++;
                }
                printf("Coord %d %f %f %f\n", thread_idx, c1[0],c1[1], c1[2]);
//                printf("Coord %d %f %f %f\n", thread_idx, c1[3*threadIdx.x],c1[3*threadIdx.x +1],c1[3*threadIdx.x +2]);
                printf("Plane %d %f %f %f %f\n", thread_idx, plane[0], plane[1], plane[2], plane[3]);
                printf("%d %f %f %f\n", thread_idx, r, R, U_i[thread_idx]);
                assert(0);
                __syncthreads();
            }
        }
    }
    else if(z[0] != 0.0) {
//        extern __shared__ floatingpoint s[];
//        floatingpoint *c1 = s;
//        floatingpoint *f1 = &c1[3 * blockDim.x];
        int offset = max(params[2] - 1,0);
        int nint = params[1];
        floatingpoint R, r;
        int n = params[0];
        const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;
        floatingpoint plane[4];
        floatingpoint *c1, *f1;
        if (thread_idx < nint) {
//            if(thread_idx == 0){
//                printf("Offset %d \n", offset);
//            }
            U_i[thread_idx] = 0.0;
//            for (auto i = 0; i < 3; i++) {
//                c1[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx] + i];
//                f1[3 * threadIdx.x + i] = f[3 * beadSet[n * thread_idx] + i];
//            }
            c1 = &coord[3 * beadSet[n * thread_idx]];
            f1 = &f[3 * beadSet[n * thread_idx]];
            //get the plane equation.
            if (thread_idx < nintvec[0]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[i];
            } else if (thread_idx >= nintvec[0] && thread_idx < nintvec[1]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[4 + i];
            } else if (thread_idx >= nintvec[1] && thread_idx < nintvec[2]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[8 + i];
            } else if (thread_idx >= nintvec[2] && thread_idx < nintvec[3]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[12 + i];
            } else if (thread_idx >= nintvec[3] && thread_idx < nintvec[4]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[16 + i];
            } else if (thread_idx >= nintvec[4] && thread_idx < nintvec[5]) {
                for (auto i = 0; i < 4; i++)
                    plane[i] = beListplane[20 + i];
            }
            //get distance from plane
//            r = getstretcheddistancefromplane(c1, f1, plane, z[0], 3 * threadIdx.x);
            r = getstretcheddistancefromplane(c1, f1, plane, z[0],0);
            R = -r / slen[thread_idx];
            U_i[thread_idx] = krep[thread_idx] * exp(R);
            U_vec[offset + thread_idx] = krep[thread_idx] * exp(R);

/*            if (fabs(U_i[thread_idx]) == __longlong_as_floatingpoint(0x7ff0000000000000) //infinity
                || U_i[thread_idx] != U_i[thread_idx] || U_i[thread_idx] < -1.0) {
                //set culprit and exit
                U_i[thread_idx] = -1.0;
                culpritID[0] = thread_idx;
                if (thread_idx < nintvec[0]) culpritID[1] = 0;
                else if (thread_idx >= nintvec[0] || thread_idx < nintvec[1]) culpritID[1] = 1;
                else if (thread_idx >= nintvec[1] || thread_idx < nintvec[2]) culpritID[1] = 2;
                else if (thread_idx >= nintvec[2] || thread_idx < nintvec[3]) culpritID[1] = 3;
                else if (thread_idx >= nintvec[3] || thread_idx < nintvec[4]) culpritID[1] = 4;
                else if (thread_idx >= nintvec[4] || thread_idx < nintvec[5]) culpritID[1] = 5;
                int j = 0;
                while (FF[j] != 0) {
                    culpritFF[j] = FF[j];
                    j++;
                }
                j = 0;
                while (interaction[j] != 0) {
                    culpritinteraction[j] = interaction[j];
                    j++;
                }
                printf("Force2 %d %f %f %f\n", thread_idx, f[3 * beadSet[n *
                        thread_idx]],f[3 * beadSet[n * thread_idx]+1],f[3 * beadSet[n *
                        thread_idx]+2]);
                printf("Force %d %f %f %f %d\n", thread_idx, f1[0],f1[1], f1[2], beadSet[n * thread_idx]);
                printf("Coord %d %f %f %f\n", thread_idx, c1[0],c1[1], c1[2]);
                printf("Plane %d %f %f %f %f\n", thread_idx, plane[0], plane[1], plane[2], plane[3]);
                printf("%d %f %f %f\n", thread_idx, r, R, U_i[thread_idx]);
                assert(0);
                __syncthreads();
            }*/
        }
    }
}

__global__ void BoundaryCylinderRepulsionExpforces(floatingpoint* coord, floatingpoint* f, int* beadSet, floatingpoint* krep, floatingpoint* slen,
                                                   int* nintvec, floatingpoint* beListplane, int* params){
//    extern __shared__ floatingpoint s[];
//    floatingpoint *c1 = s;
    floatingpoint *c1;
    int nint = params[1];
    floatingpoint R, r, norm[3], f0;
    int n = params[0];
    const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;
    floatingpoint plane[4];
    if(thread_idx<nint) {
//        for(auto i=0;i<3;i++){
//            c1[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx] + i];
//        }
//
//    }
//    __syncthreads();
//    if(thread_idx<nint) {
        c1 = &coord[3 * beadSet[n * thread_idx]];
        //get the plane equation.
        if (thread_idx < nintvec[0]) {
            for (auto i = 0; i<4; i++)
                plane[i] = beListplane [i];
        }
        else if(thread_idx >= nintvec[0] && thread_idx < nintvec[1]){
            for (auto i = 0; i<4; i++)
                plane[i] = beListplane [4 + i];
        }
        else if(thread_idx >= nintvec[1] && thread_idx < nintvec[2]){
            for (auto i = 0; i<4; i++)
                plane[i] = beListplane [8 + i];
        }
        else if(thread_idx >= nintvec[2] && thread_idx < nintvec[3]){
            for (auto i = 0; i<4; i++)
                plane[i] = beListplane [12 + i];
        }
        else if(thread_idx >= nintvec[3] && thread_idx < nintvec[4]){
            for (auto i = 0; i<4; i++)
                plane[i] = beListplane [16 + i];
        }
        else if(thread_idx >= nintvec[4] && thread_idx < nintvec[5]){
            for (auto i = 0; i<4; i++)
                plane[i] = beListplane [20 + i];
        }
        //get distance from plane
//        r = getdistancefromplane(c1, plane, 3 * threadIdx.x);
        r = getdistancefromplane(c1, plane);
        for(auto i = 0; i < 3; i++)
            norm[i] = plane[i];
        R = -r / slen[thread_idx];
        f0 = krep[thread_idx] * exp(R);
        for (int i = 0; i < 3; i++) {
            if (fabs(f0*norm[i]) == __longlong_as_floatingpoint(0x7ff0000000000000) //infinity
                || f0*norm[i] != f0*norm[i]) {
                printf("Boundary Force became infinite %f %f \n",f0, norm[i]);
                assert(0);
            }
            atomicAdd(&f[3 * beadSet[n * thread_idx] + i], f0*norm[i]);
        }
    }
}
//__global__ void BoundaryCylinderRepulsionadd(floatingpoint *force, floatingpoint *forcecopy, int *nint) {
//    const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;
//
//    if (thread_idx < nint[0]) {
//        for (auto i = 0; i < 3; i++) {
//            force[3 * thread_idx + i] =  force[3 * thread_idx + i] + forcecopy[3 * thread_idx + i];
//        }
//    }
//}
#endif

#endif //CUDA_VEC_BOUNDARYCYLINDERREPULSIONCUDA_H
