//
//  MCGFletcherRievesMethod.cpp
//  Cyto
//
//  Created by Konstantin Popov on 9/8/14.
//  Copyright (c) 2014 University of Maryland. All rights reserved.
//

#include "MCGFletcherRievesMethod.h"


    
double FletcherRieves::GradSquare(BeadDB& list)
{
    double g = 0;
	for(auto it: list) {
        
        g += (*it).CalcForceSquare();
	}
    
    return g;
}

double FletcherRieves::GradSquare(BeadDB& list, int i)
{
    double g = 0;
	for(auto it: list) {
        
        g += (*it).CalcForceSquare(i);
	}
    
    return g;
}

double FletcherRieves::GradDotProduct(BeadDB& list)
{
    double g = 0;
	for(auto it: list) {
        
        g += (*it).CalcDotForceProduct();
	}
    
    return g;
}


void FletcherRieves::MoveBeads(BeadDB& list, double d)
{
	for(auto it: list) {
        
        (*it).coordinate[0] = (*it).coordinate[0] - d* (*it).force[0];
        (*it).coordinate[1] = (*it).coordinate[1] - d* (*it).force[1];
        (*it).coordinate[2] = (*it).coordinate[2] - d* (*it).force[2];
        
	}
    
    
}

void FletcherRieves::ShiftGradient(BeadDB& list, double d)
{
	for(auto it: list) {
        
        (*it).force[0] = (*it).forceAux[0] + d* (*it).force[0];
        (*it).force[1] = (*it).forceAux[1] + d* (*it).force[1];
        (*it).force[2] = (*it).forceAux[2] + d* (*it).force[2];
        
	}
    
    
}


void FletcherRieves::PrintForces(BeadDB& list)
{
	cout << "Print Forces" << endl;
    for(auto it: list) {
        
		for (int i = 0; i<3; i++) { cout << (*it).coordinate[i] << "  "<< (*it).force[i]<<"  "<<(*it).forceAux[i]<<endl;}
	}
}



double FletcherRieves::GoldenSection(MController* mc)
{
	
    const double EPS = 1e-6;
	double a = 0;
	double b = 100;
    double phi = 0.5 * (1 + sqrt(5) );
    double inv_phi = 1/phi;
	double x1 = b - inv_phi * (b - a);
	double x2 = a + inv_phi * (b - a);
    
    
    
	while (abs(b - a) > EPS)
	{
		if (mc->ComputeEnergy(x1) >= mc->ComputeEnergy(x2) ){
            
            a = x1;
            x1 = x2;
            x2 =a + inv_phi * (b - a);
        }
		
        else {
            
            b = x2;
            
            x2 = x1;
            x1 = b - inv_phi * (b - a);
            
        }
    }
    
    
	return (a + b)/2.0;
}

void FletcherRieves::Minimize(MController* mc)
{
	
	const double EPS = 1e-10;
	
    int SpaceSize = 3 * ps->getSystemSize(); ///!!!!!! need to know
	double curVal = mc->ComputeEnergy(0.0);
    cout<<"Energy = "<< curVal <<endl;
	double prevVal = curVal;
	mc->CopmuteForce();
    
    PrintForces(*ps->getBDB());
    
    
	double gradSquare = GradSquare(*ps->getBDB());
    
    cout<<"GradSq=  "<<gradSquare<<endl;
    
	int numIter = 0;
	do
	{
		numIter++;
		double lambda, beta, newGradSquare;
		vector<double> newGrad;
        
        
		
        lambda =0.01;
        //GoldenSection(pf);
        cout<<"lambda= "<<lambda<<endl;
		PrintForces(*ps->getBDB());
        MoveBeads(*ps->getBDB(), lambda);
        PrintForces(*ps->getBDB());
        
        ps->CopmuteForceAux();
        PrintForces(*ps->getBDB());
        
		newGradSquare = GradSquare(*ps->getBDB(), 1);
		
		if (numIter % (5 * SpaceSize) == 0) beta = 0;
		else
			beta = newGradSquare / gradSquare;
		ShiftGradient(*ps->getBDB(), beta);
        
		prevVal = curVal;
		curVal = mc->ComputeEnergy(0.0);
        
		gradSquare = newGradSquare;
	}
	while (gradSquare > EPS);
    
    
	std::cout << "Fletcher-Rieves Method: " << std::endl;
    cout<<"numIter= " <<numIter<<"  Spacesize = "<<SpaceSize <<endl;
	PrintForces(*ps->getBDB());
	
    
}


