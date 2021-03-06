#ifndef ILCPIDRESPONSE_H
#define ILCPIDRESPONSE_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

//---------------------------------------------------------------//
//        Base class for handling the pid response               //
//        functions of all detectors                             //
//        and give access to the nsigmas                         //
//                                                               //
//   Origin: Jens Wiechula, Uni Tuebingen, jens.wiechula@cern.ch //
//---------------------------------------------------------------//

#include "IlcITSPIDResponse.h"
#include "IlcTPCPIDResponse.h"
#include "IlcTRDPIDResponse.h"
#include "IlcTOFPIDResponse.h"
#include "IlcEMCALPIDResponse.h"


#include "IlcVParticle.h"
#include "IlcVTrack.h"

#include "TNamed.h"

class IlcVEvent;
class TF1;
class IlcTRDPIDResponseObject; 

class IlcPIDResponse : public TNamed {
public:
  IlcPIDResponse(Bool_t isMC=kFALSE);
  virtual ~IlcPIDResponse();

  enum EDetCode {
    kDetITS = 0x1,
    kDetTPC = 0x2,
    kDetTRD = 0x4,
    kDetTOF = 0x8,
    kDetHMPID = 0x10,
    kDetEMCAL = 0x20,
    kDetPHOS = 0x40
  };

  enum EStartTimeType_t {kFILL_T0,kTOF_T0, kT0_T0, kBest_T0};

  enum ITSPIDmethod { kITSTruncMean, kITSLikelihood };

  enum EDetPidStatus {
    kDetNoSignal=0,
    kDetPidOk=1,
    kDetMismatch=2
  };
  
  IlcITSPIDResponse &GetITSResponse() {return fITSResponse;}
  IlcTPCPIDResponse &GetTPCResponse() {return fTPCResponse;}
  IlcTOFPIDResponse &GetTOFResponse() {return fTOFResponse;}
  IlcTRDPIDResponse &GetTRDResponse() {return fTRDResponse;}
  IlcEMCALPIDResponse &GetEMCALResponse() {return fEMCALResponse;}

  Float_t NumberOfSigmas(EDetCode detCode, const IlcVParticle *track, IlcPID::EParticleType type) const;
  
  virtual Float_t NumberOfSigmasITS(const IlcVParticle *track, IlcPID::EParticleType type) const;
  virtual Float_t NumberOfSigmasTPC(const IlcVParticle *track, IlcPID::EParticleType type) const;
  virtual Float_t NumberOfSigmasEMCAL(const IlcVTrack *track, IlcPID::EParticleType type) const;
  virtual Float_t NumberOfSigmasTOF(const IlcVParticle *track, IlcPID::EParticleType type) const = 0;
  virtual Bool_t IdentifiedAsElectronTRD(const IlcVTrack *track, Double_t efficiencyLevel) const;

  EDetPidStatus ComputePIDProbability  (EDetCode detCode, const IlcVTrack *track, Int_t nSpecies, Double_t p[]) const;
  
  EDetPidStatus ComputeITSProbability  (const IlcVTrack *track, Int_t nSpecies, Double_t  p[]) const;
  EDetPidStatus ComputeTPCProbability  (const IlcVTrack *track, Int_t nSpecies, Double_t p[]) const;
  EDetPidStatus ComputeTOFProbability  (const IlcVTrack *track, Int_t nSpecies, Double_t p[]) const;
  EDetPidStatus ComputeTRDProbability  (const IlcVTrack *track, Int_t nSpecies, Double_t p[]) const;
  EDetPidStatus ComputeEMCALProbability(const IlcVTrack *track, Int_t nSpecies, Double_t p[]) const;
  EDetPidStatus ComputePHOSProbability (const IlcVTrack *track, Int_t nSpecies, Double_t p[]) const;
  EDetPidStatus ComputeHMPIDProbability(const IlcVTrack *track, Int_t nSpecies, Double_t p[]) const;

  void SetTRDPIDmethod(IlcTRDPIDResponse::ETRDPIDMethod method=IlcTRDPIDResponse::kLQ1D);
  
  void SetITSPIDmethod(ITSPIDmethod pmeth) { fITSPIDmethod = pmeth; }
  virtual void SetTOFResponse(IlcVEvent */*event*/,EStartTimeType_t /*option*/) {;}
  void SetTRDslicesForPID(UInt_t slice1, UInt_t slice2) {fTRDslicesForPID[0]=slice1;fTRDslicesForPID[1]=slice2;}
  
  void SetOADBPath(const char* path) {fOADBPath=path;}
  const char *GetOADBPath() const {return fOADBPath.Data();}
  void InitialiseEvent(IlcVEvent *event, Int_t pass);
  void SetCurrentFile(const char* file) { fCurrentFile=file; }

