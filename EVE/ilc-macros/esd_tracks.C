// $Id: esd_tracks.C 58244 2012-08-21 12:57:19Z hristov $
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
#include <TString.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TMath.h>
#include <TGListTree.h>
#include <TEveVSDStructs.h>
#include <TEveManager.h>
#include <TEveTrackPropagator.h>

#include <IlcESDtrackCuts.h>
// #include <IlcPWG0Helper.h>
#include <IlcESDEvent.h>
#include <IlcESDfriend.h>
#include <IlcESDtrack.h>
#include <IlcESDfriendTrack.h>
#include <IlcExternalTrackParam.h>
#include <IlcEveTrack.h>
#include <IlcEveTrackCounter.h>
#include <IlcEveMagField.h>
#include <IlcEveEventManager.h>
#endif


// Use inner-dch track params when target refit failed.
Bool_t g_esd_tracks_use_ip_on_failed_target_refit = kFALSE;

// Use magnetic-field as retrieved from GRP.
Bool_t g_esd_tracks_true_field = kTRUE;

// Use Runge-Kutta track stepper.
Bool_t g_esd_tracks_rk_stepper = kFALSE;


//==============================================================================

void esd_track_propagator_setup(TEveTrackPropagator* trkProp,
				Float_t magF, Float_t maxR)
{
  if (g_esd_tracks_true_field)
  {
    trkProp->SetMagFieldObj(new IlcEveMagField);
  }
  else
  {
    trkProp->SetMagField(magF);
  }
  if (g_esd_tracks_rk_stepper)
  {
    trkProp->SetStepper(TEveTrackPropagator::kRungeKutta);
  }
  trkProp->SetMaxR(maxR);
}

//==============================================================================

TString esd_track_title(IlcESDtrack* t)
{
  TString s;

  Int_t label = t->GetLabel(), index = t->GetID();
  TString idx(index == kMinInt ? "<undef>" : Form("%d", index));
  TString lbl(label == kMinInt ? "<undef>" : Form("%d", label));

  Double_t p[3], v[3];
  t->GetXYZ(v);
  t->GetPxPyPz(p);
  Double_t pt    = t->Pt();
  Double_t ptsig = TMath::Sqrt(t->GetSigma1Pt2());
  Double_t ptsq  = pt*pt;
  Double_t ptm   = pt / (1.0 + pt*ptsig);
  Double_t ptM   = pt / (1.0 - pt*ptsig);

  s = Form("Index=%s, Label=%s\nChg=%d, Pdg=%d\n"
	   "pT = %.3f + %.3f - %.3f [%.3f]\n"
           "P  = (%.3f, %.3f, %.3f)\n"
           "V  = (%.3f, %.3f, %.3f)\n",
	   idx.Data(), lbl.Data(), t->Charge(), 0,
	   pt, ptM - pt, pt - ptm, ptsig*ptsq,
           p[0], p[1], p[2],
           v[0], v[1], v[2]);

  Int_t   o;
  s += "Det (in,out,refit,pid):\n";
  o  = IlcESDtrack::kVXDin;
  s += Form("TRG (%d,%d,%d,%d)  ",  t->IsOn(o), t->IsOn(o<<1), t->IsOn(o<<2), t->IsOn(o<<3));
  o  = IlcESDtrack::kTPCin;
  s += Form("DCH(%d,%d,%d,%d)\n",   t->IsOn(o), t->IsOn(o<<1), t->IsOn(o<<2), t->IsOn(o<<3));
  s += Form("ESD pid=%d", t->IsOn(IlcESDtrack::kESDpid));

  if (t->IsOn(IlcESDtrack::kESDpid))
  {
    Double_t pid[5];
    t->GetESDpid(pid);
    s += Form("\n[%.2f %.2f %.2f %.2f %.2f]", pid[0], pid[1], pid[2], pid[3], pid[4]);
  }

  return s;
}

//==============================================================================

void esd_track_add_param(IlcEveTrack* track, const IlcExternalTrackParam* tp)
{
  // Add additional track parameters as a path-mark to track.

  if (tp == 0)
    return;

  Double_t pbuf[3], vbuf[3];
  tp->GetXYZ(vbuf);
  tp->GetPxPyPz(pbuf);

  TEvePathMark pm(TEvePathMark::kReference);
  pm.fV.Set(vbuf);
  pm.fP.Set(pbuf);
  track->AddPathMark(pm);
}

//==============================================================================

