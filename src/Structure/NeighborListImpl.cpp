
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

#include "NeighborListImpl.h"

#include "Bead.h"
#include "Filament.h"
#include "Cylinder.h"
#include "Bubble.h"
#include "BoundaryElement.h"

#include "GController.h"
#include "MathFunctions.h"
#ifdef CUDAACCL
#include "CUDAcommon.h"
#endif
#include "NeighborListImplCUDA.h"
#ifdef CUDAACCL
#include "nvToolsExt.h"
#endif
using namespace mathfunc;
#ifdef NLSTENCILLIST
void CylinderCylinderNL::updateallcylinderstobin() {
    for(auto cyl:Cylinder::getCylinders())
        updatebin(cyl);
}
void CylinderCylinderNL::assignallcylinderstobin() {
    for(auto cyl:Cylinder::getCylinders())
        assignbin(cyl);
}
void CylinderCylinderNL::assignbin(Cylinder* cyl){
    Bin* _bin;
    try {_bin = getBin(cyl->coordinate);}
    catch (exception& e) {
        cout << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    _bin->addCylinder(cyl);
    cyl->_binvec.push_back(_bin);
}
void CylinderCylinderNL::unassignbin(Cylinder* cyl, Bin* bin){
    bin->removeCylinder(cyl);
}
void CylinderCylinderNL::updatebin(Cylinder *cyl){
    Bin* _bin;
//    std::cout<<coordinate[0]<<" "<<coordinate[1]<<" "<<coordinate[2]<<endl;
    try {_bin = getBin(cyl->coordinate);}
    catch (exception& e) {
        cout << e.what();
        cyl->printSelf();
        exit(EXIT_FAILURE);
    }

    if(_bin != cyl->_binvec.at(_ID)) {

#ifdef CHEMISTRY
        auto oldBin = cyl->_binvec.at(_ID);
        auto newBin = _bin;
#endif

        //remove from old compartment, add to new
        oldBin->removeCylinder(cyl);
        cyl->_binvec.at(_ID) = newBin;
        _bin->addCylinder(cyl);
    }
}


void CylinderCylinderNL::generateConnections()
{
    for(size_t i=0U; i<_grid[0]; ++i) {

        for(size_t j=0U; j<_grid[1]; ++j) {

            for(size_t k=0U; k<_grid[2]; ++k) {
                vector<size_t> indices{i,j,k};
                Bin *target = getBin(indices);

                vector<double> coordinates =
                        {indices[0] * _binSize[0] + _binSize[0] / 2,
                         indices[1] * _binSize[1] + _binSize[1] / 2,
                         indices[2] * _binSize[2] + _binSize[2] / 2};
                std::cout<<coordinates[0]<<" "<<coordinates[1]<<" "<<coordinates[2]<<endl;
                target->setCoordinates(coordinates);
                //Go through all neighbors to get the neighbors list
//                std::cout<<"ijk "<<i<<" "<<j<<" "<<k<<endl;
                for(int ii: {-1,0,1}){
                    for(int jj: {-1,0,1}){
                        for(int kk: {-1,0,1}){
                            //Consider the target bin itself as a neighbor.
//                            if(ii != 0 || jj != 0 || kk != 0) {
                                int iprime = i+ii;
                                int jprime = j+jj;
                                int kprime = k+kk;
                                if(iprime<0 or iprime==int(_grid[0]) or jprime<0 or
                                   jprime==int(_grid[1]) or kprime<0 or
                                   kprime==int(_grid[2]))
                                    continue;
                                vector<size_t> currentIndices{size_t(iprime), size_t
                                        (jprime), size_t(kprime)};
//                                std::cout<<"CI "<<currentIndices[0]<<" "
//                                        ""<<currentIndices[1]<<" "<<currentIndices[2]<<endl;
                                Bin *neighbor = getBin(currentIndices);
//                            if(ii==0 && jj == 0 &&  kk == 0 )
//                            if(neighbor->coordinates()[0] == target->coordinates()[0] &&
//                                    neighbor->coordinates()[1] == target->coordinates()
//                                                                  [1] &&
//                                    neighbor->coordinates()[2] == target->coordinates()[2])
//                                std::cout<<"Self bin added"<<endl;
                                target->addNeighbour(neighbor);
//                            }
                        }
                    }
                }
//                // one neighbor to the left and right
//                for(int ii: {-1,1})
//                {
//                    int iprime = i+ii;
//                    if(iprime<0 or iprime==int(_grid[0]))
//                        continue;
//                    vector<size_t> currentIndices{size_t(iprime), j, k};
//                    Bin *neighbor = getBin(currentIndices);
//                    target->addNeighbour(neighbor);
//                }
//                //one neighbor to the front and back
//                for(int jj: {-1,1})
//                {
//                    int jprime = j+jj;
//                    if(jprime<0 or jprime==int(_grid[1]))
//                        continue;
//                    vector<size_t> currentIndices{i, size_t(jprime), k};
//                    Bin *neighbor = getBin(currentIndices);
//                    target->addNeighbour(neighbor);
//                }
//                //one neighbor to the top and bottom
//                for(int kk: {-1,1})
//                {
//                    int kprime = k+kk;
//                    if(kprime<0 or kprime==int(_grid[2]))
//                        continue;
//                    vector<size_t> currentIndices{i, j, size_t(kprime)};
//                    Bin *neighbor = getBin(currentIndices);
//                    target->addNeighbour(neighbor);
//                }
            }
        }
    }

}

void CylinderCylinderNL::initializeBinGrid() {

//    //Initial parameters of system
    auto _nDim = SysParams::Geometry().nDim;
    double searchdist = 1.125 * (maxcylsize + _rMax);
    _binSize = {searchdist, searchdist, searchdist};
    if(_nDim >=1) {
        _size.push_back(int(SysParams::Geometry().NX * SysParams::Geometry()
                .compartmentSizeX));
        if( (_size[0]) % int(_binSize[0]) ==0)
            _grid.push_back(_size[0]/_binSize[0]);
        else
            _grid.push_back(_size[0]/_binSize[0] + 1);
    }
    if (_nDim >= 2) {
        _size.push_back(int(SysParams::Geometry().NY * SysParams::Geometry()
                .compartmentSizeY));
        if( (_size[1]) % int(_binSize[1]) ==0)
            _grid.push_back(_size[1]/_binSize[1]);
        else
            _grid.push_back(_size[1]/_binSize[1] + 1);
    }
    if (_nDim == 3) {
        _size.push_back(int(SysParams::Geometry().NZ * SysParams::Geometry()
                .compartmentSizeZ));
        if( (_size[2]) % int(_binSize[2]) ==0)
            _grid.push_back(_size[2]/_binSize[2]);
        else
            _grid.push_back(_size[2]/_binSize[2] + 1);
    }

    //Check that grid and compartmentSize match nDim
    if((_nDim == 3 &&
        _grid[0] != 0 && _grid[1] != 0 && _grid[2]!=0 &&
        _binSize[0] != 0 &&
        _binSize[1] != 0 &&
        _binSize[2] != 0)){
    }
    else {
        cout << "Bin parameters for CylinderCylinderNeighborLists are invalid. Exiting." <<
             endl;
        exit(EXIT_FAILURE);
    }
    int size = 1;
    for(auto x: _grid) {
        if(x != 0) size*=x;
    }
    //Set the instance of this grid with given parameters
    _binGrid = new BinGrid(size, _ID);
    //Create connections based on dimensionality
    generateConnections();
}

//You need a vector of all grids so you can loop through and update respective coordinates.
Bin* CylinderCylinderNL::getBin(const vector<double> &coords)
{
    //Check if out of bounds
    size_t index = 0;
    size_t i = 0;
    for(auto x: coords)
    {
        //Flatten the coordinates to 1D, get integer index
        if(i == 0) {
            if(x < 0 || x >= (_binSize[0] * _grid[0]))
                throw OutOfBoundsException();

            index += int(x / _binSize[0]);
        }
        else if(i == 1) {
            if(x < 0 || x >= (_binSize[1] * _grid[1]))
                throw OutOfBoundsException();

            index += int(x / _binSize[1]) * _grid[0];
        }
        else {
            if(x < 0 || x >= (_binSize[2] * _grid[2]))
                throw OutOfBoundsException();

            index += int(x / _binSize[2]) * _grid[0] * _grid[1];
        }
        i++;
    }

    try {
        return _binGrid->getBin(index);
    }
    catch (exception& e){
        cout << "Bad bin access at..." << endl;
        cout << "Bin index = " << index << endl;
        cout << "Coords = " << coords[0] << " " << coords[1] << " " << coords[2] << endl;
        throw NaNCoordinateException();
    }
}


Bin* CylinderCylinderNL::getBin(const vector<size_t> &indices)
{
    size_t index = 0;
    size_t i = 0;
    for(auto x: indices)
    {
        //Flatten the indices to 1D
        if(i == 0) {
            if(x >= _grid[0])
                throw OutOfBoundsException();

            index += x;
        }
        else if(i == 1) {
            if(x >= _grid[1])
                throw OutOfBoundsException();

            index += x * _grid[0];
        }
        else {
            if(x >= _grid[2])
                throw OutOfBoundsException();

            index += x * _grid[0] * _grid[1];
        }

        i++;
    }
    try {
        return _binGrid->getBin(index);
    }
    catch (exception& e){
        cout << "Bad Bin access at..." << endl;
        cout << "Bin index = " << index << endl;
        cout << "Indices = " << indices[0] << " " << indices[1] << " " << indices[2] << endl;
        throw NaNCoordinateException();
    }
}

void CylinderCylinderNL::updateNeighborsbin(Cylinder* cylinder, bool runtime){
//    if(runtime)
//        std::cout<<"Runtime is true "<<runtime<<endl;
//    else
//        std::cout<<"Runtime is false "<<runtime<<endl;
    //clear existing
    _list4mbin[cylinder].clear();
    auto binvec = cylinder->_binvec;//The different bins that this cylinder belongs to.
    if(binvec.size()<=_ID)
        assignbin(cylinder);
    binvec = cylinder->_binvec;
    auto parentbin =  binvec.at(_ID);
    vector<Bin*> _neighboringBins = binvec.at(_ID)//Get the bin that belongs to the
                    // current binGrid of interest for this NL.
            ->getNeighbours();
    for(auto &bin : _neighboringBins){
//        std::cout<<bin->getCylinders().size()<<endl;
        for(auto &ncylinder : bin->getCylinders()) {

            bool checkstatus = false;
//            std::cout<<"parent "<<cylinder->getType()<<endl;
//            std::cout<<"neighbor "<<ncylinder->getID()<<endl;
//            ncylinder->printSelf();
            if ((cylinder->getType() == NLcyltypes[0] &&
                 ncylinder->getType() == NLcyltypes[1]) || (cylinder->getType() ==
                                                            NLcyltypes[1] &&
                                                            ncylinder->getType() ==
                                                            NLcyltypes[0])) {
                checkstatus = true;
            }
            if (checkstatus) {
                //Don't add the same cylinder!
                if (cylinder == ncylinder) continue;

                //Dont add if ID is more than cylinder for half-list
                if (!_full && cylinder->getID() <= ncylinder->getID()) continue;

                //Don't add if belonging to same parent
                if (cylinder->getParent() == ncylinder->getParent()) {

                    //if not cross filament, check if not neighboring
                    auto dist = fabs(cylinder->getPosition() -
                                     ncylinder->getPosition());
                    if (dist <= 2) continue;
                }
                //Dont add if not within range
                double dist = twoPointDistance(cylinder->coordinate,
                                               ncylinder->coordinate);
                if (dist > _rMax || dist < _rMin) continue;
//            std::cout<<"V "<<cylinder->_dcIndex<<" "<<ncylinder->_dcIndex<<" "<<dist<<" "<<_rMin<<" "<<_rMax<<endl;
                //If we got through all of this, add it!
                _list4mbin[cylinder].push_back(ncylinder);

                //if runtime, add to other list as well if full
                if (runtime && _full)
                    _list4mbin[ncylinder].push_back(cylinder);
            }
        }

    }
}
vector<Cylinder*> CylinderCylinderNL::getNeighborsstencil(Cylinder* cylinder) {

    return _list4mbin[cylinder];
}
#endif

void CylinderCylinderNL::updateNeighbors(Cylinder* cylinder, bool runtime) {
//    if(runtime)
//        std::cout<<"Runtime is true "<<runtime<<endl;
//    else
//        std::cout<<"Runtime is false "<<runtime<<endl;

    //clear existing
    _list[cylinder].clear();

    //Find surrounding compartments (For now its conservative)
    vector<Compartment*> compartments;
    auto searchDist = SysParams::Geometry().largestCompartmentSide;

    GController::findCompartments(cylinder->coordinate,
                                  cylinder->getCompartment(),
                                  searchDist + _rMax, compartments);
    for(auto &comp : compartments) {
        for(auto &ncylinder : comp->getCylinders()) {

            //Don't add the same cylinder!
            if(cylinder == ncylinder) continue;

            //Dont add if ID is more than cylinder for half-list
            if(!_full && cylinder->getID() <= ncylinder->getID()) continue;

            //Don't add if belonging to same parent
            if(cylinder->getParent() == ncylinder->getParent()) {

                //if not cross filament, check if not neighboring
                auto dist = fabs(cylinder->getPosition() -
                                 ncylinder->getPosition());
                if(dist <= 2) continue;
            }
            //Dont add if not within range
            double distsq = twoPointDistancesquared(cylinder->coordinate,
                                           ncylinder->coordinate);
            if(distsq > (_rMax * _rMax) || distsq < (_rMin * _rMin)) continue;

            //If we got through all of this, add it!
            _list[cylinder].push_back(ncylinder);

            //if runtime, add to other list as well if full
            if(runtime && _full)
                _list[ncylinder].push_back(cylinder);
        }
    }
}

void CylinderCylinderNL::addNeighbor(Neighbor* n) {

    //return if not a cylinder!
    Cylinder* cylinder;
    if(!(cylinder = dynamic_cast<Cylinder*>(n))) return;

    //update neighbors
#ifdef NLORIGINAL
    updateNeighbors(cylinder, true);
#endif
#ifdef NLSTENCILLIST
    updateNeighborsbin(cylinder, true);
#endif
}

void CylinderCylinderNL::removeNeighbor(Neighbor* n) {

    Cylinder* cylinder;
    if(!(cylinder = dynamic_cast<Cylinder*>(n))) return;
#ifdef NLORIGINAL
    _list.erase(cylinder);

    //remove from other lists
    for(auto it = _list.begin(); it != _list.end(); it++) {

        auto cit = find(it->second.begin(), it->second.end(), cylinder);
        if(cit != it->second.end()) it->second.erase(cit);
    }
#endif
#ifdef NLSTENCILLIST
    std::cout<<"Removing neighbors of "<<cylinder<<" from NL "<<_ID<<endl;
    //Remove from NeighborList
    _list4mbin.erase(cylinder);
    //Remove from bin
    Bin* bin = cylinder->_binvec.at(_ID);
    unassignbin(cylinder, bin);
    //remove from other lists
    std::cout<<"Removed from cylinders ";
    for(auto it = _list4mbin.begin(); it != _list4mbin.end(); it++) {
        auto cit = find(it->second.begin(), it->second.end(), cylinder);
        {
            if (cit != it->second.end()) {
                it->second.erase(cit);
                std::cout<<it->first<<" ";
            }
        }
    }
    std::cout<<endl;
#endif
}

void CylinderCylinderNL::reset() {

#ifdef CUDAACCL_NL
    _list.clear();
//    nvtxRangePushA("NL_Prep_NeighborListImpl1");
//    pair_cIndex_cmp.clear();
    nint = 0;
    pair_cIndex_cnIndex.clear();
    //1. Get total interactions
    for(auto cylinder: Cylinder::getCylinders()) {
        auto searchDist = SysParams::Geometry().largestCompartmentSide;
        vector<Compartment *> compartments;
        GController::findCompartments(cylinder->coordinate,
                                      cylinder->getCompartment(),
                                      searchDist + _rMax, compartments);
        for (auto c:compartments) {
            nint += c->getCylinders().size();
        }
    }
    //2. Assign optimal blocks and threads
    int blockSize;   // The launch configurator returned block size
    int minGridSize; // The minimum grid size needed to achieve the maximum occupancy for a full device launch
    if(nint>0) {
        cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize,
                                                       CylinderCylinderNLCUDA, 0, 0);
        blocksnthreads.clear();
        blocksnthreads.push_back((nint + blockSize - 1) / blockSize);
        blocksnthreads.push_back(blockSize);
        std::cout<<"NL blocks and threads "<<blocksnthreads.at(0)<<" "<<blocksnthreads.at(1)<<endl;
    }
//    nvtxRangePop();
#endif
    //loop through all neighbor keys
#ifdef NLORIGINAL//serial
    _list.clear();
    int tot = 0;
    for(auto cylinder: Cylinder::getCylinders()) {
        updateNeighbors(cylinder);
        tot += _list[cylinder].size();
    }
   // std::cout<<"NLORIGINAL size "<<" "<<tot<<endl;
    tot = 0;
#endif
#ifdef NLSTENCILLIST
    _list4mbin.clear();
    //check and reassign cylinders to different bins if needed.
    updateallcylinderstobin();
    for(auto cylinder: Cylinder::getCylinders()) {
        updateNeighborsbin(cylinder);
        tot += _list4mbin[cylinder].size();
    }
    //std::cout<<"NLSTENCILLIST size "<<" "<<tot<<endl;
#endif
//        std::cout<<cylinder->_dcIndex<<" "<<_list.size()<<" "<<vec_numpairs<<" "<<_full<<endl;
#ifdef CUDAACCL_NL
//    nvtxRangePushA("NL_Prep_NeighborListImpl2");
        for(auto cylinder: Cylinder::getCylinders()) {
        //Find surrounding compartments (For now its conservative)
        vector<Compartment*> compartments;
        auto searchDist = SysParams::Geometry().largestCompartmentSide;

        GController::findCompartments(cylinder->coordinate,
                                      cylinder->getCompartment(),
                                      searchDist + _rMax, compartments);
        for (auto c:compartments) {
            for(auto ncyl:c->getCylinders()){
                pair_cIndex_cnIndex.push_back(cylinder->_dcIndex);
                pair_cIndex_cnIndex.push_back(ncyl->_dcIndex);
            }
//            pair_cIndex_cmp.push_back(cylinder->_dcIndex);
//            pair_cIndex_cmp.push_back(GController::getCompartmentID(c->coordinates()));
        }
    }
    //get total number of pairs.
    int vec_numpairs =0;//TODO remove later
//    for(auto cylinder: Cylinder::getCylinders()) {
//        vec_numpairs += _list[cylinder].size();
//    }
    std::cout<<pair_cIndex_cnIndex.size()<<" "<<nint<<endl;
//    nvtxRangePop();
//    nvtxRangePushA("NL_Prep_NeighborListImpl4");
//    int *cpu_pair_cIndex_cnIndex;
//    cpu_pair_cIndex_cnIndex = new int[pair_cIndex_cnIndex.size()];
//    for (auto i = 0; i < pair_cIndex_cnIndex.size(); i++)
//        cpu_pair_cIndex_cnIndex[i] = pair_cIndex_cnIndex.at(i);
//    nvtxRangePop();
//    nvtxRangePushA("NL_Prep_NeighborListImpl3");
//    int cpu_pair_cIndex_cmp[pair_cIndex_cmp.size()];
//    for (auto i = 0; i < pair_cIndex_cmp.size(); i++)
//        cpu_pair_cIndex_cmp[i] = pair_cIndex_cmp.at(i);
//    CUDAcommon::handleerror(cudaMalloc((void **) &gpu_pair_cIndex_cmp,  pair_cIndex_cmp.size() * sizeof(int)),
//                                "cuda data transfer", " NeighborListImpl.h");
//    CUDAcommon::handleerror(cudaMemcpy(gpu_pair_cIndex_cmp, cpu_pair_cIndex_cmp, pair_cIndex_cmp.size()
//                                           *sizeof(int), cudaMemcpyHostToDevice));
    CUDAcommon::handleerror(cudaMalloc((void **) &gpu_pair_cIndex_cnIndex,  pair_cIndex_cnIndex.size() * sizeof(int)),
                            "cuda data transfer", " NeighborListImpl.h");

    CUDAcommon::handleerror(cudaMemcpy(gpu_pair_cIndex_cnIndex, pair_cIndex_cnIndex.data(), pair_cIndex_cnIndex.size()
                                                                                 *sizeof(int), cudaMemcpyHostToDevice));
//    delete cpu_pair_cIndex_cnIndex;
    CUDAcommon::handleerror(cudaMalloc((void **) &gpu_NL, 2 * nint * sizeof(int)));

    int cpu_params2[2];
    cpu_params2[0] = nint;
    cpu_params2[1] = int(_full);
    CUDAcommon::handleerror(cudaMalloc((void **) &gpu_params2, 2 * sizeof(int)));
    CUDAcommon::handleerror(cudaMemcpy(gpu_params2, cpu_params2, 2 * sizeof(int), cudaMemcpyHostToDevice));
//    if(gpu_numpairs == NULL) {
//    numpairs[0] = 0;
        CUDAcommon::handleerror(cudaMalloc((void **) &gpu_numpairs, sizeof(int)));
//    CUDAcommon::handleerror(cudaMemcpy(gpu_numpairs, numpairs, 1 * sizeof(int), cudaMemcpyHostToDevice));
//    }

//        int *a;
//        a = new int[1];
//        a[0] = 0;
//        int *gpu_a;
//        CUDAcommon::handleerror(cudaMalloc((void **) &gpu_a,  sizeof(int)),
//                                "cuda data transfer", " NeighborListImpl.h");
//        CUDAcommon::handleerror(cudaMemcpy(gpu_a, a, sizeof(int), cudaMemcpyHostToDevice));
//        testfunction<<<1,1>>>(gpu_a);

        //Gather all necessary variables.
    auto cylcylnlvars = CUDAcommon::getCylCylNLvars();
    double* coord_com = cylcylnlvars.gpu_coord_com;
    int *beadSet = cylcylnlvars.gpu_beadSet;
    int *cylID = cylcylnlvars.gpu_cylID;
    int *filID = cylcylnlvars.gpu_filID;
    int *cmpIDlist = cylcylnlvars.gpu_cmpID;
    int *fvecposition = cylcylnlvars.gpu_fvecpos;
//    int *cylvecpospercmp = cylcylnlvars.gpu_cylvecpospercmp;
//    if(gpu_params == NULL) {
        double cpu_params[2];
        cpu_params[0] = double(_rMin);
        cpu_params[1] = double(_rMax);
        CUDAcommon::handleerror(cudaMalloc((void **) &gpu_params,  2 * sizeof(double)),
                                "cuda data transfer", " NeighborListImpl.h");
        CUDAcommon::handleerror(cudaMemcpy(gpu_params, cpu_params, 2 * sizeof(double), cudaMemcpyHostToDevice));
//    }
//    nvtxRangePop();
//    std::cout<<_rMin<<" "<<_rMax<<endl;
//    nvtxRangePushA("NL_Exec_NeighborListImpl");
    resetintvariableCUDA<<<1,1>>>(gpu_numpairs);
    CylinderCylinderNLCUDA<<<blocksnthreads[0],blocksnthreads[1]>>> (coord_com, beadSet, cylID, filID, cmpIDlist,
            fvecposition, gpu_pair_cIndex_cnIndex, gpu_params, gpu_numpairs, gpu_NL, gpu_params2);
//    CUDAcommon::handleerror(cudaDeviceSynchronize());
    //TODO make this Async and synchronize stream before binding manager call.
    cudaMemcpy(numpairs, gpu_numpairs,  sizeof(int), cudaMemcpyDeviceToHost);
//    nvtxRangePop();
    std::cout<<"Number of neighbors "<<numpairs[0]<<" "<<vec_numpairs<<" Full "<<_full<<endl;
//    nvtxRangePushA("NL_End_NeighborListImpl");
    if(true){
        //copy forces back to CUDA
//        cudaMemcpy(numpairs, gpu_numpairs,  sizeof(int), cudaMemcpyDeviceToHost);
//        std::cout<<"Number of neighbors "<<numpairs[0]<<" "<<vec_numpairs<<" Full "<<_full<<endl;
        int *NL;
        NL = new int[2 * numpairs[0]];
    cudaMemcpy(NL, gpu_NL, 2 * numpairs[0] * sizeof(int), cudaMemcpyDeviceToHost);

//    for(auto ii = 0; ii < numpairs[0]; ii++)
//        std::cout<<NL[2*ii]<<" "<<NL[2*ii+1]<<endl;
//    std::cout<<"-----------------------------------------------------"<<endl;
//    for(auto c:Cylinder::getCylinders()){
//        for(auto cn:_list[c])
//            std::cout<<c->_dcIndex<<" "<<cn->_dcIndex<<endl;
//    }
//    std::cout<<endl;

        //set neighborlist.
        for(auto c:Cylinder::getCylinders()){
            _list[c].clear();
        }
        for(auto id = 0; id < numpairs[0]; id++){
            Cylinder *cylinder = NULL;
            Cylinder *ncylinder = NULL;
            cylinder = Cylinder::getCylinders()[NL[2 * id]];
            ncylinder = Cylinder::getCylinders()[NL[2 * id + 1]];
//            std::cout<<cylinder->_dcIndex<<" "<<NL[2 * id]<<" "<<ncylinder->_dcIndex<<" "<<NL[2 * id +1]<<endl;
//            for(auto c:Cylinder::getCylinders()){
//                if(c->_dcIndex == NL[2 * id])
//                    cylinder = c;
//                else if(c->_dcIndex == NL[2 * id +1])
//                    ncylinder = c;
//                if(cylinder != NULL && ncylinder != NULL)
//                    _list[cylinder].push_back(ncylinder);
//            }

            if(cylinder == NULL || ncylinder == NULL || cylinder->_dcIndex != NL[2 * id] || ncylinder->_dcIndex !=
               NL[2 * id + 1]) {
                cout << "Error. Could not find neighborlist from CUDA in Cylinder Database. Check Cylinder IDs Exiting."
                        "." << endl;
                exit(EXIT_FAILURE);
            }
            else{
                _list[cylinder].push_back(ncylinder);
            }
        }
        if(cudacpyforces) {
            CUDAcommon::handleerror(cudaFree(gpu_NL), "cudaFree", "NeighborListImpl.cu");
            CUDAcommon::handleerror(cudaFree(gpu_numpairs), "cudaFree", "NeighborListImpl.cu");
        }
        delete NL;
    }
    CUDAcommon::handleerror(cudaFree(gpu_pair_cIndex_cnIndex),"cudaFree","NeighborListImpl.cu");
    CUDAcommon::handleerror(cudaFree(gpu_params2),"cudaFree","NeighborListImpl.cu");

    CUDAcommon::handleerror(cudaFree(gpu_params),"cudaFree","NeighborListImpl.cu");
//    nvtxRangePop();
#endif
}


