//
//  FilopodiaCSystem.h
//  CytoSim
//
//  Created by James Komianos on 7/10/14.
//  Copyright (c) 2014 University of Maryland. All rights reserved.
//

#ifndef __CytoSim__FilopodiaCSystem__
#define __CytoSim__FilopodiaCSystem__

#include <iostream>
#include "CSystem.h"
#include "CFilamentImpl.h"
#include "Mcommon.h"


namespace chem {
    
    class CMembrane;
    
    /// SimpleInitializer is a basic implementation of the Initializer class that only involves the following:
    
    /// Actin polymerization of (+) end
    /// Actin depolymerization of (+) end
    /// Actin depolymerization of (-) end
    
    /// Capping polymerization of (+) end
    /// Capping depolymerization of (+) end
    
    /// formin polymerization of (+) end
    /// formin depolymerization of (+) end
    /// increased rate of actin polymerization with formin (+) end
    
    /// Motor binding to filament
    /// Motor movement (right or left) on filament
    /// Motor loading and unloading
    
    template<size_t NDIM>
    class SimpleInitializer : public CFilamentInitializer<NDIM> {
        
    private:
        CMembrane& _membrane;
        
        ///REACTION RATES
        //basic
        float _k_on_plus = 21.0;
        float _k_off_plus = 1.4;
        float _k_off_minus = 1.4;
        
        //capping
        float _k_capping_on_plus = 50.0;
        float _k_capping_off_plus = 0.06;
        
        //formin
        float _k_formin_on_plus = 10.0;
        float _k_formin_off_plus = 1.4;
        float _k_accel_on_plus = 100.0;
        
        //motors
        float _k_binding = 19.0;
        float _k_unbinding = 10.0;
        float _k_forward_step = 50.0;
        float _k_backward_step = 5.0;
        float _k_load = 20.0;
        float _k_unload = 10.0;
        
        //Diffusion rate
        float _diffusion_rate = 2000.0;
        
        ///Compartment properties
        float _side_length = 100.0;
        
        
    public:
        
        ///Constructor, does nothing
        SimpleInitializer(ChemSim &chem, CMembrane &membrane) :
            CFilamentInitializer<NDIM>(chem), _membrane(membrane){};
        
        ///Destructor, does nothing
        ~SimpleInitializer() {};
        
        ///Find the current polymerization reactions associated with this CFilament
        virtual std::vector<ReactionBase*> findPolymerizationReactions(CFilament* f);
        
        
        ///Initialize proto compartment based on this implementation
        virtual void initializeProtoCompartment(CompartmentSpatial<NDIM>& Cproto);
        
        ///Initializer, based on the given simulation
        ///@param length - starting length of the filament initialized
        ///@param maxlength - length of entire reactive filament
        ///@param species - list of species to initialize in filament
        virtual CSubFilament* createCSubFilament(CFilament* parentFilament,
                                               Compartment* c,
                                               std::vector<std::string> species,
                                               int length);
        
        ///Remove a CSubFilament, based on the given simulation
        virtual void removeCSubFilament(CFilament* parentFilament);
        
        ///Update filaments based on a reaction
        ///In this implementation, update polymerization rates based on membrane
        virtual void update(CFilament* f, ReactionBase* r);
       
    };
    
    
    /// FilopodiaCSystem is a basic implementation for updating filopodia filaments
    template <size_t NDIM>
    class FilopodiaCSystem : public CSystem<NDIM> {
 
    public:
        ///Constructor, calls base class
        FilopodiaCSystem(CFilamentInitializer<NDIM>* initializer)
        : CSystem<NDIM>::CSystem(initializer) {};
        
        ///Default destructor, does nothing
        ~FilopodiaCSystem() {}
        
        //Initialize a number of filaments
        virtual CFilament* initializeCFilament(int length);
        
        ///Extend the front of a filament
        virtual void extendFrontOfCFilament(CFilament *f, std::vector<std::string> species);
        
        ///Retract the front of a CFilament
        virtual void retractFrontOfCFilament(CFilament *f);

        ///Perform one step of retrograde flow for this system
        virtual void retrogradeFlow();
    };
    
    
    
}; //end namespace chem


#endif /* defined(__CytoSim__FilopodiaCSystem__) */
