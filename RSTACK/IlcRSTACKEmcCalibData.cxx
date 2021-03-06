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

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// class for RSTACK EmCal calibration                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "IlcRSTACKEmcCalibData.h"

ClassImp(IlcRSTACKEmcCalibData)

//________________________________________________________________
IlcRSTACKEmcCalibData::IlcRSTACKEmcCalibData():
  TNamed(),
  fSampleTimeStep(0)
{
  // Default constructor
  Reset();
}

//________________________________________________________________
IlcRSTACKEmcCalibData::IlcRSTACKEmcCalibData(const char* name):
  TNamed(),
  fSampleTimeStep(0)
{
  // Constructor
  TString namst = "Calib_";
  namst += name;
  SetName(namst.Data());
  SetTitle(namst.Data());
  Reset();
}

//________________________________________________________________
IlcRSTACKEmcCalibData::IlcRSTACKEmcCalibData(const IlcRSTACKEmcCalibData& calibda) :
  TNamed(calibda),
  fSampleTimeStep(calibda.fSampleTimeStep)
{
  // copy constructor
  SetName(calibda.GetName());
  SetTitle(calibda.GetName());

  for(Int_t module=0; module<5; module++) {
    for(Int_t column=0; column<56; column++) {
      for(Int_t row=0; row<64; row++) {
	fADCchannelEmc[module][column][row] = calibda.fADCchannelEmc[module][column][row];
	fADCpedestalEmc[module][column][row] = calibda.fADCpedestalEmc[module][column][row];
	fHighLowRatioEmc[module][column][row] = calibda.fHighLowRatioEmc[module][column][row];
	fTimeShiftEmc[module][column][row] = calibda.fTimeShiftEmc[module][column][row];
        fAltroOffsets[module][column][row] = calibda.fAltroOffsets[module][column][row];
	fDecal[module][column][row]        = calibda.fDecal[module][column][row];
      }
    }
  }
}

//________________________________________________________________
IlcRSTACKEmcCalibData &IlcRSTACKEmcCalibData::operator =(const IlcRSTACKEmcCalibData& calibda)
{
  // assignment operator

  if(this != &calibda) { 

    SetName(calibda.GetName());
    SetTitle(calibda.GetName());

    for(Int_t module=0; module<5; module++) {
      for(Int_t column=0; column<56; column++) {
	for(Int_t row=0; row<64; row++) {
	  fADCchannelEmc[module][column][row] = calibda.fADCchannelEmc[module][column][row];
	  fADCpedestalEmc[module][column][row] = calibda.fADCpedestalEmc[module][column][row];
	  fHighLowRatioEmc[module][column][row] = calibda.fHighLowRatioEmc[module][column][row];
	  fTimeShiftEmc[module][column][row] = calibda.fTimeShiftEmc[module][column][row];
          fAltroOffsets[module][column][row] = calibda.fAltroOffsets[module][column][row];
	  fDecal[module][column][row]        = calibda.fDecal[module][column][row];
	}
      }
    }
  }

  return *this;
}

//________________________________________________________________
IlcRSTACKEmcCalibData::~IlcRSTACKEmcCalibData()
{
  // Destructor
}

//________________________________________________________________
void IlcRSTACKEmcCalibData::Reset()
{
  // Set all pedestals and all ADC channels to its ideal values = 5. (MeV/ADC)

  for (Int_t module=0; module<5; module++){
    for (Int_t column=0; column<56; column++){
      for (Int_t row=0; row<64; row++){
	fADCpedestalEmc[module][column][row] = 0.;
	fADCchannelEmc[module][column][row]  = 0.005;
	fHighLowRatioEmc[module][column][row] = 16. ;
	fTimeShiftEmc[module][column][row] = 0. ;
        fAltroOffsets[module][column][row] = 0 ;
	fDecal[module][column][row] = 1.;
      }
    }
  }
  fSampleTimeStep=100.e-9 ; //100 ns
}

//________________________________________________________________
void  IlcRSTACKEmcCalibData::Print(Option_t *option) const
{
  // Print tables of pedestals and ADC channels

  if (strstr(option,"ped")) {
    printf("\n	----	EMC Pedestal values	----\n\n");
    for (Int_t module=0; module<5; module++){
      printf("============== Module %d\n",module+1);
      for (Int_t column=0; column<56; column++){
	for (Int_t row=0; row<64; row++){
	  printf("%4.1f",fADCpedestalEmc[module][column][row]);
	}
	printf("\n");
      }
    }
  }

  if (strstr(option,"gain")) {
    printf("\n	----	EMC ADC channel values	----\n\n");
    for (Int_t module=0; module<5; module++){
      printf("============== Module %d\n",module+1);
      for (Int_t column=0; column<56; column++){
	for (Int_t row=0; row<64; row++){
	  printf("%4.1f",fADCchannelEmc[module][column][row]);
	}
	printf("\n");
      }
    }
  }

  if (strstr(option,"hilo")) {
    printf("\n	----	EMC High/Low ratio	----\n\n");
    for (Int_t module=0; module<5; module++){
      printf("============== Module %d\n",module+1);
      for (Int_t column=0; column<56; column++){
	for (Int_t row=0; row<64; row++){
	  printf("%4.1f",fHighLowRatioEmc[module][column][row]);
	}
	printf("\n");
      }
    }
  }
  if (strstr(option,"time")) {
    printf("\n	----	EMC t0 shifts	----\n\n");
    for (Int_t module=0; module<5; module++){
      printf("============== Module %d\n",module+1);
      for (Int_t column=0; column<56; column++){
	for (Int_t row=0; row<64; row++){
	  printf("%6.3e",fTimeShiftEmc[module][column][row]);
	}
	printf("\n");
      }
    }
  }
  if (strstr(option,"altro")) {
    printf("\n  ----    EMC altro offsets   ----\n\n");
    for (Int_t module=0; module<5; module++){
      printf("============== Module %d\n",module+1);
      for (Int_t column=0; column<56; column++){
        for (Int_t row=0; row<64; row++){
          printf("%5d",fAltroOffsets[module][column][row]);
        }
        printf("\n");
      }
    }
  }
}

