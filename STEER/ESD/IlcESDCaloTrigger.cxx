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

/*
 

 

Author: R. GUERNANE LPSC Grenoble CNRS/IN2P3
*/

#include "IlcESDCaloTrigger.h"
#include "IlcLog.h"

#include "TArrayI.h"
#include "Riostream.h"
#include <cstdlib>

ClassImp(IlcESDCaloTrigger)

//_______________
IlcESDCaloTrigger::IlcESDCaloTrigger() : TNamed(),
fNEntries(0),
fCurrent(-1),
fColumn(0x0),
fRow(0x0),
fAmplitude(0x0),
fTime(0x0),
fNL0Times(0x0),
fL0Times(new TArrayI()),
fL1TimeSum(0x0),
fTriggerBits(0x0),
fL1Threshold(),
fL1V0(),
fL1FrameMask(0)
{
	//
	for (int i = 0; i < 4; i++) fL1Threshold[i] = 0;
	fL1V0[0] = fL1V0[1] = 0;	
}

//_______________
IlcESDCaloTrigger::IlcESDCaloTrigger(const IlcESDCaloTrigger& src) : TNamed(src),
fNEntries(0),
fCurrent(-1),
fColumn(0x0),
fRow(0x0),
fAmplitude(0x0),
fTime(0x0),
fNL0Times(0x0),
fL0Times(new TArrayI()),
fL1TimeSum(0x0),
fTriggerBits(0x0),
fL1Threshold(),
fL1V0(),
fL1FrameMask(0)
{
	//
	src.Copy(*this);
}

//_______________
IlcESDCaloTrigger::~IlcESDCaloTrigger()
{
	//
	if (fNEntries) DeAllocate();
	
	delete fL0Times; fL0Times = 0x0;
}

//_______________
void IlcESDCaloTrigger::DeAllocate()
{
	//
	delete [] fColumn;      fColumn    = 0x0;
	delete [] fRow;         fRow       = 0x0;     
	delete [] fAmplitude;   fAmplitude = 0x0;
	delete [] fTime;        fTime      = 0x0;   
	delete [] fNL0Times;    fNL0Times  = 0x0;
	delete [] fL1TimeSum;   fL1TimeSum = 0x0;
	delete [] fTriggerBits; fTriggerBits   = 0x0;

	fNEntries =  0;
	fCurrent  = -1;

	fL0Times->Reset();
}

//_______________
IlcESDCaloTrigger& IlcESDCaloTrigger::operator=(const IlcESDCaloTrigger& src)
{
	//
	if (this != &src) src.Copy(*this);
	
	return *this;
}

//_______________
void IlcESDCaloTrigger::Copy(TObject &obj) const 
{	
	//
	TNamed::Copy(obj);
	
	IlcESDCaloTrigger& dest = static_cast<IlcESDCaloTrigger&>(obj);

	if (dest.fNEntries) dest.DeAllocate();
	
	dest.Allocate(fNEntries);
	
	for (Int_t i = 0; i < fNEntries; i++)
	{
		Int_t times[10];
		for (Int_t j = 0; j < 10; j++) times[j] = fL0Times->At(10 * i + j);
	  
		dest.Add(fColumn[i], fRow[i], fAmplitude[i], fTime[i], times, fNL0Times[i], fL1TimeSum[i], fTriggerBits[i]);
	}	

	for (int i = 0; i < 4; i++) dest.SetL1Threshold(i, fL1Threshold[i]);
	
	dest.SetL1V0(fL1V0);
	dest.SetL1FrameMask(fL1FrameMask);
}

//_______________
void IlcESDCaloTrigger::Allocate(Int_t size)
{
	//
	if (!size) return;
	
	fNEntries = size;
	
	fColumn      = new   Int_t[fNEntries];
	fRow         = new   Int_t[fNEntries];
	fAmplitude   = new Float_t[fNEntries];
	fTime        = new Float_t[fNEntries];
	fNL0Times    = new   Int_t[fNEntries];
	fL1TimeSum   = new   Int_t[fNEntries];
	fTriggerBits = new   Int_t[fNEntries];

	for (Int_t i = 0; i < fNEntries; i++) 
	{
	  fColumn[i]      = 0;
	  fRow[i]         = 0;
	  fAmplitude[i]   = 0;
	  fTime[i]        = 0;
	  fNL0Times[i]    = 0;
	  fL1TimeSum[i]   = 0;
	  fTriggerBits[i] = 0;
	}
	
	fL0Times->Set(fNEntries * 10);
}

