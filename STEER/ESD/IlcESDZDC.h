// -*- mode: C++ -*- 
#ifndef ILCESDZDC_H
#define ILCESDZDC_H

/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

//-------------------------------------------------------------------------
//                      Implementation of   Class IlcESDZDC
//   This is a class that summarizes the ZDC data for the ESD   
//   Origin: Christian Klein-Boesing, CERN, Christian.Klein-Boesing@cern.ch 
//   *** 15/10/2009 Scaler added to IlcESDZDC class ***
//   *** Scaler format:  32 floats from VME scaler  ***
//-------------------------------------------------------------------------

#include <IlcVZDC.h>

class IlcESDZDC: public IlcVZDC {
public: 
 
  IlcESDZDC();
  IlcESDZDC(const IlcESDZDC& zdc);
  IlcESDZDC& operator=(const IlcESDZDC& zdc);


  // Getters 
  
  virtual Short_t  GetZDCParticipants() const {return fZDCParticipants;}
  virtual Short_t  GetZDCPartSideA() const {return fZDCPartSideA;}
  virtual Short_t  GetZDCPartSideC() const {return fZDCPartSideC;}
  virtual Double_t GetImpactParameter() const {return fImpactParameter;}
  virtual Double_t GetImpactParamSideA() const {return fImpactParamSideA;}
  virtual Double_t GetImpactParamSideC() const {return fImpactParamSideC;}

  virtual Double_t GetZNCEnergy() const {return (Double_t) fZDCN1Energy;}
  virtual Double_t GetZPCEnergy() const {return (Double_t) fZDCP1Energy;}
  virtual Double_t GetZNAEnergy() const {return (Double_t) fZDCN2Energy;}
  virtual Double_t GetZPAEnergy() const {return (Double_t) fZDCP2Energy;}
  virtual Double_t GetZEM1Energy() const {return (Double_t) fZDCEMEnergy;}
  virtual Double_t GetZEM2Energy() const {return (Double_t) fZDCEMEnergy1;}
  
  virtual const Double_t *GetZNCTowerEnergy() const {return fZN1TowerEnergy;}
  virtual const Double_t *GetZNATowerEnergy() const {return fZN2TowerEnergy;}
  virtual const Double_t *GetZPCTowerEnergy() const {return fZP1TowerEnergy;}
  virtual const Double_t *GetZPATowerEnergy() const {return fZP2TowerEnergy;}
  virtual const Double_t *GetZNCTowerEnergyLR() const {return fZN1TowerEnergyLR;}
  virtual const Double_t *GetZNATowerEnergyLR() const {return fZN2TowerEnergyLR;}
  const Double_t *GetZPCTowerEnergyLR() const {return fZP1TowerEnergyLR;}
  const Double_t *GetZPATowerEnergyLR() const {return fZP2TowerEnergyLR;}

  UInt_t   GetESDQuality()  const {return fESDQuality;}
  Double_t GetZDCN1Energy() const {return fZDCN1Energy;}
  Double_t GetZDCP1Energy() const {return fZDCP1Energy;}
  Double_t GetZDCN2Energy() const {return fZDCN2Energy;}
  Double_t GetZDCP2Energy() const {return fZDCP2Energy;}
  Double_t GetZDCEMEnergy(Int_t i) const 
  	   {if(i==0){return fZDCEMEnergy;} else if(i==1){return fZDCEMEnergy1;}
	   return 0;}

  const Double_t * GetZN1TowerEnergy() const {return fZN1TowerEnergy;}
  const Double_t * GetZN2TowerEnergy() const {return fZN2TowerEnergy;}
  const Double_t * GetZP1TowerEnergy() const {return fZP1TowerEnergy;}
  const Double_t * GetZP2TowerEnergy() const {return fZP2TowerEnergy;}
  const Double_t * GetZN1TowerEnergyLR() const {return fZN1TowerEnergyLR;}
  const Double_t * GetZN2TowerEnergyLR() const {return fZN2TowerEnergyLR;}
  const Double_t * GetZP1TowerEnergyLR() const {return fZP1TowerEnergyLR;}
  const Double_t * GetZP2TowerEnergyLR() const {return fZP2TowerEnergyLR;}
  //
  virtual Bool_t GetZNCentroidInPbPb(Float_t beamEne, Double_t centrZNC[2], Double_t centrZNA[2]);
  virtual Bool_t GetZNCentroidInpp(Double_t centrZNC[2], Double_t centrZNA[2]);
  //

