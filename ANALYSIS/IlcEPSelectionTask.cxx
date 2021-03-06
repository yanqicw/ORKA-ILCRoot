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

//*****************************************************
//   Class IlcEPSelectionTask
//   Class to determine event plane            
//   author: Alberica Toia, Johanna Gramling
//*****************************************************

#include "IlcEPSelectionTask.h"

#include <TTree.h>
#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TFile.h>
#include <TObjString.h>
#include <TString.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TDirectory.h>
#include <TSystem.h>
#include <iostream>
#include <TRandom2.h>
#include <TArrayF.h>

#include "IlcAnalysisManager.h"
#include "IlcVEvent.h"
#include "IlcESD.h"
#include "IlcESDEvent.h"
#include "IlcMCEvent.h"
#include "IlcESDtrack.h"
#include "IlcESDtrackCuts.h"
#include "IlcESDHeader.h"
#include "IlcESDInputHandler.h"
#include "IlcAODHandler.h"
#include "IlcAODEvent.h"
#include "IlcHeader.h"
#include "IlcGenHijingEventHeader.h"
#include "IlcAnalysisTaskSE.h"
#include "IlcPhysicsSelectionTask.h"
#include "IlcPhysicsSelection.h"
#include "IlcBackgroundSelection.h"
#include "IlcESDUtils.h"
#include "IlcOADBContainer.h"
#include "IlcAODMCHeader.h"
#include "IlcAODTrack.h"
#include "IlcVTrack.h"
#include "IlcEventplane.h"

using std::cout;
using std::endl;
ClassImp(IlcEPSelectionTask)

//________________________________________________________________________
IlcEPSelectionTask::IlcEPSelectionTask():
IlcAnalysisTaskSE(),
  fAnalysisInput("ESD"),
  fTrackType("TPC"),
  fUseMCRP(kFALSE),
  fUsePhiWeight(kFALSE),
  fUsePtWeight(kFALSE),
  fSaveTrackContribution(kFALSE),
  fUserphidist(kFALSE),
  fUsercuts(kFALSE),
  fRunNumber(-15),
  fAODfilterbit(1),
  fEtaGap(0.),
  fSplitMethod(0),
  fESDtrackCuts(0),
  fEPContainer(0),
  fPhiDist(0),
  fQVector(0),
  fQContributionX(0),
  fQContributionY(0),
  fEventplaneQ(0),
  fQsub1(0),
  fQsub2(0),
  fQsubRes(0),  
  fOutputList(0),
  fHOutEventplaneQ(0),
  fHOutPhi(0),
  fHOutPhiCorr(0),
  fHOutsub1sub2(0),
  fHOutNTEPRes(0),
  fHOutPTPsi(0),
  fHOutDiff(0),
  fHOutleadPTPsi(0)
{   
  // Default constructor
  IlcInfo("Event Plane Selection enabled.");
}   

//________________________________________________________________________
IlcEPSelectionTask::IlcEPSelectionTask(const char *name):
  IlcAnalysisTaskSE(name),
  fAnalysisInput("ESD"),
  fTrackType("TPC"),
  fUseMCRP(kFALSE),
  fUsePhiWeight(kFALSE),
  fUsePtWeight(kFALSE),  
  fSaveTrackContribution(kFALSE),
  fUserphidist(kFALSE),
  fUsercuts(kFALSE),
  fRunNumber(-15),
  fAODfilterbit(1),
  fEtaGap(0.),
  fSplitMethod(0),
  fESDtrackCuts(0),
  fEPContainer(0),
  fPhiDist(0),
  fQVector(0),
  fQContributionX(0),
  fQContributionY(0),
  fEventplaneQ(0),
  fQsub1(0),
  fQsub2(0),
  fQsubRes(0),
  fOutputList(0),
  fHOutEventplaneQ(0),
  fHOutPhi(0),
  fHOutPhiCorr(0),
  fHOutsub1sub2(0),
  fHOutNTEPRes(0),
  fHOutPTPsi(0),
  fHOutDiff(0),
  fHOutleadPTPsi(0)
{
  // Default constructor
  IlcInfo("Event Plane Selection enabled.");
  DefineOutput(1, TList::Class());
}
 
