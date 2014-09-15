//
//  test_compartments.cpp
//  CytoSim
//
//  Created by James Komianos on 6/9/14.
//  Copyright (c) 2014 University of Maryland. All rights reserved.
//

//#define DO_THIS_COMPARTMENT_TEST

#ifdef DO_THIS_COMPARTMENT_TEST
#define TESTING

#include <iostream>
#include "gtest/gtest.h"
#include "GController.h"
#include "CompartmentContainer.h"

using namespace std;

//Testing neighbor capabilities
TEST(CompartmentTest, Neighbors) {

    Compartment *C1 = new Compartment;
    Compartment *C2 = C1->clone();
    C1->addNeighbour(C2);
    C2->addNeighbour(C1);
    
    EXPECT_EQ(1, C1->numberOfNeighbours());
    EXPECT_EQ(1, C2->numberOfNeighbours());
    
    Compartment *C3 = C2->clone();
    C1->addNeighbour(C3);
    
    EXPECT_EQ(2, C1->numberOfNeighbours());
    EXPECT_EQ(0, C3->numberOfNeighbours());
    
    C1->removeNeighbour(C2);
    
    EXPECT_EQ(1, C1->numberOfNeighbours());
}

//Testing species and reaction generation
TEST(CompartmentTest, SpeciesAndReactions){
    
    Compartment *C1 = new Compartment;
    C1->activate();
    
    Species *actin = C1->addSpecies("Actin",99U);
    C1->setDiffusionRate(actin,2000);
    Species *profilin = C1->addSpecies("Profilin",29U);
    C1->setDiffusionRate(profilin,2000);
    Species *arp23 = C1->addSpecies("Arp2/3",33U);
    C1->setDiffusionRate(arp23,2000);
    
    Compartment *C2 = C1->clone();
    
    EXPECT_EQ(2000,C1->getDiffusionRate("Actin"));
    EXPECT_EQ(2000,C2->getDiffusionRate("Actin"));
    
    C1->setDiffusionRate(actin, 9000);
    EXPECT_EQ(9000, C1->getDiffusionRate("Actin"));
    EXPECT_EQ(2000, C2->getDiffusionRate("Actin"));
    
    
    C1->addNeighbour(C2);
    C2->addNeighbour(C1);
    C1->generateAllDiffusionReactions();
    C2->generateAllDiffusionReactions();
    
    EXPECT_EQ(3, C1->numberOfSpecies());
    EXPECT_EQ(3, C2->numberOfSpecies());
    
    EXPECT_EQ(0, C1->numberOfInternalReactions());
    EXPECT_EQ(3, C1->numberOfReactions());
    EXPECT_EQ(0, C2->numberOfInternalReactions());
    EXPECT_EQ(3, C2->numberOfReactions());

}

//Testing neighbors, species and reaciton generation
TEST(CompartmentContainerTest, Main) {
    
    int _numSpecies; ///for testing
    int NUM_SPECIES_EXPECTED;
    
    GController g;
    g.initializeGrid();
    
    Compartment &Cproto = CompartmentGrid::Instance(CompartmentGridKey())->getProtoCompartment();
    
    Species *M1 = Cproto.addSpecies("Myosin",1U);
    Cproto.setDiffusionRate(M1,2000);
    Species *M2 = Cproto.addSpecies("Fascin",6U);
    Cproto.setDiffusionRate(M2,2000);
    
    Cproto.addInternal<Reaction,1,1>({M2,M1}, 90.9);
    Cproto.addInternal<Reaction,1,1>({M1,M2}, 40.2);
    
    ///initialize all compartments with species
    for(auto &c : CompartmentGrid::Instance(CompartmentGridKey())->children())
    {
        Compartment *C = static_cast<Compartment*>(c.get());
        *C = Cproto;
    }
    
    ///activate all compartments for diffusion
    CompartmentGrid::Instance(CompartmentGridKey())->activateAll();
    
    ///Generate all diffusion reactions
    for(auto &c : CompartmentGrid::Instance(CompartmentGridKey())->children())
    {
        Compartment *C = static_cast<Compartment*>(c.get());
        C->generateAllDiffusionReactions();
    }
    
    _numSpecies = CompartmentGrid::Instance(CompartmentGridKey())->countSpecies();
    
    NUM_SPECIES_EXPECTED = 2;
    for (auto &x : GRID) NUM_SPECIES_EXPECTED *= x;
    
    
    EXPECT_EQ(_numSpecies, NUM_SPECIES_EXPECTED);
    
    Compartment *C1 = GController::getCompartment(10U,10U,10U);
    
    EXPECT_EQ(6, C1->numberOfNeighbours());
    EXPECT_EQ(2, C1->numberOfSpecies());
    EXPECT_EQ(14, C1->numberOfReactions());
    
    Compartment*C2 = GController::getCompartment(10U,11U,10U);
    C1->removeNeighbour(C2);
    
    EXPECT_EQ(5, C1->numberOfNeighbours());
    
}

#endif // DO_THIS_COMPARTMENT_TEST





