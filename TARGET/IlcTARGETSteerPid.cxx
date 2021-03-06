/////////////////////////////////////////////////////////////////////////
//Steering Class for PID in the TARGET                                      //
//The PID is based on the likelihood of all the four TARGET' layers,        //
//without using the truncated mean for the dE/dx. The response           //
//functions for each layer are convoluted Landau-Gaussian functions.     // 
// Origin: Elena Bruna bruna@to.infn.it, Massimo Masera masera@to.infn.it//
/////////////////////////////////////////////////////////////////////////

#include "IlcTARGETSteerPid.h"
#include "TMath.h"
#include "TClonesArray.h"

ClassImp(IlcTARGETSteerPid)

  //______________________________________________________________
  IlcTARGETSteerPid::IlcTARGETSteerPid(){
  // default constructor
  fClonarr2=0;
  fVect2=0;
  fItem=0;
  fFitTree=0;
}
//______________________________________________________________
IlcTARGETSteerPid::~IlcTARGETSteerPid(){
  // destructor
  delete fClonarr2;
  delete fFitTree;
}

//______________________________________________________________________
IlcTARGETSteerPid::IlcTARGETSteerPid(const IlcTARGETSteerPid &ob) :TObject(ob) {
  // Copy constructor
  // Copies are not allowed. The method is protected to avoid misuse.
  Error("IlcTARGETSteerPid","Copy constructor not allowed\n");
}

//______________________________________________________________________
IlcTARGETSteerPid& IlcTARGETSteerPid::operator=(const IlcTARGETSteerPid& /* ob */){
  // Assignment operator
  // Assignment is not allowed. The method is protected to avoid misuse.
  Error("= operator","Assignment operator not allowed\n");
  return *this;
}

//______________________________________________________________
void IlcTARGETSteerPid::InitLayer(TString fileTARGET,TString fileFitPar){
  // it opens the files useful for the PID 
  fClonarr2=new TFile (fileTARGET,"r");
  fVect2=(TClonesArray*)fClonarr2->Get("vectfitits_0");//truncated mean
  fVect2lay1=(TClonesArray*)fClonarr2->Get("vectfitits_1");//lay 1
  fVect2lay2=(TClonesArray*)fClonarr2->Get("vectfitits_2");//lay 2
  fVect2lay3=(TClonesArray*)fClonarr2->Get("vectfitits_3");//lay 3
  fVect2lay4=(TClonesArray*)fClonarr2->Get("vectfitits_4");//lay 4
 
  TFile *fFitPar=new TFile (fileFitPar);
  fFitTree=(TTree*)fFitPar->Get("tree");
 
}

//______________________________________________________________
IlcTARGETPidParItem* IlcTARGETSteerPid::GetItemLayer(Int_t nolay,Float_t mom){
    // it gives an IlcTARGETPidParItem object for a given momentum and TARGET layer
  if(nolay==1) return Item(fVect2lay1,mom);
  if(nolay==2) return Item(fVect2lay2,mom);
  if(nolay==3) return Item(fVect2lay3,mom);
  if(nolay==4) return Item(fVect2lay4,mom);
  if(nolay!=1&&nolay!=2&&nolay!=3&&nolay!=4) {
    fItem=new IlcTARGETPidParItem();
    return fItem;

  }
  return 0;
}

