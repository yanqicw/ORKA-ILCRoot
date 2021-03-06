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

/* $Id: IlcTARGETgeomMatrix.cxx,v 1.4 2008/11/05 01:47:14 rucco Exp $ */

////////////////////////////////////////////////////////////////////////
// This is the implementation file for IlcTARGETgeomMatrix class. It 
// contains the routines to manipulate, setup, and queary the geometry 
// of a given TARGET module. An TARGET module may be one of at least three
// TARGET detector technologies, Silicon Pixel, Drift, or Strip Detectors,
// and variations of these in size and/or layout. These routines let
// one go between ILC global coordiantes (cm) to a given modules 
// specific local coordinates (cm).
////////////////////////////////////////////////////////////////////////

#include <Riostream.h>
#include <TMath.h>
#include <TBuffer.h>
#include <TClass.h>

#include "IlcLog.h"
#include "IlcTARGETgeomMatrix.h"

ClassImp(IlcTARGETgeomMatrix)
//----------------------------------------------------------------------
IlcTARGETgeomMatrix::IlcTARGETgeomMatrix(){
////////////////////////////////////////////////////////////////////////
// The Default constructor for the IlcTARGETgeomMatrix class. By Default
// the angles of rotations are set to zero, meaning that the rotation
// matrix is the unit matrix. The translation vector is also set to zero
// as are the module id number. The detector type is set to -1 (an undefined
// value). The full rotation matrix is kept so that the evaluation 
// of a coordinate transformation can be done quickly and with a minimum
// of CPU overhead. The basic coordinate systems are the ILC global
// coordinate system and the detector local coordinate system. In general
// this structure is not limited to just those two coordinate systems.
//Begin_Html
/*
<img src="picts/TARGET/IlcISgeomMatrix_L1.gif">
*/
//End_Html
////////////////////////////////////////////////////////////////////////


//     IlcDebug(2, "ADESSO SONO QUA");

    Int_t i,j;

    fDetectorIndex = -1; // a value never defined.
    for(i=0;i<3;i++){
	fid[i] = 0;
	frot[i] = ftran[i] = 0.0;
	for(j=0;j<3;j++) fm[i][j] = 0.0;
	fCylR = fCylPhi = 0.0;
    }// end for i
    fm[0][0] = fm[1][1] = fm[2][2] = 1.0;
}
//----------------------------------------------------------------------
IlcTARGETgeomMatrix::IlcTARGETgeomMatrix(const IlcTARGETgeomMatrix &sourse) : 
    TObject(sourse){
////////////////////////////////////////////////////////////////////////
// The standard Copy constructor. This make a full / proper copy of
// this class.
////////////////////////////////////////////////////////////////////////



	Int_t i,j;

	this->fDetectorIndex = sourse.fDetectorIndex;
	for(i=0;i<3;i++){
		this->fid[i]     = sourse.fid[i];
		this->frot[i]    = sourse.frot[i];
		this->ftran[i]   = sourse.ftran[i];
		this->fCylR      = sourse.fCylR;
		this->fCylPhi    = sourse.fCylPhi;
		for(j=0;j<3;j++) this->fm[i][j] = sourse.fm[i][j];
	}// end for i
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::operator=(const IlcTARGETgeomMatrix &sourse){
////////////////////////////////////////////////////////////////////////
// The standard = operator. This make a full / proper copy of
// this class.
////////////////////////////////////////////////////////////////////////



	Int_t i,j;

	this->fDetectorIndex = sourse.fDetectorIndex;
	for(i=0;i<3;i++){
		this->fid[i]     = sourse.fid[i];
		this->frot[i]    = sourse.frot[i];
		this->ftran[i]   = sourse.ftran[i];
		this->fCylR      = sourse.fCylR;
		this->fCylPhi    = sourse.fCylPhi;
		for(j=0;j<3;j++) this->fm[i][j] = sourse.fm[i][j];
	}// end for i
}
//----------------------------------------------------------------------
IlcTARGETgeomMatrix::IlcTARGETgeomMatrix(Int_t idt,const Int_t id[3],
		   const Double_t rot[3],const Double_t tran[3]){
////////////////////////////////////////////////////////////////////////
// This is a constructor for the IlcTARGETgeomMatrix class. The matrix is
// defined by 3 standard rotation angles [radians], and the translation
// vector tran [cm]. In addition the layer, ladder, and detector number
// for this particular module and the type of module must be given.
// The full rotation matrix is kept so that the evaluation 
// of a coordinate transformation can be done quickly and with a minimum
// of CPU overhead. The basic coordinate systems are the ILC global
// coordinate system and the detector local coordinate system. In general
// this structure is not limited to just those two coordinate systems.
//Begin_Html
/*
<img src="picts/TARGET/IlcISgeomMatrix_L1.gif">
*/
//End_Html
////////////////////////////////////////////////////////////////////////




    Int_t i;

    fDetectorIndex = idt; // a value never defined.
    for(i=0;i<3;i++){
	fid[i]   = id[i];
	frot[i]  = rot[i];
	ftran[i] = tran[i];
    }// end for i
    fCylR   = TMath::Sqrt(ftran[0]*ftran[0]+ftran[1]*ftran[1]);
    fCylPhi = TMath::ATan2(ftran[1],ftran[0]);
    if(fCylPhi<0.0) fCylPhi += TMath::Pi();
    this->MatrixFromAngle();
}
//----------------------------------------------------------------------
IlcTARGETgeomMatrix::IlcTARGETgeomMatrix(Int_t idt, const Int_t id[3],
                                   Double_t matrix[3][3],
                                   const Double_t tran[3]){
////////////////////////////////////////////////////////////////////////
// This is a constructor for the IlcTARGETgeomMatrix class. The rotation matrix
// is given as one of the inputs, and the translation vector tran [cm]. In 
// addition the layer, ladder, and detector number for this particular
// module and the type of module must be given. The full rotation matrix
// is kept so that the evaluation of a coordinate transformation can be
// done quickly and with a minimum of CPU overhead. The basic coordinate
// systems are the ILC global coordinate system and the detector local
// coordinate system. In general this structure is not limited to just
// those two coordinate systems.
//Begin_Html
/*
<img src="picts/TARGET/IlcISgeomMatrix_L1.gif">
*/
//End_Html
////////////////////////////////////////////////////////////////////////

//cout<<"IlcTARGETgeomMatrix::IlcTARGETgeomMatrix 0"<<endl;

    Int_t i,j;

    fDetectorIndex = idt; // a value never defined.
    for(i=0;i<3;i++){
	fid[i]   = id[i];
	ftran[i] = tran[i];
	for(j=0;j<3;j++) fm[i][j] = matrix[i][j];
    }// end for i
    fCylR   = TMath::Sqrt(ftran[0]*ftran[0]+ftran[1]*ftran[1]);
    fCylPhi = TMath::ATan2(ftran[1],ftran[0]);
    if(fCylPhi<0.0) fCylPhi += TMath::Pi();
    this->AngleFromMatrix();
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::SixAnglesFromMatrix(Double_t *ang){
////////////////////////////////////////////////////////////////////////
// This function returns the 6 GEANT 3.21 rotation angles [degrees] in
// the array ang which must be at least [6] long.
////////////////////////////////////////////////////////////////////////

//cout<<"IlcTARGETgeomMatrix::SixAnglesFromMatrix"<<endl;

    Double_t si,c=180./TMath::Pi();

    ang[1] = TMath::ATan2(fm[0][1],fm[0][0]);
    if(TMath::Cos(ang[1])!=0.0) si = fm[0][0]/TMath::Cos(ang[1]);
    else si = fm[0][1]/TMath::Sin(ang[1]);
    ang[0] = TMath::ATan2(si,fm[0][2]);

    ang[3] = TMath::ATan2(fm[1][1],fm[1][0]);
    if(TMath::Cos(ang[3])!=0.0) si = fm[1][0]/TMath::Cos(ang[3]);
    else si = fm[1][1]/TMath::Sin(ang[3]);
    ang[2] = TMath::ATan2(si,fm[1][2]);

    ang[5] = TMath::ATan2(fm[2][1],fm[2][0]);
    if(TMath::Cos(ang[5])!=0.0) si = fm[2][0]/TMath::Cos(ang[5]);
    else si = fm[2][1]/TMath::Sin(ang[5]);
    ang[4] = TMath::ATan2(si,fm[2][2]);

    for(Int_t i=0;i<6;i++) {ang[i] *= c; if(ang[i]<0.0) ang[i] += 360.;}
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::MatrixFromSixAngles(const Double_t *ang){
////////////////////////////////////////////////////////////////////////
// Given the 6 GEANT 3.21 rotation angles [degree], this will compute and
// set the rotations matrix and 3 standard rotation angles [radians].
// These angles and rotation matrix are overwrite the existing values in
// this class.
////////////////////////////////////////////////////////////////////////

//cout<<"IlcTARGETgeomMatrix::SixAnglesFromMatrix 2"<<endl;

    Int_t    i,j;
    Double_t si,lr[9],c=TMath::Pi()/180.;

    si    = TMath::Sin(c*ang[0]);
    if(ang[0]== 90.0)                 si = +1.0;
    if(ang[0]==270.0)                 si = -1.0;
    if(ang[0]==  0.0||ang[0]==180.) si =  0.0;
    lr[0] = si * TMath::Cos(c*ang[1]);
    lr[1] = si * TMath::Sin(c*ang[1]);
    lr[2] = TMath::Cos(c*ang[0]);
    if(ang[0]== 90.0||ang[0]==270.) lr[2] =  0.0;
    if(ang[0]== 0.0)                  lr[2] = +1.0;
    if(ang[0]==180.0)                 lr[2] = -1.0;
//
    si    =  TMath::Sin(c*ang[2]);
    if(ang[2]== 90.0)                 si = +1.0; 
    if(ang[2]==270.0)                 si = -1.0;
    if(ang[2]==  0.0||ang[2]==180.) si =  0.0;
    lr[3] = si * TMath::Cos(c*ang[3]);
    lr[4] = si * TMath::Sin(c*ang[3]);
    lr[5] = TMath::Cos(c*ang[2]);
    if(ang[2]== 90.0||ang[2]==270.) lr[5] =  0.0;
    if(ang[2]==  0.0)                 lr[5] = +1.0;
    if(ang[2]==180.0)                 lr[5] = -1.0;
//
    si    = TMath::Sin(c*ang[4]);
    if(ang[4]== 90.0)                 si = +1.0;
    if(ang[4]==270.0)                 si = -1.0;
    if(ang[4]==  0.0||ang[4]==180.) si =  0.0;
    lr[6] = si * TMath::Cos(c*ang[5]);
    lr[7] = si * TMath::Sin(c*ang[5]);
    lr[8] = TMath::Cos(c*ang[4]);
    if(ang[4]== 90.0||ang[4]==270.0) lr[8] =  0.0;
    if(ang[4]==  0.0)                  lr[8] = +1.0;
    if(ang[4]==180.0)                  lr[8] = -1.0;
    // Normilcze these elements and fill matrix fm.
    for(i=0;i<3;i++){// reuse si.
	si = 0.0;
	for(j=0;j<3;j++) si += lr[3*i+j]*lr[3*i+j];
	si = TMath::Sqrt(1./si);
	for(j=0;j<3;j++) fm[i][j] = si*lr[3*i+j];
    } // end for i
    this->AngleFromMatrix();
}
//----------------------------------------------------------------------
IlcTARGETgeomMatrix::IlcTARGETgeomMatrix(const Double_t rotd[6]/*degrees*/,
                                   Int_t idt,const Int_t id[3],
		                   const Double_t tran[3]){
////////////////////////////////////////////////////////////////////////
// This is a constructor for the IlcTARGETgeomMatrix class. The matrix is
// defined by the 6 GEANT 3.21 rotation angles [degrees], and the translation
// vector tran [cm]. In addition the layer, ladder, and detector number
// for this particular module and the type of module must be given.
// The full rotation matrix is kept so that the evaluation 
// of a coordinate transformation can be done quickly and with a minimum
// of CPU overhead. The basic coordinate systems are the ILC global
// coordinate system and the detector local coordinate system. In general
// this structure is not limited to just those two coordinate systems.
//Begin_Html
/*
<img src="picts/TARGET/IlcISgeomMatrix_L1.gif">
*/
//End_Html
////////////////////////////////////////////////////////////////////////
//cout<<"IlcTARGETgeomMatrix::SixAnglesFromMatrix 3"<<endl;


    Int_t i;

    fDetectorIndex = idt; // a value never defined.
    for(i=0;i<3;i++){
	fid[i]   = id[i];
	ftran[i] = tran[i];
    }// end for i
    fCylR   = TMath::Sqrt(ftran[0]*ftran[0]+ftran[1]*ftran[1]);
    fCylPhi = TMath::ATan2(ftran[1],ftran[0]);
    if(fCylPhi<0.0) fCylPhi += TMath::Pi();
    this->MatrixFromSixAngles(rotd);
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::AngleFromMatrix(){
////////////////////////////////////////////////////////////////////////
// Computes the angles from the rotation matrix up to a phase of 180 degrees.
////////////////////////////////////////////////////////////////////////

//     IlcDebug(2, "ADESSO SONO QUA");
//cout<<"IlcTARGETgeomMatrix::AnglesFromMatrix 4"<<endl;
    Double_t rx,ry,rz;
    // get angles from matrix up to a phase of 180 degrees.

    rx = TMath::ATan2(fm[2][1],fm[2][2]);if(rx<0.0) rx += 2.0*TMath::Pi();
    ry = TMath::ASin(fm[0][2]);          if(ry<0.0) ry += 2.0*TMath::Pi();
    rz = TMath::ATan2(fm[1][1],fm[0][0]);if(rz<0.0) rz += 2.0*TMath::Pi();
    frot[0] = rx;
    frot[1] = ry;
    frot[2] = rz;
    return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::MatrixFromAngle(){
////////////////////////////////////////////////////////////////////////
// Computes the Rotation matrix from the angles [radians] kept in this
// class.
////////////////////////////////////////////////////////////////////////



//cout<<"IlcTARGETgeomMatrix::SixAnglesFromMatrix 5"<<endl;
   Double_t sx,sy,sz,cx,cy,cz;

   sx = TMath::Sin(frot[0]); cx = TMath::Cos(frot[0]);
   sy = TMath::Sin(frot[1]); cy = TMath::Cos(frot[1]);
   sz = TMath::Sin(frot[2]); cz = TMath::Cos(frot[2]);
   fm[0][0] =  cz*cy;             // fr[0]
   fm[0][1] = -cz*sy*sx - sz*cx;  // fr[1]
   fm[0][2] = -cz*sy*cx + sz*sx;  // fr[2]
   fm[1][0] =  sz*cy;             // fr[3]
   fm[1][1] = -sz*sy*sx + cz*cx;  // fr[4]
   fm[1][2] = -sz*sy*cx - cz*sx;  // fr[5]
   fm[2][0] =  sy;                // fr[6]
   fm[2][1] =  cy*sx;             // fr[7]
   fm[2][2] =  cy*cx;             // fr[8]

}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::GtoLPosition(const Double_t g0[3],Double_t l[3]) const {
////////////////////////////////////////////////////////////////////////
// Returns the local coordinates given the global coordinates [cm].
////////////////////////////////////////////////////////////////////////

//cout<<"IlcTARGETgeomMatrix::SixAnglesFromMatrix 6"<<endl;

	Int_t    i,j;
	Double_t g[3];

	for(i=0;i<3;i++) {g[i] = g0[i] - ftran[i]; 
	
	  if(IlcDebugLevel()>0)
	    IlcDebug(1, Form(" g[%d] = %f, g0[%d] = %f, ftran[%d] = %f", i, g[i], i, g0[i], i, ftran[i]));
// // // // 	  IlcInfo(Form(" g[%d] = %f, g0[%d] = %f, ftran[%d] = %f", i, g[i], i, g0[i], i, ftran[i]));
	}
	
	for(i=0;i<3;i++){
		l[i] = 0.0;
		for(j=0;j<3;j++) {l[i] += fm[i][j]*g[j];
		  if(IlcDebugLevel()>0)
		    IlcDebug(1, Form(" l[%d] = %f, fm[%d][%d] = %f, ftran[%d] = %f, g[%d] = %f", i, l[i], i, j, fm[i][j], i, ftran[i], j, g[j]));
// // // // 		  IlcInfo( Form(" l[%d] = %f, fm[%d][%d] = %f, ftran[%d] = %f, g[%d] = %f", i, l[i], i, j, fm[i][j], i, ftran[i], j, g[j]));
		}
		// g = R l + translation
	} // end for i
	return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::LtoGPosition(const Double_t l[3],Double_t g[3]) const {
////////////////////////////////////////////////////////////////////////
// Returns the global coordinates given the local coordinates [cm].
////////////////////////////////////////////////////////////////////////
//cout<<"IlcTARGETgeomMatrix::SixAnglesFromMatrix 7"<<endl;


	Int_t    i,j;

	for(i=0;i<3;i++){
		g[i] = 0.0;
		for(j=0;j<3;j++) g[i] += fm[j][i]*l[j];
		g[i] += ftran[i];
		// g = R^t l + translation
	} // end for i
	return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::GtoLMomentum(const Double_t g[3],Double_t l[3]) const{
////////////////////////////////////////////////////////////////////////
// Returns the local coordinates of the momentum given the global
// coordinates of the momentum. It transforms just like GtoLPosition
// except that the translation vector is zero.
////////////////////////////////////////////////////////////////////////



	Int_t    i,j;

	for(i=0;i<3;i++){
		l[i] = 0.0;
		for(j=0;j<3;j++) l[i] += fm[i][j]*g[j];
		// g = R l
	} // end for i
	return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::LtoGMomentum(const Double_t l[3],Double_t g[3]) const {
////////////////////////////////////////////////////////////////////////
// Returns the Global coordinates of the momentum given the local
// coordinates of the momentum. It transforms just like LtoGPosition
// except that the translation vector is zero.
////////////////////////////////////////////////////////////////////////



	Int_t    i,j;

	for(i=0;i<3;i++){
		g[i] = 0.0;
		for(j=0;j<3;j++) g[i] += fm[j][i]*l[j];
		// g = R^t l
	} // end for i
	return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::GtoLPositionError(const Double_t g[3][3],
                                               Double_t l[3][3]) const {
////////////////////////////////////////////////////////////////////////
// Given an Uncertainty matrix in Global coordinates it is rotated so that 
// its representation in local coordinates can be returned. There is no
// effect due to the translation vector or its uncertainty.
////////////////////////////////////////////////////////////////////////



	Int_t    i,j,k,m;

	for(i=0;i<3;i++)for(m=0;m<3;m++){
	    l[i][m] = 0.0;
	    for(j=0;j<3;j++)for(k=0;k<3;k++)
		l[i][m] += fm[j][i]*g[j][k]*fm[k][m];
	} // end for i,m
	    // g = R^t l R
	return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::LtoGPositionError(const Double_t l[3][3],
                                               Double_t g[3][3]) const {
////////////////////////////////////////////////////////////////////////
// Given an Uncertainty matrix in Local coordinates it is rotated so that 
// its representation in global coordinates can be returned. There is no
// effect due to the translation vector or its uncertainty.
////////////////////////////////////////////////////////////////////////



	Int_t    i,j,k,m;

	for(i=0;i<3;i++)for(m=0;m<3;m++){
	    g[i][m] = 0.0;
	    for(j=0;j<3;j++)for(k=0;k<3;k++)
		g[i][m] += fm[i][j]*l[j][k]*fm[m][k];
	} // end for i,m
	    // g = R l R^t
	return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::GtoLPositionTracking(const Double_t g0[3],
					    Double_t l[3]) const {
////////////////////////////////////////////////////////////////////////
// A slightly different coordinate system is used when tracking.
// This coordinate system is only relevant when the geometry represents
// the cylindrical ILC TARGET geometry. For tracking the Z axis is left
// alone but X -> -Y and Y -> X such that X always points out of the
// TARGET Cylinder for every layer including layer 1 (where the detector 
// are mounted upside down).
//Begin_Html
/*
<img src="picts/TARGET/IlcTARGETgeomMatrix_T1.gif">
 */
//End_Html
////////////////////////////////////////////////////////////////////////



    Double_t l0[3];

    this->GtoLPosition(g0,l0);
// // // //     if(fid[0]==1){ // for layer 1 the detector are flipped upside down
// // // // 	           // with respect to the others.
// // // // 	l[0] = +l0[1];
// // // // 	l[1] = -l0[0];
// // // // 	l[2] = +l0[2];
// // // //     }else
    {
	l[0] = -l0[1];
	l[1] = +l0[0];
	l[2] = +l0[2];
    } // end if
    return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::LtoGPositionTracking(const Double_t l[3],
					    Double_t g[3]) const {
////////////////////////////////////////////////////////////////////////
// A slightly different coordinate system is used when tracking.
// This coordinate system is only relevant when the geometry represents
// the cylindrical ILC TARGET geometry. For tracking the Z axis is left
// alone but X -> -Y and Y -> X such that X always points out of the
// TARGET Cylinder for every layer including layer 1 (where the detector 
// are mounted upside down).
//Begin_Html
/*
<img src="picts/TARGET/IlcTARGETgeomMatrix_T1.gif">
 */
//End_Html
////////////////////////////////////////////////////////////////////////



    Double_t l0[3];

// // // //     if(fid[0]==1){ // for layer 1 the detector are flipped upside down
// // // // 	           // with respect to the others.
// // // // 	l0[0] = -l[1];
// // // // 	l0[1] = +l[0];
// // // // 	l0[2] = +l[2];
// // // //     }else
    {
	l0[0] = +l[1];
	l0[1] = -l[0];
	l0[2] = +l[2];
    } // end if
    this->LtoGPosition(l0,g);
    return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::GtoLMomentumTracking(const Double_t g[3],
					    Double_t l[3]) const {
////////////////////////////////////////////////////////////////////////
// A slightly different coordinate system is used when tracking.
// This coordinate system is only relevant when the geometry represents
// the cylindrical ILC TARGET geometry. For tracking the Z axis is left
// alone but X -> -Y and Y -> X such that X always points out of the
// TARGET Cylinder for every layer including layer 1 (where the detector 
// are mounted upside down).
//Begin_Html
/*
<img src="picts/TARGET/IlcTARGETgeomMatrix_T1.gif">
 */
//End_Html
////////////////////////////////////////////////////////////////////////



    Double_t l0[3];

    this->GtoLMomentum(g,l0);
// // // //     if(fid[0]==1){ // for layer 1 the detector are flipped upside down
// // // // 	           // with respect to the others.
// // // // 	l[0] = +l0[1];
// // // // 	l[1] = -l0[0];
// // // // 	l[2] = +l0[2];
// // // //     }else
    {
	l[0] = -l0[1];
	l[1] = +l0[0];
	l[2] = +l0[2];
    } // end if
    return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::LtoGMomentumTracking(const Double_t l[3],
					    Double_t g[3]) const {
////////////////////////////////////////////////////////////////////////
// A slightly different coordinate system is used when tracking.
// This coordinate system is only relevant when the geometry represents
// the cylindrical ILC TARGET geometry. For tracking the Z axis is left
// alone but X -> -Y and Y -> X such that X always points out of the
// TARGET Cylinder for every layer including layer 1 (where the detector 
// are mounted upside down).
//Begin_Html
/*
<img src="picts/TARGET/IlcTARGETgeomMatrix_T1.gif">
 */
//End_Html
////////////////////////////////////////////////////////////////////////



    Double_t l0[3];

// // // //     if(fid[0]==1){ // for layer 1 the detector are flipped upside down
// // // // 	           // with respect to the others.
// // // // 	l0[0] = -l[1];
// // // // 	l0[1] = +l[0];
// // // // 	l0[2] = +l[2];
// // // //     }else
    {
	l0[0] = +l[1];
	l0[1] = -l[0];
	l0[2] = +l[2];
    } // end if
    this->LtoGMomentum(l0,g);
	return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::GtoLPositionErrorTracking(const Double_t g[3][3],
						 Double_t l[3][3]) const {
////////////////////////////////////////////////////////////////////////
// A slightly different coordinate system is used when tracking.
// This coordinate system is only relevant when the geometry represents
// the cylindrical ILC TARGET geometry. For tracking the Z axis is left
// alone but X -> -Y and Y -> X such that X always points out of the
// TARGET Cylinder for every layer including layer 1 (where the detector 
// are mounted upside down).
//Begin_Html
/*
<img src="picts/TARGET/IlcTARGETgeomMatrix_T1.gif">
 */
//End_Html
////////////////////////////////////////////////////////////////////////



	Int_t    i,j,k,m;
	Double_t rt[3][3];
	Double_t a0[3][3] = {{0.,+1.,0.},{-1.,0.,0.},{0.,0.,+1.}};
	Double_t a1[3][3] = {{0.,-1.,0.},{+1.,0.,0.},{0.,0.,+1.}};

// // // // 	if(fid[0]==1) for(i=0;i<3;i++)for(j=0;j<3;j++)for(k=0;k<3;k++)
// // // // 	    rt[i][k] = a0[i][j]*fm[j][k];
// // // // 	else
	for(i=0;i<3;i++)for(j=0;j<3;j++)for(k=0;k<3;k++)
	    rt[i][k] = a1[i][j]*fm[j][k];
	for(i=0;i<3;i++)for(m=0;m<3;m++){
	    l[i][m] = 0.0;
	    for(j=0;j<3;j++)for(k=0;k<3;k++)
		l[i][m] += rt[j][i]*g[j][k]*rt[k][m];
	} // end for i,m
	    // g = R^t l R
	return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::LtoGPositionErrorTracking(const Double_t l[3][3],
						 Double_t g[3][3]) const {
////////////////////////////////////////////////////////////////////////
// A slightly different coordinate system is used when tracking.
// This coordinate system is only relevant when the geometry represents
// the cylindrical ILC TARGET geometry. For tracking the Z axis is left
// alone but X -> -Y and Y -> X such that X always points out of the
// TARGET Cylinder for every layer including layer 1 (where the detector 
// are mounted upside down).
//Begin_Html
/*
<img src="picts/TARGET/IlcTARGETgeomMatrix_T1.gif">
 */
//End_Html
////////////////////////////////////////////////////////////////////////



	Int_t    i,j,k,m;
	Double_t rt[3][3];
	Double_t a0[3][3] = {{0.,+1.,0.},{-1.,0.,0.},{0.,0.,+1.}};
	Double_t a1[3][3] = {{0.,-1.,0.},{+1.,0.,0.},{0.,0.,+1.}};

// // // // 	if(fid[0]==1) for(i=0;i<3;i++)for(j=0;j<3;j++)for(k=0;k<3;k++)
// // // // 	    rt[i][k] = a0[i][j]*fm[j][k];
// // // // 	else 
	for(i=0;i<3;i++)for(j=0;j<3;j++)for(k=0;k<3;k++)
	    rt[i][k] = a1[i][j]*fm[j][k];
	for(i=0;i<3;i++)for(m=0;m<3;m++){
	    g[i][m] = 0.0;
	    for(j=0;j<3;j++)for(k=0;k<3;k++)
		g[i][m] += rt[i][j]*l[j][k]*rt[m][k];
	} // end for i,m
	    // g = R l R^t
	return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::PrintTitles(ostream *os) const {
////////////////////////////////////////////////////////////////////////
// Standard output format for this class but it includes variable
// names and formatting that makes it easer to read.
////////////////////////////////////////////////////////////////////////



    Int_t i,j;

    *os << "fDetectorIndex=" << fDetectorIndex << " fid[3]={";
    for(i=0;i<3;i++) *os << fid[i]   << " ";
    *os << "} frot[3]={";
    for(i=0;i<3;i++) *os << frot[i]  << " ";
    *os << "} ftran[3]={";
    for(i=0;i<3;i++) *os << ftran[i] << " ";
    *os << "} fm[3][3]={";
    for(i=0;i<3;i++){for(j=0;j<3;j++){  *os << fm[i][j] << " ";} *os <<"}{";}
    *os << "}" << endl;
    return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::PrintComment(ostream *os) const {
////////////////////////////////////////////////////////////////////////
//  output format used by Print..
////////////////////////////////////////////////////////////////////////



    *os << "fDetectorIndex fid[0] fid[1] fid[2] ftran[0] ftran[1] ftran[2] ";
    *os << "fm[0][0]  fm[0][1]  fm[0][2]  fm[1][0]  fm[1][1]  fm[1][2]  ";
    *os << "fm[2][0]  fm[2][1]  fm[2][2] ";
    return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::Print(ostream *os){
////////////////////////////////////////////////////////////////////////
// Standard output format for this class.
////////////////////////////////////////////////////////////////////////



    Int_t i,j;
#if defined __GNUC__
#if __GNUC__ > 2
    ios::fmtflags fmt;
#else
    Int_t fmt;
#endif
#else
#if defined __ICC || defined __ECC || defined __xlC__
    ios::fmtflags fmt;
#else
    Int_t fmt;
#endif
#endif

    fmt = os->setf(ios::scientific);  // set scientific floating point output
    *os << fDetectorIndex << " ";
    for(i=0;i<3;i++) *os << fid[i]   << " ";
//    for(i=0;i<3;i++) *os << frot[i]  << " ";  // Redundant with fm[][].
    for(i=0;i<3;i++) *os << setprecision(16) << ftran[i] << " ";
    for(i=0;i<3;i++)for(j=0;j<3;j++)  *os << setprecision(16) << 
					  fm[i][j] << " ";
    *os << endl;
    os->flags(fmt); // reset back to old formating.
    return;
}
//----------------------------------------------------------------------
void IlcTARGETgeomMatrix::Read(istream *is){
////////////////////////////////////////////////////////////////////////
// Standard input format for this class.
////////////////////////////////////////////////////////////////////////

//     IlcDebug(2, "ADESSO SONO QUA");
//cout<<"IlcTARGETgeomMatrix::SixAnglesFromMatrix 8"<<endl;
    Int_t i,j;

    *is >> fDetectorIndex;
    for(i=0;i<3;i++) *is >> fid[i];
//    for(i=0;i<3;i++) *is >> frot[i]; // Redundant with fm[][].
    for(i=0;i<3;i++) *is >> ftran[i];
    for(i=0;i<3;i++)for(j=0;j<3;j++)  *is >> fm[i][j];
    AngleFromMatrix(); // compute angles frot[].
    fCylR   = TMath::Sqrt(ftran[0]*ftran[0]+ftran[1]*ftran[1]);
    fCylPhi = TMath::ATan2(ftran[1],ftran[0]);
    if(fCylPhi<0.0) fCylPhi += TMath::Pi();
    return;
}
//______________________________________________________________________
void IlcTARGETgeomMatrix::Streamer(TBuffer &R__b){
   // Stream an object of class IlcTARGETgeomMatrix.
//   cout<<"IlcTARGETgeomMatrix::Streamer 9"<<endl;
//    IlcDebug(2, "ADESSO SONO QUA");

   if (R__b.IsReading()) {
      IlcTARGETgeomMatrix::Class()->ReadBuffer(R__b, this);
      fCylR   = TMath::Sqrt(ftran[0]*ftran[0]+ftran[1]*ftran[1]);
      fCylPhi = TMath::ATan2(ftran[1],ftran[0]);
      this->AngleFromMatrix();
    if(fCylPhi<0.0) fCylPhi += TMath::Pi();
   } else {
      IlcTARGETgeomMatrix::Class()->WriteBuffer(R__b, this);
   }
}
//______________________________________________________________________
void IlcTARGETgeomMatrix::SetTranslation(const Double_t tran[3]){
  // Sets the translation vector and computes fCylR and fCylPhi.
  

  
  for(Int_t i=0;i<3;i++) ftran[i] = tran[i];
  fCylR   = TMath::Sqrt(ftran[0]*ftran[0]+ftran[1]*ftran[1]);
  fCylPhi = TMath::ATan2(ftran[1],ftran[0]);
  if(fCylPhi<0.0) fCylPhi += TMath::Pi();
}

//----------------------------------------------------------------------
ostream &operator<<(ostream &os,IlcTARGETgeomMatrix &p){
////////////////////////////////////////////////////////////////////////
// Standard output streaming function.
////////////////////////////////////////////////////////////////////////

    p.Print(&os);
    return os;
}
//----------------------------------------------------------------------
istream &operator>>(istream &is,IlcTARGETgeomMatrix &r){
////////////////////////////////////////////////////////////////////////
// Standard input streaming function.
////////////////////////////////////////////////////////////////////////

    r.Read(&is);
    return is;
}
//----------------------------------------------------------------------
