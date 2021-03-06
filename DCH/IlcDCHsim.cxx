
/**************************************************************************
 * Copyright(c) 2005-2006, ILC Project Experiment, All rights reserved.   *
 *                                                                        *
// Author: The ILC Off-line Project. 
 // Part of the code has been developed by Alice Off-line Project. 
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id: IlcDCHsim.cxx,v 1.2 2009/12/11 10:40:28 vitomeg Exp $ */

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  DCH simulation - multimodule (regular rad.)                              //
//  after: M. CASTELLANO et al., COMP. PHYS. COMM. 51 (1988) 431             //
//                             + COMP. PHYS. COMM. 61 (1990) 395             //
//                                                                           //
//   17.07.1998 - A.Andronic                                                 //
//   08.12.1998 - simplified version                                         //
//   11.07.2000 - Adapted code to ilcroot environment (C.Blume)              //
//   04.06.2004 - Momentum dependent parameters implemented (CBL)            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include <TH1.h>
#include <TRandom.h>
#include <TMath.h>
#include <TParticle.h>

#include "IlcModule.h"

#include "IlcDCHsim.h"

ClassImp(IlcDCHsim)

//_____________________________________________________________________________
IlcDCHsim::IlcDCHsim():TObject()
{
  //
  // IlcDCHsim default constructor
  // 

  fSpectrum = 0;
  fSigma    = 0;
  fNFoils   = 0;
  fNFoilsUp = 0;

  Init();

}

//_____________________________________________________________________________
IlcDCHsim::IlcDCHsim(IlcModule *mod, Int_t foil, Int_t gap)
{
  //
  // IlcDCHsim constructor. Takes the material properties of the radiator
  // foils and the gas in the gaps from IlcModule <mod>.
  // The default number of foils is 100 with a thickness of 20 mu. The 
  // thickness of the gaps is 500 mu.
  //

  Float_t aFoil, zFoil, rhoFoil;
  Float_t aGap,  zGap,  rhoGap;
  Float_t rad, abs;
  Char_t  name[21];

  fSpectrum = 0;
  fSigma    = 0;
  fNFoils   = 0;
  fNFoilsUp = 0;

  Init();

  mod->IlcGetMaterial(foil,name,aFoil,zFoil,rhoFoil,rad,abs);
  mod->IlcGetMaterial(gap ,name,aGap ,zGap ,rhoGap ,rad,abs);

  fFoilDens  = rhoFoil;
  fFoilA     = aFoil;
  fFoilZ     = zFoil;
  fFoilOmega = Omega(fFoilDens,fFoilZ,fFoilA);

  fGapDens   = rhoGap;
  fGapA      = aGap;
  fGapZ      = zGap;
  fGapOmega  = Omega(fGapDens ,fGapZ ,fGapA );

}

//_____________________________________________________________________________
IlcDCHsim::IlcDCHsim(const IlcDCHsim &s):TObject(s)
{
  //
  // IlcDCHsim copy constructor
  //

  ((IlcDCHsim &) s).Copy(*this);

}

//_____________________________________________________________________________
IlcDCHsim::~IlcDCHsim() 
{
  //
  // IlcDCHsim destructor
  //

  //  if (fSpectrum) delete fSpectrum;
  if (fSigma)    delete [] fSigma;
  if (fNFoils)   delete [] fNFoils;
  if (fNFoilsUp) delete [] fNFoilsUp;

}

//_____________________________________________________________________________
IlcDCHsim &IlcDCHsim::operator=(const IlcDCHsim &s)
{
  //
  // Assignment operator
  //

  if (this != &s) ((IlcDCHsim &) s).Copy(*this);
  return *this;

}

