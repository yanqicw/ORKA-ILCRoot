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

/* $Id: IlcReconstructor.cxx 52237 2011-10-20 19:26:08Z hristov $ */

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// base class for reconstruction algorithms                                  //
//                                                                           //
// Derived classes should implement a default constructor and                //
// the virtual methods                                                       //
// - Reconstruct : to perform the local reconstruction for all events        //
// - FillESD     : to fill the ESD for the current event                     //
//                                                                           //
// The reconstructor classes for the barrel detectors should in addition     //
// implement the method                                                      //
// - CreateTracker : to create a tracker object for the barrel detector      //
//                                                                           //
// The ITS reconstructor should in addition implement the method             //
// - CreateVertexer : to create an object for the vertex finding             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include "IlcLog.h"
#include "IlcReconstructor.h"
#include <TClass.h>
#include <TString.h>


ClassImp(IlcReconstructor)

const IlcDetectorRecoParam* IlcReconstructor::fgRecoParam[IlcReconstruction::kNDetectors] = {NULL,NULL};

//_____________________________________________________________________________
void IlcReconstructor::ConvertDigits(IlcRawReader* /*rawReader*/, 
				     TTree* /*digitsTree*/) const
{
// convert raw data digits into digit objects in a root tree

  IlcError("conversion of raw data digits into digit objects not implemented");
}


//_____________________________________________________________________________
void IlcReconstructor::Reconstruct(TTree* /*digitsTree*/,
				   TTree* /*clustersTree*/) const
{
// run the local reconstruction

  IlcError("local event reconstruction not implemented");
}

//_____________________________________________________________________________
void IlcReconstructor::Reconstruct(IlcRawReader* /*rawReader*/, 
				   TTree* /*clustersTree*/) const
{
// run the local reconstruction with raw data input

  IlcError("local event reconstruction not implemented for raw data input");
}

//_____________________________________________________________________________
void IlcReconstructor::FillESD(TTree* /*digitsTree*/, TTree* /*clustersTree*/,
			       IlcESDEvent* /*esd*/) const
{
// fill the ESD.
// by default nothing is done

}

//_____________________________________________________________________________
void IlcReconstructor::FillESD(IlcRawReader* /*rawReader*/, 
			       TTree* clustersTree, IlcESDEvent* esd) const
{
// fill the ESD in case of raw data input.
// by default the FillESD method for MC is called

  FillESD((TTree*)NULL, clustersTree, esd);
}

//_____________________________________________________________________________
const char* IlcReconstructor::GetDetectorName() const
{
// get the name of the detector

  static TString detName;
  detName = GetName();
  detName.Remove(0, 3);
  detName.Remove(detName.Index("Reconstructor"));
  detName.ReplaceAll("Upgrade","");
  return detName.Data();
}

//_____________________________________________________________________________
void IlcReconstructor::SetRecoParam(const IlcDetectorRecoParam *par)
{
  // To be implemented by the detectors.
  // As soon as we manage to remove the static members
  // and method in the detector reconstructors, we will
  // implemented this method in the base class and remove
  // the detectors implementations.
  Int_t iDet = IlcReconstruction::GetDetIndex(GetDetectorName());

  if (iDet >= 0)
    fgRecoParam[iDet] = par;
  else
    IlcError(Form("Invalid detector index for (%s)",GetDetectorName()));

  return;
}

//_____________________________________________________________________________
const IlcDetectorRecoParam* IlcReconstructor::GetRecoParam(Int_t iDet)
{
  // Get the current reconstruciton parameters
  // for a given detector 
  if (iDet >= 0 && iDet < IlcReconstruction::kNDetectors)
    return fgRecoParam[iDet];
  else {
    IlcErrorClass(Form("Invalid detector index (%d)",iDet));
    return NULL;
  }
}

//_____________________________________________________________________________
void IlcReconstructor::GetPidSettings(IlcESDpid */*esdPID*/) {
  //
  // Function to set Pid settings in esdPID
  // based on detector-specific IlcRecoParams
  // to be implemented by detectors separately (e.g TOF)
  // 
  return;
}
