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

/* $Id: IlcRawReader.cxx 57707 2012-07-06 09:17:38Z hristov $ */

///////////////////////////////////////////////////////////////////////////////
///
/// This is the base class for reading raw data.
///
/// The derived classes, which operate on concrete raw data formats,
/// should implement
/// - ReadHeader to read the next (data/equipment) header
/// - ReadNextData to read the next raw data block (=1 DDL)
/// - ReadNext to read a given number of bytes
/// - several getters like GetType
///
/// Sequential access to the raw data is provided by the methods
/// ReadHeader, ReadNextData, ReadNextInt, ReadNextShort, ReadNextChar
///
/// If only data from a specific detector (and a given range of DDL numbers)
/// should be read, this can be achieved by the Select method.
/// Several getters provide information about the current event and the
/// current type of raw data.
///
///////////////////////////////////////////////////////////////////////////////

#include <TClass.h>
#include <TPluginManager.h>
#include <TROOT.h>
#include <TInterpreter.h>
#include <TSystem.h>
#include <TPRegexp.h>
#include <THashList.h>

#include <Riostream.h>
#include "IlcRawReader.h"
#include "IlcRawReaderFile.h"
#include "IlcRawReaderDate.h"
#include "IlcRawReaderRoot.h"
#include "IlcRawReaderChain.h"
#include "IlcDAQ.h"
#include "IlcLog.h"

ClassImp(IlcRawReader)


IlcRawReader::IlcRawReader() :
  fEquipmentIdsIn(NULL),
  fEquipmentIdsOut(NULL),
  fRequireHeader(kTRUE),
  fHeader(NULL),
  fCount(0),
  fSelectEquipmentType(-1),
  fSelectMinEquipmentId(-1),
  fSelectMaxEquipmentId(-1),
  fSkipInvalid(kFALSE),
  fSelectEventType(-1),
  fSelectTriggerMask(0),
  fSelectTriggerExpr(),
  fErrorCode(0),
  fEventNumber(-1),
  fErrorLogs("IlcRawDataErrorLog",100),
  fHeaderSwapped(NULL),
  fIsValid(kTRUE),
  fIsTriggerClassLoaded(kFALSE)
{
// default constructor: initialize data members
// Allocate the swapped header in case of Mac
#ifndef R__BYTESWAP
  fHeaderSwapped=new IlcRawDataHeader();
#endif
}

Bool_t IlcRawReader::LoadEquipmentIdsMap(const char *fileName)
{
  // Open the mapping file
  // and load the mapping data
  ifstream input(fileName);
  if (input.is_open()) {
    Warning("IlcRawReader","Equipment ID mapping file is found !");
    const Int_t kMaxDDL = 256;
    fEquipmentIdsIn = new TArrayI(kMaxDDL);
    fEquipmentIdsOut = new TArrayI(kMaxDDL);
    Int_t equipIn, equipOut;
    Int_t nIds = 0;
    while (input >> equipIn >> equipOut) {
      if (nIds >= kMaxDDL) {
	Error("IlcRawReader","Too many equipment Id mappings found ! Truncating the list !");
	break;
      }
      fEquipmentIdsIn->AddAt(equipIn,nIds); 
      fEquipmentIdsOut->AddAt(equipOut,nIds);
      nIds++;
    }
    fEquipmentIdsIn->Set(nIds);
    fEquipmentIdsOut->Set(nIds);
    input.close();
    return kTRUE;
  }
  else {
    Error("IlcRawReader","equipment id map file is not found ! Skipping the mapping !");
    return kFALSE;
  }
}