IlcEveTrack* esd_make_track_DCH(IlcESDtrack *at, IlcESDfriendTrack* aft, TEveTrackList* cont)
{
  // Make a DCH track representation and put it into given container.

  IlcEveTrack* track = new IlcEveTrack(at, cont->GetPropagator());
  track->SetAttLineAttMarker(cont);
  track->SetName(Form("IlcEveTrack %d", at->GetID()));
  track->SetElementTitle(esd_track_title(at));
  track->SetSourceObject(at);

  // Add inner/outer track parameters as start point and pathmark.
  if (at->GetInnerParam()) track->SetStartParams(at->GetInnerParam());
  else return NULL;
  if (aft->GetTPCOut()) esd_track_add_param(track, aft->GetTPCOut());
  else return NULL;

  return track;
}

IlcEveTrack* esd_make_track_TARGET_standalone(IlcESDtrack *at, IlcESDfriendTrack* aft, TEveTrackList* cont)
{
  // Make a TARGET standalone track representation and put it into given container.

  if ( !(!at->IsOn(IlcESDtrack::kTPCin) &&
          at->IsOn(IlcESDtrack::kVXDout)) ) return NULL; //only TARGET standalone
  IlcEveTrack* track = new IlcEveTrack(at, cont->GetPropagator());
  track->SetAttLineAttMarker(cont);
  track->SetName(Form("IlcEveTrack %d", at->GetID()));
  track->SetElementTitle(esd_track_title(at));
  track->SetSourceObject(at);

  // Add inner/outer track parameters as path-marks.
  if (aft->GetVXDOut())
  {
    esd_track_add_param(track, aft->GetVXDOut());
  }
  else return NULL;

  return track;
}

IlcEveTrack* esd_make_track_TARGET(IlcESDtrack *at, IlcESDfriendTrack* aft, TEveTrackList* cont)
{
  // Make a TARGET track representation and put it into given container.

  if ( (!at->IsOn(IlcESDtrack::kTPCin) &&
         at->IsOn(IlcESDtrack::kVXDout)) ) return NULL; //ignore TARGET standalone
  IlcEveTrack* track = new IlcEveTrack(at, cont->GetPropagator());
  track->SetAttLineAttMarker(cont);
  track->SetName(Form("IlcEveTrack %d", at->GetID()));
  track->SetElementTitle(esd_track_title(at));
  track->SetSourceObject(at);

  // Add inner/outer track parameters as path-marks.
  if (aft->GetVXDOut())
  {
    esd_track_add_param(track, aft->GetVXDOut());
  }
  else return NULL;

  return track;
}

IlcEveTrack* esd_make_track(IlcESDtrack *at, TEveTrackList* cont)
{
  // Make a standard track representation and put it into given container.

  // Choose which parameters to use a track's starting point.
  // If gkFixFailedVXDExtr is TRUE (FALSE by default) and
  // if TARGET refit failed, take track parameters at inner DCH radius.

  const IlcExternalTrackParam* tp = at;

  Bool_t innerTaken = kFALSE;
  if ( ! at->IsOn(IlcESDtrack::kVXDrefit) && g_esd_tracks_use_ip_on_failed_target_refit)
  {
    tp = at->GetInnerParam();
    innerTaken = kTRUE;
  }

  IlcEveTrack* track = new IlcEveTrack(at, cont->GetPropagator());
  track->SetAttLineAttMarker(cont);
  track->SetName(Form("IlcEveTrack %d", at->GetID()));
  track->SetElementTitle(esd_track_title(at));
  track->SetSourceObject(at);

  // Add inner/outer track parameters as path-marks.
  if (at->IsOn(IlcESDtrack::kTPCrefit))
  {
    if ( ! innerTaken)
    {
      esd_track_add_param(track, at->GetInnerParam());
    }
    esd_track_add_param(track, at->GetOuterParam());
  }

  return track;
}


//==============================================================================
// esd_tracks()
//==============================================================================

TEveTrackList* esd_tracks_DCH()
{
  IlcESDEvent* esd = IlcEveEventManager::AssertESD();
  IlcESDfriend* esd_friend = IlcEveEventManager::AssertESDfriend();

  TEveTrackList* cont = new TEveTrackList("DCH Tracks");
  cont->SetMainColor(kMagenta);

  esd_track_propagator_setup(cont->GetPropagator(),
			     0.1*esd->GetMagneticField(), 150);

  gEve->AddElement(cont);

  Int_t count = 0;
  for (Int_t n = 0; n < esd->GetNumberOfTracks(); ++n)
  {
    ++count;
    if (!esd->GetTrack(n)) continue;
    if (!esd_friend->GetTrack(n)) continue;
    IlcEveTrack* track = esd_make_track_DCH(esd->GetTrack(n), esd_friend->GetTrack(n), cont);
    if (!track) continue;

    cont->AddElement(track);
  }
  cont->SetTitle(Form("N=%d", count));
  cont->MakeTracks();

  gEve->Redraw3D();

  return cont;
}

