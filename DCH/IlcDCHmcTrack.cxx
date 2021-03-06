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

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  DCH MC track                                                             //
//  Used for efficiency estimates and matching of reconstructed tracks       //
//  to MC particles                                                          //                    
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "IlcDCHmcTrack.h"
#include "IlcDCHgeometry.h"


ClassImp(IlcDCHmcTrack)

//_____________________________________________________________________________
IlcDCHmcTrack::IlcDCHmcTrack() 
{ 

  //
  // Default constructor 
  //

  fLab = -1; 
  fPrimary = kFALSE; 
  fMass = 0; 
  fCharge = 0;
  fN = 0; 
  
  for(Int_t ltb=0; ltb<kMAX_TB; ltb++) {
    for(Int_t plane=0; plane < 6; plane++) {
      fIndex[ltb][plane][0] = -1;
      fIndex[ltb][plane][1] = -1;
    }
  }

  for(Int_t i=0; i<6; i++) {
    for(Int_t j=0; j<3; j++) { 
      Pin[i][j]=0.; 
      Pout[i][j] = 0.;
      XYZin[i][j]=0.; 
      XYZout[i][j] = 0.;
    }
  }

}

//_____________________________________________________________________________
IlcDCHmcTrack::IlcDCHmcTrack(Int_t label, Int_t seedLabel, Bool_t primary, 
			     Float_t mass, Int_t charge, Int_t pdg) 
{ 

  //
  // Main constructor 
  //

  fLab = label; 
  fSeedLab = seedLabel; 
  fPrimary = primary; 
  fMass = mass; 
  fCharge = charge;
  fPDG = pdg;
  fN = 0; 
  
  for(Int_t ltb=0; ltb<kMAX_TB; ltb++) {
    for(Int_t plane=0; plane < 6; plane++) {
      fIndex[ltb][plane][0] = -1;
      fIndex[ltb][plane][1] = -1;
    }
  }
  
  for(Int_t i=0; i<6; i++) {
    for(Int_t j=0; j<3; j++) { 
      Pin[i][j]=0.; 
      Pout[i][j] = 0.;
      XYZin[i][j]=0.; 
      XYZout[i][j] = 0.;
    }
  }

}

//_____________________________________________________________________________
void IlcDCHmcTrack::GetPxPyPzXYZ(Double_t& px, Double_t& py, Double_t& pz, 
				 Double_t&  x, Double_t&  y, Double_t&  z, 
				 Int_t opt) const 
{
  //
  // Returns track momentum components and coordinates at the entrance 
  // (opt >= 0), or exit (opt < 0) of DCH. 
  //

  Int_t i;

  if(opt >= 0) {
    for(i = 0; i < IlcDCHgeometry::Nplan(); i++) {
      if(  Pin[i][0] * Pin[i][0]
         + Pin[i][1] * Pin[i][1]
         + Pin[i][2] * Pin[i][2] > 0.0005) break;
    }
    px = Pin[i][0];    py = Pin[i][1];   pz = Pin[i][2];
     x = XYZin[i][0];   y = XYZin[i][1];  z = XYZin[i][2];
  }
  else {
    for(i = IlcDCHgeometry::Nplan() - 1; i >= 0; i--) {
      if(  Pout[i][0] * Pout[i][0]
         + Pout[i][1] * Pout[i][1]
         + Pout[i][2] * Pout[i][2] > 0.0005) break;
    }
    px = Pout[i][0];    py = Pout[i][1];   pz = Pout[i][2];
     x = XYZout[i][0];   y = XYZout[i][1];  z = XYZout[i][2];
  }
  return;
}

//_____________________________________________________________________________
void IlcDCHmcTrack::GetPlanePxPyPz(Double_t& px, Double_t& py, Double_t& pz, 
                                   Int_t plane, Int_t opt) const 
{
  //
  // Returns momentum components at the entrance (opt >= 0), or
  // exit (opt < 0) of DCH plane <plane>. 
  //

  if(opt >= 0) {
    px = Pin[plane][0]; py = Pin[plane][1]; pz = Pin[plane][2];
  }
  else {
    px = Pout[plane][0]; py = Pout[plane][1]; pz = Pout[plane][2];
  }
  return;
}
