// Example: generation of kinematics tree with selected properties.
// Below we select events containing the decays D* -> D0 pi, D0 -> K- pi+
// inside the barrel part of the ILC detector (45 < theta < 135)

#if !defined(__CINT__) || defined(__MAKECINT__)
#include <Riostream.h>
#include <TH1F.h>
#include <TStopwatch.h>
#include <TDatime.h>
#include <TRandom.h>
#include <TDatabasePDG.h>
#include <TParticle.h>
#include <TArrayI.h>
#include <TTree.h>

#include "IlcGenerator.h"
#include "IlcPDG.h"
#include "IlcRunLoader.h"
#include "IlcRun.h"
#include "IlcStack.h"
#include "IlcHeader.h"
#include "PYTHIA6/IlcGenPythia.h"
#include "PYTHIA6/IlcPythia.h"
#endif

Float_t EtaToTheta(Float_t arg);
void GetFinalDecayProducts(Int_t ind, IlcStack & stack , TArrayI & ar);

void fastGen(Int_t nev = 1, char* filename = "gilc.root")
{
  IlcPDG::AddParticlesToPdgDataBase();
  TDatabasePDG::Instance();
 


  // Run loader
  IlcRunLoader* rl = IlcRunLoader::Open("gilc.root","FASTRUN","recreate");
  
  rl->SetCompressionLevel(2);
  rl->SetNumberOfEventsPerFile(nev);
  rl->LoadKinematics("RECREATE");
  rl->MakeTree("E");
  gIlc->SetRunLoader(rl);
  
  //  Create stack
  rl->MakeStack();
  IlcStack* stack = rl->Stack();
  
  //  Header
  IlcHeader* header = rl->GetHeader();
  
  //  Create and Initialize Generator
 
  // Example of charm generation taken from Config_PythiaHeavyFlavours.C
  IlcGenPythia *gener = new IlcGenPythia(-1);
  gener->SetEnergyCMS(14000.);
  gener->SetMomentumRange(0,999999);
  gener->SetPhiRange(0., 360.);
  gener->SetThetaRange(0.,180.);
  //  gener->SetProcess(kPyCharmppMNR); // Correct Pt distribution, wrong mult
  gener->SetProcess(kPyMb); // Correct multiplicity, wrong Pt
  gener->SetStrucFunc(kCTEQ4L);
  gener->SetPtHard(2.1,-1.0);
  gener->SetFeedDownHigherFamily(kFALSE);
  gener->SetStack(stack);
  gener->Init();

  // Go to gilc.root
  rl->CdGAFile();

  // Forbid some decays. Do it after gener->Init(0, because
  // the initialization of the generator includes reading of the decay table.

  IlcPythia * py= IlcPythia::Instance();
  py->SetMDME(737,1,0); //forbid D*+->D+ + pi0
  py->SetMDME(738,1,0);//forbid D*+->D+ + gamma

  // Forbid all D0 decays except D0->K- pi+
  for(Int_t d=747; d<=762; d++){ 
    py->SetMDME(d,1,0);
  }
  // decay 763 is D0->K- pi+
  for(Int_t d=764; d<=807; d++){ 
    py->SetMDME(d,1,0);
  }

  //
  //                        Event Loop
  //
  
  TStopwatch timer;
  timer.Start();
  for (Int_t iev = 0; iev < nev; iev++) {
    
    cout <<"Event number "<< iev << endl;
    
    //  Initialize event
    header->Reset(0,iev);
    rl->SetEventNumber(iev);
    stack->Reset();
    rl->MakeTree("K");
    
    //  Generate event
    Int_t nprim = 0;
    Int_t ntrial = 0;
    Int_t ndstar = 0;
   
   

    //-------------------------------------------------------------------------------------

    while(!ndstar) {
      // Selection of events with D*
      stack->Reset();
      stack->ConnectTree(rl->TreeK());
      gener->Generate();
      ntrial++;
      nprim = stack->GetNprimary();
      
      for(Int_t ipart =0; ipart < nprim; ipart++){
        TParticle * part = stack->Particle(ipart);
        if(part)    {
          
          if (TMath::Abs(part->GetPdgCode())== 413) {

	    TArrayI daughtersId;

	    GetFinalDecayProducts(ipart,*stack,daughtersId);

	    Bool_t kineOK = kTRUE;

	    Double_t thetaMin = TMath::Pi()/4;
	    Double_t thetaMax = 3*TMath::Pi()/4;

	    for (Int_t id=1; id<=daughtersId[0]; id++) {
	      TParticle * daughter = stack->Particle(daughtersId[id]);
	      if (!daughter) {
		kineOK = kFALSE;
		break;
	      }

	      Double_t theta = daughter->Theta();
	      if (theta<thetaMin || theta>thetaMax) {
		kineOK = kFALSE;
		break;
	      }
	    }

	    if (!kineOK) continue;

            part->Print();
            ndstar++;     
	    
          }
        }
      }   
    }   
      
    cout << "Number of particles " << nprim << endl;
    cout << "Number of trials " << ntrial << endl;
    
    //  Finish event
    header->SetNprimary(stack->GetNprimary());
    header->SetNtrack(stack->GetNtrack());  
    
    //      I/O
    stack->FinishEvent();
    header->SetStack(stack);
    rl->TreeE()->Fill();
    rl->WriteKinematics("OVERWRITE");
    
  } // event loop
  timer.Stop();
  timer.Print();
  
  //                         Termination
  //  Generator
  gener->FinishRun();
  //  Write file
  rl->WriteHeader("OVERWRITE");
  gener->Write();
  rl->Write();
}



Float_t EtaToTheta(Float_t arg){
  return (180./TMath::Pi())*2.*atan(exp(-arg));
}


void GetFinalDecayProducts(Int_t ind, IlcStack & stack , TArrayI & ar){

  // Recursive algorithm to get the final decay products of a particle
  //
  // ind is the index of the particle in the IlcStack
  // stack is the particle stack from the generator
  // ar contains the indexes of the final decay products
  // ar[0] is the number of final decay products

  if (ind<0 || ind>stack.GetNtrack()) {
    cerr << "Invalid index of the particle " << ind << endl;
    return;
  } 
  if (ar.GetSize()==0) {
    ar.Set(10);
    ar[0] = 0;
  }

  TParticle * part = stack.Particle(ind);

  Int_t iFirstDaughter = part->GetFirstDaughter();
  if( iFirstDaughter<0) {
    // This particle is a final decay product, add its index to the array
    ar[0]++;
    if (ar.GetSize() <= ar[0]) ar.Set(ar.GetSize()+10); // resize if needed
    ar[ar[0]] = ind;
    return;
  } 

  Int_t iLastDaughter = part->GetLastDaughter();

  for (Int_t id=iFirstDaughter; id<=iLastDaughter;id++) {
    // Now search for final decay products of the daughters
    GetFinalDecayProducts(id,stack,ar);
  }
}
