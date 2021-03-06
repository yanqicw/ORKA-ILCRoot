* $Id: lu4ent_hijing.F 1074 2000-05-17 15:13:04Z fca $
    
C*********************************************************************  
    
      SUBROUTINE LU4ENT_HIJING(IP,KF1,KF2,KF3,KF4,PECM,X1,X2,X4,X12,X14)   
    
C...Purpose: to store four partons or particles in their CM frame, with 
C...the first along the +z axis, the last in the xz plane with x > 0    
C...and the second having y < 0 and y > 0 with equal probability.   
#include "lujets_hijing.inc"
#include "ludat1_hijing.inc"
#include "ludat2_hijing.inc"
    
C...Standard checks.    
      MSTU(28)=0    
      IF(MSTU(12).GE.1) CALL LULIST_HIJING(0)  
      IPA=MAX(1,IABS(IP))   
      IF(IPA.GT.MSTU(4)-3) CALL LUERRM_HIJING(21,  
     &'(LU4ENT_HIJING:) writing outside LUJETS_HIJING momory')    
      KC1=LUCOMP_HIJING(KF1)   
      KC2=LUCOMP_HIJING(KF2)   
      KC3=LUCOMP_HIJING(KF3)   
      KC4=LUCOMP_HIJING(KF4)   
      IF(KC1.EQ.0.OR.KC2.EQ.0.OR.KC3.EQ.0.OR.KC4.EQ.0) CALL
     $     LUERRM_HIJING(12,'(LU4ENT_HIJING:) unknown flavour code') 
    
C...Find masses. Reset K, P and V vectors.  
      PM1=0.    
      IF(MSTU(10).EQ.1) PM1=P(IPA,5)    
      IF(MSTU(10).GE.2) PM1=ULMASS_HIJING(KF1) 
      PM2=0.    
      IF(MSTU(10).EQ.1) PM2=P(IPA+1,5)  
      IF(MSTU(10).GE.2) PM2=ULMASS_HIJING(KF2) 
      PM3=0.    
      IF(MSTU(10).EQ.1) PM3=P(IPA+2,5)  
      IF(MSTU(10).GE.2) PM3=ULMASS_HIJING(KF3) 
      PM4=0.    
      IF(MSTU(10).EQ.1) PM4=P(IPA+3,5)  
      IF(MSTU(10).GE.2) PM4=ULMASS_HIJING(KF4) 
      DO 100 I=IPA,IPA+3    
      DO 100 J=1,5  
      K(I,J)=0  
      P(I,J)=0. 
  100 V(I,J)=0. 
    
C...Check flavours. 
      KQ1=KCHG(KC1,2)*ISIGN(1,KF1)  
      KQ2=KCHG(KC2,2)*ISIGN(1,KF2)  
      KQ3=KCHG(KC3,2)*ISIGN(1,KF3)  
      KQ4=KCHG(KC4,2)*ISIGN(1,KF4)  
      IF(KQ1.EQ.0.AND.KQ2.EQ.0.AND.KQ3.EQ.0.AND.KQ4.EQ.0) THEN  
      ELSEIF(KQ1.NE.0.AND.KQ2.EQ.2.AND.KQ3.EQ.2.AND.(KQ1+KQ4.EQ.0.OR.   
     &KQ1+KQ4.EQ.4)) THEN   
      ELSEIF(KQ1.NE.0.AND.KQ1+KQ2.EQ.0.AND.KQ3.NE.0.AND.KQ3+KQ4.EQ.0.)  
     &THEN  
      ELSE  
         CALL LUERRM_HIJING(2
     $        ,'(LU4ENT_HIJING:) unphysical flavour combination')   
      ENDIF 
      K(IPA,2)=KF1  
      K(IPA+1,2)=KF2    
      K(IPA+2,2)=KF3    
      K(IPA+3,2)=KF4    
    
C...Store partons/particles in K vectors for normal case.   
      IF(IP.GE.0) THEN  
        K(IPA,1)=1  
        IF(KQ1.NE.0.AND.(KQ2.NE.0.OR.KQ3.NE.0.OR.KQ4.NE.0)) K(IPA,1)=2  
        K(IPA+1,1)=1    
        IF(KQ2.NE.0.AND.KQ1+KQ2.NE.0.AND.(KQ3.NE.0.OR.KQ4.NE.0))    
     &  K(IPA+1,1)=2    
        K(IPA+2,1)=1    
        IF(KQ3.NE.0.AND.KQ4.NE.0) K(IPA+2,1)=2  
        K(IPA+3,1)=1    
    
C...Store partons for parton shower evolution from q-g-g-qbar or    
C...g-g-g-g event.  
      ELSEIF(KQ1+KQ2.NE.0) THEN 
         IF(KQ1.EQ.0.OR.KQ2.EQ.0.OR.KQ3.EQ.0.OR.KQ4.EQ.0) CALL
     $        LUERRM_HIJING(2
     $        ,'(LU4ENT_HIJING:) requested flavours can not develop'/
     $        /' parton shower')   
        K(IPA,1)=3  
        K(IPA+1,1)=3    
        K(IPA+2,1)=3    
        K(IPA+3,1)=3    
        KCS=4   
        IF(KQ1.EQ.-1) KCS=5 
        K(IPA,KCS)=MSTU(5)*(IPA+1)  
        K(IPA,9-KCS)=MSTU(5)*(IPA+3)    
        K(IPA+1,KCS)=MSTU(5)*(IPA+2)    
        K(IPA+1,9-KCS)=MSTU(5)*IPA  
        K(IPA+2,KCS)=MSTU(5)*(IPA+3)    
        K(IPA+2,9-KCS)=MSTU(5)*(IPA+1)  
        K(IPA+3,KCS)=MSTU(5)*IPA    
        K(IPA+3,9-KCS)=MSTU(5)*(IPA+2)  
    