vector<Cylinder*> CylinderCylinderNL::getNeighbors(Cylinder* cylinder) {

    return _list[cylinder];
}


//BOUNDARYELEMENT - CYLINDER

void BoundaryCylinderNL::updateNeighbors(BoundaryElement* be) {

    //clear existing
    _list[be].clear();

    //loop through beads, add as neighbor
    for (auto &c : Cylinder::getCylinders()) {

        double dist = be->distance(c->coordinate);
        //If within range, add it
        if(dist < _rMax) _list[be].push_back(c);
    }
}

void BoundaryCylinderNL::addNeighbor(Neighbor* n) {

    //return if not a boundary element!
    BoundaryElement* be;
    if(!(be = dynamic_cast<BoundaryElement*>(n))) return;

    //update neighbors
    updateNeighbors(be);
}

void BoundaryCylinderNL::removeNeighbor(Neighbor* n) {

    BoundaryElement* be;
    if(!(be = dynamic_cast<BoundaryElement*>(n))) return;

    _list.erase(be);
}

void BoundaryCylinderNL::addDynamicNeighbor(DynamicNeighbor* n) {

    //return if not a cylinder!
    Cylinder* c;

    if(!(c = dynamic_cast<Cylinder*>(n))) return;

    for(auto it = _list.begin(); it != _list.end(); it++) {

        //if within range, add it
        if(it->first->distance(c->coordinate) < _rMax)
            it->second.push_back(c);
    }
}

