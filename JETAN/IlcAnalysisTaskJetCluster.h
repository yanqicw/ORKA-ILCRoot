#ifndef ILCANALYSISTASKJETCLUSTER_H
#define ILCANALYSISTASKJETCLUSTER_H
 
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

// **************************************
// task used for comparing different jets D parmaters from fastjet 
// *******************************************

#include  "IlcAnalysisTaskSE.h"
#include  "THnSparse.h" // cannot forward declare ThnSparseF
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/AreaDefinition.hh"
#include "fastjet/JetDefinition.hh"
#include "fastjet/PseudoJet.hh"

////////////////
class IlcJetHeader;
class IlcESDEvent;
class IlcAODEvent;
class IlcAODExtension;
class IlcAODJet;
class IlcGenPythiaEventHeader;
class IlcCFManager;
class IlcAODJetEventBackground;
class IlcJetFinder;
class TList;
class TChain;
class TH2F;
class TH1F;
class TH3F;
class TProfile;
class TRandom3;
class TRefArray;
class TClonesArray;
class TF1;

class IlcAnalysisTaskJetCluster : public IlcAnalysisTaskSE
{
 public:
    IlcAnalysisTaskJetCluster();
    IlcAnalysisTaskJetCluster(const char* name);
    virtual ~IlcAnalysisTaskJetCluster();
    // Implementation of interface methods
    virtual void UserCreateOutputObjects();
    virtual void Init();
    virtual void LocalInit() { Init(); }
    virtual void UserExec(Option_t *option);
    virtual void Terminate(Option_t *option);
    virtual Bool_t Notify();

    

    virtual void SetAODTrackInput(Bool_t b){fUseAODTrackInput = b;}
    virtual void SetAODMCInput(Bool_t b){fUseAODMCInput = b;}
    virtual void SetEventSelection(Bool_t b){fEventSelection = b;}
    virtual void SetRecEtaWindow(Float_t f){fRecEtaWindow = f;}
    virtual void SetTrackEtaWindow(Float_t f){fTrackEtaWindow = f;}
    virtual void SetTrackTypeGen(Int_t i){fTrackTypeGen = i;}
    virtual void SetTrackTypeRec(Int_t i){fTrackTypeRec = i;}
    virtual void SetTrackPtCut(Float_t x){fTrackPtCut = x;}
    virtual void SetCentralityCut(Float_t xLo,Float_t xUp){fCentCutLo = xLo; fCentCutUp = xUp;}
    virtual void SetFilterMask(UInt_t i,Int_t iType = 0){fFilterMask = i;
      fFilterType = iType;}
    virtual void SetJetTypes(UInt_t i){fJetTypes = i;}
    virtual void SetJetTriggerPtCut(Float_t x){fJetTriggerPtCut = x;}    
    virtual void SetVtxCuts(Float_t z,Float_t r = 1){fVtxZCut = z; fVtxR2Cut = r *r;}    
    virtual void SetBackgroundBranch(const char* c){fBackgroundBranch = c;}
    virtual const char* GetBackgroundBranch(){return fBackgroundBranch.Data();}    
    virtual void SetNSkipLeadingRan(Int_t x){fNSkipLeadingRan = x;}
    virtual void SetNSkipLeadingCone(Int_t x){fNSkipLeadingCone = x;}
    virtual void SetNRandomCones(Int_t x){fNRandomCones = x;}

    virtual void SetJetOutputBranch(const char *c){fNonStdBranch = c;}
    virtual const char* GetJetOutputBranch(){return fNonStdBranch.Data();}
    virtual void SetJetOutputFile(const char *c){fNonStdFile = c;}
    virtual const char* GetJetOutputFile(){return fNonStdFile.Data();}
    virtual void SetMaxTrackPtInJet(Float_t x){fMaxTrackPtInJet = x;}
    virtual void SetJetOutputMinPt(Float_t x){fJetOutputMinPt = x;}
    virtual void SetBackgroundCalc(Bool_t b){fUseBackgroundCalc = b;} 

