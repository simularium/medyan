
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

#include "BoundaryCylinderRepulsionIn.h"

#include "BoundaryCylinderRepulsionExpIn.h"
#include "BoundaryElement.h"
#include "BoundaryElementImpl.h"

#include "Bead.h"
#include "Cylinder.h"

#include "MathFunctions.h"

using namespace mathfunc;

template <class BRepulsionInteractionType>
void BoundaryCylinderRepulsionIn<BRepulsionInteractionType>::vectorize() {

    //count interactions
    nint = 0;
    for (auto be: BoundaryElement::getBoundaryElements())
    {

        for(auto &c : _neighborList->getNeighbors(be))
        {
            if(c->isMinusEnd()) nint++;
            nint++;
        }
    }

    beadSet = new int[n * nint];
    krep = new double[nint];
    slen = new double[nint];
    auto beList = BoundaryElement::getBoundaryElements();

    int nbe = BoundaryElement::getBoundaryElements().size();
    int i = 0;
    int ni = 0;
    int bindex = 0;

    nneighbors = new int[nbe];//stores number of interactions per boundary element.
    double *beListplane;
    int *nintvec;
    beListplane = new double[4 * nbe];
    nintvec = new int[nbe];//stores cumulative number of nneighbors.

    int cumnn=0;
    for (i = 0; i < nbe; i++) {

        auto be = BoundaryElement::getBoundaryElements()[i];//beList[i];
        auto nn = _neighborList->getNeighbors(be).size();

        nneighbors[i] = 0;
        auto idx = 0;

        for (ni = 0; ni < nn; ni++) {
//            auto check=false;
            auto neighbor = _neighborList->getNeighbors(be)[ni];
            /*std::cout<<"Boundary with cindex "<<neighbor->_dcIndex<<" and ID "
                    ""<<neighbor->getID()<<" with bindices "<<neighbor->getFirstBead()
                    ->_dbIndex<<" "<<neighbor->getSecondBead()->_dbIndex<<endl;*/
            if(_neighborList->getNeighbors(be)[ni]->isMinusEnd())
            {
                bindex = _neighborList->getNeighbors(be)[ni]->getFirstBead()->_dbIndex;
                beadSet[cumnn+idx] = bindex;
                krep[cumnn+idx] = be->getRepulsionConst();
                slen[cumnn+idx] = be->getScreeningLength();
                idx++;
            }
            bindex = _neighborList->getNeighbors(be)[ni]->getSecondBead()->_dbIndex;
            beadSet[cumnn+idx] = bindex;
            krep[cumnn+idx] = be->getRepulsionConst();
            slen[cumnn+idx] = be->getScreeningLength();
            idx++;


//            if (_neighborList->getNeighbors(be)[ni]->isPlusEnd())
//            {bindex = _neighborList->getNeighbors(be)[ni]->getSecondBead()->_dbIndex;check=true;}
//            else if(_neighborList->getNeighbors(be)[ni]->isMinusEnd())
//            {bindex = _neighborList->getNeighbors(be)[ni]->getFirstBead()->_dbIndex;check=true;}
//                if(check){
//                    beadSet[cumnn+idx] = bindex;
//                    krep[cumnn+idx] = be->getRepulsionConst();
//                    slen[cumnn+idx] = be->getScreeningLength();
//                    idx++;
//                }
        }
        nneighbors[i]=idx;
        cumnn+=idx;
        nintvec[i] = cumnn;
        if(dynamic_cast<PlaneBoundaryElement*>(beList[i])) {
            double *x = new double[4];
            beList[i]->elementeqn(x);
            beListplane[4 * i] = x[0];
            beListplane[4 * i +1] = x[1];
            beListplane[4 * i +2] = x[2];
            beListplane[4 * i +3] = x[3];
            delete [] x;
        }
        else{
            cout<<"CUDA cannot handle non-plane type boundaries. Exiting..."<<endl;
            exit(EXIT_FAILURE);
        }
    }
#ifdef CUDAACCL
    #ifdef CUDATIMETRACK
    chrono::high_resolution_clock::time_point tbegin, tend;
    tbegin = chrono::high_resolution_clock::now();
#endif
    //CUDA stream create
    if(stream == NULL || !(CUDAcommon::getCUDAvars().conservestreams))
        CUDAcommon::handleerror(cudaStreamCreate(&stream));
    _FFType.optimalblocksnthreads(nint, stream);
    CUDAcommon::handleerror(cudaMalloc((void **) &gpu_beadSet, n * nint * sizeof(int)));
    CUDAcommon::handleerror(cudaMemcpyAsync(gpu_beadSet, beadSet, n * nint * sizeof(int),
                                        cudaMemcpyHostToDevice, stream));

    CUDAcommon::handleerror(cudaMalloc((void **) &gpu_krep, nint * sizeof(double)));
    CUDAcommon::handleerror(cudaMemcpyAsync(gpu_krep, krep, nint * sizeof(double),
                                        cudaMemcpyHostToDevice, stream));

    CUDAcommon::handleerror(cudaMalloc((void **) &gpu_slen, nint * sizeof(double)));
    CUDAcommon::handleerror(cudaMemcpyAsync(gpu_slen, slen, nint * sizeof(double),
                                        cudaMemcpyHostToDevice, stream));

    CUDAcommon::handleerror(cudaMalloc((void **) &gpu_nintperbe, nbe * sizeof(int)));
    CUDAcommon::handleerror(cudaMemcpyAsync(gpu_nintperbe, nintvec, nbe * sizeof(int),
                                        cudaMemcpyHostToDevice, stream));

    CUDAcommon::handleerror(cudaMalloc((void **) &gpu_beListplane, 4 * nbe * sizeof(double)));
    CUDAcommon::handleerror(cudaMemcpyAsync(gpu_beListplane, beListplane, 4 * nbe * sizeof
                                                                                        (double), cudaMemcpyHostToDevice));

    //PINNED memory to accelerate data transfer speed
//    CUDAcommon::handleerror(cudaHostAlloc((void**)&U_i, sizeof(double), cudaHostAllocDefault), "cudaHOstAlloc",
//                            "BoundaryCylinderRepulsion.cu");
//    CUDAcommon::handleerror(cudaMalloc((void **) &gU, sizeof(double)), "cudaMalloc", "BoundaryCylinderRepulsion.cu");
    vector<int> params;
    params.push_back(int(n));
    params.push_back(nint);
    params.push_back(CUDAcommon::cudavars.offset_E);
    //set offset
    CUDAcommon::cudavars.offset_E += nint;
//    std::cout<<"offset "<<getName()<<" "<<CUDAcommon::cudavars.offset_E<<endl;
    CUDAcommon::handleerror(cudaMalloc((void **) &gpu_params, 3 * sizeof(int)),"cuda data"
                                    " transfer",
                            "BoundaryCylinderRepulsion.cu");
    CUDAcommon::handleerror(cudaMemcpyAsync(gpu_params, params.data(), 3 * sizeof(int),
                                       cudaMemcpyHostToDevice, stream),
                            "cuda data transfer", "BoundaryCylinderRepulsion.cu");
#ifdef CUDATIMETRACK
//    CUDAcommon::handleerror(cudaDeviceSynchronize(),"BoundaryCylinderRepulsion.cu",
//                            "vectorizeFF");
    tend= chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_run(tend - tbegin);
    CUDAcommon::cudatime.TvecvectorizeFF.push_back(elapsed_run.count());
    CUDAcommon::cudatime.TvectorizeFF += elapsed_run.count();
#endif
#endif
    delete [] beListplane;
    delete [] nintvec;
}