  UInt_t GetZDCScaler(Int_t i)  const {return fVMEScaler[i];}
  const UInt_t* GetZDCScaler()  const {return fVMEScaler;}

  Int_t GetZDCTDCData(Int_t i, Int_t j) const {return fZDCTDCData[i][j];}
  Float_t GetZDCTDCCorrected(Int_t i, Int_t j) const 
  {
    if(IlcESDZDC::kCorrectedTDCFilled && (fZDCTDCData[i][j]!=0)) return fZDCTDCCorrected[i][j];
	   else return 0.;
  }
  
  Float_t GetZNTDCSum(Int_t ihit) const 
  {
    if(ihit<4 && IlcESDZDC::kCorrectedTDCFilled && (fZDCTDCData[10][ihit]!=0) && (fZDCTDCData[12][ihit]!=0)) return (Float_t) (fZDCTDCCorrected[10][ihit]+fZDCTDCCorrected[12][ihit]);
    else return 0.;
  }
  Float_t GetZNTDCDiff(Int_t ihit) const 
  {
    if(ihit<4 && IlcESDZDC::kCorrectedTDCFilled &&  (fZDCTDCData[10][ihit]!=0) && (fZDCTDCData[12][ihit]!=0)) return (Float_t) (fZDCTDCCorrected[12][ihit]-fZDCTDCCorrected[10][ihit]);
    else return 0.;
  }
  
  virtual Float_t GetZDCTimeSum() const 
          {if(IlcESDZDC::kCorrectedTDCFilled && (fZDCTDCData[10][0]!=0) && (fZDCTDCData[12][0]!=0)) return (Float_t) (fZDCTDCCorrected[10][0]+fZDCTDCCorrected[12][0]);
           else return 0.;}
  virtual Float_t GetZDCTimeDiff() const 
          {if(IlcESDZDC::kCorrectedTDCFilled &&  (fZDCTDCData[10][0]!=0) && (fZDCTDCData[12][0]!=0)) return (Float_t) (fZDCTDCCorrected[12][0]-fZDCTDCCorrected[10][0]);
           else return 0.;}
  
  void  SetZDC(Double_t n1Energy, Double_t p1Energy, 
  	       Double_t emEnergy0, Double_t emEnergy1,
	       Double_t n2Energy, Double_t p2Energy, 
	       Short_t participants, Short_t nPartA, Short_t nPartC,
	       Double_t b, Double_t bA, Double_t bC, UInt_t recoFlag) 
   	{fZDCN1Energy=n1Energy; fZDCP1Energy=p1Energy; 
    	 fZDCEMEnergy=emEnergy0; fZDCEMEnergy1=emEnergy1;
    	 fZDCN2Energy=n2Energy; fZDCP2Energy=p2Energy; 
	 fZDCParticipants=participants; fZDCPartSideA=nPartA; fZDCPartSideC=nPartC;
	 fImpactParameter=b; fImpactParamSideA=bA, fImpactParamSideC=bC,
	 fESDQuality=recoFlag;}
  //
  void  SetZN1TowerEnergy(const Float_t tow1[5])
          {for(Int_t i=0; i<5; i++) fZN1TowerEnergy[i] = tow1[i];}
  void  SetZN2TowerEnergy(const Float_t tow2[5])
          {for(Int_t i=0; i<5; i++) fZN2TowerEnergy[i] = tow2[i];}
  void  SetZP1TowerEnergy(const Float_t tow1[5])
          {for(Int_t i=0; i<5; i++) fZP1TowerEnergy[i] = tow1[i];}
  void  SetZP2TowerEnergy(const Float_t tow2[5])
          {for(Int_t i=0; i<5; i++) fZP2TowerEnergy[i] = tow2[i];}
  void  SetZN1TowerEnergyLR(const Float_t tow1[5])
          {for(Int_t i=0; i<5; i++) fZN1TowerEnergyLR[i] = tow1[i];}
  void  SetZN2TowerEnergyLR(const Float_t tow2[5])
          {for(Int_t i=0; i<5; i++) fZN2TowerEnergyLR[i] = tow2[i];}
  void  SetZP1TowerEnergyLR(const Float_t tow1[5])
          {for(Int_t i=0; i<5; i++) fZP1TowerEnergyLR[i] = tow1[i];}
  void  SetZP2TowerEnergyLR(const Float_t tow2[5])
          {for(Int_t i=0; i<5; i++) fZP2TowerEnergyLR[i] = tow2[i];}
  void  SetZNACentroid(const Float_t centrCoord[2])
  	   {for(Int_t i=0; i<2; i++) fZNACentrCoord[i] = centrCoord[i];}
  void  SetZNCCentroid(const Float_t centrCoord[2])
  	   {for(Int_t i=0; i<2; i++) fZNCCentrCoord[i] = centrCoord[i];}
  
