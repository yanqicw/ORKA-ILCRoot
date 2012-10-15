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

/* $Id: IlcRawEventHeaderBase.cxx 46858 2011-01-31 10:26:27Z cvetan $ */

// Author: Cvetan Cheshkov  10/10/2005

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// IlcRawEventHeaderBase                                                //
// This a new versioning scheme for raw data root-ification and reading //
// For details look at offline weekly meeting 20/10/2005                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <unistd.h>

#include <Bytes.h>
#include <TClass.h>
#include <TDataMember.h>
#include <TList.h>
#include <TMethodCall.h>

#include "IlcLog.h"
#include "IlcRawEventHeaderBase.h"

#include <Riostream.h>

ClassImp(IlcRawEventHeaderBase)

//______________________________________________________________________________
IlcRawEventHeaderBase::IlcRawEventHeaderBase():
fSize(0),
fMagic(0),
fHeadSize(0),
fVersion(0),
fExtendedDataSize(0),
fExtendedAllocSize(0),
fExtendedData(NULL),
fIsSwapped(kFALSE),
fHeaderSize(0),
fHeaderBegin(NULL),
fFirstEqIndex(-1),
fLastEqIndex(-1)
{
  // Default constructor
}

//______________________________________________________________________________
void *IlcRawEventHeaderBase::HeaderBegin() const
{
  // Returns the pointer to the first data member
  // beyond the base class data members

  if (fHeaderBegin) return fHeaderBegin;

  TList *datalist = IsA()->GetListOfDataMembers();
  TIter next(datalist);                           
  TDataMember *member = (TDataMember *)next();

  if(!strcmp(member->GetTypeName(),"TClass"))
    member = (TDataMember *)next();

  void *ptr = (void *)((char *)this+member->GetOffset());
  const_cast<IlcRawEventHeaderBase*>(this)->fHeaderBegin = ptr;

  return ptr;
}

//______________________________________________________________________________
Int_t IlcRawEventHeaderBase::HeaderSize() const
{
  // Returns the size of the data members list
  // beyond the base class data members

  if (fHeaderSize) return fHeaderSize;

  Int_t size = 0;

  TList *datalist = IsA()->GetListOfDataMembers();
  TIter next(datalist);                           
  TDataMember *member;
  while ((member=(TDataMember *)next()) != 0x0) {
    if (!strcmp(member->GetTypeName(),"TClass")) continue;
    UInt_t unitsize = member->GetUnitSize();
    UInt_t ndim = member->GetArrayDim();
    if (ndim == 0)
      size += unitsize;
    else
      for(UInt_t i=0;i<ndim;i++) size += member->GetMaxIndex(i)*unitsize;
  }

  const_cast<IlcRawEventHeaderBase*>(this)->fHeaderSize = size;

  return size;
}

//______________________________________________________________________________
UInt_t IlcRawEventHeaderBase::SwapWord(UInt_t x) const
{
   // Swap the endianess of the integer value 'x'

   return (((x & 0x000000ffU) << 24) | ((x & 0x0000ff00U) <<  8) |
           ((x & 0x00ff0000U) >>  8) | ((x & 0xff000000U) >> 24));
}

void IlcRawEventHeaderBase::Swap()
{
   // Swap base header data.
   // Update the fIsSwapped flag which
   // is then use to copy in an appropriate way
   // the rest of the header data from the raw data stream

   if (IsSwapped()) {
      fIsSwapped    = kTRUE;
      fSize         = SwapWord(fSize);
      fMagic        = SwapWord(fMagic);
      fHeadSize     = SwapWord(fHeadSize);
      fVersion      = SwapWord(fVersion);
   }
}

//______________________________________________________________________________
const char *IlcRawEventHeaderBase::GetTypeName() const
{
   // Get event type as a string.
   // Will fail in case data header
   // does not contain eventType field
   UInt_t eventType = Get("Type");

   return GetTypeName(eventType);
}

