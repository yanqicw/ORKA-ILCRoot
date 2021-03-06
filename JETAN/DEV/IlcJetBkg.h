#ifndef ILCJETBKG_H
#define ILCJETBKG_H
 
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

//--------------------------------------------------
// Method implementation for background studies and background subtraction with UA1 algorithms
//
// Author: magali.estienne@subatech.in2p3.fr
//-------------------------------------------------

class TH1F;
class TH2F;
class TList;
class IlcAODJetEventBackground;

class IlcJetBkg : public TObject
{
 public:
  IlcJetBkg();
  IlcJetBkg(const IlcJetBkg &input);
  ~IlcJetBkg();
  IlcJetBkg& operator=(const IlcJetBkg& source);
  void    SetHeader(IlcJetHeader *header)  {fHeader=header;}
  void    SetCalTrkEvent(IlcJetCalTrkEvent *evt)  {fEvent=evt;}
  Bool_t  PtCutPass(Int_t id, Int_t nTracks);
  Bool_t  SignalCutPass(Int_t id, Int_t nTracks);
  Float_t CalcJetAreaEtaCut(Float_t radius, const Float_t etaJet);
  void    CalcJetAndBckgAreaEtaCut(Bool_t calcOutsideArea, Float_t rc, const Int_t nJ, const Float_t* etaJet, Float_t* &areaJet, Float_t &areaOut);
 
  void    SubtractBackg(const Int_t& nIn, const Int_t&nJ, Float_t&EtbgTotalN, Float_t&sigmaN, 
			const Float_t* ptT, const Float_t* etaT, const Float_t* phiT, 
			Float_t* etJet, const Float_t* etaJet, const Float_t* phiJet, 
			Float_t* etsigJet, Int_t* multJetT, Int_t* multJetC, Int_t* multJet, 
			Int_t* injet, Float_t* &areaJet);
  
  void    SubtractBackgCone(const Int_t& nIn, const Int_t&nJ,Float_t& EtbgTotalN, Float_t&sigmaN,
			    const Float_t* ptT, const Float_t* etaT, const Float_t* phiT, Float_t* etJet, 
			    const Float_t* etaJet, const Float_t* phiJet, Float_t* etsigJet, 
			    Int_t* multJetT, Int_t* multJetC, Int_t* multJet, Int_t* injet, Float_t* &/*areaJet*/);

  void    SubtractBackgRatio(const Int_t& nIn, const Int_t&nJ,Float_t& EtbgTotalN, Float_t&sigmaN,
			     const Float_t* ptT, const Float_t* etaT, const Float_t* phiT, Float_t* etJet, 
			     const Float_t* etaJet, const Float_t* phiJet, Float_t* etsigJet, 
			     Int_t* multJetT, Int_t* multJetC, Int_t* multJet, Int_t* injet, Float_t* &/*areaJet*/);

  void    SubtractBackgStat(const Int_t& nIn, const Int_t&nJ,Float_t&EtbgTotalN, Float_t&sigmaN,
			    const Float_t* ptT, const Float_t* etaT, const Float_t* phiT, Float_t* etJet, 
			    const Float_t* etaJet, const Float_t* phiJet, Float_t* etsigJet, 
			    Int_t* multJetT, Int_t* multJetC, Int_t* multJet, Int_t* injet, Float_t* &areaJet);
  void    SetDebug(Int_t debug){fDebug = debug;}

  enum {kMaxJets = 60};

 private:
  //    Double_t CalcRho(vector<fastjet::PseudoJet> input_particles,Double_t RparamBkg,TString method);

  IlcJetCalTrkEvent* fEvent;    //! reader
  IlcJetHeader*      fHeader;   //! header
  Int_t              fDebug;    //  Debug option

  // temporary histos for background, reset for each event, no need to stream
  TH1F*  fhEtJet[kMaxJets];   //! histogram for background subtraction
  TH1F*  fhAreaJet[kMaxJets]; //! histogram for background subtraction (store global not to create it with every event
  TH1F*  fhEtBackg;           //! histogram for background subtraction
  TH1F*  fhAreaBackg;         //! histogram for background subtraction

  ClassDef(IlcJetBkg, 2)      // background jet analysis

};
 
#endif
