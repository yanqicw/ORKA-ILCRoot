// $Id: primary_vertex.C 55699 2012-04-11 10:01:56Z hristov $
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
#include <TEveManager.h>
#include <TEveCompound.h>
#include <TEveStraightLineSet.h>

#include <IlcESDEvent.h>
#include <IlcESDVertex.h>
#include <IlcEveEventManager.h>
#endif

//==============================================================================
// Utilities
//==============================================================================

TEveCompound* assert_vertex_parent(const TString& name, Color_t col)
{
  // !!! TEveCompound should have viz-db support ... add in root, then fix here,
  // that is, remove the color var and pass viz-db tag.

  TEveCompound* parent = dynamic_cast<TEveCompound*>
    (IlcEveEventManager::GetCurrent()->FindChild(name));
  if (parent == 0)
  {
    parent = new TEveCompound(name);
    parent->OpenCompound();
    parent->SetMainColor(col);
    IlcEveEventManager::GetMaster()->AddElement(parent);
  }
  return parent;
}


//==============================================================================
// Functions to make a cross / ellipse / box
//==============================================================================

TEveStraightLineSet*
make_vertex_cross(const IlcESDVertex* v, Bool_t use_sigma, Float_t fx, Float_t fy, Float_t fz)
{
  Double_t x[3], e[3];
  v->GetXYZ(x); v->GetSigmaXYZ(e);

  TEveStraightLineSet* ls = new TEveStraightLineSet("Cross");
  TString title;
  if (use_sigma)
  {
    e[0] *= fx; e[1] *= fy; e[2] *= fz;
    title += Form("+- %.1f*sigma_x, %.1f*sigma_y, %.1f*sigma_z", fx, fy, fz);
  }
  else
  {
    e[0] = fx; e[1] = fy; e[2] = fz;
    title += Form("+- %.1f cm x %.1f cm x %.1f cm", fx, fy, fz);
  }
  title += Form("\nx=%.5f, y=%.5f, z=%.5f\nsx=%.5f, sy=%.5f, sz=%.5f",
		x[0], x[1], x[2], e[0], e[1], e[2]);
  ls->SetTitle(title);

  ls->AddLine(e[0], 0,    0,   -e[0], 0,    0);
  ls->AddLine(0,    e[1], 0,    0,   -e[1], 0);
  ls->AddLine(0,    0,    e[2], 0,    0,   -e[2]);

  ls->RefMainTrans().SetPos(x);
  return ls;
}

TEveStraightLineSet*
make_vertex_ellipse(const IlcESDVertex* v, Bool_t use_sigma, Float_t fx, Float_t fy, Float_t fz)
{
  Double_t x[3], e[3];
  v->GetXYZ(x); v->GetSigmaXYZ(e);

  TEveStraightLineSet* ls = new TEveStraightLineSet("Ellipse");
  TString title;
  if (use_sigma)
  {
    e[0] *= fx; e[1] *= fy; e[2] *= fz;
    title += Form("+- %.1f*sigma_x, %.1f*sigma_y, %.1f sigma_z", fx, fy, fz);
  }
  else
  {
    e[0] = fx; e[1] = fy; e[2] = fz;
    title += Form("+- %.1f cm x %.1f cm x %.1f cm", fx, fy, fz);
  }
  title += Form("\nx=%.5f, y=%.5f, z=%.5f\nsx=%.5f, sy=%.5f, sz=%.5f",
		x[0], x[1], x[2], e[0], e[1], e[2]);
  ls->SetTitle(title);

  const Int_t   N = 32;
  const Float_t S = 2*TMath::Pi()/N;

  Float_t a = e[0], b = e[1];
  for (Int_t i = 0; i<N; i++)
    ls->AddLine(a*TMath::Cos(i*S)  , b*TMath::Sin(i*S)  , 0,
		a*TMath::Cos(i*S+S), b*TMath::Sin(i*S+S), 0);

  a = e[0]; b = e[2];
  for (Int_t i = 0; i<N; i++)
    ls->AddLine(a*TMath::Cos(i*S)  , 0, b*TMath::Sin(i*S),
		a*TMath::Cos(i*S+S), 0, b*TMath::Sin(i*S+S));

  a = e[1]; b = e[2];
  for (Int_t i = 0; i<N; i++)
    ls->AddLine(0, a*TMath::Cos(i*S)  ,  b*TMath::Sin(i*S),
		0, a*TMath::Cos(i*S+S),  b*TMath::Sin(i*S+S));

  ls->RefMainTrans().SetPos(x);
  return ls;
}