TEveTrackList* esd_tracks_TARGET()
{
  IlcESDEvent* esd = IlcEveEventManager::AssertESD();
  IlcESDfriend* esd_friend = IlcEveEventManager::AssertESDfriend();

  TEveTrackList* cont = new TEveTrackList("TARGET Tracks");
  cont->SetMainColor(kMagenta+3);

  esd_track_propagator_setup(cont->GetPropagator(),
			     0.1*esd->GetMagneticField(), 150);
  cont->GetPropagator()->SetMaxR(85.0);
  cont->SetLineWidth(1);

  gEve->AddElement(cont);

  Int_t count = 0;
  for (Int_t n = 0; n < esd->GetNumberOfTracks(); ++n)
  {
    ++count;
    if (!esd->GetTrack(n)) continue;
    if (!esd_friend->GetTrack(n)) continue;
    IlcEveTrack* track = esd_make_track_TARGET(esd->GetTrack(n), esd_friend->GetTrack(n), cont);
    if (!track) continue;

    cont->AddElement(track);
  }
  cont->SetTitle(Form("N=%d", count));
  cont->MakeTracks();

  gEve->Redraw3D();

  return cont;
}

TEveTrackList* esd_tracks_TARGET_standalone()
{
  IlcESDEvent* esd = IlcEveEventManager::AssertESD();
  IlcESDfriend* esd_friend = IlcEveEventManager::AssertESDfriend();

  TEveTrackList* cont = new TEveTrackList("TARGET Standalone Tracks");
  cont->SetMainColor(kBlue);

  esd_track_propagator_setup(cont->GetPropagator(),
			     0.1*esd->GetMagneticField(), 150);
  cont->GetPropagator()->SetMaxR(85.0);
  cont->SetLineWidth(1);

  gEve->AddElement(cont);

  Int_t count = 0;
  for (Int_t n = 0; n < esd->GetNumberOfTracks(); ++n)
  {
    ++count;
    if (!esd->GetTrack(n)) continue;
    if (!esd_friend->GetTrack(n)) continue;
    IlcEveTrack* track = esd_make_track_TARGET_standalone(esd->GetTrack(n), esd_friend->GetTrack(n), cont);
    if (!track) continue;

    cont->AddElement(track);
  }
  cont->SetTitle(Form("N=%d", count));
  cont->MakeTracks();

  gEve->Redraw3D();

  return cont;
}

TEveTrackList* esd_tracks()
{
  IlcESDEvent* esd = IlcEveEventManager::AssertESD();

  TEveTrackList* cont = new TEveTrackList("ESD Tracks");
  cont->SetMainColor(6);

  esd_track_propagator_setup(cont->GetPropagator(),
			     0.1*esd->GetMagneticField(), 150);

  gEve->AddElement(cont);

  Int_t count = 0;
  for (Int_t n = 0; n < esd->GetNumberOfTracks(); ++n)
  {
    ++count;
    IlcEveTrack* track = esd_make_track(esd->GetTrack(n), cont);

    cont->AddElement(track);
  }
  cont->SetTitle(Form("N=%d", count));
  cont->MakeTracks();

  gEve->Redraw3D();

  return cont;
}

TEveTrackList* esd_tracks_MI()
{
  IlcESDEvent* esd = IlcEveEventManager::AssertESD();

  TEveTrackList* cont = new TEveTrackList("ESD Tracks MI");
  cont->SetLineColor(5);
  gEve->AddElement(cont);

  Int_t count = 0;
  for (Int_t n = 0; n < esd->GetNumberOfTracks(); ++n)
  {
    ++count;
    IlcESDtrack* at = esd->GetTrack(n);
    IlcEveTrack* l = new IlcEveTrack(at, cont->GetPropagator());
    l->SetName(Form("ESDTrackMI %d", at->GetID()));
    l->SetElementTitle(esd_track_title(at));
    l->SetAttLineAttMarker(cont);
    l->SetSourceObject(at);

    at->FillPolymarker(l, esd->GetMagneticField(), 0, 250, 5);

    l->SetLockPoints(kTRUE);
    cont->AddElement(l);
  }
  cont->SetTitle(Form("N=%d", count));

  gEve->Redraw3D();

  return cont;
}


