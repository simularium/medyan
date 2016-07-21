
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v3.0
//
//  Copyright (2015)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#include "ReactionBase.h"

#include "Composite.h"

ReactionBase::ReactionBase (float rate, bool isProtoCompartment)
    : _rnode(nullptr), _parent(nullptr), _rate(rate), 
      _rate_bare(rate), _isProtoCompartment(isProtoCompartment) {
#ifdef REACTION_SIGNALING
    _signal=nullptr;
#endif
#if defined TRACK_ZERO_COPY_N || defined TRACK_UPPER_COPY_N
    _passivated=true;
#endif
}

Composite* ReactionBase::getRoot() {
    if(hasParent())
        return this->getParent()->getRoot();
    return nullptr;
}

void ReactionBase::registerNewDependent(ReactionBase *r){ _dependents.insert(r);}

void ReactionBase::unregisterDependent(ReactionBase *r){ _dependents.erase(r);}

#ifdef REACTION_SIGNALING
void ReactionBase::startSignaling () {
    _signal = unique_ptr<ReactionEventSignal>(new ReactionEventSignal);
}

void ReactionBase::stopSignaling () {
    _signal = nullptr;
}

boost::signals2::connection ReactionBase::connect(
    function<void (ReactionBase *)> const &react_callback, int priority) {
    if (!isSignaling())
        startSignaling();
    return _signal->connect(priority, react_callback);
}
#endif

void ReactionBase::printDependents()  {
    cout << "ReactionBase: ptr=" << this << "\n"
    << (*this) << "the following ReactionBase objects are dependents: ";
    if(_dependents.size()==0)
        cout << "NONE" << endl;
    else
        cout << endl;
    for(auto r : _dependents)
        cout << (*r) << endl;
}