  IlcVEvent * GetCurrentEvent() const {return fCurrentEvent;}

  // User settings for the MC period and reco pass
  void SetMCperiod(const char *mcPeriod) {fMCperiodUser=mcPeriod;}
  void SetRecoPass(Int_t recoPass)       {fRecoPassUser=recoPass;}

  // event info
  Float_t GetCurrentCentrality() const {return fCurrCentrality;};

  // TOF setting
  void SetTOFtail(Float_t tail=1.1){if(tail > 0) fTOFtail=tail; else printf("TOF tail should be greater than 0 (nothing done)\n");};

  IlcPIDResponse(const IlcPIDResponse &other);
  IlcPIDResponse& operator=(const IlcPIDResponse &other);

  
protected:
  IlcITSPIDResponse fITSResponse;    //PID response function of the ITS
  IlcTPCPIDResponse fTPCResponse;    //PID response function of the TPC
  IlcTRDPIDResponse fTRDResponse;    //PID response function of the TRD
  IlcTOFPIDResponse fTOFResponse;    //PID response function of the TOF
  IlcEMCALPIDResponse fEMCALResponse;  //PID response function of the EMCAL

  Float_t           fRange;          // nSigma max in likelihood
  ITSPIDmethod      fITSPIDmethod;   // 0 = trunc mean; 1 = likelihood

private:
  Bool_t fIsMC;                        //  If we run on MC data

  TString fOADBPath;                   // OADB path to use
  
  TString fBeamType;                   //! beam type (PP) or (PBPB)
  TString fLHCperiod;                  //! LHC period
  TString fMCperiodTPC;                //! corresponding MC period to use for the TPC splines
  TString fMCperiodUser;               //  MC prodution requested by the user
  TString fCurrentFile;                //! name of currently processed file
  Int_t   fRecoPass;                   //! reconstruction pass
  Int_t   fRecoPassUser;               //  reconstruction pass explicitly set by the user
  Int_t   fRun;                        //! current run number
  Int_t   fOldRun;                     //! current run number
  
  TObjArray *fArrPidResponseMaster;    //!  TPC pid splines
  TF1       *fResolutionCorrection;    //! TPC resolution correction

  IlcTRDPIDResponseObject *fTRDPIDResponseObject; //! TRD PID Response Object
  UInt_t fTRDslicesForPID[2];           //! TRD PID slices

  Float_t fTOFtail;                    //! TOF tail effect used in TOF probability
  IlcTOFPIDParams *fTOFPIDParams;      //! TOF PID Params - period depending (OADB loaded)

  TObjArray *fEMCALPIDParams;             //! EMCAL PID Params

  IlcVEvent *fCurrentEvent;            //! event currently being processed

  Float_t fCurrCentrality;             //! current centrality
  
  void ExecNewRun();
  
  //
  //setup parametrisations
  //

  //ITS
  void SetITSParametrisation();
  
  //TPC
  void SetTPCPidResponseMaster();
  void SetTPCParametrisation();
  Double_t GetTPCMultiplicityBin(const IlcVEvent * const event);

  //TRD
  void SetTRDPidResponseMaster();
  void InitializeTRDResponse();

  //TOF
  void SetTOFPidResponseMaster();
  void InitializeTOFResponse();

  //EMCAL
  void SetEMCALPidResponseMaster();
  void InitializeEMCALResponse();

  //
  void SetRecoInfo();
  
  ClassDef(IlcPIDResponse,6);  //PID response handling
};

inline Float_t IlcPIDResponse::NumberOfSigmasTPC(const IlcVParticle *vtrack, IlcPID::EParticleType type) const {
  IlcVTrack *track=(IlcVTrack*)vtrack;
  Double_t mom  = track->GetTPCmomentum();
  Double_t sig  = track->GetTPCsignal();
  UInt_t   sigN = track->GetTPCsignalN();

  Double_t nSigma = -999.;
  if (sigN>0) nSigma=fTPCResponse.GetNumberOfSigmas(mom,sig,sigN,type);

  return nSigma;
}

inline Float_t IlcPIDResponse::NumberOfSigmasITS(const IlcVParticle *vtrack, IlcPID::EParticleType type) const {
  IlcVTrack *track=(IlcVTrack*)vtrack;
  Float_t dEdx=track->GetITSsignal();
  if (dEdx<=0) return -999.;
  
  UChar_t clumap=track->GetITSClusterMap();
  Int_t nPointsForPid=0;
  for(Int_t i=2; i<6; i++){
    if(clumap&(1<<i)) ++nPointsForPid;
  }
  Float_t mom=track->P();
  Bool_t isSA=kTRUE;
  if(track->GetTPCNcls()>0) isSA=kFALSE;
  return fITSResponse.GetNumberOfSigmas(mom,dEdx,type,nPointsForPid,isSA);
}

#endif
