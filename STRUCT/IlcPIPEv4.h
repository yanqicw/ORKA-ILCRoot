#ifndef ILCPIPEV4_H
#define ILCPIPEV4_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

//-------------------------------------------------------------------------
//  Beam pipe class for ILC MFT upgrade
//  This version uses TGeo
//  Authors:
//  F. Manso 
//  A. Morsch
//-------------------------------------------------------------------------

 
#include "IlcPIPE.h"
class TGeoPcon;
class TGeoVolume;


class IlcPIPEv4 : public IlcPIPE {
    
 public:
    enum constants {kC=6, kAlu=9, kInox=19, kGetter=20, kBe=5, kVac=16,
	  kAir=15, kAlBe=21, kPA = 22};
	
  IlcPIPEv4();
  IlcPIPEv4(const char *name, const char *title);
  IlcPIPEv4(const char *name, const char *title, const Float_t rmax, const Float_t epaisseur);
  IlcPIPEv4(const char *name, const char *title, const Float_t rmax, const Float_t epaisseur, const Float_t z2);
  virtual       ~IlcPIPEv4() {};
  virtual void   CreateGeometry();
  virtual void   CreateMaterials();
  virtual Int_t  IsVersion() const {return 0;}
 private:
  virtual TGeoPcon*   MakeMotherFromTemplate(TGeoPcon* shape, Int_t imin = -1, Int_t imax = -1, Float_t r0 = 0., Int_t nz =-1);
  virtual TGeoPcon*   MakeInsulationFromTemplate(TGeoPcon* shape);
  virtual TGeoVolume* MakeBellow(const char* ext, Int_t nc, Float_t rMin, Float_t rMax, Float_t dU, Float_t rPlie, Float_t dPlie);

  Float_t   fRmax;       // outer radius of Be beam pipe
  Float_t   fBe;         // width of Be beam pipe
  Float_t   fZ1;         // beginning of beam pipe z location (A side)
  Float_t   fZ2;         // end of Be beam pipe z location (C side)
  Float_t   fZ3;         // end of Stell beam pipe z location (C side)
 protected:
  ClassDef(IlcPIPEv4,1)  //Class for PIPE version using TGeo
};
 
#endif