//________________________________________________________________________
IlcEPSelectionTask::~IlcEPSelectionTask()
{
  // Destructor  
  if (fOutputList && !IlcAnalysisManager::GetAnalysisManager()->IsProofMode()){
      delete fOutputList;
      fOutputList = 0;
  }
  if (fESDtrackCuts){
      delete fESDtrackCuts;
      fESDtrackCuts = 0;
  }
  if (fUserphidist) {
    if (fPhiDist) {
      delete fPhiDist;
      fPhiDist = 0;
    }
  }
  if (fEPContainer){
      delete fEPContainer;
      fEPContainer = 0;
  }
}  

//________________________________________________________________________
void IlcEPSelectionTask::UserCreateOutputObjects()
{  
  // Create the output containers
  if (fDebug>1) printf("IlcEPSelectionTask::UserCreateOutputObjects() \n");
  IlcLog::SetClassDebugLevel("IlcEPSelectionTask", IlcLog::kInfo);

  fOutputList = new TList();
  fOutputList->SetOwner();
  fHOutEventplaneQ = new TH1F("fHOutEventplaneQ","fHOutEventplaneQ; Event Plane Q",100,0,TMath::Pi());
  fHOutPhi = new TH1F("fHOutPhi","fHOutPhi; Phi Distribution",100,0,TMath::TwoPi());
  fHOutPhiCorr = new TH1F("fHOutPhiCorr","fHOutPhiCorr; Corrected Phi Distribution",100,0,TMath::TwoPi());
  fHOutsub1sub2 = new TH2F("fHOutsub1sub2","fHOutsub1sub2; EP1; EP2",100,0,TMath::Pi(),100,0,TMath::Pi());
  fHOutNTEPRes = new TH2F("fHOutNTEPRes","fHOutNTEPRes; Number of Tracks; Event Plane Resolution",100,0,5000,100,-TMath::Pi(),TMath::Pi());
  fHOutPTPsi = new TH2F("fHOutPTPsi","fHOutPTPsi; PT; Phi-EP",100,0,20,100,0,TMath::Pi());
  fHOutDiff = new TH2F("fHOutDiff","fHOutDiff; EP; MCEP",100,0,TMath::Pi(),100,0,TMath::Pi());
  fHOutleadPTPsi = new TH2F("fHOutleadPTPsi","fHOutleadPTPsi; leadPT; EP",100,0,TMath::Pi(),100,0,TMath::Pi());

  fOutputList->Add(fHOutEventplaneQ);
  fOutputList->Add(fHOutPhi);
  fOutputList->Add(fHOutPhiCorr);
  fOutputList->Add(fHOutsub1sub2);
  fOutputList->Add(fHOutNTEPRes);
  fOutputList->Add(fHOutPTPsi);
  fOutputList->Add(fHOutleadPTPsi);
  fOutputList->Add(fHOutDiff);
  
  PostData(1, fOutputList); 
  
    if(!fUserphidist) { // if it's already set and custom class is required, we use the one provided by the user

    
    TString oadbfilename; 

    if (fAnalysisInput.CompareTo("AOD")==0){
      oadbfilename = (Form("%s/COMMON/EVENTPLANE/data/epphidist.aod.root", IlcAnalysisManager::GetOADBPath()));
    } else if (fAnalysisInput.CompareTo("ESD")==0){
      oadbfilename = (Form("%s/COMMON/EVENTPLANE/data/epphidist.root", IlcAnalysisManager::GetOADBPath()));
    }
 
    TFile foadb(oadbfilename); 
    if(!foadb.IsOpen()) IlcFatal(Form("Cannot open OADB file %s", oadbfilename.Data()));

    IlcInfo("Using Standard OADB");
    fEPContainer = (IlcOADBContainer*) foadb.Get("epphidist");    
    if (!fEPContainer) IlcFatal("Cannot fetch OADB container for EP selection");
    foadb.Close();
    }

}

