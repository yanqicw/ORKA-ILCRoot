
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

//-----------------------------------------------------------------------
// Author : R. Vernet, Consorzio Cometa - Catania (it)
//-----------------------------------------------------------------------


#include "IlcCFRsnTask.h"
#include "TCanvas.h"
#include "IlcStack.h"
#include "TParticle.h"
#include "TH1I.h"
#include "TChain.h"
#include "IlcMCEvent.h"
#include "IlcESDEvent.h"
#include "IlcCFManager.h"
#include "IlcCFCutBase.h"
#include "IlcCFContainer.h"
#include "IlcESDtrack.h"
#include "IlcLog.h"
#include "IlcRsnDaughter.h"
#include "IlcCFPair.h"
#include "IlcRsnMCInfo.h"
#include "IlcRsnPairParticle.h"
#include "IlcAODEvent.h"
#include "IlcRsnReader.h"

//__________________________________________________________________________
IlcCFRsnTask::IlcCFRsnTask() :
  IlcAnalysisTaskSE(),
  fRsnPDG(0),
  fCFManager(0x0),
  fHistEventsProcessed(0x0)
{
  //
  //Default ctor
  //
}
//___________________________________________________________________________
IlcCFRsnTask::IlcCFRsnTask(const Char_t* name) :
  IlcAnalysisTaskSE(name),
  fRsnPDG(0),
  fCFManager(0x0),
  fHistEventsProcessed(0x0)
{
  //
  // Constructor. Initialization of Inputs and Outputs
  //
  Info("IlcCFRsnTask","Calling Constructor");
  /*
    DefineInput(0) and DefineOutput(0)
    are taken care of by IlcAnalysisTaskSE constructor
  */
  DefineOutput(1,TH1I::Class());
  DefineOutput(2,IlcCFContainer::Class());
  //   DefineOutput(3,TList::Class());
}

//___________________________________________________________________________
IlcCFRsnTask& IlcCFRsnTask::operator=(const IlcCFRsnTask& c) 
{
  //
  // Assignment operator
  //
  if (this!=&c) {
    IlcAnalysisTaskSE::operator=(c) ;
    fRsnPDG     = c.fRsnPDG;
    fCFManager  = c.fCFManager;
    fHistEventsProcessed = c.fHistEventsProcessed;
  }
  return *this;
}

//___________________________________________________________________________
IlcCFRsnTask::IlcCFRsnTask(const IlcCFRsnTask& c) :
  IlcAnalysisTaskSE(c),
  fRsnPDG(c.fRsnPDG),
  fCFManager(c.fCFManager),
  fHistEventsProcessed(c.fHistEventsProcessed)
{
  //
  // Copy Constructor
  //
}

//___________________________________________________________________________
IlcCFRsnTask::~IlcCFRsnTask() {
  //
  //destructor
  //
  Info("~IlcCFRsnTask","Calling Destructor");
  if (fCFManager)           delete fCFManager ;
  if (fHistEventsProcessed) delete fHistEventsProcessed ;
}

