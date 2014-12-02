
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

#ifndef M3SYM_Output_h
#define M3SYM_Output_h

#include <fstream>

#include "common.h"

/// Output class is to print a specified output into a file
/*!
 *  An output object, initialized by the [Controller] (@ref Controller), can print a number of specific output formats, including 
 *  current snapshot, forces, stresses, and birth times. Upon destruction, the output file is closed.
 */

class Output {

private:
    ofstream _outputFile; ///< The output file being used
    
public:
    /// Constructor, which opens the output file
    Output(string outputFileName) {
        _outputFile.open(outputFileName);
        if(!_outputFile.is_open()) {
            cout << "There was an error opening file " << outputFileName << " for output. Exiting" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Opening file " << outputFileName << endl;
    }
    /// Destructor, which closes the output file
    ~Output() {_outputFile.close();}
    
    /// Print basic information about filaments, linkers, motors (DEPRECATED)
    void printBasicSnapshot(int step);
    
    /// New snapshot output
    void printSnapshot(int step);
    /// Print birth times of beads for each filament
    void printBirthTimes(int step);
    /// Print forces on beads for each filament
    void printForces(int step);
    /// Print stresses on beads for each filament
    void printStresses(int step);
};


#endif