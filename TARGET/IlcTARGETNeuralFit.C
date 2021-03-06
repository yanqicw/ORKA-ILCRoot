void IlcTARGETNeuralFit (Int_t field_factor = 1, Bool_t exact_pid = kTRUE,
                      const char *fname_chains = "its_chains.root",
                      const char *fname_save   = "its_neural.root", 
                      const char *fname_points = "its_recpoints_v1.root")
{
	Int_t i, k;
	
	TStopwatch timer;
	
	// Look for magnetic field
	// if 'factor' option is 0, the field is read from gIlc, 
	// else it is converted into double.
	// if 'exact_pid' is true, the "gilc.root" file is opened
	// to get informations about particles
	
	Double_t factor = (Double_t)field_factor;
	Int_t nparticles = 0;
	TFile *fileIlc = 0;
	IlcTARGETPid *pid = new IlcTARGETPid(1);
	
	// Remove an already existing Run Loader
	if (gIlc) {
		delete gIlc->GetRunLoader();
		delete gIlc; 
		gIlc=0;
	}
	
	// Instance the new Run Loader
	IlcRunLoader* rl = IlcRunLoader::Open("gilc.root");
	if (rl == 0x0) {
		cerr<<"IlcTARGETFindTracks.C : Can not open session RL=NULL"<< endl;
		return 3;
	}
	
	// Instance the TARGET Loader
	IlcTARGETLoader* itsl = (IlcTARGETLoader*)rl->GetLoader("TARGETLoader");
	if (itsl == 0x0) {
		cerr<<"IlcTARGETFindTracksV2.C : Can not get TARGET loader"<<endl;
		return 4;
	}
	
	// Load the gIlc
	if (rl->LoadgIlc()) {
		cerr<<"IlcTARGETFindTracksV2.C : LoadgIlc returned error"<<endl;
		delete rl;
		return 3;
	}
	
	rl->LoadKinematics();
	IlcStack* stack = rl->Stack();
	
	if (field_factor == 0) {
		factor = rl->GetIlcRun()->Field()->SolenoidField() / 2.;
	}
	else {
		factor = (Double_t)field_factor;
	}
		
	// Open files and trees and set branches
	TFile *fileP = new TFile(fname_points);
	TFile *fileC = new TFile(fname_chains);
	TTree *treeP = (TTree*)fileP->Get("TreeP");
	TTree *treeC = (TTree*)fileC->Get("TreeC");
	IlcTARGETglobalRecPoint *pnt = 0, *newpt = 0;
	treeP->SetBranchAddress("Points", &pnt);
	Int_t l[6];
	treeC->SetBranchAddress("l0", &l[0]);
	treeC->SetBranchAddress("l1", &l[1]);
	treeC->SetBranchAddress("l2", &l[2]);
	treeC->SetBranchAddress("l3", &l[3]);
	treeC->SetBranchAddress("l4", &l[4]);
	treeC->SetBranchAddress("l5", &l[5]);
	Int_t nP = (Int_t)treeP->GetEntries();
	Int_t nC = (Int_t)treeC->GetEntries();
	
	
	// Store points into a TClonesArray
	TClonesArray arrayP("IlcTARGETglobalRecPoint", nP);
	for (i = 0; i < nP; i++) {
		treeP->GetEntry(i);
		new(arrayP[i]) IlcTARGETglobalRecPoint(*pnt);
	}

	
	// Generate new file of fitted tracks
	Double_t mass, dedx, mom;
	Int_t label, pdg;
	IlcTARGETNeuralTrack *track = 0;
	TFile *fileT = new TFile(fname_save, "recreate");
	TTree *treeT = new TTree("TreeT", "Neural found tracks (fitted)");
	treeT->Branch("Tracks", "IlcTARGETNeuralTrack", &track);
	
	timer.Start();
	
	
	// Scan chains tree and fit tracks
	cout << "Selected field factor = " << factor << endl;
	cout << "Fitting..." << endl;
	for (i = 0; i < nC; i++) {
		
		// Get track
		treeC->GetEntry(i);
		track = new IlcTARGETNeuralTrack;
		for (k = 0; k < 6; k++) {
			newpt = (IlcTARGETglobalRecPoint*)arrayP[l[k]];
			track->Insert(newpt);
		}
		
		// Cook label and preliminary fit
		track->AssignLabel();
		track->SetFieldFactor(factor);
		track->RiemannFit();
		
		// PID
		pdg = TMath::Abs(((TParticle*)stack->Particle(lab))->GetPdgCode());
		if (pdg == 0) pdg = 211;
		switch (pdg) {
			case 11: mass = 0.000511; break;
			case 211: mass = 0.13957018; break;
			case 321: mass = 0.493677; break;
			case 2212: mass = 0.938272; break;
			default:
				cout << "PDG code " << pdg << " not recognized. Skipping track..." << endl;
				continue;
		}
		track->SetPDGcode(pdg);
		track->SetMass(mass);
		
		// Kalman Fit
		track->SeedCovariance();
		track->KalmanFit();
		if (!track->PropagateTo(3.0)) continue;
		treeT->Fill();
	}
	cerr << "DONE" << endl;
	
	timer.Stop();
	timer.Print();

	treeT->Write();
	fileT->Close();
}

