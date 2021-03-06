#ifndef ILCTARGETCILCBRATIONSPD_H
#define ILCTARGETCILCBRATIONSPD_H
/* Copyright(c) 2005-2006, ILC Project Experiment, All rights reserved.   *
 * See cxx source for full Copyright notice                               */


#include "TRandom.h"
#include "IlcTARGETCalibration.h"
#include "TArrayI.h"
#include "IlcTARGETresponseSPD.h"


////////////////////////////////////////////////////
//                                                //
// TARGET response class for SPD                     //
////////////////////////////////////////////////////
class IlcTARGETCalibrationSPD :  public IlcTARGETCalibration {
 public:
    IlcTARGETCalibrationSPD(); // default constructor
    virtual ~IlcTARGETCalibrationSPD() {;} // destructror

    // Set Threshold and noise + threshold fluctuations parameter values
    virtual  void   SetThresholds(Double_t thresh, Double_t sigma)
	{fThresh=thresh; fSigma=sigma;}
    // Get Threshold and noise + threshold fluctuations parameter values
    virtual  void   Thresholds(Double_t &thresh, Double_t &sigma) const
	{thresh=fThresh; sigma=fSigma;}
    // Set Bias Voltage parameter
    virtual void    SetBiasVoltage(Double_t bias=18.182) {fBiasVoltage=bias;}
    Double_t  GetBiasVoltage() const {return fBiasVoltage;}
    //Returns just baseline value
    Double_t GetBaseline() const {return fBaseline;}
    // Set noise and baseline in one (abstract method of IlcTARGETCalibration)
    virtual void SetNoiseParam(Double_t n,Double_t b)
        {fNoise = n;fBaseline = b;}
    // Get noise and baseline in one (abstract method of IlcTARGETCalibration)
    virtual void GetNoiseParam(Double_t &n,Double_t &b) const
        {n =fNoise;b = fBaseline;}
    // Returns just noise value
    Double_t GetNoise() const {return fNoise;} 
    //Declaration of member functions peculiar to this class
    // Applies a random noise and addes the baseline
    Double_t ApplyBaselineAndNoise() const {return fBaseline+
                                               fNoise*gRandom->Gaus();}
    // Set coupling parameters 
    virtual void SetCouplingParam(Double_t col, Double_t row)
        {fCouplCol = col; fCouplRow = row;}
    // Get coupling parameters 
    virtual void GetCouplingParam(Double_t &col, Double_t &row) const
        {col = fCouplCol; row = fCouplRow;}


    virtual void    GiveCompressParam(Int_t *) const
      {NotImplemented("GiveCompressParam");}
    virtual  void   SetDetParam(Double_t *)
      {NotImplemented("SetDetParam");}
    virtual void   GetDetParam(Double_t *) const 
      {NotImplemented("GetDetParam");}
    virtual  void   SetNDetParam(Int_t /* n */)
      {NotImplemented("SetNDetParam");}
    virtual Int_t  NDetParam() const
      {NotImplemented("NDetParam"); return 0;}
    virtual void    SetSigmaSpread(Double_t, Double_t) 
      {NotImplemented("SetSigmaSpread");}
    virtual void    SigmaSpread(Double_t & /* p1 */,Double_t & /* p2 */) const 
      {NotImplemented("SigmaSpread");}

    virtual void GetCouplingOption(char *opt) const {((IlcTARGETresponseSPD*)fResponse)->CouplingOption(opt);}
    virtual void SetCouplingOption(const char* opt) {((IlcTARGETresponseSPD*)fResponse)->SetCouplingOption(opt);}
    virtual void SetParamOptions(const char* a,const char* b) {((IlcTARGETresponseSPD*)fResponse)->SetParamOptions(a,b);}
    virtual void GetParamOptions(char *a,char* b) const {((IlcTARGETresponseSPD*)fResponse)->ParamOptions(a,b);}
    virtual void SetSigmaDiffusionAsymmetry(Double_t ecc) {((IlcTARGETresponseSPD*)fResponse)->SetSigmaDiffusionAsymmetry(ecc);}
    virtual void GetSigmaDiffusionAsymmetry(Double_t &ecc) const {((IlcTARGETresponseSPD*)fResponse)->GetSigmaDiffusionAsymmetry(ecc);}
    
    void   AddDead(UInt_t col, UInt_t row);
    Int_t  GetNrDead() const {return fNrDead;}
    Int_t  GetDeadColAt(UInt_t index); //returns -1 if out of bounds
    Int_t  GetDeadRowAt(UInt_t index); //returns -1 if out of bounds
    void   ClearDead() {fDeadChannels.Reset(); fNrDead=0;}
    Bool_t IsPixelDead(Int_t col, Int_t row) const ;
    Bool_t IsPixelDead(Int_t mod,Int_t ix,Int_t iz) const ; // remove as soon as possible!!!

    void   AddNoisy(UInt_t col, UInt_t row);
    Int_t  GetNrNoisy() const {return fNrNoisy;}
    Int_t  GetNoisyColAt(UInt_t index); //returns -1 if out of bounds
    Int_t  GetNoisyRowAt(UInt_t index); //returns -1 if out of bounds
    void   ClearNoisy() {fNoisyChannels.Reset(); fNrNoisy=0;}
    Bool_t IsPixelNoisy(Int_t col, Int_t row) const ;


 protected:
    // static const Double_t fgkDiffCoeffDefault; //default for fDiffCoeff
    static const Double_t fgkThreshDefault; //default for fThresh
    static const Double_t fgkSigmaDefault; //default for fSigma
    static const Double_t fgkCouplColDefault; //default for fCouplCol
    static const Double_t fgkCouplRowDefault; //default for fCouplRow
    static const Double_t fgkBiasVoltageDefault; //default for fBiasVoltage
    Double_t fBaseline;        // Base-line value
    Double_t fNoise;           // Gaussian noise scale
    Double_t fThresh;          // Threshold value
    Double_t fSigma;           // Noise + threshold fluctuations value
    Double_t fCouplCol;        // Coupling parameter along the cols
    Double_t fCouplRow;        // Coupling parameter along the rows
    Double_t fBiasVoltage;     // Bias Voltage for the SPD (used to compute DistanceOverVoltage)
    UInt_t   fNrDead;           // Nr of dead pixels
    TArrayI fDeadChannels;     // Array with dead channels info (col0,row0,col1...rowN) N = fNrDead
    UInt_t   fNrNoisy;          // Nr of noisy pixels
    TArrayI fNoisyChannels;    // Array with noisy channels info (col0,row0,col1...rowN) N = fNrNoisy

    ClassDef(IlcTARGETCalibrationSPD,4) // SPD response
};

#endif
