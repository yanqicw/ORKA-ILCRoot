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

/* $Id: IlcPVBAREmcRecPoint.cxx 47523 2011-02-17 14:35:05Z policheh $ */

/* History of cvs commits:
 *
 * $Log$
 * Revision 1.59  2007/10/18 15:12:22  kharlov
 * Moved MakePrimary to EMCRecPoint to rpduce correct order of primaries
 *
 * Revision 1.58  2007/04/16 09:03:37  kharlov
 * Incedent angle correction fixed
 *
 * Revision 1.57  2007/04/05 10:18:58  policheh
 * Introduced distance to nearest bad crystal.
 *
 * Revision 1.56  2007/03/06 06:47:28  kharlov
 * DP:Possibility to use actual vertex position added
 *
 * Revision 1.55  2007/01/19 20:31:19  kharlov
 * Improved formatting for Print()
 */

//_________________________________________________________________________
//  RecPoint implementation for PVBAR-EMC 
//  An EmcRecPoint is a cluster of digits   
//--
//-- Author: Dmitri Peressounko (RRC KI & SUBATECH)


// --- ROOT system ---
#include "TH2.h"
#include "TMath.h" 
#include "TCanvas.h" 
#include "TGraph.h"

// --- Standard library ---

// --- IlcRoot header files ---
#include "IlcLog.h"
#include "IlcPVBARLoader.h"
#include "IlcGenerator.h"
#include "IlcPVBARGeometry.h"
#include "IlcPVBARDigit.h"
#include "IlcPVBAREmcRecPoint.h"
#include "IlcPVBARReconstructor.h"
 
ClassImp(IlcPVBAREmcRecPoint)

//____________________________________________________________________________
IlcPVBAREmcRecPoint::IlcPVBAREmcRecPoint() : 
  IlcPVBARRecPoint(),
  fCoreEnergy(0.), fDispersion(0.),
  fEnergyList(0), fTime(-1.), fNExMax(0),
  fM2x(0.), fM2z(0.), fM3x(0.), fM4z(0.),
  fPhixe(0.), fDistToBadCrystal(-1),fDebug(0)
{
  // ctor
  fMulDigit   = 0 ;  
  fAmp   = 0. ;   
  fLocPos.SetX(1000000.)  ;      //Local position should be evaluated

  fLambda[0] = 0.;
  fLambda[1] = 0.;

}

//____________________________________________________________________________
IlcPVBAREmcRecPoint::IlcPVBAREmcRecPoint(const char * opt) : 
  IlcPVBARRecPoint(opt),
  fCoreEnergy(0.), fDispersion(0.),
  fEnergyList(0), fTime(-1.), fNExMax(0),
  fM2x(0.), fM2z(0.), fM3x(0.), fM4z(0.),
  fPhixe(0.), fDistToBadCrystal(-1), fDebug(0)
{
  // ctor
  fMulDigit   = 0 ;  
  fAmp   = 0. ;   
  fLocPos.SetX(1000000.)  ;      //Local position should be evaluated
  
  fLambda[0] = 0.;
  fLambda[1] = 0.;
}

//____________________________________________________________________________
IlcPVBAREmcRecPoint::IlcPVBAREmcRecPoint(const IlcPVBAREmcRecPoint & rp) : 
  IlcPVBARRecPoint(rp),
  fCoreEnergy(rp.fCoreEnergy), fDispersion(rp.fDispersion),
  fEnergyList(0), fTime(rp.fTime), fNExMax(rp.fNExMax),
  fM2x(rp.fM2x), fM2z(rp.fM2z), fM3x(rp.fM3x), fM4z(rp.fM4z),
  fPhixe(rp.fPhixe), fDistToBadCrystal(rp.fDistToBadCrystal), fDebug(rp.fDebug)
{
  // cpy ctor
  fMulDigit   = rp.fMulDigit ;  
  fAmp        = rp.fAmp ;   
  if (rp.fMulDigit>0) fEnergyList = new Float_t[rp.fMulDigit] ;
  for(Int_t index = 0 ; index < fMulDigit ; index++) 
    fEnergyList[index] = rp.fEnergyList[index] ;

  for(Int_t i=0; i<2; i++) {
    fLambda[i] = rp.fLambda[i];
  }
}

//____________________________________________________________________________
IlcPVBAREmcRecPoint::~IlcPVBAREmcRecPoint()
{
  // dtor
  if ( fEnergyList )
    delete[] fEnergyList ; 
}

