// Author: Stefano Carrazza 2010

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
#include <TGLWidget.h>
#include <TH2.h>
#include <TMath.h>
#include <TTree.h>
#include <TEveBrowser.h>
#include <TEveCalo.h>
#include <TEveCaloData.h>
#include <TEveCaloLegoOverlay.h>
#include <TEveLegoEventHandler.h>
#include <TEveManager.h>
#include <TEveProjectionManager.h>
#include <TEveProjectionAxes.h>
#include <TEveScene.h>
#include <TEveTrans.h>
#include <TEveViewer.h>
#include <TEveWindow.h>

#include <IlcESDEvent.h>
#include <IlcEveEventManager.h>
#endif

double pi = TMath::Pi();
TEveViewer *g_histo2d_all_events_v0 = 0;
TEveViewer *g_histo2d_all_events_v1 = 0;
TEveViewer *g_histo2d_all_events_v2 = 0;
TEveViewer *g_histo2d_all_events_v3 = 0;
TEveScene  *g_histo2d_all_events_s0 = 0;
TEveScene  *g_histo2d_all_events_s1 = 0;
TEveScene  *g_histo2d_all_events_s2 = 0;
TEveScene  *g_histo2d_all_events_s3 = 0;
TEveCaloLegoOverlay* g_histo2d_all_events_lego_overlay = 0;
TEveWindowSlot* g_histo2d_all_events_slot = 0;

Double_t GetPhi(Double_t phi);
TEveCaloLego* CreateHistoLego(TEveCaloData* data, TEveWindowSlot* slot);
TEveCalo3D* Create3DView(TEveCaloData* data, TEveWindowSlot* slot);
void CreateProjections(TEveCaloData* data, TEveCalo3D *calo3d, TEveWindowSlot* slot1, TEveWindowSlot* slot2);

TEveCaloDataHist* histo2d_all_events()
{

   TEveCaloDataHist* data_t;
   
   if ( g_histo2d_all_events_slot == 0 ) {
      Info("histo2d_all_events", "Filling histogram...");
   
      // Access to esdTree
      IlcESDEvent* esd = IlcEveEventManager::AssertESD();
      TTree* t = IlcEveEventManager::GetMaster()->GetESDTree();

      // Creating 2D histograms
      TH2F *histopos_t = new TH2F("histopos_t","Histo 2d positive",
                                 100,-1.5,1.5,80,-pi,pi);
      TH2F *histoneg_t = new TH2F("histoneg_t","Histo 2d negative",
                                 100,-1.5,1.5,80,-pi,pi);

         // Getting current tracks for each event, filling histograms 
         for ( int event = 0; event < t->GetEntries(); event++ ) {	
            t->GetEntry(event);
               for ( int n = 0; n < esd->GetNumberOfTracks(); ++n ) {    
  
                  if ( esd->GetTrack(n)->GetSign() > 0 ) {
                     histopos_t->Fill(esd->GetTrack(n)->Eta(),
	      	                      GetPhi(esd->GetTrack(n)->Phi()),
                                      fabs(esd->GetTrack(n)->Pt()));
                  } else {
                     histoneg_t->Fill(esd->GetTrack(n)->Eta(),
                                      GetPhi(esd->GetTrack(n)->Phi()),
                                      fabs(esd->GetTrack(n)->Pt()));
                  }
               }
         }

      data_t = new TEveCaloDataHist();
      data_t->AddHistogram(histoneg_t);
      data_t->RefSliceInfo(0).Setup("NegCg:", 0, kBlue);
      data_t->AddHistogram(histopos_t);
      data_t->RefSliceInfo(1).Setup("PosCg:", 0, kRed);
      data_t->GetEtaBins()->SetTitleFont(120);
      data_t->GetEtaBins()->SetTitle("h");
      data_t->GetPhiBins()->SetTitleFont(120);
      data_t->GetPhiBins()->SetTitle("f");
      data_t->IncDenyDestroy();

      // Creating frames
      g_histo2d_all_events_slot = TEveWindow::CreateWindowInTab(gEve->GetBrowser()->GetTabRight());
      TEveWindowPack* packH = g_histo2d_all_events_slot->MakePack();
      packH->SetElementName("Projections");
      packH->SetHorizontal();
      packH->SetShowTitleBar(kFALSE);

      g_histo2d_all_events_slot = packH->NewSlot();
      TEveWindowPack* pack0 = g_histo2d_all_events_slot->MakePack();
      pack0->SetShowTitleBar(kFALSE);
      TEveWindowSlot*  slotLeftTop   = pack0->NewSlot();
      TEveWindowSlot* slotLeftBottom = pack0->NewSlot();

      g_histo2d_all_events_slot = packH->NewSlot();
      TEveWindowPack* pack1 = g_histo2d_all_events_slot->MakePack();
      pack1->SetShowTitleBar(kFALSE);
      TEveWindowSlot* slotRightTop    = pack1->NewSlot();
      TEveWindowSlot* slotRightBottom = pack1->NewSlot();

      // Creating viewers and scenes   
      TEveCalo3D* calo3d = Create3DView(data_t, slotLeftTop);
      CreateHistoLego(data_t, slotLeftBottom);
      CreateProjections(data_t, calo3d, slotRightTop, slotRightBottom);

      gEve->Redraw3D(kTRUE);

      Info("histo2d_all_events", "...Finished");
   }

   return data_t;
}

//______________________________________________________________________________
Double_t GetPhi(Double_t phi)
{
   if (phi > pi) {
      phi -= 2*pi;
   }
   return phi;
}

