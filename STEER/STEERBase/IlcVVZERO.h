#ifndef ILCVVZERO_H
#define ILCVVZERO_H

//-------------------------------------------------------------------------
//     Base class for ESD and AOD VZERO data
//     Author: Cvetan Cheshkov
//     cvetan.cheshkov@cern.ch 2/02/2011
//-------------------------------------------------------------------------

#include "TObject.h"

class IlcVVZERO : public TObject 
{
public:
  IlcVVZERO() { }
  IlcVVZERO(const IlcVVZERO& source);
  IlcVVZERO &operator=(const IlcVVZERO& source);

  virtual ~IlcVVZERO() { }

  enum {
    kCorrectedLeadingTime = BIT(14),
    kTriggerBitsFilled = BIT(15),
    kDecisionFilled = BIT(16),
    kOnlineBitsFilled = BIT(17),
    kCorrectedForSaturation = BIT(18),
    kRobustMeanTime = BIT(19),
    kTriggerChargeBitsFilled = BIT(20)
  };
  enum Decision { kV0Invalid = -1, kV0Empty = 0, kV0BB, kV0BG, kV0Fake };
  enum TriggerBits {
    kBBAandBBC = 0,
    kBBAorBBC = 1,
    kBGAandBBC = 2,
    kBGA = 3,
    kBGCandBBA = 4,
    kBGC = 5,
    kCTA1andCTC1 = 6,
    kCTA1orCTC1 = 7,
    kCTA2andCTC2 = 8,
    kCTA2orCTC2 = 9,
    kMTAandMTC = 10,
    kMTAorMTC = 11,
    kBBA = 12,
    kBBC = 13,
    kBGAorBGC = 14,
    kBGAandBBCorBGCandBBA = 15
  };

  virtual Short_t  GetNbPMV0A() const = 0;
  virtual Short_t  GetNbPMV0C() const = 0;
  virtual Float_t  GetMTotV0A() const = 0;
  virtual Float_t  GetMTotV0C() const = 0; 
  virtual Float_t  GetMRingV0A(Int_t ring) const = 0;
  virtual Float_t  GetMRingV0C(Int_t ring) const = 0;

  virtual Float_t  GetMultiplicity(Int_t i) const = 0;
  virtual Float_t  GetMultiplicityV0A(Int_t i) const = 0;
  virtual Float_t  GetMultiplicityV0C(Int_t i) const = 0;    

  virtual Bool_t   BBTriggerV0A(Int_t i) const = 0;
  virtual Bool_t   BGTriggerV0A(Int_t i) const = 0;
  virtual Bool_t   BBTriggerV0C(Int_t i) const = 0;
  virtual Bool_t   BGTriggerV0C(Int_t i) const = 0;  
  virtual Bool_t   GetBBFlag(Int_t i) const = 0;
  virtual Bool_t   GetBGFlag(Int_t i) const = 0;

  virtual Float_t  GetV0ATime() const = 0;
  virtual Float_t  GetV0CTime() const = 0;

  virtual Decision GetV0ADecision() const = 0;
  virtual Decision GetV0CDecision() const = 0;

  virtual UShort_t GetTriggerChargeA() const = 0;
  virtual UShort_t GetTriggerChargeC() const = 0;
  virtual UShort_t GetTriggerBits() const = 0;

protected:  

  Bool_t OutOfRange(Int_t i, const char *s, Int_t upper) const;
    
  ClassDef(IlcVVZERO,2)
};

#endif