void BoundaryCylinderNL::removeDynamicNeighbor(DynamicNeighbor* n) {

    //return if not a cylinder!
    Cylinder* c;

    if(!(c = dynamic_cast<Cylinder*>(n))) return;

    for(auto it = _list.begin(); it != _list.end(); it++) {

        auto cit = find(it->second.begin(), it->second.end(), c);
        if(cit != it->second.end()) it->second.erase(cit);
    }
}

void BoundaryCylinderNL::reset() {

    _list.clear();

    //loop through all neighbor keys
    for(auto boundary: BoundaryElement::getBoundaryElements())

        updateNeighbors(boundary);
}

vector<Cylinder*> BoundaryCylinderNL::getNeighbors(BoundaryElement* be) {
    return _list[be];
}

//BOUNDARYELEMENT - BUBBLE

void BoundaryBubbleNL::updateNeighbors(BoundaryElement* be) {

    //clear existing
    _list[be].clear();

    //loop through beads, add as neighbor
    for (auto &b : Bubble::getBubbles()) {

        double dist = be->distance(b->coordinate);
        //If within range, add it
        if(dist < _rMax) _list[be].push_back(b);
    }
}

void BoundaryBubbleNL::addNeighbor(Neighbor* n) {

    //return if not a boundary element!
    BoundaryElement* be;
    if(!(be = dynamic_cast<BoundaryElement*>(n))) return;

    //update neighbors
    updateNeighbors(be);
}

