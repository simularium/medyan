//
//  GController.cpp
//  Cyto
//
//  Created by James Komianos on 8/5/14.
//  Copyright (c) 2014 University of Maryland. All rights reserved.
//

#include "GController.h"
#include "Parser.h"

void GController::initializeGrid(short nDim, std::vector<int> grid, std::vector<double> compartmentSize) {
    
    _nDim = nDim;
    
    /// set up
    if(_nDim == 1) {
        _grid = {grid[0]};
        _compartmentSize = {compartmentSize[0]};
    }
    else if(_nDim == 2) {
        _grid = {grid[0], grid[1]};
        _compartmentSize = {compartmentSize[0], compartmentSize[1]};
        
    }
    else {
        _grid = {grid[0], grid[1], grid[2]};
        _compartmentSize = {compartmentSize[0], compartmentSize[1], compartmentSize[2]};
    }
    
    int size = 1;
    int i = 0;
    for(auto x: _grid) {
        size*=x;
        i++;
    }
    
    ///Set the instance of this grid with given parameters
    CompartmentGrid::setInstance(CompartmentGridKey(), size);
    
    ///Create connections based on dimensionality
    generateConnections();
}


void GController::generateConnections()
{
    
    //Three dimensional
    if (_nDim == 3) {
        
        for(size_t i=0U; i<_grid[0]; ++i) {
            
            for(size_t j=0U; j<_grid[1]; ++j) {
                
                for(size_t k=0U; k<_grid[2]; ++k)
                {
                    Compartment *target = getCompartment(i,j,k);
                    
                    for(int ii: {-1,1})
                    {
                        int iprime = i+ii;
                        if(iprime<0 or iprime==int(_grid[0]))
                            continue;
                        Compartment *neighbor = getCompartment(size_t(iprime),j,k);
                        target->addNeighbour(neighbor);
                    }
                    for(int jj: {-1,1})
                    {
                        int jprime = j+jj;
                        if(jprime<0 or jprime==int(_grid[1]))
                            continue;
                        Compartment *neighbor = getCompartment(i,size_t(jprime),k);
                        target->addNeighbour(neighbor);
                    }
                    for(int kk: {-1,1})
                    {
                        int kprime = k+kk;
                        if(kprime<0 or kprime==int(_grid[2]))
                            continue;
                        Compartment *neighbor = getCompartment(i,j,size_t(kprime));
                        target->addNeighbour(neighbor);
                    }
                }
            }
        }
    }
    
    //Two dimensional
    else if (_nDim == 2) {
        
        for(size_t i=0U; i<_grid[0]; ++i) {
            
            for(size_t j=0U; j<_grid[1]; ++j) {
                
                Compartment *target = getCompartment(i,j);
                
                for(int ii: {-1,1})
                {
                    int iprime = i+ii;
                    if(iprime<0 or iprime==int(_grid[0]))
                        continue;
                    Compartment *neighbor = getCompartment(size_t(iprime),j);
                    target->addNeighbour(neighbor);
                }
                for(int jj: {-1,1})
                {
                    int jprime = j+jj;
                    if(jprime<0 or jprime==int(_grid[1]))
                        continue;
                    Compartment *neighbor = getCompartment(i,size_t(jprime));
                    target->addNeighbour(neighbor);
                }
            }
        }
    }
    
    //One dimensional
    else {
        for(size_t i=0U; i<_grid[0]; ++i) {
            
            Compartment *target = getCompartment(i);
            
            for(int ii: {-1,1})
            {
                int iprime = i+ii;
                if(iprime<0 or iprime==int(_grid[0]))
                    continue;
                Compartment *neighbor = getCompartment(size_t(iprime));
                target->addNeighbour(neighbor);
            }
        }
    }
}

short GController::_nDim = 0;
std::vector<int> GController::_grid = {};
std::vector<double> GController::_compartmentSize = {};