//______________________________________________________________________________
const char *IlcRawEventHeaderBase::GetTypeName(UInt_t eventType)
{
  // Get event type as a string.
  // Static method that could be used
  // from everywhere inside ilcroot

   switch (eventType) {
      case kStartOfRun:
         return "START_OF_RUN";
         break;
      case kEndOfRun:
         return "END_OF_RUN";
         break;
      case kStartOfRunFiles:
         return "START_OF_RUN_FILES";
         break;
      case kEndOfRunFiles:
         return "END_OF_RUN_FILES";
         break;
      case kStartOfBurst:
         return "START_OF_BURST";
         break;
      case kEndOfBurst:
         return "END_OF_BURST";
         break;
      case kPhysicsEvent:
         return "PHYSICS_EVENT";
         break;
      case kCalibrationEvent:
         return "CALIBRATION_EVENT";
         break;
      case kFormatError:
         return "EVENT_FORMAT_ERROR";
         break;
      case kStartOfData:
	 return "START_OF_DATA";
	 break;
      case kEndOfData:
	 return "END_OF_DATA";
	 break;
      case kSystemSoftwareTriggerEvent:
	 return "SYSTEM_SOFTWARE_TRIGGER_EVENT";
	 break;
      case kDetectorSoftwareTriggerEvent:
	 return "DETECTOR_SOFTWARE_TRIGGER_EVENT";
	 break;
      default:
	 return "UNKNOWN EVENT TYPE NUMBER";
         break;
   }
}

//______________________________________________________________________________
IlcRawEventHeaderBase* IlcRawEventHeaderBase::Create(char*& data)
{
  // Static method to create IlcRawEventHeaderVX object
  // The actual header class version is taken from the
  // raw data

  // First create IlcrawEVentHeaderBase class object
  IlcRawEventHeaderBase header;

  // Copy the first common part of the raw data header
  memcpy(header.HeaderBaseBegin(), data, header.HeaderBaseSize());
 
    // Swap header data if needed
  if (header.IsSwapped())
    header.Swap();

  // Is header valid...
  if (!header.IsValid()) {
    IlcFatalClass("Invalid header format!");
    // try recovery... how?
    return 0x0;
  }

  if (header.GetEventSize() < (UInt_t)header.HeaderBaseSize()) {
    IlcFatalClass("Invalid header base size!");
    // try recovery... how?
    return 0x0;
  }

  // Now check the DATE version and create the corresponding header
  // class object
  UInt_t version = header.GetVersion();
  UInt_t majorversion = (version>>16)&0x0000ffff;
  UInt_t minorversion = version&0x0000ffff;
  TString classname;
  classname.Form("IlcRawEventHeaderV%d_%d",majorversion,minorversion);
    
  TClass *tcl = TClass::GetClass(classname.Data());
  if (!tcl) {
    IlcFatalClass(Form("Unknown header version (%s)!",classname.Data()));
    return 0x0;
  }

  //  header.Dump(); tcl->Dump();

  IlcRawEventHeaderBase *hdr = (IlcRawEventHeaderBase *)tcl->New();
  if (!hdr) {
    IlcFatalClass(Form("Can not create object of class %s",classname.Data()));
    return 0x0;
  }

  // Copy the base header data members and initialize other data members
  memcpy(hdr->HeaderBaseBegin(),header.HeaderBaseBegin(), header.HeaderBaseSize());
  memset(hdr->HeaderBegin(),0, hdr->HeaderSize());
  hdr->fIsSwapped = header.fIsSwapped;

  // Consistency check
  if (hdr->GetEventSize() < ((UInt_t)hdr->HeaderBaseSize() + (UInt_t)hdr->HeaderSize())) {
    IlcFatalClass(Form("Invalid header size (%d < %d +%d)!",
		       hdr->GetEventSize(),hdr->HeaderBaseSize(),hdr->HeaderSize()));
    // try recovery... how?
    return 0x0;
  }

  // Check for the presence of header extension and its size
  Int_t extsize = (Int_t)hdr->GetHeadSize() - (hdr->HeaderBaseSize() + hdr->HeaderSize());
  if (extsize < 0) {
    IlcFatalClass(Form("Invalid header size (%d < %d +%d)!",
		       hdr->GetHeadSize(),hdr->HeaderBaseSize(),hdr->HeaderSize()));
    // try recovery... how?
    return 0x0;
  }
  else {
    if (extsize > 0) {
      hdr->AllocateExtendedData(extsize);
    }
  }

  return hdr;
}

