#ifndef IlcAODMCPARTICLE_H
#define IlcAODMCPARTICLE_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */


//-------------------------------------------------------------------------
//     IlcVParticle realisation for MC Particles in the AOD
//     Stripped dow IlcMCParticle
//     Author: Christian Klein B�sing, CERN
//-------------------------------------------------------------------------

#include <Rtypes.h>
#include <TParticlePDG.h>
#include <TExMap.h>
#include <TString.h>


#include "IlcTrackReference.h"
#include "IlcVParticle.h"
#include "IlcMCParticle.h"

class IlcAODEvent;
class TParticle;
class TClonesArray;

class IlcAODMCParticle: public IlcVParticle {
 public:
  IlcAODMCParticle();
  IlcAODMCParticle(const IlcMCParticle* part, Int_t label=0,Int_t flag = 0);
  virtual ~IlcAODMCParticle(){};
  IlcAODMCParticle(const IlcAODMCParticle& mcPart); 
  IlcAODMCParticle& operator=(const IlcAODMCParticle& mcPart);
  
  // Kinematics
    virtual Double_t Px()        const;
    virtual Double_t Py()        const;
    virtual Double_t Pz()        const;
    virtual Double_t Pt()        const;
    virtual Double_t P()         const;
    virtual Bool_t   PxPyPz(Double_t p[3]) const;
    
    virtual Double_t OneOverPt() const;
    virtual Double_t Phi()       const;
    virtual Double_t Theta()     const;
    
    virtual Double_t Xv() const;
    virtual Double_t Yv() const;
    virtual Double_t Zv() const;
    virtual Bool_t   XvYvZv(Double_t x[3]) const;  
    virtual Double_t T() const;

    virtual Double_t E()          const;
    virtual Double_t M()          const;
    
    virtual Double_t Eta()        const;
    virtual Double_t Y()          const;
    
    virtual Short_t Charge()      const;

    virtual Int_t   Label()       const;
    virtual Int_t   GetLabel()    const {return Label();} 
	    
    // PID
    virtual const Double_t *PID() const {return 0;} // return PID object (to be defined, still)

    // 
    virtual Double_t GetCalcMass() const;
    virtual void SetDaughter(Int_t i,Int_t id){if(i<2)fDaughter[i] = id;}
    virtual Int_t GetDaughter(Int_t i) const {return fDaughter[i];}
    virtual Int_t GetNDaughters  () const { return fDaughter[1]>0 ? fDaughter[1]-fDaughter[0]+1 : 0;}
    virtual void SetMother(Int_t im){fMother = im;}
    virtual Int_t GetMother() const {return fMother;}
    virtual void Print(const Option_t *opt = "") const;
    virtual Int_t GetPdgCode() const { return fPdgCode;}
    virtual Int_t PdgCode()    const { return GetPdgCode();}
    
    enum { kPrimary = 1<<0, kPhysicalPrim = 1<<1 };
    void SetFlag(UInt_t flag){fFlag = flag;}
    UInt_t GetFlag() const {return fFlag;}


    // for the status we use the upper 16 bits/2 bytes of the flag word
    void SetStatus(Int_t status){
      if(status<0)return; // a TParticle can have a negative stuts, catch this here and do nothing
      fFlag &= 0xffff;   // reset the upper bins keep the lower bins
      fFlag |= (((UInt_t)status)<<16); // bit shift by 16
    }
    UInt_t GetStatus() const {
      // bit shift by 16
      return fFlag>>16;
    }

    // Bitwise operations
    void SetPrimary(Bool_t b = kTRUE){
      if(b)fFlag |= kPrimary;
      else fFlag &= ~kPrimary;
    }
    Bool_t IsPrimary() const {return ((fFlag&kPrimary)==kPrimary);} 

    void SetPhysicalPrimary(Bool_t b = kTRUE){
     if(b)fFlag |= kPhysicalPrim;
     else fFlag &= ~kPhysicalPrim; 
    } 
    Bool_t IsPhysicalPrimary() const {return ((fFlag&kPhysicalPrim)==kPhysicalPrim);} 

#ifdef WIN32
    static const char* StdBranchName();
#else
    static const char* StdBranchName(){return fgkStdBranchName.Data();}
#endif

 private:

    static TString fgkStdBranchName;      // Standard branch name


