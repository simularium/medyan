//
// Created by aravind on 1/24/18.
//

#ifndef CUDA_VEC_FILAMENTBENDINGCOSINECUDA_H
#define CUDA_VEC_FILAMENTBENDINGCOSINECUDA_H
#ifdef CUDAACCL
#include "FilamentBendingCosine.h"

#include "FilamentBending.h"

#include "Bead.h"

#include "MathFunctions.h"
#include "SysParams.h"
#include <limits>
#include <assert.h>
#include <cuda.h>
#include <cuda_runtime.h>

using namespace mathfunc;

//#if !defined(__CUDA_ARCH__) || __CUDA_ARCH__ >= 600
//
//#else
//static __inline__ __device__ double atomicAdd(double *address, double val) {
//    unsigned long long int* address_as_ull = (unsigned long long int*)address;
//    unsigned long long int old = *address_as_ull, assumed;
//    if (val==0.0)
//      return __longlong_as_double(old);
//    do {
//      assumed = old;
//      old = atomicCAS(address_as_ull, assumed, __double_as_longlong(val +__longlong_as_double(assumed)));
//    } while (assumed != old);
//    return __longlong_as_double(old);
//  }
//
//
//#endif

//__global__ void addvectorFB(double *U, int *params, double *U_sum, double *U_tot){
//    U_sum[0] = 0.0;
//    double sum = 0.0;
//    for(auto i=0;i<params[1];i++){
//        if(U[i] == -1.0 && sum != -1.0){
//            U_sum[0] = -1.0;
//            U_tot[0] = -1.0;
//            sum = -1.0;
//            break;
//        }
//        else
//            sum  += U[i];
//    }
//    U_sum[0] = sum;
//    atomicAdd(&U_tot[0], sum);
//
//}

__global__ void FilamentBendingCosineenergy(double *coord, double *force, int *beadSet, double *kbend,
                                            double *eqt, int *params, double *U_i, double *z, int *culpritID,
                                            char* culpritFF, char* culpritinteraction, char* FF, char*
                                            interaction) {
    if(z[0] == 0.0) {
        extern __shared__ double s[];
        double *c1 = s;
        double *c2 = &c1[3 * blockDim.x];
        double *c3 = &c2[3 * blockDim.x];
        double L1, L2, L1L2, l1l2, phi, dPhi;

        int nint = params[1];
        int n = params[0];
        const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;

        if (thread_idx < nint) {
            for (auto i = 0; i < 3; i++) {
                U_i[thread_idx] = 0.0;
                c1[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx] + i];
                c2[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx + 1] + i];
                c3[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx + 2] + i];
            }
        }
        __syncthreads();
        if (thread_idx < nint) {
            L1 = sqrt(scalarProduct(c1, c2,
                                    c1, c2, 3 * threadIdx.x));
            L2 = sqrt(scalarProduct(c2, c3,
                                    c2, c3, 3 * threadIdx.x));

            L1L2 = L1 * L2;
            l1l2 = scalarProduct(c1, c2,
                                 c2, c3, 3 * threadIdx.x);

            phi = safeacos(l1l2 / L1L2);
            dPhi = phi - eqt[thread_idx];

            U_i[thread_idx] = kbend[thread_idx] * (1 - cos(dPhi));

            if (fabs(U_i[thread_idx]) == __longlong_as_double(0x7ff0000000000000) //infinity
                || U_i[thread_idx] != U_i[thread_idx] || U_i[thread_idx] < -1.0) {
                U_i[thread_idx] = -1.0;
                culpritID[0] = thread_idx;
                culpritID[1] = -1;
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
                assert(0);
                __syncthreads();
            }
        }
    }
}

__global__ void FilamentBendingCosineenergyz(double *coord, double *f, int *beadSet, double *kbend,
                                             double *eqt, int *params, double *U_i, double *z, int *culpritID,
                                             char* culpritFF, char* culpritinteraction, char* FF, char*
                                             interaction) {
    if(z[0] != 0.0) {
        extern __shared__ double s[];
        double *c1 = s;
        double *c2 = &c1[3 * blockDim.x];
        double *c3 = &c2[3 * blockDim.x];
        double *f1 = &c3[3 * blockDim.x];
        double *f2 = &f1[3 * blockDim.x];
        double *f3 = &f2[3 * blockDim.x];
        double L1, L2, L1L2, l1l2, phi, dPhi;

        int nint = params[1];
        int n = params[0];
        const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;

        if (thread_idx < nint) {
            U_i[thread_idx] = 0.0;
            for (auto i = 0; i < 3; i++) {
                c1[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx] + i];
                c2[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx + 1] + i];
                c3[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx + 2] + i];
                f1[3 * threadIdx.x + i] = f[3 * beadSet[n * thread_idx] + i];
                f2[3 * threadIdx.x + i] = f[3 * beadSet[n * thread_idx + 1] + i];
                f3[3 * threadIdx.x + i] = f[3 * beadSet[n * thread_idx + 2] + i];
            }

        }
        __syncthreads();

        if (thread_idx < nint) {
            L1 = sqrt(scalarProductStretched(c1, f1, c2, f2,
                                             c1, f1, c2, f2, z[0], 3 * threadIdx.x));
            L2 = sqrt(scalarProductStretched(c2, f2, c3, f3,
                                             c2, f2, c3, f3, z[0], 3 * threadIdx.x));

            L1L2 = L1 * L2;
            l1l2 = scalarProductStretched(c1, f1, c2, f2,
                                          c2, f2, c3, f3, z[0], 3 * threadIdx.x);

            phi = safeacos(l1l2 / L1L2);
            dPhi = phi - eqt[thread_idx];

            U_i[thread_idx] = kbend[thread_idx] * (1 - cos(dPhi));
            if (fabs(U_i[thread_idx]) == __longlong_as_double(0x7ff0000000000000) //infinity
                || U_i[thread_idx] != U_i[thread_idx] || U_i[thread_idx] < -1.0) {
                U_i[thread_idx] = -1.0;
                culpritID[0] = thread_idx;
                culpritID[1] = -1;
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
                assert(0);
                __syncthreads();
            }

        }
    }
}


