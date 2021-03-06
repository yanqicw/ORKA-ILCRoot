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


//======================================================================
// ***July 2006
// Fill Unit Array class 
// Class used by IlcJetESDReader to fill a UnitArray from the information extracted 
// from the particle tracks
// Author: magali.estienne@ires.in2p3.fr
//======================================================================


// --- ROOT system ---
#include <TVector3.h>
#include <TProcessID.h>

// --- IlcRoot header files ---
#include "IlcJetUnitArray.h"
#include "IlcJetHadronCorrectionv1.h"
#include "IlcJetESDFillUnitArrayTracks.h"

// --- ROOT system ---
class TSystem;
class TLorentzVector;
class TGeoManager;

// --- IlcRoot header files ---
class IlcJetFinder;
class IlcJetReader;
class IlcJetESDReader;

ClassImp(IlcJetESDFillUnitArrayTracks)

//_____________________________________________________________________________
IlcJetESDFillUnitArrayTracks::IlcJetESDFillUnitArrayTracks()
  : IlcJetFillUnitArray(),
    fNumUnits(0),
    fHadCorr(0),
    fApplyMIPCorrection(kTRUE),
    fESD(0x0),
    fGrid0(0x0),
    fGrid1(0x0),
    fGrid2(0x0),
    fGrid3(0x0),
    fGrid4(0x0)
{
  // constructor
}

//_____________________________________________________________________________
IlcJetESDFillUnitArrayTracks::IlcJetESDFillUnitArrayTracks(IlcESDEvent* esd)
  : IlcJetFillUnitArray(),
    fNumUnits(0),
    fHadCorr(0),
    fApplyMIPCorrection(kTRUE),
    fESD(esd),
    fGrid0(0x0),
    fGrid1(0x0),
    fGrid2(0x0),
    fGrid3(0x0),
    fGrid4(0x0)
{
  // constructor
}

//_____________________________________________________________________________
IlcJetESDFillUnitArrayTracks::IlcJetESDFillUnitArrayTracks(const IlcJetESDFillUnitArrayTracks &det)
  : IlcJetFillUnitArray(det),
    fNumUnits(det.fNumUnits),
    fHadCorr(det.fHadCorr),
    fApplyMIPCorrection(det.fApplyMIPCorrection),
    fESD(det.fESD),
    fGrid0(det.fGrid0),
    fGrid1(det.fGrid1),
    fGrid2(det.fGrid2),
    fGrid3(det.fGrid3),
    fGrid4(det.fGrid4)
{
  // Copy constructor
}

//------------------------------------------------------------------
IlcJetESDFillUnitArrayTracks& IlcJetESDFillUnitArrayTracks::operator=(const IlcJetESDFillUnitArrayTracks& other)
{
  // Assignment

    fNumUnits = other.fNumUnits;
    fHadCorr = other.fHadCorr;
    fApplyMIPCorrection = other.fApplyMIPCorrection;
    fESD = other.fESD;
    fGrid0 = other.fGrid0;
    fGrid1 = other.fGrid1;
    fGrid2 = other.fGrid2;
    fGrid3 = other.fGrid3;
    fGrid4 = other.fGrid4;

    return (*this);
}

//____________________________________________________________________________
void IlcJetESDFillUnitArrayTracks::InitParameters()
{
  //  fHadCorr        = 0;                 // For hadron correction
  fNumUnits = 0;//fGeom->GetEMCGeometry()->GetNCells();      // Number of towers in EMCAL
  cout << "In IlcJetESDFillUnitArrayTracks:InitParameters(), Ncells : " << fNumUnits << endl;

  fTPCGrid->GetAccParam(fNphi,fNeta,fPhiMin, 
			fPhiMax,fEtaMin,fEtaMax);
  fTPCGrid->GetBinParam(fPhiBinInTPCAcc,fEtaBinInTPCAcc, 
			fPhiBinInEMCalAcc,fEtaBinInEMCalAcc,fNbinPhi);

  fIndex = fTPCGrid->GetIndexObject();

  if(fDebug>20){
    for(Int_t i=0; i<fNphi+1; i++)
      for(Int_t j=0; j<fNeta+1; j++) {cout << "fIndex[" << i << "," << j << "] : " <<
	  (*fIndex)(i,j) << endl; }
  } 
  if(fDebug>1) printf("\n Parameters initiated ! \n");
}

