//
//  ChemCallbacks.h
//  Cyto
//
//  Created by James Komianos on 9/11/14.
//  Copyright (c) 2014 University of Maryland. All rights reserved.
//

#ifndef __Cyto__ChemCallbacks__
#define __Cyto__ChemCallbacks__

#include <iostream>
#include "ReactionBase.h"
#include "SubSystem.h"

#include "common.h"
#include "SystemParameters.h"
using namespace std;

///FILAMENT REACTION CALLBACKS

///Extension callback
struct FilamentExtensionFrontCallback {
    
    //members
    Filament* _filament;
    ///Constructor, sets members
    FilamentExtensionFrontCallback(Filament* filament) : _filament(filament){};
    ///Callback
    void operator() (ReactionBase *r){ _filament->ExtendFront(); }
};

///Extension callback
struct FilamentExtensionBackCallback {
    
    //members
    Filament* _filament;
    ///Constructor, sets members
    FilamentExtensionBackCallback(Filament* filament) : _filament(filament){};
    ///Callback
    void operator() (ReactionBase *r){ _filament->ExtendBack(); }
};

///Retraction callback
struct FilamentRetractionFrontCallback {
    
    //members
    Filament* _filament;
    ///Constructor, sets members
    FilamentRetractionFrontCallback(Filament* filament) : _filament(filament) {};
    ///Callback
    void operator() (ReactionBase *r){ _filament->RetractFront(); }
};

///Retraction callback
struct FilamentRetractionBackCallback {
    
    //members
    Filament* _filament;
    ///Constructor, sets members
    FilamentRetractionBackCallback(Filament* filament) : _filament(filament) {};
    ///Callback
    void operator() (ReactionBase *r){ _filament->RetractBack(); }
};

///Polymerization/depolymerization callbacks
struct FilamentPolymerizationFrontCallback {
    
    //members
    Filament* _filament;
    ///Constructor, sets members
    FilamentPolymerizationFrontCallback(Filament* filament) : _filament(filament){};
    ///Callback
    void operator() (ReactionBase *r){ _filament->PolymerizeFront(); }
};

struct FilamentPolymerizationBackCallback {
    
    //members
    Filament* _filament;
    ///Constructor, sets members
    FilamentPolymerizationBackCallback(Filament* filament) : _filament(filament){};
    ///Callback
    void operator() (ReactionBase *r){ _filament->PolymerizeBack(); }
};

///Retraction callback
struct FilamentDepolymerizationFrontCallback {
    
    //members
    Filament* _filament;
    ///Constructor, sets members
    FilamentDepolymerizationFrontCallback(Filament* filament) : _filament(filament) {};
    ///Callback
    void operator() (ReactionBase *r){ _filament->DepolymerizeFront(); }
};

///Retraction callback
struct FilamentDepolymerizationBackCallback {
    
    //members
    Filament* _filament;
    ///Constructor, sets members
    FilamentDepolymerizationBackCallback(Filament* filament) : _filament(filament) {};
    ///Callback
    void operator() (ReactionBase *r){ _filament->DepolymerizeBack(); }
};

///LINKER AND MOTOR CALLBACKS

///Linker binding callback
struct LinkerBindingCallback {
    
    ///members
    SubSystem* _ps;
    Cylinder* _c1, *_c2;
    short _linkerType;
    short _position1, _position2;

    LinkerBindingCallback(Cylinder* c1, Cylinder* c2, short linkerType, short position1, short position2, SubSystem* ps)
        : _ps(ps), _c1(c1), _c2(c2), _linkerType(linkerType),  _position1(position1), _position2(position2){}
    
    void operator() (ReactionBase *r) {
        
        ///Create a linker
        int cylinderSize = SystemParameters::Geometry().cylinderIntSize;
        
        double pos1 = double(_position1) / cylinderSize;
        double pos2 = double(_position2) / cylinderSize;
        
        _ps->AddNewLinker(_c1, _c2, _linkerType, pos1, pos2);
    }
};

///Motor binding callback
struct MotorBindingCallback {
    
    ///members
    SubSystem* _ps;
    Cylinder* _c1, *_c2;
    short _motorType;
    short _position1, _position2;
    
    MotorBindingCallback(Cylinder* c1, Cylinder* c2, short motorType, short position1, short position2, SubSystem* ps)
    : _ps(ps), _c1(c1), _c2(c2), _motorType(motorType),  _position1(position1), _position2(position2){}
    
    void operator() (ReactionBase *r) {
        
        ///Create a linker
        int cylinderSize = SystemParameters::Geometry().cylinderIntSize;
        
        double pos1 = double(_position1) / cylinderSize;
        double pos2 = double(_position2) / cylinderSize;
        
        _ps->AddNewMotorGhost(_c1, _c2, _motorType, pos1, pos2);
    }
};


///unbinding callback
struct UnbindingCallback {
    
    ///members
    SubSystem* _ps;
    SpeciesBound* _s1;
    
