// -*- mode: C++ -*- 
#ifndef ILCESDRUN_H
#define ILCESDRUN_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

//-------------------------------------------------------------------------
//                     Implementation Class IlcESDRun
//   Run by run data
//   for the ESD   
//   Origin: Christian Klein-Boesing, CERN, Christian.Klein-Boesing@cern.ch 
//-------------------------------------------------------------------------

#include <TObject.h>
#include <TObjArray.h>
#include <TString.h>

class TGeoHMatrix;
class IlcESDVertex;

class IlcESDRun: public TObject {
public:

  enum StatusBits {kBInfoStored = BIT(14), kUniformBMap = BIT(15), kConvSqrtSHalfGeV = BIT(16)};


  IlcESDRun();
  IlcESDRun(const IlcESDRun& esd);
  IlcESDRun& operator=(const IlcESDRun& esd);
  virtual void Copy(TObject &obj) const; // Interface for using TOBject::Copy()
  virtual ~IlcESDRun();

  Bool_t  InitMagneticField() const;
  Int_t   GetRunNumber() const {return fRunNumber;}
  void    SetRunNumber(Int_t n) {fRunNumber=n;}
  void    SetMagneticField(Float_t mf){fMagneticField = mf;}
  Double_t GetMagneticField() const {return fMagneticField;}
  UInt_t   GetPeriodNumber() const {return fPeriodNumber;}
  void    SetPeriodNumber(Int_t n) {fPeriodNumber=n;}
  void    Reset();
  void    Print(const Option_t *opt=0) const;
  void    SetDiamond(const IlcESDVertex *vertex);
  void    SetTriggerClass(const char*name, Int_t index);
  void    SetCurrentL3(Float_t cur)    {fCurrentL3 = cur;}
  void    SetCurrentDip(Float_t cur)   {fCurrentDip = cur;}
  void    SetBeamEnergy(Float_t be)    {fBeamEnergy = be;}
  void    SetBeamType(const char* bt)  {fBeamType = bt;}
  void    SetBeamEnergyIsSqrtSHalfGeV(Bool_t v=kTRUE) {SetBit(kConvSqrtSHalfGeV,v);}
  void    SetDetectorsInDAQ(UInt_t detmask) { fDetInDAQ = detmask; }
  void    SetDetectorsInReco(UInt_t detmask) { fDetInReco = detmask; }

  Bool_t  IsBeamEnergyIsSqrtSHalfGeV() const {return TestBit(kConvSqrtSHalfGeV);}  
  Double_t GetDiamondX() const {return fDiamondXY[0];}
  Double_t GetDiamondY() const {return fDiamondXY[1];}
  Double_t GetDiamondZ() const {return fDiamondZ;}
  Double_t GetSigma2DiamondX() const {return fDiamondCovXY[0];}
  Double_t GetSigma2DiamondY() const {return fDiamondCovXY[2];}
  Double_t GetSigma2DiamondZ() const {return fDiamondSig2Z;}
  void GetDiamondCovXY(Float_t cov[3]) const {
    for(Int_t i=0;i<3;i++) cov[i]=fDiamondCovXY[i]; return;
  }
  const char* GetTriggerClass(Int_t index) const;
  TString     GetActiveTriggerClasses() const;
  TString     GetFiredTriggerClasses(ULong64_t mask) const;
  Bool_t      IsTriggerClassFired(ULong64_t mask, const char *name) const;
  Float_t     GetCurrentL3()               const {return fCurrentL3;}
  Float_t     GetCurrentDip()              const {return fCurrentDip;}
  Float_t     GetBeamEnergy()              const {return IsBeamEnergyIsSqrtSHalfGeV() ? fBeamEnergy : fBeamEnergy/2;}
  const char* GetBeamType()                const {return (fBeamType=="Pb-Pb") ? "A-A":fBeamType.Data();}
  void        SetBeamParticle(Int_t az, Int_t ibeam) {fBeamParticle[ibeam] = az;}
  Int_t       GetBeamParticle(Int_t ibeam)  const {return fBeamParticle[ibeam];}
  Int_t       GetBeamParticleA(Int_t ibeam) const {return fBeamParticle[ibeam]/1000;}
  Int_t       GetBeamParticleZ(Int_t ibeam) const {return fBeamParticle[ibeam]%1000;}

  UInt_t      GetDetectorsInDAQ()         const {return fDetInDAQ; }
  UInt_t      GetDetectorsInReco()         const {return fDetInReco; }
 
