/**************************************************************************
 * Copyright(c) 2005-2006, ILC Project Experiment, All rights reserved.   *
 *                                                                        *
// Author: The ILC Off-line Project. 
 // Part of the code has been developed by Alice Off-line Project. 
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

/* $Id: IlcDCHcalibDB.cxx,v 1.1.1.1 2008/03/11 14:53:01 vitomeg Exp $ */

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Class providing the calibration parameters by accessing the CDB           //
//                                                                           //
// Request an instance with IlcDCHcalibDB::Instance()                 //
// If a new event is processed set the event number with SetRun              //
// Then request the calibration data                                         // 
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <TRandom.h>

#include <IlcCDBManager.h>

#include "IlcDCHcalibDB.h"
#include "IlcDCHgeometry.h"
#include "IlcDCHpadPlane.h"
#include "IlcDCHCommonParam.h"

#include "Cal/IlcDCHCalROC.h"

#include "Cal/IlcDCHCalChamberPos.h"
#include "Cal/IlcDCHCalStackPos.h"
#include "Cal/IlcDCHCalSuperModulePos.h"

#include "Cal/IlcDCHCalPad.h"
#include "Cal/IlcDCHCalDet.h"
#include "Cal/IlcDCHCalGlobals.h"
#include "Cal/IlcDCHCalPIDLQ.h"
#include "Cal/IlcDCHCalMonitoring.h"

#include "Cal/IlcDCHCalSuperModuleStatus.h"
#include "Cal/IlcDCHCalChamberStatus.h"
#include "Cal/IlcDCHCalMCMStatus.h"
#include "Cal/IlcDCHCalPadStatus.h"
#include "Cal/IlcDCHCalSingleChamberStatus.h"

ClassImp(IlcDCHcalibDB)

IlcDCHcalibDB* IlcDCHcalibDB::fgInstance = 0;
Bool_t IlcDCHcalibDB::fgTerminated = kFALSE;

//_ singleton implementation __________________________________________________
IlcDCHcalibDB* IlcDCHcalibDB::Instance()
{
  //
  // Singleton implementation
  // Returns an instance of this class, it is created if neccessary
  //
  
  if (fgTerminated != kFALSE)
    return 0;

  if (fgInstance == 0)
    fgInstance = new IlcDCHcalibDB();

  return fgInstance;
}

void IlcDCHcalibDB::Terminate()
{
  //
  // Singleton implementation
  // Deletes the instance of this class and sets the terminated flag, instances cannot be requested anymore
  // This function can be called several times.
  //
  
  fgTerminated = kTRUE;
  
  if (fgInstance != 0)
  {
    delete fgInstance;
    fgInstance = 0;
  }
}

//_____________________________________________________________________________
IlcDCHcalibDB::IlcDCHcalibDB()
{
  //
  // constructor
  //
  
  // TODO Default runnumber is set to 0, this should be changed later to an invilcd value (e.g. -1) to prevent
  // TODO invilcd calibration data to be used.
  fRun = -1;
  
  fPadResponse.fPRFbin             = 0;
  fPadResponse.fPRFlo              = 0.0;
  fPadResponse.fPRFhi              = 0.0;
  fPadResponse.fPRFwid             = 0.0;
  fPadResponse.fPRFpad             = 0;
  fPadResponse.fPRFsmp             = 0;

  for (Int_t i=0; i<kCDBCacheSize; ++i)
  {
    fCDBCache[i] = 0;
    fCDBEntries[i] = 0;
  }
  
  // Create the sampled PRF
  SamplePRF();
}

//_____________________________________________________________________________
IlcDCHcalibDB::~IlcDCHcalibDB() 
{
  //
  // destructor
  //
  
  if (fPadResponse.fPRFsmp) {
    delete [] fPadResponse.fPRFsmp;
    fPadResponse.fPRFsmp = 0;
  }

  Invilcdate();
}