    UnbindingCallback(SpeciesBound* s1, SubSystem* ps) : _s1(s1), _ps(ps) {}
    
    void operator() (ReactionBase *r) {
        
        //check if we have a basic bound element, linker, or motor
        CBound* cBound = _s1->getCBound();
        
        if(cBound != nullptr) {
            if(dynamic_cast<CLinker*>(cBound))
                _ps->RemoveLinker(static_cast<CLinker*>(cBound)->getLinker());

            else if(dynamic_cast<CMotorGhost*>(cBound))
                _ps->RemoveMotorGhost(static_cast<CMotorGhost*>(cBound)->getMotorGhost());
        }
    }
};


///motor walking forward callback
struct MotorWalkingForwardCallback {
    
    ///members
    SpeciesMotor* _sm1;
    SpeciesMotor* _sm2;
    
    MotorWalkingForwardCallback(SpeciesMotor* sm1, SpeciesMotor* sm2)
        :_sm1(sm1), _sm2(sm2) {}
    
    void operator() (ReactionBase* r) {
        
        MotorGhost* m = static_cast<CMotorGhost*>(_sm1->getCBound())->getMotorGhost();
        
        ///shift the position of one side of the motor forward
        double shift = 1 / SystemParameters::Geometry().cylinderIntSize;
        double newPosition;
        
        if(m->getCMotorGhost()->getFirstSpecies() == _sm1) {
            newPosition = m->getFirstPosition() + shift;
            m->setFirstPosition(newPosition);
            m->getCMotorGhost()->setFirstSpecies(_sm2);
        }
        else {
            newPosition = m->getSecondPosition() + shift;
            m->setSecondPosition(newPosition);
            m->getCMotorGhost()->setSecondSpecies(_sm2);
        }
    }
};

///motor walking backward callback
struct MotorWalkingBackwardCallback {
    
    ///members
    SpeciesMotor* _sm1;
    SpeciesMotor* _sm2;
    
    MotorWalkingBackwardCallback(SpeciesMotor* sm1, SpeciesMotor* sm2)
        :_sm1(sm1), _sm2(sm2) {}
    
    void operator() (ReactionBase* r) {
        
        MotorGhost* m = static_cast<CMotorGhost*>(_sm1->getCBound())->getMotorGhost();
        
        ///shift the position of one side of the motor forward
        double shift = 1 / SystemParameters::Geometry().cylinderIntSize;
        double newPosition;
        
        if(m->getCMotorGhost()->getFirstSpecies() == _sm1) {
            newPosition = m->getFirstPosition() - shift;
            m->setFirstPosition(newPosition);
            m->getCMotorGhost()->setFirstSpecies(_sm2);
        }
        else {
            newPosition = m->getSecondPosition() - shift;
            m->setSecondPosition(newPosition);
            m->getCMotorGhost()->setSecondSpecies(_sm2);
        }
    }
};

///motor moving cylinder forward callback
struct MotorMovingCylinderForwardCallback {
    
    ///members
    SpeciesMotor* _sm1;
    SpeciesMotor* _sm2;
    
    MotorMovingCylinderForwardCallback(SpeciesMotor* sm1, SpeciesMotor* sm2)
        : _sm1(sm1), _sm2(sm2) {}
    
    void operator() (ReactionBase* r) {
        
        MotorGhost* m = static_cast<CMotorGhost*>(_sm1->getCBound())->getMotorGhost();
        
        ///shift the position of one side of the motor forward
        double newPosition = 1 / SystemParameters::Geometry().cylinderIntSize;
        
        if(m->getCMotorGhost()->getFirstSpecies() == _sm1) {
            m->setFirstPosition(newPosition);
            m->getCMotorGhost()->setFirstSpecies(_sm2);
        }
        else {
            m->setSecondPosition(newPosition);
            m->getCMotorGhost()->setSecondSpecies(_sm2);
        }
    }
};


///motor moving cylinder backward callback
struct MotorMovingCylinderBackwardCallback {
    
    ///members
    SpeciesMotor* _sm1;
    SpeciesMotor* _sm2;
    
    MotorMovingCylinderBackwardCallback(SpeciesMotor* sm1, SpeciesMotor* sm2)
        : _sm1(sm1), _sm2(sm2) {}
    
    void operator() (ReactionBase* r) {
        
        MotorGhost* m = static_cast<CMotorGhost*>(_sm1->getCBound())->getMotorGhost();
        
        ///shift the position of one side of the motor forward
        double newPosition = 1.0;
        
        if(m->getCMotorGhost()->getFirstSpecies() == _sm1) {
            m->setFirstPosition(newPosition);
            m->getCMotorGhost()->setFirstSpecies(_sm2);
        }
        else {
            m->setSecondPosition(newPosition);
            m->getCMotorGhost()->setSecondSpecies(_sm2);
        }
    }
};




#endif /* defined(__Cyto__ChemCallbacks__) */