template<class BRepulsionInteractionType>
void BoundaryCylinderRepulsionIn<BRepulsionInteractionType>::deallocate() {

    delete [] beadSet;
    delete [] krep;
    delete [] slen;
    delete [] nneighbors;

#ifdef CUDAACCL
    if(!(CUDAcommon::getCUDAvars().conservestreams))
        CUDAcommon::handleerror(cudaStreamDestroy(stream));
    _FFType.deallocate();
//    if(nint>0) {
//        CUDAcommon::handleerror(cudaStreamDestroy(stream));
        CUDAcommon::handleerror(cudaFree(gpu_beadSet));
        CUDAcommon::handleerror(cudaFree(gpu_krep));
        CUDAcommon::handleerror(cudaFree(gpu_slen));
        CUDAcommon::handleerror(cudaFree(gpu_beListplane));
        CUDAcommon::handleerror(cudaFree(gpu_nintperbe));
        CUDAcommon::handleerror(cudaFree(gpu_params));
//        CUDAcommon::handleerror(cudaFreeHost(U_i));
//        CUDAcommon::handleerror(cudaFree(gU));
//    }
#endif
}

template <class BRepulsionInteractionType>
double BoundaryCylinderRepulsionIn<BRepulsionInteractionType>::computeEnergy(double
*coord, double *f, double d) {
    double U_i[1], U_ii=0.0;
    double* gU_i;
    U_ii = 0.0;
//    std::cout<<"Total boundary nint "<<nint<<endl;
#ifdef CUDATIMETRACK
    chrono::high_resolution_clock::time_point tbegin, tend;
#endif
#ifdef CUDAACCL
    //    std::cout<<"Boundary nint "<<nint<<endl;
#ifdef CUDATIMETRACK
    tbegin = chrono::high_resolution_clock::now();
#endif
    //has to be changed to accomodate aux force
    double * gpu_coord=CUDAcommon::getCUDAvars().gpu_coord;
    double * gpu_force=CUDAcommon::getCUDAvars().gpu_force;
    double * gpu_d = CUDAcommon::getCUDAvars().gpu_lambda;

//    if(d == 0.0){
//        gU_i=_FFType.energy(gpu_coord, gpu_force, gpu_beadSet, gpu_krep, gpu_slen, gpu_nintperbe, gpu_beListplane,
//                            gpu_params);
//    }
//    else{
        gU_i=_FFType.energy(gpu_coord, gpu_force, gpu_beadSet, gpu_krep, gpu_slen, gpu_nintperbe, gpu_beListplane,
                            gpu_d, gpu_params);
//    }
#ifdef CUDATIMETRACK
//    CUDAcommon::handleerror(cudaDeviceSynchronize(),"BoundaryCylinderRepulsion.cu",
//                            "computeEnergy");
    tend= chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_run(tend - tbegin);
    CUDAcommon::cudatime.TveccomputeE.push_back(elapsed_run.count());
    CUDAcommon::cudatime.TcomputeE += elapsed_run.count();
#endif
#endif
#ifdef SERIAL
#ifdef CUDATIMETRACK
    tbegin = chrono::high_resolution_clock::now();
#endif
    if (d == 0.0) {
        U_ii = _FFType.energy(coord, f, beadSet, krep, slen, nneighbors);
    }
    else {
        U_ii = _FFType.energy(coord, f, beadSet, krep, slen, nneighbors, d);
    }
#ifdef CUDATIMETRACK
    tend= chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_runs(tend - tbegin);
    CUDAcommon::serltime.TveccomputeE.push_back(elapsed_runs.count());
    CUDAcommon::serltime.TcomputeE += elapsed_runs.count();
    CUDAcommon::serltime.TcomputeEiter += elapsed_runs.count();
#endif
#endif
    return U_ii;
}

