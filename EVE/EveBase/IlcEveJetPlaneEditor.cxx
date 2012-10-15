// $Id: IlcEveJetPlaneEditor.cxx 47102 2011-02-04 11:24:48Z fca $
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

#include "IlcEveJetPlaneEditor.h"
#include "IlcEveJetPlane.h"

#include <TEveGValuators.h>

#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGFrame.h>
#include <TGTab.h>

//==============================================================================
//==============================================================================
// IlcEveJetPlaneEditor
//==============================================================================

//______________________________________________________________________________
//
// GUI editor for IlcEveJetPlane class.

IlcEveJetPlaneEditor::StaticDataWindow* IlcEveJetPlaneEditor::fgStaticWindow = 0;

ClassImp(IlcEveJetPlaneEditor)

IlcEveJetPlaneEditor::IlcEveJetPlaneEditor(const TGWindow *p, Int_t width, Int_t height,
			       UInt_t options, Pixel_t back) :
  TGedFrame(p, width, height, options | kVerticalFrame, back),
  fM(0),
  fRnrJets(0),
  fRnrTracks(0),
  fEnergyScale(0),
  fArrowJetScale(0),
  fArrowTrackScale(0),
  fOneSelection(0), fTwoSelection(0),
  fInformationSetup(0)
{
  // Constructor.

  MakeTitle("IlcEveJetPlane");
  Int_t labelW = 67;

  fRnrJets  = new TGCheckButton(this, "Rnr Jets");
  AddFrame(fRnrJets, new TGLayoutHints(kLHintsTop, 1, 1, 1, 1));
  fRnrJets->Connect("Clicked()", "IlcEveJetPlaneEditor", this, "DoRnrJets()");

  fRnrTracks  = new TGCheckButton(this, "Rnr Tracks");
  AddFrame(fRnrTracks, new TGLayoutHints(kLHintsTop, 1, 1, 1, 1));
  fRnrTracks->Connect("Clicked()", "IlcEveJetPlaneEditor", this, "DoRnrTracks()");

  fEnergyScale = new TEveGValuator(this, "Length scale:", 110, 0);
  fEnergyScale->SetLabelWidth(labelW);
  fEnergyScale->SetNELength(6);
  fEnergyScale->Build();
  fEnergyScale->SetLimits(1, 200, 200, TGNumberFormat::kNESRealOne);
  fEnergyScale->SetToolTip("Energy mapped to length of arrow.");
  fEnergyScale->Connect("ValueSet(Double_t)", "IlcEveJetPlaneEditor", this, "DoEnergyScale()");
  AddFrame(fEnergyScale, new TGLayoutHints(kLHintsTop, 1, 1, 1, 1));

  fArrowJetScale = new TEveGValuator(this, "Jet scale:", 110, 0);
  fArrowJetScale->SetLabelWidth(labelW);
  fArrowJetScale->SetNELength(6);
  fArrowJetScale->Build();
  fArrowJetScale->SetLimits(0.1, 3, 100, TGNumberFormat::kNESRealOne);
  fArrowJetScale->SetToolTip("Scale for jet arrow dimensions.");
  fArrowJetScale->Connect("ValueSet(Double_t)", "IlcEveJetPlaneEditor", this, "DoArrowJetScale()");
  AddFrame(fArrowJetScale, new TGLayoutHints(kLHintsTop, 1, 1, 1, 1));

  fArrowTrackScale = new TEveGValuator(this, "Track scale:", 110, 0);
  fArrowTrackScale->SetLabelWidth(labelW);
  fArrowTrackScale->SetNELength(6);
  fArrowTrackScale->Build();
  fArrowTrackScale->SetLimits(0.1, 3, 100, TGNumberFormat::kNESRealOne);
  fArrowTrackScale->SetToolTip("Scale for track arrow dimensions.");
  fArrowTrackScale->Connect("ValueSet(Double_t)", "IlcEveJetPlaneEditor", this, "DoArrowTrackScale()");
  AddFrame(fArrowTrackScale, new TGLayoutHints(kLHintsTop, 1, 1, 1, 1));
	
	fOneSelection = new TGRadioButton(this, "&One IlcEveTrack/Jet");
  fTwoSelection = new TGRadioButton(this, "&Two IlcEveTrack/Jet");
  AddFrame(fOneSelection, new TGLayoutHints(kLHintsTop, 1, 1, 1, 1));
  AddFrame(fTwoSelection, new TGLayoutHints(kLHintsTop, 1, 1, 1, 1));
  fOneSelection->Connect("Clicked()", "IlcEveJetPlaneEditor", this, "DoOneSelection()");
  fTwoSelection->Connect("Clicked()", "IlcEveJetPlaneEditor", this, "DoTwoSelection()");

  // fInformationSetup = new TGTextButton(this, "IlcEveTrack/Jet Print");
  // AddFrame(fInformationSetup, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 0, 2, 2));
  // fInformationSetup->Connect("Clicked()", "IlcEveJetPlaneEditor", this, "DoStaticDataWindow()");
}