IlcRawReader::IlcRawReader(const IlcRawReader& rawReader) :
  TObject(rawReader),
  fEquipmentIdsIn(rawReader.fEquipmentIdsIn),
  fEquipmentIdsOut(rawReader.fEquipmentIdsOut),
  fRequireHeader(rawReader.fRequireHeader),
  fHeader(rawReader.fHeader),
  fCount(rawReader.fCount),
  fSelectEquipmentType(rawReader.fSelectEquipmentType),
  fSelectMinEquipmentId(rawReader.fSelectMinEquipmentId),
  fSelectMaxEquipmentId(rawReader.fSelectMaxEquipmentId),
  fSkipInvalid(rawReader.fSkipInvalid),
  fSelectEventType(rawReader.fSelectEventType),
  fSelectTriggerMask(rawReader.fSelectTriggerMask),
  fSelectTriggerExpr(rawReader.fSelectTriggerExpr),
  fErrorCode(0),
  fEventNumber(-1),
  fErrorLogs("IlcRawDataErrorLog",100),
  fHeaderSwapped(NULL),
  fIsValid(rawReader.fIsValid),
  fIsTriggerClassLoaded(rawReader.fIsTriggerClassLoaded)
{
// copy constructor
// Allocate the swapped header in case of Mac
#ifndef R__BYTESWAP
  fHeaderSwapped=new IlcRawDataHeader(*rawReader.fHeaderSwapped);
#endif
}

IlcRawReader& IlcRawReader::operator = (const IlcRawReader& rawReader)
{
// assignment operator
  if(&rawReader == this) return *this;
  fEquipmentIdsIn = rawReader.fEquipmentIdsIn;
  fEquipmentIdsOut = rawReader.fEquipmentIdsOut;

  fHeader = rawReader.fHeader;
  fCount = rawReader.fCount;

  fSelectEquipmentType = rawReader.fSelectEquipmentType;
  fSelectMinEquipmentId = rawReader.fSelectMinEquipmentId;
  fSelectMaxEquipmentId = rawReader.fSelectMaxEquipmentId;
  fSkipInvalid = rawReader.fSkipInvalid;
  fSelectEventType = rawReader.fSelectEventType;
  fSelectTriggerMask = rawReader.fSelectTriggerMask;
  fSelectTriggerExpr = rawReader.fSelectTriggerExpr;

  fErrorCode = rawReader.fErrorCode;

  fEventNumber = rawReader.fEventNumber;
  fErrorLogs = *((TClonesArray*)rawReader.fErrorLogs.Clone());

  fIsValid = rawReader.fIsValid;
  fIsTriggerClassLoaded = rawReader.fIsTriggerClassLoaded;

  return *this;
}

IlcRawReader::~IlcRawReader()
{
  // destructor
  // delete the mapping arrays if
  // initialized
  if (fEquipmentIdsIn) delete fEquipmentIdsIn;
  if (fEquipmentIdsOut) delete fEquipmentIdsOut;
  fErrorLogs.Delete();
  if (fHeaderSwapped) delete fHeaderSwapped;
}