void BoundaryBubbleNL::removeNeighbor(Neighbor* n) {

    BoundaryElement* be;
    if(!(be = dynamic_cast<BoundaryElement*>(n))) return;

    _list.erase(be);
}

void BoundaryBubbleNL::addDynamicNeighbor(DynamicNeighbor* n) {

    //return if not a filament bead!
    Bubble* b;

    if(!(b = dynamic_cast<Bubble*>(n))) return;

    for(auto it = _list.begin(); it != _list.end(); it++) {

        //if within range, add it
        if(it->first->distance(b->coordinate) < _rMax)
            it->second.push_back(b);
    }
}

void BoundaryBubbleNL::removeDynamicNeighbor(DynamicNeighbor* n) {

    //return if not a filament bead!
    Bubble* b;

    if(!(b = dynamic_cast<Bubble*>(n))) return;

    for(auto it = _list.begin(); it != _list.end(); it++) {

        auto bit = find(it->second.begin(), it->second.end(), b);
        if(bit != it->second.end()) it->second.erase(bit);
    }
}

void BoundaryBubbleNL::reset() {

    _list.clear();

    //loop through all neighbor keys
    for(auto boundary: BoundaryElement::getBoundaryElements())

        updateNeighbors(boundary);
}

vector<Bubble*> BoundaryBubbleNL::getNeighbors(BoundaryElement* be) {

    return _list[be];
}