//_____________________________________________________________________________
void IlcDCHsim::Copy(TObject &s) const
{
  //
  // Copy function
  //

  ((IlcDCHsim &) s).fFoilThick  = fFoilThick;
  ((IlcDCHsim &) s).fFoilDens   = fFoilDens;
  ((IlcDCHsim &) s).fFoilOmega  = fFoilOmega;
  ((IlcDCHsim &) s).fFoilZ      = fFoilZ;
  ((IlcDCHsim &) s).fFoilA      = fFoilA;
  ((IlcDCHsim &) s).fGapThick   = fGapThick;
  ((IlcDCHsim &) s).fGapDens    = fGapDens;
  ((IlcDCHsim &) s).fGapOmega   = fGapOmega;
  ((IlcDCHsim &) s).fGapZ       = fGapZ;
  ((IlcDCHsim &) s).fGapA       = fGapA;
  ((IlcDCHsim &) s).fTemp       = fTemp;
  ((IlcDCHsim &) s).fSpNBins    = fSpNBins;
  ((IlcDCHsim &) s).fSpRange    = fSpRange;
  ((IlcDCHsim &) s).fSpBinWidth = fSpBinWidth;
  ((IlcDCHsim &) s).fSpLower    = fSpLower;
  ((IlcDCHsim &) s).fSpUpper    = fSpUpper;

  if (((IlcDCHsim &) s).fNFoils) delete [] ((IlcDCHsim &) s).fNFoils;
  ((IlcDCHsim &) s).fNFoils   = new Int_t[fNFoilsDim];
  for (Int_t iFoil = 0; iFoil < fNFoilsDim; iFoil++) {
    ((IlcDCHsim &) s).fNFoils[iFoil] = fNFoils[iFoil];
  }  

  if (((IlcDCHsim &) s).fNFoilsUp) delete [] ((IlcDCHsim &) s).fNFoilsUp;
  ((IlcDCHsim &) s).fNFoilsUp = new Double_t[fNFoilsDim];
  for (Int_t iFoil = 0; iFoil < fNFoilsDim; iFoil++) {
    ((IlcDCHsim &) s).fNFoilsUp[iFoil] = fNFoilsUp[iFoil];
  }  

  if (((IlcDCHsim &) s).fSigma)  delete [] ((IlcDCHsim &) s).fSigma;
  ((IlcDCHsim &) s).fSigma = new Double_t[fSpNBins];
  for (Int_t iBin = 0; iBin < fSpNBins; iBin++) {
    ((IlcDCHsim &) s).fSigma[iBin] = fSigma[iBin];
  }  

  fSpectrum->Copy(*((IlcDCHsim &) s).fSpectrum);

}

//_____________________________________________________________________________
void IlcDCHsim::Init()
{
  //
  // Initialization 
  // The default radiator are prolypropilene foils of 10 mu thickness
  // with gaps of 80 mu filled with N2.
  // 

  fNFoilsDim   = 7;

  if (fNFoils)   delete [] fNFoils;
  fNFoils      = new Int_t[fNFoilsDim];
  fNFoils[0]   = 170;
  fNFoils[1]   = 225; //250;
  fNFoils[2]   = 275; //310;
  fNFoils[3]   = 305; //380;
  fNFoils[4]   = 325; //430;
  fNFoils[5]   = 340; //490;
  fNFoils[6]   = 350; //550;

  if (fNFoilsUp) delete [] fNFoilsUp;
  fNFoilsUp    = new Double_t[fNFoilsDim];
  fNFoilsUp[0] = 1.25;
  fNFoilsUp[1] = 1.75;
  fNFoilsUp[2] = 2.50;
  fNFoilsUp[3] = 3.50;
  fNFoilsUp[4] = 4.50;
  fNFoilsUp[5] = 5.50;
  fNFoilsUp[6] = 10000.0;

  fFoilThick  = 0.0013;
  fFoilDens   = 0.92;   
  fFoilZ      = 5.28571;
  fFoilA      = 10.4286;
  fFoilOmega  = Omega(fFoilDens,fFoilZ,fFoilA);

  fGapThick   = 0.0060;
  fGapDens    = 0.00125;  
  fGapZ       = 7.0;
  fGapA       = 14.00674;
  fGapOmega   = Omega(fGapDens ,fGapZ ,fGapA );

  fTemp       = 293.16;

  fSpNBins    = 200;
  fSpRange    = 100;
  fSpBinWidth = fSpRange / fSpNBins;
  fSpLower    = 1.0 - 0.5 * fSpBinWidth;
  fSpUpper    = fSpLower + fSpRange;

  if (fSpectrum) delete fSpectrum;
  fSpectrum   = new TH1D("TRspectrum","TR spectrum",fSpNBins,fSpLower,fSpUpper);
  fSpectrum->SetDirectory(0);

  // Set the sigma values 
  SetSigma();

}

