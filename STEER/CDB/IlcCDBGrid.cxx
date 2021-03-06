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

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                             //
// IlcCDBGrid										       //
// access class to a DataBase in an AliEn storage  			                       //
//                                                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#define sleep(x) _sleep(x)
//#include <WinBase.h>
#endif
#include <cstdlib>
#include <TGrid.h>
#include <TGridResult.h>
#include <TFile.h>
#include <TKey.h>
#include <TROOT.h>
#include <TList.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TMath.h>
#include <TRegexp.h>

#include "IlcLog.h"
#include "IlcCDBEntry.h"
#include "IlcCDBGrid.h"
#include "IlcCDBManager.h"


ClassImp(IlcCDBGrid)

//_____________________________________________________________________________
IlcCDBGrid::IlcCDBGrid(const char *gridUrl, const char *user, const char *dbFolder,
                       const char *se, const char* cacheFolder, Bool_t operateDisconnected,
		       Long64_t cacheSize, Long_t cleanupInterval) :
IlcCDBStorage(),
fGridUrl(gridUrl),
fUser(user),
fDBFolder(dbFolder),
fSE(se),
fMirrorSEs(""),
fCacheFolder(cacheFolder),
fOperateDisconnected(operateDisconnected),
fCacheSize(cacheSize),
fCleanupInterval(cleanupInterval)
{
// constructor //

	// if the same Grid is alreay active, skip connection
	if (!gGrid || fGridUrl != gGrid->GridUrl()
	     || (( fUser != "" ) && ( fUser != gGrid->GetUser() )) ) {
   		// connection to the Grid
		IlcInfo("Connection to the Grid...");
		if(gGrid){
			IlcInfo(Form("gGrid = %p; fGridUrl = %s; gGrid->GridUrl() = %s",gGrid,fGridUrl.Data(), gGrid->GridUrl()));
			IlcInfo(Form("fUser = %s; gGrid->GetUser() = %s",fUser.Data(), gGrid->GetUser()));
		}
		TGrid::Connect(fGridUrl.Data(),fUser.Data());
	}

	if(!gGrid) {
		IlcError("Connection failed!");
		return;
	}

	TString initDir(gGrid->Pwd(0));
	if (fDBFolder[0] != '/') {
		fDBFolder.Prepend(initDir);
	}

	// check DBFolder: trying to cd to DBFolder; if it does not exist, create it
	if(!gGrid->Cd(fDBFolder.Data(),0)){
		IlcDebug(2,Form("Creating new folder <%s> ...",fDBFolder.Data()));
		TGridResult* res = gGrid->Command(Form("mkdir -p %s",fDBFolder.Data()));
		TString result = res->GetKey(0,"__result__");
		if(result == "0"){
			IlcFatal(Form("Cannot create folder <%s> !",fDBFolder.Data()));
			return;
		}
	} else {
		IlcDebug(2,Form("Folder <%s> found",fDBFolder.Data()));
	}

	// removes any '/' at the end of path, then append one '/'
	while(fDBFolder.EndsWith("/")) fDBFolder.Remove(fDBFolder.Last('/')); 
	fDBFolder+="/";

	fType="alien";
	fBaseFolder = fDBFolder;

	// Setting the cache

	// Check if local cache folder is already defined
	TString origCache(TFile::GetCacheFileDir());
	if(fCacheFolder.Length() > 0) {
		if(origCache.Length() == 0) {
			IlcInfo(Form("Setting local cache to: %s", fCacheFolder.Data()));
		} else if(fCacheFolder != origCache) {
			IlcWarning(Form("Local cache folder was already defined, changing it to: %s",
					fCacheFolder.Data()));
		}

		// default settings are: operateDisconnected=kTRUE, forceCacheread = kFALSE
		if(!TFile::SetCacheFileDir(fCacheFolder.Data(), fOperateDisconnected)) {
			IlcError(Form("Could not set cache folder %s !", fCacheFolder.Data()));
			fCacheFolder = "";
		} else {
			// reset fCacheFolder because the function may have
			// slightly changed the folder name (e.g. '/' added)
			fCacheFolder = TFile::GetCacheFileDir();
		}

		// default settings are: cacheSize=1GB, cleanupInterval = 0
		if(!TFile::ShrinkCacheFileDir(fCacheSize, fCleanupInterval)) {
			IlcError(Form("Could not set following values "
				"to ShrinkCacheFileDir: cacheSize = %lld, cleanupInterval = %ld !",
				fCacheSize, fCleanupInterval));
		}
	}

	// return to the initial directory
	gGrid->Cd(initDir.Data(),0);

	fNretry = 3;  // default
	fInitRetrySeconds = 5;   // default
}

//_____________________________________________________________________________
IlcCDBGrid::~IlcCDBGrid()
{
// destructor
	delete gGrid; gGrid=0;

}

//_____________________________________________________________________________
Bool_t IlcCDBGrid::FilenameToId(TString& filename, IlcCDBId& id) {
// build IlcCDBId from full path filename (fDBFolder/path/Run#x_#y_v#z_s0.root)

	if(filename.Contains(fDBFolder)){
		filename = filename(fDBFolder.Length(),filename.Length()-fDBFolder.Length());
	}

	TString idPath = filename(0,filename.Last('/'));
	id.SetPath(idPath);
	if(!id.IsValid()) return kFALSE;

	filename=filename(idPath.Length()+1,filename.Length()-idPath.Length());

        Ssiz_t mSize;
	// valid filename: Run#firstRun_#lastRun_v#version_s0.root
        TRegexp keyPattern("^Run[0-9]+_[0-9]+_v[0-9]+_s0.root$");
        keyPattern.Index(filename, &mSize);
        if (!mSize) {

		// TODO backward compatibility ... maybe remove later!
        	Ssiz_t oldmSize;
        	TRegexp oldKeyPattern("^Run[0-9]+_[0-9]+_v[0-9]+.root$");
        	oldKeyPattern.Index(filename, &oldmSize);
		if(!oldmSize) {
			IlcDebug(2,Form("Bad filename <%s>.", filename.Data()));
                	return kFALSE;
		} else {
			IlcDebug(2,Form("Old filename format <%s>.", filename.Data()));
			id.SetSubVersion(-11); // TODO trick to ensure backward compatibility
		}

        } else {
		id.SetSubVersion(-1); // TODO trick to ensure backward compatibility
	}

	filename.Resize(filename.Length() - sizeof(".root") + 1);

        TObjArray* strArray = (TObjArray*) filename.Tokenize("_");

	TString firstRunString(((TObjString*) strArray->At(0))->GetString());
	id.SetFirstRun(atoi(firstRunString.Data() + 3));
	id.SetLastRun(atoi(((TObjString*) strArray->At(1))->GetString()));

	TString verString(((TObjString*) strArray->At(2))->GetString());
	id.SetVersion(atoi(verString.Data() + 1));

        delete strArray;

        return kTRUE;
}