//____________________________________________________________________________
void IlcPVBAREmcRecPoint::AddDigit(IlcPVBARDigit & digit, Float_t Energy, Float_t time)
{
  // Adds a digit to the RecPoint
  // and accumulates the total amplitude and the multiplicity 
  
  if(fEnergyList == 0)
    fEnergyList =  new Float_t[fMaxDigit]; 

  if ( fMulDigit >= fMaxDigit ) { // increase the size of the lists 
    fMaxDigit*=2 ; 
    Int_t * tempo = new Int_t[fMaxDigit]; 
    Float_t * tempoE =  new Float_t[fMaxDigit];

    Int_t index ;     
    for ( index = 0 ; index < fMulDigit ; index++ ){
      tempo[index]  = fDigitsList[index] ;
      tempoE[index] = fEnergyList[index] ; 
    }
    
    delete [] fDigitsList ; 
    fDigitsList =  new Int_t[fMaxDigit];
 
    delete [] fEnergyList ;
    fEnergyList =  new Float_t[fMaxDigit];

    for ( index = 0 ; index < fMulDigit ; index++ ){
      fDigitsList[index] = tempo[index] ;
      fEnergyList[index] = tempoE[index] ; 
    }
 
    delete [] tempo ;
    delete [] tempoE ; 
  } // if
  
  //time
  Bool_t isMax=kTRUE ;
  for(Int_t index = 0 ; index < fMulDigit ; index++ ){
    if(fEnergyList[index]>Energy){
      isMax=kFALSE ;
      break ;
    }
  }
  if(isMax){
    fTime=time ;
  }
  //Alternative time calculation - still to be validated
  // fTime = (fTime*fAmp + time*Energy)/(fAmp+Energy) ;

  fDigitsList[fMulDigit]   = digit.GetIndexInList()  ; 
  fEnergyList[fMulDigit]   = Energy ;
  fMulDigit++ ; 
  fAmp += Energy ; 
  EvalPVBARMod(&digit) ;
}

//____________________________________________________________________________
Bool_t IlcPVBAREmcRecPoint::AreNeighbours(IlcPVBARDigit * digit1, IlcPVBARDigit * digit2 ) const
{
  // Tells if (true) or not (false) two digits are neighbors
  
  Bool_t aren = kFALSE ;
  
  IlcPVBARGeometry * PVBARgeom =  IlcPVBARGeometry::GetInstance() ;

  Int_t relid1[4] ; 
  PVBARgeom->AbsToRelNumbering(digit1->GetId(), relid1) ; 

  Int_t relid2[4] ; 
  PVBARgeom->AbsToRelNumbering(digit2->GetId(), relid2) ; 
  
  Int_t rowdiff = TMath::Abs( relid1[2] - relid2[2] ) ;  
  Int_t coldiff = TMath::Abs( relid1[3] - relid2[3] ) ;  

  if (( coldiff <= 1 )  && ( rowdiff <= 1 ) && (coldiff + rowdiff > 0)) 
    aren = kTRUE ;
  
  return aren ;
}

