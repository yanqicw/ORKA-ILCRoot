#ifndef ILCRSTACKGRIDFILE_H
#define ILCRSTACKGRIDFILE_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* History of cvs commits:
 *
 * $Log$
 * Revision 1.2  2005/05/28 14:19:04  schutz
 * Compilation warnings fixed by T.P.
 *
 */

//_________________________________________________________________________
// Description of logical filename in Grid catalogue
// check here : /afs/cern.ch/user/p/peters/public/README.GRID 
//*--                  
//*-- Author: Yves Schutz (CERN)

// --- ROOT system ---
#include "TObject.h" 
#include "TString.h" 
#include "TGrid.h" 

// --- IlcRoot header files ---

class IlcRSTACKGridFile : public TObject {

 public:
  
  IlcRSTACKGridFile(TString grid="alien") ; 
  IlcRSTACKGridFile(const IlcRSTACKGridFile & lfn);
  
  virtual ~IlcRSTACKGridFile(void) ; 

  virtual void Copy(TObject & obj) const;
  
  void ListEvents() const ; 
  void ListRuns() const ; 
  TString GetRootDir() const { return fRoot ; }
  TString GetLFN() const ; 
  void Help() ; 
  Bool_t IsConnected() const {
    Bool_t rv = kFALSE ; 
    if(fGrid) 
      rv = kTRUE ; 
    return rv ; }
  Bool_t SetYearProd(TString year, TString prod) ; 
  Bool_t SetVers(TString vers) ; 
  Bool_t SetType(TString type) ; 

  Bool_t SetPath(TString year, TString prod, TString vers, TString type) ; 

  Bool_t SetRun(Int_t run) ; 
  Bool_t SetEvt(Int_t evt) ; 

  TString Pwd() const { return fPath ; }

  IlcRSTACKGridFile & operator = (const IlcRSTACKGridFile & /*rvalue*/)  {
    // assignement operator requested by coding convention but not needed
    Fatal("operator =", "not implemented") ;
    return *this ; 
  }
  
private:
  
  TGrid * fGrid ; //! connection to alien data catalogue 
  TString fRoot ; //! root directory
  TString fYear ; //! year of the DC 
  TString fProd ; //! production id 
  TString fVers ; //! ilcroot tag version
  TString fType ; //! event type
  TString fRun  ; //! run number
  TString fEvt  ; //! event number
  TString fPath ; //! the lfn is fRoot/fYear/fProd/fVers/fType/fRun/fEvt
 
  ClassDef(IlcRSTACKGridFile,1)  
};

#endif // IlcRSTACKGRIDFILE_H
 
