/**************************************************************************
 * Copyright(c) 2005-2006, ILC Project Experiment, All rights reserved.   *
 *                                                                        *
// Author: The ILC Off-line Project. 
 // Part of the code has been developed by Alice Off-line Project. 
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
#include <IlcTARGETVertexerZ.h>
#include <TString.h>
#include <IlcTARGETLoader.h>
#include<TBranch.h>
#include<TH1.h>
#include<TTree.h>
#include <IlcTARGETgeom.h>
#include "IlcTARGETDetTypeRec.h"
#include <IlcTARGETRecPoint.h>

/////////////////////////////////////////////////////////////////
// this class implements a fast method to determine
// the Z coordinate of the primary vertex
// for p-p collisions it seems to give comparable or better results
// with respect to what obtained with IlcTARGETVertexerPPZ
// It can be used successfully with Pb-Pb collisions
////////////////////////////////////////////////////////////////

ClassImp(IlcTARGETVertexerZ)

 //______________________________________________________________________
#ifdef WIN32 
IlcESDVertex* IlcTARGETVertexerZ::FindVertexForCurrentEvent(TTree*)
{ 
	Fatal("FindVertexForCurrentEvent","FindVertexForCurrentEvent(TTree*) not yet implemented\n");
	return 0;
}
#endif

//______________________________________________________________________
IlcTARGETVertexerZ::IlcTARGETVertexerZ():IlcTARGETVertexer() {
  // Default constructor
  SetDiffPhiMax(0);
  fX0 = 0.;
  fY0 = 0.;
  SetFirstLayerModules(0);
  SetSecondLayerModules(0);
  fZFound = 0;
  fZsig = 0.;
  fZCombc = 0;
  fZCombf = 0;
  fZCombv = 0;
  SetLowLimit(0.);
  SetHighLimit(0.);
  SetBinWidthCoarse(0.);
  SetBinWidthFine(0.);
  SetTolerance(0.);
  SetPPsetting(0.,0.);
  ConfigIterations();
}

//______________________________________________________________________
IlcTARGETVertexerZ::IlcTARGETVertexerZ(TString fn, Float_t x0, Float_t y0):IlcTARGETVertexer(fn) {
  // Standard Constructor
  SetDiffPhiMax();
  fX0 = x0;
  fY0 = y0;
  SetFirstLayerModules();
  SetSecondLayerModules();
  fZFound = 0;
  fZsig = 0.;
  fZCombc = 0;
  fZCombf = 0;
  fZCombv = 0;
  SetLowLimit();
  SetHighLimit();
  SetBinWidthCoarse();
  SetBinWidthFine();
  SetTolerance();
  SetPPsetting();
  ConfigIterations();

}

// // // // //______________________________________________________________________
// // // // IlcTARGETVertexerZ::IlcTARGETVertexerZ(const IlcTARGETVertexerZ &vtxr) : IlcTARGETVertexer(vtxr) {
// // // //   // Copy constructor
// // // //   // Copies are not allowed. The method is protected to avoid misuse.
// // // //   Error("IlcTARGETVertexerZ","Copy constructor not allowed\n");
// // // // }

//______________________________________________________________________
IlcTARGETVertexerZ& IlcTARGETVertexerZ::operator=(const IlcTARGETVertexerZ& /* vtxr */){
  // Assignment operator
  // Assignment is not allowed. The method is protected to avoid misuse.
  Error("= operator","Assignment operator not allowed\n");
  return *this;
}

//______________________________________________________________________
IlcTARGETVertexerZ::~IlcTARGETVertexerZ() {
  // Destructor
  delete fZCombc;
  delete fZCombf;
  delete fZCombv;
}

