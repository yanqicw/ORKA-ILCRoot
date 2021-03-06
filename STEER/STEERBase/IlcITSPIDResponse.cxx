/**************************************************************************
 * 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * Author: The International Lepton Collider Off-line Project.            *
 *                                                                        *
 * Part of the code has been developed by ILC Off-line Project.         *
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

/* $Id: IlcITSPIDResponse.cxx 58331 2012-08-28 10:38:13Z hristov $ */

//-----------------------------------------------------------------
// ITS PID method # 1
//           Implementation of the ITS PID class
// Very naive one... Should be made better by the detector experts...
//      Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
//-----------------------------------------------------------------
#include "TMath.h"
#include "IlcITSPIDResponse.h"
#include "IlcITSPidParams.h"
#include "IlcExternalTrackParam.h"

ClassImp(IlcITSPIDResponse)

IlcITSPIDResponse::IlcITSPIDResponse(Bool_t isMC): 
  fRes(0.13),
  fKp1(15.77),
  fKp2(4.95),
  fKp3(0.312),
  fKp4(2.14),
  fKp5(0.82)
{
  if(!isMC){
    fBBtpcits[0]=0.73;
    fBBtpcits[1]=14.68;
    fBBtpcits[2]=0.905;
    fBBtpcits[3]=1.2;
    fBBtpcits[4]=6.6;
    fBBsa[0]=2.73198E7;
    fBBsa[1]=6.92389;
    fBBsa[2]=1.90088E-6;
    fBBsa[3]=1.90088E-6;
    fBBsa[4]=3.40644E-7;
    fBBsaElectron[0]=4.05799E6;
    fBBsaElectron[1]=38.5713;
    fBBsaElectron[2]=1.46462E-7;
    fBBsaElectron[3]=1.46462E-7;
    fBBsaElectron[4]=4.40284E-7; 
    fResolSA[0]=1.;   // 0 cluster tracks should not be used
    fResolSA[1]=0.25;  // rough values for tracks with 1
    fResolSA[2]=0.131;   // value from pp 2010 run (L. Milano, 16-Jun-11)
    fResolSA[3]=0.113; // value from pp 2010 run 
    fResolSA[4]=0.104; // value from pp 2010 run
    for(Int_t i=0; i<5;i++) fResolTPCITS[i]=0.13;
  }else{
    fBBtpcits[0]=1.04;
    fBBtpcits[1]=27.14;
    fBBtpcits[2]=1.00;
    fBBtpcits[3]=0.964;
    fBBtpcits[4]=2.59;
    fBBsa[0]=2.02078E7;
    fBBsa[1]=14.0724;
    fBBsa[2]=3.84454E-7;
    fBBsa[3]=3.84454E-7;
    fBBsa[4]=2.43913E-7;
    fBBsaElectron[0]=2.26807E6;
    fBBsaElectron[1]=99.985;
    fBBsaElectron[2]=0.000714841;
    fBBsaElectron[3]=0.000259585;
    fBBsaElectron[4]=1.39412E-7;
    fResolSA[0]=1.;   // 0 cluster tracks should not be used
    fResolSA[1]=0.25;  // rough values for tracks with 1
    fResolSA[2]=0.126;   // value from pp 2010 simulations (L. Milano, 16-Jun-11)
    fResolSA[3]=0.109; // value from pp 2010 simulations
    fResolSA[4]=0.097; // value from pp 2010 simulations
    for(Int_t i=0; i<5;i++) fResolTPCITS[i]=0.13;
  }
}

/*
//_________________________________________________________________________
IlcITSPIDResponse::IlcITSPIDResponse(Double_t *param): 
  fRes(param[0]),
  fKp1(15.77),
  fKp2(4.95),
  fKp3(0.312),
  fKp4(2.14),
  fKp5(0.82)
{
  //
  //  The main constructor
  //
  for (Int_t i=0; i<5;i++) {
      fBBsa[i]=0.; 
      fBBtpcits[i]=0.;
      fResolSA[i]=0.; 
      fResolTPCITS[i]=0.;
  }
}
*/

//_________________________________________________________________________
Double_t IlcITSPIDResponse::BetheAleph(Double_t p, Double_t mass) const {
  //
  // returns IlcExternalTrackParam::BetheBloch normalized to 
  // fgMIP at the minimum
  //
  
  Double_t bb=
    IlcExternalTrackParam::BetheBlochAleph(p/mass,fKp1,fKp2,fKp3,fKp4,fKp5);
  return bb;
}

