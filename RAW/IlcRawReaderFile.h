#ifndef ILCRAWREADERFILE_H
#define ILCRAWREADERFILE_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* $Id: IlcRawReaderFile.h 27624 2008-07-28 10:05:40Z cvetan $ */

///////////////////////////////////////////////////////////////////////////////
///
/// This is a class for reading raw data files.
///
///////////////////////////////////////////////////////////////////////////////

#include "IlcRawReader.h"
#ifdef __CINT__
class fstream;
#else
#include <Riostream.h>
#endif
#include <TString.h>

class TArrayC;

class IlcRawReaderFile: public IlcRawReader {
  public :
    IlcRawReaderFile(Int_t eventNumber = -1);
    IlcRawReaderFile(const char* dirName, Int_t eventNumber = -1);
    virtual ~IlcRawReaderFile();

    virtual void     RequireHeader(Bool_t required);

    virtual UInt_t   GetType() const {return fType;};
    virtual UInt_t   GetRunNumber() const {return fRunNb;};
    virtual const UInt_t* GetEventId() const {return fId;};
    virtual const UInt_t* GetTriggerPattern() const {return fTriggerPattern;};
    virtual const UInt_t* GetDetectorPattern() const {return &fDetectorPattern;};
    virtual const UInt_t* GetAttributes() const {return 0;};
    virtual const UInt_t* GetSubEventAttributes() const {return 0;};
    virtual UInt_t   GetLDCId() const {return 0;};
    virtual UInt_t   GetGDCId() const {return 0;};
    virtual UInt_t   GetTimestamp() const {return fTimestamp;};

    virtual Int_t    GetEquipmentSize() const {return fEquipmentSize;};
    virtual Int_t    GetEquipmentType() const {return 0;};
    virtual Int_t    GetEquipmentId() const {return fEquipmentId;};
    virtual const UInt_t* GetEquipmentAttributes() const {return NULL;};
    virtual Int_t    GetEquipmentElementSize() const {return 0;};
    virtual Int_t    GetEquipmentHeaderSize() const {return 0;};

    virtual Bool_t   ReadHeader();
    virtual Bool_t   ReadNextData(UChar_t*& data);
    virtual Bool_t   ReadNext(UChar_t* data, Int_t size);

    virtual Bool_t   Reset();

    virtual Bool_t   NextEvent();
    virtual Bool_t   RewindEvents();

    void             SetEventType(UInt_t type) { fType = type; }
    void             SetRunNb(UInt_t run) { fRunNb = run; }
    void             SetDetectorPattern(UInt_t pattern) { fDetectorPattern = pattern; }
 
  protected :
    TString          GetDirName() const;
    void*            OpenDirectory();
    Bool_t           OpenNextFile();
    Bool_t           CreateFileIndex();

    Int_t            fEventIndex;  // index of the event
    TString          fDirName;     // name of the input directory
    void*            fDirectory;   // pointer to the input directory
    fstream*         fStream;      // stream of raw digits
    Int_t            fEquipmentId; // equipment ID from file name
    UChar_t*         fBuffer;      // buffer for payload
    Int_t            fBufferSize;  // size of fBuffer in bytes
    Int_t            fEquipmentSize; // equipment size from raw-data payload
    TArrayC*         fDDLIndex;    //! the index of DDL files
    Int_t            fDDLCurrent;  //! the index of DDL files

    UInt_t           fType;        // event type (no idea from where to get it - put physics_event)
    UInt_t           fRunNb;       // run number (no idea from where to get it - put 0)
    UInt_t           fId[2];       // id field (read from the first CDH found, period is not filled)
    UInt_t           fTriggerPattern[2]; // the trigger class pattern (read from the first CDH found)
    UInt_t           fDetectorPattern;   // the detector pattern (no idea from where to read it)
    UInt_t           fTimestamp;   // event timestamp (read from the DDL file creation time)

  private :
    IlcRawReaderFile(const IlcRawReaderFile& rawReader);
    IlcRawReaderFile& operator = (const IlcRawReaderFile& rawReader);

    ClassDef(IlcRawReaderFile, 0) // class for reading raw digits from a file
};

#endif
