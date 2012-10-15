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

/* $Id: IlcPVECSurvey.cxx 46104 2010-12-16 18:00:01Z policheh $ */

/* History of cvs commits:
 *
 * $Log$
 * Revision 1.6  2007/08/28 13:12:18  hristov
 * IlcAlignObjAngles becomes IlcAlignObjParams (Raffaele)
 *
 * Revision 1.5  2007/07/10 12:41:38  kharlov
 * Added a new class IlcPVECSurvet1 which read survey data from EDMS files
 *
 * Revision 1.4  2007/05/17 17:34:54  kharlov
 * Merging differences if v1.2 and 1.3
 *
 * Revision 1.3  2007/05/17 17:13:32  kharlov
 * Coding convensions satisfied (T.Pocheptsov)
 *
 * Revision 1.1  2007/04/19 15:47:20  kharlov
 * Add misalignment of strip units with IlcPVECSurvey class
 *
 */

// Objects of this class read txt file with survey (photogrammetry) data
// and convert the data into IlcAlignObjParams of alignable PVEC volumes.
// It can be used as a base class, you need to override GetStripTransformation.
// IlcPVECSurvey inherits TObject only to use IlcLog "functions".
// Author: Timur Pocheptsov (JINR)

#include <fstream>

#include <TClonesArray.h>
#include <TGeoManager.h>
#include <TString.h>
#include <TMath.h>

#include "IlcSurveyObj.h"

#include "IlcPVECEMCAGeometry.h"
#include "IlcAlignObjParams.h"
#include "IlcPVECGeometry.h"
#include "IlcPVECSurvey.h"
#include "IlcLog.h"

ClassImp(IlcPVECSurvey)

//____________________________________________________________________________
IlcPVECSurvey::IlcPVECSurvey()
                : fStrNum(0),
                  fStripData(0)
{
  //Default constructor.
}

namespace {

  struct IlcPVECStripCoords {
    Double_t fX1; //x coordinate of the first strip point
    Double_t fZ1; //z coordinate of the first strip point
    Double_t fX2; //x coordinate of the second strip point
    Double_t fZ2; //z coordinate of the second strip point
  };

}

//____________________________________________________________________________
IlcPVECSurvey::IlcPVECSurvey(const TString &txtFileName)
                : fStrNum(0),
                  fStripData(0)
{
  //Read survey data from txt file.
  const IlcPVECGeometry *PVECGeom = IlcPVECGeometry::GetInstance("ORKA", "ORKA");
  if (!PVECGeom) {
    IlcError("Cannot obtain IlcPVECGeometry instance.");
    return;
  }

  std::ifstream inputFile(txtFileName.Data());
  if (!inputFile) {
    IlcError(("Cannot open the survey file " + txtFileName).Data());
    return;
  }

  IlcPVECEMCAGeometry * emcaGeom = PVECGeom->GetEMCAGeometry();
  fStrNum = emcaGeom->GetNStripX() * emcaGeom->GetNStripZ();

  Int_t dummyInt = 0;
  Double_t dummyY = 0.;
  Double_t *xReal = new Double_t[fStrNum * 2];//2
  Double_t *zReal = new Double_t[fStrNum * 2];//3

  for (Int_t i = 0; i < fStrNum * 2; ++i) {
    xReal[i] = 0.;
    zReal[i] = 0.;
  }
  
  for (Int_t i = 0; i < fStrNum * 2; ++i) {
    if (!inputFile) {
      IlcError("Error while reading input file.");
      delete [] xReal;
      delete [] zReal;
      return;
    }
    inputFile>>dummyInt>>xReal[i]>>dummyY>>zReal[i];
    xReal[i] *= 0.1;
    zReal[i] *= 0.1;
  }

  InitStripData(xReal, zReal);

  delete [] zReal;
  delete [] xReal;
}

//____________________________________________________________________________
IlcPVECSurvey::~IlcPVECSurvey()
{
  delete [] fStripData;
}

//____________________________________________________________________________
void IlcPVECSurvey::CreateIlcAlignObjParams(TClonesArray &array)
{
  //Create IlcAlignObjParams.
  const IlcPVECGeometry * PVECGeom = IlcPVECGeometry::GetInstance("ORKA", "ORKA");
  if (!PVECGeom) {
    IlcError("Cannot obtain IlcPVECGeometry instance.");
    return;
  }

  if (!gGeoManager) {
    IlcWarning("Cannot create local transformations for strip units - gGeoManager does not exist.");
    IlcInfo("Null shifts and rotations will be created instead.");
    return CreateNullObjects(array, PVECGeom);
  }

  IlcPVECEMCAGeometry * emcaGeom = PVECGeom->GetEMCAGeometry();
  Int_t arrayInd = array.GetEntries(), iIndex = 0;
  IlcGeomManager::ELayerID iLayer = IlcGeomManager::kInvalidLayer;
  UShort_t volid = IlcGeomManager::LayerToVolUID(iLayer,iIndex);

  for (Int_t module = 1; module <= PVECGeom->GetNModules(); ++module) {
    for (Int_t i = 0, stripNum = 0; i < emcaGeom->GetNStripX(); ++i) {
      for (Int_t j = 0; j < emcaGeom->GetNStripZ(); ++j) {
        TString stripName(TString::Format("PVEC/Module%d/Strip_%d_%d", module, i, j));
        IlcPVECStripDelta t(GetStripTransformation(stripNum++, module));
        new(array[arrayInd])
          IlcAlignObjParams(
                            stripName.Data(), volid, 
                            t.fXShift, t.fYShift, t.fZShift, 
                            -t.fPsi, -t.fTheta, -t.fPhi, 
                            false
                           );
        ++arrayInd;
      }
    }
  }
}