template <class BRepulsionInteractionType>
void BoundaryCylinderRepulsionIn<BRepulsionInteractionType>::computeForces(double *coord,
        double *f) {
#ifdef CUDATIMETRACK
    chrono::high_resolution_clock::time_point tbegin, tend;
    tbegin = chrono::high_resolution_clock::now();
#endif

#ifdef CUDAACCL
    //has to be changed to accomodate aux force
    double * gpu_coord=CUDAcommon::getCUDAvars().gpu_coord;
    double * gpu_force;

    if(cross_checkclass::Aux){
        gpu_force=CUDAcommon::getCUDAvars().gpu_forceAux;
        _FFType.forces(gpu_coord, gpu_force, gpu_beadSet, gpu_krep, gpu_slen, gpu_nintperbe, gpu_beListplane,
                       gpu_params);
    }
    else {
        gpu_force = CUDAcommon::getCUDAvars().gpu_force;
        _FFType.forces(gpu_coord, gpu_force, gpu_beadSet, gpu_krep, gpu_slen, gpu_nintperbe, gpu_beListplane,
                       gpu_params);
    }

    //TODO remove this later need not copy forces back to CPU.
//    CUDAcommon::handleerror(cudaMemcpy(F_i, gpu_force, 3 * Bead::getBeads().size() *sizeof(double),
//                                       cudaMemcpyDeviceToHost),"cuda data transfer", "BoundaryCylinderRepulsion.cu");
#endif
#ifdef CUDATIMETRACK
    tend= chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_run(tend - tbegin);
    CUDAcommon::cudatime.TveccomputeF.push_back(elapsed_run.count());
    CUDAcommon::cudatime.TcomputeF += elapsed_run.count();
    tbegin = chrono::high_resolution_clock::now();
#endif
#ifdef SERIAL
    _FFType.forces(coord, f, beadSet, krep, slen, nneighbors);
#endif
#ifdef CUDATIMETRACK
    tend= chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_runs(tend - tbegin);
    CUDAcommon::serltime.TveccomputeF.push_back(elapsed_runs.count());
    CUDAcommon::serltime.TcomputeF += elapsed_runs.count();
#endif
#ifdef DETAILEDOUTPUT
    double maxF = 0.0;
    double mag = 0.0;
    for(int i = 0; i < CGMethod::N/3; i++) {
        mag = 0.0;
        for(int j = 0; j < 3; j++)
            mag += f[3 * i + j]*f[3 * i + j];
        mag = sqrt(mag);
//        std::cout<<"SL "<<i<<" "<<mag*mag<<" "<<forceAux[3 * i]<<" "<<forceAux[3 * i + 1]<<" "<<forceAux[3 * i +
//                2]<<endl;
        if(mag > maxF) maxF = mag;
    }
    std::cout<<"max "<<getName()<<" "<<maxF<<endl;
#endif
}

