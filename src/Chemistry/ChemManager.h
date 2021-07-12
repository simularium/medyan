
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v4.0
//
//  Copyright (2015-2018)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#ifndef MEDYAN_ChemManager_h
#define MEDYAN_ChemManager_h

#include "common.h"

#include "ReactionTemplate.h"
#include "DissipationTracker.h"

//FORWARD DECLARATIONS
class Compartment;
class CompartmentGrid;
class ChemSim;

class Cylinder;
class CCylinder;
class CMonomer;

/// For initailizing chemical reactions based on a specific system
/*!
 *  ChemManager is used for initailizing all chemistry in the system.
 *  Initialized by the CController, ChemManager initializes the chemical components
 *  of the CompartmentGrid. The ChemManager can also update chemical components of
 *  a CCylinder using its stored collection of FilamentReactionTemplate, which is
 *  initialized at startup.
 */
class ChemManager {
    
private:
    chrono::high_resolution_clock::time_point mins, mine;
    //@{
    /// Helper functions. Names are pretty self-explanatory.
    void setupBindingSites();
    
    void configCMonomer();
    void initCMonomer(CMonomer* m, short filamentType, Compartment* c);
    
    void genSpecies(Compartment& protoCompartment);
    
    void genGeneralReactions(Compartment& protoCompartment);
    void genBulkReactions();
    
    void genNucleationReactions();
    void genFilBindingReactions();
    
    void genFilReactionTemplates();
    //@}
    
public:
	static floatingpoint tchemmanager1;
	static floatingpoint tchemmanager2;
	static floatingpoint tchemmanager3;
	static floatingpoint tchemmanager4;

    ///Constructor sets subsystem pointer, and loads chem data
    ChemManager(SubSystem* subSystem, ChemistryData chem)
        : _subSystem(subSystem), _chemData(chem) {}
    
    /// Initialize the system, based on the given simulation
    /// Adds all necessary reactions to the ChemSim object
    virtual void initializeSystem(ChemSim* chem);
    
    ///Initializer for chem cylinders, based on the given simulation
//    virtual void initializeCCylinder(CCylinder* cc,
//                                     bool extensionFront,
//                                     bool extensionBack,
//                                     bool initialization);

	virtual void initializeCCylinder(CCylinder* cc,
	                                      bool extensionFront,
	                                      bool extensionBack,
	                                      bool initialization,
	                                      int nummonomers = -1,
	                                      int firstmonomer = -1,
	                                      int lastmonomer = -1,
	                                      bool minusendstatus = false,
	                                      bool plusendstatus = false,
	                                      short minusendtype = -1,
	                                      short plusendtype = -1);
    
    /// Update the copy numbers of all species in the chemical network
    /// @note - this only sets the copy number if the simulation time
    ///         tau has passed the release time of the molecule. This
    ///         function is called at every set of chemical steps to check
    ///         if molecules should be released at the current time.
    virtual void updateCopyNumbers();

    virtual void restartreleaseandremovaltime(floatingpoint _minimizationTime);
    
private:
    //DATA MEMBERS
    SubSystem* _subSystem;   ///< A pointer to subsytem for creation of callbacks, etc.
    ChemistryData _chemData; ///<The chemistry data for the system
    DissipationTracker* _dt;
    
    /// A list of reactions to add to every new CCylinder
    /// @note - is a 2D vector corresponding to different filament types
    vector<vector<unique_ptr<FilamentReactionTemplate>>> _filRxnTemplates =
    vector<vector<unique_ptr<FilamentReactionTemplate>>>(MAX_FILAMENT_TYPES);
};


#endif
