//
//  RSpecies.h
//  CytoSim
//
//  Created by Garegin Papoian on 5/22/12.
//  Copyright (c) 2012 University of Maryland. All rights reserved.
//

#ifndef CytoSim_RSpecies_h
#define CytoSim_RSpecies_h

#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <stdexcept>

#include <boost/signals2/signal.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/shared_connection_block.hpp>

#include "common.h"
#include "utility.h"

#include <iostream>

namespace chem {

    class Species;
    class RSpecies;
    class Reaction;
    class ChemSignal;
    
    /// vr stands for vector of Reactions
    typedef std::vector<Reaction*>::iterator vr_iterator; 
    typedef std::vector<Reaction*>::const_iterator vr_const_iterator; 
    
    /// vsp stands for vector of RSpecies
    typedef std::vector<RSpecies*>::iterator vrsp_iterator; 
    typedef std::vector<RSpecies*>::const_iterator vrsp_const_iterator; 
    
    /// This is a RSpecies signal object that can be used to signal when the copy number changes
    typedef boost::signals2::signal<void (RSpecies *, int)> RSpeciesCopyNChangedSignal;
 
    /// RSpecies class represents the reactive aspect of chemical molecules. It tracks their copy number and can be used in [Reactions](@ref Reaction).
    
    /*!  This class represents the reactivity of chemical species. The name RSpecies stems from reacting species.
     *   RSpecies tracks the copy number of molecules and the [Reactions](@ref Reaction)
     *   in which it is involed (@see Reaction). 
     *   @note Each intantiation of RSpecies is unique, and hence, cannot be neither copied nor moved (C++11). 
     *   This has significant implications - e.g., RSpecies cannot be used in std::vector<RSpecies>. Instead, 
     *   one should use either std::vector<RSpecies*> if not owning the RSpecies pointers, or 
     *   std::vector<std::unique_ptr<RSpecies>> if owning the RSpecies pointers. A special allocator can 
     *   be written such that dynamically allocated RSpecies (through new), are arranged contigiously in memory.
     */
    class RSpecies {
        /// Reactions calls addAsReactant(), removeAsReactant() - which other classes should not call
        friend class Reaction; 
        friend class Species;
        
    private: //Variables
        std::vector<Reaction *> _as_reactants; ///< a vector of [Reactions](@ref Reaction) where this RSpecies is a Reactant
        std::vector<Reaction *> _as_products; ///< a vector of [Reactions](@ref Reaction) where this RSpecies is a Product
        Species& _species; ///< reference to the **parent** Species object
        species_copy_t _n; ///< Current copy number of this RSpecies
        species_copy_t _ulim; ///< Upper limit for the copy number, afterwards all reactions leading to further accum. are turned off
        RSpeciesCopyNChangedSignal *_signal; ///< Can be used to broadcast a signal associated with change of n of
                                              ///< this RSpecies (usually when a single step of this Reaction occurs)
        
    private:
        /// Constructors 
        /// @param parent - the Species object to which this RSpecies belongs
        /// @param n - copy number
        RSpecies (Species &parent, species_copy_t n=0, species_copy_t ulim=max_ulim) :
        _species(parent), _n(n), _ulim(ulim), _signal(nullptr) {}
        
        /// deleted copy constructor - each RSpecies is uniquely created by the parent Species
        RSpecies (const RSpecies &r) = delete;
        
        /// deleted move constructor - each RSpecies is uniquely created by the parent Species
        RSpecies (RSpecies &&r) = delete;
        
        /// deleted assignment operator - each RSpecies is uniquely created by the parent Species
        RSpecies& operator=(RSpecies&) = delete;
               
        /// Sets the copy number for this RSpecies. 
        /// @param n should be a non-negative number, but no checking is done in run time
        /// @note The operation does not emit any signals about the copy number change.
        void setN(species_copy_t n) {_n=n;}
        
        /// Sets the upper limit for the copy number of this RSpecies.
        /// @param ulim should be a non-negative number, but no checking is done in run time
        void setUpperLimitForN(species_copy_t ulim) {_ulim=ulim;}
        
        /// Increases the copy number by 1. If the copy number changes from 0 to 1, calls a "callback"-like method 
        /// to activated previously passivated [Reactions](@ref Reaction), where this RSpecies is a Reactant.
        void up() {
            _n+=1;
#ifdef TRACK_ZERO_COPY_N
            if(_n==1)
                activateAssocReactantReactions();
#endif
#ifdef TRACK_UPPER_COPY_N
            if(_n==_ulim)
                passivateAssocProductReactions();
#endif
        }
        
        /// Decreases the copy number by 1. If the copy number changes becomes 0, calls a "callback"-like method 
        /// to passivate [Reactions](@ref Reaction), where this RSpecies is a Reactant.
        void down() {
#ifdef TRACK_UPPER_COPY_N
            species_copy_t prev_n = _n;
#endif
            _n-=1;
#ifdef TRACK_ZERO_COPY_N
            if(_n == 0)
                passivateAssocReactantReactions();
#endif
#ifdef TRACK_UPPER_COPY_N
            if(prev_n == _ulim)
                activateAssocProductReactions();
#endif
        }
        
        // \internal This methods is called by the Reaction class  during construction
        // of the Reaction where this RSpecies is involved as a Reactant
        void addAsReactant(Reaction *r){_as_reactants.push_back(r);}
        