  Int_t            fPdgCode;              // PDG code of the particle
  UInt_t           fFlag;                 // Flag for indication of primary etc
  Int_t            fLabel;                // Label of the original MCParticle 
  Int_t            fMother;               // Index of the mother particles
  Int_t            fDaughter[2];          // Indices of the daughter particles
  Double32_t       fPx;                   // x component of momentum
  Double32_t       fPy;                   // y component of momentum
  Double32_t       fPz;                   // z component of momentum
  Double32_t       fE;                    // Energy

  Double32_t       fVx;                   // [0.,0.,12] x of production vertex
  Double32_t       fVy;                   // [0.,0.,12] y of production vertex
  Double32_t       fVz;                   // [0.,0.,12] z of production vertex
  Double32_t       fVt;                   // [0.,0.,12] t of production vertex

  // Copy the uniquID to another data member? unique ID is correctly handled 
  // via TOBject Copy construct but not by IlcVParticle ctor (no passing of 
  // TParticles
  // Need a flag for primaries?

  /*
    const TMCProcess kMCprocesses[kMaxMCProcess] =
    {
     kPNoProcess, kPMultipleScattering, kPEnergyLoss, kPMagneticFieldL,
     kPDecay, kPPair, kPCompton, kPPhotoelectric, kPBrem, kPDeltaRay,
     kPAnnihilation, kPHadronic, kPNoProcess, kPEvaporation, kPNuclearFission,
     kPNuclearAbsorption, kPPbarAnnihilation, kPNCapture, kPHElastic,
     kPHInhelastic, kPMuonNuclear, kPTOFlimit,kPPhotoFission, kPNoProcess,
     kPRayleigh, kPNoProcess, kPNoProcess, kPNoProcess, kPNull, kPStop
    };
  */

  ClassDef(IlcAODMCParticle,5)  // IlcVParticle realisation for AODMCParticles

};

inline Double_t IlcAODMCParticle::Px()        const {return fPx;}
inline Double_t IlcAODMCParticle::Py()        const {return fPy;}
inline Double_t IlcAODMCParticle::Pz()        const {return fPz;}
inline Double_t IlcAODMCParticle::Pt()        const {return TMath::Sqrt(fPx*fPx+fPy*fPy);}
inline Double_t IlcAODMCParticle::P()         const {return TMath::Sqrt(fPx*fPx+fPy*fPy+fPz*fPz); }
inline Double_t IlcAODMCParticle::OneOverPt() const {return 1. / Pt();}
inline Bool_t   IlcAODMCParticle::PxPyPz(Double_t p[3]) const { p[0] = fPx; p[1] = fPy; p[2] = fPz; return kTRUE; }
inline Double_t IlcAODMCParticle::Phi()       const {return TMath::Pi()+TMath::ATan2(-fPy,-fPx); }  // note that Phi() returns an angle between 0 and 2pi
inline Double_t IlcAODMCParticle::Theta()     const {return (fPz==0)?TMath::PiOver2():TMath::ACos(fPz/P()); }
inline Double_t IlcAODMCParticle::Xv()        const {return fVx;}
inline Double_t IlcAODMCParticle::Yv()        const {return fVy;}
inline Double_t IlcAODMCParticle::Zv()        const {return fVz;}
inline Bool_t   IlcAODMCParticle::XvYvZv(Double_t x[3]) const { x[0] = fVx; x[1] = fVy; x[2] = fVz; return kTRUE; }
inline Double_t IlcAODMCParticle::T()         const {return fVt;}
inline Double_t IlcAODMCParticle::E()         const {return fE;}
inline Double_t IlcAODMCParticle::Eta()       const {  
  Double_t pmom = P();
  if (pmom != TMath::Abs(fPz)) return 0.5*TMath::Log((pmom+fPz)/(pmom-fPz));
  else                         return 1.e30;
}


inline Double_t IlcAODMCParticle::Y()         const 
{
    Double_t e  = E();
    Double_t pz = Pz();
    
    if (e > TMath::Abs(pz)) { 
	return 0.5*TMath::Log((e+pz)/(e-pz));
    } else { 
	return -999.;
    }
}

inline Int_t IlcAODMCParticle::Label()       const {return fLabel;}

inline Double_t IlcAODMCParticle::GetCalcMass() const {

  Double_t m2 = E()*E()-Px()*Px()-Py()*Py()-Pz()*Pz();
  if(m2<0)return 0;
  return TMath::Sqrt(m2);
}


#endif
