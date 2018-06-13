//
// Created by aravind on 11/1/17.
//

#ifndef CUDA_VEC_CGMETHODCUDA_H
#define CUDA_VEC_CGMETHODCUDA_H
#ifdef CUDAACCL
#include <cuda.h>
#include <cuda_runtime.h>
#include "math.h"
#include "utility.h"
#include "assert.h"

__global__ void correctlambdaCUDA(double *gpu_lambda, int *gpu_state, double *gpu_params){
    printf("gpu_state %d\n",gpu_state[0]);
    if(gpu_state[0] == 1 || gpu_state[0] == 3 )
        gpu_lambda[0] = gpu_lambda[0] /gpu_params[1];
    printf("final lambda %f\n", gpu_lambda[0]);
}

__global__ void moveBeadsCUDA(double *coord, double* f, double *d,  int *nint, bool *checkin) {
    if(checkin[0] == false) return; //if it is not in minimization state
    const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;
//    if(thread_idx == 0)
//        printf("%d %f\n", checkin[0], d[0]);

    if (thread_idx < nint[0]) {
        for (auto i = 0; i < 3; i++) {
            coord[3 * thread_idx + i] = coord[3 * thread_idx + i] + d[0] * f[3 * thread_idx + i] ;
        }
    }
}
__global__ void shiftGradientCUDA(double *f, double* fAux, int * nint, double* newGrad, double* prevGrad, double*
curGrad, bool *Mstate) {
    const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;
    if(thread_idx == 0)
        printf("CUDA min state shift %d \n", Mstate[0]);
    if(Mstate[0] == false) return;

    double d  = fmax(0.0, (newGrad[0] - prevGrad[0]) / curGrad[0]);
    if(thread_idx == 0) {
        printf("beta CUDA %f\n", d);
        printf(" newGrad %f prevGrad %f, curGrad %f\n", newGrad[0], prevGrad[0], curGrad[0]);
    }
//    if(thread_idx == 0)
//        printf("Shift Gradient %f %f %f %f\n", d, newGrad[0], prevGrad[0],curGrad[0] );

    if (thread_idx < nint[0] && Mstate[0]) {
        for (auto i = 0; i < 3; i++) {
            f[3 * thread_idx + i] = fAux[3 * thread_idx + i] + d * f[3 * thread_idx + i];
            if (fabs(f[3 * thread_idx + i]) == __longlong_as_double(0x7ff0000000000000) //infinity
                || f[3 * thread_idx + i] != f[3 * thread_idx + i]) {
                printf("Force became infinite during gradient shift. \n Force %f Aux Force %f Beta %f\n NewGrad %f "
                               "PrevGrad %f curGrad %f \n",
                       f[3 * thread_idx + i], fAux[3 * thread_idx + i], d, newGrad[0], prevGrad[0],curGrad[0]);
                assert(0);
            }
        }
    }
}

__global__ void shiftGradientCUDAifsafe(double *f, double* fAux, int * nint, bool *Mstate, bool *Sstate) {
    if(Mstate[0] == false || Sstate[0] == false) return;//checks for Minimization state and Safe state.
    const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;
//    if(thread_idx == 0)
//        printf("shiftGradient safe \n");
    if (thread_idx < nint[0]) {
        for (auto i = 0; i < 3; i++) {
            f[3 * thread_idx + i] = fAux[3 * thread_idx + i];
            if (fabs(f[3 * thread_idx + i]) == __longlong_as_double(0x7ff0000000000000) //infinity
                || f[3 * thread_idx + i] != f[3 * thread_idx + i]) {
                printf("Force became infinite during SAFE gradient shift. \n Force %f Aux Force %f \n",
                       f[3 * thread_idx + i], fAux[3 * thread_idx + i]);
                assert(0);
            }
        }
    }
}

__global__ void allFADotFCUDA(double *f1, double *f2, double *g, int * nint) {
    const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;
    if (thread_idx < nint[0]) {
            g[thread_idx] = 0.0;
    }
    __syncthreads();
    if (thread_idx < nint[0]) {
        for (auto i = 0; i < 3; i++) {
           g[thread_idx] +=f1[3 * thread_idx +i] * f2[3 * thread_idx +i];
        }
//        printf("CUDA %d %f %f %f %f\n",thread_idx, g[thread_idx],f1[3 * thread_idx],f1[3 * thread_idx +1],f1[3 *
//               thread_idx +2]);
    }
}

