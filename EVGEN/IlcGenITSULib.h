#ifndef ILCGENITSULIB_H
#define ILCGENITSULIB_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

// Library class for particle pt and y distributions used for 
// LambdaB simulations.
// To be used with IlcGenParam.
//
// Author: Annalisa Mastroserio <Annalisa.Mastroserio@cern.ch>

#include "IlcGenLib.h"

class TRandom;

class IlcGenITSULib :public IlcGenLib {

 public:

  enum EPartId {kLb=5122,kLc=4122,kXi_c = 4232,kB = 521,kDs=431,kDplus=411};

  //Getters
    
  GenFunc   GetPt(Int_t iPID, const char * sForm=0) const;
  GenFunc   GetY (Int_t iPID, const char * sForm=0) const;
  GenFuncIp GetIp(Int_t iPID, const char * sForm=0) const;    

 private:

  static Int_t IpLcPlus(TRandom * /*ran*/)  {return     (int)kLc;}
  static Int_t IpLcMinus(TRandom * /*ran*/) {return    -(int)kLc;}
  static Int_t IpLb(TRandom * /*ran*/)      {return     (int)kLb;}
  static Int_t IpLbBar(TRandom * /*ran*/)   {return    -(int)kLb;}
  static Int_t IpXic(TRandom * /*ran*/)     {return   (int)kXi_c;}
  static Int_t IpXicBar(TRandom * /*ran*/)  {return  -(int)kXi_c;}
  static Int_t IpBPlus(TRandom * /*ran*/)   {return      (int)kB;}
  static Int_t IpBMinus(TRandom * /*ran*/)  {return     -(int)kB;}
  static Int_t IpDsPlus(TRandom * /*ran*/)  {return     (int)kDs;}
  static Int_t IpDsMinus(TRandom * /*ran*/) {return    -(int)kDs;}
  static Int_t IpDPlus(TRandom * /*ran*/)   {return  (int)kDplus;}
  static Int_t IpDMinus(TRandom * /*ran*/)  {return -(int)kDplus;}

  static Double_t PtFlat(const Double_t * /*px*/, const Double_t * /*dummy*/) {return 1;}
  static Double_t YFlat (const Double_t * /*py*/, const Double_t * /*dummy*/) {return 1;}

  static Double_t PtLbDist (const Double_t *px, const Double_t *dummy);
  static Double_t PtLcDist (const Double_t *px, const Double_t *dummy);
  static Double_t PtBDist  (const Double_t *px, const Double_t *dummy);



  ClassDef(IlcGenITSULib,0)
    };

#endif