//_____________________________________________________________________________
Int_t IlcDCHsim::CreatePhotons(Int_t pdg, Float_t p
                             , Int_t &nPhoton, Float_t *ePhoton)
{
  //
  // Create DCH photons for a charged particle of type <pdg> with the total 
  // momentum <p>. 
  // Number of produced TR photons:       <nPhoton>
  // Energies of the produced TR photons: <ePhoton>
  //

  // PDG codes
  const Int_t kPdgEle  =  11;
  const Int_t kPdgMuon =  13;
  const Int_t kPdgPion = 211;
  const Int_t kPdgKaon = 321;

  Float_t  mass        = 0;
  switch (TMath::Abs(pdg)) {
  case kPdgEle:
    mass      =  5.11e-4;
    break;
  case kPdgMuon:
    mass      =  0.10566;
    break;
  case kPdgPion:
    mass      =  0.13957;
    break;
  case kPdgKaon:
    mass      =  0.4937;
    break;
  default:
    return 0;
    break;
  };

  // Calculate the TR photons
  return TrPhotons(p, mass, nPhoton, ePhoton);

}

//_____________________________________________________________________________
Int_t IlcDCHsim::TrPhotons(Float_t p, Float_t mass
                         , Int_t &nPhoton, Float_t *ePhoton)
{
  //
  // Produces TR photons.
  //

  const Double_t kAlpha  = 0.0072973;
  const Int_t    kSumMax = 30;
	
  Double_t tau = fGapThick / fFoilThick;

  // Calculate gamma
  Double_t gamma = TMath::Sqrt(p*p + mass*mass) / mass;

  // Select the number of foils corresponding to momentum
  Int_t    foils = SelectNFoils(p);

  fSpectrum->Reset();

  // The TR spectrum
	Double_t csi1,csi2,rho1,rho2;
	Double_t fSigma,Sum,Nequ,theta_n,aux;
  Double_t energyeV, energykeV;
	for (Int_t iBin = 1; iBin <= fSpNBins; iBin++) {
    energykeV = fSpectrum->GetBinCenter(iBin);
    energyeV = energykeV * 1.e3;

    fSigma       = Sigma(energykeV);

    csi1 = fFoilOmega / energyeV;
    csi2 = fGapOmega  / energyeV;

    rho1 = 2.5 * energyeV * fFoilThick * 1.E4 
                                 * (1. / (gamma*gamma) + csi1*csi1);
    rho2 = 2.5 * energyeV * fFoilThick * 1.E4 
                                 * (1.0 / (gamma*gamma) + csi2 *csi2);

    // Calculate the sum
    Sum = 0;
    for (Int_t n = 1; n <= kSumMax; n++) {
      theta_n = (TMath::Pi() * 2.0 * n - (rho1 + tau * rho2)) / (1.+ tau);
      if (theta_n < 0.) theta_n = 0.0;
      aux   = 1. / (rho1 + theta_n) - 1. / (rho2 + theta_n);
      Sum  += theta_n * (aux*aux) * (1.0 - TMath::Cos(rho1 + theta_n));
    }

    // Equivalent number of foils
    Nequ      = (1. - TMath::Exp(-foils * fSigma)) / (1.- TMath::Exp(-fSigma));


    // dN / domega
    fSpectrum->SetBinContent(iBin,4. * kAlpha * Nequ * Sum /  (energykeV * (1. + tau)));
  }

  // <nTR> (binsize corr.)
  Float_t ntr = fSpBinWidth*fSpectrum->Integral();
  // Number of TR photons from Poisson distribution with mean <ntr>
  nPhoton = gRandom->Poisson(ntr);
  // Energy of the TR photons
  for (Int_t iPhoton = 0; iPhoton < nPhoton; iPhoton++) {
    ePhoton[iPhoton] = fSpectrum->GetRandom();
  }

  return 1;

}

//_____________________________________________________________________________
void IlcDCHsim::SetSigma() 
{
  //
  // Sets the absorbtion crosssection for the energies of the TR spectrum
  //

  if (fSigma) delete [] fSigma;
  fSigma = new Double_t[fSpNBins];
  for (Int_t iBin = 0; iBin < fSpNBins; iBin++) {
    Double_t energykeV = iBin * fSpBinWidth + 1.0;
    fSigma[iBin]       = Sigma(energykeV);
    //printf("SetSigma(): iBin = %d fSigma %g\n",iBin,fSigma[iBin]);
  }

}

