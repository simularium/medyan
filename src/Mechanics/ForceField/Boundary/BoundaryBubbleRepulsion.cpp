
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

#include "BoundaryBubbleRepulsion.h"

#include "BoundaryBubbleRepulsionExp.h"
#include "BoundaryElement.h"

#include "Bubble.h"
#include "Bead.h"

template <class BRepulsionInteractionType>
void BoundaryBubbleRepulsion<BRepulsionInteractionType>::vectorize() {
    //count interactions
    nint = 0;
    for (auto be: BoundaryElement::getBoundaryElements())
    {

        for(auto &c : _neighborList->getNeighbors(be)) {
            nint++;
        }
    }

    beadSet = new int[n * nint];
    krep = new floatingpoint[nint];
    slen = new floatingpoint[nint];
    auto beList = BoundaryElement::getBoundaryElements();

    int nbe = BoundaryElement::getBoundaryElements().size();
    int i = 0;
    int ni = 0;
    int bindex = 0;

    nneighbors = new int[nbe];//stores number of interactions per boundary element.
	floatingpoint *beListplane;
    int *nintvec;
    beListplane = new floatingpoint[4 * nbe];
    nintvec = new int[nbe];//stores cumulative number of nneighbors.

    int cumnn=0;
    for (i = 0; i < nbe; i++) {

        auto be = BoundaryElement::getBoundaryElements()[i];//beList[i];
        auto nn = _neighborList->getNeighbors(be).size();

        nneighbors[i] = 0;
        auto idx = 0;

        for (ni = 0; ni < nn; ni++) {

            bindex = _neighborList->getNeighbors(be)[ni]->getBead()->getStableIndex();
            beadSet[cumnn+idx] = bindex;
            krep[cumnn+idx] = be->getRepulsionConst();
            slen[cumnn+idx] = be->getScreeningLength();
            idx++;


        }
        nneighbors[i]=idx;
        cumnn+=idx;
        nintvec[i] = cumnn;
    }


}

template <class BRepulsionInteractionType>
void BoundaryBubbleRepulsion<BRepulsionInteractionType>::deallocate() {
    delete [] beadSet;
    delete [] krep;
    delete [] slen;
    delete [] nneighbors;
}

template <class BRepulsionInteractionType>
floatingpoint BoundaryBubbleRepulsion<BRepulsionInteractionType>::computeEnergy(floatingpoint *coord, floatingpoint *f, floatingpoint d) {

    floatingpoint U_i=0.0;
    
    if (d == 0.0) {
        U_i = _FFType.energy(coord, f, beadSet, krep, slen, nneighbors);
    }
    else {
        U_i = _FFType.energy(coord, f, beadSet, krep, slen, nneighbors, d);
    }
    
    return U_i;
}

template <class BRepulsionInteractionType>
void BoundaryBubbleRepulsion<BRepulsionInteractionType>::computeForces(floatingpoint *coord, floatingpoint *f) {
    
    _FFType.forces(coord, f, beadSet, krep, slen, nneighbors);
}


///Template specializations
template floatingpoint BoundaryBubbleRepulsion<BoundaryBubbleRepulsionExp>::computeEnergy
        (floatingpoint *coord, floatingpoint *f, floatingpoint d);
template void BoundaryBubbleRepulsion<BoundaryBubbleRepulsionExp>::computeForces(floatingpoint *coord, floatingpoint *f);
template void BoundaryBubbleRepulsion<BoundaryBubbleRepulsionExp>::vectorize();
template void BoundaryBubbleRepulsion<BoundaryBubbleRepulsionExp>::deallocate();