//____________________________________________________________________________
Int_t IlcPVBAREmcRecPoint::Compare(const TObject * obj) const
{
  // Compares two RecPoints according to their position in the PVBAR modules
  
  const Float_t delta = 1 ; //Width of "Sorting row". If you changibg this 
                      //value (what is senseless) change as vell delta in
                      //IlcPVBARTrackSegmentMakerv* and other RecPoints...
  Int_t rv ; 

  IlcPVBAREmcRecPoint * clu = (IlcPVBAREmcRecPoint *)obj ; 

 
  Int_t PVBARmod1 = GetPVBARMod() ;
  Int_t PVBARmod2 = clu->GetPVBARMod() ;

  TVector3 locpos1; 
  GetLocalPosition(locpos1) ;
  TVector3 locpos2;  
  clu->GetLocalPosition(locpos2) ;  

  if(PVBARmod1 == PVBARmod2 ) {
    Int_t rowdif = (Int_t)TMath::Ceil(locpos1.X()/delta)-(Int_t)TMath::Ceil(locpos2.X()/delta) ;
    if (rowdif> 0) 
      rv = 1 ;
     else if(rowdif < 0) 
       rv = -1 ;
    else if(locpos1.Z()>locpos2.Z()) 
      rv = -1 ;
    else 
      rv = 1 ; 
     }

  else {
    if(PVBARmod1 < PVBARmod2 ) 
      rv = -1 ;
    else 
      rv = 1 ;
  }

  return rv ; 
}
//______________________________________________________________________________
void IlcPVBAREmcRecPoint::ExecuteEvent(Int_t event, Int_t, Int_t) /*const*/
{
  
  // Execute action corresponding to one event
  //  This member function is called when a IlcPVBARRecPoint is clicked with the locator
  //
  //  If Left button is clicked on IlcPVBARRecPoint, the digits are switched on    
  //  and switched off when the mouse button is released.
  
  
  IlcPVBARGeometry * PVBARgeom =  IlcPVBARGeometry::GetInstance();
  
  static TGraph *  digitgraph = 0 ;
  
  if (!gPad->IsEditable()) return;
  
  TH2F * histo = 0 ;
  TCanvas * histocanvas ; 

  
  //try to get run loader from default event folder
  IlcRunLoader* rn = IlcRunLoader::GetRunLoader(IlcConfig::GetDefaultEventFolderName());
  if (rn == 0x0) 
    {
      IlcError(Form("Cannot find Run Loader in Default Event Folder"));
      return;
    }
  IlcPVBARLoader* PVBARLoader = dynamic_cast<IlcPVBARLoader*>(rn->GetLoader("PVBARLoader"));
  if (PVBARLoader == 0x0) 
    {
      IlcError(Form("Cannot find PVBAR Loader from Run Loader"));
      return;
    }
  
  
  const TClonesArray * digits = PVBARLoader->Digits() ;
  
  switch (event) {
    
  case kButton1Down: {
    IlcPVBARDigit * digit ;
    Int_t iDigit;
    Int_t relid[4] ;
    
    const Int_t kMulDigit = IlcPVBAREmcRecPoint::GetDigitsMultiplicity() ; 
    Float_t * xi = new Float_t[kMulDigit] ; 
    Float_t * zi = new Float_t[kMulDigit] ; 
    
    // create the histogram for the single cluster 
    // 1. gets histogram boundaries
    Float_t ximax = -999. ; 
    Float_t zimax = -999. ; 
    Float_t ximin = 999. ; 
    Float_t zimin = 999. ;
    
    for(iDigit=0; iDigit<kMulDigit; iDigit++) {
      digit = (IlcPVBARDigit *) digits->At(fDigitsList[iDigit])  ;
      PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
      PVBARgeom->RelPosInModule(relid, xi[iDigit], zi[iDigit]);
      if ( xi[iDigit] > ximax )
	ximax = xi[iDigit] ; 
      if ( xi[iDigit] < ximin )
	ximin = xi[iDigit] ; 
      if ( zi[iDigit] > zimax )
	zimax = zi[iDigit] ; 
      if ( zi[iDigit] < zimin )
	zimin = zi[iDigit] ;     
    }
    ximax += PVBARgeom->GetCrystalSize(0) / 2. ;
    zimax += PVBARgeom->GetCrystalSize(2) / 2. ;
    ximin -= PVBARgeom->GetCrystalSize(0) / 2. ;
    zimin -= PVBARgeom->GetCrystalSize(2) / 2. ;
    Int_t xdim = (int)( (ximax - ximin ) / PVBARgeom->GetCrystalSize(0) + 0.5  ) ; 
    Int_t zdim = (int)( (zimax - zimin ) / PVBARgeom->GetCrystalSize(2) + 0.5 ) ;
    
    // 2. gets the histogram title
    
    Text_t title[100] ; 
    snprintf(title,100,"Energy=%1.2f GeV ; Digits ; %d ", GetEnergy(), GetDigitsMultiplicity()) ;
    
    if (!histo) {
      delete histo ; 
      histo = 0 ; 
    }
    histo = new TH2F("cluster3D", title,  xdim, ximin, ximax, zdim, zimin, zimax)  ;
    
    Float_t x, z ; 
    for(iDigit=0; iDigit<kMulDigit; iDigit++) {
      digit = (IlcPVBARDigit *) digits->At(fDigitsList[iDigit])  ;
      PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
      PVBARgeom->RelPosInModule(relid, x, z);
      histo->Fill(x, z, fEnergyList[iDigit] ) ;
    }
    
    if (!digitgraph) {
      digitgraph = new TGraph(kMulDigit,xi,zi);
      digitgraph-> SetMarkerStyle(5) ; 
      digitgraph-> SetMarkerSize(1.) ;
      digitgraph-> SetMarkerColor(1) ;
      digitgraph-> Paint("P") ;
    }
    
    //    Print() ;
    histocanvas = new TCanvas("cluster", "a single cluster", 600, 500) ; 
    histocanvas->Draw() ; 
    histo->Draw("lego1") ; 
    
    delete[] xi ; 
    delete[] zi ; 
    
    break;
  }
  
  case kButton1Up: 
    if (digitgraph) {
      delete digitgraph  ;
      digitgraph = 0 ;
    }
    break;
  
   }
}

