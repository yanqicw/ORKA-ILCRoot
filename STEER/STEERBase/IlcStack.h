#ifndef ILC_STACK_H
#define ILC_STACK_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* $Id: IlcStack.h 50802 2011-07-26 10:36:40Z morsch $ */

//  Particles stack class
//  Implements the TMCVirtualStack of the Virtual Monte Carlo
//  Author A.Morsch

class TClonesArray;
class TFile;
class TObjArray;
class TParticle;
class TString;
class TTree;
#include <TClonesArray.h>
#include <TArrayI.h>
#include <TVirtualMCStack.h>

class IlcHeader;

enum {kKeepBit=1, kDaughtersBit=2, kDoneBit=4, kTransportBit=BIT(14)};

class IlcStack : public TVirtualMCStack
{
  public:
    // creators, destructors
    IlcStack(Int_t size, const char* name = "");
    IlcStack();
    IlcStack(const IlcStack& st);
    virtual ~IlcStack();
    IlcStack& operator=(const IlcStack& st)
      {st.Copy(*this); return(*this);}

    // methods

    virtual void  PushTrack(Int_t done, Int_t parent, Int_t pdg, 
                           const Float_t *pmom, const Float_t *vpos, const Float_t *polar, 
                           Float_t tof, TMCProcess mech, Int_t &ntr,
                           Float_t weight, Int_t is);

    virtual void  PushTrack(Int_t done, Int_t parent, Int_t pdg,
                           Double_t px, Double_t py, Double_t pz, Double_t e,
                           Double_t vx, Double_t vy, Double_t vz, Double_t tof,
                           Double_t polx, Double_t poly, Double_t polz,
                           TMCProcess mech, Int_t &ntr, Double_t weight,
                           Int_t is);

    virtual TParticle* PopNextTrack(Int_t& track);
    virtual TParticle* GetCurrentTrack() const {return fCurrentTrack;}
    virtual TParticle* PopPrimaryForTracking(Int_t i);    

    void   ConnectTree(TTree* tree);
    Bool_t GetEvent();
    Bool_t PurifyKine();
    Bool_t ReorderKine();
    void   FinishEvent();
    void   FlagTrack(Int_t track);
    void   KeepTrack(Int_t itrack); 
    void   Clean(Int_t size = 0);
    void   Reset(Int_t size = 0);
    void   DumpPart(Int_t i) const;
    void   DumpPStack ();
    void   DumpLoadedStack () const;

    // set methods
    void  SetNtrack(Int_t ntrack);
    virtual void  SetCurrentTrack(Int_t track);                           
    void  SetHighWaterMark(Int_t hgwmk);    
    // get methods
    virtual Int_t GetNtrack() const;
    Int_t       GetNprimary() const;
    virtual Int_t GetCurrentTrackNumber() const;
    virtual Int_t GetCurrentParentTrackNumber() const;
    TParticle*  Particle(Int_t id);
    Int_t       GetPrimary(Int_t id);
    TTree*      TreeK() const {return fTreeK;}
    TParticle*  ParticleFromTreeK(Int_t id) const;
    Int_t       TreeKEntry(Int_t id) const;
    Bool_t      IsPhysicalPrimary(Int_t i);
    Bool_t      IsSecondaryFromWeakDecay(Int_t index);
    Bool_t      IsSecondaryFromMaterial (Int_t index);
    Int_t       TrackLabel(Int_t label) const {return fTrackLabelMap[label];}
    Int_t*      TrackLabelMap() {return fTrackLabelMap.GetArray();}
    const TObjArray*  Particles() const;
    
  protected:
    // methods
    void  CleanParents();
    void  ResetArrays(Int_t size);
    TParticle* GetParticleMapEntry(Int_t id) const;
    TParticle* GetNextParticle();
    Bool_t KeepPhysics(const TParticle* part);
    Bool_t IsStable(Int_t pdg) const;
  private:
    void Copy(TObject &st) const;

    // data members
    TClonesArray   fParticles;         //! Pointer to list of particles
    TObjArray      fParticleMap;       //! Map of particles in the supporting TClonesArray
    TArrayI        fParticleFileMap;   //  Map for particle ids 
    TParticle     *fParticleBuffer;    //! Pointer to current particle for writing
    TParticle     *fCurrentTrack;      //! Pointer to particle currently transported
    TTree         *fTreeK;             //! Particle stack  
    Int_t          fNtrack;            //  Number of tracks
    Int_t          fNprimary;          //  Number of primaries
    Int_t          fCurrent;           //! Last track returned from the stack
    Int_t          fCurrentPrimary;    //! Last primary track returned from the stack
    Int_t          fHgwmk;             //! Last track purified
    Int_t          fLoadPoint;         //! Next free position in the particle buffer
    TArrayI        fTrackLabelMap;     //! Map of track labels
    ClassDef(IlcStack,5) //Particles stack
};

// inline

inline void  IlcStack::SetNtrack(Int_t ntrack)
{ fNtrack = ntrack; }

inline Int_t IlcStack::GetNtrack() const
{ return fNtrack; }

inline Int_t IlcStack::GetNprimary() const
{ return fNprimary; }

inline Int_t IlcStack::GetCurrentTrackNumber() const 
{ return fCurrent; }

inline const TObjArray* IlcStack::Particles() const
{ return &fParticleMap; }

// inline protected

inline TParticle* IlcStack::GetParticleMapEntry(Int_t id) const
{ return (TParticle*) fParticleMap.At(id); }

#endif //ILC_STACK_H