    //Setters for detector level effects
    virtual void SetSmearResolution(Bool_t b){fUseTrMomentumSmearing = b;} 
    virtual void SetDiceEfficiency(Bool_t b){fUseDiceEfficiency = b;} 
    virtual void SetMomentumResolutionHybrid(TProfile *p1, TProfile *p2, TProfile *p3);
    virtual void SetEfficiencyHybrid(TH1 *h1, TH1 *h2, TH1 *h3);

    Double_t GetMomentumSmearing(Int_t cat, Double_t pt);
    void FitMomentumResolution();


    // for Fast Jet
    fastjet::JetAlgorithm        GetAlgorithm()         const {return fAlgorithm;}
    fastjet::Strategy            GetStrategy()          const {return fStrategy;}
    fastjet::RecombinationScheme GetRecombScheme()      const {return fRecombScheme;}
    fastjet::AreaType            GetAreaType()          const {return fAreaType;}
    // Setters
    void SetRparam(Double_t f)                           {fRparam = f;}
    void SetAlgorithm(fastjet::JetAlgorithm f)           {fAlgorithm = f;}
    void SetStrategy(fastjet::Strategy f)                {fStrategy = f;}
    void SetRecombScheme(fastjet::RecombinationScheme f) {fRecombScheme = f;}
    void SetAreaType(fastjet::AreaType f)                {fAreaType = f;}
    void SetGhostArea(Double_t f) {fGhostArea = f;}
    void SetActiveAreaRepeats(Int_t f) {fActiveAreaRepeats = f;}
    void SetGhostEtamax(Double_t f) {fGhostEtamax = f;}



    // Helper
    //

    // we have different cases
    // AOD reading -> MC from AOD
    // ESD reading -> MC from Kinematics
    // this has to match with our selection of input events
    enum {kTrackUndef = 0, kTrackAOD, kTrackKineAll,kTrackKineCharged, kTrackAODMCAll, kTrackAODMCCharged, kTrackAODMCChargedAcceptance, kTrackAODextra, kTrackAODextraonly};
    enum {kMaxJets = 4};
    enum {kMaxCorrelation =  3};
    enum {kMaxRadius =       5};
    enum {kMaxCent =         4};
    enum {kJet = 1<<0,
	  kJetRan = 1<<1,	  
	  kRC = 1<<2,
	  kRCRan = 1<<3
    };
    

 private:

    IlcAnalysisTaskJetCluster(const IlcAnalysisTaskJetCluster&);
    IlcAnalysisTaskJetCluster& operator=(const IlcAnalysisTaskJetCluster&);

    Int_t GetListOfTracks(TList *list,Int_t type);

    IlcAODEvent     *fAOD;                // ! where we take the jets from can be input or output AOD
    IlcAODExtension *fAODExtension;       // ! AOD extension in case we write a non-sdt branch to a separate file and the aod is standard
    TRefArray       *fRef;               // ! trefarray for track references within the jet
    Bool_t        fUseAODTrackInput;      // take track from input AOD not from ouptu AOD
    Bool_t        fUseAODMCInput;         // take MC from input AOD not from ouptu AOD
    Bool_t        fUseBackgroundCalc;     // switches on background calculations
    Bool_t        fEventSelection;        // use the event selection of this task, otherwise analyse all
    UInt_t        fFilterMask;            // filter bit for slecected tracks
    UInt_t        fFilterType;            // filter type 0 = all, 1 = ITSTPC, 2 = TPC
    UInt_t        fJetTypes;              // 1<<0 regular jets, 1<<1 << randomized event 1<<2 random cones 1<<3 random cones randomiuzed event
    Int_t         fTrackTypeRec;          // type of tracks used for FF 
    Int_t         fTrackTypeGen;          // type of tracks used for FF 
    Int_t         fNSkipLeadingRan;       // number of leading tracks to be skipped in the randomized event
    Int_t         fNSkipLeadingCone;      // number of leading jets to be for the random cones
    Int_t         fNRandomCones;          // number of generated random cones
    Float_t       fAvgTrials;             // Average nimber of trials
    Float_t       fExternalWeight;        // external weight
    Float_t       fTrackEtaWindow;        // eta window used for corraltion plots between rec and gen 
    Float_t       fRecEtaWindow;          // eta window used for corraltion plots between rec and gen 
    Float_t       fTrackPtCut;            // minimum track pt to be accepted
    Float_t       fJetOutputMinPt;        // minimum p_t for jets to be written out
    Float_t       fMaxTrackPtInJet;       // maximum track pt within a jet for flagging...
    Float_t       fJetTriggerPtCut;       // minimum jwt pT for AOD to be written
    Float_t       fVtxZCut;               // zvtx cut
    Float_t       fVtxR2Cut;              // R vtx cut (squared) 
    Float_t       fCentCutUp;             // upper limit on centrality
    Float_t       fCentCutLo;             // lower limit on centrality
    // output configurartion
    TString       fNonStdBranch;      // the name of the non-std branch name, if empty no branch is filled
    TString       fBackgroundBranch;  // name of the branch used for background subtraction
    TString       fNonStdFile;        // The optional name of the output file the non-std branch is written to

