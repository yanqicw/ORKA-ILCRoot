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

/* $Id: IlcESDEvent.cxx 55446 2012-03-30 16:00:07Z hristov $ */

//-----------------------------------------------------------------
//           Implementation of the IlcESDEvent class
//   This is the class to deal with during the physics analysis of data.
//   It also ensures the backward compatibility with the old ESD format.
/*
   IlcESDEvent *ev= new IlcESDEvent();
   ev->ReadFromTree(esdTree);
   ...
    for (Int_t i=0; i<nev; i++) {
      esdTree->GetEntry(i);
      if(ev->GetIlcESDOld())ev->CopyFromOldESD();
*/
//   The IlcESDInputHandler does this automatically for you
//
// Origin: Christian Klein-Boesing, CERN, Christian.Klein-Boesing@cern.ch
//-----------------------------------------------------------------

#include "TList.h"
#include "TRefArray.h"
#include <TNamed.h>
#include <TROOT.h>
#include <TInterpreter.h>

#include "IlcESDEvent.h"
#include "IlcESDfriend.h"
#include "IlcESDVZERO.h"
#include "IlcESDFMD.h"
#include "IlcESD.h"
#include "IlcESDMuonTrack.h"
#include "IlcESDMuonCluster.h"
#include "IlcESDMuonPad.h"
#include "IlcESDPmdTrack.h"
#include "IlcESDTrdTrack.h"
#include "IlcESDVertex.h"
#include "IlcESDcascade.h"
#include "IlcESDPmdTrack.h"
#include "IlcESDTrdTrigger.h"
#include "IlcESDTrdTrack.h"
#include "IlcESDTrdTracklet.h"
#include "IlcESDVertex.h"
#include "IlcVertexerTracks.h"
#include "IlcESDcascade.h"
#include "IlcESDkink.h"
#include "IlcESDtrack.h"
#include "IlcESDHLTtrack.h"
#include "IlcESDCaloCluster.h"
#include "IlcESDCaloCells.h"
#include "IlcESDv0.h"
#include "IlcESDFMD.h"
#include "IlcESDVZERO.h"
#include "IlcMultiplicity.h"
#include "IlcRawDataErrorLog.h"
#include "IlcLog.h"
#include "IlcESDACORDE.h"
#include "IlcESDHLTDecision.h"
#include "IlcCentrality.h"
#include "IlcESDCosmicTrack.h"
#ifdef MFT_UPGRADE
#include "IlcESDMFT.h"
#endif
#include "IlcEventplane.h"


ClassImp(IlcESDEvent)



// here we define the names, some classes are no TNamed, therefore the classnames 
// are the Names
  const char* IlcESDEvent::fgkESDListName[kESDListN] = {"IlcESDRun",
							"IlcESDHeader",
							"IlcESDZDC",
							"IlcESDFMD",
							"IlcESDVZERO",
							"IlcESDTZERO",
							"TPCVertex",
							"SPDVertex",
							"PrimaryVertex",
							"IlcMultiplicity",
							"PHOSTrigger",
							"EMCALTrigger",
							"SPDPileupVertices",
							"TrkPileupVertices",
							"Tracks",
							"MuonTracks",
							"MuonClusters",
							"MuonPads",
							"PmdTracks",
							"IlcESDTrdTrigger",
							"TrdTracks",
  						        "TrdTracklets",
							"V0s",
							"Cascades",
							"Kinks",
							"CaloClusters",
							"EMCALCells",
							"PHOSCells",
							"IlcRawDataErrorLogs",
							"IlcESDACORDE",
							"IlcTOFHeader",
                                                        "CosmicTracks"
                              #ifdef MFT_UPGRADE
//	                        , "IlcESDMFT"
							#endif
  };

//______________________________________________________________________________
IlcESDEvent::IlcESDEvent():
  IlcVEvent(),
  fESDObjects(new TList()),
  fESDRun(0),
  fHeader(0),
  fESDZDC(0),
  fESDFMD(0),
  fESDVZERO(0),
  fESDTZERO(0),
  fTPCVertex(0),
  fSPDVertex(0),
  fPrimaryVertex(0),
  fSPDMult(0),
  fPHOSTrigger(0),
  fEMCALTrigger(0),
  fESDACORDE(0),
  fTrdTrigger(0),
  fSPDPileupVertices(0),
  fTrkPileupVertices(0),
  fTracks(0),
  fMuonTracks(0),
  fMuonClusters(0),
  fMuonPads(0),
  fPmdTracks(0),
  fTrdTracks(0),
  fTrdTracklets(0),
  fV0s(0),  
  fCascades(0),
  fKinks(0),
  fCaloClusters(0),
  fEMCALCells(0), fPHOSCells(0),
  fCosmicTracks(0),
  fErrorLogs(0),
  fOldMuonStructure(kFALSE),
  fESDOld(0),
  fESDFriendOld(0),
  fConnected(kFALSE),
  fUseOwnList(kFALSE),
  fTOFHeader(0),
  fCentrality(0),
  fEventplane(0),
  fDetectorStatus(0xFFFFFFFF)
  #ifdef MFT_UPGRADE
//  , fESDMFT(0)
  #endif
{
}
//______________________________________________________________________________
IlcESDEvent::IlcESDEvent(const IlcESDEvent& esd):
  IlcVEvent(esd),
  fESDObjects(new TList()),
  fESDRun(new IlcESDRun(*esd.fESDRun)),
  fHeader(new IlcESDHeader(*esd.fHeader)),
  fESDZDC(new IlcESDZDC(*esd.fESDZDC)),
  fESDFMD(new IlcESDFMD(*esd.fESDFMD)),
  fESDVZERO(new IlcESDVZERO(*esd.fESDVZERO)),
  fESDTZERO(new IlcESDTZERO(*esd.fESDTZERO)),
  fTPCVertex(new IlcESDVertex(*esd.fTPCVertex)),
  fSPDVertex(new IlcESDVertex(*esd.fSPDVertex)),
  fPrimaryVertex(new IlcESDVertex(*esd.fPrimaryVertex)),
  fSPDMult(new IlcMultiplicity(*esd.fSPDMult)),
  fPHOSTrigger(new IlcESDCaloTrigger(*esd.fPHOSTrigger)),
  fEMCALTrigger(new IlcESDCaloTrigger(*esd.fEMCALTrigger)),
  fESDACORDE(new IlcESDACORDE(*esd.fESDACORDE)),
  fTrdTrigger(new IlcESDTrdTrigger(*esd.fTrdTrigger)),
  fSPDPileupVertices(new TClonesArray(*esd.fSPDPileupVertices)),
  fTrkPileupVertices(new TClonesArray(*esd.fTrkPileupVertices)),
  fTracks(new TClonesArray(*esd.fTracks)),
  fMuonTracks(new TClonesArray(*esd.fMuonTracks)),
  fMuonClusters(new TClonesArray(*esd.fMuonClusters)),
  fMuonPads(new TClonesArray(*esd.fMuonPads)),
  fPmdTracks(new TClonesArray(*esd.fPmdTracks)),
  fTrdTracks(new TClonesArray(*esd.fTrdTracks)),
  fTrdTracklets(new TClonesArray(*esd.fTrdTracklets)),
  fV0s(new TClonesArray(*esd.fV0s)),  
  fCascades(new TClonesArray(*esd.fCascades)),
  fKinks(new TClonesArray(*esd.fKinks)),
  fCaloClusters(new TClonesArray(*esd.fCaloClusters)),
  fEMCALCells(new IlcESDCaloCells(*esd.fEMCALCells)),
  fPHOSCells(new IlcESDCaloCells(*esd.fPHOSCells)),
  fCosmicTracks(new TClonesArray(*esd.fCosmicTracks)),
  fErrorLogs(new TClonesArray(*esd.fErrorLogs)),
  fOldMuonStructure(esd.fOldMuonStructure),
  fESDOld(esd.fESDOld ? new IlcESD(*esd.fESDOld) : 0),
  fESDFriendOld(esd.fESDFriendOld ? new IlcESDfriend(*esd.fESDFriendOld) : 0),
  fConnected(esd.fConnected),
  fUseOwnList(esd.fUseOwnList),
  fTOFHeader(new IlcTOFHeader(*esd.fTOFHeader)),
  fCentrality(new IlcCentrality(*esd.fCentrality)),
  fEventplane(new IlcEventplane(*esd.fEventplane)),
  fDetectorStatus(esd.fDetectorStatus)
  #ifdef MFT_UPGRADE
//  , fESDMFT(new IlcESDMFT(*esd.fESDMFT))
  #endif


{
  printf("copying ESD event...\n");   // AU
  // CKB init in the constructor list and only add here ...
  AddObject(fESDRun);
  AddObject(fHeader);
  AddObject(fESDZDC);
  AddObject(fESDFMD);
  AddObject(fESDVZERO);
  AddObject(fESDTZERO);
  AddObject(fTPCVertex);
  AddObject(fSPDVertex);
  AddObject(fPrimaryVertex);
  AddObject(fSPDMult);
  AddObject(fPHOSTrigger);
  AddObject(fEMCALTrigger);
  AddObject(fTrdTrigger);
  AddObject(fSPDPileupVertices);
  AddObject(fTrkPileupVertices);
  AddObject(fTracks);
  AddObject(fMuonTracks);
  AddObject(fPmdTracks);
  AddObject(fTrdTracks);
  AddObject(fTrdTracklets);
  AddObject(fV0s);
  AddObject(fCascades);
  AddObject(fKinks);
  AddObject(fCaloClusters);
  AddObject(fEMCALCells);
  AddObject(fPHOSCells);
  AddObject(fCosmicTracks);
  AddObject(fErrorLogs);
  AddObject(fESDACORDE);
  AddObject(fTOFHeader);
  AddObject(fMuonClusters);
  AddObject(fMuonPads);
  #ifdef MFT_UPGRADE
//  AddObject(fESDMFT);
  #endif
  GetStdContent();

}