//____________________________________________________________________________
void  IlcPVBAREmcRecPoint::EvalDispersion(Float_t logWeight,TClonesArray * digits, TVector3 & /* vInc */)
{
  // Calculates the dispersion of the shower at the origine of the RecPoint
  //DP: should we correct dispersion for non-perpendicular hit????????
 
  Float_t d    = 0. ;
  Float_t wtot = 0. ;

  Float_t x = 0.;
  Float_t z = 0.;

  IlcPVBARDigit * digit ;
 
  IlcPVBARGeometry * PVBARgeom =  IlcPVBARGeometry::GetInstance();

 // Calculates the center of gravity in the local PVBAR-module coordinates 
  
  Int_t iDigit;

  for(iDigit=0; iDigit<fMulDigit; iDigit++) {
    digit = (IlcPVBARDigit *) digits->At(fDigitsList[iDigit]) ;
    Int_t relid[4] ;
    Float_t xi ;
    Float_t zi ;
    PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
    PVBARgeom->RelPosInModule(relid, xi, zi);
    if (fAmp>0 && fEnergyList[iDigit]>0) {
      Float_t w = TMath::Max( 0., logWeight + TMath::Log( fEnergyList[iDigit] / fAmp ) ) ;
      x    += xi * w ;
      z    += zi * w ;
      wtot += w ;
    }
    else
      IlcError(Form("Wrong energy %f and/or amplitude %f\n", fEnergyList[iDigit], fAmp));
  }
  if (wtot>0) {
    x /= wtot ;
    z /= wtot ;
  }
  else
    IlcError(Form("Wrong weight %f\n", wtot));


// Calculates the dispersion in coordinates 
  wtot = 0.;
  for(iDigit=0; iDigit < fMulDigit; iDigit++) {
    digit = (IlcPVBARDigit *) digits->At(fDigitsList[iDigit])  ;
    Int_t relid[4] ;
    Float_t xi ;
    Float_t zi ;
    PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
    PVBARgeom->RelPosInModule(relid, xi, zi);
    if (fAmp>0 && fEnergyList[iDigit]>0) {
      Float_t w = TMath::Max(0.,logWeight+TMath::Log(fEnergyList[iDigit]/fAmp ) ) ;
      d += w*((xi-x)*(xi-x) + (zi-z)*(zi-z) ) ; 
      wtot+=w ;
    }
    else
      IlcError(Form("Wrong energy %f and/or amplitude %f\n", fEnergyList[iDigit], fAmp));
  }
  

  if (wtot>0) {
    d /= wtot ;
  }
  else
    IlcError(Form("Wrong weight %f\n", wtot));

  fDispersion = 0;
  if (d>=0)
    fDispersion = TMath::Sqrt(d) ;

 
}
//______________________________________________________________________________
void IlcPVBAREmcRecPoint::EvalCoreEnergy(Float_t logWeight, Float_t coreRadius, TClonesArray * digits)
{
  // This function calculates energy in the core, 
  // i.e. within a radius rad = 3cm around the center. Beyond this radius
  // in accordance with shower profile the energy deposition 
  // should be less than 2%
//DP: non-perpendicular incidence??????????????

  Float_t x = 0 ;
  Float_t z = 0 ;

  IlcPVBARDigit * digit ;

  IlcPVBARGeometry * PVBARgeom =  IlcPVBARGeometry::GetInstance();

  Int_t iDigit;

// Calculates the center of gravity in the local PVBAR-module coordinates 
  Float_t wtot = 0;
  for(iDigit=0; iDigit<fMulDigit; iDigit++) {
    digit = (IlcPVBARDigit *) digits->At(fDigitsList[iDigit]) ;
    Int_t relid[4] ;
    Float_t xi ;
    Float_t zi ;
    PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
    PVBARgeom->RelPosInModule(relid, xi, zi);
    if (fAmp>0 && fEnergyList[iDigit]>0) {
      Float_t w = TMath::Max( 0., logWeight + TMath::Log( fEnergyList[iDigit] / fAmp ) ) ;
      x    += xi * w ;
      z    += zi * w ;
      wtot += w ;
    }
    else
      IlcError(Form("Wrong energy %f and/or amplitude %f\n", fEnergyList[iDigit], fAmp));
  }
  if (wtot>0) {
    x /= wtot ;
    z /= wtot ;
  }
  else
    IlcError(Form("Wrong weight %f\n", wtot));


  for(iDigit=0; iDigit < fMulDigit; iDigit++) {
    digit = (IlcPVBARDigit *) ( digits->At(fDigitsList[iDigit]) ) ;
    Int_t relid[4] ;
    Float_t xi ;
    Float_t zi ;
    PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
    PVBARgeom->RelPosInModule(relid, xi, zi);    
    Float_t distance = TMath::Sqrt((xi-x)*(xi-x)+(zi-z)*(zi-z)) ;
    if(distance < coreRadius)
      fCoreEnergy += fEnergyList[iDigit] ;
  }


}