//==============================================================================
// esd_tracks_from_array()
//==============================================================================

TEveTrackList* esd_tracks_from_array(TCollection* col, IlcESDEvent* esd=0)
{
  // Retrieves IlcESDTrack's from collection.
  // See example usage with IlcAnalysisTrackCuts in the next function.

  if (esd == 0) esd = IlcEveEventManager::AssertESD();

  TEveTrackList* cont = new TEveTrackList("ESD Tracks");
  cont->SetMainColor(6);

  esd_track_propagator_setup(cont->GetPropagator(),
			     0.1*esd->GetMagneticField(), 150);

  gEve->AddElement(cont);

  Int_t    count = 0;
  TIter    next(col);
  TObject *obj;
  while ((obj = next()) != 0)
  {
    if (obj->IsA()->InheritsFrom("IlcESDtrack") == kFALSE)
    {
      Warning("esd_tracks_from_array", "Object '%s', '%s' is not an IlcESDtrack.",
	      obj->GetName(), obj->GetTitle());
      continue;
    }

    ++count;
    IlcESDtrack* at = reinterpret_cast<IlcESDtrack*>(obj);

    IlcEveTrack* track = esd_make_track(at, cont);
    cont->AddElement(track);
  }
  cont->SetTitle(Form("N=%d", count));
  cont->MakeTracks();

  gEve->Redraw3D();

  return cont;
}

void esd_tracks_ilcanalcuts_demo()
{
  IlcESDEvent* esd = IlcEveEventManager::AssertESD();

  IlcESDtrackCuts atc;
  atc.SetPtRange(0.1, 5);
  atc.SetRapRange(-1, 1);

  esd_tracks_from_array(atc.GetAcceptedTracks(esd), esd);
}


//==============================================================================
// esd_tracks_by_category
//==============================================================================

Float_t get_sigma_to_vertex(IlcESDtrack* esdTrack)
{
  // Taken from: PWG0/esdTrackCuts/IlcESDtrackCuts.cxx
  // Float_t IlcESDtrackCuts::GetSigmaToVertex(IlcESDtrack* esdTrack)

  Float_t b[2];
  Float_t bRes[2];
  Float_t bCov[3];
  esdTrack->GetImpactParameters(b,bCov);
  if (bCov[0] <= 0 || bCov[2] <= 0)
  {
    printf("Estimated b resolution lower or equal zero!\n");
    bCov[0] = bCov[2] = 0;
  }
  bRes[0] = TMath::Sqrt(bCov[0]);
  bRes[1] = TMath::Sqrt(bCov[2]);

  // -----------------------------------
  // How to get to a n-sigma cut?
  //
  // The accumulated statistics from 0 to d is
  //
  // ->  Erf(d/Sqrt(2)) for a 1-dim gauss (d = n_sigma)
  // ->  1 - Exp(-d**2) for a 2-dim gauss (d*d = dx*dx + dy*dy != n_sigma)
  //
  // It means that for a 2-dim gauss: n_sigma(d) = Sqrt(2)*ErfInv(1 - Exp((-x**2)/2)
  // Can this be expressed in a different way?

  if (bRes[0] == 0 || bRes[1] == 0)
    return -1;

  Float_t d = TMath::Sqrt(TMath::Power(b[0]/bRes[0],2) + TMath::Power(b[1]/bRes[1],2));

  // stupid rounding problem screws up everything:
  // if d is too big, TMath::Exp(...) gets 0, and TMath::ErfInverse(1) that should be infinite, gets 0 :(
  if (TMath::Exp(-d * d / 2) < 1e-10)
    return 1000;

  d = TMath::ErfInverse(1 - TMath::Exp(-d * d / 2)) * TMath::Sqrt(2);
  return d;
}