IlcRawReader* IlcRawReader::Create(const char *uri)
{
  // RawReader's factory
  // It instantiate corresponding raw-reader implementation class object
  // depending on the URI provided
  // Normal URIs point to files, while the URI starting with
  // 'mem://:' or 'mem://<filename>' will create
  // IlcRawReaderDateOnline object which is supposed to be used
  // in the online reconstruction

  TString strURI = uri;

  if (strURI.IsNull()) {
    IlcWarningClass("No raw-reader created");
    return NULL;
  }

  TObjArray *fields = strURI.Tokenize("?");
  TString &fileURI = ((TObjString*)fields->At(0))->String();

  IlcRawReader *rawReader = NULL;
  if (fileURI.BeginsWith("mem://") || fileURI.BeginsWith("^")) {
    if (fileURI.BeginsWith("mem://")) fileURI.ReplaceAll("mem://","");
    IlcInfoClass(Form("Creating raw-reader in order to read events in shared memory (option=%s)",fileURI.Data()));

    TPluginManager* pluginManager = gROOT->GetPluginManager();
    TString rawReaderName = "IlcRawReaderDateOnline";
    TPluginHandler* pluginHandler = pluginManager->FindHandler("IlcRawReader", "online");
    // if not, add a plugin for it
    if (!pluginHandler) {
      pluginManager->AddHandler("IlcRawReader", "online", 
				"IlcRawReaderDateOnline", "RAWDatarecOnline", "IlcRawReaderDateOnline(const char*)");
      pluginHandler = pluginManager->FindHandler("IlcRawReader", "online");
    }
    if (pluginHandler && (pluginHandler->LoadPlugin() == 0)) {
      rawReader = (IlcRawReader*)pluginHandler->ExecPlugin(1,fileURI.Data());
    }
    else {
      delete fields;
      return NULL;
    }
  }
  else if (fileURI.BeginsWith("amore://")) {
    // A special raw-data URL used in case
    // the raw-data reading is steered from
    // ouside, i.e. from AMORE
    fileURI.ReplaceAll("amore://","");
    IlcInfoClass("Creating raw-reader in order to read events sent by AMORE");
    rawReader = new IlcRawReaderDate((void *)NULL);
  }
  else if (fileURI.BeginsWith("collection://")) {
    fileURI.ReplaceAll("collection://","");
    IlcInfoClass(Form("Creating raw-reader in order to read raw-data files collection defined in %s",fileURI.Data()));
    rawReader = new IlcRawReaderChain(fileURI);
  }
  else if (fileURI.BeginsWith("raw://run")) {
    fileURI.ReplaceAll("raw://run","");
    if (fileURI.IsDigit()) {
      rawReader = new IlcRawReaderChain(fileURI.Atoi());
    }
    else {
      IlcErrorClass(Form("Invalid syntax: %s",fileURI.Data()));
      fields->Delete();
      return NULL;
    }
  }
  else {
    IlcInfoClass(Form("Creating raw-reader in order to read raw-data file: %s",fileURI.Data()));
    TString filename(gSystem->ExpandPathName(fileURI.Data()));
    if (filename.EndsWith("/")) {
      rawReader = new IlcRawReaderFile(filename);
    } else if (filename.EndsWith(".root")) {
      rawReader = new IlcRawReaderRoot(filename);
    } else {
      rawReader = new IlcRawReaderDate(filename);
    }
  }

  if (!rawReader->IsRawReaderValid()) {
    IlcErrorClass(Form("Raw-reader is invalid - check the input URI (%s)",fileURI.Data()));
    delete rawReader;
    fields->Delete();
    delete fields;
    return NULL;
  }

  // Now apply event selection criteria (if specified)
  if (fields->GetEntries() > 1) {
    Int_t eventType = -1;
    ULong64_t triggerMask = 0;
    TString triggerExpr;
    for(Int_t i = 1; i < fields->GetEntries(); i++) {
      if (!fields->At(i)) continue;
      TString &option = ((TObjString*)fields->At(i))->String();
      if (option.BeginsWith("EventType=",TString::kIgnoreCase)) {
	option.ReplaceAll("EventType=","");
	eventType = option.Atoi();
	continue;
      }
      if (option.BeginsWith("Trigger=",TString::kIgnoreCase)) {
	option.ReplaceAll("Trigger=","");
	if (option.IsDigit()) {
	  triggerMask = option.Atoll();
	}
	else {
	  triggerExpr = option.Data();
	}
	continue;
      }
      IlcWarningClass(Form("Ignoring invalid event selection option: %s",option.Data()));
    }
    IlcInfoClass(Form("Event selection criteria specified:   eventype=%d   trigger mask=%llx   trigger expression=%s",
		 eventType,triggerMask,triggerExpr.Data()));
    rawReader->SelectEvents(eventType,triggerMask,triggerExpr.Data());
  }

  fields->Delete();
  delete fields;

  return rawReader;
}

Int_t IlcRawReader::GetMappedEquipmentId() const
{
  if (!fEquipmentIdsIn || !fEquipmentIdsOut) {
    Error("IlcRawReader","equipment Ids mapping is not initialized !");
    return GetEquipmentId();
  }
  Int_t equipmentId = GetEquipmentId();
  for(Int_t iId = 0; iId < fEquipmentIdsIn->GetSize(); iId++) {
    if (equipmentId == fEquipmentIdsIn->At(iId)) {
      equipmentId = fEquipmentIdsOut->At(iId);
      break;
    }
  }
  return equipmentId;
}

