//DEFINITION OF A FEW CONSTANTS
const Double_t ymin  = -1.0 ;
const Double_t ymax  =  1.0 ;
const Double_t ptmin =  0.0 ;
const Double_t ptmax =  8.0 ;
const Int_t    mintrackrefsTPC = 2 ;
const Int_t    mintrackrefsITS = 3 ;
const Int_t    charge  = 1 ;
const Int_t    PDG = 2212; 
const Int_t    minclustersTPC = 50 ;
//----------------------------------------------------

Bool_t IlcCFSingleTrackTask(
			    const Bool_t useGrid = 1,
			    const Bool_t readAOD = 0,
			    const Bool_t readTPCTracks = 0,
			    const char * kTagXMLFile="wn.xml" // XML file containing tags
			    )
{
  
  TBenchmark benchmark;
  benchmark.Start("IlcSingleTrackTask");

  IlcLog::SetGlobalDebugLevel(0);

  Load() ; //load the required libraries

  TChain * analysisChain ;

  if (useGrid) { //data located on AliEn
    TGrid::Connect("alien://") ;    //  Create an IlcRunTagCuts and an IlcEventTagCuts Object 
                                    //  and impose some selection criteria
    IlcRunTagCuts      *runCuts   = new IlcRunTagCuts(); 
    IlcEventTagCuts    *eventCuts = new IlcEventTagCuts(); 
    IlcLHCTagCuts      *lhcCuts   = new IlcLHCTagCuts(); 
    IlcDetectorTagCuts *detCuts   = new IlcDetectorTagCuts(); 
    eventCuts->SetMultiplicityRange(0,2000);

    //  Create an IlcTagAnalysis Object and chain the tags
    IlcTagAnalysis   *tagAna = new IlcTagAnalysis(); 
    if (readAOD) tagAna->SetType("AOD");  //for ilcroot > v4-05
    else         tagAna->SetType("ESD");  //for ilcroot > v4-05
    TAlienCollection *coll   = TAlienCollection::Open(kTagXMLFile); 
    TGridResult      *tagResult = coll->GetGridResult("",0,0);
    tagResult->Print();
    tagAna->ChainGridTags(tagResult);

    //  Create a new esd chain and assign the chain that is returned by querying the tags
    analysisChain = tagAna->QueryTags(runCuts,lhcCuts,detCuts,eventCuts); 
  }

  else {// local data
    //here put your input data path
    printf("\n\nRunning on local file, please check the path\n\n");

    if (readAOD) {
      analysisChain = new TChain("aodTree");
      analysisChain->Add("your_data_path/001/IlcAOD.root");
      analysisChain->Add("your_data_path/002/IlcAOD.root");
    }
    else {
      analysisChain = new TChain("esdTree");
      analysisChain->Add("your_data_path/001/IlcESDs.root");
      analysisChain->Add("your_data_path/002/IlcESDs.root");
    }
  }
  

  Info("IlcCFSingleTrackTask",Form("CHAIN HAS %d ENTRIES",(Int_t)analysisChain->GetEntries()));

  //CONTAINER DEFINITION
  Info("IlcCFSingleTrackTask","SETUP CONTAINER");
  //the sensitive variables (2 in this example), their indices
  UInt_t ipt = 0;
  UInt_t iy  = 1;
  //Setting up the container grid... 
  UInt_t nstep = 4 ; //number of selection steps MC 
  const Int_t nvar   = 2 ; //number of variables on the grid:pt,y
  const Int_t nbin1  = 8 ; //bins in pt
  const Int_t nbin2  = 8 ; //bins in y 

  //arrays for the number of bins in each dimension
  Int_t iBin[nvar];
  iBin[0]=nbin1;
  iBin[1]=nbin2;

  //arrays for lower bounds :
  Double_t *binLim1=new Double_t[nbin1+1];
  Double_t *binLim2=new Double_t[nbin2+1];

  //values for bin lower bounds
  for(Int_t i=0; i<=nbin1; i++) binLim1[i]=(Double_t)ptmin + (ptmax-ptmin)/nbin1*(Double_t)i ; 
  for(Int_t i=0; i<=nbin2; i++) binLim2[i]=(Double_t)ymin  + (ymax-ymin)  /nbin2*(Double_t)i ;
  //one "container" for MC
  IlcCFContainer* container = new IlcCFContainer("container","container for tracks",nstep,nvar,iBin);
  //setting the bin limits
  container -> SetBinLimits(ipt,binLim1);
  container -> SetBinLimits(iy,binLim2);
  container -> SetVarTitle(ipt,"pt");
  container -> SetVarTitle(iy, "y");
  container -> SetStepTitle(0, "generated");
  container -> SetStepTitle(1, "in acceptance");
  container -> SetStepTitle(2, "reconstructed");
  container -> SetStepTitle(3, "after PID");

  // SET TLIST FOR QA HISTOS
  TList* qaList = new TList();

  //CREATE THE  CUTS -----------------------------------------------

  //Event-level cuts:
  IlcCFEventRecCuts* evtRecCuts = new IlcCFEventRecCuts("evtRecCuts","Rec-event cuts");
//   evtRecCuts->SetUseTPCVertex();
//   evtRecCuts->SetRequireVtxCuts(kTRUE);
//   evtRecCuts->SetVertexNContributors(-2,5);
  evtRecCuts->SetQAOn(qaList);

  // Gen-Level kinematic cuts
  IlcCFTrackKineCuts *mcKineCuts = new IlcCFTrackKineCuts("mcKineCuts","MC-level kinematic cuts");
  mcKineCuts->SetPtRange(ptmin,ptmax);
  mcKineCuts->SetRapidityRange(ymin,ymax);
  mcKineCuts->SetChargeMC(charge);
  mcKineCuts->SetQAOn(qaList);

  //Particle-Level cuts:  
  IlcCFParticleGenCuts* mcGenCuts = new IlcCFParticleGenCuts("mcGenCuts","MC particle generation cuts");
  mcGenCuts->SetRequireIsPrimary();
  mcGenCuts->SetRequirePdgCode(PDG,/*absolute=*/kTRUE);
  mcGenCuts->SetQAOn(qaList);

  //Acceptance Cuts
  IlcCFAcceptanceCuts *mcAccCuts = new IlcCFAcceptanceCuts("mcAccCuts","MC acceptance cuts");
  mcAccCuts->SetMinNHitITS(mintrackrefsITS);
  mcAccCuts->SetMinNHitTPC(mintrackrefsTPC);
  mcAccCuts->SetQAOn(qaList);

  // Rec-Level kinematic cuts
  IlcCFTrackKineCuts *recKineCuts = new IlcCFTrackKineCuts("recKineCuts","rec-level kine cuts");
  recKineCuts->SetPtRange(ptmin,ptmax);
  recKineCuts->SetRapidityRange(ymin,ymax);
  recKineCuts->SetChargeRec(charge);
  recKineCuts->SetQAOn(qaList);

  IlcCFTrackQualityCuts *recQualityCuts = new IlcCFTrackQualityCuts("recQualityCuts","rec-level quality cuts");
  if (!readAOD)       {
//     recQualityCuts->SetMinNClusterTRD(0);
//     recQualityCuts->SetMaxChi2PerClusterTRD(10.);
  }
  recQualityCuts->SetStatus(IlcESDtrack::kTPCrefit);
  recQualityCuts->SetQAOn(qaList);

  IlcCFTrackIsPrimaryCuts *recIsPrimaryCuts = new IlcCFTrackIsPrimaryCuts("recIsPrimaryCuts","rec-level isPrimary cuts");
  if (readAOD) recIsPrimaryCuts->SetAODType(IlcAODTrack::kPrimary);
  else         recIsPrimaryCuts->SetMaxNSigmaToVertex(3);
  recIsPrimaryCuts->SetQAOn(qaList);

  IlcCFTrackCutPid* cutPID = new IlcCFTrackCutPid("cutPID","ESD_PID") ;
  int n_species = IlcPID::kSPECIES ;
  Double_t* prior = new Double_t[n_species];
  
  prior[0] = 0.0244519 ;
  prior[1] = 0.0143988 ;
  prior[2] = 0.805747  ;
  prior[3] = 0.0928785 ;
  prior[4] = 0.0625243 ;
  
  cutPID->SetPriors(prior);
  cutPID->SetProbabilityCut(0.0);
  if (readTPCTracks) cutPID->SetDetectors("TPC");
  else               cutPID->SetDetectors("ALL");
  if (readAOD) cutPID->SetAODmode(kTRUE );
  else         cutPID->SetAODmode(kFALSE);
  switch(TMath::Abs(PDG)) {
  case 11   : cutPID->SetParticleType(IlcPID::kElectron, kTRUE); break;
  case 13   : cutPID->SetParticleType(IlcPID::kMuon    , kTRUE); break;
  case 211  : cutPID->SetParticleType(IlcPID::kPion    , kTRUE); break;
  case 321  : cutPID->SetParticleType(IlcPID::kKaon    , kTRUE); break;
  case 2212 : cutPID->SetParticleType(IlcPID::kProton  , kTRUE); break;
  default   : printf("UNDEFINED PID\n"); break;
  }
  cutPID->SetQAOn(qaList);

  printf("CREATE EVENT LEVEL CUTS\n");
  TObjArray* evtList = new TObjArray(0) ;
//   evtList->AddLast(evtRecCuts);
  
  printf("CREATE MC KINE CUTS\n");
  TObjArray* mcList = new TObjArray(0) ;
  mcList->AddLast(mcKineCuts);
  mcList->AddLast(mcGenCuts);

  printf("CREATE ACCEPTANCE CUTS\n");
  TObjArray* accList = new TObjArray(0) ;
  accList->AddLast(mcAccCuts);

  printf("CREATE RECONSTRUCTION CUTS\n");
  TObjArray* recList = new TObjArray(0) ;
  recList->AddLast(recKineCuts);
  recList->AddLast(recQualityCuts);
  recList->AddLast(recIsPrimaryCuts);

  printf("CREATE PID CUTS\n");
  TObjArray* fPIDCutList = new TObjArray(0) ;
  fPIDCutList->AddLast(cutPID);

  //CREATE THE INTERFACE TO CORRECTION FRAMEWORK USED IN THE TASK
  printf("CREATE INTERFACE AND CUTS\n");
  IlcCFManager* man = new IlcCFManager() ;

  man->SetNStepEvent(1);
  man->SetEventCutsList(0,evtList);

  man->SetParticleContainer(container);
  man->SetParticleCutsList(0,mcList);
  man->SetParticleCutsList(1,accList);
  man->SetParticleCutsList(2,recList);
  man->SetParticleCutsList(3,fPIDCutList);


  //CREATE THE TASK
  printf("CREATE TASK\n");
  // create the task
  IlcCFSingleTrackTask *task = new IlcCFSingleTrackTask("IlcSingleTrackTask");
  task->SetCFManager(man); //here is set the CF manager
  task->SetQAList(qaList);
  if (readAOD)       task->SetReadAODData() ;
  if (readTPCTracks) task->SetReadTPCTracks();

  //SETUP THE ANALYSIS MANAGER TO READ INPUT CHAIN AND WRITE DESIRED OUTPUTS
  printf("CREATE ANALYSIS MANAGER\n");
  // Make the analysis manager
  IlcAnalysisManager *mgr = new IlcAnalysisManager("TestManager");

  if (useGrid) mgr->SetAnalysisType(IlcAnalysisManager::kGridAnalysis);
  else mgr->SetAnalysisType(IlcAnalysisManager::kLocalAnalysis);


  IlcMCEventHandler*  mcHandler = new IlcMCEventHandler();
  mgr->SetMCtruthEventHandler(mcHandler);
 
  IlcInputEventHandler* dataHandler ;
  
  if   (readAOD) dataHandler = new IlcAODInputHandler();
  else           dataHandler = new IlcESDInputHandler();
  mgr->SetInputEventHandler(dataHandler);

  // Create and connect containers for input/output

  //------ input data ------
  IlcAnalysisDataContainer *cinput0  = mgr->CreateContainer("cchain0",TChain::Class(),IlcAnalysisManager::kInputContainer);

  // ----- output data -----
  
  //slot 0 : default output tree (by default handled by IlcAnalysisTaskSE)
  IlcAnalysisDataContainer *coutput0 = mgr->CreateContainer("ctree0", TTree::Class(),IlcAnalysisManager::kOutputContainer,"output.root");

  //now comes user's output objects :
  
  // output TH1I for event counting
  IlcAnalysisDataContainer *coutput1 = mgr->CreateContainer("chist0", TH1I::Class(),IlcAnalysisManager::kOutputContainer,"output.root");
  // output Correction Framework Container (for acceptance & efficiency calculations)
  IlcAnalysisDataContainer *coutput2 = mgr->CreateContainer("ccontainer0", IlcCFContainer::Class(),IlcAnalysisManager::kOutputContainer,"output.root");
  // output QA histograms 
  IlcAnalysisDataContainer *coutput3 = mgr->CreateContainer("clist0", TList::Class(),IlcAnalysisManager::kOutputContainer,"output.root");

  cinput0->SetData(analysisChain);

  mgr->AddTask(task);
  mgr->ConnectInput(task,0,mgr->GetCommonInputContainer());
  mgr->ConnectOutput(task,0,coutput0);
  mgr->ConnectOutput(task,1,coutput1);
  mgr->ConnectOutput(task,2,coutput2);
  mgr->ConnectOutput(task,3,coutput3);
 
  printf("READY TO RUN\n");
  //RUN !!!
  if (mgr->InitAnalysis()) {
    mgr->PrintStatus();
    mgr->StartAnalysis("local",analysisChain);
  }

  benchmark.Stop("IlcSingleTrackTask");
  benchmark.Show("IlcSingleTrackTask");

  return kTRUE ;
}

void Load() {

  //load the required ilcroot libraries
  gSystem->Load("libANALYSIS") ;
  gSystem->Load("libANALYSISilc") ;
  gSystem->Load("libCORRFW.so") ;

  //compile online the task class
  gSystem->SetIncludePath("-I. -I$ILC_ROOT/include -I$ROOTSYS/include");
  gROOT->LoadMacro("./IlcCFSingleTrackTask.cxx++g");
}
