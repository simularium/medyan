
//------------------------------------------------------------------
//  **MEDYAN** - Simulation Package for the Mechanochemical
//               Dynamics of Active Networks, v3.1
//
//  Copyright (2015-2016)  Papoian Lab, University of Maryland
//
//                 ALL RIGHTS RESERVED
//
//  See the MEDYAN web page for more information:
//  http://www.medyan.org
//------------------------------------------------------------------

#include "CaMKIIingPoint.h"

#include "SubSystem.h"
#include "Bead.h"
#include "Cylinder.h"
#include "Filament.h"
#include "ChemRNode.h"
#include "CompartmentGrid.h"

#include "GController.h"
#include "SysParams.h"
#include "MathFunctions.h"
#include "Rand.h"
#include "../GController.h"


using namespace mathfunc;

CaMKIIingPoint::CaMKIIingPoint(Cylinder* cylinder, short camkiiType, double position)
    : Trackable(true), _camkiiType(camkiiType), _camkiiID(_camkiiingPoints.getID()), _birthTime(tau()) {

	cerr << "==== MILLAD: " << __FUNCTION__ << "   ADDR: " << (void*)this << " --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;

	assert(camkiiType == 0);

    int pos = int(position * SysParams::Geometry().cylinderNumMon[cylinder->getType()]);

#ifdef CHEMISTRY
    _cCaMKIIingPoint = unique_ptr<CCaMKIIingPoint>(
    new CCaMKIIingPoint(camkiiType, _compartment, cylinder->getCCylinder(), cylinder->getCCylinder(), pos));
    _cCaMKIIingPoint->setCaMKIIingPoint(this);
#endif

#ifdef MECHANICS
    _mCaMKIIingPoint = unique_ptr<MCaMKIIingPoint>(new MCaMKIIingPoint(camkiiType));
    _mCaMKIIingPoint->setCaMKIIingPoint(this);
#endif


    vector<double> _tmp_coordinate(3, 0.0);

    // Creating and initializing CaMKII cylinder
    Bead* b1 = _subSystem->addTrackable<Bead>(_tmp_coordinate, nullptr, 0);
    coordinate = &b1->coordinate;
    setCaMKIICylinder(_subSystem->addTrackable<CaMKIICylinder>(this, b1, _filType, 0));
    _camkiiCylinder->addToFilamentBindingManagers();

    // Setting the first species to CaMKII dummy cylinder species
	SpeciesBound* sfs = _camkiiCylinder->getCCylinder()->getCMonomer(0)->speciesCaMKIIDummyCylinder(camkiiType);
    _cCaMKIIingPoint->setFirstSpecies(sfs);

	addBond(cylinder, pos);

	//Find compartment
	updateCoordinate(coordinate);
	try {_compartment = GController::getCompartment(*coordinate);}
	catch (exception& e) {
		cout << e.what();
		printSelf();
		exit(EXIT_FAILURE);
	}
	cerr << "==== MILLAD: " << __FUNCTION__ << " --- AFTER --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;

}

void CaMKIIingPoint::addBond(Cylinder *c, short pos) {
	cerr << "==== MILLAD: " << __FUNCTION__ << "   ADDR: " << (void*)this << " --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;
	_cCaMKIIingPoint->addBond(c->getCCylinder(), pos);
	_bonds.push_back(tuple<Cylinder *, short>(c, pos));
	cerr << "==== MILLAD: " << __FUNCTION__ << " --- AFTER --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;
}

void CaMKIIingPoint::updateCoordinate(vector<double> *coordinate) {

	cerr << "==== MILLAD: " << __FUNCTION__ << "   ADDR: " << (void*)this << " --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;

	//Calculate the midpoint coordinate
	vector<double> temp(3, 0.0);
	for (size_t i = 0; i < _bonds.size(); i++) {
		Cylinder *bond = get<0>(_bonds[i]);
		auto pos = get<1>(_bonds[i]);
		double position = double(pos) / double(SysParams::Geometry().cylinderNumMon[bond->getType()]);
		auto mp = midPointCoordinate(bond->_b1->coordinate, bond->_b2->coordinate, position);
		temp[0] += mp[0];
		temp[1] += mp[1];
		temp[2] += mp[2];
	}

	// Set the CaMKII coordinates
	for (size_t i = 0; i < 3; i++) {
		(*coordinate)[i] = temp[i] / _bonds.size();
		//Constraints the coordinates inside the box
		if ((*coordinate)[i] > GController::getSize()[i])
			(*coordinate)[i] = GController::getSize()[i] - 1E-5;

		if ((*coordinate)[i] < 0.0)
			(*coordinate)[i] = 0.0 + 1E-5;
	}
	cerr << "==== MILLAD: " << __FUNCTION__ << " --- AFTER --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;
}

CaMKIIingPoint::~CaMKIIingPoint() noexcept {
	//TODO: _subSystem->removeTrackable<unique_ptr<CaMKIICylinder>>(_camkiiCylinder);

	cerr << "==== MILLAD: " << __FUNCTION__ << "   ADDR: " << (void*)this << " --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;

}

