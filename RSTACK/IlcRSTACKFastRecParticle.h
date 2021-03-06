#ifndef ILCRSTACKFASTRECPARTICLE_H
#define ILCRSTACKFASTRECPARTICLE_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* $Id: IlcRSTACKFastRecParticle.h 53186 2011-11-25 14:28:01Z kharlov $ */

/* History of cvs commits:
 *
 * $Log$
 * Revision 1.36  2005/05/28 14:19:04  schutz
 * Compilation warnings fixed by T.P.
 *
 */

//_________________________________________________________________________
//  A  Particle modified by RSTACK response and produced by IlcRSTACKvFast
//  This is also a base class for IlcRSTACKRecParticle produced by IlcRSTACKPIDv1
//  Defines the particle type
//  To become a general class of IlcRoot ?    
//--               
//*-- Author: Yves Schutz (SUBATECH)

// --- ROOT system ---

class TClonesArray;
#include "TParticle.h"
#include "IlcPID.h"
// --- Standard library ---

// --- IlcRoot header files ---

class IlcRSTACKFastRecParticle : public TParticle {
  
 public:
  
  IlcRSTACKFastRecParticle() ;
  
  IlcRSTACKFastRecParticle(const IlcRSTACKFastRecParticle & rp) ;  // ctor
  IlcRSTACKFastRecParticle(const TParticle & p) ;  // ctor
  virtual ~IlcRSTACKFastRecParticle(){ } //dtor
  IlcRSTACKFastRecParticle & operator = (const IlcRSTACKFastRecParticle & /*rp*/);

  virtual Int_t DistancetoPrimitive(Int_t px, Int_t py) ; 
  virtual void Draw(Option_t *option) ;  
  virtual void ExecuteEvent(Int_t event, Int_t px, Int_t py) ;
  Int_t GetIndexInList() const { 
    // returns the index of this in the list
    return fIndexInList ; 
  } 
  virtual Int_t GetNPrimaries() const {return 0 ;}
  virtual const TParticle * GetPrimary(Int_t) const  {return 0 ;} 
  Int_t GetType() const { 
    // returns the type of the particle
    return fType ; 
  } 
  
  void SetPIDBit(UInt_t fSet) {
    // Set PID bit number fSet
    fType |= (1<<fSet) ; 
  } 
  
  Bool_t TestPIDBit(UInt_t fTest) const {
    // Check PID bit number fTest
    if (fType & (1<<fTest) ) return  kTRUE ;	
    else return kFALSE ;
  }
  
  Bool_t IsPhoton           (TString purity = "low") const;
  Bool_t IsPi0              (TString purity = "low") const;
  Bool_t IsElectron         (TString purity = "low") const;
  Bool_t IsHardPhoton       () const;
  Bool_t IsHardPi0          () const;
  Bool_t IsHadron           () const;
  Bool_t IsChargedHadron    () const;
  Bool_t IsNeutralHadron    () const;
  Bool_t IsFastChargedHadron() const;
  Bool_t IsSlowChargedHadron() const;
  Bool_t IsFastNeutralHadron() const;
  Bool_t IsSlowNeutralHadron() const;
  Bool_t IsEleCon(TString purity = "low") const; 

  TString Name() const ; 
  virtual void Paint(Option_t * option="");
  virtual void Print(const Option_t * = "") const ; 
  
  void SetTof(Float_t tof) { fTof = tof ; } 
  Float_t ToF() const { return fTof ; } 
  void SetType(Int_t type) ;
  
  void SetIndexInList(Int_t val) { 
    // sets the value of the index in the list 
    fIndexInList = val ; 
  }
  //This has to disappear
  enum EParticleType { kTYPE = 8, 
                       kUNDEFINED=-1, 
		       kNEUTRALEMFAST, kNEUTRALHAFAST,  kNEUTRALEMSLOW, kNEUTRALHASLOW, 
		       kCHARGEDEMFAST, kCHARGEDHAFAST,  kCHARGEDEMSLOW, kCHARGEDHASLOW } ; 
  
  typedef TClonesArray  FastRecParticlesList ; 
  
 protected:

  Int_t fIndexInList ; // the index of this RecParticle in the list stored in TreeR (to be set by analysis)
  Float_t fTof ;       // time of fliht
  Int_t fType ;        // particle type obtained by "virtual" reconstruction
  Float_t fPID[IlcPID::kSPECIESN] ; // PID probability densities

 private:

  ClassDef(IlcRSTACKFastRecParticle,2)  // Reconstructed Particle produced by the fast simulation 

};

#endif // IlcRSTACKFASTRECPARTICLE_H
