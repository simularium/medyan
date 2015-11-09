
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

#ifndef M3SYM_BubbleInitializer_h
#define M3SYM_BubbleInitializer_h

#include "common.h"

typedef vector<tuple<short, vector<double>>> BubbleData;

///FORWARD DECLARATIONS
class Boundary;

/// An interface to initialize an initial configuration of [Bubbles](@ref Bubble)
/// in the SubSystem.
/*!
 *  Bubblenitiazer class should be inherited to provide an intial scheme for
 *  filling a SubSystem with [Bubbles](@ref Bubble). The bubbles could be
 *  completely random or distributed in other ways.
 */
class BubbleInitializer {
    
public:
    /// Destructor
    /// @note noexcept is important here. Otherwise, gcc flags the constructor as
    /// potentially throwing, which in turn disables move operations by the STL
    /// containers. This behaviour is a gcc bug (as of gcc 4.703), and will presumbaly
    /// be fixed in the future.
    virtual ~BubbleInitializer() noexcept {}
    
    /// Returns a vector of tuples representing the Bubble type and beginning and end
    /// coordinates, similar to the structure of manual parsing.
    virtual BubbleData createBubbles(Boundary* b, int numBubbles,
                                                  int bubbleType) = 0;
};

/// An implementation of BubbleInitialzer that creates a completely random
/// Bubble distribution.
class RandomBubbleDist : public BubbleInitializer {
    
public:
    BubbleData createBubbles(Boundary* b, int numBubbles,
                                          int bubbleType);
};

#endif