#ifndef ILCVZDC_H
#define ILCVZDC_H

//-------------------------------------------------------------------------
//     Base class for ESD and AOD ZDC data
//     Author: Chiara Oppedisano
//     Chiara.Oppedisano@cern.ch 
//-------------------------------------------------------------------------

#include "TObject.h"

class IlcVZDC : public TObject 
{
public:
  IlcVZDC() { }
  IlcVZDC(const IlcVZDC& source);
  IlcVZDC &operator=(const IlcVZDC& source);

  virtual ~IlcVZDC() { }
 
  enum {
    kCorrectedTDCFilled = BIT(14),
    kEnergyCalibratedSignal = BIT(15)
  };

  virtual Short_t  GetZDCParticipants() const = 0; 
  virtual Short_t  GetZDCPartSideA()	const = 0; 
  virtual Short_t  GetZDCPartSideC()	const  = 0;
  virtual Double_t GetImpactParameter()  const = 0;
  virtual Double_t GetImpactParamSideA() const = 0;
  virtual Double_t GetImpactParamSideC() const = 0;

  virtual Double_t GetZNCEnergy() const = 0;
  virtual Double_t GetZNAEnergy() const = 0;
  virtual Double_t GetZPCEnergy() const = 0;
  virtual Double_t GetZPAEnergy() const = 0;
  virtual Double_t GetZEM1Energy() const = 0; 
  virtual Double_t GetZEM2Energy() const = 0; 
    
  virtual const Double_t *GetZNCTowerEnergy() const = 0; 
  virtual const Double_t *GetZNATowerEnergy() const = 0; 
  virtual const Double_t *GetZPCTowerEnergy() const  = 0;
  virtual const Double_t *GetZPATowerEnergy() const  = 0;
  virtual const Double_t *GetZNCTowerEnergyLR() const = 0; 
  virtual const Double_t *GetZNATowerEnergyLR() const = 0; 
  
  virtual Bool_t GetZNCentroidInPbPb(Float_t /*beamEne*/, Double_t* /*centrZNC[2]*/, Double_t*
  /*centrZNA[2]*/) {return kFALSE;}
  virtual Bool_t GetZNCentroidInpp(Double_t* /*centrZNC[2]*/, Double_t* /*centrZNA[2]*/)  {return kFALSE;}

  virtual Float_t GetZDCTimeSum() const = 0;
  virtual Float_t GetZDCTimeDiff() const = 0;
    
  ClassDef(IlcVZDC,1)
};

#endif
