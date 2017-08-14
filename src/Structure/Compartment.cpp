
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

#include "Compartment.h"
#include "MathFunctions.h"
#include "Visitor.h"

Compartment& Compartment::operator=(const Compartment &other) {
    
    _species.clear();
    _internal_reactions.clear();
    _diffusion_reactions.clear();
    other.cloneSpecies(this);
    other.cloneReactions(this);
    _diffusion_rates = other._diffusion_rates;
    
    return *this;
    
}
    
bool Compartment::apply_impl(SpeciesVisitor &v) {
    for(auto &s : _species.species()) {
        v.visit(s.get());
    }
    return true;
}

bool Compartment::apply_impl(ReactionVisitor &v) {
    for(auto &r : _internal_reactions.reactions()) {
        v.visit(r.get());
    }
    return true;
}

vector<ReactionBase*> Compartment::generateDiffusionReactions(Compartment* C)
{
    vector<ReactionBase*> rxns;
    
    for(auto &sp_this : _species.species()) {
        int molecule = sp_this->getMolecule();
        float diff_rate = _diffusion_rates[molecule];
        if(diff_rate<0)  continue;
    
        if(C->isActivated()) {
            Species *sp_neighbour = C->_species.findSpeciesByMolecule(molecule);
            ReactionBase *R = new DiffusionReaction({sp_this.get(),sp_neighbour},diff_rate);
            this->addDiffusionReaction(R);
            rxns.push_back(R);
        }
    }
    return vector<ReactionBase*>(rxns.begin(), rxns.end());
}

vector<ReactionBase*> Compartment::generateAllDiffusionReactions() {
    
    vector<ReactionBase*> rxns;
    
    if(_activated) {
        for (auto &C: _neighbours) {
            auto newRxns = generateDiffusionReactions(C);
            rxns.insert(rxns.begin(), newRxns.begin(), newRxns.end());
        }
    }
    return vector<ReactionBase*>(rxns.begin(), rxns.end());
}

void Compartment::removeDiffusionReactions(ChemSim* chem, Compartment* C)
{
    //look for neighbor's diffusion reactions
    vector<ReactionBase*> to_remove;
    
    for(auto &r : C->_diffusion_reactions.reactions()) {
        
        auto rs = r.get()->rspecies()[1];
        if(rs->getSpecies().getParent() == this) {
            
            r->passivateReaction();
            chem->removeReaction(r.get());
            
            to_remove.push_back(r.get());
        }
    }
    
    //remove them
    for(auto &r : to_remove)
        C->_diffusion_reactions.removeReaction(r);
    
}

void Compartment::removeAllDiffusionReactions(ChemSim* chem) {
    
    //remove all diffusion reactions that this has ownership of
    for(auto &r : _diffusion_reactions.reactions()) {
        r->passivateReaction();
        chem->removeReaction(r.get());
    }
    
    _diffusion_reactions.clear();
    
    //remove neighboring diffusing reactions with this compartment
    for (auto &C: _neighbours)
        removeDiffusionReactions(chem, C);
}


void Compartment::transferSpecies(int i) {
    //i axis
    //0 X
    //1 Y
    //2 Z
    //3 all directions
    //get active neighbors
    vector<Compartment*> activeNeighbors;
    
    for(auto &neighbor : _neighbours){
        auto ncoord=neighbor->coordinates();
//                    std::cout<<_coords[0]<<" "<<_coords[1]<<" "<<_coords[2]<<" "<<ncoord[0]<<" "<<ncoord[1]<<" "<<ncoord[2]<<endl;
        if(neighbor->isActivated()){
            if(i==3)
                activeNeighbors.push_back(neighbor);
            else if(mathfunc::twoPointDistance(ncoord,_coords)==(abs(_coords[i]-ncoord[i])))
                activeNeighbors.push_back(neighbor);
        }}
    
    assert(activeNeighbors.size() != 0
           && "Cannot transfer species to another compartment... no neighbors are active");
    if(i<3 && activeNeighbors.size()>1){
        cout<<"Error transferring species along an axis. More than 1 neighbor. Exiting. "<< endl;
        exit(EXIT_FAILURE);
    }
    
    //go through species
    Species* sp_neighbor;
    vector<Species*> sp_neighbors;
    
    for(auto &sp : _species.species()) {
        
        int copyNumber = sp->getN();
        auto nit = activeNeighbors.begin();
        
        if(sp->getFullName().find("Bound") == string::npos){
            while(copyNumber > 0) {
                sp->down();
                
                //choose a random active neighbor
                auto neighbor = *nit;
                
                sp_neighbor = neighbor->findSpeciesByName(sp->getName());
                
                //add to list if not already
                auto spit = find(sp_neighbors.begin(),
                                 sp_neighbors.end(),
                                 sp_neighbor);
                
                if(spit == sp_neighbors.end())
                    sp_neighbors.push_back(sp_neighbor);
                
                //increase copy number
                
                sp_neighbor->up();
                
                //reset if we've looped through
                if(++nit == activeNeighbors.end())
                    nit = activeNeighbors.begin();
                copyNumber--;
                
            }
        }
        
        //activate all reactions changed
        for(auto spn : sp_neighbors)
            spn->updateReactantPropensities();
        for(auto &sp : _species.species())
            sp->updateReactantPropensities();
    }
}