//_________________________________________________
void IlcCFRsnTask::UserExec(Option_t *)
{
  //
  // Main loop function
  //
  Info("UserExec","") ;

  if (!fInputEvent) {
    Error("UserExec","NO EVENT FOUND!");
    return;
  }

  if (!fMCEvent) Error("UserExec","NO MC INFO FOUND!");
  fCFManager->SetEventInfo(fMCEvent);

  IlcStack*   stack   = fMCEvent->Stack();

  Bool_t isESDEvent = strcmp(fInputEvent->ClassName(),"IlcESDEvent") == 0 ? kTRUE : kFALSE ;
  Bool_t isAODEvent = strcmp(fInputEvent->ClassName(),"IlcAODEvent") == 0 ? kTRUE : kFALSE ;

  // MC-event selection
  Double_t containerInput[2] ;
        
  //loop on the MC event
  Info("UserExec","Looping on MC event");
  for (Int_t ipart=0; ipart<stack->GetNprimary(); ipart++) { 
    IlcMCParticle *mcPart  = fMCEvent->GetTrack(ipart);

    //check the MC-level cuts
    if (!fCFManager->CheckParticleCuts(IlcCFManager::kPartGenCuts,mcPart)) continue;
    containerInput[0] = mcPart->Pt();
    containerInput[1] = mcPart->Y() ;
    //fill the container for Gen-level selection
    fCFManager->GetParticleContainer()->Fill(containerInput,kStepGenerated);
    
    //check the Acceptance-level cuts
    if (!fCFManager->CheckParticleCuts(IlcCFManager::kPartAccCuts,mcPart)) continue;
    //fill the container for Acceptance-level selection
    fCFManager->GetParticleContainer()->Fill(containerInput,kStepReconstructible);
  }    


  //Now go to rec level
  Info("UserExec","Looping on %s",fInputEvent->ClassName());
  
  // Loop on negative tracks
  for (Int_t iTrack1 = 0; iTrack1<fInputEvent->GetNumberOfTracks(); iTrack1++) {
    IlcVParticle* track1 = fInputEvent->GetTrack(iTrack1);
    //track1 is negative
    if (track1->Charge()>=0) continue;
    Int_t label1 = track1->GetLabel();
    if (label1<0) continue;

    //Loop on positive tracks
    for (Int_t iTrack2 = 0; iTrack2<fInputEvent->GetNumberOfTracks(); iTrack2++) {
      IlcVParticle* track2 = fInputEvent->GetTrack(iTrack2);
      //track2 is positive
      if (track2->Charge()<=0) continue;
      Int_t label2 = track2->GetLabel();
      if (label2<0) continue;

      IlcRsnDaughter daughter1;
      IlcRsnDaughter daughter2;

      IlcRsnReader reader ; //needed for conversion ESD/AOD->IlcRsnDaughter

      if (isESDEvent) {
	if (!reader.ConvertTrack(&daughter1,(IlcESDtrack*)track1)) Error("UserExec","Could not convert ESD track") ;
	if (!reader.ConvertTrack(&daughter2,(IlcESDtrack*)track2)) Error("UserExec","Could not convert ESD track") ;
      }
      else if (isAODEvent) {
	if (!reader.ConvertTrack(&daughter1,(IlcAODTrack*)track1)) Error("UserExec","Could not convert AOD track") ;
	if (!reader.ConvertTrack(&daughter2,(IlcAODTrack*)track2)) Error("UserExec","Could not convert AOD track") ;
      }
      else {
	Error("UserExec","Error: input data file is not an ESD nor an AOD");
	return;
      }

      IlcCFPair pair(track1,track2); // This object is used for cuts 
                                     // (to be replaced when IlcRsnPairParticle 
                                     // inherits from IlcVParticle)

      //Set MC PDG information to resonance daughters
      TParticle *part1 = stack->Particle(label1);
      daughter1.InitMCInfo(part1);
      daughter1.GetMCInfo()->SetPDG(part1->GetPdgCode());
      daughter1.SetM(part1->GetCalcMass()); // assign true mass

      Int_t mother1 = part1->GetFirstMother();
      daughter1.GetMCInfo()->SetMother(mother1);
      if (mother1 >= 0) {
	TParticle *mum = stack->Particle(mother1);
	daughter1.GetMCInfo()->SetMotherPDG(mum->GetPdgCode());
      }

      TParticle *part2 = stack->Particle(label2);
      daughter2.InitMCInfo(part2);
      daughter2.GetMCInfo()->SetPDG(part2->GetPdgCode());
      daughter2.SetM(part2->GetCalcMass()); // assign true mass

      Int_t mother2 = part2->GetFirstMother();
      daughter2.GetMCInfo()->SetMother(mother2);
      if (mother2 >= 0) {
	TParticle *mum = stack->Particle(mother2);
	daughter2.GetMCInfo()->SetMotherPDG(mum->GetPdgCode());
      }
	
      //make a mother resonance from the 2 candidate daughters
      IlcRsnPairParticle rsn;
      rsn.SetPair(&daughter1,&daughter2);

      //check if true resonance
      if (!rsn.IsTruePair(fRsnPDG)) continue;
      if (!fCFManager->CheckParticleCuts(IlcCFManager::kPartRecCuts,&pair)) continue;

      //check if associated MC resonance passes the cuts
      Int_t motherLabel = rsn.GetDaughter(0)->GetMCInfo()->Mother() ;
      if (motherLabel<0) continue ;

      IlcMCParticle* mcRsn = fMCEvent->GetTrack(motherLabel);
      if (!mcRsn) continue;
      if (!fCFManager->CheckParticleCuts(IlcCFManager::kPartGenCuts,mcRsn)) continue; 

      // fill the container
      Double_t rsnEnergy = rsn.GetDaughter(0)->E()  + rsn.GetDaughter(1)->E()  ;
      Double_t rsnPz     = rsn.GetDaughter(0)->Pz() + rsn.GetDaughter(1)->Pz() ;

      containerInput[0] = rsn.GetPt() ;
      containerInput[1] = GetRapidity(rsnEnergy,rsnPz);
      fCFManager->GetParticleContainer()->Fill(containerInput,kStepReconstructed) ;   

      if (!fCFManager->CheckParticleCuts(IlcCFManager::kPartSelCuts,&pair)) continue ;
      fCFManager->GetParticleContainer()->Fill(containerInput,kStepSelected);
    }
  }
    
  fHistEventsProcessed->Fill(0);
  /* PostData(0) is taken care of by IlcAnalysisTaskSE */
  PostData(1,fHistEventsProcessed) ;
  PostData(2,fCFManager->GetParticleContainer()) ;
  
  //   TList * list = new TList();
  //   fCFManager->AddQAHistosToList(list);
  //   PostData(2,list) ;
}


