// ### Settings that make sense when using the Alien plugin
//==============================================================================
Int_t       runOnData          = 1;       // Set to 1 if processing real data
Int_t       iCollision         = 1;       // 0=pp, 1=Pb-Pb
Int_t       run_flag           = 1100;    // year (2011 = 1100)
//==============================================================================
Bool_t      usePhysicsSelection = kTRUE; // use physics selection
Bool_t      useTender           = kFALSE; // use tender wagon
Bool_t      useCentrality       = kTRUE; // centrality
Bool_t      useV0tender         = kFALSE;  // use V0 correction in tender
Bool_t      useDBG              = kTRUE;  // activate debugging
Bool_t      useMC               = kFALSE;  // use MC info
Bool_t      useKFILTER          = kFALSE;  // use Kinematics filter
Bool_t      useTR               = kFALSE;  // use track references
Bool_t      useCORRFW           = kFALSE; // do not change
Bool_t      useAODTAGS          = kFALSE; // use AOD tags
Bool_t      useSysInfo          = kFALSE; // use sys info

// ### Analysis modules to be included. Some may not be yet fully implemented.
//==============================================================================
Int_t       iAODhandler        = 1;      // Analysis produces an AOD or dAOD's
Int_t       iESDfilter         = 1;      // ESD to AOD filter (barrel + muon tracks)
Int_t       iMUONcopyAOD       = 1;      // Task that copies only muon events in a separate AOD (PWG3)
Int_t       iJETAN             = 1;      // Jet analysis (PWG4)
Int_t       iJETANdelta        = 1;      // Jet delta AODs
Int_t       iPWGHFvertexing     = 1;      // Vertexing HF task (PWG3)
Int_t       iPWGDQJPSIfilter    = 1;      // JPSI filtering (PWG3)
Int_t       iPWGHFd2h           = 1;      // D0->2 hadrons (PWG3)

// ### Configuration macros used for each module
//==============================================================================
 TString configPWGHFd2h = (iCollision==0)?"$ILC_ROOT/PWGHF/vertexingHF/ConfigVertexingHF.C"
                          :"$ILC_ROOT/PWGHF/vertexingHF/ConfigVertexingHF_highmult.C";
                                                 
// Temporaries.
void AODmerge();
void AddAnalysisTasks();
Bool_t LoadCommonLibraries();
Bool_t LoadAnalysisLibraries();
Bool_t LoadLibrary(const char *);
TChain *CreateChain();

//______________________________________________________________________________
void AODtrain(Int_t merge=0)
{
// Main analysis train macro.

  if (merge) {
    TGrid::Connect("alien://");
    if (!gGrid || !gGrid->IsConnected()) {
      ::Error("QAtrain", "No grid connection");
      return;
    }
  }
  // Set temporary merging directory to current one
  gSystem->Setenv("TMPDIR", gSystem->pwd());
  // Set temporary compilation directory to current one
  gSystem->SetBuildDir(gSystem->pwd(), kTRUE);
   printf("==================================================================\n");
   printf("===========    RUNNING FILTERING TRAIN   ==========\n");
   printf("==================================================================\n");
   printf("=  Configuring analysis train for:                               =\n");
   if (usePhysicsSelection)   printf("=  Physics selection                                                =\n");
   if (useTender)    printf("=  TENDER                                                        =\n");
   if (iESDfilter)   printf("=  ESD filter                                                    =\n");
   if (iMUONcopyAOD) printf("=  MUON copy AOD                                                 =\n");
   if (iJETAN)       printf("=  Jet analysis                                                  =\n");
   if (iJETANdelta)  printf("=     Jet delta AODs                                             =\n");
   if (iPWGHFvertexing) printf("=  PWGHF vertexing                                                =\n");
   if (iPWGDQJPSIfilter) printf("=  PWGDQ j/psi filter                                             =\n");
   if (iPWGHFd2h) printf("=  PWGHF D0->2 hadrons QA                                     =\n");

   // Load common libraries and set include path
   if (!LoadCommonLibraries()) {
      ::Error("AnalysisTrain", "Could not load common libraries");
      return;
   }
    
   // Make the analysis manager and connect event handlers
   IlcAnalysisManager *mgr  = new IlcAnalysisManager("Analysis Train", "Production train");
   if (useSysInfo) mgr->SetNSysInfo(100);
   // Load analysis specific libraries
   if (!LoadAnalysisLibraries()) {
      ::Error("AnalysisTrain", "Could not load analysis libraries");
      return;
   }   

   // Create input handler (input container created automatically)
   // ESD input handler
   IlcESDInputHandler *esdHandler = new IlcESDInputHandler();
   mgr->SetInputEventHandler(esdHandler);       
   // Monte Carlo handler
   if (useMC) {
      IlcMCEventHandler* mcHandler = new IlcMCEventHandler();
      mgr->SetMCtruthEventHandler(mcHandler);
      mcHandler->SetReadTR(useTR); 
   }   
   // AOD output container, created automatically when setting an AOD handler
   if (iAODhandler) {
      // AOD output handler
      IlcAODHandler* aodHandler   = new IlcAODHandler();
      aodHandler->SetOutputFileName("IlcAOD.root");
      mgr->SetOutputEventHandler(aodHandler);
   }
   // Debugging if needed
   if (useDBG) mgr->SetDebugLevel(3);

   AddAnalysisTasks();
   if (merge) {
      AODmerge();
      mgr->InitAnalysis();
      mgr->SetGridHandler(new IlcAnalysisAlien);
      mgr->StartAnalysis("gridterminate",0);
      return;
   }   
   // Run the analysis                                                                                                                     
   //
   TChain *chain = CreateChain();
   if (!chain) return;
                                                                                                                                                   
   TStopwatch timer;
   timer.Start();
   mgr->SetSkipTerminate(kTRUE);
   if (mgr->InitAnalysis()) {
      mgr->PrintStatus();
      mgr->StartAnalysis("local", chain);
   }
   timer.Print();
}                                                                                                                                          
                                                                                                                                            
