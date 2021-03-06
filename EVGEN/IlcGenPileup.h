#ifndef ILCGENPILEUP_H
#define ILCGENPILEUP_H
/* 2012-2013 Infrastructure for Large Collider Experiment.       *
 * All rights reserved.                                                   *
 * See cxx source for full Copyright notice                               */

//-------------------------------------------------------------------------
//                          Class IlcGenPileup
//   This is a generator of beam-beam pileup.
//   It generates interactions within 3 orbits (+-1) around
//   the trigger event. The trigger event itself is chosen
//   randomly among the bunch crossings within the central orbit.
//   The user can decide whenever to include in the simulation the
//   "trigger" interaction or not. This is handled by the
//   GenerateTrigInteraction(Bool_t flag) method.
//   In the case the trigger interaction is included, it is
//   generated using the same settings (vertex smear for example) as
//   the pileup events.
//   In case the trigger simulation is not included, the user can make
//   a cocktail of generator used to produce the trigger interaction and
//   IlcGenPileup. In this case in order to avoid a fake increase of the rate around the
//   trigger, the number of background events within the bunch
//   crossing of the trigger is readuced by one.
//   The beam profile (the list of the active bunch crossings) can be
//   controlled via the SetBCMask(const char *mask) method. The syntax
//   follows the one in IlcTriggerBCMask class. For example:
//   "3564H" would mean that all the bunch corssings within the orbit
//   are aloowed (which is of course unphysical). In case one wants to simulate
//   one-bunch-crossing-per-orbit scenario, the way to do it is to put something like:
//   "1H3563L" or similar.
//   The SetGenerator(IlcGenerator *generator, Float_t rate) method is
//   used in order to define the generator to be used. The second argument is the pileup
//   rate in terms of #_of_interactions/bunch-crossing = sigma_tot * luminosity.
//   The pileup generation time window can be set via
//   IlcGenerator::SetPileUpTimeWindow(Float_t pileUpTimeW) method. By the default the
//   window is set to 88micros (= TPC readout window).
//      
// cvetan.cheshkov@cern.ch  9/12/2008
//-------------------------------------------------------------------------

#include "IlcGenCocktail.h"
#include "IlcTriggerBCMask.h"
class TFormula;
class IlcGenPileup : public IlcGenCocktail
{
 public:
    IlcGenPileup();
    virtual ~IlcGenPileup();

    virtual void Generate();
    virtual void SetRandomise(Bool_t flag);
    virtual void UsePerEventRates();
	    
    void         SetGenerator(IlcGenerator *generator, Float_t rate, Bool_t flag = kFALSE);
    //void         SetGenerator(IlcGenerator *generator, Float_t rate);
    Bool_t       SetBCMask(const char *mask);
    void         GenerateTrigInteraction(Bool_t flag) {fGenTrig = flag;}

 protected:
    virtual void AddGenerator
      (IlcGenerator *Generator, const char* Name, Float_t RateExp, TFormula* formula = 0 );

    IlcTriggerBCMask fBCMask;    // Mask used to tag the active bunch-crossings within an orbit
    Bool_t           fGenTrig;   // Generate or not the trigger interaction
    Bool_t           fFlag;      // fixed interaction rate (integer)

 private:
    IlcGenPileup(const IlcGenPileup &gen);
    IlcGenPileup & operator=(const IlcGenPileup & gen);

    ClassDef(IlcGenPileup,1) // Beam-beam pileup generator based on cocktail generator
};

#endif