//______________________________________________________________________
void IlcTARGETVertexerZ::ConfigIterations(Int_t noiter,Float_t *ptr){
  // configure the iterative procedure to gain efficiency for
  // pp events with very low multiplicity
  Float_t defaults[5]={0.05,0.1,0.2,0.3,0.5};
  fMaxIter=noiter;
  if(noiter>5){
    Error("ConfigIterations","Maximum number of iterations is 5\n");
    fMaxIter=5;
  }
  for(Int_t j=0;j<5;j++)fPhiDiffIter[j]=defaults[j];
  if(ptr)for(Int_t j=0;j<fMaxIter;j++)fPhiDiffIter[j]=ptr[j];
}

//______________________________________________________________________
IlcESDVertex* IlcTARGETVertexerZ::FindVertexForCurrentEvent(Int_t evnumber){
  // Defines the IlcESDVertex for the current event
  VertexZFinder(evnumber);
  Int_t ntrackl=0;
  for(Int_t iteraz=0;iteraz<fMaxIter;iteraz++){
    if(fCurrentVertex) ntrackl=fCurrentVertex->GetNContributors();
    if(!fCurrentVertex || ntrackl==0 || ntrackl==-1){
      Float_t diffPhiMaxOrig=fDiffPhiMax;
      fDiffPhiMax=GetPhiMaxIter(iteraz);
      VertexZFinder(evnumber);
      fDiffPhiMax=diffPhiMaxOrig;
    }
  }
  FindMultiplicity(evnumber);
  return fCurrentVertex;
}  