//_____________________________________________________________________________
Double_t IlcDCHsim::Sigma(Double_t energykeV)
{
  //
  // Calculates the absorbtion crosssection for a one-foil-one-gap-radiator
  //

  // keV -> MeV
  Double_t energyMeV = energykeV * 0.001;
  if (energyMeV >= 0.001) {
    return(GetMuPo(energyMeV) * fFoilDens * fFoilThick +
           GetMuAi(energyMeV) * fGapDens  * fGapThick  * GetTemp());
  }
  else {
    return 1e6;
  }

}

//_____________________________________________________________________________
Double_t IlcDCHsim::GetMuPo(Double_t energyMeV)
{
  //
  // Returns the photon absorbtion cross section for polypropylene
  //

  const Int_t kN = 36;

  Double_t mu[kN] = { 1.894E+03, 5.999E+02, 2.593E+02
                    , 7.743E+01, 3.242E+01, 1.643E+01
                    , 9.432E+00, 3.975E+00, 2.088E+00
                    , 7.452E-01, 4.315E-01, 2.706E-01
                    , 2.275E-01, 2.084E-01, 1.970E-01
                    , 1.823E-01, 1.719E-01, 1.534E-01
                    , 1.402E-01, 1.217E-01, 1.089E-01
                    , 9.947E-02, 9.198E-02, 8.078E-02
                    , 7.262E-02, 6.495E-02, 5.910E-02   
                    , 5.064E-02, 4.045E-02, 3.444E-02
                    , 3.045E-02, 2.760E-02, 2.383E-02
		    , 2.145E-02, 1.819E-02, 1.658E-02 };

  Double_t en[kN] = { 1.000E-03, 1.500E-03, 2.000E-03
                    , 3.000E-03, 4.000E-03, 5.000E-03
                    , 6.000E-03, 8.000E-03, 1.000E-02
                    , 1.500E-02, 2.000E-02, 3.000E-02
                    , 4.000E-02, 5.000E-02, 6.000E-02
                    , 8.000E-02, 1.000E-01, 1.500E-01
                    , 2.000E-01, 3.000E-01, 4.000E-01
                    , 5.000E-01, 6.000E-01, 8.000E-01
                    , 1.000E+00, 1.250E+00, 1.500E+00
                    , 2.000E+00, 3.000E+00, 4.000E+00
                    , 5.000E+00, 6.000E+00, 8.000E+00
		    , 1.000E+01, 1.500E+01, 2.000E+01 };

  return Interpolate(energyMeV,en,mu,kN);

}

//_____________________________________________________________________________
Double_t IlcDCHsim::GetMuCO(Double_t energyMeV)
{
  //
  // Returns the photon absorbtion cross section for CO2
  //

  const Int_t kN = 36;

  Double_t mu[kN] = { 0.39383E+04, 0.13166E+04, 0.58750E+03
                    , 0.18240E+03, 0.77996E+02, 0.40024E+02
                    , 0.23116E+02, 0.96997E+01, 0.49726E+01
                    , 0.15543E+01, 0.74915E+00, 0.34442E+00
                    , 0.24440E+00, 0.20589E+00, 0.18632E+00
                    , 0.16578E+00, 0.15394E+00, 0.13558E+00
                    , 0.12336E+00, 0.10678E+00, 0.95510E-01
                    , 0.87165E-01, 0.80587E-01, 0.70769E-01
                    , 0.63626E-01, 0.56894E-01, 0.51782E-01
                    , 0.44499E-01, 0.35839E-01, 0.30825E-01
                    , 0.27555E-01, 0.25269E-01, 0.22311E-01
		    , 0.20516E-01, 0.18184E-01, 0.17152E-01 };

  Double_t en[kN] = { 0.10000E-02, 0.15000E-02, 0.20000E-02
                    , 0.30000E-02, 0.40000E-02, 0.50000E-02
                    , 0.60000E-02, 0.80000E-02, 0.10000E-01
                    , 0.15000E-01, 0.20000E-01, 0.30000E-01
                    , 0.40000E-01, 0.50000E-01, 0.60000E-01
                    , 0.80000E-01, 0.10000E+00, 0.15000E+00
                    , 0.20000E+00, 0.30000E+00, 0.40000E+00
                    , 0.50000E+00, 0.60000E+00, 0.80000E+00
                    , 0.10000E+01, 0.12500E+01, 0.15000E+01
                    , 0.20000E+01, 0.30000E+01, 0.40000E+01
                    , 0.50000E+01, 0.60000E+01, 0.80000E+01
		    , 0.10000E+02, 0.15000E+02, 0.20000E+02 };

  return Interpolate(energyMeV,en,mu,kN);

}

