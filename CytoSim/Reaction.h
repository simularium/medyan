//
//  Reaction.h
//  CytoSim-Experimenting
//
//  Created by Garegin Papoian on 4/19/12.
//  Copyright (c) 2012 University of Maryland. All rights reserved.
//

#ifndef CytoSim_Experimenting_Reaction_h
#define CytoSim_Experimenting_Reaction_h

#include <iostream>
#include <algorithm>
#include <utility>
#include <unordered_set>
#include <cmath>
#include <initializer_list>


#include "common.h"
#include "Species.h"


class Reaction {
private:
    std::vector<Species*> _species;
    std::vector<Reaction*> _dependents;
    const unsigned char _m;
    float _rate;
public:
    // Constructor    
    Reaction (std::initializer_list<Species*> species, unsigned char M, unsigned char N, float rate);    
    Reaction (const Reaction &r) = delete; // no copying (including all derived classes)
    Reaction& operator=(Reaction &r) = delete;  // no assignment (including all derived classes)
    // Destructor
    ~Reaction();
//    // Setters 
    void setRate(float rate){_rate=rate;}
//    // Accessors 
    float getRate(){return _rate;}
    std::vector<Reaction*> getAffectedReactions() {return _dependents;}
    //Iterators
    VR_ITER beginAffected() {return _dependents.begin();}
    VR_ITER endAffected() {return _dependents.begin();}
//    //Fire Reactions
    void makeStep() {
        std::for_each(_species.begin(), _species.begin()+_m, [](Species* s){s->incrementN(-1);} );
        std::for_each(_species.begin()+_m, _species.end(),   [](Species* s){s->incrementN(+1);} );
    }
    float computePropensity (){
        return std::accumulate(_species.begin(), _species.begin()+_m, _rate, [](float prod, Species *s){ return prod*=s->getN();} );
    }
    void printSelf(); 
private:
    std::vector<Reaction*> _getAffectedReactions();
    void _registerNewDependent(Reaction *r);
    void _unregisterDependent(Reaction *r);    
};


#endif