//______________________________________________________________________________
IlcESDEvent & IlcESDEvent::operator=(const IlcESDEvent& source) {

  // Assignment operator

  if(&source == this) return *this;
  IlcVEvent::operator=(source);

  // This assumes that the list is already created
  // and that the virtual void Copy(Tobject&) function
  // is correctly implemented in the derived class
  // otherwise only TObject::Copy() will be used



  if((fESDObjects->GetSize()==0)&&(source.fESDObjects->GetSize()>=kESDListN)){
    // We cover the case that we do not yet have the 
    // standard content but the source has it
    CreateStdContent();
  }

  TIter next(source.GetList());
  TObject *its = 0;
  TString name;
  while ((its = next())) {
    name.Form("%s", its->GetName());
    TObject *mine = fESDObjects->FindObject(name.Data());
    if(!mine){
      TClass* pClass=TClass::GetClass(its->ClassName());
      if (!pClass) {
	IlcWarning(Form("Can not find class description for entry %s (%s)\n",
			its->ClassName(), name.Data()));
	continue;
      }

      mine=(TObject*)pClass->New();
      if(!mine){
      // not in this: can be added to list
	IlcWarning(Form("%s:%d Could not find %s for copying \n",
			(char*)__FILE__,__LINE__,name.Data()));
	continue;
      }  
      if(mine->InheritsFrom("TNamed")){
	((TNamed*)mine)->SetName(name);
      }
      else if(mine->InheritsFrom("TCollection")){
	if(mine->InheritsFrom("TClonesArray")) {
	  TClonesArray* tcits = dynamic_cast<TClonesArray*>(its);
	  if (tcits)
	    dynamic_cast<TClonesArray*>(mine)->SetClass(tcits->GetClass());
	}
	dynamic_cast<TCollection*>(mine)->SetName(name);
      }
      IlcDebug(1, Form("adding object %s of type %s", mine->GetName(), mine->ClassName()));
      AddObject(mine);
    }  
   
    if(!its->InheritsFrom("TCollection")){
      // simple objects
      its->Copy(*mine);
    }
    else if(its->InheritsFrom("TClonesArray")){
      // Create or expand the tclonesarray pointers
      // so we can directly copy to the object
      TClonesArray *itstca = (TClonesArray*)its;
      TClonesArray *minetca = (TClonesArray*)mine;

      // this leaves the capacity of the TClonesArray the same
      // except for a factor of 2 increase when size > capacity
      // does not release any memory occupied by the tca
      minetca->ExpandCreate(itstca->GetEntriesFast());
      for(int i = 0;i < itstca->GetEntriesFast();++i){
	// copy 
	TObject *minetcaobj = minetca->At(i);
	TObject *itstcaobj = itstca->At(i);
	// no need to delete first
	// pointers within the class should be handled by Copy()...
	// Can there be Empty slots?
	itstcaobj->Copy(*minetcaobj);
      }
    }
    else{
      IlcWarning(Form("%s:%d cannot copy TCollection \n",
		      (char*)__FILE__,__LINE__));
    }
  }

  fOldMuonStructure = source.fOldMuonStructure;
  
  fCentrality = source.fCentrality;
  fEventplane = source.fEventplane;

  fConnected  = source.fConnected;
  fUseOwnList = source.fUseOwnList;
  
  fDetectorStatus = source.fDetectorStatus;

  return *this;
}


//______________________________________________________________________________
IlcESDEvent::~IlcESDEvent()
{
  //
  // Standard destructor
  //

  // everthing on the list gets deleted automatically

  
  if(fESDObjects&&!fConnected)
    {
      delete fESDObjects;
      fESDObjects = 0;
    }
  if (fCentrality) delete fCentrality;
  if (fEventplane) delete fEventplane;
  
}

void IlcESDEvent::Copy(TObject &obj) const {

  // interface to TOBject::Copy
  // Copies the content of this into obj!
  // bascially obj = *this

  if(this==&obj)return;
  IlcESDEvent *robj = dynamic_cast<IlcESDEvent*>(&obj);
  if(!robj)return; // not an IlcESDEvent
  *robj = *this;
  return;
}

//______________________________________________________________________________
void IlcESDEvent::Reset()
{

  // Handle the cases
  // Std content + Non std content

  // Reset the standard contents
  ResetStdContent(); 

  //  reset for the old data without IlcESDEvent...
  if(fESDOld)fESDOld->Reset();
  if(fESDFriendOld){
    fESDFriendOld->~IlcESDfriend();
    new (fESDFriendOld) IlcESDfriend();
  }
  // 

  if(fESDObjects->GetSize()>kESDListN){
    // we have non std content
    // this also covers esdfriends
    for(int i = kESDListN;i < fESDObjects->GetSize();++i){
      TObject *pObject = fESDObjects->At(i);
      // TClonesArrays
      if(pObject->InheritsFrom(TClonesArray::Class())){
	((TClonesArray*)pObject)->Delete();
      }
      else if(!pObject->InheritsFrom(TCollection::Class())){
	TClass *pClass = TClass::GetClass(pObject->ClassName());
	if (pClass && pClass->GetListOfMethods()->FindObject("Clear")) {
	  IlcDebug(1, Form("Clear for object %s class %s", pObject->GetName(), pObject->ClassName()));
	  pObject->Clear();
	}
	else {
	  IlcDebug(1, Form("ResetWithPlacementNew for object %s class %s", pObject->GetName(), pObject->ClassName()));
	  ResetWithPlacementNew(pObject);
	}
      }
      else{
	IlcWarning(Form("No reset for %s \n",
			pObject->ClassName()));
      }
    }
  }

}

Bool_t IlcESDEvent::ResetWithPlacementNew(TObject *pObject){
  //
  // funtion to reset using the already allocated space
  //
  Long_t dtoronly = TObject::GetDtorOnly();
  TClass *pClass = TClass::GetClass(pObject->ClassName()); 
  TObject::SetDtorOnly(pObject);
  delete pObject;
  // Recreate with placement new
  pClass->New(pObject);
  // Restore the state.
  TObject::SetDtorOnly((void*)dtoronly);
  return kTRUE;
}

void IlcESDEvent::ResetStdContent()
{
  // Reset the standard contents
  if(fESDRun) fESDRun->Reset();
  if(fHeader) fHeader->Reset();
  if(fCentrality) fCentrality->Reset();
  if(fEventplane) fEventplane->Reset();
  if(fESDZDC) fESDZDC->Reset();
  if(fESDFMD) {
    fESDFMD->Clear();
  }
  if(fESDVZERO){
    // reset by callin d'to /c'tor keep the pointer
    fESDVZERO->~IlcESDVZERO();
    new (fESDVZERO) IlcESDVZERO();
  }  
  if(fESDACORDE){
    fESDACORDE->~IlcESDACORDE();
    new (fESDACORDE) IlcESDACORDE();	
  } 
  if(fESDTZERO) fESDTZERO->Reset(); 
  // CKB no clear/reset implemented
  if(fTPCVertex){
    fTPCVertex->~IlcESDVertex();
    new (fTPCVertex) IlcESDVertex();
    fTPCVertex->SetName(fgkESDListName[kTPCVertex]);
  }
  if(fSPDVertex){
    fSPDVertex->~IlcESDVertex();
    new (fSPDVertex) IlcESDVertex();
    fSPDVertex->SetName(fgkESDListName[kSPDVertex]);
  }
  if(fPrimaryVertex){
    fPrimaryVertex->~IlcESDVertex();
    new (fPrimaryVertex) IlcESDVertex();
    fPrimaryVertex->SetName(fgkESDListName[kPrimaryVertex]);
  }
  if(fSPDMult){
    fSPDMult->~IlcMultiplicity();
    new (fSPDMult) IlcMultiplicity();
  }
  if(fTOFHeader){
    fTOFHeader->~IlcTOFHeader();
    new (fTOFHeader) IlcTOFHeader();
    //fTOFHeader->SetName(fgkESDListName[kTOFHeader]);
  }
  if (fTrdTrigger) {
    fTrdTrigger->~IlcESDTrdTrigger();
    new (fTrdTrigger) IlcESDTrdTrigger();
  }
  #ifdef MFT_UPGRADE
  //if(fESDMFT){
//	fESDMFT->~IlcESDMFT();
//	new (fESDMFT) IlcESDMFT();
 // }  
  #endif
	
  if(fPHOSTrigger)fPHOSTrigger->DeAllocate(); 
  if(fEMCALTrigger)fEMCALTrigger->DeAllocate(); 
  if(fSPDPileupVertices)fSPDPileupVertices->Delete();
  if(fTrkPileupVertices)fTrkPileupVertices->Delete();
  if(fTracks)fTracks->Delete();
  if(fMuonTracks)fMuonTracks->Clear("C");
  if(fMuonClusters)fMuonClusters->Clear("C");
  if(fMuonPads)fMuonPads->Clear("C");
  if(fPmdTracks)fPmdTracks->Delete();
  if(fTrdTracks)fTrdTracks->Delete();
  if(fTrdTracklets)fTrdTracklets->Delete();
  if(fV0s)fV0s->Delete();
  if(fCascades)fCascades->Delete();
  if(fKinks)fKinks->Delete();
  if(fCaloClusters)fCaloClusters->Delete();
  if(fPHOSCells)fPHOSCells->DeleteContainer();
  if(fEMCALCells)fEMCALCells->DeleteContainer();
  if(fCosmicTracks)fCosmicTracks->Delete();
  if(fErrorLogs) fErrorLogs->Delete();

  // don't reset fconnected fConnected and the list

}


Int_t IlcESDEvent::AddV0(const IlcESDv0 *v) {
  //
  // Add V0
  //
  TClonesArray &fv = *fV0s;
  Int_t idx=fV0s->GetEntriesFast();
  new(fv[idx]) IlcESDv0(*v);
  return idx;
}  