Int_t IlcRawReader::GetDetectorID() const
{
  // Get the detector ID
  // The list of detector IDs
  // can be found in IlcDAQ.h
  Int_t equipmentId;
  if (fEquipmentIdsIn && fEquipmentIdsIn)
    equipmentId = GetMappedEquipmentId();
  else
    equipmentId = GetEquipmentId();

  if (equipmentId >= 0) {
    Int_t ddlIndex;
    return IlcDAQ::DetectorIDFromDdlID(equipmentId,ddlIndex);
  }
  else
    return -1;
}

Int_t IlcRawReader::GetDDLID() const
{
  // Get the DDL ID (within one sub-detector)
  // The list of detector IDs
  // can be found in IlcDAQ.h
  Int_t equipmentId;
  if (fEquipmentIdsIn && fEquipmentIdsIn)
    equipmentId = GetMappedEquipmentId();
  else
    equipmentId = GetEquipmentId();

  if (equipmentId >= 0) {
    Int_t ddlIndex;
    IlcDAQ::DetectorIDFromDdlID(equipmentId,ddlIndex);
    return ddlIndex;
  }
  else
    return -1;
}

void IlcRawReader::Select(const char *detectorName, Int_t minDDLID, Int_t maxDDLID)
{
// read only data of the detector with the given name and in the given
// range of DDLs (minDDLID <= DDLID <= maxDDLID).
// no selection is applied if a value < 0 is used.
  Int_t detectorID = IlcDAQ::DetectorID(detectorName);
  if(detectorID >= 0)
    Select(detectorID,minDDLID,maxDDLID);
}

void IlcRawReader::Select(Int_t detectorID, Int_t minDDLID, Int_t maxDDLID)
{
// read only data of the detector with the given ID and in the given
// range of DDLs (minDDLID <= DDLID <= maxDDLID).
// no selection is applied if a value < 0 is used.

  fSelectEquipmentType = -1;

  if (minDDLID < 0)
    fSelectMinEquipmentId = IlcDAQ::DdlIDOffset(detectorID);
  else
    fSelectMinEquipmentId = IlcDAQ::DdlID(detectorID,minDDLID);

  if (maxDDLID < 0)
    fSelectMaxEquipmentId = IlcDAQ::DdlID(detectorID,IlcDAQ::NumberOfDdls(detectorID)-1);
  else
    fSelectMaxEquipmentId = IlcDAQ::DdlID(detectorID,maxDDLID);
}

void IlcRawReader::SelectEquipment(Int_t equipmentType, 
				   Int_t minEquipmentId, Int_t maxEquipmentId)
{
// read only data of the equipment with the given type and in the given
// range of IDs (minEquipmentId <= EquipmentId <= maxEquipmentId).
// no selection is applied if a value < 0 is used.

  fSelectEquipmentType = equipmentType;
  fSelectMinEquipmentId = minEquipmentId;
  fSelectMaxEquipmentId = maxEquipmentId;
}

void IlcRawReader::SelectEvents(Int_t type, ULong64_t triggerMask,
				const char *triggerExpr)
{
// read only events with the given type and optionally
// trigger mask.
// no selection is applied if value = 0 is used.
// Trigger selection can be done via string (triggerExpr)
// which defines the trigger logic to be used. It works only
// after LoadTriggerClass() method is called for all involved
// trigger classes.

  fSelectEventType = type;
  fSelectTriggerMask = triggerMask;
  if (triggerExpr) fSelectTriggerExpr = triggerExpr;
}

void IlcRawReader::LoadTriggerClass(const char* name, Int_t index)
{
  // Loads the list of trigger classes defined.
  // Used in conjunction with IsEventSelected in the
  // case when the trigger selection is given by
  // fSelectedTriggerExpr

  if (fSelectTriggerExpr.IsNull()) return;

  fIsTriggerClassLoaded = kTRUE;

  if (index >= 0)
    fSelectTriggerExpr.ReplaceAll(name,Form("[%d]",index));
  else
    fSelectTriggerExpr.ReplaceAll(name,"0");
}

