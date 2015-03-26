
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

#ifndef M3SYM_CBranchingPoint_h
#define M3SYM_CBranchingPoint_h

#include "common.h"

#include "CBound.h"
#include "Compartment.h"

//FORWARD DECLARATIONS
class BranchingPoint;
class SubSystem;

/// A class to represent the chemical component of a BranchingPoint.
/*!
 *  The CBranchingPoint class contains chemical info of the parent BranchingPoint.
 */
class CBranchingPoint : public CBound {
    
private:
    BranchingPoint* _pBranchingPoint; ///< Pointer to parent
    
    int _pos; ///< Monomer position on cylinder
    short _branchType; ///< Branching point type
    
public:
    /// Default constructor and destructor
    /// @param pos - monomer index on first cylinder
    CBranchingPoint(short branchType, Compartment* c,
                    CCylinder* cc1, CCylinder* cc2, int pos);
    ///Destructor, removes off reaction from system
    ~CBranchingPoint();
    
    /// Copy constructor, standard
    CBranchingPoint(const CBranchingPoint& rhs, Compartment* c)
        : CBound(c, rhs._cc1, rhs._cc2), _pBranchingPoint(rhs._pBranchingPoint) {
        
          setFirstSpecies(rhs._firstSpecies);
          setOffReaction(rhs._offRxn);
    }
    
    /// Assignment is not allowed
    CBranchingPoint& operator=(CBranchingPoint &rhs) = delete;
    
    /// Clone, calls copy constructor
    virtual CBranchingPoint* clone(Compartment* c) {
        
        CBranchingPoint* cb = new CBranchingPoint(*this, c);
        _offRxn = nullptr; return cb;
    }
    
    /// Set parent
    void setBranchingPoint(BranchingPoint* BranchingPoint) {
        _pBranchingPoint = BranchingPoint;
    }
    /// Get parent
    BranchingPoint* getBranchingPoint() {return _pBranchingPoint;}
    
    virtual void createOffReaction(ReactionBase* onRxn, float offRate, SubSystem* ps);
};

#endif
