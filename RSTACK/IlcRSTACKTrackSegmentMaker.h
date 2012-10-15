#ifndef ILCRSTACKTRACKSEGMENTMAKER_H
#define ILCRSTACKTRACKSEGMENTMAKER_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* $Id: IlcRSTACKTrackSegmentMaker.h 20956 2007-09-26 14:22:18Z cvetan $ */

/* History of cvs commits:
 *
 * $Log$
 * Revision 1.43  2007/08/28 12:55:08  policheh
 * Loaders removed from the reconstruction code (C.Cheshkov)
 *
 * Revision 1.42  2007/08/07 14:12:03  kharlov
 * Quality assurance added (Yves Schutz)
 *
 * Revision 1.41  2007/07/11 13:43:30  hristov
 * New class IlcESDEvent, backward compatibility with the old IlcESD (Christian)
 *
 * Revision 1.40  2006/08/29 11:41:19  kharlov
 * Missing implementation of ctors and = operator are added
 *
 * Revision 1.39  2006/08/25 16:00:53  kharlov
 * Compliance with Effective C++IlcRSTACKHit.cxx
 *
 * Revision 1.38  2005/05/28 14:19:05  schutz
 * Compilation warnings fixed by T.P.
 *
 */

//_________________________________________________________________________
// Algorithm Base class to construct RSTACK track segments
// Associates EMC and CPV clusters
// Unfolds the EMC cluster   
//                  
//*-- Author: Dmitri Peressounko (RRC Kurchatov Institute  & SUBATECH)

// --- ROOT system ---
#include <TObject.h>
class TTree;

// --- IlcRoot header files ---
class IlcRSTACKGeometry ;
class IlcESDEvent ;

class  IlcRSTACKTrackSegmentMaker : public TObject {

public:

  IlcRSTACKTrackSegmentMaker();
  IlcRSTACKTrackSegmentMaker(IlcRSTACKGeometry *geom);
  IlcRSTACKTrackSegmentMaker(const IlcRSTACKTrackSegmentMaker & tsmaker) ;
  virtual ~ IlcRSTACKTrackSegmentMaker() ;
  IlcRSTACKTrackSegmentMaker & operator = (const IlcRSTACKTrackSegmentMaker & /*rvalue*/)  {
    Fatal("operator =", "not implemented") ; return *this ; }

  virtual void   Clusters2TrackSegments(Option_t *option) = 0;

  void    SetInput(TTree *clustersTree);

  virtual void    Print(const Option_t * = "")const {Warning("Print", "Not Defined" ) ; }

  void SetESD(IlcESDEvent *esd) { fESD = esd; }

  IlcESDEvent *GetESD()             const {return fESD;            }

  virtual TClonesArray * GetTrackSegments() const = 0;

protected:

  IlcESDEvent * fESD;              //! ESD object
  IlcRSTACKGeometry *fGeom;           //! Pointer to the RSTACK geometry
  TObjArray *fEMCRecPoints;         //  Array with the EMC clusters
  TObjArray *fCPVRecPoints;         //  Array with the CPV clusters

  ClassDef( IlcRSTACKTrackSegmentMaker,6)  // Algorithm class to make RSTACK track segments (Base Class)
};

#endif // ILCRSTACKTRACKSEGMENTMAKER_H