//_____________________________________________________________________________
Bool_t IlcCDBGrid::IdToFilename(const IlcCDBId& id, TString& filename) const {
// build file name from IlcCDBId (path, run range, version) and fDBFolder

	if (!id.GetIlcCDBRunRange().IsValid()) {
		IlcDebug(2,Form("Invalid run range <%d, %d>.",
			id.GetFirstRun(), id.GetLastRun()));
		return kFALSE;
	}

	if (id.GetVersion() < 0) {
		IlcDebug(2,Form("Invalid version <%d>.", id.GetVersion()));
                return kFALSE;
	}

	filename = Form("Run%d_%d_v%d",
				id.GetFirstRun(),
				id.GetLastRun(),
				id.GetVersion());

	if (id.GetSubVersion() != -11) filename += "_s0"; // TODO to ensure backward compatibility
	filename += ".root";

	filename.Prepend(fDBFolder + id.GetPath() + '/');

        return kTRUE;
}

//_____________________________________________________________________________
void IlcCDBGrid::SetRetry(Int_t nretry, Int_t initsec) {

	// Function to set the exponential retry for putting entries in the OCDB

	IlcWarning("WARNING!!! You are changing the exponential retry times and delay: this function should be used by experts!"); 
	fNretry = nretry;
	fInitRetrySeconds = initsec;
	IlcDebug(2,Form("fNretry = %d, fInitRetrySeconds = %d", fNretry, fInitRetrySeconds));
} 


//_____________________________________________________________________________
Bool_t IlcCDBGrid::PrepareId(IlcCDBId& id) {
// prepare id (version) of the object that will be stored (called by PutEntry)

	TString initDir(gGrid->Pwd(0));

	TString dirName(fDBFolder);

	Bool_t dirExist=kFALSE;



	// go to the path; if directory does not exist, create it
	for(int i=0;i<3;i++){
	        //TString cmd("find -d ");
		//cmd += Form("%s ",dirName);
		//cmd += 
		//gGrid->Command(cmd.Data());
 		dirName+=Form("%s/",id.GetPathLevel(i).Data());
		dirExist=gGrid->Cd(dirName,0);
		if (!dirExist) {
			IlcDebug(2,Form("Creating new folder <%s> ...",dirName.Data()));
			if(!gGrid->Mkdir(dirName,"",0)){
				IlcError(Form("Cannot create directory <%s> !",dirName.Data()));
				gGrid->Cd(initDir.Data());
			return kFALSE;
			}

			// if folders are new add tags to them
			if(i == 1) {
				// TODO Currently disabled
				// add short lived tag!
				// IlcInfo("Tagging level 1 folder with \"ShortLived\" tag");
				// if(!AddTag(dirName,"ShortLived_try")){
				//	IlcError(Form("Could not tag folder %s !", dirName.Data()));
				//	if(!gGrid->Rmdir(dirName.Data())){
				//		IlcError(Form("Unexpected: could not remove %s directory!", dirName.Data()));
				//	}
				//	return 0;
				//}

			} else if(i == 2) {
				IlcDebug(2,"Tagging level 2 folder with \"CDB\" and \"CDB_MD\" tag");
				if(!AddTag(dirName,"CDB")){
					IlcError(Form("Could not tag folder %s !", dirName.Data()));
					if(!gGrid->Rmdir(dirName.Data())){
						IlcError(Form("Unexpected: could not remove %s directory!", dirName.Data()));
					}
					return 0;
				}
				if(!AddTag(dirName,"CDB_MD")){
					IlcError(Form("Could not tag folder %s !", dirName.Data()));
					if(!gGrid->Rmdir(dirName.Data())){
						IlcError(Form("Unexpected: could not remove %s directory!", dirName.Data()));
					}
					return 0;
				}

				// TODO Currently disabled
				// add short lived tag!
				// TString path=id.GetPath();
				// if(IlcCDBManager::Instance()->IsShortLived(path.Data())) {
				//	IlcInfo(Form("Tagging %s as short lived", dirName.Data()));
				//	if(!TagShortLived(dirName, kTRUE)){
				//		IlcError(Form("Could not tag folder %s !", dirName.Data()));
				//		if(!gGrid->Rmdir(dirName.Data())){
				//			IlcError(Form("Unexpected: could not remove %s directory!", dirName.Data()));
				//		}
				//		return 0;
				//	}
				// } else {
				//	IlcInfo(Form("Tagging %s as long lived", dirName.Data()));
				//	if(!TagShortLived(dirName, kFALSE)){
				//		IlcError(Form("Could not tag folder %s !", dirName.Data()));
				//		if(!gGrid->Rmdir(dirName.Data())){
				//			IlcError(Form("Unexpected: could not remove %s directory!", dirName.Data()));
				//		}
				//		return 0;
				//	}
				// }
			}
		}
	}
	gGrid->Cd(initDir,0);

	TString filename;
	IlcCDBId anId; // the id got from filename
	IlcCDBRunRange lastRunRange(-1,-1); // highest runRange found
	Int_t lastVersion=0; // highest version found

	TGridResult *res = gGrid->Ls(dirName);

	//loop on the files in the directory, look for highest version
	for(int i=0; i < res->GetEntries(); i++){
		filename=res->GetFileNamePath(i);
		if (!FilenameToId(filename, anId)) continue;
		if (anId.GetIlcCDBRunRange().Overlaps(id.GetIlcCDBRunRange()) && anId.GetVersion() > lastVersion) {
			lastVersion = anId.GetVersion();
			lastRunRange = anId.GetIlcCDBRunRange();
		}

	}
	delete res;

	// GRP entries with explicitly set version escape default incremental versioning
	if(id.GetPath().Contains("GRP") && id.HasVersion() && lastVersion!=0)
	{
		IlcDebug(5,Form("Entry %s won't be put in the destination OCDB", id.ToString().Data()));
		return kFALSE;
	}

	id.SetVersion(lastVersion + 1);
	id.SetSubVersion(0);

	TString lastStorage = id.GetLastStorage();
	if(lastStorage.Contains(TString("new"), TString::kIgnoreCase) && id.GetVersion() > 1 ){
		IlcDebug(2, Form("A NEW object is being stored with version %d",
					id.GetVersion()));
		IlcDebug(2, Form("and it will hide previously stored object with version %d!",
					id.GetVersion()-1));
	}

	if(!lastRunRange.IsAnyRange() && !(lastRunRange.IsEqual(&id.GetIlcCDBRunRange())))
    		IlcWarning(Form("Run range modified w.r.t. previous version (Run%d_%d_v%d)",
    		     	lastRunRange.GetFirstRun(), lastRunRange.GetLastRun(), id.GetVersion()));

	return kTRUE;
}

