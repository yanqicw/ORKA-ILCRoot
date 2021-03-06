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

/* $Id: IlcAODCluster.cxx 50615 2011-07-16 23:19:19Z hristov $ */

//-------------------------------------------------------------------------
//     AOD cluster base class
//     Author: Markus Oldenburg, CERN
//-------------------------------------------------------------------------

#include "IlcAODCluster.h"

ClassImp(IlcAODCluster)

//______________________________________________________________________________
IlcAODCluster::IlcAODCluster() :
  IlcVCluster(),
  fEnergy(0),
  fChi2(-999.),
  fID(-999),
  fNLabel(0),
  fLabel(0x0),
  fFilterMap(0),
  fType(kUndef)
{
  // default constructor

  SetPosition(NULL);
  SetPID(NULL);
}

//______________________________________________________________________________
IlcAODCluster::IlcAODCluster(Int_t id,
			     UInt_t nLabel,
			     Int_t *label, 
			     Double_t energy,
			     Double_t x[3],
			     Double_t pid[13],
			     Char_t ttype,
			     UInt_t selectInfo) :
  IlcVCluster(),
  fEnergy(energy),
  fChi2(-999.),
  fID(id),
  fNLabel(0),
  fLabel(0x0),
  fFilterMap(selectInfo),
  fType(ttype)
{
  // constructor
  for (Int_t i = 0; i <  3; i++) fPosition[i] = 0.;
  for (Int_t i = 0; i < 13; i++) fPID[i]      = 0;
 
  if(x)   {for (Int_t i = 0; i < 3  ; i++) SetPositionAt(x[i]  ,i);}
  if(pid) {for (Int_t i = 0; i < 13 ; i++) SetPIDAt     (pid[i],i);}
  SetLabel(label, nLabel);
}

//______________________________________________________________________________
IlcAODCluster::IlcAODCluster(Int_t id,
			     UInt_t nLabel,
			     Int_t *label, 
			     Float_t energy,
			     Float_t x[3],
			     Float_t pid[13],
			     Char_t ttype,
			     UInt_t selectInfo) :
  IlcVCluster(),
  fEnergy(energy),
  fChi2(-999.),
  fID(id),
  fNLabel(0),
  fLabel(0x0),
  fFilterMap(selectInfo),
  fType(ttype)
{
  // constructor
  for (Int_t i = 0; i <  3; i++) fPosition[i] = 0.;
  for (Int_t i = 0; i < 13; i++) fPID[i]      = 0;

  if(x)   {for (Int_t i = 0; i < 3  ; i++) SetPositionAt(x[i]  ,i);}
  if(pid) {for (Int_t i = 0; i < 13 ; i++) SetPIDAt     (pid[i],i);}
  SetLabel(label, nLabel);
}


//______________________________________________________________________________
IlcAODCluster::~IlcAODCluster() 
{
  // destructor

  RemoveLabel();
}

//______________________________________________________________________________
void IlcAODCluster::Clear(const Option_t*) 
{
  // Clear
  
  RemoveLabel();
}


//______________________________________________________________________________
IlcAODCluster::IlcAODCluster(const IlcAODCluster& clus) :
  IlcVCluster(clus),
  fEnergy(clus.fEnergy),
  fChi2(clus.fChi2),
  fID(clus.fID),
  fNLabel(0),
  fLabel(0x0),
  fFilterMap(clus.fFilterMap),
  fType(clus.fType)
{
  // Copy constructor

  for(Int_t i = 0; i < 3  ; i++) fPosition[i]  = clus.fPosition[i];
  for(Int_t i = 0; i < 13 ; i++)  fPID[i]      = clus.fPID[i];

  SetLabel(clus.fLabel, clus.fNLabel);
}

//______________________________________________________________________________
IlcAODCluster& IlcAODCluster::operator=(const IlcAODCluster& clus)
{
  // Assignment operator
  if(this!=&clus) {
    
	for(Int_t i = 0; i < 3 ;  i++) fPosition[i] = clus.fPosition[i];
	for(Int_t i = 0; i < 13 ; i++) fPID[i]      = clus.fPID[i];
    
    fEnergy = clus.fEnergy;
    fChi2 = clus.fChi2;

    fID = clus.fID;
    SetLabel(clus.fLabel, clus.fNLabel);
    fFilterMap = clus.fFilterMap;

    fType = clus.fType;
  }

  return *this;
}

//______________________________________________________________________________
void IlcAODCluster::SetPosition(Float_t *x) 
{
  // set the position
  
  if (x) {
    fPosition[0] = x[0];
    fPosition[1] = x[1];
    fPosition[2] = x[2];
  } else {
    fPosition[0] = -999.;
    fPosition[1] = -999.;
    fPosition[2] = -999.;
  }
}

//______________________________________________________________________________
UShort_t IlcAODCluster::GetMostProbablePID() const 
{
  // Returns the most probable PID array element.
  
  Int_t nPID = 13;
  UShort_t unknown = IlcVCluster::kUnknown;
  
  UShort_t loc = unknown;
  Double_t max = 0.;
  Bool_t allTheSame = kTRUE;
  
  for (Int_t iPID = 0; iPID < nPID; iPID++) {
    if (fPID[iPID] >= max) {
      if (fPID[iPID] > max) {
	allTheSame = kFALSE;
	max = fPID[iPID];
	loc = (UShort_t)iPID;
      } else {
	allTheSame = kTRUE;
      }
    }
  }
  return allTheSame ? unknown : loc;
}

//______________________________________________________________________________
void IlcAODCluster::SetLabel(Int_t *label, UInt_t size) 
{
  if (label && size>0) {
    if (size != (UInt_t)fNLabel) {
      RemoveLabel();
      fNLabel = size;
      fLabel = new Int_t[fNLabel];
    }
    
    for (Int_t i = 0; i < fNLabel; i++) {
      fLabel[i] = label[i];
    }
  } else {
    RemoveLabel();
  }

  return;
}

//______________________________________________________________________________
Int_t IlcAODCluster::GetLabelAt(UInt_t i) const
{
  if (fLabel && i < (UInt_t)fNLabel) {
    return fLabel[i];
  } else {
    return -999;
  }
}

//______________________________________________________________________________
void IlcAODCluster::RemoveLabel()
{
  delete[] fLabel;
  fLabel = 0x0;
  fNLabel = 0;

  return;
}

//______________________________________________________________________________
void IlcAODCluster::Print(Option_t* /* option */) const
{
  // prints information about IlcAODCluster

  printf("Cluster type: %d\n", GetType()); 
  printf("     energy = %f\n", E());
  printf("       chi2 = %f\n", Chi2());
  const Double_t *pid = GetPID();
  printf("PID weights: photon %0.2f, pi0 %0.2f, electron %0.2f, conversion electron %0.2f\n, hadrons: pion %0.2f, kaon %0.2f, proton %0.2f , neutron %0.2f, kaon %0.2f \n",
	 pid[IlcVCluster::kPhoton],   pid[IlcVCluster::kPi0],
	 pid[IlcVCluster::kElectron], pid[IlcVCluster::kEleCon],
	 pid[IlcVCluster::kPion],     pid[IlcVCluster::kKaon],   pid[IlcVCluster::kProton],
	 pid[IlcVCluster::kNeutron],  pid[IlcVCluster::kKaon0]);
}
