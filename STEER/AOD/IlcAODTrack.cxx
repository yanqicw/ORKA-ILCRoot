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

/* $Id: IlcAODTrack.cxx 58259 2012-08-22 07:07:20Z hristov $ */

//-------------------------------------------------------------------------
//     AOD track implementation of IlcVTrack
//     Author: Markus Oldenburg, CERN
//     Markus.Oldenburg@cern.ch
//-------------------------------------------------------------------------

#include "IlcLog.h"
#include "IlcExternalTrackParam.h"
#include "IlcVVertex.h"
#include "IlcAODTrack.h"
#include "IlcAODEvent.h"

ClassImp(IlcAODTrack)

//______________________________________________________________________________
IlcAODTrack::IlcAODTrack() : 
  IlcVTrack(),
  fRAtAbsorberEnd(0.),
  fChi2perNDF(-999.),
  fChi2MatchTrigger(0.),
  fFlags(0),
  fLabel(-999),
  fITSMuonClusterMap(0),
  fMUONtrigHitsMapTrg(0),
  fMUONtrigHitsMapTrk(0),
  fFilterMap(0),
  fTPCFitMap(),
  fTPCClusterMap(),
  fTPCSharedMap(),
  fTPCnclsF(0),
  fTPCNCrossedRows(0),
  fID(-999),
  fCharge(-99),
  fType(kUndef),
  fCaloIndex(kEMCALNoMatch),
  fCovMatrix(NULL),
  fDetPid(NULL),
  fProdVertex(NULL),
  fTrackPhiOnEMCal(-999),
  fTrackEtaOnEMCal(-999),
  fAODEvent(NULL)
{
  // default constructor

  SetP();
  SetPosition((Float_t*)NULL);
  SetXYAtDCA(-999., -999.);
  SetPxPyPzAtDCA(-999., -999., -999.);
  SetPID((Float_t*)NULL);
}

//______________________________________________________________________________
IlcAODTrack::IlcAODTrack(Short_t id,
			 Int_t label, 
			 Double_t p[3],
			 Bool_t cartesian,
			 Double_t x[3],
			 Bool_t isDCA,
			 Double_t covMatrix[21],
			 Short_t charge,
			 UChar_t itsClusMap,
			 Double_t pid[10],
			 IlcAODVertex *prodVertex,
			 Bool_t usedForVtxFit,
			 Bool_t usedForPrimVtxFit,
			 AODTrk_t ttype,
			 UInt_t selectInfo,
			 Float_t chi2perNDF) :
  IlcVTrack(),
  fRAtAbsorberEnd(0.),
  fChi2perNDF(chi2perNDF),
  fChi2MatchTrigger(0.),
  fFlags(0),
  fLabel(label),
  fITSMuonClusterMap(0),
  fMUONtrigHitsMapTrg(0),
  fMUONtrigHitsMapTrk(0),
  fFilterMap(selectInfo),
  fTPCFitMap(),
  fTPCClusterMap(),
  fTPCSharedMap(),
  fTPCnclsF(0),
  fTPCNCrossedRows(0),
  fID(id),
  fCharge(charge),
  fType(ttype),
  fCaloIndex(kEMCALNoMatch),
  fCovMatrix(NULL),
  fDetPid(NULL),
  fProdVertex(prodVertex),
  fTrackPhiOnEMCal(-999),
  fTrackEtaOnEMCal(-999),
  fAODEvent(NULL)
{
  // constructor
 
  SetP(p, cartesian);
  SetPosition(x, isDCA);
  SetXYAtDCA(-999., -999.);
  SetPxPyPzAtDCA(-999., -999., -999.);
  SetUsedForVtxFit(usedForVtxFit);
  SetUsedForPrimVtxFit(usedForPrimVtxFit);
  if(covMatrix) SetCovMatrix(covMatrix);
  SetPID(pid);
  SetITSClusterMap(itsClusMap);
}