  void SetZDCScaler(const UInt_t count[32]) 
       {for(Int_t k=0; k<32; k++) fVMEScaler[k] = count[k];}
  
  void SetZDCTDCData(const Int_t values[32][4]) 
       {for(Int_t k=0; k<32; k++)
       	   for(Int_t j=0; j<4; j++) fZDCTDCData[k][j] = values[k][j];}
  
  void SetZDCTDCCorrected(const Float_t values[32][4]) 
       {for(Int_t k=0; k<32; k++)
       	   for(Int_t j=0; j<4; j++) fZDCTDCCorrected[k][j] = values[k][j];}

  void    Reset();
  void    Print(const Option_t *opt=0) const;
private:
  virtual void Copy(TObject &obj) const;
private:

  Double32_t   fZDCN1Energy;  // reconstructed energy in the neutron ZDC
  Double32_t   fZDCP1Energy;  // reconstructed energy in the proton ZDC
  Double32_t   fZDCN2Energy;  // reconstructed energy in the neutron ZDC
  Double32_t   fZDCP2Energy;  // reconstructed energy in the proton ZDC
  Double32_t   fZDCEMEnergy;  // signal in the electromagnetic ZDCs
  Double32_t   fZDCEMEnergy1; // second EM signal,cannot change fZDCEMEnergy to array (not backward compatible)
  Double32_t   fZN1TowerEnergy[5];// reco E in 5 ZN1 sectors - high gain chain
  Double32_t   fZN2TowerEnergy[5];// reco E in 5 ZN2 sectors - high gain chain
  Double32_t   fZP1TowerEnergy[5];// reco E in 5 ZP1 sectors - high gain chain
  Double32_t   fZP2TowerEnergy[5];// reco E in 5 ZP2 sectors - high gain chain
  Double32_t   fZN1TowerEnergyLR[5];// reco E in 5 ZN1 sectors - low gain chain
  Double32_t   fZN2TowerEnergyLR[5];// reco E in 5 ZN2 sectors - low gain chain
  Double32_t   fZP1TowerEnergyLR[5];// reco E in 5 ZP1 sectors - low gain chain
  Double32_t   fZP2TowerEnergyLR[5];// reco E in 5 ZP2 sectors - low gain chain
  Short_t      fZDCParticipants;    // number of participants estimated by the ZDC (ONLY in A-A)
  Short_t      fZDCPartSideA;     // number of participants estimated by the ZDC (ONLY in A-A)
  Short_t      fZDCPartSideC;     // number of participants estimated by the ZDC (ONLY in A-A)
  Double32_t   fImpactParameter;  // impact parameter estimated by the ZDC (ONLY in A-A)
  Double32_t   fImpactParamSideA; // impact parameter estimated by the ZDC (ONLY in A-A)
  Double32_t   fImpactParamSideC; // impact parameter estimated by the ZDC (ONLY in A-A)
  Double32_t   fZNACentrCoord[2]; // Coordinates of the centroid over ZNC
  Double32_t   fZNCCentrCoord[2]; // Coordinates of the centroid over ZNA
  UInt_t       fESDQuality;	  // flags from reconstruction
  UInt_t       fVMEScaler[32]; 	  // counts from VME scaler
  Int_t        fZDCTDCData[32][4];     // ZDC TDC data
  Float_t      fZDCTDCCorrected[32][4];// ZDC TDC data in ns corrected 4 phase shift
  
  ClassDef(IlcESDZDC,17)
};

#endif

