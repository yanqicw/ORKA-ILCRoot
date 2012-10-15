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

/* $Id: IlcGenReaderTreeK.cxx 48102 2011-03-05 20:43:27Z morsch $ */
//
// Realisation of IlcGenReader to be used with IlcGenExtFile
// It reads events from a kinematics TreeK.
// NextEvent() is used to loop over events 
// and NextParticle() to loop over particles.  
// Author: andreas.morsch@cern.ch
//
#include <TFile.h>
#include <TTree.h>
#include <TParticle.h>
#include <TObjString.h>
#include <TObjArray.h>

#include "IlcGenReaderTreeK.h"
#include "IlcHeader.h"
#include "IlcRun.h"
#include "IlcStack.h"
#include "IlcRunLoader.h"

ClassImp(IlcGenReaderTreeK)

const TString IlcGenReaderTreeK::fgkEventFolderName("GenReaderTreeK");

IlcGenReaderTreeK::IlcGenReaderTreeK():
    IlcGenReader(),
    fNcurrent(0),
    fNparticle(0),
    fNp(0),
    fInRunLoader(0),
    fBaseFile(0),
    fStack(0),
    fOnlyPrimaries(kFALSE),
    fDirs(0x0),
    fCurrentDir(0)
{
//  Default constructor
}

IlcGenReaderTreeK::IlcGenReaderTreeK(const IlcGenReaderTreeK &reader):
    IlcGenReader(reader),
    fNcurrent(0),
    fNparticle(0),
    fNp(0),
    fInRunLoader(0),
    fBaseFile(0),
    fStack(0),
    fOnlyPrimaries(kFALSE),
    fDirs(0x0),
    fCurrentDir(0)
{
    reader.Copy(*this);
}


IlcGenReaderTreeK::~IlcGenReaderTreeK() 
{
// Destructor
    delete fInRunLoader;//it cleans all the loaded data
    delete fDirs;
}

void IlcGenReaderTreeK::Init() 
{
// Initialization
// Connect base file and file to read from

    TTree *ilc = IlcRunLoader::Instance()->TreeE();
    if (ilc) {
      fBaseFile = ilc->GetCurrentFile();
    } else {
      printf("\n Warning: Basefile cannot be found !\n");
    }
    //if (!fFile) fFile  = new TFile(fFileName);
    if (fInRunLoader == 0x0) 
     {
       fInRunLoader = IlcRunLoader::Open((GetDirName(fCurrentDir++)+"/")+fFileName,fgkEventFolderName);
       fInRunLoader->LoadHeader();
       fInRunLoader->LoadKinematics("READ");
     }
}

Int_t IlcGenReaderTreeK::NextEvent() 
{
//  Read the next event  
//  cd to file with old kine tree    
    if (!fBaseFile) Init();
//  Get next event
    
    if (fNcurrent >= fInRunLoader->GetNumberOfEvents())
     {
      if (fCurrentDir >= fDirs->GetEntries())
       {
         Warning("NextEvent","No more events");
         return 0;
       }
      delete fInRunLoader;
      fInRunLoader = IlcRunLoader::Open((GetDirName(fCurrentDir++)+"/")+fFileName,fgkEventFolderName);
      fInRunLoader->LoadHeader();
      fInRunLoader->LoadKinematics("READ");
      fNcurrent = 0;
     }
    fInRunLoader->GetEvent(fNcurrent);
    fStack = fInRunLoader->Stack();
    
//  cd back to base file
    fBaseFile->cd();
//
    fNcurrent++;
    fNparticle = 0;
    fNp =  fStack->GetNtrack();
    printf("\n Next event contains %d particles", fNp);
//    
    return  fNp;
}

TParticle* IlcGenReaderTreeK::NextParticle() 
{
//Return next particle
  TParticle* part = GetParticle(fNparticle++);
  if (part == 0x0) return 0x0;
  //if only primaries are to be read, and this particle is not primary enter loop  
  if (fOnlyPrimaries && ( part->GetFirstMother() > -1) ) 
   for (;;)
    { //look for a primary
      part = GetParticle(fNparticle++);
      if (part == 0x0) return 0x0;
      if (part->GetFirstMother() == -1) return part;
    }

  return part;
}

void IlcGenReaderTreeK::RewindEvent()
{
  // Go back to the first particle of the event
  fNparticle = 0;
}


IlcGenReaderTreeK& IlcGenReaderTreeK::operator=(const  IlcGenReaderTreeK& rhs)
{
// Assignment operator
    rhs.Copy(*this);
    return *this;
}

void IlcGenReaderTreeK::Copy(TObject&) const
{
    //
    // Copy 
    //
    Fatal("Copy","Not implemented!\n");
}



TString& IlcGenReaderTreeK::GetDirName(Int_t entry)
 {
// Get the current directory name

   TString* retval;//return value
   if (fDirs ==  0x0)
    {
      retval = new TString(".");
      return *retval;
    }
   
   if ( (entry>fDirs->GetEntries()) || (entry<0))//if out of bounds return empty string
    {                                            //note that entry==0 is accepted even if array is empty (size=0)
      Error("GetDirName","Name out of bounds");
      retval = new TString();
      return *retval;
    }
   
   if (fDirs->GetEntries() == 0)
    { 
      retval = new TString(".");
      return *retval;
    }
   
   TObjString *dir = dynamic_cast<TObjString*>(fDirs->At(entry));
   if(dir == 0x0)
    {
      Error("GetDirName","Object in TObjArray is not a TObjString or its descendant");
      retval = new TString();
      return *retval;
    }
   if (gDebug > 0) Info("GetDirName","Returned ok %s",dir->String().Data());
   return dir->String();
 }
 
void IlcGenReaderTreeK::AddDir(const char* dirname)
{
  //adds a directory to the list of directories where data are looked for
  if(fDirs == 0x0) 
   {
     fDirs = new TObjArray();
     fDirs->SetOwner(kTRUE);
   }
  TObjString *odir= new TObjString(dirname);
  fDirs->Add(odir);
}

TParticle* IlcGenReaderTreeK::GetParticle(Int_t i)
 {
  if (fStack && i<fNp) return fStack->Particle(i);
  return 0x0;
 }