//____________________________________________________________________________
void  IlcPVBAREmcRecPoint::EvalElipsAxis(Float_t logWeight,TClonesArray * digits, TVector3 & /* vInc */)
{
  // Calculates the axis of the shower ellipsoid

  Double_t wtot = 0. ;
  Double_t x    = 0.;
  Double_t z    = 0.;
  Double_t dxx  = 0.;
  Double_t dzz  = 0.;
  Double_t dxz  = 0.;

  IlcPVBARDigit * digit ;

  IlcPVBARGeometry * PVBARgeom =  IlcPVBARGeometry::GetInstance();

  Int_t iDigit;


  for(iDigit=0; iDigit<fMulDigit; iDigit++) {
    digit = (IlcPVBARDigit *) digits->At(fDigitsList[iDigit])  ;
    Int_t relid[4] ;
    Float_t xi ;
    Float_t zi ;
    PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
    PVBARgeom->RelPosInModule(relid, xi, zi);
    if (fAmp>0 && fEnergyList[iDigit]>0) {
      Double_t w = TMath::Max(0.,logWeight+TMath::Log(fEnergyList[iDigit]/fAmp ) ) ;
      dxx  += w * xi * xi ;
      x    += w * xi ;
      dzz  += w * zi * zi ;
      z    += w * zi ; 
      dxz  += w * xi * zi ; 
      wtot += w ;
    }
    else
      IlcError(Form("Wrong energy %f and/or amplitude %f\n", fEnergyList[iDigit], fAmp));
  }
  if (wtot>0) {
    dxx /= wtot ;
    x   /= wtot ;
    dxx -= x * x ;
    dzz /= wtot ;
    z   /= wtot ;
    dzz -= z * z ;
    dxz /= wtot ;
    dxz -= x * z ;

//   //Apply correction due to non-perpendicular incidence
//   Double_t CosX ;
//   Double_t CosZ ;
//   IlcPVBARGeometry * PVBARgeom = IlcPVBARGeometry::GetInstance() ;
//   Double_t DistanceToIP= (Double_t ) PVBARgeom->GetIPtoCrystalSurface() ;
  
//   CosX = DistanceToIP/TMath::Sqrt(DistanceToIP*DistanceToIP+x*x) ;
//   CosZ = DistanceToIP/TMath::Sqrt(DistanceToIP*DistanceToIP+z*z) ;

//   dxx = dxx/(CosX*CosX) ;
//   dzz = dzz/(CosZ*CosZ) ;
//   dxz = dxz/(CosX*CosZ) ;


    fLambda[0] =  0.5 * (dxx + dzz) + TMath::Sqrt( 0.25 * (dxx - dzz) * (dxx - dzz) + dxz * dxz )  ;
    if(fLambda[0] > 0)
      fLambda[0] = TMath::Sqrt(fLambda[0]) ;

    fLambda[1] =  0.5 * (dxx + dzz) - TMath::Sqrt( 0.25 * (dxx - dzz) * (dxx - dzz) + dxz * dxz )  ;
    if(fLambda[1] > 0) //To avoid exception if numerical errors lead to negative lambda.
      fLambda[1] = TMath::Sqrt(fLambda[1]) ;
    else
      fLambda[1]= 0. ;
  }
  else {
    IlcError(Form("Wrong weight %f\n", wtot));
    fLambda[0]=fLambda[1]=0.;
  }
}