    //Detector level effects
    TProfile *fMomResH1; // Momentum resolution from TrackQA Hybrid Category 1
    TProfile *fMomResH2; // Momentum resolution from TrackQA Hybrid Category 2
    TProfile *fMomResH3; // Momentum resolution from TrackQA Hybrid Category 3
    TF1 *fMomResH1Fit; //fit
    TF1 *fMomResH2Fit; //fit
    TF1 *fMomResH3Fit; //fit

    TH1      *fhEffH1;        // Efficiency for Spectra Hybrid Category 1
    TH1      *fhEffH2;        // Efficiency for Spectra Hybrid Category 2
    TH1      *fhEffH3;        // Efficiency for Spectra Hybrid Category 3
    Bool_t    fUseTrMomentumSmearing;     // Apply momentum smearing on track level
    Bool_t    fUseDiceEfficiency;         // Apply efficiency on track level by dicing

    // Fast jet
    Double_t fRparam;                  // fastjet distance parameter
    fastjet::JetAlgorithm fAlgorithm; //fastjet::kt_algorithm
    fastjet::Strategy fStrategy;  //= fastjet::Best;
    fastjet::RecombinationScheme fRecombScheme; // = fastjet::BIpt_scheme;
    fastjet::AreaType fAreaType;  // fastjet area type
    Double_t fGhostArea;          // fasjet ghost area
    Int_t fActiveAreaRepeats;     // fast jet active area repeats
    Double_t fGhostEtamax;        // fast jet ghost area

    TClonesArray  *fTCAJetsOut;           //! TCA of output jets
    TClonesArray  *fTCAJetsOutRan;        //! TCA of output jets in randomized event
    TClonesArray  *fTCARandomConesOut;    //! TCA of output jets in randomized event
    TClonesArray  *fTCARandomConesOutRan; //! TCA of output jets in randomized event
    IlcAODJetEventBackground *fAODJetBackgroundOut; //! jet background to be written out

    TRandom3*     fRandom;   //! random number generator
    TProfile*     fh1Xsec;   //! pythia cross section and trials
    TH1F*         fh1Trials; //! trials are added
    TH1F*         fh1PtHard;  //! Pt har of the event...       
    TH1F*         fh1PtHardNoW;  //! Pt har of the event without weigt       
    TH1F*         fh1PtHardTrials;  //! Number of trials 

    TH1F*         fh1NJetsRec; //! number of reconstructed jets
    TH1F*         fh1NConstRec;//! number of constiutens in leading jet
    TH1F*         fh1NConstLeadingRec;//! number of constiutens in leading jet
    TH1F*         fh1PtJetsRecIn;  //! Jet pt for all jets
    TH1F*         fh1PtJetsLeadingRecIn;  //! Jet pt for all jets
    TH1F*         fh1PtJetConstRec;//! pt of constituents
    TH1F*         fh1PtJetConstLeadingRec;// pt of constituents
    TH1F*         fh1PtTracksRecIn;  //! track pt for all tracks
    TH1F*         fh1PtTracksLeadingRecIn;  //! track pt for all tracks

    // Randomized track histos
    TH1F*         fh1NJetsRecRan; //! number of reconstructed jets from randomized
    TH1F*         fh1NConstRecRan;//! number of constiutens in leading jet
    TH1F*         fh1PtJetsLeadingRecInRan;  //! Jet pt for all jets
    TH1F*         fh1NConstLeadingRecRan;//! number of constiutens in leading jet
    TH1F*         fh1PtJetsRecInRan;  //! Jet pt for all jets