//______________________________________________________________________________                                                           
void AddAnalysisTasks(){                                                                                                                                          
  // Add all analysis task wagons to the train                                                                                               
   IlcAnalysisManager *mgr = IlcAnalysisManager::GetAnalysisManager();                                                                     

  //
  // Tender and supplies. Needs to be called for every event.
  //
   IlcAnalysisManager::SetCommonFileName("AODQA.root");
   if (useTender) {
      gROOT->LoadMacro("$ILC_ROOT/ANALYSIS/TenderSupplies/AddTaskTender.C");
      // IF V0 tender needed, put kTRUE below
      IlcAnalysisTaskSE *tender = AddTaskTender(useV0tender);
//      tender->SetDebugLevel(2);
   }

   if (usePhysicsSelection) {
   // Physics selection task
      gROOT->LoadMacro("$ILC_ROOT/ANALYSIS/macros/AddTaskPhysicsSelection.C");
      mgr->RegisterExtraFile("event_stat.root");
      IlcPhysicsSelectionTask *physSelTask = AddTaskPhysicsSelection(useMC);
//      IlcOADBPhysicsSelection * oadbDefaultPbPb = CreateOADBphysicsSelection();      
//      physSelTask->GetPhysicsSelection()->SetCustomOADBObjects(oadbDefaultPbPb,0,0);
      mgr->AddStatisticsTask(IlcVEvent::kAny);
   }
   // Centrality (only Pb-Pb)
   if (iCollision && useCentrality) {
      gROOT->LoadMacro("$ILC_ROOT/ANALYSIS/macros/AddTaskCentrality.C");
      IlcCentralitySelectionTask *taskCentrality = AddTaskCentrality();
      taskCentrality->SelectCollisionCandidates(IlcVEvent::kAny);
   }

   if (iESDfilter) {
      //  ESD filter task configuration.
      gROOT->LoadMacro("$ILC_ROOT/ANALYSIS/macros/AddTaskESDFilter.C");
      if (iMUONcopyAOD) {
         printf("Registering delta AOD file\n");
         mgr->RegisterExtraFile("IlcAOD.Muons.root");
         mgr->RegisterExtraFile("IlcAOD.Dimuons.root");
         IlcAnalysisTaskESDfilter *taskesdfilter = AddTaskESDFilter(useKFILTER, kTRUE, kFALSE, kFALSE /*usePhysicsSelection*/,kFALSE,IlcESDpid::kTOF_T0,kTRUE,kFALSE,kFALSE,run_flag);
      } else {
   	   IlcAnalysisTaskESDfilter *taskesdfilter = AddTaskESDFilter(useKFILTER, kFALSE, kFALSE, kFALSE /*usePhysicsSelection*/,kFALSE,IlcESDpid::kTOF_T0,kTRUE,kFALSE,kFALSE,run_flag); // others
      }   
   }   

// ********** PWG3 wagons ******************************************************           
   // PWGHF vertexing
   if (iPWGHFvertexing) {
      gROOT->LoadMacro("$ILC_ROOT/PWGHF/vertexingHF/macros/AddTaskVertexingHF.C");
      if (!iPWGHFd2h) TFile::Cp(gSystem->ExpandPathName(configPWGHFd2h.Data()), "file:ConfigVertexingHF.C");
      IlcAnalysisTaskSEVertexingHF *taskvertexingHF = AddTaskVertexingHF();
      if (!taskvertexingHF) ::Warning("AnalysisTrainNew", "IlcAnalysisTaskSEVertexingHF cannot run for this train conditions - EXCLUDED");
      else mgr->RegisterExtraFile("IlcAOD.VertexingHF.root");
      taskvertexingHF->SelectCollisionCandidates(0);
   }   
      
   // PWGDQ JPSI filtering (only pp)
   if (iPWGDQJPSIfilter && (iCollision==0)) {
      gROOT->LoadMacro("$ILC_ROOT/PWGDQ/dielectron/macros/AddTaskJPSIFilter.C");
      IlcAnalysisTaskSE *taskJPSIfilter = AddTaskJPSIFilter();
      if (!taskJPSIfilter) ::Warning("AnalysisTrainNew", "IlcAnalysisTaskDielectronFilter cannot run for this train conditions - EXCLUDED");
      else mgr->RegisterExtraFile("IlcAOD.Dielectron.root");
      taskJPSIfilter->SelectCollisionCandidates(0);
   }   

   // PWGHF D2h
   if (iPWGHFd2h) {   
     gROOT->LoadMacro("$ILC_ROOT/PWGHF/vertexingHF/AddD2HTrain.C");
     TFile::Cp(gSystem->ExpandPathName(configPWGHFd2h.Data()), "file:ConfigVertexingHF.C");
     AddD2HTrain(kFALSE, 1,0,0,0,0,0,0,0,0,0,0);                                 
   }
   
   // ********** PWG4 wagons ******************************************************
   // Jet analysis

   // Configurations flags, move up?
   TString kDeltaAODJetName = "IlcAOD.Jets.root"; //
   Bool_t  kIsPbPb = (iCollision==0)?false:true; // can be more intlligent checking the name of the data set
   TString kDefaultJetBackgroundBranch = "";
   TString kJetSubtractBranches = "";
   UInt_t kHighPtFilterMask = 128;// from esd filter
   UInt_t iPhysicsSelectionFlag = IlcVEvent::kMB;
   if (iJETAN) {
     gROOT->LoadMacro("$ILC_ROOT/PWGJE/macros/AddTaskJets.C");
     // Default jet reconstructor running on ESD's
     IlcAnalysisTaskJets *taskjets = AddTaskJets("AOD","UA1",0.4,kHighPtFilterMask,1.,0); // no background subtraction     
     if (!taskjets) ::Fatal("AnalysisTrainNew", "IlcAnalysisTaskJets cannot run for this train conditions - EXCLUDED");
     if(kDeltaAODJetName.Length()>0) taskjets->SetNonStdOutputFile(kDeltaAODJetName.Data());
     if (iJETANdelta) {
        //            AddTaskJetsDelta("IlcAOD.Jets.root"); // need to modify this accordingly in the add task jets
        mgr->RegisterExtraFile(kDeltaAODJetName.Data());
        TString cTmp("");
        if(kIsPbPb){
          // UA1 intrinsic background subtraction
          taskjets = AddTaskJets("AOD","UA1",0.4,kHighPtFilterMask,1.,2); // background subtraction
          if(kDeltaAODJetName.Length()>0)taskjets->SetNonStdOutputFile(kDeltaAODJetName.Data());
       }
       // SICONE 
       taskjets = AddTaskJets("AOD","SISCONE",0.4,kHighPtFilterMask,0.15,0); //no background subtration to be done later....                                                                                  
       if(kDeltaAODJetName.Length()>0)taskjets->SetNonStdOutputFile(kDeltaAODJetName.Data());
       cTmp = taskjets->GetNonStdBranch();
       if(cTmp.Length()>0)kJetSubtractBranches += Form("%s ",cTmp.Data());
	 
       // Add the clusters..
       gROOT->LoadMacro("$ILC_ROOT/PWGJE/macros/AddTaskJetCluster.C");
       IlcAnalysisTaskJetCluster *taskCl = 0;
       Float_t fCenUp = 0;
       Float_t fCenLo = 0;
       Float_t fTrackEtaWindow = 0.9;
       taskCl = AddTaskJetCluster("AOD","",kHighPtFilterMask,iPhysicsSelectionFlag,"KT",0.4,0,1, kDeltaAODJetName.Data(),0.15,fTrackEtaWindow,0); // this one is for the background and random jets, random cones with no skip                                                                                 
       taskCl->SetBackgroundCalc(kTRUE);
       taskCl->SetNRandomCones(10);
       taskCl->SetCentralityCut(fCenLo,fCenUp);
       taskCl->SetGhostEtamax(fTrackEtaWindow);
       kDefaultJetBackgroundBranch = Form("%s_%s",IlcAODJetEventBackground::StdBranchName(),taskCl->GetJetOutputBranch());

       taskCl = AddTaskJetCluster("AOD","",kHighPtFilterMask,iPhysicsSelectionFlag,"ANTIKT",0.4,2,1,kDeltaAODJetName.Data(),0.15);
       taskCl->SetCentralityCut(fCenLo,fCenUp);
       if(kIsPbPb)taskCl->SetBackgroundBranch(kDefaultJetBackgroundBranch.Data());
       taskCl->SetNRandomCones(10);
       kJetSubtractBranches += Form("%s ",taskCl->GetJetOutputBranch());

       taskCl = AddTaskJetCluster("AOD","",kHighPtFilterMask,iPhysicsSelectionFlag,"ANTIKT",0.2,0,1,kDeltaAODJetName.Data(),0.15);
       taskCl->SetCentralityCut(fCenLo,fCenUp);
       if(kIsPbPb)taskCl->SetBackgroundBranch(kDefaultJetBackgroundBranch.Data());
       kJetSubtractBranches += Form("%s ",taskCl->GetJetOutputBranch());
	 
       // DO THE BACKGROUND SUBTRACTION
       if(kIsPbPb&&kJetSubtractBranches.Length()){
         gROOT->LoadMacro("$ILC_ROOT/PWGJE/macros/AddTaskJetBackgroundSubtract.C");
         IlcAnalysisTaskJetBackgroundSubtract *taskSubtract = 0;
         taskSubtract = AddTaskJetBackgroundSubtract(kJetSubtractBranches,1,"B0","B%d");
         taskSubtract->SetBackgroundBranch(kDefaultJetBackgroundBranch.Data());
         if(kDeltaAODJetName.Length()>0)taskSubtract->SetNonStdOutputFile(kDeltaAODJetName.Data());
       }
     } 
   }
}