TEveElementList* esd_tracks_by_category()
{
   // Import ESD tracks, separate them into several containers
   // according to primary-vertex cut and TARGET&DCH refit status.

   IlcESDEvent* esd = IlcEveEventManager::AssertESD();

   TEveElementList* cont = new TEveElementList("ESD Tracks by category");
   gEve->AddElement(cont);

   const Int_t   nCont = 9;
   const Float_t maxR  = 150;
   const Float_t magF  = 0.1*esd->GetMagneticField();

   TEveTrackList *tl[nCont];
   Int_t          tc[nCont];
   Int_t          count = 0;

   tl[0] = new TEveTrackList("Sigma < 3");
   tc[0] = 0;
   esd_track_propagator_setup(tl[0]->GetPropagator(), magF, maxR);
   tl[0]->SetMainColor(3);
   cont->AddElement(tl[0]);

   tl[1] = new TEveTrackList("3 < Sigma < 5");
   tc[1] = 0;
   esd_track_propagator_setup(tl[1]->GetPropagator(), magF, maxR);
   tl[1]->SetMainColor(7);
   cont->AddElement(tl[1]);

   tl[2] = new TEveTrackList("5 < Sigma");
   tc[2] = 0;
   esd_track_propagator_setup(tl[2]->GetPropagator(), magF, maxR);
   tl[2]->SetMainColor(46);
   cont->AddElement(tl[2]);

   tl[3] = new TEveTrackList("no TARGET refit; Sigma < 5");
   tc[3] = 0;
   esd_track_propagator_setup(tl[3]->GetPropagator(), magF, maxR);
   tl[3]->SetMainColor(41);
   cont->AddElement(tl[3]);

   tl[4] = new TEveTrackList("no TARGET refit; Sigma > 5");
   tc[4] = 0;
   esd_track_propagator_setup(tl[4]->GetPropagator(), magF, maxR);
   tl[4]->SetMainColor(48);
   cont->AddElement(tl[4]);

   tl[5] = new TEveTrackList("no DCH refit");
   tc[5] = 0;
   esd_track_propagator_setup(tl[5]->GetPropagator(), magF, maxR);
   tl[5]->SetMainColor(kRed);
   cont->AddElement(tl[5]);

   tl[6] = new TEveTrackList("TARGET ncl>=3 & SPD Inner");
   tc[6] = 0;
   esd_track_propagator_setup(tl[6]->GetPropagator(), magF, maxR);
   tl[6]->SetMainColor(kGreen);
   cont->AddElement(tl[6]);

   tl[7] = new TEveTrackList("TARGET ncl>=3 & b<3 cm");
   tc[7] = 0;
   esd_track_propagator_setup(tl[7]->GetPropagator(), magF, maxR);
   tl[7]->SetMainColor(kMagenta - 9);
   cont->AddElement(tl[7]);

   tl[8] = new TEveTrackList("TARGET others");
   tc[8] = 0;
   esd_track_propagator_setup(tl[8]->GetPropagator(), magF, maxR);
   tl[8]->SetMainColor(kRed + 2);
   cont->AddElement(tl[8]);

   for (Int_t n = 0; n < esd->GetNumberOfTracks(); ++n)
   {
      IlcESDtrack* at = esd->GetTrack(n);

      Float_t s  = get_sigma_to_vertex(at);
      Int_t   ti;
      if      (s <  3) ti = 0;
      else if (s <= 5) ti = 1;
      else             ti = 2;

      Int_t    nclvxd;
      Double_t dtobeam;

      if (at->IsOn(IlcESDtrack::kVXDin) && ! at->IsOn(IlcESDtrack::kTPCin))
      {
         UChar_t vxdclmap = at->GetVXDClusterMap();
         Bool_t  spdinner = (vxdclmap & 3) != 0;

         nclvxd = 0;
         for (Int_t iter = 0; iter < 6; ++iter)
            if (vxdclmap & (1 << iter)) nclvxd ++;

         Double_t xyz[3];
         at->GetXYZ(xyz);
         dtobeam = TMath::Hypot(xyz[0], xyz[1]);

         if ((nclvxd >= 3) && (spdinner))
            ti = 6;
         else if ((nclvxd >= 3) && (dtobeam < 3.0))
            ti = 7;
         else
            ti = 8;
      }
      else if (at->IsOn(IlcESDtrack::kTPCin) && ! at->IsOn(IlcESDtrack::kTPCrefit))
      {
         ti = 5;
      }
      else if ( ! at->IsOn(IlcESDtrack::kVXDrefit))
      {
         ti = (ti == 2) ? 4 : 3;
      }

      TEveTrackList* tlist = tl[ti];
      ++tc[ti];
      ++count;

      IlcEveTrack* track = esd_make_track(at, tlist);
      if (ti<6)
         track->SetName(Form("ESD Track idx=%d, sigma=%5.3f", at->GetID(), s));
      else
         track->SetName(Form("ESD Track idx=%d, dxy=%5.3f cl=%i", at->GetID(), dtobeam, nclvxd));
      tlist->AddElement(track);
   }

   for (Int_t ti = 0; ti < nCont; ++ti)
   {
      TEveTrackList* tlist = tl[ti];
      tlist->SetName(Form("%s [%d]", tlist->GetName(), tlist->NumChildren()));
      tlist->SetTitle(Form("N tracks=%d", tc[ti]));

      tlist->MakeTracks();

      //    Bool_t good_cont = ti <= 1;
      Bool_t good_cont = ((ti == 6) || (ti == 7));
      if (IlcEveTrackCounter::IsActive())
      {
         IlcEveTrackCounter::fgInstance->RegisterTracks(tlist, good_cont);
      }
      else
      {
         if ( ! good_cont)
            tlist->SetLineStyle(6);
      }
   }
   cont->SetTitle(Form("N all tracks = %d", count));
   // ??? The following does not always work:
   cont->FindListTreeItem(gEve->GetListTree())->SetOpen(kTRUE);

   gEve->Redraw3D();

   return cont;
}