//___________________________________________________________________________
void IlcCFRsnTask::Terminate(Option_t*)
{
  // The Terminate() function is the last function to be called during
  // a query. It always runs on the client, it can be used to present
  // the results graphically or save the results to file.

  Info("Terminate","");
  IlcAnalysisTaskSE::Terminate();


  //draw some example plots....

  IlcCFContainer *cont= dynamic_cast<IlcCFContainer*> (GetOutputData(2));

  TH1D* h00 =   cont->ShowProjection(0,0) ;
  TH1D* h01 =   cont->ShowProjection(0,1) ;
  TH1D* h02 =   cont->ShowProjection(0,2) ;
  TH1D* h03 =   cont->ShowProjection(0,3) ;

  TH1D* h10 =   cont->ShowProjection(1,0) ;
  TH1D* h11 =   cont->ShowProjection(1,1) ;
  TH1D* h12 =   cont->ShowProjection(1,2) ;
  TH1D* h13 =   cont->ShowProjection(1,3) ;

  Double_t max1 = h00->GetMaximum();
  Double_t max2 = h10->GetMaximum();

  h00->GetYaxis()->SetRangeUser(0,max1*1.2);
  h01->GetYaxis()->SetRangeUser(0,max1*1.2);
  h02->GetYaxis()->SetRangeUser(0,max1*1.2);
  h03->GetYaxis()->SetRangeUser(0,max1*1.2);

  h10->GetYaxis()->SetRangeUser(0,max2*1.2);
  h11->GetYaxis()->SetRangeUser(0,max2*1.2);
  h12->GetYaxis()->SetRangeUser(0,max2*1.2);
  h13->GetYaxis()->SetRangeUser(0,max2*1.2);

  h00->SetMarkerStyle(23) ;
  h01->SetMarkerStyle(24) ;
  h02->SetMarkerStyle(25) ;
  h03->SetMarkerStyle(26) ;

  h10->SetMarkerStyle(23) ;
  h11->SetMarkerStyle(24) ;
  h12->SetMarkerStyle(25) ;
  h13->SetMarkerStyle(26) ;

  TCanvas * c =new TCanvas("c","",1400,800);
  c->Divide(4,2);

  c->cd(1);
  h00->Draw("p");
  c->cd(2);
  h01->Draw("p");
  c->cd(3);
  h02->Draw("p");
  c->cd(4);
  h03->Draw("p");
  c->cd(5);
  h10->Draw("p");
  c->cd(6);
  h11->Draw("p");
  c->cd(7);
  h12->Draw("p");
  c->cd(8);
  h13->Draw("p");

  c->SaveAs("plots.eps");
}

//___________________________________________________________________________
void IlcCFRsnTask::UserCreateOutputObjects() {
  //HERE ONE CAN CREATE OUTPUT OBJECTS, IN PARTICULAR IF THE OBJECT PARAMETERS DON'T NEED
  //TO BE SET BEFORE THE EXECUTION OF THE TASK
  //
  Info("UserCreateOutputObjects","CreateOutputObjects of task %s\n", GetName());

  //slot #1
  OpenFile(1);
  fHistEventsProcessed = new TH1I("fHistEventsProcessed","",1,0,1) ;
}

//___________________________________________________________________________
Double_t IlcCFRsnTask::GetRapidity(Double_t energy, Double_t pz) {
  if (energy == pz || energy == -pz) {
    printf("GetRapidity : ERROR : rapidity for 4-vector with E = Pz -- infinite result");
    return 999;
  }
  if (energy < pz) {
    printf("GetRapidity : ERROR : rapidity for 4-vector with E = Pz -- infinite result");
    return 999;
  }
  Double_t y = 0.5 * log((energy + pz) / (energy - pz));
  return y;
} 


