#ifndef TAMPT_H
#define TAMPT_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

#ifndef ROOT_TGenerator
#include "TGenerator.h"
#endif
class TObjArray;

class TAmpt : public TGenerator {
public:
  TAmpt();
  TAmpt(Double_t efrm, const char *frame, const char *proj, const char *targ,
        Int_t iap, Int_t izp, Int_t iat, Int_t izt, Double_t bmin,  Double_t bmax);
  virtual            ~TAmpt();
  virtual void        Initialize();
  virtual void        GenerateEvent();
  virtual Int_t       ImportParticles(TClonesArray *particles, Option_t *option="");
  virtual TObjArray*  ImportParticles(Option_t *option="");
  virtual Int_t       ImportNucleons(TClonesArray *nucleons, Option_t *option="");

  //Parameters for the generation:
  virtual void        SetEFRM(Float_t efrm);
  virtual Float_t     GetEFRM() const;

  virtual void        SetFRAME(const char *frame);
  virtual const char *GetFRAME() const;

  virtual void        SetPROJ(const char *frame);
  virtual const char *GetPROJ() const;

  virtual void        SetTARG(const char *frame);
  virtual const char *GetTARG() const;

  virtual void        SetIAP(Int_t iap);
  virtual Int_t       GetIAP() const;

  virtual void        SetIZP(Int_t izp);
  virtual Int_t       GetIZP() const;

  virtual void        SetIAT(Int_t iat);
  virtual Int_t       GetIAT() const;

  virtual void        SetIZT(Int_t izt);
  virtual Int_t       GetIZT() const;

  virtual void        SetBMIN(Float_t bmin);
  virtual Float_t     GetBMIN() const;

  virtual void        SetBMAX(Float_t bmax);
  virtual Float_t     GetBMAX() const;

  //common HIPARNT access routines:
  virtual void        SetHIPR1(Int_t key, Float_t value);
  virtual Float_t     GetHIPR1(Int_t key) const;

  virtual void        SetIHPR2(Int_t key, Int_t value);
  virtual Int_t       GetIHPR2(Int_t key) const;

  virtual Float_t     GetHINT1(Int_t key) const;
  virtual Int_t       GetIHNT2(Int_t key) const;

  //common HIMAIN1 access routines - read-only common:
   
  virtual Int_t       GetNATT() const;
  virtual Float_t     GetEATT() const;
  virtual Int_t       GetJATT() const;
  virtual Int_t       GetNT()   const;
  virtual Int_t       GetNP()   const;
  virtual Int_t       GetN0()   const;
  virtual Int_t       GetN01()  const;
  virtual Int_t       GetN10()  const;
  virtual Int_t       GetN11()  const;
  virtual Float_t     GetBB()   const;
  virtual Float_t     GetPhi()  const { return GetHINT1(20); }

  // common HIMAIN2 access routines - read-only common:
  virtual Int_t       GetKATT(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPATT(Int_t key1, Int_t key2) const;
  virtual Float_t     GetVATT(Int_t key1, Int_t key2) const;

  // common HIJJET1 access routines - read-only common: 
  virtual Int_t       GetNPJ(Int_t key) const;
  virtual Int_t       GetKFPJ(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPJPX(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPJPY(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPJPZ(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPJPE(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPJPM(Int_t key1, Int_t key2) const;
  virtual Int_t       GetNTJ(Int_t key) const;
  virtual Int_t       GetKFTJ(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPJTX(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPJTY(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPJTZ(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPJTE(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPJTM(Int_t key1, Int_t key2) const;

  // common HIJJET2 access routines - read-only common: 
  virtual Int_t       GetNSG() const;
  virtual Int_t       GetNJSG(Int_t key) const;
  virtual Int_t       GetIASG(Int_t key1, Int_t key2) const;
  virtual Int_t       GetK1SG(Int_t key1, Int_t key2) const;
  virtual Int_t       GetK2SG(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPXSG(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPYSG(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPZSG(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPESG(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPMSG(Int_t key1, Int_t key2) const;

  // common HISTRNG access routines - read-only common: 
  virtual Int_t       GetNFP(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPP(Int_t key1, Int_t key2)  const;
  virtual Int_t       GetNFT(Int_t key1, Int_t key2) const;
  virtual Float_t     GetPT(Int_t key1, Int_t key2)  const;

  // common LUDAT1_AMPT common:
  virtual void        SetPARJ(Int_t key, Float_t parm);
  virtual void        SetMSTJ(Int_t key, Int_t   parm);   
   
  // access to AMPT routines:
  virtual void        Amptset(Double_t efrm, const char* frame, const char*
                              proj, const char* targ, Int_t iap, Int_t izp,
                              Int_t iat, Int_t izt);
			      
  virtual void        Ampt(const char* frame, Double_t bmin, Double_t bmax);

  virtual Float_t     Profile(Float_t b);

  // access to Jetset routines
  virtual void        Rluget(Int_t lfn, Int_t move=0);
  virtual void        Rluset(Int_t lfn, Int_t move=0);   

  // set AMPT specific
  virtual void        SetIsoft(Int_t i);
  virtual void        SetNtMax(Int_t max);
  virtual void        SetIpop(Int_t pop);
  virtual void        SetXmu(Float_t m);
  virtual void        SetAlpha(Float_t alpha);
  virtual void        SetStringFrag(Float_t a, Float_t b);
  virtual void        SetReactionPlaneAngle(Double_t psi){fPsi=psi;};  // Set reaction plane angle
  virtual Double_t    GetReactionPlaneAngle(){return fPsi;};           // Get reaction plane angle

protected:
  Double_t     fEfrm;     // Energy in the centre of mass (CMS) or lab-frame (LAB)
  TString      fFrame;    // Reference frame CMS or LAB
  TString      fProj;     // Projectile name
  TString      fTarg;     // Target name
  Int_t        fIap;      // Atomic number of projectile
  Int_t        fIzp;      // Charge number of projectile 
  Int_t        fIat;      // Atomic number of target
  Int_t        fIzt;      // Charge number of target
  Double_t     fBmin;     // Minimum impact parameter
  Double_t     fBmax;     // Maximum impact parameter
  Double_t     fPsi;      // Reaction plane angle
  ClassDef(TAmpt,3)  //Interface to Ampt Event Generator
};
#endif
