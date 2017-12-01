#ifndef MEDYAN_Edge_h
#define MEDYAN_Edge_h

/*
 
 This structure defines the shared directional edge between two
 triangular surface patches.
 
 The structure is similar to the cylinder.
 
*/

#include <array>

#include "common.h"

#include "Trackable.h"
#include "Movable.h"
#include "DynamicNeighbor.h"
#include "Component.h"

#include "Bead.h"
#include "Triangle.h"
#include "MTriangle.h"

class HalfEdge:
    public Component,
    public Trackable,
    public Movable,
    public DynamicNeighbor {

private:
    // Pointers to the beads.
    std::array<Bead*, 2> _b;

    // ptr to neighbor triangles (_b1, _b2, other1) and (_b2, _b1, other2) <- counter-clockwise dir
    std::array<Triangle*, 2> _neighborTriangle;

public:
    HalfEdge(Composite *parent, Bead *b1, Bead *b2);

    // Get Beads
    std::array<Bead*, 2> getBeads() { return _b; }
    
	// TODO: Add getter and setter for neighbor triangles

};


#endif
