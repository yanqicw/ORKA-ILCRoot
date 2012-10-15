#ifndef ILCPVECEMCBADCHANNELSMAP
#define ILCPVECEMCBADCHANNELSMAP
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                          */

/* $Id: IlcPVECEmcBadChannelsMap.h 40123 2010-03-31 22:18:54Z kharlov $ */

// This class keeps the EMC bad channels map 
// (bad means dead or noisy).

#include "TObject.h"

class IlcPVECEmcBadChannelsMap : public TObject {

public:

  IlcPVECEmcBadChannelsMap();
  IlcPVECEmcBadChannelsMap(const IlcPVECEmcBadChannelsMap &map);
  IlcPVECEmcBadChannelsMap& operator= (const IlcPVECEmcBadChannelsMap &map);
  ~IlcPVECEmcBadChannelsMap() {}

  void  SetBadChannel(Int_t module, Int_t col, Int_t row);
  Bool_t IsBadChannel(Int_t module, Int_t col, Int_t row) const { return fBadChannelEmc[module-1][col-1][row-1]; }
  Int_t GetNumOfBadChannels() const {  return fBads; }
  void BadChannelIds(Int_t *badIds=0);
  void Reset();

private:
  
  Bool_t fBadChannelEmc[5][56][64]; //[mod][col][row]
  Int_t fBads;

  ClassDef(IlcPVECEmcBadChannelsMap,2)

};

#endif
