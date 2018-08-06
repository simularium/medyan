
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

#include "Bubble.h"

#include "SubSystem.h"
#include "Bead.h"

#include "SysParams.h"

Bubble::Bubble(SubSystem* ps, vector<double> coordinates, short type)

    : Trackable(true, false, true, false), _ps(ps), _type(type),
      _ID(_bubbles.getID()), coordinate(coordinates) {
    
    //set up mechanical constants
    _kRepuls = SysParams::Mechanics().BubbleK[_type];
    _radius  = SysParams::Mechanics().BubbleRadius[_type];
    _screenLength = SysParams::Mechanics().BubbleScreenLength[_type];
          
    //set up bead
    _bead = _ps->addTrackable<Bead>(coordinates, this, 0);
}

void Bubble::updatePosition() {
    
    coordinate = _bead->coordinate;
}

void Bubble::printSelf() {
    
    cout << endl;
    
    cout << "Bubble: ptr = " << this << endl;
    cout << "Bubble ID = " << _ID << endl;
    cout << "Bubble type = " << _type << endl;
    cout << "Bubble radius = " << _radius << endl;
    
    cout << endl;
    
    cout << "Bead information..." << endl;
    
    _bead->printSelf();
    
    cout << endl;
}

Database<Bubble*> Bubble::_bubbles;