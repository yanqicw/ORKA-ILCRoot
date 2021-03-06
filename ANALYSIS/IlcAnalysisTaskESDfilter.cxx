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

/* $Id$ */
 
#include <TChain.h>
#include <TTree.h>
#include <TList.h>
#include <TArrayI.h>
#include <TParameter.h>
#include <TRandom.h>
#include <TParticle.h>
#include <TFile.h>

#include "IlcAnalysisTaskESDfilter.h"
#include "IlcAnalysisManager.h"
#include "IlcESDEvent.h"
#include "IlcESDRun.h"
#include "IlcStack.h"
#include "IlcAODEvent.h"
#include "IlcMCEvent.h"
#include "IlcMCEventHandler.h"
#include "IlcESDInputHandler.h"
#include "IlcAODHandler.h"
#include "IlcAODMCParticle.h"
#include "IlcAnalysisFilter.h"
#include "IlcESDMuonTrack.h"
#include "IlcESDVertex.h"
#include "IlcCentrality.h"
#include "IlcEventplane.h"
#include "IlcESDv0.h"
#include "IlcESDkink.h"
#include "IlcESDcascade.h"
#include "IlcESDPmdTrack.h"
#include "IlcESDCaloCluster.h"
#include "IlcESDCaloCells.h"
#include "IlcMultiplicity.h"
#include "IlcLog.h"
#include "IlcCodeTimer.h"
#include "IlcESDtrackCuts.h"
#include "IlcESDpid.h"
#include "IlcAODHMPIDrings.h"
#include "IlcV0vertexer.h"
#include "IlcCascadeVertexer.h"
#include "Riostream.h"
#include "IlcExternalTrackParam.h"
#include "IlcTrackerBase.h"
#include "TVector3.h"
#include "IlcTPCdEdxInfo.h"

using std::cout;
using std::endl;
ClassImp(IlcAnalysisTaskESDfilter)

////////////////////////////////////////////////////////////////////////

IlcAnalysisTaskESDfilter::IlcAnalysisTaskESDfilter():
  IlcAnalysisTaskSE(),
  fTrackFilter(0x0),
  fKinkFilter(0x0),
  fV0Filter(0x0),
  fCascadeFilter(0x0),
  fHighPthreshold(0),
  fPtshape(0x0),
  fEnableFillAOD(kTRUE),
  fUsedTrack(0x0),
  fUsedKink(0x0),
  fUsedV0(0x0),
  fAODTrackRefs(0x0),
  fAODV0VtxRefs(0x0),
  fAODV0Refs(0x0),
  fMChandler(0x0),
  fNumberOfTracks(0),
  fNumberOfPositiveTracks(0),
  fNumberOfV0s(0),
  fNumberOfVertices(0),
  fNumberOfCascades(0),
  fNumberOfKinks(0),
  fOldESDformat(kFALSE),
  fPrimaryVertex(0x0),
  fTPCConstrainedFilterMask(0),
  fHybridFilterMaskTPCCG(0),
  fWriteHybridTPCCOnly(kFALSE),
  fGlobalConstrainedFilterMask(0),
  fHybridFilterMaskGCG(0),
  fWriteHybridGCOnly(kFALSE),
  fIsVZEROEnabled(kTRUE),
  fIsTZEROEnabled(kTRUE),
  fIsZDCEnabled(kTRUE),
  fIsHMPIDEnabled(kTRUE), 
  fIsV0CascadeRecoEnabled(kFALSE),
  fAreCascadesEnabled(kTRUE),
  fAreV0sEnabled(kTRUE),
  fAreKinksEnabled(kTRUE),
  fAreTracksEnabled(kTRUE),
  fArePmdClustersEnabled(kTRUE),
  fAreCaloClustersEnabled(kTRUE),
  fAreEMCALCellsEnabled(kTRUE),
  fArePHOSCellsEnabled(kTRUE),
  fAreEMCALTriggerEnabled(kTRUE),
  fArePHOSTriggerEnabled(kTRUE),
  fAreTrackletsEnabled(kTRUE),
  fESDpid(0x0),
  fIsPidOwner(kFALSE),
  fTimeZeroType(IlcESDpid::kTOF_T0),
  fTPCaloneTrackCuts(0),
  fDoPropagateTrackToEMCal(kTRUE)
{
  // Default constructor
    fV0Cuts[0] =  33.   ;   // max allowed chi2
    fV0Cuts[1] =   0.1  ;   // min allowed impact parameter for the 1st daughter
    fV0Cuts[2] =   0.1  ;   // min allowed impact parameter for the 2nd daughter
    fV0Cuts[3] =   1.   ;   // max allowed DCA between the daughter tracks
    fV0Cuts[4] =    .998;   // min allowed cosine of V0's pointing angle
    fV0Cuts[5] =   0.9  ;   // min radius of the fiducial volume
    fV0Cuts[6] = 100.   ;   // max radius of the fiducial volume

    fCascadeCuts[0] =  33.   ; // max allowed chi2 (same as PDC07)
    fCascadeCuts[1] =   0.05 ; // min allowed V0 impact parameter
    fCascadeCuts[2] =   0.008; // "window" around the Lambda mass
    fCascadeCuts[3] =   0.03 ; // min allowed bachelor's impact parameter
    fCascadeCuts[4] =   0.3  ; // max allowed DCA between the V0 and the bachelor
    fCascadeCuts[5] =   0.999; // min allowed cosine of the cascade pointing angle
    fCascadeCuts[6] =   0.9  ; // min radius of the fiducial volume
    fCascadeCuts[7] = 100.   ; // max radius of the fiducial volume
}