TEveStraightLineSet*
make_vertex_box(const IlcESDVertex* v, Bool_t use_sigma, Float_t fx, Float_t fy, Float_t fz)
{
  Double_t x[3], e[3];
  v->GetXYZ(x); v->GetSigmaXYZ(e);

  TEveStraightLineSet* ls = new TEveStraightLineSet("Box");
  TString title;
  if (use_sigma)
  {
    e[0] *= fx; e[1] *= fy; e[2] *= fz;
    title += Form("+- %.1f*sigma_x, %.1f*sigma_y, %.1f*sigma_z", fx, fy, fz);
  }
  else
  {
    e[0] = fx; e[1] = fy; e[2] = fz;
    title += Form("+- %.1f cm x %.1f cm x %.1f cm", fx, fy, fz);
  }
  title += Form("\nx=%.5f, y=%.5f, z=%.5f\nsx=%.5f, sy=%.5f, sz=%.5f",
		x[0], x[1], x[2], e[0], e[1], e[2]);
  ls->SetTitle(title);

  // pos z
  ls->AddLine( e[0],  e[1],  e[2],  e[0], -e[1],  e[2]);
  ls->AddLine( e[0], -e[1],  e[2], -e[0], -e[1],  e[2]);
  ls->AddLine(-e[0], -e[1],  e[2], -e[0],  e[1],  e[2]);
  ls->AddLine(-e[0],  e[1],  e[2],  e[0],  e[1],  e[2]);
  // lines along z
  ls->AddLine( e[0],  e[1],  e[2],  e[0],  e[1], -e[2]);
  ls->AddLine( e[0], -e[1],  e[2],  e[0], -e[1], -e[2]);
  ls->AddLine(-e[0], -e[1],  e[2], -e[0], -e[1], -e[2]);
  ls->AddLine(-e[0],  e[1],  e[2], -e[0],  e[1], -e[2]);
  // neg z
  ls->AddLine( e[0],  e[1], -e[2],  e[0], -e[1], -e[2]);
  ls->AddLine( e[0], -e[1], -e[2], -e[0], -e[1], -e[2]);
  ls->AddLine(-e[0], -e[1], -e[2], -e[0],  e[1], -e[2]);
  ls->AddLine(-e[0],  e[1], -e[2],  e[0],  e[1], -e[2]);

  ls->RefMainTrans().SetPos(x);
  return ls;
}


//==============================================================================
// Element making functions
//==============================================================================

TEveStraightLineSet*
primary_vertex(Bool_t use_sigma=kTRUE, Float_t fx=1, Float_t fy=1, Float_t fz=1)
{
  IlcESDEvent  *esd = IlcEveEventManager::AssertESD();
  const IlcESDVertex *pv  = esd->GetPrimaryVertex();
  if ( ! pv->GetStatus()) {
    Info("primary_vertex", "Primary vertex not available.");
    return 0;
  }

  TEveStraightLineSet* ls = make_vertex_cross(pv, use_sigma, fx, fy, fz);
  ls->ApplyVizTag("REC PVTX");
  assert_vertex_parent("Primary Vertex", 7)->AddElement(ls);
  gEve->Redraw3D();
  return ls;
}

TEveStraightLineSet*
primary_vertex_spd(Bool_t use_sigma=kTRUE, Float_t fx=1, Float_t fy=1, Float_t fz=1)
{
  IlcESDEvent  *esd  = IlcEveEventManager::AssertESD();
  const IlcESDVertex *spdv = esd->GetPrimaryVertexSPD();
  if ( ! spdv->GetStatus()) {
    Info("primary_vertex_spd", "Primary vertex SPD not available.");
    return 0;
  }

  TEveStraightLineSet* ls = make_vertex_cross(spdv, use_sigma, fx, fy, fz);
  ls->ApplyVizTag("REC PVTX SPD");
  assert_vertex_parent("Primary Vertex SPD", 6)->AddElement(ls);
  gEve->Redraw3D();
  return ls;
}

TEveStraightLineSet*
primary_vertex_tpc(Bool_t use_sigma=kTRUE, Float_t fx=1, Float_t fy=1, Float_t fz=1)
{
  IlcESDEvent  *esd  = IlcEveEventManager::AssertESD();
  const IlcESDVertex *tpcv = esd->GetPrimaryVertexTPC();
  if ( ! tpcv->GetStatus()) {
    Info("primary_vertex_tpc", "Primary vertex TPC not available.");
    return 0;
  }

  TEveStraightLineSet* ls = make_vertex_cross(tpcv, use_sigma, fx, fy, fz);
  ls->ApplyVizTag("REC PVTX TPC");
  assert_vertex_parent("Primary Vertex TPC", 5)->AddElement(ls);
  gEve->Redraw3D();
  return ls;
}

//------------------------------------------------------------------------------
// Ellipse
//------------------------------------------------------------------------------

