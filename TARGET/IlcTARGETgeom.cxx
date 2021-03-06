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

/* $Id: IlcTARGETgeom.cxx,v 1.3 2008/11/07 00:00:41 cgatto Exp $ */

///////////////////////////////////////////////////////////////////////
// TARGET geometry manipulation routines.                               //
// Created April 15 1999.                                            //
// version: 0.0.0                                                    //
// By: Bjorn S. Nilsen                                               //
// version: 0.0.1                                                    //
// Updated May 27 1999.                                              //
// Added Cylindrical random and global based changes.               //
// Added  function PrintComparison.                                  //
// Modified and added functions Feb. 7 2006                          //
///////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
//     The local coordinate system by, default, is show in the following
// figures. Also shown are the ladder numbering scheme.
//Begin_Html
/*
<img src="picts/TARGET/IlcTARGETgeomMatrix_L1.gif">
</pre>
<br clear=left>
<font size=+2 color=blue>
<p>This shows the relative geometry differences between the ILC Global
coordinate system and the local detector coordinate system.
</font>
<pre>

<pre>
<img src="picts/TARGET/its1+2_convention_front_5.gif">
</pre>
<br clear=left>
<font size=+2 color=blue>
<p>This shows the front view of the SPDs and the orientation of the local
pixel coordinate system. Note that the inner pixel layer has its y coordinate
in the opposite direction from all of the other layers.
</font>
<pre>

<pre>
<img src="picts/TARGET/its3+4_convention_front_5.gif">
</pre>
<br clear=left>
<font size=+2 color=blue>
<p>This shows the front view of the SDDs and the orientation of the local
pixel coordinate system.
</font>
<pre>

<pre>
<img src="picts/TARGET/its5+6_convention_front_5.gif">
</pre>
<br clear=left>
<font size=+2 color=blue>
<p>This shows the front view of the SSDs and the orientation of the local
pixel coordinate system.
</font>
<pre>
*/
//End_Html
//
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// version: 0
// Written by Bjorn S. Nilsen
//
// Data Members:
//
// TString    fVersion 
//     Transformation version.
// Int_t      fTrans
//     Flag to keep track of which transformation 
// Int_t      fNmodules
//      The total number of modules
// Int_t fNlayers
//     The number of TARGET layers for this geometry. By default this
//  is 6, but can be modified by the creator function if there are
// more layers defined.
//
// TArrayI fNlad
//     A pointer to an array fNlayers long containing the number of 
// ladders for each layer. This array is typically created and filled 
// by the IlcTARGETgeom creator function.
//
// TArrayI fNdet
//     A pointer to an array fNlayers long containing the number of
// active detector volumes for each ladder. This array is typically
// created and filled by the IlcTARGETgeom creator function.
//
// TObjArray fGm containing objects of type IlcTARGETgeomMatrix
//     A pointer to an array of IlcTARGETgeomMatrix classes. One element 
// per module (detector) in the TARGET. IlcTARGETgeomMatrix basicly contains
// all of the necessary information about the detector and it's coordinate
// transformations.
//
// TObjArray fShape containting objects of type IlcTARGETgeom
//     A pointer to an array of TObjects containing the detailed shape
// information for each type of detector used in the TARGET. For example
// I have created IlcTARGETgeomSPD, IlcTARGETgeomSDD, and 
// IlcTARGETsegmenttionSSD as example structures, derived from TObjects, 
// to hold the detector information. I would recommend that one element 
// in each of these structures, that which describes the shape of the 
// active volume, be one of the ROOT classes derived from TShape. In this 
// way it would be easy to have the display program display the correct 
// active TARGET volumes. See the example classes IlcTARGETgeomSPD, 
// IlcTARGETgeomSDD, and IlcTARGETgeomSSD for a more detailed 
// example.
////////////////////////////////////////////////////////////////////////
#include <Riostream.h>
#include <ctype.h>

#include <TRandom.h>
#include <TSystem.h>
#include <TArrayI.h>
#include <TGeoVolume.h>
#include <TGeoManager.h>
#include <TGeoPgon.h>
#include <TGeoCompositeShape.h>

#include "IlcRun.h"
#include "IlcTARGETParam.h"
#include "IlcTARGETgeom.h"
#include "IlcTARGETgeomSDD.h"
#include "IlcTARGETgeomSPD.h"
#include "IlcTARGETgeomSSD.h"
#include "IlcLog.h"


ClassImp(IlcTARGETgeom)

//______________________________________________________________________
IlcTARGETgeom::IlcTARGETgeom():
TObject(),
fVersion("GEANT"),// Transformation version.
fTrans(0),       // Flag to keep track of which transformation 
fNmodules(0),    // The total number of modules
fNlayers(0),     // The number of layers.
fNlad(),         //[] Array of the number of ladders/layer(layer)
fNdet(),         //[] Array of the number of detector/ladder(layer)
fGm(0,0),        // Structure of translation. and rotation.
fShape(0,0),      // Array of shapes and detector information.
fShape2(0,0)      // Array of shapes and detector information.
{
    //     The default constructor for the IlcTARGETgeom class. It, by default,
    // sets fNlayers to zero and zeros all pointers.
    // Do not allocate anything zero everything.
    // Inputs:
    //    none.
    // Outputs:
    //    none.
    // Return:
    //    a zeroed IlcTARGETgeom object.
    fPixelX.Set(fNlayers);
    fPixelY.Set(fNlayers);
    fPixelZ.Set(fNlayers);
    fNPixelX.Set(fNlayers);
    fNPixelY.Set(fNlayers);
    fNPixelZ.Set(fNlayers);
    fDetVersion=-11;

    fGm.SetOwner(kTRUE);
//     fShape.SetOwner(kTRUE);
    return;
}
//______________________________________________________________________
IlcTARGETgeom::IlcTARGETgeom(Int_t itype,Int_t nlayers,const Int_t *nlads,
                       const Int_t *ndets,Int_t mods,Int_t MinorVersion):