void IlcRawReader::LoadTriggerAlias(const THashList *lst)
{
  // Loads the list of trigger aliases defined.
  // Replaces the alias by the OR of the triggers included in it.
  // The subsiquent call to LoadTriggerClass is needed
  // to obtain the final expression in
  // fSelectedTriggerExpr

  if (fSelectTriggerExpr.IsNull()) return;

  // Make a THashList alias -> trigger classes

  THashList alias2trig;
  TIter iter(lst);
  TNamed *nmd = 0;

  // Loop on triggers

  while((nmd = dynamic_cast<TNamed*>(iter.Next()))){

    TString aliasList(nmd->GetTitle());
    TObjArray* arrAliases = aliasList.Tokenize(',');
    Int_t nAliases = arrAliases->GetEntries();

    // Loop on aliases for the current trigger
    for(Int_t i=0; i<nAliases; i++){

      TObjString *alias = (TObjString*) arrAliases->At(i);

      // Find the current alias in the hash list. If it is not there, add TNamed entry
      TNamed * inlist = (TNamed*)alias2trig.FindObject((alias->GetString()).Data());
      if (!inlist) {
	inlist = new TNamed((alias->GetString()).Data(),nmd->GetName());
	alias2trig.Add(inlist);
      }
      else {
	TString tt(inlist->GetTitle());
	tt += " || ";
	tt += nmd->GetName();
	inlist->SetTitle(tt.Data());
      }
    }
    
    delete arrAliases;
  }
  alias2trig.Sort(kSortDescending);

  // Replace all the aliases by the OR of triggers
  TIter iter1(&alias2trig);
  while((nmd = dynamic_cast<TNamed*>(iter1.Next()))){
    fSelectTriggerExpr.ReplaceAll(nmd->GetName(),nmd->GetTitle());
  }
}

Bool_t IlcRawReader::IsSelected() const
{
// apply the selection (if any)

  if (fSkipInvalid && !IsValid()) return kFALSE;

  if (fSelectEquipmentType >= 0)
    if (GetEquipmentType() != fSelectEquipmentType) return kFALSE;

  Int_t equipmentId;
  if (fEquipmentIdsIn && fEquipmentIdsIn)
    equipmentId = GetMappedEquipmentId();
  else
    equipmentId = GetEquipmentId();

  if ((fSelectMinEquipmentId >= 0) && 
      (equipmentId < fSelectMinEquipmentId))
    return kFALSE;
  if ((fSelectMaxEquipmentId >= 0) && 
      (equipmentId > fSelectMaxEquipmentId))
    return kFALSE;

  return kTRUE;
}

Bool_t IlcRawReader::IsEventSelected() const
{
  // apply the event selection (if any)

  // First check the event type
  if (fSelectEventType >= 0) {
    if (GetType() != (UInt_t) fSelectEventType) return kFALSE;
  }

  // Then check the trigger pattern and compared it
  // to the required trigger mask
  if (fSelectTriggerMask != 0) {
    if ((GetClassMask() & fSelectTriggerMask) != fSelectTriggerMask) return kFALSE;
  }

  if (  fIsTriggerClassLoaded && !fSelectTriggerExpr.IsNull()) {
    TString expr(fSelectTriggerExpr);
    ULong64_t mask = GetClassMask();
    for(Int_t itrigger = 0; itrigger < 50; itrigger++) {
      if (mask & ((ULong64_t)1 << itrigger)) {
	expr.ReplaceAll(Form("[%d]",itrigger),"1");
      }
      else {
	expr.ReplaceAll(Form("[%d]",itrigger),"0");
      }
    }
    // Possibility to introduce downscaling
    TPRegexp("(%\\s*\\d+)").Substitute(expr,Form("&& !(%d$1)",GetEventIndex()),"g");
    Int_t error;
    if ((gROOT->ProcessLineFast(expr.Data(),&error) == 0) &&
	(error == TInterpreter::kNoError)) {
      return kFALSE;
    }
  }

  return kTRUE;
}

UInt_t IlcRawReader::SwapWord(UInt_t x) const
{
   // Swap the endianess of the integer value 'x'

   return (((x & 0x000000ffU) << 24) | ((x & 0x0000ff00U) <<  8) |
           ((x & 0x00ff0000U) >>  8) | ((x & 0xff000000U) >> 24));
}