//________________________________________________________________________
void IlcEPSelectionTask::UserExec(Option_t */*option*/)
{ 
  // Execute analysis for current event:
  if (fDebug>1) printf(" **** IlcEPSelectionTask::UserExec() \n");
  
//   fRunNumber = -15;
 
  IlcEventplane *esdEP;
  TVector2 qq1;
  TVector2 qq2;
  Double_t fRP = 0.; // the monte carlo reaction plane angle
    
  if (fAnalysisInput.CompareTo("ESD")==0){

    IlcVEvent* event = InputEvent();
    IlcESDEvent* esd = dynamic_cast<IlcESDEvent*>(event);
    if (esd){    
      if (!(fRunNumber == esd->GetRunNumber())) {
	  fRunNumber = esd->GetRunNumber();
	    SetPhiDist();      
      }
      
      
      if (fUseMCRP) {
	  IlcMCEvent* mcEvent  = MCEvent();      
	  if (mcEvent && mcEvent->GenEventHeader()) {
	      IlcGenHijingEventHeader* headerH = dynamic_cast<IlcGenHijingEventHeader*>(mcEvent->GenEventHeader());
	      if (headerH) fRP = headerH->ReactionPlaneAngle();
	  }
      }
      
      esdEP = esd->GetEventplane();
      if (fSaveTrackContribution) {
	esdEP->GetQContributionXArray()->Set(esd->GetNumberOfTracks());
	esdEP->GetQContributionYArray()->Set(esd->GetNumberOfTracks());
        esdEP->GetQContributionXArraysub1()->Set(esd->GetNumberOfTracks());
	esdEP->GetQContributionYArraysub1()->Set(esd->GetNumberOfTracks());
        esdEP->GetQContributionXArraysub2()->Set(esd->GetNumberOfTracks());
	esdEP->GetQContributionYArraysub2()->Set(esd->GetNumberOfTracks());
      }
      
      TObjArray* tracklist = new TObjArray;
      if (fTrackType.CompareTo("GLOBAL")==0) tracklist = fESDtrackCuts->GetAcceptedTracks(esd,kFALSE);
      if (fTrackType.CompareTo("TPC")==0) tracklist = fESDtrackCuts->GetAcceptedTracks(esd,kTRUE);
      const int nt = tracklist->GetEntries();
      
      if (nt>4){
	fQVector = new TVector2(GetQ(esdEP,tracklist));
	fEventplaneQ = fQVector->Phi()/2; 
	GetQsub(qq1, qq2, tracklist, esdEP);
	fQsub1 = new TVector2(qq1);
	fQsub2 = new TVector2(qq2);
	fQsubRes = (fQsub1->Phi()/2 - fQsub2->Phi()/2);
	
	esdEP->SetQVector(fQVector);
	esdEP->SetEventplaneQ(fEventplaneQ);
	esdEP->SetQsub(fQsub1,fQsub2);
	esdEP->SetQsubRes(fQsubRes);
	
	fHOutEventplaneQ->Fill(fEventplaneQ);
	fHOutsub1sub2->Fill(fQsub1->Phi()/2,fQsub2->Phi()/2);
	fHOutNTEPRes->Fill(nt,fQsubRes);

	if (fUseMCRP) fHOutDiff->Fill(fEventplaneQ, fRP);
	
	for (int iter = 0; iter<nt;iter++){
	  IlcESDtrack* track = dynamic_cast<IlcESDtrack*> (tracklist->At(iter));
	  if (track) {
	    float delta = track->Phi()-fEventplaneQ;
	    while (delta < 0) delta += TMath::Pi();
	    while (delta > TMath::Pi()) delta -= TMath::Pi();
	    fHOutPTPsi->Fill(track->Pt(),delta);
	    fHOutPhi->Fill(track->Phi());
	    fHOutPhiCorr->Fill(track->Phi(),GetPhiWeight(track));
	  }
	}
	
	IlcESDtrack* trmax = esd->GetTrack(0);
	for (int iter = 1; iter<nt;iter++){
	  IlcESDtrack* track = dynamic_cast<IlcESDtrack*> (tracklist->At(iter));
	  if (track && (track->Pt() > trmax->Pt())) trmax = track;
	}
	fHOutleadPTPsi->Fill(trmax->Phi(),fEventplaneQ);      
      }
      delete tracklist;
      tracklist = 0;
    }
  }
  
    else if (fAnalysisInput.CompareTo("AOD")==0){
    IlcVEvent* event = InputEvent();
    IlcAODEvent* aod = dynamic_cast<IlcAODEvent*>(event);

    if (aod){
      if (!(fRunNumber == aod->GetRunNumber())) {
        fRunNumber = aod->GetRunNumber();
        SetPhiDist();      
      }

      if (fUseMCRP) {
	IlcAODMCHeader *headerH = dynamic_cast<IlcAODMCHeader*>(aod->GetList()->FindObject(IlcAODMCHeader::StdBranchName()));
	if (headerH) fRP = headerH->GetReactionPlaneAngle();
      }
  
      esdEP = aod->GetHeader()->GetEventplaneP();
      esdEP->Reset(); 
     
      Int_t maxID = 0;
      TObjArray* tracklist = GetAODTracksAndMaxID(aod,maxID);
	
      if (fSaveTrackContribution) {
	esdEP->GetQContributionXArray()->Set(maxID+1);
	esdEP->GetQContributionYArray()->Set(maxID+1);
	esdEP->GetQContributionXArraysub1()->Set(maxID+1);
	esdEP->GetQContributionYArraysub1()->Set(maxID+1);
	esdEP->GetQContributionXArraysub2()->Set(maxID+1);
	esdEP->GetQContributionYArraysub2()->Set(maxID+1);
      }
	
      const int NT = tracklist->GetEntries();
      
      if (NT>4){
	fQVector = new TVector2(GetQ(esdEP,tracklist));
	fEventplaneQ = fQVector->Phi()/2.; 
	GetQsub(qq1, qq2, tracklist, esdEP);
	fQsub1 = new TVector2(qq1);
	fQsub2 = new TVector2(qq2);
	fQsubRes = (fQsub1->Phi()/2. - fQsub2->Phi()/2.);
	
	esdEP->SetQVector(fQVector);
	esdEP->SetEventplaneQ(fEventplaneQ);
	esdEP->SetQsub(fQsub1,fQsub2);
	esdEP->SetQsubRes(fQsubRes);
	
	fHOutEventplaneQ->Fill(fEventplaneQ);
	fHOutsub1sub2->Fill(fQsub1->Phi()/2.,fQsub2->Phi()/2.);
	fHOutNTEPRes->Fill(NT,fQsubRes);
	
	if (fUseMCRP) fHOutDiff->Fill(fEventplaneQ, fRP);
	
	for (int iter = 0; iter<NT;iter++){
	  IlcAODTrack* track = dynamic_cast<IlcAODTrack*> (tracklist->At(iter));
	  if (track) {
	    float delta = track->Phi()-fEventplaneQ;
	    while (delta < 0) delta += TMath::Pi();
	    while (delta > TMath::Pi()) delta -= TMath::Pi();
	    fHOutPTPsi->Fill(track->Pt(),delta);
	    fHOutPhi->Fill(track->Phi());
	    fHOutPhiCorr->Fill(track->Phi(),GetPhiWeight(track));
	  }
	}
	
	IlcAODTrack* trmax = aod->GetTrack(0);
	for (int iter = 1; iter<NT;iter++){
	  IlcAODTrack* track = dynamic_cast<IlcAODTrack*> (tracklist->At(iter));
	  if (track && (track->Pt() > trmax->Pt())) trmax = track;
	}
	fHOutleadPTPsi->Fill(trmax->Phi(),fEventplaneQ);      
      }     
      delete tracklist;
      tracklist = 0;
    }	
	
    
  }  

  
  else {
    printf(" Analysis Input not known!\n\n ");
    return;
  }
  PostData(1, fOutputList); 
}

