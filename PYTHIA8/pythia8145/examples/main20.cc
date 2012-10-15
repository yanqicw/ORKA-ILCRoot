// main20.cc is a part of the PYTHIA event generator.
// Copyright (C) 2010 Torbjorn Sjostrand.
// PYTHIA is licenced under the GNU GPL version 2, see COPYING for details.
// Please respect the MCnet Guidelines, see GUIDELINES for details.

// This is a simple test program. It shows how PYTHIA 8 can write
// a Les Houches Event File based on its process-level events.

#include "Pythia.h"
using namespace Pythia8; 
int main() {

  // Generator. 
  Pythia pythia;

  // Process selection. Minimal masses for gamma*/Z and W+-.
  pythia.readString("WeakDoubleBoson:all = on");    
  pythia.readString("23:mMin = 50.");
  pythia.readString("24:mMin = 50.");

  // Switch off generation of steps subsequent to the process level one.
  // (These will not be stored anyway, so only steal time.)
  pythia.readString("PartonLevel:all = off");    

  // Create an LHAup object that can access relevant information in pythia.
  LHAupFromPYTHIA8 myLHA(&pythia.process, &pythia.info);

  // Open a file on which LHEF events should be stored, and write header.
  myLHA.openLHEF("weakbosons.lhe"); 

  // Tevatron initialization. 
  pythia.init( 2212, -2212, 1960.);

  // Store initialization info in the LHAup object. 
  myLHA.setInit();

  // Write out this initialization info on the file.
  myLHA.initLHEF();

  // Loop over events.
  for (int i = 0; i < 100; ++i) {

    // Generate an event.
    pythia.next();

    // Store event info in the LHAup object. 
    myLHA.setEvent();

    // Write out this event info on the file. 
    myLHA.eventLHEF();
  }

  // Statistics: full printout.
  pythia.statistics();

  // Update the cross section info based on Monte Carlo integration during run.
  myLHA.updateSigma();

  // Write endtag. Overwrite initialization info with new cross sections.
  myLHA.closeLHEF(true);

  // Done.
  return 0;
}
