// $Id: hits_from_label.C 54257 2012-01-30 20:52:05Z quark $
// Main authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007

/**************************************************************************
 * 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * Author: The International Lepton Collider Off-line Project.            *
 *                                                                        *
 * Part of the code has been developed by ILC Off-line Project.         *
 * Contributors are mentioned in the code where appropriate.              *
 * See http://ilcinfo.cern.ch/Offline/IlcRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#if !defined(__CINT__) || defined(__MAKECINT__)
#include <TEveUtil.h>
#include <TEvePointSet.h>
#include <TEveElement.h>
#include <TEveManager.h>

#include "its_hits.C"
#include "tof_hits.C"
#include "tpc_hits.C"
#include "trd_hits.C"
#endif

void hits_from_label(Int_t label=0, TEveElement* cont=0)
{
  TEveUtil::LoadMacro("its_hits.C");

  TEvePointSet* h = 0;
  //PH The line below is replaced waiting for a fix in Root
  //PH which permits to use variable siza arguments in CINT
  //PH on some platforms (alphalinuxgcc, solariscc5, etc.)
  //PH  h = its_hits("fX:fY:fZ", Form("ITS.fTrack==%d", label));
  char form[1000];
  sprintf(form,"ITS.fTrack==%d", label);
  h = its_hits("fX:fY:fZ", form, cont);
  if (h) h->SetMarkerSize(1);

  TEveUtil::LoadMacro("tpc_hits.C");
  sprintf(form,"TPC2.fArray.fTrackID==%d", label);
  h = tpc_hits("TPC2.fArray.fR:TPC2.fArray.fFi:TPC2.fArray.fZ",form, cont);
  //PH  h = tpc_hits("TPC2.fArray.fR:TPC2.fArray.fFi:TPC2.fArray.fZ",
  //PH	       Form("TPC2.fArray.fTrackID==%d", label));
  if (h) h->SetMarkerSize(1);

  TEveUtil::LoadMacro("trd_hits.C");
  sprintf(form,"TRD.fTrack==%d", label);
  h = trd_hits("fX:fY:fZ", form, cont);
  if (h) h->SetMarkerSize(1);

  TEveUtil::LoadMacro("tof_hits.C");
  sprintf(form,"TOF.fTrack==%d", label);
  h = tof_hits("fX:fY:fZ", form, cont);
  if (h) h->SetMarkerSize(1);

  gEve->Redraw3D();
}