//____________________________________________________________________________
void  IlcPVBAREmcRecPoint::EvalMoments(Float_t logWeight,TClonesArray * digits, TVector3 & /* vInc */)
{
  // Calculate the shower moments in the eigen reference system
  // M2x, M2z, M3x, M4z
  // Calculate the angle between the shower position vector and the eigen vector

  Double_t wtot = 0. ;
  Double_t x    = 0.;
  Double_t z    = 0.;
  Double_t dxx  = 0.;
  Double_t dzz  = 0.;
  Double_t dxz  = 0.;
  Double_t lambda0=0, lambda1=0;

  IlcPVBARDigit * digit ;

  IlcPVBARGeometry * PVBARgeom = IlcPVBARGeometry::GetInstance() ;

  Int_t iDigit;

  // 1) Find covariance matrix elements:
  //    || dxx dxz ||
  //    || dxz dzz ||

  Float_t xi ;
  Float_t zi ;
  Int_t relid[4] ;
  Double_t w;
  for(iDigit=0; iDigit<fMulDigit; iDigit++) {
    digit = (IlcPVBARDigit *) digits->At(fDigitsList[iDigit])  ;
    PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
    PVBARgeom->RelPosInModule(relid, xi, zi);
    if (fAmp>0 && fEnergyList[iDigit]>0) {
      w     = TMath::Max(0.,logWeight+TMath::Log(fEnergyList[iDigit]/fAmp ) ) ;
      x    += w * xi ;
      z    += w * zi ; 
      dxx  += w * xi * xi ;
      dzz  += w * zi * zi ;
      dxz  += w * xi * zi ; 
      wtot += w ;
    }
    else
      IlcError(Form("Wrong energy %f and/or amplitude %f\n", fEnergyList[iDigit], fAmp));
    
  }
  if (wtot>0) {
    x   /= wtot ;
    z   /= wtot ;
    dxx /= wtot ;
    dzz /= wtot ;
    dxz /= wtot ;
    dxx -= x * x ;
    dzz -= z * z ;
    dxz -= x * z ;

  // 2) Find covariance matrix eigen values lambda0 and lambda1

    lambda0 =  0.5 * (dxx + dzz) + TMath::Sqrt( 0.25 * (dxx - dzz) * (dxx - dzz) + dxz * dxz )  ;
    lambda1 =  0.5 * (dxx + dzz) - TMath::Sqrt( 0.25 * (dxx - dzz) * (dxx - dzz) + dxz * dxz )  ;
  }
  else {
    IlcError(Form("Wrong weight %f\n", wtot));
    lambda0=lambda1=0.;
  }

  // 3) Find covariance matrix eigen vectors e0 and e1

  TVector2 e0,e1;
  if (dxz != 0)
    e0.Set(1.,(lambda0-dxx)/dxz);
  else
    e0.Set(0.,1.);

  e0 = e0.Unit();
  e1.Set(-e0.Y(),e0.X());

  // 4) Rotate cluster tensor from (x,z) to (e0,e1) system
  //    and calculate moments M3x and M4z

  Float_t cosPhi = e0.X();
  Float_t sinPhi = e0.Y();

  Float_t xiPVBAR ;
  Float_t ziPVBAR ;
  Double_t dx3, dz3, dz4;
  wtot = 0.;
  x    = 0.;
  z    = 0.;
  dxx  = 0.;
  dzz  = 0.;
  dxz  = 0.;
  dx3  = 0.;
  dz3  = 0.;
  dz4  = 0.;
  for(iDigit=0; iDigit<fMulDigit; iDigit++) {
    digit = (IlcPVBARDigit *) digits->At(fDigitsList[iDigit])  ;
    PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
    PVBARgeom->RelPosInModule(relid, xiPVBAR, ziPVBAR);
    xi    = xiPVBAR*cosPhi + ziPVBAR*sinPhi;
    zi    = ziPVBAR*cosPhi - xiPVBAR*sinPhi;
    if (fAmp>0 && fEnergyList[iDigit]>0) {
      w     = TMath::Max(0.,logWeight+TMath::Log(fEnergyList[iDigit]/fAmp ) ) ;
      x    += w * xi ;
      z    += w * zi ; 
      dxx  += w * xi * xi ;
      dzz  += w * zi * zi ;
      dxz  += w * xi * zi ; 
      dx3  += w * xi * xi * xi;
      dz3  += w * zi * zi * zi ;
      dz4  += w * zi * zi * zi * zi ;
      wtot += w ;
    }
    else
      IlcError(Form("Wrong energy %f and/or amplitude %f\n", fEnergyList[iDigit], fAmp));
  }
  if (wtot>0) {
    x   /= wtot ;
    z   /= wtot ;
    dxx /= wtot ;
    dzz /= wtot ;
    dxz /= wtot ;
    dx3 /= wtot ;
    dz3 /= wtot ;
    dz4 /= wtot ;
    dx3 += -3*dxx*x + 2*x*x*x;
    dz4 += -4*dz3*z + 6*dzz*z*z -3*z*z*z*z;
    dxx -= x * x ;
    dzz -= z * z ;
    dxz -= x * z ;
  }
  else
    IlcError(Form("Wrong weight %f\n", wtot));

  // 5) Find an angle between cluster center vector and eigen vector e0

  Float_t phi = 0;
  if ( (x*x+z*z) > 0 ) 
    phi = TMath::ACos ((x*e0.X() + z*e0.Y()) / sqrt(x*x + z*z));

  fM2x   = lambda0;
  fM2z   = lambda1;
  fM3x   = dx3;
  fM4z   = dz4;
  fPhixe = phi;

}
//______________________________________________________________________________
void  IlcPVBAREmcRecPoint::EvalPrimaries(TClonesArray * digits)
{
  // Constructs the list of primary particles (tracks) which have contributed to this RecPoint
  
  IlcPVBARDigit * digit ;
  Int_t * tempo    = new Int_t[fMaxTrack] ;
  
  //First find digit with maximal energy deposition and copy its primaries
  Float_t emax=0.;
  Int_t imaxDigit=0;
  for(Int_t id=0; id<GetDigitsMultiplicity(); id++){
    if(emax<fEnergyList[id])
      imaxDigit=id ;
  }
  digit = static_cast<IlcPVBARDigit *>(digits->At( fDigitsList[imaxDigit] )) ; 
  Int_t nprimaries = digit->GetNprimary() ;
  if ( nprimaries > fMaxTrack ) {
    fMulTrack = - 1 ;
    Error("EvalPrimaries", "GetNprimaries ERROR > increase fMaxTrack" ) ;
    nprimaries = fMaxTrack; //skip the rest
  }
  for(fMulTrack=0; fMulTrack<nprimaries ; fMulTrack++){
    tempo[fMulTrack] = digit->GetPrimary(fMulTrack+1) ;
  }

  //Now add other digits contributions
  for (Int_t index = 0 ; index < GetDigitsMultiplicity() ; index++ ) { // all digits
    if(index==imaxDigit) //already in
      continue ; 
    digit = static_cast<IlcPVBARDigit *>(digits->At( fDigitsList[index] )) ; 
    nprimaries = digit->GetNprimary() ;
    for(Int_t ipr=0; ipr<nprimaries; ipr++){
      Int_t iprimary = digit->GetPrimary(ipr+1) ;
      Bool_t notIn=1 ;
      for(Int_t kndex = 0 ; (kndex < fMulTrack)&& notIn ; kndex++ ) { //check if not already stored
	if(iprimary == tempo[kndex]){
	  notIn = kFALSE ;
	}
      }
      if(notIn){
	if(fMulTrack<fMaxTrack){
	  tempo[fMulTrack]=iprimary ;
	  fMulTrack++ ;
	}
	else{
	  Error("EvalPrimaries", "GetNprimaries ERROR > increase fMaxTrack!!!" ) ;
	  break ;
	}
      }
    }
  } // all digits
  if(fMulTrack > 0){
    if(fTracksList)delete [] fTracksList;
    fTracksList = new Int_t[fMulTrack] ;
  }
  for(Int_t index = 0; index < fMulTrack; index++)
    fTracksList[index] = tempo[index] ;
  
  delete [] tempo ;
  
}

