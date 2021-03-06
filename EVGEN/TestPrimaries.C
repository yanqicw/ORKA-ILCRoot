void TestPrimaries(Int_t evNumber1=0, Int_t evNumber2=0) 
{
// Dynamically link some shared libs                    
    if (gClassTable->GetID("IlcRun") < 0) {
	gROOT->LoadMacro("loadlibs.C");
	loadlibs();
    }
// Connect the Root Gilc file containing Geometry, Kine and Hits
    IlcRunLoader* rl = IlcRunLoader::Open("gilc.root");
//
    TDatabasePDG*  DataBase = TDatabasePDG::Instance();
    
//  Create some histograms

    TH1F *thetaH   =  new TH1F("thetaH","Theta distribution",180,0,180);
    thetaH->Sumw2();
    TH1F *phiH     =  new TH1F("phiH","Phi distribution"  ,180,-180,180);
    phiH->Sumw2();
    TH1F *phiH1    =  new TH1F("phiH1","Phi distribution"  ,180,-180,180);
    phiH1->Sumw2();
    TH1F *etaH     =  new TH1F("etaH","Pseudorapidity",120,-12,12);
    etaH->Sumw2();
    TH1F *yH       =  new TH1F("yH","Rapidity distribution",120,-12,12);
    yH->Sumw2();
    TH1F *eH       =  new TH1F("eH","Energy distribution",100,0,100);
    eH->Sumw2();
    TH1F *eetaH    =  new TH1F("eetaH","Pseudorapidity",120,0,12);
    eetaH->Sumw2();
    TH1F *ptH      =  new TH1F("ptH","Pt distribution",150,0,15);
    ptH->Sumw2();
//
//   Loop over events 
//
    rl->LoadKinematics();
    rl->LoadHeader();    
    
    for (Int_t nev=0; nev<= evNumber2; nev++) {
	rl->GetEvent(nev);
	IlcStack* stack = rl->Stack();
	Int_t npart = stack->GetNprimary();
	if (nev < evNumber1) continue;
//
// Loop over primary particles (jpsi. upsilon, ...)
//       
	
	for (Int_t part=0; part<npart; part++) {
	    TParticle *MPart = stack->Particle(part);
	    Int_t mpart  = MPart->GetPdgCode();
	    Int_t child1 = MPart->GetFirstDaughter();
	    Int_t child2 = MPart->GetLastDaughter();	
	    Int_t mother = MPart->GetFirstMother();	   
	    Float_t Pt = MPart->Pt();
	    Float_t E  = MPart->Energy();
	    Float_t Pz = MPart->Pz();
	    Float_t Py = MPart->Py();
	    Float_t Px = MPart->Px();
	    Float_t pT = TMath::Sqrt(Px*Px+Py*Py);
	    Float_t theta = MPart->Theta();
	    Float_t phi   = MPart->Phi()-TMath::Pi();
	    Float_t eta   = -TMath::Log(TMath::Tan(theta/2.));
	    Float_t y     = 0.5*TMath::Log((E+Pz+1.e-13)/(E-Pz+1.e-13));
	    
	    if (child1 >= 0) continue;
	    if (DataBase->GetParticle(mpart)->Charge() == 0) continue;
	    Float_t wgt = 1./(Float_t ((evNumber2-evNumber1)+1.));
	    thetaH->Fill(theta*180./TMath::Pi(),wgt);
	    if (pT<2)
	      phiH->Fill(phi*180./TMath::Pi(),wgt);
	    else
	      phiH1->Fill(phi*180./TMath::Pi(),wgt);
	    etaH->Fill(eta,5.*wgt);    
	    eetaH->Fill(eta,E);    
	    yH->Fill(y,5.*wgt);      
	    eH->Fill(E,wgt);      
	    ptH->Fill(pT,wgt);     
	} // primary loop
    }
    
//Create a canvas, set the view range, show histograms
    TCanvas *c1 = new TCanvas("c1","Canvas 1",400,10,600,700);
    c1->Divide(2,2);
    c1->cd(1); ptH->Draw();
    c1->cd(2); etaH->Draw();
    c1->cd(3); yH->Draw();
    c1->cd(4); eH->Draw();

    TCanvas *c2 = new TCanvas("c2","Canvas 1",400,10,600,700);
    c2->Divide(2,2);
    c2->cd(1); phiH->Draw();
    c2->cd(2); phiH1->Draw();
    c2->cd(3); thetaH->Draw();
    c2->cd(4); eetaH->Draw();

    TFile* f = new TFile("kineH.root", "update");
    ptH->Write();
    etaH->Write();
    yH->Write();
    phiH->Write();
    phiH1->Write();
    f->Close();
    

}







