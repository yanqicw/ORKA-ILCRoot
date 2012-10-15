#ifndef ILCPVECSURVEY_H
#define ILCPVECSURVEY_H

/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* $Id: IlcPVECSurvey.h 20504 2007-08-28 13:12:19Z hristov $ */

/* History of cvs commits:
 *
 * $Log$
 * Revision 1.3  2007/07/10 12:41:38  kharlov
 * Added a new class IlcPVECSurvet1 which read survey data from EDMS files
 *
 * Revision 1.2  2007/05/17 17:13:32  kharlov
 * Coding convensions satisfied (T.Pocheptsov)
 *
 * Revision 1.1  2007/04/19 15:47:20  kharlov
 * Add misalignment of strip units with IlcPVECSurvey class
 *
 */

#include <TObject.h>
#include <Rtypes.h>

class TClonesArray;
class TString;

class IlcPVECGeometry;

/*
  Objects of this class read txt file with survey (photogrammetry) data
  and convert the data into IlcAlignObjParams of alignable PVEC volumes.
  It can be used as a base class, you need to override GetStripTransformation.
  IlcPVECSurvey inherits TObject only to use IlcLog "functions".
*/

class IlcPVECSurvey : public TObject {
public:
  IlcPVECSurvey();
  IlcPVECSurvey(const TString &txtFileName);

  virtual ~IlcPVECSurvey();

  //Create IlcAlignObjParams for strips.
  void CreateIlcAlignObjParams(TClonesArray &array);
  //Create IlcAlignObjParams with null shifts and rotations.
  void CreateNullObjects(TClonesArray &alObj, const IlcPVECGeometry *geom)const;

protected:

  struct IlcPVECStripDelta {
    Float_t fXShift; //x shift
    Float_t fYShift; //y shift
    Float_t fZShift; //z shift
    Float_t fPsi;    //psi
    Float_t fTheta;  //theta
    Float_t fPhi;    //phi
  };

  Int_t 	           fStrNum; // Number of strips.
  IlcPVECStripDelta *fStripData; // Strip unit transformation data

  void InitStripData(const Double_t *xReal, const Double_t *zReal);

private:
  //Calculate shifts and rotations for strip number stripIndex in a module moduleIndex.
  virtual IlcPVECStripDelta GetStripTransformation(Int_t stripIndex, Int_t moduleIndex)const;

  IlcPVECSurvey(const IlcPVECSurvey &);
  IlcPVECSurvey &operator = (const IlcPVECSurvey &);

  ClassDef(IlcPVECSurvey, 1) //Survey data reader
};

#endif