//____________________________________________________________________________
void IlcPVBAREmcRecPoint::EvalAll(TClonesArray * digits )
{
//   EvalCoreEnergy(logWeight, digits);
  EvalTime(digits) ;
  EvalPrimaries(digits) ;
  IlcPVBARRecPoint::EvalAll(digits) ;
}
//____________________________________________________________________________
void IlcPVBAREmcRecPoint::EvalAll(Float_t logWeight, TVector3 &vtx, TClonesArray * digits )
{
  // Evaluates all shower parameters
  TVector3 vInc ;
  EvalLocalPosition(logWeight, vtx, digits,vInc) ;
  EvalElipsAxis(logWeight, digits, vInc) ; //they are evaluated with momenta
  EvalMoments(logWeight, digits, vInc) ;
  EvalDispersion(logWeight, digits, vInc) ;
}
//____________________________________________________________________________
void IlcPVBAREmcRecPoint::EvalLocalPosition(Float_t logWeight, TVector3 &vtx, TClonesArray * digits, TVector3 &vInc)
{
  // Calculates the center of gravity in the local PVBAR-module coordinates 
  Float_t wtot = 0. ;
 
  Int_t relid[4] ;

  Float_t x = 0. ;
  Float_t z = 0. ;
  
  IlcPVBARDigit * digit ;

  IlcPVBARGeometry * PVBARgeom =  IlcPVBARGeometry::GetInstance() ;

  Int_t iDigit;

  for(iDigit=0; iDigit<fMulDigit; iDigit++) {
    digit = (IlcPVBARDigit *) digits->At(fDigitsList[iDigit]) ;

    Float_t xi ;
    Float_t zi ;
    PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
    PVBARgeom->RelPosInModule(relid, xi, zi);
    if (fAmp>0 && fEnergyList[iDigit]>0) {
      Float_t w = TMath::Max( 0., logWeight + TMath::Log( fEnergyList[iDigit] / fAmp ) ) ;
      x    += xi * w ;
      z    += zi * w ;
      wtot += w ;
    }
    else
      IlcError(Form("Wrong energy %f and/or amplitude %f\n", fEnergyList[iDigit], fAmp));
  }
  if (wtot>0) {
    x /= wtot ;
    z /= wtot ;
  }
  else
    IlcError(Form("Wrong weight %f\n", wtot));

  // Correction for the depth of the shower starting point (TDR p 127)  
  Float_t para = 0.925 ; 
  Float_t parb = 6.52 ; 

  PVBARgeom->GetIncidentVector(vtx,GetPVBARMod(),x,z,vInc) ;

  Float_t depthx = 0.; 
  Float_t depthz = 0.;
  if (fAmp>0&&vInc.Y()!=0.) {
    depthx = ( para * TMath::Log(fAmp) + parb ) * vInc.X()/TMath::Abs(vInc.Y()) ;
    depthz = ( para * TMath::Log(fAmp) + parb ) * vInc.Z()/TMath::Abs(vInc.Y()) ;
  }
  else 
    IlcError(Form("Wrong amplitude %f\n", fAmp));

  fLocPos.SetX(x - depthx)  ;
  fLocPos.SetY(0.) ;
  fLocPos.SetZ(z - depthz)  ;

}

//____________________________________________________________________________
Float_t IlcPVBAREmcRecPoint::GetMaximalEnergy(void) const
{
  // Finds the maximum energy in the cluster
  
  Float_t menergy = 0. ;

  Int_t iDigit;

  for(iDigit=0; iDigit<fMulDigit; iDigit++) {
 
    if(fEnergyList[iDigit] > menergy) 
      menergy = fEnergyList[iDigit] ;
  }
  return menergy ;
}

//____________________________________________________________________________
Int_t IlcPVBAREmcRecPoint::GetMultiplicityAtLevel(Float_t H) const
{
  // Calculates the multiplicity of digits with energy larger than H*energy 
  
  Int_t multipl   = 0 ;
  Int_t iDigit ;
  for(iDigit=0; iDigit<fMulDigit; iDigit++) {

    if(fEnergyList[iDigit] > H * fAmp) 
      multipl++ ;
  }
  return multipl ;
}