//_caching functions____________________________________________________________
const TObject* IlcDCHcalibDB::GetCachedCDBObject(Int_t id)
{
    //
    // Retrieves a cdb object with the given id. The objects are cached as long as the run number is not changed.
    //
    // Put together the available objects here by using the lines
    //   a) For usual calibration objects:
    //      ase kID<Name> : return CacheCDBEntry(kID<Name>, "DCH/Calib/<Path>"); break;
    //      See function CacheCDBEntry for details.
    //   and
    //   b) For calibration data which depends on two objects: One containing a value per detector and one the local fluctuations per pad:
    //      case kID<Name> : return CacheMergeCDBEntry(kID<Name>, "DCH/Calib/<padPath>", "DCH/Calib/<chamberPath>"); break;
    //      See function CacheMergeCDBEntry for details.
    //
    
    switch (id)
    {
      // parameters defined per pad and chamber
      case kIDVdriftPad : return CacheCDBEntry(kIDVdriftPad, "DCH/Calib/LocalVdrift"); break;
      case kIDVdriftChamber : return CacheCDBEntry(kIDVdriftChamber, "DCH/Calib/ChamberVdrift"); break;

      case kIDT0Pad : return CacheCDBEntry(kIDT0Pad, "DCH/Calib/LocalT0"); break;
      case kIDT0Chamber : return CacheCDBEntry(kIDT0Chamber, "DCH/Calib/ChamberT0"); break;

      case kIDGainFactorPad : return CacheCDBEntry(kIDGainFactorPad, "DCH/Calib/LocalGainFactor"); break;
      case kIDGainFactorChamber : return CacheCDBEntry(kIDGainFactorChamber, "DCH/Calib/ChamberGainFactor"); break;

      // parameters defined per pad
      case kIDPRFWidth : return CacheCDBEntry(kIDPRFWidth, "DCH/Calib/PRFWidth"); break;

      // status values
      case kIDSuperModuleStatus : return CacheCDBEntry(kIDSuperModuleStatus, "DCH/Calib/SuperModuleStatus"); break;
      case kIDChamberStatus : return CacheCDBEntry(kIDChamberStatus, "DCH/Calib/ChamberStatus"); break;
      case kIDMCMStatus : return CacheCDBEntry(kIDMCMStatus, "DCH/Calib/MCMStatus"); break;
      case kIDPadStatus : return CacheCDBEntry(kIDPadStatus, "DCH/Calib/PadStatus"); break;

      // global parameters
      case kIDMonitoringData : return CacheCDBEntry(kIDMonitoringData, "DCH/Calib/MonitoringData"); break;
      case kIDGlobals : return CacheCDBEntry(kIDGlobals, "DCH/Calib/Globals"); break;
      case kIDSuperModulePos : return CacheCDBEntry(kIDSuperModulePos, "DCH/Calib/SuperModulePos"); break;
      case kIDChamberPos : return CacheCDBEntry(kIDChamberPos, "DCH/Calib/ChamberPos"); break;
      case kIDStackPos : return CacheCDBEntry(kIDStackPos, "DCH/Calib/StackPos"); break;
      case kIDPIDLQ : return CacheCDBEntry(kIDPIDLQ, "DCH/Calib/PIDLQ"); break;
    }
    return 0;
}

//_____________________________________________________________________________
IlcCDBEntry* IlcDCHcalibDB::GetCDBEntry(const char* cdbPath)
{
  // 
  // Retrieves an entry with path <cdbPath> from the CDB.
  //
    
  IlcCDBEntry* entry = IlcCDBManager::Instance()->Get(cdbPath, fRun);
  if (!entry)
  { 
    std::cerr << "IlcDCHcalibDB: Failed to get entry: " << cdbPath << std::endl; 
    return 0; 
  }
  
  std::cout << "IlcDCHcalibDB: Retrieved object: " << cdbPath << std::endl;
  return entry;
}

//_____________________________________________________________________________
const TObject* IlcDCHcalibDB::CacheCDBEntry(Int_t id, const char* cdbPath)
{
  //
  // Caches the entry <id> with cdb path <cdbPath>
  //
  
  if (!fCDBCache[id])
  {
    fCDBEntries[id] = GetCDBEntry(cdbPath);
    if (fCDBEntries[id])
      fCDBCache[id] = fCDBEntries[id]->GetObject();
  }
  return fCDBCache[id];
}

//_____________________________________________________________________________
void IlcDCHcalibDB::SetRun(Long64_t run)
{
  //
  // Sets current run number. Calibration data is read from the corresponding file.
  // When the run number changes the caching is invilcdated.
  //

  if (fRun == run)
    return;

  fRun = run;
  Invilcdate();
}