//______________________________________________________________________________
IlcAODTrack::IlcAODTrack(Short_t id,
			 Int_t label, 
			 Float_t p[3],
			 Bool_t cartesian,
			 Float_t x[3],
			 Bool_t isDCA,
			 Float_t covMatrix[21],
			 Short_t charge,
			 UChar_t itsClusMap,
			 Float_t pid[10],
			 IlcAODVertex *prodVertex,
			 Bool_t usedForVtxFit,
			 Bool_t usedForPrimVtxFit,
			 AODTrk_t ttype,
			 UInt_t selectInfo,
			 Float_t chi2perNDF) :
  IlcVTrack(),
  fRAtAbsorberEnd(0.),
  fChi2perNDF(chi2perNDF),
  fChi2MatchTrigger(0.),
  fFlags(0),
  fLabel(label),
  fITSMuonClusterMap(0),
  fMUONtrigHitsMapTrg(0),
  fMUONtrigHitsMapTrk(0),
  fFilterMap(selectInfo),
  fTPCFitMap(),
  fTPCClusterMap(),
  fTPCSharedMap(),
  fTPCnclsF(0),
  fTPCNCrossedRows(0),
  fID(id),
  fCharge(charge),
  fType(ttype),
  fCaloIndex(kEMCALNoMatch),
  fCovMatrix(NULL),
  fDetPid(NULL),
  fProdVertex(prodVertex),
  fTrackPhiOnEMCal(-999),
  fTrackEtaOnEMCal(-999),
  fAODEvent(NULL)
{
  // constructor
 
  SetP(p, cartesian);
  SetPosition(x, isDCA);
  SetXYAtDCA(-999., -999.);
  SetPxPyPzAtDCA(-999., -999., -999.);
  SetUsedForVtxFit(usedForVtxFit);
  SetUsedForPrimVtxFit(usedForPrimVtxFit);
  if(covMatrix) SetCovMatrix(covMatrix);
  SetPID(pid);
  SetITSClusterMap(itsClusMap);
  SetVXDClusterMap(itsClusMap);
}

//______________________________________________________________________________
IlcAODTrack::~IlcAODTrack() 
{
  // destructor
  delete fCovMatrix;
  delete fDetPid;
}


//______________________________________________________________________________
IlcAODTrack::IlcAODTrack(const IlcAODTrack& trk) :
  IlcVTrack(trk),
  fRAtAbsorberEnd(trk.fRAtAbsorberEnd),
  fChi2perNDF(trk.fChi2perNDF),
  fChi2MatchTrigger(trk.fChi2MatchTrigger),
  fFlags(trk.fFlags),
  fLabel(trk.fLabel),
  fITSMuonClusterMap(trk.fITSMuonClusterMap),
  fMUONtrigHitsMapTrg(trk.fMUONtrigHitsMapTrg),
  fMUONtrigHitsMapTrk(trk.fMUONtrigHitsMapTrk),
  fFilterMap(trk.fFilterMap),
  fTPCFitMap(trk.fTPCFitMap),
  fTPCClusterMap(trk.fTPCClusterMap),
  fTPCSharedMap(trk.fTPCSharedMap),
  fTPCnclsF(trk.fTPCnclsF),
  fTPCNCrossedRows(trk.fTPCNCrossedRows),
  fID(trk.fID),
  fCharge(trk.fCharge),
  fType(trk.fType),
  fCaloIndex(trk.fCaloIndex),
  fCovMatrix(NULL),
  fDetPid(NULL),
  fProdVertex(trk.fProdVertex),
  fTrackPhiOnEMCal(trk.fTrackPhiOnEMCal),
  fTrackEtaOnEMCal(trk.fTrackEtaOnEMCal),
  fAODEvent(trk.fAODEvent)
{
  // Copy constructor

  trk.GetP(fMomentum);
  trk.GetPosition(fPosition);
  SetXYAtDCA(trk.XAtDCA(), trk.YAtDCA());
  SetPxPyPzAtDCA(trk.PxAtDCA(), trk.PyAtDCA(), trk.PzAtDCA());
  SetUsedForVtxFit(trk.GetUsedForVtxFit());
  SetUsedForPrimVtxFit(trk.GetUsedForPrimVtxFit());
  if(trk.fCovMatrix) fCovMatrix=new IlcAODRedCov<6>(*trk.fCovMatrix);
  if(trk.fDetPid) fDetPid=new IlcAODPid(*trk.fDetPid);
  SetPID(trk.fPID);
}

