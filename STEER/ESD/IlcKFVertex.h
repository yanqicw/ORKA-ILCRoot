//---------------------------------------------------------------------------------
// The IlcKFVertex class
// .
// @author  S.Gorbunov, I.Kisel
// @version 1.0
// @since   13.05.07
// 
// Class to reconstruct and store primary and secondary vertices.
// The method is described in CBM-SOFT note 2007-003, 
// ``Reconstruction of decayed particles based on the Kalman filter'', 
// http://www.gsi.de/documents/DOC-2007-May-14-1.pdf
//
// This class is ILC interface to general mathematics in IlcKFParticleBase
// 
//  -= Copyright &copy ILC HLT Group =-
//_________________________________________________________________________________

#ifndef ILCKFVERTEX_H
#define ILCKFVERTEX_H

#include "IlcKFParticle.h"
#include "IlcVVertex.h"

class IlcKFVertex : public IlcKFParticle
{
  
 public:

  //*
  //*  INITIALIZATION
  //*

  //* Constructor (empty)

  IlcKFVertex():IlcKFParticle(),fIsConstrained(0){ } 

  //* Destructor (empty)

  ~IlcKFVertex(){}

  //* Initialisation from VVertex 

  IlcKFVertex( const IlcVVertex &vertex );


  //*
  //*  ACCESSORS
  //*

  //* Number of tracks composing the vertex

  Int_t GetNContributors() const { return fIsConstrained ?fNDF/2:(fNDF+3)/2; }

  //* 
  //* CONSTRUCTION OF THE VERTEX BY ITS DAUGHTERS 
  //* USING THE KALMAN FILTER METHOD
  //*


  //* Simple way to construct vertices ex. D0 = Pion + Kaon;   

  void operator +=( const IlcKFParticle &Daughter );  

  //* Subtract particle from vertex

  IlcKFVertex operator -( const IlcKFParticle &Daughter ) const;

  void operator -=( const IlcKFParticle &Daughter );  

  //* Set beam constraint to the primary vertex

  void SetBeamConstraint( Double_t X, Double_t Y, Double_t Z, 
			  Double_t ErrX, Double_t ErrY, Double_t ErrZ );

  //* Set beam constraint off

  void SetBeamConstraintOff();

  //* Construct vertex with selection of tracks (primary vertex)

  void ConstructPrimaryVertex( const IlcKFParticle *vDaughters[], int NDaughters,
			       Bool_t vtxFlag[], Double_t ChiCut=3.5  );

 protected:

  Bool_t fIsConstrained; // Is the beam constraint set

  ClassDef( IlcKFVertex, 1 );

};


//---------------------------------------------------------------------
//
//     Inline implementation of the IlcKFVertex methods
//
//---------------------------------------------------------------------


inline void IlcKFVertex::operator +=( const IlcKFParticle &Daughter )
{
  IlcKFParticle::operator +=( Daughter );
}
  

inline void IlcKFVertex::operator -=( const IlcKFParticle &Daughter )
{
  Daughter.SubtractFromVertex( *this );
}
  
inline IlcKFVertex IlcKFVertex::operator -( const IlcKFParticle &Daughter ) const 
{
  IlcKFVertex tmp = *this;
  Daughter.SubtractFromVertex( tmp );
  return tmp;
}


#endif 