        // \internal This methods is called by the Reaction class during construction
        // of the Reaction where this RSpecies is involved as a Product    
        void addAsProduct(Reaction *r){_as_products.push_back(r);}
        
        // \internal This method is called by the Reaction class during destruction
        // of the Reaction where this RSpecies is involved as a Reactant
        void removeAsReactant(const Reaction *r) {
            auto rxit = std::find(_as_reactants.begin(),_as_reactants.end(),r);
            if(rxit!=_as_products.end()){
                _as_reactants.erase(rxit);
            }
            
        }
        // \internal This method is called by the Reaction class during destruction
        // of the Reaction where this RSpecies is involved as a Product
        void removeAsProduct(const Reaction *r) {
            auto rxit = std::find(_as_products.begin(),_as_products.end(),r);
            if(rxit!=_as_products.end()){
                _as_products.erase(rxit);
            }
        }  
        
        /// \internal Attempts to activate previously passivated [Reactions](@ref Reaction) where this RSpecies is involved as a 
        /// Reactant. Usually, the Reaction was first passivated, for example if the RSpecies copy number of 
        /// one of the reactants dropeed to zero. This attempt may not succeed if there are still other
        /// reactants in the same Reaction with zero copy count.
        void activateAssocReactantReactions();
        
        /// \internal Attempts to activate previously passivated [Reactions](@ref Reaction) where this RSpecies is involved as a
        /// Product. Usually, the Reaction was first passivated, for example if the RSpecies copy number of
        /// one of the reactants went up to max_ulim. 
        void activateAssocProductReactions();
        
        /// \internal Passivates all [Reactions](@ref Reaction) where this RSpecies is among the reactants.
        void passivateAssocReactantReactions();
        
        /// \internal Passivates all [Reactions](@ref Reaction) where this RSpecies is among the products.
        void passivateAssocProductReactions();
               
        /// Set the signaling behavior of this RSpecies
        void startSignaling ();
        
        /// Destroy the signal associated with this RSpecies; all associated slots will be destroyed
        /// @note To start signaling again, startSignaling() needs to be called
        void stopSignaling ();
                        
    public:
         /// It is required that all [Reactions](@ref Reaction) associated with this RSpecies are destructed before this RSpecies is destructed. 
        /// Most of the time, this will occur naturally. If not, an assertion will ungracefully terminate the program.
        ~RSpecies(){
            assert((_as_reactants.empty() and _as_products.empty()) && "Major bug: RSpecies should not contain Reactions when being destroyed.");//Should not throw an exception from a destructor - that would be undefined behavior
//            std::cout << "Destructor ~RSpecies() called on ptr=" << this << std::endl;
            if(_signal!=nullptr)
                delete _signal;
        }
        
        /// Broadcasts signal indicating that the copy number of this RSpecies has changed
        /// This method should usually called by the code which runs the chemical dynamics (i.e. Gillespie-like algorithm)
        void emitSignal(int delta) {
            if(isSignaling())
                (*_signal)(this, delta);
        }
        
        /// Return the current copy number of this RSpecies
        species_copy_t getN() const {return _n;}
        
        /// Return the upper limit for the copy number of this RSpecies
        species_copy_t getUpperLimitForN() const {return _ulim;}
        
        /// return parent Species as a reference
        Species& getSpecies () {return _species;}
        
        /// return parent Species as a const reference
        const Species& getSpecies () const {return _species;}
        
        /// Return the full name of this Species in a std::string format (e.g. "Arp2/3{Bulk}"
        std::string getFullName() const;
                        
        /// Return true if this RSpecies emits signals on copy number change
        bool isSignaling () const {return _signal!=nullptr;}
                
        /// Return std::vector<Reaction *>, which contains pointers to all [Reactions](@ref Reaction) where this RSpecies 
        /// is involved as a Reactant
        std::vector<Reaction *> ReactantReactions(){return _as_reactants;}
        
        /// Return std::vector<Reaction *>, which contains pointers to all [Reactions](@ref Reaction) where this RSpecies 
        /// is involved as a Product
        std::vector<Reaction *> ProductReactions(){return _as_products;}
        
        /// Return std::vector<Reaction *>::iterator, which points to the beginning of all 
        /// [Reactions](@ref Reaction) where this RSpecies is involved as a Reactant
        vr_iterator beginReactantReactions() {return _as_reactants.begin();}
        
        /// Return std::vector<Reaction *>::iterator, which points to the beginning of all 
        /// [Reactions](@ref Reaction) where this RSpecies is involved as a Product
        vr_iterator beginProductReactions() {return _as_products.begin();}
        
        /// Return std::vector<Reaction *>::iterator, which points to the end of all 
        /// [Reactions](@ref Reaction) where this RSpecies is involved as a Reactant    
        vr_iterator endReactantReactions() {return _as_reactants.end();}
        
        /// Return std::vector<Reaction *>::iterator, which points to the end of all 
        /// [Reactions](@ref Reaction) where this RSpecies is involved as a Product
        vr_iterator endProductReactions() {return _as_products.end();}
        
//        /// Print self into an iostream
//        friend std::ostream& operator<<(std::ostream& os, const RSpecies& s);
    };
    
} // end of chem namespace 

/// Print self into an iostream
std::ostream& operator<<(std::ostream& os, const chem::RSpecies& s);

#endif