UShort_t IlcRawReader::SwapShort(UShort_t x) const
{
   // Swap the endianess of the short value 'x'

   return (((x & 0x00ffU) <<  8) | ((x & 0xff00U) >>  8)) ;
}

Bool_t IlcRawReader::ReadNextInt(UInt_t& data)
{
// reads the next 4 bytes at the current position
// returns kFALSE if the data could not be read

  while (fCount == 0) {
    if (!ReadHeader()) return kFALSE;
  }
  if (fCount < (Int_t) sizeof(data)) {
    Error("ReadNextInt", 
	  "too few data left (%d bytes) to read an UInt_t!", fCount);
    return kFALSE;
  }
  if (!ReadNext((UChar_t*) &data, sizeof(data))) {
    Error("ReadNextInt", "could not read data!");
    return kFALSE;
  }
#ifndef R__BYTESWAP
  data=SwapWord(data);
#endif
  return kTRUE;
}

Bool_t IlcRawReader::ReadNextShort(UShort_t& data)
{
// reads the next 2 bytes at the current position
// returns kFALSE if the data could not be read

  while (fCount == 0) {
    if (!ReadHeader()) return kFALSE;
  }
  if (fCount < (Int_t) sizeof(data)) {
    Error("ReadNextShort", 
	  "too few data left (%d bytes) to read an UShort_t!", fCount);
    return kFALSE;
  }
  if (!ReadNext((UChar_t*) &data, sizeof(data))) {
    Error("ReadNextShort", "could not read data!");
    return kFALSE;
  }
#ifndef R__BYTESWAP
  data=SwapShort(data);
#endif
  return kTRUE;
}

Bool_t IlcRawReader::ReadNextChar(UChar_t& data)
{
// reads the next 1 byte at the current stream position
// returns kFALSE if the data could not be read

  while (fCount == 0) {
    if (!ReadHeader()) return kFALSE;
  }
  if (!ReadNext((UChar_t*) &data, sizeof(data))) {
    Error("ReadNextChar", "could not read data!");
    return kFALSE;
  }
  return kTRUE;
}

Bool_t  IlcRawReader::GotoEvent(Int_t event)
{
  // Random access to certain
  // event index. Could be very slow
  // for some non-root raw-readers.
  // So it should be reimplemented there.
  if (event < fEventNumber) RewindEvents();

  while (fEventNumber < event) {
    if (!NextEvent()) return kFALSE;
  }

  return kTRUE;
}

Int_t IlcRawReader::CheckData() const
{
// check the consistency of the data
// derived classes should overwrite the default method which returns 0 (no err)

  return 0;
}


