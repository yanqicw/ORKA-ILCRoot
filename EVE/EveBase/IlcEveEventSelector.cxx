/**************************************************************************
 * 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * Author: The International Lepton Collider Off-line Project.            *
 *                                                                        *
 * Part of the code has been developed by ILC Off-line Project.         *
 * Contributors are mentioned in the code where appropriate.              *
 * See http://ilcinfo.cern.ch/Offline/IlcRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

////////////////////////////////////////////////////////////////////////////
//
//  IlcEveEventSelector class
//  selects events according to given criteria
//
//  origin: Mikolaj Krzewicki, Nikhef, Mikolaj.Krzewicki@cern.ch
//
////////////////////////////////////////////////////////////////////////////

#include "IlcEveEventSelector.h"
#include "IlcEveEventManager.h"
#include "IlcESD.h"
#include "IlcESDEvent.h"
#include "IlcESDRun.h"
#include <TEntryList.h>
#include <TDirectory.h>
#include <TTree.h>
#include <TObjArray.h>
#include <TRegexp.h>
#include <TROOT.h>

ClassImp(IlcEveEventSelector)

//_____________________________________________________________________________
IlcEveEventSelector::IlcEveEventSelector(IlcEveEventManager* evman):
  fPEventManager(evman),
  fWrapAround(kFALSE),
  fSelectOnString(kFALSE),
  fString(""),
  fPEntryList(0),
  fEntryListId(0),
  fLastTreeSize(-1),
  fSelectOnTriggerType(kFALSE),
  fTriggerType(""),
  fSelectOnTriggerString(kFALSE),
  fTriggerSelectionString(""),
  fTriggerMaskPatternString("trgmask"),
  fSelectOnMultiplicity(kFALSE),
  fMultiplicityLow(0),
  fMultiplicityHigh(0)
{
  //ctor
}

//_____________________________________________________________________________
void IlcEveEventSelector::SetSelectionString( const TString& str )
{
  //selection string setter
  //takes care of producing a list of entries passing selection

  TTree* pESDTree = fPEventManager->GetESDTree();
  if (!pESDTree || fString==str) return;//don't waist time
  fString = str;
  if (str == "" ) return;//on reset don't recalculate
  pESDTree->Draw( ">>listofentries", fString, "entrylist");
  fPEntryList = dynamic_cast<TEntryList*>(gDirectory->Get("listofentries"));
}

//_____________________________________________________________________________
void IlcEveEventSelector::SetSelectionString( const char* str )
{
  //selection string setter

  TString ts = str;
  SetSelectionString(ts);
}

//_____________________________________________________________________________
void IlcEveEventSelector::SetTriggerType( const TString& type )
{
  //trigger type setter

  fTriggerType = type;
}

//_____________________________________________________________________________
void IlcEveEventSelector::SetTriggerType( const char* type)
{
  //trigger type setter

  TString ts = type;
  SetTriggerType(ts);
}

//_____________________________________________________________________________
void IlcEveEventSelector::UpdateEntryList()
{
  //update the entrylist from if file changed

  TTree* pESDTree = fPEventManager->GetESDTree();
  if (!pESDTree) return;
  
  Long64_t treesize = fPEventManager->GetMaxEventId()+1;
  if (treesize<=fLastTreeSize) return; //nothing changed, do nothing
  pESDTree->Draw(">>+fPEntryList", fString, "entrylist",
                 fLastTreeSize+1, treesize-fLastTreeSize );
  fLastTreeSize = treesize;
}

//_____________________________________________________________________________
void IlcEveEventSelector::Update()
{
  //refresh stuff

  UpdateEntryList();
}

//_____________________________________________________________________________
Bool_t IlcEveEventSelector::FindNext( Int_t& output )
{
  //does the magick of selecting the next event

  static const TEveException kEH("IlcEveEventSelector::GetNext ");
  
  TTree* pESDTree = fPEventManager->GetESDTree();
  if (!pESDTree) return kFALSE;
  IlcESDEvent* pESDEvent = fPEventManager->GetESD();
  Int_t eventId = fPEventManager->GetEventId();
  Int_t fMaxEventId = fPEventManager->GetMaxEventId();

  if (!fSelectOnString)
  {
    // pure non-string
    for (Int_t i = eventId+1; i<fMaxEventId+1; i++)
    {
      if (pESDTree->GetEntry(i) <= 0)
        throw (kEH + "failed getting required event from ESD.");
      if (CheckOtherSelection(pESDEvent))
      {
        output = i;
        return kTRUE;
      }
    }
    if (!fWrapAround) return kFALSE;
    for (Int_t i = 0; i<eventId+1; i++)
    {
      if (pESDTree->GetEntry(i) <= 0)
        throw (kEH + "failed getting required event from ESD.");
      if (CheckOtherSelection(pESDEvent))
      {
        output = i;
        return kTRUE;
      }
    }
    return kFALSE;
  }
  else
  {
    //select using the entrylist
    UpdateEntryList(); //update entry list if tree got bigger
    for (Long64_t i=fEntryListId+1; i<fPEntryList->GetN(); i++ )
    {
      Long64_t entry = fPEntryList->GetEntry(i);
      if (pESDTree->GetEntry(entry) <= 0)
        throw (kEH + "failed getting required event from ESD.");
      if (CheckOtherSelection(pESDEvent))
      {
        output = entry;
        fEntryListId = i;
        return kTRUE;
      }
    }
    if (!fWrapAround) return kFALSE;
    for (Long64_t i=0; i<fEntryListId+1; i++ )
    {
      Long64_t entry = fPEntryList->GetEntry(i);
      if (pESDTree->GetEntry(entry) <= 0)
        throw (kEH + "failed getting required event from ESD.");
      if (CheckOtherSelection(pESDEvent))
      {
        output = entry;
        fEntryListId=i;
        return kTRUE;
      }
    }
    return kFALSE;
  }
  return kFALSE;

}

//_____________________________________________________________________________
Bool_t IlcEveEventSelector::FindPrev( Int_t& output )
{
  //does the magick of selecting the previous event

  static const TEveException kEH("IlcEveEventSelector::GetNext ");
  
  TTree* pESDTree = fPEventManager->GetESDTree();
  if (!pESDTree) return kFALSE;
  IlcESDEvent* pESDEvent = fPEventManager->GetESD();
  Int_t eventId = fPEventManager->GetEventId();
  Int_t fMaxEventId = fPEventManager->GetMaxEventId();

  if (!fSelectOnString)
  {
    // pure non-string
    for (Int_t i = eventId-1; i>-1; i--)
    {
      if (pESDTree->GetEntry(i) <= 0)
        throw (kEH + "failed getting required event from ESD.");
      if (CheckOtherSelection(pESDEvent))
      {
        output = i;
        return kTRUE;
      }
    }
    if (!fWrapAround) return kFALSE;
    for (Int_t i = fMaxEventId; i>eventId-1; i--)
    {
      if (pESDTree->GetEntry(i) <= 0)
        throw (kEH + "failed getting required event from ESD.");
      if (CheckOtherSelection(pESDEvent))
      {
        output = i;
        return kTRUE;
      }
    }
    return kFALSE;
  }
  else
  {
    //select using the entrylist
    for (Long64_t i=fEntryListId-1; i>-1; i--)
    {
      Long64_t entry = fPEntryList->GetEntry(i);
      if (pESDTree->GetEntry(entry) <= 0)
        throw (kEH + "failed getting required event from ESD.");
      if (CheckOtherSelection(pESDEvent))
      {
        output = entry;
        fEntryListId = i;
        return kTRUE;
      }
    }
    if (!fWrapAround) return kFALSE;
    for (Long64_t i=fPEntryList->GetN()-1; i>fEntryListId-1; i--)
    {
      Long64_t entry = fPEntryList->GetEntry(i);
      if (pESDTree->GetEntry(entry) <= 0)
        throw (kEH + "failed getting required event from ESD.");
      if (CheckOtherSelection(pESDEvent))
      {
        output = entry;
        fEntryListId=i;
        return kTRUE;
      }
    }
    return kFALSE;
  }
  return kFALSE;
}

//_____________________________________________________________________________
Bool_t IlcEveEventSelector::CheckOtherSelection( IlcESDEvent* pESDEvent )
{
  //checks the event for any other hardcoded selection criteria
  
  Bool_t ret=kTRUE;

  //trigger stuff
  if (fSelectOnTriggerType)
  {
    TString firedtrclasses = pESDEvent->GetFiredTriggerClasses();
    if (!(firedtrclasses.Contains(fTriggerType))) return kFALSE;
    //if (!pESDEvent->IsTriggerClassFired(fTriggerType.Data())) return kFALSE;
  }

  if (fSelectOnMultiplicity)
  {
    Int_t mult = pESDEvent->GetNumberOfTracks();
    Int_t mhigh = (fMultiplicityHigh==0)?100000000:fMultiplicityHigh;
    if (mult<fMultiplicityLow || mult>mhigh) return kFALSE;
  }

  if (fSelectOnTriggerString)
  {
    ULong64_t triggermask = pESDEvent->GetTriggerMask();
    TString triggermaskstr;
    triggermaskstr += triggermask;
    TString selstr(fTriggerSelectionString); //make copy
    selstr.ReplaceAll(fTriggerMaskPatternString,triggermaskstr);
    //Int_t returncode;
    Bool_t result = static_cast<Bool_t>(gROOT->ProcessLine(selstr));//,&returncode));
    //if (!returncode) return kFALSE;
    if (!result) return kFALSE;
  }

  return ret;
}

//_____________________________________________________________________________
void IlcEveEventSelector::SetTriggerSelectionString( TString str )
{
  //parses and sets the trigger selection formula
  
  const IlcESDRun* run = fPEventManager->GetESD()->GetESDRun();
  for (Int_t i=0; i<run->kNTriggerClasses; i++)
  {
    TString name(run->GetTriggerClass(i));
    if (name.IsNull()) continue;
    TString valuestr("(");
    valuestr += fTriggerMaskPatternString;
    valuestr += "&";
    valuestr += static_cast<ULong64_t>(1)<<i;
    valuestr += ")";
    str.ReplaceAll(name,valuestr);
  }//for i
  fTriggerSelectionString = str;
}