//______________________________________________________________________________
Bool_t LoadCommonLibraries()
{
// Load common analysis libraries.
   if (!gSystem->Getenv("ILC_ROOT")) {
      ::Error("AnalysisTrainNew.C::LoadCommonLibraries", "Analysis train requires that analysis libraries are compiled with a local IlcRoot"); 
      return kFALSE;
   }   
   Bool_t success = kTRUE;
   // Load framework classes. Par option ignored here.
   success &= LoadLibrary("libSTEERBase.so");
   success &= LoadLibrary("libESD.so");
   success &= LoadLibrary("libAOD.so");
   success &= LoadLibrary("libANALYSIS.so");
   success &= LoadLibrary("libOADB.so");
   success &= LoadLibrary("libANALYSISilc.so");
   success &= LoadLibrary("libCORRFW.so");
   gROOT->ProcessLine(".include $ILC_ROOT/include");
   if (success) {
      ::Info("AnalysisTrainNew.C::LoadCommodLibraries", "Load common libraries:    SUCCESS");
      ::Info("AnalysisTrainNew.C::LoadCommodLibraries", "Include path for Aclic compilation:\n%s",
              gSystem->GetIncludePath());
   } else {           
      ::Info("AnalysisTrainNew.C::LoadCommodLibraries", "Load common libraries:    FAILED");
   }   
   return success;
}