//_____________________________________________________________________________
IlcCDBId* IlcCDBGrid::GetId(const TObjArray& validFileIds, const IlcCDBId& query) {
// look for the Id that matches query's requests (highest or exact version)

	if(validFileIds.GetEntriesFast() < 1)
		return NULL;

	TIter iter(&validFileIds);

	IlcCDBId *anIdPtr=0;
	IlcCDBId* result=0;

	while((anIdPtr = dynamic_cast<IlcCDBId*> (iter.Next()))){
		if(anIdPtr->GetPath() != query.GetPath()) continue;

		//if(!CheckVersion(query, anIdPtr, result)) return NULL;

		if (!query.HasVersion()){ // look for highest version
			if(result && result->GetVersion() > anIdPtr->GetVersion()) continue;
			if(result && result->GetVersion() == anIdPtr->GetVersion()) {
				IlcError(Form("More than one object valid for run %d, version %d!",
					query.GetFirstRun(), anIdPtr->GetVersion()));
				return NULL;
			}
			result = anIdPtr;
		} else { // look for specified version
			if(query.GetVersion() != anIdPtr->GetVersion()) continue;
			if(result && result->GetVersion() == anIdPtr->GetVersion()){
				IlcError(Form("More than one object valid for run %d, version %d!",
					query.GetFirstRun(), anIdPtr->GetVersion()));
				return NULL;
			}
			result = anIdPtr;
		}

	}
	
	if (!result) return NULL;

	return dynamic_cast<IlcCDBId*> (result->Clone());
}

//_____________________________________________________________________________
IlcCDBId* IlcCDBGrid::GetEntryId(const IlcCDBId& queryId) {
// get IlcCDBId from the database
// User must delete returned object

	IlcCDBId* dataId=0;

	IlcCDBId selectedId(queryId);
	if (!selectedId.HasVersion()) {
		// if version is not specified, first check the selection criteria list
		GetSelection(&selectedId);
	}

	TObjArray validFileIds;
	validFileIds.SetOwner(1);

	// look for file matching query requests (path, runRange, version)
	if(selectedId.GetFirstRun() == fRun && fPathFilter.Comprises(selectedId.GetIlcCDBPath()) &&
			fVersion == selectedId.GetVersion() && !fMetaDataFilter){
		// look into list of valid files previously loaded with IlcCDBStorage::FillValidFileIds()
		IlcDebug(2, Form("List of files valid for run %d was loaded. Looking there for fileids valid for path %s!",
					selectedId.GetFirstRun(), selectedId.GetPath().Data()));
		dataId = GetId(fValidFileIds, selectedId);

	} else {
		// List of files valid for reqested run was not loaded. Looking directly into CDB
		IlcDebug(2, Form("List of files valid for run %d and version %d was not loaded. Looking directly into CDB for fileids valid for path %s!",
					selectedId.GetFirstRun(), selectedId.GetVersion(), selectedId.GetPath().Data()));

		TString filter;
		MakeQueryFilter(selectedId.GetFirstRun(), selectedId.GetLastRun(), 0, filter);

		TString pattern = ".root";
		TString optionQuery = "-y -m";
		if(selectedId.GetVersion() >= 0) {
			pattern.Prepend(Form("_v%d_s0",selectedId.GetVersion()));
			optionQuery = "";
		}

		TString folderCopy(Form("%s%s/Run",fDBFolder.Data(),selectedId.GetPath().Data()));

		if (optionQuery.Contains("-y")){
			IlcInfo("Only latest version will be returned");
		}

		IlcDebug(2,Form("** fDBFolder = %s, pattern = %s, filter = %s",folderCopy.Data(), pattern.Data(), filter.Data()));
		TGridResult *res = gGrid->Query(folderCopy, pattern, filter, optionQuery.Data());
		if (res) {
			IlcCDBId validFileId;
			for(int i=0; i<res->GetEntries(); i++){
				TString filename = res->GetKey(i, "lfn");
				if(filename == "") continue;
				if(FilenameToId(filename, validFileId))
						validFileIds.AddLast(validFileId.Clone());
			}
			delete res;
		}	
		dataId = GetId(validFileIds, selectedId);
	}

	return dataId;
}

//_____________________________________________________________________________
IlcCDBEntry* IlcCDBGrid::GetEntry(const IlcCDBId& queryId) {
// get IlcCDBEntry from the database

	IlcCDBId* dataId = GetEntryId(queryId);

	if (!dataId) return NULL;

	TString filename;
	if (!IdToFilename(*dataId, filename)) {
		IlcDebug(2,Form("Bad data ID encountered! Subnormal error!"));
		delete dataId;
		return NULL;
	}

	IlcCDBEntry* anEntry = GetEntryFromFile(filename, dataId);

	delete dataId;
	return anEntry;
}

//_____________________________________________________________________________
IlcCDBEntry* IlcCDBGrid::GetEntryFromFile(TString& filename, IlcCDBId* dataId){
// Get IlcCBEntry object from file "filename"

	IlcDebug(2,Form("Opening file: %s",filename.Data()));

	filename.Prepend("/alien");

	// if option="CACHEREAD" TFile will use the local caching facility!
	TString option="READ";
	if(fCacheFolder != ""){

		// Check if local cache folder was changed in the meanwhile
		TString origCache(TFile::GetCacheFileDir());
		if(fCacheFolder != origCache) {
			IlcWarning(Form("Local cache folder has been overwritten!! fCacheFolder = %s origCache = %s",
					fCacheFolder.Data(), origCache.Data()));
			TFile::SetCacheFileDir(fCacheFolder.Data(), fOperateDisconnected);
			TFile::ShrinkCacheFileDir(fCacheSize, fCleanupInterval);
		}

		option.Prepend("CACHE");
        }

	IlcDebug(2, Form("Option: %s", option.Data()));

	TFile *file = TFile::Open(filename, option);
	if (!file) {
		IlcDebug(2,Form("Can't open file <%s>!", filename.Data()));
		return NULL;
	}

	// get the only IlcCDBEntry object from the file
	// the object in the file is an IlcCDBEntry entry named "IlcCDBEntry"

	IlcCDBEntry* anEntry = dynamic_cast<IlcCDBEntry*> (file->Get("IlcCDBEntry"));

	if (!anEntry) {
		IlcDebug(2,Form("Bad storage data: file does not contain an IlcCDBEntry object!"));
		file->Close();
		return NULL;
	}

	// The object's Id is not reset during storage
	// If object's Id runRange or version do not match with filename,
	// it means that someone renamed file by hand. In this case a warning msg is issued.

	if(anEntry){
		IlcCDBId entryId = anEntry->GetId();
		Int_t tmpSubVersion = dataId->GetSubVersion();
		dataId->SetSubVersion(entryId.GetSubVersion()); // otherwise filename and id may mismatch
		if(!entryId.IsEqual(dataId)){
			IlcWarning(Form("Mismatch between file name and object's Id!"));
			IlcWarning(Form("File name: %s", dataId->ToString().Data()));
			IlcWarning(Form("Object's Id: %s", entryId.ToString().Data()));
		}
		dataId->SetSubVersion(tmpSubVersion);
	}

	anEntry->SetLastStorage("grid");

	// Check whether entry contains a TTree. In case load the tree in memory!
	LoadTreeFromFile(anEntry);

	// close file, return retieved entry
	file->Close(); delete file; file=0;

	return anEntry;
}

