// $Id: vplot_tpc.C 55699 2012-04-11 10:01:56Z hristov $
// Main authors: Adam Jacholkowski & Matevz Tadel: 2009

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
#include <TGLViewer.h>
#include <TGLCameraOverlay.h>
#include <TEveManager.h>
#include <TEveBrowser.h>
#include <TEveViewer.h>
#include <TEveScene.h>
#include <TEvePointSet.h>

#include <IlcCluster.h>
#include <IlcESDEvent.h>
#include <IlcESDVertex.h>
#include <IlcRunLoader.h>
#include <IlcTPCClustersRow.h>
#include <IlcEveEventManager.h>
#endif

TEveViewer *gVPTPCView   = 0;
TEveScene  *gVPTPCScene  = 0;

TEvePointSet* vplot_tpc(TEveElement* cont=0, Float_t maxR=270)
{
  if (gVPTPCView == 0)
  {
    TEveWindowSlot *slot    = 0;
    TEveBrowser    *browser = gEve->GetBrowser();

    slot = TEveWindow::CreateWindowInTab(browser->GetTabRight());
    slot->MakeCurrent();
    gVPTPCView  = gEve->SpawnNewViewer("V-Plot", "");
    gVPTPCScene = gEve->SpawnNewScene("V-Plot", "Scene holding elements for the V-Plot TPC.");
    gVPTPCView->AddScene(gVPTPCScene);

    TGLViewer *glv = gVPTPCView->GetGLViewer();
    glv->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
    glv->ResetCamerasAfterNextUpdate();

    TGLCameraOverlay* co = glv->GetCameraOverlay();
    co->SetShowOrthographic(true); //(false);
    co->SetOrthographicMode(TGLCameraOverlay::kAxis); // ::kPlaneIntersect or ::kBar
  }

  const Int_t kMaxCl=100*160;

  IlcEveEventManager::AssertGeometry();

  Double_t pvert[3] = { 0, 0, 0 };
  if (IlcEveEventManager::HasESD())
  {
    IlcESDEvent  *esd  = IlcEveEventManager::AssertESD();
    const IlcESDVertex *tpcv = esd->GetPrimaryVertexTPC();
    if (tpcv->GetStatus())
      tpcv->GetXYZ(pvert);
    else
      Info("vplot_tpc", "Primary vertex TPC not available, using 0.");
  }
  else
  {
      Info("vplot_tpc", "ESD not available, using 0 for primary vertex.");
  }

  IlcRunLoader* rl = IlcEveEventManager::AssertRunLoader();
  rl->LoadRecPoints("TPC");

  TTree *cTree = rl->GetTreeR("TPC", false);
  if (cTree == 0)
    return 0;

  IlcTPCClustersRow *clrow = new IlcTPCClustersRow();
  clrow->SetClass("IlcTPCclusterMI");
  clrow->SetArray(kMaxCl);
  cTree->SetBranchAddress("Segment", &clrow);

  TEvePointSet* vplot = new TEvePointSet(kMaxCl);
  vplot->SetOwnIds(kTRUE);

  const Float_t phimin = -3.15;
  const Float_t phimax =  3.15;
  const Float_t etamin = -1.2;
  const Float_t etamax =  1.2;
  const Float_t vconst =  0.0003;

  Float_t rhomax = 246.6;
  Float_t rholim, rhoup;
  Float_t zmax = 250.0;
  Float_t rho, eta, phi, theta;
  Float_t r3d, r3dmax, r3d1, r3d2;
  //
  Float_t maxRsqr = maxR*maxR;

  Int_t nentr = (Int_t) cTree->GetEntries();
  for (Int_t i = 0; i < nentr; ++i)
  {
    if (!cTree->GetEvent(i)) continue;

    TClonesArray *cl = clrow->GetArray();
    Int_t ncl = cl->GetEntriesFast();

    while (ncl--)
    {
      IlcCluster *c = (IlcCluster*) cl->UncheckedAt(ncl);
      Float_t g[3]; //global coordinates
      c->GetGlobalXYZ(g);
      g[0] -= pvert[0];
      g[1] -= pvert[1];
      g[2] -= pvert[2];
      if (g[0]*g[0] + g[1]*g[1] < maxRsqr)
      {
        phi    =  TMath::ATan2(g[1], g[0]);
        rho    =  TMath::Sqrt(g[0]*g[0] + g[1]*g[1]);
        theta  =  TMath::ATan2(rho, g[2]);
        eta    = -1.0*TMath::Log(TMath::Tan(0.5*theta));
        rhoup  =  zmax*rho / TMath::Abs(g[2]);
        rholim =  TMath::Min(rhoup,rhomax);
	// a version using rather r3d
        r3d    =  TMath::Sqrt(rho*rho + g[2]*g[2]);
        r3d1   =  rhomax / TMath::Sin(theta);
        r3d2   =  TMath::Abs(zmax/TMath::Cos(theta));
        r3dmax =  TMath::Min(r3d1, r3d2);

        if (eta>etamin && eta<etamax && phi>phimin && phi<phimax)
	{
	  Float_t deta = vconst*(r3dmax - r3d);

	  vplot->SetNextPoint(eta + deta, phi, 0);
	  vplot->SetNextPoint(eta - deta, phi, 0);
        }
      }
    }
    cl->Clear();
  }

  delete clrow;

  rl->UnloadRecPoints("TPC");

  if (vplot->Size() == 0 && gEve->GetKeepEmptyCont() == kFALSE)
  {
    Warning("vplot_tpc.C", "No TPC clusters were found.");
    delete vplot;
    return 0;
  }

  vplot->SetName("V=Plot TPC");
  vplot->SetTitle(Form("N=%d", vplot->Size() / 2));

  vplot->SetMainColor (kOrange);
  vplot->SetMarkerSize(0.2);
  vplot->SetMarkerStyle(1);
  // vplot->ApplyVizTag("V-Plot TPC", "V-Plot");

  if (cont)
  {
    cont->AddElement(vplot);
  }
  else
  {
    gVPTPCScene->AddElement(vplot);
  }
  IlcEveEventManager::RegisterTransient(vplot);

  gEve->Redraw3D();

  return vplot;
}
