// $Id: esd_spd_tracklets.C 55699 2012-04-11 10:01:56Z hristov $
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
#include <TMath.h>
#include <TEveManager.h>
#include <TEveElement.h>
#include <TEveTrack.h>
#include <TEveTrackPropagator.h>

#include <IlcESDEvent.h>
#include <IlcESDVertex.h>
#include <IlcMultiplicity.h>
#include <IlcMagF.h>
#include <IlcEveEventManager.h>
#include <IlcEveTracklet.h>
#include <IlcEveTrackCounter.h>
#endif

TEveElementList* esd_spd_tracklets(Float_t radius=8, Width_t line_width=2,
				   Float_t dPhiWindow=0.080, Float_t dThetaWindow=0.025, 
				   Float_t dPhiShift05T=0.0045)
{
  // radius - cylindrical radius to which the tracklets should be extrapolated

  IlcESDEvent     *esd = IlcEveEventManager::AssertESD();
  const IlcESDVertex    *pv  = esd->GetPrimaryVertexSPD();
  const IlcMultiplicity *mul = esd->GetMultiplicity();

  IlcMagF *field = IlcEveEventManager::AssertMagField();

  TEveElementList* cont = new TEveElementList("SPD Tracklets");
  gEve->AddElement(cont);

  TEveTrackList *tg = new TEveTrackList("Good");
  tg->SetMainColor(kCyan);
  tg->SetLineWidth(line_width);
  cont->AddElement(tg);

  TEveTrackPropagator* pg = tg->GetPropagator();
  pg->SetMaxR(radius);

  TEveTrackList *tb = new TEveTrackList("Bad");
  tb->SetMainColor(kMagenta);
  tb->SetLineWidth(line_width);
  cont->AddElement(tb);

  TEveTrackPropagator* pb = tb->GetPropagator();
  pb->SetMaxR(radius);

  const Float_t  Bz = TMath::Abs(field->SolenoidField());

  const Double_t dPhiShift     = dPhiShift05T / 5.0 * Bz;
  const Double_t dPhiWindow2   = dPhiWindow * dPhiWindow;
  const Double_t dThetaWindow2 = dThetaWindow * dThetaWindow;

  for (Int_t i = 0; i < mul->GetNumberOfTracklets(); ++i)
  {
    Float_t theta  = mul->GetTheta(i);
    Float_t phi    = mul->GetPhi(i);
    Float_t dTheta = mul->GetDeltaTheta(i);
    Float_t dPhi   = mul->GetDeltaPhi(i) - dPhiShift;
    Float_t d      = dPhi*dPhi/dPhiWindow2 + dTheta*dTheta/dThetaWindow2;

    TEveTrackList* tl = (d < 1.0f) ? tg : tb;

    IlcEveTracklet *t = new IlcEveTracklet(i, pv, theta, phi, tl->GetPropagator());
    t->SetAttLineAttMarker(tl);
    t->SetElementName(Form("Tracklet %d", i));
    t->SetElementTitle(Form("Id = %d\nEta=%.3f, Theta=%.3f, dTheta=%.3f\nPhi=%.3f dPhi=%.3f",
			    i, mul->GetEta(i), theta, dTheta, phi, dPhi));

    tl->AddElement(t);
  }

  tg->MakeTracks();
  tg->SetTitle(Form("N=%d", tg->NumChildren()));

  tb->MakeTracks();
  tb->SetTitle(Form("N=%d", tb->NumChildren()));

  if (IlcEveTrackCounter::IsActive())
  {
    IlcEveTrackCounter::fgInstance->RegisterTracklets(tg, kTRUE);
    IlcEveTrackCounter::fgInstance->RegisterTracklets(tb, kFALSE);
  }
  else
  {
//==========================================
    tb->SetLineStyle(1);
    tb->SetLineWidth(2);
//==========================================

  }

  gEve->Redraw3D();

  return cont;
}