//_____________________________________________________________________________
TList* IlcCDBGrid::GetEntries(const IlcCDBId& queryId) {
// multiple request (IlcCDBStorage::GetAll)

	TList* result = new TList();
	result->SetOwner();

	TObjArray validFileIds;
	validFileIds.SetOwner(1);

	Bool_t alreadyLoaded = kFALSE;

	// look for file matching query requests (path, runRange)
	if(queryId.GetFirstRun() == fRun &&
			fPathFilter.Comprises(queryId.GetIlcCDBPath()) && fVersion < 0 && !fMetaDataFilter){
		// look into list of valid files previously loaded with IlcCDBStorage::FillValidFileIds()
		IlcDebug(2,Form("List of files valid for run %d and for path %s was loaded. Looking there!",
					queryId.GetFirstRun(), queryId.GetPath().Data()));

		alreadyLoaded = kTRUE;

	} else {
		// List of files valid for reqested run was not loaded. Looking directly into CDB
		IlcDebug(2,Form("List of files valid for run %d and for path %s was not loaded. Looking directly into CDB!",
					queryId.GetFirstRun(), queryId.GetPath().Data()));

		TString filter;
		MakeQueryFilter(queryId.GetFirstRun(), queryId.GetLastRun(), 0, filter);

		TString path = queryId.GetPath();

		TString pattern = "Run*.root";
		TString optionQuery = "-y";

		TString addFolder = "";
		if (!path.Contains("*")){
		    if (!path.BeginsWith("/")) addFolder += "/";
		    addFolder += path;
		}
		else{
		    if (path.BeginsWith("/")) path.Remove(0,1);
		    if (path.EndsWith("/")) path.Remove(path.Length()-1,1);	
		    TObjArray* tokenArr = path.Tokenize("/");
		    if (tokenArr->GetEntries() != 3) {
			IlcError("Not a 3 level path! Keeping old query...");
			pattern.Prepend(path+"/");
		    }
		    else{
			TString str0 = ((TObjString*)tokenArr->At(0))->String();
			TString str1 = ((TObjString*)tokenArr->At(1))->String();
			TString str2 = ((TObjString*)tokenArr->At(2))->String();
			if (str0 != "*" && str1 != "*" && str2 == "*"){
			    // e.g. "ITS/Calib/*"
			    addFolder = "/"+str0+"/"+str1;
			}
			else if (str0 != "*" && str1 == "*" && str2 == "*"){	
			    // e.g. "ITS/*/*"
			    addFolder = "/"+str0;
			}
			else if (str0 == "*" && str1 == "*" && str2 == "*"){	
			    // e.g. "*/*/*"
			    // do nothing: addFolder is already an empty string;
			}
			else{
			    // e.g. "ITS/*/RecoParam"
			    pattern.Prepend(path+"/");
			}
		    }
		    delete tokenArr; tokenArr=0;
		}

		TString folderCopy(Form("%s%s",fDBFolder.Data(),addFolder.Data()));

		IlcDebug(2,Form("fDBFolder = %s, pattern = %s, filter = %s",folderCopy.Data(), pattern.Data(), filter.Data()));

		TGridResult *res = gGrid->Query(folderCopy, pattern, filter, optionQuery.Data());  

		if (!res) {
		    IlcError("Grid query failed");
		    return 0;
		}

		IlcCDBId validFileId;
		for(int i=0; i<res->GetEntries(); i++){
			TString filename = res->GetKey(i, "lfn");
			if(filename == "") continue;
			if(FilenameToId(filename, validFileId))
					validFileIds.AddLast(validFileId.Clone());
		}
		delete res;
	}

	TIter *iter=0;
	if(alreadyLoaded){
		iter = new TIter(&fValidFileIds);
	} else {
		iter = new TIter(&validFileIds);
	}

	TObjArray selectedIds;
	selectedIds.SetOwner(1);

	// loop on list of valid Ids to select the right version to get.
	// According to query and to the selection criteria list, version can be the highest or exact
	IlcCDBPath pathCopy;
	IlcCDBId* anIdPtr=0;
	IlcCDBId* dataId=0;
	IlcCDBPath queryPath = queryId.GetIlcCDBPath();
	while((anIdPtr = dynamic_cast<IlcCDBId*> (iter->Next()))){
		IlcCDBPath thisCDBPath = anIdPtr->GetIlcCDBPath();
		if(!(queryPath.Comprises(thisCDBPath)) || pathCopy.GetPath() == thisCDBPath.GetPath()) continue;
		pathCopy = thisCDBPath;

		// check the selection criteria list for this query
		IlcCDBId thisId(*anIdPtr);
		thisId.SetVersion(queryId.GetVersion());
		if(!thisId.HasVersion()) GetSelection(&thisId);

		if(alreadyLoaded){
			dataId = GetId(fValidFileIds, thisId);
		} else {
			dataId = GetId(validFileIds, thisId);
		}
		if(dataId) selectedIds.Add(dataId);
	}

	delete iter; iter=0;

	// selectedIds contains the Ids of the files matching all requests of query!
	// All the objects are now ready to be retrieved
	iter = new TIter(&selectedIds);
	while((anIdPtr = dynamic_cast<IlcCDBId*> (iter->Next()))){
		TString filename;
		if (!IdToFilename(*anIdPtr, filename)) {
			IlcDebug(2,Form("Bad data ID encountered! Subnormal error!"));
			continue;
		}

		IlcCDBEntry* anEntry = GetEntryFromFile(filename, anIdPtr);

		if(anEntry) result->Add(anEntry);

	}
	delete iter; iter=0;

	return result;
}

