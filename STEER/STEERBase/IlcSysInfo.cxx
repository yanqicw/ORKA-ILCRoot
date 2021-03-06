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


//
// Origin:  marian.ivanov@cern.ch
//
//  Make   a log file for the CPU and Memory usage
//
//  Principles:
//  Snapshots of the system information are writen to the text log files.
//  Text files were chosen in order to get the ouptu also in case code 
//  is crashing. 
//  Following information is stored in the log file:
//  TTimeStamp stamp;
//  CpuInfo_t  cpuInfo;
//  MemInfo_t  memInfo;
//  ProcInfo_t procInfo;
// 
//  Root TSystem is used to retrieve this information:
//  gSystem->GetCpuInfo(&cpuInfo, 10);
//  gSystem->GetMemInfo(&memInfo);
//  gSystem->GetProcInfo(&procInfo);
//  for details see:
//  http://root.cern.ch/root/html/TUnixSystem.html
//  http://root.cern.ch/root/html/ProcInfo_t.html
//  http://root.cern.ch/root/html/MemInfo_t.html
//  http://root.cern.ch/root/html/CpuInfo_t.html
//  -------------------------------------------------------------------
//  class CpuInfo_t
//   Float_t	fIdle	cpu idle percentage
//   Float_t	fLoad15m	cpu load average over 15 m
//   Float_t	fLoad1m	cpu load average over 1 m
//   Float_t	fLoad5m	cpu load average over 5 m
//   Float_t	fSys	cpu sys load in percentage
//   Float_t	fTotal	cpu user+sys load in percentage
//   Float_t	fUser	cpu user load in percentage

//  -------------------------------------------------------------------
//  class ProcInfo_t:
//  Float_t	fCpuSys	system time used by this process in seconds
//  Float_t	fCpuUser	user time used by this process in seconds
//  Long_t	fMemResident	resident memory used by this process in KB
//  Long_t	fMemVirtual	virtual memory used by this process in KB
//  -------------------------------------------------------------------


//  The information from the IlcSysInfo can be used as measurement
//  of the code quality. Be aware of the limitation induced by
//  using only system info described in the  IlcSysInfo::Test() function
// 
//  The example usage of the IlcSysInfo is shown in the
//  IlcSysInfo::Test() example.
//  
//  
//
//  
//  The typical usage in the IlcRoot code:
//  Make a set of stamps in the code in the place of interest
//  e.g. 
//
//  IlcSysInfo::AddStamp("Start");
//
//  loader->UnloadRecPoints();
//    IlcSysInfo::AddStamp(Form("LRec%s_%d",fgkDetectorName[iDet],eventNr), iDet,1,eventNr);
//

// The log file can be transformed to the tree - to make a visualization
// See $ILC_ROOT/macros/PlotSysInfo.C as an example


#include <Riostream.h>
//#include "IlcLog.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "TTree.h"
#include "TFile.h"

#include "TTimeStamp.h"
#include "IlcSysInfo.h"
#include "TBufferFile.h"

//#include "TMemStatManager.h"  //USE IFDEF


ClassImp(IlcSysInfo)

IlcSysInfo* IlcSysInfo::fInstance=0;

IlcSysInfo::IlcSysInfo():
    TObject(),
    fSysWatch(0),
    fTimer(0),
    fMemStat(0),
    fCallBackFunc(0),
    fNCallBack(0)
{
  fTimer = new TStopwatch;
  fSysWatch  = new fstream("syswatch.log", ios_base::out|ios_base::trunc);

  //hname/C:sname/C:sec/D:mI.fMemUsed/F:mI.fSwapUsed/F:pI.fMemResident/F:pI.fMemVirtual/F:cI.fUser/F:cI.fSys/F:cI.fCpuUser/F:pI.fCpuSys/F


  (*fSysWatch) <<"hname"<<"/C:"               // hname - hostname  
               <<"sname"<<"/C:"              // stamp name
               <<"id0"<<"/I:"                // 0 id
               <<"id1"<<"/I:"                // 1 id
               <<"id2"<<"/I:"                // 1 id
               <<"first"<<"/D:"              // first stamp
    //
	       <<"stampSec"<<"/D:"         // time  - time stamp in seconds
	       <<"mi.fMemUsed"<<"/D:"       // system info 
	       <<"mi.fSwapUsed"<<"/D:"      //
	       <<"cI.fUser"<<"/D:"         //
	       <<"cI.fSys"<<"/D:"         //
    // 
	       <<"pI.fMemResident"<<"/D:"  // process info
	       <<"pI.fMemVirtual"<<"/D:"   //    
	       <<"pI.fCpuUser"<<"/D:"      //
	       <<"pI.fCpuSys"<<"/D:"       //
    //
    	       <<"stampOldSec"<<"/D:"         // time  - time stamp in seconds
	       <<"miOld.fMemUsed"<<"/D:"       // system info - previous
	       <<"miOld.fSwapUsed"<<"/D:"      //
	       <<"cIOld.fUser"<<"/D:"         //
	       <<"cIOld.fSys"<<"/D:"         //
    // 
	       <<"pIOld.fMemResident"<<"/D:"  // process info -previous
	       <<"pIOld.fMemVirtual"<<"/D:"   //    
	       <<"pIOld.fCpuUser"<<"/D:"      //
	       <<"pIOld.fCpuSys"<<"/D:"       //
    // 
	       <<"fileBytesRead"<<"/D:"       // file IO information
	       <<"fileBytesWritten"<<"/D:"    //    
	       <<"fileCounter"<<"/D:"         //
	       <<"fileReadCalls"<<"/D"        //
	       << endl;
  
}