//______________________________________________________________________________
IlcAODTrack& IlcAODTrack::operator=(const IlcAODTrack& trk)
{
  // Assignment operator
  if(this!=&trk) {

    IlcVTrack::operator=(trk);

    trk.GetP(fMomentum);
    trk.GetPosition(fPosition);
    SetXYAtDCA(trk.XAtDCA(), trk.YAtDCA());
    SetPxPyPzAtDCA(trk.PxAtDCA(), trk.PyAtDCA(), trk.PzAtDCA());
    fRAtAbsorberEnd    = trk.fRAtAbsorberEnd;
    fChi2perNDF        = trk.fChi2perNDF;
    fChi2MatchTrigger  = trk.fChi2MatchTrigger;
    trk.GetPID(fPID);
    fFlags             = trk.fFlags;
    fLabel             = trk.fLabel;    
    fITSMuonClusterMap = trk.fITSMuonClusterMap;
    fMUONtrigHitsMapTrg = trk.fMUONtrigHitsMapTrg;
    fMUONtrigHitsMapTrk = trk.fMUONtrigHitsMapTrk;
    fFilterMap         = trk.fFilterMap;
    fTPCFitMap         = trk.fTPCFitMap;
    fTPCClusterMap     = trk.fTPCClusterMap;
    fTPCSharedMap      = trk.fTPCSharedMap;
    fTPCnclsF          = trk.fTPCnclsF;
    fTPCNCrossedRows   = trk.fTPCNCrossedRows;
    fID                = trk.fID;
    fCharge            = trk.fCharge;
    fType              = trk.fType;
    fCaloIndex         = trk.fCaloIndex;
    fTrackPhiOnEMCal   = trk.fTrackPhiOnEMCal;
    fTrackEtaOnEMCal   = trk.fTrackEtaOnEMCal;

    delete fCovMatrix;
    if(trk.fCovMatrix) fCovMatrix=new IlcAODRedCov<6>(*trk.fCovMatrix);
    else fCovMatrix=NULL;


    fProdVertex        = trk.fProdVertex;
    SetUsedForVtxFit(trk.GetUsedForVtxFit());
    SetUsedForPrimVtxFit(trk.GetUsedForPrimVtxFit());

    delete fDetPid;
    if(trk.fDetPid) fDetPid=new IlcAODPid(*trk.fDetPid);
    else fDetPid=NULL;


  }

  return *this;
}

//______________________________________________________________________________
Double_t IlcAODTrack::M(AODTrkPID_t pid) const
{
  // Returns the mass.
  // Masses for nuclei don't exist in the PDG tables, therefore they were put by hand.

  switch (pid) {

  case kElectron :
    return 0.000510999; //TDatabasePDG::Instance()->GetParticle(11/*::kElectron*/)->Mass();
    break;

  case kMuon :
    return 0.1056584; //TDatabasePDG::Instance()->GetParticle(13/*::kMuonMinus*/)->Mass();
    break;

  case kPion :
    return 0.13957; //TDatabasePDG::Instance()->GetParticle(211/*::kPiPlus*/)->Mass();
    break;

  case kKaon :
    return 0.4937; //TDatabasePDG::Instance()->GetParticle(321/*::kKPlus*/)->Mass();
    break;

  case kProton :
    return 0.9382720; //TDatabasePDG::Instance()->GetParticle(2212/*::kProton*/)->Mass();
    break;

  case kDeuteron :
    return 1.8756; //TDatabasePDG::Instance()->GetParticle(1000010020)->Mass();
    break;

  case kTriton :
    return 2.8089; //TDatabasePDG::Instance()->GetParticle(1000010030)->Mass();
    break;

  case kHelium3 :
    return 2.8084; //TDatabasePDG::Instance()->GetParticle(1000020030)->Mass();
    break;

  case kAlpha :
    return 3.7274; //TDatabasePDG::Instance()->GetParticle(1000020040)->Mass();
    break;

  case kUnknown :
    return -999.;
    break;

  default :
    return -999.;
  }
}

//______________________________________________________________________________
Double_t IlcAODTrack::E(AODTrkPID_t pid) const
{
  // Returns the energy of the particle of a given pid.
  
  if (pid != kUnknown) { // particle was identified
    Double_t m = M(pid);
    return TMath::Sqrt(P()*P() + m*m);
  } else { // pid unknown
    return -999.;
  }
}

//______________________________________________________________________________
Double_t IlcAODTrack::Y(AODTrkPID_t pid) const
{
  // Returns the rapidity of a particle of a given pid.
  
  if (pid != kUnknown) { // particle was identified
    Double_t e = E(pid);
    Double_t pz = Pz();
    if (e>=0 && e!=pz) { // energy was positive (e.g. not -999.) and not equal to pz
      return 0.5*TMath::Log((e+pz)/(e-pz));
    } else { // energy not known or equal to pz
      return -999.;
    }
  } else { // pid unknown
    return -999.;
  }
}