//_____________________________________________________________________________
Bool_t IlcCDBGrid::PutEntry(IlcCDBEntry* entry, const char* mirrors) {
// put an IlcCDBEntry object into the database

	IlcCDBId& id = entry->GetId();

	// set version for the entry to be stored
	if (!PrepareId(id)) return kFALSE;

	// build filename from entry's id
	TString filename;
	if (!IdToFilename(id, filename)) {
		IlcError("Bad ID encountered! Subnormal error!");
		return kFALSE;
	}

	TString folderToTag = Form("%s%s",
					fDBFolder.Data(),
					id.GetPath().Data());

	TDirectory* saveDir = gDirectory;

	TString fullFilename = Form("/alien%s", filename.Data());
	TString seMirrors(mirrors);
	if(seMirrors.IsNull() || seMirrors.IsWhitespace()) seMirrors=GetMirrorSEs();
	// specify SE to filename
	// if a list of SEs was passed to this method or set via SetMirrorSEs, set the first as SE for opening the file.
	// The other SEs will be used in cascade in case of failure in opening the file.
	// The remaining SEs will be used to create replicas.
	TObjArray *arraySEs = seMirrors.Tokenize(',');
	Int_t nSEs = arraySEs->GetEntries();
	Int_t remainingSEs = 1;
	if(nSEs == 0){
	    if (fSE != "default") fullFilename += Form("?se=%s",fSE.Data());
	}else{
	    remainingSEs = nSEs;
	}

	// open file
	TFile *file=0;
	IlcDebug(2, Form("fNretry = %d, fInitRetrySeconds = %d",fNretry,fInitRetrySeconds));
	TString targetSE("");
	while(remainingSEs>0){
	    if(nSEs!=0){
		TObjString *target = (TObjString*) arraySEs->At(nSEs-remainingSEs);
		targetSE=target->String();
		if ( !(targetSE.BeginsWith("ILC::") && targetSE.CountChar(':')==4) ) {
		    IlcError(Form("\"%s\" is an invalid storage element identifier.",targetSE.Data()));
		    continue;
		}
		if(fullFilename.Contains('?')) fullFilename.Remove(fullFilename.Last('?'));
		fullFilename += Form("?se=%s",targetSE.Data());
	    }
	    Int_t remainingAttempts=fNretry;
	    Int_t nsleep = fInitRetrySeconds; // number of seconds between attempts. We let it increase exponentially
	    while(remainingAttempts > 0) {
		IlcDebug(2, Form("Putting the file in the OCDB - Attempt n. %d",fNretry-remainingAttempts+1));
		file = TFile::Open(fullFilename,"CREATE");
		remainingAttempts--;
		if(!file || !file->IsWritable()){
		    IlcError(Form("Can't open file <%s>!", filename.Data()));
		    if(file && !file->IsWritable()) file->Close(); delete file; file=0;
		    IlcDebug(2,Form("Attempt %d failed, sleeping for %d seconds",fNretry-remainingAttempts+1,nsleep));
		    if(remainingAttempts>0) sleep(nsleep);
		}else{
		    remainingAttempts=0;
		}
		nsleep*=fInitRetrySeconds;
	    }
	    remainingSEs--;
	    if(file) break;
	}
	if(!file){
	    IlcError(Form("All %d attempts have failed on all %d SEs. Returning...",fNretry,nSEs));
	    return kFALSE;
	}


	file->cd();

	//SetTreeToFile(entry, file);

	entry->SetVersion(id.GetVersion());

	// write object (key name: "IlcCDBEntry")
	Bool_t result = (file->WriteTObject(entry, "IlcCDBEntry") != 0);
	if (!result) IlcError(Form("Can't write entry to file <%s>!", filename.Data()));


	if (saveDir) saveDir->cd(); else gROOT->cd();
	file->Close(); delete file; file=0;

	if(result) {
	
		if(!TagFileId(filename, &id)){
			IlcInfo(Form("CDB tagging failed. Deleting file %s!",filename.Data()));
			if(!gGrid->Rm(filename.Data()))
				IlcError("Can't delete file!");
			return kFALSE;
		}

		TagFileMetaData(filename, entry->GetMetaData());
	}

	IlcInfo(Form("CDB object stored into file %s", filename.Data()));
	if(nSEs==0)
	    IlcInfo(Form("Storage Element: %s", fSE.Data()));
	else
	    IlcInfo(Form("Storage Element: %s", targetSE.Data()));

	//In case of other SEs specified by the user, mirror the file to the remaining SEs
	while(remainingSEs>0){
	    TString mirrorCmd("mirror ");
	    mirrorCmd += filename;
	    mirrorCmd += " ";
	    TObjString *target = (TObjString*) arraySEs->At(nSEs-remainingSEs);
	    TString mirrorSE(target->String());
	    mirrorCmd += mirrorSE;
	    IlcDebug(5,Form("mirror command: \"%s\"",mirrorCmd.Data()));
	    IlcInfo(Form("Mirroring to storage element: %s", mirrorSE.Data()));
	    gGrid->Command(mirrorCmd.Data());
	    remainingSEs--;
	}

	return result;
}

//_____________________________________________________________________________
Bool_t IlcCDBGrid::AddTag(TString& folderToTag, const char* tagname){
// add "tagname" tag (CDB or CDB_MD) to folder where object will be stored

	Bool_t result = kTRUE;
	IlcDebug(2, Form("adding %s tag to folder %s", tagname, folderToTag.Data()));
	TString addTag = Form("addTag %s %s", folderToTag.Data(), tagname);
	TGridResult *gridres = gGrid->Command(addTag.Data());
	const char* resCode = gridres->GetKey(0,"__result__"); // '1' if success
	if(resCode[0] != '1') {
		IlcError(Form("Couldn't add %s tags to folder %s !",
						tagname, folderToTag.Data()));
		result = kFALSE;
	}
	delete gridres;
	return result;
}

//_____________________________________________________________________________
Bool_t IlcCDBGrid::TagFileId(TString& filename, const IlcCDBId* id){
// tag stored object in CDB table using object Id's parameters


        TString dirname(filename);
	Int_t dirNumber = gGrid->Mkdir(dirname.Remove(dirname.Last('/')),"-d");
	
	TString addTagValue1 = Form("addTagValue %s CDB ", filename.Data());
	TString addTagValue2 = Form("first_run=%d last_run=%d version=%d ",
					id->GetFirstRun(),
					id->GetLastRun(),
					id->GetVersion());
	TString addTagValue3 = Form("path_level_0=\"%s\" path_level_1=\"%s\" path_level_2=\"%s\" ",
					id->GetPathLevel(0).Data(),
					id->GetPathLevel(1).Data(),
					id->GetPathLevel(2).Data());
	//TString addTagValue4 = Form("version_path=\"%s\" dir_number=%d",Form("%d_%s",id->GetVersion(),filename.Data()),dirNumber); 
	TString addTagValue4 = Form("version_path=\"%09d%s\" dir_number=%d",id->GetVersion(),filename.Data(),dirNumber); 
	TString addTagValue = Form("%s%s%s%s",
					addTagValue1.Data(),
					addTagValue2.Data(),
					addTagValue3.Data(),
					addTagValue4.Data());

	Bool_t result = kFALSE;
	IlcDebug(2, Form("Tagging file. Tag command: %s", addTagValue.Data()));
	TGridResult* res = gGrid->Command(addTagValue.Data());
	const char* resCode = res->GetKey(0,"__result__"); // '1' if success
	if(resCode[0] != '1') {
		IlcError(Form("Couldn't add CDB tag value to file %s !",
						filename.Data()));
		result = kFALSE;
	} else {
		IlcDebug(2, "Object successfully tagged.");
		result = kTRUE;
	}
	delete res;
	return result;

}

