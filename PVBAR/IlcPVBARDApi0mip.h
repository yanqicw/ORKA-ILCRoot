#ifndef IlcPVBARDApi0mip_H
#define IlcPVBARDApi0mip_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* $Id: IlcPVBARDApi0mip.h 52964 2011-11-17 12:12:19Z policheh $ */

// --
// --
// Implementation for TTree output in PVBAR DA
// for calibrating energy by pi0 and MIP.
// --
// -- Author: Hisayuki Torii (Hiroshima Univ.)
// --


#include <time.h>

#include "TNamed.h"
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TFile.h"
#include "TTree.h"
#include "IlcPVBARDATreeEvent.h"

class IlcPVBARDApi0mip : public TNamed {
 public:
  IlcPVBARDApi0mip(int module,int iterid=0,const char* fopt="RECREATE");
  IlcPVBARDApi0mip(const IlcPVBARDApi0mip& da);
  IlcPVBARDApi0mip& operator= (const IlcPVBARDApi0mip&);
  ~IlcPVBARDApi0mip();
  
  void NewEvent();
  void FillDigit(float adc,int row,int col);
  void SetTime(time_t& intime){fTime=intime;};
  time_t GetTime(){return fTime;};
  void FillTree(IlcPVBARDATreeEvent* event=0);
  void FillHist(IlcPVBARDATreeEvent* event=0);
  void Print(Option_t *option="") const;

 private:
  Bool_t CreateTree();
  Bool_t CreateHist();
  Bool_t fCreateTree;           //! Flag of tree initialization
  Bool_t fCreateHist;           //! Flag of hist initialization
  Int_t  fMod;                  // Module ID [0-4] ([2-4] for 2009)
  Int_t  fIterId;               // Iteration step [0-*]
  TFile* fTFile;                //! output file
  TTree* fTTree;                //! output TTree
  IlcPVBARDATreeEvent* fEvent;   //! Contents of TTree
  Bool_t fEventClustered;       //! Flag for
  time_t fTime;                 // time
  TH1I*  fH1Time;               // x:bin1=StartTime bin2=EndTime
  TH1F*  fH1DigitNum;           // x:Number of digits
  TH1F*  fH1ClusterNum;         // x:Number of clusters
  TH2F*  fH2EneDigitId;         // x:DigitId[0-3583] y:Digit Energy
  TH2F*  fH2MipDigitId;         // x:DigitId[0-3583] y:Cluster Energy
  TH2F*  fH2Pi0DigitId;         // x:DigitId[0-3583] y:Cluster Pair Mass
  TH3F*  fH3Pi0AsymPt;          // x:asym y:pT(GeV/c) z:Cluster Pair Mass

  ClassDef(IlcPVBARDApi0mip,1)
};
#endif
