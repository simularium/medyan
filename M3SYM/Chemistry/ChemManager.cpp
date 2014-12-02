
//------------------------------------------------------------------
//  **M3SYM** - Simulation Package for the Mechanochemical
//              Dynamics of Active Networks, 3rd Generation
//
//  Copyright (2014) Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the Papoian lab page for installation and documentation:
//  http://papoian.chem.umd.edu/
//------------------------------------------------------------------

#include "ChemManager.h"

ChemManagerImpl* ChemManager::_pimpl = 0;

void ChemManager::setInstance(ChemManagerImpl *cii)
{
    if(_pimpl != 0) delete _pimpl;
    _pimpl=cii;
}

void ChemManager::initialize(ChemistryData& chem)
{
    _pimpl->initialize(chem);
}

void ChemManager::initializeCCylinder(CCylinder* cc, Filament* f, bool extensionFront, bool extensionBack, bool creation)

{
    _pimpl->initializeCCylinder(cc, f, extensionFront, extensionBack, creation);
}

void ChemManager::updateCCylinder(CCylinder* cc) {
    
    _pimpl->updateCCylinder(cc);
}