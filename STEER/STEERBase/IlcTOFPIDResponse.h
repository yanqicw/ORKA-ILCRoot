#ifndef ILCTOFPIDRESPONSE_H
#define ILCTOFPIDRESPONSE_H

/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

//-------------------------------------------------------
//                    TOF PID class
//   Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch 
//-------------------------------------------------------

#include "TObject.h"
#include "IlcPID.h"

class IlcTOFPIDParams;

class IlcTOFPIDResponse : public TObject {
public:

  IlcTOFPIDResponse();
  IlcTOFPIDResponse(Double_t *param);
  ~IlcTOFPIDResponse(){}

  void     SetTimeResolution(Float_t res) { fSigma = res; }
  void     SetTimeZero(Double_t t0) { fTime0=t0; }
  Double_t GetTimeZero() const { return fTime0; }

  void     SetMaxMismatchProbability(Double_t p) {fPmax=p;}
  Double_t GetMaxMismatchProbability() const {return fPmax;}

  Double_t GetExpectedSigma(Float_t mom, Float_t tof, Float_t mass) const;


  Double_t GetMismatchProbability(Double_t p,Double_t mass) const;

  void     SetT0event(Float_t *t0event){for(Int_t i=0;i < fNmomBins;i++) fT0event[i] = t0event[i];};
  void     SetT0resolution(Float_t *t0resolution){for(Int_t i=0;i < fNmomBins;i++) fT0resolution[i] = t0resolution[i];};
  void     ResetT0info(){ for(Int_t i=0;i < fNmomBins;i++){ fT0event[i] = 0.0; fT0resolution[i] = 0.0; fMaskT0[i] = 0;} };
  void     SetMomBoundary();
  Int_t    GetMomBin(Float_t p) const;
  Int_t    GetNmomBins(){return fNmomBins;};
  Float_t  GetMinMom(Int_t ibin) const {if(ibin >=0 && ibin < fNmomBins) return fPCutMin[ibin]; else return 0.0;}; // overrun static array - coverity
  Float_t  GetMaxMom(Int_t ibin) const {if(ibin >=0 && ibin < fNmomBins) return fPCutMin[ibin+1]; else return 0.0;}; // overrun static array - coverity
  void     SetT0bin(Int_t ibin,Float_t t0bin){if(ibin >=0 && ibin < fNmomBins) fT0event[ibin] = t0bin;}; // overrun static array - coverity
  void     SetT0binRes(Int_t ibin,Float_t t0binRes){if(ibin >=0 && ibin < fNmomBins) fT0resolution[ibin] = t0binRes;}; // overrun static array - coverity
  void     SetT0binMask(Int_t ibin,Int_t t0binMask){if(ibin >=0 && ibin < fNmomBins) fMaskT0[ibin] = t0binMask;}; // overrun static array - coverity
  Float_t  GetT0bin(Int_t ibin) const {if(ibin >=0 && ibin < fNmomBins) return fT0event[ibin]; else return 0.0;}; // overrun static array - coverity
  Float_t  GetT0binRes(Int_t ibin) const {if(ibin >=0 && ibin < fNmomBins) return fT0resolution[ibin]; else return 0.0;}; // overrun static array - coverity
  Int_t    GetT0binMask(Int_t ibin) const {if(ibin >=0 && ibin < fNmomBins) return fMaskT0[ibin]; else return 0;}; // overrun static array - coverity

  // Get Start Time for a track
  Float_t  GetStartTime(Float_t mom) const;
  Float_t  GetStartTimeRes(Float_t mom) const;
  Int_t    GetStartTimeMask(Float_t mom) const;

  // Tracking resolution for expected times
  void SetTrackParameter(Int_t ip,Float_t value){if(ip>=0 && ip < 4) fPar[ip] = value;};
  Float_t GetTrackParameter(Int_t ip){if(ip>=0 && ip < 4) return fPar[ip]; else return -1.0;};

 private:
  Double_t fSigma;        // intrinsic TOF resolution

  // obsolete
  Double_t fPmax;         // "maximal" probability of mismathing (at ~0.5 GeV/c)
  Double_t fTime0;        // time zero
  //--------------

  // About event time (t0) info
  static const Int_t fNmomBins = 10; // number of momentum bin 
  Float_t fT0event[fNmomBins];    // t0 (best, T0, T0-TOF, ...) of the event as a function of p 
  Float_t fT0resolution[fNmomBins]; // t0 (best, T0, T0-TOF, ...) resolution as a function of p 
  Float_t fPCutMin[fNmomBins+1]; // min values for p bins
  Int_t fMaskT0[fNmomBins]; // mask withthe T0 used (0x1=T0-TOF,0x2=T0A,0x3=TOC) for p bins
  Float_t fPar[4]; // parameter for expected times resolution

  ClassDef(IlcTOFPIDResponse,4)   // TOF PID class
};

#endif
