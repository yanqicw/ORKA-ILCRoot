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


//_________________________________________________________________________
//  RSTACK digit: Id
//              energy
//              3 identifiers for the primary particle(s) at the origine of the digit
//  The digits are made by summing all the hits in a single RSTACK tile
//
//*-- Author: Laurent Aphecetche & Yves Schutz (SUBATECH) & Dmitri Peressounko (RRC KI & SUBATECH)


// --- ROOT system ---

#include "TMath.h"

// --- Standard library ---

// --- IlcRoot header files ---

#include "IlcRSTACKDigit.h"
#include "IlcLog.h"



ClassImp(IlcRSTACKDigit)

//____________________________________________________________________________
IlcRSTACKDigit::IlcRSTACKDigit() :
  IlcDigitNew(),
  fNprimary(0),  
  fPrimary(0x0),
  fEnergy(0.),
  fTime(0.),
  fTimeR(0.),
  fNSamplesHG(0),
  fNSamplesLG(0),
  fSamplesHG(0),
  fSamplesLG(0)
{
  // default ctor 
  fNPE[0] = 0.;
  fNPE[1] = 0.;
}

//____________________________________________________________________________
IlcRSTACKDigit::IlcRSTACKDigit(Int_t primary, Int_t id, Int_t digEnergy, Float_t *NPE, Float_t time, Int_t index) :
  fNprimary(0),
  fPrimary(0),
  fEnergy(0.f),
  fTime(0.f),
  fTimeR(0.f),
  fNSamplesHG(0),
  fNSamplesLG(0),
  fSamplesHG(0),
  fSamplesLG(0)
{  
  // ctor with all data 

  fAmp         = digEnergy ;
  fEnergy      = 0 ;
  fNPE[0]      = NPE[0];
  fNPE[1]      = NPE[1];
  fTime        = time ;
  fTimeR       = fTime ;
  fId          = id ;
  fIndexInList = index ; 
  if( primary != -1){
    fNprimary    = 1 ; 
    fPrimary = new Int_t[fNprimary] ;
    fPrimary[0]  = primary ;
  }
  else{  //If the contribution of this primary smaller than fDigitThreshold (IlcRSTACKv1)
    fNprimary = 0 ; 
    fPrimary  = 0 ;
  }
}

//____________________________________________________________________________
IlcRSTACKDigit::IlcRSTACKDigit(Int_t primary, Int_t id, Float_t energy, Float_t *NPE, Float_t time, Int_t index) :
  fNprimary(0),
  fPrimary(0),
  fEnergy(0.f),
  fTime(0.f),
  fTimeR(0.f),
  fNSamplesHG(0),
  fNSamplesLG(0),
  fSamplesHG(0),
  fSamplesLG(0)
{  
  // ctor with all data 

  fAmp         = 0 ;
  fEnergy      = energy ;
  fNPE[0]      = NPE[0];
  fNPE[1]      = NPE[1];
  fTime        = time ;
  fTimeR       = fTime ;
  fId          = id ;
  fIndexInList = index ; 
  if( primary != -1){
    fNprimary    = 1 ; 
    fPrimary = new Int_t[fNprimary] ;
    fPrimary[0]  = primary ;
  }
  else{  //If the contribution of this primary smaller than fDigitThreshold (IlcRSTACKv1)
    fNprimary = 0 ; 
    fPrimary  = 0 ;
  }
}

//____________________________________________________________________________
IlcRSTACKDigit::IlcRSTACKDigit(const IlcRSTACKDigit & digit) : 
  IlcDigitNew(digit),
  fNprimary(digit.fNprimary),
  fPrimary(0),
  fEnergy(digit.fEnergy),
  fTime(digit.fTime),
  fTimeR(digit.fTimeR),
  fNSamplesHG(0),
  fNSamplesLG(0),
  fSamplesHG(0),
  fSamplesLG(0)
{
  // copy ctor
  fNPE[0]      = digit.fNPE[0];
  fNPE[1]      = digit.fNPE[1];
  if(fNprimary){
    fPrimary = new Int_t[fNprimary] ;
    for (Int_t i = 0; i < fNprimary ; i++)
       fPrimary[i]  = digit.fPrimary[i] ;
  }
  else
    fPrimary = 0 ;
  fAmp         = digit.fAmp ;
  fId          = digit.fId;
  fIndexInList = digit.fIndexInList ; 
}

