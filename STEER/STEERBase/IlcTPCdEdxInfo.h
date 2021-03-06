#ifndef IlcTPCdEdxInfo_H
#define IlcTPCdEdxInfo_H

#include <TObject.h>

class IlcTPCdEdxInfo : public TObject 
{
public:
  IlcTPCdEdxInfo();
  IlcTPCdEdxInfo(const IlcTPCdEdxInfo& source);
  IlcTPCdEdxInfo& operator=(const IlcTPCdEdxInfo& source);
  //
  void     GetTPCSignalRegionInfo(Double32_t signal[4], Char_t ncl[3], Char_t nrows[3]) const;
  void     SetTPCSignalRegionInfo(Double32_t signal[4], Char_t ncl[3], Char_t nrows[3]);
  
private:

  Double32_t  fTPCsignalRegion[4]; //[0.,0.,10] TPC dEdx signal in 4 different regions - 0 - IROC, 1- OROC medium, 2 - OROC long, 3- OROC all, (default truncation used)  
  Char_t      fTPCsignalNRegion[3]; // number of clusters above threshold used in the dEdx calculation
  Char_t      fTPCsignalNRowRegion[3]; // number of crosed rows used in the dEdx calculation - signal below threshold included

  
  ClassDef(IlcTPCdEdxInfo,2)
};

#endif