//_____________________________________________________________________________
void IlcDCHcalibDB::Invilcdate()
{
  //
  // Invilcdates cache (when run number is changed).
  //
  
  for (Int_t i=0; i<kCDBCacheSize; ++i)
  {
    if (fCDBEntries[i])
    {
      if (IlcCDBManager::Instance()->GetCacheFlag() == kFALSE)
      {
        if (fCDBEntries[i]->IsOwner() == kFALSE && fCDBCache[i])
          delete fCDBCache[i];

        delete fCDBEntries[i];
      }
      fCDBEntries[i] = 0;
      fCDBCache[i] = 0;
    }
  }
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::GetChamberPos(Int_t det, Float_t* xyz)
{
  //
  // Returns the deviation of the chamber position from the nominal position.
  //
  
  const IlcDCHCalChamberPos* chamber = dynamic_cast<const IlcDCHCalChamberPos*>(GetCachedCDBObject(kIDChamberPos));
  if (!chamber)
    return kFALSE;
  
  const Float_t* kvalues = chamber->GetPos(det);
  if (!kvalues)
    return kFALSE;

  xyz[0] = kvalues[0];
  xyz[1] = kvalues[1];
  xyz[2] = kvalues[2];
  
  return kTRUE;
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::GetChamberRot(Int_t det, Float_t* xyz)
{
  //
  // Returns the rotation of the chamber from the nominal position.
  //
  
  const IlcDCHCalChamberPos* chamber = dynamic_cast<const IlcDCHCalChamberPos*>(GetCachedCDBObject(kIDChamberPos));
  if (!chamber)
    return kFALSE;
  
  const Float_t* kvalues = chamber->GetRot(det);
  if (!kvalues)
    return kFALSE;
  
  xyz[0] = kvalues[0];
  xyz[1] = kvalues[1];
  xyz[2] = kvalues[2];
  
  return kTRUE;
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::GetStackPos(Int_t chamber, Int_t sector, Float_t* xyz)
{
  //
  // Returns the deviation of the stack position from the nominal position.
  //
  
  const IlcDCHCalStackPos* stack = dynamic_cast<const IlcDCHCalStackPos*>(GetCachedCDBObject(kIDStackPos));
  if (!stack)
    return kFALSE;
  
  const Float_t* kvalues = stack->GetPos(chamber, sector);
  if (!kvalues)
    return kFALSE;
  
  xyz[0] = kvalues[0];
  xyz[1] = kvalues[1];
  xyz[2] = kvalues[2];
  
  return kTRUE;
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::GetStackRot(Int_t chamber, Int_t sector, Float_t* xyz)
{
  //
  // Returns the rotation of the stack from the nominal position.
  //
  
  const IlcDCHCalStackPos* stack = dynamic_cast<const IlcDCHCalStackPos*>(GetCachedCDBObject(kIDStackPos));
  if (!stack)
    return kFALSE;
  
  const Float_t* kvalues = stack->GetRot(chamber, sector);
  if (!kvalues)
    return kFALSE;

  xyz[0] = kvalues[0];
  xyz[1] = kvalues[1];
  xyz[2] = kvalues[2];
  
  return kTRUE;
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::GetSuperModulePos(Int_t sm, Float_t* xyz)
{
  //
  // Returns the deviation of the supermodule position from the nominal position.
  //
  
  const IlcDCHCalSuperModulePos* smPos = dynamic_cast<const IlcDCHCalSuperModulePos*>(GetCachedCDBObject(kIDSuperModulePos));
  if (!smPos)
    return kFALSE;
  
  const Float_t* kvalues = smPos->GetPos(sm);
  if (!kvalues)
    return kFALSE;
  
  xyz[0] = kvalues[0];
  xyz[1] = kvalues[1];
  xyz[2] = kvalues[2];
  
  return kTRUE;
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::GetSuperModuleRot(Int_t sm, Float_t* xyz)
{
  //
  // Returns the rotation of the supermodule from the nominal position.
  //
  
  const IlcDCHCalSuperModulePos* smPos = dynamic_cast<const IlcDCHCalSuperModulePos*>(GetCachedCDBObject(kIDSuperModulePos));
  if (!smPos)
    return kFALSE;
  
  const Float_t* kvalues = smPos->GetRot(sm);
  if (!kvalues)
    return kFALSE;
  
  xyz[0] = kvalues[0];
  xyz[1] = kvalues[1];
  xyz[2] = kvalues[2];
  
  return kTRUE;
}

//_____________________________________________________________________________
Float_t IlcDCHcalibDB::GetVdrift(Int_t det, Int_t col, Int_t row)
{
  //
  // Returns the drift velocity for the given pad.
  //

  const IlcDCHCalPad* calPad = dynamic_cast<const IlcDCHCalPad*> (GetCachedCDBObject(kIDVdriftPad));
  if (!calPad)
    return -1;

  IlcDCHCalROC* roc = calPad->GetCalROC(det);
  if (!roc)
    return -1;

  const IlcDCHCalDet* calChamber = dynamic_cast<const IlcDCHCalDet*> (GetCachedCDBObject(kIDVdriftChamber));
  if (!calChamber)
    return -1;

  return calChamber->GetValue(det) * roc->GetValue(col, row);
}

//_____________________________________________________________________________
Float_t IlcDCHcalibDB::GetVdriftAverage(Int_t det)
{
  //
  // Returns the average drift velocity for the given detector
  //

  const IlcDCHCalDet* calDet = dynamic_cast<const IlcDCHCalDet*> (GetCachedCDBObject(kIDVdriftChamber));
  if (!calDet)
    return -1;

  return calDet->GetValue(det);
}

//_____________________________________________________________________________
Float_t IlcDCHcalibDB::GetT0(Int_t det, Int_t col, Int_t row)
{
  //
  // Returns t0 for the given pad.
  //
  
  const IlcDCHCalPad* calPad = dynamic_cast<const IlcDCHCalPad*> (GetCachedCDBObject(kIDT0Pad));
  if (!calPad)
    return -1;

  IlcDCHCalROC* roc = calPad->GetCalROC(det);
  if (!roc)
    return -1;

  const IlcDCHCalDet* calChamber = dynamic_cast<const IlcDCHCalDet*> (GetCachedCDBObject(kIDT0Chamber));
  if (!calChamber)
    return -1;

  return calChamber->GetValue(det) * roc->GetValue(col, row);
}

//_____________________________________________________________________________
Float_t IlcDCHcalibDB::GetT0Average(Int_t det)
{
  //
  // Returns the average t0 for the given detector
  //

  const IlcDCHCalDet* calDet = dynamic_cast<const IlcDCHCalDet*> (GetCachedCDBObject(kIDT0Chamber));
  if (!calDet)
    return -1;

  return calDet->GetValue(det);
}

//_____________________________________________________________________________
Float_t IlcDCHcalibDB::GetGainFactor(Int_t det, Int_t col, Int_t row)
{
  //
  // Returns the gain factor for the given pad.
  //
  
  const IlcDCHCalPad* calPad = dynamic_cast<const IlcDCHCalPad*> (GetCachedCDBObject(kIDGainFactorPad));
  if (!calPad)
    return -1;

  IlcDCHCalROC* roc = calPad->GetCalROC(det);
  if (!roc)
    return -1;

  const IlcDCHCalDet* calChamber = dynamic_cast<const IlcDCHCalDet*> (GetCachedCDBObject(kIDGainFactorChamber));
  if (!calChamber)
    return -1;

  return calChamber->GetValue(det) * roc->GetValue(col, row);
}

//_____________________________________________________________________________
Float_t IlcDCHcalibDB::GetGainFactorAverage(Int_t det)
{
  //
  // Returns the average gain factor for the given detector
  //

  const IlcDCHCalDet* calDet = dynamic_cast<const IlcDCHCalDet*> (GetCachedCDBObject(kIDGainFactorChamber));
  if (!calDet)
    return -1;

  return calDet->GetValue(det);
}

//_____________________________________________________________________________
Float_t IlcDCHcalibDB::GetPRFWidth(Int_t det, Int_t col, Int_t row)
{
  //
  // Returns the PRF width for the given pad.
  //
  
  const IlcDCHCalPad* calPad = dynamic_cast<const IlcDCHCalPad*> (GetCachedCDBObject(kIDPRFWidth));
  if (!calPad)
    return -1;

  IlcDCHCalROC* roc = calPad->GetCalROC(det);
  if (!roc)
    return -1;

  return roc->GetValue(col, row);
}

//_____________________________________________________________________________
Float_t IlcDCHcalibDB::GetSamplingFrequency()
{
  //
  // Returns the sampling frequency of the DCH read-out.
  //
  
  const IlcDCHCalGlobals* calGlobal = dynamic_cast<const IlcDCHCalGlobals*> (GetCachedCDBObject(kIDGlobals));
  if (!calGlobal)
    return -1;  
  
  return calGlobal->GetSamplingFrequency();
}
  
//_____________________________________________________________________________
Int_t IlcDCHcalibDB::GetNumberOfTimeBins()
{
  //
  // Returns the number of time bins which are read-out.
  //

  const IlcDCHCalGlobals* calGlobal = dynamic_cast<const IlcDCHCalGlobals*> (GetCachedCDBObject(kIDGlobals));
  if (!calGlobal)
    return -1;

  return calGlobal->GetNumberOfTimeBins();
}

//_____________________________________________________________________________
Char_t IlcDCHcalibDB::GetPadStatus(Int_t det, Int_t col, Int_t row)
{
  //
  // Returns the status of the given pad
  //

  const IlcDCHCalPadStatus* cal = dynamic_cast<const IlcDCHCalPadStatus*> (GetCachedCDBObject(kIDPadStatus));
  if (!cal)
    return -1;

  const IlcDCHCalSingleChamberStatus* roc = cal->GetCalROC(det);
  if (!roc)
    return -1;

  return roc->GetStatus(col, row);
}

//_____________________________________________________________________________
Char_t IlcDCHcalibDB::GetMCMStatus(Int_t det, Int_t col, Int_t row)
{
  //
  // Returns the status of the given MCM
  //

  const IlcDCHCalMCMStatus* cal = dynamic_cast<const IlcDCHCalMCMStatus*> (GetCachedCDBObject(kIDMCMStatus));
  if (!cal)
    return -1;

  const IlcDCHCalSingleChamberStatus* roc = cal->GetCalROC(det);
  if (!roc)
    return -1;

  return roc->GetStatus(col, row);
}

//_____________________________________________________________________________
Char_t IlcDCHcalibDB::GetChamberStatus(Int_t det)
{
  //
  // Returns the status of the given chamber
  //

  const IlcDCHCalChamberStatus* cal = dynamic_cast<const IlcDCHCalChamberStatus*> (GetCachedCDBObject(kIDChamberStatus));
  if (!cal)
    return -1;

  return cal->GetStatus(det);
}

//_____________________________________________________________________________
Char_t IlcDCHcalibDB::GetSuperModuleStatus(Int_t sm)
{
  //
  // Returns the status of the given chamber
  //

  const IlcDCHCalSuperModuleStatus* cal = dynamic_cast<const IlcDCHCalSuperModuleStatus*> (GetCachedCDBObject(kIDSuperModuleStatus));
  if (!cal)
    return -1;

  return cal->GetStatus(sm);
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::IsPadMasked(Int_t det, Int_t col, Int_t row)
{
  //
  // Returns status, see name of functions for details ;-)
  //

  const IlcDCHCalPadStatus* cal = dynamic_cast<const IlcDCHCalPadStatus*> (GetCachedCDBObject(kIDPadStatus));
  if (!cal)
    return -1;

  return cal->IsMasked(det, col, row);
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::IsPadBridgedLeft(Int_t det, Int_t col, Int_t row)
{
  //
  // Returns status, see name of functions for details ;-)
  //

  const IlcDCHCalPadStatus* cal = dynamic_cast<const IlcDCHCalPadStatus*> (GetCachedCDBObject(kIDPadStatus));
  if (!cal)
    return -1;

  return cal->IsBridgedLeft(det, col, row);
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::IsPadBridgedRight(Int_t det, Int_t col, Int_t row)
{
  //
  // Returns status, see name of functions for details ;-)
  //

  const IlcDCHCalPadStatus* cal = dynamic_cast<const IlcDCHCalPadStatus*> (GetCachedCDBObject(kIDPadStatus));
  if (!cal)
    return -1;

  return cal->IsBridgedRight(det, col, row);
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::IsMCMMasked(Int_t det, Int_t col, Int_t row)
{
  //
  // Returns status, see name of functions for details ;-)
  //

  const IlcDCHCalMCMStatus* cal = dynamic_cast<const IlcDCHCalMCMStatus*> (GetCachedCDBObject(kIDMCMStatus));
  if (!cal)
    return -1;

  return cal->IsMasked(det, col, row);
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::IsChamberInstalled(Int_t det)
{
  //
  // Returns status, see name of functions for details ;-)
  //

  const IlcDCHCalChamberStatus* cal = dynamic_cast<const IlcDCHCalChamberStatus*> (GetCachedCDBObject(kIDChamberStatus));
  if (!cal)
    return -1;

  return cal->IsInstalled(det);
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::IsChamberMasked(Int_t det)
{
  //
  // Returns status, see name of functions for details ;-)
  //

  const IlcDCHCalChamberStatus* cal = dynamic_cast<const IlcDCHCalChamberStatus*> (GetCachedCDBObject(kIDChamberStatus));
  if (!cal)
    return -1;

  return cal->IsMasked(det);
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::IsSuperModuleInstalled(Int_t det)
{
  //
  // Returns status, see name of functions for details ;-)
  //

  const IlcDCHCalSuperModuleStatus* cal = dynamic_cast<const IlcDCHCalSuperModuleStatus*> (GetCachedCDBObject(kIDSuperModuleStatus));
  if (!cal)
    return -1;

  return cal->IsInstalled(det);
}

//_____________________________________________________________________________
Bool_t IlcDCHcalibDB::IsSuperModuleMasked(Int_t det)
{
  //
  // Returns status, see name of functions for details ;-)
  //

  const IlcDCHCalSuperModuleStatus* cal = dynamic_cast<const IlcDCHCalSuperModuleStatus*> (GetCachedCDBObject(kIDSuperModuleStatus));
  if (!cal)
    return -1;

  return cal->IsMasked(det);
}

//_____________________________________________________________________________
const IlcDCHCalPIDLQ* IlcDCHcalibDB::GetPIDLQObject()
{
  //
  // Returns the object storing the distributions for PID with likelihood
  //

  return dynamic_cast<const IlcDCHCalPIDLQ*> (GetCachedCDBObject(kIDPIDLQ));
}

//_____________________________________________________________________________
const IlcDCHCalMonitoring* IlcDCHcalibDB::GetMonitoringObject()
{
  //
  // Returns the object storing the monitoring data
  //

  return dynamic_cast<const IlcDCHCalMonitoring*> (GetCachedCDBObject(kIDMonitoringData));
}

//_____________________________________________________________________________
Float_t IlcDCHcalibDB::GetOmegaTau(Float_t vdrift)
{
  //
  // Returns omega*tau (tan(Lorentz-angle)) for a given drift velocity <vd> 
  // and a B-field <b> for Xe/CO2 (15%).
  // The values are according to a GARFIELD simulation.
  //
  // This function basically does not belong to the calibration class. It should be moved somewhere else. 
  // However, currently it is in use by simulation and reconstruction.
  //
  
  IlcDCHCommonParam* commonParam = IlcDCHCommonParam::Instance();
  if (!commonParam)
    return -1;
  Float_t fieldAbs = TMath::Abs(commonParam->GetField());
  Float_t fieldSgn = 1.0;
  if (fieldAbs > 0.0) {
    fieldSgn = commonParam->GetField() / fieldAbs;
  }

  const Int_t kNb = 5;
  Float_t p0[kNb] = {  0.004810,  0.007412,  0.010252,  0.013409,  0.016888 };
  Float_t p1[kNb] = {  0.054875,  0.081534,  0.107333,  0.131983,  0.155455 };
  Float_t p2[kNb] = { -0.008682, -0.012896, -0.016987, -0.020880, -0.024623 };
  Float_t p3[kNb] = {  0.000155,  0.000238,  0.000330,  0.000428,  0.000541 };

  Int_t ib = ((Int_t) (10 * (fieldAbs - 0.15)));
  ib       = TMath::Max(  0,ib);
  ib       = TMath::Min(kNb,ib);

  Float_t alphaL = p0[ib] 
                 + p1[ib] * vdrift
                 + p2[ib] * vdrift*vdrift
                 + p3[ib] * vdrift*vdrift*vdrift;

  return TMath::Tan(fieldSgn * alphaL);

}

//_____________________________________________________________________________
void IlcDCHcalibDB::SamplePRF()
{
  //
  // Samples the pad response function
  //

  const Int_t kPRFbin = 61;

  Float_t prf[kNplan][kPRFbin] = { {2.9037e-02, 3.3608e-02, 3.9020e-02, 4.5292e-02,
                    5.2694e-02, 6.1362e-02, 7.1461e-02, 8.3362e-02,
                    9.7063e-02, 1.1307e-01, 1.3140e-01, 1.5235e-01,
                    1.7623e-01, 2.0290e-01, 2.3294e-01, 2.6586e-01,
                    3.0177e-01, 3.4028e-01, 3.8077e-01, 4.2267e-01,
                    4.6493e-01, 5.0657e-01, 5.4655e-01, 5.8397e-01,
                    6.1767e-01, 6.4744e-01, 6.7212e-01, 6.9188e-01,
                    7.0627e-01, 7.1499e-01, 7.1851e-01, 7.1499e-01,
                    7.0627e-01, 6.9188e-01, 6.7212e-01, 6.4744e-01,
                    6.1767e-01, 5.8397e-01, 5.4655e-01, 5.0657e-01,
                    4.6493e-01, 4.2267e-01, 3.8077e-01, 3.4028e-01,
                    3.0177e-01, 2.6586e-01, 2.3294e-01, 2.0290e-01,
                    1.7623e-01, 1.5235e-01, 1.3140e-01, 1.1307e-01,
                    9.7063e-02, 8.3362e-02, 7.1461e-02, 6.1362e-02,
                    5.2694e-02, 4.5292e-02, 3.9020e-02, 3.3608e-02,
                    2.9037e-02},
                   {2.5478e-02, 2.9695e-02, 3.4655e-02, 4.0454e-02,
                    4.7342e-02, 5.5487e-02, 6.5038e-02, 7.6378e-02,
                    8.9696e-02, 1.0516e-01, 1.2327e-01, 1.4415e-01,
                    1.6794e-01, 1.9516e-01, 2.2573e-01, 2.5959e-01,
                    2.9694e-01, 3.3719e-01, 3.7978e-01, 4.2407e-01,
                    4.6889e-01, 5.1322e-01, 5.5569e-01, 5.9535e-01,
                    6.3141e-01, 6.6259e-01, 6.8882e-01, 7.0983e-01,
                    7.2471e-01, 7.3398e-01, 7.3761e-01, 7.3398e-01,
                    7.2471e-01, 7.0983e-01, 6.8882e-01, 6.6259e-01,
                    6.3141e-01, 5.9535e-01, 5.5569e-01, 5.1322e-01,
                    4.6889e-01, 4.2407e-01, 3.7978e-01, 3.3719e-01,
                    2.9694e-01, 2.5959e-01, 2.2573e-01, 1.9516e-01,
                    1.6794e-01, 1.4415e-01, 1.2327e-01, 1.0516e-01,
                    8.9696e-02, 7.6378e-02, 6.5038e-02, 5.5487e-02,
                    4.7342e-02, 4.0454e-02, 3.4655e-02, 2.9695e-02,
                    2.5478e-02},
                   {2.2363e-02, 2.6233e-02, 3.0782e-02, 3.6140e-02,
                    4.2535e-02, 5.0157e-02, 5.9197e-02, 6.9900e-02,
                    8.2707e-02, 9.7811e-02, 1.1548e-01, 1.3601e-01,
                    1.5998e-01, 1.8739e-01, 2.1840e-01, 2.5318e-01,
                    2.9182e-01, 3.3373e-01, 3.7837e-01, 4.2498e-01,
                    4.7235e-01, 5.1918e-01, 5.6426e-01, 6.0621e-01,
                    6.4399e-01, 6.7700e-01, 7.0472e-01, 7.2637e-01,
                    7.4206e-01, 7.5179e-01, 7.5551e-01, 7.5179e-01,
                    7.4206e-01, 7.2637e-01, 7.0472e-01, 6.7700e-01,
                    6.4399e-01, 6.0621e-01, 5.6426e-01, 5.1918e-01,
                    4.7235e-01, 4.2498e-01, 3.7837e-01, 3.3373e-01,
                    2.9182e-01, 2.5318e-01, 2.1840e-01, 1.8739e-01,
                    1.5998e-01, 1.3601e-01, 1.1548e-01, 9.7811e-02,
                    8.2707e-02, 6.9900e-02, 5.9197e-02, 5.0157e-02,
                    4.2535e-02, 3.6140e-02, 3.0782e-02, 2.6233e-02,
                    2.2363e-02},
                   {1.9635e-02, 2.3167e-02, 2.7343e-02, 3.2293e-02,
                    3.8224e-02, 4.5335e-02, 5.3849e-02, 6.4039e-02,
                    7.6210e-02, 9.0739e-02, 1.0805e-01, 1.2841e-01,
                    1.5216e-01, 1.7960e-01, 2.1099e-01, 2.4671e-01,
                    2.8647e-01, 3.2996e-01, 3.7660e-01, 4.2547e-01,
                    4.7536e-01, 5.2473e-01, 5.7215e-01, 6.1632e-01,
                    6.5616e-01, 6.9075e-01, 7.1939e-01, 7.4199e-01,
                    7.5838e-01, 7.6848e-01, 7.7227e-01, 7.6848e-01,
                    7.5838e-01, 7.4199e-01, 7.1939e-01, 6.9075e-01,
                    6.5616e-01, 6.1632e-01, 5.7215e-01, 5.2473e-01,
                    4.7536e-01, 4.2547e-01, 3.7660e-01, 3.2996e-01,
                    2.8647e-01, 2.4671e-01, 2.1099e-01, 1.7960e-01,
                    1.5216e-01, 1.2841e-01, 1.0805e-01, 9.0739e-02,
                    7.6210e-02, 6.4039e-02, 5.3849e-02, 4.5335e-02,
                    3.8224e-02, 3.2293e-02, 2.7343e-02, 2.3167e-02,
                    1.9635e-02},
                   {1.7224e-02, 2.0450e-02, 2.4286e-02, 2.8860e-02,
                    3.4357e-02, 4.0979e-02, 4.8966e-02, 5.8612e-02,
                    7.0253e-02, 8.4257e-02, 1.0102e-01, 1.2094e-01,
                    1.4442e-01, 1.7196e-01, 2.0381e-01, 2.4013e-01,
                    2.8093e-01, 3.2594e-01, 3.7450e-01, 4.2563e-01,
                    4.7796e-01, 5.2991e-01, 5.7974e-01, 6.2599e-01,
                    6.6750e-01, 7.0344e-01, 7.3329e-01, 7.5676e-01,
                    7.7371e-01, 7.8410e-01, 7.8793e-01, 7.8410e-01,
                    7.7371e-01, 7.5676e-01, 7.3329e-01, 7.0344e-01,
                    6.6750e-01, 6.2599e-01, 5.7974e-01, 5.2991e-01,
                    4.7796e-01, 4.2563e-01, 3.7450e-01, 3.2594e-01,
                    2.8093e-01, 2.4013e-01, 2.0381e-01, 1.7196e-01,
                    1.4442e-01, 1.2094e-01, 1.0102e-01, 8.4257e-02,
                    7.0253e-02, 5.8612e-02, 4.8966e-02, 4.0979e-02,
                    3.4357e-02, 2.8860e-02, 2.4286e-02, 2.0450e-02,
                    1.7224e-02},
                   {1.5096e-02, 1.8041e-02, 2.1566e-02, 2.5793e-02,
                    3.0886e-02, 3.7044e-02, 4.4515e-02, 5.3604e-02,
                    6.4668e-02, 7.8109e-02, 9.4364e-02, 1.1389e-01,
                    1.3716e-01, 1.6461e-01, 1.9663e-01, 2.3350e-01,
                    2.7527e-01, 3.2170e-01, 3.7214e-01, 4.2549e-01,
                    4.8024e-01, 5.3460e-01, 5.8677e-01, 6.3512e-01,
                    6.7838e-01, 7.1569e-01, 7.4655e-01, 7.7071e-01,
                    7.8810e-01, 7.9871e-01, 8.0255e-01, 7.9871e-01,
                    7.8810e-01, 7.7071e-01, 7.4655e-01, 7.1569e-01,
                    6.7838e-01, 6.3512e-01, 5.8677e-01, 5.3460e-01,
                    4.8024e-01, 4.2549e-01, 3.7214e-01, 3.2170e-01,
                    2.7527e-01, 2.3350e-01, 1.9663e-01, 1.6461e-01,
                    1.3716e-01, 1.1389e-01, 9.4364e-02, 7.8109e-02,
                    6.4668e-02, 5.3604e-02, 4.4515e-02, 3.7044e-02,
                    3.0886e-02, 2.5793e-02, 2.1566e-02, 1.8041e-02,
                    1.5096e-02}};

  // More sampling precision with linear interpolation
  fPadResponse.fPRFlo  = -1.5;
  fPadResponse.fPRFhi  =  1.5;
  Float_t pad[kPRFbin];
  Int_t   sPRFbin = kPRFbin;  
  Float_t sPRFwid = (fPadResponse.fPRFhi - fPadResponse.fPRFlo) / ((Float_t) sPRFbin);
  for (Int_t iPad = 0; iPad < sPRFbin; iPad++) {
    pad[iPad] = ((Float_t) iPad + 0.5) * sPRFwid + fPadResponse.fPRFlo;
  }
  fPadResponse.fPRFbin = 500;  
  fPadResponse.fPRFwid = (fPadResponse.fPRFhi - fPadResponse.fPRFlo) / ((Float_t) fPadResponse.fPRFbin);
  fPadResponse.fPRFpad = ((Int_t) (1.0 / fPadResponse.fPRFwid));

  if (fPadResponse.fPRFsmp) delete [] fPadResponse.fPRFsmp;
  fPadResponse.fPRFsmp = new Float_t[kNplan*fPadResponse.fPRFbin];

  Int_t   ipos1;
  Int_t   ipos2;
  Float_t diff;

  for (Int_t iPla = 0; iPla < kNplan; iPla++) {

    for (Int_t iBin = 0; iBin < fPadResponse.fPRFbin; iBin++) {

      Float_t bin = (((Float_t) iBin) + 0.5) * fPadResponse.fPRFwid + fPadResponse.fPRFlo;
      ipos1 = ipos2 = 0;
      diff  = 0;
      do {
        diff = bin - pad[ipos2++];
      } while ((diff > 0) && (ipos2 < kPRFbin));
      if      (ipos2 == kPRFbin) {
        fPadResponse.fPRFsmp[iPla*fPadResponse.fPRFbin+iBin] = prf[iPla][ipos2-1];
      }
      else if (ipos2 == 1) {
        fPadResponse.fPRFsmp[iPla*fPadResponse.fPRFbin+iBin] = prf[iPla][ipos2-1];
      }
      else {
        ipos2--;
        if (ipos2 >= kPRFbin) ipos2 = kPRFbin - 1;
        ipos1 = ipos2 - 1;
        fPadResponse.fPRFsmp[iPla*fPadResponse.fPRFbin+iBin] = prf[iPla][ipos2] 
                                   + diff * (prf[iPla][ipos2] - prf[iPla][ipos1]) 
                                          / sPRFwid;
      }

    }
  } 

}

//_____________________________________________________________________________
Int_t IlcDCHcalibDB::PadResponse(Double_t signal, Double_t dist
    , Int_t plane, Double_t *pad) const
{
  //
  // Applies the pad response
  //

  Int_t iBin  = ((Int_t) (( - dist - fPadResponse.fPRFlo) / fPadResponse.fPRFwid));
  Int_t iOff  = plane * fPadResponse.fPRFbin;

  Int_t iBin0 = iBin - fPadResponse.fPRFpad + iOff;
  Int_t iBin1 = iBin           + iOff;
  Int_t iBin2 = iBin + fPadResponse.fPRFpad + iOff;

  pad[0] = 0.0;
  pad[1] = 0.0;
  pad[2] = 0.0;
  if ((iBin1 >= 0) && (iBin1 < (fPadResponse.fPRFbin*kNplan))) {

    if (iBin0 >= 0) {
      pad[0] = signal * fPadResponse.fPRFsmp[iBin0];
    }
    pad[1] = signal * fPadResponse.fPRFsmp[iBin1];
    if (iBin2 < (fPadResponse.fPRFbin*kNplan)) {
      pad[2] = signal * fPadResponse.fPRFsmp[iBin2];
    }

    return 1;

  }
  else {

    return 0;

  }

}

