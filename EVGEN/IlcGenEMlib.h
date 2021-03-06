#ifndef ILCGENEMLIB_H
#define ILCGENEMLIB_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* $Id: IlcGenEMlib.h 30052 2008-11-25 14:54:18Z morsch $ */

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Implementation of IlcGenEMlib for electron, di-electron, and photon     //
// cocktail calculations.                                                  //
// It is based on IlcGenGSIlib.                                            //
//                                                                         //
// Responsible: R.Averbeck@gsi.de                                          //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "IlcGenLib.h"
class TRandom;

class IlcGenEMlib :public IlcGenLib {
 public:
    GenFunc   GetPt(Int_t param, const char * tname=0) const;
    GenFunc   GetY(Int_t param, const char * tname=0) const;
    GenFuncIp GetIp(Int_t param, const char * tname=0) const;    

    enum constants{kPizero, kEta, kRho, kOmega, kEtaprime, kPhi};

 private:

// Pizero
    static Int_t    IpPizero(TRandom *ran);
    static Double_t PtPizero( const Double_t *px, const Double_t *dummy );
    static Double_t YPizero(const Double_t *py, const Double_t *dummy);
    static Double_t MtScal(Double_t pt, Int_t np);

// Eta
    static Int_t    IpEta(TRandom *ran);
    static Double_t PtEta( const Double_t *px, const Double_t *dummy );
    static Double_t YEta(const Double_t *py, const Double_t *dummy);

// Rho
    static Int_t    IpRho(TRandom *ran);
    static Double_t PtRho( const Double_t *px, const Double_t *dummy );
    static Double_t YRho(const Double_t *py, const Double_t *dummy);

// Omega
    static Int_t    IpOmega(TRandom *ran);
    static Double_t PtOmega( const Double_t *px, const Double_t *dummy );
    static Double_t YOmega(const Double_t *py, const Double_t *dummy);

// Etaprime
    static Int_t    IpEtaprime(TRandom *ran);
    static Double_t PtEtaprime( const Double_t *px, const Double_t *dummy );
    static Double_t YEtaprime(const Double_t *py, const Double_t *dummy);

// Phi
    static Int_t    IpPhi(TRandom *ran);
    static Double_t PtPhi( const Double_t *px, const Double_t *dummy );
    static Double_t YPhi(const Double_t *py, const Double_t *dummy);

// General
    static Double_t PtFlat(const Double_t *px, const Double_t *dummy);
    static Double_t YFlat(Double_t y);


  ClassDef(IlcGenEMlib,0)
};

#endif
