//
//  FilamentBending.h
//  Cyto
//
//  Created by Konstantin Popov on 8/27/14.
//  Copyright (c) 2014 University of Maryland. All rights reserved.
//

#ifndef Cyto_FilamentBending_h
#define Cyto_FilamentBending_h

#include "common.h"
#include "FilamentInteractions.h"

class Filament;

template <class FBendingInteractionType>
class FilamentBending : public FilamentInteractions {
    
private:
    FBendingInteractionType _FFType;
    
public:
    virtual double ComputeEnergy( Filament*, double d);
    virtual void ComputeForces( Filament*);
    virtual void ComputeForcesAux( Filament*);
};

#endif /* defined(__Cyto__FilamentBending__) */
