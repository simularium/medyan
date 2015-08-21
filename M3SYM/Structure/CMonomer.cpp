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

#include "CMonomer.h"

#include "CBound.h"

#include "SysParams.h"

vector<short> CMonomer::_speciesFilamentIndex;
vector<short> CMonomer::_speciesBoundIndex;

short CMonomer::_numFSpecies = 0;
short CMonomer::_numBSpecies = 0;


CMonomer::CMonomer() {
    
    _speciesFilament = new SpeciesFilament*[_numFSpecies]();
    _speciesBound = new SpeciesBound*[_numBSpecies]();
};

CMonomer::~CMonomer() noexcept{
    
    delete[] _speciesFilament;
    delete[] _speciesBound;
}


CMonomer::CMonomer(const CMonomer& rhs, Compartment* c) : CMonomer() {

    for(int i = 0; i < _numFSpecies; i++) {
        
        //clone and add to array
        SpeciesFilament* s = rhs._speciesFilament[i];
        SpeciesFilament* sNew = s->clone();
        
        c->addSpeciesUnique(unique_ptr<Species>(sNew));
        _speciesFilament[i] = sNew;
    }
    
    //For bound species, transfer the CBound (if any)
    
    for(int i = 0; i < _numBSpecies; i++) {
        
        //clone and add to array
        SpeciesBound* s = rhs._speciesBound[i];
        SpeciesBound* sNew = s->clone();
        
        c->addSpeciesUnique(unique_ptr<Species>(sNew));
        _speciesBound[i] = sNew;
        
        //update cbound
        CBound* cBound = s->getCBound();
        if(cBound != nullptr) {
            //set species
            if(cBound->getFirstSpecies() == s)
                cBound->setFirstSpecies(sNew);
            else
                cBound->setSecondSpecies(sNew);
        }
    }
}

//PRINT

void CMonomer::print()
{
    for(int i = 0; i < _numFSpecies; i++) {
        SpeciesFilament* s = _speciesFilament[i];
        if(s != nullptr && s->getN() == 1)
            cout << s->getName();
    }
    for(int i = 0; i < _numBSpecies; i++) {
        SpeciesBound* s = _speciesBound[i];
        if(s != nullptr && s->getN() == 1)
            cout << s->getName();
    }
}

//GETTERS

SpeciesFilament* CMonomer::speciesFilament(int index) {
    short offset = _speciesFilamentIndex[SPECIESFILAMENT];
    return _speciesFilament[index + offset];
}
SpeciesFilament* CMonomer::speciesPlusEnd (int index) {
    short offset = _speciesFilamentIndex[SPECIESPLUSEND];
    return _speciesFilament[index + offset];
}
SpeciesFilament* CMonomer::speciesMinusEnd(int index) {
    short offset = _speciesFilamentIndex[SPECIESMINUSEND];
    return _speciesFilament[index + offset];
}

SpeciesBound* CMonomer::speciesBound(int index) {
    short offset = _speciesBoundIndex[SPECIESBOUND];
    return _speciesBound[index + offset];
}
SpeciesBound* CMonomer::speciesLinker(int index) {
    short offset = _speciesBoundIndex[SPECIESLINKER];
    return _speciesBound[index + offset];
}
SpeciesBound* CMonomer::speciesMotor(int index) {
    short offset = _speciesBoundIndex[SPECIESMOTOR];
    return _speciesBound[index + offset];
}
SpeciesBound* CMonomer::speciesBrancher(int index) {
    short offset = _speciesBoundIndex[SPECIESBRANCHER];
    return _speciesBound[index + offset];
}


//GET ACTIVE

short CMonomer::activeSpeciesFilament() {
    short numFilamentSpecies = SysParams::Chemistry().numFilamentSpecies;
    short offset = _speciesFilamentIndex[SPECIESFILAMENT];
    
    for(int i = 0; i < numFilamentSpecies; i++) {
        SpeciesFilament* s = _speciesFilament[i + offset];
        if(s != nullptr && s->getN() == 1) return i;
    }
    return -1;
}
short CMonomer::activeSpeciesPlusEnd() {
    short numPlusEndSpecies = SysParams::Chemistry().numPlusEndSpecies;
    short offset = _speciesFilamentIndex[SPECIESPLUSEND];
    
    for(int i = 0; i < numPlusEndSpecies; i++) {
        SpeciesFilament* s = _speciesFilament[i + offset];
        if(s != nullptr && s->getN() == 1)
            return i;
    }
    return -1;
}
short CMonomer::activeSpeciesMinusEnd() {
    short numMinusEndSpecies = SysParams::Chemistry().numMinusEndSpecies;
    short offset = _speciesFilamentIndex[SPECIESMINUSEND];
    
    for(int i = 0; i < numMinusEndSpecies; i++) {
        SpeciesFilament* s = _speciesFilament[i + offset];
        if(s != nullptr && s->getN() == 1)
            return i;
    }
    return -1;
}

short CMonomer::activeSpeciesLinker() {
    short numLinkerSpecies = SysParams::Chemistry().numLinkerSpecies;
    short offset = _speciesBoundIndex[SPECIESLINKER];
    
    for(int i = 0; i < numLinkerSpecies; i++) {
        SpeciesBound* s = _speciesBound[i + offset];
        if(s != nullptr && s->getN() == 1) return i;
    }
    return -1;
    
}
short CMonomer::activeSpeciesMotor() {
    short numMotorSpecies = SysParams::Chemistry().numMotorSpecies;
    short offset = _speciesBoundIndex[SPECIESMOTOR];
    
    for(int i = 0; i < numMotorSpecies; i++) {
        SpeciesBound* s = _speciesBound[i + offset];
        if(s != nullptr && s->getN() == 1) return i;
    }
    return -1;
}
short CMonomer::activeSpeciesBrancher() {
    short numBrancherSpecies = SysParams::Chemistry().numBrancherSpecies;
    short offset = _speciesBoundIndex[SPECIESBRANCHER];
    
    for(int i = 0; i < numBrancherSpecies; i++) {
        SpeciesBound* s = _speciesBound[i + offset];
        if(s != nullptr && s->getN() == 1) return i;
    }
    return -1;
}

bool CMonomer::isConsistent() {
    
    //check all species between 0 and 1 inclusive
    for(int i = 0; i < _numFSpecies; i++) {
        
        if(_speciesFilament[i]->getN() != 1 &&
           _speciesFilament[i]->getN() != 0) {
            
            cout << _speciesFilament[i]->getName() << " has an invalid copy number. It is = "
                 << _speciesFilament[i]->getN() << " and is at species index " << i << "." << endl;
            
            return false;
            
        }
    }
    
    //check filament species
    if(activeSpeciesFilament() != -1 &&
       (activeSpeciesPlusEnd() != -1 ||
        activeSpeciesMinusEnd() != -1)) {
           
        cout << "Has a simultaneously active filament and plus/minus end species." << endl;
           
        return false;
    }
    
    return true;
}