//_________________________________________________________________________
Double_t IlcITSPIDResponse::Bethe(Double_t p, Double_t mass, Bool_t isSA) const {
  //
  // returns IlcExternalTrackParam::BetheBloch normalized to 
  // fgMIP at the minimum
  //

  Double_t bg=p/mass;
  Double_t beta = bg/TMath::Sqrt(1.+ bg*bg);
  Double_t gamma=bg/beta;
  Double_t par[5];
  if(isSA){
    if(mass>0.0005 && mass<0.00052){
      //if is an electron use a specific BB parameterization
      //To be used only between 100 and 160 MeV/c
      for(Int_t ip=0; ip<5;ip++) par[ip]=fBBsaElectron[ip];
    }else{
      for(Int_t ip=0; ip<5;ip++) par[ip]=fBBsa[ip];
    }
  }else{
    for(Int_t ip=0; ip<5;ip++) par[ip]=fBBtpcits[ip];
  }
  Double_t eff=1.0;
  if(bg<par[2])
    eff=(bg-par[3])*(bg-par[3])+par[4];
  else
    eff=(par[2]-par[3])*(par[2]-par[3])+par[4];
  
  Double_t bb=0.;
  if(gamma>=0. && beta>0.){
    bb=(par[1]+2.0*TMath::Log(gamma)-beta*beta)*(par[0]/(beta*beta))*eff;
  }
  return bb;
}

//_________________________________________________________________________
Double_t IlcITSPIDResponse::GetResolution(Double_t bethe, 
					  Int_t nPtsForPid, 
					  Bool_t isSA) const {
  // 
  // Calculate expected resolution for truncated mean
  //
  Float_t r;
  if(isSA) r=fResolSA[nPtsForPid];
  else r=fResolTPCITS[nPtsForPid];
  return r*bethe;
}




//_________________________________________________________________________
void IlcITSPIDResponse::GetITSProbabilities(Float_t mom, Double_t qclu[4], Double_t condprobfun[IlcPID::kSPECIES], Bool_t isMC) const {
  //
  // Method to calculate PID probabilities for a single track
  // using the likelihood method
  //
  const Int_t nLay = 4;
  const Int_t nPart= 4;

  static IlcITSPidParams pars(isMC);  // Pid parametrisation parameters
  
  Double_t itsProb[nPart] = {1,1,1,1}; // e, p, K, pi

  for (Int_t iLay = 0; iLay < nLay; iLay++) {
    if (qclu[iLay] <= 50.)
      continue;

    Float_t dedx = qclu[iLay];
    Float_t layProb = pars.GetLandauGausNorm(dedx,IlcPID::kProton,mom,iLay+3);
    itsProb[0] *= layProb;
    
    layProb = pars.GetLandauGausNorm(dedx,IlcPID::kKaon,mom,iLay+3);
    itsProb[1] *= layProb;
    
    layProb = pars.GetLandauGausNorm(dedx,IlcPID::kPion,mom,iLay+3);
    itsProb[2] *= layProb;
   
    layProb = pars.GetLandauGausNorm(dedx,IlcPID::kElectron,mom,iLay+3);
    itsProb[3] *= layProb;
  }

  // Normalise probabilities
  Double_t sumProb = 0;
  for (Int_t iPart = 0; iPart < nPart; iPart++) {
    sumProb += itsProb[iPart];
  }
  sumProb += itsProb[2]; // muon cannot be distinguished from pions

  for (Int_t iPart = 0; iPart < nPart; iPart++) {
    itsProb[iPart]/=sumProb;
  }
  condprobfun[IlcPID::kElectron] = itsProb[3];
  condprobfun[IlcPID::kMuon] = itsProb[2];
  condprobfun[IlcPID::kPion] = itsProb[2];
  condprobfun[IlcPID::kKaon] = itsProb[1];
  condprobfun[IlcPID::kProton] = itsProb[0];
  return;
}

//_________________________________________________________________________
Int_t IlcITSPIDResponse::GetParticleIdFromdEdxVsP(Float_t mom, Float_t signal, Bool_t isSA) const{
  // method to get particle identity with simple cuts on dE/dx vs. momentum

  Double_t massp=IlcPID::ParticleMass(IlcPID::kProton);
  Double_t massk=IlcPID::ParticleMass(IlcPID::kKaon);
  Double_t bethep=Bethe(mom,massp,isSA);
  Double_t bethek=Bethe(mom,massk,isSA);
  if(signal>(0.5*(bethep+bethek))) return IlcPID::kProton;
  Double_t masspi=IlcPID::ParticleMass(IlcPID::kPion);
  Double_t bethepi=Bethe(mom,masspi,isSA);
  if(signal>(0.5*(bethepi+bethek))) return IlcPID::kKaon;
  return IlcPID::kPion;
    
}
