// One can use the configuration macro in compiled mode by
// root [0] gSystem->Load("libgeant321");
// root [0] gSystem->SetIncludePath("-I$ROOTSYS/include -I$ILC_ROOT/include\
//                   -I$ILC_ROOT -I$ILC/geant3/TGeant3");
// root [0] .x grun.C(1,"Config.C++")

#if !defined(__CINT__) || defined(__MAKECINT__)
#include <Riostream.h>
#include <TPDGCode.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TVirtualMC.h>
#include <TGeant3TGeo.h>
#include "STEER/IlcRunLoader.h"
#include "STEER/IlcRun.h"
#include "STEER/IlcConfig.h"
#include "PYTHIA6/IlcDecayerPythia.h"
#include "EVGEN/IlcGenCocktail.h"
#include "EVGEN/IlcGenHIJINGpara.h"
#include "EVGEN/IlcGenFixed.h"
#include "EVGEN/IlcGenBox.h"
#include "STEER/IlcMagF.h"
#include "STEER/IlcPID.h"
#include "STRUCT/IlcBODY.h"
#include "STRUCT/IlcMAG.h"
#include "STRUCT/IlcABSOv3.h"
#include "STRUCT/IlcDIPOv3.h"
#include "STRUCT/IlcHALLv3.h"
#include "STRUCT/IlcFRAMEv2.h"
#include "STRUCT/IlcSHILv3.h"
#include "STRUCT/IlcPIPEv3.h"
#include "ITS/IlcITSv11.h"
#include "TPC/IlcTPCv2.h"
#include "TOF/IlcTOFv6T0.h"
#include "HMPID/IlcHMPIDv3.h"
#include "ZDC/IlcZDCv4.h"
#include "TRD/IlcTRDv1.h"
#include "TRD/IlcTRDgeometry.h"
#include "FMD/IlcFMDv1.h"
#include "MUON/IlcMUONv1.h"
#include "PHOS/IlcPHOSv1.h"
#include "PMD/IlcPMDv1.h"
#include "T0/IlcT0v1.h"
#include "EMCAL/IlcEMCALv2.h"
#include "ACORDE/IlcACORDEv1.h"
#include "VZERO/IlcVZEROv7.h"
#endif

enum PprTrigConf_t
{
    kDefaultPPTrig, kDefaultPbPbTrig
};

const char * pprTrigConfName[] = {
    "p-p","Pb-Pb"
};

Float_t EtaToTheta(Float_t arg);

static IlcMagF::BeamType_t beamType = IlcMagF::kBeamTypeAA;
static Double_t            beamEnergy = 7000.*82./208;
static PprTrigConf_t strig = kDefaultPPTrig;// default PP trigger configuration

