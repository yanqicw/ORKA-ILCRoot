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

/* $Id: IlcAODCaloCells.cxx 56416 2012-05-11 08:43:19Z hristov $ */

//-------------------------------------------------------------------------
//     AOD class to store calorimeter cell data
//     Author: Markus Oldenburg, CERN
//-------------------------------------------------------------------------

#include "IlcAODCaloCells.h"

ClassImp(IlcAODCaloCells)

//_______________________________________________________
IlcAODCaloCells::IlcAODCaloCells() : 
    IlcVCaloCells(), fNCells(0), fCellNumber(0), 
    fAmplitude(0), fTime(0), fEFraction(0), fMCLabel(0),
    fIsSorted(kTRUE), fType(kUndef)
{
  // default constructor
}

//_____________________________________________________________________________________
IlcAODCaloCells::IlcAODCaloCells(const char* name, const char* title, VCells_t ttype) :
    IlcVCaloCells(name, title), fNCells(0), fCellNumber(0), 
    fAmplitude(0), fTime(0), fEFraction(0), fMCLabel(0),
    fIsSorted(kTRUE), fType(ttype)
{
  //constructor
}

//________________________________________________________________
IlcAODCaloCells::IlcAODCaloCells(const IlcAODCaloCells& cells) :
    IlcVCaloCells(cells), fNCells(cells.fNCells), fCellNumber(0),
    fAmplitude(0), fTime(0), fEFraction(0), fMCLabel(0),
    fIsSorted(cells.fIsSorted), fType(cells.fType)
{
// Copy constructor
  fCellNumber = new Short_t[fNCells];
  fAmplitude  = new Double32_t[fNCells]; 
  fTime       = new Double32_t[fNCells]; 
  fMCLabel    = new Short_t[fNCells]; 
  fEFraction  = new Double32_t[fNCells]; 

  for (Int_t i = 0; i < fNCells; i++) {
    fCellNumber[i]    = cells.fCellNumber[i];
    fAmplitude[i]     = cells.fAmplitude[i];
    if(cells.fTime)  fTime[i]      = cells.fTime[i];
    if(cells.fMCLabel)  fMCLabel[i]   = cells.fMCLabel[i];
    if(cells.fEFraction)fEFraction[i] = cells.fEFraction[i];    
  }
}

//________________________________________________________________________
IlcAODCaloCells& IlcAODCaloCells::operator=(const IlcAODCaloCells& source)
{
  // Assignment operator
  if(this != &source) 
  {
    IlcVCaloCells::operator=(source);
    
    if(fNCells != source.fNCells) 
    {
      delete [] fCellNumber;
      delete [] fAmplitude;
      delete [] fTime;
      delete [] fMCLabel;
      delete [] fEFraction;

      fNCells     = source.fNCells;
      
      fCellNumber = new Short_t[fNCells];
      fAmplitude  = new Double32_t[fNCells];
      fTime       = new Double32_t[fNCells];
      fMCLabel    = new Short_t[fNCells];
      fEFraction  = new Double32_t[fNCells];
    }
    
    memcpy(fCellNumber,source.fCellNumber, fNCells*sizeof(Short_t));
    memcpy(fAmplitude, source.fAmplitude,  fNCells*sizeof(Double32_t));
    if(source.fTime      && fTime)      memcpy(fTime,      source.fTime,      fNCells*sizeof(Double32_t));
    if(source.fMCLabel   && fMCLabel)   memcpy(fMCLabel,   source.fMCLabel,   fNCells*sizeof(Short_t));
    if(source.fEFraction && fEFraction) memcpy(fEFraction, source.fEFraction, fNCells*sizeof(Double32_t));

    fIsSorted = source.fIsSorted;
    fType     = source.fType;
  }

  return *this;
  
}

//_________________________________
IlcAODCaloCells::~IlcAODCaloCells()
{
  // destructor

  DeleteContainer();
}

void IlcAODCaloCells::Clear(const Option_t*)
{
  // clear
  
  DeleteContainer();
}

void IlcAODCaloCells::Copy(TObject &obj) const 
{
  
  // this overwrites the virtual TOBject::Copy()
  // to allow run time copying without casting
  // in IlcESDEvent
  
  if(this==&obj)return;
  IlcAODCaloCells *robj = dynamic_cast<IlcAODCaloCells*>(&obj);
  if(!robj)return; // not an IlcAODCaloCells
  *robj = *this;
  
}

//______________________________________________________________________
IlcVCaloCells *IlcAODCaloCells::CopyCaloCells(Bool_t all = kTRUE) const 
{
  
  // copy the calo cells into a new object. If option all=FALSE, just the object type, 
  // for mixing
  
  IlcVCaloCells *obj =  new IlcAODCaloCells();
  
  if(all){
    obj->SetName (GetName()) ; 
    obj->SetTitle(GetTitle()) ; 
    obj->SetType (GetType()) ; 
  
    obj->SetNumberOfCells(fNCells);
    for (Short_t i = 0; i < fNCells; i++) 
    {
      Int_t mclabel = -1;
      if(fMCLabel) mclabel = fMCLabel[i];
      
      Float_t efrac = 0.;
      if(fEFraction) efrac = fEFraction[i];
      
      Float_t time = -1;
      if(fTime) time = fTime[i];
      
      obj->SetCell(i,fCellNumber[i],fAmplitude[i],time,mclabel,efrac);
    }
  }
  
  return obj;
  
}