/******************************************************************************/

void IlcEveJetPlaneEditor::SetModel(TObject* obj)
{
  // Set model object.

  fM = static_cast<IlcEveJetPlane*>(obj);

  // Set values of widgets
  // fXYZZ->SetValue(fM->GetXYZZ());
  fRnrJets->SetState(fM->GetRnrJets() ? kButtonDown : kButtonUp);
  fRnrTracks->SetState(fM->GetRnrTracks() ? kButtonDown : kButtonUp);
  fEnergyScale->SetValue(fM->GetEnergyScale());
  fArrowJetScale->SetValue(fM->GetArrowJetScale());
  fArrowTrackScale->SetValue(fM->GetArrowTrackScale());
	fOneSelection->SetState(fM->GetOneSelection() ? kButtonDown : kButtonUp);
  fTwoSelection->SetState(fM->GetTwoSelection() ? kButtonDown : kButtonUp);
}

/******************************************************************************/

void IlcEveJetPlaneEditor::DoRnrJets()
{
  // Slot for RnrJets.

  fM->SetRnrJets(fRnrJets->IsOn());
  Update();
}

void IlcEveJetPlaneEditor::DoRnrTracks()
{
  // Slot for RnrTracks.

  fM->SetRnrTracks(fRnrTracks->IsOn());
  Update();
}

void IlcEveJetPlaneEditor::DoArrowJetScale()
{
  // Slot for JetScale.

  fM->SetArrowJetScale(fArrowJetScale->GetValue());
  Update();
}

void IlcEveJetPlaneEditor::DoArrowTrackScale()
{
  // Slot for TrackScale.

  fM->SetArrowTrackScale(fArrowTrackScale->GetValue());
  Update();
}

void IlcEveJetPlaneEditor::DoEnergyScale()
{
  // Slot for EnergyScale.

  fM->SetEnergyScale(fEnergyScale->GetValue());
  Update();
}

void IlcEveJetPlaneEditor::DoOneSelection()
{
  // Slot for OneSelection.

  fTwoSelection->SetState(kButtonUp);
  fM->SetOneSelection(fOneSelection->IsOn());
  fM->SetTwoSelection(fTwoSelection->IsOn());
  Update();
}

void IlcEveJetPlaneEditor::DoTwoSelection()
{
  // Slot for TwoSelection.

  fOneSelection->SetState(kButtonUp);
  fM->SetOneSelection(fOneSelection->IsOn());
  fM->SetTwoSelection(fTwoSelection->IsOn());
  Update();
}

void IlcEveJetPlaneEditor::DoStaticDataWindow()
{
  // Slot for StaticDataWindow.

  printf("\n Soon available ... \n");
  if (fgStaticWindow == 0)
    fgStaticWindow = new StaticDataWindow(gClient->GetRoot(), this, 400, 200);

  // call fgStaticWindow->ReadValues(); // like setmodel

  // position relative to the parent's window
  fgStaticWindow->MapWindow();
  fgStaticWindow->RaiseWindow();
  fgStaticWindow->CenterOnParent();
}


//==============================================================================
//==============================================================================
// IlcEveJetPlaneEditor::StaticDataWindow
//==============================================================================

//______________________________________________________________________________
//
// Common settings for all IlcEveJetPlane objects.
// Used as a pop-up from IlcEveJetPlaneEditor.

ClassImp(IlcEveJetPlaneEditor::StaticDataWindow)