void Config()
{
    // ThetaRange is (0., 180.). It was (0.28,179.72) 7/12/00 09:00
    // Theta range given through pseudorapidity limits 22/6/2001

    // Set Random Number seed
  gRandom->SetSeed(123456); // Set 0 to use the currecnt time


   // libraries required by geant321
#if defined(__CINT__)
    gSystem->Load("liblhapdf");
    gSystem->Load("libEGPythia6");
    gSystem->Load("libpythia6");
    gSystem->Load("libIlcPythia6");
    gSystem->Load("libgeant321");
#endif

    new     TGeant3TGeo("C++ Interface to Geant3");

    IlcRunLoader* rl=0x0;


    rl = IlcRunLoader::Open("gilc.root",
			    IlcConfig::GetDefaultEventFolderName(),
			    "recreate");
    if (rl == 0x0)
      {
	gIlc->Fatal("Config.C","Can not instatiate the Run Loader");
	return;
      }
    rl->SetCompressionLevel(2);
    rl->SetNumberOfEventsPerFile(3);
    gIlc->SetRunLoader(rl);

    // Set the trigger configuration
    IlcSimulation::Instance()->SetTriggerConfig(pprTrigConfName[strig]);
    cout<<"Trigger configuration is set to  "<<pprTrigConfName[strig]<<endl;

    //
    // Set External decayer
    TVirtualMCDecayer *decayer = new IlcDecayerPythia();

    decayer->SetForceDecay(kAll);
    decayer->Init();
    gMC->SetExternalDecayer(decayer);
    //=======================================================================
    // ************* STEERING parameters FOR ILC SIMULATION **************
    // --- Specify event type to be tracked through the ILC setup
    // --- All positions are in cm, angles in degrees, and P and E in GeV


    gMC->SetProcess("DCAY",1);
    gMC->SetProcess("PAIR",1);
    gMC->SetProcess("COMP",1);
    gMC->SetProcess("PHOT",1);
    gMC->SetProcess("PFIS",0);
    gMC->SetProcess("DRAY",0);
    gMC->SetProcess("ANNI",1);
    gMC->SetProcess("BREM",1);
    gMC->SetProcess("MUNU",1);
    gMC->SetProcess("CKOV",1);
    gMC->SetProcess("HADR",1);
    gMC->SetProcess("LOSS",2);
    gMC->SetProcess("MULS",1);
    gMC->SetProcess("RAYL",1);

    Float_t cut = 1.e-3;        // 1MeV cut by default
    Float_t tofmax = 1.e10;

    gMC->SetCut("CUTGAM", cut);
    gMC->SetCut("CUTELE", cut);
    gMC->SetCut("CUTNEU", cut);
    gMC->SetCut("CUTHAD", cut);
    gMC->SetCut("CUTMUO", cut);
    gMC->SetCut("BCUTE",  cut); 
    gMC->SetCut("BCUTM",  cut); 
    gMC->SetCut("DCUTE",  cut); 
    gMC->SetCut("DCUTM",  cut); 
    gMC->SetCut("PPCUTM", cut);
    gMC->SetCut("TOFMAX", tofmax); 

    // Special generation for Valgrind tests
    // Each detector is fired by few particles selected 
    // to cover specific cases


    // The cocktail itself

    IlcGenCocktail *gener = new IlcGenCocktail();
    gener->SetEnergyCMS(beamEnergy); // Needed by ZDC
    gener->SetPhiRange(0, 360);
    // Set pseudorapidity range from -8 to 8.
    Float_t thmin = EtaToTheta(8);   // theta min. <---> eta max
    Float_t thmax = EtaToTheta(-8);  // theta max. <---> eta min 
    gener->SetThetaRange(thmin,thmax);
    gener->SetOrigin(0, 0, 0);  //vertex position
    gener->SetSigma(0, 0, 0);   //Sigma in (X,Y,Z) (cm) on IP position


    // Particle guns for the barrel part (taken from RichConfig)

    IlcGenFixed *pG1=new IlcGenFixed(1);
    pG1->SetPart(kProton);
    pG1->SetMomentum(2.5);
    pG1->SetTheta(109.5-3);
    pG1->SetPhi(10);
    gener->AddGenerator(pG1,"g1",1);
    
    IlcGenFixed *pG2=new IlcGenFixed(1);
    pG2->SetPart(kPiPlus);
    pG2->SetMomentum(1.0);
    pG2->SetTheta( 90.0-3);
    pG2->SetPhi(10);
    gener->AddGenerator(pG2,"g2",1);

    IlcGenFixed *pG3=new IlcGenFixed(1);
    pG3->SetPart(kPiMinus);
    pG3->SetMomentum(1.5);
    pG3->SetTheta(109.5-3);
    pG3->SetPhi(30);
    gener->AddGenerator(pG3,"g3",1);
    
    IlcGenFixed *pG4=new IlcGenFixed(1);
    pG4->SetPart(kKPlus);
    pG4->SetMomentum(0.7);
    pG4->SetTheta( 90.0-3);
    pG4->SetPhi(30);
    gener->AddGenerator(pG4,"g4",1);
    
    IlcGenFixed *pG5=new IlcGenFixed(1);
    pG5->SetPart(kKMinus);
    pG5->SetMomentum(1.0);
    pG5->SetTheta( 70.0-3);
    pG5->SetPhi(30);
    gener->AddGenerator(pG5,"g5",1);
    
    IlcGenFixed *pG6=new IlcGenFixed(1);
    pG6->SetPart(kProtonBar);
    pG6->SetMomentum(2.5);
    pG6->SetTheta( 90.0-3);
    pG6->SetPhi(50);
    gener->AddGenerator(pG6,"g6",1);
    
    IlcGenFixed *pG7=new IlcGenFixed(1);
    pG7->SetPart(kPiMinus);
    pG7->SetMomentum(0.7);
    pG7->SetTheta( 70.0-3);
    pG7->SetPhi(50);
    gener->AddGenerator(pG7,"g7",1);

    // Electrons for TRD

    IlcGenFixed *pG8=new IlcGenFixed(1);
    pG8->SetPart(kElectron);
    pG8->SetMomentum(1.2);
    pG8->SetTheta( 95.0);
    pG8->SetPhi(190);
    gener->AddGenerator(pG8,"g8",1);

    IlcGenFixed *pG9=new IlcGenFixed(1);
    pG9->SetPart(kPositron);
    pG9->SetMomentum(1.2);
    pG9->SetTheta( 85.0);
    pG9->SetPhi(190);
    gener->AddGenerator(pG9,"g9",1);

    // PHOS

    IlcGenBox *gphos = new IlcGenBox(1);
    gphos->SetMomentumRange(10,11.);
    gphos->SetPhiRange(270.5,270.7);
    gphos->SetThetaRange(90.5,90.7);
    gphos->SetPart(kGamma);
    gener->AddGenerator(gphos,"GENBOX GAMMA for PHOS",1);

    // EMCAL

    IlcGenBox *gemcal = new IlcGenBox(1);
    gemcal->SetMomentumRange(10,11.);
    gemcal->SetPhiRange(90.5,199.5);
    gemcal->SetThetaRange(90.5,90.7);
    gemcal->SetPart(kGamma);
    gener->AddGenerator(gemcal,"GENBOX GAMMA for EMCAL",1);

    // MUON
    IlcGenBox * gmuon1 = new IlcGenBox(1);
    gmuon1->SetMomentumRange(20.,20.1);
    gmuon1->SetPhiRange(0., 360.);         
    gmuon1->SetThetaRange(171.000,178.001);
    gmuon1->SetPart(kMuonMinus);           // Muons
    gener->AddGenerator(gmuon1,"GENBOX MUON1",1);

    IlcGenBox * gmuon2 = new IlcGenBox(1);
    gmuon2->SetMomentumRange(20.,20.1);
    gmuon2->SetPhiRange(0., 360.);         
    gmuon2->SetThetaRange(171.000,178.001);
    gmuon2->SetPart(kMuonPlus);           // Muons
    gener->AddGenerator(gmuon2,"GENBOX MUON1",1);

    //TOF
    IlcGenFixed *gtof=new IlcGenFixed(1);
    gtof->SetPart(kProton);
    gtof->SetMomentum(2.5);
    gtof->SetTheta(95);
    gtof->SetPhi(340);
    gener->AddGenerator(gtof,"Proton for TOF",1);

    //FMD1
    IlcGenFixed *gfmd1=new IlcGenFixed(1);
    gfmd1->SetPart(kGamma);
    gfmd1->SetMomentum(25);
    gfmd1->SetTheta(1.8);
    gfmd1->SetPhi(10);
    gener->AddGenerator(gfmd1,"Gamma for FMD1",1);
    
    //FMD2i
    IlcGenFixed *gfmd2i=new IlcGenFixed(1);
    gfmd2i->SetPart(kPiPlus);
    gfmd2i->SetMomentum(1.5);
    gfmd2i->SetTheta(7.3);
    gfmd2i->SetPhi(20);
    gener->AddGenerator(gfmd2i,"Pi+ for FMD2i",1);
    
    //FMD2o
    IlcGenFixed *gfmd2o=new IlcGenFixed(1);
    gfmd2o->SetPart(kPiMinus);
    gfmd2o->SetMomentum(1.5);
    gfmd2o->SetTheta(16.1);
    gfmd2o->SetPhi(30);
    gener->AddGenerator(gfmd2o,"Pi- for FMD2o",1);
    
    //FMD3o
    IlcGenFixed *gfmd3o=new IlcGenFixed(1);
    gfmd3o->SetPart(kPiPlus);
    gfmd3o->SetMomentum(1.5);
    gfmd3o->SetTheta(163.9);
    gfmd3o->SetPhi(40);
    gener->AddGenerator(gfmd3o,"Pi+ for FMD3o",1);
    
    //FMD3i
    IlcGenFixed *gfmd3i=new IlcGenFixed(1);
    gfmd3i->SetPart(kPiMinus);
    gfmd3i->SetMomentum(1.5);
    gfmd3i->SetTheta(170.5);
    gfmd3i->SetPhi(50);
    gener->AddGenerator(gfmd3i,"Pi- for FMD3i",1);
    
    //VZERO C
    IlcGenFixed *gv0c=new IlcGenFixed(1);
    gv0c->SetPart(kPiPlus);
    gv0c->SetMomentum(1.5);
    gv0c->SetTheta(170);
    gv0c->SetPhi(50);
    gener->AddGenerator(gv0c,"Pi+ for V0C",1);
    
    //VZERO A
    IlcGenFixed *gv0a=new IlcGenFixed(1);
    gv0a->SetPart(kPiMinus);
    gv0a->SetMomentum(1.5);
    gv0a->SetTheta(1.5);
    gv0a->SetPhi(70);
    gener->AddGenerator(gv0a,"Pi- for V0A",1);


    //PMD
    IlcGenFixed *gpmd=new IlcGenFixed(1);
    gpmd->SetPart(kGamma);
    gpmd->SetMomentum(2);
    gpmd->SetTheta(12.6);
    gpmd->SetPhi(60);
    gener->AddGenerator(gpmd,"Gamma for PMD",1);

    //ZDC
    IlcGenFixed *gzdc1=new IlcGenFixed(1);
    gzdc1->SetPart(kProton);
    gzdc1->SetMomentum(700);
    gzdc1->SetTheta(0.6);
    gzdc1->SetPhi(60);
    gener->AddGenerator(gzdc1,"Proton for ZDC",1);

    IlcGenFixed *gzdc2=new IlcGenFixed(1);
    gzdc2->SetPart(kNeutron);
    gzdc2->SetMomentum(500);
    gzdc2->SetTheta(0.6);
    gzdc2->SetPhi(60);
    gener->AddGenerator(gzdc2,"Neutron for ZDC",1);

    //T0
    IlcGenFixed *gt0=new IlcGenFixed(1);
    gt0->SetPart(kPiPlus);
    gt0->SetMomentum(2);
    gt0->SetTheta(5.1);
    gt0->SetPhi(60);
    gener->AddGenerator(gt0,"Pi+ for T0",1);

    IlcGenFixed *gt01=new IlcGenFixed(1);
    gt01->SetPart(kPiMinus);
    gt01->SetMomentum(2);
    gt01->SetTheta(5.1);
    gt01->SetPhi(60);
    gener->AddGenerator(gt01,"Pi- for T0",1);


    //ACORDE
    IlcGenFixed *gacorde=new IlcGenFixed(1);
    gacorde->SetPart(kMuonPlus);
    gacorde->SetMomentum(20);
    gacorde->SetTheta(90.);
    gacorde->SetPhi(90);
    gener->AddGenerator(gacorde,"Muon+ for ACORDE",1);

    IlcGenFixed *gacorde1=new IlcGenFixed(1);
    gacorde1->SetPart(kMuonMinus);
    gacorde1->SetMomentum(20);
    gacorde1->SetTheta(90.);
    gacorde1->SetPhi(90);
    gener->AddGenerator(gacorde1,"Muon- for ACORDE",1);

    gener->Init();


    // 
    // Activate this line if you want the vertex smearing to happen
    // track by track
    //
    //gener->SetVertexSmear(perTrack); 
    // Field (L3 0.5 T)
    TGeoGlobalMagField::Instance()->SetField(new IlcMagF("Maps","Maps", -1., -1., IlcMagF::k5kG,beamType,beamEnergy));

    Int_t   iABSO  =  1;
    Int_t   iDIPO  =  1;
    Int_t   iFMD   =  1;
    Int_t   iFRAME =  1;
    Int_t   iHALL  =  1;
    Int_t   iITS   =  1;
    Int_t   iMAG   =  1;
    Int_t   iMUON  =  1;
    Int_t   iPHOS  =  1;
    Int_t   iPIPE  =  1;
    Int_t   iPMD   =  1;
    Int_t   iHMPID =  1;
    Int_t   iSHIL  =  1;
    Int_t   iT0    =  1;
    Int_t   iTOF   =  1;
    Int_t   iTPC   =  1;
    Int_t   iTRD   =  1;
    Int_t   iZDC   =  1;
    Int_t   iEMCAL =  1;
    Int_t   iACORDE=  1;
    Int_t   iVZERO =  1;
    rl->CdGAFile();
    //=================== Ilc BODY parameters =============================
    IlcBODY *BODY = new IlcBODY("BODY", "Ilc envelop");

    if (iMAG)
    {
        //=================== MAG parameters ============================
        // --- Start with Magnet since detector layouts may be depending ---
        // --- on the selected Magnet dimensions ---
        IlcMAG *MAG = new IlcMAG("MAG", "Magnet");
    }


    if (iABSO)
    {
        //=================== ABSO parameters ============================
        IlcABSO *ABSO = new IlcABSOv3("ABSO", "Muon Absorber");
    }

    if (iDIPO)
    {
        //=================== DIPO parameters ============================

        IlcDIPO *DIPO = new IlcDIPOv3("DIPO", "Dipole version 3");
    }

    if (iHALL)
    {
        //=================== HALL parameters ============================

        IlcHALL *HALL = new IlcHALLv3("HALL", "Ilc Hall");
    }


    if (iFRAME)
    {
        //=================== FRAME parameters ============================

        IlcFRAMEv2 *FRAME = new IlcFRAMEv2("FRAME", "Space Frame");
	FRAME->SetHoles(1);
    }

    if (iSHIL)
    {
        //=================== SHIL parameters ============================

        IlcSHIL *SHIL = new IlcSHILv3("SHIL", "Shielding Version 3");
    }


    if (iPIPE)
    {
        //=================== PIPE parameters ============================

        IlcPIPE *PIPE = new IlcPIPEv3("PIPE", "Beam Pipe");
    }
 
    if (iITS)
    {
        //=================== ITS parameters ============================

	IlcITS *ITS  = new IlcITSv11("ITS","ITS v11");
    }

    if (iTPC)
    {
        //============================ TPC parameters ===================
        IlcTPC *TPC = new IlcTPCv2("TPC", "Default");
    }


    if (iTOF) {
        //=================== TOF parameters ============================
	IlcTOF *TOF = new IlcTOFv6T0("TOF", "normal TOF");
     }


    if (iHMPID)
    {
        //=================== HMPID parameters ===========================
        IlcHMPID *HMPID = new IlcHMPIDv3("HMPID", "normal HMPID");

    }


    if (iZDC)
    {
        //=================== ZDC parameters ============================

        IlcZDC *ZDC = new IlcZDCv4("ZDC", "normal ZDC");
    }

    if (iTRD)
    {
        //=================== TRD parameters ============================

        IlcTRD *TRD = new IlcTRDv1("TRD", "TRD slow simulator");
        IlcTRDgeometry *geoTRD = TRD->GetGeometry();
	// Partial geometry: modules at 0,1,7,8,9,10,17
	// starting at 3h in positive direction
	geoTRD->SetSMstatus(2,0);
	geoTRD->SetSMstatus(3,0);
	geoTRD->SetSMstatus(4,0);
        geoTRD->SetSMstatus(5,0);
	geoTRD->SetSMstatus(6,0);
        geoTRD->SetSMstatus(11,0);
        geoTRD->SetSMstatus(12,0);
        geoTRD->SetSMstatus(13,0);
        geoTRD->SetSMstatus(14,0);
        geoTRD->SetSMstatus(15,0);
        geoTRD->SetSMstatus(16,0);
    }

    if (iFMD)
    {
        //=================== FMD parameters ============================
	IlcFMD *FMD = new IlcFMDv1("FMD", "normal FMD");
   }

    if (iMUON)
    {
        //=================== MUON parameters ===========================
        // New MUONv1 version (geometry defined via builders)
        IlcMUON *MUON = new IlcMUONv1("MUON","default");
    }
    //=================== PHOS parameters ===========================

    if (iPHOS)
    {
        IlcPHOS *PHOS = new IlcPHOSv1("PHOS", "IHEP");
    }


    if (iPMD)
    {
        //=================== PMD parameters ============================
        IlcPMD *PMD = new IlcPMDv1("PMD", "normal PMD");
    }

    if (iT0)
    {
        //=================== T0 parameters ============================
        IlcT0 *T0 = new IlcT0v1("T0", "T0 Detector");
    }

    if (iEMCAL)
    {
        //=================== EMCAL parameters ============================
        IlcEMCAL *EMCAL = new IlcEMCALv2("EMCAL", "EMCAL_COMPLETEV1");
    }

     if (iACORDE)
    {
        //=================== ACORDE parameters ============================
        IlcACORDE *ACORDE = new IlcACORDEv1("ACORDE", "normal ACORDE");
    }

     if (iVZERO)
    {
        //=================== VZERO parameters ============================
        IlcVZERO *VZERO = new IlcVZEROv7("VZERO", "normal VZERO");
    }


}

Float_t EtaToTheta(Float_t arg){
  return (180./TMath::Pi())*2.*atan(exp(-arg));
}
