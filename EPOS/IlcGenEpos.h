//
// IlcGenEpos.cpp
//
//  ILC event generator based on EPOS model from Klaus Werner
//
//  Created on: Feb 28, 2009
//      Author: Piotr Ostrowski, postrow@if.pw.edu.pl
//

#ifndef ILC_GEN_EPOS_H
#define ILC_GEN_EPOS_H

#include "IlcGenMC.h"
#include "TEpos.h"


class IlcGenEpos: public IlcGenMC {
public:
	IlcGenEpos();
	IlcGenEpos(Int_t npart);
	virtual void Init();
	virtual void Generate();

	virtual ~IlcGenEpos();

	void SetImpactParameterRange(Float_t bmin, Float_t bmax) { fBmin = bmin; fBmax = bmax; }
	void SetReactionPlaneAngleRange(Float_t phimin, Float_t phimax) { fPhiMin = phimin; fPhiMax = phimax; }
	void AddNoDecay(Int_t nodecay) { GetTEpos()->AddNoDecay(nodecay); }
	void AddExtraInputLine(const char *line) { GetTEpos()->AddExtraInputLine(line); }
	Float_t GetPhiMin() const { return fPhiMin; }
	Float_t GetPhiMax() const { return fPhiMax; }
	Float_t GetBmin() const { return fBmin; }
	Float_t GetBMax() const { return fBmax; }

	void FilterModelOutput(Bool_t value) {fFilterModelOutput = value;}
	Bool_t IsModelOutputFiltered() const { return fFilterModelOutput; }
protected:
	virtual TEpos* GetTEpos() { return (TEpos *)fMCEvGen; }

	Float_t fBmin; //minimum impact parameter
	Float_t fBmax; //maximum impact parameter

	Float_t fPhiMin; // reaction plane angle minimum
	Float_t fPhiMax; // reaction plane angle maximum

	Bool_t fFilterModelOutput; //if true it will filter out internal model entities from the stack
private:

	ClassDef(IlcGenEpos,1)
};

#endif /* ILC_GEN_EPOS_H */
