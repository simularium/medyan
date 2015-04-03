
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

#include "NeighborListImpl.h"

#include "Bead.h"
#include "Cylinder.h"
#include "BoundaryElement.h"

#include "GController.h"
#include "MathFunctions.h"

using namespace mathfunc;

//CYLINDER-CYLINDER

void CCNeighborList::updateNeighbors(Cylinder* cylinder) {
    
    //clear existing
    _list[cylinder].clear();
    
    //Find surrounding compartments (For now its conservative, change soon)
    vector<Compartment*> compartments;
    
    GController::findCompartments(
        cylinder->coordinate, cylinder->getCompartment(),
        SysParams::Geometry().largestCompartmentSide * 2, compartments);
    
    for(auto &comp : compartments) {
        for(auto &nearbyCylinder : comp->getCylinders()) {
            
            //Dont add if ID is more than cylinder
            if(cylinder->getID() <= nearbyCylinder->getID()) continue;
            
            //Don't add if on the same filament
            if(cylinder->getFilament() == nearbyCylinder->getFilament()) {
                
                //if cross filament only interaction, dont add
                if(_crossFilamentOnly) continue;
                
                //if not cross filament, check if not neighboring
                auto dist = abs(cylinder->getPositionFilament() -
                                nearbyCylinder->getPositionFilament());
                if(dist <= 2) continue;
            }
            
            //Dont add if not within range
            double dist = twoPointDistance(cylinder->coordinate,
                                           nearbyCylinder->coordinate);
            if(dist > _rMax || dist < _rMin) continue;
            
            //If we got through all of this, add it!
            _list[cylinder].push_back(nearbyCylinder);
        }
    }
}

void CCNeighborList::addNeighbor(Neighbor* n) {

    //return if not a cylinder!
    Cylinder* cylinder;
    if(!(cylinder = dynamic_cast<Cylinder*>(n))) return;
    
    //update neighbors
    updateNeighbors(cylinder);
}

void CCNeighborList::removeNeighbor(Neighbor* n) {
    
    Cylinder* cylinder;
    if(!(cylinder = dynamic_cast<Cylinder*>(n))) return;
    
    _list.erase(cylinder);
}

void CCNeighborList::removeDynamicNeighbor(Neighbor* n) {
    
    //return if not a cylinder!
    Cylinder* cylinder;
    if(!(cylinder = dynamic_cast<Cylinder*>(n))) return;
    
    //remove its own list
    removeNeighbor(n);
    
    //remove from other lists
    for(auto it = _list.begin(); it != _list.end(); it++) {
        
        auto cit = find(it->second.begin(), it->second.end(), cylinder);
        if(cit != it->second.end()) it->second.erase(cit);
    }
}

void CCNeighborList::reset() {
    
    //loop through all neighbor keys
    for(auto it = _list.begin(); it != _list.end(); it++) {
    
        it->second.clear(); ///clear vector of neighbors
        updateNeighbors(it->first);
    }
}

vector<Cylinder*> CCNeighborList::getNeighbors(Cylinder* cylinder) {
    
    return _list[cylinder];
}

//BEAD-BOUNDARY ELEMENT

void BBENeighborList::updateNeighbors(BoundaryElement* be) {
    
    //clear existing
    _list[be].clear();
    
    //loop through beads, add as neighbor
    for (auto &b : *BeadDB::instance()) {
        
        double dist = be->distance(b->coordinate);
        //If within range, add it
        if(dist < _rMax) _list[be].push_back(b);
    }
}

void BBENeighborList::addNeighbor(Neighbor* n) {
    
    //return if not a boundary element!
    BoundaryElement* be;
    if(!(be = dynamic_cast<BoundaryElement*>(n))) return;
    
    //update neighbors
    updateNeighbors(be);
}

void BBENeighborList::removeNeighbor(Neighbor* n) {
    
    BoundaryElement* be;
    if(!(be = dynamic_cast<BoundaryElement*>(n))) return;
    
    _list.erase(be);
}

void BBENeighborList::addDynamicNeighbor(Neighbor* n) {
    
    //return if not a boundary element!
    Bead* b; if(!(b = dynamic_cast<Bead*>(n))) return;
    
    for(auto it = _list.begin(); it != _list.end(); it++) {
        
        //if within range, add it
        if(it->first->distance(b->coordinate) < _rMax)
            it->second.push_back(b);
    }
}

void BBENeighborList::removeDynamicNeighbor(Neighbor* n) {
    
    //return if not a bead!
    Bead* b; if(!(b = dynamic_cast<Bead*>(n))) return;
    
    for(auto it = _list.begin(); it != _list.end(); it++) {
        
        auto bit = find(it->second.begin(), it->second.end(), b);
        if(bit != it->second.end()) it->second.erase(bit);
    }
}

void BBENeighborList::reset() {
    
    //loop through all neighbor keys
    for(auto it = _list.begin(); it != _list.end(); it++) {
        
        it->second.clear(); ///clear vector of neighbors
        updateNeighbors(it->first);
    }
}

vector<Bead*> BBENeighborList::getNeighbors(BoundaryElement* be) {
    
    return _list[be];
}

//CYLINDER-BOUNDARY ELEMENT

void CBENeighborList::updateNeighbors(BoundaryElement* be) {
    
    //clear existing
    _list[be].clear();
    
    //loop through beads, add as neighbor
    for (auto &c : *CylinderDB::instance()) {
        
        double dist = be->distance(c->coordinate);
        //If within range, add it
        if(dist < _rMax) _list[be].push_back(c);
    }
}

void CBENeighborList::addNeighbor(Neighbor* n) {
    
    //return if not a boundary element!
    BoundaryElement* be;
    if(!(be = dynamic_cast<BoundaryElement*>(n))) return;
    
    //update neighbors
    updateNeighbors(be);
}

void CBENeighborList::removeNeighbor(Neighbor* n) {
    
    BoundaryElement* be;
    if(!(be = dynamic_cast<BoundaryElement*>(n))) return;
    
    _list.erase(be);
}

void CBENeighborList::addDynamicNeighbor(Neighbor* n) {
    
    //return if not a boundary element!
    Cylinder* c; if(!(c = dynamic_cast<Cylinder*>(n))) return;
    
    for(auto it = _list.begin(); it != _list.end(); it++) {
        
        //Add if within range
        if(it->first->distance(c->coordinate) < _rMax)
            it->second.push_back(c);
    }
}

void CBENeighborList::removeDynamicNeighbor(Neighbor* n) {
    
    //return if not a boundary element!
    Cylinder* cylinder;
    if(!(cylinder = dynamic_cast<Cylinder*>(n))) return;
    
    for(auto it = _list.begin(); it != _list.end(); it++) {
        
        auto cit = find(it->second.begin(), it->second.end(), cylinder);
        if(cit != it->second.end()) it->second.erase(cit);
    }
}

void CBENeighborList::reset() {
    
    //loop through all neighbor keys
    for(auto it = _list.begin(); it != _list.end(); it++) {
        
        it->second.clear(); ///clear vector of neighbors
        updateNeighbors(it->first);
    }
}

vector<Cylinder*> CBENeighborList::getNeighbors(BoundaryElement* be) {
    
    return _list[be];
}