//______________________________________________________________________
void IlcTARGETVertexerZ::VertexZFinder(Int_t evnumber){
  // Defines the IlcESDVertex for the current event
  fCurrentVertex = 0;
  IlcRunLoader *rl =IlcRunLoader::Instance();
  IlcTARGETLoader* itsLoader = (IlcTARGETLoader*)rl->GetLoader("TARGETLoader");
  IlcTARGETgeom* geom = itsLoader->GetTARGETgeom();
  itsLoader->LoadRecPoints();
  rl->GetEvent(evnumber);

  IlcTARGETDetTypeRec detTypeRec;

  TTree *tR = itsLoader->TreeR();
  detTypeRec.SetTreeAddressR(tR);
  TClonesArray *itsRec  = 0;
  // lc and gc are local and global coordinates for layer 1
  Float_t lc[3]; for(Int_t ii=0; ii<3; ii++) lc[ii]=0.;
  Float_t gc[3]; for(Int_t ii=0; ii<3; ii++) gc[ii]=0.;
  // lc2 and gc2 are local and global coordinates for layer 2
  Float_t lc2[3]; for(Int_t ii=0; ii<3; ii++) lc2[ii]=0.;
  Float_t gc2[3]; for(Int_t ii=0; ii<3; ii++) gc2[ii]=0.;

  itsRec = detTypeRec.RecPoints();
  TBranch *branch;
  branch = tR->GetBranch("TARGETRecPoints");

  Int_t nrpL1 = 0;
  Int_t nrpL2 = 0;
  // By default fFirstL1=0 and fLastL1=79
  // This loop counts the number of recpoints on layer1 (central modules)
  for(Int_t module= fFirstL1; module<=fLastL1;module++){
    // Keep only central modules
    if(module%4==0 || module%4==3)continue;
    //   cout<<"Procesing module "<<module<<" ";
    branch->GetEvent(module);
    //    cout<<"Number of clusters "<<clusters->GetEntries()<<endl;
    nrpL1+= itsRec->GetEntries();
    detTypeRec.ResetRecPoints();
  }
  //By default fFirstL2=80 and fLastL2=239
  //This loop counts the number of RP on layer 2
  for(Int_t module= fFirstL2; module<=fLastL2;module++){
    branch->GetEvent(module);
    nrpL2+= itsRec->GetEntries();
    detTypeRec.ResetRecPoints();
  }
  if(nrpL1 == 0 || nrpL2 == 0){
    ResetHistograms();
    itsLoader->UnloadRecPoints();
    fCurrentVertex = new IlcESDVertex(0.,5.3,-2);
    return;
  }
  // The vertex finding is attempted only if the number of RP is !=0 on
  // both layers
  Float_t *xc1 = new Float_t [nrpL1]; // coordinates of the L1 Recpoints
  Float_t *yc1 = new Float_t [nrpL1];
  Float_t *zc1 = new Float_t [nrpL1];
  Float_t *phi1 = new Float_t [nrpL1];
  Float_t *xc2 = new Float_t [nrpL2]; // coordinates of the L1 Recpoints
  Float_t *yc2 = new Float_t [nrpL2];
  Float_t *zc2 = new Float_t [nrpL2];
  Float_t *phi2 = new Float_t [nrpL2];
  Int_t ind = 0;// running index for RP
  // Force a coarse bin size of 200 microns if the number of clusters on layer 2
  // is low
  if(nrpL2<fPPsetting[0])SetBinWidthCoarse(fPPsetting[1]);
  // By default nbinfine = (10+10)/0.0005=40000
  Int_t nbinfine = static_cast<Int_t>((fHighLim-fLowLim)/fStepFine);
  // By default nbincoarse=(10+10)/0.01=2000
  Int_t nbincoarse = static_cast<Int_t>((fHighLim-fLowLim)/fStepCoarse);
  // Set stepverycoarse = 3*fStepCoarse
  Int_t nbinvcoarse = static_cast<Int_t>((fHighLim-fLowLim)/(3.*fStepCoarse));
  if(fZCombc)delete fZCombc;
  fZCombc = new TH1F("fZCombc","Z",nbincoarse,fLowLim,fLowLim+nbincoarse*fStepCoarse);
  if(fZCombv)delete fZCombv;
  fZCombv = new TH1F("fZCombv","Z",nbinvcoarse,fLowLim,fLowLim+nbinvcoarse*3.*fStepCoarse);
  if(fZCombf)delete fZCombf;
  fZCombf = new TH1F("fZCombf","Z",nbinfine,fLowLim,fLowLim+nbinfine*fStepFine);
  // Loop on modules of layer 1 (restricted to central modules)
  for(Int_t module= fFirstL1; module<=fLastL1;module++){
    if(module%4==0 || module%4==3)continue;
    branch->GetEvent(module);
    Int_t nrecp1 = itsRec->GetEntries();
    for(Int_t j=0;j<nrecp1;j++){
      IlcTARGETRecPoint *recp = (IlcTARGETRecPoint*)itsRec->At(j);
      // Local coordinates of this recpoint
      lc[0]=recp->GetDetLocalX();
      lc[2]=recp->GetDetLocalZ();
      geom->LtoG(module,lc,gc);
      // Global coordinates of this recpoints
      gc[0]-=fX0; // Possible beam offset in the bending plane
      gc[1]-=fY0; //   "               "
      xc1[ind]=gc[0];
      yc1[ind]=gc[1];
      zc1[ind]=gc[2];
      // azimuthal angle is computed in the interval 0 --> 2*pi
      phi1[ind] = TMath::ATan2(gc[1],gc[0]);
      if(phi1[ind]<0)phi1[ind]=2*TMath::Pi()+phi1[ind];
      ind++;
    }
    detTypeRec.ResetRecPoints();
  }
  ind = 0; // the running index is reset for Layer 2
  for(Int_t module= fFirstL2; module<=fLastL2;module++){
    branch->GetEvent(module);
    Int_t nrecp2 = itsRec->GetEntries();
    for(Int_t j=0;j<nrecp2;j++){
      IlcTARGETRecPoint *recp = (IlcTARGETRecPoint*)itsRec->At(j);
      lc[0]=recp->GetDetLocalX();
      lc[2]=recp->GetDetLocalZ();
      geom->LtoG(module,lc,gc);
      gc[0]-=fX0;
      gc[1]-=fY0;
      xc2[ind]=gc[0];
      yc2[ind]=gc[1];
      zc2[ind]=gc[2];
      phi2[ind] = TMath::ATan2(gc[1],gc[0]);
      if(phi2[ind]<0)phi2[ind]=2*TMath::Pi()+phi2[ind];
      ind++;
    }
    detTypeRec.ResetRecPoints();
  }
 
  Int_t nolines=0;
  for(Int_t i=0;i<nrpL1;i++){ // loop on L1 RP
    Float_t r1=TMath::Sqrt(xc1[i]*xc1[i]+yc1[i]*yc1[i]); // radius L1 RP
    for(Int_t j=0;j<nrpL2;j++){ // loop on L2 RP
      Float_t diff = TMath::Abs(phi2[j]-phi1[i]); // diff in azimuth
      if(diff>TMath::Pi())diff=2.*TMath::Pi()-diff; //diff<pi
      if(diff<fDiffPhiMax){ // cut on 10 milliradians by def.
	Float_t r2=TMath::Sqrt(xc2[j]*xc2[j]+yc2[j]*yc2[j]); // radius L2 RP
	Float_t zr0=(r2*zc1[i]-r1*zc2[j])/(r2-r1); //Z @ null radius
	fZCombf->Fill(zr0);
	fZCombc->Fill(zr0);
	fZCombv->Fill(zr0);
	Double_t pA[3];
	Double_t pB[3];
	pA[0]=xc1[i];
	pA[1]=yc1[i];
	pA[2]=zc1[i];
	pB[0]=xc2[j];
	pB[1]=yc2[j];
	pB[2]=zc2[j];
	MakeTracklet(pA,pB,nolines);
      }
    }
  }
  delete [] xc1;
  delete [] yc1;
  delete [] zc1;
  delete [] phi1;
  delete [] xc2;
  delete [] yc2;
  delete [] zc2;
  delete [] phi2;
  Double_t contents = fZCombc->GetEntries()- fZCombc->GetBinContent(0)-fZCombc->GetBinContent(nbincoarse+1);
  if(contents<1.){
    //    Warning("FindVertexForCurrentEvent","Insufficient number of rec. points\n");
    ResetHistograms();
    itsLoader->UnloadRecPoints();
    fCurrentVertex = new IlcESDVertex(0.,5.3,-1);
    return;
  }
  /*
  else {
    if(fDebug>0)cout<<"Number of entries in hist. "<<fZCombc->GetEntries()<<endl;
  }
  */

  TH1F *hc = fZCombc;
  Bool_t goon = kFALSE;
  Int_t cnt = 0;
  Int_t bi;

  do {
    goon = kFALSE;
    cnt++;
    bi = hc->GetMaximumBin();   // bin with maximal content on coarse histogram
    if(hc->GetBinContent(bi)<3){
      if(cnt==1)goon = kTRUE;
      hc = fZCombv;
    }
  } while(goon);


  Float_t centre = hc->GetBinCenter(bi);  // z value of the bin with maximal content
  
  // evaluation of the centroid
  Int_t ii1=TMath::Max(bi-3,1);
  Int_t ii2=TMath::Min(bi+3,hc->GetNbinsX());
  centre = 0.;
  Int_t nn=0;
  for(Int_t ii=ii1;ii<ii2;ii++){
    centre+= hc->GetBinCenter(ii)*hc->GetBinContent(ii);
    nn+=static_cast<Int_t>(hc->GetBinContent(ii));
  }
  if (nn) centre/=nn;
  /*
  if(fDebug>0){
    cout<<"Value of center "<<centre<<endl;
    cout<<"Population of 3 central bins: "<<hc->GetBinContent(bi-1)<<", ";
    cout<<hc->GetBinContent(bi)<<", ";
    cout<<hc->GetBinContent(bi+1)<<endl;
  }
  */
  // n1 is the bin number of fine histogram containing the point located 1 coarse bin less than "centre"
  Int_t n1 = static_cast<Int_t>((centre-hc->GetBinWidth(bi)-fZCombf->GetBinLowEdge(0))/fStepFine);
  // n2 is the bin number of fine histogram containing the point located 1 coarse bin more than "centre"
  Int_t n2 = static_cast<Int_t>((centre+hc->GetBinWidth(bi)-fZCombf->GetBinLowEdge(0))/fStepFine);
  if(n1<1)n1=1;
  if(n2>nbinfine)n2=nbinfine;
  Int_t niter = 0;
  goon = kTRUE;
  Int_t num=0;
  Bool_t last = kFALSE;

  while(goon || last){
    fZFound = 0.;
    fZsig = 0.;
    num=0;
    // at the end of the loop:
    // fZFound = N*(Average Z) - where N is the number of tracklets
    // num=N
    // fZsig = N*Q - where Q is the average of Z*Z
    for(Int_t n=n1;n<=n2;n++){
      fZFound+=fZCombf->GetBinCenter(n)*fZCombf->GetBinContent(n);
      num+=static_cast<Int_t>(fZCombf->GetBinContent(n));
      fZsig+=fZCombf->GetBinCenter(n)*fZCombf->GetBinCenter(n)*fZCombf->GetBinContent(n);
    }
    if(num<2){
      fZsig = 5.3;  // Default error from the beam sigmoid
    }
    else{
      Float_t radi =  fZsig/(num-1)-fZFound*fZFound/num/(num-1);
      // radi = square root of sample variance of Z
      if(radi>0.)fZsig=TMath::Sqrt(radi);
      else fZsig=5.3;  // Default error from the beam sigmoid
      // fZfound - Average Z
      fZFound/=num;
    }
    if(!last){
      // goon is true if the distance between the found Z and the lower bin differs from the distance between the found Z and
      // the upper bin by more than tolerance (0.002)
      goon = TMath::Abs(TMath::Abs(fZFound-fZCombf->GetBinCenter(n1))-TMath::Abs(fZFound-fZCombf->GetBinCenter(n2)))>fTolerance;
      // a window in the fine grained histogram is centered aroung the found Z. The width is 2 bins of
      // the coarse grained histogram
      if(num>0){
	n1 = static_cast<Int_t>((fZFound-hc->GetBinWidth(bi)-fZCombf->GetBinLowEdge(0))/fStepFine);
	if(n1<1)n1=1;
	n2 = static_cast<Int_t>((fZFound+hc->GetBinWidth(bi)-fZCombf->GetBinLowEdge(0))/fStepFine);
	if(n2>nbinfine)n2=nbinfine;
	/*
	  if(fDebug>0){
	  cout<<"Search restricted to n1 = "<<n1<<", n2= "<<n2<<endl;
	  cout<<"z1= "<<fZCombf->GetBinCenter(n1)<<", n2 = "<<fZCombf->GetBinCenter(n2)<<endl;
	  }
	*/
      }
      else {
	n1 = static_cast<Int_t>((centre-(niter+2)*hc->GetBinWidth(bi)-fZCombf->GetBinLowEdge(0))/fStepFine);
	n2 = static_cast<Int_t>((centre+(niter+2)*hc->GetBinWidth(bi)-fZCombf->GetBinLowEdge(0))/fStepFine);
	if(n1<1)n1=1;
	if(n2>nbinfine)n2=nbinfine;
      }
      niter++;
      // no more than 10 adjusting iterations
      if(niter>=10){
	goon = kFALSE;
	//	Warning("FindVertexForCurrentEvent","The procedure does not converge\n");
      }

      if((fZsig> 0.0001) && !goon && num>5){
	last = kTRUE;
	n1 = static_cast<Int_t>((fZFound-fZsig-fZCombf->GetBinLowEdge(0))/fZCombf->GetBinWidth(0));
	if(n1<1)n1=1;
	n2 = static_cast<Int_t>((fZFound+fZsig-fZCombf->GetBinLowEdge(0))/fZCombf->GetBinWidth(0));
	if(n2>nbinfine)n2=nbinfine;
	/*
	if(fDebug>0){
	  cout<<"FINAL Search restricted to n1 = "<<n1<<", n2= "<<n2<<endl;
	  cout<<"z1= "<<fZCombf->GetBinCenter(n1)<<", n2 = "<<fZCombf->GetBinCenter(n2)<<endl;
	}
	*/
      }
    }
    else {
      last = kFALSE;
    }

  }
  //  if(fDebug>0)cout<<"Numer of Iterations "<<niter<<endl<<endl;
  //  if(num!=0)fZsig/=TMath::Sqrt(num);
  if (fZsig<=0) fZsig=5.3; // Default error from the beam sigmoid
  fCurrentVertex = new IlcESDVertex(fZFound,fZsig,num);
  fCurrentVertex->SetTitle("vertexer: B");
  ResetHistograms();
  itsLoader->UnloadRecPoints();
  return;
}