//______________________________________________________________________________
void IlcESDEvent::Print(Option_t *) const 
{
  //
  // Print header information of the event
  //
  printf("ESD run information\n");
  printf("Event # in file %d Bunch crossing # %d Orbit # %d Period # %d Run # %d Trigger %lld Magnetic field %f \n",
	 GetEventNumberInFile(),
	 GetBunchCrossNumber(),
	 GetOrbitNumber(),
	 GetPeriodNumber(),
	 GetRunNumber(),
	 GetTriggerMask(),
	 GetMagneticField() );
  if (fPrimaryVertex)
    printf("Vertex: (%.4f +- %.4f, %.4f +- %.4f, %.4f +- %.4f) cm\n",
	   fPrimaryVertex->GetXv(), fPrimaryVertex->GetXRes(),
	   fPrimaryVertex->GetYv(), fPrimaryVertex->GetYRes(),
	   fPrimaryVertex->GetZv(), fPrimaryVertex->GetZRes());
  printf("Mean vertex in RUN: X=%.4f Y=%.4f Z=%.4f cm\n",
	 GetDiamondX(),GetDiamondY(),GetDiamondZ());
  if(fSPDMult)
    printf("SPD Multiplicity. Number of tracklets %d \n",
           fSPDMult->GetNumberOfTracklets());
  printf("Number of pileup primary vertices reconstructed with SPD %d\n", 
	 GetNumberOfPileupVerticesSPD());
  printf("Number of pileup primary vertices reconstructed using the tracks %d\n",
	 GetNumberOfPileupVerticesTracks());
  printf("Number of tracks: \n");
  printf("                 charged   %d\n", GetNumberOfTracks());
  printf("                 muon      %d\n", GetNumberOfMuonTracks());
  printf("                 pmd       %d\n", GetNumberOfPmdTracks());
  printf("                 trd       %d\n", GetNumberOfTrdTracks());
  printf("                 trd trkl  %d\n", GetNumberOfTrdTracklets());
  printf("                 v0        %d\n", GetNumberOfV0s());
  printf("                 cascades  %d\n", GetNumberOfCascades());
  printf("                 kinks     %d\n", GetNumberOfKinks());
  if(fPHOSCells)printf("                 PHOSCells %d\n", fPHOSCells->GetNumberOfCells());
  else printf("                 PHOSCells not in the Event\n");
  if(fEMCALCells)printf("                 EMCALCells %d\n", fEMCALCells->GetNumberOfCells());
  else printf("                 EMCALCells not in the Event\n");
  printf("                 CaloClusters %d\n", GetNumberOfCaloClusters());
  printf("                 FMD       %s\n", (fESDFMD ? "yes" : "no"));
  printf("                 VZERO     %s\n", (fESDVZERO ? "yes" : "no"));
  printf("                 muClusters %d\n", fMuonClusters ? fMuonClusters->GetEntriesFast() : 0);
  printf("                 muPad     %d\n", fMuonPads ? fMuonPads->GetEntriesFast() : 0);
  if (fCosmicTracks) printf("                 Cosmics   %d\n",  GetNumberOfCosmicTracks());
  #ifdef MFT_UPGRADE
  //printf("                 MFT     %s\n", (fESDMFT ? "yes" : "no"));
  #endif
	
	
  TObject* pHLTDecision=GetHLTTriggerDecision();
  printf("HLT trigger decision: %s\n", pHLTDecision?pHLTDecision->GetOption():"not available");
  if (pHLTDecision) pHLTDecision->Print("compact");

  return;
}

void IlcESDEvent::SetESDfriend(const IlcESDfriend *ev) const {
  //
  // Attaches the complementary info to the ESD
  //
  if (!ev) return;

  // to be sure that we set the tracks also
  // in case of old esds 
  // if(fESDOld)CopyFromOldESD();

  Int_t ntrk=ev->GetNumberOfTracks();
 
  for (Int_t i=0; i<ntrk; i++) {
    const IlcESDfriendTrack *f=ev->GetTrack(i);
    if (!f) {IlcFatal(Form("NULL pointer for ESD track %d",i));}
    GetTrack(i)->SetFriendTrack(f);
  }
}

Bool_t  IlcESDEvent::RemoveKink(Int_t rm) const {
  // ---------------------------------------------------------
  // Remove a kink candidate and references to it from ESD,
  // if this candidate does not come from a reconstructed decay
  // Not yet implemented...
  // ---------------------------------------------------------
  Int_t last=GetNumberOfKinks()-1;
  if ((rm<0)||(rm>last)) return kFALSE;

  return kTRUE;
}

Bool_t  IlcESDEvent::RemoveV0(Int_t rm) const {
  // ---------------------------------------------------------
  // Remove a V0 candidate and references to it from ESD,
  // if this candidate does not come from a reconstructed decay
  // ---------------------------------------------------------
  Int_t last=GetNumberOfV0s()-1;
  if ((rm<0)||(rm>last)) return kFALSE;

  IlcESDv0 *v0=GetV0(rm);
  Int_t idxP=v0->GetPindex(), idxN=v0->GetNindex();

  v0=GetV0(last);
  Int_t lastIdxP=v0->GetPindex(), lastIdxN=v0->GetNindex();

  Int_t used=0;

  // Check if this V0 comes from a reconstructed decay
  Int_t ncs=GetNumberOfCascades();
  for (Int_t n=0; n<ncs; n++) {
    IlcESDcascade *cs=GetCascade(n);

    Int_t csIdxP=cs->GetPindex();
    Int_t csIdxN=cs->GetNindex();

    if (idxP==csIdxP)
       if (idxN==csIdxN) return kFALSE;

    if (csIdxP==lastIdxP)
       if (csIdxN==lastIdxN) used++;
  }

  //Replace the removed V0 with the last V0 
  TClonesArray &a=*fV0s;
  delete a.RemoveAt(rm);

  if (rm==last) return kTRUE;

  //v0 is pointing to the last V0 candidate... 
  new (a[rm]) IlcESDv0(*v0);
  delete a.RemoveAt(last);

  if (!used) return kTRUE;
  

  // Remap the indices of the daughters of reconstructed decays
  for (Int_t n=0; n<ncs; n++) {
    IlcESDcascade *cs=GetCascade(n);


    Int_t csIdxP=cs->GetPindex();
    Int_t csIdxN=cs->GetNindex();

    if (csIdxP==lastIdxP)
      if (csIdxN==lastIdxN) {
         cs->IlcESDv0::SetIndex(1,idxP);
         cs->IlcESDv0::SetIndex(0,idxN);
         used--;
         if (!used) return kTRUE;
      }
  }

  return kTRUE;
}

Bool_t  IlcESDEvent::RemoveTrack(Int_t rm) const {
  // ---------------------------------------------------------
  // Remove a track and references to it from ESD,
  // if this track does not come from a reconstructed decay
  // ---------------------------------------------------------
  Int_t last=GetNumberOfTracks()-1;
  if ((rm<0)||(rm>last)) return kFALSE;

  Int_t used=0;

  // Check if this track comes from the reconstructed primary vertices
  if (fTPCVertex && fTPCVertex->GetStatus()) {
     UShort_t *primIdx=fTPCVertex->GetIndices();
     Int_t n=fTPCVertex->GetNIndices();
     while (n--) {
       Int_t idx=Int_t(primIdx[n]);
       if (rm==idx) return kFALSE;
       if (idx==last) used++; 
     }
  }
  if (fPrimaryVertex && fPrimaryVertex->GetStatus()) {
     UShort_t *primIdx=fPrimaryVertex->GetIndices();
     Int_t n=fPrimaryVertex->GetNIndices();
     while (n--) {
       Int_t idx=Int_t(primIdx[n]);
       if (rm==idx) return kFALSE;
       if (idx==last) used++; 
     }
  }
  
  // Check if this track comes from a reconstructed decay
  Int_t nv0=GetNumberOfV0s();
  for (Int_t n=0; n<nv0; n++) {
    IlcESDv0 *v0=GetV0(n);

    Int_t idx=v0->GetNindex();
    if (rm==idx) return kFALSE;
    if (idx==last) used++;

    idx=v0->GetPindex();
    if (rm==idx) return kFALSE;
    if (idx==last) used++;
  }

  Int_t ncs=GetNumberOfCascades();
  for (Int_t n=0; n<ncs; n++) {
    IlcESDcascade *cs=GetCascade(n);

    Int_t idx=cs->GetIndex();
    if (rm==idx) return kFALSE;
    if (idx==last) used++;

    IlcESDv0 *v0=cs;
    idx=v0->GetNindex();
    if (rm==idx) return kFALSE;
    if (idx==last) used++;

    idx=v0->GetPindex();
    if (rm==idx) return kFALSE;
    if (idx==last) used++;
  }

  Int_t nkn=GetNumberOfKinks();
  for (Int_t n=0; n<nkn; n++) {
    IlcESDkink *kn=GetKink(n);

    Int_t idx=kn->GetIndex(0);
    if (rm==idx) return kFALSE;
    if (idx==last) used++;

    idx=kn->GetIndex(1);
    if (rm==idx) return kFALSE;
    if (idx==last) used++;
  }

  // Check if this track is associated with a CaloCluster
  Int_t ncl=GetNumberOfCaloClusters();
  for (Int_t n=0; n<ncl; n++) {
    IlcESDCaloCluster *cluster=GetCaloCluster(n);
    TArrayI *arr=cluster->GetTracksMatched();
    Int_t s=arr->GetSize();
    while (s--) {
      Int_t idx=arr->At(s);
      if (rm==idx) return kFALSE;
      if (idx==last) used++;     
    }
  }



  //Replace the removed track with the last track 
  TClonesArray &a=*fTracks;
  delete a.RemoveAt(rm);

  if (rm==last) return kTRUE;

  IlcESDtrack *t=GetTrack(last);
  if (!t) {IlcFatal(Form("NULL pointer for ESD track %d",last));}
  t->SetID(rm);
  new (a[rm]) IlcESDtrack(*t);
  delete a.RemoveAt(last);


  if (!used) return kTRUE;
  

  // Remap the indices of the tracks used for the primary vertex reconstruction
  if (fTPCVertex && fTPCVertex->GetStatus()) {
     UShort_t *primIdx=fTPCVertex->GetIndices();
     Int_t n=fTPCVertex->GetNIndices();
     while (n--) {
       Int_t idx=Int_t(primIdx[n]);
       if (idx==last) {
          primIdx[n]=Short_t(rm); 
          used--;
          if (!used) return kTRUE;
       }
     }
  }  
  if (fPrimaryVertex && fPrimaryVertex->GetStatus()) {
     UShort_t *primIdx=fPrimaryVertex->GetIndices();
     Int_t n=fPrimaryVertex->GetNIndices();
     while (n--) {
       Int_t idx=Int_t(primIdx[n]);
       if (idx==last) {
          primIdx[n]=Short_t(rm); 
          used--;
          if (!used) return kTRUE;
       }
     }
  }  

  // Remap the indices of the daughters of reconstructed decays
  for (Int_t n=0; n<nv0; n++) {
    IlcESDv0 *v0=GetV0(n);
    if (v0->GetIndex(0)==last) {
       v0->SetIndex(0,rm);
       used--;
       if (!used) return kTRUE;
    }
    if (v0->GetIndex(1)==last) {
       v0->SetIndex(1,rm);
       used--;
       if (!used) return kTRUE;
    }
  }

  for (Int_t n=0; n<ncs; n++) {
    IlcESDcascade *cs=GetCascade(n);
    if (cs->GetIndex()==last) {
       cs->SetIndex(rm);
       used--;
       if (!used) return kTRUE;
    }
    IlcESDv0 *v0=cs;
    if (v0->GetIndex(0)==last) {
       v0->SetIndex(0,rm);
       used--;
       if (!used) return kTRUE;
    }
    if (v0->GetIndex(1)==last) {
       v0->SetIndex(1,rm);
       used--;
       if (!used) return kTRUE;
    }
  }

  for (Int_t n=0; n<nkn; n++) {
    IlcESDkink *kn=GetKink(n);
    if (kn->GetIndex(0)==last) {
       kn->SetIndex(rm,0);
       used--;
       if (!used) return kTRUE;
    }
    if (kn->GetIndex(1)==last) {
       kn->SetIndex(rm,1);
       used--;
       if (!used) return kTRUE;
    }
  }

  // Remap the indices of the tracks accosicated with CaloClusters
  for (Int_t n=0; n<ncl; n++) {
    IlcESDCaloCluster *cluster=GetCaloCluster(n);
    TArrayI *arr=cluster->GetTracksMatched();
    Int_t s=arr->GetSize();
    while (s--) {
      Int_t idx=arr->At(s);
      if (idx==last) {
         arr->AddAt(rm,s);
         used--; 
         if (!used) return kTRUE;
      }
    }
  }

  return kTRUE;
}