  void    SetPHOSMatrix(TGeoHMatrix*matrix, Int_t i) {
    if ((i >= 0) && (i < kNPHOSMatrix)) fPHOSMatrix[i] = matrix;
  }
  const TGeoHMatrix* GetPHOSMatrix(Int_t i) const {
    return ((i >= 0) && (i < kNPHOSMatrix)) ? fPHOSMatrix[i] : NULL;
  }
	
  void    SetEMCALMatrix(TGeoHMatrix*matrix, Int_t i) {
	if ((i >= 0) && (i < kNEMCALMatrix)) fEMCALMatrix[i] = matrix;
  }
  const TGeoHMatrix* GetEMCALMatrix(Int_t i) const {
	return ((i >= 0) && (i < kNEMCALMatrix)) ? fEMCALMatrix[i] : NULL;
  }
	
  enum {kNTriggerClasses = 50};
  enum {kNPHOSMatrix = 5};
  enum {kNEMCALMatrix = 12};
  enum {kT0spreadSize = 4};
  //
  Double_t   GetMeanIntensity(int beam,int btp)     const 
  { return (beam>=0&&beam<2&&btp>=0&&btp<2) ? fMeanBeamInt[beam][btp]:0;}
  void       SetMeanIntensity(int beam,int btp, double v=-1) 
  { if (beam>=0&&beam<2&&btp>=0&&btp<2) fMeanBeamInt[beam][btp]=v;}  
  Double_t   GetMeanIntensityIntecting(int beam)    const {return GetMeanIntensity(beam,0);}
  Double_t   GetMeanIntensityNonIntecting(int beam) const {return GetMeanIntensity(beam,1);}
  // 
  Float_t    GetT0spread(Int_t i) const {
    return ((i >= 0)  && (i<kT0spreadSize)) ? fT0spread[i] : 0;}
  void       SetT0spread(Int_t i, Float_t t);
  void       SetT0spread(Float_t *t);
	
  void       SetCaloTriggerType(const Int_t* in) {for (int i = 0; i < 15; i++) fCaloTriggerType[i] = in[i];}
  Int_t*     GetCaloTriggerType() {return fCaloTriggerType;}

  void           SetVZEROEqFactors(Float_t factors[64]) {for (Int_t i = 0; i < 64; ++i) fVZEROEqFactors[i] = factors[i];}
  const Float_t* GetVZEROEqFactors() const {return fVZEROEqFactors;}
  Float_t        GetVZEROEqFactors(Int_t i) const {return fVZEROEqFactors[i];}

private:
  Float_t         fCurrentL3;       // signed current in the L3     (LHC convention: +current -> +Bz)
  Float_t         fCurrentDip;      // signed current in the Dipole (LHC convention: +current -> -Bx)
  Float_t         fBeamEnergy;      // beamEnergy entry from GRP
  Double32_t      fMagneticField;   // Solenoid Magnetic Field in kG : for compatibility with IlcMagF
  Double32_t      fMeanBeamInt[2][2]; // mean intensity of interacting and non-intercting bunches per beam
  Double32_t      fDiamondXY[2];    // Interaction diamond (x,y) in RUN
  Double32_t      fDiamondCovXY[3]; // Interaction diamond covariance (x,y) in RUN
  Double32_t      fDiamondZ;        // Interaction diamond (z) in RUN
  Double32_t      fDiamondSig2Z;    // Interaction diamond sigma^2 (z) in RUN
  UInt_t          fPeriodNumber;    // PeriodNumber
  Int_t           fRunNumber;       // Run Number
  Int_t           fRecoVersion;     // Version of reconstruction
  Int_t           fBeamParticle[2]; // A*1000+Z for each beam particle
  TString         fBeamType;        // beam type from GRP
  TObjArray       fTriggerClasses;  // array of TNamed containing the names of the active trigger classes
  UInt_t          fDetInDAQ;        // Detector mask for detectors in datataking
  UInt_t          fDetInReco;       // Detector mask for detectors in reconstruction
  TGeoHMatrix*    fPHOSMatrix[kNPHOSMatrix]; //PHOS module position and orientation matrices
  TGeoHMatrix*    fEMCALMatrix[kNEMCALMatrix]; //EMCAL supermodule position and orientation matrices
  Float_t         fT0spread[kT0spreadSize];     // spread of time distributions on T0A, T0C, (T0A+T0C)/2, (T0A-T0C)/2
  Int_t           fCaloTriggerType[15]; // Calorimeter trigger type
  Float_t         fVZEROEqFactors[64]; // V0 channel equalization factors for event-plane reconstruction
	
  ClassDef(IlcESDRun,14)
};

#endif 