//______________________________________________________________
void IlcTARGETSteerPid::GetParFitLayer(Int_t nolay,Float_t mom,Double_t *parp,Double_t *park,Double_t *parpi){
  //it gives the parameters of the convoluted functions (WL, MP, WG) for 
  //protons, kaons and pions for a given momentum and TARGET layer

  Double_t parfit0pro[3]={0,0,0};
  Double_t parfit1pro[3]={0,0,0};
  Double_t parfit3pro[3]={0,0,0};
  Double_t parfit0kao[3]={0,0,0};
  Double_t parfit1kao[3]={0,0,0};
  Double_t parfit3kao[3]={0,0,0};
  Double_t parfit0pi[3]={0,0,0};
  Double_t parfit1pi[3]={0,0,0};
  Double_t parfit3pi[3]={0,0,0};
 
  fFitTree->SetBranchAddress("par0pro",parfit0pro);
  fFitTree->SetBranchAddress("par1pro",parfit1pro);
  fFitTree->SetBranchAddress("par3pro",parfit3pro);

  fFitTree->SetBranchAddress("par0kao",parfit0kao);
  fFitTree->SetBranchAddress("par1kao",parfit1kao);
  fFitTree->SetBranchAddress("par3kao",parfit3kao);

  fFitTree->SetBranchAddress("par0pi",parfit0pi);
  fFitTree->SetBranchAddress("par1pi",parfit1pi);
  fFitTree->SetBranchAddress("par3pi",parfit3pi);
  fFitTree->GetEvent(nolay);
 
  GetLangausProPars(mom,parfit0pro,parfit1pro,parfit3pro,parp);
  GetLangausKaoPars(mom,parfit0kao,parfit1kao,parfit3kao,park);
  GetLangausPiPars(mom,parfit0pi,parfit1pi,parfit3pi,parpi);


}//______________________________________________________________
void IlcTARGETSteerPid::GetLangausProPars(Float_t mom,Double_t *parfit0,Double_t *parfit1,Double_t *parfit3,Double_t *par){
 
  //It finds the parameters of the convoluted Landau-Gaussian response 
  //function for protons (Width Landau, Most Probable, Width Gaussian)
  par[0]=parfit0[0]+parfit0[1]/mom;
  par[1]=parfit1[0]/(mom*mom)+parfit1[1]/(mom*mom)*TMath::Log(mom*mom)+parfit1[2];
  par[2]=parfit3[0]/(mom*mom)+parfit3[1]/(mom*mom)*TMath::Log(mom*mom)+parfit3[2];
}
//______________________________________________________________
void IlcTARGETSteerPid::GetLangausKaoPars(Float_t mom,Double_t *parfit0,Double_t *parfit1,Double_t *parfit3,Double_t *par){
  // It finds the parameters of the convoluted Landau-Gaussian response 
  //function for kaons (Width Landau, Most Probable, Width Gaussian)

  par[0]=parfit0[0]+parfit0[1]/(mom*mom);
  par[1]=parfit1[0]/(mom*mom)+parfit1[1]/(mom*mom)*TMath::Log(mom*mom)+parfit1[2];
  par[2]=parfit3[0]/(mom*mom)+parfit3[1]/(mom*mom)*TMath::Log(mom*mom)+parfit3[2];
}

//______________________________________________________________
void IlcTARGETSteerPid::GetLangausPiPars(Float_t mom,Double_t *parfit0,Double_t *parfit1,Double_t *parfit3,Double_t *par){
  //It finds the parameters of the convoluted Landau-Gaussian response 
  //function for pions (Width Landau, Most Probable, Width Gaussian)

  par[0]=parfit0[0]/(mom*mom)+parfit0[1]/(mom*mom)*TMath::Log(mom*mom)+parfit0[2];
  par[1]=parfit1[0]/(mom)+parfit1[1]/(mom)*TMath::Log(mom*mom)+parfit1[2];
  par[2]=parfit3[0]/(mom*mom)+parfit3[1]/(mom*mom)*TMath::Log(mom*mom)+parfit3[2];
}



//______________________________________________________________
IlcTARGETPidParItem* IlcTARGETSteerPid::Item(TClonesArray *Vect,Float_t mom){

  //it gives an IlcTARGETPidParItem object taken from the TClonesArray. 
  Int_t mybin=-1;

  IlcTARGETPidParItem* punt;
  
  for (Int_t a=0;a<50;a++){
    punt=(IlcTARGETPidParItem*)Vect->At(a);
    Float_t centerp=punt->GetMomentumCenter(); 
    Float_t widthp=punt->GetWidthMom();
    if (mom>(centerp-widthp/2) && mom<=(centerp+widthp/2)) mybin=a; 
  }
  if (mybin!=-1) fItem=(IlcTARGETPidParItem*)Vect->At(mybin);
  else {
    fPCenter=0;
    fPWidth=0;
    for (Int_t ii=0;ii<52;ii++) fBuff[ii]=0;
    fItem = new IlcTARGETPidParItem(fPCenter,fPWidth,fBuff);
  }
  
  return fItem;



}