//_____________________________________________________________________________
Double_t IlcDCHsim::GetMuXe(Double_t energyMeV)
{
  //
  // Returns the photon absorbtion cross section for xenon
  //

  const Int_t kN = 48;

  Double_t mu[kN] = { 9.413E+03, 8.151E+03, 7.035E+03
                    , 7.338E+03, 4.085E+03, 2.088E+03
                    , 7.780E+02, 3.787E+02, 2.408E+02
                    , 6.941E+02, 6.392E+02, 6.044E+02
                    , 8.181E+02, 7.579E+02, 6.991E+02
                    , 8.064E+02, 6.376E+02, 3.032E+02
                    , 1.690E+02, 5.743E+01, 2.652E+01
                    , 8.930E+00, 6.129E+00, 3.316E+01
                    , 2.270E+01, 1.272E+01, 7.825E+00
                    , 3.633E+00, 2.011E+00, 7.202E-01
                    , 3.760E-01, 1.797E-01, 1.223E-01
                    , 9.699E-02, 8.281E-02, 6.696E-02
                    , 5.785E-02, 5.054E-02, 4.594E-02
                    , 4.078E-02, 3.681E-02, 3.577E-02
                    , 3.583E-02, 3.634E-02, 3.797E-02
		    , 3.987E-02, 4.445E-02, 4.815E-02 };

  Double_t en[kN] = { 1.00000E-03, 1.07191E-03, 1.14900E-03
                    , 1.14900E-03, 1.50000E-03, 2.00000E-03
                    , 3.00000E-03, 4.00000E-03, 4.78220E-03
                    , 4.78220E-03, 5.00000E-03, 5.10370E-03
                    , 5.10370E-03, 5.27536E-03, 5.45280E-03
                    , 5.45280E-03, 6.00000E-03, 8.00000E-03
                    , 1.00000E-02, 1.50000E-02, 2.00000E-02
                    , 3.00000E-02, 3.45614E-02, 3.45614E-02
                    , 4.00000E-02, 5.00000E-02, 6.00000E-02
                    , 8.00000E-02, 1.00000E-01, 1.50000E-01
                    , 2.00000E-01, 3.00000E-01, 4.00000E-01
                    , 5.00000E-01, 6.00000E-01, 8.00000E-01
                    , 1.00000E+00, 1.25000E+00, 1.50000E+00
                    , 2.00000E+00, 3.00000E+00, 4.00000E+00
                    , 5.00000E+00, 6.00000E+00, 8.00000E+00
		    , 1.00000E+01, 1.50000E+01, 2.00000E+01 };

  return Interpolate(energyMeV,en,mu,kN);

}

//_____________________________________________________________________________
Double_t IlcDCHsim::GetMuBu(Double_t energyMeV)
{
  //
  // Returns the photon absorbtion cross section for isobutane
  //

  const Int_t kN = 36;

  Double_t mu[kN] = { 0.38846E+03, 0.12291E+03, 0.53225E+02
                    , 0.16091E+02, 0.69114E+01, 0.36541E+01
                    , 0.22282E+01, 0.11149E+01, 0.72887E+00
                    , 0.45053E+00, 0.38167E+00, 0.33920E+00
                    , 0.32155E+00, 0.30949E+00, 0.29960E+00
                    , 0.28317E+00, 0.26937E+00, 0.24228E+00
                    , 0.22190E+00, 0.19289E+00, 0.17288E+00
                    , 0.15789E+00, 0.14602E+00, 0.12829E+00
                    , 0.11533E+00, 0.10310E+00, 0.93790E-01
                    , 0.80117E-01, 0.63330E-01, 0.53229E-01
                    , 0.46390E-01, 0.41425E-01, 0.34668E-01
		    , 0.30267E-01, 0.23910E-01, 0.20509E-01 };

  Double_t en[kN] = { 0.10000E-02, 0.15000E-02, 0.20000E-02
                    , 0.30000E-02, 0.40000E-02, 0.50000E-02
                    , 0.60000E-02, 0.80000E-02, 0.10000E-01
                    , 0.15000E-01, 0.20000E-01, 0.30000E-01
                    , 0.40000E-01, 0.50000E-01, 0.60000E-01
                    , 0.80000E-01, 0.10000E+00, 0.15000E+00
                    , 0.20000E+00, 0.30000E+00, 0.40000E+00
                    , 0.50000E+00, 0.60000E+00, 0.80000E+00
                    , 0.10000E+01, 0.12500E+01, 0.15000E+01
                    , 0.20000E+01, 0.30000E+01, 0.40000E+01
                    , 0.50000E+01, 0.60000E+01, 0.80000E+01
		    , 0.10000E+02, 0.15000E+02, 0.20000E+02 };

  return Interpolate(energyMeV,en,mu,kN);

}