IlcEveJetPlaneEditor::StaticDataWindow::StaticDataWindow(const TGWindow *p, const TGWindow *main,
                                                         UInt_t w, UInt_t h, UInt_t options) :
  TGTransientFrame(p, main, w, h, options),
  fFrame1(0), fF2(0),
  fOkButton(0), fCancelButton(0),
  fL1(0), fL2(0), fL3(0), fL5(0),
  fTab(0),
  fChk1(0), fChk2(0), fChk3(0), fChk4(0), fChk5(0)
{
  // Constructor.
  // Create a dialog window. A dialog window pops up with respect to its
  // "main" window.

  Connect("CloseWindow()", "IlcEveJetPlaneEditor::StaticDataWindow", this, "DoClose()");
  DontCallClose(); // to avoid double deletions.

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  fFrame1 = new TGHorizontalFrame(this, 60, 20, kFixedWidth);

  fOkButton = new TGTextButton(fFrame1, "&Ok", 1);
  fOkButton->Connect("Clicked()", "IlcEveJetPlaneEditor::StaticDataWindow", this, "DoOK()");
  fCancelButton = new TGTextButton(fFrame1, "&Cancel", 2);
  fCancelButton->Connect("Clicked()", "IlcEveJetPlaneEditor::StaticDataWindow", this, "DoCancel()");

  fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,2, 2, 2, 2);
  fL2 = new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1);

  fFrame1->AddFrame(fOkButton, fL1);
  fFrame1->AddFrame(fCancelButton, fL1);

  fFrame1->Resize(150, fOkButton->GetDefaultHeight());
  AddFrame(fFrame1, fL2);

  // Tabs for one and two track information

  fTab = new TGTab(this, 300, 300);
  fTab->Connect("Selected(Int_t)", "IlcEveJetPlaneEditor::StaticDataWindow", this, "DoTab(Int_t)");

  fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);

  TGCompositeFrame *tf = fTab->AddTab("One IlcEveTrack/Jet");

  //    fF1 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);
  //    fF1->AddFrame(new TGTextButton(fF1, "&Test button", 0), fL3);
  //    fF1->AddFrame(fTxt1 = new TGTextEntry(fF1, new TGTextBuffer(100)), fL3);
  //    fF1->AddFrame(fTxt2 = new TGTextEntry(fF1, new TGTextBuffer(100)), fL3);
  //    tf->AddFrame(fF1, fL3);
  //    fTxt1->Resize(150, fTxt1->GetDefaultHeight());
  //    fTxt2->Resize(150, fTxt2->GetDefaultHeight());
  fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2);
  fF2 = new TGCompositeFrame(tf, 60, 60, kVerticalFrame);
  fF2->AddFrame(fChk1 = new TGCheckButton(fF2, "4-Momentum: {pt, px, py, pz} "), fL1);
  fF2->AddFrame(fChk2 = new TGCheckButton(fF2, "4-Momentum: {pt, Phi, Theta}"), fL1);
  fF2->AddFrame(fChk3 = new TGCheckButton(fF2, "Pseudorapidity: Eta"), fL1);
  fF2->AddFrame(fChk4 = new TGCheckButton(fF2, "Energy: E"), fL1);
  fF2->AddFrame(fChk5 = new TGCheckButton(fF2, "Charge and Mass"), fL1);

  tf = fTab->AddTab("Two Tracks/Jets");

  tf->AddFrame(fF2, fL3);

  //    fBtn1->Connect("Clicked()", "TestDialog", this, "HandleButtons()");
  //    fBtn2->Connect("Clicked()", "TestDialog", this, "HandleButtons()");
  //    fChk1->Connect("Clicked()", "TestDialog", this, "HandleButtons()");
  //    fChk2->Connect("Clicked()", "TestDialog", this, "HandleButtons()");
  //    fRad1->Connect("Clicked()", "TestDialog", this, "HandleButtons()");
  //    fRad2->Connect("Clicked()", "TestDialog", this, "HandleButtons()");


  fL5 = new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsExpandY, 2, 2, 5, 1);
  AddFrame(fTab, fL5);

  MapSubwindows();
  Resize();

  SetWindowName("IlcEveTrack/Jet Common Setup");
}

IlcEveJetPlaneEditor::StaticDataWindow::~StaticDataWindow()
{
  // Destructor, deletes the window.

  DeleteWindow();
}

void IlcEveJetPlaneEditor::StaticDataWindow::DoClose()
{
  // Close the window.

  UnmapWindow();
}

void IlcEveJetPlaneEditor::StaticDataWindow::DoOK()
{
  // Read data from widgets, copy to static members of IlcEveJetPlane

  SendCloseMessage();
}

void IlcEveJetPlaneEditor::StaticDataWindow::DoCancel()
{
  // Cancel, close the window.

  SendCloseMessage();
}

void IlcEveJetPlaneEditor::StaticDataWindow::DoTab(Int_t /*id*/)
{
  // Tab selected.

  // printf("Tab item %d activated\n", id);
}


