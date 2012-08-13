//
//  ChemNRMImpl.h
//  CytoSim
//
//  Created by Garegin Papoian on 5/6/12.
//  Copyright (c) 2012 University of Maryland. All rights reserved.
//

/// \section nrm_sec Next Reaction Method

/// The algorithm implemented here is based on the following reference: ** Michael A. Gibson, and Jehoshua Bruck J. Phys. Chem. A, 2000, 104 (9), 1876-1889 **

#ifndef CytoSim_ChemNRMImpl_h
#define CytoSim_ChemNRMImpl_h

#include <vector>
#include <random>

#include <boost/heap/pairing_heap.hpp>

#include "utility.h"
#include "Reaction.h"
#include "ChemSimImpl.h"
#include "ChemRNode.h"

namespace chem {

class PQNode;
class RNodeNRM;
class ChemNRMImpl;

typedef boost::heap::pairing_heap<PQNode> boost_heap;
typedef boost::heap::pairing_heap<PQNode>::handle_type handle_t;
    
/// PQNode stands for Priority Queue Node. It is stored as an element of a heap, such as 
/// boost::heap::pairing_heap<PQNode>. There will be an associated heap handle which can be 
/// used to dynamically access PQNode's fields (e.g. boost::heap::pairing_heap<PQNode>::handle_type)

/*! This is a simple structure which holds a pointer to RNodeNRM and stores the last computed tau 
 *  assoicated with this reaction. On some occasions tau needs to be recomputed, for example when 
 *  another reaction took place which affects this reaction. handle_t of the corresponding heap element 
 *  is used to get access to tau (and RNodeNRM).
*/    
class PQNode {
public:
    /// Ctor
    /// @param *rnode is a pointer to the RNodeNRM instance which this PQNode tracks (no ownership - make sure it is not null)
    /// @note tau is set to infinity in the constructor
    PQNode(RNodeNRM *rnode) : _rn(rnode), _tau (std::numeric_limits<double>::infinity()) {}

    /// Dtor: we only reassign _rn to null, the actual pointer needs to be deleted somewhere else to avoid memory leaks
    ~PQNode(){_rn=nullptr;}
    
    /// The PQ heap requires a comparision operator for ordering elements within the heap/
    /// @note In a bit of hack, the less operator is actually defined via real greater comparison of tau's
    /// of two corresponding PQNode objects. We do this so the top PQ node has the smallest tau and not largest.
    bool operator<(PQNode const &rhs) const{
        return _tau > rhs._tau;
    }
private: 
    RNodeNRM *_rn; ///< Pointer to the reaction node (RNodeNRM) which this PQNode represents (or tracks)
    double _tau; ///< tau for this reaction for the Gibson-Bruck NRM algoritm
private:
    // Think of PQNode as a simple C-like structure (i.e. no methods), but that is private to ChemNRMImpl and RNodeNRM
    friend class ChemNRMImpl;
    friend class RNodeNRM;
};

    
/// RNodeNRM stands for Reaction Node Next Reaction Method.

/*! RNodeNRM manages a single chemical reaction within the NRM algorithm. It has a pointer to the PQ element 
 *  containing the Reaction via a handle_t object (and hence can modify both the corresponding PQNode, such as PQNode's tau 
 *  or the underlying Reaction instance). RNodeNRM can recompute tau if needed and has auxiliary methods for computing 
 *  reaction's propensity. When the propensity drops to zero, the RNodeNRM can execute the passivateReaction() method. 
 *  Alternatively, passivated RNodeNRM can be activated via activateReaction(). The main part of the NRM algoritm is 
 *  implemented in the makeStep() method. 
 */
class RNodeNRM : public RNode {
public:
    /// Ctor: 
    /// @param *r is the Reaction object corresponding to this RNodeNRM
    /// @param &chem_nrm is a refernce to ChemNRMImpl object, which does the overall management of the NRM scheme (e.g. it 
    /// gives acces to the heap itself, random distribution generators, etc.)
    RNodeNRM(Reaction *r, ChemNRMImpl &chem_nrm);
    
