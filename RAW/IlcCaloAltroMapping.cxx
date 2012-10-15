/**************************************************************************
 * 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * Author: The International Lepton Collider Off-line Project.            *
 *                                                                        *
 * Part of the code has been developed by ILC Off-line Project.         *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
/* $Id: IlcCaloAltroMapping.cxx 42247 2010-07-12 14:06:37Z cvetan $ */

// This class handles the mapping of the Altro channels in the PHOS/EMCAL
// The mapping is read from an external mapping files
// Author: C.Cheshkov

/// Exported from PHOS to be used also by EMCAL
/// November 2006 Gustavo Conesa Balbastre

#include "IlcCaloAltroMapping.h"
#include "IlcLog.h"
#include <Riostream.h>
//#include <stdlib.h>


ClassImp(IlcCaloAltroMapping)

//_____________________________________________________________________________
IlcCaloAltroMapping::IlcCaloAltroMapping():
  IlcAltroMapping(),
  fMinRow(0),
  fMaxRow(0),
  fMinCol(0),
  fMaxCol(0),
  fInvMappingLow(NULL),
  fInvMappingHigh(NULL)
{
  // Default constructor
}

//_____________________________________________________________________________
IlcCaloAltroMapping::IlcCaloAltroMapping(const char *mappingFile):
  IlcAltroMapping(mappingFile),
  fMinRow(0),
  fMaxRow(0),
  fMinCol(0),
  fMaxCol(0),
  fInvMappingLow(NULL),
  fInvMappingHigh(NULL)
{
  // Constructor
  ReadMapping();
  CloseMappingFile();
}

//_____________________________________________________________________________
IlcCaloAltroMapping::~IlcCaloAltroMapping()
{
  // destructor
  // Deletes the arrays which have been
  // allocated during the reading of the
  // mapping file
  if (fInvMappingLow) delete [] fInvMappingLow;

  if (fInvMappingHigh) delete [] fInvMappingHigh;
}

//_____________________________________________________________________________
Bool_t IlcCaloAltroMapping::ReadMapping()
{
  // Initalizes the ALTRO mapping from a file
  // Look at the Calo module for the format of
  // the mapping file
  if (!fIn) {
    IlcFatal("Mapping file has not been opened !");
    return kFALSE;
  }

  fMinRow = 0x7fffffff;
  fMaxRow = 0;
  fMinCol = 0x7fffffff;
  fMaxCol = 0;
  fMappingSize = 3*(fMaxHWAddress+1);
  fMapping = new Short_t[fMappingSize];
  for (Int_t i = 0; i <= fMaxHWAddress; i++) {
    fMapping[3*i] = fMapping[3*i+1] = fMapping[3*i+2] = -1;
  }
 
  for(Int_t i = 0; i < fNumberOfChannels ; i++) { // 1792 = 2*896 channels connected to each RCU
    Int_t hwAddress;
    if (!(*fIn >> hwAddress)) {
      IlcFatal("Syntax of the mapping file is wrong !");
      return kFALSE;
    }
    if (hwAddress > fMaxHWAddress) {
      IlcFatal(Form("Hardware (ALTRO) adress (%d) outside the range (0 -> %d) !",hwAddress,fMaxHWAddress));
      return kFALSE;
    }
    Int_t row,col,caloFlag;
    if (!(*fIn >> row >> col >> caloFlag)) {
      IlcFatal("Syntax of the mapping file is wrong !");
      return kFALSE;
    }

    if (caloFlag < 0 || caloFlag > 3) {
      IlcFatal(Form("Wrong CaloFlag value found (%d)! Should be 0 ,1, 2 or 3 !",caloFlag));
      return kFALSE;
    }
 
    fMapping[3*hwAddress] = row;
    fMapping[3*hwAddress+1] = col;
    fMapping[3*hwAddress+2] = caloFlag;

    if (row > fMaxRow) fMaxRow = row;
    if (row < fMinRow) fMinRow = row;
    if (col > fMaxCol) fMaxCol = col;
    if (col < fMinCol) fMinCol = col;

  }

  return kTRUE;
}

