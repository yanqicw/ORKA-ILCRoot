#ifndef ILCFIELDMAP_H
#define ILCFIELDMAP_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

/* $Id: IlcFieldMap.h 30421 2008-12-16 15:42:04Z shahoian $ */

//
// Class to handle the field map of ILC
// I/O and interpolation
// Author: Andreas Morsch <andreas.morsch@cern.ch>
//

#include <TNamed.h>
#include <TVector.h>

class IlcFieldMap : public TNamed
{
  //Ilc Magnetic Field with constant mesh

public:
    IlcFieldMap();
    IlcFieldMap(const char *name, const char *title);
    IlcFieldMap(const IlcFieldMap &mag);
    virtual ~IlcFieldMap();
    void Copy(TObject &map) const;
    virtual IlcFieldMap & operator=(const IlcFieldMap &map);
    virtual void Field(float *x, float *b) const;
    virtual void Field(double *x, double *b) const;
    Float_t Bx(Int_t ix, Int_t iy, Int_t iz) const{
	return (*fB)(3*((ix*fYn+iy)*fZn+iz));
    }
    Float_t By(Int_t ix, Int_t iy, Int_t iz) const{
	return (*fB)(3*((ix*fYn+iy)*fZn+iz)+1);
    }
    Float_t Bz(Int_t ix, Int_t iy, Int_t iz) const{
	return (*fB)(3*((ix*fYn+iy)*fZn+iz)+2);
    }

    Bool_t Inside(Float_t x, Float_t y, Float_t z) const
	{ return (x > fXbeg && x < fXend &&
		  y > fYbeg && y < fYend &&
		  z > fZbeg && z < fZend);
	}
    Float_t Xmin()  const {return fXbeg;}
    Float_t Xmax()  const {return fXend;}
    Float_t DelX()  const {return fXdel;}
    Float_t DeliX() const {return fXdeli;}
    
    Float_t Ymin()  const {return fYbeg;}
    Float_t Ymax()  const {return fYend;}
    Float_t DelY()  const {return fYdel;}
    Float_t DeliY() const {return fYdeli;}
    
    Float_t Zmin()  const {return fZbeg;}
    Float_t Zmax()  const {return fZend;}
    Float_t DelZ()  const {return fZdel;}
    Float_t DeliZ() const {return fZdeli;}
    void    SetLimits(Float_t xmin, Float_t xmax, Float_t ymin, Float_t ymax,
		      Float_t zmin, Float_t zmax)
	{
	    fXbeg = xmin; fXend = xmax; fYbeg = ymin; fYend = ymax;
	    fZbeg = zmin; fZend = zmax;
	}
    void SetWriteEnable(Int_t flag = 1) {fWriteEnable = flag;}
 protected:
    
    Float_t    fXbeg;         // Start of mesh in x
    Float_t    fYbeg;         // Start of mesh in y
    Float_t    fZbeg;         // Start of mesh in z
    Float_t    fXend;         // End of mesh in x
    Float_t    fYend;         // End of mesh in y
    Float_t    fZend;         // End of mesh in z
    Float_t    fXdel;         // Mesh step in x
    Float_t    fYdel;         // Mesh step in y
    Float_t    fZdel;         // Mesh step in z
    Double_t   fXdeli;        // Inverse of Mesh step in x
    Double_t   fYdeli;        // Inverse of Mesh step in y
    Double_t   fZdeli;        // Inverse of Mesh step in z
    Int_t      fXn;           // Number of mesh points in x
    Int_t      fYn;           // Number of mesh points in y
    Int_t      fZn;           // Number of mesh points in z
    Int_t      fWriteEnable;  // Enable flag for writing of field data.
    TVector*   fB;            // Field map
    
 private:
    void    ReadField();

    ClassDef(IlcFieldMap,3)  //Class for Field Map
};

#endif
