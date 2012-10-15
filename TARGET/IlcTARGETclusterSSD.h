#ifndef ILCTARGETCLUSTERSSD_H
#define ILCTARGETCLUSTERSSD_H
/* Copyright(c) 2005-2006, ILC Project Experiment, All rights reserved.   *
 * See cxx source for full Copyright notice                               */

/*
  $Id: IlcTARGETclusterSSD.h,v 1.1.1.1 2008/03/11 14:52:50 vitomeg Exp $
 */

    //Piotr Krzysztof Skowronski
    //Warsaw University of Technology
    //skowron@if.pw.edu.pl
    //
    // Describes set of neighbouring digits on one side of detector,
    // that are supposed to be generated by one particle.
    // includes information about clusters on other side that it crosses with
#include "TObject.h"

class TArrayI;
class TConesArray;
class IlcTARGETdigitSSD;

class IlcTARGETclusterSSD : public TObject{
 public:
    IlcTARGETclusterSSD();    //Default Constructor
    virtual ~IlcTARGETclusterSSD();
    IlcTARGETclusterSSD(Int_t ndigits, Int_t *DigitIndexes,
		     TObjArray *Digits, Bool_t side);		
    IlcTARGETclusterSSD(const IlcTARGETclusterSSD &source); 
    IlcTARGETclusterSSD& operator=( const IlcTARGETclusterSSD & source);
    void AddDigit(Int_t index){//adds on digit
	(*fDigitsIndex)[fNDigits++]=index;} 
    TObjArray* GetPointer2Digits(){return fDigits;}// comment to be written
    void SetPointer2Digits(TObjArray *digits){// comment to be written
	fDigits = digits;}
    Int_t GetNumOfDigits() const {//Returns number of digits that creates this cluster
	return fNDigits;}
    Int_t GetDigitSignal(Int_t digit);
    IlcTARGETdigitSSD *GetDigit(Int_t idx) { // comment to be written
	return (IlcTARGETdigitSSD *)((*fDigits)[GetDigitIndex(idx)]);}
    Int_t GetDigitIndex (Int_t digit) const {// comment to be written
	return (*fDigitsIndex)[digit];}
    Int_t GetDigitStripNo(Int_t digit);
    // comment to be written
    Int_t GetFirstDigitStripNo(){return GetDigitStripNo(0);}
    Int_t GetLastDigitStripNo(){// comment to be written
	return GetDigitStripNo(fNDigits-1);}
     //splits this one side cluster for two
    Int_t SplitCluster(Int_t where,Int_t *outdigits);
    void AddCross(Int_t clIndex);  //Add index of cluster that it crosses with
     //return index of cluster that it crosses with
    Int_t GetCross(Int_t crIndex) const ;
    Int_t GetCrossNo()  const {// Returns number of crosses
	return fNCrosses;}
    void DelCross(Int_t index);
    Double_t GetPosition();
    Double_t GetPositionError() const;
    Float_t GetTotalSignal();  
    Float_t GetTotalSignalError();
    void CutTotalSignal(Float_t sx) {// comment to be written
	if (fTotalSignal>sx) fTotalSignal-=sx;}
    Bool_t GetRightNeighbour() const {// comment to be written
	return fRightNeighbour;}
    Bool_t GetLeftNeighbour() const {// comment to be written
	return fLeftNeighbour;}
    void SetRightNeighbour(Bool_t nei) {// comment to be written
	fRightNeighbour=nei;}
    //comment to be written
    void SetLeftNeighbour(Bool_t nei){fLeftNeighbour=nei;}
    void SetNTracks(Int_t ntracks) {fNTracks=ntracks;}// set ntracks
    Int_t GetNTracks() const {return fNTracks;}// comment to be written
    Bool_t GetSide() const {return fSide;}// comment to be written
    Int_t CheckSatus(Int_t *) const {return 0;}//check if dig's comes from same track
    Int_t *GetTracks(Int_t &nt);
    void Consume(){// comment
	fConsumed = kTRUE;}
    Bool_t IsConsumed() const{// comment
	return fConsumed;}
    Bool_t IsCrossingWith(Int_t idx) const;
 protected:
    Bool_t    fSide;        //True if P
    TObjArray *fDigits;     //Pointer to List of Digitsbelonging to IlcTARGET
    Int_t     fNDigits;     //Number of digits that make up that cluster 
    TArrayI  *fDigitsIndex; // Digit indexes in fDigits array
    Int_t    fNCrosses;     //Number of crossed one side clusters on the
                            // other side (max 8 : it is not aribtrary)
    TArrayI *fCrossedClusterIndexes; //Array of inedexes of crossed clusters
                                     // (in TClonesArray)
                                     // not in TArrayI in package
    Bool_t   fLeftNeighbour;   // comment to be written
    Bool_t   fRightNeighbour;  // comment to be written 
    Bool_t   fConsumed;        // comment to be written
    Float_t  fTotalSignal;     // comment to be written
    Int_t    fTrack[10];       // comment to be written
    Int_t    fNTracks;         // comment to be written
    
    Double_t CentrOfGravity();
    Double_t EtaAlgorithm();
  ClassDef(IlcTARGETclusterSSD,1) // Cluster class for reconstructing SSD
};

#endif