//__global__ void maxFCUDA(double *f, int * nint, double *fmax) {
//
//    double mag;
//    fmax[0]=0.0;
//    for(int i = 0; i < 3 * nint[0]; i++) {
//        mag = sqrt(f[i]*f[i]);
//        if(mag > fmax[0]) {fmax[0] = mag;}
//    }
////    id = id -id%3;
//    printf("Fmaxv1 %f \n", fmax[0]);
//}

__global__ void maxFCUDAred(double *f, int * nint, double *fmax) {


    double mag;
    int offset = 0;
    extern __shared__ double s[];
    double *c1 = s;
    double temp = -1.0;
    const unsigned int thread_idx = (blockIdx.x * blockDim.x) + threadIdx.x;
    while(thread_idx + offset < 3 * nint[0]){
        if(fabs(f[thread_idx + offset]) > temp)
            temp = fabs(f[thread_idx + offset]);
        offset += blockDim.x; //3
    }
    c1[threadIdx.x] = temp;
    __syncthreads();
    if(threadIdx.x == 0){
        fmax[0] = 0.0;
        for(auto i=0; i <3; i++){
            if(c1[i] > fmax[0])
                fmax[0] = c1[i];
        }
        printf("Fmax %f %f %f %f \n", fmax[0], c1[0], c1[1], c1[2]);
    }
}

__global__ void maxFCUDAredv2(double *f, int * nint, double *fmax) {

    extern __shared__ double s[];
    double *c1 = s;
    int start = 0;
    int end = nint[0];
    int factor = nint[0]/blockDim.x;
    if(threadIdx.x ==0)
        printf("CMAXF %d %d %d\n", factor, nint[0],blockDim.x);
    if (threadIdx.x > 0)
        start = threadIdx.x * factor;
    if (threadIdx.x < blockDim.x - 1)
        end = (threadIdx.x + 1) * factor;
    c1[threadIdx.x] = -1.0;
    for (auto i = start; i < end; i++) {
        if(fabs(sqrt(f[i])) > c1[threadIdx.x])
            c1[threadIdx.x] = sqrt(fabs(f[i]));
    }
    __syncthreads();

    if (threadIdx.x == 0) {
        fmax[0] = -1.0;
        for (auto i = 0; i < blockDim.x; i++) {
            if(fabs(c1[i]) > fmax[0])
                fmax[0] = c1[i];
        }
//        printf("Fmaxv2 %f f_x %f f_y %f f_z %f \n", fmax[0], c1[0], c1[1], c1[2]);
        printf("Fmax CUDA %f \n", fmax[0]);
    }
}

//__global__ void maxFCUDAredv2(double *f, int * nint, double *fmax) {
//
//    extern __shared__ double s[];
//    double *c1 = s;
//    int start = 0;
//    int end = 3 * nint[0];
//    int factor = 3 * nint[0]/blockDim.x;
//    if(threadIdx.x ==0)
//        printf("CMAXF %d %d %d\n", factor, nint[0],blockDim.x);
//    if (threadIdx.x > 0)
//        start = threadIdx.x * factor;
//    if (threadIdx.x < blockDim.x - 1)
//        end = (threadIdx.x + 1) * factor;
//    c1[threadIdx.x] = -1.0;
//    for (auto i = start; i < end; i++) {
//        if(fabs(f[i]) > c1[threadIdx.x])
//            c1[threadIdx.x] = fabs(f[i]);
//    }
//    __syncthreads();
//
//    if (threadIdx.x == 0) {
//        fmax[0] = -1.0;
//        for (auto i = 0; i < blockDim.x; i++) {
//            if(fabs(c1[i]) > fmax[0])
//                fmax[0] = c1[i];
//        }
////        printf("Fmaxv2 %f f_x %f f_y %f f_z %f \n", fmax[0], c1[0], c1[1], c1[2]);
//        printf("Fmax CUDA %f \n", fmax[0]);
//    }
//}