template <class BRepulsionInteractionType>
void BoundaryCylinderRepulsionIn<BRepulsionInteractionType>::computeLoadForces() {
//    std::cout<<"BOUNDARY REPULSION LOAD FORCES DOES NOT USE VECTORIZED FORCES/COORDINATES"<<endl;
    for (auto be: BoundaryElement::getBoundaryElements()) {

        for(auto &c : _neighborList->getNeighbors(be)) {

            double kRep = be->getRepulsionConst();
            double screenLength = be->getScreeningLength();


            //potential acts on second cylinder bead unless this is a minus end
            Bead* bd;
            Bead* bo;
            if(c->isPlusEnd()) {

                bd = c->getSecondBead();
                bo = c->getFirstBead();

                ///this normal is in the direction of polymerization
                auto normal = normalizeVector(twoPointDirection(bo->coordinate, bd->coordinate));

                //array of coordinate values to update
                auto monSize = SysParams::Geometry().monomerSize[bd->getType()];
                auto cylSize = SysParams::Geometry().cylinderNumMon[bd->getType()];

                bd->lfip = 0;
                for (int i = 0; i < cylSize; i++) {

                    auto newCoord = vector<double>{bd->coordinate[0] + i * normal[0] * monSize,
                                                   bd->coordinate[1] + i * normal[1] * monSize,
                                                   bd->coordinate[2] + i * normal[2] * monSize};

                    // Projection magnitude ratio on the direction of the cylinder
                    // (Effective monomer size) = (monomer size) * proj
                    double proj = -dotProduct(be->normal(newCoord), normal);
                    if(proj < 0.0) proj = 0.0;

                    double loadForce = _FFType.loadForces(be->distance(newCoord), kRep, screenLength);
                    // The load force stored in bead also considers effective monomer size.
                    bd->loadForcesP[bd->lfip++] += proj * loadForce;
                }
                //reset lfi
                bd->lfip = 0;
            }

            if(c->isMinusEnd()) {

                bd = c->getFirstBead();
                bo = c->getSecondBead();

                ///this normal is in the direction of polymerization
                auto normal = normalizeVector(twoPointDirection(bo->coordinate, bd->coordinate));

                //array of coordinate values to update
                auto monSize = SysParams::Geometry().monomerSize[bd->getType()];
                auto cylSize = SysParams::Geometry().cylinderNumMon[bd->getType()];


                bd->lfim = 0;
                for (int i = 0; i < cylSize; i++) {

                    auto newCoord = vector<double>{bd->coordinate[0] + i * normal[0] * monSize,
                                                   bd->coordinate[1] + i * normal[1] * monSize,
                                                   bd->coordinate[2] + i * normal[2] * monSize};

                    // Projection magnitude ratio on the direction of the cylinder
                    // (Effective monomer size) = (monomer size) * proj
                    double proj = -dotProduct(be->normal(newCoord), normal);
                    if(proj < 0.0) proj = 0.0;
                    double loadForce = _FFType.loadForces(be->distance(newCoord), kRep, screenLength);
                    // The load force stored in bead also considers effective monomer size.
                    bd->loadForcesM[bd->lfim++] += proj * loadForce;
                }
                //reset lfi
                bd->lfim = 0;
            }

        }

    }
}