TEveStraightLineSet*
primary_vertex_ellipse(Bool_t use_sigma=kTRUE, Float_t fx=30, Float_t fy=30, Float_t fz=10)
{
  IlcESDEvent  *esd = IlcEveEventManager::AssertESD();
  const IlcESDVertex *pv  = esd->GetPrimaryVertex();
  if ( ! pv->GetStatus()) {
    Info("primary_vertex_ellipse", "Primary vertex not available.");
    return 0;
  }

  TEveStraightLineSet* ls = make_vertex_ellipse(pv, use_sigma, fx, fy, fz);
  ls->ApplyVizTag("REC PVTX Ellipse");
  assert_vertex_parent("Primary Vertex", 7)->AddElement(ls);
  gEve->Redraw3D();
  return ls;
}

TEveStraightLineSet*
primary_vertex_ellipse_spd(Bool_t use_sigma=kTRUE, Float_t fx=30, Float_t fy=30, Float_t fz=10)
{
  IlcESDEvent  *esd  = IlcEveEventManager::AssertESD();
  const IlcESDVertex *spdv = esd->GetPrimaryVertexSPD();
  if ( ! spdv->GetStatus()) {
    Info("primary_vertex_ellipse_spd", "Primary vertex SPD not available.");
    return 0;
  }

  TEveStraightLineSet* ls = make_vertex_ellipse(spdv, use_sigma, fx, fy, fz);
  ls->ApplyVizTag("REC PVTX Ellipse SPD");
  assert_vertex_parent("Primary Vertex SPD", 6)->AddElement(ls);
  gEve->Redraw3D();
  return ls;
}

TEveStraightLineSet*
primary_vertex_ellipse_tpc(Bool_t use_sigma=kTRUE, Float_t fx=30, Float_t fy=30, Float_t fz=10)
{
  IlcESDEvent  *esd  = IlcEveEventManager::AssertESD();
  const IlcESDVertex *tpcv = esd->GetPrimaryVertexTPC();
  if ( ! tpcv->GetStatus()) {
    Info("primary_vertex_ellipse_tpc", "Primary vertex TPC not available.");
    return 0;
  }

  TEveStraightLineSet* ls = make_vertex_ellipse(tpcv, use_sigma, fx, fy, fz);
  ls->ApplyVizTag("REC PVTX Ellipse TPC");
  assert_vertex_parent("Primary Vertex TPC", 5)->AddElement(ls);
  gEve->Redraw3D();
  return ls;
}

//------------------------------------------------------------------------------
// Box
//------------------------------------------------------------------------------

TEveStraightLineSet*
primary_vertex_box(Bool_t use_sigma=kTRUE, Float_t fx=30, Float_t fy=30, Float_t fz=10)
{
  IlcESDEvent  *esd = IlcEveEventManager::AssertESD();
  const IlcESDVertex *pv  = esd->GetPrimaryVertex();
  if ( ! pv->GetStatus()) {
    Info("primary_vertex_box", "Primary vertex not available.");
    return 0;
  }

  TEveStraightLineSet* ls = make_vertex_box(pv, use_sigma, fx, fy, fz);
  ls->ApplyVizTag("REC PVTX Box");
  assert_vertex_parent("Primary Vertex", 7)->AddElement(ls);
  gEve->Redraw3D();
  return ls;
}

TEveStraightLineSet*
primary_vertex_box_spd(Bool_t use_sigma=kTRUE, Float_t fx=30, Float_t fy=30, Float_t fz=10)
{
  IlcESDEvent  *esd  = IlcEveEventManager::AssertESD();
  const IlcESDVertex *spdv = esd->GetPrimaryVertexSPD();
  if ( ! spdv->GetStatus()) {
    Info("primary_vertex_box_spd", "Primary vertex SPD not available.");
    return 0;
  }

  TEveStraightLineSet* ls = make_vertex_box(spdv, use_sigma, fx, fy, fz);
  ls->ApplyVizTag("REC PVTX Box SPD");
  assert_vertex_parent("Primary Vertex SPD", 6)->AddElement(ls);
  gEve->Redraw3D();
  return ls;
}

TEveStraightLineSet*
primary_vertex_box_tpc(Bool_t use_sigma=kTRUE, Float_t fx=30, Float_t fy=30, Float_t fz=10)
{
  IlcESDEvent  *esd  = IlcEveEventManager::AssertESD();
  const IlcESDVertex *tpcv = esd->GetPrimaryVertexTPC();
  if ( ! tpcv->GetStatus()) {
    Info("primary_vertex_box_tpc", "Primary vertex TPC not available.");
    return 0;
  }

  TEveStraightLineSet* ls = make_vertex_box(tpcv, use_sigma, fx, fy, fz);
  ls->ApplyVizTag("REC PVTX Box TPC");
  assert_vertex_parent("Primary Vertex TPC", 5)->AddElement(ls);
  gEve->Redraw3D();
  return ls;
}