//________________________________________________________________________
void IlcEPSelectionTask::Terminate(Option_t */*option*/)
{
  // Terminate analysis
}

//__________________________________________________________________________
TVector2 IlcEPSelectionTask::GetQ(IlcEventplane* EP, TObjArray* tracklist)
{
// Get the Q vector
  TVector2 mQ;
  float mQx=0, mQy=0;
  IlcVTrack* track;
  Double_t weight;
  Int_t idtemp = -1;
  
  int nt = tracklist->GetEntries();

  for (int i=0; i<nt; i++){
    weight = 1;
    track = dynamic_cast<IlcVTrack*> (tracklist->At(i));
    if (track) {
      weight = GetWeight(track);
    if (fSaveTrackContribution){
      idtemp = track->GetID(); 
      if ((fAnalysisInput.CompareTo("AOD")==0) && (fAODfilterbit == 128)) idtemp = idtemp*(-1) - 1;
      EP->GetQContributionXArray()->AddAt(weight*cos(2*track->Phi()),idtemp);
      EP->GetQContributionYArray()->AddAt(weight*sin(2*track->Phi()),idtemp);
     }
     mQx += (weight*cos(2*track->Phi()));
     mQy += (weight*sin(2*track->Phi()));
    }
  }
  mQ.Set(mQx,mQy);
  return mQ;
}
  
  //________________________________________________________________________