//template <class BRepulsionInteractionType>
//double BoundaryCylinderRepulsionIn<BRepulsionInteractionType>::computeEnergy(double d) {
//
//    double U = 0;
//    double U_i;
//
//    for (auto be: BoundaryElement::getBoundaryElements()) {
//
//        for(auto &c : _neighborList->getNeighbors(be)) {
//
//            double kRep = be->getRepulsionConst();
//            double screenLength = be->getScreeningLength();
//
//            //potential acts on second bead unless this is a minus end
//            Bead* bd;
//            if(c->isMinusEnd()) {
//
//                bd = c->getFirstBead();
//
//                if (d == 0.0)
//                    U_i =  _FFType.energy(bd, be->distance(bd->coordinate), kRep, screenLength);
//                else
//                    U_i = _FFType.energy(bd, be->stretchedDistance(bd->coordinate, bd->force, d), kRep, screenLength);
//
//                if(fabs(U_i) == numeric_limits<double>::infinity()
//                   || U_i != U_i || U_i < -1.0) {
//
//                    //set culprits and return
//                    _otherCulprit = c;
//                    _boundaryElementCulprit = be;
//
//                    return -1;
//                }
//                else {
//                    U += U_i;
//                }
//
//
//                bd = c->getSecondBead();
//
//                if (d == 0.0)
//                    U_i =  _FFType.energy(bd, be->distance(bd->coordinate), kRep, screenLength);
//                else
//                    U_i = _FFType.energy(bd, be->stretchedDistance(bd->coordinate, bd->force, d), kRep, screenLength);
//
//                if(fabs(U_i) == numeric_limits<double>::infinity()
//                   || U_i != U_i || U_i < -1.0) {
//
//                    //set culprits and return
//                    _otherCulprit = c;
//                    _boundaryElementCulprit = be;
//
//                    return -1;
//                }
//                else {
//                    U += U_i;
//                }
//
//            }
//
//            else {
//                bd = c->getSecondBead();
//
//                if (d == 0.0)
//                    U_i =  _FFType.energy(bd, be->distance(bd->coordinate), kRep, screenLength);
//                else
//                    U_i = _FFType.energy(bd, be->stretchedDistance(bd->coordinate, bd->force, d), kRep, screenLength);
//
//                if(fabs(U_i) == numeric_limits<double>::infinity()
//                   || U_i != U_i || U_i < -1.0) {
//
//                    //set culprits and return
//                    _otherCulprit = c;
//                    _boundaryElementCulprit = be;
//
//                    return -1;
//                }
//                else {
//                    U += U_i;
//                }
//            }
//        }
//    }
//
//    return U;
//}
//
//template <class BRepulsionInteractionType>
//void BoundaryCylinderRepulsionIn<BRepulsionInteractionType>::computeForces() {
//
//    for (auto be: BoundaryElement::getBoundaryElements()) {
//
//        for(auto &c: _neighborList->getNeighbors(be)) {
//
//            double kRep = be->getRepulsionConst();
//            double screenLength = be->getScreeningLength();
//
//            //potential acts on second cylinder bead unless this is a minus end
//            Bead* bd;
//            if(c->isMinusEnd()) {
//                bd = c->getFirstBead();
//                auto normal = be->normal(bd->coordinate);
//                _FFType.forces(bd, be->distance(bd->coordinate), normal, kRep, screenLength);
//
//                bd = c->getSecondBead();
//                normal = be->normal(bd->coordinate);
//                _FFType.forces(bd, be->distance(bd->coordinate), normal, kRep, screenLength);
//            }
//            else {
//                bd = c->getSecondBead();
//                auto normal = be->normal(bd->coordinate);
//                _FFType.forces(bd, be->distance(bd->coordinate), normal, kRep, screenLength);
//            }
//
//        }
//    }
//}
//
//
//template <class BRepulsionInteractionType>
//void BoundaryCylinderRepulsionIn<BRepulsionInteractionType>::computeForcesAux() {
//
//    for (auto be: BoundaryElement::getBoundaryElements()) {
//
//        for(auto &c : _neighborList->getNeighbors(be)) {
//
//            double kRep = be->getRepulsionConst();
//            double screenLength = be->getScreeningLength();
//
//            //potential acts on second cylinder bead unless this is a minus end, then we compute forces
//            //for both first and second bead
//            Bead* bd;
//            if(c->isMinusEnd()) {
//                bd = c->getFirstBead();
//                auto normal = be->normal(bd->coordinate);
//                _FFType.forcesAux(bd, be->distance(bd->coordinate), normal, kRep, screenLength);
//
//                bd = c->getSecondBead();
//                normal = be->normal(bd->coordinate);
//                _FFType.forcesAux(bd, be->distance(bd->coordinate), normal, kRep, screenLength);
//            }
//            else {
//                bd = c->getSecondBead();
//                auto normal = be->normal(bd->coordinate);
//                _FFType.forcesAux(bd, be->distance(bd->coordinate), normal, kRep, screenLength);
//            }
//        }
//    }
//}
//
//template <class BRepulsionInteractionType>
//void BoundaryCylinderRepulsionIn<BRepulsionInteractionType>::computeLoadForces() {
//
//    for (auto be: BoundaryElement::getBoundaryElements()) {
//
//        for(auto &c : _neighborList->getNeighbors(be)) {
//
//            double kRep = be->getRepulsionConst();
//            double screenLength = be->getScreeningLength();
//
//
//            //potential acts on second cylinder bead unless this is a minus end
//            Bead* bd;
//            Bead* bo;
//            if(c->isPlusEnd()) {
//
//                bd = c->getSecondBead();
//                bo = c->getFirstBead();
//
//                ///this normal is in the direction of polymerization
//                auto normal = normalizedVector(twoPointDirection(bo->coordinate, bd->coordinate));
//
//                //array of coordinate values to update
//                auto monSize = SysParams::Geometry().monomerSize[bd->getType()];
//                auto cylSize = SysParams::Geometry().cylinderNumMon[bd->getType()];
//
//                bd->lfip = 0;
//                for (int i = 0; i < cylSize; i++) {
//
//                    auto newCoord = vector<double>{bd->coordinate[0] + i * normal[0] * monSize,
//                        bd->coordinate[1] + i * normal[1] * monSize,
//                        bd->coordinate[2] + i * normal[2] * monSize};
//
//                    double loadForce = _FFType.loadForces(be->distance(newCoord), kRep, screenLength);
//                    bd->loadForcesP[bd->lfip++] += loadForce;
//                }
//                //reset lfi
//                bd->lfip = 0;
//            }
//
//            if(c->isMinusEnd()) {
//
//                bd = c->getFirstBead();
//                bo = c->getSecondBead();
//
//                ///this normal is in the direction of polymerization
//                auto normal = normalizedVector(twoPointDirection(bo->coordinate, bd->coordinate));
//
//                //array of coordinate values to update
//                auto monSize = SysParams::Geometry().monomerSize[bd->getType()];
//                auto cylSize = SysParams::Geometry().cylinderNumMon[bd->getType()];
//
//
//                bd->lfim = 0;
//                for (int i = 0; i < cylSize; i++) {
//
//                    auto newCoord = vector<double>{bd->coordinate[0] + i * normal[0] * monSize,
//                        bd->coordinate[1] + i * normal[1] * monSize,
//                        bd->coordinate[2] + i * normal[2] * monSize};
//
//                    double loadForce = _FFType.loadForces(be->distance(newCoord), kRep, screenLength);
//                    bd->loadForcesM[bd->lfim++] += loadForce;
//                }
//                //reset lfi
//                bd->lfim = 0;
//            }
//
//        }
//    }
//}


///Template specializations
template double BoundaryCylinderRepulsionIn<BoundaryCylinderRepulsionExpIn>::computeEnergy
        (double *coord, double *f, double d);
template void BoundaryCylinderRepulsionIn<BoundaryCylinderRepulsionExpIn>::computeForces
        (double *coord, double *f);
template void BoundaryCylinderRepulsionIn<BoundaryCylinderRepulsionExpIn
>::computeLoadForces();
template void BoundaryCylinderRepulsionIn<BoundaryCylinderRepulsionExpIn>::vectorize();
template void BoundaryCylinderRepulsionIn<BoundaryCylinderRepulsionExpIn>::deallocate();
//template double BoundaryCylinderRepulsionIn<BoundaryCylinderRepulsionExpIn>::computeEnergy(double d);
//template void BoundaryCylinderRepulsionIn<BoundaryCylinderRepulsionExpIn>::computeForces();
//template void BoundaryCylinderRepulsionIn<BoundaryCylinderRepulsionExpIn>::computeForcesAux();
//template void BoundaryCylinderRepulsionIn<BoundaryCylinderRepulsionExpIn>::computeLoadForces();