//____________________________________________________________________________
IlcRSTACKDigit & IlcRSTACKDigit::operator = (const IlcRSTACKDigit &)
{
  Fatal("operator =", "not implemented");
  return *this;
}
//____________________________________________________________________________
IlcRSTACKDigit::~IlcRSTACKDigit() 
{
  // Delete array of primaries if any
  if (fPrimary)   delete [] fPrimary  ;
  // Delete arrays of ALTRO samples if any
  if (fSamplesHG) delete [] fSamplesHG;
  if (fSamplesLG) delete [] fSamplesLG;
}

//____________________________________________________________________________
Int_t IlcRSTACKDigit::Compare(const TObject * obj) const
{
  // Compares two digits with respect to its Id
  // to sort according increasing Id

  Int_t rv ;

  IlcRSTACKDigit * digit = (IlcRSTACKDigit *)obj ; 

  Int_t iddiff = fId - digit->GetId() ; 

  if ( iddiff > 0 ) 
    rv = 1 ;
  else if ( iddiff < 0 )
    rv = -1 ; 
  else
    rv = 0 ;
  
  return rv ; 

}

//____________________________________________________________________________
Int_t IlcRSTACKDigit::GetPrimary(Int_t index) const
{
  // retrieves the primary particle number given its index in the list 
  Int_t rv = -1 ;
  if ( index <= fNprimary && index > 0){
    rv = fPrimary[index-1] ;
  } 

  return rv ; 
  
}

//____________________________________________________________________________
void IlcRSTACKDigit::SetALTROSamplesHG(Int_t nSamplesHG, Int_t *samplesHG)
{
  fNSamplesHG = nSamplesHG;
  if (fSamplesHG) delete [] fSamplesHG;
  fSamplesHG = new UShort_t[fNSamplesHG];
  UShort_t i;
  for (i=0; i<fNSamplesHG; i++) {
    fSamplesHG[i] = samplesHG[i];
  }
}
//____________________________________________________________________________
void IlcRSTACKDigit::SetALTROSamplesLG(Int_t nSamplesLG, Int_t *samplesLG)
{
  fNSamplesLG = nSamplesLG;
  if (fSamplesLG) delete [] fSamplesLG;
  fSamplesLG = new UShort_t[fNSamplesLG];
  UShort_t i;
  for (i=0; i<fNSamplesLG; i++) {
    fSamplesLG[i] = samplesLG[i];
  }
}
//____________________________________________________________________________
void IlcRSTACKDigit::Print(const Option_t *) const
{
  // Print the digit together with list of primaries
  TString line = Form("RSTACK digit: E=%.3f, Id=%d, Time=%.3e, TimeR=%.3e, NPrim=%d, nHG=%d, nLG=%d \n", fEnergy,fId,fTime,fTimeR,fNprimary,fNSamplesHG,fNSamplesLG);
  line += "\tList of primaries: ";
  for (Int_t index = 0; index <fNprimary; index ++ )
    line += Form(" %d ",fPrimary[index]); 
  line += "\n";
  line += "\tSamples HG: ";
  for (Int_t i = 0; i <fNSamplesHG; i++)
    line += Form(" %d ",fSamplesHG[i]); 
  line += "\n";
  line += "\tSamples LG: ";
  for (Int_t i = 0; i <fNSamplesLG; i++)
    line += Form(" %d ",fSamplesLG[i]); 
  line += "\n";
  IlcInfo(line);
}
//____________________________________________________________________________
void IlcRSTACKDigit::ShiftPrimary(Int_t shift)
{
  //shifts primary number to BIG offset, to separate primary in different TreeK
  for(Int_t index = 0; index <fNprimary; index ++ ){
    fPrimary[index]+= shift ;
  } 
}
//____________________________________________________________________________
Bool_t IlcRSTACKDigit::operator==(IlcRSTACKDigit const & digit) const 
{
  // Two digits are equal if they have the same Id
  
  if ( fId == digit.fId ) 
    return kTRUE ;
  else 
    return kFALSE ;
}
 