//______________________________________________________________________________
Double_t IlcAODTrack::Y(Double_t m) const
{
  // Returns the rapidity of a particle of a given mass.
  
  if (m >= 0.) { // mass makes sense
    Double_t e = E(m);
    Double_t pz = Pz();
    if (e>=0 && e!=pz) { // energy was positive (e.g. not -999.) and not equal to pz
      return 0.5*TMath::Log((e+pz)/(e-pz));
    } else { // energy not known or equal to pz
      return -999.;
    }
  } else { // pid unknown
    return -999.;
  }
}

//______________________________________________________________________________
IlcAODTrack::AODTrkPID_t IlcAODTrack::GetMostProbablePID() const 
{
  // Returns the most probable PID array element.
  
  Int_t nPID = 10;
  AODTrkPID_t loc = kUnknown;
  Double_t max = 0.;
  Bool_t allTheSame = kTRUE;
  
  for (Int_t iPID = 0; iPID < nPID; iPID++) {
    if (fPID[iPID] >= max) {
      if (fPID[iPID] > max) {
	allTheSame = kFALSE;
	max = fPID[iPID];
	loc = (AODTrkPID_t)iPID;
      } else {
	allTheSame = kTRUE;
      }
    }
  }
  return allTheSame ? kUnknown : loc;
}

//______________________________________________________________________________
void IlcAODTrack::ConvertIlcPIDtoAODPID()
{
  // Converts IlcPID array.
  // The numbering scheme is the same for electrons, muons, pions, kaons, and protons.
  // Everything else has to be set to zero.

  fPID[kDeuteron] = 0.;
  fPID[kTriton]   = 0.;
  fPID[kHelium3]  = 0.;
  fPID[kAlpha]    = 0.;
  fPID[kUnknown]  = 0.;
  
  return;
}


//______________________________________________________________________________
template <class T> void IlcAODTrack::SetP(const T *p, const Bool_t cartesian) 
{
  // Set the momentum

  if (p) {
    if (cartesian) {
      Double_t pt2 = p[0]*p[0] + p[1]*p[1];
      Double_t pp  = TMath::Sqrt(pt2 + p[2]*p[2]);
      
      fMomentum[0] = TMath::Sqrt(pt2); // pt
      fMomentum[1] = (pt2 != 0.) ? TMath::Pi()+TMath::ATan2(-p[1], -p[0]) : -999; // phi
      fMomentum[2] = (pp != 0.) ? TMath::ACos(p[2] / pp) : -999.; // theta
    } else {
      fMomentum[0] = p[0];  // pt
      fMomentum[1] = p[1];  // phi
      fMomentum[2] = p[2];  // theta
    }
  } else {
    fMomentum[0] = -999.;
    fMomentum[1] = -999.;
    fMomentum[2] = -999.;
  }
}

//______________________________________________________________________________
template <class T> void IlcAODTrack::SetPosition(const T *x, const Bool_t dca) 
{
  // set the position

  if (x) {
    if (!dca) {
      ResetBit(kIsDCA);

      fPosition[0] = x[0];
      fPosition[1] = x[1];
      fPosition[2] = x[2];
    } else {
      SetBit(kIsDCA);
      // don't know any better yet
      fPosition[0] = -999.;
      fPosition[1] = -999.;
      fPosition[2] = -999.;
    }
  } else {
    ResetBit(kIsDCA);

    fPosition[0] = -999.;
    fPosition[1] = -999.;
    fPosition[2] = -999.;
  }
}

//______________________________________________________________________________
void IlcAODTrack::SetDCA(Double_t d, Double_t z) 
{
  // set the dca
  fPosition[0] = d;
  fPosition[1] = z;
  fPosition[2] = 0.;
  SetBit(kIsDCA);
}

//______________________________________________________________________________
void IlcAODTrack::Print(Option_t* /* option */) const
{
  // prints information about IlcAODTrack

  printf("Object name: %s   Track type: %s\n", GetName(), GetTitle()); 
  printf("        px = %f\n", Px());
  printf("        py = %f\n", Py());
  printf("        pz = %f\n", Pz());
  printf("        pt = %f\n", Pt());
  printf("      1/pt = %f\n", OneOverPt());
  printf("     theta = %f\n", Theta());
  printf("       phi = %f\n", Phi());
  printf("  chi2/NDF = %f\n", Chi2perNDF());
  printf("    charge = %d\n", Charge());
}

