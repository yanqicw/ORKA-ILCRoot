* $Id: fgp2.F 1074 2000-05-17 15:13:04Z fca $
C
	FUNCTION FGP2(X)
#include "pact_hijing.inc"
#include "hiparnt.inc"
        EXTERNAL FGP3
        Z1=X
        FGP2=2.0*GAUSS4(FGP3,0.0,HIPR1(35),0.01)
	RETURN
	END
