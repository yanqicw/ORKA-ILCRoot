#ifndef ILCTARGETHIT_H
#define ILCTARGETHIT_H
/* Copyright(c) 2005-2006, ILC Project Experiment, All rights reserved.   *
 * See cxx source for full Copyright notice                               */

/* $Id: IlcTARGEThit.h,v 1.1.1.1 2008/03/11 14:52:50 vitomeg Exp $ */

////////////////////////////////////////////////////////////////////////
// Version: 0
// Written by Rene Brun, Federico Carminati, and Roberto Barbera
//
// Version: 1
// Modified and documented by Bjorn S. Nilsen
// July 11 1999
//
// IlcTARGEThit is the hit class for the TARGET. Hits are the information
// that comes from a Monte Carlo at each step as a particle mass through
// sensitive detector elements as particles are transported through a
// detector.
//
// Data members:
//
// Int_t fTrack
//     See IlcHit for a full description. The track number of the track
// that made this hit.
//
// Float_t fX
//     See IlcHit for a full description. The global x position of the
// hit (in the standard units of the Monte Carlo).
//
// Float_t fY
//     See IlcHit for a full description. The global y position of the
// hit (in the standard units of the Monte Carlo).
//
// Float_t fZ
//     See IlcHit for a full description. The global z position of the
// hit (in the standard units of the Monte Carlo).
//
// Int_t fStatus
//     The track status flag. This flag indicates the track status
// at the time of creating this hit. It is made up of the following 8
// status bits from highest order to lowest order bits
// 0           :  IsTrackAlive():    IsTrackStop():IsTrackDisappeared():
// IsTrackOut():IsTrackExiting():IsTrackEntering():IsTrackInside()     .
// See IlcMC for a description of these functions. If the function is
// true then the bit is set to one, otherwise it is zero.
//
// Int_t fLayer
//     The layer number of the detector that contains this hit. See
// IlcTARGETgeom and IlcTARGETv? for a description of the geometry.
//
// Int_t fLadder
//     The ladder number of the detector that contains this hit. See
// IlcTARGETgeom and IlcTARGETv? for a description of the geometry.
//
// Int_t fDet
//     The detector number of the detector that contains this hit. See
// IlcTARGETgeom and IlcTARGETv? for a description of the geometry.
//
// Float_t fPx
//     The x momentum, in global coordinates, of the particle that
// "created" the hit at the time and position of the hit. The units
// are those determined by the Monte Carlo.
//
// Float_t fPy
//     The y momentum, in global coordinates, of the particle that
// "created" the hit at the time and position of the hit. The units
// are those determined by the Monte Carlo.
//
// Float_t fPz
//     The z momentum, in global coordinates, of the particle that
// "created" the hit at the time and position of the hit. The units
// are those determined by the Monte Carlo.
//
// Float_t fDestep
//     The energy lost by the particle during the step ending in this
// hit. The units are those determined by the Monte Carlo.
//
// Float_t fTof
//     The time of flight associated with the particle ending in this
// hit. The time is typically measured from the point of creation of the
// original particle (if this particle is a daughter).  The units
// are those determined by the Monte Carlo.
//
//
////////////////////////////////////////////////////////////////////////

#include "IlcHit.h" 

class TLorentzVector;
class TParticle;

class IlcTARGEThit : public IlcHit {

