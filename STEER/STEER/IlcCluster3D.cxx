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

//-------------------------------------------------------------------------
//                         Class IlcCluster3D
//  This is an extension of the IlcCluster class for the case when
//  the sensitive plane this cluster belongs to is arbitrarily oriented
//  in space.  This class can serve as the base for the TOF and HMPID
//  clusters.
//
// cvetan.cheshkov@cern.ch & jouri.belikov@cern.ch    5/3/2007
//-------------------------------------------------------------------------
#include <TGeoManager.h>
#include <TGeoMatrix.h>

#include "IlcCluster3D.h"
#include "IlcLog.h"

ClassImp(IlcCluster3D)

//______________________________________________________________________________
IlcCluster3D::IlcCluster3D():
  IlcCluster(),
  fSigmaX2(0),
  fSigmaXY(0),
  fSigmaXZ(0)
{
  // Default constructor
}

//______________________________________________________________________________
IlcCluster3D::IlcCluster3D(UShort_t volId, 
   Float_t x,   Float_t y,   Float_t z,
   Float_t sx2, Float_t sxy, Float_t sxz,
                Float_t sy2, Float_t syz, 
                             Float_t sz2, const Int_t *lab):
  IlcCluster(volId,x,y,z,sy2,sz2,syz,lab),
  fSigmaX2(sx2),
  fSigmaXY(sxy),
  fSigmaXZ(sxz)
{
//-------------------------------------------------------------------------
// The main constructor
//-------------------------------------------------------------------------
}

//______________________________________________________________________________
IlcCluster3D::IlcCluster3D(const IlcCluster3D& cluster):
  IlcCluster(cluster),
  fSigmaX2(cluster.fSigmaX2),
  fSigmaXY(cluster.fSigmaXY),
  fSigmaXZ(cluster.fSigmaXZ)
{
  // Copy constructor
}

//______________________________________________________________________________
Bool_t IlcCluster3D::GetGlobalCov(Float_t cov[6]) const
{

  // Get the covariance matrix in the global coordinate system.
  // All the needed information is taken only
  // from TGeo.

  if (!gGeoManager || !gGeoManager->IsClosed()) {
    IlcError("gGeoManager doesn't exist or it is still opened !");
    return kFALSE;
  }

  const TGeoHMatrix *mt = GetTracking2LocalMatrix();
  if (!mt) return kFALSE;

  TGeoHMatrix *ml = GetMatrix();
  if (!ml) return kFALSE;

  TGeoHMatrix m;
  Double_t tcov[9] = { fSigmaX2, fSigmaXY,     fSigmaXZ, 
  	               fSigmaXY, GetSigmaY2(), GetSigmaYZ(), 
  	               fSigmaXZ, GetSigmaYZ(), GetSigmaZ2()};
  m.SetRotation(tcov);
  m.Multiply(&mt->Inverse());
  m.Multiply(&ml->Inverse());
  m.MultiplyLeft(mt);
  m.MultiplyLeft(ml);
  Double_t *ncov = m.GetRotationMatrix();
  cov[0] = ncov[0]; cov[1] = ncov[1]; cov[2] = ncov[2];
  cov[3] = ncov[4]; cov[4] = ncov[5];
  cov[5] = ncov[8];

  return kTRUE;
}