void IlcRawEventHeaderBase::SwapData(const void* inbuf, const void* outbuf, UInt_t size) {
  // The method swaps the contents of the
  // raw-data event header
  UInt_t  intCount = size/sizeof(UInt_t);

  UInt_t* buf = (UInt_t*) inbuf;    // temporary integers buffer
  for (UInt_t i=0; i<intCount; i++, buf++) {
      UInt_t value = SwapWord(*buf);
      memcpy((UInt_t*)outbuf+i, &value, sizeof(UInt_t)); 
  }
}

//______________________________________________________________________________
Int_t IlcRawEventHeaderBase::ReadHeader(char*& data)
{
  // Read header info from DATE data stream.
  // Returns bytes read

  Long_t start = (Long_t)data;
  // Swap header data if needed
  if (DataIsSwapped()) {
    SwapData(data, HeaderBaseBegin(), HeaderBaseSize());
    data += HeaderBaseSize();
    SwapData(data, HeaderBegin(), HeaderSize());
    data += HeaderSize();
  }
  else {
    memcpy(HeaderBaseBegin(), data, HeaderBaseSize());
    data += HeaderBaseSize();
    memcpy(HeaderBegin(), data, HeaderSize());
    data += HeaderSize();
  }
  data += ReadExtendedData(data);

  return (Int_t)((Long_t)data - start);
}

//______________________________________________________________________________
void IlcRawEventHeaderBase::AllocateExtendedData(Int_t extsize)
{
  // Allocate the space for the header
  // extended data
  if (fExtendedData) delete [] fExtendedData;
  
  fExtendedDataSize = fExtendedAllocSize = extsize;
  fExtendedData = new char[fExtendedAllocSize];
  memset(fExtendedData,0,fExtendedAllocSize);
}

//______________________________________________________________________________
Int_t IlcRawEventHeaderBase::ReadExtendedData(char*& data)
{
  // Read extended header data
  // Reallocates memory if the present
  // buffer is insufficient
  Int_t extsize = (Int_t)GetHeadSize() - (HeaderBaseSize() + HeaderSize());

  if (extsize == 0) {
    fExtendedDataSize = 0;
    return 0;
  }

  if (extsize < 0) {
    IlcFatal(Form("Invalid header size (%d < %d +%d)!",
		  GetHeadSize(),HeaderBaseSize(),HeaderSize()));
    // try recovery... how?
    return 0;
  }

  fExtendedDataSize = extsize;
  if (fExtendedDataSize > fExtendedAllocSize)
    AllocateExtendedData(fExtendedDataSize);

  if (DataIsSwapped())
    SwapData(data, fExtendedData, fExtendedDataSize);
  else
    memcpy(fExtendedData, data, fExtendedDataSize);

  return fExtendedDataSize;
}

//______________________________________________________________________________
UInt_t IlcRawEventHeaderBase::Get(const char *datamember) const
{
  // The method to get a data member from the header object
  // Except for the data members of the base class, all the
  // other header data should be retrieved ONLY by this method
  // The name of the data member should be supplied without "f"
  // in front

  char buf[256] = "f";
  strncat(buf,datamember,sizeof(buf)-2);

  TDataMember *member = IsA()->GetDataMember(buf);
  if (!member) {
    IlcFatal(Form("No data member %s is found! Check the raw data version!",buf));
    return 0;
  }

  if (member->GetArrayDim() != 0) {
    IlcFatal(Form("Member %s is an array! Use the GetP() method!",buf));
    return 0;
  }

  if (strcmp(member->GetTypeName(),"UInt_t") != 0) {
    IlcFatal(Form("Member %s is not of type UInt_t!",buf));
    return 0;
  }

  const void *pointer = (char *)this+member->GetOffset();

  return *((UInt_t *)pointer);
}