//______________________________________________________________________________
IlcAnalysisTaskESDfilter::IlcAnalysisTaskESDfilter(const char* name):
    IlcAnalysisTaskSE(name),
    fTrackFilter(0x0),
    fKinkFilter(0x0),
    fV0Filter(0x0),
    fCascadeFilter(0x0),
    fHighPthreshold(0),
    fPtshape(0x0),
    fEnableFillAOD(kTRUE),
    fUsedTrack(0x0),
    fUsedKink(0x0),
    fUsedV0(0x0),
    fAODTrackRefs(0x0),
    fAODV0VtxRefs(0x0),
    fAODV0Refs(0x0),
    fMChandler(0x0),
    fNumberOfTracks(0),
    fNumberOfPositiveTracks(0),
    fNumberOfV0s(0),
    fNumberOfVertices(0),
    fNumberOfCascades(0),
    fNumberOfKinks(0),
    fOldESDformat(kFALSE),
    fPrimaryVertex(0x0),
  fTPCConstrainedFilterMask(0),
  fHybridFilterMaskTPCCG(0),
  fWriteHybridTPCCOnly(kFALSE),
  fGlobalConstrainedFilterMask(0),
  fHybridFilterMaskGCG(0),
  fWriteHybridGCOnly(kFALSE),
    fIsVZEROEnabled(kTRUE),
    fIsTZEROEnabled(kTRUE),
    fIsZDCEnabled(kTRUE),
    fIsHMPIDEnabled(kTRUE), 
    fIsV0CascadeRecoEnabled(kFALSE),
    fAreCascadesEnabled(kTRUE),
    fAreV0sEnabled(kTRUE),
    fAreKinksEnabled(kTRUE),
    fAreTracksEnabled(kTRUE),
    fArePmdClustersEnabled(kTRUE),
    fAreCaloClustersEnabled(kTRUE),
    fAreEMCALCellsEnabled(kTRUE),
    fArePHOSCellsEnabled(kTRUE),
		fAreEMCALTriggerEnabled(kTRUE),
		fArePHOSTriggerEnabled(kTRUE),
		fAreTrackletsEnabled(kTRUE),
    fESDpid(0x0),
    fIsPidOwner(kFALSE),
    fTimeZeroType(IlcESDpid::kTOF_T0),
    fTPCaloneTrackCuts(0),
  fDoPropagateTrackToEMCal(kTRUE)
{
  // Constructor

    fV0Cuts[0] =  33.   ;   // max allowed chi2
    fV0Cuts[1] =   0.1  ;   // min allowed impact parameter for the 1st daughter
    fV0Cuts[2] =   0.1  ;   // min allowed impact parameter for the 2nd daughter
    fV0Cuts[3] =   1.   ;   // max allowed DCA between the daughter tracks
    fV0Cuts[4] =    .998;   // min allowed cosine of V0's pointing angle
    fV0Cuts[5] =   0.9  ;   // min radius of the fiducial volume
    fV0Cuts[6] = 100.   ;   // max radius of the fiducial volume

    fCascadeCuts[0] =  33.   ; // max allowed chi2 (same as PDC07)
    fCascadeCuts[1] =   0.05 ; // min allowed V0 impact parameter
    fCascadeCuts[2] =   0.008; // "window" around the Lambda mass
    fCascadeCuts[3] =   0.03 ; // min allowed bachelor's impact parameter
    fCascadeCuts[4] =   0.3  ; // max allowed DCA between the V0 and the bachelor
    fCascadeCuts[5] =   0.999; // min allowed cosine of the cascade pointing angle
    fCascadeCuts[6] =   0.9  ; // min radius of the fiducial volume
    fCascadeCuts[7] = 100.   ; // max radius of the fiducial volume



}
IlcAnalysisTaskESDfilter::~IlcAnalysisTaskESDfilter(){
    if(fIsPidOwner) delete fESDpid;
}
//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::UserCreateOutputObjects()
{
  //
  // Create Output Objects conenct filter to outputtree
  // 
  if(OutputTree())
  {
    OutputTree()->GetUserInfo()->Add(fTrackFilter);
  }
  else
  {
    IlcError("No OutputTree() for adding the track filter");
  }
  fTPCaloneTrackCuts = IlcESDtrackCuts::GetStandardTPCOnlyTrackCuts();
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::Init()
{
  // Initialization
  if (fDebug > 1) IlcInfo("Init() \n");
  // Call configuration file
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::PrintTask(Option_t *option, Int_t indent) const
{
// Print selection task information
  IlcInfo("");
  
  IlcAnalysisTaskSE::PrintTask(option,indent);
  
  TString spaces(' ',indent+3);
  
	cout << spaces.Data() << Form("Cascades       are %s",fAreCascadesEnabled ? "ENABLED":"DISABLED") << endl;
	cout << spaces.Data() << Form("V0s            are %s",fAreV0sEnabled ? "ENABLED":"DISABLED") << endl;
	cout << spaces.Data() << Form("Kinks          are %s",fAreKinksEnabled ? "ENABLED":"DISABLED") << endl;
	cout << spaces.Data() << Form("Tracks         are %s",fAreTracksEnabled ? "ENABLED":"DISABLED") << endl;
	cout << spaces.Data() << Form("PmdClusters    are %s",fArePmdClustersEnabled ? "ENABLED":"DISABLED") << endl;
	cout << spaces.Data() << Form("CaloClusters   are %s",fAreCaloClustersEnabled ? "ENABLED":"DISABLED") << endl;
  cout << spaces.Data() << Form("EMCAL cells    are %s",fAreEMCALCellsEnabled ? "ENABLED":"DISABLED") << endl;
	cout << spaces.Data() << Form("EMCAL triggers are %s",fAreEMCALTriggerEnabled ? "ENABLED":"DISABLED") << endl;
	cout << spaces.Data() << Form("PHOS triggers  are %s",fArePHOSTriggerEnabled ? "ENABLED":"DISABLED") << endl;
	cout << spaces.Data() << Form("Tracklets      are %s",fAreTrackletsEnabled ? "ENABLED":"DISABLED") << endl;  
	cout << spaces.Data() << Form("PropagateTrackToEMCal  is %s", fDoPropagateTrackToEMCal ? "ENABLED":"DISABLED") << endl; 
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::UserExec(Option_t */*option*/)
{
// Execute analysis for current event
//
					    
  Long64_t ientry = Entry();
  
  if (fDebug > 0) {
      printf("Filter: Analysing event # %5d\n", (Int_t) ientry);
      if (fHighPthreshold == 0) IlcInfo("detector PID signals are stored in each track");
      if (!fPtshape) IlcInfo("detector PID signals are not stored below the pt threshold");
  }
  // Filters must explicitely enable AOD filling in their UserExec (AG)
  if (!IlcAnalysisManager::GetAnalysisManager()->GetOutputEventHandler()) IlcFatal("Cannot run ESD filter without an output event handler");
  if(fEnableFillAOD) {
     IlcAnalysisManager::GetAnalysisManager()->GetOutputEventHandler()->SetFillAOD(kTRUE);
     IlcAnalysisManager::GetAnalysisManager()->GetOutputEventHandler()->SetFillExtension(kTRUE);
  }   
  ConvertESDtoAOD();
}

//______________________________________________________________________________
TClonesArray& IlcAnalysisTaskESDfilter::Cascades()
{
  return *(AODEvent()->GetCascades());
}

//______________________________________________________________________________
TClonesArray& IlcAnalysisTaskESDfilter::Tracks()
{
  return *(AODEvent()->GetTracks());
}

//______________________________________________________________________________
TClonesArray& IlcAnalysisTaskESDfilter::V0s()
{
  return *(AODEvent()->GetV0s());
}

//______________________________________________________________________________
TClonesArray& IlcAnalysisTaskESDfilter::Vertices()
{
  return *(AODEvent()->GetVertices());
}

//______________________________________________________________________________
IlcAODHeader* IlcAnalysisTaskESDfilter::ConvertHeader(const IlcESDEvent& esd)
{
// Convert header information

  IlcCodeTimerAuto("",0);
  
  IlcAODHeader* header = AODEvent()->GetHeader();
  
  header->SetRunNumber(esd.GetRunNumber());
  header->SetOfflineTrigger(fInputHandler->IsEventSelected()); // propagate the decision of the physics selection

  TTree* tree = fInputHandler->GetTree();
  if (tree) {
    TFile* file = tree->GetCurrentFile();
    if (file) header->SetESDFileName(file->GetName());
  }
  
  if (fOldESDformat) {
    header->SetBunchCrossNumber(0);
    header->SetOrbitNumber(0);
    header->SetPeriodNumber(0);
    header->SetEventType(0);
    header->SetMuonMagFieldScale(-999.);
    header->SetCentrality(0);       
    header->SetEventplane(0);
  } else {
    header->SetBunchCrossNumber(esd.GetBunchCrossNumber());
    header->SetOrbitNumber(esd.GetOrbitNumber());
    header->SetPeriodNumber(esd.GetPeriodNumber());
    header->SetEventType(esd.GetEventType());
    
    header->SetEventNumberESDFile(esd.GetHeader()->GetEventNumberInFile());
    if(const_cast<IlcESDEvent&>(esd).GetCentrality()){
      header->SetCentrality(const_cast<IlcESDEvent&>(esd).GetCentrality());
    }
    else{
      header->SetCentrality(0);
    }
    if(const_cast<IlcESDEvent&>(esd).GetEventplane()){
      header->SetEventplane(const_cast<IlcESDEvent&>(esd).GetEventplane());
    }
    else{
      header->SetEventplane(0);
    }
  }
  
  // Trigger
  header->SetFiredTriggerClasses(esd.GetFiredTriggerClasses());
  header->SetTriggerMask(esd.GetTriggerMask()); 
  header->SetTriggerCluster(esd.GetTriggerCluster());
  header->SetL0TriggerInputs(esd.GetHeader()->GetL0TriggerInputs());    
  header->SetL1TriggerInputs(esd.GetHeader()->GetL1TriggerInputs());    
  header->SetL2TriggerInputs(esd.GetHeader()->GetL2TriggerInputs());    
  
  header->SetMagneticField(esd.GetMagneticField());
  header->SetMuonMagFieldScale(esd.GetCurrentDip()/6000.);
  header->SetZDCN1Energy(esd.GetZDCN1Energy());
  header->SetZDCP1Energy(esd.GetZDCP1Energy());
  header->SetZDCN2Energy(esd.GetZDCN2Energy());
  header->SetZDCP2Energy(esd.GetZDCP2Energy());
  header->SetZDCEMEnergy(esd.GetZDCEMEnergy(0),esd.GetZDCEMEnergy(1));
  
  // ITS Cluster Multiplicty
  const IlcMultiplicity *mult = esd.GetMultiplicity();
  for (Int_t ilay = 0; ilay < 6; ilay++) header->SetITSClusters(ilay, mult->GetNumberOfITSClusters(ilay));
  
  // TPC only Reference Multiplicty
  Int_t refMult  = fTPCaloneTrackCuts ? (Short_t)fTPCaloneTrackCuts->GetReferenceMultiplicity(&esd, kTRUE) : -1;
  header->SetTPConlyRefMultiplicity(refMult);
  
  //
  Float_t diamxy[2]={esd.GetDiamondX(),esd.GetDiamondY()};
  Float_t diamcov[3]; 
  esd.GetDiamondCovXY(diamcov);
  header->SetDiamond(diamxy,diamcov);
  header->SetDiamondZ(esd.GetDiamondZ(),esd.GetSigma2DiamondZ());
  
  // VZERO channel equalization factors for event-plane reconstruction 	 
  header->SetVZEROEqFactors(esd.GetVZEROEqFactors());

  // T0 Resolution information                                                                                                                                          
  const IlcESDRun* esdRun = esd.GetESDRun();
  for (Int_t i=0;i<IlcESDRun::kT0spreadSize;i++) header->SetT0spread(i,esdRun->GetT0spread(i));

  return header;
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertCascades(const IlcESDEvent& esd) 
{

  // Convert the cascades part of the ESD.
  // Return the number of cascades
 
  IlcCodeTimerAuto("",0);
  
  // Create vertices starting from the most complex objects
  Double_t chi2 = 0.;
  
  const IlcESDVertex* vtx = esd.GetPrimaryVertex();
  Double_t pos[3] = { 0. };
  Double_t covVtx[6] = { 0. };
  Double_t momBach[3]={0.};
  Double_t covTr[21]={0.};
  Double_t pid[10]={0.};
  IlcAODPid* detpid(0x0);
  IlcAODVertex* vV0FromCascade(0x0);
  IlcAODv0* aodV0(0x0);
  IlcAODcascade* aodCascade(0x0);
  IlcAODTrack* aodTrack(0x0);
  Double_t momPos[3]={0.};
  Double_t momNeg[3] = { 0. };
  Double_t momPosAtV0vtx[3]={0.};
  Double_t momNegAtV0vtx[3]={0.};

  TClonesArray& verticesArray = Vertices();
  TClonesArray& tracksArray = Tracks();
  TClonesArray& cascadesArray = Cascades();
  
  // Cascades (Modified by A.Maire - February 2009)
  for (Int_t nCascade = 0; nCascade < esd.GetNumberOfCascades(); ++nCascade) {
    
    // 0- Preparation
    //
    IlcESDcascade *esdCascade = esd.GetCascade(nCascade);
		Int_t  idxPosFromV0Dghter  = esdCascade->GetPindex();
		Int_t  idxNegFromV0Dghter  = esdCascade->GetNindex();
		Int_t  idxBachFromCascade  = esdCascade->GetBindex();
    
    IlcESDtrack  *esdCascadePos  = esd.GetTrack( idxPosFromV0Dghter);
    IlcESDtrack  *esdCascadeNeg  = esd.GetTrack( idxNegFromV0Dghter);
    IlcESDtrack  *esdCascadeBach = esd.GetTrack( idxBachFromCascade);
    
    // Identification of the V0 within the esdCascade (via both daughter track indices)
    IlcESDv0 * currentV0   = 0x0;
    Int_t      idxV0FromCascade = -1;
    
    for (Int_t iV0=0; iV0<esd.GetNumberOfV0s(); ++iV0) {
      
      currentV0 = esd.GetV0(iV0);
      Int_t posCurrentV0 = currentV0->GetPindex();
      Int_t negCurrentV0 = currentV0->GetNindex();
      
      if (posCurrentV0==idxPosFromV0Dghter && negCurrentV0==idxNegFromV0Dghter) {
        idxV0FromCascade = iV0;
        break;
      }
    }
    
    if(idxV0FromCascade < 0){
      printf("Cascade - no matching for the V0 (index V0 = -1) ! Skip ... \n");
      continue;
    }// a priori, useless check, but safer ... in case of pb with tracks "out of bounds"
    
    IlcESDv0 *esdV0FromCascade   = esd.GetV0(idxV0FromCascade);
        
    // 1 - Cascade selection 
    
    //	IlcESDVertex *esdPrimVtx = new IlcESDVertex(*(esd.GetPrimaryVertex()));
    // 	TList cascadeObjects;
    // 	cascadeObjects.AddAt(esdV0FromCascade, 0);
    // 	cascadeObjects.AddAt(esdCascadePos,    1);
    // 	cascadeObjects.AddAt(esdCascadeNeg,    2);
    // 	cascadeObjects.AddAt(esdCascade,       3);
    // 	cascadeObjects.AddAt(esdCascadeBach,   4);
    // 	cascadeObjects.AddAt(esdPrimVtx,       5);
    // 
    // 	UInt_t selectCascade = 0;
    // 	if (fCascadeFilter) {
    // 	  // selectCascade = fCascadeFilter->IsSelected(&cascadeObjects); 
    // 	  	// FIXME IlcESDCascadeCuts to be implemented ...
    // 
    // 		// Here we may encounter a moot point at the V0 level 
    // 		// between the cascade selections and the V0 ones :
    // 		// the V0 selected along with the cascade (secondary V0) may 
    // 		// usually be removed from the dedicated V0 selections (prim V0) ...
    // 		// -> To be discussed !
    // 
    // 	  // this is a little awkward but otherwise the 
    // 	  // list wants to access the pointer (delete it) 
    // 	  // again when going out of scope
    // 	  delete cascadeObjects.RemoveAt(5); // esdPrimVtx created via copy construct
    // 	  esdPrimVtx = 0;
    // 	  if (!selectCascade) 
    // 	    continue;
    // 	}
    // 	else{
    // 	  delete cascadeObjects.RemoveAt(5); // esdPrimVtx created via copy construct
    // 	  esdPrimVtx = 0;
    // 	}
    
    // 2 - Add the cascade vertex
    
    esdCascade->GetXYZcascade(pos[0], pos[1], pos[2]);
    esdCascade->GetPosCovXi(covVtx);
    chi2 = esdCascade->GetChi2Xi(); 
    
    IlcAODVertex *vCascade = new(verticesArray[fNumberOfVertices++]) IlcAODVertex( pos,
                                                                     covVtx,
                                                                     chi2, // FIXME = Chi2/NDF will be needed
                                                                     fPrimaryVertex,
                                                                     nCascade, // id
                                                                     IlcAODVertex::kCascade);
    fPrimaryVertex->AddDaughter(vCascade);
    
//    if (fDebug > 2) {
//      printf("---- Cascade / Cascade Vertex (AOD) : \n");
//      vCascade->Print();
//    }
    
//    if(esd.GetTOFHeader() && fIsPidOwner) fESDpid->SetTOFResponse(const_cast<IlcESDEvent*>(&esd), (IlcESDpid::EStartTimeType_t)fTimeZeroType); //in case of AOD production starting form LHC10e without Tender. 


    // 3 - Add the bachelor track from the cascade
    
    if (!fUsedTrack[idxBachFromCascade]) {
      
      esdCascadeBach->GetPxPyPz(momBach);
      esdCascadeBach->GetXYZ(pos);
      esdCascadeBach->GetCovarianceXYZPxPyPz(covTr);
      esdCascadeBach->GetESDpid(pid);
      
	    fUsedTrack[idxBachFromCascade] = kTRUE;
	    UInt_t selectInfo = 0;
	    if (fTrackFilter) selectInfo = fTrackFilter->IsSelected(esdCascadeBach);
	    if (fMChandler) fMChandler->SelectParticle(esdCascadeBach->GetLabel());
	    aodTrack = new(tracksArray[fNumberOfTracks++]) IlcAODTrack(esdCascadeBach->GetID(),
                                                                           esdCascadeBach->GetLabel(), 
                                                                           momBach, 
                                                                           kTRUE,
                                                                           pos,
                                                                           kFALSE, // Why kFALSE for "isDCA" ? FIXME
                                                                           covTr, 
                                                                           (Short_t)esdCascadeBach->GetSign(),
                                                                           esdCascadeBach->GetITSClusterMap(), 
                                                                           pid,
                                                                           vCascade,
                                                                           kTRUE,  // usedForVtxFit = kFALSE ? FIXME
                                                                           vtx->UsesTrack(esdCascadeBach->GetID()),
                                                                           IlcAODTrack::kSecondary,
                                                                           selectInfo);
	    aodTrack->SetTPCFitMap(esdCascadeBach->GetTPCFitMap());
	    aodTrack->SetTPCClusterMap(esdCascadeBach->GetTPCClusterMap());
	    aodTrack->SetTPCSharedMap (esdCascadeBach->GetTPCSharedMap());
	    aodTrack->SetChi2perNDF(Chi2perNDF(esdCascadeBach));
	    aodTrack->SetTPCPointsF(esdCascadeBach->GetTPCNclsF());
	    aodTrack->SetTPCNCrossedRows(UShort_t(esdCascadeBach->GetTPCCrossedRows()));
	    fAODTrackRefs->AddAt(aodTrack,idxBachFromCascade);
	    
	    if (esdCascadeBach->GetSign() > 0) ++fNumberOfPositiveTracks;
	    aodTrack->ConvertIlcPIDtoAODPID();
	    aodTrack->SetFlags(esdCascadeBach->GetStatus());
      SetAODPID(esdCascadeBach,aodTrack,detpid);
    }
    else {
	    aodTrack = static_cast<IlcAODTrack*>( fAODTrackRefs->At(idxBachFromCascade) );
    }
    
    vCascade->AddDaughter(aodTrack);
    
//    if (fDebug > 4) {
//      printf("---- Cascade / bach dghter : \n");
//      aodTrack->Print();
//    }
    
    
    // 4 - Add the V0 from the cascade. 
    // = V0vtx + both pos and neg daughter tracks + the aodV0 itself
    //
    
    if ( !fUsedV0[idxV0FromCascade] ) {
      // 4.A - if VO structure hasn't been created yet
      
      // 4.A.1 - Create the V0 vertex of the cascade
      
      esdV0FromCascade->GetXYZ(pos[0], pos[1], pos[2]);
      esdV0FromCascade->GetPosCov(covVtx);
      chi2 = esdV0FromCascade->GetChi2V0();  // = chi2/NDF since NDF = 2*2-3 ?
			
      vV0FromCascade = new(verticesArray[fNumberOfVertices++]) IlcAODVertex(pos,
                                                                         covVtx,
                                                                         chi2,
                                                                         vCascade,
                                                                         idxV0FromCascade, //id of ESDv0
                                                                         IlcAODVertex::kV0);
      // Note:
      //    one V0 can be used by several cascades.
      // So, one AOD V0 vtx can have several parent vtx.
      // This is not directly allowed by IlcAODvertex.
      // Setting the parent vtx (here = param "vCascade") doesn't lead to a crash
      // but to a problem of consistency within AODEvent.
      // -> See below paragraph 4.B, for the proposed treatment of such a case.
      
      // Add the vV0FromCascade to the aodVOVtxRefs
      fAODV0VtxRefs->AddAt(vV0FromCascade,idxV0FromCascade);
      
      
      // 4.A.2 - Add the positive tracks from the V0
      
      esdCascadePos->GetPxPyPz(momPos);
      esdCascadePos->GetXYZ(pos);
      esdCascadePos->GetCovarianceXYZPxPyPz(covTr);
      esdCascadePos->GetESDpid(pid);
      
      
      if (!fUsedTrack[idxPosFromV0Dghter]) {
        fUsedTrack[idxPosFromV0Dghter] = kTRUE;
        
        UInt_t selectInfo = 0;
        if (fTrackFilter) selectInfo = fTrackFilter->IsSelected(esdCascadePos);
        if(fMChandler) fMChandler->SelectParticle(esdCascadePos->GetLabel());
        aodTrack = new(tracksArray[fNumberOfTracks++]) 
        IlcAODTrack(  esdCascadePos->GetID(),
                    esdCascadePos->GetLabel(), 
                    momPos, 
                    kTRUE,
                    pos,
                    kFALSE, // Why kFALSE for "isDCA" ? FIXME
                    covTr, 
                    (Short_t)esdCascadePos->GetSign(),
                    esdCascadePos->GetITSClusterMap(), 
                    pid,
                    vV0FromCascade,
                    kTRUE,  // usedForVtxFit = kFALSE ? FIXME
                    vtx->UsesTrack(esdCascadePos->GetID()),
                    IlcAODTrack::kSecondary,
                    selectInfo);
        aodTrack->SetTPCFitMap(esdCascadePos->GetTPCFitMap());
        aodTrack->SetTPCClusterMap(esdCascadePos->GetTPCClusterMap());
        aodTrack->SetTPCSharedMap (esdCascadePos->GetTPCSharedMap());
        aodTrack->SetChi2perNDF(Chi2perNDF(esdCascadePos));
	aodTrack->SetTPCPointsF(esdCascadePos->GetTPCNclsF());
	aodTrack->SetTPCNCrossedRows(UShort_t(esdCascadePos->GetTPCCrossedRows()));
        fAODTrackRefs->AddAt(aodTrack,idxPosFromV0Dghter);
        
        if (esdCascadePos->GetSign() > 0) ++fNumberOfPositiveTracks;
        aodTrack->ConvertIlcPIDtoAODPID();
        aodTrack->SetFlags(esdCascadePos->GetStatus());
        SetAODPID(esdCascadePos,aodTrack,detpid);
      }
      else {
        aodTrack = static_cast<IlcAODTrack*>(fAODTrackRefs->At(idxPosFromV0Dghter));
      }
      vV0FromCascade->AddDaughter(aodTrack);
      
      
      // 4.A.3 - Add the negative tracks from the V0
      
      esdCascadeNeg->GetPxPyPz(momNeg);
      esdCascadeNeg->GetXYZ(pos);
      esdCascadeNeg->GetCovarianceXYZPxPyPz(covTr);
      esdCascadeNeg->GetESDpid(pid);
      
      
      if (!fUsedTrack[idxNegFromV0Dghter]) {
        fUsedTrack[idxNegFromV0Dghter] = kTRUE;
        
        UInt_t selectInfo = 0;
        if (fTrackFilter) selectInfo = fTrackFilter->IsSelected(esdCascadeNeg);
        if(fMChandler)fMChandler->SelectParticle(esdCascadeNeg->GetLabel());
        aodTrack = new(tracksArray[fNumberOfTracks++]) IlcAODTrack(  esdCascadeNeg->GetID(),
                                                      esdCascadeNeg->GetLabel(),
                                                      momNeg,
                                                      kTRUE,
                                                      pos,
                                                      kFALSE, // Why kFALSE for "isDCA" ? FIXME
                                                      covTr, 
                                                      (Short_t)esdCascadeNeg->GetSign(),
                                                      esdCascadeNeg->GetITSClusterMap(), 
                                                      pid,
                                                      vV0FromCascade,
                                                      kTRUE,  // usedForVtxFit = kFALSE ? FIXME
                                                      vtx->UsesTrack(esdCascadeNeg->GetID()),
                                                      IlcAODTrack::kSecondary,
                                                      selectInfo);
        aodTrack->SetTPCFitMap(esdCascadeNeg->GetTPCFitMap());
        aodTrack->SetTPCClusterMap(esdCascadeNeg->GetTPCClusterMap());
        aodTrack->SetTPCSharedMap (esdCascadeNeg->GetTPCSharedMap());
        aodTrack->SetChi2perNDF(Chi2perNDF(esdCascadeNeg));
	aodTrack->SetTPCPointsF(esdCascadeNeg->GetTPCNclsF());
	aodTrack->SetTPCNCrossedRows(UShort_t(esdCascadeNeg->GetTPCCrossedRows()));
        fAODTrackRefs->AddAt(aodTrack,idxNegFromV0Dghter);
        
        if (esdCascadeNeg->GetSign() > 0) ++fNumberOfPositiveTracks;
        aodTrack->ConvertIlcPIDtoAODPID();
        aodTrack->SetFlags(esdCascadeNeg->GetStatus());
        SetAODPID(esdCascadeNeg,aodTrack,detpid);
      }
      else {
        aodTrack = static_cast<IlcAODTrack*>(fAODTrackRefs->At(idxNegFromV0Dghter));
      }
      
      vV0FromCascade->AddDaughter(aodTrack);
      
			
      // 4.A.4 - Add the V0 from cascade to the V0 array
      
      Double_t  dcaV0Daughters      = esdV0FromCascade->GetDcaV0Daughters();
      Double_t  dcaV0ToPrimVertex   = esdV0FromCascade->GetD( esd.GetPrimaryVertex()->GetX(),
                                                             esd.GetPrimaryVertex()->GetY(),
                                                             esd.GetPrimaryVertex()->GetZ() );
      esdV0FromCascade->GetPPxPyPz( momPosAtV0vtx[0],momPosAtV0vtx[1],momPosAtV0vtx[2] ); 
      esdV0FromCascade->GetNPxPyPz( momNegAtV0vtx[0],momNegAtV0vtx[1],momNegAtV0vtx[2] ); 
      
      Double_t dcaDaughterToPrimVertex[2] = { 999., 999.}; // ..[0] = DCA in (x,y) for Pos and ..[1] = Neg
      dcaDaughterToPrimVertex[0] = TMath::Abs(esdCascadePos->GetD(	esd.GetPrimaryVertex()->GetX(),
                                                                  esd.GetPrimaryVertex()->GetY(),
                                                                  esd.GetMagneticField())        );
      dcaDaughterToPrimVertex[1] = TMath::Abs(esdCascadeNeg->GetD(	esd.GetPrimaryVertex()->GetX(),
                                                                  esd.GetPrimaryVertex()->GetY(),
                                                                  esd.GetMagneticField())        );
      
      aodV0 = new(V0s()[fNumberOfV0s++]) IlcAODv0( vV0FromCascade, 
                                                  dcaV0Daughters,
                                                  dcaV0ToPrimVertex, 
                                                  momPosAtV0vtx, 
                                                  momNegAtV0vtx, 
                                                  dcaDaughterToPrimVertex); 
      // set the aod v0 on-the-fly status
      aodV0->SetOnFlyStatus(esdV0FromCascade->GetOnFlyStatus());
      
      // Add the aodV0 to the aodVORefs
      fAODV0Refs->AddAt(aodV0,idxV0FromCascade);
      
      fUsedV0[idxV0FromCascade] = kTRUE;
      
    } else { 
      // 4.B - if V0 structure already used
      
      // Note :
      //    one V0 can be used by several cascades (frequent in PbPb evts) : 
      // same V0 which used but attached to different bachelor tracks
      // -> aodVORefs and fAODV0VtxRefs are needed.
      // Goal : avoid a redundancy of the info in "Vertices" and "v0s" clones array.
      
      vV0FromCascade = static_cast<IlcAODVertex*>( fAODV0VtxRefs->At(idxV0FromCascade) );
      aodV0          = static_cast<IlcAODv0*>    ( fAODV0Refs   ->At(idxV0FromCascade) );
      
      // - Treatment of the parent for such a "re-used" V0 :
      // Insert the cascade that reuses the V0 vertex in the lineage chain
      // Before : vV0 -> vCascade1 -> vPrimary
      //  - Hyp : cascade2 uses the same V0 as cascade1
      //  After :  vV0 -> vCascade2 -> vCascade1 -> vPrimary
      
      IlcAODVertex *vCascadePreviousParent = static_cast<IlcAODVertex*> (vV0FromCascade->GetParent());
			vV0FromCascade->SetParent(vCascade);
			vCascade      ->SetParent(vCascadePreviousParent);
      
//      if(fDebug > 2)	
//        printf("---- Cascade / Lineage insertion\n"
//               "Parent of V0 vtx                 = Cascade vtx %p\n"
//               "Parent of the cascade vtx        = Cascade vtx %p\n"
//               "Parent of the parent cascade vtx = Cascade vtx %p\n", 
//               static_cast<void*> (vV0FromCascade->GetParent()),
//               static_cast<void*> (vCascade->GetParent()),
//               static_cast<void*> (vCascadePreviousParent->GetParent()) );
      
    }// end if V0 structure already used
    
//    if (fDebug > 2) {
//      printf("---- Cascade / V0 vertex: \n");
//      vV0FromCascade->Print();
//    }
//    
//    if (fDebug > 4) {
//      printf("---- Cascade / pos dghter : \n");
//			aodTrack->Print();
//      printf("---- Cascade / neg dghter : \n");
//			aodTrack->Print();
//      printf("---- Cascade / aodV0 : \n");
//			aodV0->Print();
//    }
    
    // In any case (used V0 or not), add the V0 vertex to the cascade one.
    vCascade->AddDaughter(vV0FromCascade);	
    
		
    // 5 - Add the primary track of the cascade (if any)
    
    
    // 6 - Add the cascade to the AOD array of cascades
    
    Double_t dcaBachToPrimVertexXY = TMath::Abs(esdCascadeBach->GetD(esd.GetPrimaryVertex()->GetX(),
                                                                     esd.GetPrimaryVertex()->GetY(),
                                                                     esd.GetMagneticField())        );
    
    Double_t momBachAtCascadeVtx[3]={0.};

    esdCascade->GetBPxPyPz(momBachAtCascadeVtx[0], momBachAtCascadeVtx[1], momBachAtCascadeVtx[2]);
    
    aodCascade = new(cascadesArray[fNumberOfCascades++]) IlcAODcascade(  vCascade,
                                                          esdCascade->Charge(),
                                                          esdCascade->GetDcaXiDaughters(),
                                                          -999.,
                                                          // DCAXiToPrimVtx -> needs to be calculated   ----|
                                                          // doesn't exist at ESD level;
                                                          // See AODcascade::DcaXiToPrimVertex(Double, Double, Double)
                                                          dcaBachToPrimVertexXY,
                                                          momBachAtCascadeVtx,
                                                          *aodV0);
    
    if (fDebug > 10) {
      printf("---- Cascade / AOD cascade : \n\n");
      aodCascade->PrintXi(fPrimaryVertex->GetX(), fPrimaryVertex->GetY(), fPrimaryVertex->GetZ());
    }
    
  } // end of the loop on cascades
  
  Cascades().Expand(fNumberOfCascades);
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertV0s(const IlcESDEvent& esd)
{
  // Access to the AOD container of V0s
  
  IlcCodeTimerAuto("",0);

  //
  // V0s
  //
  
  Double_t pos[3] = { 0. };      
  Double_t chi2(0.0);
  Double_t covVtx[6] = { 0. };
  Double_t momPos[3]={0.};
  Double_t covTr[21]={0.};
  Double_t pid[10]={0.};
  IlcAODTrack* aodTrack(0x0);
  IlcAODPid* detpid(0x0);
  Double_t momNeg[3]={0.};
  Double_t momPosAtV0vtx[3]={0.};
  Double_t momNegAtV0vtx[3]={0.};
    
  for (Int_t nV0 = 0; nV0 < esd.GetNumberOfV0s(); ++nV0) 
  {
    if (fUsedV0[nV0]) continue; // skip if already added to the AOD
    
    IlcESDv0 *v0 = esd.GetV0(nV0);
    Int_t posFromV0 = v0->GetPindex();
    Int_t negFromV0 = v0->GetNindex();
    
    // V0 selection 
    //
    IlcESDVertex *esdVtx   = new IlcESDVertex(*(esd.GetPrimaryVertex()));
    IlcESDtrack  *esdV0Pos = esd.GetTrack(posFromV0);
    IlcESDtrack  *esdV0Neg = esd.GetTrack(negFromV0);
    TList v0objects;
    v0objects.AddAt(v0,                      0);
    v0objects.AddAt(esdV0Pos,                1);
    v0objects.AddAt(esdV0Neg,                2);
    v0objects.AddAt(esdVtx,                  3);
    UInt_t selectV0 = 0;
    if (fV0Filter) {
      selectV0 = fV0Filter->IsSelected(&v0objects);
      // this is a little awkward but otherwise the 
      // list wants to access the pointer (delete it) 
      // again when going out of scope
      delete v0objects.RemoveAt(3); // esdVtx created via copy construct
      esdVtx = 0;
      if (!selectV0) 
        continue;
    }
    else{
      delete v0objects.RemoveAt(3); // esdVtx created via copy construct
      esdVtx = 0;
    }
    
    v0->GetXYZ(pos[0], pos[1], pos[2]);
    
    if (!fOldESDformat) {
	    chi2 = v0->GetChi2V0(); // = chi2/NDF since NDF = 2*2-3
	    v0->GetPosCov(covVtx);
    } else {
	    chi2 = -999.;
	    for (Int_t i = 0; i < 6; i++)  covVtx[i] = 0.;
    }
    
    
    IlcAODVertex * vV0 = 
	  new(Vertices()[fNumberOfVertices++]) IlcAODVertex(pos,
                                                      covVtx,
                                                      chi2,
                                                      fPrimaryVertex,
                                                      nV0,
                                                      IlcAODVertex::kV0);
    fPrimaryVertex->AddDaughter(vV0);
    
    
    // Add the positive tracks from the V0
    

    esdV0Pos->GetPxPyPz(momPos);
    esdV0Pos->GetXYZ(pos);
    esdV0Pos->GetCovarianceXYZPxPyPz(covTr);
    esdV0Pos->GetESDpid(pid);
    
    const IlcESDVertex *vtx = esd.GetPrimaryVertex();

    if (!fUsedTrack[posFromV0]) {
	    fUsedTrack[posFromV0] = kTRUE;
	    UInt_t selectInfo = 0;
	    if (fTrackFilter) selectInfo = fTrackFilter->IsSelected(esdV0Pos);
	    if(fMChandler)fMChandler->SelectParticle(esdV0Pos->GetLabel());
	    aodTrack = new(Tracks()[fNumberOfTracks++]) IlcAODTrack(esdV0Pos->GetID(),
                                                    esdV0Pos->GetLabel(), 
                                                    momPos, 
                                                    kTRUE,
                                                    pos,
                                                    kFALSE,
                                                    covTr, 
                                                    (Short_t)esdV0Pos->GetSign(),
                                                    esdV0Pos->GetITSClusterMap(), 
                                                    pid,
                                                    vV0,
                                                    kTRUE,  // check if this is right
                                                    vtx->UsesTrack(esdV0Pos->GetID()),
                                                    IlcAODTrack::kSecondary,
                                                    selectInfo);
	    aodTrack->SetTPCFitMap(esdV0Pos->GetTPCFitMap());
	    aodTrack->SetTPCClusterMap(esdV0Pos->GetTPCClusterMap());
	    aodTrack->SetTPCSharedMap (esdV0Pos->GetTPCSharedMap());
	    aodTrack->SetChi2perNDF(Chi2perNDF(esdV0Pos));
	    aodTrack->SetTPCPointsF(esdV0Pos->GetTPCNclsF());
	    aodTrack->SetTPCNCrossedRows(UShort_t(esdV0Pos->GetTPCCrossedRows()));
	    fAODTrackRefs->AddAt(aodTrack,posFromV0);
	    //	    if (fDebug > 0) printf("-------------------Bo: pos track from original pt %.3f \n",aodTrack->Pt());
	    if (esdV0Pos->GetSign() > 0) ++fNumberOfPositiveTracks;
	    aodTrack->ConvertIlcPIDtoAODPID();
	    aodTrack->SetFlags(esdV0Pos->GetStatus());
      SetAODPID(esdV0Pos,aodTrack,detpid);
    }
    else {
	    aodTrack = static_cast<IlcAODTrack*>(fAODTrackRefs->At(posFromV0));
	    //	    if (fDebug > 0) printf("-------------------Bo pos track from refArray pt %.3f \n",aodTrack->Pt());
    }
    vV0->AddDaughter(aodTrack);
    
    // Add the negative tracks from the V0
    
    esdV0Neg->GetPxPyPz(momNeg);
    esdV0Neg->GetXYZ(pos);
    esdV0Neg->GetCovarianceXYZPxPyPz(covTr);
    esdV0Neg->GetESDpid(pid);
    
    if (!fUsedTrack[negFromV0]) {
	    fUsedTrack[negFromV0] = kTRUE;
	    UInt_t selectInfo = 0;
	    if (fTrackFilter) selectInfo = fTrackFilter->IsSelected(esdV0Neg);
	    if(fMChandler)fMChandler->SelectParticle(esdV0Neg->GetLabel());
	    aodTrack = new(Tracks()[fNumberOfTracks++]) IlcAODTrack(esdV0Neg->GetID(),
                                                    esdV0Neg->GetLabel(),
                                                    momNeg,
                                                    kTRUE,
                                                    pos,
                                                    kFALSE,
                                                    covTr, 
                                                    (Short_t)esdV0Neg->GetSign(),
                                                    esdV0Neg->GetITSClusterMap(), 
                                                    pid,
                                                    vV0,
                                                    kTRUE,  // check if this is right
                                                    vtx->UsesTrack(esdV0Neg->GetID()),
                                                    IlcAODTrack::kSecondary,
                                                    selectInfo);
	    aodTrack->SetTPCFitMap(esdV0Neg->GetTPCFitMap());
	    aodTrack->SetTPCClusterMap(esdV0Neg->GetTPCClusterMap());
	    aodTrack->SetTPCSharedMap (esdV0Neg->GetTPCSharedMap());
	    aodTrack->SetChi2perNDF(Chi2perNDF(esdV0Neg));
	    aodTrack->SetTPCPointsF(esdV0Neg->GetTPCNclsF());
	    aodTrack->SetTPCNCrossedRows(UShort_t(esdV0Neg->GetTPCCrossedRows()));
	    
	    fAODTrackRefs->AddAt(aodTrack,negFromV0);
	    //	    if (fDebug > 0) printf("-------------------Bo: neg track from original pt %.3f \n",aodTrack->Pt());
	    if (esdV0Neg->GetSign() > 0) ++fNumberOfPositiveTracks;
	    aodTrack->ConvertIlcPIDtoAODPID();
	    aodTrack->SetFlags(esdV0Neg->GetStatus());
      SetAODPID(esdV0Neg,aodTrack,detpid);
    }
    else {
	    aodTrack = static_cast<IlcAODTrack*>(fAODTrackRefs->At(negFromV0));
	    //	    if (fDebug > 0) printf("-------------------Bo neg track from refArray pt %.3f \n",aodTrack->Pt());
    }
    vV0->AddDaughter(aodTrack);
    
    
    // Add the V0 the V0 array as well
    
    Double_t  dcaV0Daughters      = v0->GetDcaV0Daughters();
    Double_t  dcaV0ToPrimVertex   = v0->GetD(esd.GetPrimaryVertex()->GetX(),
                                             esd.GetPrimaryVertex()->GetY(),
                                             esd.GetPrimaryVertex()->GetZ());
    
    v0->GetPPxPyPz(momPosAtV0vtx[0],momPosAtV0vtx[1],momPosAtV0vtx[2]); 
    v0->GetNPxPyPz(momNegAtV0vtx[0],momNegAtV0vtx[1],momNegAtV0vtx[2]); 
    
    Double_t dcaDaughterToPrimVertex[2] = { 999., 999.}; // ..[0] = DCA in (x,y) for Pos and ..[1] = Neg
    dcaDaughterToPrimVertex[0] = TMath::Abs(esdV0Pos->GetD(  esd.GetPrimaryVertex()->GetX(),
                                                           esd.GetPrimaryVertex()->GetY(),
                                                           esd.GetMagneticField()) );
    dcaDaughterToPrimVertex[1] = TMath::Abs(esdV0Neg->GetD(  esd.GetPrimaryVertex()->GetX(),
                                                           esd.GetPrimaryVertex()->GetY(),
                                                           esd.GetMagneticField()) );
    
    IlcAODv0* aodV0 = new(V0s()[fNumberOfV0s++]) IlcAODv0(vV0, 
                                                dcaV0Daughters,
                                                dcaV0ToPrimVertex,
                                                momPosAtV0vtx,
                                                momNegAtV0vtx,
                                                dcaDaughterToPrimVertex);
    
    // set the aod v0 on-the-fly status
    aodV0->SetOnFlyStatus(v0->GetOnFlyStatus());
  }//End of loop on V0s 
  
  V0s().Expand(fNumberOfV0s);	 
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertTPCOnlyTracks(const IlcESDEvent& esd)
{
  // Convert TPC only tracks
  // Here we have wo hybrid appraoch to remove fakes
  // ******* ITSTPC ********
  // Uses a cut on the ITS properties to select global tracks
  // which are than marked as HybdridITSTPC for the remainder 
  // the TPC only tracks are flagged as HybridITSTPConly. 
  // Note, in order not to get fakes back in the TPC cuts, one needs 
  // two "ITS" cuts one tight (1) (to throw out fakes) and one lose (2) (to NOT flag the trakcs in the TPC only)
  // using cut number (3)
  // so fHybridFilterMask == (1)|(2) fTPCFilterMask = (3), Usercode needs to slect with mask = (1)|(3) and track->IsHybridITSTPC()
  // ******* TPC ********
  // Here, only TPC tracks are flagged that pass the tight ITS cuts and tracks that pass the TPC cuts and NOT the loose ITS cuts
  // the ITS cuts neeed to be added to the filter as extra cuts, since here the selections info is reset in the global and put to the TPC only track

  IlcCodeTimerAuto("",0);
  
  // Loop over the tracks and extract and mask out all aod tracks that pass the selections for AODt racks
  for(int it = 0;it < fNumberOfTracks;++it)
  {
    IlcAODTrack *tr = (IlcAODTrack*)(Tracks().At(it));
    if(!tr)continue;
    UInt_t map = tr->GetFilterMap();
    if(map&fTPCConstrainedFilterMask){
      // we only reset the track select ionfo, no deletion...
      tr->SetFilterMap(map&~fTPCConstrainedFilterMask);
    }
    if(map&fHybridFilterMaskTPCCG){
      // this is one part of the hybrid tracks
      // the others not passing the selection will be TPC only selected below
      tr->SetIsHybridTPCConstrainedGlobal(kTRUE);
    }
  }
  // Loop over the ESD trcks and pick out the tracks passing TPC only cuts
  
  
  const IlcESDVertex *vtxSPD = esd.GetPrimaryVertexSPD();
  const IlcESDVertex *vtx = esd.GetPrimaryVertex();

  Double_t pos[3] = { 0. };      
  Double_t covTr[21]={0.};
  Double_t pid[10]={0.};  


  Double_t p[3] = { 0. };

  Double_t pDCA[3] = { 0. }; // momentum at DCA
  Double_t rDCA[3] = { 0. }; // position at DCA
  Float_t  dDCA[2] = {0.};    // DCA to the vertex d and z
  Float_t  cDCA[3] = {0.};    // covariance of impact parameters


  IlcAODTrack* aodTrack(0x0);
  //  IlcAODPid* detpid(0x0);

  // account for change in pT after the constraint
  Float_t ptMax = 1E10;
  Float_t ptMin = 0;
  for(int i = 0;i<32;i++){
    if(fTPCConstrainedFilterMask&(1<<i)){
      IlcESDtrackCuts*cuts = (IlcESDtrackCuts*)fTrackFilter->GetCuts()->At(i);
      Float_t tmp1= 0,tmp2 = 0;
      cuts->GetPtRange(tmp1,tmp2);
      if(tmp1>ptMin)ptMin=tmp1;
      if(tmp2<ptMax)ptMax=tmp2;
    }
  } 

  for (Int_t nTrack = 0; nTrack < esd.GetNumberOfTracks(); ++nTrack) 
  {
    IlcESDtrack* esdTrack = esd.GetTrack(nTrack); //carefull do not modify it othwise  need to work with a copy 
    
    UInt_t selectInfo = 0;
    Bool_t isHybridITSTPC = false;
    //
    // Track selection
    if (fTrackFilter) {
      selectInfo = fTrackFilter->IsSelected(esdTrack);
    }

    if(!(selectInfo&fHybridFilterMaskTPCCG)){
      // not already selected tracks, use second part of hybrid tracks
      isHybridITSTPC = true;
      // too save space one could only store these...
    }

    selectInfo &= fTPCConstrainedFilterMask;
    if (!selectInfo)continue;
    if (fWriteHybridTPCCOnly&&!isHybridITSTPC)continue; // write only complementary tracks
    // create a tpc only tracl
    IlcESDtrack *track = IlcESDtrackCuts::GetTPCOnlyTrack(const_cast<IlcESDEvent*>(&esd),esdTrack->GetID());
    if(!track) continue;
    
    if(track->Pt()>0.)
    {
      // only constrain tracks above threshold
      IlcExternalTrackParam exParam;
      // take the B-field from the ESD, no 3D fieldMap available at this point
      Bool_t relate = false;
      relate = track->RelateToVertexTPC(vtxSPD,esd.GetMagneticField(),kVeryBig,&exParam);
      if(!relate){
        delete track;
        continue;
      }
      // fetch the track parameters at the DCA (unconstraint)
      if(track->GetTPCInnerParam()){
	track->GetTPCInnerParam()->GetPxPyPz(pDCA);
	track->GetTPCInnerParam()->GetXYZ(rDCA);
      }
      // get the DCA to the vertex:
      track->GetImpactParametersTPC(dDCA,cDCA);
      // set the constrained parameters to the track
      track->Set(exParam.GetX(),exParam.GetAlpha(),exParam.GetParameter(),exParam.GetCovariance());
    }
    
    track->GetPxPyPz(p);

    Float_t pT = track->Pt();
    if(pT<ptMin||pT>ptMax){
      delete track;
      continue;
    }

    // 


    track->GetXYZ(pos);
    track->GetCovarianceXYZPxPyPz(covTr);
    esdTrack->GetESDpid(pid);// original PID

    if(fMChandler)fMChandler->SelectParticle(esdTrack->GetLabel());
    aodTrack = new(Tracks()[fNumberOfTracks++]) IlcAODTrack((track->GetID()+1)*-1,
                                                            track->GetLabel(),
                                                            p,
                                                            kTRUE,
                                                            pos,
                                                            kFALSE,
                                                            covTr, 
                                                            (Short_t)track->GetSign(),
                                                            track->GetITSClusterMap(), 
                                                            pid,
                                                            fPrimaryVertex,
                                                            kTRUE, // check if this is right
                                                            vtx->UsesTrack(track->GetID()),
                                                            IlcAODTrack::kPrimary, 
                                                            selectInfo);
    aodTrack->SetIsHybridTPCConstrainedGlobal(isHybridITSTPC);    
    aodTrack->SetTPCFitMap(track->GetTPCFitMap());
    aodTrack->SetTPCClusterMap(track->GetTPCClusterMap());
    aodTrack->SetTPCSharedMap (track->GetTPCSharedMap());
    aodTrack->SetIsTPCConstrained(kTRUE);    
    aodTrack->SetChi2perNDF(Chi2perNDF(esdTrack)); // original track
    // set the DCA values to the AOD track
    aodTrack->SetPxPyPzAtDCA(pDCA[0],pDCA[1],pDCA[2]);
    aodTrack->SetXYAtDCA(rDCA[0],rDCA[1]);
    aodTrack->SetDCA(dDCA[0],dDCA[1]);

    aodTrack->SetFlags(track->GetStatus());
    aodTrack->SetTPCPointsF(track->GetTPCNclsF());
    aodTrack->SetTPCNCrossedRows(UShort_t(track->GetTPCCrossedRows()));

    //Perform progagation of tracks if needed
    if(fDoPropagateTrackToEMCal) PropagateTrackToEMCal(esdTrack);
    aodTrack->SetTrackPhiEtaOnEMCal(esdTrack->GetTrackPhiOnEMCal(),esdTrack->GetTrackEtaOnEMCal());

    // do not duplicate PID information 
    //    aodTrack->ConvertIlcPIDtoAODPID();
    //    SetAODPID(esdTrack,aodTrack,detpid);

    delete track;
  } // end of loop on tracks
  
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertGlobalConstrainedTracks(const IlcESDEvent& esd)
{

  // Here we have the option to store the complement from global constraint information
  // to tracks passing tight cuts (1) in order not to get fakes back in, one needs 
  // two sets of cuts one tight (1) (to throw out fakes) and one lose (2) (fakes/bad tracks would pass (2) but not (1))
  // using cut number (3) selects the tracks that complement (1) e.g. tracks witout ITS refit or cluster requirement


  IlcCodeTimerAuto("",0);
  
  // Loop over the tracks and extract and mask out all aod tracks that pass the selections for AODt racks
  for(int it = 0;it < fNumberOfTracks;++it)
  {
    IlcAODTrack *tr = (IlcAODTrack*)(Tracks().At(it));
    if(!tr)continue;
    UInt_t map = tr->GetFilterMap();
    if(map&fGlobalConstrainedFilterMask){
      // we only reset the track select info, no deletion...
      // mask reset mask in case track is already taken
      tr->SetFilterMap(map&~fGlobalConstrainedFilterMask);
    }
    if(map&fHybridFilterMaskGCG){
      // this is one part of the hybrid tracks
      // the others not passing the selection will be the ones selected below
      tr->SetIsHybridGlobalConstrainedGlobal(kTRUE);
    }
  }
  // Loop over the ESD trcks and pick out the tracks passing the GlobalConstraint cuts
 

  Double_t pos[3] = { 0. };      
  Double_t covTr[21]={0.};
  Double_t pid[10]={0.};  
  Double_t p[3] = { 0. };

  Double_t pDCA[3] = { 0. }; // momentum at DCA
  Double_t rDCA[3] = { 0. }; // position at DCA
  Float_t  dDCA[2] = {0.};    // DCA to the vertex d and z
  Float_t  cDCA[3] = {0.};    // covariance of impact parameters


  IlcAODTrack* aodTrack(0x0);
  IlcAODPid* detpid(0x0);
  const IlcESDVertex *vtx = esd.GetPrimaryVertex();

  // account for change in pT after the constraint
  Float_t ptMax = 1E10;
  Float_t ptMin = 0;
  for(int i = 0;i<32;i++){
    if(fGlobalConstrainedFilterMask&(1<<i)){
      IlcESDtrackCuts*cuts = (IlcESDtrackCuts*)fTrackFilter->GetCuts()->At(i);
      Float_t tmp1= 0,tmp2 = 0;
      cuts->GetPtRange(tmp1,tmp2);
      if(tmp1>ptMin)ptMin=tmp1;
      if(tmp2<ptMax)ptMax=tmp2;
    }
  } 



 for (Int_t nTrack = 0; nTrack < esd.GetNumberOfTracks(); ++nTrack) 
  {
    IlcESDtrack* esdTrack = esd.GetTrack(nTrack); //carefull do not modify it othwise  need to work with a copy 
    const IlcExternalTrackParam * exParamGC = esdTrack->GetConstrainedParam();
    if(!exParamGC)continue;

    UInt_t selectInfo = 0;
    Bool_t isHybridGC = false;

    //
    // Track selection
    if (fTrackFilter) {
      selectInfo = fTrackFilter->IsSelected(esdTrack);
    }


    if(!(selectInfo&fHybridFilterMaskGCG))isHybridGC = true;
    if (fWriteHybridGCOnly&&!isHybridGC)continue; // write only complementary tracks

    selectInfo &= fGlobalConstrainedFilterMask;
    if (!selectInfo)continue;
    // fetch the track parameters at the DCA (unconstrained)
    esdTrack->GetPxPyPz(pDCA);
    esdTrack->GetXYZ(rDCA);
    // get the DCA to the vertex:
    esdTrack->GetImpactParameters(dDCA,cDCA);

    if (!esdTrack->GetConstrainedPxPyPz(p)) continue;


    Float_t pT = exParamGC->Pt();
    if(pT<ptMin||pT>ptMax){
      continue;
    }


    esdTrack->GetConstrainedXYZ(pos);
    exParamGC->GetCovarianceXYZPxPyPz(covTr);
    esdTrack->GetESDpid(pid);
    if(fMChandler)fMChandler->SelectParticle(esdTrack->GetLabel());
    aodTrack = new(Tracks()[fNumberOfTracks++]) IlcAODTrack((esdTrack->GetID()+1)*-1,
                                                            esdTrack->GetLabel(),
                                                            p,
                                                            kTRUE,
                                                            pos,
                                                            kFALSE,
                                                            covTr, 
                                                            (Short_t)esdTrack->GetSign(),
                                                            esdTrack->GetITSClusterMap(), 
                                                            pid,
                                                            fPrimaryVertex,
                                                            kTRUE, // check if this is right
                                                            vtx->UsesTrack(esdTrack->GetID()),
                                                            IlcAODTrack::kPrimary, 
                                                            selectInfo);
    aodTrack->SetIsHybridGlobalConstrainedGlobal(isHybridGC);    
    aodTrack->SetIsGlobalConstrained(kTRUE);    
    aodTrack->SetTPCFitMap(esdTrack->GetTPCFitMap());
    aodTrack->SetTPCClusterMap(esdTrack->GetTPCClusterMap());
    aodTrack->SetTPCSharedMap (esdTrack->GetTPCSharedMap());
    aodTrack->SetChi2perNDF(Chi2perNDF(esdTrack));


    // set the DCA values to the AOD track
    aodTrack->SetPxPyPzAtDCA(pDCA[0],pDCA[1],pDCA[2]);
    aodTrack->SetXYAtDCA(rDCA[0],rDCA[1]);
    aodTrack->SetDCA(dDCA[0],dDCA[1]);

    aodTrack->SetFlags(esdTrack->GetStatus());
    aodTrack->SetTPCPointsF(esdTrack->GetTPCNclsF());
    aodTrack->SetTPCNCrossedRows(UShort_t(esdTrack->GetTPCCrossedRows()));

    if(isHybridGC){
      // only copy AOD information for hybrid, no duplicate information
      aodTrack->ConvertIlcPIDtoAODPID();
      SetAODPID(esdTrack,aodTrack,detpid);
    }

    //Perform progagation of tracks if needed
    if(fDoPropagateTrackToEMCal) PropagateTrackToEMCal(esdTrack);
    aodTrack->SetTrackPhiEtaOnEMCal(esdTrack->GetTrackPhiOnEMCal(),esdTrack->GetTrackEtaOnEMCal());
  } // end of loop on tracks
  
}


//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertTracks(const IlcESDEvent& esd)
{
  // Tracks (primary and orphan)

  IlcCodeTimerAuto("",0);
  
  IlcDebug(1,Form("NUMBER OF ESD TRACKS %5d\n", esd.GetNumberOfTracks()));
  
  const IlcESDVertex *vtx = esd.GetPrimaryVertex();
  Double_t p[3] = { 0. };
  Double_t pos[3] = { 0. };
  Double_t covTr[21] = { 0. };
  Double_t pid[10] = { 0. };
  IlcAODTrack* aodTrack(0x0);
  IlcAODPid* detpid(0x0);
  
  for (Int_t nTrack = 0; nTrack < esd.GetNumberOfTracks(); ++nTrack) 
  {
    if (fUsedTrack[nTrack]) continue;
    
    IlcESDtrack *esdTrack = esd.GetTrack(nTrack);
    UInt_t selectInfo = 0;
    //
    // Track selection
    if (fTrackFilter) {
	    selectInfo = fTrackFilter->IsSelected(esdTrack);
	    if (!selectInfo && !vtx->UsesTrack(esdTrack->GetID())) continue;
    }
    
    
    esdTrack->GetPxPyPz(p);
    esdTrack->GetXYZ(pos);
    esdTrack->GetCovarianceXYZPxPyPz(covTr);
    esdTrack->GetESDpid(pid);
    if(fMChandler)fMChandler->SelectParticle(esdTrack->GetLabel());
    fPrimaryVertex->AddDaughter(aodTrack =
                         new(Tracks()[fNumberOfTracks++]) IlcAODTrack(esdTrack->GetID(),
                                                            esdTrack->GetLabel(),
                                                            p,
                                                            kTRUE,
                                                            pos,
                                                            kFALSE,
                                                            covTr, 
                                                            (Short_t)esdTrack->GetSign(),
                                                            esdTrack->GetITSClusterMap(), 
                                                            pid,
                                                            fPrimaryVertex,
                                                            kTRUE, // check if this is right
                                                            vtx->UsesTrack(esdTrack->GetID()),
                                                            IlcAODTrack::kPrimary, 
                                                            selectInfo)
                         );
    aodTrack->SetTPCFitMap(esdTrack->GetTPCFitMap());
    aodTrack->SetTPCClusterMap(esdTrack->GetTPCClusterMap());
    aodTrack->SetTPCSharedMap (esdTrack->GetTPCSharedMap());
    aodTrack->SetChi2perNDF(Chi2perNDF(esdTrack));
    aodTrack->SetTPCPointsF(esdTrack->GetTPCNclsF());
    aodTrack->SetTPCNCrossedRows(UShort_t(esdTrack->GetTPCCrossedRows()));
    if(esdTrack->IsEMCAL()) aodTrack->SetEMCALcluster(esdTrack->GetEMCALcluster());
    if(esdTrack->IsPHOS())  aodTrack->SetPHOScluster(esdTrack->GetPHOScluster());

    //Perform progagation of tracks if needed
    if(fDoPropagateTrackToEMCal) PropagateTrackToEMCal(esdTrack);
    aodTrack->SetTrackPhiEtaOnEMCal(esdTrack->GetTrackPhiOnEMCal(),esdTrack->GetTrackEtaOnEMCal());

    fAODTrackRefs->AddAt(aodTrack, nTrack);
    
    
    if (esdTrack->GetSign() > 0) ++fNumberOfPositiveTracks;
    aodTrack->SetFlags(esdTrack->GetStatus());
    aodTrack->ConvertIlcPIDtoAODPID();
    SetAODPID(esdTrack,aodTrack,detpid);
  } // end of loop on tracks
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::PropagateTrackToEMCal(IlcESDtrack *esdTrack)
{
  Double_t trkPos[3] = {0.,0.,0.};
  Double_t EMCalEta=-999, EMCalPhi=-999;
  Double_t trkphi = esdTrack->Phi()*TMath::RadToDeg();
  if(TMath::Abs(esdTrack->Eta())<0.9 && trkphi > 10 && trkphi < 250 )
    {
      IlcExternalTrackParam *trkParam = const_cast<IlcExternalTrackParam*>(esdTrack->GetInnerParam());
      if(trkParam)
	{
	  IlcExternalTrackParam trkParamTmp(*trkParam);
	  if(IlcTrackerBase::PropagateTrackToBxByBz(&trkParamTmp, 430, esdTrack->GetMass(), 20, kTRUE, 0.8, -1))
	    {
	      trkParamTmp.GetXYZ(trkPos);
	      TVector3 trkPosVec(trkPos[0],trkPos[1],trkPos[2]);
	      EMCalEta = trkPosVec.Eta();
	      EMCalPhi = trkPosVec.Phi();
	      if(EMCalPhi<0)  EMCalPhi += 2*TMath::Pi();
	      esdTrack->SetTrackPhiEtaOnEMCal(EMCalPhi,EMCalEta);
	    }
	}
    }
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertPmdClusters(const IlcESDEvent& esd)
{
// Convert PMD Clusters 
  IlcCodeTimerAuto("",0);
  Int_t jPmdClusters=0;
  // Access to the AOD container of PMD clusters
  TClonesArray &pmdClusters = *(AODEvent()->GetPmdClusters());
  for (Int_t iPmd = 0; iPmd < esd.GetNumberOfPmdTracks(); ++iPmd) {
    // file pmd clusters, to be revised!
    IlcESDPmdTrack *pmdTrack = esd.GetPmdTrack(iPmd);
    Int_t nLabel = 0;
    Int_t *label = 0x0;
    Double_t posPmd[3] = { pmdTrack->GetClusterX(), pmdTrack->GetClusterY(), pmdTrack->GetClusterZ()};
    Double_t pidPmd[13] = { 0.}; // to be revised!
    // type not set!
    // assoc cluster not set
    new(pmdClusters[jPmdClusters++]) IlcAODPmdCluster(iPmd, nLabel, label, pmdTrack->GetClusterADC(), posPmd, pidPmd);
  }
}


//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertCaloClusters(const IlcESDEvent& esd)
{
// Convert Calorimeter Clusters
  IlcCodeTimerAuto("",0);
  
  // Access to the AOD container of clusters
  TClonesArray &caloClusters = *(AODEvent()->GetCaloClusters());
  Int_t jClusters(0);
  
  for (Int_t iClust=0; iClust<esd.GetNumberOfCaloClusters(); ++iClust) {
    
    IlcESDCaloCluster * cluster = esd.GetCaloCluster(iClust);
    
    Int_t  id        = cluster->GetID();
    Int_t  nLabel    = cluster->GetNLabels();
    Int_t *labels    = cluster->GetLabels();
    if(labels){ 
		  for(int i = 0;i < nLabel;++i){
			  if(fMChandler)fMChandler->SelectParticle(labels[i]);
		  }
	  }		
    
    Float_t energy = cluster->E();
    Float_t posF[3] = { 0.};
    cluster->GetPosition(posF);
    
    IlcAODCaloCluster *caloCluster = new(caloClusters[jClusters++]) IlcAODCaloCluster(id,
                                                                                      nLabel,
                                                                                      labels,
                                                                                      energy,
                                                                                      posF,
                                                                                      NULL,
                                                                                      cluster->GetType(),0);
    
    caloCluster->SetCaloCluster(cluster->GetDistanceToBadChannel(),
                                cluster->GetDispersion(),
                                cluster->GetM20(), cluster->GetM02(),
                                cluster->GetEmcCpvDistance(),  
                                cluster->GetNExMax(),cluster->GetTOF()) ;
    
    caloCluster->SetPIDFromESD(cluster->GetPID());
    caloCluster->SetNCells(cluster->GetNCells());
    caloCluster->SetCellsAbsId(cluster->GetCellsAbsId());
    caloCluster->SetCellsAmplitudeFraction(cluster->GetCellsAmplitudeFraction());

    caloCluster->SetTrackDistance(cluster->GetTrackDx(), cluster->GetTrackDz());
    
    Int_t nMatchCount = 0;
    TArrayI* matchedT = 	cluster->GetTracksMatched();
    if (fNumberOfTracks>0 && matchedT && cluster->GetTrackMatchedIndex() >= 0) {	
      for (Int_t im = 0; im < matchedT->GetSize(); im++) {
        Int_t iESDtrack = matchedT->At(im);;
        if (fAODTrackRefs->At(iESDtrack) != 0) {
          caloCluster->AddTrackMatched((IlcAODTrack*)fAODTrackRefs->At(iESDtrack));
	  nMatchCount++;
        }
      }
    }
    if(nMatchCount==0)
      caloCluster->SetTrackDistance(-999,-999);
    
  } 
  caloClusters.Expand(jClusters); // resize TObjArray to 'remove' slots for pseudo clusters	 
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertCaloTrigger(TString calo, const IlcESDEvent& esd)
{
	IlcCodeTimerAuto("",0);
		
	if (calo == "PHOS") 
	{
	  IlcAODCaloTrigger &aodTrigger = *(AODEvent()->GetCaloTrigger(calo));
	  IlcESDCaloTrigger &esdTrigger = *(esd.GetCaloTrigger(calo));

	  aodTrigger.Allocate(esdTrigger.GetEntries());
	  esdTrigger.Reset();

	  Float_t a;
	  Int_t tmod,tabsId;

	  while (esdTrigger.Next()) {
	    esdTrigger.GetPosition(tmod,tabsId);
	    esdTrigger.GetAmplitude(a);
	    aodTrigger.Add(tmod,tabsId,a,0.,(Int_t*)NULL,0,0,0);
	  }

	  return;
	}
			
	IlcAODHandler *aodHandler = dynamic_cast<IlcAODHandler*>(IlcAnalysisManager::GetAnalysisManager()->GetOutputEventHandler()); 
			
	if (aodHandler)
	{
		TTree *aodTree = aodHandler->GetTree();
					
		if (aodTree)
		{
			Int_t *type = esd.GetCaloTriggerType();
							
			for (Int_t i = 0; i < 15; i++) 
			{
				aodTree->GetUserInfo()->Add(new TParameter<int>(Form("EMCALCaloTrigger%d",i), type[i]));
			}
		}
	}
						
	IlcAODCaloTrigger &aodTrigger = *(AODEvent()->GetCaloTrigger(calo));
						
	IlcESDCaloTrigger &esdTrigger = *(esd.GetCaloTrigger(calo));
						
	aodTrigger.Allocate(esdTrigger.GetEntries());
						
	esdTrigger.Reset();
	while (esdTrigger.Next())
	{	  
		Int_t px, py, ts, nTimes, times[10], b; 
		Float_t a, t;
								
		esdTrigger.GetPosition(px, py);
						
		esdTrigger.GetAmplitude(a);
		esdTrigger.GetTime(t);
								
		esdTrigger.GetL0Times(times);
		esdTrigger.GetNL0Times(nTimes);
								
		esdTrigger.GetL1TimeSum(ts);
								
		esdTrigger.GetTriggerBits(b);
								
		aodTrigger.Add(px, py, a, t, times, nTimes, ts, b);
	}
							
	for (int i = 0; i < 4; i++) aodTrigger.SetL1Threshold(i, esdTrigger.GetL1Threshold(i));
							
	Int_t v0[2] = 
	{
		esdTrigger.GetL1V0(0),
		esdTrigger.GetL1V0(1)
	};		
								
	aodTrigger.SetL1V0(v0);	
	aodTrigger.SetL1FrameMask(esdTrigger.GetL1FrameMask());
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertEMCALCells(const IlcESDEvent& esd)
{
// Convert EMCAL Cells
  IlcCodeTimerAuto("",0);
  // fill EMCAL cell info
  if (esd.GetEMCALCells()) { // protection against missing ESD information
    IlcESDCaloCells &esdEMcells = *(esd.GetEMCALCells());
    Int_t nEMcell = esdEMcells.GetNumberOfCells() ;
    
    IlcAODCaloCells &aodEMcells = *(AODEvent()->GetEMCALCells());
    aodEMcells.CreateContainer(nEMcell);
    aodEMcells.SetType(IlcAODCaloCells::kEMCALCell);
    for (Int_t iCell = 0; iCell < nEMcell; iCell++) {      
      aodEMcells.SetCell(iCell,esdEMcells.GetCellNumber(iCell),esdEMcells.GetAmplitude(iCell),
                         esdEMcells.GetTime(iCell), esdEMcells.GetMCLabel(iCell), esdEMcells.GetEFraction(iCell));
    }
    aodEMcells.Sort();
  }
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertPHOSCells(const IlcESDEvent& esd)
{
// Convert PHOS Cells
  IlcCodeTimerAuto("",0);
  // fill PHOS cell info
  if (esd.GetPHOSCells()) { // protection against missing ESD information
    IlcESDCaloCells &esdPHcells = *(esd.GetPHOSCells());
    Int_t nPHcell = esdPHcells.GetNumberOfCells() ;
    
    IlcAODCaloCells &aodPHcells = *(AODEvent()->GetPHOSCells());
    aodPHcells.CreateContainer(nPHcell);
    aodPHcells.SetType(IlcAODCaloCells::kPHOSCell);
    for (Int_t iCell = 0; iCell < nPHcell; iCell++) {      
      aodPHcells.SetCell(iCell,esdPHcells.GetCellNumber(iCell),esdPHcells.GetAmplitude(iCell),
                         esdPHcells.GetTime(iCell), esdPHcells.GetMCLabel(iCell), esdPHcells.GetEFraction(iCell));
    }
    aodPHcells.Sort();
  }
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertTracklets(const IlcESDEvent& esd)
{
  // tracklets    
  IlcCodeTimerAuto("",0);

  IlcAODTracklets &SPDTracklets = *(AODEvent()->GetTracklets());
  const IlcMultiplicity *mult = esd.GetMultiplicity();
  if (mult) {
    if (mult->GetNumberOfTracklets()>0) {
      SPDTracklets.CreateContainer(mult->GetNumberOfTracklets());
      
      for (Int_t n=0; n<mult->GetNumberOfTracklets(); n++) {
        if(fMChandler){
          fMChandler->SelectParticle(mult->GetLabel(n, 0));
          fMChandler->SelectParticle(mult->GetLabel(n, 1));
        }
        SPDTracklets.SetTracklet(n, mult->GetTheta(n), mult->GetPhi(n), mult->GetDeltaPhi(n), mult->GetLabel(n, 0),mult->GetLabel(n, 1));
      }
    }
  } else {
    //Printf("ERROR: IlcMultiplicity could not be retrieved from ESD");
  }
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertKinks(const IlcESDEvent& esd)
{
  IlcCodeTimerAuto("",0);
  
  // Kinks: it is a big mess the access to the information in the kinks
  // The loop is on the tracks in order to find the mother and daugther of each kink
  
  Double_t covTr[21]={0.};
  Double_t pid[10]={0.};
  IlcAODPid* detpid(0x0);
  
  fNumberOfKinks = esd.GetNumberOfKinks();

  const IlcESDVertex* vtx = esd.GetPrimaryVertex();
  
  for (Int_t iTrack=0; iTrack<esd.GetNumberOfTracks(); ++iTrack) 
  {
    IlcESDtrack * esdTrack = esd.GetTrack(iTrack);
    
    Int_t ikink = esdTrack->GetKinkIndex(0);
    
    if (ikink && fNumberOfKinks) {
	    // Negative kink index: mother, positive: daughter
	    
	    // Search for the second track of the kink
	    
	    for (Int_t jTrack = iTrack+1; jTrack<esd.GetNumberOfTracks(); ++jTrack) {
        
        IlcESDtrack * esdTrack1 = esd.GetTrack(jTrack);
        
        Int_t jkink = esdTrack1->GetKinkIndex(0);
        
        if ( TMath::Abs(ikink)==TMath::Abs(jkink) ) {
          
          // The two tracks are from the same kink
          
          if (fUsedKink[TMath::Abs(ikink)-1]) continue; // skip used kinks
          
          Int_t imother = -1;
          Int_t idaughter = -1;
          
          if (ikink<0 && jkink>0) {
            
            imother = iTrack;
            idaughter = jTrack;
          }
          else if (ikink>0 && jkink<0) {
            
            imother = jTrack;
            idaughter = iTrack;
          }
          else {
            //			cerr << "Error: Wrong combination of kink indexes: "
            //			     << ikink << " " << jkink << endl;
            continue;
          }
          
          // Add the mother track if it passed primary track selection cuts
          
          IlcAODTrack * mother = NULL;
          
          UInt_t selectInfo = 0;
          if (fTrackFilter) {
            selectInfo = fTrackFilter->IsSelected(esd.GetTrack(imother));
            if (!selectInfo) continue;
          }
          
          if (!fUsedTrack[imother]) {
            
            fUsedTrack[imother] = kTRUE;
            
            IlcESDtrack *esdTrackM = esd.GetTrack(imother);
            Double_t p[3] = { 0. };
            Double_t pos[3] = { 0. };
            esdTrackM->GetPxPyPz(p);
            esdTrackM->GetXYZ(pos);
            esdTrackM->GetCovarianceXYZPxPyPz(covTr);
            esdTrackM->GetESDpid(pid);
            if(fMChandler)fMChandler->SelectParticle(esdTrackM->GetLabel());
            mother = 
            new(Tracks()[fNumberOfTracks++]) IlcAODTrack(esdTrackM->GetID(),
                                               esdTrackM->GetLabel(),
                                               p,
                                               kTRUE,
                                               pos,
                                               kFALSE,
                                               covTr, 
                                               (Short_t)esdTrackM->GetSign(),
                                               esdTrackM->GetITSClusterMap(), 
                                               pid,
                                               fPrimaryVertex,
                                               kTRUE, // check if this is right
                                               vtx->UsesTrack(esdTrack->GetID()),
                                               IlcAODTrack::kPrimary,
                                               selectInfo);
            mother->SetTPCFitMap(esdTrackM->GetTPCFitMap());
            mother->SetTPCClusterMap(esdTrackM->GetTPCClusterMap());
            mother->SetTPCSharedMap (esdTrackM->GetTPCSharedMap());
            mother->SetChi2perNDF(Chi2perNDF(esdTrackM));
            mother->SetTPCPointsF(esdTrackM->GetTPCNclsF());
	    mother->SetTPCNCrossedRows(UShort_t(esdTrackM->GetTPCCrossedRows()));

            fAODTrackRefs->AddAt(mother, imother);
            
            if (esdTrackM->GetSign() > 0) ++fNumberOfPositiveTracks;
            mother->SetFlags(esdTrackM->GetStatus());
            mother->ConvertIlcPIDtoAODPID();
            fPrimaryVertex->AddDaughter(mother);
            mother->ConvertIlcPIDtoAODPID();
            SetAODPID(esdTrackM,mother,detpid);
          }
          else {
            //			cerr << "Error: event " << esd.GetEventNumberInFile() << " kink " << TMath::Abs(ikink)-1
            //			     << " track " << imother << " has already been used!" << endl;
          }
          
          // Add the kink vertex
          IlcESDkink * kink = esd.GetKink(TMath::Abs(ikink)-1);
          
          IlcAODVertex * vkink = 
          new(Vertices()[fNumberOfVertices++]) IlcAODVertex(kink->GetPosition(),
                                                  NULL,
                                                  0.,
                                                  mother,
                                                  esdTrack->GetID(),  // This is the track ID of the mother's track!
                                                  IlcAODVertex::kKink);
          // Add the daughter track
          
          IlcAODTrack * daughter = NULL;
          
          if (!fUsedTrack[idaughter]) {
            
            fUsedTrack[idaughter] = kTRUE;
            
            IlcESDtrack *esdTrackD = esd.GetTrack(idaughter);
            Double_t p[3] = { 0. };
            Double_t pos[3] = { 0. };

            esdTrackD->GetPxPyPz(p);
            esdTrackD->GetXYZ(pos);
            esdTrackD->GetCovarianceXYZPxPyPz(covTr);
            esdTrackD->GetESDpid(pid);
            selectInfo = 0;
            if (fTrackFilter) selectInfo = fTrackFilter->IsSelected(esdTrackD);
            if(fMChandler)fMChandler->SelectParticle(esdTrackD->GetLabel());
            daughter = 
            new(Tracks()[fNumberOfTracks++]) IlcAODTrack(esdTrackD->GetID(),
                                               esdTrackD->GetLabel(),
                                               p,
                                               kTRUE,
                                               pos,
                                               kFALSE,
                                               covTr, 
                                               (Short_t)esdTrackD->GetSign(),
                                               esdTrackD->GetITSClusterMap(), 
                                               pid,
                                               vkink,
                                               kTRUE, // check if this is right
                                               vtx->UsesTrack(esdTrack->GetID()),
                                               IlcAODTrack::kSecondary,
                                               selectInfo);
            daughter->SetTPCFitMap(esdTrackD->GetTPCFitMap());
            daughter->SetTPCClusterMap(esdTrackD->GetTPCClusterMap());
            daughter->SetTPCSharedMap (esdTrackD->GetTPCSharedMap());
	    daughter->SetTPCPointsF(esdTrackD->GetTPCNclsF());
	    daughter->SetTPCNCrossedRows(UShort_t(esdTrackD->GetTPCCrossedRows()));
            fAODTrackRefs->AddAt(daughter, idaughter);
            
            if (esdTrackD->GetSign() > 0) ++fNumberOfPositiveTracks;
            daughter->SetFlags(esdTrackD->GetStatus());
            daughter->ConvertIlcPIDtoAODPID();
            vkink->AddDaughter(daughter);
            daughter->ConvertIlcPIDtoAODPID();
            SetAODPID(esdTrackD,daughter,detpid);
          }
          else {
            //			cerr << "Error: event " << esd.GetEventNumberInFile() << " kink " << TMath::Abs(ikink)-1
            //			     << " track " << idaughter << " has already been used!" << endl;
          }
        }
	    }
    }      
  }
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertPrimaryVertices(const IlcESDEvent& esd)
{
  IlcCodeTimerAuto("",0);
  
  // Access to the AOD container of vertices
  fNumberOfVertices = 0;
  
  Double_t pos[3] = { 0. };
  Double_t covVtx[6] = { 0. };

  // Add primary vertex. The primary tracks will be defined
  // after the loops on the composite objects (V0, cascades, kinks)
  const IlcESDVertex *vtx = esd.GetPrimaryVertex();
  
  vtx->GetXYZ(pos); // position
  vtx->GetCovMatrix(covVtx); //covariance matrix
  
  fPrimaryVertex = new(Vertices()[fNumberOfVertices++])
  IlcAODVertex(pos, covVtx, vtx->GetChi2toNDF(), NULL, -1, IlcAODVertex::kPrimary);
  fPrimaryVertex->SetName(vtx->GetName());
  fPrimaryVertex->SetTitle(vtx->GetTitle());
  
  TString vtitle = vtx->GetTitle();
  if (!vtitle.Contains("VertexerTracks")) 
    fPrimaryVertex->SetNContributors(vtx->GetNContributors());
  
  if (fDebug > 0) fPrimaryVertex->Print();  
  
  // Add SPD "main" vertex 
  const IlcESDVertex *vtxS = esd.GetPrimaryVertexSPD();
  vtxS->GetXYZ(pos); // position
  vtxS->GetCovMatrix(covVtx); //covariance matrix
  IlcAODVertex * mVSPD = new(Vertices()[fNumberOfVertices++])
  IlcAODVertex(pos, covVtx, vtxS->GetChi2toNDF(), NULL, -1, IlcAODVertex::kMainSPD);
  mVSPD->SetName(vtxS->GetName());
  mVSPD->SetTitle(vtxS->GetTitle());
  mVSPD->SetNContributors(vtxS->GetNContributors()); 
  
  // Add SPD pileup vertices
  for(Int_t iV=0; iV<esd.GetNumberOfPileupVerticesSPD(); ++iV)
  {
    const IlcESDVertex *vtxP = esd.GetPileupVertexSPD(iV);
    vtxP->GetXYZ(pos); // position
    vtxP->GetCovMatrix(covVtx); //covariance matrix
    IlcAODVertex * pVSPD =  new(Vertices()[fNumberOfVertices++])
    IlcAODVertex(pos, covVtx, vtxP->GetChi2toNDF(), NULL, -1, IlcAODVertex::kPileupSPD);
    pVSPD->SetName(vtxP->GetName());
    pVSPD->SetTitle(vtxP->GetTitle());
    pVSPD->SetNContributors(vtxP->GetNContributors()); 
    pVSPD->SetBC(vtxP->GetBC());
  }
  
  // Add TRK pileup vertices
  for(Int_t iV=0; iV<esd.GetNumberOfPileupVerticesTracks(); ++iV)
  {
    const IlcESDVertex *vtxP = esd.GetPileupVertexTracks(iV);
    vtxP->GetXYZ(pos); // position
    vtxP->GetCovMatrix(covVtx); //covariance matrix
    IlcAODVertex * pVTRK = new(Vertices()[fNumberOfVertices++])
    IlcAODVertex(pos, covVtx, vtxP->GetChi2toNDF(), NULL, -1, IlcAODVertex::kPileupTracks);
    pVTRK->SetName(vtxP->GetName());
    pVTRK->SetTitle(vtxP->GetTitle());
    pVTRK->SetNContributors(vtxP->GetNContributors());
    pVTRK->SetBC(vtxP->GetBC());
  }
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertVZERO(const IlcESDEvent& esd)
{
  // Convert VZERO data
  IlcAODVZERO* vzeroData = AODEvent()->GetVZEROData();
  *vzeroData = *(esd.GetVZEROData());
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertTZERO(const IlcESDEvent& esd)
{
  // Convert TZERO data
  const IlcESDTZERO* esdTzero = esd.GetESDTZERO(); 
  IlcAODTZERO* aodTzero = AODEvent()->GetTZEROData();

  for (Int_t icase=0; icase<3; icase++){ 
    aodTzero->SetT0TOF(    icase, esdTzero->GetT0TOF(icase));
    aodTzero->SetT0TOFbest(icase, esdTzero->GetT0TOFbest(icase)); 
  }
  aodTzero->SetBackgroundFlag(esdTzero->GetBackgroundFlag());
  aodTzero->SetPileupFlag(esdTzero->GetPileupFlag());
  aodTzero->SetSatelliteFlag(esdTzero->GetSatellite()); 

  Float_t rawTime[24];
  for(Int_t ipmt=0; ipmt<24; ipmt++)
    rawTime[ipmt] = esdTzero->GetTimeFull(ipmt,0);
   
  Int_t idxOfFirstPmtA = -1,       idxOfFirstPmtC = -1;
  Float_t timeOfFirstPmtA = 9999, timeOfFirstPmtC = 9999;
  for(int ipmt=0;  ipmt<12; ipmt++){
    if( rawTime[ipmt] > -200 && rawTime[ipmt] < timeOfFirstPmtC && rawTime[ipmt]!=0){
      timeOfFirstPmtC = rawTime[ipmt];
      idxOfFirstPmtC  = ipmt;
    }
  }
  for(int ipmt=12; ipmt<24; ipmt++){
    if( rawTime[ipmt] > -200 && rawTime[ipmt] < timeOfFirstPmtA && rawTime[ipmt]!=0 ){
      timeOfFirstPmtA = rawTime[ipmt];
      idxOfFirstPmtA  = ipmt;
    }
  }

  if(idxOfFirstPmtA != -1 && idxOfFirstPmtC != -1){
    //speed of light in cm/ns   TMath::C()*1e-7 
    Float_t vertexraw = TMath::C()*1e-7 * (rawTime[idxOfFirstPmtA] - rawTime[idxOfFirstPmtC])/2;
    aodTzero->SetT0VertexRaw( vertexraw );
  }else{
    aodTzero->SetT0VertexRaw(99999);
  }

}


//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertZDC(const IlcESDEvent& esd)
{
  // Convert ZDC data
  IlcESDZDC* esdZDC = esd.GetZDCData();
  
  const Double_t zem1Energy = esdZDC->GetZEM1Energy();
  const Double_t zem2Energy = esdZDC->GetZEM2Energy();
   
  const Double_t *towZNC = esdZDC->GetZNCTowerEnergy();
  const Double_t *towZPC = esdZDC->GetZPCTowerEnergy();
  const Double_t *towZNA = esdZDC->GetZNATowerEnergy();
  const Double_t *towZPA = esdZDC->GetZPATowerEnergy();
  const Double_t *towZNCLG = esdZDC->GetZNCTowerEnergyLR();
  const Double_t *towZNALG = esdZDC->GetZNATowerEnergyLR();
  
  IlcAODZDC* zdcAOD = AODEvent()->GetZDCData();

  zdcAOD->SetZEM1Energy(zem1Energy);
  zdcAOD->SetZEM2Energy(zem2Energy);
  zdcAOD->SetZNCTowers(towZNC, towZNCLG);
  zdcAOD->SetZNATowers(towZNA, towZNALG);
  zdcAOD->SetZPCTowers(towZPC);
  zdcAOD->SetZPATowers(towZPA);
  
  zdcAOD->SetZDCParticipants(esdZDC->GetZDCParticipants(), esdZDC->GetZDCPartSideA(), esdZDC->GetZDCPartSideC());
  zdcAOD->SetZDCImpactParameter(esdZDC->GetImpactParameter(), esdZDC->GetImpactParamSideA(), 
  	esdZDC->GetImpactParamSideC());
  zdcAOD->SetZDCTDCSum(esdZDC->GetZNTDCSum(0));	
  zdcAOD->SetZDCTDCDiff(esdZDC->GetZNTDCDiff(0));	

}

//_______________________________________________________________________________________________________________________________________
Int_t IlcAnalysisTaskESDfilter::ConvertHMPID(const IlcESDEvent& esd) // clm
{
  //
  // Convtert ESD HMPID info to AOD and return the number of good tracks with HMPID signal.
  // We need to return an int since there is no signal counter in the ESD.
  //
  
  IlcCodeTimerAuto("",0);
  
  Int_t cntHmpidGoodTracks = 0;
  
  Float_t  xMip = 0;
  Float_t  yMip = 0;
  Int_t    qMip = 0;
  Int_t    nphMip = 0;
  
  Float_t xTrk = 0;
  Float_t yTrk = 0;
  Float_t thetaTrk = 0;
  Float_t phiTrk = 0;
  
  Double_t hmpPid[5]={0};
  Double_t hmpMom[3]={0};
  
  TClonesArray &hmpidRings = *(AODEvent()->GetHMPIDrings());
  
 for (Int_t iTrack=0; iTrack<esd.GetNumberOfTracks(); ++iTrack) 
  {
    if(! esd.GetTrack(iTrack) ) continue;
      
    if(esd.GetTrack(iTrack)->GetHMPIDsignal() > -20 ) {                  // 
       
      (esd.GetTrack(iTrack))->GetHMPIDmip(xMip, yMip, qMip, nphMip);     // Get MIP properties
      (esd.GetTrack(iTrack))->GetHMPIDtrk(xTrk,yTrk,thetaTrk,phiTrk);
      (esd.GetTrack(iTrack))->GetHMPIDpid(hmpPid);
      if((esd.GetTrack(iTrack))->GetOuterHmpParam()) (esd.GetTrack(iTrack))->GetOuterHmpPxPyPz(hmpMom);
      
     if(esd.GetTrack(iTrack)->GetHMPIDsignal() == 0 && thetaTrk == 0 && qMip == 0 && nphMip ==0 ) continue;  //
      
     new(hmpidRings[cntHmpidGoodTracks++]) IlcAODHMPIDrings(
                                                                 (esd.GetTrack(iTrack))->GetID(),             // Unique track id to attach the ring to
                                                                 1000000*nphMip+qMip,                         // MIP charge and number of photons
                                                                 (esd.GetTrack(iTrack))->GetHMPIDcluIdx(),    // 1000000*chamber id + cluster idx of the assigned MIP cluster  
                                                                 thetaTrk,                                    // track inclination angle theta
                                                                 phiTrk,                                      // track inclination angle phi
                                                                 (esd.GetTrack(iTrack))->GetHMPIDsignal(),    // Cherenkov angle
                                                                 (esd.GetTrack(iTrack))->GetHMPIDoccupancy(), // Occupancy claculated for the given chamber 
                                                                 (esd.GetTrack(iTrack))->GetHMPIDchi2(),      // Ring resolution squared
                                                                  xTrk,                                       // Track x coordinate (LORS)
                                                                  yTrk,                                       // Track y coordinate (LORS)
                                                                  xMip,                                       // MIP x coordinate (LORS)
                                                                  yMip,                                       // MIP y coordinate (LORS)
                                                                  hmpPid,                                     // PID probablities from ESD, remove later once it is in CombinedPid
                                                                  hmpMom                                      // Track momentum in HMPID at ring reconstruction
                                                               );  
     
      //  Printf(Form("+++++++++ yes/no: %d  %lf %lf %lf %lf %lf %lf ",(esd.GetTrack(iTrack))->IsHMPID(),thetaTrk, (esd.GetTrack(iTrack))->GetHMPIDchi2(),xTrk, yTrk , xMip, yMip));
     
                                                                
   }// HMPID signal > -20
  }//___esd track loop
  
  return cntHmpidGoodTracks;
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::ConvertESDtoAOD() 
{
  // ESD Filter analysis task executed for each event
  
  IlcESDEvent* esd = dynamic_cast<IlcESDEvent*>(InputEvent());
  
  if(!esd)return;

  IlcCodeTimerAuto("",0);
  
  fOldESDformat = ( esd->GetIlcESDOld() != 0x0 );
 
      // Reconstruct cascades and V0 here
  if (fIsV0CascadeRecoEnabled) {
    esd->ResetCascades();
    esd->ResetV0s();

    IlcV0vertexer lV0vtxer;
    IlcCascadeVertexer lCascVtxer;

    lV0vtxer.SetCuts(fV0Cuts);
    lCascVtxer.SetCuts(fCascadeCuts);


    lV0vtxer.Tracks2V0vertices(esd);
    lCascVtxer.V0sTracks2CascadeVertices(esd);
  }

 
  fNumberOfTracks = 0;
  fNumberOfPositiveTracks = 0;
  fNumberOfV0s = 0;
  fNumberOfVertices = 0;
  fNumberOfCascades = 0;
  fNumberOfKinks = 0;
    
  IlcAODHeader* header = ConvertHeader(*esd);

  if ( fIsVZEROEnabled ) ConvertVZERO(*esd);
  if ( fIsTZEROEnabled ) ConvertTZERO(*esd);
  
  // Fetch Stack for debuggging if available 
  fMChandler=0x0;
  if(MCEvent())
  {
    fMChandler = (IlcMCEventHandler*) ((IlcAnalysisManager::GetAnalysisManager())->GetMCtruthEventHandler()); 
  }
  
  // loop over events and fill them
  // Multiplicity information needed by the header (to be revised!)
  Int_t nTracks    = esd->GetNumberOfTracks();
  for (Int_t iTrack=0; iTrack<nTracks; ++iTrack) esd->GetTrack(iTrack)->SetESDEvent(esd);

  // Update the header

  Int_t nV0s      = esd->GetNumberOfV0s();
  Int_t nCascades = esd->GetNumberOfCascades();
  Int_t nKinks    = esd->GetNumberOfKinks();
  Int_t nVertices = nV0s + nCascades /*V0 wihtin cascade already counted*/+ nKinks + 1 /* = prim. vtx*/;
  Int_t nPileSPDVertices=1+esd->GetNumberOfPileupVerticesSPD(); // also SPD main vertex
  Int_t nPileTrkVertices=esd->GetNumberOfPileupVerticesTracks();
  nVertices+=nPileSPDVertices;
  nVertices+=nPileTrkVertices;
  Int_t nJets     = 0;
  Int_t nCaloClus = esd->GetNumberOfCaloClusters();
  Int_t nFmdClus  = 0;
  Int_t nPmdClus  = esd->GetNumberOfPmdTracks();
  Int_t nHmpidRings = 0;  
    
  IlcDebug(1,Form("   NV0=%d  NCASCADES=%d  NKINKS=%d", nV0s, nCascades, nKinks));
       
  AODEvent()->ResetStd(nTracks, nVertices, nV0s, nCascades, nJets, nCaloClus, nFmdClus, nPmdClus,nHmpidRings);

  if (nV0s > 0) 
  {
    // RefArray to store a mapping between esd V0 number and newly created AOD-Vertex V0
    fAODV0VtxRefs = new TRefArray(nV0s);
    // RefArray to store the mapping between esd V0 number and newly created AOD-V0
    fAODV0Refs = new TRefArray(nV0s); 
    // Array to take into account the V0s already added to the AOD (V0 within cascades)
    fUsedV0 = new Bool_t[nV0s];
    for (Int_t iV0=0; iV0<nV0s; ++iV0) fUsedV0[iV0]=kFALSE;
  }
  
  if (nTracks>0) 
  {
    // RefArray to store the mapping between esd track number and newly created AOD-Track
    
    fAODTrackRefs = new TRefArray(nTracks);

    // Array to take into account the tracks already added to the AOD    
    fUsedTrack = new Bool_t[nTracks];
    for (Int_t iTrack=0; iTrack<nTracks; ++iTrack) fUsedTrack[iTrack]=kFALSE;
  }
  
  // Array to take into account the kinks already added to the AOD
  if (nKinks>0) 
  {
    fUsedKink = new Bool_t[nKinks];
    for (Int_t iKink=0; iKink<nKinks; ++iKink) fUsedKink[iKink]=kFALSE;
  }
    
  ConvertPrimaryVertices(*esd);

  //setting best TOF PID
  IlcESDInputHandler* esdH = dynamic_cast<IlcESDInputHandler*>(IlcAnalysisManager::GetAnalysisManager()->GetInputEventHandler());
  if (esdH)
      fESDpid = esdH->GetESDpid();

  if (fIsPidOwner && fESDpid){
    delete fESDpid;
    fESDpid = 0;
  }
  if(!fESDpid)
  { //in case of no Tender attached 
    fESDpid = new IlcESDpid;
    fIsPidOwner = kTRUE;
  }
  
  if(!esd->GetTOFHeader())
  { //protection in case the pass2 LHC10b,c,d have been processed without tender. 
    Float_t t0spread[10];
    Float_t intrinsicTOFres=100; //ps ok for LHC10b,c,d pass2!! 
    for (Int_t i=0; i<10; i++) t0spread[i] = (TMath::Sqrt(esd->GetSigma2DiamondZ()))/0.03; //0.03 to convert from cm to ps
    fESDpid->GetTOFResponse().SetT0resolution(t0spread);
    fESDpid->GetTOFResponse().SetTimeResolution(intrinsicTOFres);
      //    fESDpid->SetTOFResponse(esd, (IlcESDpid::EStartTimeType_t)fTimeZeroType);    
    IlcTOFHeader tmpTOFHeader(0,t0spread[0],0,NULL,NULL,NULL,intrinsicTOFres,t0spread[0]);   
    AODEvent()->SetTOFHeader(&tmpTOFHeader);         // write dummy TOF header in AOD
  } else {
    AODEvent()->SetTOFHeader(esd->GetTOFHeader());    // write TOF header in AOD
  }
  
  //  if(esd->GetTOFHeader() && fIsPidOwner) fESDpid->SetTOFResponse(esd, (IlcESDpid::EStartTimeType_t)fTimeZeroType); //in case of AOD production strating form LHC10e without Tender. 
  
  if ( fAreCascadesEnabled ) ConvertCascades(*esd);

  if ( fAreV0sEnabled ) ConvertV0s(*esd);
  
  if ( fAreKinksEnabled ) ConvertKinks(*esd);
  
  if ( fAreTracksEnabled ) ConvertTracks(*esd);
  
  // Update number of AOD tracks in header at the end of track loop (M.G.)
  header->SetRefMultiplicity(fNumberOfTracks);
  header->SetRefMultiplicityPos(fNumberOfPositiveTracks);
  header->SetRefMultiplicityNeg(fNumberOfTracks - fNumberOfPositiveTracks);

  if ( fTPCConstrainedFilterMask ) ConvertTPCOnlyTracks(*esd);
  if( fGlobalConstrainedFilterMask) ConvertGlobalConstrainedTracks(*esd);  

  if ( fArePmdClustersEnabled ) ConvertPmdClusters(*esd);
  
  if ( fAreCaloClustersEnabled ) ConvertCaloClusters(*esd);
  
  if ( fAreEMCALCellsEnabled )ConvertEMCALCells(*esd);
  
  if ( fArePHOSCellsEnabled )ConvertPHOSCells(*esd);
	
	if ( fAreEMCALTriggerEnabled )ConvertCaloTrigger(TString("EMCAL"), *esd);

	if ( fArePHOSTriggerEnabled )ConvertCaloTrigger(TString("PHOS"), *esd);
  
  if ( fAreTrackletsEnabled ) ConvertTracklets(*esd);
  if ( fIsZDCEnabled ) ConvertZDC(*esd);
  
  if(fIsHMPIDEnabled) nHmpidRings = ConvertHMPID(*esd); 

  delete fAODTrackRefs; fAODTrackRefs=0x0;
  delete fAODV0VtxRefs; fAODV0VtxRefs=0x0;
  delete fAODV0Refs; fAODV0Refs=0x0;
  
  delete[] fUsedTrack; fUsedTrack=0x0;
  delete[] fUsedV0; fUsedV0=0x0;
  delete[] fUsedKink; fUsedKink=0x0;

  if ( fIsPidOwner){
    delete fESDpid;
    fESDpid = 0x0;
  }


}


//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::SetAODPID(IlcESDtrack *esdtrack, IlcAODTrack *aodtrack, IlcAODPid *detpid)
{
  //
  // Setter for the raw PID detector signals
  //

  // Save PID object for candidate electrons
    Bool_t pidSave = kFALSE;
    if (fTrackFilter) {
	Bool_t selectInfo = fTrackFilter->IsSelected((char*) "Electrons");
	if (selectInfo)  pidSave = kTRUE;
    }


    // Tracks passing pt cut 
    if(esdtrack->Pt()>fHighPthreshold) {
	pidSave = kTRUE;
    } else {
	if(fPtshape){
	    if(esdtrack->Pt()> fPtshape->GetXmin()){
		Double_t y = fPtshape->Eval(esdtrack->Pt())/fPtshape->Eval(fHighPthreshold);
		if(gRandom->Rndm(0)<1./y){
		    pidSave = kTRUE;
		}//end rndm
	    }//end if p < pmin
	}//end if p function
    }// end else

    if (pidSave) {
      if(!aodtrack->GetDetPid()){// prevent memory leak when calling SetAODPID twice for the same track
	detpid = new IlcAODPid();
	SetDetectorRawSignals(detpid,esdtrack);
	aodtrack->SetDetPID(detpid);
      }
    }
}

//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::SetDetectorRawSignals(IlcAODPid *aodpid, IlcESDtrack *track)
{
//
//assignment of the detector signals (AliXXXesdPID inspired)
//
 if(!track){
 IlcInfo("no ESD track found. .....exiting");
 return;
 }
 // TPC momentum
 const IlcExternalTrackParam *in=track->GetInnerParam();
 if (in) {
   aodpid->SetTPCmomentum(in->GetP());
 }else{
   aodpid->SetTPCmomentum(-1.);
 }


 aodpid->SetITSsignal(track->GetITSsignal());
 Double_t itsdedx[4]; // dE/dx samples for individual ITS layers
 track->GetITSdEdxSamples(itsdedx);
 aodpid->SetITSdEdxSamples(itsdedx);

 aodpid->SetTPCsignal(track->GetTPCsignal());
 aodpid->SetTPCsignalN(track->GetTPCsignalN());
 if(track->GetTPCdEdxInfo()) aodpid->SetTPCdEdxInfo(track->GetTPCdEdxInfo());

 //n TRD planes = 6
 Int_t nslices = track->GetNumberOfTRDslices()*6;
 TArrayD trdslices(nslices);
 for(Int_t iSl =0; iSl < track->GetNumberOfTRDslices(); iSl++) {
   for(Int_t iPl =0; iPl<6; iPl++) trdslices[iPl*track->GetNumberOfTRDslices()+iSl] = track->GetTRDslice(iPl,iSl);
 }
 
//TRD momentum
 for(Int_t iPl=0;iPl<6;iPl++){
   Double_t trdmom=track->GetTRDmomentum(iPl);
   aodpid->SetTRDmomentum(iPl,trdmom);
 }

 aodpid->SetTRDsignal(track->GetNumberOfTRDslices()*6,trdslices.GetArray());

 //TRD clusters and tracklets
 aodpid->SetTRDncls(track->GetTRDncls());
 aodpid->SetTRDntrackletsPID(track->GetTRDntrackletsPID());
 
 aodpid->SetTRDChi2(track->GetTRDchi2());

 //TOF PID  
 Double_t times[IlcAODPid::kSPECIES]; track->GetIntegratedTimes(times);
 aodpid->SetIntegratedTimes(times);

   //  Float_t tzeroTrack = fESDpid->GetTOFResponse().GetStartTime(track->P());
   //  aodpid->SetTOFsignal(track->GetTOFsignal()-tzeroTrack);
   aodpid->SetTOFsignal(track->GetTOFsignal());
  
  Double_t tofRes[5];
  for (Int_t iMass=0; iMass<5; iMass++){
    //    tofRes[iMass]=(Double_t)fESDpid->GetTOFResponse().GetExpectedSigma(track->P(), times[iMass], IlcPID::ParticleMass(iMass));
    tofRes[iMass]=0; //backward compatibility
  }
  aodpid->SetTOFpidResolution(tofRes);

//  aodpid->SetHMPIDsignal(0); // set to zero for compression but it will be removed later

}

Double_t  IlcAnalysisTaskESDfilter::Chi2perNDF(IlcESDtrack* track)
{
    // Calculate chi2 per ndf for track
    Int_t  nClustersTPC = track->GetTPCNcls();

    if ( nClustersTPC > 5) {
       return (track->GetTPCchi2()/Float_t(nClustersTPC - 5));
    } else {
       return (-1.);
    }
 }


//______________________________________________________________________________
void IlcAnalysisTaskESDfilter::Terminate(Option_t */*option*/)
{
// Terminate analysis
//
    if (fDebug > 1) printf("AnalysisESDfilter: Terminate() \n");
}

//______________________________________________________________________________
void  IlcAnalysisTaskESDfilter::PrintMCInfo(IlcStack *pStack,Int_t label){
// Print MC info
  if(!pStack)return;
  label = TMath::Abs(label);
  TParticle *part = pStack->Particle(label);
  Printf("########################");
  Printf("%s:%d %d UniqueID %d PDG %d P %3.3f",(char*)__FILE__,__LINE__,label,part->GetUniqueID(),part->GetPdgCode(),part->P());
  part->Print();
  TParticle* mother = part;
  Int_t imo = part->GetFirstMother();
  Int_t nprim = pStack->GetNprimary();
  //  while((imo >= nprim) && (mother->GetUniqueID() == 4)) {
  while((imo >= nprim)) {
    mother =  pStack->Particle(imo);
    Printf("Mother %s:%d Label %d UniqueID %d PDG %d P %3.3f",(char*)__FILE__,__LINE__,imo,mother->GetUniqueID(),mother->GetPdgCode(),mother->P());
    mother->Print();
    imo =  mother->GetFirstMother();
  }
  Printf("########################");
}

//______________________________________________________