C...Store partons for parton shower evolution from q-qbar-q-qbar event. 
      ELSE  
         IF(KQ1.EQ.0.OR.KQ2.EQ.0.OR.KQ3.EQ.0.OR.KQ4.EQ.0) CALL
     $        LUERRM_HIJING(2
     $        ,'(LU4ENT_HIJING:) requested flavours can not develop'/
     $        /' parton shower')   
        K(IPA,1)=3  
        K(IPA+1,1)=3    
        K(IPA+2,1)=3    
        K(IPA+3,1)=3    
        K(IPA,4)=MSTU(5)*(IPA+1)    
        K(IPA,5)=K(IPA,4)   
        K(IPA+1,4)=MSTU(5)*IPA  
        K(IPA+1,5)=K(IPA+1,4)   
        K(IPA+2,4)=MSTU(5)*(IPA+3)  
        K(IPA+2,5)=K(IPA+2,4)   
        K(IPA+3,4)=MSTU(5)*(IPA+2)  
        K(IPA+3,5)=K(IPA+3,4)   
      ENDIF 
    
C...Check kinematics.   
      MKERR=0   
      IF(0.5*X1*PECM.LE.PM1.OR.0.5*X2*PECM.LE.PM2.OR.0.5*(2.-X1-X2-X4)* 
     &PECM.LE.PM3.OR.0.5*X4*PECM.LE.PM4) MKERR=1    
      PA1=SQRT(MAX(0.,(0.5*X1*PECM)**2-PM1**2)) 
      PA2=SQRT(MAX(0.,(0.5*X2*PECM)**2-PM2**2)) 
      PA3=SQRT(MAX(0.,(0.5*(2.-X1-X2-X4)*PECM)**2-PM3**2))  
      PA4=SQRT(MAX(0.,(0.5*X4*PECM)**2-PM4**2)) 
      X24=X1+X2+X4-1.-X12-X14+(PM3**2-PM1**2-PM2**2-PM4**2)/PECM**2 
      CTHE4=(X1*X4-2.*X14)*PECM**2/(4.*PA1*PA4) 
      IF(ABS(CTHE4).GE.1.002) MKERR=1   
      CTHE4=MAX(-1.,MIN(1.,CTHE4))  
      STHE4=SQRT(1.-CTHE4**2)   
      CTHE2=(X1*X2-2.*X12)*PECM**2/(4.*PA1*PA2) 
      IF(ABS(CTHE2).GE.1.002) MKERR=1   
      CTHE2=MAX(-1.,MIN(1.,CTHE2))  
      STHE2=SQRT(1.-CTHE2**2)   
      CPHI2=((X2*X4-2.*X24)*PECM**2-4.*PA2*CTHE2*PA4*CTHE4)/    
     &(4.*PA2*STHE2*PA4*STHE4)  
      IF(ABS(CPHI2).GE.1.05) MKERR=1    
      CPHI2=MAX(-1.,MIN(1.,CPHI2))  
      IF(MKERR.EQ.1) CALL LUERRM_HIJING(13,    
     &'(LU4ENT_HIJING:) unphysical kinematical variable setup')    
    
C...Store partons/particles in P vectors.   
      P(IPA,3)=PA1  
      P(IPA,4)=SQRT(PA1**2+PM1**2)  
      P(IPA,5)=PM1  
      P(IPA+3,1)=PA4*STHE4  
      P(IPA+3,3)=PA4*CTHE4  
      P(IPA+3,4)=SQRT(PA4**2+PM4**2)    
      P(IPA+3,5)=PM4    
      P(IPA+1,1)=PA2*STHE2*CPHI2    
      P(IPA+1,2)=PA2*STHE2*SQRT(1.-CPHI2**2)*(-1.)**INT(RLU_HIJING(0)+0
     $     .5) 
      P(IPA+1,3)=PA2*CTHE2  
      P(IPA+1,4)=SQRT(PA2**2+PM2**2)    
      P(IPA+1,5)=PM2    
      P(IPA+2,1)=-P(IPA+1,1)-P(IPA+3,1) 
      P(IPA+2,2)=-P(IPA+1,2)    
      P(IPA+2,3)=-P(IPA,3)-P(IPA+1,3)-P(IPA+3,3)    
      P(IPA+2,4)=SQRT(P(IPA+2,1)**2+P(IPA+2,2)**2+P(IPA+2,3)**2+PM3**2) 
      P(IPA+2,5)=PM3    
    
C...Set N. Optionally fragment/decay.   
      N=IPA+3   
      IF(IP.EQ.0) CALL LUEXEC_HIJING   
    
      RETURN    
      END   