//_____________________________________________________________________________
Double_t IlcDCHsim::GetMuMy(Double_t energyMeV)
{
  //
  // Returns the photon absorbtion cross section for mylar
  //

  const Int_t kN = 36;

  Double_t mu[kN] = { 2.911E+03, 9.536E+02, 4.206E+02
                    , 1.288E+02, 5.466E+01, 2.792E+01
                    , 1.608E+01, 6.750E+00, 3.481E+00
                    , 1.132E+00, 5.798E-01, 3.009E-01
                    , 2.304E-01, 2.020E-01, 1.868E-01
                    , 1.695E-01, 1.586E-01, 1.406E-01
                    , 1.282E-01, 1.111E-01, 9.947E-02
                    , 9.079E-02, 8.395E-02, 7.372E-02
                    , 6.628E-02, 5.927E-02, 5.395E-02
                    , 4.630E-02, 3.715E-02, 3.181E-02
                    , 2.829E-02, 2.582E-02, 2.257E-02
                    , 2.057E-02, 1.789E-02, 1.664E-02 };

  Double_t en[kN] = { 1.00000E-03, 1.50000E-03, 2.00000E-03
                    , 3.00000E-03, 4.00000E-03, 5.00000E-03
                    , 6.00000E-03, 8.00000E-03, 1.00000E-02
                    , 1.50000E-02, 2.00000E-02, 3.00000E-02
                    , 4.00000E-02, 5.00000E-02, 6.00000E-02
                    , 8.00000E-02, 1.00000E-01, 1.50000E-01
                    , 2.00000E-01, 3.00000E-01, 4.00000E-01
                    , 5.00000E-01, 6.00000E-01, 8.00000E-01
                    , 1.00000E+00, 1.25000E+00, 1.50000E+00
                    , 2.00000E+00, 3.00000E+00, 4.00000E+00
                    , 5.00000E+00, 6.00000E+00, 8.00000E+00
                    , 1.00000E+01, 1.50000E+01, 2.00000E+01 };

  return Interpolate(energyMeV,en,mu,kN);

}

//_____________________________________________________________________________
Double_t IlcDCHsim::GetMuN2(Double_t energyMeV)
{
  //
  // Returns the photon absorbtion cross section for nitrogen
  //

  const Int_t kN = 36;

  Double_t mu[kN] = { 3.311E+03, 1.083E+03, 4.769E+02
                    , 1.456E+02, 6.166E+01, 3.144E+01
                    , 1.809E+01, 7.562E+00, 3.879E+00
                    , 1.236E+00, 6.178E-01, 3.066E-01
                    , 2.288E-01, 1.980E-01, 1.817E-01
                    , 1.639E-01, 1.529E-01, 1.353E-01
                    , 1.233E-01, 1.068E-01, 9.557E-02
                    , 8.719E-02, 8.063E-02, 7.081E-02
                    , 6.364E-02, 5.693E-02, 5.180E-02
                    , 4.450E-02, 3.579E-02, 3.073E-02
                    , 2.742E-02, 2.511E-02, 2.209E-02
                    , 2.024E-02, 1.782E-02, 1.673E-02 };

  Double_t en[kN] = { 1.00000E-03, 1.50000E-03, 2.00000E-03
                    , 3.00000E-03, 4.00000E-03, 5.00000E-03
                    , 6.00000E-03, 8.00000E-03, 1.00000E-02
                    , 1.50000E-02, 2.00000E-02, 3.00000E-02
                    , 4.00000E-02, 5.00000E-02, 6.00000E-02
                    , 8.00000E-02, 1.00000E-01, 1.50000E-01
                    , 2.00000E-01, 3.00000E-01, 4.00000E-01
                    , 5.00000E-01, 6.00000E-01, 8.00000E-01
                    , 1.00000E+00, 1.25000E+00, 1.50000E+00
                    , 2.00000E+00, 3.00000E+00, 4.00000E+00
                    , 5.00000E+00, 6.00000E+00, 8.00000E+00
                    , 1.00000E+01, 1.50000E+01, 2.00000E+01 };

  return Interpolate(energyMeV,en,mu,kN);

}