//______________________________________________________________________________
Bool_t LoadAnalysisLibraries()
{
// Load common analysis libraries.
   if (useTender) {
      if (!LoadLibrary("TENDER") ||
          !LoadLibrary("TENDERSupplies")) return kFALSE;
   }       
   if (iESDfilter || iPWGMuonTrain) {
      if (!LoadLibrary("PWGmuon")) return kFALSE;
   }   
   // JETAN
   if (iJETAN) {
      if (!LoadLibrary("JETAN")) return kFALSE;
   }
   if (iJETANdelta) {
      if (!LoadLibrary("JETAN") ||
          !LoadLibrary("CGAL") ||
          !LoadLibrary("fastjet") ||
          !LoadLibrary("siscone") ||
          !LoadLibrary("SISConePlugin") ||
          !LoadLibrary("FASTJETAN")) return kFALSE;
   }     
   // PWG3 Vertexing HF
   if (iPWGHFvertexing || iPWG3d2h) {
      if (!LoadLibrary("PWG3base") ||
          !LoadLibrary("PWGHFvertexingHF")) return kFALSE;
   }   
   // PWG3 dielectron
   if (iPWGDQJPSIfilter) {
      if (!LoadLibrary("PWGDQdielectron")) return kFALSE;
   }   
   
   ::Info("AnalysisTrainNew.C::LoadAnalysisLibraries", "Load other libraries:   SUCCESS");
   return kTRUE;
}