//BUBBLE - BUBBLE

void BubbleBubbleNL::updateNeighbors(Bubble* bb) {

    //clear existing
    _list[bb].clear();

    //loop through beads, add as neighbor
    for (auto &bbo : Bubble::getBubbles()) {

        double distsq = twoPointDistancesquared(bb->coordinate, bbo->coordinate);

        if(bb->getID() <= bbo->getID()) continue;

        //If within range, add it
        if(distsq < (_rMax * _rMax)) _list[bb].push_back(bbo);
    }
}

void BubbleBubbleNL::addNeighbor(Neighbor* n) {

    //return if not a bubble!
    Bubble* bb;
    if(!(bb = dynamic_cast<Bubble*>(n))) return;

    //update neighbors
    updateNeighbors(bb);
}

void BubbleBubbleNL::removeNeighbor(Neighbor* n) {

    Bubble* bb;
    if(!(bb = dynamic_cast<Bubble*>(n))) return;

    _list.erase(bb);

    //remove from other lists
    for(auto it = _list.begin(); it != _list.end(); it++) {

        auto bit = find(it->second.begin(), it->second.end(), bb);
        if(bit != it->second.end()) it->second.erase(bit);
    }
}

void BubbleBubbleNL::reset() {

    _list.clear();

    //loop through all neighbor keys
    for(auto bb: Bubble::getBubbles())
        updateNeighbors(bb);
}