IlcSysInfo * IlcSysInfo::Instance(){
  //
  //
  //
  if (!fInstance){
    fInstance = new IlcSysInfo;
  }
  return fInstance;
}


void IlcSysInfo::AddStamp(const char *sname, Int_t id0, Int_t id1, Int_t id2){
  //
  // 
  //
  //
  TTimeStamp stamp;
  CpuInfo_t  cpuInfo;
  MemInfo_t  memInfo;
  ProcInfo_t procInfo;
#ifdef WIN32
  gSystem->GetCpuInfo(&cpuInfo, 100);		// it likes to crash in windows if time sample is too small
#else
  gSystem->GetCpuInfo(&cpuInfo, 10);
#endif
  gSystem->GetMemInfo(&memInfo);
  gSystem->GetProcInfo(&procInfo);
  //  procInfo.fMemVirtual/=1024;  //size in MBy
  //procInfo.fMemResident/=1024;  //size in MBy

  const char * hname = gSystem->HostName();

  static Int_t entry=0;
  static Double_t  first=stamp.GetSec()+stamp.GetNanoSec()/1000000000.;
  //
  static TTimeStamp stampOld;
  static CpuInfo_t  cpuInfoOld;
  static MemInfo_t  memInfoOld;
  static ProcInfo_t procInfoOld;  
  Double_t fileBytesRead    = TFile::GetFileBytesRead();
  Double_t fileBytesWritten = TFile::GetFileBytesWritten();
  Double_t fileCounter      = TFile::GetFileCounter();
  Double_t fileReadCalls    = TFile::GetFileReadCalls();


  (*(Instance()->fSysWatch)) 
    << hname   <<"\t"               // hname - hostname  
    << sname    <<"\t"              // stamp name
    << id0      <<"\t"
    << id1      <<"\t"
    << id2      <<"\t"
    <<setprecision(15)<< first    <<"\t"              // first stamp               
    //
    <<setprecision(15)<< stamp.GetSec()+stamp.GetNanoSec()/1000000000.<<"\t"         // time  - time stamp in seconds
    << memInfo.fMemUsed<<"\t"       // system info 
    << memInfo.fSwapUsed<<"\t"      //
    << cpuInfo.fUser <<"\t"         //
    << cpuInfo.fSys  <<"\t"         //
    // 
    <<setprecision(15)<< procInfo.fMemResident/1024.<<"\t"  // process info
    <<setprecision(15)<< procInfo.fMemVirtual/1024.<<"\t"   //    
    << procInfo.fCpuUser<<"\t"      //
    << procInfo.fCpuSys<<"\t"       //
    //
    <<setprecision(15)<< stampOld.GetSec()+stampOld.GetNanoSec()/1000000000.<<"\t"         // time  - time stamp in seconds
    << memInfoOld.fMemUsed<<"\t"       // system info - previous
    << memInfoOld.fSwapUsed<<"\t"      //
    << cpuInfoOld.fUser <<"\t"         //
    << cpuInfoOld.fSys  <<"\t"         //
    // 
    <<setprecision(15)<< procInfoOld.fMemResident/1024.<<"\t"  // process info -previous
    <<setprecision(15)<< procInfoOld.fMemVirtual/1024.<<"\t"   //    
    << procInfoOld.fCpuUser<<"\t"      //
    << procInfoOld.fCpuSys<<"\t"       //
    //
    <<fileBytesRead<<"\t"           // file IO information
    <<fileBytesWritten<<"\t"        //    
    <<fileCounter<<"\t"             //
    <<fileReadCalls<<"\t"            //
    << endl;

  stampOld   = stamp;
  cpuInfoOld = cpuInfo;
  memInfoOld = memInfo;
  procInfoOld= procInfo;

  //  if (fInstance->fMemStat) fInstance->fMemStat->AddStamps(sname);
  for (Int_t icallback=0; icallback<Instance()->fNCallBack; icallback++){
    Instance()->fCallBackFunc[icallback](sname);
  }
  entry++;
}


