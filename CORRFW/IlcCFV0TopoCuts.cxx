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


///////////////////////////////////////////////////////////////////////////
//          ----   CORRECTION FRAMEWORK   ----
// Class to cut on V0 topology
//   -> support for :
//                    DCA between V0 daughters
//                    V0 daughter impact parameters wrt primary vertex
//                    cosine of V0 pointing angle
//
///////////////////////////////////////////////////////////////////////////
// author : R. Vernet (renaud.vernet@cern.ch)
///////////////////////////////////////////////////////////////////////////

#include "IlcCFV0TopoCuts.h"
#include "IlcESDv0.h"
#include "IlcESDtrack.h"
#include "IlcCFPair.h"
#include "IlcAODv0.h"
#include "IlcVEvent.h"
#include "IlcAODEvent.h"

ClassImp(IlcCFV0TopoCuts)

//______________________________________________________________
IlcCFV0TopoCuts::IlcCFV0TopoCuts() : 
  IlcCFCutBase(),
  fMaxDcaDaughters(1.e99),
  fMinDcaNeg(0),
  fMinDcaPos(0),
  fMinCosP(0),
  fEvent(0x0)
{
  //
  //default constructor
  //
}

//______________________________________________________________
IlcCFV0TopoCuts::IlcCFV0TopoCuts(const Char_t* name, const Char_t* title) : 
  IlcCFCutBase(name,title),
  fMaxDcaDaughters(1.e99),
  fMinDcaNeg(0),
  fMinDcaPos(0),
  fMinCosP(0),
  fEvent(0x0)
{
  //
}

//______________________________________________________________
IlcCFV0TopoCuts::IlcCFV0TopoCuts(const IlcCFV0TopoCuts& c) : 
  IlcCFCutBase(c),
  fMaxDcaDaughters(c.fMaxDcaDaughters),
  fMinDcaNeg(c.fMinDcaNeg),
  fMinDcaPos(c.fMinDcaPos),
  fMinCosP(c.fMinCosP),
  fEvent(c.fEvent)
{
  //
  // copy constructor
  //
}

//______________________________________________________________
IlcCFV0TopoCuts& IlcCFV0TopoCuts::operator=(const IlcCFV0TopoCuts& c) 
{
  //
  // assignment operator
  //

  if (this != &c) {
    IlcCFCutBase::operator=(c) ;
    fMaxDcaDaughters = c.fMaxDcaDaughters ;
    fMinDcaNeg       = c.fMinDcaNeg ;
    fMinDcaPos       = c.fMinDcaPos ;
    fMinCosP         = c.fMinCosP ;
    fEvent           = c.fEvent ;
  }
  return *this ;
}

//______________________________________________________________
Bool_t IlcCFV0TopoCuts::IsSelected(TObject *obj) {
  //
  // computes V0 topological variables to cut on and return true 
  // in case the V0 is accepted
  //


  IlcCFPair* pair = dynamic_cast<IlcCFPair*>(obj);
  if (!pair) return kFALSE ;

  TString className(pair->ClassName());
  if (className.CompareTo("IlcCFPair") != 0) {
    Error("IsSelected","obj must point to an IlcCFPair !");
    return kFALSE ;
  }

  IlcESDv0     * esdV0       = pair->GetESDV0();
  IlcAODv0     * aodV0       = pair->GetAODV0();
  IlcVParticle * negDaughter = pair->GetNeg();
  IlcVParticle * posDaughter = pair->GetPos();

  Double32_t dcaDaughters = 0. ;
  Double32_t cosP   = 0. ;
  Double32_t negDca = 0. ;
  Double32_t posDca = 0. ;

  if (esdV0) {
    dcaDaughters = esdV0->GetDcaV0Daughters() ;
    cosP         = esdV0->GetV0CosineOfPointingAngle() ;

    Float_t tDca[2];
    if (negDaughter) ((IlcESDtrack*)negDaughter)->GetImpactParameters(tDca[0],tDca[1]);
    else { tDca[0]=1.e+09;  tDca[1]=1.e+09;}
    negDca = TMath::Sqrt(tDca[0]*tDca[0]+tDca[1]*tDca[1]);
    if (posDaughter) ((IlcESDtrack*)posDaughter)->GetImpactParameters(tDca[0],tDca[1]);
    else { tDca[0]=1.e+09;  tDca[1]=1.e+09;}
    posDca = TMath::Sqrt(tDca[0]*tDca[0]+tDca[1]*tDca[1]);
  }
  else if (aodV0) {
    dcaDaughters = aodV0->DcaV0Daughters() ;
    negDca       = aodV0->DcaNegToPrimVertex() ;
    posDca       = aodV0->DcaPosToPrimVertex() ;
    cosP         = aodV0->CosPointingAngle(((IlcAODEvent*)fEvent)->GetPrimaryVertex()) ;
  }
  else Error("IsSelected","No V0 pointer available");

  if (dcaDaughters > fMaxDcaDaughters) return kFALSE ;
  if (cosP         < fMinCosP        ) return kFALSE ;
  if (negDca       < fMinDcaNeg      ) return kFALSE ;
  if (posDca       < fMinDcaPos      ) return kFALSE ; 

  return kTRUE ;
}
  