void IlcRawReader::DumpData(Int_t limit)
{
// print the raw data
// if limit is not negative, only the first and last "limit" lines of raw data
// are printed

  Reset();
  if (!ReadHeader()) {
    Error("DumpData", "no header");
    return;
  }
  printf("header:\n"
	 " type = %d  run = %d  ", GetType(), GetRunNumber());
  if (GetEventId()) {
    printf("event = %8.8x %8.8x\n", GetEventId()[1], GetEventId()[0]);
  } else {
    printf("event = -------- --------\n");
  }
  if (GetTriggerPattern()) {
    printf(" trigger = %8.8x %8.8x  ",
	   GetTriggerPattern()[1], GetTriggerPattern()[0]);
  } else {
    printf(" trigger = -------- --------  ");
  }
  if (GetDetectorPattern()) {
    printf("detector = %8.8x\n", GetDetectorPattern()[0]);
  } else {
    printf("detector = --------\n");
  }
  if (GetAttributes()) {
    printf(" attributes = %8.8x %8.8x %8.8x  ",
	   GetAttributes()[2], GetAttributes()[1], GetAttributes()[0]);
  } else {
    printf(" attributes = -------- -------- --------  ");
  }
  printf("GDC = %d\n", GetGDCId());
  printf("\n");

  do {
    printf("-------------------------------------------------------------------------------\n");
    printf("LDC = %d\n", GetLDCId());

    printf("equipment:\n"
	   " size = %d  type = %d  id = %d\n",
	   GetEquipmentSize(), GetEquipmentType(), GetEquipmentId());
    if (GetEquipmentAttributes()) {
      printf(" attributes = %8.8x %8.8x %8.8x  ", GetEquipmentAttributes()[2],
	     GetEquipmentAttributes()[1], GetEquipmentAttributes()[0]);
    } else {
      printf(" attributes = -------- -------- --------  ");
    }
    printf("element size = %d\n", GetEquipmentElementSize());

    printf("data header:\n"
	   " size = %d  version = %d  valid = %d  compression = %d\n",
	   GetDataSize(), GetVersion(), IsValid(), IsCompressed());

    printf("\n");
    if (limit == 0) continue;

    Int_t size = GetDataSize();
    char line[70];
    for (Int_t i = 0; i < 70; i++) line[i] = ' ';
    line[69] = '\0';
    Int_t pos = 0;
    Int_t max = 16;
    UChar_t byte;

    for (Int_t n = 0; n < size; n++) {
      if (!ReadNextChar(byte)) {
	Error("DumpData", "couldn't read byte number %d\n", n);
	break;
      }
      if (pos >= max) {
	printf("%8.8x  %s\n", n-pos, line);
	for (Int_t i = 0; i < 70; i++) line[i] = ' ';
	line[69] = '\0';
	pos = 0;
	if ((limit > 0) && (n/max == limit)) {
	  Int_t nContinue = ((size-1)/max+1-limit) * max;
	  if (nContinue > n) {
	    printf(" [skipping %d bytes]\n", nContinue-n);
	    n = nContinue-1;
	    continue;
	  }
	}
      }
      Int_t offset = pos/4;
      if ((byte > 0x20) && (byte < 0x7f)) {
	line[pos+offset] = byte;
      } else {
	line[pos+offset] = '.';
      }
      char hex[3];
      snprintf(hex, 3, "%2.2x", byte);
      line[max+max/4+3+2*pos+offset] = hex[0];
      line[max+max/4+4+2*pos+offset] = hex[1];
      pos++;
    }

    if (pos > 0) printf("%8.8x  %s\n", size-pos, line);
    printf("\n");
	   
  } while (ReadHeader());
}

void IlcRawReader::AddErrorLog(IlcRawDataErrorLog::ERawDataErrorLevel level,
			       Int_t code,
			       const char *message)
{
  // Add a raw data error message to the list
  // of raw-data decoding errors
  if (fEventNumber < 0) {
    return;
  }
  Int_t ddlId = GetEquipmentId();
  if (ddlId < 0) {
    IlcError("No ddl raw data have been read so far! Impossible to add a raw data error log!");
    return;
  }

  Int_t prevEventNumber = -1;
  Int_t prevDdlId = -1;
  Int_t prevErrorCode = -1;
  IlcRawDataErrorLog *prevLog = (IlcRawDataErrorLog *)fErrorLogs.Last();
  if (prevLog) {
    prevEventNumber = prevLog->GetEventNumber();
    prevDdlId       = prevLog->GetDdlID();
    prevErrorCode   = prevLog->GetErrorCode();
  }

  if ((prevEventNumber != fEventNumber) ||
      (prevDdlId != ddlId) ||
      (prevErrorCode != code)) {
    new (fErrorLogs[fErrorLogs.GetEntriesFast()])
      IlcRawDataErrorLog(fEventNumber,
			 ddlId,
			 level,
			 code,
			 message);
  }
  else
    if (prevLog) prevLog->AddCount();

}

Bool_t IlcRawReader::GotoEventWithID(Int_t event, 
				     UInt_t period,
				     UInt_t orbitID,
				     UShort_t bcID)
{
  // Go to certain event number by
  // checking the event ID.
  // Useful in case event-selection
  // is applied and the 'event' is
  // relative
  if (!GotoEvent(event)) return kFALSE;

  while (GetBCID()    != period  ||
	 GetOrbitID() != orbitID ||
	 GetPeriod()  != bcID) {
    if (!NextEvent()) return kFALSE;
  }

  return kTRUE;
}

