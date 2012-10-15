/////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                         //
//  class IlcTreeLoader                                                                    //
//                                                                                         //
//  Container of all data needed for full                                                  //
//  description of each data type                                                          //
//  (Hits, Kine, ...)                                                                      //
//                                                                                         //
//  Each data loader has a basic standard setup of BaseLoaders                             //
//  which can be identuified by indexes (defined by EStdBasicLoaders)                      //
//  Data managed by these standard base loaders has fixed naming convention                //
//  e.g. - tree with hits is always named TreeH                                            //
//                     (defined in IlcLoader::fgkDefaultHitsContainerName)                 //
//       - task DtectorName+Name defined                                                   //
//                                                                                         //
//  EStdBasicLoaders   idx     Object Type        Description                              //
//      kData           0    TTree or TObject     main data itself (hits,digits,...)       //
//      kTask           1        TTask            object producing main data               //
//      kQA             2        TTree                quality assurance tree               //
//      kQATask         3        TTask            task producing QA object                 //
//                                                                                         //
//                                                                                         //
//  User can define and add more basic loaders even Run Time.                              //
//  Caution: in order to save information about added base loader                          //
//  user must rewrite Run Loader to gilc.file, overwriting old setup                     //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

/* $Id: IlcTreeLoader.cxx 50615 2011-07-16 23:19:19Z hristov $ */

#include "IlcTreeLoader.h"
#include "IlcRunLoader.h"
#include "IlcLog.h"

#include <TFile.h>
#include <TFolder.h>

ClassImp(IlcTreeLoader)

//______________________________________________________________________________
IlcTreeLoader::IlcTreeLoader(const TString& name, IlcDataLoader* dl,Bool_t storeontop):
 IlcObjectLoader(name,dl,storeontop)
{
  //
  // Constructor
  //
}

//______________________________________________________________________________
Int_t IlcTreeLoader::WriteData(Option_t* opt)
{
  //
  // Writes data defined by di object
  // opt might be "OVERWRITE" in case of forcing overwriting
  //
  IlcDebug(1, Form("Writing %s container for %s data. Option is %s.",
		   GetName(),GetDataLoader()->GetName(),opt));
  
  TObject *data = Get();
  if(data == 0x0)
    {//did not get, nothing to write
      IlcWarning(Form("Tree named %s not found in folder. Nothing to write.",GetName()));
      return 0;
    }
  
  //check if file is opened
  if (GetDirectory() == 0x0)
    { 
      //if not try to open
      GetDataLoader()->SetFileOption("UPDATE");
      if (GetDataLoader()->OpenFile("UPDATE"))
	{  
	  //oops, can not open the file, give an error message and return error code
	  IlcError(Form("Can not open hits file. %s ARE NOT WRITTEN",GetName()));
	  return 1;
	}
    }
  
  if (GetDataLoader()->IsFileWritable() == kFALSE)
    {
      IlcError(Form("File %s is not writable",GetDataLoader()->GetFileName().Data()));
      return 2;
    }
  
  GetDirectory()->cd(); //set the proper directory active
  
  //see if hits container already exists in this (root) directory
  TObject* obj = GetFromDirectory(GetName());
  if (obj)
    { //if they exist, see if option OVERWRITE is used
      const char *oOverWrite = strstr(opt,"OVERWRITE");
      if(!oOverWrite)
	{//if it is not used -  give an error message and return an error code
	  IlcError("Tree already exisists. Use option \"OVERWRITE\" to overwrite previous data");
	  return 3;
	}
    }
  
  IlcDebug(1, Form("DataName = %s, opt = %s, data object name = %s",
                   GetName(),opt,data->GetName()));
  IlcDebug(1, Form("File Name = %s, Directory Name = %s Directory's File Name = %s",
                   GetDataLoader()->GetFile()->GetName(),GetDirectory()->GetName(),
                   GetDirectory()->GetFile()->GetName()));
  
  //if a data object is a tree set the directory
  TTree* tree = dynamic_cast<TTree*>(data);
  if (tree) tree->SetDirectory(GetDirectory()); //forces setting the directory to this directory (we changed dir few lines above)
  
  IlcDebug(1, "Writing tree");
  data->Write(0,TObject::kOverwrite);
  
  fIsLoaded = kTRUE;  // Just to ensure flag is on. Object can be posted manually to folder structure, not using loader.
  
  return 0;
  
}

//______________________________________________________________________________
void IlcTreeLoader::MakeTree()
{
  //
  // This virtual method creates the tree in the file
  //
  if (Tree()) 
    {
      IlcDebug(1, Form("name = %s, Data Name = %s Tree already exists.",
		       GetName(),GetDataLoader()->GetName()));
      return;//tree already made 
    }
  IlcDebug(1, Form("Making Tree named %s.",GetName()));
  
  TString dtypename(GetDataLoader()->GetName());
  TTree* tree = new TTree(GetName(), dtypename + " Container"); //make a tree
  if (tree == 0x0)
    {
      IlcError(Form("Can not create %s tree.",GetName()));
      return;
   }
  tree->SetAutoSave(1000000000); //no autosave
  GetFolder()->Add(tree);
  WriteData("OVERWRITE");//write tree to the file
}