//_____________________________________________________________________________
Bool_t IlcCDBGrid::TagShortLived(TString& filename, Bool_t value){
// tag folder with ShortLived tag

	TString addTagValue = Form("addTagValue %s ShortLived_try value=%d", filename.Data(), value);

	Bool_t result = kFALSE;
	IlcDebug(2, Form("Tagging file. Tag command: %s", addTagValue.Data()));
	TGridResult* res = gGrid->Command(addTagValue.Data());
	const char* resCode = res->GetKey(0,"__result__"); // '1' if success
	if(resCode[0] != '1') {
		IlcError(Form("Couldn't add ShortLived tag value to file %s !", filename.Data()));
		result = kFALSE;
	} else {
		IlcDebug(2,"Object successfully tagged.");
		result = kTRUE;
	}
	delete res;
	return result;

}

//_____________________________________________________________________________
Bool_t IlcCDBGrid::TagFileMetaData(TString& filename, const IlcCDBMetaData* md){
// tag stored object in CDB table using object Id's parameters

	TString addTagValue1 = Form("addTagValue %s CDB_MD ", filename.Data());
	TString addTagValue2 = Form("object_classname=\"%s\" responsible=\"%s\" beam_period=%d ",
					md->GetObjectClassName(),
					md->GetResponsible(),
					md->GetBeamPeriod());
	TString addTagValue3 = Form("ilcroot_version=\"%s\" comment=\"%s\"",
					md->GetIlcRootVersion(),
					md->GetComment());
	TString addTagValue = Form("%s%s%s",
					addTagValue1.Data(),
					addTagValue2.Data(),
					addTagValue3.Data());

	Bool_t result = kFALSE;
	IlcDebug(2, Form("Tagging file. Tag command: %s", addTagValue.Data()));
	TGridResult* res = gGrid->Command(addTagValue.Data());
	const char* resCode = res->GetKey(0,"__result__"); // '1' if success
	if(resCode[0] != '1') {
		IlcWarning(Form("Couldn't add CDB_MD tag value to file %s !",
						filename.Data()));
		result = kFALSE;
	} else {
		IlcDebug(2,"Object successfully tagged.");
		result = kTRUE;
	}
	return result;
}

//_____________________________________________________________________________
TList* IlcCDBGrid::GetIdListFromFile(const char* fileName){

	TString turl(fileName);
	turl.Prepend("/alien" + fDBFolder);
	turl += "?se="; turl += fSE.Data();
	TFile *file = TFile::Open(turl);
	if (!file) {
		IlcError(Form("Can't open selection file <%s>!", turl.Data()));
		return NULL;
	}

	TList *list = new TList();
	list->SetOwner();
	int i=0;
	TString keycycle;

	IlcCDBId *id;
	while(1){
		i++;
		keycycle = "IlcCDBId;";
		keycycle+=i;
		
		id = (IlcCDBId*) file->Get(keycycle);
		if(!id) break;
		list->AddFirst(id);
	}
	file->Close(); delete file; file=0;
	
	return list;


}

//_____________________________________________________________________________
Bool_t IlcCDBGrid::Contains(const char* path) const{
// check for path in storage's DBFolder

	TString initDir(gGrid->Pwd(0));
	TString dirName(fDBFolder);
	dirName += path; // dirName = fDBFolder/path
	Bool_t result=kFALSE;
	if (gGrid->Cd(dirName,0)) result=kTRUE;
	gGrid->Cd(initDir.Data(),0);
	return result;
}

//_____________________________________________________________________________
void IlcCDBGrid::QueryValidFiles()
{
// Query the CDB for files valid for IlcCDBStorage::fRun
// fills list fValidFileIds with IlcCDBId objects created from file name

	TString filter;
	MakeQueryFilter(fRun, fRun, fMetaDataFilter, filter);

	TString path = fPathFilter.GetPath();

	TString pattern = "Run*";
	TString optionQuery = "-y";
	if(fVersion >= 0) {
		pattern += Form("_v%d_s0", fVersion);
		optionQuery = "";
	}
	pattern += ".root";
	IlcDebug(2,Form("pattern: %s", pattern.Data()));

	TString addFolder = "";
	if (!path.Contains("*")){
		if (!path.BeginsWith("/")) addFolder += "/";
		addFolder += path;
	}
	else{
		if (path.BeginsWith("/")) path.Remove(0,1);
		if (path.EndsWith("/")) path.Remove(path.Length()-1,1);	
		TObjArray* tokenArr = path.Tokenize("/");
		if (tokenArr->GetEntries() != 3) {
			IlcError("Not a 3 level path! Keeping old query...");
			pattern.Prepend(path+"/");
		}
		else{
			TString str0 = ((TObjString*)tokenArr->At(0))->String();
			TString str1 = ((TObjString*)tokenArr->At(1))->String();
			TString str2 = ((TObjString*)tokenArr->At(2))->String();
			if (str0 != "*" && str1 != "*" && str2 == "*"){
				// e.g. "ITS/Calib/*"
				addFolder = "/"+str0+"/"+str1;
			}
			else if (str0 != "*" && str1 == "*" && str2 == "*"){	
				// e.g. "ITS/*/*"
				addFolder = "/"+str0;
			}
			else if (str0 == "*" && str1 == "*" && str2 == "*"){	
				// e.g. "*/*/*"
				// do nothing: addFolder is already an empty string;
			}
			else{
				// e.g. "ITS/*/RecoParam"
				pattern.Prepend(path+"/");
			}
		}
		delete tokenArr; tokenArr=0;
	}

	TString folderCopy(Form("%s%s",fDBFolder.Data(),addFolder.Data()));

	IlcDebug(2,Form("fDBFolder = %s, pattern = %s, filter = %s",folderCopy.Data(), pattern.Data(), filter.Data()));

	if (optionQuery == "-y"){
		IlcInfo("Only latest version will be returned");
	} 

	TGridResult *res = gGrid->Query(folderCopy, pattern, filter, optionQuery.Data());  

	if (!res) {
		IlcError("Grid query failed");
		return;
	}

	TIter next(res);
	TMap *map;
	while ((map = (TMap*)next())) {
	  TObjString *entry;
	  if ((entry = (TObjString *) ((TMap *)map)->GetValue("lfn"))) {
	    TString& filename = entry->String();
	    if(filename.IsNull()) continue;
	    IlcDebug(2,Form("Found valid file: %s", filename.Data()));
	    IlcCDBId *validFileId = new IlcCDBId;
	    Bool_t result = FilenameToId(filename, *validFileId);
	    if(result) {
	      fValidFileIds.AddLast(validFileId);
	    }
	    else {
	      delete validFileId;
	    }
	  }
	}
	delete res;

}