//______________________________________________________________________________
TEveCaloLego* CreateHistoLego(TEveCaloData* data, TEveWindowSlot* slot){

   TEveCaloLego* lego;

   // Viewer initialization, tab creation
   if ( g_histo2d_all_events_v0 == 0 ) {

      TEveBrowser *browser = gEve->GetBrowser();
      slot->MakeCurrent();
      g_histo2d_all_events_v0 = gEve->SpawnNewViewer("2D Lego Histogram", "2D Lego Histogram");
      g_histo2d_all_events_s0 = gEve->SpawnNewScene("2D Lego Histogram", "2D Lego Histogram");
      g_histo2d_all_events_v0->AddScene(g_histo2d_all_events_s0);
      g_histo2d_all_events_v0->SetElementName("2D Lego Viewer");
      g_histo2d_all_events_s0->SetElementName("2D Lego Scene");

      TGLViewer* glv = g_histo2d_all_events_v0->GetGLViewer();
      g_histo2d_all_events_lego_overlay = new TEveCaloLegoOverlay();
      glv->AddOverlayElement(g_histo2d_all_events_lego_overlay);
      glv->SetCurrentCamera(TGLViewer::kCameraPerspXOY);

      // Plotting histogram lego
      lego = new TEveCaloLego(data);
      g_histo2d_all_events_s0->AddElement(lego);

      // Move to real world coordinates
      lego->InitMainTrans();
      Float_t sc = TMath::Min(lego->GetEtaRng(), lego->GetPhiRng());
      lego->RefMainTrans().SetScale(sc, sc, sc);

      // Set event handler to move from perspective to orthographic view.
      glv->SetEventHandler(new TEveLegoEventHandler(glv->GetGLWidget(), glv, lego));

      g_histo2d_all_events_lego_overlay->SetCaloLego(lego);
   }

   return lego;
}

//______________________________________________________________________________
TEveCalo3D* Create3DView(TEveCaloData* data, TEveWindowSlot* slot){

   TEveCalo3D* calo3d;

   if ( g_histo2d_all_events_v1 == 0 ) {
      
      TEveBrowser *browser = gEve->GetBrowser();
      slot->MakeCurrent();
      g_histo2d_all_events_v1 = gEve->SpawnNewViewer("3D Histogram", "3D Histogram");
      g_histo2d_all_events_s1 = gEve->SpawnNewScene("3D Histogram", "3D Histogram");
      g_histo2d_all_events_v1->AddScene(g_histo2d_all_events_s1);
      g_histo2d_all_events_v1->SetElementName("3D Histogram Viewer");
      g_histo2d_all_events_s1->SetElementName("3D Histogram Scene");

      calo3d = new TEveCalo3D(data);
   
      calo3d->SetBarrelRadius(550);
      calo3d->SetEndCapPos(550);
      g_histo2d_all_events_s1->AddElement(calo3d);
   } 

   return calo3d;
}

//______________________________________________________________________________
void CreateProjections(TEveCaloData* data, TEveCalo3D *calo3d, TEveWindowSlot* slot1, TEveWindowSlot* slot2){

   if ( g_histo2d_all_events_v2 == 0 ) {
      
      TEveBrowser *browser = gEve->GetBrowser();
      slot1->MakeCurrent();
      g_histo2d_all_events_v2 = gEve->SpawnNewViewer("RPhi projection", "RPhi projection");
      g_histo2d_all_events_s2 = gEve->SpawnNewScene("RPhi projection", "RPhi projection");
      g_histo2d_all_events_v2->AddScene(g_histo2d_all_events_s2);
      g_histo2d_all_events_v2->SetElementName("RPhi Projection Viewer");
      g_histo2d_all_events_s2->SetElementName("RPhi Projection Scene");

      TEveProjectionManager* mng1 = new TEveProjectionManager();
      mng1->SetProjection(TEveProjection::kPT_RPhi);

      TEveProjectionAxes* axeg_histo2d_all_events_s1 = new TEveProjectionAxes(mng1);
      g_histo2d_all_events_s2->AddElement(axeg_histo2d_all_events_s1);
      TEveCalo2D* calo2d1 = (TEveCalo2D*) mng1->ImportElements(calo3d);
      g_histo2d_all_events_s2->AddElement(calo2d1);

      g_histo2d_all_events_v2->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
   } 

   if ( g_histo2d_all_events_v3 == 0 ) {
      
      TEveBrowser *browser = gEve->GetBrowser();
      slot2->MakeCurrent();
      g_histo2d_all_events_v3 = gEve->SpawnNewViewer("RhoZ projection", "RhoZ projection");
      g_histo2d_all_events_s3 = gEve->SpawnNewScene("RhoZ projection", "RhoZ projection");
      g_histo2d_all_events_v3->AddScene(g_histo2d_all_events_s3);
      g_histo2d_all_events_v3->SetElementName("RhoZ Projection Viewer");
      g_histo2d_all_events_s3->SetElementName("RhoZ Projection Viewer");

      TEveProjectionManager* mng2 = new TEveProjectionManager();
      mng2->SetProjection(TEveProjection::kPT_RhoZ);

      TEveProjectionAxes* axeg_histo2d_all_events_s2 = new TEveProjectionAxes(mng2);
      g_histo2d_all_events_s3->AddElement(axeg_histo2d_all_events_s2);
      TEveCalo2D* calo2d2 = (TEveCalo2D*) mng2->ImportElements(calo3d);
      g_histo2d_all_events_s3->AddElement(calo2d2);

      g_histo2d_all_events_v3->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
   } 

   return;
}