//___________________________________________________
void IlcAODCaloCells::CreateContainer(Short_t nCells)
{
  // function that creates container to store calorimeter cell data

  DeleteContainer();
  
  if (nCells <= 0) 
  {
    fNCells = 0;
    return;
  }

  fNCells = nCells;

  fCellNumber = new Short_t[fNCells];
  fAmplitude  = new Double32_t[fNCells];
  fTime       = new Double32_t[fNCells];
  fMCLabel    = new Short_t[fNCells];
  fEFraction  = new Double32_t[fNCells];

  // set to zero
  for(int i = 0;i<fNCells;++i)
  {
    fAmplitude [i] =  0.; 
    fCellNumber[i] = -1 ; 
    fEFraction [i] =  0.;
    fTime      [i] = -1.;
    fMCLabel   [i] = -1 ;
  }
}

//_____________________________________
void IlcAODCaloCells::DeleteContainer()
{
  // deletes allocated memory

  if (fCellNumber)
  {
    delete[] fCellNumber;
    fCellNumber = NULL;
  }

  if (fAmplitude)
  {
    delete[] fAmplitude;
    fAmplitude = NULL;
  }
  
  if (fTime)
  {
    delete[] fTime;
    fTime = NULL;
  }
  
  if (fMCLabel)
  {
    delete[] fMCLabel;
    fMCLabel = NULL;
  }
  
  if (fEFraction)
  {
    delete[] fEFraction;
    fEFraction = NULL;
  }
  
  
  fNCells = 0;
  fIsSorted = kFALSE;
}

//__________________________
void IlcAODCaloCells::Sort() 
{
  // sort the cell array by cell number
  
  Int_t *idxArray = new Int_t[fNCells];
  TMath::Sort(fNCells,fCellNumber,idxArray,kFALSE);
  
  Short_t    *newIndex     = new Short_t[fNCells];
  Double32_t *newAmplitude = new Double32_t[fNCells];
  
  Double32_t *newTime      = 0; 
  Short_t    *newMCLabel   = 0 ;
  Double32_t *newEFraction = 0 ; 
  if(fTime)      newTime      = new Double32_t[fNCells];
  if(fMCLabel)   newMCLabel   = new Short_t[fNCells];
  if(fEFraction) newEFraction = new Double32_t[fNCells];
  
  for (Int_t i=0; i < fNCells; i++) 
  {
    newIndex[i]     = fCellNumber[idxArray[i]];
    newAmplitude[i] = fAmplitude [idxArray[i]];
    if(fTime)      newTime[i]      = fTime     [idxArray[i]];
    if(fMCLabel)   newMCLabel[i]   = fMCLabel  [idxArray[i]];
    if(fEFraction) newEFraction[i] = fEFraction[idxArray[i]];  
  }
  
  delete [] fCellNumber;
  delete [] fAmplitude;
  delete [] fTime;
  delete [] fMCLabel;
  delete [] fEFraction;

  fCellNumber = newIndex;
  fAmplitude  = newAmplitude;
  if(fTime)      fTime       = newTime;
  if(fMCLabel)   fMCLabel    = newMCLabel;
  if(fEFraction) fEFraction  = newEFraction;

  delete [] idxArray;
  
  fIsSorted = kTRUE;
} 

//________________________________________________________________________________________
Bool_t IlcAODCaloCells::SetCell(Short_t pos,     Short_t cellNumber, Double32_t amplitude, 
                                Double32_t time, Short_t mclabel,    Double32_t efrac)
{
  // Sets a cell at the given position

  if (pos>=0 && pos < fNCells) 
  {
    fCellNumber[pos] = cellNumber;
    fAmplitude[pos]  = amplitude;
    
    // note: initialize (can't use memset for non-0 values)
    //       plus sizeof(Double32_t) is 0
    if(!fTime){
      fTime  = new Double32_t[fNCells];
      
      for( Int_t i = 0; i < fNCells; i++ )
        fTime[i] = -1;
    }
    if(!fMCLabel){
      fMCLabel = new Short_t[fNCells];
      
      for( Int_t i = 0; i < fNCells; i++ )
        fMCLabel[i] = -1;
    }
    if(!fEFraction){
      fEFraction = new Double32_t[fNCells];
      
      for( Int_t i = 0; i < fNCells; i++ )
        fEFraction[i] = 0;
    }
    
    fTime[pos]       = time;
    fMCLabel[pos]    = mclabel;
    fEFraction[pos]  = efrac;
    
    fIsSorted        = kFALSE;
    
    return kTRUE;
    
  } else {
    return kFALSE;
  }
}