//____________________________________________________________________________
Int_t  IlcPVBAREmcRecPoint::GetNumberOfLocalMax( IlcPVBARDigit **  maxAt, Float_t * maxAtEnergy,
					       Float_t locMaxCut,TClonesArray * digits) const
{ 
  // Calculates the number of local maxima in the cluster using fLocalMaxCut as the minimum
  // energy difference between two local maxima

  IlcPVBARDigit * digit ;
  IlcPVBARDigit * digitN ;
  

  Int_t iDigitN ;
  Int_t iDigit ;

  for(iDigit = 0; iDigit < fMulDigit; iDigit++)
    maxAt[iDigit] = (IlcPVBARDigit*) digits->At(fDigitsList[iDigit])  ;

  
  for(iDigit = 0 ; iDigit < fMulDigit; iDigit++) {   
    if(maxAt[iDigit]) {
      digit = maxAt[iDigit] ;
          
      for(iDigitN = 0; iDigitN < fMulDigit; iDigitN++) {	
	if(iDigit == iDigitN)
	  continue ;
	
	digitN = (IlcPVBARDigit *) digits->At(fDigitsList[iDigitN]) ; 
	
	if ( AreNeighbours(digit, digitN) ) {
	  if (fEnergyList[iDigit] > fEnergyList[iDigitN] ) {    
	    maxAt[iDigitN] = 0 ;
	    // but may be digit too is not local max ?
	    if(fEnergyList[iDigit] < fEnergyList[iDigitN] + locMaxCut) 
	      maxAt[iDigit] = 0 ;
	  }
	  else {
	    maxAt[iDigit] = 0 ;
	    // but may be digitN too is not local max ?
	    if(fEnergyList[iDigit] > fEnergyList[iDigitN] - locMaxCut) 
	      maxAt[iDigitN] = 0 ; 
	  } 
	} // if Areneighbours
      } // while digitN
    } // slot not empty
  } // while digit
  
  iDigitN = 0 ;
  for(iDigit = 0; iDigit < fMulDigit; iDigit++) { 
    if(maxAt[iDigit]){
      maxAt[iDigitN] = maxAt[iDigit] ;
      maxAtEnergy[iDigitN] = fEnergyList[iDigit] ;
      iDigitN++ ; 
    }
  }
  return iDigitN ;
}
//____________________________________________________________________________
void IlcPVBAREmcRecPoint::EvalTime(TClonesArray * /*digits*/)
{
  // Define a rec.point time as a time in the cell with the maximum energy
  //Time already evaluated during AddDigit()

/*
  Float_t maxE = 0;
  Int_t maxAt = 0;
  for(Int_t idig=0; idig < fMulDigit; idig++){
    if(fEnergyList[idig] > maxE){
      maxE = fEnergyList[idig] ;
      maxAt = idig;
    }
  }
  fTime = ((IlcPVBARDigit*) digits->At(fDigitsList[maxAt]))->GetTime() ;
*/  
}
//____________________________________________________________________________
void IlcPVBAREmcRecPoint::Purify(Float_t threshold){
  //Removes digits below threshold

  Int_t * tempo = new Int_t[fMaxDigit]; 
  Float_t * tempoE =  new Float_t[fMaxDigit];

  Int_t mult = 0 ;
  for(Int_t iDigit=0;iDigit< fMulDigit ;iDigit++){
    if(fEnergyList[iDigit] > threshold){
      tempo[mult] = fDigitsList[iDigit] ;
      tempoE[mult] = fEnergyList[iDigit] ;
      mult++ ;
    }
  }
  
  fMulDigit = mult ;
  delete [] fDigitsList ;
  delete [] fEnergyList ;
  fDigitsList = new Int_t[fMulDigit];
  fEnergyList = new Float_t[fMulDigit];

  fAmp = 0.; //Recalculate total energy
  for(Int_t iDigit=0;iDigit< fMulDigit ;iDigit++){
     fDigitsList[iDigit] = tempo[iDigit];
     fEnergyList[iDigit] = tempoE[iDigit] ;
     fAmp+=tempoE[iDigit];
  }
      
  delete [] tempo ;
  delete [] tempoE ;

}
//____________________________________________________________________________
void IlcPVBAREmcRecPoint::Print(Option_t *) const
{
  // Print the list of digits belonging to the cluster
  
  TString message ; 
  message  = "IlcPVBAREmcRecPoint:\n" ;
  message += "Digit multiplicity = %d" ;
  message += ", cluster Energy = %f" ; 
  message += ", number of primaries = %d" ; 
  message += ", rec.point index = %d \n" ; 
  printf(message.Data(), fMulDigit, fAmp, fMulTrack,GetIndexInList() ) ;  

  Int_t iDigit;
  printf(" digits ids = ") ; 
  for(iDigit=0; iDigit<fMulDigit; iDigit++)
    printf(" %d ", fDigitsList[iDigit] ) ;  
  
  printf("\n digit energies = ") ;
  for(iDigit=0; iDigit<fMulDigit; iDigit++) 
    printf(" %f ", fEnergyList[iDigit] ) ;

  printf("\n digit primaries  ") ;
  if (fMulTrack<1) printf("... no primaries");
  for(iDigit = 0;iDigit < fMulTrack; iDigit++)
    printf(" %d ", fTracksList[iDigit]) ;
  printf("\n") ; 	

}
 
  