//==============================================================================
// esd_tracks_by_anal_cuts
//==============================================================================

IlcESDtrackCuts* g_esd_tracks_anal_cuts = 0;

TEveElementList* esd_tracks_by_anal_cuts()
{
  IlcESDEvent* esd = IlcEveEventManager::AssertESD();

//   if (g_esd_tracks_anal_cuts == 0)
//   {
//     gSystem->Load("libPWGUDbase");
//     gROOT->ProcessLine(".L $ILC_ROOT/PWGUD/CreateStandardCuts.C");
//     Int_t mode = IlcPWG0Helper::kTPC;
//     if (TMath::Abs(esd->GetMagneticField()) > 0.01)
//       mode |= IlcPWG0Helper::kFieldOn;
//     gROOT->ProcessLine(Form("g_esd_tracks_anal_cuts = CreateTrackCuts(%d, kFALSE)", mode));
//   }

  TEveElementList* cont = new TEveElementList("ESD Tracks by Analysis Cuts");
  gEve->AddElement(cont);

  const Int_t   nCont = 2;
  const Float_t maxR  = 150;
  const Float_t magF  = 0.1*esd->GetMagneticField();

  TEveTrackList *tl[nCont];
  Int_t          tc[nCont];
  Int_t          count = 0;

  tl[0] = new TEveTrackList("Passed");
  tc[0] = 0;
  esd_track_propagator_setup(tl[0]->GetPropagator(), magF, maxR);
  tl[0]->SetMainColor(3);
  cont->AddElement(tl[0]);

  tl[1] = new TEveTrackList("Rejected");
  tc[1] = 0;
  esd_track_propagator_setup(tl[1]->GetPropagator(), magF, maxR);
  tl[1]->SetMainColor(kRed);
  cont->AddElement(tl[1]);

  for (Int_t n = 0; n < esd->GetNumberOfTracks(); ++n)
  {
    IlcESDtrack* at = esd->GetTrack(n);

    Float_t s  = get_sigma_to_vertex(at);
    Int_t   ti = (g_esd_tracks_anal_cuts->AcceptTrack(at)) ? 0 : 1;

    TEveTrackList* tlist = tl[ti];
    ++tc[ti];
    ++count;

    IlcEveTrack* track = esd_make_track(at, tlist);
    track->SetName(Form("ESD Track idx=%d, sigma=%5.3f", at->GetID(), s));
    tlist->AddElement(track);
  }

  for (Int_t ti = 0; ti < nCont; ++ti)
  {
    TEveTrackList* tlist = tl[ti];
    tlist->SetName(Form("%s [%d]", tlist->GetName(), tlist->NumChildren()));
    tlist->SetTitle(Form("N tracks=%d", tc[ti]));

    tlist->MakeTracks();

    Bool_t good_cont = ti < 1;
    if (IlcEveTrackCounter::IsActive())
    {
      IlcEveTrackCounter::fgInstance->RegisterTracks(tlist, good_cont);
    }
    else
    {
      if ( ! good_cont)
        tlist->SetLineStyle(6);
    }
  }
  cont->SetTitle(Form("N all tracks = %d", count));
  // ??? The following does not always work:
  cont->FindListTreeItem(gEve->GetListTree())->SetOpen(kTRUE);

  gEve->Redraw3D();

  return cont;
}
