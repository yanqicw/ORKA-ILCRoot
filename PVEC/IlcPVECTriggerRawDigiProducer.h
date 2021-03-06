#ifndef ILCPVECTRIGGERRAWDIGIPRODUCER_H
#define ILCPVECTRIGGERRAWDIGIPRODUCER_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                          */

class IlcPVECTriggerRawReader;
class IlcPVECTriggerParameters;
class IlcRawReader;
class IlcCaloRawStreamV3;

#include "TString.h"
#include "TClonesArray.h"

#include <vector>

class IlcPVECTriggerRawDigiProducer
{
 public:
  
  IlcPVECTriggerRawDigiProducer();
  IlcPVECTriggerRawDigiProducer(IlcRawReader *rawReader);
  
  virtual ~IlcPVECTriggerRawDigiProducer();
  
  void ProcessEvent(TClonesArray* tdigits);

  void SetTriggerParameters(IlcPVECTriggerParameters* parameters) {fParameters = parameters;}
  void SetAnalyseModule(int mod, bool analyse = true) {fModules[mod] = analyse;}
    
  static int Get2x2Max(IlcPVECTriggerRawReader*, IlcPVECTriggerParameters*, int mod, int xIdx, int zIdx);
  static int Get2x2Signal(IlcPVECTriggerRawReader*, IlcPVECTriggerParameters*, int mod, int xIdx, int zIdx, int timeBin);
  static int Get4x4Max(IlcPVECTriggerRawReader*, IlcPVECTriggerParameters*, int mod, int TRURow, int branch, int xIdx, int zIdx);
  static int Get4x4Signal(IlcPVECTriggerRawReader*, IlcPVECTriggerParameters*, int mod, int TRURow, int branch, int xIdx, int zIdx, int timeBin);
  
  static bool Is2x2Active(IlcPVECTriggerRawReader*, int mod, int xIdx, int zIdx);
  static bool Is2x2Active(IlcPVECTriggerRawReader*, int mod, int xIdx, int zIdx, int timeBin);
    
  const static int kNMods = 5; // number of PVEC modules
  const static int kNTRURows = 4; // number of TRU rows
  const static int kNBranches = 2; // number of branches
  const static int kN2x2X = 32; // (=64/2) Number of 2x2 in X direction
  const static int kN2x2Z = 28; // (=56/2) Number of 2x2 in Z direction
  const static Int_t kN2x2XPrTRURow = 8; // (=64/2/4) Number of 2x2 pr. row
  const static Int_t kN2x2ZPrBranch = 14; // (=56/2/2) Number of 2x2 pr. branch
  const static Int_t kN4x4XPrTRURow = 7; // (=64/2/4 -1) Number of 4x4 pr. row
  const static Int_t kN4x4ZPrBranch = 13; // (=56/2/2) -1 Number of 4x4 pr. branch  
  const static int kNTRUTimeBins = 128; // number of TRU time bins
  const static int kNDefaultNEMCTimeBins = 62;

 private:  
  IlcPVECTriggerRawDigiProducer(const IlcPVECTriggerRawDigiProducer &tdp); // not implemented
  IlcPVECTriggerRawDigiProducer& operator= (const IlcPVECTriggerRawDigiProducer &tdp); // not implemented

 protected:
  std::vector<bool> fModules; // , per module: should analyser analyse module
  UShort_t fSaturationThreshold;
  IlcPVECTriggerParameters* fParameters;
  
private:
  IlcRawReader            * fRawReader;       //! Raw data reader
  IlcCaloRawStreamV3      * fRawStream;       //! Calorimeter decoder of ALTRO format
  IlcPVECTriggerRawReader * fTriggerReader;   //! TriggerRawReader

  ClassDef(IlcPVECTriggerRawDigiProducer,1)
};

#endif
