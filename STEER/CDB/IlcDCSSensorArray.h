#ifndef IlcDCSSensorArray_H
#define IlcDCSSensorArray_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Calibration class for DCS sensors                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "TNamed.h"
#include "TMap.h"
#include "TObjString.h"
#include "IlcCDBManager.h"
#include "IlcCDBEntry.h"
#include "IlcDCSValue.h"
#include "IlcDCSSensor.h"

class TGraph;
class TTimeStamp;


class IlcDCSSensorArray : public TNamed {
 public:
  IlcDCSSensorArray();
  IlcDCSSensorArray(TClonesArray *arr);
  IlcDCSSensorArray(Int_t run, const char* dbEntry);
  IlcDCSSensorArray(UInt_t startTime, UInt_t endTime, TTree* confTree);
  IlcDCSSensorArray(UInt_t startTime, UInt_t endTime, TClonesArray *sensors);
  IlcDCSSensorArray(const IlcDCSSensorArray &c);
  virtual ~IlcDCSSensorArray();
  IlcDCSSensorArray &operator=(const IlcDCSSensorArray &c);
  void SetStartTime (const TTimeStamp& start) { fStartTime = start; }
  void SetEndTime   (const TTimeStamp& end) { fEndTime = end; }
  TTimeStamp GetStartTime () const { return fStartTime; }
  TTimeStamp GetEndTime () const { return fEndTime; }
  void  SetMinGraph(const Int_t minGraph) { fMinGraph=minGraph;}
  Int_t GetMinGraph()  const { return fMinGraph; }
  void  SetMinPoints(const Int_t minPoints) { fMinPoints=minPoints;}
  Int_t GetMinPoints() const { return fMinPoints; }
  void  SetIter(const Int_t iter ) { fIter=iter; }
  Int_t GetIter() const { return fIter; }
  void  SetMaxDelta(const Double_t maxDelta) { fMaxDelta= maxDelta;}
  Double_t GetMaxDelta() const { return fMaxDelta; }
  void  SetFitReq(const Int_t fitReq) { fFitReq=fitReq; }
  Int_t GetFitReq() const {return fFitReq;}
  void  SetValCut(const Int_t valCut) { fValCut=valCut;}
  Int_t GetValCut() const {return fValCut;}
  void  SetDiffCut(const Int_t diffCut) { fDiffCut=diffCut; }
  Int_t GetDiffCut() const {return fDiffCut;}

  void SetGraph     (TMap *map);
  void MakeSplineFit(TMap *map, Bool_t keepMap=kFALSE);
  void MakeSplineFitAddPoints(TMap *map);
  TMap* ExtractDCS  (TMap *dcsMap, Bool_t keepStart=kFALSE);
  TGraph* MakeGraph (TObjArray *valueSet, Bool_t keepStart=kFALSE);
  void ClearGraph();
  void ClearFit();
  Double_t GetValue  (UInt_t timeSec, Int_t sensor);
  IlcDCSSensor* GetSensor (Int_t IdDCS);
  IlcDCSSensor* GetSensor (Double_t x, Double_t y, Double_t z);
  IlcDCSSensor* GetSensor (const TString& stringID);
  IlcDCSSensor* GetSensorNum (Int_t ind);
  Int_t SetSensor (const TString& stringID, const IlcDCSSensor& sensor);
  void SetSensorNum (const Int_t ind, const IlcDCSSensor& sensor);
  void RemoveSensorNum(Int_t ind);
  void RemoveSensor(Int_t IdDCS);
  void AddSensors(IlcDCSSensorArray *sensors);
  void RemoveGraphDuplicates (Double_t tolerance=1e-6);
  TArrayI OutsideThreshold(Double_t threshold, UInt_t timeSec=0, Bool_t below=kTRUE) const;
  Int_t NumSensors() const { return fSensors->GetEntries(); }
  Int_t NumFits() const;

  Int_t GetFirstIdDCS() const;
  Int_t GetLastIdDCS()  const;


 protected:
  Int_t  fMinGraph;              // minimum #points of graph to be fitted
  Int_t  fMinPoints;             // minimum number of points per knot in fit
  Int_t  fIter;                  // number of iterations for spline fit
  Double_t  fMaxDelta;           // precision parameter for spline fit
  Int_t  fFitReq;                // fit requirement, 2 = continuous 2nd derivative
  Int_t  fValCut;                // cut on absolute value read from DCS map
  Int_t  fDiffCut;               // cut on difference in value read from DCS map
  TTimeStamp  fStartTime;        // start time for measurements in this entry
  TTimeStamp  fEndTime;          // end time for measurements in this entry
  TClonesArray *fSensors;        // Array of sensors



  ClassDef(IlcDCSSensorArray,3)       //  TPC calibration class for parameters which are saved per pad

};

#endif
