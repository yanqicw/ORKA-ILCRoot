#ifndef ILCRSTACKDigitizer_H
#define ILCRSTACKDigitizer_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */


//_________________________________________________________________________
//  Task Class for making Digits in RSTACK      
// Class performs digitization of Summable digits (in the RSTACK this is
// the sum of contributions of all primary particles into given tile). 
// In addition it performs mixing of summable digits from different events.
//                  


// --- ROOT system ---
//#include "TObjString.h"
class TArrayI ;
class TClonesArray ; 

// --- Standard library ---

// --- IlcRoot header files ---
#include "IlcDigitizer.h"
#include "IlcConfig.h"
#include "IlcRSTACKPulseGenerator.h"
class IlcDigitizationInput ;
class IlcRSTACKCalibData ; 

class IlcRSTACKDigitizer: public IlcDigitizer {

public:
  IlcRSTACKDigitizer() ;          // ctor
  IlcRSTACKDigitizer(TString ilcrunFileNameFile, TString eventFolderName = IlcConfig::GetDefaultEventFolderName()) ; 
  IlcRSTACKDigitizer(IlcDigitizationInput * digInput) ;
  virtual ~IlcRSTACKDigitizer() ;       

  void    Digitize(Int_t event) ;       // Make Digits from SDigits 
  void    Digitize(Option_t *option);   // Supervising method

  void   SetEventRange(Int_t first=0, Int_t last=-1) {fFirstEvent=first; fLastEvent=last; }

  //General
  Int_t   GetDigitsInRun()  const { return fDigitsInRun ;}  

  void    Print(const Option_t * = "")const ;
 
private:
  IlcRSTACKDigitizer(const IlcRSTACKDigitizer & dtizer) ;
  IlcRSTACKDigitizer & operator = (const IlcRSTACKDigitizer & /*rvalue*/);

  virtual Bool_t Init() ; 
  void    InitParameters() ; 
  void    PrintDigits(Option_t * option) ;
  void    Unload() ; 
  void    WriteDigits() ;                     // Writes Digits for the current event
  Float_t TimeOfNoise(void) const;            // Calculate time signal generated by noise

  Float_t TimeResolution(Float_t energy) ;    //TOF resolution

  //Calculate the time of crossing of the threshold by front edge
  //  Float_t FrontEdgeTime(TClonesArray * ticks) const ; 
  //Calculate digitized signal with gived ADC parameters
  Float_t Calibrate(Float_t amp, Int_t absId) ;
  Float_t CalibrateT(Float_t time, Int_t absId) ;
  void    Decalibrate(IlcRSTACKDigit * digit);

private:

  Bool_t  fDefaultInit;             //! Says if the task was created by defaut ctor (only parameters are initialized)
  Int_t   fDigitsInRun ;            //! Total number of digits in one run
  Bool_t  fInit ;                   //! To avoid overwriting existing files

  Int_t   fInput ;                  // Number of files to merge
  TString * fInputFileNames ;       //[fInput] List of file names to merge 
  TString * fEventNames ;           //[fInput] List of event names to merge

  Int_t   fTiles ;            // Number of scintillator tiles in the given geometry

  TString fEventFolderName;         // skowron: name of EFN to read data from in stand alone mode
  Int_t   fFirstEvent;              // first event to process
  Int_t   fLastEvent;               // last  event to process 
  IlcRSTACKCalibData* fcdb;           //! Calibration parameters DB

  Int_t fEventCounter ;             //! counts the events processed

  IlcRSTACKPulseGenerator *fPulse;    //! Pulse shape generator
  Int_t *fADCValuesLG;              //! Array of low-gain ALTRO samples
  Int_t *fADCValuesHG;              //! Array of high-gain ALTRO samples
  

  Float_t fSiPMPixels;         //Number of pixels in a SiPM
  Float_t fSiPMNoise;          //RMS of SiPM noise
  Float_t fElectronicGain;     //Electronic Gain
  Float_t fConversionFactor;   //factor to convert number of p.e. in ADC counts
  Float_t fENF;                //Excess Noise Factor
  Float_t fDigitsThreshold ;   //minimal energy to keep digit 
  Float_t fADCchannel ;        //width of ADC channel in GeV
  Int_t   fADCbits ;           //ADC bits

  ClassDef(IlcRSTACKDigitizer,5)  // description 

};


#endif // IlcRSTACKDigitizer_H
