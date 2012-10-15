/**************************************************************************
 * 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * Author: The International Lepton Collider Off-line Project.            *
 *                                                                        *
 * Part of the code has been developed by ILC Off-line Project.         *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

//_________________________________________________________________________
//  Base class for the clusterization algorithm (pure abstract)
//*--
//*-- Author: Yves Schutz  SUBATECH 
//////////////////////////////////////////////////////////////////////////////

#include <TClonesArray.h>
#include <TTree.h>

#include "IlcPVECClusterizer.h"
#include "IlcPVECDigit.h"
#include "IlcLog.h"

ClassImp(IlcPVECClusterizer)

IlcPVECCalibData * IlcPVECClusterizer::fgCalibData  = 0 ; 

//____________________________________________________________________________
IlcPVECClusterizer::IlcPVECClusterizer():
  fGeom(NULL),
  fDigitsArr(0),
  fTreeR(0),
  fEMCRecPoints(0),
  fCPVRecPoints(0)
{
  // ctor
  fDigitsArr    = new TClonesArray("IlcPVECDigit",100);
  fEMCRecPoints = new TObjArray(100) ;
  fEMCRecPoints ->SetName("EMCRECPOINTS") ;
  fCPVRecPoints = new TObjArray(100) ;
  fCPVRecPoints ->SetName("CPVRECPOINTS") ;
}

//____________________________________________________________________________
IlcPVECClusterizer::IlcPVECClusterizer(IlcPVECGeometry *geom):
  fGeom(geom),
  fDigitsArr(0),
  fTreeR(0),
  fEMCRecPoints(0),
  fCPVRecPoints(0)
{
  // ctor
  fDigitsArr    = new TClonesArray("IlcPVECDigit",100);
  fEMCRecPoints = new TObjArray(100) ;
  fEMCRecPoints ->SetName("EMCRECPOINTS") ;
  fCPVRecPoints = new TObjArray(100) ;
  fCPVRecPoints ->SetName("CPVRECPOINTS") ;
}

//____________________________________________________________________________
IlcPVECClusterizer::~IlcPVECClusterizer()
{
  // dtor
  if (fDigitsArr) {
    fDigitsArr->Delete();
    delete fDigitsArr;
  }
  if (fEMCRecPoints) {
    fEMCRecPoints->Delete();
    delete fEMCRecPoints;
  }
  if (fCPVRecPoints) {
    fCPVRecPoints->Delete();
    delete fCPVRecPoints;
  }
}

//____________________________________________________________________________
void IlcPVECClusterizer::SetInput(TTree * digitsTree) 
{
  // Get the tree with digits and sets
  // the input array with digits for PVEC
  TBranch *branch = digitsTree->GetBranch("PVEC");
  if (!branch) { 
    IlcError("can't get the branch with the PVEC digits !");
    return;
  }
  fDigitsArr->Clear();
  branch->SetAddress(&fDigitsArr);
  branch->GetEntry(0);
}

//____________________________________________________________________________
void IlcPVECClusterizer::SetOutput(TTree * clustersTree) 
{
  // Set the output clusters tree,
  // creates the arrays for EMC and CPV,
  // and set the corresponding branch addresses
  fTreeR = clustersTree;

  IlcDebug(9, "Making array for EMC clusters");
  Int_t split = 0;
  Int_t bufsize = 32000;
  fTreeR->Branch("PVECEmcRP", "TObjArray", &fEMCRecPoints, bufsize, split);

  IlcDebug(9, "Making array for CPV clusters");
  fTreeR->Branch("PVECCpvRP", "TObjArray", &fCPVRecPoints, bufsize, split);
}