//_____________________________________________________________________________
void IlcCDBGrid::MakeQueryFilter(Int_t firstRun, Int_t lastRun,
					const IlcCDBMetaData* md, TString& result) const
{
// create filter for file query

	result = Form("CDB:first_run<=%d and CDB:last_run>=%d", firstRun, lastRun);

//	if(version >= 0) {
//		result += Form(" and CDB:version=%d", version);
//	}
//	if(pathFilter.GetLevel0() != "*") {
//		result += Form(" and CDB:path_level_0=\"%s\"", pathFilter.GetLevel0().Data());
//	}
//	if(pathFilter.GetLevel1() != "*") {
//		result += Form(" and CDB:path_level_1=\"%s\"", pathFilter.GetLevel1().Data());
//	}
//	if(pathFilter.GetLevel2() != "*") {
//		result += Form(" and CDB:path_level_2=\"%s\"", pathFilter.GetLevel2().Data());
//	}

	if(md){
		if(md->GetObjectClassName()[0] != '\0') {
			result += Form(" and CDB_MD:object_classname=\"%s\"", md->GetObjectClassName());
		}
		if(md->GetResponsible()[0] != '\0') {
			result += Form(" and CDB_MD:responsible=\"%s\"", md->GetResponsible());
		}
		if(md->GetBeamPeriod() != 0) {
			result += Form(" and CDB_MD:beam_period=%d", md->GetBeamPeriod());
		}
		if(md->GetIlcRootVersion()[0] != '\0') {
			result += Form(" and CDB_MD:ilcroot_version=\"%s\"", md->GetIlcRootVersion());
		}
		if(md->GetComment()[0] != '\0') {
			result += Form(" and CDB_MD:comment=\"%s\"", md->GetComment());
		}
	}
	IlcDebug(2, Form("filter: %s",result.Data()));

}

//_____________________________________________________________________________
Int_t IlcCDBGrid::GetLatestVersion(const char* path, Int_t run){
// get last version found in the database valid for run and path

	IlcCDBPath aCDBPath(path);
	if(!aCDBPath.IsValid() || aCDBPath.IsWildcard()) {
		IlcError(Form("Invalid path in request: %s", path));
		return -1;
	}
	IlcCDBId query(path, run, run, -1, -1);

	IlcCDBId* dataId = 0;
	// look for file matching query requests (path, runRange, version)
	if(run == fRun && fPathFilter.Comprises(aCDBPath) && fVersion < 0){
		// look into list of valid files previously loaded with IlcCDBStorage::FillValidFileIds()
		IlcDebug(2, Form("List of files valid for run %d and for path %s was loaded. Looking there!",
					run, path));
		dataId = GetId(fValidFileIds, query);
		if (!dataId) return -1;
		Int_t version = dataId->GetVersion();
		delete dataId;
		return version;

	}
	// List of files valid for reqested run was not loaded. Looking directly into CDB
	IlcDebug(2, Form("List of files valid for run %d and for path %s was not loaded. Looking directly into CDB!",
				run, path));

	TObjArray validFileIds;
	validFileIds.SetOwner(1);

	TString filter;
	MakeQueryFilter(run, run, 0, filter);

	TString pattern = ".root";

	TString folderCopy(Form("%s%s/Run",fDBFolder.Data(),path));

	IlcDebug(2,Form("** fDBFolder = %s, pattern = %s, filter = %s",folderCopy.Data(), pattern.Data(), filter.Data()));
	TGridResult *res = gGrid->Query(folderCopy, pattern, filter, "-y -m");

	IlcCDBId validFileId;
	if (res->GetEntries()>1){
		IlcWarning("Number of found entries >1, even if option -y was used");
		for(int i=0; i<res->GetEntries(); i++){
			TString filename = res->GetKey(i, "lfn");
			if(filename == "") continue;
			if(FilenameToId(filename, validFileId))
				validFileIds.AddLast(validFileId.Clone());
		}
		dataId = GetId(validFileIds, query);
		if (!dataId) return -1;
		
		Int_t version = dataId->GetVersion();
		delete dataId;
		return version;
	}
	else if (res->GetEntries()==1){
		TString filename = res->GetKey(0, "lfn");
		if(filename == "") {
			IlcError("The only entry found has filename empty");
			return -1;
		}
		if(FilenameToId(filename, validFileId)) return validFileId.GetVersion();
		else{
			IlcError("Impossible to get FileId from filename");
			return -1;
		}
	}
	else {
		IlcError("No entries found");
		return -1;
	}

	delete res;


}