//________________________________________________________________
Float_t IlcRSTACKEmcCalibData::GetADCchannelEmc(Int_t module, Int_t column, Int_t row) const
{
  //Return EMC calibration coefficient
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64

  return fADCchannelEmc[module-1][column-1][row-1];
}

//________________________________________________________________
Float_t IlcRSTACKEmcCalibData::GetADCpedestalEmc(Int_t module, Int_t column, Int_t row) const
{
  //Return EMC pedestal
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64

  return fADCpedestalEmc[module-1][column-1][row-1];
}

//________________________________________________________________
Float_t IlcRSTACKEmcCalibData::GetHighLowRatioEmc(Int_t module, Int_t column, Int_t row) const
{
  //Return EMC pedestal
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64

  return fHighLowRatioEmc[module-1][column-1][row-1];
}

//________________________________________________________________
Float_t IlcRSTACKEmcCalibData::GetTimeShiftEmc(Int_t module, Int_t column, Int_t row) const
{
  //Return EMC pedestal
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64

  return fTimeShiftEmc[module-1][column-1][row-1];
}
//________________________________________________________________
Float_t IlcRSTACKEmcCalibData::GetSampleTimeStep()const
{
   //Returns conversion coefficient from ALTRO smaple time step and secods
   //Negative value not used in reconstruction (conversion taken from TRU trailer)
   //and only in raw simulation
   return fSampleTimeStep ;
}
//________________________________________________________________
Int_t IlcRSTACKEmcCalibData::GetAltroOffsetEmc(Int_t module, Int_t column, Int_t row) const
{
  //Return EMC altro offsets
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64
 
  return fAltroOffsets[module-1][column-1][row-1];
}
//________________________________________________________________
void IlcRSTACKEmcCalibData::SetADCchannelEmc(Int_t module, Int_t column, Int_t row, Float_t value)
{
  //Set EMC calibration coefficient
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64

  fADCchannelEmc[module-1][column-1][row-1] = value;
}

//________________________________________________________________
void IlcRSTACKEmcCalibData::SetADCpedestalEmc(Int_t module, Int_t column, Int_t row, Float_t value)
{
  //Set EMC pedestal
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64
  fADCpedestalEmc[module-1][column-1][row-1] = value;
}

//________________________________________________________________
void IlcRSTACKEmcCalibData::SetHighLowRatioEmc(Int_t module, Int_t column, Int_t row, Float_t value)
{
  //Set EMC pedestal
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64
  fHighLowRatioEmc[module-1][column-1][row-1] = value;
}
//________________________________________________________________
void IlcRSTACKEmcCalibData::SetTimeShiftEmc(Int_t module, Int_t column, Int_t row, Float_t value)
{
  //Set EMC pedestal
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64
  fTimeShiftEmc[module-1][column-1][row-1] = value;
}
//________________________________________________________________
void IlcRSTACKEmcCalibData::SetAltroOffsetEmc(Int_t module, Int_t column, Int_t row, Int_t value)
{
  //Set EMC pedestal
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64
  fAltroOffsets[module-1][column-1][row-1] = value;
}
//________________________________________________________________
void IlcRSTACKEmcCalibData::SetSampleTimeStep(Float_t step)
{
   //Sets conversion coefficient from ALTRO smaple time step and secods
   //Negative value not used in reconstruction (conversion taken from TRU trailer)
   //and only in raw simulation
   fSampleTimeStep = step ;
}

//________________________________________________________________
Float_t IlcRSTACKEmcCalibData::GetADCchannelEmcDecalib(Int_t module, Int_t column, Int_t row) const
{
  //Return EMC random (de)calibration coefficient O(1). Used in simulation.
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64

  return fDecal[module-1][column-1][row-1];
}

void IlcRSTACKEmcCalibData::SetADCchannelEmcDecalib(Int_t module, Int_t column, Int_t row, Float_t value)
{
  //Set EMC (de)calibration coefficient O(1). Used in simulation.
  //module, column,raw should follow the internal RSTACK convention:
  //module 1:5, column 1:56, row 1:64

  fDecal[module-1][column-1][row-1] = value;
}