Bool_t IlcESDEvent::Clean(Float_t *cleanPars) {
  //
  // Remove the data which are not needed for the physics analysis.
  //
  // 1) Cleaning the V0 candidates
  //    ---------------------------
  //    If the cosine of the V0 pointing angle "csp" and 
  //    the DCA between the daughter tracks "dca" does not satisfy 
  //    the conditions 
  //
  //     csp > cleanPars[1] + dca/cleanPars[0]*(1.- cleanPars[1])
  //
  //    an attempt to remove this V0 candidate from ESD is made.
  //
  //    The V0 candidate gets removed if it does not belong to any 
  //    recosntructed cascade decay
  //
  //    12.11.2007, optimal values: cleanPars[0]=0.5, cleanPars[1]=0.999
  //
  // 2) Cleaning the tracks
  //    ----------------------
  //    If track's transverse parameter is larger than cleanPars[2]
  //                       OR
  //    track's longitudinal parameter is larger than cleanPars[3]
  //    an attempt to remove this track from ESD is made.
  //
  //    The track gets removed if it does not come 
  //    from a reconstructed decay
  //
  Bool_t rc=kFALSE;

  Float_t dcaMax=cleanPars[0];
  Float_t cspMin=cleanPars[1];

  Int_t nV0s=GetNumberOfV0s();
  for (Int_t i=nV0s-1; i>=0; i--) {
    IlcESDv0 *v0=GetV0(i);

    Float_t dca=v0->GetDcaV0Daughters();
    Float_t csp=v0->GetV0CosineOfPointingAngle();
    Float_t cspcut=cspMin + dca/dcaMax*(1.-cspMin);
    if (csp > cspcut) continue;
    if (RemoveV0(i)) rc=kTRUE;
  }


  Float_t dmax=cleanPars[2], zmax=cleanPars[3];

  const IlcESDVertex *vertex=GetPrimaryVertexSPD();
  Bool_t vtxOK=vertex->GetStatus();
  
  Int_t nTracks=GetNumberOfTracks();
  for (Int_t i=nTracks-1; i>=0; i--) {
    IlcESDtrack *track=GetTrack(i);
    if (!track) {IlcFatal(Form("NULL pointer for ESD track %d",i));}
    Float_t xy,z; track->GetImpactParameters(xy,z);
    if ((TMath::Abs(xy) > dmax) || (vtxOK && (TMath::Abs(z) > zmax))) {
      if (RemoveTrack(i)) rc=kTRUE;
    }
  }

  return rc;
}

Char_t  IlcESDEvent::AddPileupVertexSPD(const IlcESDVertex *vtx) 
{
    // Add a pileup primary vertex reconstructed with SPD
    TClonesArray &ftr = *fSPDPileupVertices;
    Char_t n=Char_t(ftr.GetEntriesFast());
    IlcESDVertex *vertex = new(ftr[n]) IlcESDVertex(*vtx);
    vertex->SetID(n);
    return n;
}

Char_t  IlcESDEvent::AddPileupVertexTracks(const IlcESDVertex *vtx) 
{
    // Add a pileup primary vertex reconstructed with SPD
    TClonesArray &ftr = *fTrkPileupVertices;
    Char_t n=Char_t(ftr.GetEntriesFast());
    IlcESDVertex *vertex = new(ftr[n]) IlcESDVertex(*vtx);
    vertex->SetID(n);
    return n;
}

Int_t  IlcESDEvent::AddTrack(const IlcESDtrack *t) 
{
    // Add track
    TClonesArray &ftr = *fTracks;
    IlcESDtrack * track = new(ftr[fTracks->GetEntriesFast()])IlcESDtrack(*t);
    track->SetID(fTracks->GetEntriesFast()-1);
    return  track->GetID();
}

IlcESDtrack*  IlcESDEvent::NewTrack() 
{
    // Add a new track
    TClonesArray &ftr = *fTracks;
    IlcESDtrack * track = new(ftr[fTracks->GetEntriesFast()])IlcESDtrack();
    track->SetID(fTracks->GetEntriesFast()-1);
    return  track;
}

//______________________________________________________________________________
Bool_t IlcESDEvent::MoveMuonObjects() 
{
  // move MUON clusters and pads to the new ESD structure in needed.
  // to ensure backward compatibility
  
  if (!fOldMuonStructure) return kTRUE;
  
  if (!fMuonTracks || !fMuonClusters || !fMuonPads) return kFALSE;
  
  Bool_t reset = kTRUE;
  Bool_t containTrackerData = kFALSE;
  for (Int_t i = 0; i < fMuonTracks->GetEntriesFast(); i++) {
    
    IlcESDMuonTrack *track = (IlcESDMuonTrack*) fMuonTracks->UncheckedAt(i);
    
    if (track->ContainTrackerData()) containTrackerData = kTRUE;
    else continue;
    
    if (!track->IsOldTrack()) continue;
    
    // remove objects connected to previous event if needed
    if (reset) {
      if (fMuonClusters->GetEntriesFast() > 0) fMuonClusters->Clear("C");
      if (fMuonPads->GetEntriesFast() > 0) fMuonPads->Clear("C");
      reset = kFALSE;
    }
    
    track->MoveClustersToESD(*this);
    
  }
  
  // remove objects connected to previous event if needed
  if (!containTrackerData) {
    if (fMuonClusters->GetEntriesFast() > 0) fMuonClusters->Clear("C");
    if (fMuonPads->GetEntriesFast() > 0) fMuonPads->Clear("C");
  }
  
  return kTRUE;
}

//______________________________________________________________________________
IlcESDMuonTrack* IlcESDEvent::GetMuonTrack(Int_t i)
{
  // get the MUON track at the position i in the internal array of track
  if (!fMuonTracks) return 0x0;
  if (!MoveMuonObjects()) return 0x0;
  IlcESDMuonTrack *track = (IlcESDMuonTrack*) fMuonTracks->UncheckedAt(i);
  track->SetESDEvent(this);
  return track;
}

//______________________________________________________________________________
void IlcESDEvent::AddMuonTrack(const IlcESDMuonTrack *t) 
{
  // add a MUON track
  TClonesArray &fmu = *fMuonTracks;
  IlcESDMuonTrack *track = new(fmu[fMuonTracks->GetEntriesFast()]) IlcESDMuonTrack(*t);
  track->MoveClustersToESD(*this);
}

//______________________________________________________________________________
IlcESDMuonTrack* IlcESDEvent::NewMuonTrack() 
{
  // create a new MUON track at the end of the internal array of track
  TClonesArray &fmu = *fMuonTracks;
  return new(fmu[fMuonTracks->GetEntriesFast()]) IlcESDMuonTrack();
}

//______________________________________________________________________________
Int_t IlcESDEvent::GetNumberOfMuonClusters()
{
  // get the number of MUON clusters
  if (!fMuonClusters) return 0;
  if (!MoveMuonObjects()) return 0;
  return fMuonClusters->GetEntriesFast();
}

//______________________________________________________________________________
IlcESDMuonCluster* IlcESDEvent::GetMuonCluster(Int_t i)
{
  // get the MUON cluster at the position i in the internal array of cluster
  if (!fMuonClusters) return 0x0;
  if (!MoveMuonObjects()) return 0x0;
  return (IlcESDMuonCluster*) fMuonClusters->UncheckedAt(i);
}