__global__ void addvector(double *U, int *params, double *U_sum){
    U_sum[0] = 0.0;
//    printf("%d \n", params[0]);

    for(auto i=0;i<params[0];i++){
        U_sum[0]  += U[i];
    }
    printf("add1 %f \n", U_sum[0]);
}

//__global__ void addvectorred(double *U, int *params, double *U_sum){
//    extern __shared__ double s[];
//    double *c1 = s;
//    int start = 0;
//    int end = params[0];
//    int factor = params[0]/blockDim.x;
//    if(threadIdx.x > 0)
//        start = threadIdx.x * factor;
//    if(threadIdx.x < blockDim.x - 1)
//        end = (threadIdx.x + 1) * factor;
//    c1[threadIdx.x] = 0.0;
//    for(auto i = start; i < end; i++)
//        c1[threadIdx.x] += U[i];
////    printf("%d \n", params[0]);
//    __syncthreads();
//
//    if(threadIdx.x == 0) {
//        U_sum[0] = 0.0;
//        for (auto i = 0; i < blockDim.x; i++) {
//            U_sum[0] += c1[i];
//        }
////        printf("add2 %f \n", U_sum[0]);
//    }
//}

__global__ void addvectorred2(double *g_idata, int *num, double *g_odata)
{
    extern __shared__ double sdata[];
    unsigned int tid = threadIdx.x;
    auto blockSize = blockDim.x;
    unsigned int i = blockIdx.x*(blockSize*2) + tid;
    unsigned int gridSize = blockSize*2*gridDim.x;
    int n = num[0];
    sdata[tid] = 0;
    while (i < n) {
        sdata[tid] += g_idata[i] + g_idata[i+blockSize];
        i += gridSize;
    }
    __syncthreads();
    if(blockSize >=2048 ) {printf("Cannot handle blocks with threads larger than 2048\n");assert(0);}
    if (blockSize >= 1024) { if (tid < 512) { sdata[tid] += sdata[tid + 512]; } __syncthreads(); }
    if (blockSize >= 512) { if (tid < 256) { sdata[tid] += sdata[tid + 256]; } __syncthreads(); }
    if (blockSize >= 256) { if (tid < 128) { sdata[tid] += sdata[tid + 128]; } __syncthreads(); }
    if (blockSize >= 128) { if (tid < 64) { sdata[tid] += sdata[tid + 64]; } __syncthreads(); }
    if (tid < 32) {
        if (blockSize >= 64) sdata[tid] += sdata[tid + 32];
        if (blockSize >= 32) sdata[tid] += sdata[tid + 16];
        if (blockSize >= 16) sdata[tid] += sdata[tid + 8];
        if (blockSize >= 8) sdata[tid] += sdata[tid + 4];
        if (blockSize >= 4) sdata[tid] += sdata[tid + 2];
        if (blockSize >= 2) sdata[tid] += sdata[tid + 1];
    }
    if (tid == 0) {atomicAdd(&g_odata[0], sdata[0]);
//        printf("addv2 %f %f %f\n", sdata[0], sdata[1], sdata[2]);
    }
}

__global__ void initializeLambdaCUDA(bool *checkin, bool* checkout, double *currentEnergy, double *energy,
                                     double* CUDA_lambda, double* fmax, double* params, bool *Safestate, int *status){
    checkin[0] = false;
    checkout[0] = false;
    status[0] = 0;
//    printf("lambda_status %d\n", status[0]);
    double LAMBDAMAX = params[3];
//    printf("SS %d \n",Safestate[0]);
    if(Safestate[0] == true){//safebacktrackinglinesearch
        CUDA_lambda[0] = LAMBDAMAX;
    }
    else{//backtrackinglinesearch
        double MAXDIST = params[4];

        if(fmax[0]==0.0) {
            CUDA_lambda[0] = 0.0;
            checkout[0] = true;
        }
        else
            CUDA_lambda[0] = fmin(LAMBDAMAX, MAXDIST / fmax[0]);
    }
    printf("CL lambdamax %f cuda_lambda %f fmax %f MAXDIST %f state %d\n", LAMBDAMAX, CUDA_lambda[0], fmax[0],
           params[4],
           checkout[0]);
}

