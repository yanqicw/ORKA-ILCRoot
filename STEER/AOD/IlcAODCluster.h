#ifndef IlcAODCluster_H
#define IlcAODCluster_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* $Id: IlcAODCluster.h 50615 2011-07-16 23:19:19Z hristov $ */

//-------------------------------------------------------------------------
//     AOD cluster base class
//     Author: Markus Oldenburg, CERN
//-------------------------------------------------------------------------

#include <IlcVCluster.h>

class IlcAODCluster : public IlcVCluster {

 public:
  
  IlcAODCluster();
  IlcAODCluster(Int_t id,
		UInt_t nLabel,
		Int_t *label,
		Double_t energy,
		Double_t x[3],
		Double_t pid[13],
		Char_t ttype=kUndef,
		UInt_t selectInfo=0);

   IlcAODCluster(Int_t id,
		 UInt_t nLabel,
		 Int_t *label,
		 Float_t energy,
		 Float_t x[3],
		 Float_t pid[13],
		 Char_t ttype=kUndef,
		 UInt_t selectInfo=0);
   
  virtual ~IlcAODCluster();
  IlcAODCluster(const IlcAODCluster& clus); 
  IlcAODCluster& operator=(const IlcAODCluster& clus);
  void Clear(const Option_t*);
  
  Double_t Chi2() const { return fChi2; }
  
  Double_t E() const { return fEnergy; }
  
  // PID
  
  UShort_t  GetMostProbablePID() const;
  const Double_t *GetPID() const { return fPID; }//{ for(Int_t i=0; i<13; ++i) pid[i]=fPID[i]; }
  Int_t     GetID()  const { return fID; }
  Int_t     GetLabel() const   {
    if( fLabel &&  fNLabel > 0)  return  fLabel[0]; 
    else return -1;} //Most likely the track associated to the cluster
  Int_t  GetLabelAt(UInt_t i) const;
  Int_t  * GetLabels() const {return fLabel ; }
  UInt_t GetNLabels() const { return (UInt_t)fNLabel; }
  Bool_t TestFilterBit(UInt_t filterBit) const { return (Bool_t) ((filterBit & fFilterMap) != 0); }
  Char_t GetType() const { return fType; }
  
  void GetPosition(Float_t *x) const {
    x[0]=fPosition[0]; x[1]=fPosition[1]; x[2]=fPosition[2];}
  
  Bool_t IsEMCAL() const {if(fType == kEMCALClusterv1) return kTRUE;
    else return kFALSE;}
  Bool_t IsPHOS() const {if(fType == kPHOSCharged || fType == kPHOSNeutral) return kTRUE;
    else return kFALSE;}
  
  // print
  void  Print(const Option_t *opt = "") const;
  
  // setters
  void SetE(Double32_t energy) {fEnergy = energy ; }
  void SetID(Int_t id) { fID = id; }
  void SetType(Char_t ttype) { fType=ttype; }
  void SetLabel(Int_t *label, UInt_t size);  
  void SetChi2(Double_t chi2) { fChi2 = chi2; }
  
  void SetPosition(Float_t *x);
  void SetPositionAt(Float_t x,Int_t i) { if(i>=0 && i<3) fPosition[i] = x ; 
    else printf("Bad index for position array, i = %d\n",i);}
  
  void SetPIDAt(Float_t x,Int_t i) { if(i>=0 && i<13) fPID[i] = x ; 
    else printf("Bad index for PID array, i = %d\n",i);}
  void SetPID(const Float_t *pid) {
    if(pid) for(Int_t i=0; i<13; ++i) fPID[i]=pid[i];
    else {for(Int_t i=0; i<13; fPID[i++]=0) ;} fPID[IlcAODCluster::kUnknown]=1.;}
  template <class T> void SetPIDFromESD(const T *pid) {
    if(pid) {for(Int_t i=0; i<11; ++i) fPID[i]=pid[i];  fPID[11]=0;   fPID[12]=0;}
    else {for(Int_t i=0; i<13; fPID[i++]=0) ;} fPID[IlcAODCluster::kUnknown]=1.;}
  
  void RemoveLabel();
  
  
 private :
  
  // Energy & position
  Double32_t    fEnergy;         // energy
  Double32_t    fPosition[3];    // position of the cluster
  
  Double32_t    fChi2;           // chi2 (probably not necessary for PMD)
  Double32_t    fPID[13];         // [0.,1.,8] pointer to PID object
  
  Int_t         fID;             // unique cluster ID, points back to the ESD cluster
  Int_t         fNLabel;         // number of original track for this cluster      
  Int_t        *fLabel;          // [fNLabel] particle label, points back to MC tracks
  UInt_t        fFilterMap;      // filter information, one bit per set of cuts
  
  Char_t        fType;           // cluster type
  
  ClassDef(IlcAODCluster,6);
};

#endif