//_____________________________________________________________________________
Double_t IlcDCHsim::GetMuO2(Double_t energyMeV)
{
  //
  // Returns the photon absorbtion cross section for oxygen
  //

  const Int_t kN = 36;

  Double_t mu[kN] = { 4.590E+03, 1.549E+03, 6.949E+02
                    , 2.171E+02, 9.315E+01, 4.790E+01
                    , 2.770E+01, 1.163E+01, 5.952E+00
                    , 1.836E+00, 8.651E-01, 3.779E-01
                    , 2.585E-01, 2.132E-01, 1.907E-01
                    , 1.678E-01, 1.551E-01, 1.361E-01
                    , 1.237E-01, 1.070E-01, 9.566E-02
                    , 8.729E-02, 8.070E-02, 7.087E-02
                    , 6.372E-02, 5.697E-02, 5.185E-02
                    , 4.459E-02, 3.597E-02, 3.100E-02
                    , 2.777E-02, 2.552E-02, 2.263E-02
                    , 2.089E-02, 1.866E-02, 1.770E-02 };

  Double_t en[kN] = { 1.00000E-03, 1.50000E-03, 2.00000E-03
                    , 3.00000E-03, 4.00000E-03, 5.00000E-03
                    , 6.00000E-03, 8.00000E-03, 1.00000E-02
                    , 1.50000E-02, 2.00000E-02, 3.00000E-02
                    , 4.00000E-02, 5.00000E-02, 6.00000E-02
                    , 8.00000E-02, 1.00000E-01, 1.50000E-01
                    , 2.00000E-01, 3.00000E-01, 4.00000E-01
                    , 5.00000E-01, 6.00000E-01, 8.00000E-01
                    , 1.00000E+00, 1.25000E+00, 1.50000E+00
                    , 2.00000E+00, 3.00000E+00, 4.00000E+00
                    , 5.00000E+00, 6.00000E+00, 8.00000E+00
                    , 1.00000E+01, 1.50000E+01, 2.00000E+01 };

  return Interpolate(energyMeV,en,mu,kN);

}

//_____________________________________________________________________________
Double_t IlcDCHsim::GetMuHe(Double_t energyMeV)
{
  //
  // Returns the photon absorbtion cross section for helium
  //

  const Int_t kN = 36;

  Double_t mu[kN] = { 6.084E+01, 1.676E+01, 6.863E+00
                    , 2.007E+00, 9.329E-01, 5.766E-01
                    , 4.195E-01, 2.933E-01, 2.476E-01
                    , 2.092E-01, 1.960E-01, 1.838E-01
                    , 1.763E-01, 1.703E-01, 1.651E-01
                    , 1.562E-01, 1.486E-01, 1.336E-01
                    , 1.224E-01, 1.064E-01, 9.535E-02
                    , 8.707E-02, 8.054E-02, 7.076E-02
                    , 6.362E-02, 5.688E-02, 5.173E-02
                    , 4.422E-02, 3.503E-02, 2.949E-02
                    , 2.577E-02, 2.307E-02, 1.940E-02
                    , 1.703E-02, 1.363E-02, 1.183E-02 };

  Double_t en[kN] = { 1.00000E-03, 1.50000E-03, 2.00000E-03
                    , 3.00000E-03, 4.00000E-03, 5.00000E-03
                    , 6.00000E-03, 8.00000E-03, 1.00000E-02
                    , 1.50000E-02, 2.00000E-02, 3.00000E-02
                    , 4.00000E-02, 5.00000E-02, 6.00000E-02
                    , 8.00000E-02, 1.00000E-01, 1.50000E-01
                    , 2.00000E-01, 3.00000E-01, 4.00000E-01
                    , 5.00000E-01, 6.00000E-01, 8.00000E-01
                    , 1.00000E+00, 1.25000E+00, 1.50000E+00
                    , 2.00000E+00, 3.00000E+00, 4.00000E+00
                    , 5.00000E+00, 6.00000E+00, 8.00000E+00
                    , 1.00000E+01, 1.50000E+01, 2.00000E+01 };

  return Interpolate(energyMeV,en,mu,kN);

}

