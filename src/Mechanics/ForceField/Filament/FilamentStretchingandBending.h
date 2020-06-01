
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

#ifndef MEDYAN_FilamentStretchingandBending_h
#define MEDYAN_FilamentStretchingandBending_h

#include "common.h"
#include "FilamentInteractions.h"
//FORWARD DECLARATIONS
class Filament;

/// Represents a Filament bending interaction
template <class FStretchingandBendingInteractionType>
class FilamentStretchingandBending : public FilamentInteractions {

private:
	FStretchingandBendingInteractionType _FFType;
	const int numthreads = 1;

	// Cache of vectorized data
	std::size_t _numInteractions;
	std::size_t _strnumInteractions;
	int *beadSet;
	///Array describing the constants in calculation
	floatingpoint *kbend;
	floatingpoint *eqt;

	int *beadSetcylsansbending;
	floatingpoint *kstrsansbending;
	floatingpoint *eqlsansbending;
	floatingpoint *kstr;
	floatingpoint *eql;
	floatingpoint *totalenergy;//Will have 3 entries. The total of stretching, bending
	// and sum of stretching + bending
	int *cylSet;//Informs which element in cyllength set to consider.
	int *cylSetcylsansbending;//Informs which element in cyllength set to consider.

	int *beadSetall;//element i helps determine position in coord array to look at.
	bool *beadpaircyllengthstatus;//element i informs if beads i and i+1 form a bond.
	bool *beadtriplethingestatus;//element i informs if bead i, i+1, and i+2 form a hinge.
	floatingpoint *cyllengthset;
	floatingpoint *cylbenddotproduct;


	void precomputevars(floatingpoint *coord, floatingpoint *cyllengthset,
	                    floatingpoint *cylbenddotproduct);

public:

	///Array describing indexed set of interactions
	///For filaments, this is a 3-bead potential
	const static int n = 3;
	const static int nstr = 2;
	const static int ncylperint = 2;

	virtual void vectorize();
	virtual void deallocate();

	virtual floatingpoint computeEnergy(floatingpoint *coord) override;
	virtual void computeForces(floatingpoint *coord, floatingpoint *f);

	virtual const string getName() {return "Filament Stretching and Bending";}
};
#endif