 public:
    IlcTARGEThit();// Default consrtructor
    // Old Standard Constructor
    IlcTARGEThit(Int_t shunt, Int_t track, Int_t *vol, Float_t *hits);
    // New Standard Constructor
    IlcTARGEThit(Int_t shunt,Int_t track,Int_t *vol,Float_t edep,Float_t tof,
	      TLorentzVector &x,TLorentzVector &x0,TLorentzVector &p);
    // Default destructor
    virtual ~IlcTARGEThit() {}
    // Get Hit information functions.
    // virtual int GetTrack() const {return fTrack;} // define in IlcHit
    // virtual void SetTrack(int track) const {fTrack=track;) // IlcHit
    virtual Int_t GetTrackStatus() const {//returns the status code
	return fStatus;}
    virtual Int_t GetTrackStatus0() const {//returns the status code
	return fStatus0;}
    virtual Int_t GetLayer() const {// returns the layer number
	return fLayer;}
    virtual Int_t GetLadder() const {// returns the ladder number
	return fLadder;}
    virtual Int_t GetDetector() const {// returns the detector number
	return fDet;}
    virtual void  GetDetectorID(Int_t &layer,Int_t &ladder,Int_t &det)const {
	// returns the detector ID
	layer=fLayer;ladder=fLadder;det=fDet;return;};
    virtual Int_t GetModule();
    virtual Float_t GetIonization() const {// returns the Destep
	return fDestep;}
    //
    virtual void GetPositionG(Float_t &x,Float_t &y,Float_t &z)const {
	// returns the position in the Global frame
	x=fX;y=fY;z=fZ;return;};
    virtual void GetPositionG(Double_t &x,Double_t &y,Double_t &z)const {
	// returns the position in the Global frame
	x=fX;y=fY;z=fZ;return;};
    virtual Float_t GetTOF() const {// returns the time of flight
	return fTof;}
    // Returns particle 3 position at this hit in global coordinates.
    virtual void GetPositionG(Float_t &x,Float_t &y,Float_t &z,Float_t &tof)
	const {// returns the position in the Global frame and the time of
	// flight
	x=fX;y=fY;z=fZ,tof=fTof;return;};
    virtual void GetPositionG(Double_t &x,Double_t &y,Double_t &z,Double_t &t)
	const {// Returns particle 3 position and the time of flight at this
	// hit in global coordinates.
	x=fX;y=fY;z=fZ,t=fTof;return;};
    virtual Float_t GetXG()const {// Returns particle X position at this hit
	// in global coordinates.
	return fX;}
    virtual Float_t GetYG()const {// Returns particle X position at this hit
	// in global coordinates.
	return fY;}
    virtual Float_t GetZG()const {// Returns particle Z position at this hit
	// in global coordinates.
	return fZ;}
    // Returns particle 3 position at this hit in global coordinates.
    virtual void GetPositionG0(Float_t &x,Float_t &y,Float_t &z,Float_t &tof)
	const {// returns the initial position in the Global frame and the
	// time of flight
	x=fx0;y=fy0;z=fz0,tof=fTof;return;};
    // Returns particle 3 position at this hit in global coordinates.
    virtual void GetPositionG0(Double_t &x,Double_t &y,Double_t &z,
			       Double_t &tof)
	const {// returns the initial position in the Global frame and the
	// time of flight
	x=fx0;y=fy0;z=fz0,tof=fTof;return;};
    virtual void GetPositionL(Float_t &x,Float_t &y,Float_t &z);
    // Returns particle 3 position at this hit in local coordinates.
    virtual void GetPositionL(Float_t &x,Float_t &y,Float_t &z,Float_t &tof);
    virtual void GetPositionL(Double_t &x,Double_t &y,Double_t &z){
	 // Returns particle 3 position at this hit in local coordinates.
	 Float_t xf,yf,zf;GetPositionL(xf,yf,zf);x=xf,y=yf;z=zf;}
    virtual void GetPositionL(Double_t &x,Double_t &y,Double_t &z,Double_t &t){
    // Returns particle 3 position and the time of flight at this hit
    // in local coordinates.
	 Float_t xf,yf,zf,tf;GetPositionL(xf,yf,zf,tf);x=xf,y=yf;z=zf;t=tf;}
    // Returns particle 3 initial position and the time of flight at this hit
    // in local coordinates.
    virtual void GetPositionL0(Double_t &x,Double_t &y,Double_t &z,Double_t &t);
    virtual Float_t GetXL();
    // Returns particle X position at this hit in local coordinates.
    virtual Float_t GetYL();
    // Returns particle Y position at this hit in local coordinates.
    virtual Float_t GetZL();
    // Returns particle Z position at this hit in local coordinates.
    // Get Monti Carlo information about hit.
    virtual void GetMomentumG(Float_t &px,Float_t &py,Float_t &pz)const {
	// returns the particle momentum in the Global frame
	px=fPx;py=fPy;pz=fPz;return;};
    virtual void GetMomentumG(Double_t &px,Double_t &py,Double_t &pz)const {
	// returns the particle momentum in the Global frame
	px=fPx;py=fPy;pz=fPz;return;};
    virtual Float_t GetPXG()const {// Returns particle X momentum at this hit
	// in global coordinates.
	return fPx;}
    virtual Float_t GetPYG()const {// Returns particle Y momentum at
	// this hit in global coordinates.
	return fPy;}
    virtual Float_t GetPZG()const {// Returns particle Z momentum at
	// this hit in global coordinates.
	return fPz;}
    virtual void GetMomentumL(Float_t &px,Float_t &py,Float_t &pz);
    virtual void GetMomentumL(Double_t &px,Double_t &py,Double_t &pz){
       // Returns particle 3 momentum at this hit in local coordinates.	 
	    Float_t x,y,z;GetMomentumL(x,y,z);px=x,py=y,pz=z;}
    