//_____________________________________________________________________________
Double_t IlcDCHsim::GetMuAi(Double_t energyMeV)
{
  //
  // Returns the photon absorbtion cross section for air
  // Implemented by Oliver Busch
  //

  const Int_t kN = 38;

  Double_t mu[kN] = { 0.35854E+04, 0.11841E+04, 0.52458E+03,
                      0.16143E+03, 0.14250E+03, 0.15722E+03,
                      0.77538E+02, 0.40099E+02, 0.23313E+02,
                      0.98816E+01, 0.51000E+01, 0.16079E+01,
                      0.77536E+00, 0.35282E+00, 0.24790E+00,
                      0.20750E+00, 0.18703E+00, 0.16589E+00,
                      0.15375E+00, 0.13530E+00, 0.12311E+00,
                      0.10654E+00, 0.95297E-01, 0.86939E-01,
                      0.80390E-01, 0.70596E-01, 0.63452E-01,
                      0.56754E-01, 0.51644E-01, 0.44382E-01,
                      0.35733E-01, 0.30721E-01, 0.27450E-01,
                      0.25171E-01, 0.22205E-01, 0.20399E-01,
                      0.18053E-01, 0.18057E-01 };



  Double_t en[kN] = { 0.10000E-02, 0.15000E-02, 0.20000E-02,
                      0.30000E-02, 0.32029E-02, 0.32029E-02,
                      0.40000E-02, 0.50000E-02, 0.60000E-02,
                      0.80000E-02, 0.10000E-01, 0.15000E-01,
                      0.20000E-01, 0.30000E-01, 0.40000E-01,
                      0.50000E-01, 0.60000E-01, 0.80000E-01,
                      0.10000E+00, 0.15000E+00, 0.20000E+00,
                      0.30000E+00, 0.40000E+00, 0.50000E+00,
                      0.60000E+00, 0.80000E+00, 0.10000E+01,
                      0.12500E+01, 0.15000E+01, 0.20000E+01,
                      0.30000E+01, 0.40000E+01, 0.50000E+01,
                      0.60000E+01, 0.80000E+01, 0.10000E+02,
                      0.15000E+02, 0.20000E+02 };

  return Interpolate(energyMeV,en,mu,kN);

}

//_____________________________________________________________________________
Double_t IlcDCHsim::Interpolate(Double_t energyMeV
                              , Double_t *en, Double_t *mu, Int_t n)
{
  //
  // Interpolates the photon absorbtion cross section 
  // for a given energy <energyMeV>.
  //

  Double_t de    = 0;
  Int_t    index = 0;
  Int_t    istat = Locate(en,n,energyMeV,index,de);
  if (istat == 0) {
    return (mu[index] - de * (mu[index]   - mu[index+1]) 
                           / (en[index+1] - en[index]  ));
  }
  else {
    return 0.0; 
  }

}

//_____________________________________________________________________________
Int_t IlcDCHsim::Locate(Double_t *xv, Int_t n, Double_t xval
                      , Int_t &kl, Double_t &dx) 
{
  //
  // Locates a point (xval) in a 1-dim grid (xv(n))
  //

  if (xval >= xv[n-1]) return  1;
  if (xval <  xv[0])   return -1;

  Int_t km;
  Int_t kh = n - 1;

  kl = 0;
  while (kh - kl > 1) {
    if (xval < xv[km = (kl+kh)/2]) kh = km; 
    else                           kl = km;
  }
  if (xval < xv[kl] || xval > xv[kl+1] || kl >= n-1) {
    printf("Locate failed xv[%d] %f xval %f xv[%d] %f!!!\n"
          ,kl,xv[kl],xval,kl+1,xv[kl+1]);
    exit(1);
  }

  dx = xval - xv[kl];

  return 0;

}

//_____________________________________________________________________________
Int_t IlcDCHsim::SelectNFoils(Float_t p)
{
  //
  // Selects the number of foils corresponding to the momentum
  //

  Int_t foils = fNFoils[fNFoilsDim-1];

  for (Int_t iFoil = 0; iFoil < fNFoilsDim; iFoil++) {
    if (p < fNFoilsUp[iFoil]) {
      foils = fNFoils[iFoil];
      break;
    }
  }

  return foils;

}