//______________________________________________________________________________
IlcESDMuonCluster* IlcESDEvent::FindMuonCluster(UInt_t clusterId)
{
  // find the MUON cluster with this Id in the internal array of cluster
  if (!fMuonClusters) return 0x0;
  if (!MoveMuonObjects()) return 0x0;
  for (Int_t i = 0; i < fMuonClusters->GetEntriesFast(); i++) {
    IlcESDMuonCluster *cluster = (IlcESDMuonCluster*) fMuonClusters->UncheckedAt(i);
    if (cluster->GetUniqueID() == clusterId) return cluster;
  }
  return 0x0;
}

//______________________________________________________________________________
IlcESDMuonCluster* IlcESDEvent::NewMuonCluster() 
{
  // create a new MUON cluster at the end of the internal array of cluster
  TClonesArray &fmu = *fMuonClusters;
  return new(fmu[fMuonClusters->GetEntriesFast()]) IlcESDMuonCluster();
}

//______________________________________________________________________________
Int_t IlcESDEvent::GetNumberOfMuonPads()
{
  // get the number of MUON pads
  if (!fMuonPads) return 0;
  if (!MoveMuonObjects()) return 0;
  return fMuonPads->GetEntriesFast();
}

//______________________________________________________________________________
IlcESDMuonPad* IlcESDEvent::GetMuonPad(Int_t i)
{
  // get the MUON pad at the position i in the internal array of pad
  if (!fMuonPads) return 0x0;
  if (!MoveMuonObjects()) return 0x0;
  return (IlcESDMuonPad*) fMuonPads->UncheckedAt(i);
}

//______________________________________________________________________________
IlcESDMuonPad* IlcESDEvent::FindMuonPad(UInt_t padId)
{
  // find the MUON pad with this Id in the internal array of pad
  if (!fMuonPads) return 0x0;
  if (!MoveMuonObjects()) return 0x0;
  for (Int_t i = 0; i < fMuonPads->GetEntriesFast(); i++) {
    IlcESDMuonPad *pad = (IlcESDMuonPad*) fMuonPads->UncheckedAt(i);
    if (pad->GetUniqueID() == padId) return pad;
  }
  return 0x0;
}

//______________________________________________________________________________
IlcESDMuonPad* IlcESDEvent::NewMuonPad() 
{
  // create a new MUON pad at the end of the internal array of pad
  TClonesArray &fmu = *fMuonPads;
  return new(fmu[fMuonPads->GetEntriesFast()]) IlcESDMuonPad();
}

void IlcESDEvent::AddPmdTrack(const IlcESDPmdTrack *t) 
{
  TClonesArray &fpmd = *fPmdTracks;
  new(fpmd[fPmdTracks->GetEntriesFast()]) IlcESDPmdTrack(*t);
}

void IlcESDEvent::SetTrdTrigger(const IlcESDTrdTrigger *t)
{
  *fTrdTrigger = *t;
}

void IlcESDEvent::AddTrdTrack(const IlcESDTrdTrack *t) 
{
  TClonesArray &ftrd = *fTrdTracks;
  new(ftrd[fTrdTracks->GetEntriesFast()]) IlcESDTrdTrack(*t);
}

void IlcESDEvent::AddTrdTracklet(const IlcESDTrdTracklet *trkl)
{
  new ((*fTrdTracklets)[fTrdTracklets->GetEntriesFast()]) IlcESDTrdTracklet(*trkl);
}

Int_t IlcESDEvent::AddKink(const IlcESDkink *c) 
{
    // Add kink
    TClonesArray &fk = *fKinks;
    IlcESDkink * kink = new(fk[fKinks->GetEntriesFast()]) IlcESDkink(*c);
    kink->SetID(fKinks->GetEntriesFast()); // CKB different from the other imps..
    return fKinks->GetEntriesFast()-1;
}


void IlcESDEvent::AddCascade(const IlcESDcascade *c) 
{
  TClonesArray &fc = *fCascades;
  new(fc[fCascades->GetEntriesFast()]) IlcESDcascade(*c);
}

void IlcESDEvent::AddCosmicTrack(const IlcESDCosmicTrack *t) 
{
  TClonesArray &ft = *fCosmicTracks;
  new(ft[fCosmicTracks->GetEntriesFast()]) IlcESDCosmicTrack(*t);
} 


Int_t IlcESDEvent::AddCaloCluster(const IlcESDCaloCluster *c) 
{
    // Add calocluster
    TClonesArray &fc = *fCaloClusters;
    IlcESDCaloCluster *clus = new(fc[fCaloClusters->GetEntriesFast()]) IlcESDCaloCluster(*c);
    clus->SetID(fCaloClusters->GetEntriesFast()-1);
    return fCaloClusters->GetEntriesFast()-1;
  }


void  IlcESDEvent::AddRawDataErrorLog(const IlcRawDataErrorLog *log) const {
  TClonesArray &errlogs = *fErrorLogs;
  new(errlogs[errlogs.GetEntriesFast()])  IlcRawDataErrorLog(*log);
}

void IlcESDEvent::SetZDCData(const IlcESDZDC * obj)
{ 
  // use already allocated space
  if(fESDZDC)
    *fESDZDC = *obj;
}

void  IlcESDEvent::SetPrimaryVertexTPC(const IlcESDVertex *vertex) 
{
  // Set the TPC vertex
  // use already allocated space
  if(fTPCVertex){
    *fTPCVertex = *vertex;
    fTPCVertex->SetName(fgkESDListName[kTPCVertex]);
  }
}

void  IlcESDEvent::SetPrimaryVertexSPD(const IlcESDVertex *vertex) 
{
  // Set the SPD vertex
  // use already allocated space
  if(fSPDVertex){
    *fSPDVertex = *vertex;
    fSPDVertex->SetName(fgkESDListName[kSPDVertex]);
  }
}

void  IlcESDEvent::SetPrimaryVertexTracks(const IlcESDVertex *vertex) 
{
  // Set the primary vertex reconstructed using he ESD tracks.
  // use already allocated space
  if(fPrimaryVertex){
    *fPrimaryVertex = *vertex;
    fPrimaryVertex->SetName(fgkESDListName[kPrimaryVertex]);
  }
}

const IlcESDVertex * IlcESDEvent::GetPrimaryVertex() const 
{
  //
  // Get the "best" available reconstructed primary vertex.
  //
  if(fPrimaryVertex){
    if (fPrimaryVertex->GetStatus()) return fPrimaryVertex;
  }
  if(fSPDVertex){
    if (fSPDVertex->GetStatus()) return fSPDVertex;
  }
  if(fTPCVertex) return fTPCVertex;
  
  IlcWarning("No primary vertex available. Returning the \"default\"...");
  return fSPDVertex;
}

IlcESDVertex * IlcESDEvent::PrimaryVertexTracksUnconstrained() const 
{
  //
  // Removes diamond constraint from fPrimaryVertex (reconstructed with tracks)
  // Returns a IlcESDVertex which has to be deleted by the user
  //
  if(!fPrimaryVertex) {
    IlcWarning("No primary vertex from tracks available.");
    return 0;
  }
  if(!fPrimaryVertex->GetStatus()) {
    IlcWarning("No primary vertex from tracks available.");
    return 0;
  }

  IlcVertexerTracks vertexer(GetMagneticField());
  Float_t diamondxyz[3]={(Float_t)GetDiamondX(),(Float_t)GetDiamondY(),0.};
  Float_t diamondcovxy[3]; GetDiamondCovXY(diamondcovxy);
  Float_t diamondcov[6]={diamondcovxy[0],diamondcovxy[1],diamondcovxy[2],0.,0.,7.};
  IlcESDVertex *vertex = 
    (IlcESDVertex*)vertexer.RemoveConstraintFromVertex(fPrimaryVertex,diamondxyz,diamondcov);

  return vertex;
}

void IlcESDEvent::SetMultiplicity(const IlcMultiplicity *mul) 
{
  // Set the SPD Multiplicity
  if(fSPDMult){
    *fSPDMult = *mul;
  }
}


void IlcESDEvent::SetFMDData(IlcESDFMD * obj) 
{ 
  // use already allocated space
  if(fESDFMD){
    *fESDFMD = *obj;
  }
}

void IlcESDEvent::SetVZEROData(const IlcESDVZERO * obj)
{ 
  // use already allocated space
  if(fESDVZERO)
    *fESDVZERO = *obj;
}

void IlcESDEvent::SetTZEROData(const IlcESDTZERO * obj)
{ 
  // use already allocated space
  if(fESDTZERO)
    *fESDTZERO = *obj;
}

#ifdef MFT_UPGRADE
//void IlcESDEvent::SetMFTData(IlcESDMFT * obj)
//{ 
//  if(fESDMFT)
//	*fESDMFT = *obj;
//}
#endif

void IlcESDEvent::SetACORDEData(IlcESDACORDE * obj)
{
  if(fESDACORDE)
    *fESDACORDE = *obj;
}


void IlcESDEvent::GetESDfriend(IlcESDfriend *ev) const 
{
  //
  // Extracts the complementary info from the ESD
  //
  if (!ev) return;

  Int_t ntrk=GetNumberOfTracks();

  for (Int_t i=0; i<ntrk; i++) {
    IlcESDtrack *t=GetTrack(i);
    if (!t) {IlcFatal(Form("NULL pointer for ESD track %d",i));}
    const IlcESDfriendTrack *f=t->GetFriendTrack();
    ev->AddTrack(f);

    t->ReleaseESDfriendTrack();// Not to have two copies of "friendTrack"

  }

  IlcESDfriend *fr = (IlcESDfriend*)(const_cast<IlcESDEvent*>(this)->FindListObject("IlcESDfriend"));
  if (fr) ev->SetVZEROfriend(fr->GetVZEROfriend());
}