//____________________________________________________________________________
void IlcPVECSurvey::CreateNullObjects(TClonesArray &array, const IlcPVECGeometry *PVECGeom)const
{
  //Create null shifts and rotations.
  const IlcPVECEMCAGeometry * emcaGeom = PVECGeom->GetEMCAGeometry();
  Int_t arrayInd = array.GetEntries(), iIndex = 0;
  IlcGeomManager::ELayerID iLayer = IlcGeomManager::kInvalidLayer;
  UShort_t volid = IlcGeomManager::LayerToVolUID(iLayer,iIndex);

  for (Int_t module = 1; module <= PVECGeom->GetNModules(); ++module)
    for (Int_t i = 0; i < emcaGeom->GetNStripX(); ++i)
      for (Int_t j = 0; j < emcaGeom->GetNStripZ(); ++j) {
        TString stripName(TString::Format("PVEC/Module%d/Strip_%d_%d", module, i, j));
        new(array[arrayInd]) IlcAlignObjParams(stripName.Data(), volid, 0., 0., 0., 0., 0., 0., true);
        ++arrayInd;
      }
}

//____________________________________________________________________________
IlcPVECSurvey::IlcPVECStripDelta IlcPVECSurvey::GetStripTransformation(Int_t stripIndex, Int_t module)const
{
  //Strip 'stripIndex' transformation.
  IlcPVECStripDelta t = {0., 0., 0., 0., 0., 0.};
  if (module != 3 || !fStripData)
    return t;
  return fStripData[stripIndex];
}

//____________________________________________________________________________
void IlcPVECSurvey::InitStripData(const Double_t *xReal, const Double_t *zReal)
{
  const IlcPVECGeometry *PVECGeom = IlcPVECGeometry::GetInstance("ORKA", "ORKA");
  IlcPVECEMCAGeometry * emcaGeom = PVECGeom->GetEMCAGeometry();
  const Float_t *strip = emcaGeom->GetStripHalfSize();
  const Float_t *cell  = emcaGeom->GetAirCellHalfSize(); 

  IlcPVECStripCoords *idealStrips = new IlcPVECStripCoords[fStrNum];//1
  for (Int_t ix = 0, stripNumber = 0; ix < emcaGeom->GetNStripX(); ++ix) {
    for (Int_t iz = 0; iz < emcaGeom->GetNStripZ(); ++iz) {
      IlcPVECStripCoords &str = idealStrips[stripNumber++];
      str.fX1 = ix * 2 * strip[0];
      str.fX2 = str.fX1 + 14 * cell[0];
      str.fZ1 = iz * 2 * strip[2];
      str.fZ2 = str.fZ1 + 2 * cell[2];
    }
  }

  IlcPVECStripCoords *realStrips = new IlcPVECStripCoords[fStrNum];//4
  for (Int_t j = 0, stripNumber = 0; j < emcaGeom->GetNStripX() * 2; j += 2) {
    for (Int_t i = 0; i < emcaGeom->GetNStripZ(); ++i) {
      IlcPVECStripCoords &str = realStrips[stripNumber++];
      str.fX1 = xReal[i + j * emcaGeom->GetNStripZ()];
      str.fZ1 = zReal[i + j * emcaGeom->GetNStripZ()];
      str.fX2 = xReal[i + (j + 1) * emcaGeom->GetNStripZ()];
      str.fZ2 = zReal[i + (j + 1) * emcaGeom->GetNStripZ()];
    }
  }

  fStripData = new IlcPVECStripDelta[fStrNum];
  
  for (Int_t i = 0; i < fStrNum; ++i) {
    const IlcPVECStripCoords &real = realStrips[i];
    const IlcPVECStripCoords &ideal = idealStrips[i];
    IlcPVECStripDelta &t = fStripData[i];
    t.fTheta = TMath::ATan((real.fZ2 - real.fZ1)  / (real.fX2 - real.fX1)) - 
               TMath::ATan((ideal.fZ2 - ideal.fZ1) / (ideal.fX2 - ideal.fX1));
    t.fTheta *= TMath::RadToDeg();
    t.fXShift = (real.fX1 + real.fX2) / 2 - (ideal.fX1 + ideal.fX2) / 2;
    t.fZShift = (real.fZ1 + real.fZ2) / 2 - (ideal.fZ1 + ideal.fZ2) / 2;
    t.fYShift = 0., t.fPsi = 0., t.fPhi = 0.;
  }

  delete [] realStrips;
  delete [] idealStrips;
}