vector<Bubble*> BubbleBubbleNL::getNeighbors(Bubble* bb) {

    return _list[bb];
}

///BUBBLE - CYLINDER

void BubbleCylinderNL::updateNeighbors(Bubble* bb) {

    //clear existing
    _list[bb].clear();

    //loop through beads, add as neighbor
    for (auto &c : Cylinder::getCylinders()) {

        double distsq = twoPointDistancesquared(c->coordinate, bb->coordinate);

        //If within range, add it
        if(distsq < (_rMax * _rMax)) _list[bb].push_back(c);
    }
}

void BubbleCylinderNL::addNeighbor(Neighbor* n) {

    Bubble* bb; Cylinder* c;
    if((bb = dynamic_cast<Bubble*>(n))) {
        updateNeighbors(bb);
    }
    else if((c = dynamic_cast<Cylinder*>(n))) {

        for(auto it = _list.begin(); it != _list.end(); it++) {

            //if within range, add it
            if(twoPointDistancesquared(it->first->coordinate, c->coordinate) < (_rMax * _rMax))
                it->second.push_back(c);
        }
    }
    else return;
}

void BubbleCylinderNL::removeNeighbor(Neighbor* n) {

    Bubble* bb; Cylinder* c;
    if((bb = dynamic_cast<Bubble*>(n))) {
        _list.erase(bb);
    }
    else if((c = dynamic_cast<Cylinder*>(n))) {
        for(auto it = _list.begin(); it != _list.end(); it++) {

            auto cit = find(it->second.begin(), it->second.end(), c);
            if(cit != it->second.end()) it->second.erase(cit);
        }
    }
    else return;
}

void BubbleCylinderNL::reset() {

    _list.clear();

    //loop through all neighbor keys
    for(auto bb: Bubble::getBubbles())
        updateNeighbors(bb);
}

vector<Cylinder*> BubbleCylinderNL::getNeighbors(Bubble* bb) {

    return _list[bb];
}