void CaMKIIingPoint::updatePosition() {
	cerr << "==== MILLAD: " << __FUNCTION__ << "   ADDR: " << (void*)this << " --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;

#ifdef CHEMISTRY
    //update ccylinders
//    for (size_t i=0; i<_bonds.size(); i++)
//        _cCaMKIIingPoint->setConnectedCCylinder(getCylinder(i)->getCCylinder());
//	_cCaMKIIingPoint->setConnectedCCylinder(get<0>(_bonds[0])->getCCylinder());

//	auto ccyl=get<0>(_bonds[0]);
//	if(ccyl == nullptr){
//	    //A cylinder was removed
//	    cerr << "CC1 is NULL\n";
//		exit(1);
//	}
//	assert(_bonds.size() == 1);

#endif

	assert(coordinate != nullptr);
    //Find compartment
    updateCoordinate(coordinate);
    
    Compartment* c;
    
    try {c = GController::getCompartment(*coordinate);}
    catch (exception& e) {
        cout << e.what();
        
        printSelf();

		cerr << "==== MILLAD: " << __FUNCTION__ << " --- AFTER --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;
        exit(EXIT_FAILURE);
    }
    
    if(c != _compartment) {
        _compartment = c;
#ifdef CHEMISTRY
        SpeciesBound* firstSpecies = _cCaMKIIingPoint->getFirstSpecies();
        
        CCaMKIIingPoint* clone = _cCaMKIIingPoint->clone(c);
        setCCaMKIIingPoint(clone);

        _cCaMKIIingPoint->setFirstSpecies(firstSpecies);
#endif
    }
	cerr << "==== MILLAD: " << __FUNCTION__ << " --- AFTER --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;
}
            
void CaMKIIingPoint::printSelf() {

	cerr << "==== MILLAD: " << __FUNCTION__ << "   ADDR: " << (void*)this << " --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;
    cout << endl;
    
    cout << "CaMKIIingPoint: ptr = " << this << endl;
    cout << "CaMKIIing type = " << _camkiiType << ", CaMKII ID = " << _camkiiID << endl;
    cout << "Coordinates = " << (*coordinate)[0] << ", " << (*coordinate)[1] << ", " << (*coordinate)[2] << endl;
    
    cout << "Position on mother cylinder (short) = " << get<1>(_bonds.at(0)) << endl;
    cout << "Birth time = " << _birthTime << endl;
    
    cout << endl;
    
#ifdef CHEMISTRY
    cout << "Associated species = " << _cCaMKIIingPoint->getFirstSpecies()->getName()
         << " , copy number = " << _cCaMKIIingPoint->getFirstSpecies()->getN()
         << " , position on mother cylinder (int) = " << _cCaMKIIingPoint->getFirstPosition() << endl;
#endif
    
    cout << endl;
    
    cout << "Associated cylinders (mother and camkiiing): " << endl;
    getCylinder(0)->printSelf();
    //getCylinder(1)->printSelf();
    
    cout << endl;
	cerr << "==== MILLAD: " << __FUNCTION__ << " --- AFTER --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;
}
            
species_copy_t CaMKIIingPoint::countSpecies(const string& name) {
	// TODO: make countSpecies count for numbers of CA species on cylinder from bonds
    species_copy_t copyNum = 0;

    for(auto b : _camkiiingPoints.getElements()) {

        auto s = b->getCCaMKIIingPoint()->getFirstSpecies();
        string sname = SpeciesNamesDB::removeUniqueFilName(s->getName());

        if(sname == (name + "D"))
//            copyNum += s->getN();
			copyNum += 1;
    }
    return copyNum;
}

species_copy_t CaMKIIingPoint::countDummySpecies(const string& name) {

	/*
	 * This function counts the total number of the dummy cylinder species in the system.
	 * In case of multiple type of CaMKII, sname and name as in countSpecies(const string& name) should be implemented.
	 */

	species_copy_t copyNum = 0;

	for(auto b : _camkiiingPoints.getElements()) {

		auto c = b->getCaMKIICylinder();

		for(int i = 0; i < c->getCCylinder()->getSize(); i++) {
			auto m = c->getCCylinder()->getCMonomer(i);

			//CaMKII species
			const int activeIndex = m->activeSpeciesCaMKIIDummyCylinder();

			if(activeIndex != -1) copyNum++;
		}
	}
	return copyNum;
}


tuple<Cylinder*, short> CaMKIIingPoint::removeRandomBond() {
	cerr << "==== MILLAD: " << __FUNCTION__ << "   ADDR: " << (void*)this << " --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;
	assert(_bonds.size() > 1);

	size_t sz = _bonds.size();
	size_t index = (size_t) Rand::randInteger(0, sz-1);
	cerr << "=== MILLAD INDEX : " << index << " out of " << sz << endl;
	tuple<Cylinder*, short> bondToRemove = _bonds[index];

	_cCaMKIIingPoint->removeBond(get<0>(bondToRemove)->getCCylinder(), get<1>(bondToRemove));


	_bonds.erase(_bonds.begin() + index);
	cerr << "==== MILLAD: " << __FUNCTION__ << " --- AFTER --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;

	return bondToRemove;
}

void CaMKIIingPoint::updateReactionRates() {
	cerr << "==== MILLAD: " << __FUNCTION__ << "   ADDR: " << (void*)this << " --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;

	//get the unbinding reaction
	ReactionBase* offRxn = getCCaMKIIingPoint()->getOffReaction();

	//change the rate
	float newRate = offRxn->getBareRate();
	if(SysParams::RUNSTATE==false)
		newRate=0.0;

	offRxn->setRate(newRate);
	offRxn->activateReaction();
	offRxn->updatePropensity();
//	assert(offRxn->isPassivated() == true);

	cerr << "==== MILLAD: " << __FUNCTION__ << " --- AFTER --- ID: " << getID() << "  SIZE: " << _bonds.size() << endl;
}


Database<CaMKIIingPoint*> CaMKIIingPoint::_camkiiingPoints;
