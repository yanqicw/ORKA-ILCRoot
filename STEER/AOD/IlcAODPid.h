#ifndef IlcAODPID_H
#define IlcAODPID_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* $Id: IlcAODPid.h 58259 2012-08-22 07:07:20Z hristov $ */

//-------------------------------------------------------------------------
//     AOD Pid object for additional pid information
//     Author: Annalisa Mastroserio, CERN
//-------------------------------------------------------------------------

#include <TObject.h>

class IlcTPCdEdxInfo;

class IlcAODPid : public TObject {

 public:
  IlcAODPid();
  virtual ~IlcAODPid();
  IlcAODPid(const IlcAODPid& pid); 
  IlcAODPid& operator=(const IlcAODPid& pid);
  
  enum{kSPECIES=5, kTRDnPlanes=6};

 //setters
  void      SetITSsignal(Double_t its)                         {fITSsignal=its;}
  void      SetITSdEdxSamples(const Double_t s[4]);
  void      SetVXDsignal(Double_t vxd)                         {fVXDsignal=vxd;}
  void      SetVXDdEdxSamples(const Double_t s[4]);
  void      SetTPCsignal(Double_t tpc)                         {fTPCsignal=tpc;}
  void      SetTPCsignalN(UShort_t tpcN)                       {fTPCsignalN=(UChar_t)((tpcN<160)?tpcN:160);}
  void      SetTPCmomentum(Double_t tpcMom)                    {fTPCmomentum=tpcMom;}
  void      SetTPCdEdxInfo(IlcTPCdEdxInfo * dEdxInfo);
  inline void  SetTRDsignal(Int_t nslices, const Double_t * const trdslices);  
  void      SetTRDmomentum(Int_t nplane, Float_t trdMom)       {fTRDmomentum[nplane]=trdMom;}
  inline void  SetTRDncls(UChar_t ncls, Int_t layer = -1);
  void      SetTRDntrackletsPID(UChar_t ntls) {fTRDntls = ntls;}
  void      SetTRDChi2(Double_t chi2)          {fTRDChi2 = chi2;}
  void      SetTOFsignal(Double_t tof)                         {fTOFesdsignal=tof;}
  void      SetTOFpidResolution(Double_t tofPIDres[5]);
  void      SetIntegratedTimes(Double_t timeint[5]);

  Double_t  GetITSsignal()       const {return  fITSsignal;}
  void      GetITSdEdxSamples(Double_t *s) const;
  Double_t  GetITSdEdxSample(Int_t i) const {
    if(i>=0 && i<4) return fITSdEdxSamples[i];
    else return 0.;
  }
  Double_t  GetVXDsignal()       const {return  fVXDsignal;}
  void      GetVXDdEdxSamples(Double_t *s) const;
  Double_t  GetVXDdEdxSample(Int_t i) const {
    if(i>=0 && i<4) return fVXDdEdxSamples[i];
    else return 0.;
  }
  Double_t  GetTPCsignal()       const {return  fTPCsignal;}
  UShort_t  GetTPCsignalN()      const {return  (UShort_t)fTPCsignalN;}
  IlcTPCdEdxInfo * GetTPCdEdxInfo()const{return fTPCdEdxInfo;}

  Double_t  GetTPCmomentum()     const {return  fTPCmomentum;}
  Int_t     GetTRDnSlices()      const {return  fTRDnSlices;}
  Double_t* GetTRDsignal()       const {return  fTRDslices;}
  Double_t  GetTRDChi2()         const {return fTRDChi2;}
  const Double_t*  GetTRDmomentum() const {return  fTRDmomentum;}
  UChar_t   GetTRDncls(UChar_t layer) const { if(layer > 5) return 0; return fTRDncls[layer];}
  inline UChar_t GetTRDncls() const;
  UChar_t   GetTRDntrackletsPID() const {return fTRDntls;}
  Double_t  GetTOFsignal()       const {return  fTOFesdsignal;}

  void      GetIntegratedTimes(Double_t timeint[5])  const; 
  void      GetTOFpidResolution (Double_t tofRes[5]) const;

 private :
  Double32_t  fITSsignal;        //[0.,0.,10] detector raw signal
  Double32_t  fITSdEdxSamples[4];//[0.,0.,10] ITS dE/dx samples
  Double32_t  fVXDsignal;        //[0.,0.,10] detector raw signal
  Double32_t  fVXDdEdxSamples[4];//[0.,0.,10] VXD dE/dx samples
  Double32_t  fTPCsignal;        //[0.,0.,10] detector raw signal
  UChar_t     fTPCsignalN;       // number of points used for TPC dE/dx
  Double32_t  fTPCmomentum;      //[0.,0.,20] momentum at the inner wall of TPC;

  Int_t       fTRDnSlices;       // N slices used for PID in the TRD
  UChar_t     fTRDntls;          // number of tracklets used for PID calculation
  UChar_t     fTRDncls[6];       // number of clusters used for dE/dx calculation
  Double32_t* fTRDslices;        //[fTRDnSlices][0.,0.,10]
  Double32_t  fTRDmomentum[6];   //[0.,0.,10]  momentum at the TRD layers
  Double32_t  fTRDChi2;          //TRD chi2

  Double32_t  fTOFesdsignal;     //[0.,0.,20] TOF signal - t0 (T0 interaction time)
  Double32_t  fTOFpidResolution[5]; //[0.,0.,20] TOF pid resolution for each mass hypotesys 
  Double32_t  fIntTime[5];       //[0.,0.,20] track time hypothesis
 
  IlcTPCdEdxInfo * fTPCdEdxInfo; // object containing dE/dx information for different pad regions

  ClassDef(IlcAODPid, 12);
};

//_____________________________________________________________
void IlcAODPid::SetTRDsignal(Int_t nslices, const Double_t * const trdslices) {
  //
  // Set TRD dE/dx slices and the number of dE/dx slices per track
  //
  if(fTRDslices && fTRDnSlices != nslices) {
    delete [] fTRDslices; fTRDslices = NULL;
  };
  if(!fTRDslices) fTRDslices = new Double32_t[nslices];
  fTRDnSlices = nslices; 
  for(Int_t is = 0; is < fTRDnSlices; is++) fTRDslices[is] = trdslices[is];
}

//_____________________________________________________________
void IlcAODPid::SetTRDncls(UChar_t ncls, Int_t layer) { 
  //
  // Set the number of clusters / tracklet
  // If no layer is specified the full number of clusters will be put in layer 0
  //
  if(layer > 5) return; 
  if(layer < 0) fTRDncls[0] = ncls;
  else fTRDncls[layer] = ncls;
}

//_____________________________________________________________
UChar_t IlcAODPid::GetTRDncls() const{
  //
  // Get number of clusters per track
  // Calculated as sum of the number of clusters per tracklet
  //
  UChar_t ncls = 0;
  for(Int_t ily = 0; ily < 6; ily++) ncls += fTRDncls[ily];
  return ncls;
}
#endif
