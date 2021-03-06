#ifndef ILCTARGETVERTEXERPPZ_H
#define ILCTARGETVERTEXERPPZ_H
/* Copyright(c) 2005-2006, ILC Project Experiment, All rights reserved.   *
 * See cxx source for full Copyright notice                               */


#include <IlcTARGETVertexer.h>

/////////////////////////////////////////////////////////////////////
//                                                                 //
// Class for primary vertex Z coordinate reconstruction            //
// Optimized for p-p events (in general: low multiplicity events)  //
//                                                                 //
/////////////////////////////////////////////////////////////////////

class TH1F; 
class TArrayF;

class IlcTARGETVertexerPPZ : public IlcTARGETVertexer {

 public:
  IlcTARGETVertexerPPZ();
  IlcTARGETVertexerPPZ(TString fn, Float_t x0=0., Float_t y0=0.);  // standard constructor 
  virtual ~IlcTARGETVertexerPPZ(); // destructor
  virtual IlcESDVertex* FindVertexForCurrentEvent(Int_t event);

#ifdef WIN32
  virtual IlcESDVertex* FindVertexForCurrentEvent(TTree*);
#else
  virtual IlcESDVertex* FindVertexForCurrentEvent(TTree*){ Fatal("FindVertexForCurrentEvent","FindVertexForCurrentEvent(TTree*) not yet implemented\n");}
#endif

  virtual void FindVertices();
  virtual Float_t GetZFound() const {return fZFound;}
  virtual Float_t GetZsig() const {return fZsig;}
  virtual void PrintStatus() const;
  virtual void SetDiffPhiMax(Float_t pm = 0.05){fDiffPhiMax = pm;}
  virtual void SetFirstLayerModules(Int_t m1 = 0, Int_t m2 = 79){fFirstL1 = m1; fLastL1 = m2;}
  virtual void SetSecondLayerModules(Int_t m1 = 80, Int_t m2 = 239){fFirstL2 = m1; fLastL2 = m2;}
  virtual void SetWindow(Float_t w=3.){fWindow = w;}
  static Float_t Curv(Double_t x1,Double_t y1, Double_t x2,Double_t y2,
		       Double_t x3,Double_t y3); 


 protected:
  Int_t fFirstL1;          // first module of the first pixel layer
  Int_t fLastL1;           // last module of the first pixel layer
  Int_t fFirstL2;          // first module of the second pixel layer
  Int_t fLastL2;           // last module of the second pixel layer
  Float_t fDiffPhiMax;     // Maximum delta phi allowed among corr. pixels
  Float_t fX0;             // Nominal x coordinate of the vertex
  Float_t fY0;             // Nominal y coordinate of the vertex
  //IlcTARGET *fTARGET;            //! pointer to the IlcTARGET object
  Float_t fZFound;         //! found value for the current event
  Float_t fZsig;           //! RMS of Z
  Float_t fWindow;         // window width for Z search in mm (3 mm by def.)

 private:
  void EvalZ(TH1F *hist,Int_t sepa, Int_t ncoinc, TArrayF *zval);

  ClassDef(IlcTARGETVertexerPPZ,3);
};

#endif