//______________________________________________________________________________
Bool_t LoadLibrary(const char *module)
{
// Load a module library in a given mode. Reports success.
   Int_t result;
   TString mod(module);
   if (!mod.Length()) {
      ::Error("AnalysisTrainNew.C::LoadLibrary", "Empty module name");
      return kFALSE;
   }   
   // If a library is specified, just load it
   if (mod.EndsWith(".so")) {
      mod.Remove(mod.Index(".so"));
      result = gSystem->Load(mod);
      if (result < 0) {
         ::Error("AnalysisTrainNew.C::LoadLibrary", "Could not load library %s", module);
         return kFALSE;
      }
      return kTRUE;
   } 
   // Check if the library is already loaded
   if (strlen(gSystem->GetLibraries(Form("%s.so", module), "", kFALSE)) > 0) return kTRUE;    
   result = gSystem->Load(Form("lib%s.so", module));
   if (result < 0) {
      ::Error("AnalysisTrainNew.C::LoadLibrary", "Could not load module %s", module);
      return kFALSE;
   }
   return kTRUE;
}           


//______________________________________________________________________________
TChain *CreateChain()
{
// Create the input chain
   chain = new TChain("esdTree");
   if (gSystem->AccessPathName("IlcESDs.root")) 
      ::Error("AnalysisTrainNew.C::CreateChain", "File: IlcESDs.root not in ./data dir");
   else 
       chain->Add("IlcESDs.root");
   if (chain->GetNtrees()) return chain;
   return NULL;
}   

//______________________________________________________________________________
void AODmerge()
{
// Merging method. No staging and no terminate phase.
  TStopwatch timer;
  timer.Start();
  TString outputDir = "wn.xml";
  TString outputFiles = "EventStat_temp.root,AODQA.root,IlcAOD.root,IlcAOD.VertexingHF.root,";
  outputFiles +=        "IlcAOD.Muons.root,IlcAOD.Jets.root";
  TString mergeExcludes = "";
  TObjArray *list = outputFiles.Tokenize(",");
  TIter *iter = new TIter(list);
  TObjString *str;
  TString outputFile;
  Bool_t merged = kTRUE;
  while((str=(TObjString*)iter->Next())) {
    outputFile = str->GetString();
    // Skip already merged outputs
    if (!gSystem->AccessPathName(outputFile)) {
       printf("Output file <%s> found. Not merging again.",outputFile.Data());
       continue;
    }
    if (mergeExcludes.Contains(outputFile.Data())) continue;
    merged = IlcAnalysisAlien::MergeOutput(outputFile, outputDir, 10, 0);
    if (!merged) {
       printf("ERROR: Cannot merge %s\n", outputFile.Data());
       return;
    }
  }
  // all outputs merged, validate
  ofstream out;
  out.open("outputs_valid", ios::out);
  out.close();
  timer.Print();
}