//_____________________________________________________________________________
Bool_t IlcCaloAltroMapping::CreateInvMapping()
{
  // Create the inverse mapping
  // needed for the simulation of
  // raw data
  if (fInvMappingLow) return kTRUE;

  if (!fMapping) {
    IlcWarning("Mapping array was not initalized correctly ! Impossible to create the inverse mapping !");
    return kFALSE;
  }

  Int_t nRows = fMaxRow - fMinRow + 1;
  Int_t nCols = fMaxCol - fMinCol + 1;
  Int_t invMappingSize = nRows*nCols;

  fInvMappingLow  = new Short_t[invMappingSize];
  fInvMappingHigh = new Short_t[invMappingSize];
  for (Int_t i = 0; i < nRows; i++) {
    for (Int_t j = 0; j < nCols; j++) {
      fInvMappingLow[nCols*i+j]  = -1;
      fInvMappingHigh[nCols*i+j] = -1;
    }
  }
  for(Int_t i = 0; i <= fMaxHWAddress; i++) {
    Int_t row = fMapping[3*i];
    Int_t col = fMapping[3*i+1];
    Int_t caloFlag = fMapping[3*i+2];
    if(row != -1 && col != -1) {
      if (caloFlag == 0)
	fInvMappingLow[nCols*(row-fMinRow)+(col-fMinCol)] = i;
      if (caloFlag == 1)
	fInvMappingHigh[nCols*(row-fMinRow)+(col-fMinCol)] = i;
    }
  }

  return kTRUE;
}

//_____________________________________________________________________________
Int_t IlcCaloAltroMapping::GetHWAddress(Int_t row, Int_t col, Int_t caloFlag)
{
  // Get the content of the mapping array
  // return -1 in case there is no hardware
  // adress defined for these row-column-caloFlag
  if (!fInvMappingLow || !fInvMappingHigh) {
    if (!CreateInvMapping()) return -1;
  }
  if (row < fMinRow || row > fMaxRow) {
    IlcWarning(Form("Index of row (%d) outside the range (%d -> %d) !",row,fMinRow,fMaxRow));
    return -1;
  }
  if (col < fMinCol || col > fMaxCol) {
    IlcWarning(Form("Index of column (%d) outside the range (%d -> %d) !",col,fMinCol,fMaxCol));
    return -1;
  }
  if (caloFlag < 0 || caloFlag > 3) {
    IlcWarning(Form("Invalid caloFlag (%d)! Should be 0, 1, 2 or 3 !",caloFlag));
    return -1;
  }
  Int_t hwAddress = -1;
  if (caloFlag == 0)
    hwAddress = fInvMappingLow[(fMaxCol - fMinCol + 1)*(row-fMinRow)+(col-fMinCol)];
  if (caloFlag == 1)
    hwAddress = fInvMappingHigh[(fMaxCol - fMinCol + 1)*(row-fMinRow)+(col-fMinCol)];

  if (hwAddress == -1)
    IlcWarning(Form("Hardware (ALTRO) adress is not defined for these row (%d), column (%d) and caloFlag (%d) !",row,col,caloFlag));

  return hwAddress;
}

//_____________________________________________________________________________
Int_t IlcCaloAltroMapping::GetPadRow(Int_t hwAddress) const
{
  // Return the row index
  // Note the difference w.r.t to the base class notation
  if (!fMapping) {
    IlcWarning("Mapping array was not initalized correctly !");
    return -1;
  }
  if (hwAddress > fMaxHWAddress) {
    IlcWarning(Form("Hardware (ALTRO) adress (%d) outside the range (0 -> %d) !",hwAddress,fMaxHWAddress));
    return -1;
  }
  Int_t row = fMapping[3*hwAddress];
  if (row == -1)
    IlcWarning(Form("Hardware (ALTRO) adress (%d) is not defined !",hwAddress));

  return row;
}

//_____________________________________________________________________________
Int_t IlcCaloAltroMapping::GetPad(Int_t hwAddress) const
{
  // Return the column index
  // Note the difference w.r.t to the base class notation
  if (!fMapping) {
    IlcWarning("Mapping array was not initalized correctly !");
    return -1;
  }
  if (hwAddress > fMaxHWAddress) {
    IlcWarning(Form("Hardware (ALTRO) adress (%d) outside the range (0 -> %d) !",hwAddress,fMaxHWAddress));
    return -1;
  }
  Int_t col = fMapping[3*hwAddress+1];
  if (col == -1)
    IlcWarning(Form("Hardware (ALTRO) adress (%d) is not defined !",hwAddress));

  return col;
}

//_____________________________________________________________________________
Int_t IlcCaloAltroMapping::GetSector(Int_t hwAddress) const
{
  // Return the caloFlag factor (0/1)
  // Note the difference w.r.t to the base class notation
  if (!fMapping) {
    IlcWarning("Mapping array was not initalized correctly !");
    return -1;
  }
  if (hwAddress > fMaxHWAddress) {
    IlcWarning(Form("Hardware (ALTRO) adress (%d) outside the range (0 -> %d) !",hwAddress,fMaxHWAddress));
    return -1;
  }
  Int_t caloFlag = fMapping[3*hwAddress+2];
  if (caloFlag == -1)
    IlcWarning(Form("Hardware (ALTRO) adress (%d) is not defined !",hwAddress));

  return caloFlag;
}