void IlcESDEvent::AddObject(TObject* obj) 
{
  // Add an object to the list of object.
  // Please be aware that in order to increase performance you should
  // refrain from using TObjArrays (if possible). Use TClonesArrays, instead.
  fESDObjects->SetOwner(kTRUE);
  fESDObjects->AddLast(obj);
}


void IlcESDEvent::GetStdContent() 
{
  // set pointers for standard content
  // get by name much safer and not a big overhead since not called very often
 
  fESDRun = (IlcESDRun*)fESDObjects->FindObject(fgkESDListName[kESDRun]);
  fHeader = (IlcESDHeader*)fESDObjects->FindObject(fgkESDListName[kHeader]);
  fESDZDC = (IlcESDZDC*)fESDObjects->FindObject(fgkESDListName[kESDZDC]);
  fESDFMD = (IlcESDFMD*)fESDObjects->FindObject(fgkESDListName[kESDFMD]);
  fESDVZERO = (IlcESDVZERO*)fESDObjects->FindObject(fgkESDListName[kESDVZERO]);
  fESDTZERO = (IlcESDTZERO*)fESDObjects->FindObject(fgkESDListName[kESDTZERO]);
  fTPCVertex = (IlcESDVertex*)fESDObjects->FindObject(fgkESDListName[kTPCVertex]);
  fSPDVertex = (IlcESDVertex*)fESDObjects->FindObject(fgkESDListName[kSPDVertex]);
  fPrimaryVertex = (IlcESDVertex*)fESDObjects->FindObject(fgkESDListName[kPrimaryVertex]);
  fSPDMult =       (IlcMultiplicity*)fESDObjects->FindObject(fgkESDListName[kSPDMult]);
  fPHOSTrigger = (IlcESDCaloTrigger*)fESDObjects->FindObject(fgkESDListName[kPHOSTrigger]);
  fEMCALTrigger = (IlcESDCaloTrigger*)fESDObjects->FindObject(fgkESDListName[kEMCALTrigger]);
  fSPDPileupVertices = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kSPDPileupVertices]);
  fTrkPileupVertices = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kTrkPileupVertices]);
  fTracks = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kTracks]);
  fMuonTracks = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kMuonTracks]);
  fMuonClusters = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kMuonClusters]);
  fMuonPads = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kMuonPads]);
  fPmdTracks = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kPmdTracks]);
  fTrdTrigger = (IlcESDTrdTrigger*)fESDObjects->FindObject(fgkESDListName[kTrdTrigger]);
  fTrdTracks = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kTrdTracks]);
  fTrdTracklets = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kTrdTracklets]);
  fV0s = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kV0s]);
  fCascades = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kCascades]);
  fKinks = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kKinks]);
  fCaloClusters = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kCaloClusters]);
  fEMCALCells = (IlcESDCaloCells*)fESDObjects->FindObject(fgkESDListName[kEMCALCells]);
  fPHOSCells = (IlcESDCaloCells*)fESDObjects->FindObject(fgkESDListName[kPHOSCells]);
  fErrorLogs = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kErrorLogs]);
  fESDACORDE = (IlcESDACORDE*)fESDObjects->FindObject(fgkESDListName[kESDACORDE]);
  fTOFHeader = (IlcTOFHeader*)fESDObjects->FindObject(fgkESDListName[kTOFHeader]);
  fCosmicTracks = (TClonesArray*)fESDObjects->FindObject(fgkESDListName[kCosmicTracks]);
  #ifdef MFT_UPGRADE
 // fESDMFT = (IlcESDMFT*)fESDObjects->FindObject(fgkESDListName[kESDMFT]);
  #endif
}

void IlcESDEvent::SetStdNames(){
  // Set the names of the standard contents
  // 
  if(fESDObjects->GetEntries()>=kESDListN){
    for(int i = 0;i < fESDObjects->GetEntries() && i<kESDListN;i++){
      TObject *fObj = fESDObjects->At(i);
      if(fObj->InheritsFrom("TNamed")){
	((TNamed*)fObj)->SetName(fgkESDListName[i]);
      }
      else if(fObj->InheritsFrom("TClonesArray")){
	((TClonesArray*)fObj)->SetName(fgkESDListName[i]);
      }
    }
  }
  else{
     IlcWarning("Std Entries missing");
  }
} 


void IlcESDEvent::CreateStdContent(Bool_t bUseThisList){
  fUseOwnList = bUseThisList;
  CreateStdContent();
}

void IlcESDEvent::CreateStdContent() 
{
  // create the standard AOD content and set pointers

  // create standard objects and add them to the TList of objects
  AddObject(new IlcESDRun());
  AddObject(new IlcESDHeader());
  AddObject(new IlcESDZDC());
  AddObject(new IlcESDFMD());
  AddObject(new IlcESDVZERO());
  AddObject(new IlcESDTZERO());
  AddObject(new IlcESDVertex());
  AddObject(new IlcESDVertex());
  AddObject(new IlcESDVertex());
  AddObject(new IlcMultiplicity());
  AddObject(new IlcESDCaloTrigger());
  AddObject(new IlcESDCaloTrigger());
  AddObject(new TClonesArray("IlcESDVertex",0));
  AddObject(new TClonesArray("IlcESDVertex",0));
  AddObject(new TClonesArray("IlcESDtrack",0));
  AddObject(new TClonesArray("IlcESDMuonTrack",0));
  AddObject(new TClonesArray("IlcESDMuonCluster",0));
  AddObject(new TClonesArray("IlcESDMuonPad",0));
  AddObject(new TClonesArray("IlcESDPmdTrack",0));
  AddObject(new IlcESDTrdTrigger());
  AddObject(new TClonesArray("IlcESDTrdTrack",0));
  AddObject(new TClonesArray("IlcESDTrdTracklet",0));
  AddObject(new TClonesArray("IlcESDv0",0));
  AddObject(new TClonesArray("IlcESDcascade",0));
  AddObject(new TClonesArray("IlcESDkink",0));
  AddObject(new TClonesArray("IlcESDCaloCluster",0));
  AddObject(new IlcESDCaloCells());
  AddObject(new IlcESDCaloCells());
  AddObject(new TClonesArray("IlcRawDataErrorLog",0));
  AddObject(new IlcESDACORDE()); 
  AddObject(new IlcTOFHeader());
  AddObject(new TClonesArray("IlcESDCosmicTrack",0));
  #ifdef MFT_UPGRADE
  //AddObject(new IlcESDMFT());
  #endif
	
  // check the order of the indices against enum...

  // set names
  SetStdNames();
  // read back pointers
  GetStdContent();
}

void IlcESDEvent::CompleteStdContent() 
{
  // create missing standard objects and add them to the TList of objects
  
  // add new MUON containers if missing (for backward compatibility)
  if (!fESDObjects->FindObject(fgkESDListName[kMuonClusters])) {
    TClonesArray* muonClusters = new TClonesArray("IlcESDMuonCluster",0);
    muonClusters->SetName(fgkESDListName[kMuonClusters]);
    fESDObjects->AddAt(muonClusters, kMuonClusters);
    fESDObjects->SetOwner(kTRUE);
  }
  if (!fESDObjects->FindObject(fgkESDListName[kMuonPads])) {
    TClonesArray* muonPads = new TClonesArray("IlcESDMuonPad",0);
    muonPads->SetName(fgkESDListName[kMuonPads]);
    fESDObjects->AddAt(muonPads, kMuonPads);
    fESDObjects->SetOwner(kTRUE);
  }
}

TObject* IlcESDEvent::FindListObject(const char *name) const {
//
// Find object with name "name" in the list of branches
//
  if(fESDObjects){
    return fESDObjects->FindObject(name);
  }
  return 0;
} 

Int_t IlcESDEvent::GetPHOSClusters(TRefArray *clusters) const
{
  // fills the provided TRefArray with all found phos clusters
  
  clusters->Clear();
  
  IlcESDCaloCluster *cl = 0;
  for (Int_t i = 0; i < GetNumberOfCaloClusters(); i++) {
    
    if ( (cl = GetCaloCluster(i)) ) {
      if (cl->IsPHOS()){
	clusters->Add(cl);
	IlcDebug(1,Form("IsPHOS cluster %d Size: %d \n",i,clusters->GetEntriesFast()));
      }
    }
  }
  return clusters->GetEntriesFast();
}

Int_t IlcESDEvent::GetEMCALClusters(TRefArray *clusters) const
{
  // fills the provided TRefArray with all found emcal clusters

  clusters->Clear();

  IlcESDCaloCluster *cl = 0;
  for (Int_t i = 0; i < GetNumberOfCaloClusters(); i++) {

    if ( (cl = GetCaloCluster(i)) ) {
      if (cl->IsEMCAL()){
	clusters->Add(cl);
	IlcDebug(1,Form("IsEMCAL cluster %d Size: %d \n",i,clusters->GetEntriesFast()));
      }
    }
  }
  return clusters->GetEntriesFast();
}

void IlcESDEvent::WriteToTree(TTree* tree) const {
  // Book the branches as in TTree::Branch(TCollection*)
  // but add a "." at the end of top level branches which are
  // not a TClonesArray


  TString branchname;
  TIter next(fESDObjects);
  const Int_t kSplitlevel = 99; // default value in TTree::Branch()
  const Int_t kBufsize = 32000; // default value in TTree::Branch()
  TObject *obj = 0;

  while ((obj = next())) {
    branchname.Form("%s", obj->GetName());
    if(branchname.CompareTo("IlcESDfriend")==0)branchname = "ESDfriend.";
    if ((kSplitlevel > 1) &&  !obj->InheritsFrom(TClonesArray::Class())) {
      if(!branchname.EndsWith("."))branchname += ".";
    }
    if (!tree->FindBranch(branchname)) {
      // For the custom streamer to be called splitlevel
      // has to be negative, only needed for HLT
      Int_t splitLevel = (TString(obj->ClassName()) == "IlcHLTGlobalTriggerDecision") ? -1 : kSplitlevel - 1;
      tree->Bronch(branchname, obj->ClassName(), fESDObjects->GetObjectRef(obj),kBufsize, splitLevel);
    }
  }
}