TTree * IlcSysInfo::MakeTree(const char *lname){
  // char * lname = "syswatch.log"
  TTree * tree = new TTree;
  tree->ReadFile(lname);
  tree->SetAlias("deltaT","stampSec-stampOldSec");
  tree->SetAlias("T","stampSec-first");
  tree->SetAlias("deltaVM","(pI.fMemVirtual-pIOld.fMemVirtual)");
  tree->SetAlias("VM","pI.fMemVirtual");
  return tree;
}


Bool_t IlcSysInfo::Contain(const char * str1, const char * str2){
  //
  //
  //
  TString str(str1);
  return str.Contains(str2);
}



void IlcSysInfo::OpenMemStat(){
  //
  //
  //
  //USE IFDEF if MEMSTAT ENABLED  
  //  Instance()->fMemStat = TMemStatManager::GetInstance();
  //   Instance()->fMemStat->SetAutoStamp(10000000, 10000000,1000000);
  //   Instance()->fMemStat->Enable();  
}

void IlcSysInfo::CloseMemStat(){
  //
  //
  //
  //USE IFDEF if MEMSTAT ENABLED
  //if (Instance()->fMemStat  == TMemStatManager::GetInstance()) Instance()->fMemStat->Close();
  //Instance()->fMemStat=0;
}



void IlcSysInfo::AddCallBack(StampCallback_t callback){
  //
  // add cal back function
  //
  IlcSysInfo *info =  Instance();
  if (!info->fCallBackFunc)
    info->fCallBackFunc = new StampCallback_t[100];
  info->fCallBackFunc[info->fNCallBack]=callback;
  info->fNCallBack++;
}



TTree*  IlcSysInfo::Test(){
  //
  // Test example for IlcSysInfo:
  // 1. Make huge memory leak
  // 2. Slow down execution
  /*
    To use the test:
    TTree * tree = IlcSysInfo::Test();
    // Make alias what we set as input
    tree->SetAlias("deltaVMIn","(id0*100000+id1*10000+id2*1000)/1000000.")
    tree->SetAlias("deltaVM","(pI.fMemVirtual-pIOld.fMemVirtual)");
    tree->SetAlias("deltaTIn","(id1+id0*10)");
    tree->SetAlias("deltaT","stampSec-stampOldSec");
    //
    tree->SetMarkerStyle(23); tree->SetMarkerSize(0.5);
    // Memory usage
    tree->Draw("deltaVM:deltaVMIn","Entry$>0");
    // or alternative
    tree->Draw("deltaVM:deltaVMIn","Entry$>0","prof");
    //
    // draw time usage
    tree->Draw("deltaT:deltaTIn","Entry$>0"); 
  */
  //
  // The delta of VM as obtained from the IlcSysInfo starts to be proportional
  // to  the input allocation after 0.12 MBy (and it is system dependent) 
  // Bellow these limit the deltaVM can be used only in mean.
  // (compare first and  profile histogram)
  for (Int_t id0=0; id0<5; id0++)
    for (Int_t id1=1; id1<10; id1++)
      for (Int_t id2=0; id2<20; id2++){
	new Char_t[id2*1000+id1*10000+id0*100000];  // emulate memory leak
	gSystem->Sleep(id1+id0*10);         // emulate CPU usage 
	IlcSysInfo::AddStamp("Leak",id0,id1,id2);
      }
  TTree * tree = IlcSysInfo::MakeTree("syswatch.log");
  return tree;  
}

Double_t IlcSysInfo::EstimateObjectSize(TObject* object){
  //
  // Estimate size of object as represented in the memory size in bytes
  // Warnings:
  //  1. Only data memebrs which are persistent are counted
  //  2. Local copy of the object is temporary created in memory
  //  3. Do not use it in standard programs, time and memory consument procedure
  //
  if (!object) return 0;
  TBufferFile * file = new TBufferFile(TBuffer::kWrite);
  file->WriteObject(object);
  Double_t size=file->Length();
  delete file;
  return size;
}