//______________________________________________________________________________
void IlcAODTrack::SetMatchTrigger(Int_t matchTrig)
{
  // Set the MUON trigger information
  switch(matchTrig){
    case 0: // 0 track does not match trigger
      fITSMuonClusterMap=fITSMuonClusterMap&0x3fffffff;
      break;
    case 1: // 1 track match but does not pass pt cut
      fITSMuonClusterMap=(fITSMuonClusterMap&0x3fffffff)|0x40000000;
      break;
    case 2: // 2 track match Low pt cut
      fITSMuonClusterMap=(fITSMuonClusterMap&0x3fffffff)|0x80000000;
      break;
    case 3: // 3 track match High pt cut
      fITSMuonClusterMap=fITSMuonClusterMap|0xc0000000;
      break;
    default:
      fITSMuonClusterMap=fITSMuonClusterMap&0x3fffffff;
      IlcWarning(Form("unknown case for matchTrig: %d\n",matchTrig));
  }
}

//______________________________________________________________________________
Bool_t IlcAODTrack::HitsMuonChamber(Int_t MuonChamber, Int_t cathode) const
{
  // return kTRUE if the track fires the given tracking or trigger chamber.
  // If the chamber is a trigger one:
  // - if cathode = 0 or 1, the track matches the corresponding cathode
  // - if cathode = -1, the track matches both cathodes
  
  if (MuonChamber < 0) return kFALSE;
  
  if (MuonChamber < 10) return TESTBIT(GetMUONClusterMap(), MuonChamber);
  
  if (MuonChamber < 14) {
    
    if (cathode < 0) return TESTBIT(GetHitsPatternInTrigCh(), 13-MuonChamber) &&
                            TESTBIT(GetHitsPatternInTrigCh(), 13-MuonChamber+4);
    
    if (cathode < 2) return TESTBIT(GetHitsPatternInTrigCh(), 13-MuonChamber+(1-cathode)*4);
    
  }
  
  return kFALSE;
}

//______________________________________________________________________________
Bool_t IlcAODTrack::MatchTriggerDigits() const
{
  // return kTRUE if the track matches a digit on both planes of at least 2 trigger chambers
  
  Int_t nMatchedChambers = 0;
  for (Int_t ich=10; ich<14; ich++) if (HitsMuonChamber(ich)) nMatchedChambers++;
  
  return (nMatchedChambers >= 2);
}

//______________________________________________________________________________
Bool_t IlcAODTrack::PropagateToDCA(const IlcVVertex *vtx, 
    Double_t b, Double_t maxd, Double_t dz[2], Double_t covar[3])
{
  // compute impact parameters to the vertex vtx and their covariance matrix
  // b is the Bz, needed to propagate correctly the track to vertex 
  // only the track parameters are update after the propagation (pos and mom),
  // not the covariance matrix. This is OK for propagation over short distance
  // inside the beam pipe.
  // return kFALSE is something went wrong

  // convert to IlcExternalTrackParam
  IlcExternalTrackParam etp; etp.CopyFromVTrack(this);  

  Float_t xstart = etp.GetX();
  if(xstart>3.) {
    IlcError("This method can be used only for propagation inside the beam pipe");
    return kFALSE; 
  }

  if(!etp.PropagateToDCA(vtx,b,maxd,dz,covar)) return kFALSE;

  // update track position and momentum
  Double_t mom[3];
  etp.GetPxPyPz(mom);
  SetP(mom,kTRUE);
  etp.GetXYZ(mom);
  SetPosition(mom,kFALSE);


  return kTRUE;
}

//______________________________________________________________________________
Bool_t IlcAODTrack::GetPxPyPz(Double_t p[3]) const 
{
    //---------------------------------------------------------------------
    // This function returns the global track momentum components
    //---------------------------------------------------------------------
  p[0]=Px(); p[1]=Py(); p[2]=Pz();
  return kTRUE;
}