__global__ void FilamentBendingCosineforces(double *coord, double *f, int *beadSet,
                                                 double *kbend, double *eqt, int *params){
    extern __shared__ double s[];
    double *c1 = s;
    double *c2 = &c1[3 * blockDim.x];
    double *c3 = &c2[3 * blockDim.x];
    double  f1[3], f2[3], f3[3], L1, L2, l1l2, invL1, invL2, A,B,C, phi, dPhi, k;

    int nint = params[1];
    int n = params[0];
    const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;

    if(thread_idx<nint) {
        for(auto i=0;i<3;i++){
            c1[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx] + i];
            c2[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx + 1] + i];
            c3[3 * threadIdx.x + i] = coord[3 * beadSet[n * thread_idx + 2] + i];
        }

    }
    __syncthreads();

    if(thread_idx<nint) {
        L1 = sqrt(scalarProduct(c1, c2,
                                c1, c2, 3 * threadIdx.x));
        L2 = sqrt(scalarProduct(c2, c3,
                                c2, c3, 3 * threadIdx.x));

        l1l2 = scalarProduct(c1, c2,
                             c2, c3, 3 * threadIdx.x);
        invL1 = 1/L1;
        invL2 = 1/L2;
        A = invL1*invL2;
        B = l1l2*invL1*A*A*L2;
        C = l1l2*invL2*A*A*L1;

        if (areEqual(eqt[thread_idx], 0.0)) k = kbend[thread_idx];

        else{
            phi = safeacos(l1l2 *A);
            dPhi = phi-eqt[thread_idx];

            k = kbend[thread_idx] * sin(dPhi)/sin(phi);
        }
//        printf("%d %f %f\n", thread_idx, k, eqt[thread_idx]);
        //force on i-1, f = k*(-A*l2 + B*l1):
        f1[0] =  k * ((-c3[3 * threadIdx.x] + c2[3 * threadIdx.x])*A +
                           (c2[3 * threadIdx.x] - c1[3 * threadIdx.x])*B );
        f1[1] =  k * ((-c3[3 * threadIdx.x + 1] + c2[3 * threadIdx.x + 1])*A +
                           (c2[3 * threadIdx.x + 1] - c1[3 * threadIdx.x + 1])*B );
        f1[2] =  k * ((-c3[3 * threadIdx.x + 2] + c2[3 * threadIdx.x + 2])*A +
                           (c2[3 * threadIdx.x + 2] - c1[3 * threadIdx.x + 2])*B );

        //force on i, f = k*(A*(l1-l2) - B*l1 + C*l2):
        f2[0] =  k *( (c3[3 * threadIdx.x] - 2.0*c2[3 * threadIdx.x] + c1[3 * threadIdx.x])*A -
                           (c2[3 * threadIdx.x] - c1[3 * threadIdx.x])*B +
                           (c3[3 * threadIdx.x] - c2[3 * threadIdx.x])*C );

        f2[1] =  k *( (c3[3 * threadIdx.x + 1] - 2.0*c2[3 * threadIdx.x + 1] + c1[3 * threadIdx.x + 1])*A -
                           (c2[3 * threadIdx.x + 1] - c1[3 * threadIdx.x + 1])*B +
                           (c3[3 * threadIdx.x + 1] - c2[3 * threadIdx.x + 1])*C );

        f2[2] =  k *( (c3[3 * threadIdx.x + 2] - 2.0*c2[3 * threadIdx.x + 2] + c1[3 * threadIdx.x + 2])*A -
                           (c2[3 * threadIdx.x + 2] - c1[3 * threadIdx.x + 2])*B +
                           (c3[3 * threadIdx.x + 2] - c2[3 * threadIdx.x + 2])*C );

        //force on i-1, f = k*(A*l - B*l2):
        f3[0] =  k *( (c2[3 * threadIdx.x] - c1[3 * threadIdx.x])*A -
                           (c3[3 * threadIdx.x] - c2[3 * threadIdx.x])*C );

        f3[1] =  k *( (c2[3 * threadIdx.x + 1] - c1[3 * threadIdx.x + 1])*A -
                           (c3[3 * threadIdx.x + 1] - c2[3 * threadIdx.x + 1])*C );

        f3[2] =  k *( (c2[3 * threadIdx.x + 2] - c1[3 * threadIdx.x + 2])*A -
                           (c3[3 * threadIdx.x + 2] - c2[3 * threadIdx.x + 2])*C );
//        printf("%d %f %f %f %f %f %f %f %f %f\n",thread_idx, f1[0], f1[1], f1[2], f2[0], f2[1], f2[2], f3[0], f3[1],
//               f3[2]);
        for (int i = 0; i < 3; i++) {
            atomicAdd(&f[3 * beadSet[n * thread_idx] + i], f1[i]);
            atomicAdd(&f[3 * beadSet[n * thread_idx + 1] + i], f2[i]);
            atomicAdd(&f[3 * beadSet[n * thread_idx + 2] + i], f3[i]);
        }
    }
}

#endif
#endif //CUDA_VEC_FILAMENTBENDINGCOSINECUDA_H