void IlcEPSelectionTask::GetQsub(TVector2 &Q1, TVector2 &Q2, TObjArray* tracklist,IlcEventplane* EP)
{
// Get Qsub
  TVector2 mQ[2];
  float mQx1=0, mQy1=0, mQx2=0, mQy2=0;
  Double_t weight;

  IlcVTrack* track;
  TRandom2 rn = 0;
  
  int nt = tracklist->GetEntries();
  int trackcounter1=0, trackcounter2=0;
  int idtemp = 0;

  if (fSplitMethod == IlcEPSelectionTask::kRandom){
    
    for (Int_t i = 0; i < nt; i++) {
      weight = 1;
      track = dynamic_cast<IlcVTrack*> (tracklist->At(i));
      if (!track) continue;
      weight = GetWeight(track);
      idtemp = track->GetID(); 
      if ((fAnalysisInput.CompareTo("AOD")==0) && (fAODfilterbit == 128)) idtemp = idtemp*(-1) - 1;
    
      // This loop splits the track set into 2 random subsets
      if( trackcounter1 < int(nt/2.) && trackcounter2 < int(nt/2.)){
        float random = rn.Rndm();
        if(random < .5){
          mQx1 += (weight*cos(2*track->Phi()));
          mQy1 += (weight*sin(2*track->Phi()));
          if (fSaveTrackContribution){
            EP->GetQContributionXArraysub1()->AddAt(weight*cos(2*track->Phi()),idtemp);
            EP->GetQContributionYArraysub1()->AddAt(weight*sin(2*track->Phi()),idtemp);
          }
          trackcounter1++;
        }
        else {
          mQx2 += (weight*cos(2*track->Phi()));
          mQy2 += (weight*sin(2*track->Phi()));
          if (fSaveTrackContribution){
            EP->GetQContributionXArraysub2()->AddAt(weight*cos(2*track->Phi()),idtemp);
            EP->GetQContributionYArraysub2()->AddAt(weight*sin(2*track->Phi()),idtemp);
          }
          trackcounter2++;
        }
      }
      else if( trackcounter1 >= int(nt/2.)){
        mQx2 += (weight*cos(2*track->Phi()));
        mQy2 += (weight*sin(2*track->Phi()));
        if (fSaveTrackContribution){
          EP->GetQContributionXArraysub2()->AddAt(weight*cos(2*track->Phi()),idtemp);
          EP->GetQContributionYArraysub2()->AddAt(weight*sin(2*track->Phi()),idtemp);
        }
        trackcounter2++;
      }
      else {
        mQx1 += (weight*cos(2*track->Phi()));
        mQy1 += (weight*sin(2*track->Phi()));
        if (fSaveTrackContribution){
          EP->GetQContributionXArraysub1()->AddAt(weight*cos(2*track->Phi()),idtemp);
          EP->GetQContributionYArraysub1()->AddAt(weight*sin(2*track->Phi()),idtemp);
        }
        trackcounter1++;
      }
    }
  } else if (fSplitMethod == IlcEPSelectionTask::kEta) {
     
    for (Int_t i = 0; i < nt; i++) {
      weight = 1;
      track = dynamic_cast<IlcVTrack*> (tracklist->At(i));
      if (!track) continue;
      weight = GetWeight(track);
      Double_t eta = track->Eta();
      idtemp = track->GetID(); 
      if ((fAnalysisInput.CompareTo("AOD")==0) && (fAODfilterbit == 128)) idtemp = idtemp*(-1) - 1;

      if (eta > fEtaGap/2.) {  
        mQx1 += (weight*cos(2*track->Phi()));
        mQy1 += (weight*sin(2*track->Phi()));
        if (fSaveTrackContribution){
          EP->GetQContributionXArraysub1()->AddAt(weight*cos(2*track->Phi()),idtemp);
          EP->GetQContributionYArraysub1()->AddAt(weight*sin(2*track->Phi()),idtemp);
        }
      } else if (eta < -1.*fEtaGap/2.) {
        mQx2 += (weight*cos(2*track->Phi()));
        mQy2 += (weight*sin(2*track->Phi()));
        if (fSaveTrackContribution){
          EP->GetQContributionXArraysub2()->AddAt(weight*cos(2*track->Phi()),idtemp);
          EP->GetQContributionYArraysub2()->AddAt(weight*sin(2*track->Phi()),idtemp);
        }
      }
    }
  } else {
    printf("plane resolution determination method not available!\n\n ");
    return;
  }
     
  mQ[0].Set(mQx1,mQy1);
  mQ[1].Set(mQx2,mQy2);
  Q1 = mQ[0];
  Q2 = mQ[1];
}