    TH1F*         fh1PtTracksGenIn;  //! track pt for all tracks
    TH1F*         fh1Nch;            //! charged particle mult
    TH1F*         fh1BiARandomCones[3]; //! Residual distribtion from reandom cones on real event
    TH1F*         fh1BiARandomConesRan[3]; //! Residual distribtion from reandom cones on random event
    TH1F*         fh1CentralityPhySel;          // ! centrality of anaylsed events 
    TH1F*         fh1Centrality;                // ! centrality of anaylsed events 
    TH1F*         fh1CentralitySelect;          // ! centrality of selected events 
    TH1F*         fh1ZPhySel;          // ! centrality of anaylsed events 
    TH1F*         fh1Z;                // ! centrality of anaylsed events 
    TH1F*         fh1ZSelect;          // ! centrality of selected events 


    TH2F*         fh2NRecJetsPt;            //! Number of found jets above threshold
    TH2F*         fh2NRecTracksPt;          //! Number of found tracks above threshold
    TH2F*         fh2NConstPt;           //! number of constituents vs. pt
    TH2F*         fh2NConstLeadingPt;           //! number of constituents vs. pt
    TH2F*         fh2JetPhiEta;             //! jet phi eta
    TH2F*         fh2LeadingJetPhiEta;      //! leading jet phi eta
    TH2F*         fh2JetEtaPt;              //! leading jet eta
    TH2F*         fh2LeadingJetEtaPt;              //! leading jet eta
    TH2F*         fh2TrackEtaPt;              //! track eta
    TH2F*         fh2LeadingTrackEtaPt;       //! leading track eta
    TH2F*         fh2JetsLeadingPhiEta;     //! jet phi eta
    TH2F*         fh2JetsLeadingPhiPt;      //! jet correlation with leading jet
    TH2F*         fh2TracksLeadingPhiEta;   //! track correlation with leading track
    TH2F*         fh2TracksLeadingPhiPt;    //! track correlation with leading track
    TH2F*         fh2TracksLeadingJetPhiPt; //! track correlation with leading Jet
    TH2F*         fh2JetsLeadingPhiPtW;      //! jet correlation with leading jet
    TH2F*         fh2TracksLeadingPhiPtW;   //! track correlation with leading track
    TH2F*         fh2TracksLeadingJetPhiPtW; //! track correlation with leading Jet
    TH2F*         fh2NRecJetsPtRan;            //! Number of found jets above threshold
    TH2F*         fh2NConstPtRan;           //! number of constituents vs. pt
    TH2F*         fh2NConstLeadingPtRan;           //! number of constituents vs. pt
    TH2F*         fh2PtNch;               //! p_T of cluster vs. multiplicity,
    TH2F*         fh2PtNchRan;            //! p_T of cluster vs. multiplicity,random
    TH2F*         fh2PtNchN;               //! p_T of cluster vs. multiplicity, weigthed with constituents
    TH2F*         fh2PtNchNRan;            //! p_T of cluster vs. multiplicity, weigthed with constituents random
    TH2F*         fh2TracksLeadingJetPhiPtRan; //! track correlation with leading Jet
    TH2F*         fh2TracksLeadingJetPhiPtWRan; //! track correlation with leading Jet


    TH2F*         fh2JetsLeadingPhiPtC[kMaxCent]; //! jet correlation with leading jet    
    TH2F*         fh2JetsLeadingPhiPtWC[kMaxCent];      //! jet correlation with leading jet
    TH2F*         fh2TracksLeadingJetPhiPtC[kMaxCent]; //! track correlation with leading Jet
    TH2F*         fh2TracksLeadingJetPhiPtWC[kMaxCent]; //! track correlation with leading Jet

    //Histos for detector level effects from toy model
    TH2F *fh2PtGenPtSmeared;     //! Control histo smeared momentum
    TProfile *fp1Efficiency;     //! Control profile efficiency
    TProfile *fp1PtResolution;   //! Control profile for pT resolution

    TList *fHistList; //!leading tracks to be skipped in the randomized event Output list
   

    ClassDef(IlcAnalysisTaskJetCluster, 20) 
};
 
#endif
