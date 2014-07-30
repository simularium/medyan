//
//  ChemSim.cpp
//  CytoSim
//
//  Created by Garegin Papoian on 5/10/12.
//  Copyright (c) 2012 University of Maryland. All rights reserved.
//

#include <iostream>
#include "ChemSim.h"
#include "ChemSimImpl.h"
#include "utility.h"

namespace chem {
    
ChemSimImpl* ChemSim::_pimpl = 0;
    
void ChemSim::setInstance(ChemSimInitKey k, ChemSimImpl *csi)
{
    _pimpl=csi;
}

void ChemSim::addReaction(ChemSimReactionKey k, ReactionBase *r){
    _pimpl->addReaction(r);
}
    
void ChemSim::removeReaction(ChemSimReactionKey k, ReactionBase *r){
    _pimpl->removeReaction(r); 
}

bool ChemSim::run(ChemSimRunKey k, int steps){
    return _pimpl->run(steps);
}

void ChemSim::initialize(ChemSimInitKey k) {
    return _pimpl->initialize();
}

void ChemSim::printReactions() const {
    return _pimpl->printReactions();
}

} // end of namespace