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

/* $Id: IlcPVBARCpvRecPoint.cxx 44661 2010-10-27 09:18:21Z hristov $ */

/* History of cvs commits:
 *
 * $Log$
 * Revision 1.26  2007/06/18 07:02:44  kharlov
 * Change the signature of EvalLocalPosition() to obey the method virtuality from the parent class
 *
 * Revision 1.25  2007/03/06 06:47:28  kharlov
 * DP:Possibility to use actual vertex position added
 */

//_________________________________________________________________________
//  RecPoint implementation for PVBAR-CPV
//  An CpvRecPoint is a cluster of digits   
//-- Author: Yuri Kharlov
//  (after Dmitri Peressounko (RRC KI & SUBATECH))
//  30 October 2000 

// --- ROOT system ---

#include "TMath.h" 
#include "TClonesArray.h" 

// --- Standard library ---

// --- IlcRoot header files ---
#include "IlcLog.h"
#include "IlcPVBARGeometry.h" 
#include "IlcPVBARDigit.h"
#include "IlcPVBARCpvRecPoint.h"
#include "IlcPVBARLoader.h"

ClassImp(IlcPVBARCpvRecPoint)

//____________________________________________________________________________
IlcPVBARCpvRecPoint::IlcPVBARCpvRecPoint() : 
  IlcPVBAREmcRecPoint(),
  fLengX(-1),
  fLengZ(-1)
{
  // ctor
}

//____________________________________________________________________________
IlcPVBARCpvRecPoint::IlcPVBARCpvRecPoint(const char * opt) : 
  IlcPVBAREmcRecPoint(opt),
  fLengX(-1),
  fLengZ(-1)
{
   // ctor
}

//____________________________________________________________________________
IlcPVBARCpvRecPoint::~IlcPVBARCpvRecPoint()
{
  // dtor
}