//________________________________________________________________________
void IlcEPSelectionTask::SetPersonalESDtrackCuts(IlcESDtrackCuts* trackcuts){
  
  if(fESDtrackCuts){ 
    delete fESDtrackCuts;
    fESDtrackCuts = 0;
  }
  if (fAnalysisInput.CompareTo("AOD")==0){
    IlcInfo("ESD track cuts not possible for AOD analysis; please use SetPersonalAODtrackCuts(); using TPC only track cuts");  
    fUsercuts = kFALSE;
    SetTrackType("TPC");
    return;
  } 
  fUsercuts = kTRUE;
  fESDtrackCuts = trackcuts;
}

//________________________________________________________________________
void IlcEPSelectionTask::SetPersonalAODtrackCuts(UInt_t filterbit, Float_t etalow, Float_t etaup, Float_t ptlow, Float_t ptup){
  
  if(fESDtrackCuts){ 
    delete fESDtrackCuts;
    fESDtrackCuts = 0;
  }
  if (fAnalysisInput.CompareTo("ESD")==0){
    IlcInfo("AOD track cuts not possible for ESD analysis; please use SetPersonalESDtrackCuts(); using TPC only track cuts");  
    fUsercuts = kFALSE;
    SetTrackType("TPC");
    return;
  }
  fUsercuts = kTRUE;
  fESDtrackCuts = new IlcESDtrackCuts();
  fESDtrackCuts->SetPtRange(ptlow,ptup);
  fESDtrackCuts->SetEtaRange(etalow,etaup);
  fAODfilterbit = filterbit;
}

//_____________________________________________________________________________

void IlcEPSelectionTask::SetTrackType(TString tracktype){
  fTrackType = tracktype;
  if (!fUsercuts) {
  if (fTrackType.CompareTo("GLOBAL")==0){ 
    fESDtrackCuts = IlcESDtrackCuts::GetStandardITSTPCTrackCuts2010(kTRUE);
    fAODfilterbit = 32;
  }	
  if (fTrackType.CompareTo("TPC")==0){  
    fESDtrackCuts = IlcESDtrackCuts::GetStandardTPCOnlyTrackCuts();
    fAODfilterbit = 128;
  }
  fESDtrackCuts->SetPtRange(0.15,20.);
  fESDtrackCuts->SetEtaRange(-0.8,0.8);
  }
}

