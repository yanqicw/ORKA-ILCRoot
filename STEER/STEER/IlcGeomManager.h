#ifndef ILC_GEOM_MANAGER_H
#define ILC_GEOM_MANAGER_H

//
// Class for interfacing to the geometry; it also builds and manages the
// look-up tables for fast access to geometry and alignment information
// for sensitive alignable volumes:
// 1) the look-up table mapping unique volume ids to TGeoPNEntries
//    this allows to access directly by means of the unique index
//    the associated symbolic name and original global matrix
//    in addition to the functionality of the physical node
//    associated to a given alignable volume
// 2) the look-up table of the alignment objects associated to the
//    indexed alignable volumes
//

#include <TObject.h>

class TGeoManager;
class TGeoPNEntry;
class TGeoHMatrix;
class TObjArray;

class IlcAlignObj;
class IlcCDBId;
class IlcCDBParam;

class IlcGeomManager: public TObject {

public:
  enum ELayerID{kInvalidLayer=0,
		kFirstLayer=1,
		kSPD1=1, kSPD2=2,
		kSDD1=3, kSDD2=4,
		kSSD1=5, kSSD2=6,
		kTPC1=7, kTPC2=8,
		kTRD1=9, kTRD2=10, kTRD3=11, kTRD4=12, kTRD5=13, kTRD6=14,
		kTOF=15,
		kPHOS1=16, kPHOS2=17,
		kHMPID=18,
		kMUON=19,
		kEMCAL=20,
		kPFCALO=21,
		kPVD1=22, kPVD2=23,
		kPVBAR1=24, kPVBAR2=25,
		kPVEC1=26, kPVEC2=27,
		kRSTACK1=28, kRSTACK2=29,
		kLastLayer=30};

#ifdef WIN32
  static TGeoManager* GetGeometry();
#else
  static TGeoManager* GetGeometry() { return fgGeometry; }
#endif

  static Int_t       LayerSize(Int_t layerId);
  static const char* LayerName(Int_t layerId);
  static UShort_t LayerToVolUID(ELayerID layerId, Int_t modId);
  static UShort_t LayerToVolUID(Int_t    layerId, Int_t modId);
  static ELayerID VolUIDToLayer(UShort_t voluid, Int_t &modId);
  static ELayerID VolUIDToLayer(UShort_t voluid);
  static UShort_t LayerToVolUIDSafe(ELayerID layerId, Int_t modId);
  static UShort_t LayerToVolUIDSafe(Int_t    layerId, Int_t modId);
  static ELayerID VolUIDToLayerSafe(UShort_t voluid, Int_t &modId);
  static ELayerID VolUIDToLayerSafe(UShort_t voluid);

  static const char* SymName(UShort_t voluid);
  static const char* SymName(ELayerID layerId, Int_t modId);

  static Bool_t   GetFromGeometry(const char *symname, IlcAlignObj &alobj);
  static IlcAlignObj* GetAlignObj(UShort_t voluid);
  static IlcAlignObj* GetAlignObj(ELayerID layerId, Int_t modId);

  //to be used making a copy of the returned pointer to TGeoHMatrix!!
  static TGeoHMatrix* GetMatrix(Int_t index);
  static TGeoHMatrix* GetMatrix(const char *symname);
  static Bool_t GetTranslation(Int_t index, Double_t t[3]);
  static Bool_t GetRotation(Int_t index, Double_t r[9]);

  static Bool_t GetDeltaForBranch(IlcAlignObj& aao, TGeoHMatrix &inclusiveD);
  static Bool_t GetDeltaForBranch(Int_t index, TGeoHMatrix &inclusiveD);
  static TGeoHMatrix* GetOrigGlobalMatrix(Int_t index);
  static Bool_t GetOrigGlobalMatrix(const char *symname, TGeoHMatrix &m);
  static Bool_t GetOrigTranslation(Int_t index, Double_t t[3]);
  static Bool_t GetOrigRotation(Int_t index, Double_t r[9]);

  static const TGeoHMatrix* GetTracking2LocalMatrix(Int_t index);
  static Bool_t GetTrackingMatrix(Int_t index, TGeoHMatrix &m);

  static void        LoadGeometry(const char *geomFileName = NULL);
  static void        SetGeometry(TGeoManager * const geom);
  static void        CheckOverlapsOverPNs(Double_t threshold);  

  static Bool_t         ApplyAlignObjsToGeom(TObjArray& alObjArray, Bool_t ovlpcheck=kFALSE);

  static Bool_t         ApplyAlignObjsToGeom(const char* fileName,
				      const char* clArrayName);
  static Bool_t         ApplyAlignObjsToGeom(IlcCDBParam* param,
				      IlcCDBId& Id);
  static Bool_t         ApplyAlignObjsToGeom(const char* uri, const char* path,
				      Int_t runnum, Int_t version,
				      Int_t sversion);
  static Bool_t         ApplyAlignObjsToGeom(const char* detName, Int_t runnum, Int_t version,
				      Int_t sversion);
  static Bool_t         ApplyAlignObjsFromCDB(const char* AlDetsList);
  static Bool_t         LoadAlignObjsFromCDBSingleDet(const char* detName, TObjArray& alignObjArray);
  static Bool_t         CheckSymNamesLUT(const char* detsToBeChecked);
  static Int_t          GetNalignable(const char* module);

  ~IlcGeomManager();

 private:
  IlcGeomManager();
  IlcGeomManager(const IlcGeomManager&);
  IlcGeomManager& operator=(const IlcGeomManager&);

  static TGeoHMatrix* GetMatrix(TGeoPNEntry * const pne);
  static TGeoHMatrix* GetOrigGlobalMatrix(TGeoPNEntry * const pne);
  static Bool_t       GetOrigGlobalMatrixFromPath(const char *path, TGeoHMatrix &m);

  static TGeoPNEntry* GetPNEntry(Int_t index);
  static TGeoPNEntry* GetPNEntry(ELayerID layerId, Int_t modId);

  static void        ResetPNEntriesLUT();
  static void        InitAlignObjFromGeometry();
  static void        InitPNEntriesLUT();
  static void        InitNalignable();

  static TGeoManager* fgGeometry;

  static Int_t       fgLayerSize[kLastLayer - kFirstLayer]; // Size of layers
  static const char* fgLayerName[kLastLayer - kFirstLayer]; // Name of layers
  static TGeoPNEntry** fgPNEntry[kLastLayer - kFirstLayer]; // TGeoPNEntries
  static IlcAlignObj** fgAlignObjs[kLastLayer - kFirstLayer]; // Alignment objects
//   #ifdef MFT_UPGRADE
//   static const Int_t   fgkNDetectors = 16;              // number of detectors
//   #else
//   static const Int_t   fgkNDetectors = 15;              // number of detectors
//   #endif
  static const Int_t   fgkNDetectors = 6;              // number of detectors    // AU
  static const char *  fgkDetectorName[fgkNDetectors] ; // name of detectors
  static Int_t       fgNalignable[fgkNDetectors];

  ClassDef(IlcGeomManager, 0); // Manager of geometry information for alignment
};

#endif