TObject(),
fVersion("GEANT"),    // Transformation version.
fTrans(itype),       // Flag to keep track of which transformation 
fNmodules(mods),     // The total number of modules
fNlayers(nlayers),   // The number of layers.
fNlad(nlayers,nlads),//[] Array of the number of ladders/layer(layer)
fNdet(nlayers,ndets),//[] Array of the number of detector/ladder(layer)
fGm(mods,0),         // Structure of translation. and rotation.
fShape(nlayers,0),          // Array of shapes and detector information.
fShape2(nlayers,0)          // Array of shapes and detector information.
{
    //     A simple constructor to set basic geometry class variables
    // Inputs:
    //      Int_t itype   the type of transformation kept.
    //                    bit 0 => Standard GEANT
    //                    bit 1 => TARGET tracking
    //                    bit 2 => A change in the coordinate system has been made.
    //                    others are still to be defined as needed.
    //      Int_t nlayers The number of TARGET layers also set the size of the arrays
    //      Int_t *nlads  an array of the number of ladders for each layer. This
    //                    array must be nlayers long.
    //      Int_t *ndets  an array of the number of detectors per ladder for each
    //                    layer. This array must be nlayers long.
    //      Int_t mods    The number of modules. Typically the sum of all the 
    //                    detectors on every layer and ladder.
    // Outputs:
    //     none
    // Return:
    //     A properly inilized IlcTARGETgeom object.

    fPixelX.Set(fNlayers);
    fPixelY.Set(fNlayers);
    fPixelZ.Set(fNlayers);
    fNPixelX.Set(fNlayers);
    fNPixelY.Set(fNlayers);
    fNPixelZ.Set(fNlayers);
    
    TGeoVolume *vol=0x0;
    TString s_shape;
    Char_t namevol[100];
    

    fDetVersion=MinorVersion;
    fGm.SetOwner(kTRUE);
    
    //useful for kalman filter as requested by Fedor
    fRmin.Set(32);
    fRmax.Set(32);
    fDz.Set(32);
    fPosz1.Set(32);
    fPosz2.Set(32);
    fNFakeLayers=0;

    fPosz1_Fake.Set(fNFakeLayers);
    fPosz2_Fake.Set(fNFakeLayers);
    if (GetDetVersion()==1 || GetDetVersion()==2) {
      for (Int_t lay=0;lay<fNlayers;lay++) {
	sprintf(namevol,"TARGET_layer%02d",lay);
	vol=gGeoManager->GetVolume(namevol);
	s_shape = vol->GetShape()->ClassName();

	if(s_shape.Contains("TGeoPgon")){	
	  fRmin.AddAt(((TGeoPgon*)vol->GetShape())->GetRmin(0),lay);  
	  fRmax.AddAt(((TGeoPgon*)vol->GetShape())->GetRmax(0),lay);
	  fDz.AddAt(((TGeoPgon*)vol->GetShape())->GetDZ(),lay);
	  fPosz1.AddAt(gGeoManager->GetVolume("TARGET_TARGETV")->GetNode(lay)->GetMatrix()->GetTranslation()[2]-((TGeoPgon*)vol->GetShape())->GetDZ(),lay);
	  fPosz2.AddAt(gGeoManager->GetVolume("TARGET_TARGETV")->GetNode(lay)->GetMatrix()->GetTranslation()[2]+((TGeoPgon*)vol->GetShape())->GetDZ(),lay);
	}
	else if(s_shape.Contains("TGeoCompositeShape")){	
	  fRmin.AddAt(((TGeoCompositeShape*)vol->GetShape())->GetDX()-2.*((TGeoBBox*)vol->GetNode(0)->GetVolume()->GetShape())->GetDX(),lay);  
	  fRmax.AddAt(((TGeoCompositeShape*)vol->GetShape())->GetDX(),lay);
	  fDz.AddAt(((TGeoCompositeShape*)vol->GetShape())->GetDZ(),lay);
	  fPosz1.AddAt(gGeoManager->GetVolume("TARGET_TARGETV")->GetNode(lay)->GetMatrix()->GetTranslation()[2]-((TGeoCompositeShape*)vol->GetShape())->GetDZ(),lay);
	  fPosz2.AddAt(gGeoManager->GetVolume("TARGET_TARGETV")->GetNode(lay)->GetMatrix()->GetTranslation()[2]+((TGeoCompositeShape*)vol->GetShape())->GetDZ(),lay);
	}

	
	IlcDebug(1,Form("for layer %i:  dz = %f , rmin = %f , rmax = %f , zmin=%f, zmax=%f", lay+1,fDz.At(lay),fRmin.At(lay),fRmax.At(lay),fPosz1.At(lay),fPosz2.At(lay)));
      }


    }

    return;
}
//______________________________________________________________________
void IlcTARGETgeom::Init(Int_t itype,Int_t nlayers,const Int_t *nlads,
                      const Int_t *ndets,Int_t mods,Int_t MinorVersion){
    //     A simple Inilizer to set basic geometry class variables
    // Inputs:
    //      Int_t itype   the type of transformation kept.
    //                    bit 0 => Standard GEANT
    //                    bit 1 => TARGET tracking
    //                    bit 2 => A change in the coordinate system has been made.
    //                    others are still to be defined as needed.
    //      Int_t nlayers The number of TARGET layers also set the size of the arrays
    //      Int_t *nlads  an array of the number of ladders for each layer. This
    //                    array must be nlayers long.
    //      Int_t *ndets  an array of the number of detectors per ladder for each
    //                    layer. This array must be nlayers long.
    //      Int_t mods    The number of modules. Typically the sum of all the 
    //                    detectors on every layer and ladder.
    // Outputs:
    //     none
    // Return:
    //     A properly inilized IlcTARGETgeom object.
    

    //cout << "chiama questo metodo per inizializzazione geom"<<endl;
    fVersion  = "GEANT";     // Transformation version.
    fDetVersion = MinorVersion;
    fTrans    = itype;       // Flag to keep track of which transformation 
    fNmodules = mods;        // The total number of modules
    fNlayers  = nlayers;     // The number of layers.
    fNlad.Set(nlayers,nlads);//[] Array of the number of ladders/layer(layer)
    fNdet.Set(nlayers,ndets);//[] Array of the number of detector/ladder(layer)
    fPixelX.Set(fNlayers);
    fPixelY.Set(fNlayers);
    fPixelZ.Set(fNlayers);
    fNPixelX.Set(fNlayers);
    fNPixelY.Set(fNlayers);
    fNPixelZ.Set(fNlayers);

    fGm.Clear();
    fGm.Expand(mods);        // Structure of translation. and rotation.
    fGm.SetOwner(kTRUE);
    fShape.Clear();
    fShape2.Clear();


    IlcTARGETParam *Param = new IlcTARGETParam(GetDetVersion());

    fShape.Expand(Param->GetNlayers()+Param->GetNendcaps()+Param->GetNlayersStrip()+Param->GetNendcapsStrip());         // Array of shapes and detector information.
    fShape2.Expand(Param->GetNlayers()+Param->GetNendcaps()+Param->GetNlayersStrip()+Param->GetNendcapsStrip());         // Array of shapes and detector information.
//     fShape.SetOwner(kTRUE);
    return;
}
//______________________________________________________________________
void IlcTARGETgeom::CreatMatrix(Int_t mod,Int_t lay,Int_t lad,Int_t det,
			     IlcTARGETDetector idet,const Double_t tran[3],
			     const Double_t rot[10]){
    // Given the translation vector tran[3] and the rotation matrix rot[1],
    // this function creates and adds to the TObject Array fGm the
    // IlcTARGETgeomMatrix object.
    // The rot[10] matrix is set up like:
    /*   / rot[0]  rot[1]  rot[2] \
    //  |  rot[3]  rot[4]  rot[5]  |
    //   \ rot[6]  rot[7]  rot[8] /  if(rot[9]!=0) then the Identity matrix
    // is used regardless of the values in rot[0]-rot[8].
    */
    // Inputs:
    //    Int_t           mod     The module number. The location in TObjArray
    //    Int_t           lay     The layer where this module is
    //    Int_t           lad     On which ladder this module is
    //    Int_t           det     Which detector on this ladder this module is
    //    IlcTARGETDetector idet     The type of detector see IlcTARGETgeom.h
    //    Double_t       tran[3]  The translation vector
    //    Double_t       rot[10]  The rotation matrix.
    // Outputs:
    //    none
    // Return:
    //    none.
    

    
    Int_t id[3];
    Double_t r[3][3] = {{1.0,0.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0}};

    if(mod<0||mod>=fGm.GetSize()){ 
	Error("CreateMatrix","mod=%d is out of bounds max value=%d",mod,
	      fGm.GetSize());
	return;
    } // end if
    delete fGm.UncheckedAt(mod);
    id[0] = lay; id[1] = lad; id[2] = det;
    if(rot[9]!=0.0) { // null rotation
	r[0][0] = rot[0]; r[0][1] = rot[1]; r[0][2] = rot[2];
	r[1][0] = rot[3]; r[1][1] = rot[4]; r[1][2] = rot[5];
	r[2][0] = rot[6]; r[2][1] = rot[7]; r[2][2] = rot[8];
    } // end if
    fGm.AddAt(new IlcTARGETgeomMatrix(idet,id,r,tran),mod);
}
//______________________________________________________________________
IlcTARGETgeom::~IlcTARGETgeom(){
    //     The destructor for the IlcTARGETgeom class. If the arrays fNlad,
    // fNdet, or fGm have had memory allocated to them, there pointer values
    // are non zero, then this memory space is freed and they are set
    // to zero. In addition, fNlayers is set to zero. The destruction of
    // TObjArray fShape is, by default, handled by the TObjArray destructor.
    // Inputs:
    //    none.
    // Outputs:
    //    none.
    // Return:
    //    none.
    return;
}
//______________________________________________________________________
void IlcTARGETgeom::ReadNewFile(const char *filename){
    // It is generally preferred to define the geometry in IlcTARGETgeom
    // directly from the GEANT geometry, see IlcTARGETvPPRasymm.cxx for
    // and example. Under some circumstances this may not be possible.
    // This function will read in a formatted file for all of the
    // information needed to define the geometry in IlcTARGETgeom.
    // Unlike the older file format, this file may contain comments
    // and the order of the data does not need to be completely
    // respected. A file can be created using the function WriteNewFile
    // defined below.
    // Inputs:
    //    const char *filename The file name of the file to be read in.
    // Outputs:
    //     none
    // Return:
    //     none.
    
    

    
    Int_t ncmd=9;
    const char *cmda[]={"Version"        ,"fTrans"  ,"fNmodules",
	                "fNlayers"       ,"fNladers","fNdetectors",
	                "fNDetectorTypes","fShape"  ,"Matrix"};
    Int_t i,j,lNdetTypes,ldet;
    char cmd[20],c;
    IlcTARGETgeomMatrix *m=0;
    ifstream *fp=0;
    char *filtmp=0;
    Bool_t arrayGm = kFALSE, arrayShape = kFALSE;

    filtmp = gSystem->ExpandPathName(filename);
    IlcInfo(Form("Reading New .det file %s",filtmp));
    fp = new ifstream(filtmp,ios::in);  // open file to write
    while(fp->get(c)!=NULL){ // for ever loop
        if(c==' ') continue; // remove blanks
        if(c=='\n') continue;
        if(c=='#' || c=='!') {while(fp->get(c)) if(c=='\n') break; continue;}
        if(c=='/'){
            fp->get(c);{
                if(c=='/') {while(fp->get(c)) if(c=='\n') break; continue;}
                if(c=='*'){
                  NotYet:
                    while(fp->get(c)) if(c=='*') break;
                    fp->get(c);{
                        if(c=='/') continue;
                        goto NotYet;
                    } //
                } // end if c=='*'
            } // end if second /
        } // end if first /
        fp->putback(c);
        *fp >> cmd;
        for(i=0;i<ncmd;i++) if(strcmp(cmd,cmda[i])==0) break;
        switch (i){
        case 0:   // Version
	    while(isspace(fp->peek())) fp->get(); // skip spaces
	    if(isdigit(fp->peek())){ // new TString
		*fp >> j;
		fVersion.Resize(j);
		for(j=0;j<fVersion.Length();j++) *fp >> fVersion[j];
	    }else{
		fVersion.Resize(20);
		for(j=0;isprint(fp->peek())&&j<20;j++) *fp >> fVersion[j];
	    } // end if isdigit
            break;
        case 1:  // fTrans
            *fp >> fTrans;
            break;
        case 2:  // fNModules
            *fp >> fNmodules;
            fGm.Clear();
            fGm.Expand(fNmodules);
            fGm.SetOwner(kTRUE);
            arrayGm = kTRUE;
            break;
        case 3:  // fNlayers
            *fp >> fNlayers;
            fNlad.Set(fNlayers);
            fNdet.Set(fNlayers);
            break;
        case 4:  // fNladers
            for(j=0;j<fNlayers;j++) *fp >> fNlad[j];
            break;
        case 5:  // fNdetectors
            for(j=0;j<fNlayers;j++) *fp >> fNdet[j];
            break;
        case 6:  // fNDetectorTypes
            *fp >> lNdetTypes;
            fShape.Clear();
            fShape.Expand(lNdetTypes);
            fShape.SetOwner(kTRUE);
            arrayShape = kTRUE;
            break;
        case 7:  // fShape
            *fp >> ldet;
            if(!arrayShape) fShape.Expand(5);
            fShape.SetOwner(kTRUE);
            switch (ldet){
            case kSPD :{
                IlcTARGETgeomSPD *spd = new IlcTARGETgeomSPD();
                *fp >> *spd;
                ReSetShape(ldet,spd);
            } break;
            case kSDD : case kSDDp:{
                IlcTARGETgeomSDD *sdd = new IlcTARGETgeomSDD();
                *fp >> *sdd;
                ReSetShape(ldet,sdd);
            }break;
            case kSSD : case kSSDp :{
                IlcTARGETgeomSSD *ssd = new IlcTARGETgeomSSD();
                *fp >> *ssd;
                ReSetShape(ldet,ssd);
            }break;
            default:{
                IlcError(Form("Unknown fShape type number=%d c=%c",ldet,c));
                while(fp->get(c)) if(c=='\n') break; // skip to end of line.
            }break;
            } // end switch
            break;
        case 8:  // Matrix
            *fp >> ldet;
            if(!arrayGm){
                fGm.Clear();
                fGm.Expand(2270);
                arrayGm = kTRUE;
            } // end if
	    if(ldet<0||ldet>=fGm.GetSize()){
		Error("ReadNewFile","ldet<0||ldet>=fGm.GetSize()=%d",
		      ldet,fGm.GetSize());
		return;
	    } // end if
            delete fGm.UncheckedAt(ldet);
            fGm.AddAt((TObject*)new IlcTARGETgeomMatrix(),ldet);
            m = (IlcTARGETgeomMatrix*) fGm.UncheckedAt(ldet);
            *fp >> *m;
            m = 0;
            break;
        default:
            IlcError(Form("ReadNewFile","Data line i=%d c=%c",i,c));
            while(fp->get(c)) if(c=='\n') break; // skip this line
            break;
        } // end switch i
    } // end while
    delete fp;

    return;
}
//______________________________________________________________________
void IlcTARGETgeom::WriteNewFile(const char *filename)const{
    // Writes IlcTARGETgeom, IlcTARGETgeomMatrix, and the defined 
    // IlcTARGETgeomS*D classes to a file in a format that 
    // is more readable and commendable.
    // Inputs:
    //     const char *filename The file name of the file to be write to.
    // Outputs:
    //     none
    // Return:
    //     none
    
    

    
    ofstream *fp;
    Int_t i;
    char *filtmp;

    filtmp = gSystem->ExpandPathName(filename);
    fp = new ofstream(filtmp,ios::out);  // open file to write
    *fp << "//Comment lines begin with two //, one #, or one !" << endl;
    *fp << "#Blank lines are skipped including /* and */ sections." << endl;
    *fp << "!and, in principle the order of the lines is not important" <<endl;
    *fp << "/* In IlcTARGETgeom.h are defined an enumerated type called" << endl;
    *fp << " IlcTARGETDetectors These are kSPD=" << (Int_t) kSPD ;
    *fp << ", kSDD=" << (Int_t) kSDD << ", kSSD=" << (Int_t) kSSD;
    *fp << ", kSSDp=" << (Int_t) kSSDp << ", and kSDDp=" << (Int_t) kSDDp;
    *fp << "*/" << endl;
    *fp << "Version "<< fVersion.Length()<<" " << fVersion.Data() << endl;//This should be consistent
                                           // with the geometry version.
    *fp << "fTrans " << fTrans << endl;
    *fp << "fNmodules " << fNmodules << endl;
    *fp << "fNlayers " << fNlayers << endl;
    *fp << "fNladers ";
    for(i=0;i<fNlayers;i++) *fp << fNlad[i] << " ";
    *fp << endl;
    *fp << "fNdetectors ";
    for(i=0;i<fNlayers;i++) *fp << fNdet[i] << " ";
    *fp << endl;
    *fp << "fNDetectorTypes " << fShape.GetEntriesFast() << endl;
    for(i=0;i<fShape.GetEntriesFast();i++){
	if(!IsShapeDefined(i)) continue; // only print out used shapes.
	switch (i){
	case kSPD :
	    *fp << "fShape " << (Int_t) kSPD << " ";
            *fp << *((IlcTARGETgeomSPD*)(fShape.UncheckedAt(i)));
	    break;
	case kSDD :
	    *fp << "fShape " << (Int_t) kSDD << " ";
            *fp << *((IlcTARGETgeomSDD*)(fShape.UncheckedAt(i)));
	    break;
	case kSSD : case kSSDp :
	    *fp << "fShape " << i << " ";
            *fp << *((IlcTARGETgeomSSD*)(fShape.UncheckedAt(i)));
	    break;
	default:
	    Error("IlcTARGETgeom::WriteNewFile","Unknown Shape value");
	} // end switch (i)
    } // end for i
    for(i=0;i<fNmodules;i++){
	*fp << "Matrix " << i << " ";
	*fp << *GetGeomMatrix(i);
    } // end for i
    *fp << "//End of File" << endl;;

    delete fp;
    return;
}
//______________________________________________________________________
IlcTARGETgeom::IlcTARGETgeom(const char *filename):
TObject(),
fVersion("test"),// Transformation version.
fTrans(0),       // Flag to keep track of which transformation 
fNmodules(0),    // The total number of modules
fNlayers(0),     // The number of layers.
fNlad(),         // TArrayI of the number of ladders/layer(layer)
fNdet(),         // TArrayI of the number of detector/ladder(layer)
fGm(0,0),        // TObjArray Structure of translation. and rotation.
fShape(0,0),      // TObjArray of detector geom.
fShape2(0,0)      // TObjArray of detector geom.
{
    //     The constructor for the IlcTARGETgeom class. All of the data to fill
    // this structure is read in from the file given my the input filename.
    // Inputs:
    //    const char *filename The file name of the file to be read in.
    // Outputs:
    //    none
    // Return:
    //    An IlcTARGETgeom class initialized from a file.
    
    

    
    FILE     *pf=0;
    Int_t    i,lm=0,id[3];
    Int_t    l,a,d;
    Float_t  x,y,z,o,p,q,r,s,t;
    Double_t rot6[6],tran[3];
    char     buf[200],*buff=0; // input character buffer;
    char    *filtmp;

    filtmp = gSystem->ExpandPathName(filename);
    Info("IlcTARGETgeom","reading old .det file %s",filtmp);
    fVersion="GEANT5";
    pf = fopen(filtmp,"r");

    fNlayers = 6; // set default number of ladders
  TryAgain:
    fNlad.Set(fNlayers);
    fNdet.Set(fNlayers);
    fNmodules = 0;
    // find the number of ladders and detectors in this geometry.
    for(i=0;i<fNlayers;i++){fNlad[i]=fNdet[i]=0;} // zero out arrays
    while(fgets(buf,200,pf)!=NULL){ // for ever loop
        for(i=0;i<200;i++)if(buf[i]!=' '){ // remove blank spaces.
            buff = &(buf[i]);
            break;
        } // end for i
        // remove blank lines and comments.
        if(buff[0]=='\n'||buff[0]=='#'||buff[0]=='!'||
           (buff[0]=='/'&&buff[1]=='/')) continue;
        if(isalpha(buff[0])) { // must be the new file formated file.
            fclose(pf);
            ReadNewFile(filename);
            return;
        } // end if isalpha(buff[0])
        sscanf(buff,"%d %d %d %f %f %f %f %f %f %f %f %f",
               &l,&a,&d,&x,&y,&z,&o,&p,&q,&r,&s,&t);
        if(l>lm) lm = l;
        if(l<1 || l>fNlayers) {
            printf("error in file %s layer=%d min. is 1 max is %d Trying new format\n",
                   filename,l,fNlayers);
            fclose(pf);
            ReadNewFile(filename);
            return;
            //continue;
        }// end if l
        fNmodules++;
        if(l<=fNlayers&&fNlad[l-1]<a) fNlad[l-1] = a;
        if(l<=fNlayers&&fNdet[l-1]<d) fNdet[l-1] = d;
    } // end while ever loop
    if(lm>fNlayers){
        fNlayers = lm;
        goto TryAgain;
    } // end if lm>fNlayers
    // counted the number of ladders and detectors now allocate space.
    fGm.Expand(fNmodules);
    fGm.SetOwner(kTRUE);
    fShape.SetOwner(kTRUE);

    // Set up Shapes for a default configuration of 6 layers.
    fTrans   = 0; // standard GEANT global/local coordinate system.
    // prepare to read in transforms
    lm = 0; // reuse lm as counter of modules.
    rewind(pf); // start over reading file
    while(fgets(buf,200,pf)!=NULL){ // for ever loop
        for(i=0;i<200;i++)if(buf[i]!=' '){ // remove blank spaces.
            buff = &(buf[i]);
            break;
        } // end for i
        // remove blank lines and comments.
        if(buff[0]=='\n'||buff[0]=='#'||buff[0]=='!'||
           (buff[0]=='/'&&buff[1]=='/')) continue;
        x = y = z = o = p = q = r = s = t = 0.0;
        sscanf(buff,"%d %d %d %f %f %f %f %f %f %f %f %f",
               &l,&a,&d,&x,&y,&z,&o,&p,&q,&r,&s,&t);
        if(l<1 || l>fNlayers) {
            Warning("IlcTARGETgeom","error in file %s layer=%d min. is 1 max is %d",
                   filename,l,fNlayers);
            continue;
        }// end if l
        id[0] = l;id[1] = a;id[2] = d;
        tran[0] = tran[1] = tran[2]  = 0.0;
        tran[0] = (Double_t)x;tran[1] = (Double_t)y;tran[2] = (Double_t)z;
        rot6[0] = rot6[1] = rot6[2] = rot6[3] = rot6[4] = rot6[5] =0.0;
        rot6[0] = (Double_t)o;rot6[1] = (Double_t)p;rot6[2] = (Double_t)q;
        rot6[3] = (Double_t)r;rot6[4] = (Double_t)s;rot6[5] = (Double_t)t;
	if(lm<0||lm>=fGm.GetSize()){
	    Error("IlcTARGETgeom(filename)","lm<0||lm>=fGm.GetSize()=%d",
		  lm,fGm.GetSize());
	    return;
	} // end if
        switch (l){
        case 1: case 2: // layer 1 or2 SPD
            fGm.AddAt(new IlcTARGETgeomMatrix(rot6,kSPD,id,tran),lm++);
            break;
        case 3: case 4: // layer 3 or 4 SDD
            fGm.AddAt(new IlcTARGETgeomMatrix(rot6,kSDD,id,tran),lm++);
            break;
        case 5: case 6: // layer 5 or 6 SSD
            fGm.AddAt(new IlcTARGETgeomMatrix(rot6,kSSD,id,tran),lm++);
            break;
        } // end switch
    } // end while ever loop
    fclose(pf);
}
//______________________________________________________________________
IlcTARGETgeom::IlcTARGETgeom(const IlcTARGETgeom &source) : TObject(source){
    //     The copy constructor for the IlcTARGETgeom class. It calls the
    // = operator function. See the = operator function for more details.
    // Inputs:
    //     IlcTARGETgeom &source  The IlcTARGETgeom class with which to make this
    //                         a copy of.
    // Outputs:
    //     none.
    // Return:
    //     none.


    
    *this = source;  // Just use the = operator for now.
    return;
}
//______________________________________________________________________
IlcTARGETgeom& IlcTARGETgeom::operator=(const IlcTARGETgeom &source){
    //     The = operator function for the IlcTARGETgeom class. It makes an
    // independent copy of the class in such a way that any changes made
    // to the copied class will not affect the source class in any way.
    // This is required for many TARGET alignment studies where the copied
    // class is then modified by introducing some misalignment.
    // Inputs:
    //     IlcTARGETgeom &source  The IlcTARGETgeom class with which to make this
    //                         a copy of.
    // Outputs:
    //     none.
    // Return:
    //     *this The a new copy of source.
    Int_t i;

    if(this == &source) return *this; // don't assign to ones self.

    // if there is an old structure allocated delete it first.
    this->fGm.Clear();
    this->fShape.Clear();
    this->fShape2.Clear();

    this->fVersion  = source.fVersion;
    this->fTrans    = source.fTrans;
    this->fNmodules = source.fNmodules;
    this->fNlayers = source.fNlayers;
    this->fNlad.Set(fNlayers,source.fNlad.GetArray());
    this->fNdet.Set(fNlayers,source.fNdet.GetArray());
    this->fShape.Expand(source.fShape.GetEntriesFast());
    this->fShape2.Expand(source.fShape2.GetEntriesFast());
    for(i=0;i<source.fShape.GetEntriesFast();i++)
        this->fShape.AddAt(new TObject(*(source.fShape.At(i))),i);
    for(i=0;i<source.fShape2.GetEntriesFast();i++)
        this->fShape2.AddAt(new TObject(*(source.fShape2.At(i))),i);

    this->fShape.SetOwner(kTRUE);
    this->fShape2.SetOwner(kTRUE);
    this->fGm.Expand(this->fNmodules);
    this->fGm.SetOwner(kTRUE);
    for(i=0;i<this->fNmodules;i++)
	if(i<0||i>=fGm.GetSize()){
	    Error("ReadNewFile","i<0||i>=fGm.GetSize()=%d",
		  i,fGm.GetSize());
	    return *this;
	} // end if
        this->fGm.AddAt(new TObject(*(source.fGm.UncheckedAt(i))),i);
    return *this;
}
//______________________________________________________________________
Int_t IlcTARGETgeom::GetModuleIndex(Int_t lay,Int_t lad,Int_t det)const{
    //      This routine computes the module index number from the layer,
    // ladder, and detector numbers. The number of ladders and detectors
    // per layer is determined when this geometry package is constructed,
    // see IlcTARGETgeom(const char *filename) for specifics.
    // Inputs:
    //    Int_t lay  The layer number. Starting from 1.
    //    Int_t lad  The ladder number. Starting from 1.
    //    Int_t det  The detector number. Starting from 1.
    // Outputs:
    //    none.
    // Return:
    //    the module index number, starting from zero.
    

    
    Int_t i,j,k,id[3];
    
    if(IlcDebugLevelClass()>0)
       IlcDebug(1, Form("layer %d, ladder %d, detector %d", lay, lad, det));

    i = fNdet[lay-1] * (lad-1) + det - 1;   
    j = 0;
    for(k=0;k<lay-1;k++) j += fNdet[k]*fNlad[k];    
    i = i+j;
    if(i>=fNmodules) return -1;

    GetGeomMatrix(i)->GetIndex(id);

    
    if(IlcDebugLevelClass()>0)
      IlcDebug(1, Form(" id[0]= %d, id[1]= %d, id[2]= %d, index= %d", id[0], id[1], id[2], i));
    
    if(id[0]==lay&&id[1]==lad&&id[2]==det) return i;
    
    // Array of modules fGm is not in expected order. Search for this index
    for(i=0;i<fNmodules;i++){
        GetGeomMatrix(i)->GetIndex(id);
	
	if(IlcDebugLevelClass()>0)
	  IlcDebug(1, Form(" id[0]= %d, id[1]= %d, id[2]= %d, index= %d", id[0], id[1], id[2], i));
	
        if(id[0]==lay&&id[1]==lad&&id[2]==det) return i;
	
    } // end for i
    // This layer ladder and detector combination does not exist return -1.
    return -1;
}
//______________________________________________________________________
void IlcTARGETgeom::GetModuleId(Int_t index,Int_t &lay,Int_t &lad,Int_t &det)const{
    //      This routine computes the layer, ladder and detector number 
    // given the module index number. The number of ladders and detectors
    // per layer is determined when this geometry package is constructed,
    // see IlcTARGETgeom(const char *filename) for specifics.
    // Inputs:
    //     Int_t index  The module index number, starting from zero.
    // Outputs:
    //     Int_t lay    The layer number. Starting from 1.
    //     Int_t lad    The ladder number. Starting from 1.
    //     Int_t det    The detector number. Starting from 1.
    // Return:
    //     none.
    

    
    Int_t id[3];
    IlcTARGETgeomMatrix *g = GetGeomMatrix(index);

    if (g == 0x0){
        Error("GetModuleId","Can not get GeoMatrix for index = %d",index);
        lay = -1; lad = -1; det = -1;
    }else{
        g->GetIndex(id);
        lay = id[0]; lad = id[1]; det = id[2];
    
// // // //       IlcInfo(Form("index: %d, lay: %d, lad: %d, det: %d", index, lay, lad, det));
    }// End if
    return;
    // The old way kept for posterity.
/*
    Int_t i,j,k;
    j = 0;
    for(k=0;k<fNlayers;k++){
	j += fNdet[k]*fNlad[k];
	if(j>index)break;
    } // end for k
    lay = k+1;
    i = index -j + fNdet[k]*fNlad[k];
    j = 0;
    for(k=0;k<fNlad[lay-1];k++){
	j += fNdet[lay-1];
	if(j>i)break;
    } // end for k
    lad = k+1;
    det = 1+i-fNdet[lay-1]*k;
    return;
*/
}
//______________________________________________________________________
Int_t IlcTARGETgeom::GetNDetTypes(Int_t &max)const{
    // Finds and returns the number of detector types used and the
    // maximum detector type value. Only counts id >=0 (no undefined
    // values. See IlcTARGETgeom.h for list of IlcTARGETDetecor enumerated types.
    // Inputs:
    //    none.
    // Outputs:
    //    The maximum detector type used
    // Return:
    //    The number of detector types used
    

    
    Int_t i,*n,id;

    max = -1;
    for(i=0;i<GetIndexMax();i++){
        id = GetModuleType(i);
        if(id>max) max=id;
    } // end for i
    n = new Int_t[max+1];
    for(i=0;i<max;i++) n[i] = 0;
    for(i=0;i<GetIndexMax();i++){
        id = GetModuleType(i);
        if(id>-1)n[id]++; // note id=-1 => undefined.
    } // end for i
    id = 0;
    for(i=0;i<max;i++) if(n[i]!=0) id++;
    delete[] n;
    return id+1;
}
//______________________________________________________________________
Int_t IlcTARGETgeom::GetNDetTypes(TArrayI &maxs,IlcTARGETDetector *types)const{
    // Finds and returns the number of detector types used and the
    // number of each detector type. Only counts id >=0 (no undefined
    // values. See IlcTARGETgeom.h for list of IlcTARGETDetecor enumerated types.
    // Inputs:
    //    none.
    // Outputs:
    //    The maximum detector type used
    // Return:
    //    The number of detector types used
    

    
    Int_t i,j,*n,id,max;

    max = -1;
    for(i=0;i<GetIndexMax();i++){
        id = GetModuleType(i);
        if(id>max) max=id;
    } // end for i
    n = new Int_t[max+1];
    for(i=0;i<max;i++) n[i] = 0;
    for(i=0;i<GetIndexMax();i++){
        id = GetModuleType(i);
        if(id>-1)n[id]++; // note id=-1 => undefined.
    } // end for i
    id = 0;
    for(i=0;i<=max;i++) if(n[i]!=0) id++;
    maxs.Set(id);
    j = 0;
    for(i=0;i<=max;i++) if(n[i]!=0){
        maxs[j] = n[i];
        types[j++] = (IlcTARGETDetector) i;
    } // end for i/end if
    delete[] n;
    return id;
}
//______________________________________________________________________
Int_t IlcTARGETgeom::GetStartDet(Int_t dtype)const{
    // returns the starting module index value for a give type of detector id.
    // This assumes that the detector types are different on different layers
    // and that they are not mixed up.
    // Inputs:
    //    Int_t dtype A detector type number. 0 for SPD, 1 for SDD, and 2 for SSD.
    // Outputs:
    //    none.
    // Return:
    //    the module index for the first occurrence of that detector type.

    switch(dtype){
    case 0:
        return GetModuleIndex(1,1,1);
        break;
// // // //     case 1:
// // // //         return GetModuleIndex(14,1,1);
// // // //         break;
// // // //     case 2:
// // // //         return GetModuleIndex(5,1,1);
// // // //         break;
    default:
        Warning("GetStartDet","undefined detector type %d",dtype);
        return 0;
    } // end switch

    Warning("GetStartDet","undefined detector type %d",dtype);
    return 0;
}
//______________________________________________________________________
Int_t IlcTARGETgeom::GetLastDet(Int_t dtype)const{
    // returns the last module index value for a give type of detector id.
    // This assumes that the detector types are different on different layers
    // and that they are not mixed up.
    // Inputs:
    //     Int_t dtype A detector type number. 0 for SPD, 1 for SDD, and 2 for SSD.
    // Outputs:
    // Return:
    //     the module index for the last occurrence of that detector type.
    
    


    switch((IlcTARGETDetector)dtype){
    case kSPD:
        return GetModuleIndex(GetNlayers(),GetNladders(GetNlayers()),1);
        break;
    case kSDD:
        return GetModuleIndex(5,1,1)-1;
        break;
    case kSSD:
        return GetIndexMax()-1;
        break;
    case kSSDp: case kSDDp: case kND:
    default:
        Warning("GetLastDet","undefined detector type %d",dtype);
        return 0;
    } // end switch

    Warning("GetLastDet","undefined detector type %d",dtype);
    return 0;
}
//______________________________________________________________________
void IlcTARGETgeom::PrintComparison(FILE *fp,IlcTARGETgeom *other)const{
    //     This function was primarily created for diagnostic reasons. It
    // print to a file pointed to by the file pointer fp the difference
    // between two IlcTARGETgeom classes. The format of the file is basically,
    // define d? to be the difference between the same element of the two
    // classes. For example dfrx = this->GetGeomMatrix(i)->frx 
    // - other->GetGeomMatrix(i)->frx.
    // if(at least one of dfx0, dfy0, dfz0,dfrx,dfry,dfrz are non zero) then
    // print layer ladder detector dfx0 dfy0 dfz0 dfrx dfry dfrz
    // if(at least one of the 9 elements of dfr[] are non zero) then print
    // layer ladder detector dfr[0] dfr[1] dfr[2]
    //                       dfr[3] dfr[4] dfr[5]
    //                       dfr[6] dfr[7] dfr[8]
    // Only non zero values are printed to save space. The differences are
    // typical written to a file because there are usually a lot of numbers
    // printed out and it is usually easier to read them in some nice editor
    // rather than zooming quickly past you on a screen. fprintf is used to
    // do the printing. The fShapeIndex difference is not printed at this time.
    // Inputs:
    //    FILE *fp           A file pointer to an opened file for writing in which
    //                       the results of the comparison will be written.
    //    IlcTARGETgeom *other  The other IlcTARGETgeom class to which this one is
    //                       being compared.
    // Outputs:
    //    none.
    // Return:
    //    none.
    
    

    
    Int_t    i,j,idt[3],ido[3];
    Double_t tt[3],to[3];  // translation
    Double_t rt[3],ro[3];  // phi in radians
    Double_t mt[3][3],mo[3][3]; // matrices
    IlcTARGETgeomMatrix *gt,*go;
    Bool_t   t;

    for(i=0;i<this->fNmodules;i++){
        gt  =  this->GetGeomMatrix(i);
        go  = other->GetGeomMatrix(i);
        gt->GetIndex(idt);
        go->GetIndex(ido);
        t = kFALSE;
        for(i=0;i<3;i++) t = t&&idt[i]!=ido[i];
        if(t) fprintf(fp,"%4.4d %1.1d %2.2d %2.2d %1.1d %2.2d %2.2d\n",i,
                      idt[0],idt[1],idt[2],ido[0],ido[1],ido[2]);
        gt->GetTranslation(tt);
        go->GetTranslation(to);
        gt->GetAngles(rt);
        go->GetAngles(ro);
        t = kFALSE;
        for(i=0;i<3;i++) t = t&&tt[i]!=to[i];
        if(t) fprintf(fp,"%1.1d %2.2d %2.2d dTrans=%f %f %f drot=%f %f %f\n",
                      idt[0],idt[1],idt[2],
                      tt[0]-to[0],tt[1]-to[1],tt[2]-to[2],
                      rt[0]-ro[0],rt[1]-ro[1],rt[2]-ro[2]);
        t = kFALSE;
        gt->GetMatrix(mt);
        go->GetMatrix(mo);
        for(i=0;i<3;i++)for(j=0;j<3;j++)  t = mt[i][j] != mo[i][j];
        if(t){
            fprintf(fp,"%1.1d %2.2d %2.2d dfr= %e %e %e\n",
                    idt[0],idt[1],idt[2],
                    mt[0][0]-mo[0][0],mt[0][1]-mo[0][1],mt[0][2]-mo[0][2]);
            fprintf(fp,"        dfr= %e %e %e\n",
                    mt[1][0]-mo[1][0],mt[1][1]-mo[1][1],mt[1][2]-mo[1][2]);
            fprintf(fp,"        dfr= %e %e %e\n",
                    mt[2][0]-mo[2][0],mt[2][1]-mo[2][1],mt[2][2]-mo[2][2]);
        } // end if t
    } // end for i
    return;
}
//______________________________________________________________________
void IlcTARGETgeom::PrintData(FILE *fp,Int_t lay,Int_t lad,Int_t det)const{
    //     This function prints out the coordinate transformations for
    // the particular detector defined by layer, ladder, and detector
    // to the file pointed to by the File pointer fp. fprintf statements
    // are used to print out the numbers. The format is
    // layer ladder detector Trans= fx0 fy0 fz0 rot= frx fry frz 
    // Shape=fShapeIndex
    //                         dfr= fr[0] fr[1] fr[2]
    //                         dfr= fr[3] fr[4] fr[5]
    //                         dfr= fr[6] fr[7] fr[8]
    // By indicating which detector, some control over the information 
    // is given to the user. The output it written to the file pointed
    // to by the file pointer fp. This can be set to stdout if you want.
    // Inputs:
    //     FILE *fp           A file pointer to an opened file for writing in which
    //                        the results of the comparison will be written.
    //     Int_t lay          The layer number. Starting from 1.
    //     Int_t lad          The ladder number. Starting from 1.
    //     Int_t det          The detector number. Starting from 1.
    // Outputs:
    //     none
    // Return:
    //     none.
    
    

    
    IlcTARGETgeomMatrix *gt;
    Double_t t[3],r[3],m[3][3];

    gt = this->GetGeomMatrix(GetModuleIndex(lay,lad,det));
    gt->GetTranslation(t);
    gt->GetAngles(r);
    fprintf(fp,"%1.1d %2.2d %2.2d Trans=%f %f %f rot=%f %f %f Shape=%d\n",
            lay,lad,det,t[0],t[1],t[2],r[0],r[1],r[2],
            gt->GetDetectorIndex());
    gt->GetMatrix(m);
    fprintf(fp,"        dfr= %e %e %e\n",m[0][0],m[0][1],m[0][2]);
    fprintf(fp,"        dfr= %e %e %e\n",m[1][0],m[1][1],m[1][2]);
    fprintf(fp,"        dfr= %e %e %e\n",m[2][0],m[2][1],m[2][2]);
    return;
}
//______________________________________________________________________
ofstream & IlcTARGETgeom::PrintGeom(ofstream &rb)const{
    //     Stream out an object of class IlcTARGETgeom to standard output.
    // Intputs:
    //     ofstream &rb    The output streaming buffer.
    // Outputs:
    //     none.
    // Return:
    //     ofstream &rb    The output streaming buffer.
    

    
    Int_t i,nshapes;

    rb.setf(ios::scientific);
    rb << fTrans << " ";
    rb << fNmodules << " ";
    rb << fNlayers << " ";
    for(i=0;i<fNlayers;i++) rb << fNlad[i] << " ";
    for(i=0;i<fNlayers;i++) rb << fNdet[i] << "\n";
    for(i=0;i<fNmodules;i++) {
        rb <<setprecision(16) << *(GetGeomMatrix(i)) << "\n";
    } // end for i
    nshapes = fShape.GetEntries();
    rb << nshapes <<endl;
    for(i=0;i<nshapes;i++) if(fShape.UncheckedAt(i)!=0) switch (i){
    case kSPD:
      rb << kSPD <<","<< (IlcTARGETgeomSPD*)(fShape.UncheckedAt(kSPD));
        break;
    case kSDD:
      rb << kSDD <<","<< (IlcTARGETgeomSDD*)(fShape.UncheckedAt(kSDD));
        break;
    case kSSD:
      rb << kSSD <<","<< (IlcTARGETgeomSSD*)(fShape.UncheckedAt(kSSD));
        break;
    case kSSDp:
      rb << kSSDp <<","<< (IlcTARGETgeomSSD*)(fShape.UncheckedAt(kSSDp));
        break;
    case kSDDp:
      rb << kSDDp <<","<< (IlcTARGETgeomSDD*)(fShape.UncheckedAt(kSDDp));
        break;
    } // end for i / switch
    return rb;
}
//______________________________________________________________________
ifstream & IlcTARGETgeom::ReadGeom(ifstream &rb){
    //     Stream in an object of class IlcTARGETgeom from standard input.
    // Intputs:
    //     ifstream &rb    The input streaming buffer.
    // Outputs:
    //     none.
    // Return:
    //     ifstream &rb    The input streaming buffer.
    
    

    
    Int_t i,j;

    fGm.Clear();

    rb >> fTrans >> fNmodules >> fNlayers;
    fNlad.Set(fNlayers);
    fNdet.Set(fNlayers);
    for(i=0;i<fNlayers;i++) rb >> fNlad[i];
    for(i=0;i<fNlayers;i++) rb >> fNdet[i];
    fGm.Expand(fNmodules);
    fGm.SetOwner(kTRUE);
    for(i=0;i<fNmodules;i++){
	if(i<0||i>=fGm.GetSize()){
	    Error("ReadGeom","i<0||i>=fGm.GetSize()=%d",
		  i,fGm.GetSize());
	    return rb;
	} // end if
        fGm.AddAt(new IlcTARGETgeomMatrix,i);
        rb >> *(GetGeomMatrix(i));
    } // end for i
    rb >> i;
    fShape.Expand(i);
    fShape.SetOwner(kTRUE);
    for(i=0;i<fShape.GetEntries();i++) {
        rb >> j;
        switch (j){
        case kSPD:{
            IlcTARGETgeomSPD *s = new IlcTARGETgeomSPD();
            rb >> *s;
            fShape.AddAt(s,kSPD);}
            break;
        case kSDD:{
            IlcTARGETgeomSDD *s = new IlcTARGETgeomSDD();
            rb >> *s;
            fShape.AddAt(s,kSDD);}
            break;
        case kSSD:{
            IlcTARGETgeomSSD *s = new IlcTARGETgeomSSD();
            rb >> *s;
            fShape.AddAt(s,kSSD);}
            break;
        case kSSDp:{
            IlcTARGETgeomSSD *s = new IlcTARGETgeomSSD();
            rb >> *s;
            fShape.AddAt(s,kSSDp);}
            break;
        case kSDDp:{
            IlcTARGETgeomSDD *s = new IlcTARGETgeomSDD();
            rb >> *s;
            fShape.AddAt(s,kSDDp);}
            break;
        } // end  switch
    } //  end for i
    return rb;
}
//______________________________________________________________________
//     The following routines modify the transformation of "this"
// geometry transformations in a number of different ways.
//______________________________________________________________________
void IlcTARGETgeom::GlobalChange(const Float_t *tran,const Float_t *rot){
    //     This function performs a Cartesian translation and rotation of
    // the full TARGET from its default position by an amount determined by
    // the three element arrays tran and rot. If every element
    // of tran and rot are zero then there is no change made
    // the geometry. The change is global in that the exact same translation
    // and rotation is done to every detector element in the exact same way.
    // The units of the translation are those of the Monte Carlo, usually cm,
    // and those of the rotation are in radians. The elements of tran
    // are tran[0] = x, tran[1] = y, and tran[2] = z.
    // The elements of rot are rot[0] = rx, rot[1] = ry, and
    // rot[2] = rz. A change in x will move the whole TARGET in the ILC
    // global x direction, the same for a change in y. A change in z will
    // result in a translation of the TARGET as a whole up or down the beam line.
    // A change in the angles will result in the inclination of the TARGET with
    // respect to the beam line, except for an effective rotation about the
    // beam axis which will just rotate the TARGET as a hole about the beam axis.
    // Intputs:
    //     Float_t *tran   A 3 element array representing the global translations.
    //                     the elements are x,y,z in cm.
    //     Float_t *rot    A 3 element array representing the global rotation
    //                     angles about the three axis x,y,z in radians
    // Outputs:
    //     none.
    // Return:
    //     none.
    
    

    
    Int_t    i,j;
    Double_t t[3],r[3];
    IlcTARGETgeomMatrix *g;

    fTrans = (fTrans && 0xfffd) + 2;  // set bit 1 true.
    for(i=0;i<fNmodules;i++){
        g = this->GetGeomMatrix(i);
        g->GetTranslation(t);
        g->GetAngles(r);
        for(j=0;j<3;j++){
            t[j] += tran[j];
            r[j] += rot[j];
        } // end for j
        g->SetTranslation(t);
        g->SetAngles(r);
    } // end for i
    return;
}
//______________________________________________________________________
void IlcTARGETgeom::GlobalCylindericalChange(const Float_t *tran,
					  const Float_t *rot){
    //     This function performs a cylindrical translation and rotation of
    // each TARGET element by a fixed about in radius, rphi, and z from its
    // default position by an amount determined by the three element arrays
    // tran and rot. If every element of tran and
    // rot are zero then there is no change made the geometry. The
    // change is global in that the exact same distance change in translation
    // and rotation is done to every detector element in the exact same way.
    // The units of the translation are those of the Monte Carlo, usually cm,
    // and those of the rotation are in radians. The elements of tran
    // are tran[0] = r, tran[1] = rphi, and tran[2] = z.
    // The elements of rot are rot[0] = rx, rot[1] = ry, and
    // rot[2] = rz. A change in r will results in the increase of the
    // radius of each layer by the same about. A change in rphi will results in
    // the rotation of each layer by a different angle but by the same
    // circumferential distance. A change in z will result in a translation
    // of the TARGET as a hole up or down the beam line. A change in the angles
    // will result in the inclination of the TARGET with respect to the beam
    // line, except for an effective rotation about the beam axis which will
    // just rotate the TARGET as a hole about the beam axis.
    // Intputs:
    //     Float_t *tran   A 3 element array representing the global translations.
    //                     the elements are r,theta,z in cm/radians.
    //     Float_t *rot    A 3 element array representing the global rotation
    //                     angles about the three axis x,y,z in radians
    // Outputs:
    //     none.
    // Return:
    //     none.
    
    

    
    Int_t    i,j;
    Double_t t[3],ro[3],r,r0,phi,rphi;
    IlcTARGETgeomMatrix *g;

    fTrans = (fTrans && 0xfffd) + 2;  // set bit 1 true.
    for(i=0;i<fNmodules;i++){
        g = this->GetGeomMatrix(i);
        g->GetTranslation(t);
        g->GetAngles(ro);
        r = r0= TMath::Hypot(t[1],t[0]);
        phi   = TMath::ATan2(t[1],t[0]);
        rphi  = r0*phi;
        r    += tran[0];
        rphi += tran[1];
        phi   = rphi/r0;
        t[0]  = r*TMath::Cos(phi);
        t[1]  = r*TMath::Sin(phi);
        t[2] += tran[2];
        for(j=0;j<3;j++){
            ro[j] += rot[j];
        } // end for j
        g->SetTranslation(t);
        g->SetAngles(ro);
    } // end for i
    return;
}
//______________________________________________________________________
void IlcTARGETgeom::RandomChange(const Float_t *stran,const Float_t *srot){
    //     This function performs a Gaussian random displacement and/or
    // rotation about the present global position of each active
    // volume/detector of the TARGET. The sigma of the random displacement
    // is determined by the three element array stran, for the
    // x y and z translations, and the three element array srot,
    // for the three rotation about the axis x y and z.
    // Intputs:
    //     Float_t *stran  A 3 element array representing the global translations
    //                     variances. The elements are x,y,z in cm.
    //     Float_t *srot   A 3 element array representing the global rotation
    //                     angles variances about the three axis x,y,z in radians.
    // Outputs:
    //     none.
    // Return:
    //     none.
    
    

    
    Int_t    i,j;
    Double_t t[3],r[3];
    IlcTARGETgeomMatrix *g;

    fTrans = (fTrans && 0xfffd) + 2;  // set bit 1 true.
    for(i=0;i<fNmodules;i++){
        g = this->GetGeomMatrix(i);
        g->GetTranslation(t);
        g->GetAngles(r);
        for(j=0;j<3;j++){
            t[j] += gRandom->Gaus(0.0,stran[j]);
            r[j] += gRandom->Gaus(0.0, srot[j]);
        } // end for j
        g->SetTranslation(t);
        g->SetAngles(r);
    } // end for i
    return;
}
//______________________________________________________________________
void IlcTARGETgeom::RandomCylindericalChange(const Float_t *stran,
					  const Float_t *srot){
    //     This function performs a Gaussian random displacement and/or
    // rotation about the present global position of each active
    // volume/detector of the TARGET. The sigma of the random displacement
    // is determined by the three element array stran, for the
    // r rphi and z translations, and the three element array srot,
    // for the three rotation about the axis x y and z. This random change
    // in detector position allow for the simulation of a random uncertainty
    // in the detector positions of the TARGET.
    // Intputs:
    //     Float_t *stran  A 3 element array representing the global translations
    //                     variances. The elements are r,theta,z in cm/radians.
    //     Float_t *srot   A 3 element array representing the global rotation
    //                     angles variances about the three axis x,y,z in radians.
    // Outputs:
    //     none.
    // Return:
    //     none.
    
    

    
    Int_t    i,j;
    Double_t t[3],ro[3],r,r0,phi,rphi;
    TRandom ran;
    IlcTARGETgeomMatrix *g;

    fTrans = (fTrans && 0xfffd) + 2;  // set bit 1 true.
    for(i=0;i<fNmodules;i++){
        g = this->GetGeomMatrix(i);
        g->GetTranslation(t);
        g->GetAngles(ro);
        r = r0= TMath::Hypot(t[1],t[0]);
        phi   = TMath::ATan2(t[1],t[0]);
        rphi  = r0*phi;
        r    += ran.Gaus(0.0,stran[0]);
        rphi += ran.Gaus(0.0,stran[1]);
        phi   = rphi/r0;
        t[0]  = r*TMath::Cos(phi);
        t[1]  = r*TMath::Sin(phi);
        t[2] += ran.Gaus(0.0,stran[2]);
        for(j=0;j<3;j++){
            ro[j] += ran.Gaus(0.0, srot[j]);
        } // end for j
        g->SetTranslation(t);
        g->SetAngles(ro);
    } // end for i
    return;
}
//______________________________________________________________________
void IlcTARGETgeom::GeantToTracking(const IlcTARGETgeom &source){
    //     Copy the geometry data but change it to go between the ILC
    // Global coordinate system to that used by the TARGET tracking. A slightly
    // different coordinate system is used when tracking. This coordinate 
    // system is only relevant when the geometry represents the cylindrical
    // ILC TARGET geometry. For tracking the Z axis is left alone but X-> -Y
    // and Y-> X such that X always points out of the TARGET cylinder for every
    // layer including layer 1 (where the detectors are mounted upside down).
    //Begin_Html
    /*
      <img src="picts/TARGET/IlcTARGETgeomMatrix_T1.gif">
    */
    //End_Html
    // Input:
    //     IlcTARGETgeom &source  The IlcTARGETgeom class with which to make this
    //                         a copy of.
    // Output:
    //     none.
    // Return:
    //     none.
    

    
    Int_t    i,j,k,l,id[3];
    Double_t r0[3][3],r1[3][3];
    Double_t a1[3][3] = {{0.,-1.,0.},{+1.,0.,0.},{0.,0.,+1.}};

    *this = source;  // copy everything
    for(i=0;i<GetIndexMax();i++){
        GetGeomMatrix(i)->GetIndex(id);
        GetGeomMatrix(i)->GetMatrix(r0);
//         if(id[0]==1){ // Layer 1 is treated different from the others.
//             for(j=0;j<3;j++) for(k=0;k<3;k++){
//                 r1[j][k] = 0.;
//                 for(l=0;l<3;l++) r1[j][k] += a0[j][l]*r0[l][k];
//             } // end for j,k
//         }else{
            for(j=0;j<3;j++) for(k=0;k<3;k++){
                r1[j][k] = 0.;
                for(l=0;l<3;l++) r1[j][k] += a1[j][l]*r0[l][k];
            } // end for j,k
 //       } // end if
        GetGeomMatrix(i)->SetMatrix(r1);
    } // end for i
    this->fTrans = (this->fTrans && 0xfffe) + 1;  // set bit 0 true.
    return;
}
//______________________________________________________________________
Int_t IlcTARGETgeom::GetNearest(const Double_t g[3],Int_t lay)const{
    //      Finds the Detector (Module) that is nearest the point g [cm] in
    // ILC Global coordinates. If layer !=0 then the search is restricted
    // to Detectors (Modules) in that particular layer.
    // Inputs:
    //     Double_t g[3]  The ILC Cartesian global coordinate from which the
    //                    distance is to be calculated with.
    //     Int_t lay      The layer to restrict the search to. If layer=0 then
    //                    all layers are searched. Default is lay=0.
    // Output:
    //     none.
    // Return:
    //     The module number representing the nearest module.
    

    
    Int_t    i,l,a,e,in=0;
    Double_t d,dn=1.0e10;
    Bool_t   t=lay!=0; // skip if lay = 0 default value check all layers.

    for(i=0;i<fNmodules;i++){
        if(t){GetModuleId(i,l,a,e);if(l!=lay) continue;}
        if((d=GetGeomMatrix(i)->Distance2(g))<dn){
            dn = d;
            in = i;
        } // end if
    } // end for i
    return in;
}
//______________________________________________________________________
void IlcTARGETgeom::GetNearest27(const Double_t g[3],Int_t n[27],Int_t lay)const{
    //      Finds 27 Detectors (Modules) that are nearest the point g [cm] in
    // ILC Global coordinates. If layer !=0 then the search is restricted
    // to Detectors (Modules) in that particular layer. The number 27 comes 
    // from including the nearest detector and all those around it (up, down,
    // left, right, forwards, backwards, and the corners).
    // Input:
    //     Double_t g[3]  The ILC Cartesian global coordinate from which the
    //                    distance is to be calculated with.
    //     Int_t lay      The layer to restrict the search to. If layer=0 then
    //                    all layers are searched. Default is lay=0.
    // Output:
    //     Int_t n[27]    The module number representing the nearest 27 modules
    //                    in order.
    // Return:
    //     none.
    

    
    Int_t    i,l,a,e,in[27]={0,0,0,0,0,0,0,0,0,
                             0,0,0,0,0,0,0,0,0,
                             0,0,0,0,0,0,0,0,0,};
    Double_t d,dn[27]={1.0e10,1.0e10,1.0e10,1.0e10,1.0e10,1.0e10,
                       1.0e10,1.0e10,1.0e10,1.0e10,1.0e10,1.0e10,
                       1.0e10,1.0e10,1.0e10,1.0e10,1.0e10,1.0e10,
                       1.0e10,1.0e10,1.0e10,1.0e10,1.0e10,1.0e10,
                       1.0e10,1.0e10,1.0e10};
    Bool_t   t=(lay!=0); // skip if lay = 0 default value check all layers.

    for(i=0;i<fNmodules;i++){
        if(t){GetModuleId(i,l,a,e);if(l!=lay) continue;}
        for(a=0;a<27;a++){
            d = GetGeomMatrix(i)->Distance2(g);
            if(d<dn[a]){
                for(e=26;e>a;e--){dn[e] = dn[e-1];in[e] = in[e-1];}
                dn[a] = d; in[a] = i;
            } // end if d<dn[i]
        } // end for a
    } // end for i
    for(i=0;i<27;i++) n[i] = in[i];
}
//----------------------------------------------------------------------
Double_t IlcTARGETgeom::GetAverageRadiusOfLayer(Int_t layer,Double_t &range)const{
    // Loops over all modules for a given layer and computes the
    // average cylindrical radius (about the z axis) and the range of
    // radii covered by this layer. Units, [cm] the Ilc default unit.
    // Input:
    //    Int_t layer     The layer for which the average radii is to be found
    // Output:
    //    Double_t &range The range of radii covered by this layer
    // Return:
    //    The average radii for this layer.
    
    

    
    Double_t r=0.0,rmin=1.0e6,rmax=-1.0,rp,t[3],l[3],dl[3];
    Int_t    n=0,i,j,lay,lad,det;
    IlcTARGETDetector idet;

    for(i=0;i<GetIndexMax();i++) {
        GetModuleId(i,lay,lad,det);
	idet = GetModuleType(i);
        if(lay!=layer) continue;
        dl[0] = dl[1] = dl[2] = 0.0;
        if(IsShapeDefined((Int_t)idet)) {
	    switch(idet){
	    case kSPD:{
		dl[0] = ((IlcTARGETgeomSPD*)GetShape(idet))->GetDx();
		dl[1] = ((IlcTARGETgeomSPD*)GetShape(idet))->GetDy();
		dl[2] = ((IlcTARGETgeomSPD*)GetShape(idet))->GetDz();
	    } break;
	    case kSDD: case kSDDp:{
		dl[0] = ((IlcTARGETgeomSDD*)GetShape(idet))->GetDx();
		dl[1] = ((IlcTARGETgeomSDD*)GetShape(idet))->GetDy();
		dl[2] = ((IlcTARGETgeomSDD*)GetShape(idet))->GetDz();
	    } break;
	    case kSSD: case kSSDp:{
		dl[0] = ((IlcTARGETgeomSSD*)GetShape(idet))->GetDx();
		dl[1] = ((IlcTARGETgeomSSD*)GetShape(idet))->GetDy();
		dl[2] = ((IlcTARGETgeomSSD*)GetShape(idet))->GetDz();
	    } break;
	    case kND:{
		Warning("GetAverageRadiusOfLayer",
			"idet=kND undefined detector type");
		continue;
	    }break;
	    default:{
		Warning("GetAverageRadiusOfLayer",
			"idet=%d not a defined value",(Int_t)idet);
		continue;
	    }break;
	    }// end switch.
        } // end of
        n++;
        GetTransCyln(i,t);
        rp = t[0];
        r += rp;
        if(rmin>rp) rmin = rp;
        if(rmax<rp) rmax = rp;
        for(j=0;j<8;j++){ // loop over the corners
            l[0] = dl[0];if(j%2==0) l[0] = -dl[0];
            l[1] = dl[1];if(j==2||j==3||j==6||j==7) l[1] = -dl[1];
            l[2] = dl[2];if(j>3) l[2] = -dl[2];
            LtoG(i,l,t);
            rp = TMath::Sqrt(t[0]*t[0]+t[1]*t[1]);
            if(rmin>rp) rmin = rp;
            if(rmax<rp) rmax = rp;
        } // end for j
    } // end for i
    r /= (Double_t)n;
    range = TMath::Max(rmax-r,r-rmin);
    return r;
}
//_______________________________________________________________________
void IlcTARGETgeom::DetLToTrackingV2(Int_t md, Float_t xin, Float_t zin, Float_t &yout, Float_t &zout) {

  //Conversion from local coordinates on detectors to local
  //coordinates used for tracking ("v2")
  

  
  Float_t x,y,z; Double_t rt[9];GetTrans(md,x,y,z);GetRotMatrix(md,rt);

//   cout<<"translation "<<x<<" "<<y<<" "<<z<<endl;
//   cout<<"rotation: "<<rt[0]<<" "<<rt[1]<<" "<<rt[2]<<endl;
//   cout<<"          "<<rt[3]<<" "<<rt[4]<<" "<<rt[5]<<endl;
//   cout<<"          "<<rt[6]<<" "<<rt[7]<<" "<<rt[8]<<endl;
  
  Double_t al=TMath::ATan2(rt[1],rt[0])+TMath::Pi();
  double sign=TMath::Sign(1.,-y*rt[0]+x*rt[1]);
  double signz=TMath::Sign(1.,rt[8]);
  double yout0;
  if(fabs(rt[8])<0.5){
    yout0=0;
    yout=sign*xin;
    zout=TMath::Hypot(x,y)-zin;
  }else{
    yout0=(x*TMath::Cos(al)+y*TMath::Sin(al));
    yout=(yout0+sign*xin);
    zout=z+signz*zin; 
  }
  IlcDebug(1, Form(" module = %d, xin = %f, yout = %f; zin = %f, zout = % f", md, xin, yout, zin, zout));

  //check tracking coordinate
  double xyzL[3]={0,0,0};
  double xyzG0[3],xyzG[3];
  LtoG(md,xyzL,xyzG0);
  double phi=-TMath::ATan2(xyzG0[1],xyzG0[0]);
  double rmiddle=-TMath::Hypot(xyzG0[0],xyzG0[1]);
  xyzL[0]=xin;xyzL[2]=zin;
  LtoG(md,xyzL,xyzG);
  phi+=TMath::ATan2(xyzG[1],xyzG[0]);
  if((xyzG0[0]*xyzG0[0]+xyzG0[1]*xyzG0[1])>1.e-9)
    rmiddle=(xyzG[0]*xyzG0[0]+xyzG[1]*xyzG0[1])/(xyzG0[0]*xyzG0[0]+xyzG0[1]*xyzG0[1])-1;
  else
    rmiddle=(xyzG[0]*xyzG0[0]+xyzG[1]*xyzG0[1])/(1.e-9)-1;
  if(phi<-TMath::Pi()) phi+=2*TMath::Pi();
  if(phi>TMath::Pi()) phi-=2*TMath::Pi();

  double dyout=yout-yout0;
  if(phi*dyout<0) {
    IlcError(Form("Wrong rotation of Tracking local coordinate: module = %d, xin = %f, yout = %f; zin = %f, zout = %f", md, xin, yout, zin, zout));
    IlcError(Form("dphi global = %f , but y tracking local =%f ", phi, dyout));
  }
  if(fabs(rt[8])<0.5&&(zout-TMath::Hypot(x,y))*rmiddle<0){
    IlcError(Form("Wrong rotation of Tracking local coordinate: module = %d, xin = %f, yout = %f; zin = %f, zout = %f", md, xin, yout, zin, zout));
    IlcError(Form("dr global = %f , but x tracking local =%f ", rmiddle, zout-TMath::Hypot(x,y)));
  }
  if(fabs(rt[8])>0.5&&fabs(xyzG[2]-zout)>1e-4){
    IlcError(Form("Wrong z-coordinate Tracking local coordinate: module = %d, xin = %f, yout = %f; zin = %f, zout = %f", md, xin, yout, zin, zout));
    IlcError(Form("Z global = %f , but z tracking local =%f ", xyzG[2], zout));
  }
  
}

//_______________________________________________________________________
void IlcTARGETgeom::TrackingV2ToDetL(Int_t md,Float_t yin,Float_t zin,Float_t &xout,Float_t &zout) {
  //Conversion from local coordinates used for tracking ("v2") to
  //local detector coordinates
  

  Float_t x,y,z; Double_t rt[9];GetTrans(md,x,y,z);GetRotMatrix(md,rt);
  Double_t al=TMath::ATan2(rt[1],rt[0])+TMath::Pi();
  double sign=TMath::Sign(1.,-y*rt[0]+x*rt[1]);
  double signz=TMath::Sign(1.,rt[8]);
  if(fabs(rt[8])<0.5){
    xout=sign*yin;
    zout=TMath::Hypot(x,y)-zin;
  }else{
    double yin0=(x*TMath::Cos(al)+y*TMath::Sin(al));
    xout=sign*(yin-yin0);
    zout=signz*(zin-z);
  }
  
}