void IlcESDEvent::ReadFromTree(TTree *tree, Option_t* opt){
//
// Connect the ESDEvent to a tree
//
  if(!tree){
    IlcWarning("IlcESDEvent::ReadFromTree() Zero Pointer to Tree \n");
    return;
  }
  // load the TTree
  if(!tree->GetTree())tree->LoadTree(0);

  // if we find the "ESD" branch on the tree we do have the old structure
  if(tree->GetBranch("ESD")) {
    fOldMuonStructure = kFALSE;
    char ** address  = (char **)(tree->GetBranch("ESD")->GetAddress());
    // do we have the friend branch
    TBranch * esdFB = tree->GetBranch("ESDfriend.");
    char ** addressF = 0;
    if(esdFB)addressF = (char **)(esdFB->GetAddress());
    if (!address) {
      IlcInfo("IlcESDEvent::ReadFromTree() Reading old Tree");
      tree->SetBranchAddress("ESD",       &fESDOld);
      if(esdFB){
	tree->SetBranchAddress("ESDfriend.",&fESDFriendOld);
      }
    } else {
      IlcInfo("IlcESDEvent::ReadFromTree() Reading old Tree");
      IlcInfo("Branch already connected. Using existing branch address.");
      fESDOld       = (IlcESD*)       (*address);
      // addressF can still be 0, since branch needs to switched on
      if(addressF)fESDFriendOld = (IlcESDfriend*) (*addressF);
    }
				       
    //  have already connected the old ESD structure... ?
    // reuse also the pointer of the AlliESDEvent
    // otherwise create new ones
    TList* connectedList = (TList*) (tree->GetUserInfo()->FindObject("ESDObjectsConnectedToTree"));
  
    if(connectedList){
      // If connected use the connected list of objects
      if(fESDObjects!= connectedList){
	// protect when called twice 
	fESDObjects->Delete();
	fESDObjects = connectedList;
      }
      GetStdContent(); 

      
      // The pointer to the friend changes when called twice via InitIO
      // since IlcESDEvent is deleted
      TObject* oldf = FindListObject("IlcESDfriend");
      TObject* newf = 0;
      if(addressF){
	newf = (TObject*)*addressF;
      }
      if(newf!=0&&oldf!=newf){
	// remove the old reference
	// Should we also delete it? Or is this handled in TTree I/O
	// since it is created by the first SetBranchAddress
	fESDObjects->Remove(oldf);
	// add the new one 
	fESDObjects->Add(newf);
      }
      
      fConnected = true;
      return;
    }
    // else...    
    CreateStdContent(); // create for copy
    // if we have the esdfriend add it, so we always can access it via the userinfo
    if(fESDFriendOld)AddObject(fESDFriendOld);
    // we are not owner of the list objects 
    // must not delete it
    fESDObjects->SetOwner(kTRUE);
    fESDObjects->SetName("ESDObjectsConnectedToTree");
    tree->GetUserInfo()->Add(fESDObjects);
    fConnected = true;
    return;
  }
  

    delete fESDOld;
    fESDOld = 0;
  // Try to find IlcESDEvent
  IlcESDEvent *esdEvent = 0;
  esdEvent = (IlcESDEvent*)tree->GetTree()->GetUserInfo()->FindObject("IlcESDEvent");
  if(esdEvent){   
      // Check if already connected to tree
    esdEvent->Reset();
    TList* connectedList = (TList*) (tree->GetUserInfo()->FindObject("ESDObjectsConnectedToTree"));

    
    if (connectedList && (strcmp(opt, "reconnect"))) {
      // If connected use the connected list if objects
      fESDObjects->Delete();
      fESDObjects = connectedList;
      GetStdContent(); 
      fOldMuonStructure = fESDObjects->TestBit(BIT(23));
      fConnected = true;
      return;
    }

    // Connect to tree
    // prevent a memory leak when reading back the TList
    // if (!(strcmp(opt, "reconnect"))) fESDObjects->Delete();
    
    if(!fUseOwnList){
      // create a new TList from the UserInfo TList... 
      // copy constructor does not work...
      fESDObjects = (TList*)(esdEvent->GetList()->Clone());
      fESDObjects->SetOwner(kTRUE);
    }
    else if ( fESDObjects->GetEntries()==0){
      // at least create the std content if we want to read to our list
      CreateStdContent(); 
    }

    // in principle
    // we only need new things in the list if we do no already have it..
    // TODO just add new entries
    CompleteStdContent();

    if(fESDObjects->GetEntries()<kESDListN){
      IlcWarning(Form("IlcESDEvent::ReadFromTree() TList contains less than the standard contents %d < %d \n",
		      fESDObjects->GetEntries(),kESDListN));
    }
    // set the branch addresses
    fOldMuonStructure = kFALSE;
    TIter next(fESDObjects);
    TNamed *el;
    while((el=(TNamed*)next())){
      TString bname(el->GetName());
      if(bname.CompareTo("IlcESDfriend")==0)
	{
	  // IlcESDfriend does not have a name ...
	    TBranch *br = tree->GetBranch("ESDfriend.");
	    if (br) tree->SetBranchAddress("ESDfriend.",fESDObjects->GetObjectRef(el));
	}
      else{
	// check if branch exists under this Name
        TBranch *br = tree->GetBranch(bname.Data());
        if(br){
          tree->SetBranchAddress(bname.Data(),fESDObjects->GetObjectRef(el));
        }
        else{
          br = tree->GetBranch(Form("%s.",bname.Data()));
          if(br){
            tree->SetBranchAddress(Form("%s.",bname.Data()),fESDObjects->GetObjectRef(el));
          }
          else{
            IlcWarning(Form("IlcESDEvent::ReadFromTree() No Branch found with Name %s or %s.",bname.Data(),bname.Data()));
	    if (bname == fgkESDListName[kMuonClusters]) {
	      fOldMuonStructure = kTRUE;
	    }
          }

	}
      }
    }
    GetStdContent();
    // when reading back we are not owner of the list 
    // must not delete it
    fESDObjects->SetOwner(kTRUE);
    fESDObjects->SetName("ESDObjectsConnectedToTree");
    fESDObjects->SetBit(BIT(23), fOldMuonStructure);
    // we are not owner of the list objects 
    // must not delete it
    tree->GetUserInfo()->Add(fESDObjects);
    tree->GetUserInfo()->SetOwner(kFALSE);
    fConnected = true;
  }// no esdEvent -->
  else {
    // we can't get the list from the user data, create standard content
    // and set it by hand (no ESDfriend at the moment
    CreateStdContent();
    fOldMuonStructure = kFALSE;
    TIter next(fESDObjects);
    TNamed *el;
    while((el=(TNamed*)next())){
      TString bname(el->GetName());    
      TBranch *br = tree->GetBranch(bname.Data());
      if(br){
	tree->SetBranchAddress(bname.Data(),fESDObjects->GetObjectRef(el));
      }
      else{
	br = tree->GetBranch(Form("%s.",bname.Data()));
	if(br){
	  tree->SetBranchAddress(Form("%s.",bname.Data()),fESDObjects->GetObjectRef(el));
	}
	else if (bname == fgkESDListName[kMuonClusters]) {
	  fOldMuonStructure = kTRUE;
	}
      }
    }
    GetStdContent();
    // when reading back we are not owner of the list 
    // must not delete it
    fESDObjects->SetOwner(kTRUE);
  }
}


void IlcESDEvent::CopyFromOldESD()
{
  // Method which copies over everthing from the old esd structure to the 
  // new  
  if(fESDOld){
    ResetStdContent();
     // Run
    SetRunNumber(fESDOld->GetRunNumber());
    SetPeriodNumber(fESDOld->GetPeriodNumber());
    SetMagneticField(fESDOld->GetMagneticField());
  
    // leave out diamond ...
    // SetDiamond(const IlcESDVertex *vertex) { fESDRun->SetDiamond(vertex);}

    // header
    SetTriggerMask(fESDOld->GetTriggerMask());
    SetOrbitNumber(fESDOld->GetOrbitNumber());
    SetTimeStamp(fESDOld->GetTimeStamp());
    SetEventType(fESDOld->GetEventType());
    SetEventNumberInFile(fESDOld->GetEventNumberInFile());
    SetBunchCrossNumber(fESDOld->GetBunchCrossNumber());
    SetTriggerCluster(fESDOld->GetTriggerCluster());

    // ZDC

    SetZDC(fESDOld->GetZDCN1Energy(),
           fESDOld->GetZDCP1Energy(),
           fESDOld->GetZDCEMEnergy(),
           0,
           fESDOld->GetZDCN2Energy(),
           fESDOld->GetZDCP2Energy(),
           fESDOld->GetZDCParticipants(),
	   0,
	   0,
	   0,
	   0,
	   0,
	   0);

    // FMD
    
    if(fESDOld->GetFMDData())SetFMDData(fESDOld->GetFMDData());

    // T0

    SetT0zVertex(fESDOld->GetT0zVertex());
    SetT0(fESDOld->GetT0());
    //  leave amps out

    // VZERO
    if (fESDOld->GetVZEROData()) SetVZEROData(fESDOld->GetVZEROData());

    if(fESDOld->GetVertex())SetPrimaryVertexSPD(fESDOld->GetVertex());

    if(fESDOld->GetPrimaryVertex())SetPrimaryVertexTracks(fESDOld->GetPrimaryVertex());

    if(fESDOld->GetMultiplicity())SetMultiplicity(fESDOld->GetMultiplicity());

    for(int i = 0;i<fESDOld->GetNumberOfTracks();i++){
      AddTrack(fESDOld->GetTrack(i));
    }

    for(int i = 0;i<fESDOld->GetNumberOfMuonTracks();i++){
      AddMuonTrack(fESDOld->GetMuonTrack(i));
    }

    for(int i = 0;i<fESDOld->GetNumberOfPmdTracks();i++){
      AddPmdTrack(fESDOld->GetPmdTrack(i));
    }

    for(int i = 0;i<fESDOld->GetNumberOfTrdTracks();i++){
      AddTrdTrack(fESDOld->GetTrdTrack(i));
    }

    for(int i = 0;i<fESDOld->GetNumberOfV0s();i++){
      AddV0(fESDOld->GetV0(i));
    }

    for(int i = 0;i<fESDOld->GetNumberOfCascades();i++){
      AddCascade(fESDOld->GetCascade(i));
    }

    for(int i = 0;i<fESDOld->GetNumberOfKinks();i++){
      AddKink(fESDOld->GetKink(i));
    }


    for(int i = 0;i<fESDOld->GetNumberOfCaloClusters();i++){
      AddCaloCluster(fESDOld->GetCaloCluster(i));
    }
	  
	#ifdef MFT_UPGRADE  
	// MFT
//	if (fESDOld->GetMFTData()) SetMFTData(fESDOld->GetMFTData());
    #endif

  }// if fesdold
}

