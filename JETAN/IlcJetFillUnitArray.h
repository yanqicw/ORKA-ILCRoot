#ifndef ILCJETFILLUNITARRAY_H
#define ILCJETFILLUNITARRAY_H
 
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */
 
//---------------------------------------------------------------------
// Base class used to fill Unit Array 
// Called by ESD Reader for jet analysis
// Author: Magali Estienne (magali.estienne@subatech.in2p3.fr)
//---------------------------------------------------------------------

//#include <Riostream.h>
#include <TMath.h>
#include <TTask.h>

#include <TArrayD.h>
#include <TArrayI.h>
#include <TRefArray.h>
#include <TClonesArray.h>

#include "IlcJetReaderHeader.h"
#include "IlcJetAODReaderHeader.h"
#include "IlcPVBARGeometry.h"
#include "IlcJetGrid.h"
#include "IlcESDEvent.h"
#include "IlcAODEvent.h"
#include "IlcJetHadronCorrection.h"

class Riostream;

class IlcPVBARGeometry;
class IlcJetReader;
class IlcJetESDReader;

class IlcJetFillUnitArray : public TTask
{
 public: 
  IlcJetFillUnitArray();
  virtual ~IlcJetFillUnitArray();
  IlcJetFillUnitArray(const IlcJetFillUnitArray &det);
  IlcJetFillUnitArray &operator=(const IlcJetFillUnitArray &det);
  
  // Setter
  virtual void SetReaderHeader(IlcJetReaderHeader* const readerHeader) {fReaderHeader = readerHeader;}
  virtual void SetGeom(IlcPVBARGeometry* const geom)                     {fGeom = geom;}
  virtual void SetMomentumArray(TClonesArray* const momentumArray)     {fMomentumArray = momentumArray;}
  virtual void SetUnitArray(TClonesArray* const unitArray)             {fUnitArray = unitArray;}
  virtual void SetRefArray(TRefArray* const refArray)                  {fRefArray = refArray;}
  virtual void SetReferences(TRefArray *refArray)                      {fRef = refArray;}
  virtual void SetSignalFlag(TArrayI sflag)                            {fSignalFlag = sflag;}
  virtual void SetCutFlag(TArrayI cflag)                               {fCutFlag = cflag;}
  virtual void SetTPCGrid(IlcJetGrid* const grid)                      {fTPCGrid = grid;}
  virtual void SetEMCalGrid(IlcJetGrid* const grid)                    {fEMCalGrid = grid;}
  virtual void SetProcId(Bool_t id)                              {fProcId = id;}
  virtual void SetGrid0(IlcJetGrid */*grid0*/)                    {;}
  virtual void SetGrid1(IlcJetGrid */*grid1*/)                    {;}
  virtual void SetGrid2(IlcJetGrid */*grid2*/)                    {;}
  virtual void SetGrid3(IlcJetGrid */*grid3*/)                    {;}
  virtual void SetGrid4(IlcJetGrid */*grid4*/)                    {;}
  virtual void SetHadCorrector(IlcJetHadronCorrection* /*corr*/)  {;}
  virtual void SetApplyMIPCorrection(Bool_t /*val*/)              {;}
  virtual void SetESD(IlcESDEvent */*esd*/)                       {;}
  virtual void SetAOD(IlcAODEvent */*aod*/)                       {;}
  virtual void SetApplyElectronCorrection(Int_t /*flag*/)         {;}
  virtual void SetApplyFractionHadronicCorrection(Bool_t /*val*/) {;}
  virtual void SetFractionHadronicCorrection(Double_t /*val*/)    {;}
  virtual void SetInitMult(Int_t /*mult*/)                        {;}
  virtual void SetInitMultCut(Int_t /*multcut*/)                  {;}

  // Getter
  virtual TClonesArray* GetUnitArray() const {return fUnitArray;}
  virtual TRefArray*    GetRefArray() const {return fRefArray;}
  virtual void          GetEtaPhiFromIndex(Int_t index,Float_t &eta,Float_t &phi);
  virtual Int_t         GetNeta() const {return fNeta;}
  virtual Int_t         GetNphi() const {return fNphi;}
  virtual Int_t         GetMult() const {return 0;} // To be checked
  virtual Int_t         GetMultCut() const {return 0;} // To be checked
  virtual Bool_t        GetProcId() const {return fProcId;}
  virtual TArrayI       GetSignalFlag() const {return fSignalFlag;}
  virtual TArrayI       GetCutFlag() const {return fCutFlag;}

  // Other
  virtual void          Exec(Option_t* const /*option*/) {;}
  virtual Float_t       EtaToTheta(Float_t arg);
  virtual void          InitParameters() {;}

 protected:
  Int_t                 fNTracks;         // Number of tracks stored in UnitArray
  Int_t                 fNTracksCut;      // Number of tracks stored in UnitArray with a pt cut 
  Int_t                 fOpt;             // Detector to be used for jet reconstruction
  Bool_t                fDZ;              // Use or not dead zones
  Int_t                 fDebug;           // Debug option

  IlcJetReaderHeader   *fReaderHeader;    // ReaderHeader
  TClonesArray         *fMomentumArray;   // MomentumArray
  TClonesArray         *fUnitArray;       // UnitArray
  TRefArray            *fRefArray;        // UnitArray
  TRefArray            *fRef;             // ref Array to aod tracks
  TArrayI               fSignalFlag;      // to flag if a particle comes from pythia or
                                          // from the underlying event
  TArrayI               fCutFlag;         // to flag if a particle passed the pt cut or not
 
  Bool_t                fProcId;          // Bool_t for TProcessID synchronization  
  IlcJetGrid           *fTPCGrid;         // Define filled grid
  IlcJetGrid           *fEMCalGrid;       // Define filled grid
  IlcPVBARGeometry     *fGeom;            // Define PVBAR geometry

  Int_t                 fNphi;            // number of points in the grid:   phi
  Int_t                 fNeta;            //               "                 eta
  Int_t                 fGrid;            // Select the grid acceptance you want to fill
                                          // 0 = TPC acceptance, 1 = TPC-EMCal acceptance
  TArrayD              *fPhi2;            // grid points in phi
  TArrayD              *fEta2;            // grid points in eta
  TMatrixD             *fIndex;           // grid points in (phi,eta) 
  TMatrixD             *fParams;          // matrix of parameters in the grid points  
  Float_t               fPhiMin;          // EMCal acceptance
  Float_t               fPhiMax;          // EMCal acceptance
  Float_t               fEtaMin;          // EMCal acceptance
  Float_t               fEtaMax;          // EMCal acceptance
  Int_t                 fEtaBinInTPCAcc;  // Number of bins in Eta in TPC acceptance
  Int_t                 fPhiBinInTPCAcc;  // Number of bins in phi in TPC acceptance
  Int_t                 fEtaBinInEMCalAcc;// Number of bins in Eta in EMCal acceptance
  Int_t                 fPhiBinInEMCalAcc;// Number of bins in phi in EMCal acceptance
  Int_t                 fNbinPhi;         // Number of phi bins

 private:


  ClassDef(IlcJetFillUnitArray,1) // Fill Unit Array with tpc and/or emcal information
};

#endif
