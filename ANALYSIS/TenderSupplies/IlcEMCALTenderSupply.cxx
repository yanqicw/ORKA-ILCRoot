/**************************************************************************
 * 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * Author: The International Lepton Collider Off-line Project.            *
 *                                                                        *
 * Part of the code has been developed by ILC Off-line Project.         *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  EMCAL tender, apply corrections to EMCAL clusters and do track matching. //
//                                                                           //
//  Author: Deepa Thomas (Utrecht University)                                // 
//  Later mods/rewrite: Jiri Kral (University of Jyvaskyla)                  //
//  S. Aiola, C. Loizides : Make it work for AODs                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TInterpreter.h>
#include <TObjArray.h>
#include <TClonesArray.h>
#include <TGeoGlobalMagField.h>
#include "IlcEMCALAfterBurnerUF.h"
#include "IlcEMCALClusterizer.h"
#include "IlcEMCALClusterizerNxN.h"
#include "IlcEMCALClusterizerv1.h"
#include "IlcEMCALClusterizerv2.h"
#include "IlcEMCALDigit.h"
#include "IlcEMCALGeometry.h"
#include "IlcEMCALRecParam.h"
#include "IlcEMCALRecParam.h"
#include "IlcEMCALRecPoint.h"
#include "IlcEMCALRecoUtils.h"
#include "IlcEMCALTenderSupply.h"
#include "IlcESDCaloCluster.h"
#include "IlcMagF.h"
#include "IlcOADBContainer.h"
#include "IlcAODEvent.h"
#include "IlcAnalysisManager.h"
#include "IlcESDEvent.h"
#include "IlcLog.h"
#include "IlcTender.h"

ClassImp(IlcEMCALTenderSupply)

IlcEMCALTenderSupply::IlcEMCALTenderSupply() :
IlcTenderSupply()
,fTask(0)
,fRun(0)
,fEMCALGeo(0x0)
,fEMCALGeoName("EMCAL_COMPLETEV1")
,fEMCALRecoUtils(0)
,fConfigName("")
,fDebugLevel(0)
,fNonLinearFunc(-1) 
,fNonLinearThreshold(-1)
,fReCalibCluster(kFALSE)
,fUpdateCell(kFALSE)  
,fCalibrateEnergy(kFALSE)
,fCalibrateTime(kFALSE)
,fDoNonLinearity(kFALSE)
,fBadCellRemove(kFALSE)
,fRejectExoticCells(kFALSE)
,fRejectExoticClusters(kFALSE)
,fClusterBadChannelCheck(kFALSE)
,fRecalClusPos(kFALSE)
,fFiducial(kFALSE) 
,fNCellsFromEMCALBorder(-1)
,fRecalDistToBadChannels(kFALSE)
,fRecalShowerShape(kFALSE)
,fInputTree(0)
,fInputFile(0)
,fFilepass(0) 
,fMass(-1)
,fStep(-1)
,fCutEtaPhiSum(kTRUE)
,fCutEtaPhiSeparate(kFALSE)
,fRcut(-1)
,fEtacut(-1)
,fPhicut(-1)
,fBasePath("")
,fReClusterize(kFALSE)
,fClusterizer(0)
,fGeomMatrixSet(kFALSE)
,fLoadGeomMatrices(kFALSE)
,fRecParam(0x0)
,fDoTrackMatch(kFALSE)
,fDoUpdateOnly(kFALSE)
,fUnfolder(0)
,fDigitsArr(0)
,fClusterArr(0)
,fMisalignSurvey(kdefault)  
,fExoticCellFraction(-1)
,fExoticCellDiffTime(-1)
,fExoticCellMinAmplitude(-1)
{
  // Default constructor.
  for(Int_t i = 0; i < 10; i++) fEMCALMatrix[i] = 0 ;
  fEMCALRecoUtils  = new IlcEMCALRecoUtils;
}

//_____________________________________________________
IlcEMCALTenderSupply::IlcEMCALTenderSupply(const char *name, const IlcTender *tender) :
IlcTenderSupply(name,tender)
,fTask(0)
,fRun(0)
,fEMCALGeo(0x0)
,fEMCALGeoName("EMCAL_COMPLETEV1")
,fEMCALRecoUtils(0)
,fConfigName("")
,fDebugLevel(0)
,fNonLinearFunc(-1) 
,fNonLinearThreshold(-1)        
,fReCalibCluster(kFALSE)  
,fUpdateCell(kFALSE)  
,fCalibrateEnergy(kFALSE)
,fCalibrateTime(kFALSE)
,fDoNonLinearity(kFALSE)
,fBadCellRemove(kFALSE)
,fRejectExoticCells(kFALSE)
,fRejectExoticClusters(kFALSE)
,fClusterBadChannelCheck(kFALSE)
,fRecalClusPos(kFALSE)
,fFiducial(kFALSE) 
,fNCellsFromEMCALBorder(-1)  
,fRecalDistToBadChannels(kFALSE)  
,fRecalShowerShape(kFALSE)
,fInputTree(0)  
,fInputFile(0)
,fFilepass(0) 
,fMass(-1)
,fStep(-1)
,fCutEtaPhiSum(kTRUE)
,fCutEtaPhiSeparate(kFALSE)
,fRcut(-1)  
,fEtacut(-1)  
,fPhicut(-1)  
,fBasePath("")
,fReClusterize(kFALSE)
,fClusterizer(0)
,fGeomMatrixSet(kFALSE)
,fLoadGeomMatrices(kFALSE)
,fRecParam(0x0)
,fDoTrackMatch(kFALSE)
,fDoUpdateOnly(kFALSE)
,fUnfolder(0)
,fDigitsArr(0)
,fClusterArr(0)
,fMisalignSurvey(kdefault)  
,fExoticCellFraction(-1)
,fExoticCellDiffTime(-1)
,fExoticCellMinAmplitude(-1)
{
  // Named constructor
  
  for(Int_t i = 0; i < 10; i++) fEMCALMatrix[i] = 0 ;
  fEMCALRecoUtils  = new IlcEMCALRecoUtils;
}

//_____________________________________________________
IlcEMCALTenderSupply::IlcEMCALTenderSupply(const char *name, IlcAnalysisTaskSE *task) :
IlcTenderSupply(name)
,fTask(task)
,fRun(0)
,fEMCALGeo(0x0)
,fEMCALGeoName("EMCAL_COMPLETEV1")
,fEMCALRecoUtils(0)
,fConfigName("")
,fDebugLevel(0)
,fNonLinearFunc(-1) 
,fNonLinearThreshold(-1)        
,fReCalibCluster(kFALSE)  
,fUpdateCell(kFALSE)  
,fCalibrateEnergy(kFALSE)
,fCalibrateTime(kFALSE)
,fDoNonLinearity(kFALSE)
,fBadCellRemove(kFALSE)
,fRejectExoticCells(kFALSE)
,fRejectExoticClusters(kFALSE)
,fClusterBadChannelCheck(kFALSE)
,fRecalClusPos(kFALSE)
,fFiducial(kFALSE) 
,fNCellsFromEMCALBorder(-1)  
,fRecalDistToBadChannels(kFALSE)  
,fRecalShowerShape(kFALSE)
,fInputTree(0)  
,fInputFile(0)
,fFilepass(0) 
,fMass(-1)
,fStep(-1)
,fCutEtaPhiSum(kTRUE)
,fCutEtaPhiSeparate(kFALSE)
,fRcut(-1)  
,fEtacut(-1)  
,fPhicut(-1)  
,fBasePath("")
,fReClusterize(kFALSE)
,fClusterizer(0)
,fGeomMatrixSet(kFALSE)
,fLoadGeomMatrices(kFALSE)
,fRecParam(0x0)
,fDoTrackMatch(kFALSE)
,fDoUpdateOnly(kFALSE)
,fUnfolder(0)
,fDigitsArr(0)
,fClusterArr(0)
,fMisalignSurvey(kdefault)  
,fExoticCellFraction(-1)
,fExoticCellDiffTime(-1)
,fExoticCellMinAmplitude(-1)
{
  // Named constructor
  
  for(Int_t i = 0; i < 10; i++) fEMCALMatrix[i] = 0 ;
  fEMCALRecoUtils  = new IlcEMCALRecoUtils;
}

//_____________________________________________________
IlcEMCALTenderSupply::~IlcEMCALTenderSupply()
{
  //Destructor
  
  delete fEMCALRecoUtils;
  delete fRecParam;
  delete fUnfolder;
  if (!fClusterizer) {
    fDigitsArr->Clear("C");
    delete fDigitsArr; 
  } else {
    delete fClusterizer;
    fDigitsArr = 0;
  }
}

//_____________________________________________________
Bool_t IlcEMCALTenderSupply::RunChanged() const
{
  // Get run number
  return (fTender && fTender->RunChanged()) || (fTask && fRun != fTask->InputEvent()->GetRunNumber()); 
}

//_____________________________________________________
void IlcEMCALTenderSupply::Init()
{
  // Initialise EMCAL tender.

  if (fDebugLevel>0) 
    IlcWarning("Init EMCAL Tender supply"); 
  
  if (fConfigName.Length()>0 && gROOT->LoadMacro(fConfigName) >=0) {
    IlcDebug(1, Form("Loading settings from macro %s", fConfigName.Data()));
    IlcEMCALTenderSupply *tender = (IlcEMCALTenderSupply*)gInterpreter->ProcessLine("ConfigEMCALTenderSupply()");
    fDebugLevel             = tender->fDebugLevel;
    fEMCALGeoName           = tender->fEMCALGeoName; 
    fEMCALRecoUtils         = tender->fEMCALRecoUtils; 
    fConfigName             = tender->fConfigName;
    fNonLinearFunc          = tender->fNonLinearFunc;
    fNonLinearThreshold     = tender->fNonLinearThreshold;
    fReCalibCluster         = tender->fReCalibCluster;
    fUpdateCell             = tender->fUpdateCell;
    fRecalClusPos           = tender->fRecalClusPos;
    fCalibrateEnergy        = tender->fCalibrateEnergy;
    fCalibrateTime          = tender->fCalibrateTime;
    fFiducial               = tender->fFiducial;
    fNCellsFromEMCALBorder  = tender->fNCellsFromEMCALBorder;
    fRecalDistToBadChannels = tender->fRecalDistToBadChannels;    
    fRecalShowerShape       = tender->fRecalShowerShape;
    fClusterBadChannelCheck = tender->fClusterBadChannelCheck;
    fBadCellRemove          = tender->fBadCellRemove;
    fRejectExoticCells      = tender->fRejectExoticCells;
    fRejectExoticClusters   = tender->fRejectExoticClusters;
    fMass                   = tender->fMass;
    fStep                   = tender->fStep;
    fCutEtaPhiSum           = tender->fCutEtaPhiSum;
    fCutEtaPhiSeparate      = tender->fCutEtaPhiSeparate;
    fRcut                   = tender->fRcut;
    fEtacut                 = tender->fEtacut;
    fPhicut                 = tender->fPhicut;
    fReClusterize           = tender->fReClusterize;
    fLoadGeomMatrices       = tender->fLoadGeomMatrices;
    fRecParam               = tender->fRecParam;
    fDoNonLinearity         = tender->fDoNonLinearity;
    fDoTrackMatch           = tender->fDoTrackMatch;
    fDoUpdateOnly           = tender->fDoUpdateOnly;
    fMisalignSurvey         = tender->fMisalignSurvey;
    fExoticCellFraction     = tender->fExoticCellFraction;
    fExoticCellDiffTime     = tender->fExoticCellDiffTime;
    fExoticCellMinAmplitude = tender->fExoticCellMinAmplitude;

    for(Int_t i = 0; i < 10; i++) 
      fEMCALMatrix[i] = tender->fEMCALMatrix[i] ;
  }
  
  if (fDebugLevel>0){
    IlcInfo( "Emcal Tender settings: ======================================" ); 
    IlcInfo( "------------ Switches --------------------------" ); 
    IlcInfo( Form( "BadCellRemove : %d", fBadCellRemove )); 
    IlcInfo( Form( "ExoticCellRemove : %d", fRejectExoticCells )); 
    IlcInfo( Form( "CalibrateEnergy : %d", fCalibrateEnergy )); 
    IlcInfo( Form( "CalibrateTime : %d", fCalibrateTime )); 
    IlcInfo( Form( "UpdateCell : %d", fUpdateCell )); 
    IlcInfo( Form( "DoUpdateOnly : %d", fDoUpdateOnly )); 
    IlcInfo( Form( "Reclustering : %d", fReClusterize )); 
    IlcInfo( Form( "ClusterBadChannelCheck : %d", fClusterBadChannelCheck )); 
    IlcInfo( Form( "ClusterExoticChannelCheck : %d", fRejectExoticClusters )); 
    IlcInfo( Form( "CellFiducialRegion : %d", fFiducial )); 
    IlcInfo( Form( "ReCalibrateCluster : %d", fReCalibCluster )); 
    IlcInfo( Form( "RecalculateClusPos : %d", fRecalClusPos )); 
    IlcInfo( Form( "RecalShowerShape : %d", fRecalShowerShape )); 
    IlcInfo( Form( "NonLinearityCorrection : %d", fDoNonLinearity )); 
    IlcInfo( Form( "RecalDistBadChannel : %d", fRecalDistToBadChannels )); 
    IlcInfo( Form( "TrackMatch : %d", fDoTrackMatch )); 
    IlcInfo( "------------ Variables -------------------------" ); 
    IlcInfo( Form( "DebugLevel : %d", fDebugLevel )); 
    IlcInfo( Form( "BasePath : %s", fBasePath.Data() )); 
    IlcInfo( Form( "ConfigFileName : %s", fConfigName.Data() )); 
    IlcInfo( Form( "EMCALGeometryName : %s", fEMCALGeoName.Data() )); 
    IlcInfo( Form( "NonLinearityFunction : %d", fNonLinearFunc )); 
    IlcInfo( Form( "NonLinearityThreshold : %d", fNonLinearThreshold )); 
    IlcInfo( Form( "MisalignmentMatrixSurvey : %d", fMisalignSurvey )); 
    IlcInfo( Form( "NumberOfCellsFromEMCALBorder : %d", fNCellsFromEMCALBorder )); 
    IlcInfo( Form( "RCut : %f", fRcut )); 
    IlcInfo( Form( "Mass : %f", fMass )); 
    IlcInfo( Form( "Step : %f", fStep )); 
    IlcInfo( Form( "EtaCut : %f", fEtacut )); 
    IlcInfo( Form( "PhiCut : %f", fPhicut )); 
    IlcInfo( Form( "ExoticCellFraction : %f", fExoticCellFraction )); 
    IlcInfo( Form( "ExoticCellDiffTime : %f", fExoticCellDiffTime )); 
    IlcInfo( Form( "ExoticCellMinAmplitude : %f", fExoticCellMinAmplitude )); 
    IlcInfo( "=============================================================" ); 
  }

  // Init geometry  
  fEMCALGeo = IlcEMCALGeometry::GetInstance(fEMCALGeoName) ;

  // digits array
  fDigitsArr       = new TClonesArray("IlcEMCALDigit",1000);

  // Initialising non-linearity parameters
  if( fNonLinearThreshold != -1 )
    fEMCALRecoUtils->SetNonLinearityThreshold(fNonLinearThreshold);
  if( fNonLinearFunc != -1 )
    fEMCALRecoUtils->SetNonLinearityFunction(fNonLinearFunc);

  // missalignment function
  fEMCALRecoUtils->SetPositionAlgorithm(IlcEMCALRecoUtils::kPosTowerGlobal);

  // fiducial cut
  // do not do the eta0 fiducial cut
  if( fNCellsFromEMCALBorder != -1 )
    fEMCALRecoUtils->SetNumberOfCellsFromEMCALBorder(fNCellsFromEMCALBorder);
  fEMCALRecoUtils->SwitchOnNoFiducialBorderInEMCALEta0();
    
  // exotic cell rejection
  if( fExoticCellFraction != -1 )
    fEMCALRecoUtils->SetExoticCellFractionCut( fExoticCellFraction );
  if( fExoticCellDiffTime != -1 )
    fEMCALRecoUtils->SetExoticCellDiffTimeCut( fExoticCellDiffTime );
  if( fExoticCellMinAmplitude != -1 )
    fEMCALRecoUtils->SetExoticCellMinAmplitudeCut( fExoticCellMinAmplitude );

  // Setting track matching parameters ... mass, step size and residual cut
  if( fMass != -1 )
    fEMCALRecoUtils->SetMass(fMass);
  if( fStep != -1 )
    fEMCALRecoUtils->SetStep(fStep);
  
  // spatial cut based on separate eta/phi or common processing
  if(fCutEtaPhiSum){ 
    fEMCALRecoUtils->SwitchOnCutEtaPhiSum(); 
    if( fRcut != -1 )
      fEMCALRecoUtils->SetCutR(fRcut);
  } else if (fCutEtaPhiSeparate) {
    fEMCALRecoUtils->SwitchOnCutEtaPhiSeparate();
    if( fEtacut != -1 )
      fEMCALRecoUtils->SetCutEta(fEtacut);
    if( fPhicut != -1 )
      fEMCALRecoUtils->SetCutPhi(fPhicut);
  }
}

//_____________________________________________________
IlcVEvent* IlcEMCALTenderSupply::GetEvent()
{
  // return the event pointer
  
  if (fTender) {
    return fTender->GetEvent();
  }
  else if (fTask) {
    return fTask->InputEvent();
  }
  
  return 0;
  
}

//_____________________________________________________
void IlcEMCALTenderSupply::ProcessEvent()
{
  // Event loop.
  
  IlcVEvent *event = GetEvent();
  
  if (!event) {
    IlcError("Event ptr = 0, returning");
    return;
  }
  
 // Initialising parameters once per run number
  if (RunChanged()) { 

    fRun = event->GetRunNumber();

    IlcWarning( "Run changed, initializing parameters" );

    // get pass
    GetPass();

    // define what recalib parameters are needed for various switches
    // this is based on implementation in IlcEMCALRecoUtils
    Bool_t needRecoParam   = fReClusterize;
    Bool_t needBadChannels = fBadCellRemove   | fClusterBadChannelCheck | fRecalDistToBadChannels | fReClusterize;
    Bool_t needRecalib     = fCalibrateEnergy | fReClusterize;
    Bool_t needTimecalib   = fCalibrateTime   | fReClusterize;
    Bool_t needMisalign    = fRecalClusPos    | fReClusterize;
    Bool_t needClusterizer = fReClusterize;

    // initiate reco params with some defaults 
    // will not overwrite, if those have been provided by user
    if( needRecoParam ){
      Int_t initRC = InitRecParam();
      
      if( initRC == 0 )
        IlcInfo("Defaults reco params loaded.");
      if( initRC > 1 )
        IlcWarning("User defined reco params.");
    }

    // init bad channels
    if( needBadChannels ){
      Int_t fInitBC = InitBadChannels();
      if (fInitBC==0)
        IlcError("InitBadChannels returned false, returning");
      if (fInitBC==1)
        IlcWarning("InitBadChannels OK");
      if (fInitBC>1)
        IlcWarning(Form("No external hot channel set: %d - %s", event->GetRunNumber(), fFilepass.Data()));
    }

    // init recalibration factors
    if( needRecalib ) { 
      Int_t fInitRecalib = InitRecalib();
      if (fInitRecalib==0)
        IlcError("InitRecalib returned false, returning");
      if (fInitRecalib==1)
        IlcWarning("InitRecalib OK");
      if (fInitRecalib>1)
        IlcWarning(Form("No recalibration available: %d - %s", event->GetRunNumber(), fFilepass.Data()));
        fReCalibCluster = kFALSE;
    }
    
    // init time calibration
    if( needTimecalib ){
      Int_t initTC = InitTimeCalibration();
      if ( !initTC ) 
        IlcError("InitTimeCalibration returned false, returning");
      if (initTC==1)
        IlcWarning("InitTimeCalib OK");
      if( initTC > 1 )
        IlcWarning(Form("No external time calibration set: %d - %s", event->GetRunNumber(), fFilepass.Data()));
    }

    // init misalignment matrix
    if( needMisalign ) { 
      if (!InitMisalignMatrix())
        IlcError("InitMisalignmentMatrix returned false, returning");
      else
        IlcWarning("InitMisalignMatrix OK");
    }
    
    // init clusterizer
    if( needClusterizer ) {
      if (!InitClusterization()) 
        IlcError("InitClusterization returned false, returning");
      else
        IlcWarning("InitClusterization OK");
    }
    
    if(fDebugLevel>1) 
      fEMCALRecoUtils->Print("");
  }
  
  // disable implied switches -------------------------------------------------
  // IlcEMCALRecoUtils or clusterizer functions alredy include some
  // recalibration so based on those implied callibration te switches
  // here are disabled to avoid duplication
    
  // clusterizer does cluster energy recalibration, position recomputation
  // and shower shape
  if( fReClusterize ){
    fReCalibCluster   = kFALSE;
    fRecalClusPos     = kFALSE;
    fRecalShowerShape = kFALSE;
  }
  
  // CONFIGURE THE RECO UTILS -------------------------------------------------
  // configure the reco utils
  // this option does energy recalibration
  if( fCalibrateEnergy )
    fEMCALRecoUtils->SwitchOnRecalibration();
  else
    fEMCALRecoUtils->SwitchOffRecalibration();
  
  // allows time calibration
  if( fCalibrateTime )
    fEMCALRecoUtils->SwitchOnTimeRecalibration();
  else
    fEMCALRecoUtils->SwitchOffTimeRecalibration();

  // allows to zero bad cells
  if( fBadCellRemove )
    fEMCALRecoUtils->SwitchOnBadChannelsRemoval();
  else
    fEMCALRecoUtils->SwitchOffBadChannelsRemoval();
  
  // distance to bad channel recalibration
  if( fRecalDistToBadChannels )
    fEMCALRecoUtils->SwitchOnDistToBadChannelRecalculation();
  else
    fEMCALRecoUtils->SwitchOffDistToBadChannelRecalculation();

  // exclude exotic cells
  if( fRejectExoticCells )
    fEMCALRecoUtils->SwitchOnRejectExoticCell();
  else
    fEMCALRecoUtils->SwitchOffRejectExoticCell();
  
  // exclude clusters with exotic cells
  if( fRejectExoticClusters )
    fEMCALRecoUtils->SwitchOnRejectExoticCluster();
  else
    fEMCALRecoUtils->SwitchOffRejectExoticCluster();

  // TODO: not implemented switches
  // SwitchOnClusterEnergySmearing
  // SwitchOnRunDepCorrection

  // START PROCESSING ---------------------------------------------------------
  // Test if cells present
  IlcVCaloCells *cells= event->GetEMCALCells();
  if (cells->GetNumberOfCells()<=0) 
  {
    if(fDebugLevel>1) 
      IlcWarning(Form("Number of EMCAL cells = %d, returning", cells->GetNumberOfCells()));
    return;
  }
  
  if (fDebugLevel>2)
    IlcInfo(Form("Re-calibrate cluster %d\n",fReCalibCluster));

  // mark the cells not recalibrated in case of selected
  // time, energy recalibration or bad channel removal
  if( fCalibrateEnergy || fCalibrateTime || fBadCellRemove )
    fEMCALRecoUtils->ResetCellsCalibrated();
  
 // CELL RECALIBRATION -------------------------------------------------------
  // cell objects will be updated
  // the cell calibrations are also processed locally any time those are needed
  // in case that the cell objects are not to be updated here for later use
  if( fUpdateCell )
  {
    // do the update
    // includes exotic cell check in the UpdateCells function - is not provided
    // by the reco utils
    UpdateCells();

    // switch off recalibrations so those are not done multiple times
    // this is just for safety, the recalibrated flag of cell object
    // should not allow for farther processing anyways
    fEMCALRecoUtils->SwitchOffRecalibration();
    fEMCALRecoUtils->SwitchOffTimeRecalibration();  
  
    if (fDoUpdateOnly)
      return;
  }

  // RECLUSTERIZATION ---------------------------------------------------------
  if (fReClusterize)
  {
    FillDigitsArray();
    Clusterize();
    UpdateClusters();
  }

  // Store good clusters
  TClonesArray *clusArr = dynamic_cast<TClonesArray*>(event->FindListObject("caloClusters"));
  if (!clusArr) 
    clusArr = dynamic_cast<TClonesArray*>(event->FindListObject("CaloClusters"));
  if (!clusArr) {
    IlcWarning(Form("No cluster array, number of cells in event = %d, returning", cells->GetNumberOfCells()));
    return;
  }

  // PROCESS SINGLE CLUSTER RECALIBRATION -------------------------------------
  // now go through clusters one by one and process separate correction
  // as those were defined or not
  Int_t nclusters = clusArr->GetEntriesFast();
  for (Int_t icluster=0; icluster < nclusters; ++icluster) 
  { 
    IlcVCluster *clust = static_cast<IlcVCluster*>(clusArr->At(icluster));
    if (!clust) 
      continue;
    if  (!clust->IsEMCAL()) 
      continue;

    // REMOVE CLUSTERS WITH BAD CELLS -----------------------------
    if( fClusterBadChannelCheck )
    {
      // careful, the the ClusterContainsBadChannel is dependent on
      // SwitchOnBadChannelsRemoval, switching it ON automatically
      // and returning to original value after processing
      Bool_t badRemoval = fEMCALRecoUtils->IsBadChannelsRemovalSwitchedOn();
      fEMCALRecoUtils->SwitchOnBadChannelsRemoval();
      
      Bool_t badResult = fEMCALRecoUtils->ClusterContainsBadChannel(fEMCALGeo, clust->GetCellsAbsId(), clust->GetNCells());

      // switch the bad channels removal back
      if( ! badRemoval )
        fEMCALRecoUtils->SwitchOffBadChannelsRemoval();
      
      if( badResult )
      {
        delete clusArr->RemoveAt(icluster);
        continue; //TODO is it really needed to remove it? Or should we flag it?
      }
    }
    
    // REMOVE EXOTIC CLUSTERS -------------------------------------
    // does process local cell recalibration energy and time without replacing
    // the global cell values, in case of no cell recalib done yet
    if( fRejectExoticClusters )
    {
      // careful, the IsExoticCluster is dependent on
      // SwitchOnRejectExoticCell, switching it ON automatically
      // and returning to original value after processing
      Bool_t exRemoval = fEMCALRecoUtils->IsRejectExoticCell();
      fEMCALRecoUtils->SwitchOnRejectExoticCell();

      // get bunch crossing
      Int_t bunchCrossNo = event->GetBunchCrossNumber();

      Bool_t exResult = fEMCALRecoUtils->IsExoticCluster(clust, cells, bunchCrossNo );

      // switch the exotic channels removal back
      if( ! exRemoval )
        fEMCALRecoUtils->SwitchOffRejectExoticCell();
      
      if( exResult )
      {
        delete clusArr->RemoveAt(icluster);
        continue; //TODO is it really needed to remove it? Or should we flag it?
      }
    }
    
    // FIDUCIAL CUT -----------------------------------------------
    if (fFiducial)
    {
      // depends on SetNumberOfCellsFromEMCALBorder
      // SwitchOnNoFiducialBorderInEMCALEta0
      if (!fEMCALRecoUtils->CheckCellFiducialRegion(fEMCALGeo, clust, cells)){
        delete clusArr->RemoveAt(icluster);
        continue; //TODO it would be nice to store the distance
      }
    }
    
    // CLUSTER ENERGY ---------------------------------------------
    // does process local cell recalibration energy and time without replacing
    // the global cell values, in case of no cell recalib done yet
    if( fReCalibCluster ) 
      fEMCALRecoUtils->RecalibrateClusterEnergy(fEMCALGeo, clust, cells);

    // CLUSTER POSITION -------------------------------------------
    // does process local cell energy recalibration, if enabled and cells
    // not calibratied yet
    if( fRecalClusPos ) 
      fEMCALRecoUtils->RecalculateClusterPosition(fEMCALGeo, cells, clust);
    
    // SHOWER SHAPE -----------------------------------------------
    if( fRecalShowerShape )
      fEMCALRecoUtils->RecalculateClusterShowerShapeParameters(fEMCALGeo, cells, clust);  

    // NONLINEARITY -----------------------------------------------
    if( fDoNonLinearity )
    {
      Float_t correctedEnergy = fEMCALRecoUtils->CorrectClusterEnergyLinearity(clust);
      clust->SetE(correctedEnergy);
    }

    // DISTANCE TO BAD CHANNELS -----------------------------------
    if( fRecalDistToBadChannels )
      fEMCALRecoUtils->RecalculateClusterDistanceToBadChannel(fEMCALGeo, cells, clust);  
  }

  clusArr->Compress();

  if (!fDoTrackMatch)
    return;

  // TRACK MATCHING -----------------------------------------------------------
  if (!TGeoGlobalMagField::Instance()->GetField()) {
    IlcESDEvent *esd = dynamic_cast<IlcESDEvent*>(event);
    if (esd)
      esd->InitMagneticField();
    else {
      IlcAODEvent *aod = dynamic_cast<IlcAODEvent*>(event);
      Double_t curSol = 30000*aod->GetMagneticField()/5.00668;
      Double_t curDip = 6000 *aod->GetMuonMagFieldScale();
      IlcMagF *field  = IlcMagF::CreateFieldMap(curSol,curDip);
      TGeoGlobalMagField::Instance()->SetField(field);
    }
  }

  fEMCALRecoUtils->FindMatches(event,0x0,fEMCALGeo);
  fEMCALRecoUtils->SetClusterMatchedToTrack(event);
  fEMCALRecoUtils->SetTracksMatchedToCluster(event);
}

//_____________________________________________________
Bool_t IlcEMCALTenderSupply::InitMisalignMatrix()
{
  // Initialising misalignment matrices
  IlcVEvent *event = GetEvent();
  
  if (!event) 
    return kFALSE;
  
  if (fGeomMatrixSet) 
  {
    IlcInfo("Misalignment matrix already set");  
    return kTRUE;
  }
  
  if (fDebugLevel>0) 
    IlcInfo("Initialising misalignment matrix");  
  
  if (fLoadGeomMatrices) {
    for(Int_t mod=0; mod < fEMCALGeo->GetNumberOfSuperModules(); ++mod)
    {
      if (fEMCALMatrix[mod]){
        if(fDebugLevel > 2) 
          fEMCALMatrix[mod]->Print();
        fEMCALGeo->SetMisalMatrix(fEMCALMatrix[mod],mod);  
      }
    }
    fGeomMatrixSet = kTRUE;
    return kTRUE;
  }
  
  Int_t runGM = event->GetRunNumber();
  TObjArray *mobj = 0;

 if(fMisalignSurvey == kdefault)
 { //take default alignment corresponding to run no
    IlcOADBContainer emcalgeoCont(Form("emcal"));
    emcalgeoCont.InitFromFile("$ILC_ROOT/OADB/EMCAL/EMCALlocal2master.root",Form("IlcEMCALgeo"));
    mobj=(TObjArray*)emcalgeoCont.GetObject(runGM,"EmcalMatrices");
 }

 if(fMisalignSurvey == kSurveybyS)
 { //take alignment at sector level
  if (runGM <= 140000) { //2010 data
    IlcOADBContainer emcalgeoCont(Form("emcal2010"));
    emcalgeoCont.InitFromFile("$ILC_ROOT/OADB/EMCAL/EMCALlocal2master.root",Form("IlcEMCALgeo"));
    mobj=(TObjArray*)emcalgeoCont.GetObject(100,"survey10");
    
  } else if (runGM>140000)
  { // 2011 LHC11a pass1 data
    IlcOADBContainer emcalgeoCont(Form("emcal2011"));
    emcalgeoCont.InitFromFile("$ILC_ROOT/OADB/EMCAL/EMCALlocal2master.root",Form("IlcEMCALgeo"));
    mobj=(TObjArray*)emcalgeoCont.GetObject(100,"survey11byS");      
  }
 }

 if(fMisalignSurvey == kSurveybyM)
 { //take alignment at module level
  if (runGM <= 140000) { //2010 data
    IlcOADBContainer emcalgeoCont(Form("emcal2010"));
    emcalgeoCont.InitFromFile("$ILC_ROOT/OADB/EMCAL/EMCALlocal2master.root",Form("IlcEMCALgeo"));
    mobj=(TObjArray*)emcalgeoCont.GetObject(100,"survey10");
    
  } else if (runGM>140000) 
  { // 2011 LHC11a pass1 data
    IlcOADBContainer emcalgeoCont(Form("emcal2011"));
    emcalgeoCont.InitFromFile("$ILC_ROOT/OADB/EMCAL/EMCALlocal2master.root",Form("IlcEMCALgeo"));
    mobj=(TObjArray*)emcalgeoCont.GetObject(100,"survey11byM");      
  }
 }

  if(!mobj)
  {
    IlcFatal("Geometry matrix array not found");
    return kFALSE;
  }
  
 for(Int_t mod=0; mod < (fEMCALGeo->GetEMCGeometry())->GetNumberOfSuperModules(); mod++)
 {
   fEMCALMatrix[mod] = (TGeoHMatrix*) mobj->At(mod);
   fEMCALGeo->SetMisalMatrix(fEMCALMatrix[mod],mod); 
   fEMCALMatrix[mod]->Print();
 }
  
  return kTRUE;
}

//_____________________________________________________
Int_t IlcEMCALTenderSupply::InitBadChannels()
{
  // Initialising bad channel maps

  IlcVEvent *event = GetEvent();

  if (!event) 
    return 0;
  
  if (fDebugLevel>0) 
    IlcInfo("Initialising Bad channel map");
  
  // init default maps first
  if( !fEMCALRecoUtils->GetEMCALBadChannelStatusMapArray() )
    fEMCALRecoUtils->InitEMCALBadChannelStatusMap() ;
  
  Int_t runBC = event->GetRunNumber();
  
  IlcOADBContainer *contBC = new IlcOADBContainer("");
  if (fBasePath!="")
  { //if fBasePath specified in the ->SetBasePath()
    if (fDebugLevel>0) IlcInfo(Form("Loading Bad Channels OADB from given path %s",fBasePath.Data()));
    
    TFile *fbad=new TFile(Form("%s/EMCALBadChannels.root",fBasePath.Data()),"read");
    if (!fbad || fbad->IsZombie())
    {
      IlcFatal(Form("EMCALBadChannels.root was not found in the path provided: %s",fBasePath.Data()));
      return 0;
    }  
    
    if (fbad) delete fbad;
    
    contBC->InitFromFile(Form("%s/EMCALBadChannels.root",fBasePath.Data()),"IlcEMCALBadChannels");    
  } 
  else 
  { // Else choose the one in the $ILC_ROOT directory
    if (fDebugLevel>0) IlcInfo("Loading Bad Channels OADB from $ILC_ROOT/OADB/EMCAL");
    
    TFile *fbad=new TFile("$ILC_ROOT/OADB/EMCAL/EMCALBadChannels.root","read");
    if (!fbad || fbad->IsZombie())
    {
      IlcFatal("$ILC_ROOT/OADB/EMCAL/EMCALBadChannels.root was not found");
      return 0;
    }  
      
    if (fbad) delete fbad;
    
    contBC->InitFromFile("$ILC_ROOT/OADB/EMCAL/EMCALBadChannels.root","IlcEMCALBadChannels"); 
  }
  
  TObjArray *arrayBC=(TObjArray*)contBC->GetObject(runBC);
  if (!arrayBC)
  {
    IlcError(Form("No external hot channel set for run number: %d", runBC));
    return 2; 
  }

  Int_t sms = fEMCALGeo->GetEMCGeometry()->GetNumberOfSuperModules();
  for (Int_t i=0; i<sms; ++i) 
  {
    TH2I *h = fEMCALRecoUtils->GetEMCALChannelStatusMap(i);
    if (h)
      delete h;
    h=(TH2I*)arrayBC->FindObject(Form("EMCALBadChannelMap_Mod%d",i));

    if (!h) 
    {
      IlcError(Form("Can not get EMCALBadChannelMap_Mod%d",i));
      continue;
    }
    h->SetDirectory(0);
    fEMCALRecoUtils->SetEMCALChannelStatusMap(i,h);
  }
  return 1;  
}

//_____________________________________________________
Int_t IlcEMCALTenderSupply::InitRecalib()
{
  // Initialising recalibration factors.
  
  IlcVEvent *event = GetEvent();

  if (!event) 
    return 0;
  
  if (fDebugLevel>0) 
    IlcInfo("Initialising recalibration factors");
  
  // init default maps first
  if( !fEMCALRecoUtils->GetEMCALRecalibrationFactorsArray() )
    fEMCALRecoUtils->InitEMCALRecalibrationFactors() ;

  Int_t runRC = event->GetRunNumber();
      
  IlcOADBContainer *contRF=new IlcOADBContainer("");
  if (fBasePath!="") 
  { //if fBasePath specified in the ->SetBasePath()
    if (fDebugLevel>0)  IlcInfo(Form("Loading Recalib OADB from given path %s",fBasePath.Data()));
    
    TFile *fRecalib= new TFile(Form("%s/EMCALRecalib.root",fBasePath.Data()),"read");
    if (!fRecalib || fRecalib->IsZombie()) 
    {
      IlcFatal(Form("EMCALRecalib.root not found in %s",fBasePath.Data()));
      return 0;
    }
    
    if (fRecalib) delete fRecalib;
    
    contRF->InitFromFile(Form("%s/EMCALRecalib.root",fBasePath.Data()),"IlcEMCALRecalib");
  }
    else
    { // Else choose the one in the $ILC_ROOT directory
      if (fDebugLevel>0)  IlcInfo("Loading Recalib OADB from $ILC_ROOT/OADB/EMCAL");
      
      TFile *fRecalib= new TFile("$ILC_ROOT/OADB/EMCAL/EMCALRecalib.root","read");
      if (!fRecalib || fRecalib->IsZombie()) 
      {
        IlcFatal("$ILC_ROOT/OADB/EMCAL/EMCALRecalib.root was not found");
        return 0;
      }
      
      if (fRecalib) delete fRecalib;
      
      contRF->InitFromFile("$ILC_ROOT/OADB/EMCAL/EMCALRecalib.root","IlcEMCALRecalib");     
    }

  TObjArray *recal=(TObjArray*)contRF->GetObject(runRC);
  if (!recal)
  {
    IlcError(Form("No Objects for run: %d",runRC));
    return 2;
  } 

  TObjArray *recalpass=(TObjArray*)recal->FindObject(fFilepass);
  if (!recalpass)
  {
    IlcError(Form("No Objects for run: %d - %s",runRC,fFilepass.Data()));
    return 2;
  }

  TObjArray *recalib=(TObjArray*)recalpass->FindObject("Recalib");
  if (!recalib)
  {
    IlcError(Form("No Recalib histos found for  %d - %s",runRC,fFilepass.Data())); 
    return 2;
  }

  if (fDebugLevel>0) recalib->Print();

  Int_t sms = fEMCALGeo->GetEMCGeometry()->GetNumberOfSuperModules();
  for (Int_t i=0; i<sms; ++i) 
  {
    TH2F *h = fEMCALRecoUtils->GetEMCALChannelRecalibrationFactors(i);
    if (h)
      delete h;
    h = (TH2F*)recalib->FindObject(Form("EMCALRecalFactors_SM%d",i));
    if (!h) 
    {
      IlcError(Form("Could not load EMCALRecalFactors_SM%d",i));
      continue;
    }
    h->SetDirectory(0);
    fEMCALRecoUtils->SetEMCALChannelRecalibrationFactors(i,h);
  }
  return 1;
}

//_____________________________________________________
Int_t IlcEMCALTenderSupply::InitTimeCalibration()
{
  // Initialising bad channel maps
  IlcVEvent *event = GetEvent();

  if (!event) 
    return 0;
  
  if (fDebugLevel>0) 
    IlcInfo("Initialising time calibration map");
  
  // init default maps first
  if( !fEMCALRecoUtils->GetEMCALTimeRecalibrationFactorsArray() )
    fEMCALRecoUtils->InitEMCALTimeRecalibrationFactors() ;

  Int_t runBC = event->GetRunNumber();
  
  IlcOADBContainer *contBC = new IlcOADBContainer("");
  if (fBasePath!="")
  { //if fBasePath specified in the ->SetBasePath()
    if (fDebugLevel>0) IlcInfo(Form("Loading time calibration OADB from given path %s",fBasePath.Data()));
    
    TFile *fbad=new TFile(Form("%s/EMCALTimeCalib.root",fBasePath.Data()),"read");
    if (!fbad || fbad->IsZombie())
    {
      IlcFatal(Form("EMCALTimeCalib.root was not found in the path provided: %s",fBasePath.Data()));
      return 0;
    }  
    
    if (fbad) delete fbad;
    
    contBC->InitFromFile(Form("%s/EMCALTimeCalib.root",fBasePath.Data()),"IlcEMCALTimeCalib");    
  } 
  else 
  { // Else choose the one in the $ILC_ROOT directory
    if (fDebugLevel>0) IlcInfo("Loading time calibration OADB from $ILC_ROOT/OADB/EMCAL");
    
    TFile *fbad=new TFile("$ILC_ROOT/OADB/EMCAL/EMCALTimeCalib.root","read");
    if (!fbad || fbad->IsZombie())
    {
      IlcFatal("$ILC_ROOT/OADB/EMCAL/EMCALTimeCalib.root was not found");
      return 0;
    }  
      
    if (fbad) delete fbad;
    
    contBC->InitFromFile("$ILC_ROOT/OADB/EMCAL/EMCALTimeCalib.root","IlcEMCALTimeCalib"); 
  }
  
  TObjArray *arrayBC=(TObjArray*)contBC->GetObject(runBC);
  if (!arrayBC)
  {
    IlcError(Form("No external time calibration set for run number: %d", runBC));
    return 2; 
  }
  
  // Here, it looks for a specific pass
  TObjArray *arrayBCpass=(TObjArray*)arrayBC->FindObject(fFilepass); 
  if (!arrayBCpass)
  {
    IlcError(Form("No external time calibration set for: %d -%s", runBC,fFilepass.Data()));
    return 2; 
  }

  if (fDebugLevel>0) arrayBCpass->Print();

  for( Int_t i = 0; i < 4; i++ )
  {
    TH1F *h = fEMCALRecoUtils->GetEMCALChannelTimeRecalibrationFactors( i );
    if( h )
      delete h;
    
    h = (TH1F*)arrayBCpass->FindObject(Form("hAllTimeAvBC%d",i));
    
    if (!h)
    {
      IlcError(Form("Can not get hAllTimeAvBC%d",i));
      continue;
    }
    h->SetDirectory(0);
    fEMCALRecoUtils->SetEMCALChannelTimeRecalibrationFactors(i,h);
  }
  return 1;  
}

//_____________________________________________________
void IlcEMCALTenderSupply::UpdateCells()
{
  //Remove bad cells from the cell list
  //Recalibrate energy and time cells 
  //This is required for later reclusterization

  IlcVEvent *event = GetEvent();

  IlcVCaloCells *cells = event->GetEMCALCells();
  Int_t bunchCrossNo = event->GetBunchCrossNumber();

  fEMCALRecoUtils->RecalibrateCells(cells, bunchCrossNo); 
  
  // remove exotic cells - loop through cells and zero the exotic ones
  // just like with bad cell rejection in reco utils (inside RecalibrateCells)
  if( fRejectExoticCells )
  {
    Short_t  absId  =-1;
    Double_t ecell = 0;
    Double_t tcell = 0;
    Double_t efrac = 0;
    Short_t  mclabel = -1;
    Bool_t   isExot = kFALSE;
  
    // loop through cells
    Int_t nEMcell  = cells->GetNumberOfCells() ;  
    for (Int_t iCell = 0; iCell < nEMcell; iCell++) 
    { 
      cells->GetCell( iCell, absId, ecell, tcell, mclabel, efrac );
    
      isExot = fEMCALRecoUtils->IsExoticCell( absId, cells, bunchCrossNo ); 
      // zero if exotic
      if( isExot )
        cells->SetCell( iCell, absId, 0.0, -1.0, mclabel, efrac );
    } // cell loop
  } // reject exotic cells

  cells->Sort();
}

//_____________________________________________________
TString IlcEMCALTenderSupply::GetBeamType()
{
  
  // Get beam type : pp-AA-pA
  // ESDs have it directly, AODs we hardcode it
  
  IlcVEvent *event = GetEvent();

  if (!event) { 
    IlcError("Couldn't retrieve event!");
    return "";
  }

  TString beamType;

  if (event->InheritsFrom("IlcESDEvent")) {
    IlcESDEvent *esd = dynamic_cast<IlcESDEvent*>(event);
    const IlcESDRun *run = esd->GetESDRun();
    beamType = run->GetBeamType();
  }
  else if (event->InheritsFrom("IlcAODEvent")) {
    Int_t runNumber = event->GetRunNumber();
    if ((runNumber >= 136851 && runNumber <= 139517)  // LHC10h
  || (runNumber >= 166529 && runNumber <= 170593))  // LHC11h
      {
	beamType = "A-A";
      }
    else 
      {
	beamType = "p-p";
      }
  }

  return beamType;    
}

//_____________________________________________________
Int_t IlcEMCALTenderSupply::InitRecParam()
{
  // exit if reco params exist (probably shipped by the user already)
  if( fRecParam != 0 )
    return 2;

  TString beamType = GetBeamType();

  // set some default reco params
  fRecParam = new IlcEMCALRecParam();
  fRecParam->SetClusteringThreshold(0.100);
  fRecParam->SetMinECut(0.050);
  fRecParam->SetTimeCut(250);
  fRecParam->SetTimeMin(425);
  fRecParam->SetTimeMax(825);
  if ( beamType == "A-A") {
    fRecParam->SetClusterizerFlag(IlcEMCALRecParam::kClusterizerv2);
  } 
  else {
    fRecParam->SetClusterizerFlag(IlcEMCALRecParam::kClusterizerv1);
  }

  return 0;
}

//_____________________________________________________
Bool_t IlcEMCALTenderSupply::InitClusterization()
{
  // Initialing clusterization/unfolding algorithm and set all the needed parameters.
  
  IlcVEvent *event = GetEvent();

  if (!event) 
    return kFALSE;
  
  if (fDebugLevel>0) 
    IlcInfo(Form("Initialising reclustering parameters: Clusterizer type: %d",fRecParam->GetClusterizerFlag()));
  
  //---setup clusterizer
  delete fClusterizer;
  if     (fRecParam->GetClusterizerFlag() == IlcEMCALRecParam::kClusterizerv1)
    fClusterizer = new IlcEMCALClusterizerv1 (fEMCALGeo);
  else if (fRecParam->GetClusterizerFlag() == IlcEMCALRecParam::kClusterizerv2) 
    fClusterizer = new IlcEMCALClusterizerv2(fEMCALGeo);
  else if (fRecParam->GetClusterizerFlag() == IlcEMCALRecParam::kClusterizerNxN) 
  {
    IlcEMCALClusterizerNxN *clusterizer = new IlcEMCALClusterizerNxN(fEMCALGeo);
    clusterizer->SetNRowDiff(fRecParam->GetNRowDiff());
    clusterizer->SetNColDiff(fRecParam->GetNColDiff());
    fClusterizer = clusterizer;
  } 
  else 
  {
    IlcFatal(Form("Clusterizer < %d > not available", fRecParam->GetClusterizerFlag()));
    return kFALSE;
  }
  
  // Set the clustering parameters
  fClusterizer->SetECAClusteringThreshold( fRecParam->GetClusteringThreshold() );
  fClusterizer->SetECALogWeight          ( fRecParam->GetW0()                  );
  fClusterizer->SetMinECut               ( fRecParam->GetMinECut()             );    
  fClusterizer->SetUnfolding             ( fRecParam->GetUnfold()              );
  fClusterizer->SetECALocalMaxCut        ( fRecParam->GetLocMaxCut()           );
  fClusterizer->SetTimeCut               ( fRecParam->GetTimeCut()             );
  fClusterizer->SetTimeMin               ( fRecParam->GetTimeMin()             );
  fClusterizer->SetTimeMax               ( fRecParam->GetTimeMax()             );
  fClusterizer->SetInputCalibrated       ( kTRUE                               );
  fClusterizer->SetJustClusters          ( kTRUE                               );  
  
  // In case of unfolding after clusterization is requested, set the corresponding parameters
  if (fRecParam->GetUnfold()) 
  {
    for (Int_t i = 0; i < 8; ++i) 
    {
      fClusterizer->SetSSPars(i, fRecParam->GetSSPars(i));
    }
    for (Int_t i = 0; i < 3; ++i)
    {
      fClusterizer->SetPar5  (i, fRecParam->GetPar5(i));
      fClusterizer->SetPar6  (i, fRecParam->GetPar6(i));
    }
    fClusterizer->InitClusterUnfolding();
  }
  
  fClusterizer->SetDigitsArr(fDigitsArr);
  fClusterizer->SetOutput(0);
  fClusterArr = const_cast<TObjArray *>(fClusterizer->GetRecPoints());
  return kTRUE;
}

//_____________________________________________________
void IlcEMCALTenderSupply::FillDigitsArray()
{
  // Fill digits from cells to a TClonesArray.
  
  IlcVEvent *event = GetEvent();

 if (!event)
    return;
  
  fDigitsArr->Clear("C");
  IlcVCaloCells *cells = event->GetEMCALCells();
  Int_t ncells = cells->GetNumberOfCells();
  for (Int_t icell = 0, idigit = 0; icell < ncells; ++icell) 
  {
    Double_t cellAmplitude=0, cellTime=0, efrac = 0;
    Short_t  cellNumber=0, mcLabel=-1;

    if (cells->GetCell(icell, cellNumber, cellAmplitude, cellTime, mcLabel, efrac) != kTRUE)
      break;

    // Do not add if already too low (some cells set to 0 if bad channels)
    if (cellAmplitude < fRecParam->GetMinECut() ) 
      continue;

    // If requested, do not include exotic cells
   if (fEMCALRecoUtils->IsExoticCell(cellNumber,cells,event->GetBunchCrossNumber())) 
      continue;
        
    IlcEMCALDigit *digit = static_cast<IlcEMCALDigit*>(fDigitsArr->New(idigit));
    digit->SetId(cellNumber);
    digit->SetTime((Float_t)cellTime);
    digit->SetTimeR((Float_t)cellTime);
    digit->SetIndexInList(idigit);
    digit->SetType(IlcEMCALDigit::kHG);
    digit->SetAmplitude((Float_t)cellAmplitude);
    idigit++;
  }
}

//_____________________________________________________
void IlcEMCALTenderSupply::Clusterize()
{
  // Clusterize.
  
  fClusterizer->Digits2Clusters("");
}

//_____________________________________________________
void IlcEMCALTenderSupply::UpdateClusters()
{
  // Update ESD cluster list.
  
  IlcVEvent *event = GetEvent();

  if (!event)
    return;
  
  TClonesArray *clus = dynamic_cast<TClonesArray*>(event->FindListObject("caloClusters"));
  if (!clus) 
    clus = dynamic_cast<TClonesArray*>(event->FindListObject("CaloClusters"));
  if (!clus) 
  {
    IlcError(" Null pointer to calo clusters array, returning");
    return;
  }
  
  Int_t nents = clus->GetEntriesFast();
  for (Int_t i=0; i < nents; ++i) 
  {
    IlcESDCaloCluster *c = static_cast<IlcESDCaloCluster*>(clus->At(i));
    if (!c)
      continue;
    if (c->IsEMCAL())
      delete clus->RemoveAt(i);
  }
  
  clus->Compress();
  
  RecPoints2Clusters(clus);
  
}

//_____________________________________________________
void IlcEMCALTenderSupply::RecPoints2Clusters(TClonesArray *clus)
{
  // Convert IlcEMCALRecoPoints to IlcESDCaloClusters.
  // Cluster energy, global position, cells and their amplitude fractions are restored.
  
  IlcVEvent *event = GetEvent();

  if (!event)
    return;

  Int_t ncls = fClusterArr->GetEntriesFast();
  for(Int_t i=0, nout=clus->GetEntriesFast(); i < ncls; ++i) 
  {
    IlcEMCALRecPoint *recpoint = static_cast<IlcEMCALRecPoint*>(fClusterArr->At(i));
    
    Int_t ncellsTrue = 0;
    const Int_t ncells = recpoint->GetMultiplicity();
    UShort_t   absIds[ncells];  
    Double32_t ratios[ncells];
    Int_t *dlist = recpoint->GetDigitsList();
    Float_t *elist = recpoint->GetEnergiesList();
    for (Int_t c = 0; c < ncells; ++c) 
    {
      IlcEMCALDigit *digit = static_cast<IlcEMCALDigit*>(fDigitsArr->At(dlist[c]));
      absIds[ncellsTrue] = digit->GetId();
      ratios[ncellsTrue] = elist[c]/digit->GetAmplitude();
      if (ratios[ncellsTrue] < 0.001) 
        continue;
      ++ncellsTrue;
    }
    
    if (ncellsTrue < 1) 
    {
      IlcWarning("Skipping cluster with no cells");
      continue;
    }
    
    // calculate new cluster position
    TVector3 gpos;
    recpoint->GetGlobalPosition(gpos);
    Float_t g[3];
    gpos.GetXYZ(g);
    
    IlcESDCaloCluster *c = static_cast<IlcESDCaloCluster*>(clus->New(nout++));
    c->SetID(nout-1); 
    c->SetType(IlcVCluster::kEMCALClusterv1);
    c->SetE(recpoint->GetEnergy());
    c->SetPosition(g);
    c->SetNCells(ncellsTrue);
    c->SetDispersion(recpoint->GetDispersion());
    c->SetEmcCpvDistance(-1);            //not yet implemented
    c->SetChi2(-1);                      //not yet implemented
    c->SetTOF(recpoint->GetTime()) ;     //time-of-flight
    c->SetNExMax(recpoint->GetNExMax()); //number of local maxima
    Float_t elipAxis[2];
    recpoint->GetElipsAxis(elipAxis);
    c->SetM02(elipAxis[0]*elipAxis[0]) ;
    c->SetM20(elipAxis[1]*elipAxis[1]) ;
    IlcESDCaloCluster *cesd = static_cast<IlcESDCaloCluster*>(c);
    cesd->SetCellsAbsId(absIds);
    cesd->SetCellsAmplitudeFraction(ratios);
  }
}

//_____________________________________________________
void IlcEMCALTenderSupply::GetPass()
{
  // Get passx from filename.
  
  IlcAnalysisManager *mgr = IlcAnalysisManager::GetAnalysisManager();
  fInputTree = mgr->GetTree();
  
  if (!fInputTree) 
  {
    IlcError("Pointer to tree = 0, returning");
    return;
  }
  
  fInputFile = fInputTree->GetCurrentFile();
  if (!fInputFile) {
    IlcError("Null pointer input file, returning");
    return;
  }
  
  TString fname(fInputFile->GetName());
  if      (fname.Contains("pass1")) fFilepass = TString("pass1");
  else if (fname.Contains("pass2")) fFilepass = TString("pass2");
  else if (fname.Contains("pass3")) fFilepass = TString("pass3");
  else 
  {
    IlcError(Form("Pass number string not found: %s", fname.Data()));
    return;            
  }
}