__global__ void resetlambdaCUDA (double *CUDA_lambda){
    CUDA_lambda[0] = 0.0;
}
//__global__ void prepbacktracking(bool *checkin, bool* checkout, double *currentEnergy, double *energy,
//                                 double* CUDA_lambda, double* fmax, double* params){
//    //if(checkin[0]) return;
//    checkin[0] = false;
//    checkout[0] = false;
//    currentEnergy[0] = energy[0];
//    checkout[0] = false;
//    double LAMBDAMAX = params[3];
//    double MAXDIST = params[4];
//
//    if(fmax[0]==0.0) {
//       CUDA_lambda[0] = 0.0;
//        checkout[0] = true;
//    }
//    else
//        CUDA_lambda[0] = fmin(LAMBDAMAX, MAXDIST / fmax[0]);
//    //printf("%f \n", CUDA_lambda[0]);
//}
//
//__global__ void prepsafebacktracking(bool* checkin, bool *checkout, double* currentEnergy, double* energy, double*
//CUDA_lambda, double* params) {
//    checkin[0] = false;
//    checkout[0] = false;
//    currentEnergy[0] = energy[0];
//    double LAMBDAMAX = params[3];
//    CUDA_lambda[0] = LAMBDAMAX;
//}
__global__ void setcurrentenergy( double* energy, double* currentenergy, double *CUDAlambda, double *initlambdalocal){
    currentenergy[0] = energy[0];
    CUDAlambda[0] = initlambdalocal[0];
}

__global__ void findLambdaCUDA(double* energyLambda, double* currentEnergy, double* FDotFA, double *fmax, double*
lambda, double *params, bool* prev_convergence, bool*  current_convergence, bool *safestate, int *status){
//    printf("prev_conv %d \n", prev_convergence[0]);
    if(prev_convergence[0]) return;
//    current_convergence[0] = false;
//    double LAMBDAREDUCE = params[1];
//    double LAMBDATOL = params[2];
//    double MAXDIST = params[4];
    double idealEnergyChange = 0.0;
    double energyChange = 0.0;
    printf("safestate %d\n",safestate[0]);
    if(safestate[0] == true){
        energyChange = energyLambda[0] - currentEnergy[0];
        printf("energyChange %f \n", energyChange);
        if (energyChange <= 0.0) {
            current_convergence[0] = true;
            atomicAdd(&status[0], 1);
            printf("energyChange %f lambda_converged %.8f\n", energyChange, lambda[0]);
            return;
        }
    }
    else {
        double BACKTRACKSLOPE = params[0];
        idealEnergyChange = -BACKTRACKSLOPE * lambda[0] * FDotFA[0];
        energyChange = energyLambda[0] - currentEnergy[0];
        printf("BACKTRACKSLOPE %f lambda %f allFDotFA %f \n", BACKTRACKSLOPE, lambda[0], FDotFA[0]);
        printf("idealEnergyChange %f energyChange %f \n",
               idealEnergyChange, energyChange);
        if (energyChange <= idealEnergyChange) {
            current_convergence[0] = true;
//            printf("lambda_statusb %d\n", status[0]);
            atomicAdd(&status[0], 1);
//            printf("lambda_status %d\n", status[0]);
            printf("idealEnergyChange %f energyChange %f lambda_converged %.8f \n",
                   idealEnergyChange, energyChange,
                   lambda[0]);
            return;
        }
    }
//    lambda[0] *= LAMBDAREDUCE;
//
//    if(lambda[0] <= 0.0 || lambda[0] <= LAMBDATOL) {
//        current_convergence[0] = true;
//        if(safestate[0] == true)
//            lambda[0] = MAXDIST / fmax[0];
//        else
//            lambda[0] = 0.0;
//    }
//    printf("lambda %.8f %.8f\n", lambda[0], LAMBDATOL);
}
__global__ void findLambdaCUDA2(double *fmax, double* lambda, double *params, bool* prev_convergence, bool*
current_convergence, bool *safestate, int *status){
    printf("prev_conv %d \n", prev_convergence[0]);
    if(prev_convergence[0]) return;
    double LAMBDAREDUCE = params[1];
    double MAXDIST = params[4];
    double LAMBDATOL = params[2];
    lambda[0] *= LAMBDAREDUCE;
    if(lambda[0] <= 0.0 || lambda[0] <= LAMBDATOL) {
        current_convergence[0] = true;
        atomicAdd(&status[0], 2);
        if(safestate[0] == true)
            lambda[0] = MAXDIST / fmax[0];
        else
            lambda[0] = 0.0;

    }
    printf("lambda2 %.8f state %d\n", lambda[0], current_convergence[0]);
}
//__global__ void CUDAbacktrackingfindlambda(double* energyLambda, double* currentEnergy, double* FDotFA, double*
//lambda, double *params, bool* prev_convergence, bool*  current_convergence){
////    printf("%d %d %f %f\n", prev_convergence[0],current_convergence[0],energyLambda[0],currentEnergy[0]);
//    if(prev_convergence[0]) return;
//    current_convergence[0] = false;
//    double BACKTRACKSLOPE = params[0];
//    double LAMBDAREDUCE = params[1];
//    double LAMBDATOL = params[2];
//    double idealEnergyChange = -BACKTRACKSLOPE * lambda[0] * FDotFA[0];
//    double energyChange =  energyLambda[0] - currentEnergy[0];
////    printf("%f \n", lambda[0]);
//    if(energyChange <= idealEnergyChange) {
//        current_convergence[0] = true;
//        return;}
//    lambda[0] *= LAMBDAREDUCE;
////    printf("lambdareduce %f \n", lambda[0]);
//    if(lambda[0] <= 0.0 || lambda[0] <= LAMBDATOL) {
//        current_convergence[0] = true;
//        lambda[0] = 0.0;
//    }
//}