    /// Copying is not allowed
    RNodeNRM(const RNodeNRM& rhs) = delete;

    /// Assignment is not allowed
    RNodeNRM& operator=(RNodeNRM &rhs) = delete;
    
    /// Dtor: 1) Erases the corresponding PQNode element in the heap via the handle; 2) The RNode pointer of the 
    /// tracked Reaction object is set to nullptr 
    virtual ~RNodeNRM(); 
    
    /// This methods recomputes the reaction propensity based on current coefficients and the rate,
    /// and then obtains a corresponding new tau drawn from an exponential distribution.
    /// @note This methods modifies tau which is stored in the associated PQNode object. However, 
    /// it does not update the heap - this needs to be done seprately.
    void generateNewRandTau();
    
    /// Returns a pointer to the Reaction which corresponds to this RNodeNRM.
    Reaction* getReaction() const {return _react;};
    
    /// The heap is updated only with respect to the specific PQNode element which presumably 
    /// was modified (e.g. via generateNewRandTau()).
    void updateHeap();
    
    /// Returns the tau from the the associated PQNode element
    double getTau() const {return (*_handle)._tau;}
    
    /// Sets the tau of the the associated PQNode element. Does not update the heap.
    void setTau(double tau) {(*_handle)._tau=tau;}
    
    /// Returns a handle to the associated PQNode element, which can be used to access tau, for example.
    handle_t& getHandle() {return _handle;}
    
    /// Return the currently stored propensity, "a", for this Reaction.
    /// @note The propensity is not recomputed in this method, so it potentially can be out of sync.
    double getPropensity() const {return _a;}
    
    /// (Re)Compute and return the propensity associated with this Reaction.
    double reComputePropensity() {_a=_react->computePropensity(); return _a;}

    /// Compute and return the product of reactant copy numbers. This method can be used to quickly check 
    /// whether this reaction needs to be passivated, if the returned result is zero.
    int getProductOfReactants () {return _react->getProductOfReactants ();};
    
    /// This method calls the corresponding Reaction::makeStep() method of the underyling Reaction object
    void makeStep() {_react->makeStep();}
    
    /// When this method is called, a new tau is computed and the corresponding PQNode element is updated in the heap.
    /// @note This method does not activate the Reaction itself, but instead only deals with the activation 
    ///       process related to the corresponding PQNode element.
    virtual void activateReaction();
    
    /// When this method is called, reaction's tau is set to infinity, the propensity is set to 0, and 
    /// the corresponding PQNode element is updated in the heap.
    /// @note This method does not passivate the Reaction itself, but instead only deals with the activation 
    ///        process related to the corresponding PQNode element.
    virtual void passivateReaction();
    
    /// Return true if the Reaction is currently passivated
    bool isPassivated() const {return _react->isPassivated();}
    
    /// Print information about this RNodeNRM such as tau, a and the Reaction which this RNodeNRM tracks.
    void printSelf() const;
    
    /// Print the RNode objects which are dependents of this RNode (via the tracked Reaction object dependencies)
    void printDependents() const;
private:
    ChemNRMImpl &_chem_nrm; ///< A reference to the ChemNRMImpl which containts the heap, random number generators, etc.
    handle_t _handle; ///< The handle to the associated PQNode element in the heap.
    Reaction *_react; ///< The pointer to the associated Reaction object. The corresponding memory is not managed by RNodeNRM.
    double _a; ///< The propensity associated with the Reaction. It may be outdated and may need to be recomputed if needed.
};


    
/// ChemNRMImpl stands for Chemical Next Reaction Method Implementation. 

/*! ChemNRMImpl manages the NRM algorithm at the level of the network of reactions. In particular, this class contains 
 *  the NRM heap and the exponential random number generator. Reaction objects can be added and removed from the
 *  ChemNRMImpl instance. 
 */
class ChemNRMImpl : public ChemSimImpl {
public:
    /// Ctor: Seeds the random number generator, sets global time to 0.0 and the number of reactions to 0
    ChemNRMImpl() : 
    ChemSimImpl(), _eng(static_cast<unsigned long>(time(nullptr))), _exp_distr(0.0), _n_reacts(0) {
        resetTime();
    }
    