//________________________________________________________________________
Double_t IlcEPSelectionTask::GetWeight(TObject* track1)
{
  Double_t ptweight=1;
  IlcVTrack* track = dynamic_cast<IlcVTrack*>(track1);
  if (fUsePtWeight && track) {      
    if (track->Pt()<2) ptweight=track->Pt();
    else ptweight=2;
  }
  return ptweight*GetPhiWeight(track);
}

//________________________________________________________________________
Double_t IlcEPSelectionTask::GetPhiWeight(TObject* track1)
{
  Double_t phiweight=1;
  IlcVTrack* track = dynamic_cast<IlcVTrack*>(track1);
  
  if (fUsePhiWeight && fPhiDist && track) {
    Double_t nParticles = fPhiDist->Integral();
    Double_t nPhibins = fPhiDist->GetNbinsX();
  
    Double_t Phi = track->Phi();
    
    while (Phi<0) Phi += TMath::TwoPi();
    while (Phi>TMath::TwoPi()) Phi -= TMath::TwoPi();
      
    Double_t PhiDistValue = fPhiDist->GetBinContent(1+TMath::FloorNint((track->Phi())*nPhibins/TMath::TwoPi()));
    
    if (PhiDistValue > 0) phiweight = nParticles/nPhibins/PhiDistValue;
  }
  return phiweight;
}

//__________________________________________________________________________
void IlcEPSelectionTask::SetPhiDist() 
{
  if(!fUserphidist) { // if it's already set and custom class is required, we use the one provided by the user

    fPhiDist = (TH1F*) fEPContainer->GetObject(fRunNumber, "Default");
    if (!fPhiDist) IlcFatal(Form("Cannot find OADB phi distribution for run %d", fRunNumber));

  } 
  else {
    IlcInfo("Using Custom Phi Distribution");
  }
    
  Bool_t emptybins;

  int iter = 0;  
  while (iter<3){
      emptybins = kFALSE;
   
      for (int i=1; i<fPhiDist->GetNbinsX(); i++){
	if (!((fPhiDist->GetBinContent(i))>0)) {
	  emptybins = kTRUE;
	}
      }  
      if (emptybins) {
	cout << "empty bins - rebinning!" << endl;
	fPhiDist->Rebin();
	iter++;
      }      
      else iter = 3;
  }
  
  if (emptybins) {
    IlcError("After Maximum of rebinning still empty Phi-bins!!!");
  }
}

//__________________________________________________________________________
void IlcEPSelectionTask::SetPersonalPhiDistribution(const char* infilename, char* listname)
{
  
  fUserphidist = kTRUE;
  
  TFile f(infilename);
  TObject* list = f.Get(listname);
  fPhiDist = (TH1F*)list->FindObject("fHOutPhi");
  if (!fPhiDist) IlcFatal("Phi Distribution not found!!!");

  f.Close();
} 


//_________________________________________________________________________
TObjArray* IlcEPSelectionTask::GetAODTracksAndMaxID(IlcAODEvent* aod, Int_t& maxid)
{
  TObjArray *acctracks = new TObjArray();
  
  IlcAODTrack *tr = 0;
  Int_t maxid1 = 0;
  Int_t maxidtemp = -1;
  Float_t ptlow = 0;
  Float_t ptup = 0;
  Float_t etalow = 0;
  Float_t etaup = 0;
  fESDtrackCuts->GetPtRange(ptlow,ptup);
  fESDtrackCuts->GetEtaRange(etalow,etaup);
  
  for (Int_t i = 0; i < aod->GetNumberOfTracks() ; i++){
     tr = aod->GetTrack(i);
     maxidtemp = tr->GetID(); 
     if(maxidtemp < 0 && fAODfilterbit != 128) continue;
     if(maxidtemp > -1 && fAODfilterbit == 128) continue;
     if (fAODfilterbit == 128) maxidtemp = maxidtemp*(-1) - 1;
     if (maxidtemp > maxid1) maxid1 = maxidtemp;
     if(tr->TestFilterBit(fAODfilterbit) && tr->Pt() < ptup && tr->Pt() > ptlow && tr->Eta() < etaup && tr->Eta() > etalow){
     acctracks->Add(tr);
     }
  }
  
  maxid = maxid1;
  
  return acctracks;
  
}