//_______________
Bool_t IlcESDCaloTrigger::Add(Int_t col, Int_t row, Float_t amp, Float_t time, Int_t trgtimes[], Int_t ntrgtimes, Int_t trgts, Int_t trgbits)
{
	//
	fCurrent++;
	
	     fColumn[fCurrent] = col;
	        fRow[fCurrent] = row;
	  fAmplitude[fCurrent] = amp;
	       fTime[fCurrent] = time;
	   fNL0Times[fCurrent] = ntrgtimes;
	  fL1TimeSum[fCurrent] = trgts;	
	fTriggerBits[fCurrent] = trgbits;
	
	if (ntrgtimes > 9) 
	{
		IlcError("Should not have more than 10 L0 times");
		return kFALSE;
	}
	
	for (Int_t i = 0; i < fNL0Times[fCurrent]; i++) fL0Times->AddAt(trgtimes[i], 10 * fCurrent + i);

	return kTRUE;
}

//_______________
Bool_t IlcESDCaloTrigger::Next()
{
	//
	if (fCurrent >= fNEntries - 1 || !fNEntries) return kFALSE;
	
	fCurrent++;
	
	return kTRUE;
}

//_______________
void IlcESDCaloTrigger::GetPosition(Int_t& col, Int_t& row) const
{
	//
	if (fCurrent == -1) return;
	
	col = fColumn?fColumn[fCurrent]:0;
	row =    fRow?fRow[fCurrent]:0;
}

//_______________
void IlcESDCaloTrigger::GetAmplitude(Float_t& amp) const
{
	//
	if (fCurrent == -1) return;

	amp = fAmplitude?fAmplitude[fCurrent]:0;
}

//_______________
void IlcESDCaloTrigger::GetTime(Float_t& time) const
{
	//
	if (fCurrent == -1) return;

	time = fTime?fTime[fCurrent]:0;
}

//_______________
void IlcESDCaloTrigger::GetL1TimeSum(Int_t& amp) const
{
	//	
	if (fCurrent == -1) return;

	amp = fL1TimeSum?fL1TimeSum[fCurrent]:0;
}

//_______________
void IlcESDCaloTrigger::GetNL0Times(Int_t& ntimes) const
{
	//
	if (fCurrent == -1) return;

	ntimes = fNL0Times?fNL0Times[fCurrent]:0;
}

//_______________
void IlcESDCaloTrigger::GetTriggerBits(Int_t& bits) const
{
	//
	if (fCurrent == -1) return;

	bits = fTriggerBits?fTriggerBits[fCurrent]:0;
}

//_______________
void IlcESDCaloTrigger::GetL0Times(Int_t times[]) const
{
	//
	if (fCurrent == -1) return;
	
	if (fNL0Times && fL0Times) {
	  for (Int_t i = 0; i < fNL0Times[fCurrent]; i++) times[i] = fL0Times->At(10 * fCurrent + i);
	}
}

//_______________
void IlcESDCaloTrigger::Print(const Option_t* /*opt*/) const
{
	//
	if (fCurrent == -1) return;
	if (!fColumn) return;
	if (!fRow) return;
	if (!fAmplitude) return;
	if (!fTime) return;
	if (!fNL0Times) return;
	if (!fL0Times) return;
	if (!fL1TimeSum) return;
	if (!fTriggerBits) return;

	printf("============\n");
	printf("--L0:\n");
	printf("\tPOSITION (X: %2d Y: %2d) / FITTED F-ALTRO (AMP: %4f TIME: %3f)\n", 
		   fColumn[fCurrent], fRow[fCurrent], fAmplitude[fCurrent], fTime[fCurrent]);
	printf("\t%d L0 TIMES (", fNL0Times[fCurrent]); 
	for (Int_t i = 0; i < fNL0Times[fCurrent]; i++) printf("%2d ",fL0Times->At(10 * fCurrent + i));
	printf(")\n");
	printf("--L1:\n");
	printf("\tTIME SUM: %4d\n", fL1TimeSum[fCurrent]);
	printf("\tHIGH THRESHOLDS (GAMMA: %4d, JET: %4d)\n", fL1Threshold[0], fL1Threshold[1]);
	printf("\tLOW THRESHOLDS (GAMMA: %4d, JET: %4d)\n", fL1Threshold[2], fL1Threshold[3]);
	printf("--TRIGGER BITS: 0x%x\n", fTriggerBits[fCurrent]);
}	
