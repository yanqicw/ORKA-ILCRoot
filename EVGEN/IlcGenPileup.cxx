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

//-------------------------------------------------------------------------
//                          Class IlcGenPileup
//   This is a generator of beam-beam pileup.
//   It generates interactions within 3 orbits (+-1) around
//   the trigger event. The trigger event itself is chosen
//   randomly among the bunch crossings within the central orbit.
//   The user can decide whenever to include in the simulation the
//   "trigger" interaction or not. This is handled by the
//   GenerateTrigInteraction(Bool_t flag) method.
//   In the case the trigger interaction is included, it is
//   generated using the same settings (vertex smear for example) as
//   the pileup events.
//   In case the trigger simulation is not included, the user can make
//   a cocktail of generator used to produce the trigger interaction and
//   IlcGenPileup. In this case in order to avoid a fake increase of the rate around the
//   trigger, the number of background events within the bunch
//   crossing of the trigger is readuced by one.
//   The beam profile (the list of the active bunch crossings) can be
//   controlled via the SetBCMask(const char *mask) method. The syntax
//   follows the one in IlcTriggerBCMask class. For example:
//   "3564H" would mean that all the bunch corssings within the orbit
//   are aloowed (which is of course unphysical). In case one wants to simulate
//   one-bunch-crossing-per-orbit scenario, the way to do it is to put something like:
//   "1H3563L" or similar.
//   The SetGenerator(IlcGenerator *generator, Float_t rate) method is
//   used in order to define the generator to be used. The second argument is the pileup
//   rate in terms of #_of_interactions/bunch-crossing = sigma_tot * luminosity.
//   The pileup generation time window can be set via
//   IlcGenerator::SetPileUpTimeWindow(Float_t pileUpTimeW) method. By the default the
//   window is set to 88micros (= TPC readout window).
//      
// cvetan.cheshkov@cern.ch  9/12/2008
//-------------------------------------------------------------------------

#include <TParticle.h>
#include <TFormula.h>

#include "IlcGenPileup.h"
#include "IlcLog.h"
#include "IlcGenCocktailEventHeader.h"
#include "IlcGenCocktailEntry.h"
#include "IlcRun.h"
#include "IlcStack.h"

ClassImp(IlcGenPileup)

IlcGenPileup::IlcGenPileup():
  IlcGenCocktail(),
  fBCMask("bcm","3564H"),
  fGenTrig(kFALSE),
  fFlag(kFALSE)
{
// Constructor
// The pileup time window is by default
// set to the TPC readout one
    fName = "Pileup";
    fTitle= "Beam-beam pileup";

    fPileUpTimeWindow = 88e-6;
}

IlcGenPileup::~IlcGenPileup()
{
// Destructor
}

void IlcGenPileup::SetGenerator(IlcGenerator *generator, Float_t rate, Bool_t flag)
{
  // The method sets the geenrator to be used
  // for pileup simulation.
  // The second argument is the pileup rate in terms of
  // #_of_interactions/bunch-crossing = sigma_tot * luminosity.
  // There is a protection in case the generator was already set.
  if (fEntries) {
    if (FirstGenerator()) {
      IlcError("Pileup generator has been already set! Nothing done");
      return;
    }
  }
  AddGenerator(generator,"pileup generator",rate);
  fFlag = flag;
}

void IlcGenPileup::AddGenerator(IlcGenerator *Generator,
				const char* Name,
				Float_t RateExp , TFormula* /*form*/)
{
  // The method used to add the pileup generator
  // in the cocktail list.
  // The method is protected in order to avoid
  // its misusage
  IlcGenCocktail::AddGenerator(Generator,Name,RateExp);
}

Bool_t IlcGenPileup::SetBCMask(const char *mask)
{
  // Set the active bunch-crossings that
  // will be included in the pileup
  // simulation. For more details on the
  // syntax of the mask - see
  // STEER/IlcTriggerBCMask.* and the comments
  // in the header of this file
  return fBCMask.SetMask(mask);
}

