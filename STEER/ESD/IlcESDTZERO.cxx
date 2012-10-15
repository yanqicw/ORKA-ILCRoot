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

//-------------------------------------------------------------------------
//                        Implemenation Class IlcESDTZERO
//   This is a class that summarizes the TZERO data for the ESD   
//   Origin: Christian Klein-Boesing, CERN, Christian.Klein-Boesing@cern.ch 
//-------------------------------------------------------------------------



#include "IlcESDTZERO.h"
#include "IlcLog.h"


ClassImp(IlcESDTZERO)

//______________________________________________________________________________
IlcESDTZERO::IlcESDTZERO() :
  TObject(),
  fT0clock(0),
  fT0zVertex(0),
  fT0timeStart(0),   
  fT0trig(0),
  fPileup(kFALSE),
  fSattelite(kFALSE),
  fMultC(0),
  fMultA(0),
  fBackground(0)

{
  for(int i = 0;i<24;i++) {
    fT0time[i] = fT0amplitude[i] = 0;
    for(Int_t iHit=0; iHit<5; iHit++) {
      fTimeFull[i][iHit] = -9999;   
      if (i==0) fOrA[iHit] = -9999; 
      if (i==0)fOrC[iHit] = -9999;  
      if (i==0) fTVDC[iHit] = -9999; 
    }
  }
  for(Int_t iHit=0; iHit<6; iHit++) fPileupTime[iHit]= -9999;
  for(int i = 0;i<3;i++) {
    fT0TOF[i] = -9999;
    fT0TOFbest[i] = -9999;
  }
}
//______________________________________________________________________________
IlcESDTZERO::IlcESDTZERO(const IlcESDTZERO &tzero ) :
  TObject(tzero),
  fT0clock(tzero.fT0clock),  
  fT0zVertex(tzero.fT0zVertex),
  fT0timeStart(tzero.fT0timeStart),
  fT0trig(tzero.fT0trig),
  fPileup(tzero.fPileup),
  fSattelite(tzero.fSattelite),
  fMultC(tzero.fMultC),
  fMultA(tzero.fMultA),
  fBackground(tzero.fBackground)
{
  // copy constuctor
  for(int i = 0;i<3;i++) {
    fT0TOF[i] = tzero.fT0TOF[i];
    fT0TOFbest[i] = tzero.fT0TOFbest[i];
  }
  for(int iHit=0; iHit<6; iHit++)  fPileupTime[iHit] = tzero.fPileupTime[iHit]; 
  for(int i = 0;i<24;i++){
    fT0time[i] = tzero.fT0time[i]; 
    fT0amplitude[i] = tzero.fT0amplitude[i];
    for(Int_t iHit=0; iHit<5; iHit++) {
      fTimeFull[i][iHit] = tzero.fTimeFull[i][iHit];   
     if (i==0)  fOrA[iHit] = tzero.fOrA[iHit]; 
     if (i==0)  fOrC[iHit] = tzero.fOrC[iHit];  
     if (i==0)  fTVDC[iHit] = tzero.fTVDC[iHit]; 
    }
  }
}
//______________________________________________________________________________
IlcESDTZERO& IlcESDTZERO::operator=(const IlcESDTZERO& tzero){
  // assigmnent operator
  if(this!=&tzero) {
    TObject::operator=(tzero);
    fT0clock = tzero.fT0clock;
    fT0zVertex = tzero.fT0zVertex;
    fT0timeStart = tzero.fT0timeStart;
    fPileup = tzero.fPileup;
    fSattelite = tzero.fSattelite;
    fBackground = tzero.fBackground;
    fMultC = tzero.fMultC;
    fMultA = tzero.fMultA;
    fT0trig = tzero.fT0trig;
    for(int i = 0;i<3;i++) {
      fT0TOF[i] = tzero.fT0TOF[i];
      fT0TOFbest[i] = tzero.fT0TOFbest[i];
    }

    for(int iHit=0; iHit<6; iHit++)  fPileupTime[iHit] = tzero.fPileupTime[iHit]; 
    for(int i = 0;i<24;i++){
      fT0time[i] = tzero.fT0time[i]; 
      fT0amplitude[i] = tzero.fT0amplitude[i];
      for(Int_t iHit=0; iHit<5; iHit++) {
	fTimeFull[i][iHit] = tzero.fTimeFull[i][iHit];   
	if (i==0) 	fOrA[iHit] = tzero.fOrA[iHit]; 
	if (i==0) 	fOrC[iHit] = tzero.fOrC[iHit];  
	if (i==0) 	fTVDC[iHit] = tzero.fTVDC[iHit]; 
    }
   }
  } 
  return *this;
}
//______________________________________________________________________________
void IlcESDTZERO::Copy(TObject &obj) const {
  
  // this overwrites the virtual TOBject::Copy()
  // to allow run time copying without casting
  // in IlcESDEvent

  if(this==&obj)return;
  IlcESDTZERO *robj = dynamic_cast<IlcESDTZERO*>(&obj);
  if(!robj)return; // not an IlcESDTZERO
  *robj = *this;

}


//______________________________________________________________________________
void IlcESDTZERO::Reset()
{
  // reset contents
  fT0clock=0;
  fT0zVertex = -9999;  
  fT0timeStart = 0;
  for(int i = 0;i<24;i++) {
    fT0time[i] = fT0amplitude[i] =  0;
    for(Int_t iHit=0; iHit<5; iHit++)  fTimeFull[i][iHit] = -9999;
  }
  for(Int_t iHit=0; iHit<5; iHit++) fOrA[iHit] = fOrC[iHit] = fTVDC[iHit] = -9999; 
  for(Int_t iHit=0; iHit<6; iHit++) fPileupTime[iHit]= -9999;
  for(int i = 0;i<3;i++) {
    fT0TOF[i] = -9999;
    fT0TOFbest[i] = -9999;
  }
}

//______________________________________________________________________________
void IlcESDTZERO::Print(const Option_t *) const
{
  // does noting fornow
  IlcInfo(Form(" Vertex %f (T0A+T0C)/2 %f #channels T0signal %f ns OrA %f ns OrC %f \n",fT0zVertex,  fT0timeStart, fT0TOF[0],fT0TOF[1],fT0TOF[2]));

  Bool_t tr[5];
  for (Int_t i=0; i<5; i++) tr[i] = fT0trig & (1<<i);
  IlcInfo(Form("T0 triggers %d %d %d %d %d",tr[0],tr[1],tr[2],tr[3],tr[4])); 

}