//____________________________________________________________________________
Bool_t IlcPVBARCpvRecPoint::AreNeighbours(IlcPVBARDigit * digit1, IlcPVBARDigit * digit2 ) const
{
  // Tells if (true) or not (false) two digits are neighbors)
  
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
Int_t IlcPVBARCpvRecPoint::Compare(const TObject * obj) const
{
  // Compares two RecPoints according to their position in the PVBAR modules

  Float_t delta = 1 ; //Width of "Sorting row". If you changibg this 
                      //value (what is senseless) change as vell delta in
                      //IlcPVBARTrackSegmentMakerv* and other RecPoints...

  Int_t rv ; 

  IlcPVBARCpvRecPoint * clu  = (IlcPVBARCpvRecPoint *) obj ; 
  
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
void IlcPVBARCpvRecPoint::ExecuteEvent(Int_t, Int_t, Int_t ) /*const*/
{
//   // Execute action corresponding to one event
//   //  This member function is called when a IlcPVBARRecPoint is clicked with the locator
//   //
//   //  If Left button is clicked on IlcPVBARRecPoint, the digits are switched on    
//   //  and switched off when the mouse button is released.
//   //

//   //   static Int_t pxold, pyold;

//   IlcPVBARLoader * gime =  IlcPVBARLoader::GetInstance() ; 
  
//   static TGraph *  digitgraph = 0 ;
  
//   if (!gPad->IsEditable()) return;
  
//   TH2F * histo = 0 ;
//   TCanvas * histocanvas ; 
  
//   switch (event) {
    
//   case kButton1Down: {
//     IlcPVBARDigit * digit ;
//     IlcPVBARGeometry * PVBARgeom =  (IlcPVBARGeometry *) fGeom ;
//     Int_t iDigit;
//     Int_t relid[4] ;
    
//     const Int_t kMulDigit = IlcPVBARCpvRecPoint::GetDigitsMultiplicity() ; 
//     Float_t * xi = new Float_t[kMulDigit] ; 
//     Float_t * zi = new Float_t[kMulDigit] ; 
    
//     // create the histogram for the single cluster 
//     // 1. gets histogram boundaries
//     Float_t ximax = -999. ; 
//     Float_t zimax = -999. ; 
//     Float_t ximin = 999. ; 
//     Float_t zimin = 999. ;
    
//     for(iDigit=0; iDigit<kMulDigit; iDigit++) {
//       digit = (IlcPVBARDigit *) ( gime->Digit(fDigitsList[iDigit]) ) ;
//       PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
//       PVBARgeom->RelPosInModule(relid, xi[iDigit], zi[iDigit]);
//       if ( xi[iDigit] > ximax )
// 	ximax = xi[iDigit] ; 
//       if ( xi[iDigit] < ximin )
// 	ximin = xi[iDigit] ; 
//       if ( zi[iDigit] > zimax )
// 	zimax = zi[iDigit] ; 
//       if ( zi[iDigit] < zimin )
// 	zimin = zi[iDigit] ;     
//     }
//     ximax += PVBARgeom->GetCrystalSize(0) / 2. ;
//     zimax += PVBARgeom->GetCrystalSize(2) / 2. ;
//     ximin -= PVBARgeom->GetCrystalSize(0) / 2. ;
//     zimin -= PVBARgeom->GetCrystalSize(2) / 2. ;
//     Int_t xdim = (int)( (ximax - ximin ) / PVBARgeom->GetCrystalSize(0) + 0.5  ) ; 
//     Int_t zdim = (int)( (zimax - zimin ) / PVBARgeom->GetCrystalSize(2) + 0.5 ) ;
    
//     // 2. gets the histogram title
    
//     Text_t title[100] ; 
//     sprintf(title,"Energy=%1.2f GeV ; Digits ; %d ", GetEnergy(), GetDigitsMultiplicity()) ;
    
//     if (!histo) {
//       delete histo ; 
//       histo = 0 ; 
//     }
//     histo = new TH2F("cluster3D", title,  xdim, ximin, ximax, zdim, zimin, zimax)  ;
    
//     Float_t x, z ; 
//     for(iDigit=0; iDigit<kMulDigit; iDigit++) {
//       digit = (IlcPVBARDigit *) ( gime->Digit(fDigitsList[iDigit]) ) ;
//       PVBARgeom->AbsToRelNumbering(digit->GetId(), relid) ;
//       PVBARgeom->RelPosInModule(relid, x, z);
//       histo->Fill(x, z, fEnergyList[iDigit] ) ;
//     }
    
//     if (!digitgraph) {
//       digitgraph = new TGraph(kMulDigit,xi,zi);
//       digitgraph-> SetMarkerStyle(5) ; 
//       digitgraph-> SetMarkerSize(1.) ;
//       digitgraph-> SetMarkerColor(1) ;
//       digitgraph-> Paint("P") ;
//     }
    
//     Print() ;
//     histocanvas = new TCanvas("cluser", "a single cluster", 600, 500) ; 
//     histocanvas->Draw() ; 
//     histo->Draw("lego1") ; 
    
//     delete[] xi ; 
//     delete[] zi ; 
    
//     break;
//   }
  
//   case kButton1Up: 
//     if (digitgraph) {
//       delete digitgraph  ;
//       digitgraph = 0 ;
//     }
//     break;
  
//    }
}

//____________________________________________________________________________
void IlcPVBARCpvRecPoint::EvalAll(TClonesArray * digits)
{
  // Evaluate local coordinate assuming the vertex in (000) and no inclination
  IlcPVBAREmcRecPoint::EvalAll(digits) ;
}
//____________________________________________________________________________
void IlcPVBARCpvRecPoint::EvalAll(Float_t logWeight, TVector3 &vtx, TClonesArray * digits)
{
  // wraps other methods
  TVector3 vInc(0,1,0);
  IlcPVBAREmcRecPoint::EvalAll(logWeight,vtx,digits) ;
  EvalLocalPosition(logWeight, vtx, digits,vInc) ;
  EvalClusterLengths(digits) ;
}
//____________________________________________________________________________
void IlcPVBARCpvRecPoint::EvalLocalPosition(Float_t logWeight, TVector3 & /*vtx */, TClonesArray * digits, TVector3 &/* vInc */)
{
  // Calculates the center of gravity in the local PVBAR-module coordinates 

  Float_t wtot = 0. ;
 
  Int_t relid[4] ;

  Float_t x = 0. ;
  Float_t z = 0. ;
  
  IlcPVBARDigit * digit ;

  IlcPVBARGeometry * PVBARgeom =  IlcPVBARGeometry::GetInstance();
  
  Int_t iDigit;

  for(iDigit=0; iDigit<fMulDigit; iDigit++) {
    digit = (IlcPVBARDigit *) digits->At(fDigitsList[iDigit]);

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

  if (wtot != 0) {
    x /= wtot ;
    z /= wtot ;
  } else {
    x = -1e6 ;
    z = -1e6 ;
    if (fMulDigit != 0) 
      IlcWarning(Form("Too low log weight factor to evaluate cluster's center" )) ;
  }
  fLocPos.SetX(x)  ;
  fLocPos.SetY(0.) ;
  fLocPos.SetZ(z)  ;

}

//____________________________________________________________________________
void IlcPVBARCpvRecPoint::EvalClusterLengths(TClonesArray * digits)
{
  //Modified 15.03.2001 by Dmitri Peressounko
  
  // Calculates the cluster lengths along X and Z axes
  // These characteristics are needed for CPV to tune
  // digitization+reconstruction to experimental data
  // Yuri Kharlov. 24 October 2000

  Int_t relid[4] ;

  IlcPVBARDigit * digit ;

  IlcPVBARGeometry * PVBARgeom =  IlcPVBARGeometry::GetInstance();

  const Int_t kMaxLeng=20;
  Int_t idX[kMaxLeng], idZ[kMaxLeng];
  fLengX = 0;
  fLengZ = 0;
  Bool_t dejavu;

  for(Int_t iDigit=0; iDigit<fMulDigit; iDigit++) {
    digit = (IlcPVBARDigit *) digits->At(fDigitsList[iDigit]) ;
    Int_t absId = digit->GetId();
    PVBARgeom->AbsToRelNumbering(absId, relid) ;

    Int_t i;
    dejavu=kFALSE;
    for (i=0; i<fLengX; i++) if (relid[2]==idX[i]) { dejavu=kTRUE; break; }
    if (!dejavu) {
      idX[fLengX]=relid[2];
      fLengX++;
      fLengX = TMath::Min(fLengX,kMaxLeng);
    }

    dejavu=kFALSE;
    for (i=0; i<fLengZ; i++) if (relid[3]==idZ[i]) { dejavu=kTRUE; break; }
    if (!dejavu) {
      idZ[fLengZ]=relid[3];
      fLengZ++;
      fLengZ = TMath::Min(fLengZ,kMaxLeng);
    }
  }
}

//____________________________________________________________________________
void IlcPVBARCpvRecPoint::Print(const Option_t *) const
{
  // Print the list of digits belonging to the cluster
  
  TString message ; 
  message  =  "IlcPVBARCpvRecPoint: " ;
  message +=  "Digits #   " ;
  IlcInfo(message.Data()) ; 
  
  Int_t iDigit;

  for(iDigit=0; iDigit<fMulDigit; iDigit++) 
    printf(" %d \n", fDigitsList[iDigit]) ; 

  printf("Energies: \n")  ;
  for(iDigit=0; iDigit<fMulDigit; iDigit++) 
    printf(" %f ", fEnergyList[iDigit]) ; 
  
  message  = "       Multiplicity    = %d\n" ;
  message += "       Cluster Energy  = %f\n" ;
  message += "       Stored at position %d\n" ; 
 
  printf(message.Data(), fMulDigit, fAmp, GetIndexInList() ) ; 

}