//__global__ void CUDAsafebacktrackingfindlambda(double* energyLambda, double* currentenergy, double* fmax,
//                                               double* lambda, double* params,  bool* prev_convergence,
//                                               bool*  current_convergence){
//    if(prev_convergence[0]) return;
//    current_convergence[0] = false;
//    double energyChange = energyLambda[0] - currentenergy[0];
//    double LAMBDAREDUCE = params[1];
//    double LAMBDATOL = params[2];
//    double MAXDIST = params[4];
//    //return if ok
//    if(energyChange <= 0.0) {current_convergence[0] = true; return;}
//    //reduce lambda
//    lambda[0] *= LAMBDAREDUCE;
//
//    //just shake if we cant find an energy min,
//    //so we dont get stuck
//    if(lambda[0] <= 0.0 || lambda[0] <= LAMBDATOL) {current_convergence[0] = true; lambda[0] = MAXDIST / fmax[0];  }
//}


__global__ void getsafestateCUDA(double* FDotFA, double* curGrad, double* newGrad, bool* checkout){
    //if(checkin[0] == true) return;
    checkout[0] = false;
    if(FDotFA[0] <= 0.0 || areEqual(curGrad[0], newGrad[0]))
        checkout[0] = true;
//    printf("safe state %d \n", checkout[0]);
    curGrad[0] = newGrad[0];
}
__global__ void getminimizestateCUDA(double *fmax, double *GRADTOL, bool *checkin, bool *checkout) {
    //maxF() > GRADTOL
//    printf("minstate %f %f %d %d\n", fmax[0],GRADTOL[0],checkin[0],checkout[0]);
    checkout[0] = false;
    if(checkin[0] == false) return;
    if(fmax[0] > GRADTOL[0])
        checkout[0] = true;
    printf("minstate %f %f %d %d\n", fmax[0],GRADTOL[0],checkin[0],checkout[0]);
}

__global__ void initializePolak(bool* Mcheckin, bool *Mcheckout, bool *Scheckin, bool *Scheckout){
    Mcheckin[0] = true;
    Mcheckout[0] = true;
    Scheckin[0] = false;
    Scheckout[0] = false;
}
#endif
#endif //CUDA_VEC_CGMETHODCUDA_H