    /// Copying is not allowed
    ChemNRMImpl(const ChemNRMImpl &rhs) = delete;
    
    /// Assignment is not allowed
    ChemNRMImpl& operator=(ChemNRMImpl &rhs) = delete;
    
    ///Dtor: The reaction network is cleared. The RNodeNRM objects will be destructed, but Reaction objects will stay intact. 
    /// When the RNodeNRM objects are destructed, they in turn destruct the corresponding PQNode element, setting the RNode 
    /// pointer of the Reaction object to null. At the end, the heap itself will go out of scope.
    ~ChemNRMImpl();
    
    /// Return the number of reactions in the network.
    size_t getSize() const {return _n_reacts;}
    
    /// Return the current global time (as defined in the NRM algorithm)
    double getTime() const {return _t;}
    
    /// Sets global time to 0.0
    void resetTime() {_t=0.0; syncGlobalTime(); }
    
    /// Sets global time variable to ChemNRMImpl's global time
    void syncGlobalTime() {global_time=_t;}
    
    /// Return a pointer to the heap
    boost_heap* getHeap() {return &_heap;}
    
    /// Add Reaction *r to the network
    void addReaction(Reaction *r);
    
    /// Remove Reaction *r from the network
    void removeReaction(Reaction *r);
    
    /// A pure function (without sideeffects), which returns a random time tau, drawn from the exponential distribution, 
    /// with the propensity given by a.
    double generateTau(double a);
        
    /// This function iterates over all RNodeNRM objects in the network, generating new tau-s for each case and 
    /// subsequently updating the heap. It needs to be called before calling run(...). 
    /// @note If somewhere in the middle of simulaiton initialize() is called, it will be analogous to starting 
    /// the simulation from scratch, except with the Species copy numbers given at that moment in time. The global time 
    /// is reset to zero again.
    void initialize();
    
    /// This method runs the NRM algorithm for the given number of steps. 
    bool run(int steps) {
        for(int i=0; i<steps; ++i){
            bool success = makeStep();
            if(!success)
                return false;
//            if(i%1000000==0)
//                std::cout << "ChemNRMImpl::run(): i=" << i << std::endl;
        }
        return true;
    }
    
    /// Prints all RNodes in the reaction network
    void printReactions() const;
    
private:
    /// This is a somewhat complex subroutine which implements the main part of the Gibson-Bruck NRM algoritm. See the 
    /// implementation for details. After this method returns, roughly the following will have happened:
    /// 1) The Reaction corresponding to the lowest tau RNodeNRM is executed and the corresponding Species copy numbers are changed
    /// 2) A new tau is computed from this Reaction and the corresponding PQNode element is updated in the heap
    /// 3) The other affected Reaction objects are found, their taus are recomputed and corresponding PQNode elements are 
    ///    updated in the heap.
    /// 4) For the Reaction and associated Species signals are emitted, if these objects broadcast signals upon change.
    /// Returns true if successful, and false if the heap is exchausted and there no more reactions to fire
    bool makeStep();
private:
    std::unordered_map<Reaction*, std::unique_ptr<RNodeNRM>> _map_rnodes; ///< The database of RNodeNRM objects, representing the reaction network
    boost_heap _heap; ///< A priority queue for the NRM algorithm, containing PQNode elements
    std::mt19937 _eng; ///< Random number generator
    std::exponential_distribution<double> _exp_distr; ///< Adaptor for the exponential distribution
    double _t; ///< global time
    size_t _n_reacts; ///< number of reactions in the network
};
    
} // end of namespace
#endif