void Compartment::shareSpecies(int i) {
    //i axis
    //0 X
    //1 Y
    //2 Z
    //3 all directions
    //get active neighbors
    vector<Compartment*> activeNeighbors;
    
    for(auto &neighbor : _neighbours){
        auto ncoord=neighbor->coordinates();
    if(neighbor->isActivated()){
        if(i==3)
            activeNeighbors.push_back(neighbor);
        else if(mathfunc::twoPointDistance(ncoord,_coords)==(abs(_coords[i]-ncoord[i])))
        activeNeighbors.push_back(neighbor);
    }}
    
    assert(activeNeighbors.size() != 0
           && "Cannot share species to another compartment... no neighbors are active");
    if(i<3 && activeNeighbors.size()>1){
        cout<<"Error sharing species along an axis. More than 1 neighbor. Exiting."<< endl;
        exit(EXIT_FAILURE);
    }
    //go through species
    Species* sp_neighbor;
    vector<Species*> sp_neighbors;
    
    for(auto &sp : _species.species()) {
        auto nit = activeNeighbors.begin();
        auto neighbor = *nit;
        sp_neighbor = neighbor->findSpeciesByName(sp->getName());
        int copyNumber = sp->getN();
        int lowerlimit = (int) sp->getN()/2;
        
        if(sp->getFullName().find("Bound") == string::npos){
            while(copyNumber > lowerlimit) {
                sp_neighbor->down();
                
                //add to list if not already
                auto spit = find(sp_neighbors.begin(),
                                 sp_neighbors.end(),
                                 sp_neighbor);
                
                if(spit == sp_neighbors.end())
                    sp_neighbors.push_back(sp_neighbor);
                
                //increase copy number
                sp->up();
                
                //reset if we've looped through
                if(++nit == activeNeighbors.end())
                    nit = activeNeighbors.begin();
                copyNumber--;
                
            }
        }
        
        //activate all reactions changed
        for(auto spn : sp_neighbors)
            spn->updateReactantPropensities();
        for(auto &sp : _species.species())
            sp->updateReactantPropensities();
    }
}

void Compartment::activate(ChemSim* chem) {
    
    assert(!_activated && "Compartment is already activated.");
    
    //set marker
    _activated = true;
    shareSpecies(SysParams::Mechanics().transfershareaxis);
    //add all diffusion reactions
    auto rxns = generateAllDiffusionReactions();
    for(auto &r : rxns) chem->addReaction(r);
    

}

void Compartment::deactivate(ChemSim* chem) {
    
    //assert no cylinders in this compartment
    assert((_cylinders.size() == 0)
           && "Compartment cannot be deactivated when containing active cylinders.");
    
    assert(_activated && "Compartment is already deactivated.");
    
    //set marker
    _activated = false;
    
    transferSpecies(SysParams::Mechanics().transfershareaxis);
    removeAllDiffusionReactions(chem);
}

bool operator==(const Compartment& a, const Compartment& b) {
    if(a.numberOfSpecies()!=b.numberOfSpecies() or
       a.numberOfInternalReactions()!=b.numberOfInternalReactions())
        return false;
    
    if(typeid(a)!=typeid(b))
        return false;
    
    bool spec_bool = false;
    auto sit_pair = mismatch(a._species.species().begin(),
                             a._species.species().end(),
                             b._species.species().begin(),
            [](const unique_ptr<Species> &A, const unique_ptr<Species> &B)
            {return (*A)==(*B); });
    if(sit_pair.first==a._species.species().end())
        spec_bool=true;
    
    
    bool reac_bool = false;
    auto rit_pair = mismatch(a._internal_reactions.reactions().begin(),
                             a._internal_reactions.reactions().end(),
                             b._internal_reactions.reactions().begin(),
            [](const unique_ptr<ReactionBase> &A, const unique_ptr<ReactionBase> &B)
            {return (*A)==(*B);});
    if(rit_pair.first==a._internal_reactions.reactions().end())
        reac_bool=true;
    
    return spec_bool && reac_bool;
}