//_____________________________________________________________________________
IlcJetESDFillUnitArrayTracks::~IlcJetESDFillUnitArrayTracks()
{
  // destructor
}

//_____________________________________________________________________________
void IlcJetESDFillUnitArrayTracks::Exec(Option_t* const /*option*/)
{
  //
  // Main method.
  // Fill the unit array with the charged particle information in ESD
  //

  fDebug = fReaderHeader->GetDebug();
  
  // Set parameters
  InitParameters();
  //  fRef->Clear();

  // get number of tracks in event (for the loop)
  Int_t goodTrack = 0;
  Int_t nt = 0;
//(not used ?)  Int_t nt2 = 0;
  Int_t nmax = 0;
  Float_t pt,eta,phi;
  //  Int_t sflag = 0;
  TVector3 p3;

  if(fDebug>1) cout << "fESD in Fill array : " << fESD << endl;

  nt = fESD->GetNumberOfTracks();
  if(fDebug>1) cout << "Number of Tracks in ESD : " << nt << endl;
 
  // temporary storage of signal and pt cut flag
  Int_t* sflag  = new Int_t[nt];
  Int_t* cflag  = new Int_t[nt];

  // get cuts set by user
  Float_t ptMin  = fReaderHeader->GetPtCut();
  Float_t etaMin = fReaderHeader->GetFiducialEtaMin();
  Float_t etaMax = fReaderHeader->GetFiducialEtaMax();  
  fOpt = fReaderHeader->GetDetector();
  fDZ  = fReaderHeader->GetDZ();

  Int_t nTracksEmcal      = 0;
  Int_t nTracksEmcalDZ    = 0;
  Int_t nTracksTpc        = 0;
  Int_t nTracksTpcOnly    = 0;
  Int_t nTracksEmcalCut   = 0;
  Int_t nTracksEmcalDZCut = 0;
  Int_t nTracksTpcCut     = 0;
  Int_t nTracksTpcOnlyCut = 0;

//(not used ?)  Int_t nElements  = fTPCGrid->GetNEntries();
//(not used ?)  Int_t nElements2 = fEMCalGrid->GetNEntries();
  fGrid = fTPCGrid->GetGridType();

//(not used ?)  Int_t nEntries = fUnitArray->GetEntries();

//(not used ?)  Int_t nRefEnt = fRefArray->GetEntries();

  //loop over tracks
    nmax = nt;  
//     for (Int_t it = 0; it < nmax; it++) {
//       IlcESDtrack *track;
//       track = fESD->GetTrack(it);
//       UInt_t status = track->GetStatus();
//     
//       Double_t mom[3];
//       track->GetPxPyPz(mom);
// 
//       p3.SetXYZ(mom[0],mom[1],mom[2]);
//       pt = p3.Pt();
// 
//       Float_t mass = 0.;
//       mass = track->GetMass();
// 
//       if ((status & IlcESDtrack::kTPCrefit) == 0)    continue;      // quality check
//       if ((status & IlcESDtrack::kITSrefit) == 0)    continue;      // quality check
//       if (((IlcJetESDReaderHeader*) fReaderHeader)->ReadSignalOnly() 
// 	  && TMath::Abs(track->GetLabel()) > 10000)  continue;   // quality check
//       if (((IlcJetESDReaderHeader*) fReaderHeader)->ReadBkgdOnly() 
// 	  && TMath::Abs(track->GetLabel()) < 10000)  continue;   // quality check
//        eta = p3.Eta();
//        phi = ( (p3.Phi()) < 0) ? (p3.Phi()) + 2. * TMath::Pi() : (p3.Phi());
//    
//       if ( (eta > etaMax) || (eta < etaMin)) continue;           // checking eta cut
// 
//       sflag[goodTrack]=0;
//       if (TMath::Abs(track->GetLabel()) < 10000) sflag[goodTrack]=1;
//       cflag[goodTrack]=0;
//       if (pt > ptMin) cflag[goodTrack]=1;                       // pt cut
//       //      fRef->Add(track);
// 
//       if(fGrid==0)
// 	{
// 	  // Only TPC filled from its grid in its total acceptance
// 	  
// 	  Int_t idTPC = fTPCGrid->GetIndex(phi,eta);
// 	  Bool_t ok = kFALSE;
// 	  Bool_t ref = kFALSE;
// 
// 	  IlcJetUnitArray *uArray = (IlcJetUnitArray*)fUnitArray->At(idTPC-1);
// 	  TRefArray *reference = uArray->GetUnitTrackRef();
// 	  if (reference->GetEntries() == 0)  {
// 	      new(reference) TRefArray(TProcessID::GetProcessWithUID(track));
// 	  }
// 
// 	  reference->Add(track);
// 
// 	  Float_t unitEnergy = 0.;
// 	  unitEnergy = uArray->GetUnitEnergy();
// 	  // nTracksTpcOnly is necessary to count the number of candidate cells
// 	  // but it doesn't give the real multiplicity -> it will be extracted 
// 	  // in the jet finder through the reference to tracks
// 	  if(unitEnergy==0.){
// 	    nTracksTpcOnly++;
// 	    ok = kTRUE;
//             ref = kTRUE;
// 	  }
// 
// 	  // Fill energy in TPC acceptance
// 	  uArray->SetUnitEnergy(unitEnergy + pt);
// 	  uArray->SetUnitMass(mass);
// 
// 	  // Pt cut flag
// 	  if(uArray->GetUnitEnergy()<ptMin){
// 	    uArray->SetUnitCutFlag(kPtSmaller);
// 	  }
// 	  else {
// 	    uArray->SetUnitCutFlag(kPtHigher);
// 	    if(ok) nTracksTpcOnlyCut++;
// 	  }
// 
// 	  // Signal flag
// 	  if(sflag[goodTrack] == 1) { 
// 	    uArray->SetUnitSignalFlag(kGood);
//             uArray->SetUnitSignalFlagC(kFALSE,kGood);
//           } else uArray->SetUnitSignalFlagC(kFALSE,kBad);
// 
// 	  if(uArray->GetUnitEnergy()>0 && ref){
// 	    if(!fProcId) {
// 	      fProcId = kTRUE;
// 	      new(fRefArray) TRefArray(TProcessID::GetProcessWithUID(uArray));
// 	    }
// 	    fRefArray->Add(uArray);
// 	  }
// 	}
//     
//       if(fGrid==1)
// 	{
// 	  Int_t nElements = fTPCGrid->GetNEntries();
// 	  // Fill track information in EMCAL acceptance
// 	    if((eta >= fEtaMin && eta <= fEtaMax) &&
// 	     (phi >= fPhiMin && phi <= fPhiMax))// &&
// 	    {
// 
// 	      // Include dead-zones
// // 	      if(fDZ)
// // 		{
// // 		  Double_t phimin0 = 0., phimin1 = 0., phimin2 = 0., phimin3 = 0., phimin4 = 0.;
// // 		  Double_t phimax0 = 0., phimax1 = 0., phimax2 = 0., phimax3 = 0., phimax4 = 0.;
// // 		  fGeom->GetEMCGeometry()->GetPhiBoundariesOfSMGap(0,phimin0,phimax0);
// // 		  fGeom->GetEMCGeometry()->GetPhiBoundariesOfSMGap(1,phimin1,phimax1);
// // 		  fGeom->GetEMCGeometry()->GetPhiBoundariesOfSMGap(2,phimin2,phimax2);
// // 		  fGeom->GetEMCGeometry()->GetPhiBoundariesOfSMGap(3,phimin3,phimax3);
// // 		  fGeom->GetEMCGeometry()->GetPhiBoundariesOfSMGap(4,phimin4,phimax4);
// // 		  Int_t n0 = fGrid0->GetNEntries();
// // 		  Int_t n1 = fGrid1->GetNEntries();
// // 		  Int_t n2 = fGrid2->GetNEntries();
// // 		  Int_t n3 = fGrid3->GetNEntries();
// // //(not used ?)		  Int_t n4 = fGrid4->GetNEntries();
// // 		  
// // 		  if(phi >= phimin0 && phi <= phimax0){
// // 		    Int_t id0 = fGrid0->GetIndex(phi,eta)-1;
// // 		    IlcJetUnitArray *uArray0 = (IlcJetUnitArray*)fUnitArray->At(id0+fNumUnits+nElements);
// // 		    TRefArray *reference0 = uArray0->GetUnitTrackRef();
// // 		    if (reference0->GetEntries() == 0)  {
// // 			new(reference0) TRefArray(TProcessID::GetProcessWithUID(track));
// // 		    }
// // 
// // 		    reference0->Add(track);
// // 
// // 		    Float_t uEnergy0 = uArray0->GetUnitEnergy();
// // 		    Bool_t ok0 = kFALSE;
// // 		    Bool_t ref0 = kFALSE;
// // 		    if(uEnergy0==0.){
// // 		      nTracksEmcalDZ++;
// // 		      ok0 = kTRUE;
// // 		      ref0 = kTRUE;
// // 		    }
// // 		    uArray0->SetUnitEnergy(uEnergy0+pt);
// // 
// // 		    if(uArray0->GetUnitEnergy()<ptMin)
// // 		      uArray0->SetUnitCutFlag(kPtSmaller);
// // 		    else {
// // 		      uArray0->SetUnitCutFlag(kPtHigher);
// // 		      if(ok0) nTracksEmcalDZCut++;
// // 		    }
// // 		    if(sflag[goodTrack] == 1) {
// // 		      uArray0->SetUnitSignalFlag(kGood);
// //                       uArray0->SetUnitSignalFlagC(kFALSE,kGood);
// //                     } else uArray0->SetUnitSignalFlagC(kFALSE,kBad);
// // 
// // 		    if(uArray0->GetUnitEnergy()>0 && ref0){
// // 		      if(!fProcId){
// // 			new(fRefArray) TRefArray(TProcessID::GetProcessWithUID(uArray0));
// // 			fProcId = kTRUE;
// // 		      }
// // 		      fRefArray->Add(uArray0);
// // 		    }
// // 		  }
// // 
// // 		  if(phi >= phimin1 && phi <= phimax1){
// // 		    Int_t id1 = fGrid1->GetIndex(phi,eta)-1+n0;
// // 		    IlcJetUnitArray *uArray1 = (IlcJetUnitArray*)fUnitArray->At(id1+fNumUnits+nElements);
// // 		    TRefArray *reference1 = uArray1->GetUnitTrackRef();
// // 		    if (reference1->GetEntries() == 0)  {
// // 			new(reference1) TRefArray(TProcessID::GetProcessWithUID(track));
// // 		    }
// // 
// // 		    reference1->Add(track);
// // 
// // 		    Float_t uEnergy1 = uArray1->GetUnitEnergy();
// // 		    Bool_t ok1 = kFALSE;
// // 		    Bool_t ref1 = kFALSE;
// // 		    if(uEnergy1==0.){
// // 		      nTracksEmcalDZ++;
// // 		      ok1 = kTRUE;
// // 		      ref1 = kTRUE;
// // 		    }
// // 		    uArray1->SetUnitEnergy(uEnergy1+pt);
// // 
// // 		    if(uArray1->GetUnitEnergy()<ptMin)
// // 		      uArray1->SetUnitCutFlag(kPtSmaller);
// // 		    else {
// // 		      uArray1->SetUnitCutFlag(kPtHigher);
// // 		      if(ok1) nTracksEmcalDZCut++;
// // 		    }
// // 		    if(sflag[goodTrack] == 1) {
// // 		      uArray1->SetUnitSignalFlag(kGood);
// //                       uArray1->SetUnitSignalFlagC(kFALSE,kGood);
// //                     } else uArray1->SetUnitSignalFlagC(kFALSE,kBad);
// // 
// // 		    if(uArray1->GetUnitEnergy()>0 && ref1){
// // 		      if(!fProcId){
// // 			new(fRefArray) TRefArray(TProcessID::GetProcessWithUID(uArray1));
// // 			fProcId = kTRUE;
// // 		      }
// // 		      fRefArray->Add(uArray1);
// // 		    }
// // 		  }
// // 
// // 		  if(phi >= phimin2 && phi <= phimax2){
// // 		    Int_t id2 = fGrid2->GetIndex(phi,eta)-1+n0+n1;
// // 		    IlcJetUnitArray *uArray2 = (IlcJetUnitArray*)fUnitArray->At(id2+fNumUnits+nElements);
// // 		    TRefArray *reference2 = uArray2->GetUnitTrackRef();
// // 		    if (reference2->GetEntries() == 0)  {
// // 			new(reference2) TRefArray(TProcessID::GetProcessWithUID(track));
// // 		    }
// // 
// // 		    reference2->Add(track);
// // 
// // 		    Float_t uEnergy2 = uArray2->GetUnitEnergy();
// // 		    Bool_t ok2 = kFALSE;
// // 		    Bool_t ref2 = kFALSE;
// // 		    if(uEnergy2==0.){
// // 		      nTracksEmcalDZ++;
// // 		      ok2 = kTRUE;
// // 		      ref2 = kTRUE;
// // 		    }
// // 		    uArray2->SetUnitEnergy(uEnergy2+pt);
// // 
// // 	      	    if(uArray2->GetUnitEnergy()<ptMin)
// // 		      uArray2->SetUnitCutFlag(kPtSmaller);
// // 		    else {
// // 		      uArray2->SetUnitCutFlag(kPtHigher);
// // 		      if(ok2) nTracksEmcalDZCut++;
// // 		    }
// // 		    if(sflag[goodTrack] == 1) {
// // 		      uArray2->SetUnitSignalFlag(kGood);
// //                       uArray2->SetUnitSignalFlagC(kFALSE,kGood);
// //                     } else uArray2->SetUnitSignalFlagC(kFALSE,kBad);
// // 
// // 		    if(uArray2->GetUnitEnergy()>0 && ref2){
// // 		      if(!fProcId){
// // 			new(fRefArray) TRefArray(TProcessID::GetProcessWithUID(uArray2));
// // 			fProcId = kTRUE;
// // 		      }
// // 		      fRefArray->Add(uArray2);
// // 		    }
// // 		  }
// // 
// // 		  if(phi >= phimin3 && phi <= phimax3){
// // 		    Int_t id3 = fGrid3->GetIndex(phi,eta)-1+n0+n1+n2;
// // 		    IlcJetUnitArray *uArray3 = (IlcJetUnitArray*)fUnitArray->At(id3+fNumUnits+nElements);
// // 		    TRefArray *reference3 = uArray3->GetUnitTrackRef();
// // 		    if (reference3->GetEntries() == 0)  {
// // 			new(reference3) TRefArray(TProcessID::GetProcessWithUID(track));
// // 		    }
// // 
// // 		    reference3->Add(track);
// // 
// // 		    Float_t uEnergy3 = uArray3->GetUnitEnergy();
// // 		    Bool_t ok3 = kFALSE;
// // 		    Bool_t ref3 = kFALSE;
// // 		    if(uEnergy3==0.){
// // 		      nTracksEmcalDZ++;
// // 		      ok3 = kTRUE;
// // 		      ref3 = kTRUE;
// // 		    }
// // 		    uArray3->SetUnitEnergy(uEnergy3+pt);
// // 
// // 		    if(uArray3->GetUnitEnergy()<ptMin)
// // 		      uArray3->SetUnitCutFlag(kPtSmaller);
// // 		    else {
// // 		      uArray3->SetUnitCutFlag(kPtHigher);
// // 		      if(ok3) nTracksEmcalDZCut++;
// // 		    }
// // 		    if(sflag[goodTrack] == 1) {
// // 		      uArray3->SetUnitSignalFlag(kGood);
// //                       uArray3->SetUnitSignalFlagC(kFALSE,kGood);
// //                     } else uArray3->SetUnitSignalFlagC(kFALSE,kBad);
// // 
// // 		    if(uArray3->GetUnitEnergy()>0 && ref3){
// // 		      if(!fProcId){
// // 			new(fRefArray) TRefArray(TProcessID::GetProcessWithUID(uArray3));
// // 			fProcId = kTRUE;
// // 		      }
// // 		      fRefArray->Add(uArray3);
// // 		    }
// // 		  }
// // 
// // 		  if(phi >= phimin4 && phi <= phimax4){
// // 		    Int_t id4 = fGrid4->GetIndex(phi,eta)-1+n0+n1+n2+n3;
// // 		    IlcJetUnitArray *uArray4 = (IlcJetUnitArray*)fUnitArray->At(id4+fNumUnits+nElements);
// // 		    TRefArray *reference4 = uArray4->GetUnitTrackRef();
// // 		    if (reference4->GetEntries() == 0)  {
// // 			new(reference4) TRefArray(TProcessID::GetProcessWithUID(track));
// // 		    }
// // 
// // 		    reference4->Add(track);
// // 
// // 		    Float_t uEnergy4 = uArray4->GetUnitEnergy();
// // 		    Bool_t ok4 = kFALSE;
// // 		    Bool_t ref4 = kFALSE;
// // 		    if(uEnergy4==0.){
// // 		      nTracksEmcalDZ++;
// // 		      ok4 = kTRUE;
// // 		      ref4 = kTRUE;
// // 		    }
// // 		    uArray4->SetUnitEnergy(uEnergy4+pt);
// // 
// // 		    if(uArray4->GetUnitEnergy()<ptMin)
// // 		      uArray4->SetUnitCutFlag(kPtSmaller);
// // 		    else {
// // 		      uArray4->SetUnitCutFlag(kPtHigher);
// // 		      if(ok4) nTracksEmcalDZCut++;
// // 		    }
// // 		    if(sflag[goodTrack] == 1) {
// // 		      uArray4->SetUnitSignalFlag(kGood);
// //                       uArray4->SetUnitSignalFlagC(kFALSE,kGood);
// //                     } else uArray4->SetUnitSignalFlagC(kFALSE,kBad);
// // 
// // 		    if(uArray4->GetUnitEnergy()>0 && ref4){
// // 		      if(!fProcId){
// // 			new(fRefArray) TRefArray(TProcessID::GetProcessWithUID(uArray4));
// // 			fProcId = kTRUE;
// // 		      }
// // 		      fRefArray->Add(uArray4);
// // 		    }
// // 		  }
// // 		} // end fDZ
// 	    
// 	      Int_t towerID = 0;
// 	      fGeom->GetAbsCellIdFromEtaPhi(eta,phi,towerID);
// 	      if(towerID==-1) continue;
// 	      
// 	      IlcJetUnitArray *uArray = (IlcJetUnitArray*)fUnitArray->At(towerID);
// 	      TRefArray *reference = uArray->GetUnitTrackRef();
// 	      if (reference->GetEntries() == 0)  {
// 		  new(reference) TRefArray(TProcessID::GetProcessWithUID(track));
// 	      }
// 
// 	      reference->Add(track);
// 
// 	      Float_t unitEnergy = uArray->GetUnitEnergy(); 
// 	      Bool_t ok = kFALSE;
// 	      Bool_t ref = kFALSE;
// 	      if(unitEnergy==0.){
// 		nTracksEmcal++;
// 		ok=kTRUE;
// 		ref=kTRUE;
// 	      }
// 
// 	      // Do Hadron Correction
// 	      // This is under construction !!!!!!!!!!!!!!!!!!!!!!!
// 	      // For the moment I apply MIP correction if p >= 0.5 GeV/c
// 	      if (fApplyMIPCorrection != 0 && p3.Mag() >= 0.5) 
// 		{ 
// 		  ((IlcJetHadronCorrectionv1*)fHadCorr)->SetGeometry("EMCAL_COMPLETEV1",1.);
// 
// 		  // Get track position at the outer part of the reconstruction ~ TRD
// 		  Double_t phiOut = track->GetOuterParam()->Phi();
// 		  Double_t etaOut = track->GetOuterParam()->Eta();
// 
// 		  // If the track in the outer part of the TPC/TDR ? is inside 
// 		  // the calorimeter, it can deposit part of its energy
// 		  // We can then correct on average for these particles
// 		  if((etaOut >= fEtaMin && etaOut <= fEtaMax) &&
// 		     (phiOut >= fPhiMin && phiOut <= fPhiMax))// &&
// 		    {
// 		      Double_t   hCEnergy = (Double_t)fHadCorr->GetEnergy(p3.Mag(), (Double_t)eta,0);
// 		      unitEnergy -= hCEnergy*TMath::Sin(2.0*TMath::ATan(TMath::Exp(-eta)));
// 		    }
// 		} //end Hadron Correction loop
// 
// 	      cout << "unitEnergy + pt = " << unitEnergy << " + " << pt << " = " << unitEnergy + pt << endl;
// 
// 	      if((unitEnergy + pt) > 0.) uArray->SetUnitEnergy(unitEnergy + pt);
// 	      else uArray->SetUnitEnergy(0.);
// 
// 	      // Put a pt cut flag
// 	      if(uArray->GetUnitEnergy()<ptMin){
// 		uArray->SetUnitCutFlag(kPtSmaller);
// 	      }
// 	      else {
// 		uArray->SetUnitCutFlag(kPtHigher);
// 		if(ok) nTracksEmcalCut++;
// 	      }
// 
// 	      // Signal flag
// 	      if(sflag[goodTrack] == 1) { 
// 		uArray->SetUnitSignalFlag(kGood);
//                 uArray->SetUnitSignalFlagC(kFALSE,kGood);
//               } else uArray->SetUnitSignalFlagC(kFALSE,kBad);
// 
// 	    
// 	      if(uArray->GetUnitEnergy()>0 && ref){
// 		if(!fProcId){
// 		  new(fRefArray) TRefArray(TProcessID::GetProcessWithUID(uArray));
// 		  fProcId = kTRUE;
// 		}
// 		fRefArray->Add(uArray);
// 	      }
// 	    } // end loop on EMCal acceptance cut + tracks quality
// 	  else{ 
// 	    // Outside EMCal acceptance
// 	  
// 	    Int_t idTPC = fTPCGrid->GetIndex(phi,eta);
// 
// 	    IlcJetUnitArray *uArray = (IlcJetUnitArray*)fUnitArray->At(fNumUnits-1+idTPC);
// 	    TRefArray *reference = uArray->GetUnitTrackRef();
// 	    if (reference->GetEntries() == 0)  {
// 		new(reference) TRefArray(TProcessID::GetProcessWithUID(track));
// 	    }
// 
// 	    reference->Add(track);
// 
// 	    Float_t unitEnergy2 = uArray->GetUnitEnergy(); 
// 	    Bool_t okout = kFALSE;
// 	    Bool_t refout = kFALSE;
// 	    if(unitEnergy2==0.){
// 	      nTracksTpc++;
// 	      okout=kTRUE;
// 	      refout=kTRUE;
// 	    }
// 	    // Fill energy outside emcal acceptance
// 	    uArray->SetUnitEnergy(unitEnergy2 + pt);
// 	  
// 	    // Pt cut flag
// 	    if(uArray->GetUnitEnergy()<ptMin){
// 	      uArray->SetUnitCutFlag(kPtSmaller);
// 	    }
// 	    else {
// 	      uArray->SetUnitCutFlag(kPtHigher);
// 	      if(okout) nTracksTpcCut++;
// 	    }
// 	    // Signal flag
// 	    if(sflag[goodTrack] == 1) {
// 	      uArray->SetUnitSignalFlag(kGood);
//               uArray->SetUnitSignalFlagC(kFALSE,kGood);
//             } else uArray->SetUnitSignalFlagC(kFALSE,kBad);
// 
// 
// 	    if(uArray->GetUnitEnergy()>0 && refout){
// 	      if(!fProcId){
// 		new(fRefArray) TRefArray(TProcessID::GetProcessWithUID(uArray));
// 		fProcId = kTRUE;
// 	      }
// 	      fRefArray->Add(uArray);
// 	    }
// 	  }
// 	} // end fGrid==1
// 
//       goodTrack++;
// 
//     } // end loop on entries (tpc tracks)

  if(fDebug>0)
    {
      cout << "End of Tracks %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
      cout << "goodTracks: " << goodTrack << endl;
    }

  //  fSignalFlag.Set(goodTrack,sflag);
  //  fCutFlag.Set(goodTrack,cflag);

  delete[] sflag;
  delete[] cflag;
      
  if(fGrid==0) {
    fNTracks = nTracksTpcOnly;
    fNTracksCut = nTracksTpcOnlyCut;
    if(fDebug>10){
      cout << "fNTracks : " << fNTracks << endl;
      cout << "fNTracksCut : " << fNTracksCut << endl;
    }
  }
  if(fGrid==1) {
    fNTracks = nTracksEmcal+nTracksEmcalDZ+nTracksTpc;
    fNTracksCut = nTracksEmcalCut+nTracksEmcalDZCut+nTracksTpcCut;
    if(fDebug>10){
      cout << "fNTracks : " << fNTracks << endl;
      cout << "fNTracksCut : " << fNTracksCut << endl;
    }
  }  
  
}














