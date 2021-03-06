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

///////////////////////////////////////////////////////////////////////////////
///
/// This class provides access to CTP DDL raw data.
///
/// The raw data format is taken form the trigger TDR.
/// The meaning of the trigger class and cluster masks
/// are given in the trigger description file (in /data)
/// and in the IlcCentralTrigger class.
///
///////////////////////////////////////////////////////////////////////////////

#include "IlcCTPRawStream.h"
#include "IlcRawReader.h"
#include "IlcLog.h"
#include "IlcDAQ.h"
#include "IlcTriggerIR.h"

ClassImp(IlcCTPRawStream)

//_____________________________________________________________________________
IlcCTPRawStream::IlcCTPRawStream(IlcRawReader* rawReader) :
  fIRArray("IlcTriggerIR",3),
  fOrbit(0),
  fBC(0),
  fL0TriggerInputs(0),
  fL1TriggerInputs(0),
  fL2TriggerInputs(0),
  fClassMask(0),
  fClusterMask(0),
  fRawReader(rawReader)
{
  // create an object to read CTP raw data
  //
  // select the raw data corresponding to
  // the CTP detector id
  fRawReader->Reset();
  IlcDebug(1,Form("Selecting raw data for detector %d",IlcDAQ::DetectorID("TRG")));
  fRawReader->Select("TRG");
}

//_____________________________________________________________________________
IlcCTPRawStream::IlcCTPRawStream(const IlcCTPRawStream& stream) :
  TObject(stream),
  fIRArray("IlcTriggerIR",3),
  fOrbit(0),
  fBC(0),
  fL0TriggerInputs(0),
  fL1TriggerInputs(0),
  fL2TriggerInputs(0),
  fClassMask(0),
  fClusterMask(0),
  fRawReader(NULL)
{
  // Copy constructor
  IlcFatal("Copy constructor not implemented");
}

//_____________________________________________________________________________
IlcCTPRawStream& IlcCTPRawStream::operator = (const IlcCTPRawStream& 
					      /* stream */)
{
  IlcFatal("Assignment operator not implemented");
  return *this;
}

//_____________________________________________________________________________
IlcCTPRawStream::~IlcCTPRawStream()
{
  // destructor
  fIRArray.Delete();
}

//_____________________________________________________________________________
void IlcCTPRawStream::Reset()
{
  // reset raw stream params
  fIRArray.Clear();

  fClassMask = fClusterMask = 0;

  if (fRawReader) fRawReader->Reset();
}

//_____________________________________________________________________________
Bool_t IlcCTPRawStream::Next()
{
  // read the whole CTP raw data stream
  // return kFALSE in case of error

  UChar_t *data = NULL;

  // CTP raw data does not contain CDH
  fRawReader->RequireHeader(kFALSE);

  if (!fRawReader->ReadNextData(data)) {
    fRawReader->RequireHeader(kTRUE);
    return kFALSE;
  }

  if ((fRawReader->GetDataSize()) < 32 ||
      ((fRawReader->GetDataSize() % 4) != 0)) {
    IlcError(Form("Wrong CTP raw data size: %d",fRawReader->GetDataSize()));
    fRawReader->RequireHeader(kTRUE);
    return kFALSE;
  }

  fBC = data[0];
  fBC |= (data[1] & 0xF) << 8;

  fOrbit = data[4] << 12;
  fOrbit |= (data[5] & 0xF) << 20;
  fOrbit |= data[8];
  fOrbit |= (data[9] & 0xF) << 8;

  fClusterMask = data[12] >> 2;

  fClassMask =  ((ULong64_t)data[12] & 0x3) << 48;

  fClassMask |= (ULong64_t)data[16] << 36;
  fClassMask |= ((ULong64_t)data[17] & 0xF) << 44;

  fClassMask |= (ULong64_t)data[20] << 24;
  fClassMask |= ((ULong64_t)data[21] & 0xF) << 32;

  fClassMask |= (ULong64_t)data[24] << 12;
  fClassMask |= ((ULong64_t)data[25] & 0xF) << 20;

  fClassMask |= (ULong64_t)data[28];
  fClassMask |= ((ULong64_t)data[29] & 0xF) << 8;

  if (fRawReader->GetDataSize() == 32) {
    IlcDebug(1,"No trigger input and interaction records found");
    fRawReader->RequireHeader(kTRUE);
    return kTRUE;
  }

  // Read detector trigger inputs
  if (fRawReader->GetDataSize() < 52) {
    IlcError(Form("Wrong CTP raw data size: %d",fRawReader->GetDataSize()));
    fRawReader->RequireHeader(kTRUE);
    return kFALSE;
  }

  fL0TriggerInputs = data[32] << 12;
  fL0TriggerInputs |= (data[33] & 0xF) << 20;
  fL0TriggerInputs |= data[36];
  fL0TriggerInputs |= (data[37] & 0xF) << 8;

  fL1TriggerInputs = data[40] << 12;
  fL1TriggerInputs |= (data[41] & 0xF) << 20;
  fL1TriggerInputs |= data[44];
  fL1TriggerInputs |= (data[45] & 0xF) << 8;

  fL2TriggerInputs = data[48] << 12;
  fL2TriggerInputs |= (data[49] & 0xF) << 20;

  if (fRawReader->GetDataSize() == 52) {
    IlcDebug(1,"No interaction records found");
    fRawReader->RequireHeader(kTRUE);
    return kTRUE;
  }

  // Read IRs
  Int_t iword = 52;
  UChar_t level = 0;
  UInt_t *irdata = NULL;
  UInt_t irsize = 0;
  UInt_t orbit = 0;
  Bool_t incomplete = kFALSE, transerr = kFALSE;
  while (iword < fRawReader->GetDataSize()) {
    if (data[iword+1] & 0x80) {
      UChar_t flag = ((data[iword+1] >> 4) & 0x3);
      if (flag == 0) {
	if (irdata) {
	  new (fIRArray[fIRArray.GetEntriesFast()])
	    IlcTriggerIR(orbit,irsize,irdata,incomplete,transerr);
	  irdata = NULL; irsize = 0;
	}
	level = 1;
	orbit = data[iword] << 12;
	orbit |= (data[iword+1] & 0xF) << 20;
	transerr = ((data[iword+1] >> 6) & 0x1);
	iword += 4;
	continue;
      }
      else if (flag == 3) {
	if (level == 1) {
	  level = 2;
	  orbit |= data[iword];
	  orbit |= ((data[iword+1] & 0xF) << 8);
	  iword += 4;
	  IlcDebug(1,Form("Orbit=0x%x\n",orbit));
	  continue;
	}
      }
      UShort_t bc = data[iword];
      bc |= ((data[iword+1] & 0xF) << 8);
      IlcDebug(1,Form("BC=0x%x\n",bc));
      if (bc == 0xFFF) {
	incomplete = kTRUE;
      }
      else {
	if (level == 2) {
	  level = 3;
	  irdata = (UInt_t *)&data[iword];
	  irsize = 0;
	}
	if (level == 3) {
	  irsize++;
	}
      }
    }
    else
      IlcWarning(Form("Invalid interaction record (%d %d)",iword,fRawReader->GetDataSize()));

    iword += 4;
  }

  if (irdata) {
    new (fIRArray[fIRArray.GetEntriesFast()])
      IlcTriggerIR(orbit,irsize,irdata,incomplete,transerr);
    irdata = NULL; irsize = 0;
  }

  // Restore the raw-reader state!!
  fRawReader->RequireHeader(kTRUE);

  return kTRUE;
}