void IlcGenPileup::Generate()
{
  //
  // Generate pileup event 
  // For details see the coments inline

  // Check that the pileup generator is correctly set
  IlcGenCocktailEntry *entry = FirstGenerator();
  if (!entry) {
    IlcFatal("No pileup generator entry is found!");
  }

  IlcGenerator *gen = entry->Generator();
  if (!gen) {
    IlcFatal("No pileup generator specified!");
  }
  else if (gen->NeedsCollisionGeometry()) {
    IlcFatal("No Collision Geometry Provided");
  }

  // Check that the pileup rate is correctly set
  Float_t rate = entry->Rate();
  if (rate <= 0) {
    IlcFatal(Form("Invalid rate value: %f",rate));
  }

  // Create cocktail header
  if (fHeader) delete fHeader;
  fHeader = new IlcGenCocktailEventHeader("Pileup Cocktail Header");

  // Generate time of all
  // the collisions within one orbit
  Int_t *nIntBC = new Int_t[3*IlcTriggerBCMask::kNBits];
  Int_t *indexBC = new Int_t[3*IlcTriggerBCMask::kNBits];
  Int_t nTotBC = 0;
  while (nTotBC == 0) {
    for(Int_t iBC = 0; iBC <  IlcTriggerBCMask::kNBits; iBC++) {

      if (!fBCMask.GetMask(iBC)) continue;

      //      Int_t nInteractions = gRandom->Poisson(rate);
      Int_t nInteractions;
      if (!fFlag) 
	nInteractions = gRandom->Poisson(rate);
      else 
	nInteractions = TMath::Nint(rate) + 1;

      if (nInteractions == 0) continue;

      nIntBC[nTotBC] = nInteractions;
      indexBC[nTotBC] = iBC;
      nTotBC++;
    }
  }

  // Select the bunch crossing for triggered event
  Int_t iTrgBC = gRandom->Integer(nTotBC);
  // Subtract one from the number of events
  // generated within this bc (only in case
  // the user disabled the generation of the trigger
  // interaction)
  if (!fGenTrig) nIntBC[iTrgBC]--;

  // Remove bunch crossings outside pileup
  // time window
  for(Int_t iBC = 0; iBC <  nTotBC; iBC++) {
   if (TMath::Abs(25e-9*(indexBC[iBC]-indexBC[iTrgBC])) > fPileUpTimeWindow)
     nIntBC[iBC] = 0;
  }

  // Generate the two orbits around the central one
  // taking into account the pileup time window
  for(Int_t iBC = 0; iBC <  IlcTriggerBCMask::kNBits; iBC++) {

    if (!fBCMask.GetMask(iBC)) continue;

    if (TMath::Abs(25e-9*(iBC-IlcTriggerBCMask::kNBits-indexBC[iTrgBC])) > fPileUpTimeWindow) continue;

    Int_t nInteractions = gRandom->Poisson(rate);
    if (nInteractions == 0) continue;

    nIntBC[nTotBC] = nInteractions;
    indexBC[nTotBC] = iBC-IlcTriggerBCMask::kNBits;
    nTotBC++;
  }
  for(Int_t iBC = 0; iBC <  IlcTriggerBCMask::kNBits; iBC++) {

    if (!fBCMask.GetMask(iBC)) continue;

    if (TMath::Abs(25e-9*(iBC+IlcTriggerBCMask::kNBits-indexBC[iTrgBC])) > fPileUpTimeWindow) continue;

    Int_t nInteractions = gRandom->Poisson(rate);
    if (nInteractions == 0) continue;

    nIntBC[nTotBC] = nInteractions;
    indexBC[nTotBC] = iBC+IlcTriggerBCMask::kNBits;
    nTotBC++;
  }

  // Loop over the generated collision times, call the generator
  // and correct the partcile times in the stack
  IlcStack *stack = IlcRunLoader::Instance()->Stack();
  Int_t lastpart=0;
  entry->SetFirst(lastpart);

  for(Int_t iBC = 0; iBC <  nTotBC; iBC++) {
    Float_t deltat = 25e-9*(indexBC[iBC] - indexBC[iTrgBC]);
    for (Int_t i = 0; i < nIntBC[iBC]; i++) {
      //  Generate the vertex position and time
      Vertex();
      TArrayF eventVertex(3);
      for (Int_t j=0; j < 3; j++) eventVertex[j] = fVertex[j];
      Double_t vTime = deltat + gRandom->Gaus(0,fOsigma[2]/TMath::Ccgs());
    
      gen->SetVertex(fVertex.At(0), fVertex.At(1), fVertex.At(2));
      gen->Generate();

      for (Int_t k = lastpart; k < stack->GetNprimary(); k++) {
	TLorentzVector v;
	stack->Particle(k)->ProductionVertex(v);
	v[3] = vTime;
	stack->Particle(k)->SetProductionVertex(v);
      }
      lastpart = stack->GetNprimary();

      // Store the interaction header in the container of the headers
      ((IlcGenEventHeader*) fHeader->GetHeaders()->Last())->SetPrimaryVertex(eventVertex);
      ((IlcGenEventHeader*) fHeader->GetHeaders()->Last())->SetInteractionTime(vTime);
    }
  }
  delete [] nIntBC;
  delete [] indexBC;

  entry->SetLast(stack->GetNprimary());

  fHeader->CalcNProduced();

  if (fContainer) {
    fContainer->AddHeader(fHeader);
  } else {
    gIlc->SetGenEventHeader(fHeader);	
  }
 
}

void IlcGenPileup::SetRandomise(Bool_t /*flag*/)
{
  // This setting is not implemented in
  // case of pileup generation
  // So the method gives an warning and exits
  IlcWarning("This setting has no effect on the generator!");
}

void IlcGenPileup::UsePerEventRates()
{
  // This setting is not implemented in
  // case of pileup generation
  // So the method gives an warning and exits
  IlcWarning("This setting has no effect on the generator!");
}
