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
// CPV Preprocessor class. It runs by Shuttle at the end of the run,
// calculates calibration coefficients and dead/bad channels
// to be posted in OCDB.
//
// Author: Boris Polichtchouk, 25 January 2008
///////////////////////////////////////////////////////////////////////////////

#include "IlcPVECCpvPreprocessor.h"
#include "IlcLog.h"
#include "IlcCDBMetaData.h"
#include "IlcPVECCpvCalibData.h"
#include "TFile.h"
#include "TH1.h"
#include "TMap.h"
#include "TRandom.h"
#include "TKey.h"
#include "TList.h"
#include "TObjString.h"

ClassImp(IlcPVECCpvPreprocessor)

//_______________________________________________________________________________________
IlcPVECCpvPreprocessor::IlcPVECCpvPreprocessor() :
IlcPreprocessor("CPV",0)
{
  //default constructor
}

//_______________________________________________________________________________________
IlcPVECCpvPreprocessor::IlcPVECCpvPreprocessor(IlcShuttleInterface* shuttle):
IlcPreprocessor("CPV",shuttle)
{
  // Constructor

  AddRunType("PHYSICS");
  AddRunType("STANDALONE");

}

//_______________________________________________________________________________________
UInt_t IlcPVECCpvPreprocessor::Process(TMap* /*valueSet*/)
{
  // process data retrieved by the Shuttle

  // The fileName with the histograms which have been produced by
  // IlcPVECCpvDAs.
  // It is a responsibility of the SHUTTLE framework to form the fileName.
  
  TString runType = GetRunType();
  Log(Form("Run type: %s",runType.Data()));

  if(runType=="PHYSICS") {

    gRandom->SetSeed(0); //the seed is set to the current  machine clock!
    IlcPVECCpvCalibData calibData;

    TList* list = GetFileSources(kDAQ, "AMPLITUDES");
    if(!list) {
      Log("Sources list not found, exit.");
      return 1;
    }

    TIter iter(list);
    TObjString *source;
  
    while ((source = dynamic_cast<TObjString *> (iter.Next()))) {
      IlcInfo(Form("found source %s", source->String().Data()));

      TString fileName = GetFile(kDAQ, "AMPLITUDES", source->GetName());
      IlcInfo(Form("Got filename: %s",fileName.Data()));

      TFile f(fileName);

      if(!f.IsOpen()) {
	Log(Form("File %s is not opened, something goes wrong!",fileName.Data()));
	return 1;
      }

      const Int_t nMod=5;   // 1:5 modules
      const Int_t nCol=56;  // 1:56 columns in each CPV module (along the global Z axis)
      const Int_t nRow=128; // 1:128 rows in each CPV module

      Double_t coeff;
      char hnam[80];
      TH1F* histo=0;
    
      //Get the reference histogram
      //(author: Gustavo Conesa Balbastre)

      TList * keylist = f.GetListOfKeys();
      Int_t nkeys   = f.GetNkeys();
      Bool_t ok = kFALSE;
      TKey  *key;
      TString refHistoName= "";
      Int_t ikey = 0;
      Int_t counter = 0;
      TH1F* hRef = 0;
    
      //Check if the file contains any histogram
    
      if(nkeys< 2){
	Log(Form("Not enough histograms (%d) for calibration.",nkeys));
	return 1;
      }

      while(!ok){
	ikey = gRandom->Integer(nkeys);
	key = (TKey*)keylist->At(ikey);
	refHistoName = key->GetName();
	hRef = (TH1F*)f.Get(refHistoName);
	counter++;
	// Check if the reference histogram has too little statistics
	if(hRef->GetEntries()>2) ok=kTRUE;
	if(!ok && counter >= nkeys){
	  Log("No histogram with enough statistics for reference.");
	  return 1;
	}
      }
    
      Log(Form("reference histogram %s, %.1f entries, mean=%.3f, rms=%.3f.",
	       hRef->GetName(),hRef->GetEntries(),
	       hRef->GetMean(),hRef->GetRMS()));

      Double_t refMean=hRef->GetMean();
    
      // Calculates relative calibration coefficients for all non-zero channels
      
      for(Int_t mod=0; mod<nMod; mod++) {
	for(Int_t col=0; col<nCol; col++) {
	  for(Int_t row=0; row<nRow; row++) {
	    snprintf(hnam,80,"%d_%d_%d",mod,row,col); // mod_X_Z
	    histo = (TH1F*)f.Get(hnam);
	    //TODO: dead channels exclusion!
	    if(histo) {
	      coeff = histo->GetMean()/refMean;
	      if(coeff>0)
		calibData.SetADCchannelCpv(mod+1,col+1,row+1,1./coeff);
	      IlcInfo(Form("mod %d col %d row %d  coeff %f\n",mod,col,row,coeff));
	    }
	  }
	}
      }
    
      f.Close();
    }
  
    //Store CPV calibration data
  
    IlcCDBMetaData cpvMetaData;
    Bool_t cpvOK = Store("Calib", "CpvGainPedestals", &calibData, &cpvMetaData);

    if(cpvOK) return 0;
    else
      return 1;
  }

  Log(Form("Unknown or unused run type %s. Do nothing and return OK.",runType.Data()));
  return 0;

}