Bool_t IlcESDEvent::IsEventSelected(const char *trigExpr) const
{
  // Check if the event satisfies the trigger
  // selection expression trigExpr.
  // trigExpr can be any logical expression
  // of the trigger classes defined in IlcESDRun
  // In case of wrong syntax return kTRUE.

  TString expr(trigExpr);
  if (expr.IsNull()) return kTRUE;

  ULong64_t mask = GetTriggerMask();
  for(Int_t itrig = 0; itrig < IlcESDRun::kNTriggerClasses; itrig++) {
    if (mask & (1ull << itrig)) {
      expr.ReplaceAll(GetESDRun()->GetTriggerClass(itrig),"1");
    }
    else {
      expr.ReplaceAll(GetESDRun()->GetTriggerClass(itrig),"0");
    }
  }

  Int_t error;
  if ((gROOT->ProcessLineFast(expr.Data(),&error) == 0) &&
      (error == TInterpreter::kNoError)) {
    return kFALSE;
  }

  return kTRUE;

}

TObject*  IlcESDEvent::GetHLTTriggerDecision() const
{
  // get the HLT trigger decission object

  // cast away const'nes because the FindListObject method
  // is not const
  IlcESDEvent* pNonConst=const_cast<IlcESDEvent*>(this);
  return pNonConst->FindListObject("HLTGlobalTrigger");
}

TString   IlcESDEvent::GetHLTTriggerDescription() const
{
  // get the HLT trigger decission description
  TString description;
  TObject* pDecision=GetHLTTriggerDecision();
  if (pDecision) {
    description=pDecision->GetTitle();
  }

  return description;
}

Bool_t    IlcESDEvent::IsHLTTriggerFired(const char* name) const
{
  // get the HLT trigger decission description
  TObject* pDecision=GetHLTTriggerDecision();
  if (!pDecision) return kFALSE;

  Option_t* option=pDecision->GetOption();
  if (option==NULL || *option!='1') return kFALSE;

  if (name) {
    TString description=GetHLTTriggerDescription();
    Int_t index=description.Index(name);
    if (index<0) return kFALSE;
    index+=strlen(name);
    if (index>=description.Length()) return kFALSE;
    if (description[index]!=0 && description[index]!=' ') return kFALSE;
  }
  return kTRUE;
}

//______________________________________________________________________________
Bool_t  IlcESDEvent::IsPileupFromSPD(Int_t minContributors, 
				     Double_t minZdist, 
				     Double_t nSigmaZdist, 
				     Double_t nSigmaDiamXY, 
				     Double_t nSigmaDiamZ) const{
  //
  // This function checks if there was a pile up
  // reconstructed with SPD
  //
  Int_t nc1=fSPDVertex->GetNContributors();
  if(nc1<1) return kFALSE;
  Int_t nPileVert=GetNumberOfPileupVerticesSPD();
  if(nPileVert==0) return kFALSE;
  
  for(Int_t i=0; i<nPileVert;i++){
    const IlcESDVertex* pv=GetPileupVertexSPD(i);
    Int_t nc2=pv->GetNContributors();
    if(nc2>=minContributors){
      Double_t z1=fSPDVertex->GetZ();
      Double_t z2=pv->GetZ();
      Double_t distZ=TMath::Abs(z2-z1);
      Double_t distZdiam=TMath::Abs(z2-GetDiamondZ());
      Double_t cutZdiam=nSigmaDiamZ*TMath::Sqrt(GetSigma2DiamondZ());
      if(GetSigma2DiamondZ()<0.0001)cutZdiam=99999.; //protection for missing z diamond information
      if(distZ>minZdist && distZdiam<cutZdiam){
	Double_t x2=pv->GetX();
	Double_t y2=pv->GetY();
	Double_t distXdiam=TMath::Abs(x2-GetDiamondX());
	Double_t distYdiam=TMath::Abs(y2-GetDiamondY());
	Double_t cov1[6],cov2[6];	
	fSPDVertex->GetCovarianceMatrix(cov1);
	pv->GetCovarianceMatrix(cov2);
	Double_t errxDist=TMath::Sqrt(cov2[0]+GetSigma2DiamondX());
	Double_t erryDist=TMath::Sqrt(cov2[2]+GetSigma2DiamondY());
	Double_t errzDist=TMath::Sqrt(cov1[5]+cov2[5]);
	Double_t cutXdiam=nSigmaDiamXY*errxDist;
	if(GetSigma2DiamondX()<0.0001)cutXdiam=99999.; //protection for missing diamond information
	Double_t cutYdiam=nSigmaDiamXY*erryDist;
	if(GetSigma2DiamondY()<0.0001)cutYdiam=99999.; //protection for missing diamond information
	if( (distXdiam<cutXdiam) && (distYdiam<cutYdiam) && (distZ>nSigmaZdist*errzDist) ){
	  return kTRUE;
	}
      }
    }
  }
  return kFALSE;
}

//______________________________________________________________________________
void IlcESDEvent::EstimateMultiplicity(Int_t &tracklets, Int_t &trITSTPC, Int_t &trITSSApure, Double_t eta, Bool_t useDCAFlag,Bool_t useV0Flag) const
{
  //
  // calculates 3 estimators for the multiplicity in the -eta:eta range
  // tracklets   : using SPD tracklets only
  // trITSTPC    : using TPC/ITS + complementary ITS SA tracks + tracklets from clusters not used by tracks
  // trITSSApure : using ITS standalone tracks + tracklets from clusters not used by tracks
  // if useDCAFlag is true: account for the ESDtrack flag marking the tracks with large DCA
  // if useV0Flag  is true: account for the ESDtrack flag marking conversion and K0's V0s
  tracklets = trITSSApure = trITSTPC = 0;
  int ntr = fSPDMult ? fSPDMult->GetNumberOfTracklets() : 0;
  //
  // count tracklets
  for (int itr=ntr;itr--;) { 
    if (TMath::Abs(fSPDMult->GetEta(itr))>eta) continue;
    tracklets++;
    if (fSPDMult->FreeClustersTracklet(itr,0)) trITSTPC++;    // not used in ITS/TPC or ITS_SA track
    if (fSPDMult->FreeClustersTracklet(itr,1)) trITSSApure++; // not used in ITS_SA_Pure track
  }
  //
  // count real tracks
  ntr = GetNumberOfTracks();
  for (int itr=ntr;itr--;) {
    IlcESDtrack *t = GetTrack(itr);
    if (!t) {IlcFatal(Form("NULL pointer for ESD track %d",itr));}
    if (TMath::Abs(t->Eta())>eta) continue;
    if (!t->IsOn(IlcESDtrack::kITSin)) continue;
    if (useDCAFlag && t->IsOn(IlcESDtrack::kMultSec))  continue;
    if (useV0Flag  && t->IsOn(IlcESDtrack::kMultInV0)) continue;    
    if (t->IsOn(IlcESDtrack::kITSpureSA)) trITSSApure++;
    else                                  trITSTPC++;
  }
  //
}

Bool_t IlcESDEvent::IsPileupFromSPDInMultBins() const {
    Int_t nTracklets=GetMultiplicity()->GetNumberOfTracklets();
    if(nTracklets<20) return IsPileupFromSPD(3,0.8);
    else if(nTracklets<50) return IsPileupFromSPD(4,0.8);
    else return IsPileupFromSPD(5,0.8);
}

void  IlcESDEvent::SetTOFHeader(const IlcTOFHeader *header)
{
  //
  // Set the TOF event_time
  //

  if (fTOFHeader) {
    *fTOFHeader=*header;
    //fTOFHeader->SetName(fgkESDListName[kTOFHeader]);
  }
  else {
    // for analysis of reconstructed events
    // when this information is not avaliable
    fTOFHeader = new IlcTOFHeader(*header);
    //AddObject(fTOFHeader);
  }

}

IlcCentrality* IlcESDEvent::GetCentrality()
{
    if (!fCentrality) fCentrality = new IlcCentrality();
    return  fCentrality;
}

IlcEventplane* IlcESDEvent::GetEventplane()
{
    if (!fEventplane) fEventplane = new IlcEventplane();
    return  fEventplane;
}

Float_t IlcESDEvent::GetVZEROEqMultiplicity(Int_t i) const
{
  // Get VZERO Multiplicity for channel i
  // Themethod uses the equalization factors
  // stored in the ESD-run object in order to
  // get equal multiplicities within a VZERO rins (1/8 of VZERO)
  if (!fESDVZERO || !fESDRun) return -1;

  Int_t ring = i/8;
  Float_t factorSum = 0;
  for(Int_t j = 8*ring; j < (8*ring+8); ++j) {
    factorSum += fESDRun->GetVZEROEqFactors(j);
  }
  Float_t factor = fESDRun->GetVZEROEqFactors(i)*8./factorSum;

  return (fESDVZERO->GetMultiplicity(i)/factor);
}