//____________________________________________________________________________
IlcRSTACKDigit& IlcRSTACKDigit::operator+=(IlcRSTACKDigit const & digit) 
{

  // Adds the amplitude of digits and completes the list of primary particles
  if(digit.fNprimary>0){
     Int_t *tmp = new Int_t[fNprimary+digit.fNprimary] ;
     if(fAmp < digit.fAmp || fEnergy < digit.fEnergy){//most energetic primary in second digit => first primaries in list from second digit
        for (Int_t index = 0 ; index < digit.fNprimary ; index++)
           tmp[index]=(digit.fPrimary)[index] ;
        for (Int_t index = 0 ; index < fNprimary ; index++)
           tmp[index+digit.fNprimary]=fPrimary[index] ;
     }
     else{ //add new primaries to the end
        for (Int_t index = 0 ; index < fNprimary ; index++)
           tmp[index]=fPrimary[index] ;
        for (Int_t index = 0 ; index < digit.fNprimary ; index++)
           tmp[index+fNprimary]=(digit.fPrimary)[index] ;
     }
     if(fPrimary)
       delete []fPrimary ;
     fPrimary = tmp ;
   }
   fNprimary+=digit.fNprimary ;
   fAmp     += digit.fAmp ;
   fEnergy  += digit.fEnergy ;
   fNPE[0]  += digit.fNPE[0];
   fNPE[1]  += digit.fNPE[1];
   if(fTime > digit.fTime)
      fTime = digit.fTime ;
   fTimeR = fTime ; 

   // Add high-gain ALTRO samples
   UShort_t i;
   if (digit.fNSamplesHG > fNSamplesHG) {
     UShort_t newNSamplesHG = digit.fNSamplesHG;
     UShort_t *newSamplesHG = new UShort_t[newNSamplesHG];
     for (i=0; i<newNSamplesHG; i++) {
       if (i<fNSamplesHG)
	 newSamplesHG[i] = TMath::Max(1023,fSamplesHG[i] + (digit.fSamplesHG)[i]);
       else
	 newSamplesHG[i] = (digit.fSamplesHG)[i];
     }
     delete [] fSamplesHG;
     fSamplesHG = new UShort_t[newNSamplesHG];
     for (i=0; i<newNSamplesHG; i++) {
       fSamplesHG[i] = newSamplesHG[i];
     }
     delete [] newSamplesHG;
   }
   else {
     for (i=0; i<fNSamplesHG; i++)
       fSamplesHG[i] = TMath::Max(1023,fSamplesHG[i] + (digit.fSamplesHG)[i]);
   }

   // Add low-gain ALTRO samples
   if (digit.fNSamplesLG > fNSamplesLG) {
     UShort_t newNSamplesLG = digit.fNSamplesLG;
     UShort_t *newSamplesLG = new UShort_t[newNSamplesLG];
     for (i=0; i<newNSamplesLG; i++) {
       if (i<fNSamplesLG)
	 newSamplesLG[i] = TMath::Max(1023,fSamplesLG[i] + (digit.fSamplesLG)[i]);
       else
	 newSamplesLG[i] = (digit.fSamplesLG)[i];
     }
     delete [] fSamplesLG;
     fSamplesLG = new UShort_t[newNSamplesLG];
     for (i=0; i<newNSamplesLG; i++) {
       fSamplesLG[i] = newSamplesLG[i];
     }
     delete [] newSamplesLG;
   }
   else {
     for (i=0; i<fNSamplesLG; i++)
       fSamplesLG[i] = TMath::Max(1023,fSamplesLG[i] + (digit.fSamplesLG)[i]);
   }

   return *this ;
}
//____________________________________________________________________________
IlcRSTACKDigit& IlcRSTACKDigit::operator *= (Float_t factor) 
{
  // Multiplies the amplitude by a factor
  
  Float_t tempo = static_cast<Float_t>(fAmp) ; 
  tempo *= factor ; 
  fAmp = static_cast<Int_t>(TMath::Ceil(tempo)) ; 
  return *this ;
}

//____________________________________________________________________________
ostream& operator << ( ostream& out , const IlcRSTACKDigit & digit)
{
  // Prints the data of the digit
  
//   out << "ID " << digit.fId << " Energy = " << digit.fAmp << " Time = " << digit.fTime << endl ; 
//   Int_t i ;
//   for(i=0;i<digit.fNprimary;i++)
//     out << "Primary " << i+1 << " = " << digit.fPrimary[i] << endl ;
//   out << "Position in list = " << digit.fIndexInList << endl ; 
  digit.Warning("operator <<", "Implement differently") ; 
  return out ;
}