    virtual Float_t GetPXL();
    // Returns particle X momentum at this hit in local coordinates.
    virtual Float_t GetPYL();
    // Returns particle Y momentum at this hit in local coordinates.
    virtual Float_t GetPZL();
    // Returns particle Z momentum at this hit in local coordinates.
    virtual TParticle * GetParticle() const; // Returns ptr to this particle.
    Bool_t StatusInside() const {// checks if the particle is "inside"
	if((fStatus&0x0001)==0) return kFALSE;else return kTRUE;}
    Bool_t StatusEntering() const {// checks if the particle is "entering"
	if((fStatus&0x0002)==0) return kFALSE;else return kTRUE;}
    Bool_t StatusExiting() const {// checks if the particle is "exiting"
	if((fStatus&0x0004)==0) return kFALSE;else return kTRUE;}
    Bool_t StatusOut() const {// checks if the particle is "out"
	if((fStatus&0x0008)==0) return kFALSE;else return kTRUE;}
    Bool_t StatusDisappeared() const {// checks if the part. is "disappeared"
	if((fStatus&0x00010)==0) return kFALSE;else return kTRUE;}
    Bool_t StatusStop() const {// checks if the particle is "stopped"
	if((fStatus&0x00020)==0) return kFALSE;else return kTRUE;}
    Bool_t StatusAlive() const {// checks if the particle is "ilcve"
	if((fStatus&0x00030)==0) return kFALSE;else return kTRUE;}
    // Prints out the content of this class in ASCII format.
    void Print(ostream *os) const; 
    // Reads in the content of this class in the format of Print
    void Read(istream *is);
    virtual void Print(Option_t *option="") const {TObject::Print(option);}
    virtual Int_t Read(const char *name) {return TObject::Read(name);}


 protected:
    Int_t     fStatus; // Track Status
    Int_t     fLayer;  // Layer number
    Int_t     fLadder; // Ladder number
    Int_t     fDet;    // Detector number  
    Float_t   fPx;     // PX of particle at the point of the hit
    Float_t   fPy;     // PY of particle at the point of the hit
    Float_t   fPz;     // PZ of particle at the point of the hit
    Float_t   fDestep; // Energy deposited in the current step
    Float_t   fTof;    // Time of flight at the point of the hit
    Int_t     fStatus0;// Track Status of Starting point
    Float_t   fx0;     // Starting point of this step
    Float_t   fy0;     // Starting point of this step
    Float_t   fz0;     // Starting point of this step
    Float_t   ft0;     // Starting point of this step

    ClassDef(IlcTARGEThit,2)  //Hits object for set:TARGET
	 
}; 
// Input and output function for standard C++ input/output.
ostream& operator<<(ostream &os,IlcTARGEThit &source);
istream& operator>>(istream &os,IlcTARGEThit &source);

#endif