//_______________________________________________________________________
Float_t IlcAODTrack::GetTPCClusterInfo(Int_t nNeighbours/*=3*/, Int_t type/*=0*/, Int_t row0, Int_t row1, Int_t bitType ) const
{
  //
  // TPC cluster information 
  // type 0: get fraction of found/findable clusters with neighbourhood definition
  //      1: findable clusters with neighbourhood definition
  //      2: found clusters
  // bitType:
  //      0 - all cluster used
  //      1 - clusters  used for the kalman update
  // definition of findable clusters:
  //            a cluster is defined as findable if there is another cluster
  //           within +- nNeighbours pad rows. The idea is to overcome threshold
  //           effects with a very simple algorithm.
  //

  
  Int_t found=0;
  Int_t findable=0;
  Int_t last=-nNeighbours;
  const TBits & clusterMap = (bitType%2==0) ? fTPCClusterMap : fTPCFitMap;
  
  Int_t upperBound=clusterMap.GetNbits();
  if (upperBound>row1) upperBound=row1;
  for (Int_t i=row0; i<upperBound; ++i){
    //look to current row
    if (clusterMap[i]) {
      last=i;
      ++found;
      ++findable;
      continue;
    }
    //look to nNeighbours before
    if ((i-last)<=nNeighbours) {
      ++findable;
      continue;
    }
    //look to nNeighbours after
    for (Int_t j=i+1; j<i+1+nNeighbours; ++j){
      if (clusterMap[j]){
        ++findable;
        break;
      }
    }
  }
  if (type==2) return found;
  if (type==1) return findable;
  
  if (type==0){
    Float_t fraction=0;
    if (findable>0) 
      fraction=(Float_t)found/(Float_t)findable;
    else 
      fraction=0;
    return fraction;
  }  
  return 0;  // undefined type - default value
}


//______________________________________________________________________________
Double_t  IlcAODTrack::GetTRDslice(Int_t plane, Int_t slice) const {
  //
  // return TRD Pid information
  //
  if (!fDetPid) return -1;
  Double32_t *trdSlices=fDetPid->GetTRDsignal();
  if (!trdSlices) return -1;
  if ((plane<0) || (plane>=kTRDnPlanes)) {
    return -1.;
  }

  Int_t ns=fDetPid->GetTRDnSlices()/kTRDnPlanes;
  if ((slice<-1) || (slice>=ns)) {
    return -1.;
  }

  if(slice>=0) return trdSlices[plane*ns + slice];

  // return average of the dEdx measurements
  Double_t q=0.; Double32_t *s = &trdSlices[plane*ns];
  for (Int_t i=0; i<ns; i++, s++) if((*s)>0.) q+=(*s);
  return q/ns;
}

//______________________________________________________________________________
UChar_t IlcAODTrack::GetTRDntrackletsPID() const{
  //
  // return number of tracklets calculated from the slices
  //
  if(!fDetPid) return -1;
  return fDetPid->GetTRDntrackletsPID();
}

//______________________________________________________________________________
UChar_t IlcAODTrack::GetTRDncls(Int_t layer) const {
  // 
  // return number of TRD clusters
  //
  if(!fDetPid || layer > 5) return -1;
  if(layer < 0) return fDetPid->GetTRDncls();
  else return fDetPid->GetTRDncls(layer);
}

//______________________________________________________________________________
Double_t IlcAODTrack::GetTRDmomentum(Int_t plane, Double_t */*sp*/) const
{
  //Returns momentum estimation
  // in TRD layer "plane".

  if (!fDetPid) return -1;
  const Double_t *trdMomentum=fDetPid->GetTRDmomentum();

  if (!trdMomentum) {
    return -1.;
  }
  if ((plane<0) || (plane>=kTRDnPlanes)) {
    return -1.;
  }

  return trdMomentum[plane];
}

//_______________________________________________________________________
Int_t IlcAODTrack::GetTOFBunchCrossing(Double_t b) const 
{
  // Returns the number of bunch crossings after trigger (assuming 25ns spacing)
  const double kSpacing = 25e3; // min interbanch spacing
  const double kShift = 0;
  Int_t bcid = kTOFBCNA; // defualt one
  if (!IsOn(kTOFout) || !IsOn(kESDpid)) return bcid; // no info
  //
  double tdif = GetTOFsignal();
  if (IsOn(kTIME)) { // integrated time info is there
    int pid = (int)GetMostProbablePID();
    double ttimes[10]; 
    GetIntegratedTimes(ttimes);
    tdif -= ttimes[pid];
  }
  else { // assume integrated time info from TOF radius and momentum
    const double kRTOF = 385.;
    const double kCSpeed = 3.e-2; // cm/ps
    double p = P();
    if (p<0.001) p = 1.0;
    double m = M();
    double path =  kRTOF;     // mean TOF radius
    if (TMath::Abs(b)>kAlmost0) {  // account for curvature
      double curv = Pt()/(b*kB2C);
      if (curv>kAlmost0) {
	double tgl = Pz()/Pt();
	path = 2./curv*TMath::ASin(kRTOF*curv/2.)*TMath::Sqrt(1.+tgl*tgl);
      }
    }
    tdif -= path/kCSpeed*TMath::Sqrt(1.+m*m/(p*p));
  }
  bcid = TMath::Nint((tdif - kShift)/kSpacing);
  return bcid;
}