//______________________________________________________________________________
const UInt_t* IlcRawEventHeaderBase::GetP(const char *datamember) const
{
  // The method to get a data member from the header object
  // Except for the data members of the base class, all the
  // other header data should be retrieved ONLY by this method
  // The name of the data member should be supplied without "f"
  // in front

  char buf[256] = "f";
  strncat(buf,datamember,sizeof(buf)-2);

  TDataMember *member = IsA()->GetDataMember(buf);
  if (!member) {
    IlcFatal(Form("No data member %s is found! Check the raw data version!",buf));
    return 0;
  }

  //  if (member->GetArrayDim() == 0) {
  //    IlcFatal(Form("Member %s is not an array! Use the Get() method!",buf));
  //    return 0;
  //  }

  if (strcmp(member->GetTypeName(),"UInt_t") != 0) {
    IlcFatal(Form("Member %s is not of type UInt_t*!",buf));
    return 0;
  }

  const void *pointer = (char *)this+member->GetOffset();

  return (const UInt_t*)pointer;
}

//_____________________________________________________________________________
void IlcRawEventHeaderBase::Print( const Option_t* opt ) const
{
  // Dumps the event or sub-event
  // header fields

  cout << opt << "  Event size: " << GetEventSize() << endl;
  cout << opt << "  Event header size: " << GetHeadSize() << endl;
  cout << opt << "  Event header version: " << GetMajorVersion() << "." << GetMinorVersion() << endl;
  cout << opt << "  Event type: " << Get("Type") << "( " << GetTypeName() << " )" << endl;
  cout << opt << "  Run Number: " << Get("RunNb") << endl;
  const UInt_t *id = GetP("Id");
  cout << opt << "  Period: " << (((id)[0]>>4)&0x0fffffff) << " Orbit: " << ((((id)[0]<<20)&0xf00000)|(((id)[1]>>12)&0xfffff)) << " Bunch-crossing: " << ((id)[1]&0x00000fff) << endl;
  cout << opt << "  Trigger pattern: " << GetP("TriggerPattern")[0] << "-" << GetP("TriggerPattern")[1] << endl;
  cout << opt << "  Detector pattern: " << Get("DetectorPattern") << endl;
  cout << opt << "  Type attribute: " << GetP("TypeAttribute")[0] << "-" << GetP("TypeAttribute")[1] << "-" << GetP("TypeAttribute")[2] << endl;
  cout << opt << "  GDC: " << Get("GdcId") << " LDC: " << Get("LdcId") << endl;
}

//_____________________________________________________________________________
void IlcRawEventHeaderBase::AddEqIndex(Int_t index)
{
  // Adds an equipment by changing properly
  // the first and last equipment indexes
  if (fFirstEqIndex < 0) fFirstEqIndex = index; 
  if (index > fLastEqIndex) fLastEqIndex = index;
}

//_____________________________________________________________________________
void IlcRawEventHeaderBase::Reset()
{
  fFirstEqIndex = fLastEqIndex = -1;
}

//______________________________________________________________________________
void IlcRawEventHeaderBase::Streamer(TBuffer &R__b)
{
   // Stream an object of class IlcRawEventHeaderBase.

   if (R__b.IsReading()) {
      UInt_t R__s, R__c;
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v > 3) {
	R__b.ReadClassBuffer(IlcRawEventHeaderBase::Class(),this,R__v,R__s,R__c);
	return;
      }
      TObject::Streamer(R__b);
      R__b >> fSize;
      R__b >> fMagic;
      R__b >> fHeadSize;
      R__b >> fVersion;
      R__b >> fExtendedDataSize;
      delete [] fExtendedData;
      fExtendedData = new char[fExtendedDataSize];
      R__b.ReadFastArray(fExtendedData,fExtendedDataSize);
      R__b >> fIsSwapped;
      R__b.CheckByteCount(R__s, R__c, IlcRawEventHeaderBase::IsA());
   } else {
      R__b.WriteClassBuffer(IlcRawEventHeaderBase::Class(),this);
   }
}
