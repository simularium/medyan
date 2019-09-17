
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

/*! \mainpage MEDYAN software package
 
 \section intro_sec Introduction
 
The cell cytoskeleton plays a key role in human biology and disease, contributing ubiquitously
 to such important processes as embryonic development, wound repair and cancer 
 metastasis. Papoian laboratory is interested in gaining deeper understanding of the
 physical chemistry behind these complex, far-from-equilibrium mechanochemical 
 processes. Their approach and model, named Mechanochemical Dynamics of Active Networks
 (**MEDYAN**), is based on combining stochastic reaction-diffusion treatment
 of cellular biochemical processes with polymer physics of cytoskeletal filament network 
 growth, while explicitly coupling chemistry and mechanics.
 
 Papoian laboratory has developed a third-generation software package based on
 the **MEDYAN** model, to simulate growth dynamics of actin based filamentous networks *in vitro* 
 and *in vivo*. Recent papers where **MEDYAN** or its second-generation predecessor, **StochTools**,
 were used can be found on the publication section of [the Papoian group's main web page]
 (http://papoian.chem.umd.edu/ ) or on [the MEDYAN web page] (http://www.medyan.org ).
 The **MEDYAN** package can also be extended to simulate the dynamics of any active
 matter network.
 
 \section install_sec Installation
 
 \subsection step1 Step 1: Prerequisites
 
 The following libraries need to be installed first:
 See Installation guide (docs/InstallGuide.pdf) for more details.
 
 - Boost 1.49
 - GSL ...
 
 \subsection step2 Step 2: Installation of MEDYAN itself
 
 Untar the MEDYAN source code into some directory, enter
 into the "MEDYAN" folder and execute "make" from the command line.
 
 See Installation guide (docs/InstallGuide.pdf) for more information
 on setting command line compilation macros, compiler compatibility, etc.
 
 \subsection step3 Step 3: Running MEDYAN
 
 See the Usage guide (docs/UsageGuide.pdf) for more information. The MEDYAN
 executable must be run with the following command line arguments:
 
 -s : System input file to be used. Must be an absolute path
 -i : Input directory to be used, where all files specified in the
      system input file must be located. Must be an absolute path.
 -o : Output directory to be used (must be created beforehand),
      where all output files will be placed. Must be an absolute path.
 
 Run -h for help.
 
 */

#include <thread>

#include "common.h"

#include "Analysis/Io/read_snapshot.h"
#include "Controller.h"
#include "Core/Globals.hpp"
#include "MedyanArgs.hpp"
#include "Util/ThreadPool.hpp"
#include "Visual/Window.hpp"
#include "VisualHelper.hpp"

using namespace medyan;

int main(int argc, char **argv) {

    cout << endl;
    cout << "*********************** MEDYAN ************************" << endl;
    cout << "   Simulation package for the Mechanochemical Dynamics " << endl;
    cout << "         of Active Networks, Third Generation.         " << endl;
    cout << "         PAPOIAN LAB 2015, ALL RIGHTS RESERVED         " << endl;
    cout << "*******************************************************" << endl;
    
    cout.precision(8);

    auto cmdRes = medyanInitFromCommandLine(argc, argv);

    // Initialize the thread pool for use in MEDYAN
    ThreadPool tp(cmdRes.numThreads);

    /**************************************************************************
    Start program 
    **************************************************************************/
    switch(global().mode) {
    case GlobalVar::RunMode::Simulation:
        //initialize and run system
        {
            Controller c;
            c.initialize(cmdRes.inputFile, cmdRes.inputDirectory, cmdRes.outputDirectory, tp);

            std::thread mainThread(&Controller::run, &c);
#ifdef VISUAL
            {
                auto vd = std::make_shared< visual::VisualDisplay >();
                visual::vdWeak = vd;
                vd->run();
            }
#endif // VISUAL
            mainThread.join();
        }
        break;
    case GlobalVar::RunMode::Analysis:
        {
            string inputFilePath = cmdRes.inputDirectory + "/snapshot.traj";
            string pdbFilePath = cmdRes.outputDirectory + "/snapshot.pdb";
            string psfFilePath = cmdRes.outputDirectory + "/snapshot.psf";
            analysis::SnapshotReader sr(inputFilePath, pdbFilePath, psfFilePath);
            sr.readAndConvertToVmd();
        }
        break;
    }

    return 0;
}