//_____________________________________________________________________
void IlcTARGETVertexerZ::ResetHistograms(){
  // delete TH1 data members
  if(fZCombc)delete fZCombc;
  if(fZCombf)delete fZCombf;
  if(fZCombv)delete fZCombv;
  fZCombc = 0;
  fZCombf = 0;
  fZCombv = 0;
}

//______________________________________________________________________
void IlcTARGETVertexerZ::FindVertices(){
  // computes the vertices of the events in the range FirstEvent - LastEvent
  IlcRunLoader *rl = IlcRunLoader::Instance();
  IlcTARGETLoader* itsLoader =  (IlcTARGETLoader*) rl->GetLoader("TARGETLoader");
  itsLoader->ReloadRecPoints();
  for(Int_t i=fFirstEvent;i<=fLastEvent;i++){
    //  cout<<"Processing event "<<i<<endl;
    rl->GetEvent(i);
    FindVertexForCurrentEvent(i);
    if(fCurrentVertex){
      WriteCurrentVertex();
    }
    /*
    else {
      if(fDebug>0){
	cout<<"Vertex not found for event "<<i<<endl;
	cout<<"fZFound = "<<fZFound<<", fZsig= "<<fZsig<<endl;
      }
    }
    */
  }
}

//________________________________________________________
void IlcTARGETVertexerZ::PrintStatus() const {
  // Print current status
  cout <<"=======================================================\n";
  cout <<" First layer first and last modules: "<<fFirstL1<<", ";
  cout <<fLastL1<<endl;
  cout <<" Second layer first and last modules: "<<fFirstL2<<", ";
  cout <<fLastL2<<endl;
  cout <<" Max Phi difference: "<<fDiffPhiMax<<endl;
  cout <<"Limits for Z histograms: "<<fLowLim<<"; "<<fHighLim<<endl;
  cout <<"Bin sizes for coarse and fine z histos "<<fStepCoarse<<"; "<<fStepFine<<endl;
  cout <<" Current Z "<<fZFound<<"; Z sig "<<fZsig<<endl;
  cout <<" Debug flag: "<<IlcDebugLevelClass()<<endl;
  cout <<"First event to be processed "<<fFirstEvent;
  cout <<"\n Last event to be processed "<<fLastEvent<<endl;
  if(fZCombc){
    cout<<"fZCombc exists - entries="<<fZCombc->GetEntries()<<endl;
  }
  else{
    cout<<"fZCombc does not exist\n";
  }
  if(fZCombv){
    cout<<"fZCombv exists - entries="<<fZCombv->GetEntries()<<endl;
  }
  else{
    cout<<"fZCombv does not exist\n";
  }
   if(fZCombf){
    cout<<"fZCombf exists - entries="<<fZCombv->GetEntries()<<endl;
  }
  else{
    cout<<"fZCombf does not exist\n";
  }
 
  cout <<"=======================================================\n";
}