//_____________________________________________________________________________
Int_t IlcCDBGrid::GetLatestSubVersion(const char* /*path*/, Int_t /*run*/, Int_t /*version*/){
// get last subversion found in the database valid for run and path
	IlcError("Objects in GRID storage have no sub version!");
	return -1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                             //
// IlcCDBGrid factory  			                                                       //
//                                                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////

ClassImp(IlcCDBGridFactory)

//_____________________________________________________________________________
Bool_t IlcCDBGridFactory::Validate(const char* gridString) {
// check if the string is valid Grid URI

        TRegexp gridPattern("^alien://.+$");

        return TString(gridString).Contains(gridPattern);
}

//_____________________________________________________________________________
IlcCDBParam* IlcCDBGridFactory::CreateParameter(const char* gridString) {
// create IlcCDBGridParam class from the URI string

	if (!Validate(gridString)) {
		return NULL;
	}

	TString buffer(gridString);

 	TString gridUrl 	= "alien://";
	TString user 		= "";
	TString dbFolder 	= "";
	TString se		= "default";
	TString cacheFolder	= "";
	Bool_t  operateDisconnected = kTRUE;
	Long64_t cacheSize          = (UInt_t) 1024*1024*1024; // 1GB
	Long_t cleanupInterval      = 0;

	TObjArray *arr = buffer.Tokenize('?');
	TIter iter(arr);
	TObjString *str = 0;

	while((str = (TObjString*) iter.Next())){
		TString entry(str->String());
		Int_t indeq = entry.Index('=');
		if(indeq == -1) {
			if(entry.BeginsWith("alien://")) { // maybe it's a gridUrl!
				gridUrl = entry;
				continue;
			} else {
				IlcError(Form("Invalid entry! %s",entry.Data()));
				continue;
			}
		}
		
		TString key = entry(0,indeq);
		TString value = entry(indeq+1,entry.Length()-indeq);

		if(key.Contains("grid",TString::kIgnoreCase)) {
			gridUrl += value;
		}
		else if (key.Contains("user",TString::kIgnoreCase)){
			user = value;
		}
		else if (key.Contains("se",TString::kIgnoreCase)){
			se = value;
		}
		else if (key.Contains("cacheF",TString::kIgnoreCase)){
			cacheFolder = value;
			if (!cacheFolder.IsNull() && !cacheFolder.EndsWith("/"))
      				cacheFolder += "/";
		}
		else if (key.Contains("folder",TString::kIgnoreCase)){
			dbFolder = value;
		}
		else if (key.Contains("operateDisc",TString::kIgnoreCase)){
			if(value == "kTRUE") {
				operateDisconnected = kTRUE;
			} else if (value == "kFALSE") {
   				operateDisconnected = kFALSE;
			} else if (value == "0" || value == "1") {
				operateDisconnected = (Bool_t) value.Atoi();
			} else {
				IlcError(Form("Invalid entry! %s",entry.Data()));
				return NULL;
			}
		}
		else if (key.Contains("cacheS",TString::kIgnoreCase)){
			if(value.IsDigit()) {
				cacheSize = value.Atoi();
			} else {
				IlcError(Form("Invalid entry! %s",entry.Data()));
				return NULL;
			}
		}
		else if (key.Contains("cleanupInt",TString::kIgnoreCase)){
			if(value.IsDigit()) {
				cleanupInterval = value.Atoi();
			} else {
				IlcError(Form("Invalid entry! %s",entry.Data()));
				return NULL;
			}
		}
		else{
			IlcError(Form("Invalid entry! %s",entry.Data()));
			return NULL;
		}
	}
	delete arr; arr=0;

	IlcDebug(2, Form("gridUrl:	%s", gridUrl.Data()));
	IlcDebug(2, Form("user:	%s", user.Data()));
	IlcDebug(2, Form("dbFolder:	%s", dbFolder.Data()));
	IlcDebug(2, Form("s.e.:	%s", se.Data()));
	IlcDebug(2, Form("local cache folder: %s", cacheFolder.Data()));
	IlcDebug(2, Form("local cache operate disconnected: %d", operateDisconnected));
	IlcDebug(2, Form("local cache size: %lld", cacheSize));
	IlcDebug(2, Form("local cache cleanup interval: %ld", cleanupInterval));

	if(dbFolder == ""){
		IlcError("Base folder must be specified!");
		return NULL;
	}

	return new IlcCDBGridParam(gridUrl.Data(), user.Data(),
	                  dbFolder.Data(), se.Data(), cacheFolder.Data(),
			  operateDisconnected, cacheSize, cleanupInterval);
}

//_____________________________________________________________________________
IlcCDBStorage* IlcCDBGridFactory::Create(const IlcCDBParam* param) {
// create IlcCDBGrid storage instance from parameters
	
	IlcCDBGrid *grid = 0;
	if (IlcCDBGridParam::Class() == param->IsA()) {

		const IlcCDBGridParam* gridParam = (const IlcCDBGridParam*) param;
		grid = new IlcCDBGrid(gridParam->GridUrl().Data(),
				      gridParam->GetUser().Data(),
				      gridParam->GetDBFolder().Data(),
				      gridParam->GetSE().Data(),
				      gridParam->GetCacheFolder().Data(),
				      gridParam->GetOperateDisconnected(),
				      gridParam->GetCacheSize(),
				      gridParam->GetCleanupInterval());

	}

	if(!gGrid && grid) {
		delete grid; grid=0;
	}

	return grid;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                             //
// IlcCDBGrid Parameter class  			                                               //                                         //
//                                                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////

ClassImp(IlcCDBGridParam)

//_____________________________________________________________________________
IlcCDBGridParam::IlcCDBGridParam():
 IlcCDBParam(),
 fGridUrl(),
 fUser(),
 fDBFolder(),
 fSE(),
 fCacheFolder(),
 fOperateDisconnected(),
 fCacheSize(),
 fCleanupInterval()

 {
// default constructor

}

//_____________________________________________________________________________
IlcCDBGridParam::IlcCDBGridParam(const char* gridUrl, const char* user, const char* dbFolder,
				const char* se, const char* cacheFolder, Bool_t operateDisconnected,
				Long64_t cacheSize, Long_t cleanupInterval):
 IlcCDBParam(),
 fGridUrl(gridUrl),
 fUser(user),
 fDBFolder(dbFolder),
 fSE(se),
 fCacheFolder(cacheFolder),
 fOperateDisconnected(operateDisconnected),
 fCacheSize(cacheSize),
 fCleanupInterval(cleanupInterval)
{
// constructor

	SetType("alien");

	TString uri = Form("%s?User=%s?DBFolder=%s?SE=%s?CacheFolder=%s"
			"?OperateDisconnected=%d?CacheSize=%lld?CleanupInterval=%ld",
			fGridUrl.Data(), fUser.Data(),
			fDBFolder.Data(), fSE.Data(), fCacheFolder.Data(),
			fOperateDisconnected, fCacheSize, fCleanupInterval);

	SetURI(uri.Data());
}

//_____________________________________________________________________________
IlcCDBGridParam::~IlcCDBGridParam() {
// destructor

}

//_____________________________________________________________________________
IlcCDBParam* IlcCDBGridParam::CloneParam() const {
// clone parameter

        return new IlcCDBGridParam(fGridUrl.Data(), fUser.Data(),
					fDBFolder.Data(), fSE.Data(), fCacheFolder.Data(),
					fOperateDisconnected, fCacheSize, fCleanupInterval);
}

//_____________________________________________________________________________
ULong_t IlcCDBGridParam::Hash() const {
// return Hash function

        return fGridUrl.Hash()+fUser.Hash()+fDBFolder.Hash()+fSE.Hash()+fCacheFolder.Hash();
}

//_____________________________________________________________________________
Bool_t IlcCDBGridParam::IsEqual(const TObject* obj) const {
// check if this object is equal to IlcCDBParam obj

        if (this == obj) {
                return kTRUE;
        }

        if (IlcCDBGridParam::Class() != obj->IsA()) {
                return kFALSE;
        }

        IlcCDBGridParam* other = (IlcCDBGridParam*) obj;

        if(fGridUrl != other->fGridUrl) return kFALSE;
        if(fUser != other->fUser) return kFALSE;
        if(fDBFolder != other->fDBFolder) return kFALSE;
        if(fSE != other->fSE) return kFALSE;
        if(fCacheFolder != other->fCacheFolder) return kFALSE;
        if(fOperateDisconnected != other->fOperateDisconnected) return kFALSE;
        if(fCacheSize != other->fCacheSize) return kFALSE;
        if(fCleanupInterval != other->fCleanupInterval) return kFALSE;
	return kTRUE;
}

