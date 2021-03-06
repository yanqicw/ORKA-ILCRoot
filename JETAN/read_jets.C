
void read_jets(const char* fn = "jets.root")

{
    //
    // Some histos
    //
    TH1F* eH     = new TH1F("eH"  , "Jet Energy", 40.,  0., 200.);
    TH1F* e1H    = new TH1F("e1H" , "Jet Energy", 40.,  0., 200.);
    TH1F* e2H    = new TH1F("e2H" , "Jet Energy", 40.,  0., 200.);
    TH1F* e3H    = new TH1F("e3H" , "Jet Energy", 40.,  0., 200.);
    TH1F* e4H    = new TH1F("e4H" , "Jet Energy", 40.,  0., 200.);
    TH1F* dr1H = new TH1F("dr1H", "delta R",  160., 0.,   2.);
    TH1F* dr2H = new TH1F("dr2H", "delta R",  160., 0.,   2.);
    TH1F* dr3H = new TH1F("dr4H", "delta R",  160., 0.,   2.);
    TH1F* etaH  = new TH1F("etaH",  "eta",  160., -2.,   2.);
    TH1F* eta1H = new TH1F("eta1H", "eta",  160., -2.,   2.);
    TH1F* eta2H = new TH1F("eta2H", "eta",  160., -2.,   2.);

    TH1F* phiH  = new TH1F("phiH",  "phi",  160., -3.,   3.);
    TH1F* phi1H = new TH1F("phi1H", "phi",  160.,  0.,   6.28);
    TH1F* phi2H = new TH1F("phi2H", "phi",  160.,  0.,   6.28);
    

  // load jet library
  gSystem->Load("$(ILC_ROOT)/lib/tgt_$(ILC_TARGET)/libJETAN");

  // open file
  TFile *jFile = new TFile(fn);

  // get jet header and display parameters
  IlcUA1JetHeader* jHeader = 
    (IlcUA1JetHeader*) (jFile->Get("IlcUA1JetHeader"));
  jHeader->PrintParameters();

  // get reader header and events to be looped over
  IlcJetESDReaderHeader *jReaderH = 
    (IlcJetESDReaderHeader*)(jFile->Get("IlcJetESDReaderHeader"));
  Int_t first = jReaderH->GetFirstEvent(); 
  Int_t last  = jReaderH->GetLastEvent();
  cout << "First event = " << first << "   Last event = " << last << endl;


  // loop over events
  IlcJet *jets, *gjets;
  IlcLeading *leading;

  for (Int_t i=first; i< last; i++) {
      cout << "  Analyzing event " << i << endl;
      // get next tree with IlcJet
      char nameT[100];
      sprintf(nameT, "TreeJ%d",i);
      TTree *jetT =(TTree *)(jFile->Get(nameT));
      jetT->SetBranchAddress("FoundJet",    &jets);
      jetT->SetBranchAddress("GenJet",      &gjets);
      jetT->SetBranchAddress("LeadingPart", &leading);
      jetT->GetEntry(0);

//
//    Find the jet with the highest E_T 
//
      Int_t njets = jets->GetNJets();
      
      Float_t emax = 0.;
      Int_t   imax = -1;
      for (Int_t j = 0; j < njets; j++) {
	  if (jets->GetPt(j) > emax && TMath::Abs(jets->GetEta(j)) < 0.5) {
	      emax = jets->GetPt(j);
	      imax = j;
	  }
      }
      
      if (imax == -1) {
	  e2H->Fill(gjets->GetPt(0));
      } else {
	  eH->Fill(jets->GetPt(imax));
	  dr1H->Fill(jets->GetEta(imax) - gjets->GetEta(0));
//
//    Find the generated jet closest to the reconstructed 
//
	  
	  Float_t rmin;
	  Int_t   igen;
	  Float_t etaj = jets->GetEta(imax);
	  Float_t phij = jets->GetPhi(imax);
	  
	  Int_t ngen = gjets->GetNJets();
	  if (ngen != 0) {
	      rmin = 1.e6;
	      igen = -1;
	      for (Int_t j = 0; j < ngen; j++) {
		  Float_t etag = gjets->GetEta(j);
		  Float_t phig = gjets->GetPhi(j);
		  Float_t deta = etag - etaj;
		  Float_t dphi = TMath::Abs(phig - phij);
		  if (dphi > TMath::Pi()) dphi = 2. * TMath::Pi() - dphi;
		  Float_t r = TMath::Sqrt(deta * deta + dphi * dphi);
		  if (r  < rmin) {
		      rmin = r;
		      igen = j;
		  }
	      }

	      Float_t egen = gjets->GetPt(igen);
	      e1H->Fill(gjets->GetPt(igen));
	      Float_t etag = gjets->GetEta(igen);
	      Float_t phig = gjets->GetPhi(igen);
	      Float_t dphi = phig - phij;

	      if (egen > 125. && egen < 150.) {
		  phiH->Fill((dphi));
		  etaH->Fill(etag - etaj);
		  phi1H->Fill(phig);
		  phi2H->Fill(phij);		  
		  eta1H->Fill(etag);
		  eta2H->Fill(etaj);
		  e4H->Fill(emax);
		  dr2H->Fill(rmin);
	      }
	  }
      }
      

//
//   Leading particle
//
    Float_t etal = leading->GetLeading()->Eta();
    Float_t phil = leading->GetLeading()->Phi();
    Float_t el   = leading->GetLeading()->E();
    e3H->Fill(el);
    
    Float_t rmin = 1.e6;
    Int_t igen = -1;
    Int_t ngen = gjets->GetNJets();
    for (Int_t j = 0; j < ngen; j++) {
	Float_t etag = gjets->GetEta(j);
	Float_t phig = gjets->GetPhi(j);
	Float_t deta = etag-etal;
	Float_t dphi = TMath::Abs(phig - phil);
	if (dphi > TMath::Pi()) dphi = 2. * TMath::Pi() - dphi;

	Float_t r = TMath::Sqrt(deta * deta + dphi * dphi);
	
	if (r  < rmin) {
		rmin = r;
		igen = j;
	}
    }
    if (egen > 125. && egen < 150.) 
	dr3H->Fill(rmin);

//    cout << " Generated Jets:" << endl;
//    gjets->PrintJets();
//    cout << " Leading particle: " << endl;
//    leading->PrintLeading();
  }

  // Get Control Plots
  TCanvas* c1 = new TCanvas("c1");
  eH->Draw();
  e1H->SetLineColor(2);
  e2H->SetLineColor(4);
  e3H->SetLineColor(5);
  e1H->Draw("same");
  e2H->Draw("same");
  e3H->Draw("same");

  TCanvas* c2 = new TCanvas("c2");
//  dr1H->Draw();
  dr2H->SetLineColor(2);
  dr2H->Draw("");
  
  TCanvas* c3 = new TCanvas("c3");
  dr2H->Draw();
  dr3H->Draw("same");

  TCanvas* c4 = new TCanvas("c4");
  eH->Draw();

  TCanvas* c5 = new TCanvas("c5");
  etaH->Draw();

  TCanvas* c5a = new TCanvas("c5a");
  eta1H->Draw();

  TCanvas* c5b = new TCanvas("c5b");
  eta2H->Draw();

  TCanvas* c6 = new TCanvas("c6");
  e4H->Draw();
  TCanvas* c7 = new TCanvas("c7");
  phiH->Draw();

  TCanvas* c7a = new TCanvas("c7a");
  phi1H->Draw();
  TCanvas* c7b = new TCanvas("c7b");
  phi2H->Draw();
}
