#ifndef TARGET_H
#define TARGET_H
/*===========================================================================

      T A R G E T   C O N F I G U R A T I O N   H E A D E R   F I L E

DESCRIPTION
  All the declarations and definitions necessary for general configuration
  of the DMSS software for a given target environment.

-----------------------------------------------------------------------------
Copyright (c) 1998-2002 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
-----------------------------------------------------------------------------
===========================================================================*/

/*===========================================================================
                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/oncrpc/main/source/inc/target.h#3 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/12/02   jct     The appropriate target file is now included via customer.h
                   in cust*.h.  TG is now fixed to be T_G and all other T_xxx
                   vars that could be assigned to T_G are deleted.  The use of
                   TG is deprecated. 
10/06/00   dsb     The appropriate target file is now included automatically
                   a make macro. Removed check on T_ACP, T_SMS, and T_SMS_UI.
                   Cannot remove check on T_AUTH as many files still use this 
                   instead of FEATURE_AUTH as they should.
                   Eventually, we want to eliminate T==T_xxx codes, but many
                   upper layer files still use them, so I have left them in.
                   When the rest of the DMSS stops using them, they should be
                   removed here.
08/13/99   jkl     Included T_Q2
01/15/99   hrk     Integrating from COMMON_ARM.03.01.00 VU from MSM3000 subsystem.
                   Re-introduced #defines for all targets. Removal of support for
                   these targets will be done in the next phase.
12/16/98   jct     Created for MSM3000+ support

===========================================================================*/

#define T_PC    1       /* PC test software, 80386/80486, running MSDOS  */
#define T_P     2       /* Portable Phone hardware                       */
#define T_M     3       /* Mobile Phone hardware                         */
#define T_REX   4       /* REX test software, 80386/80486, REX and MSDOS */
#define T_S     5       /* Stretch-portable hardware                     */
#define T_B2    6       /* Beta II mobile hardware                       */
#define T_I1    7       /* ISS1 WLL hardware                             */
#define T_DM    8       /* Diagnostic Monitor                            */
#define T_G     9       /* Gemini Portable hardware                      */
#define T_I2    10      /* ISS2 WLL hardware                             */
#define T_C1    11      /* CSS1 WLL hardware                             */
#define T_SP    12      /* Service Programming Tool                      */
#define T_T     13      /* TGP (Third Generation Portable)               */
#define T_MD    14      /* 1900 MHz Module (Charon)                      */
#define T_Q     15      /* Q phones (Q-1900, Q-800)                      */
#define T_O     16      /* Odie (5GP)                                    */
#define T_Q2    17      /* Q phones (Q-1900, Q-800)                      */             

/* This is deprecated - force to always be T_G
*/
#define TG T_G

/* Backward compatibile migration, needs to be eliminated completely
*/
#undef T_AUTH
#ifdef FEATURE_AUTH
   #define T_AUTH
#endif

/* All featurization starts from customer.h which includes the appropriate
**    cust*.h and targ*.h
*/
#ifdef CUST_H
   #include "customer.h"
#endif

#endif /* TARGET_H */
