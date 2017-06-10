#ifndef WMS_H_trimmed
#define WMS_H_trimmed
/*=============================================================================
  @file  wms.h

  ---------------------------------------------------------------------------
  Copyright (c) 2012 Qualcomm Technologies, Inc. 
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

  $Id: //depot/asic/msm7x30/LA/4.0/MODEM_APIS/libs/remote_apis/wms/inc/wms.h#9 $

  Notes: 
     ==== Auto-Generated File, do not edit manually ====
     Generated from build type: AABBQMAAH 
     #defines containing ABBQMAAH replaced with ________
=============================================================================*/
#ifndef WMS_H
#define WMS_H

/**
@file wms.h
  Wireless Messaging Services (WMS) Application Programming Interface (API).

  The WMS API provides the client software with the ability to send and receive 
  point-to-point Short Message Service (SMS) or Enhanced Messaging Service (EMS) 
  messages, to receive broadcast in a multimode environment (cdmaOne&tm;, 
  cdma2000&reg;, 1xEV, GSM/GPRS, and WCDMA).
*/

#ifndef COMDEF_H
#define COMDEF_H
/*===========================================================================

                   S T A N D A R D    D E C L A R A T I O N S

DESCRIPTION
  This header file contains general types and macros that are of use
  to all modules.  The values or definitions are dependent on the specified
  target.  T_WINNT specifies Windows NT based targets, otherwise the
  default is for ARM targets.

       T_WINNT  Software is hosted on an NT platforn, triggers macro and
                type definitions, unlike definition above which triggers
                actual OS calls

DEFINED CONSTANTS

       Name      Definition
       -------   --------------------------------------------------------
       ON        Asserted condition
       OFF       Deasserted condition

       NULL      Pointer to nothing

       PACK()    Used to declare a C type packed for either GCC or ARM
                 compilers.

       *** DEPRECATED - WON'T WORK FOR newer versions (3.x) of GCC ***
       PACKED    Used to indicate structures which should use packed
                 alignment
       *** DEPRECATED - WON'T WORK FOR newer versions (3.x) of GCC ***

       INLINE    Used to inline functions for compilers which support this

Copyright (c) 1990-2010 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/

#ifndef COM_DTYPES_H
#define COM_DTYPES_H
/*===========================================================================

                   S T A N D A R D    D E C L A R A T I O N S

DESCRIPTION
  This header file contains general data types that are of use to all modules.  
  The values or definitions are dependent on the specified
  target.  T_WINNT specifies Windows NT based targets, otherwise the
  default is for ARM targets.

  T_WINNT  Software is hosted on an NT platforn, triggers macro and
           type definitions, unlike definition above which triggers
           actual OS calls

Copyright (c) 2009-2010 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/


/* ------------------------------------------------------------------------
** Constants
** ------------------------------------------------------------------------ */

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE   1   /* Boolean true value. */
#define FALSE  0   /* Boolean false value. */

#define  ON   1    /* On value. */
#define  OFF  0    /* Off value. */

#ifndef NULL
  #define NULL  0
#endif

/* -----------------------------------------------------------------------
** Standard Types
** ----------------------------------------------------------------------- */

/* The following definitions are the same accross platforms.  This first
** group are the sanctioned types.
*/
#ifndef _ARM_ASM_

#if defined(DALSTDDEF_H) /* guards against a known re-definer */
#define _UINT32_DEFINED
#define _UINT16_DEFINED
#define _UINT8_DEFINED
#define _INT32_DEFINED
#define _INT16_DEFINED
#define _INT8_DEFINED
#define _UINT64_DEFINED
#define _INT64_DEFINED
#define _BYTE_DEFINED
#endif /* #if !defined(DALSTDDEF_H) */

#ifndef _UINT32_DEFINED
typedef  unsigned long int  uint32;      /* Unsigned 32 bit value */
#define _UINT32_DEFINED
#endif

#ifndef _UINT16_DEFINED
typedef  unsigned short     uint16;      /* Unsigned 16 bit value */
#define _UINT16_DEFINED
#endif

#ifndef _UINT8_DEFINED
typedef  unsigned char      uint8;       /* Unsigned 8  bit value */
#define _UINT8_DEFINED
#endif

#ifndef _INT32_DEFINED
typedef  signed long int    int32;       /* Signed 32 bit value */
#define _INT32_DEFINED
#endif

#ifndef _INT16_DEFINED
typedef  signed short       int16;       /* Signed 16 bit value */
#define _INT16_DEFINED
#endif

#ifndef _INT8_DEFINED
typedef  signed char        int8;        /* Signed 8  bit value */
#define _INT8_DEFINED
#endif

/* This group are the deprecated types.  Their use should be
** discontinued and new code should use the types above
*/
#ifndef _BYTE_DEFINED
typedef  unsigned char      byte;         /* Unsigned 8  bit value type. */
#define  _BYTE_DEFINED
#endif

typedef  unsigned short     word;         /* Unsinged 16 bit value type. */
typedef  unsigned long      dword;        /* Unsigned 32 bit value type. */

typedef  unsigned char      uint1;        /* Unsigned 8  bit value type. */
typedef  unsigned short     uint2;        /* Unsigned 16 bit value type. */
typedef  unsigned long      uint4;        /* Unsigned 32 bit value type. */

typedef  signed char        int1;         /* Signed 8  bit value type. */
typedef  signed short       int2;         /* Signed 16 bit value type. */
typedef  long int           int4;         /* Signed 32 bit value type. */

typedef  signed long        sint31;       /* Signed 32 bit value */
typedef  signed short       sint15;       /* Signed 16 bit value */
typedef  signed char        sint7;        /* Signed 8  bit value */

#if (! defined T_WINNT) && (! defined __GNUC__)
  /* Non WinNT Targets */
  #ifndef _INT64_DEFINED
    typedef long long     int64;       /* Signed 64 bit value */
    #define _INT64_DEFINED
  #endif
  #ifndef _UINT64_DEFINED
    typedef  unsigned long long  uint64;      /* Unsigned 64 bit value */
    #define _UINT64_DEFINED
  #endif
#else /* T_WINNT || TARGET_OS_SOLARIS || __GNUC__ */
#endif /* T_WINNT */

#endif /* _ARM_ASM_ */

#endif  /* COM_DTYPES_H */

#ifndef _MSC_VER
#ifndef TARGET_H
#define TARGET_H
/*===========================================================================

      T A R G E T   C O N F I G U R A T I O N   H E A D E R   F I L E

DESCRIPTION
  All the declarations and definitions necessary for general configuration
  of the DMSS software for a given target environment.

Copyright (c) 1998,1999,2000,2001,2002  by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/


/* All featurization starts from customer.h which includes the appropriate
**    cust*.h and targ*.h
*/
#ifdef CUST_H
#ifndef CUSTOMER_H
#define CUSTOMER_H
/*===========================================================================

                   C U S T O M E R    H E A D E R    F I L E

DESCRIPTION
  This header file provides customer specific information for the current
  build.  It expects the compile time switch /DCUST_H=CUSTxxxx.H.  CUST_H
  indicates which customer file is to be used during the current build.
  Note that cust_all.h contains a list of ALL the option currently available.
  The individual CUSTxxxx.H files define which options a particular customer
  has requested.


Copyright (c) 1996, 1997       by Qualcomm Technologies, Inc.  All Rights Reserved.
Copyright (c) 1998, 1999, 2000 by Qualcomm Technologies, Inc.  All Rights Reserved.
Copyright (c) 2001, 2002       by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/


/* Make sure that CUST_H is defined and then include whatever file it
** specifies.
*/
#ifdef CUST_H
#ifndef CUSTA_________H
#define CUSTA_________H
/* ========================================================================
FILE: CUSTA________

Copyright (c) 2001-2012 by Qualcomm Technologies, Inc.  All Rights Reserved.        
=========================================================================== */

#define FEATURE_GSM 
#define FEATURE_WCDMA 
#define FEATURE_CDMA 
#define FEATURE_RUIM 
#define FEATURE_UIM_RUIM 
#define FEATURE_UIM_RUN_TIME_ENABLE 
#define FEATURE_CDSMS 
#define FEATURE_WCDMA 
#define FEATURE_GWSMS 
#define FEATURE_HDR 
#define FEATURE_MULTIPROCESSOR 
#define FEATURE_L4 
#ifndef CUSTMODEM_MULTIMODE_H
#define CUSTMODEM_MULTIMODE_H
/*===========================================================================

            " M O D E M M U L T I M O D E "   H E A D E R   F I L E

DESCRIPTION
  Configuration for Multime-Mode support on QSC6695 targets.

  Copyright (c) 2009 by Qualcomm Technologies, Inc. All Rights Reserved.
===========================================================================*/

#define FEATURE_WCDMA
#define FEATURE_GSM
#define FEATURE_CDMA
#define FEATURE_BUILD_CDMA_CELL_PCS
#define FEATURE_HDR
#define FEATURE_DS
#ifndef CUSTCDMA_H
#define CUSTCDMA_H
/*===========================================================================

DESCRIPTION
  Configuration for CDMA and AMPS Operation

  Copyright (c) 2002 through 2010  by Qualcomm Technologies, Inc. All Rights Reserved.
===========================================================================*/


/* -----------------------------------------------------------------------
** Top Level CDMA Call Processing Mode Feature Selection
** ----------------------------------------------------------------------- */
#if (defined (FEATURE_BUILD_CDMA_CELL) && !defined (FEATURE_BUILD_JCDMA))
#elif defined FEATURE_BUILD_CDMA_CELL_AMPS
#elif (defined (FEATURE_BUILD_JCDMA) && !defined (FEATURE_BUILD_CDMA_PCS))
#elif (defined (FEATURE_BUILD_CDMA_PCS) && !defined (FEATURE_BUILD_JCDMA))
#elif (defined (FEATURE_BUILD_CDMA_PCS) && defined (FEATURE_BUILD_JCDMA))
#elif defined FEATURE_BUILD_CDMA_TRIMODE
#elif defined FEATURE_BUILD_CDMA_CELL_PCS

   /* Supports CDMA at 800  MHz
   **          CDMA at 1900 Mhz
   */
   #define FEATURE_CDMA_800
   #define FEATURE_CDMA_1900

#else
#endif


/* -----------------------------------------------------------------------
** IS-95B
** ----------------------------------------------------------------------- */
#define FEATURE_IS95B

#ifdef FEATURE_IS95B
  /*---------------------------------------*/
  /* Enable the individual IS-95B features */
  /*---------------------------------------*/

  /* IS95B compliant version of Calling NAme Presentation (CNAP)
  ** This is included only if FEATURE_IS95B is included
  */
  #define FEATURE_IS95B_EXT_DISP

  /* IS95B compliant version of Network Directed System Selection (NDSS)
  ** This is included only if FEATURE_IS95B is included
  */
  #define FEATURE_IS95B_NDSS

  /* IS95B compliant version of the Alert Answer Bypass Feature
  ** This is included only if FEATURE_IS95B is included
  */
  #define FEATURE_IS95B_ALERT_ANS_BYPASS

  /*-----------------------------------------------------------*/
  /* Enable the individual IS-95B Protocol Revision 5 features */
  /* These are optional for Revsion 4                          */
  /*-----------------------------------------------------------*/

  /*-----------------------------------------------------------*/
  /* End of IS-95B Feature Definitions                         */
  /*-----------------------------------------------------------*/

#endif /* FEATURE_IS95B */

/* -----------------------------------------------------------------------
** IS-2000
** ----------------------------------------------------------------------- */
#define FEATURE_IS2000

#endif /* CUSTCDMA_H */
#ifndef CUSTCDMA2000_H
#define CUSTCDMA2000_H
/*===========================================================================

            " C D M A 2 0 0 0 "   H E A D E R   F I L E

DESCRIPTION
  Configuration for IS-856 support on MSM6800 targets.

  Copyright (c) 2001, 2002, 2003
                2004, 2005, 2006 by Qualcomm Technologies, Inc. All Rights Reserved.
===========================================================================*/

#define FEATURE_BUILD_CDMA_TRIMODE 

/* FEATURE_CDMA feature should be defined only in C2k 
 * and multimode (UMTS + CDMA) builds. 
 * support for IS-95/IS2000/1X Data Services   */
#define FEATURE_CDMA

#endif /* CUSTCDMA2000_H */
#ifndef CUSTUSURF_H
#define CUSTUSURF_H
/*=========================================================================== 

                           C U S T U S U R F

DESCRIPTION
  Customer file for the MSM6280 UMTS (GSM + WCDMA) SURF build.

  Copyright (c) 1996-2002 by Qualcomm Technologies, Inc. All Rights Reserved.
===========================================================================*/


// ---  GSM  ---
/* Enable GSM code
*/
#define FEATURE_GSM

// --- WCDMA ---
/* Enable WCDMA code
*/
#define FEATURE_WCDMA

/* Enable MM GSDI
*/
#define FEATURE_MMGSDI 

/* USAT support
*/
#define FEATURE_GSTK
#define FEATURE_ALS

#define FEATURE_UUS

#define FEATURE_CCBS

#endif /* CUSTUSURF_H */


/*****************************************************************************/
#ifndef CUSTHDR_H
#define CUSTHDR_H
/*===========================================================================

            " M S M 6 8 0 0 -  S U R F "   H E A D E R   F I L E

DESCRIPTION
  Configuration for IS-856 support on MSM6800 targets.

  Copyright (c) 2001 - 2010 by Qualcomm Technologies, Inc. All Rights Reserved.
===========================================================================*/


/* HDR Air Interface support.
*/
#define FEATURE_HDR

/* HDR-1X handoff */
#define FEATURE_HDR_HANDOFF 

/* Hybrid operation
*/
#define FEATURE_HDR_HYBRID

/* Data Dedicated Transmission mode 
   to minimize the hyrbid HDR data call interruption 
*/ 
#define FEATURE_DDTM_CNTL

/* HDR Broadcast related features */
#define FEATURE_BCMCS

/* Feature for Enhanced Multi-flow Packet Application. Turn on only for 7800 */
#define FEATURE_HDR_EMPA

#endif /* CUSTHDR_H */
#ifndef CUSTMODEMDATA_H
#define CUSTMODEMDATA_H
/*===========================================================================
 
DESCRIPTION
  Configuration for modem data.

  Copyright (c) 2002 - 2010  by Qualcomm Technologies, Inc. All Rights Reserved.
===========================================================================*/


/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

$Header: //depot/asic/msm7x30/LA/4.0/MODEM_APIS/libs/remote_apis/wms/inc/wms.h#9 $
$DateTime: 2012/01/05 02:03:01 $ $Author: c_ryalav $

when       who     what, where, why
--------   ---     ------------------------------------------------------------ 
05/27/08   ua      HS_NEW_INTERFACE is removed for THIN BUILDS. 
05/29/10   ls      Added FEATURE_DATA_REFRESH_VOTING
12/10/09   nc      Moved FEATURE_DSAT_DEV_CMDS to custdataumts.h
11/01/09   vg      removed third party OS featurization of ATCOP
09/30/09   mg      Added feature for ds profile modem 
04/14/08   hs      Created new file for multimode targets
===========================================================================*/
/* -----------------------------------------------------------------------
** Data Services
** ----------------------------------------------------------------------- */
#define FEATURE_DS

#ifdef FEATURE_CDMA
#ifndef CUSTDATAC2K_H
#define CUSTDATAC2K_H

/*===========================================================================

DESCRIPTION
  Configuration for C2K Data.

  Copyright (c) 2002 - 2010  by Qualcomm Technologies, Inc. All Rights Reserved.
===========================================================================*/


/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

$Header: //depot/asic/msm7x30/LA/4.0/MODEM_APIS/libs/remote_apis/wms/inc/wms.h#9 $ 
$DateTime: 2012/01/05 02:03:01 $ $Author: c_ryalav $

when       who     what, where, why
--------   ---     ------------------------------------------------------------
10/13/10   ls      Added FEATURE_CCM_FLOW_CONTROL
07/18/10   mg      Added feature FEATURE_DATA_EHRPD_PDN_THROTTLE
05/10/10   kk      Correcting typo in CDMA_SMS feature and adding CDMA_PBM.
05/05/10   ls      Added FEATURE_IS95_RLP_MUX_COMMON_API 
03/30/10   ssh     FEATURE_DATA_PS_PAP_INTERNAL_AUTH: Gobi mainlining
03/11/10   sn      Enabled FEATURE_DATA_DISABLE_IS95B_MDR to disable MDR SOs.
02/24/10   sn      Enabled FEATURE_DATA_DISABLE_CS_SO to disable Async/Fax SOs.
02/11/10   sn      Removed FEATURE_DS_OMH as OMH support is mainlined.
01/15/10   ua      Added support for ^CPIN, +CPIN, +CLCK for RUIM card.
01/05/10   ua      Turning ON CDMA SMS AT commands. 
                          Turning ON MMGSDI Authentication AT commands under 
                          FEATURE_MMGSDI_SESSION_LIB
                          Moving all ATCoP features to one end. 
09/30/09   mg      Added feature for ds profile api
09/09/08   sn      Removed dependency on FEATURE_UIM_3GPD_MULTIPLE_SIP_PROFILES 
                   in enabling OMH feature.
08/05/08   hs      Included FEATURE_OMH. Made MIP Feature changes.
07/14/08   psng    Added FEATURE_DS_OMH for multiple profiles functionality.
03/06/08   mga     Enabled FEATURE_DS_LN_UPDATE
09/28/07   as      Switched on FEATURE_DATA_MOBILE_IP_DMU_MODIFIES_AN_AUTH
08/21/07   wsh     Retired EATURE_C2K_INTERNAL_USE_ONLY flag in 1H07 branch
08/09/07   wli     added support for FEATURE_DADA_ON_APPS
08/03/07   pm      updated to use FEATURE_INTERNAL_USE_FFABUILDS_ONLY
05/23/07   dnn     Mainlined FEATURE_HDR_QOS_DQOS_SUPPORTED_ALWAYS
05/23/07   ssr     Adding GSM custdata.h under FEATURE_GSM_GPRS
05/17/07   ssr     Adding GSM / GPRS fearure sets to enable combined 1x/GSM PLF
04/17/07   ac      Turn off certain features when VoIP is off
11/14/06   vrk/gr  turning off RDUD, CAM, DCTM for JCDMA builds
11/15/06   vas     Added FEATURE_C2K_INTERNAL_USE_ONLY related features
05/23/06   vrk     enable FEATURE_DATA_SERIALIZER for 6800 5.0
12/13/05   vrk     enable FEATURE_CALL_THROTTLE (DCTM) and move RSVP Featurization
10/17/05   vrk     disable FEATURE_DS_QNC in JCDMA builds and modified comments
04/05/05   gr      Integrated EPZID functionality onto the main line
02/28/05   ks      Enabled FEATURE_DATA_PS_DATA_LOGGING for Data Protocol
                   Logging.
02/25/05   kvd     Removed FEATURE_DS_MOBILE_IP_MULTIPLE_NAI as the feature is
                   now appicable to SIP calls also and is covered by
                   FEATURE_DS_MULTIPLE_NAI.
02/15/05   kvd     Added FEATURE_DS_MULTIPLE_NAI.
02/03/05   kvd     Added FEATURE_DS_MOBILE_IP_MULTIPLE_NAI.
11/11/02   cah     Added GSM features.
05/01/02   jct     Created
===========================================================================*/



#ifdef FEATURE_DS

   /* Support for IS-95/IS2000/1X Data Services.   */
   #define FEATURE_CDMA

#endif /* FEATURE_DS */
/* -------------  ATCoP CDMA Features   END ----------------- */

#endif /* CUSTDATAC2K_H */
#endif /*FEATURE_CDMA*/


#endif /* CUSTMODEMDATA_H */
#ifndef CUSTUIM_H
  #define CUSTUIM_H
  /*===========================================================================

              " C u s t -  U I M "   H E A D E R   F I L E

  DESCRIPTION
  Configuration for UIM Feature.

  Copyright (c) 2008-2010 by Qualcomm Technologies, Inc. All Rights Reserved.
  ===========================================================================*/
  /*===========================================================================

                        EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/msm7x30/LA/4.0/MODEM_APIS/libs/remote_apis/wms/inc/wms.h#9 $ $DateTime: 2012/01/05 02:03:01 $ $Author: c_ryalav $

  when       who     what, where, why
  --------   ---     ---------------------------------------------------------- 
  10/30/10   yb      6x15 bring up changes
  10/14/10   dd      Changes for LTE only build
  09/28/10   yt      Disabled RAT balancing support for Gobi builds
  09/21/10   nk      Removed CAT/ICAT Support for BMP Builds
  09/17/10   yk      Added support dynamic registration on WDOG. 
  09/13/10   yt      Enabled RAT indicator support independently of FEATURE_THIN_UI
  09/10/10   nmb     Enabled FEATURE_MMGSDI_ABSENT_MANDATORY_FILES_INIT  
  08/20/10   dd      No icon support if FEATURE_GOBI is defined
  07/29/10   nk      Enabled FEATURE_2ND_PRL
  07/27/10   nmb     Added FEATURE_MMGSDI_ABSENT_MANDATORY_FILES_INIT  
  07/26/10   ssr     Enabled FEATURE_UIM_SUPPORT_3GPD_NV  
  07/09/10   ssr     Enabled FEATURE_UIM_MISCONFIG_RUIM_N5_WORKAROUND  
  07/14/10   mib     Added FEATURE_TMC_REMOVE_UIM_NV_INIT
  06/30/10   dd      Icon display confirmation
  06/16/10   tkl     Merge in FEATURE_BMP_M2MPROFILE changes
  06/11/10   mib     Added support for QMI CAT
  06/09/10   nmb     Enabled FEATURE_PERSO_RUIM 
  05/28/10   yb      8660 Bring up changes
  05/26/10   yb      Removing inclusion of deprecated files
  05/24/10   bd      Disabled FEATURE_GSTK_FDN_CC_SUPPORT
  05/23/10   xz      Enable FEATURE_GSTK_NMR_UTRAN_SUPPORT
  05/10/10   tkl     Disable Perso feature in Gobi C2K build for RUIM_W_GSM_ACCESS
  04/26/10   ssr     Fixed GOBI 2000 feature support
  04/22/10   bd      Enabled FEATURE_GSTK_FDN_CC_SUPPORT
  04/21/10   mib     Fixed definition of FEATURE_CSIM
  04/16/10   mib     Added FEATURE_MMGSDI_SESSION_LIB_WITH_LEGACY. Added
                     FEATURE_CSIM and FEATURE_MMGSDI_SESSION_LIB for off-target
  03/22/10   mib     Moved FEATURE_CSIM and FEATURE_MMGSDI_SESSION_LIB to build level
  03/15/10   mib     Added FEATURE_DATA_QMI_UIM for QMI UIM service
  03/10/10   adp     Reverting the change for adding 6 MOB flavors for now (need more discussion)
  03/04/10   adp     Adding support for various MOB flavors
  03/05/10   nb      Enabled FEATURE_MMGSDI_MBMS unconditionally
  02/26/10   nk      Enabled FEATURE_MMGSDI_INIT_RTRE_CONFIG
  02/15/10   nb      Enabled DS/DS GSTK support
  02/09/10   jk      Enable FEATURE_MMGSDI_SESSION_READ_PRL
  02/04/10   nk      Reverted Memory Reduction Feature and disabling of Automation features
  02/02/10   sg      ESTK BIP enablement
  01/18/10   nk      Disabled MMGSDI Automation and enabled MMGSDI MEMORY Reduction
  01/13/10   nb/nk   Enabled CSIM features
  01/05/10   kp      Condition FEATURE_MODEM_HEAP removed for
                     FEATURE_UIM_ZI_MEMORY_REDUCTION
  11/11/09   sk      Enabling FEATURE_VIRTUAL_SIM
  10/20/09   ssr     Added OMH-related features
  10/08/09   dd      Enable raw messages in Symbian targets
  10/08/09   kp      Added FEATURE_UIM_ZI_MEMORY_REDUCTION
  09/25/09   yk      1) Move the UART selection to the HW cust file
                     2) Add support for 7x30
  09/13/09   ps      Add support for MDM9K
  07/21/09   mib     Conditionally defined FEATURE_UIM_TEST_FRAMEWORK if
                     TEST_FRAMEWORK is defined
  06/22/09   ssr     Moved AN authentication features from CUSTHW to CUST file
  06/15/09   sg      Enabled PERSO FEATURE for QTF
  06/01/09   dd      Enable FEATURE_CAT_REL6 for all targets
  06/05/09   yk      Enabled FEATURE_UIM_UICC required for FEATURE_UIM_USIM
  05/29/09   sg      Featurized FEATURE_UIM_WIM
  05/28/09   sg      Updated to work for test framework
  05/28/09   adp     Enable FEATURE_UIM_WIM.
  05/28/09   adp     Define UIM MFLO features conditionally based on feature
                     FEATURE_MFLO_OPENCA
  05/20/09   ssr     Fixed FEATURE_RUIM for multimode support.
  05/13/09   kk      Eliminated FEATURE_MMGSDI_NO_TCB_PTR_OR_CRIT_SEC feature
  04/25/09   ssr     Added FEATURE_UIM_RUIM_SUPPORT_SCI
  04/13/09   ssr     Add feature for SC2x target
  04/03/09   yk      SCMM specific cust file inclusion
  03/19/09   rm      Platform specific cust file inclusions
  02/23/09   nd      Fixed linker error when FEATURE_VIRTUAL_SIM is defined
  02/10/09   yb      Added FEATURE_GSTK_TEST_AUTOMATION
  01/14/09   vs      Added FEATURE_UIM_USES_NEW_SLEEP_API
  01/12/09   sun     Removed dependency on FEATURE_MMGSDI_PERSONALIZATION and
                     MODEM_MBMS
  01/07/09   sk      Added FEATURE_GSTK_ENH_MULTI_CLIENT_SUPPORT and
                     related support for ESTK and gstk streaming apdu.
  12/11/08   yk      Removed FEATURE_UIM_PMIC_ON_UIM1 which is a target
                     specific feature
  11/25/08   nk      Updated with RUIM support
  11/17/08   as/nk   Initial Revision.
  ===========================================================================*/
  /*---------------------------------------------------------------------------
                              Task Related Defines
  ---------------------------------------------------------------------------*/
  /* UIM Task Support */
  #define FEATURE_MMGSDI

  /* USAT support*/
  #define FEATURE_GSTK
  /* Adds support for the basic toolkit features */

  /*---------------------------------------------------------------------------
                                  MMGSDI
  ---------------------------------------------------------------------------*/
  /* MMGSDI Task support */
  #ifdef FEATURE_MMGSDI
    /* Enable the GSDI - DIAG Interface for Test Autoamtion */
    /* Enable the GSDI/MMGSDI Interface to provide Detailed Error Events */
    #define FEATURE_MMGSDI_CARD_ERROR_INFO
    /* Enable the support Factory Test Mode Software SIM */
  #endif /* FEATURE_MMGSDI */

  /*---------------------------------------------------------------------------
                     GENERIC SIM DRIVER INTERFACE (GSDI)
  ---------------------------------------------------------------------------*/


  /* R-UIM Support*/
  #if defined(FEATURE_RUIM)
    /* R-UIM Support from the UIM server */
    #define FEATURE_UIM_RUIM
    /* R-UIM run time enable support by other modules */
    #define FEATURE_UIM_RUN_TIME_ENABLE
    /* PrefMode is read from NV rather than the RUIM card. */

  #endif /* FEATURE_RUIM */

#endif /* CUSTUIM_H */
#ifndef CUSTWMS_H
#define CUSTWMS_H
/*===========================================================================

DESCRIPTION
  Configuration for WMS

  Copyright (c) 2002,2003,2004, 2005, 2006 by Qualcomm Technologies, Inc. 
  All Rights Reserved.
===========================================================================*/


#if defined(FEATURE_CDMA) || defined(FEATURE_LTE)
  #define FEATURE_CDSMS
#endif

#if (defined(FEATURE_WCDMA) || defined(FEATURE_GSM))
  #define FEATURE_GWSMS
#endif

/* Common Features across CDMA and GW Modes */
#if defined(FEATURE_CDSMS) || defined(FEATURE_GWSMS)
  
  /* Enables user data headers to support voice and email address.
  */
  #define FEATURE_SMS_UDH

  /* Features specific to CDMA Mode */
  #ifdef FEATURE_CDSMS

    /* Broadcast SMS support
    */
    #define FEATURE_BROADCAST_SMS

    /* New Implementation of CDMA Broadcast API
    */
    #define FEATURE_CDSMS_BROADCAST

  #endif /* FEATURE_CDSMS */

#endif /*defined(FEATURE_CDSMS) || defined(FEATURE_GWSMS)*/

#endif /* CUSTWMS_H */
#ifndef CUSTWCDMA_H
#define CUSTWCDMA_H
/*=========================================================================== 

                           C U S T    W C D M A 

DESCRIPTION
  Customer file for WCDMA

  Copyright (c) 2002-2008 by Qualcomm Technologies, Inc. All Rights Reserved.
===========================================================================*/


/* BMC Feature */       
#define FEATURE_UMTS_BMC

#define FEATURE_MODEM_MBMS
#endif /* CUSTWCDMA_H */
#ifndef CUSTDATACOMMON_H
#define CUSTDATACOMMON_H
/*===========================================================================

DESCRIPTION
  Configuration for DATACOMMON SU

Copyright (c) 2008-2010 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

#define FEATURE_DS

#endif /* CUSTDATACOMMON_H */
#ifndef CUSTMMODE_H
#define CUSTMMODE_H
/*========================================================================
DESCRIPTION

  Configuration CUST file for all of Multi Mode code

  Copyright (c) 2010  by Qualcomm Technologies, Inc. All Rights Reserved.
========================================================================*/


  /* Features that need to be defined on build to enable HDR meas */
  #ifdef FEATURE_IRAT_PILOT_MEAS
  #define FEATURE_CM_SS_MEAS
  #endif


#endif /* CUSTMMODE_H */
#ifndef CUSTNAS_H
#define CUSTNAS_H
/*===========================================================================

                           C U S T U S U R F

DESCRIPTION
  NAS File containing the Customized feature flags for the Target Baselines.

  Copyright (c) 1996-2009 by Qualcomm Technologies, Inc. All Rights Reserved.
===========================================================================*/

/* This feature lets UE to register on the network even if Ef LOCI is missing in the card*/

#define FEATURE_CCBS
/* Call Completion Busy Subscriber support*/


#endif /* CUSTNAS_H */


/*****************************************************************************/
#endif /* CUSTMODEM_MULTIMODE_H */


#endif /* CUSTA_________H */
#else
#endif

/* Now perform certain Sanity Checks on the various options and combinations
** of option.  Note that this list is probably NOT exhaustive, but just
** catches the obvious stuff.
*/


#endif /* CUSTOMER_H */
#endif

#endif /* TARGET_H */
#endif

#if defined FEATURE_L4  && !defined FEATURE_L4_KERNEL && !defined BUILD_BOOT_CHAIN && !defined ARCH_QDSP6
  #ifndef _ARM_ASM_
/*===========================================================================
     Copyright (c) 2004-2008 Qualcomm Technologies, Inc. All Rights Reserved.

     Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

#ifndef __MSM_SYSCALL_H__
#define __MSM_SYSCALL_H__
/*===========================================================================

              M S M    S Y S C A L L    H E A D E R    F I L E

DESCRIPTION
   This file contains the functions in assembly, which are equivelent
   to those inline assembly functions defined in msm_syscall.h

===========================================================================*/

 
/*
 * stdlib.h declares four types, several general purpose functions,
 * and defines several macros.
 */

#ifndef __stdlib_h
#define __stdlib_h


  #ifndef __STDLIB_DECLS
  #define __STDLIB_DECLS


#undef NULL
#define NULL 0                   /* see <stddef.h> */


   /*
    * Defining __USE_ANSI_EXAMPLE_RAND at compile time switches to
    * the example implementation of rand() and srand() provided in
    * the ANSI C standard. This implementation is very poor, but is
    * provided for completeness.
    */
#ifdef __USE_ANSI_EXAMPLE_RAND
#define rand _ANSI_rand
#else
#endif
   /*
    * RAND_MAX: an integral constant expression, the value of which
    * is the maximum value returned by the rand function.
    */
  #endif /* __STDLIB_DECLS */


#endif
/* end of stdlib.h */
#ifndef _L4_KERNEL_H_
#define _L4_KERNEL_H_


/*
 * The following types and macros are defined in several headers referred to in
 * the descriptions of the functions declared in that header. They are also
 * defined in this header file.
 */

#ifndef __stddef_h
#define __stddef_h

  #ifndef __STDDEF_DECLS
  #define __STDDEF_DECLS

#undef NULL  /* others (e.g. <stdio.h>) also define */
#define NULL 0
   /* null pointer constant. */

  #endif /* __STDDEF_DECLS */


#endif

/* end of stddef.h */

/*****************************************************************************/

#endif //_L4_KERNEL_H_

#endif  /* __MSM_SYSCALL_H__ */
  #endif
#endif


/* ---------------------------------------------------------------------
** Compiler Keyword Macros
** --------------------------------------------------------------------- */


#if (! defined T_WINNT) && (! defined __GNUC__)
  
    /* Non WinNT Targets */

    #if defined(__ARMCC_VERSION) 
      #define PACKED __packed
      #define PACKED_POST
    #else  /* __GNUC__ */
    #endif /* defined (__GNUC__) */
  

#else /* T_WINNT || TARGET_OS_SOLARIS || __GNUC__ */
#endif /* T_WINNT */

/* ----------------------------------------------------------------------
** Lint does not understand __packed, so we define it away here.  In the
** past we did this:
**   This helps us catch non-packed pointers accessing packed structures,
**   for example, (although lint thinks it is catching non-volatile pointers
**   accessing volatile structures).
**   This does assume that volatile is not being used with __packed anywhere
**   because that would make Lint see volatile volatile (grrr).
** but found it to be more trouble than it was worth as it would emit bogus
** errors
** ---------------------------------------------------------------------- */
#ifdef _lint
  #ifdef __packed
    #undef __packed
  #endif /* __packed */
  #define __packed
#endif

/* ----------------------------------------------------------------------
**                          STANDARD MACROS
** ---------------------------------------------------------------------- */


/*===========================================================================

MACRO MAX
MACRO MIN

DESCRIPTION
  Evaluate the maximum/minimum of 2 specified arguments.

PARAMETERS
  x     parameter to compare to 'y'
  y     parameter to compare to 'x'

DEPENDENCIES
  'x' and 'y' are referenced multiple times, and should remain the same
  value each time they are evaluated.

RETURN VALUE
  MAX   greater of 'x' and 'y'
  MIN   lesser of 'x' and 'y'

SIDE EFFECTS
  None

===========================================================================*/
#ifndef MAX
   #define  MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#endif

#ifndef MIN
   #define  MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif


/* Provide a new macro that will hopefully be used correctly to conditionally
** turn static on/off
*/
#define STATIC static


/*===========================================================================

                      FUNCTION DECLARATIONS

===========================================================================*/

#endif  /* COMDEF_H */

/* ========================================================================= */
/* ========================== Common Data Types ============================ */
/* ========================================================================= */
/** @ingroup common_data_types_group
@{ */
/** API version.\ Not currently used.
*/
#define WMS_API_VERSION           0x0500

enum { WMS_MAX_LEN                = 255 }; /* Maximum length; typically, the 
                                              number of bytes. */

enum { WMS_ADDRESS_MAX            = 48  }; /* Maximum number of bytes for an 
                                              address. */
enum { WMS_SUBADDRESS_MAX         = 48  }; /* Maximum number of bytes for a 
                                              subaddress. */
enum { WMS_CDMA_USER_DATA_MAX     = 229 }; /* Maximum number of bytes for CDMA 
                                              user data. */

enum { WMS_MESSAGE_LIST_MAX     = 255 };   /* Maximum number of messages. */

enum { WMS_MAX_UD_HEADERS       = 7};    /* Maximum number of user data headers. */
  /* Maximum size for UDH if the data encoding is the default 7-bit alphabet. */
enum { WMS_UDH_OTHER_SIZE            = 226 };
  /* Maximum size for UDH if the data encoding is other than a 7-bit or 8-bit 
     alphabet. */
enum { WMS_UDH_MAX_SND_SIZE          = 128 };
  /* Maximum size of user data that can be sent in a part, excluding the UDH 
     information. */
/* 229 (CDMA MAX USER DATA LEN) - 1(UDHL) - 1(UDH ID) - 1(UDH Len) */

/* ----- Picture definitions ----- */
enum { WMS_UDH_LARGE_PIC_SIZE    = 128 };
  /* Size of a large picture that can be sent in an EMS part. */
enum { WMS_UDH_SMALL_PIC_SIZE    = 32  };
  /* Size of a small picture that can be sent in an EMS part. */
enum { WMS_UDH_VAR_PIC_SIZE      = 134 };
  /* Maximum size of a variable picture that can be sent in an EMS part. */
/* 140 - 1(UDHL) - 5(UDH) */ 

/* ----- Animation definitions ----- */
enum { WMS_UDH_ANIM_NUM_BITMAPS  = 4 };  /* Number of animation bitmaps that can 
                                            be sent in an EMS part. */
enum { WMS_UDH_LARGE_BITMAP_SIZE = 32 }; /* Bitmap size of a large animation. */
enum { WMS_UDH_SMALL_BITMAP_SIZE = 8 };  /* Bitmap size of a small animation. */


/** @ingroup common_data_types_group
@{ */
/** Digit mode indicator for the address parameters.
*/
typedef enum
{
  WMS_DIGIT_MODE_4_BIT     = 0, /**< Four-bit DTMF digit codes. */
  WMS_DIGIT_MODE_8_BIT     = 1, /**< Eight-bit DTMF digit codes. */
/** @cond */
  WMS_DIGIT_MODE_MAX32     = 0x10000000
/** @endcond */
} wms_digit_mode_e_type;

/** @ingroup common_data_types_group
@{ */
/** Number type (originating or destination) for the SMS or EMS message. 
  The first seven types are not data network addresses. The last two types are 
  data network addresses. In the address data digits, each byte contains an ASCII 
  character e.g., x\@y.com,a\@b.com (see TIA/EIA-637A 3.4.3.3). 
  @sa wms_digit_mode_e_type, wms_number_mode_e_type, wms_address_s_type
*/
typedef enum
{
  WMS_NUMBER_UNKNOWN        = 0,  /**< &nbsp; */
  WMS_NUMBER_INTERNATIONAL  = 1,  /**< &nbsp; */
  WMS_NUMBER_NATIONAL       = 2,  /**< &nbsp; */
  WMS_NUMBER_NETWORK        = 3,  /**< &nbsp; */
  WMS_NUMBER_SUBSCRIBER     = 4,  /**< &nbsp; */
  WMS_NUMBER_ALPHANUMERIC   = 5,  /**< For GSM SMS, the address value is 
                                       GSM 7-bit characters. */
  WMS_NUMBER_ABBREVIATED    = 6,  /**< &nbsp; */
  WMS_NUMBER_RESERVED_7     = 7,  /**< &nbsp; */
  
  WMS_NUMBER_DATA_IP        = 1,  /**< &nbsp; */
  WMS_NUMBER_INTERNET_EMAIL = 2,  /**< &nbsp; */
/** @cond */ 
  WMS_NUMBER_MAX32        = 0x10000000
/** @endcond */
} wms_number_type_e_type;

/** @ingroup common_data_types_group
@{ */
/** Numbering plan used for the address.
  This enumeration is used only if digit_mode is 8-bit and number_mode is 
  WMS_NUMBER_MODE_NONE_DATA_NETWORK.
*/
typedef enum
{
  WMS_NUMBER_PLAN_UNKNOWN     = 0,  /**< &nbsp; */
  WMS_NUMBER_PLAN_TELEPHONY   = 1,  /**< As defined in CCITT E.164 and E.163, 
                                         including the ISDN plan. */
  WMS_NUMBER_PLAN_RESERVED_2  = 2,  /**< &nbsp; */
  WMS_NUMBER_PLAN_DATA        = 3,  /**< As defined in CCITT X.121. */
  WMS_NUMBER_PLAN_TELEX       = 4,  /**< As defined in CCITT F.69. */
  WMS_NUMBER_PLAN_RESERVED_5  = 5,  /**< &nbsp; */
  WMS_NUMBER_PLAN_RESERVED_6  = 6,  /**< &nbsp; */ 
  WMS_NUMBER_PLAN_RESERVED_7  = 7,  /**< &nbsp; */
  WMS_NUMBER_PLAN_RESERVED_8  = 8,  /**< &nbsp; */
  WMS_NUMBER_PLAN_PRIVATE     = 9,  /**< &nbsp; */
  WMS_NUMBER_PLAN_RESERVED_10 = 10, /**< &nbsp; */
  WMS_NUMBER_PLAN_RESERVED_11 = 11, /**< &nbsp; */
  WMS_NUMBER_PLAN_RESERVED_12 = 12, /**< &nbsp; */
  WMS_NUMBER_PLAN_RESERVED_13 = 13, /**< &nbsp; */
  WMS_NUMBER_PLAN_RESERVED_14 = 14, /**< &nbsp; */
  WMS_NUMBER_PLAN_RESERVED_15 = 15, /**< &nbsp; */
/** @cond */
  WMS_NUMBER_PLAN_MAX32       = 0x10000000
/** @endcond */
} wms_number_plan_e_type;

/** @ingroup common_data_types_group
@{ */
/** Number mode that indicates whether the address type is a data network 
  address.
*/
typedef enum
{
  WMS_NUMBER_MODE_NONE_DATA_NETWORK      = 0, /**< &nbsp; */
  WMS_NUMBER_MODE_DATA_NETWORK           = 1, /**< &nbsp;*/
/** @cond */
  WMS_NUMBER_MODE_DATA_NETWORK_MAX32     = 0x10000000
/** @endcond */
} wms_number_mode_e_type;

/** @ingroup common_data_types_group
@{ */
/** Holds the address parameters for sending an SMS or EMS message.
*/
typedef struct wms_address_s
{
  wms_digit_mode_e_type          digit_mode;
    /**< Indicates 4 bit or 8 bit. */
  wms_number_mode_e_type         number_mode;
    /**< Used in CDMA only.\ It is meaningful only when digit_mode is 8 bit. */
  wms_number_type_e_type         number_type;
    /**< For a CDMA address, this is used only when digit_mode is 8 bit.\ 
         To specify an international address for CDMA, use the following 
         settings:
          - Digit_mode = 8-bit
          - Number_mode = NONE_DATA_NETWORK
          - Number_type = INTERNATIONAL
          - Number_plan = TELEPHONY
          - Number_of_digits = Number of digits
          - Digits = ASCII digits, e.g., 1, 2, 3, 4, and 5 */

  wms_number_plan_e_type         number_plan;
    /**< For a CDMA address, this is used only when digit_mode is 8 bit. */
  uint8                          number_of_digits;
    /**< Number of bytes in the digits array. */
  uint8                          digits[ WMS_ADDRESS_MAX ];
    /**< Each byte in this array represents a 4-bit or 8-bit digit of address 
         data. */
} wms_address_s_type;

/** @ingroup common_data_types_group
@{ */
/** Subaddress types.
*/
typedef enum
{
  WMS_SUBADDRESS_NSAP           = 0,  /**< Network Service Access Protocol 
                                           (CCITT X.213 or ISO 8348 AD2). */
  WMS_SUBADDRESS_USER_SPECIFIED = 1,  /**< E.g., X.25. */
/** @cond */ 
  WMS_SUBADDRESS_MAX32          = 0x10000000
/** @endcond */
} wms_subaddress_type_e_type;

/** @ingroup common_data_types_group
@{ */
/** Holds the subaddress parameters.
*/
typedef struct wms_subaddress_s
{
  wms_subaddress_type_e_type           type;
    /**< Type of subaddress. */
  boolean                              odd; 
    /**< TRUE means the last byte's lower four bits are to be ignored. */
  uint8                                number_of_digits;
    /**< Number of bytes in the digits array. */
  uint8                                digits[ WMS_SUBADDRESS_MAX ];
    /**< Each byte in this array represents an 8-bit digit of subaddress data. */
} wms_subaddress_s_type;

/** @ingroup common_data_types_group
@{ */
/** Message status tags.
  The values assigned here match those values in the SIM or R-UIM card.
*/
typedef enum
{
  WMS_TAG_NONE        = 0x00,             /**< &nbsp; */
  WMS_TAG_MT_READ     = 0x01,             /**< &nbsp; */
  WMS_TAG_MT_NOT_READ = 0x03,             /**< &nbsp; */
  WMS_TAG_MO_SENT     = 0x05,             /**< No status report is requested. */
  WMS_TAG_MO_NOT_SENT = 0x07,             /**< &nbsp; */
  WMS_TAG_MO_SENT_ST_NOT_RECEIVED = 0x0D, /**< GSM/WCDMA only. */
  WMS_TAG_MO_SENT_ST_NOT_STORED   = 0x15, /**< GSM/WCDMA only. */
  WMS_TAG_MO_SENT_ST_STORED       = 0x35, /**< GSM/WCDMA only. */
  WMS_TAG_MO_TEMPLATE = 0x100,            /**< SMS parameter. */
  WMS_TAG_STATUS_RPT  = 0x200,            /**< SMSR parameter. */
/** @cond */
  WMS_TAG_MAX32       = 0x10000000
/** @endcond */
} wms_message_tag_e_type;

/** @ingroup common_data_types_group
@{ */
/** Macro that checks if the tag represents a mobile-originated message. */
#define WMS_IS_MO( tag )           \
  ( ( (tag) & 0x7 ) == WMS_TAG_MO_SENT ||  \
    (tag) == WMS_TAG_MO_NOT_SENT ||  \
    (tag) == WMS_TAG_MO_TEMPLATE )

/** Macro that checks if the tag represents a mobile-terminated message. */
#define WMS_IS_MT( tag )           \
  ( (tag) == WMS_TAG_MT_READ ||      \
    (tag) == WMS_TAG_MT_NOT_READ )

/** Macro that checks if the tag is valid. */
#define WMS_IS_VALID_TAG( tag )    \
  ( (tag) == WMS_TAG_NONE ||         \
    (tag) == WMS_TAG_MT_READ ||      \
    (tag) == WMS_TAG_MT_NOT_READ ||  \
    (tag) == WMS_TAG_MO_SENT ||      \
    (tag) == WMS_TAG_MO_NOT_SENT ||  \
    (tag) == WMS_TAG_MO_SENT_ST_NOT_RECEIVED || \
    (tag) == WMS_TAG_MO_SENT_ST_NOT_STORED ||   \
    (tag) == WMS_TAG_MO_SENT_ST_STORED ||       \
    (tag) == WMS_TAG_MO_TEMPLATE ||  \
    (tag) == WMS_TAG_STATUS_RPT )

/** @ingroup common_data_types_group
@{ */
/** Timestamp structure. 
  If the year is between 96 and 99, the actual year is 1900 + year. If the year 
  is between 00 and 95, the actual year is 2000 + year.
    @note Each field has two BCD digits. The byte arrangement is <MSB...LSB>. 
*/
typedef struct
{
  uint8      year;     /**< 00 through 99. */
  uint8      month;    /**< 01 through 12. */
  uint8      day;      /**< 01 through 31. */
  uint8      hour;     /**< 00 through 23. */
  uint8      minute;   /**< 00 through 59. */
  uint8      second;   /**< 00 through 59. */
  sint7      timezone; /**< +/-, [-48,+48] number of 15 minutes; GSM/WCDMA only. */
} wms_timestamp_s_type;

/** @ingroup common_data_types_group
@{ */
/** Language types.
*/
typedef enum
{
  WMS_LANGUAGE_UNSPECIFIED = 0, /**< &nbsp; */
  WMS_LANGUAGE_ENGLISH,         /**< &nbsp; */
  WMS_LANGUAGE_FRENCH,          /**< &nbsp; */
  WMS_LANGUAGE_SPANISH,         /**< &nbsp; */
  WMS_LANGUAGE_JAPANESE,        /**< &nbsp; */
  WMS_LANGUAGE_KOREAN,          /**< &nbsp; */
  WMS_LANGUAGE_CHINESE,         /**< &nbsp; */
  WMS_LANGUAGE_HEBREW,          /**< &nbsp; */
/** @cond */
  WMS_LANGUAGE_MAX32       = 0x10000000
/** @endcond */
} wms_language_e_type;

/** @ingroup common_data_types_group
@{ */
/** System mode (CDMA or GSM/WCDMA) to be used for a message.
*/
typedef enum
{
  WMS_MESSAGE_MODE_CDMA = 0, /**< &nbsp; */
  WMS_MESSAGE_MODE_GW,       /**< &nbsp; */
/** @cond */
  WMS_MESSAGE_MODE_MAX,
  WMS_MESSAGE_MODE_MAX32 = 0x100000
/** @endcond */
} wms_message_mode_e_type;

/** @ingroup common_data_types_group
@{ */
/** Transport Service data formats.
*/ 
typedef enum
{
  WMS_FORMAT_CDMA         = 0,  /**< CDMA format, IS-95. */
  WMS_FORMAT_ANALOG_CLI,        /**< Analog calling line identification, IS-91. */
  WMS_FORMAT_ANALOG_VOICE_MAIL, /**< Analog voice mail, IS-91. */
  WMS_FORMAT_ANALOG_SMS,        /**< Analog SMS, IS-91. */
  WMS_FORMAT_ANALOG_AWISMS,     /**< Analog IS-95 Alert With Information SMS. */
  WMS_FORMAT_MWI,               /**< Message Waiting Indication as voice mail. */
  WMS_FORMAT_GW_PP,             /**< GSM/WCDMA Point-to-Point SMS. */
  WMS_FORMAT_GW_CB,             /**< GSM/WCDMA Cell Broadcast SMS. */
/** @cond */
  WMS_FORMAT_MAX,
  WMS_FORMAT_MAX32        = 0x10000000
/** @endcond */
} wms_format_e_type;

/** @ingroup common_data_types_group
@{ */
/** Specific requirements for sending a message.
*/
typedef enum
{
  WMS_SEND_MODE_CLIENT_MESSAGE = 0, /**< &nbsp; */
  WMS_SEND_MODE_MEMORY_STORE,       /**< &nbsp; */
/** @cond */
  WMS_SEND_MODE_MAX,
  WMS_SEND_MODE_MAX32 = 0x10000000  /* Pad to 32-bit int */
/** @endcond */
} wms_send_mode_e_type;

/** @ingroup common_data_types_group
@{ */
/** Specific requirements for storing a message.
*/
typedef enum
{
  WMS_WRITE_MODE_INSERT = 0, /**< &nbsp; */
  WMS_WRITE_MODE_REPLACE,    /**< &nbsp; */
/** @cond */
  WMS_WRITE_MODE_MAX,
  WMS_WRITE_MODE_MAX32  = 0x10000000
/** @endcond */
} wms_write_mode_e_type;

/** @ingroup common_data_types_group
@{ */
/** Message classes in the Data Coding Scheme of a TPDU or Cell Broadcast 
  message.
*/
typedef enum
{
  WMS_MESSAGE_CLASS_0 = 0, /**< &nbsp; */
  WMS_MESSAGE_CLASS_1,     /**< &nbsp; */
  WMS_MESSAGE_CLASS_2,     /**< &nbsp; */
  WMS_MESSAGE_CLASS_3,     /**< &nbsp; */
  WMS_MESSAGE_CLASS_NONE,  /**< &nbsp; */
  WMS_MESSAGE_CLASS_CDMA, /**< &nbsp; */
/** @cond */
  WMS_MESSAGE_CLASS_MAX,
  WMS_MESSAGE_CLASS_MAX32 = 0x10000000
/** @endcond */
} wms_message_class_e_type;

/** @ingroup common_data_types_group
@{ */
/** Message routing types.
*/
typedef enum
{
  WMS_ROUTE_DISCARD = 0,      /**< Discard the message. */
  WMS_ROUTE_STORE_AND_NOTIFY, /**< Store the message and notify the clients. */
  WMS_ROUTE_TRANSFER_ONLY,    /**< Do not store the message, but send it to 
                                   the clients, */
  WMS_ROUTE_TRANSFER_AND_ACK, /**< Send the message to the clients and 
                                   acknowledge it. */
  WMS_ROUTE_NO_CHANGE,        /**< Do not change the current setting. */
/** @cond */
  WMS_ROUTE_INTERNAL_PROC,    /* INTERNAL USE ONLY */
  WMS_ROUTE_MAX,
  WMS_ROUTE_MAX32   = 0x10000000
/** @endcond */
  } wms_route_e_type;

/** @ingroup common_data_types_group
@{ */
/** Types of memory stores that are supported.
*/
typedef enum
{
  WMS_MEMORY_STORE_NONE = 0, /**< No memory storage. */

  /* GSM/WCDMA memory stores  */
  WMS_MEMORY_STORE_RAM_GW, /**< GSM/WCDMA memory store in RAM. */
  WMS_MEMORY_STORE_SIM,    /**< GSM/WCDMA memory store in SIM. */
  WMS_MEMORY_STORE_NV_GW,  /**< GSM/WCDMA memory store in nonvolatile memory. */

  /* CDMA memory stores   */
  WMS_MEMORY_STORE_RAM_CDMA, /**< CDMA memory store in RAM.\ Not supported. */
  WMS_MEMORY_STORE_RUIM,     /**< CDMA memory store in R-UIM. */
  WMS_MEMORY_STORE_NV_CDMA,  /**< CDMA memory store in nonvolatile memory. */

/** @cond */
  WMS_MEMORY_STORE_MAX,
  WMS_MEMORY_STORE_MAX32  = 0x10000000
/** @endcond */
} wms_memory_store_e_type;

/** @ingroup common_data_types_group
@{ */
/** Preferred Mobile Originated SMS domain settings for GSM/GPRS/UMTS.
*/
typedef enum
{
  WMS_GW_DOMAIN_PREF_CS_PREFERRED  = 0, /**< &nbsp; */
  WMS_GW_DOMAIN_PREF_PS_PREFERRED,      /**< &nbsp; */
  WMS_GW_DOMAIN_PREF_CS_ONLY,           /**< &nbsp; */
  WMS_GW_DOMAIN_PREF_PS_ONLY,           /**< &nbsp; */
/** @cond */
  WMS_GW_DOMAIN_PREF_MAX,
  WMS_GW_DOMAIN_PREF_MAX32  = 0x10000000
/** @endcond */
} wms_gw_domain_pref_e_type;

/** @ingroup common_data_types_group
@{ */
/** Primary client configuration parameters.
*/
typedef struct
{
  boolean      set_primary;        /**< Indicates whether to set the client as 
                                        the primary client. */
  boolean      use_client_memory;  /**< Indicates whether to use the client 
                                        memory. */
} wms_cfg_set_primary_client_s_type;

/** @ingroup common_data_types_group
@{ */
/** Mode preferences [0,1,2] for link control.
*/
typedef enum
{
  WMS_LINK_CONTROL_DISABLED = 0, /**< &nbsp; */
  WMS_LINK_CONTROL_ENABLED_ONE,  /**< &nbsp; */
  WMS_LINK_CONTROL_ENABLE_ALL,   /**< &nbsp; */
/** @cond */
  WMS_LINK_CONTROL_MAX,
  WMS_LINK_CONTROL_MAX32 = 0x10000000
/** @endcond */
} wms_cfg_link_control_mode_e_type;

/** @ingroup common_data_types_group
@{ */
/** Status of the link control.
*/
typedef enum
{
  WMS_LINK_CONTROL_INACTIVE = 0,       /**< &nbsp; */
  WMS_LINK_CONTROL_ACTIVE,             /**< &nbsp; */
  WMS_LINK_CONTROL_TIMEOUT,            /**< &nbsp; */
  WMS_LINK_CONTROL_CONNECTION_RELEASED /**< &nbsp; */
} wms_cfg_link_control_status_e_type;

/** @ingroup common_data_types_group
@{ */
/** Mode and status of the link control.
*/
typedef struct wms_cfg_link_control_s
{
  wms_cfg_link_control_mode_e_type    link_control_mode; 
    /**< Link control mode. */
  wms_cfg_link_control_status_e_type  link_control_status; 
    /**< Link control status. */
} wms_cfg_link_control_s_type;

/** @ingroup common_data_types_group
@{ */
/** Types of SMS bearers.
*/
typedef enum
{
  WMS_BEARER_CDMA_1X = 0, /**< &nbsp; */
  WMS_BEARER_CDMA_EVDO,   /**< &nbsp; */
  WMS_BEARER_CDMA_WLAN,   /**< &nbsp; */

  //GSM/UMTS bearers to be supported later:
  WMS_BEARER_GW_CS,     /**< To be supported at a later date. */
  WMS_BEARER_GW_PS,     /**< To be supported at a later date. */
  WMS_BEARER_GW_PS_SIP, /**< To be supported at a later date. */
  WMS_BEARER_GW_WLAN,   /**< To be supported at a later date. */

/** @cond */
  WMS_BEARER_MAX,
  WMS_BEARER_MAX32   = 0x10000000
/** @endcond */
} wms_bearer_e_type; 

/** @ingroup common_data_types_group
@{ */
/** Message number type.
  In CDMA mode, the message number is the Message ID of the bearer data. 
  In GSM/WCDMA mode, it is the Message Reference number in the TPDU.
*/
typedef uint32  wms_message_number_type;

/** @ingroup common_data_types_group
@{ */
/* Invalid message number. */
#define WMS_DUMMY_MESSAGE_NUMBER  0xFFFFFFFF

/** @ingroup common_data_types_group
@{ */
/** Message index in a message list.
*/
typedef uint32  wms_message_index_type;

/** @ingroup common_data_types_group
@{ */
/* Invalid message index number. */
#define WMS_DUMMY_MESSAGE_INDEX  0xFFFFFFFF

/** @ingroup common_data_types_group
@{ */
/** Transaction ID to help differentiate between multiple incoming messages.
*/
typedef uint32  wms_transaction_id_type;

/** @ingroup common_data_types_group
@{ */
/* Invalid transaction ID. */
#define WMS_DUMMY_TRANSACTION_ID  0xFFFFFFFF 

/** @ingroup common_data_types_group
@{ */
/** WMS command IDs for Configuration group, Message group, Dedicated Channel, 
  Broadcast Multimode group, and Debug group commands. 
*/
typedef enum
{
  /* Configuration group commands */
  WMS_CMD_CFG_SET_ROUTES = 0x0000,         /**< &nbsp; */
  WMS_CMD_CFG_GET_ROUTES,                  /**< &nbsp; */
  WMS_CMD_CFG_GET_MEMORY_STATUS,           /**< &nbsp; */
  WMS_CMD_CFG_GET_MESSAGE_LIST,            /**< &nbsp; */
  WMS_CMD_CFG_SET_GW_DOMAIN_PREF,          /**< &nbsp; */
  WMS_CMD_CFG_GET_GW_DOMAIN_PREF,          /**< &nbsp; */
  WMS_CMD_CFG_SET_PRIMARY_CLIENT,          /**< &nbsp; */
  WMS_CMD_CFG_SET_MEMORY_FULL,             /**< &nbsp; */
  WMS_CMD_CFG_SET_LINK_CONTROL,            /**< &nbsp; */
  WMS_CMD_CFG_GET_LINK_CONTROL,            /**< &nbsp; */

  /* Message group commands  */
  WMS_CMD_MSG_SEND       = 100,            /**< &nbsp; */
  WMS_CMD_MSG_ACK,                         /**< &nbsp; */
  WMS_CMD_MSG_READ,                        /**< &nbsp; */
  WMS_CMD_MSG_WRITE,                       /**< &nbsp; */
  WMS_CMD_MSG_DELETE,                      /**< &nbsp; */
  WMS_CMD_MSG_DELETE_ALL,                  /**< &nbsp; */
  WMS_CMD_MSG_MODIFY_TAG,                  /**< &nbsp; */
  WMS_CMD_MSG_READ_TEMPLATE,               /**< &nbsp; */
  WMS_CMD_MSG_WRITE_TEMPLATE,              /**< &nbsp; */
  WMS_CMD_MSG_DELETE_TEMPLATE,             /**< &nbsp; */
  WMS_CMD_MSG_DELETE_TEMPLATE_ALL,         /**< &nbsp; */

  WMS_CMD_MSG_READ_STS_REPORT,             /**< &nbsp; */
  WMS_CMD_MSG_WRITE_STS_REPORT,            /**< &nbsp; */
  WMS_CMD_MSG_DELETE_STS_REPORT,           /**< &nbsp; */
  WMS_CMD_MSG_DELETE_STS_REPORT_ALL,       /**< &nbsp; */
  WMS_CMD_MSG_TRANSPORT_REG,               /**< &nbsp; */

  /* DC commands  */   
  WMS_CMD_DC_CONNECT     = 200,            /**< &nbsp; */
  WMS_CMD_DC_DISCONNECT,                   /**< &nbsp; */
  WMS_CMD_DC_ENABLE_AUTO_DISCONNECT,       /**< &nbsp; */
  WMS_CMD_DC_DISABLE_AUTO_DISCONNECT,      /**< &nbsp; */

  /* BC_MM commands  */
  WMS_CMD_BC_MM_GET_CONFIG   = 400,        /**< &nbsp; */
  WMS_CMD_BC_MM_GET_PREF,                  /**< &nbsp; */
  WMS_CMD_BC_MM_SET_PREF,                  /**< &nbsp; */
  WMS_CMD_BC_MM_GET_TABLE,                 /**< &nbsp; */
  WMS_CMD_BC_MM_SELECT_SRV,                /**< &nbsp; */
  WMS_CMD_BC_MM_GET_ALL_SRV_IDS,           /**< &nbsp; */
  WMS_CMD_BC_MM_GET_SRV_INFO,              /**< &nbsp; */
  WMS_CMD_BC_MM_ADD_SRV,                   /**< &nbsp; */
  WMS_CMD_BC_MM_DELETE_SRV,                /**< &nbsp; */
  WMS_CMD_BC_MM_CHANGE_SRV_INFO,           /**< &nbsp; */
  WMS_CMD_BC_MM_DELETE_ALL_SERVICES,       /**< &nbsp; */
  WMS_CMD_BC_MM_SELECT_ALL_SERVICES,       /**< &nbsp; */
  WMS_CMD_BC_MM_SET_PRIORITY_ALL_SERVICES, /**< &nbsp; */
  WMS_CMD_BC_MM_MSG_DELETE_INDICATION,     /**< &nbsp; */
  WMS_CMD_BC_MM_MSG_DELETE_ALL_INDICATION, /**< &nbsp; */
  WMS_CMD_BC_MM_GET_READING_PREF,          /**< &nbsp; */
  WMS_CMD_BC_MM_SET_READING_PREF,          /**< &nbsp; */

  /* DBG commands  */
  WMS_CMD_DBG_RESET_TL_SEQ_NUM  = 500,     /**< &nbsp; */
  WMS_CMD_DBG_SET_MSG_REF_NUM,             /**< &nbsp; */
  WMS_CMD_DBG_CLEAR_SMMA,                  /**< &nbsp; */
  WMS_CMD_DBG_GET_RETRY_INTERVAL,          /**< &nbsp; */
  WMS_CMD_DBG_SET_RETRY_INTERVAL,          /**< &nbsp; */

  /* -- NOTE: the following are for internal use only --- */

/** @cond */
  /* Internal: events from MC  */
  WMS_CMD_MSG_MC_MT_MSG_E     = 600,
  WMS_CMD_MSG_MC_MO_STATUS_E,
  WMS_CMD_MSG_MC_STATE_CHANGED_E,
  WMS_CMD_MSG_MC_MWI_E,
  WMS_CMD_MSG_GSTK_EVT,

  /* Internal: events from CM  */
  WMS_CMD_DC_CALL_INCOM_E,
  WMS_CMD_DC_CALL_CONNECT_E,
  WMS_CMD_DC_CALL_END_E,
  WMS_CMD_DC_CALL_ERROR_E,  /* in case of CM cmd errors */

  WMS_CMD_CM_BC_ENABLE_FAILURE_E,
  WMS_CMD_CM_START_FULL_SERVICE_E,
  WMS_CMD_CM_ONLINE_E,
  WMS_CMD_CM_LPM_E,

  WMS_CMD_GSDI_ASYNC_CB,
  WMS_CMD_GSDI_CARD_STATUS_CB,
  WMS_CMD_GSDI_REFRESH_FCN_IND,
  WMS_CMD_BC_SCPT_DATA,

  WMS_CMD_SS_CHANGE_INFO,
  WMS_CMD_IPAPP_CHANGE_INFO,

  /* Internal events from DEM  */
  WMS_CMD_DEM_APPS_POWERDOWN,
  WMS_CMD_DEM_APPS_SUSPEND,
  WMS_CMD_DEM_APPS_POWER_RESTORED,
  WMS_CMD_DEM_APPS_RUN_QUIET,
  WMS_CMD_DEM_APPS_RUN_NORMAL,

  WMS_CMD_DUMMY,
  
  WMS_CMD_MMGSDI_EVENT_CB,
  WMS_CMD_MMGSDI_RESPONSE_CB,
  
  WMS_CMD_DBG_GET_RETRY_PERIOD, 
  WMS_CMD_DBG_SET_RETRY_PERIOD, 
  
  WMS_CMD_MAX,
  WMS_CMD_MAX32 = 0x10000000  /* pad to 32 bit int */
/** @endcond */
} wms_cmd_id_e_type;
/*~ SENTINEL wms_cmd_id_e_type.WMS_CMD_MAX */

/** @ingroup common_data_types_group
@{ */
/** Command status errors reported in the command callbacks.
*/
typedef enum
{
  WMS_CMD_ERR_NONE       = 0,          /**< &nbsp; */

  WMS_CMD_ERR_CLIENT_ID,               /**< &nbsp; */
  WMS_CMD_ERR_NO_RESOURCE,             /**< &nbsp; */
  WMS_CMD_ERR_UNSUPPORTED,             /**< &nbsp; */
  WMS_CMD_ERR_BUSY,                    /**< &nbsp; */
  WMS_CMD_ERR_NULL_PTR,                /**< &nbsp; */
  WMS_CMD_ERR_NO_NETWORK,              /**< &nbsp; */
  WMS_CMD_ERR_BAD_PARM,                /**< &nbsp; */
  WMS_CMD_ERR_CS_ERROR,                /**< &nbsp; */

  WMS_CMD_ERR_CFG_NV_WRITE  = 100,     /**< &nbsp; */
  WMS_CMD_ERR_CFG_NV_READ,             /**< &nbsp; */
  WMS_CMD_ERR_CFG_ROUTE,               /**< &nbsp; */
  WMS_CMD_ERR_CFG_MEM_STORE,           /**< &nbsp; */
  WMS_CMD_ERR_CFG_MSG_CLASS,           /**< &nbsp; */
  WMS_CMD_ERR_CFG_DOMAIN_PREF,         /**< &nbsp; */
  WMS_CMD_ERR_CFG_UNPRIVILEGED_CLIENT, /**< &nbsp; */
  WMS_CMD_ERR_CFG_LINK_CONTROL,        /**< &nbsp; */
  
  WMS_CMD_ERR_MSG_MEMORY_STORE = 200,  /**< &nbsp; */
  WMS_CMD_ERR_MSG_INDEX,               /**< &nbsp; */
  WMS_CMD_ERR_MSG_EMPTY_MESSAGE,       /**< &nbsp; */
  WMS_CMD_ERR_MSG_TAG,                 /**< &nbsp; */
  WMS_CMD_ERR_MSG_ENCODE,              /**< &nbsp; */
  WMS_CMD_ERR_MSG_DECODE,              /**< &nbsp; */
  WMS_CMD_ERR_MSG_SEND_MODE,           /**< &nbsp; */
  WMS_CMD_ERR_MSG_WRITE_MODE,          /**< &nbsp; */
  WMS_CMD_ERR_MSG_MEMORY_FULL,         /**< &nbsp; */
  WMS_CMD_ERR_MSG_RAM_FULL,            /**< &nbsp; */
  WMS_CMD_ERR_MSG_SIM_FULL,            /**< &nbsp; */
  WMS_CMD_ERR_MSG_NV_FULL,             /**< &nbsp; */
  WMS_CMD_ERR_MSG_TRANSACTION_ID,      /**< &nbsp; */
  WMS_CMD_ERR_MSG_SIM_WRITE,           /**< &nbsp; */
  WMS_CMD_ERR_MSG_SIM_READ,            /**< &nbsp; */
  WMS_CMD_ERR_MSG_SIM_DELETE,          /**< &nbsp; */
  WMS_CMD_ERR_MSG_RUIM_WRITE,          /**< &nbsp; */
  WMS_CMD_ERR_MSG_RUIM_READ,           /**< &nbsp; */
  WMS_CMD_ERR_MSG_RUIM_DELETE,         /**< &nbsp; */
  WMS_CMD_ERR_MSG_NV_WRITE,            /**< &nbsp; */
  WMS_CMD_ERR_MSG_NV_READ,             /**< &nbsp; */
  WMS_CMD_ERR_MSG_NV_DELETE,           /**< &nbsp; */
  WMS_CMD_ERR_MSG_TPDU_TYPE,           /**< &nbsp; */
  WMS_CMD_ERR_MSG_FORMAT,              /**< &nbsp; */
  WMS_CMD_ERR_MSG_NO_MO_MSG,           /**< &nbsp; */
  WMS_CMD_ERR_MSG_NO_SC_ADDRESS,       /**< &nbsp; */
  WMS_CMD_ERR_MSG_LPM,                 /**< &nbsp; */
  WMS_CMD_ERR_MSG_BLOCKED,             /**< &nbsp; */ 

  WMS_CMD_ERR_BC_BAD_PREF    = 300,    /**< &nbsp; */
  WMS_CMD_ERR_BC_CANNOT_ACTIVATE,      /**< &nbsp; */
  WMS_CMD_ERR_BC_CANNOT_ACTIVATE_ALL,  /**< &nbsp; */
  WMS_CMD_ERR_BC_NV_WRITE,             /**< &nbsp; */
  WMS_CMD_ERR_BC_BAD_SRV_ID,           /**< &nbsp; */
  WMS_CMD_ERR_BC_TABLE_FULL,           /**< &nbsp; */
  WMS_CMD_ERR_BC_DUPLICATE_SRV,        /**< &nbsp; */
  WMS_CMD_ERR_BC_SIM_WRITE,            /**< &nbsp; */
  WMS_CMD_ERR_BC_SIM_READ,             /**< &nbsp; */

  WMS_CMD_ERR_DC_BAD_STATE   = 400,    /**< &nbsp; */
  WMS_CMD_ERR_DC_BAD_CALL_ID,          /**< &nbsp; */
  WMS_CMD_ERR_DC_BAD_PARM,             /**< &nbsp; */
  WMS_CMD_ERR_DC_LPM,                  /**< &nbsp; */

  WMS_CMD_ERR_DBG_BAD_PARAM  = 500,    /**< &nbsp; */

  WMS_CMD_ERR_TRANSPORT_NOT_READY  = 600,  /**< Transport is not ready 
                                                to send MO SMS */
  WMS_CMD_ERR_TRANSPORT_NOT_ALLOWED,       /**< Transport does not allow MO SMS
                                                e.g. limited service */

/** @cond */
  WMS_CMD_ERR_MAX, 
  WMS_CMD_ERR_MAX32 = 0x10000000     /* pad to 32 bit int */
/** @endcond */
} wms_cmd_err_e_type;
/*~ SENTINEL wms_cmd_err_e_type.WMS_CMD_ERR_MAX */

/** @ingroup common_data_types_group
@{ */
/** Command callback function for reporting the command execution status of 
  the API.
*/
typedef void (* wms_cmd_cb_type )
(
  wms_cmd_id_e_type        cmd,        /**< ID of the command whose status 
                                            is being reported. */
  void                     *user_data, /**< Pointer to the user data. */
  wms_cmd_err_e_type       cmd_err     /**< Status of the command. */  
);
/*~ CALLBACK wms_cmd_cb_type
    ONERROR return  */

/** @ingroup common_data_types_group
@{ */
/** Client request errors.
*/
typedef enum
{
  WMS_CLIENT_ERR_NONE,                /**< &nbsp; */
  WMS_CLIENT_ERR_CLIENT_ID_PTR,       /**< &nbsp; */
  WMS_CLIENT_ERR_CLIENT_TYPE,         /**< &nbsp; */
  WMS_CLIENT_ERR_CLIENT_ID,           /**< &nbsp; */
  WMS_CLIENT_ERR_TASK_NOT_READY,      /**< &nbsp; */
  WMS_CLIENT_ERR_INVALID_PROCESSOR,   /**< &nbsp; */
  WMS_CLIENT_ERR_INVALID_NOTIFY_TYPE, /**< &nbsp; */
  WMS_CLIENT_ERR_UNSUPPORTED,         /**< &nbsp; */
  WMS_CLIENT_ERR_GENERAL,             /**< &nbsp; */
  WMS_CLIENT_ERR_RPC,                 /**< &nbsp; */  

/** @cond */
  WMS_CLIENT_ERR_MAX,                 /* FOR INTERNAL USE OF CM ONLY! */
  WMS_CLIENT_ERR_MAX32 = 0x10000000   /* pad to 32 bit int */
/** @endcond */
} wms_client_err_e_type;

/** @ingroup common_data_types_group
@{ */
/** Command or request status messages.
*/
typedef enum
{
  WMS_OK_S                 = 0,    /**< &nbsp; */
  WMS_OUT_OF_RESOURCES_S,          /**< &nbsp; */
  WMS_TERMINAL_BLOCKED_S,          /**< &nbsp; */
  WMS_TERMINAL_BUSY_S,             /**< &nbsp; */
  WMS_INVALID_TRANSACTION_ID_S,    /**< &nbsp; */
  WMS_INVALID_FORMAT_S,            /**< &nbsp; */
  WMS_GENERAL_ERROR_S,             /**< &nbsp; */
  WMS_UNSUPPORTED_S,               /**< &nbsp; */ 
  WMS_NULL_PTR_S,                  /**< &nbsp; */

  WMS_INVALID_PARM_SIZE_S   = 100, /**< CDMA only. */
  WMS_INVALID_USER_DATA_SIZE_S,    /**< CDMA only. */
  WMS_INVALID_PARM_VALUE_S,        /**< CDMA only. */
  WMS_MISSING_PARM_S,              /**< CDMA only. */
  WMS_NETWORK_NOT_READY_S,         /**< CDMA only. */
  WMS_PHONE_NOT_READY_S,           /**< CDMA only. */
  WMS_NOT_ALLOWED_IN_AMPS_S,       /**< CDMA only. */
  WMS_NETWORK_FAILURE_S,           /**< CDMA only. */
  WMS_ACCESS_TOO_LARGE_S,          /**< CDMA only. */
  WMS_DTC_TOO_LARGE_S,             /**< CDMA only. */
  WMS_ACCESS_BLOCK_S,              /**< CDMA only. */
  WMS_ESN_MISMATCH_S,              /**< JCDMA2 feature only. */

  WMS_INVALID_TPDU_TYPE_S  = 200,  /**< GSM/WCDMA only. */
  WMS_INVALID_VALIDITY_FORMAT_S,   /**< GSM/WCDMA only. */
  WMS_INVALID_CB_DATA_S,           /**< GSM/WCDMA only. */

/** @cond */
  WMS_MT_MSG_FAILED_S,     /* internal use */
/** @endcond */

  WMS_SIP_PERM_ERROR_S     = 300,  /**< &nbsp; */
  WMS_SIP_TEMP_ERROR_S,            /**< &nbsp; */

  /* WMSC, CS and RPC errors  */
  WMS_WMSC_ERROR_S         = 400,  /**< &nbsp; */
  WMS_CS_ERROR_S,                  /**< &nbsp; */
  WMS_RPC_ERROR_S,                 /**< &nbsp; */

  /* Transport layer errors */
  WMS_TRANSPORT_NOT_READY_S = 500, /**< &nbsp; */
  WMS_TRANSPORT_NOT_ALLOWED_S,     /**< &nbsp; */

/** @cond */
  WMS_STATUS_MAX,
  WMS_STATUS_MAX32 = 0x10000000  /* pad to 32 bit int */
/** @endcond */
} wms_status_e_type;
/*~ SENTINEL wms_status_e_type.WMS_STATUS_MAX */

/** @ingroup common_data_types_group
@{ */
/** Information about a message report (SUBMIT-REPORT) received from the 
  network.
*/
typedef enum
{
  WMS_RPT_OK        = 0, /**< Success; submit_report_acknowledgment information 
                              is available for GSM/WCDMA. */
  WMS_RPT_LL_ERROR  = 1, /**< Lower Layer errors.\ For GSM/WCDMA, 
                              cause_value = LL Cause.\ This information is not 
                              available to clients. */
  WMS_RPT_GENERAL_ERROR = 2, /**< Any other error. */

  /* ------- CDMA specific failures -------- */
  WMS_RPT_OUT_OF_RESOURCES = 3,  /**< CDMA-specific failure. */
  WMS_RPT_NETWORK_NOT_READY = 4, /**< CDMA-specific failure. */
  WMS_RPT_PHONE_NOT_READY   = 5, /**< CDMA-specific failure. */
  WMS_RPT_NO_ACK            = 6, /**< CDMA Transport Layer acknowledgment was 
                                         not received. */
  WMS_RPT_CDMA_TL_ERROR     = 7, /**< CDMA Transport Layer error from the 
                                         network.\ Error_class and status in 
                                         cause_info are available. */
  WMS_RPT_ACCESS_TOO_LARGE  = 8, /**< CDMA message is too large to be sent 
                                         over the access channel. */
  WMS_RPT_DC_TOO_LARGE      = 9, /**< CDMA message is too large to be sent 
                                         over the dedicated channel. */
  WMS_RPT_NOT_ALLOWED_IN_AMPS = 10,   /**< CDMA-specific failure. */
  WMS_RPT_CANNOT_SEND_BROADCAST = 11, /**< CDMA-specific failure. */
  WMS_RPT_ACCESS_BLOCK       = 12,    /**< CDMA-specific failure. */

  /* ------- GSM/WCDMA specific failures -------- */
  WMS_RPT_RP_ERROR         = 100, 
   /**< GSM/WCDMA-specific failure: submit_report_error TPDU is available.\ 
           cause_value = RP Cause as defined in section 8.2.5.4 TS 24.011; 
           diagnostics might be available as defined in section 8.2.5.4 TS 24.011. */
  WMS_RPT_CP_ERROR         = 101, 
   /**< GSM/WCDMA-specific failure: no submit_report TPDU is available.\ 
           cause_value = CP Cause as defined in section 8.1.4.2 TS 24.011. */
  WMS_RPT_SMR_TR1M_TIMEOUT = 102, 
   /**< GSM/WCDMA-specific failure: the SMS entity's Timer 1M timed out in the 
           attempt to relay a MO message or notification to the network. */
  WMS_RPT_SMR_TR2M_TIMEOUT = 103, 
   /**< GSM/WCDMA-specific failure: the SMR entity's Timer 2M timed out in the 
           attempt to transfer an MT message. */
  WMS_RPT_SMR_NO_RESOURCES = 104, 
   /**< GSM/WCDMA-specific failure: SMR entity has no resources available.  */
  WMS_RPT_MO_CONTROL_DISALLOW = 105,
   /**< GSM/WCDMA-specific failure: MO SMS control disallows the MO message. */
  WMS_RPT_MO_CONTROL_ERROR = 106,    
   /**< GSM/WCDMA-specific failure: MO SMS control problem. */
  WMS_RPT_MO_RETRY_PERIOD_EXPIRED = 107,  
   /**< GSM/WCDMA-specific failure: MO retry period expired. */ 
  WMS_RPT_NO_RESPONSE_FROM_NETWORK = 108, 
   /**< No response from the GSM/WCDMA network. */

  /* ------ SIP specific errors ----- */
  WMS_RPT_SIP_PERM_ERROR   = 200, /**< &nbsp; */
  WMS_RPT_SIP_TEMP_ERROR   = 201, /**< &nbsp; */

/** @cond */
  WMS_RPT_MAX,
  WMS_RPT_MAX32 = 0x10000000
/** @endcond */
} wms_report_status_e_type;

/** @} */ /* end_ingroup common_data_types_group */

/** @ingroup common_data_types_group
@{ */
/** Includes all the information in an MO/MT message.
*/
typedef struct 
{
  wms_format_e_type               format;      /**< Format of the message type. */
  uint16                          data_len;    /**< Length of the message. */
  uint8                           data[WMS_MAX_LEN]; /**< Raw PDU SMS data. */
  wms_address_s_type              address;     /**< SMS destination address. */
} wms_ota_msg_s_type;

/** @ingroup common_data_types_group
@{ */
/** Supported SMS transport types.
*/
typedef enum 
{
  WMS_MSG_TRANSPORT_TYPE_SMS_OVER_IMS,  /**< &nbsp; */   
  WMS_MSG_TRANSPORT_TYPE_SMS_OVER_SGS,  /**< &nbsp; */
  WMS_MSG_TRANSPORT_TYPE_SMS_OVER_S102, /**< &nbsp; */   
  WMS_MSG_TRANSPORT_TYPE_FLOATING1,     /**< &nbsp; */
  WMS_MSG_TRANSPORT_TYPE_FLOATING2,     /**< &nbsp; */
  WMS_MSG_TRANSPORT_TYPE_INTERNAL,      /** Internal use only. */

/** @cond */
  WMS_MSG_TRANSPORT_TYPE_MAX,
  WMS_MSG_TRANSPORT_TYPE_MAX32 = 0x10000000
/** @endcond */
} wms_msg_transport_type_e_type;

/** @ingroup common_data_types_group
@{ */
/** SMS transport layer network registration status
*/
typedef enum {
   WMS_MSG_TRANSPORT_NW_REG_STATUS_NO_SRV,       /**< &nbsp; */
   WMS_MSG_TRANSPORT_NW_REG_STATUS_IN_PROGRESS,  /**< &nbsp; */
   WMS_MSG_TRANSPORT_NW_REG_STATUS_FAILED,       /**< &nbsp; */
   WMS_MSG_TRANSPORT_NW_REG_STATUS_LIMITED_SRV,  /**< &nbsp; */
   WMS_MSG_TRANSPORT_NW_REG_STATUS_FULL_SRV,     /**< &nbsp; */

/** @cond */
   WMS_MSG_TRANSPORT_NW_REG_MAX32 = 0x10000000
/** @endcond */
} wms_msg_transport_nw_reg_status_e_type;

/** @ingroup common_data_types_group
@{ */
/** SMS transport identifier assigned by the WMS when a transport is registered 
  with WMS.
*/
typedef uint8 wms_msg_transport_id_type;

/** @ingroup common_data_types_group
@{ */
/** Invalid Transport ID.
*/  
#define WMS_MSG_TRANSPORT_ID_INVALID          0xFF

/** @ingroup common_data_types_group
@{ */
/* SMS transport capability. */
#define  WMS_MSG_TRANSPORT_CAP_3GPP2_1X       0x000000100 
#define  WMS_MSG_TRANSPORT_CAP_3GPP2_EVDO     0x000000200 
#define  WMS_MSG_TRANSPORT_CAP_3GPP2_IMS      0x000000400 

#define  WMS_MSG_TRANSPORT_CAP_3GPP_CS        0x000010000 
#define  WMS_MSG_TRANSPORT_CAP_3GPP_PS        0x000020000 
#define  WMS_MSG_TRANSPORT_CAP_3GPP_IMS       0x000040000 
#define  WMS_MSG_TRANSPORT_CAP_3GPP_LTE       0x000080000 


/** @ingroup common_data_types_group
@{ */
/** SMS transport capability type.
*/
typedef uint32 wms_msg_transport_cap_type;  

/** @ingroup common_data_types_group
@{ */
/** Transport layer status
*/
typedef struct 
{
  wms_msg_transport_type_e_type            transport_type;   /**< Transport type. */
  wms_msg_transport_nw_reg_status_e_type   transport_status; /**< Transport status. */
  wms_msg_transport_cap_type               transport_cap;    /**< Transport capability. */
} wms_msg_transport_status_s_type;

/** @ingroup common_data_types_group
@{ */
/** Event data for the WMS_MSG_EVENT_TRANSPORT_REG event to notify WMS clients 
  about the registration/deregistration of a particular Transport Layer.
*/
typedef struct
{
   boolean                        is_registered;
     /**< Indicates whether the Transport Layer is registered. */
   wms_msg_transport_type_e_type  transport_type; /**< Transport type. */   
   wms_msg_transport_cap_type     transport_cap; /**< Transport capability. */   
} wms_msg_transport_reg_info_s_type;

/** @ingroup common_data_types_group
@{ */
/** Callback function type invoked by the WMS to send an SMS over a selected 
  registered Transport Layer.
*/
typedef boolean (*wms_msg_transport_mo_sms_cb_type) (
  wms_ota_msg_s_type *msg_ptr      /* The pointer to MO SMS message */
);
/*~ PARAM IN msg_ptr         POINTER */
/*~ CALLBACK wms_msg_transport_mo_sms_cb_type 
    ONERROR return FALSE */

/** @ingroup common_data_types_group
@{ */
/** SMS Transport Layer report indicator type.
*/
typedef enum
{
  WMS_MSG_TRANSPORT_MT_SMS_IND,
    /**< Mobile Terminated indication. */
  WMS_MSG_TRANSPORT_3GPP2_MO_SMS_STATUS_IND,
    /**< 3GPP2 Mobile Originated indication. */
  WMS_MSG_TRANSPORT_3GPP_MO_SMS_STATUS_IND,
    /**< 3GPP Mobile Originated indication. */ 

/** @cond */
  WMS_MSG_TRANSPORT_RPT_IND_MAX32 = 0x10000000
/** @endcond */
} wms_msg_transport_rpt_ind_e_type;

/** @ingroup common_data_types_group
@{ */
/** Data structure for the SMS Transport Layer to send a report indicator to 
  the WMS.
*/
typedef struct 
{ 
   wms_msg_transport_rpt_ind_e_type  ind_type;     /**< Transport report type. */
   wms_msg_transport_id_type         transport_id; /**< Transport ID. */ 
   wms_status_e_type                 mo_status;    /**< MO Status type. */
   wms_ota_msg_s_type                mt_sms;       /**< Over-the-air message. */
} wms_msg_transport_rpt_ind_s_type;

/** @} */ /* end_ingroup common_data_types_group */

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific teleservice IDs. 
  The values 0 through 0xFFFF are used in CDMA mode and Analog AWI SMS as defined 
  in IS-637/IS-41.

  @note For teleservice CM_91, the encoding type of the user data indicates 
        whether the teleservice is actually CPT, Voice Mail Notification (VMN), 
        or CMT. The user data is extracted into the corresponding bearer data 
        fields.
*/
typedef enum
{
  WMS_TELESERVICE_CMT_91             = 4096, /**< Embedded IS91 SMS. */
  WMS_TELESERVICE_CPT_95             = 4097, /**< Pager. */
  WMS_TELESERVICE_CMT_95             = 4098, /**< Short message. */
  WMS_TELESERVICE_VMN_95             = 4099, /**< Voice mail notification. */
  WMS_TELESERVICE_WAP                = 4100, /**< Wireless Application Protocol. */
  WMS_TELESERVICE_WEMT               = 4101, /**< Enhanced messaging/EMS. */
  WMS_TELESERVICE_SCPT               = 4102, /**< Service Category Programming. */
  WMS_TELESERVICE_CATPT              = 4103, /**< Card Application Toolkit 
                                                  Protocol. */

  WMS_TELESERVICE_GSM1x_01           = 4104, /**< GSM1x signaling message. */
  WMS_TELESERVICE_GSM1x_02           = 4105, /**< GSM1x short message. */

  /* GSM1x_03 thru _10 are reserved for now. */

  WMS_TELESERVICE_GSM1x_03           = 4106, /**< Reserved. */
  WMS_TELESERVICE_GSM1x_04           = 4107, /**< Reserved. */
  WMS_TELESERVICE_GSM1x_05           = 4108, /**< Reserved. */
  WMS_TELESERVICE_GSM1x_06           = 4109, /**< Reserved. */
  WMS_TELESERVICE_GSM1x_07           = 4110, /**< Reserved. */
  WMS_TELESERVICE_GSM1x_08           = 4111, /**< Reserved. */
  WMS_TELESERVICE_GSM1x_09           = 4112, /**< Reserved. */
  WMS_TELESERVICE_GSM1x_10           = 4113, /**< Reserved. */

  WMS_TELESERVICE_IMSST              = 4242, /**< IMS Services Teleservice. */

  WMS_TELESERVICE_IS91_PAGE          = 0x00010000,
    /**< Extension to standard teleservices: IS91 Paging in Analog mode. */
  WMS_TELESERVICE_IS91_VOICE_MAIL    = 0x00020000, 
    /**< Extension to standard teleservices: IS91 Voice Mail in Analog mode. */
  WMS_TELESERVICE_IS91_SHORT_MESSAGE = 0x00030000, 
    /**< Extension to standard teleservices: IS91 Short Message Service in 
         Analog mode. */
  WMS_TELESERVICE_MWI                = 0x00040000,
    /**< Extension to standard teleservices: Voice mail notification through 
         Message Waiting indication in CDMA or Analog mode. */
  WMS_TELESERVICE_BROADCAST          = 0x00050000,
    /**< Extension to standard teleservices: Broadcast SMS message. */
  WMS_TELESERVICE_UNKNOWN            = 0x0FFFFFFF   
    /**< Extension to standard teleservices: Invalid teleservice ID. */
} wms_teleservice_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific service categories.
*/
typedef enum
{
  WMS_SRV_UNKOWN  = 0,    /**< &nbsp; */
  WMS_SRV_EMERGENCY,      /**< &nbsp; */ 
  WMS_SRV_ADMIN,          /**< &nbsp; */
  WMS_SRV_MAINTENANCE,    /**< &nbsp; */
  WMS_SRV_GEN_NEWS_LOC,   /**< General news: local. */
  WMS_SRV_GEN_NEWS_REG,   /**< General news: regional. */
  WMS_SRV_GEN_NEWS_NAT,   /**< General news: national. */
  WMS_SRV_GEN_NEWS_INT,   /**< General news: international. */
  WMS_SRV_FIN_NEWS_LOC,   /**< Business/financial news: local. */
  WMS_SRV_FIN_NEWS_REG,   /**< Business/financial news: regional. */
  WMS_SRV_FIN_NEWS_NAT,   /**< Business/financial news: national. */
  WMS_SRV_FIN_NEWS_INT,   /**< Business/financial news: international. */
  WMS_SRV_SPT_NEWS_LOC,   /**< Sports news: local. */
  WMS_SRV_SPT_NEWS_REG,   /**< Sports news: regional. */
  WMS_SRV_SPT_NEWS_NAT,   /**< Sports news: national. */
  WMS_SRV_SPT_NEWS_INT,   /**< Sports news: international. */
  WMS_SRV_ENT_NEWS_LOC,   /**< Entertainment news: local. */
  WMS_SRV_ENT_NEWS_REG,   /**< Entertainment news: regional. */
  WMS_SRV_ENT_NEWS_NAT,   /**< Entertainment news: national. */
  WMS_SRV_ENT_NEWS_INT,   /**< Entertainment news: international. */
  WMS_SRV_LOC_WEATHER,    /**< &nbsp; */
  WMS_SRV_AREA_TRAFFIC,   /**< Area traffic reports. */
  WMS_SRV_AIRPORT_SCHED,  /**< Local airport flight schedules. */
  WMS_SRV_RESTAURANTS,    /**< &nbsp; */
  WMS_SRV_LODGINGS,       /**< &nbsp; */
  WMS_SRV_RETAILS,        /**< &nbsp; */
  WMS_SRV_ADS,            /**< &nbsp; */
  WMS_SRV_STOCK_QUOTES,   /**< &nbsp; */
  WMS_SRV_JOBS,           /**< Employment opportunities. */
  WMS_SRV_MEDICAL,        /**< &nbsp; */
  WMS_SRV_TECH_NEWS,      /**< Technology news. */
  WMS_SRV_MULTI,          /**< Multi-category. */

  WMS_SRV_CATPT,          /**< 0x20 Card Application Toolkit Protocol Teleservice. */ 
  WMS_SRV_CMAS_PRESIDENTIAL_ALERT  = 0x1000, /**< Presidential-level alert. */
  WMS_SRV_CMAS_EXTREME_ALERT       = 0x1001, /**< Extreme threat to life and 
                                                  property. */
  WMS_SRV_CMAS_SEVERE_ALERT        = 0x1002, /**< Severe threat to life and 
                                                  property. */
  WMS_SRV_CMAS_AMBER_ALERT         = 0x1003, /**< AMBER (child abduction 
                                                  emergency). */
  WMS_SRV_CMAS_TEST_MSG            = 0x1004, /**< Commercial Mobile Alert System 
                                                  test message. */
/** @cond */
  WMS_SRV_MAX32           = 0x10000000
/** @endcond */
} wms_service_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific error classes.
*/
typedef enum
{
  WMS_ERROR_NONE        = 0, /**< &nbsp; */
  WMS_ERROR_RESERVED_1  = 1, /**< &nbsp; */
  WMS_ERROR_TEMP        = 2, /**< &nbsp; */
  WMS_ERROR_PERM        = 3, /**< &nbsp; */
/** @cond */
  WMS_ERROR_MAX32       = 0x10000000
/** @endcond */
} wms_error_class_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific status codes.
  The first half of the enumerators are from IS-41D SMS cause codes with the 
  exact binary values as defined in IS-41D.\ They are in the range of 0x00 to 
  0xFF.  

  The status codes can be grouped as follows:
  - All values within the range of 6 to 31 are treated as 
    WMS_TL_OTHER_NETWORK_PROBLEM_S.
  - All values within the range of 40 to 47 are treated as 
    WMS_TL_OTHER_TERMINAL_PROBLEM_S.
  - All values within the range of 67 to 95 are treated as 
    WMS_TL_OTHER_RADIO_IF_PROBLEM_S.
  - All values within the range of 108 to 255 are treated as 
    WMS_TL_OTHER_GENERAL_PROBLEMS_S.
*/
typedef enum
{
  /* A. Network Problems:  */
  WMS_TL_ADDRESS_VACANT_S                     = 0, /**< Network problem. */
  WMS_TL_ADDRESS_TRANSLATION_FAILURE_S,            /**< Network problem. */
  WMS_TL_NETWORK_RESOURCE_SHORTAGE_S,              /**< Network problem. */
  WMS_TL_NETWORK_FAILURE_S,                        /**< Network problem. */
  WMS_TL_INVALID_TELESERVICE_ID_S,                 /**< Network problem. */
  WMS_TL_OTHER_NETWORK_PROBLEM_S,                  /**< Other network problem. */
  WMS_TL_OTHER_NETWORK_PROBLEM_MORE_FIRST_S   = 6, /**< First of other network 
												        problems. */

  /* All values within the range of 6 to 31 are treated as 
     WMS_TL_OTHER_NETWORK_PROBLEM_S. */

  WMS_TL_OTHER_NETWORK_PROBLEM_MORE_LAST_S    = 31, /**< Last of other network 
													     problems. */

  /* B. Terminal Problems:  */
  WMS_TL_NO_PAGE_RESPONSE_S                   = 32, /**< Terminal problem. */
  WMS_TL_DESTINATION_BUSY_S,                        /**< Terminal problem. */
  WMS_TL_NO_ACK_S,                                  /**< Terminal problem. */
  WMS_TL_DESTINATION_RESOURCE_SHORTAGE_S,           /**< Terminal problem. */
  WMS_TL_SMS_DELIVERY_POSTPONED_S,                  /**< Terminal problem. */
  WMS_TL_DESTINATION_OUT_OF_SERVICE_S,              /**< Terminal problem. */
  WMS_TL_DESTINATION_NO_LONGER_AT_THIS_ADDRESS_S,   /**< Terminal problem. */
  WMS_TL_OTHER_TERMINAL_PROBLEM_S,                  /**< Other terminal problem. */
  WMS_TL_OTHER_TERMINAL_PROBLEM_MORE_FIRST_S  = 40, /**< First of other terminal 
                                                         problems. */

  /* All values within the range of 40 to 47 are treated as 
     WMS_TL_OTHER_TERMINAL_PROBLEM_S. */

  WMS_TL_OTHER_TERMINAL_PROBLEM_MORE_LAST_S   = 47, /**< Last of other terminal 
                                                         problems. */
  WMS_TL_SMS_DELIVERY_POSTPONED_MORE_FIRST_S  = 48, /**< First of SMS delivery 
                                                         postponed problems. */
  WMS_TL_SMS_DELIVERY_POSTPONED_MORE_LAST_S   = 63, /**< Last of SMS delivery 
                                                         postponed problems. */

  /* C. Radio Interface Problems:  */
  WMS_TL_RADIO_IF_RESOURCE_SHORTAGE_S         = 64, /**< Radio interface problem. */
  WMS_TL_RADIO_IF_INCOMPATIBLE_S,                   /**< Radio interface problem. */
  WMS_TL_OTHER_RADIO_IF_PROBLEM_S,                  /**< Other radio interface 
													     problem. */
  WMS_TL_OTHER_RADIO_IF_PROBLEM_MORE_FIRST_S  = 67, /**< First of other radio 
                                                         interface problems. */

  /* All values within the range of 67 to 95 are treated as 
     WMS_TL_OTHER_RADIO_IF_PROBLEM_S. */

  WMS_TL_OTHER_RADIO_IF_PROBLEM_MORE_LAST_S   = 95, /**< Last of other radio 
                                                         interface problems. */

  /* D. General Problems:  */
  WMS_TL_UNEXPECTED_PARM_SIZE_S               = 96,  /**< General problem. */
  WMS_TL_SMS_ORIGINATION_DENIED_S,                   /**< General problem. */         
  WMS_TL_SMS_TERMINATION_DENIED_S,                   /**< General problem. */
  WMS_TL_SUPPL_SERVICE_NOT_SUPPORTED,                /**< General problem. */
  WMS_TL_SMS_NOT_SUPPORTED_S,                        /**< General problem. */
  WMS_TL_RESERVED_101_S,                             /**< General problem. */
  WMS_TL_MISSING_EXPECTED_PARM_S,                    /**< General problem. */
  WMS_TL_MISSING_MANDATORY_PARM_S,                   /**< General problem. */
  WMS_TL_UNRECOGNIZED_PARM_VALUE_S,                  /**< General problem. */
  WMS_TL_UNEXPECTED_PARM_VALUE_S,                    /**< General problem. */
  WMS_TL_USER_DATA_SIZE_ERROR_S,                     /**< General problem. */
  WMS_TL_OTHER_GENERAL_PROBLEMS_S,                   /**< Other general problem. */
  WMS_TL_OTHER_GENERAL_PROBLEMS_MORE_FIRST_S  = 108, /**< First of other general 
                                                          problems. */

  /* All values within the range of 108 to 255 are treated as 
     WMS_TL_OTHER_GENERAL_PROBLEMS_S. */

  WMS_TL_OTHER_GENERAL_PROBLEMS_MORE_LAST_S   = 255, /**< Last of other general 
                                                          problems. */
/** @cond */
  WMS_TL_MAX32                                = 0x10000000
/** @endcond */
} wms_cdma_tl_status_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific message types.
*/
typedef enum
{
  WMS_BD_TYPE_RESERVED_0 = 0, /**< &nbsp; */
  WMS_BD_TYPE_DELIVER,        /**< MT only. */
  WMS_BD_TYPE_SUBMIT,         /**< MO only. */
  WMS_BD_TYPE_CANCELLATION,   /**< MO only. */
  WMS_BD_TYPE_DELIVERY_ACK,   /**< MT only. */
  WMS_BD_TYPE_USER_ACK,       /**< MT and MO. */
  WMS_BD_TYPE_READ_ACK,       /**< MT and MO. */
/** @cond */
  WMS_BD_TYPE_MAX32          = 0x10000000
/** @endcond */
} wms_bd_message_type_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific message types.
*/
#define WMS_IS_MO_CDMA_MSG_TYPE( type )         \
          ( (type) == WMS_BD_TYPE_SUBMIT ||       \
            (type) == WMS_BD_TYPE_CANCELLATION || \
            (type) == WMS_BD_TYPE_USER_ACK ||     \
            (type) == WMS_BD_TYPE_READ_ACK )

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific message ID.
*/
typedef struct wms_message_id_s
{
  wms_bd_message_type_e_type   type;        /**< CDMA-specific message type. */
  wms_message_number_type      id_number;   /**< Message ID of the bearer data. */
  boolean                      udh_present;
    /**< Indicates whether UDH is present.\ If FEATURE_SMS_UDH is not defined, 
         ignore udh_present. */
} wms_message_id_s_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific user data encoding types.
*/
typedef enum
{
  WMS_ENCODING_OCTET        = 0,    /**< 8 bit. */
  WMS_ENCODING_IS91EP,              /**< Varies. */
  WMS_ENCODING_ASCII,               /**< 7 bit. */
  WMS_ENCODING_IA5,                 /**< 7 bit. */
  WMS_ENCODING_UNICODE,             /**< 16 bit. */
  WMS_ENCODING_SHIFT_JIS,           /**< 8 or 16 bit. */
  WMS_ENCODING_KOREAN,              /**< 8 or 16 bit. */
  WMS_ENCODING_LATIN_HEBREW,        /**< 8 bit. */
  WMS_ENCODING_LATIN,               /**< 8 bit. */
  WMS_ENCODING_GSM_7_BIT_DEFAULT,   /**< 7 bit. */
/** @cond */
  WMS_ENCODING_MAX32        = 0x10000000
/** @endcond */
} wms_user_data_encoding_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific IS-91 EP data types.
*/
typedef enum
{
  WMS_IS91EP_VOICE_MAIL         = 0x82,      /**< &nbsp; */
  WMS_IS91EP_SHORT_MESSAGE_FULL = 0x83,      /**< &nbsp; */ 
  WMS_IS91EP_CLI_ORDER          = 0x84,      /**< Calling Line Identification. */ 
  WMS_IS91EP_SHORT_MESSAGE      = 0x85,      /**< &nbsp; */ 
/** @cond */
  WMS_IS91EP_MAX32              = 0x10000000 /**< &nbsp; */
/** @endcond */
} wms_IS91EP_type_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific priorities for sending a message.
*/
typedef enum
{
  WMS_PRIORITY_NORMAL      = 0,         /**< &nbsp; */
  WMS_PRIORITY_INTERACTIVE,             /**< &nbsp; */
  WMS_PRIORITY_URGENT,                  /**< &nbsp; */
  WMS_PRIORITY_EMERGENCY,               /**< &nbsp; */
/** @cond */
  WMS_PRIORITY_MAX32       = 0x10000000 /**< &nbsp; */
/** @endcond */
} wms_priority_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific privacy types for sending a message.
*/
typedef enum
{
  WMS_PRIVACY_NORMAL      = 0,         /**< &nbsp; */
  WMS_PRIVACY_RESTRICTED,              /**< &nbsp; */
  WMS_PRIVACY_CONFIDENTIAL,            /**< &nbsp; */
  WMS_PRIVACY_SECRET,                  /**< &nbsp; */
/** @cond */
  WMS_PRIVACY_MAX32       = 0x10000000 /**< &nbsp; */
/** @endcond */
} wms_privacy_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific display modes for when to display a message.
*/
typedef enum
{
  WMS_DISPLAY_MODE_IMMEDIATE   = 0, /**< &nbsp; */
  WMS_DISPLAY_MODE_DEFAULT     = 1, /**< &nbsp; */
  WMS_DISPLAY_MODE_USER_INVOKE = 2, /**< &nbsp; */
  WMS_DISPLAY_MODE_RESERVED    = 3  /**< &nbsp; */
} wms_display_mode_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific alert modes.\ For pre-IS637A implementations, alert_mode has 
  values of TRUE or FALSE only. 
*/
typedef enum
{
  WMS_ALERT_MODE_DEFAULT         = 0, /**< &nbsp; */
  WMS_ALERT_MODE_LOW_PRIORITY    = 1, /**< &nbsp; */
  WMS_ALERT_MODE_MEDIUM_PRIORITY = 2, /**< &nbsp; */
  WMS_ALERT_MODE_HIGH_PRIORITY   = 3, /**< &nbsp; */

  WMS_ALERT_MODE_OFF   = 0,           /**< &nbsp; */
  WMS_ALERT_MODE_ON    = 1            /**< &nbsp; */
} wms_alert_mode_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific download modes.
*/
typedef enum
{
  WMS_DOWNLOAD_MODE_NONE         = 0,    /**< &nbsp; */
  WMS_DOWNLOAD_MODE_PP_VAS       = 0x20, /**< Download Value Added Services 
                                              through SMS Point-to-Point. */
  WMS_DOWNLOAD_MODE_PP_PRL       = 0x30, /**< Download the Preferred Roaming 
                                              List through SMS Point-to-Point. */
  WMS_DOWNLOAD_MODE_UPDATE_PRL   = 0x10  /**< Update the Preferred Roaming List. */
} wms_download_mode_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific delivery status types (IS-637B parameters/fields). 
*/
typedef enum
{
  /* ERROR_CLASS_NONE */
  WMS_DELIVERY_STATUS_ACCEPTED              = 0, /**< No error class. */ 
  WMS_DELIVERY_STATUS_DEPOSITED_TO_INTERNET = 1, /**< No error class. */ 
  WMS_DELIVERY_STATUS_DELIVERED             = 2, /**< No error class. */ 
  WMS_DELIVERY_STATUS_CANCELLED             = 3, /**< No error class. */

  /* ERROR_CLASS_TEMP & PERM */ 
  WMS_DELIVERY_STATUS_NETWORK_CONGESTION  = 4,   /**< Temporary and permanent 
                                                      error classes. */ 
  WMS_DELIVERY_STATUS_NETWORK_ERROR       = 5,   /**< Temporary and permanent 
                                                      error classes. */ 

  /* ERROR_CLASS_PERM */
  WMS_DELIVERY_STATUS_CANCEL_FAILED       = 6,   /**< Permanent error class. */ 
  WMS_DELIVERY_STATUS_BLOCKED_DESTINATION = 7,   /**< Permanent error class. */ 
  WMS_DELIVERY_STATUS_TEXT_TOO_LONG       = 8,   /**< Permanent error class. */ 
  WMS_DELIVERY_STATUS_DUPLICATE_MESSAGE   = 9,   /**< Permanent error class. */ 
  WMS_DELIVERY_STATUS_INVALID_DESTINATION = 10,  /**< Permanent error class. */ 
  WMS_DELIVERY_STATUS_MESSAGE_EXPIRED     = 13,  /**< Permanent error class. */ 
  WMS_DELIVERY_STATUS_UNKNOWN_ERROR       = 0x1F /**< Permanent error class. */ 

/* All the other values are reserved */

} wms_delivery_status_e_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific type definition.
*/
typedef struct wms_delivery_status_s
{
  wms_error_class_e_type       error_class; /**< Type of error (permanent 
                                                 or temporary). */
  wms_delivery_status_e_type   status;      /**< Status returned for message 
                                                 delivery. */
} wms_delivery_status_s_type;

/** @ingroup cdma_specific_type_defs_group 
@{ */
/** CDMA-specific Dedicated Channel (DC) Service Options (SO).
*/
typedef enum
{
  WMS_DC_SO_AUTO = 0,  /**< &nbsp; */
  WMS_DC_SO_6    = 6,  /**< &nbsp; */
  WMS_DC_SO_14   = 14, /**< &nbsp; */
/** @cond */
  WMS_DC_SO_MAX32   = 0x10000000      /* pad to 32 bit int*/
/** @endcond */
} wms_dc_so_e_type;

/** @} */ /* end_ingroup cdma_specific_type_defs_group */

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA-specific TPDU types.
*/
typedef enum
{
  WMS_TPDU_DELIVER = 0,          /**< Deliver message TPDU type; from SC to MS. */
  WMS_TPDU_DELIVER_REPORT_ACK,   /**< Report acknowledgment of delivery TPDU 
                                      type; from MS to SC. */
  WMS_TPDU_DELIVER_REPORT_ERROR, /**< Report error in delivery TPDU type; from 
                                      MS to SC. */
  WMS_TPDU_SUBMIT,               /**< From MS to SC. */
  WMS_TPDU_SUBMIT_REPORT_ACK,    /**< Report acknowledgment of submission TPDU 
                                      type; from SC to MS. */
  WMS_TPDU_SUBMIT_REPORT_ERROR,  /**< Report error in submission TPDU type; from 
                                      SC to MS. */
  WMS_TPDU_STATUS_REPORT,        /**< From SC to MS. */
  WMS_TPDU_COMMAND,              /**< From MS to SC. */
  WMS_TPDU_RESERVED,             /**< From SC to MS.  */

/** @cond */
  WMS_TPDU_MAX,
  WMS_TPDU_NONE,                 /**< &nbsp; */
  WMS_TPDU_MAX32 = 0x10000000
/** @endcond */
} wms_gw_tpdu_type_e_type;
/*~ SENTINEL wms_gw_tpdu_type_e_type.WMS_TPDU_MAX */

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA-specific TPDU types.
*/
#define WMS_IS_MO_TPDU_TYPE( type )      \
          ( (type) == WMS_TPDU_SUBMIT ||   \
            (type) == WMS_TPDU_COMMAND )

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA-specific Transport Protocol (TP) failure-cause types.\ 
  See 3GPP TS 23.040, section 9.2.3.22.
  Reserved address blocks: 
  - 0x00 through 0x7f
  - 0x83 through 0x8e
  - 0x92 through 0x9e
  - 0xa2 through 0xae
  - 0xb1 through 0xbf
  - 0xc8 through 0xcf
  - 0xd6 through 0xdf
  Values from 0xe0 through 0xfe are for errors specific to an application.
*/
typedef enum
{
  /* values from 0x00 to 0x7f are reserved */

  WMS_TP_CAUSE_TELEMATIC_NOT_SUPPORTED    = 0x80,
     /**< TP-PID: Mobile Originated Protocol Identifier Data (PID) error. */
  WMS_TP_CAUSE_SM_TYPE0_NOT_SUPPORTED     = 0x81, /**< TP-PID: MO and MT PID 
                                                       error. */
  WMS_TP_CAUSE_CANNOT_REPLACE_SM          = 0x82, /**< TP-PID: MO and MT PID 
                                                       error. */
  /* ... values from 0x83 to 0x8e are reserved .. */

  WMS_TP_CAUSE_UNSPECIFIED_PID_ERROR      = 0x8f, /**< TP-PID: MO and MT PID 
                                                       error. */
  WMS_TP_CAUSE_ALPHABET_NOT_SUPPORTED     = 0x90, /**< TP-DCS: MO DCS error. */
  WMS_TP_CAUSE_MSG_CLASS_NOT_SUPPORTED    = 0x91, /**< TP-DCS: MT DCS error. */

  /* ... values from 0x92 to 0x9e are reserved .. */

  WMS_TP_CAUSE_UNSPECIFIED_DCS_ERROR      = 0x9f, /**< TP-DCS: MO and MT DCS 
                                                       error. */
  WMS_TP_CAUSE_CANNOT_PERFORM_COMMAND     = 0xa0, /**< TP-Command: MO command 
                                                       error. */
  WMS_TP_CAUSE_COMMAND_NOT_SUPPORTED      = 0xa1, /**< TP-Command: MO command 
                                                       error. */
  /* ... values from 0xa2 to 0xae are reserved .. */

  WMS_TP_CAUSE_UNSPECIFIED_CMD_ERROR      = 0xaf, /**< TP-Command: MO command 
                                                       error. */
  WMS_TP_CAUSE_TPDU_NOT_SUPPORTED         = 0xb0, /**< TP-Command: MO and MT 
                                                       command error. */
  /* ... values from 0xb1 to 0xbf are reserved .. */

  WMS_TP_CAUSE_SC_BUSY                    = 0xc0, /**< TP-Command: MO command 
                                                       error. */
  WMS_TP_CAUSE_NO_SC_SUBSCRIPTION         = 0xc1, /**< TP-Command: MO command 
                                                       error. */
  WMS_TP_CAUSE_SC_FAILURE                 = 0xc2, /**< TP-Command: MO command 
                                                       error. */
  WMS_TP_CAUSE_INVALID_SME_ADDRESS        = 0xc3,/**< TP-Command: MO command 
                                                       error. */
  WMS_TP_CAUSE_DESTINATION_SME_BARRED     = 0xc4, /**< TP-Command: MO command 
                                                       error. */
  WMS_TP_CAUSE_REJECTED_DUPLICATE_SM      = 0xc5, /**< TP-Command: MO command 
                                                       error. */
  WMS_TP_CAUSE_VPF_NOT_SUPPORTED          = 0xc6, /**< TP-Command: MO command 
                                                       error, Validity Period 
                                                       Format. */
  WMS_TP_CAUSE_VP_NOT_SUPPORTED           = 0xc7, /**< TP-Command: MO command 
                                                       error, Validity Period. */
  /* values from 0xc8 to 0xcf are reserved */

  WMS_TP_CAUSE_SIM_FULL                   = 0xd0, /**< TP-Command: MT command 
                                                       error. */
  WMS_TP_CAUSE_NO_SM_STORAGE_IN_SIM       = 0xd1, /**< TP-Command: MT command 
                                                       error. */
  WMS_TP_CAUSE_ERROR_IN_MS                = 0xd2, /**< TP-Command: MT command 
                                                       error. */
  WMS_TP_CAUSE_MEMORY_FULL                = 0xd3, /**< TP-Command: MT command 
                                                       error. */
  WMS_TP_CAUSE_SAT_BUSY                   = 0xd4, /**< TP-Command: MT command 
                                                       error. */
  WMS_TP_CAUSE_SIM_DATA_DOWNLOAD_ERROR    = 0xd5, /**< TP-Command: MT command 
                                                       error. */
  /* values from 0xd6 to 0xdf are reserved */
  /* values from 0xe0 to 0xfe are for errors specific to an application */

  WMS_TP_CAUSE_UNSPECIFIED_ERROR          = 0xff /**< TP-Command: MO and MT 
                                                      command error. */
} wms_tp_cause_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA-specific Relay Protocol (RP) cause types.\ See TS 24.011, section 
  8.2.5.4.

  Error classifications:
  - All MO errors other than those listed below shall be treated as 
    WMS_RP_CAUSE_TEMPORARY_FAILURE.
  - All MT errors other than those listed below shall be treated as 
    WMS_RP_CAUSE_PROTOCOL_ERROR_UNSPECIFIED.
  - All SMMA errors other than those listed below shall be treated as 
    WMS_RP_CAUSE_TEMPORARY_FAILURE. 
  - All CP errors other than those listed below shall be treated as 
    WMS_RP_CAUSE_PROTOCOL_ERROR_UNSPECIFIED.
*/
typedef enum
{
  WMS_RP_CAUSE_UNASSIGNED_NUMBER                  = 1,  /**< MO error. */
  WMS_RP_CAUSE_OPERATOR_DETERMINED_BARRING        = 8,  /**< MO error. */
  WMS_RP_CAUSE_CALL_BARRED                        = 10, /**< MO error. */
  WMS_RP_CAUSE_RESERVED_11                        = 11, /**< MO error. */
  WMS_RP_CAUSE_NETWORK_FAILURE                    = 17, /**< CP error. */
  WMS_RP_CAUSE_SM_REJECTED                        = 21, /**< MO error. */
  WMS_RP_CAUSE_MEMORY_EXCEEDED                    = 22, /**< MT and CP error. */
  WMS_RP_CAUSE_DESTINATION_OUT_OF_ORDER           = 27, /**< MO error. */
  WMS_RP_CAUSE_UNIDENTIFIED_SUBSCRIBER            = 28, /**< MO error. */
  WMS_RP_CAUSE_FACILITY_REJECTED                  = 29, /**< MO error. */
  WMS_RP_CAUSE_UNKNOWN_SUBSCRIBER                 = 30, /**< MO and SMMA error. */
  WMS_RP_CAUSE_NETWORK_OUT_OF_ORDER               = 38, /**< MO and SMMA error. */
  WMS_RP_CAUSE_TEMPORARY_FAILURE                  = 41, /**< MO and SMMA error. */
  WMS_RP_CAUSE_CONGESTION                         = 42, /**< MO and SMMA error. */
  WMS_RP_CAUSE_RESOURCES_UNAVAILABLE_UNSPECIFIED  = 47, /**< MO and SMMA error. */
  WMS_RP_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED  = 50, /**< MO error. */
  WMS_RP_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED = 69, /**< MO and SMMA error. */
  WMS_RP_CAUSE_INVALID_SM_TRANSFER_REF_VALUE      = 81, /**< MO/MT/CP error. */
  WMS_RP_CAUSE_SEMANTICALLY_INCORRECT_MESSAGE     = 95, /**< MO/MT/SMMA/CP error. */
  WMS_RP_CAUSE_INVALID_MANDATORY_INFORMATION      = 96, /**< MO/MT/SMMA/CP error. */
  WMS_RP_CAUSE_MESSAGE_TYPE_NON_EXISTENT          = 97, /**< MO/MT/SMMA/CP error. */
  WMS_RP_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE        = 98, /**< MO/MT/SMMA/CP error. */
  WMS_RP_CAUSE_IE_NON_EXISTENT_OR_NOT_IMPLEMENTED = 99, /**< MO/MT/SMMA/CP error. */
  WMS_RP_CAUSE_PROTOCOL_ERROR_UNSPECIFIED         = 111,/**< MO/MT/SMMA/CP error. */
  WMS_RP_CAUSE_INTERWORKING_UNSPECIFIED           = 127 /**< MO and SMMA error. */
} wms_rp_cause_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA-specific Protocol Identifier Data (PID).\ See 3GPP TS 23.040, 
  section 9.2.3.9. 

  WMS_PID_SC_SPECIFIC_1 through WMS_PID_SC_SPECIFIC_7 can hold values specific to 
  each SC. The usage is based on mutual agreement between the SME and the SC. 
  Seven combinations are available for each SC.
*/
typedef enum
{
  /* values from 0x00 to 0x1f are for SM-AL protocols */
  WMS_PID_DEFAULT           = 0x00, /**< SM-AL protocols: default value. */

  /* values from 0x20 to 0x3f are for telematic interworking */
  WMS_PID_IMPLICIT          = 0x20,
    /**< Telematic interworking: device type is specific to this SC, or it can 
         be concluded on the basis of the address. */
  WMS_PID_TELEX             = 0x21, /**< Telematic interworking. */
  WMS_PID_G3_FAX            = 0x22, /**< Telematic interworking: Group 3 telefax. */
  WMS_PID_G4_FAX            = 0x23, /**< Telematic interworking: Group 4 telefax. */
  WMS_PID_VOICE_PHONE       = 0x24, /**< Telematic interworking. */
  WMS_PID_ERMES             = 0x25, /**< Telematic interworking: European Radio 
                                         Messaging System. */
  WMS_PID_NAT_PAGING        = 0x26, /**< Telematic interworking: National Paging 
                                         system (known to the SC). */
  WMS_PID_VIDEOTEX          = 0x27, /**< Telematic interworking: Videotex (T.100 
                                         [20] /T.101 [21]). */
  WMS_PID_TELETEX_UNSPEC    = 0x28, /**< Telematic interworking. */
  WMS_PID_TELETEX_PSPDN     = 0x29, /**< Telematic interworking: Packet-Switched 
                                         Packet Data Network. */
  WMS_PID_TELETEX_CSPDN     = 0x2a, /**< Telematic interworking: Circuit-Switched 
                                         Packet Data Network.*/
  WMS_PID_TELETEX_PSTN      = 0x2b, /**< Telematic interworking: analog Public 
                                         Switched Telephone Network. */
  WMS_PID_TELETEX_ISDN      = 0x2c, /**< Telematic interworking: digital 
                                         Integrated Services Digital Network. */
  WMS_PID_UCI               = 0x2d, /**< Telematic interworking: Universal 
                                         Computer Interface, ETSI DE/PS 3 01 3.*/
  WMS_PID_RESERVED_0x2e     = 0x2e, /**< Telematic interworking. */
  WMS_PID_RESERVED_0x2f     = 0x2f, /**< Telematic interworking.*/
  WMS_PID_MSG_HANDLING      = 0x30, /**< Telematic interworking. */
  WMS_PID_X400              = 0x31, /**< Telematic interworking: any public X.400-
                                         based message handling system. */
  WMS_PID_INTERNET_EMAIL    = 0x32, /**< Telematic interworking. */
  WMS_PID_RESERVED_0x33     = 0x33, /**< Telematic interworking. */
  WMS_PID_RESERVED_0x34     = 0x34, /**< Telematic interworking. */
  WMS_PID_RESERVED_0x35     = 0x35, /**< Telematic interworking. */
  WMS_PID_RESERVED_0x36     = 0x36, /**< Telematic interworking. */
  WMS_PID_RESERVED_0x37     = 0x37, /**< Telematic interworking. */
  WMS_PID_SC_SPECIFIC_1     = 0x38, /**< Telematic interworking. */
  WMS_PID_SC_SPECIFIC_2     = 0x39, /**< Telematic interworking. */
  WMS_PID_SC_SPECIFIC_3     = 0x3a, /**< Telematic interworking. */
  WMS_PID_SC_SPECIFIC_4     = 0x3b, /**< Telematic interworking. */
  WMS_PID_SC_SPECIFIC_5     = 0x3c, /**< Telematic interworking. */
  WMS_PID_SC_SPECIFIC_6     = 0x3d, /**< Telematic interworking. */
  WMS_PID_SC_SPECIFIC_7     = 0x3e, /**< Telematic interworking. */
  WMS_PID_GSM_UMTS          = 0x3f, /**< Telematic interworking. */

  /* values from 0x40 to 0x7f: */
  WMS_PID_SM_TYPE_0         = 0x40, /**< Short Message type. */
  WMS_PID_REPLACE_SM_1      = 0x41, /**< Replace Short Message type. */
  WMS_PID_REPLACE_SM_2      = 0x42, /**< &nbsp; */
  WMS_PID_REPLACE_SM_3      = 0x43, /**< &nbsp; */
  WMS_PID_REPLACE_SM_4      = 0x44, /**< &nbsp; */
  WMS_PID_REPLACE_SM_5      = 0x45, /**< &nbsp; */
  WMS_PID_REPLACE_SM_6      = 0x46, /**< &nbsp; */
  WMS_PID_REPLACE_SM_7      = 0x47, /**< &nbsp; */
  /* ... values reserved not listed ... */
  WMS_PID_EMS               = 0x5e, /**< Enhanced Message Service (Obsolete).*/
  WMS_PID_RETURN_CALL       = 0x5f, /**< &nbsp; */
  /* ... values reserved not listed ... */
  WMS_PID_ANSI136_R_DATA    = 0x7c, /**< &nbsp; */
  WMS_PID_ME_DATA_DOWNLOAD  = 0x7d, /**< &nbsp; */
  WMS_PID_ME_DEPERSONALIZE  = 0x7e, /**< &nbsp; */
  WMS_PID_SIM_DATA_DOWNLOAD = 0x7f, /**< &nbsp; */

/** @cond */
  WMS_PID_E_MAX32 = 0x10000000   /* pad to 32 bit int */
/** @endcond */

  /* values from 0x80 to 0xbf are reserved */
  /* values from 0xc0 to 0xff are for SC specific use */
} wms_pid_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA-specific alphabet sets.
*/
typedef enum
{
  WMS_GW_ALPHABET_7_BIT_DEFAULT, /**< GSM/WCDMA default. */
  WMS_GW_ALPHABET_8_BIT,         /**< &nbsp; */
  WMS_GW_ALPHABET_UCS2,          /**< Universal Character Set 16-bit variant. */
/** @cond */
  WMS_GW_ALPHABET_MAX32 = 0x10000000   /* pad to 32 bit int */
/** @endcond */
} wms_gw_alphabet_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA-specific message waiting actions.
*/
typedef enum
{
  WMS_GW_MSG_WAITING_NONE,      /**< No messages waiting. */
  WMS_GW_MSG_WAITING_DISCARD,   /**< Discard message after updating indication. */
  WMS_GW_MSG_WAITING_STORE,     /**< Store message after updating indication. */
  WMS_GW_MSG_WAITING_NONE_1111, /**< Number of messages waiting. */
/** @cond */
  WMS_GW_MSG_WAITING_MAX32 = 0x10000000 /* pad to 32 bit int */
/** @endcond */
} wms_gw_msg_waiting_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA-specific message waiting types.
*/
typedef enum
{
  WMS_GW_MSG_WAITING_VOICEMAIL, /**< &nbsp; */
  WMS_GW_MSG_WAITING_FAX,       /**< &nbsp; */
  WMS_GW_MSG_WAITING_EMAIL,     /**< &nbsp; */
  WMS_GW_MSG_WAITING_OTHER,     /**< Extended Message Type Waiting (equivalent to 
                                     Other in 3GPP TS 23.038 [9]). */
  WMS_GW_MSG_WAITING_VIDEOMAIL, /**< &nbsp; */
/** @cond */
  WMS_GW_MSG_WAITING_KIND_MAX32 = 0x10000000   /* pad to 32 bit int */
/** @endcond */
} wms_gw_msg_waiting_kind_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** Message Waiting Indication (MWI) information.
*/
typedef struct 
{
  wms_gw_msg_waiting_kind_e_type  type;      /**< Message indication type. */
  boolean                         is_active; /**< Set indication active/inactive. */
  uint32                          count;     /**< Message count. */
} wms_gw_mwi_info_s_type ;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** SSM/WCDMA-specific MS User Data Header (UDH) IDs.

  Reserved address blocks:
  - 15 through 1F: Reserved for future EMS.
  - 21 through 23: Reserved for future use.
  - 26 through 6F: Reserved for future use.
  - 70 through 7f: Reserved for (U)SIM toolkit security headers.
  - 80 through 9F: SME to SME specific use.
  - A0 through BF: Reserved for future use.
  - C0 through DF: SC-specific use.
  - E0 through FF: Reserved for future use.
*/
typedef enum
{
  WMS_UDH_CONCAT_8         = 0x00, /**< Concatenated short messages, 8-bit 
                                        reference number. */
  WMS_UDH_SPECIAL_SM,              /**< Special SMS message indication. */

  /* 02 - 03    Reserved */

  WMS_UDH_PORT_8           = 0x04, /**< Application port addressing scheme, 
                                        8-bit address. */
  WMS_UDH_PORT_16,                 /**< Application port addressing scheme, 
                                        16-bit address. */
  WMS_UDH_SMSC_CONTROL,            /**< &nbsp; */
  WMS_UDH_SOURCE,                  /**< &nbsp; */
  WMS_UDH_CONCAT_16,               /**< Concatenated short message, 16-bit 
                                        reference number. */
  WMS_UDH_WCMP,                    /**< Wireless Control Message Protocol. */
  WMS_UDH_TEXT_FORMATING,          /**< &nbsp; */
  WMS_UDH_PRE_DEF_SOUND,           /**< &nbsp; */
  WMS_UDH_USER_DEF_SOUND,          /**< &nbsp; */
  WMS_UDH_PRE_DEF_ANIM,            /**< &nbsp; */
  WMS_UDH_LARGE_ANIM,              /**< &nbsp;. */
  WMS_UDH_SMALL_ANIM,              /**< &nbsp; */
  WMS_UDH_LARGE_PICTURE,           /**< &nbsp; */
  WMS_UDH_SMALL_PICTURE,           /**< &nbsp; */
  WMS_UDH_VAR_PICTURE,             /**< &nbsp; */

  WMS_UDH_USER_PROMPT      = 0x13, /**< &nbsp; */
  WMS_UDH_EXTENDED_OBJECT  = 0x14, /**< &nbsp; */

  /* 15 - 1F    Reserved for future EMS */

  WMS_UDH_RFC822           = 0x20, /**< RFC 822 E-Mail Header. */

  WMS_UDH_NAT_LANG_SS      = 0x24, /**< National Language Single Shift. */
  WMS_UDH_NAT_LANG_LS      = 0x25, /**< National Language Locking Shift. */

  /*  21 - 23, 26 - 6F    Reserved for future use */
  /*  70 - 7f    Reserved for (U)SIM Toolkit Security Headers */
  /*  80 - 9F    SME to SME specific use */
  /*  A0 - BF    Reserved for future use */
  /*  C0 - DF    SC specific use */
  /*  E0 - FF    Reserved for future use */

  WMS_UDH_OTHER            = 0xFFFF, /**< For unsupported or proprietary headers. */

/** @cond */
  WMS_UDH_ID_MAX32 = 0x10000000   /* pad to 32 bit int */
/** @endcond */
} wms_udh_id_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH short message information element.
*/
typedef struct wms_udh_concat_8_s
{
  uint8       msg_ref;
    /**< Reference number for a particular concatenated short message. It is 
         constant for every short message that makes up a particular concatenated 
         short message. */
  uint8       total_sm;
    /**< Total number of short messages within the concatenated short message. 
         The value shall start at 1 and remain constant for every short message 
         that makes up the concatenated short message. If the value is zero, the 
         receiving entity shall ignore the entire Information Element. */
  uint8      seq_num;
    /**< Sequence number of a particular short message within the concatenated 
         short message. The value shall start at 1 and increment by one for every 
         short message sent within the concatenated short message. If the value 
         is zero or the value is greater than the value in octet 2, the receiving 
         entity shall ignore the entire Information Element. */
} wms_udh_concat_8_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH Special short message information element.
*/
typedef struct wms_udh_special_sm_s
{
  wms_gw_msg_waiting_e_type                  msg_waiting;
    /**< Waiting action. */
  wms_gw_msg_waiting_kind_e_type             msg_waiting_kind;
    /**< Type of message waiting. */
  uint8                                      message_count;
    /**< Number of messages waiting that are of the type specified in octet 1.*/

} wms_udh_special_sm_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH Wireless Application Protocol 8-bit port information element.
*/
typedef struct wms_udh_wap_8_s
{
  uint8  dest_port; /**< Receiving port (i.e., application) in the receiving 
                         device. */
  uint8  orig_port; /**< Sending port (i.e., application) in the sending 
                         device. */
} wms_udh_wap_8_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH Wireless Application Protocol 16-bit port information element.
*/
typedef struct wms_udh_wap_16_s
{
  uint16  dest_port; /**< Receiving port (i.e., application) in the receiving 
                          device. */
  uint16  orig_port; /**< Sending port (i.e., application) in the sending 
                          device. */
} wms_udh_wap_16_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH 16-bit concatenated information element.
*/
typedef struct wms_udh_concat_16_s
{
  uint16      msg_ref;  /**< Concatenated short message reference number. */
  uint8       total_sm; /**< Maximum number of short messages in the 
                             concatenated short message. */
  uint8       seq_num;  /**< Sequence number of the current short message. */
} wms_udh_concat_16_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/* EMS Headers */
/* --------------------------- */
/** GSM/WCDMA-specific alignment definitions for text formatting.
*/
typedef enum
{
  WMS_UDH_LEFT_ALIGNMENT = 0, /**< &nbsp; */
  WMS_UDH_CENTER_ALIGNMENT,   /**< &nbsp; */
  WMS_UDH_RIGHT_ALIGNMENT,    /**< &nbsp; */
  WMS_UDH_DEFAULT_ALIGNMENT,  /**< Language dependent (default) alignment. */
  WMS_UDH_MAX_ALIGNMENT,      /**< &nbsp; */
/** @cond */
  WMS_UDH_ALIGNMENT_MAX32 = 0x10000000   /* pad to 32 bit int */
/** @endcond */
} wms_udh_alignment_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA-specific font size definitions for text formatting.
*/
typedef enum
{
  WMS_UDH_FONT_NORMAL = 0, /**< Default. */
  WMS_UDH_FONT_LARGE,      /**< &nbsp; */
  WMS_UDH_FONT_SMALL,      /**< &nbsp; */
  WMS_UDH_FONT_RESERVED,   /**< &nbsp; */
/** @cond */
  WMS_UDH_FONT_MAX,
  WMS_UDH_FONT_MAX32 = 0x10000000   /* pad to 32 bit int */
/** @endcond */
} wms_udh_font_size_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA-specific color definitions for text formatting.\ The color values 
  defined are simple primary and secondary colors plus four levels of grey 
  (black to white).\ Bright colors have a higher intensity than dark colors.
*/
typedef enum
{
  WMS_UDH_TEXT_COLOR_BLACK          = 0x0, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_DARK_GREY      = 0x1, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_DARK_RED       = 0x2, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_DARK_YELLOW    = 0x3, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_DARK_GREEN     = 0x4, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_DARK_CYAN      = 0x5, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_DARK_BLUE      = 0x6, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_DARK_MAGENTA   = 0x7, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_GREY           = 0x8, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_WHITE          = 0x9, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_BRIGHT_RED     = 0xA, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_BRIGHT_YELLOW  = 0xB, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_BRIGHT_GREEN   = 0xC, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_BRIGHT_CYAN    = 0xD, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_BRIGHT_BLUE    = 0xE, /**< &nbsp; */
  WMS_UDH_TEXT_COLOR_BRIGHT_MAGENTA = 0xF, /**< &nbsp; */
/** @cond */
  WMS_UDH_TEXT_COLOR_MAX32 = 0x10000000   /* pad to 32 bit int */
/** @endcond */
} wms_udh_text_color_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH text formatting information element.
*/
typedef struct wms_udh_text_formating_s
{
  uint8                     start_position;
    /**< Starting position of the text formatting. */
  uint8                     text_formatting_length; 
    /**< Gives the number of formatted characters or sets a default text 
         formatting. */
  wms_udh_alignment_e_type  alignment_type;
    /**< Indicated by bit 0 and bit 1 of the formatting mode octet. */
  wms_udh_font_size_e_type  font_size;
    /**< Indicated by bit 3 and bit 2 of the formatting mode octet. */
  boolean                   style_bold;
    /**< Indicated by bit 4 of the formatting mode octet. */
  boolean                   style_italic;
    /**< Indicated by bit 5 of the formatting mode octet. */
  boolean                   style_underlined;
    /**< Indicated by bit 6 of the formatting mode octet. */
  boolean                   style_strikethrough;
    /**< Indicated by bit 7 of the formatting mode octet. */
  boolean                   is_color_present;
    /**< If FALSE, ignores the following color information. */
  wms_udh_text_color_e_type text_color_foreground;
    /**< Defines the text foreground color. */
  wms_udh_text_color_e_type text_color_background;
    /**< Defines the text background color. */
} wms_udh_text_formating_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/* --------------------------- */
/* Sound Related Definitions   */
/* --------------------------- */
/** UDH predefined sound information element.
*/
typedef struct wms_udh_pre_def_sound_s
{
  uint8       position;   /**< Number of characters from the beginning of the 
                               SM data after which the sound shall be played. */
  uint8       snd_number; /**< Sound number encoded as an integer value. */
} wms_udh_pre_def_sound_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH user-defined sound information element.
*/
typedef struct wms_udh_user_def_sound_s
{
  uint8       data_length;
    /**< Length of the user-defined sound. */
  uint8       position;
    /**< Indicates in the SM data the instant after which the sound shall be 
         played. */
  uint8       user_def_sound[WMS_UDH_MAX_SND_SIZE];
    /**< Number of the user-defined sound. */
} wms_udh_user_def_sound_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/* --------------------------- */
/* Picture Related Definitions */
/* --------------------------- */
/** UDH large picture information element.
*/
typedef struct wms_udh_large_picture_data_s
{
  uint8             position;
    /**< Number of characters from the beginning of the SM data after which the 
         picture shall be displayed. */
  uint8             data[WMS_UDH_LARGE_PIC_SIZE];
    /**< Data for the large picture. */
} wms_udh_large_picture_data_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH small picture information element.
*/
typedef struct wms_udh_small_picture_data_s
{
  uint8               position;
    /**< Number of characters from the beginning of the SM data after which the 
         picture shall be displayed. */
  uint8               data[WMS_UDH_SMALL_PIC_SIZE];
    /**< Data for the small picture. */
} wms_udh_small_picture_data_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH variable-length picture information element.
*/
typedef struct wms_udh_var_picture_s
{
  uint8       position;
    /**< Number of characters from the beginning of the SM data after which the 
         picture shall be displayed. */
  uint8       width;
    /**< Horizontal dimension of the picture. Number of pixels in multiples of 8. */
  uint8       height;
    /**< Vertical dimension of the picture. Number of pixels in multiples of 8. */ 
  uint8       data[WMS_UDH_VAR_PIC_SIZE];
    /**< Data for the variable picture, line by line from top left to bottom right. */
} wms_udh_var_picture_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/* ----------------------------- */
/* Animation Related Definitions */
/* ----------------------------- */
/** UDH predefined animation information element.
*/
typedef struct wms_udh_pre_def_anim_s
{
  uint8       position;
    /**< Number of characters from the beginning of the SM data after which the 
         animation shall be displayed. */
  uint8       animation_number;
    /**< Animation number encoded as an integer. */
} wms_udh_pre_def_anim_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH large animation information element.
*/
typedef struct wms_udh_large_anim_s
{
  uint8        position;
    /**< Number of characters from the beginning of the SM data after which the 
         animation shall be displayed. */
  uint8        data[WMS_UDH_ANIM_NUM_BITMAPS][WMS_UDH_LARGE_BITMAP_SIZE]; 
    /**< Data for the large animation. */
} wms_udh_large_anim_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH small animation information element.
*/
typedef struct wms_udh_small_anim_s
{
  uint8         position;
    /**< Number of characters from the beginning of the SM data after which the 
         animation shall be displayed. */
  uint8         data[WMS_UDH_ANIM_NUM_BITMAPS][WMS_UDH_SMALL_BITMAP_SIZE];
    /**< Data for the small animation. */
} wms_udh_small_anim_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH email information element.
*/
typedef struct wms_udh_rfc822_s
{
  uint8        header_length; 
    /**< Using 8-bit data, an integer representation of the number of octets 
         within (that fraction of) the RFC 822 E-Mail Header that is located at 
         the beginning of the data part of the current (segment of the 
         concatenated) SM. */
} wms_udh_rfc822_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/* ------------------------------------- */
/* National Language Related Definitions */
/* ------------------------------------- */
/** GSM/WCDMA-specific national language type IDs.
*/
typedef enum
{
  WMS_UDH_NAT_LANG_TURKISH          = 0x1, /**< &nbsp; */
  WMS_UDH_NAT_LANG_SPANISH          = 0x2, /**< &nbsp; */
  WMS_UDH_NAT_LANG_PORTUGUESE       = 0x3, /**< &nbsp; */
  /* 0x0    Reserved */
  /* 0x4 - 0xFF Reserved */
/** @cond */
  WMS_UDH_NAT_LANG_MAX32 = 0x10000000   /* pad to 32 bit int */
/** @endcond */
} wms_udh_nat_lang_id_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH language single shift information element for a national language.

  This structure indicates which National Language Single Shift Table is used 
  instead of the GSM 7-bit default alphabet extension table specified in 
  3GPP TS 23.038 [9].
*/
typedef struct wms_udh_nat_lang_ss_s
{
  wms_udh_nat_lang_id_e_type nat_lang_id; /**< National Language Identifier. */
} wms_udh_nat_lang_ss_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH language locking shift information element for a national language.

  This structure indicates which National Language Locking Shift Table is used 
  instead of the GSM 7-bit default alphabet extension table specified in 
  3GPP TS 23.038 [9].
*/
typedef struct wms_udh_nat_lang_ls_s
{
  wms_udh_nat_lang_id_e_type nat_lang_id; /**< National Language Identifier. */
} wms_udh_nat_lang_ls_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH user prompt indicator information element.

  This structure indicates to the receiving entity that the following object is 
  intended to be handled at the time of reception, e.g., by means of user 
  interaction.
*/
typedef struct wms_udh_user_prompt_s
{      
  uint8       number_of_objects;
    /**< Number of objects (all of the same kind) that follow this header and that 
         will be stitched together by the applications. Objects that can be 
         stitched are images (small, large, variable) and user-defined sounds. 
         Following are two examples.
         - Five small pictures are to be stitched together horizontally.
         - Six iMelody tones are to be connected with the intermediate iMelody 
           header and footer ignored. */
   } wms_udh_user_prompt_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** Extended Object control data; an object can be forwarded.
*/
#define WMS_UDH_EO_CONTROL_FORWARD        0x01

/** @ingroup gw_specific_type_defs_group 
@{ */
/** Extended Object control data; an object can be handled as a User Prompt.
*/
#define WMS_UDH_EO_CONTROL_USER_PROMPT    0x02

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA-specific Extended Object (EO) IDs/types.
*/
typedef enum
{
  WMS_UDH_EO_VCARD                   = 0x09, /**< &nbsp; */
  WMS_UDH_EO_VCALENDAR               = 0x0A, /**< &nbsp; */
/** @cond */
  WMS_UDH_EO_MAX32 = 0x10000000   /* pad to 32 bit int */
/** @endcond */
} wms_udh_eo_id_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** GSM/WCDMA CB Error Indication.
*/
typedef enum 
{
  WMS_GW_CB_ERROR_MEMORY_FULL,   
  /**< Indicates memory is full for GW cell broadcast SMS messages. 
       Users need to delete existing CB SMS messages to receive new messages. */

/** @cond */
  WMS_GW_CB_ERROR_MAX32 = 0x10000000
/** @endcond */
} wms_gw_cb_error_e_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** Maximum amount of GSM/WCDMA-specific data allowed in an Extended Object 
  segment. */
/* 3 bytes for UDHL, EO UDH_ID, EO UDH_LEN; 6 bytes for the whole CONCAT16 */
#define WMS_UDH_EO_DATA_SEGMENT_MAX    131  
/* 140 - 3 - 6 */ 

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH Extended Object content information element.
*/
typedef struct wms_udh_eo_content_s
{
  uint8         length;
    /**< Extended Object length in number of octets (integer representation). */
  uint8         data[WMS_UDH_EO_DATA_SEGMENT_MAX];
    /**< Buffer to store data for vCard or vCalendar contents. */

    /* WMS_UDH_EO_VCARD: See http://www.imc.org/pdi/vcard-21.doc for payload.
     WMS_UDH_EO_VCALENDAR: See http://www.imc.org/pdi/vcal-10.doc.
     Or, Unsupported/proprietary extended objects. */

} wms_udh_eo_content_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH Extended Object information element.

  Extended Objects are to be used together with 16-bit concatenation UDH. The 
  maximum number of segments supported for extended objects is a minumum of 8. 
  The reference, length, control, type, and position fields are only present in 
  the first segment of a concatenated SMS message. 
*/
typedef struct wms_udh_eo_s
{
  wms_udh_eo_content_s_type         content;
    /**< UDH Extended Object content information element. */
  boolean                           first_segment;
    /**< Indicates whether it is the first segment. */
  uint8                             reference;
    /**< Identify those Extended Object segments that are to be linked together. */
  uint16                            length;
    /**< Length of the entire Extended Object data. */                                uint8                             control;
    /**< Control data. */
  wms_udh_eo_id_e_type              type;
    /**< ID of the Extended Object. */
  uint16                            position;
    /**< Absolute position of the Extended Object in the entire text after 
         concatenation, starting from 1. */
} wms_udh_eo_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH for other information elements that may be supported.
*/
typedef struct wms_udh_other_s
{
   wms_udh_id_e_type  header_id;
     /**< User Data Header ID, indicating the type of user data. */
   uint8              header_length;
     /**< Length of the User Data Header. */
   uint8              data[WMS_UDH_OTHER_SIZE];
     /**< User data. */
} wms_udh_other_s_type;

/** @ingroup gw_specific_type_defs_group 
@{ */
/** UDH information element definition.\ This structure uses the wms_udh_u union.
*/
typedef struct wms_udh_s
{
  wms_udh_id_e_type               header_id;
    /**< User Data Header ID, indicating the type of user data. */

  /** Used by the wms_udh_s_type structure.
  */
  union wms_udh_u 
  {
    wms_udh_concat_8_s_type             concat_8;
      /**< Concatenated short messages, 8-bit reference number. */
 /*~ CASE WMS_UDH_CONCAT_8 wms_udh_u.concat_8 */

    wms_udh_special_sm_s_type           special_sm;
      /**< Special SMS Message Indication. */
 /*~ CASE WMS_UDH_SPECIAL_SM wms_udh_u.special_sm */

    wms_udh_wap_8_s_type                wap_8;
      /**< Wireless application port addressing scheme, 8-bit address. */ 
 /*~ CASE WMS_UDH_PORT_8 wms_udh_u.wap_8 */

    wms_udh_wap_16_s_type               wap_16;
      /**< Wireless application port addressing scheme, 16-bit address. */
 /*~ CASE WMS_UDH_PORT_16 wms_udh_u.wap_16 */

    wms_udh_concat_16_s_type            concat_16;
      /**< Concatenated short message, 16-bit reference number. */
 /*~ CASE WMS_UDH_CONCAT_16 wms_udh_u.concat_16 */

    wms_udh_text_formating_s_type       text_formating; /**< Text formatting. */
 /*~ CASE WMS_UDH_TEXT_FORMATING wms_udh_u.text_formating */

    wms_udh_pre_def_sound_s_type        pre_def_sound;  /**< Predefined sound. */
 /*~ CASE WMS_UDH_PRE_DEF_SOUND wms_udh_u.pre_def_sound */

    wms_udh_user_def_sound_s_type       user_def_sound; /**< User-defined sound. */
 /*~ CASE WMS_UDH_USER_DEF_SOUND wms_udh_u.user_def_sound */

    wms_udh_pre_def_anim_s_type         pre_def_anim;   /**< Predefined animation. */
 /*~ CASE WMS_UDH_PRE_DEF_ANIM wms_udh_u.pre_def_anim */

    wms_udh_large_anim_s_type           large_anim;     /**< Large animation. */
 /*~ CASE WMS_UDH_LARGE_ANIM wms_udh_u.large_anim */

    wms_udh_small_anim_s_type           small_anim;     /**< Small animation. */
 /*~ CASE WMS_UDH_SMALL_ANIM wms_udh_u.small_anim */ 

    wms_udh_large_picture_data_s_type   large_picture;  /**< Large picture. */
 /*~ CASE WMS_UDH_LARGE_PICTURE wms_udh_u.large_picture */

    wms_udh_small_picture_data_s_type   small_picture;  /**< Small picture. */
 /*~ CASE WMS_UDH_SMALL_PICTURE wms_udh_u.small_picture */

    wms_udh_var_picture_s_type          var_picture;    /**< Variable picture. */
 /*~ CASE WMS_UDH_VAR_PICTURE wms_udh_u.var_picture */

    wms_udh_user_prompt_s_type          user_prompt; /**< User prompt indicator. */
 /*~ CASE WMS_UDH_USER_PROMPT wms_udh_u.user_prompt */

    wms_udh_eo_s_type                   eo;          /**< Extended Object. */
 /*~ CASE WMS_UDH_EXTENDED_OBJECT wms_udh_u.eo */

    wms_udh_rfc822_s_type               rfc822;      /**< RFC 822 E-Mail Header. */
 /*~ CASE WMS_UDH_RFC822 wms_udh_u.rfc822 */

    wms_udh_nat_lang_ss_s_type          nat_lang_ss;
      /**< National Language Single Shift. */
 /*~ CASE WMS_UDH_NAT_LANG_SS wms_udh_u.nat_lang_ss */

    wms_udh_nat_lang_ls_s_type          nat_lang_ls;
      /**< National Language Locking Shift. */ 
 /*~ CASE WMS_UDH_NAT_LANG_LS wms_udh_u.nat_lang_ls */

    wms_udh_other_s_type                other;
      /**< Other type of information element supported. */
 /*~ DEFAULT wms_udh_u.other */

  }u;   /*~ FIELD wms_udh_s.u DISC _OBJ_.header_id */
} wms_udh_s_type;

/** @} */ /* end_ingroup gw_specific_type_defs_group */

/** @ingroup client_group
@{ */
/** Client types.
*/
typedef enum
{
  WMS_CLIENT_TYPE_JAVA = 0, /**< &nbsp; */
  WMS_CLIENT_TYPE_BREW,     /**< Brew telephone API. */
  WMS_CLIENT_TYPE_WAP_MMS,  /**< Wireless Application Protocol/
                                 More-Messages_to_Send browser. */
  WMS_CLIENT_TYPE_WMS_APP,  /**< Messaging application. */
  WMS_CLIENT_TYPE_UI,       /**< Softkey UI and Core applications. */
  WMS_CLIENT_TYPE_CAT,      /**< Card Application Toolkit module (e.g., GSTK). */
  WMS_CLIENT_TYPE_ATCOP,    /**< AT Command Processor. */
  WMS_CLIENT_TYPE_DIAG,     /**< Diagnostic task. */
  WMS_CLIENT_TYPE_GPS,      /**< GPS application. */
  WMS_CLIENT_TYPE_GSM1x,    /**< GSM1x application. */
  WMS_CLIENT_TYPE_WINCE,    /**< WinCE applications. */
  WMS_CLIENT_TYPE_PBM,      /**< Phonebook Manager. */
  WMS_CLIENT_TYPE_IMS,      /**< IMS/MMD/AIMS. */
  WMS_CLIENT_TYPE_RIL,      /**< Radio Interface Layer. */
  WMS_CLIENT_TYPE_BREW_MP,  /**< Brew Mobile Platform. */
  WMS_CLIENT_FLOATING1,     /**< Floating type 1. */
  WMS_CLIENT_FLOATING2,     /**< Floating type 2. */
  WMS_CLIENT_TYPE_INTERNAL, /**< Internal use only. */
  WMS_CLIENT_TYPE_QMI,      /**< Qualcomm Modem Interface. */
  WMS_CLIENT_TYPE_MFLO,     /**< MediaFlo. */
/** @cond */
  WMS_CLIENT_TYPE_MAX,
  WMS_CLIENT_TYPE_MAX32 = 0x10000000   /* pad to 32 bit */
/** @endcond */
} wms_client_type_e_type;
/*~ SENTINEL wms_client_type_e_type.WMS_CLIENT_TYPE_MAX */

/** @ingroup client_group
@{ */
/** Client ID.
*/
typedef uint8 wms_client_id_type;

/** @ingroup config_group
@{ */
/** Routing information.
*/
typedef struct wms_routing_s
{
  wms_route_e_type            route;     /**< Type of message routing action. */
  wms_memory_store_e_type     mem_store; /**< Type of memory storage for the 
                                              message. */
} wms_routing_s_type;

/** @ingroup config_group
@{ */
/** Message route configuration.
*/
typedef struct wms_routes_s
{
  wms_routing_s_type       pp_routes[WMS_MESSAGE_CLASS_MAX];
    /**< Type of Point-to-Point message routing and storing actions. */
  wms_routing_s_type       bc_routes[WMS_MESSAGE_CLASS_MAX];
    /**< Type of broadcast message routing and storing actions. Meaningful only if 
         FEATURE_CDSMS_BROADCAST is defined. */
  boolean                  transfer_status_report; 
    /**< Indicate whether to transfer a status report. For GSM/WCDMA only: if TRUE, 
         do not store status reports but send them to clients. */
} wms_routes_s_type;

/** @ingroup config_group
@{ */
/** Memory store usage status.
*/
typedef struct wms_memory_status_s
{
  wms_memory_store_e_type     mem_store;      /**< Memory store. */
  uint32                      max_slots;      /**< Maximum number of slots. */
  uint32                      free_slots;     /**< Number of free slots. */
  uint32                      used_tag_slots;
    /**< Number of slots used by messages of the following tag. If the tag is 
         WMS_TAG_NONE, this is the number of all used/occupied slots. */
  wms_message_tag_e_type      tag;            /**< Message tag type. */
} wms_memory_status_s_type;

/** @ingroup config_group
@{ */
/** Message list for messages with a specific tag in a memory store.
*/
typedef struct wms_message_list_s
{
  wms_memory_store_e_type     mem_store;         /**< Memory store. */
  wms_message_tag_e_type      tag;     /**< Messages to be included in the list. */
  uint32                      len;     /**< Number of indices returned. */
  wms_message_index_type      voice_mail_index;  /**< Voice mail index. */
  boolean                     voice_mail_active;
    /**< Indicates whether voice mail is active. */
  wms_message_index_type      indices[WMS_MESSAGE_LIST_MAX]; 
    /**< Indices to message list. */
  wms_message_tag_e_type      tags[WMS_MESSAGE_LIST_MAX]; 
    /**< Message tags in message list. */
} wms_message_list_s_type;

/** @ingroup config_group
@{ */
/** Cell change causes.
*/
typedef enum
{
  WMS_GW_CB_NO_SERVICE,  /**< &nbsp; */
  WMS_GW_CB_SAME_CELL,   /**< &nbsp; */
  WMS_GW_CB_CELL_CHANGE, /**< &nbsp; */
  WMS_GW_CB_LAC_CHANGE,  /**< &nbsp; */
  WMS_GW_CB_PLMN_CHANGE, /**< &nbsp;*/
/** @cond */
  WMS_GW_CB_MAX32
/** @endcond */
}wms_gw_cb_cell_change_e_type;

/** @ingroup config_group
@{ */
/** Configuration event types.
*/
typedef enum
{
  WMS_CFG_EVENT_GW_READY = 0,       /**< GSM/WCDMA ready. */
  WMS_CFG_EVENT_CDMA_READY,         /**< CDMA ready. */
  WMS_CFG_EVENT_ROUTES,             /**< Reports the routing configuration. */
  WMS_CFG_EVENT_MEMORY_STATUS,      /**< Reports the current memory status. */
  WMS_CFG_EVENT_MESSAGE_LIST,       /**< Message list is updated. */
  WMS_CFG_EVENT_MEMORY_FULL,        /**< Memory is full event. */
  WMS_CFG_EVENT_GW_DOMAIN_PREF,     /**< GSM/WCDMA domain preference event. */
  WMS_CFG_EVENT_CELL_CHANGE,        /**< Cell change event.*/
  WMS_CFG_EVENT_PRIMARY_CLIENT_SET, /**< Primary client sets the event. */
  WMS_CFG_EVENT_MEMORY_STATUS_SET,  /**< Set the memory status. */
  WMS_CFG_EVENT_LINK_CONTROL,       /**< Reports the current link control information. */
  WMS_CFG_EVENT_CB_ERROR,           /**< Event from NAS layer to indicate a CB error */
/** @cond */
  WMS_CFG_EVENT_MAX,
  WMS_CFG_EVENT_MAX32 = 0x10000000
/** @endcond */
} wms_cfg_event_e_type;
/*~ SENTINEL wms_cfg_event_e_type.WMS_CFG_EVENT_MAX */

/** @ingroup config_group
@{ */
/** Card Ready event.
*/
#define WMS_CFG_EVENT_SIM_READY WMS_CFG_EVENT_GW_READY

/** @ingroup config_group
@{ */
/** Configuration event information for each event.
*/
typedef union wms_cfg_event_info_u
{
  wms_routes_s_type           routes;
    /**< Event information associated with WMS_CFG_EVENT_ROUTES. */
 /*~ CASE WMS_CFG_EVENT_ROUTES wms_cfg_event_info_u.routes */

  wms_memory_status_s_type    memory_status;
    /**< Event information associated with WMS_CFG_EVENT_MEMORY_STATUS. */
 /*~ CASE WMS_CFG_EVENT_MEMORY_STATUS wms_cfg_event_info_u.memory_status */

  wms_message_list_s_type     message_list;
    /**< Event information associated with WMS_CFG_EVENT_MESSAGE_LIST. */
 /*~ CASE WMS_CFG_EVENT_MESSAGE_LIST wms_cfg_event_info_u.message_list */

  wms_memory_store_e_type      mem_store; 
    /**< Event information associated with WMS_CFG_EVENT_MEMORY_FULL. */
 /*~ CASE WMS_CFG_EVENT_MEMORY_FULL wms_cfg_event_info_u.mem_store */
 
  wms_gw_domain_pref_e_type    gw_domain_pref; 
    /**< Event information associated with WMS_CFG_EVENT_GW_DOMAIN_PREF. */
  /*~ CASE WMS_CFG_EVENT_GW_DOMAIN_PREF wms_cfg_event_info_u.gw_domain_pref */

  wms_gw_cb_cell_change_e_type cell_change_type; 
    /**< Event information associated with WMS_CFG_EVENT_CELL_CHANGE. */
/*~ CASE WMS_CFG_EVENT_CELL_CHANGE wms_cfg_event_info_u.cell_change_type */

   boolean                      memory_full; 
    /**< Event information associated with WMS_CFG_EVENT_MEMORY_STATUS_SET. */
 /*~ CASE WMS_CFG_EVENT_MEMORY_STATUS_SET wms_cfg_event_info_u.memory_full */

  wms_cfg_set_primary_client_s_type  set_primary;
    /**< Event information associated with WMS_CFG_PRIMARY_CLIENT_SET. */
 /*~ CASE WMS_CFG_EVENT_PRIMARY_CLIENT_SET wms_cfg_event_info_u.set_primary */

  wms_cfg_link_control_s_type  link_control;
    /**< Event information associated with WMS_CFG_EVENT_LINK_CONTROL. */
 /*~ CASE WMS_CFG_EVENT_LINK_CONTROL wms_cfg_event_info_u.link_control */

  wms_gw_cb_error_e_type       cb_error_info;
    /**< Event information associated with WMS_CFG_EVENT_CB_ERROR */
 /*~ CASE WMS_CFG_EVENT_CB_ERROR wms_cfg_event_info_u.cb_error_info */

 /*~ DEFAULT wms_cfg_event_info_u.void */
 /*  used as dummy data */

} wms_cfg_event_info_s_type;

/** @ingroup config_group
@{ */
/** Configuration event callback.
*/
typedef void (*wms_cfg_event_cb_type)
(
  wms_cfg_event_e_type         event,    /**< Type of configuration event. */
  wms_cfg_event_info_s_type    *info_ptr /**< Pointer to event information. */
);
/*~ PARAM IN info_ptr POINTER DISC event */
/*~ CALLBACK wms_cfg_event_cb_type
    ONERROR return */

/** @ingroup messaging_group
@{ */
/** Over-the-air raw Transport Service data structure.
*/
typedef struct wms_raw_ts_data_s
{
  wms_format_e_type           format;    /**< Format of the message. */
  wms_gw_tpdu_type_e_type     tpdu_type;
    /**< Type of TPDU.\ Meaningful only if the format is a GSM/WCDMA message. */
  uint32                      len;
    /**< Length of the message.\ Meaningful only if the format is a GSM/WCDMA 
         message. */
  uint8                       data[ WMS_MAX_LEN ]; 
    /**< Data in the message.\ Meaningful only if the format is a GSM/WCDMA 
         message. */
} wms_raw_ts_data_s_type;

/** @ingroup messaging_group
@{ */
/* ------------------- */
/* ---- User Data ---- */
/* ------------------- */
/** User data for CDMA messages.
  If message_id.udh_present == TRUE, num_headers is the number of User Data Headers 
  (UDHs), and headers includes all those headers.
*/
typedef struct wms_cdma_user_data_s
{
  uint8                              num_headers; /**< Number of UDHs. */
  wms_udh_s_type                     headers[WMS_MAX_UD_HEADERS]; 
                                                  /**< All the UDHs. */
  wms_user_data_encoding_e_type      encoding;    /**< Encoding type. */
  wms_IS91EP_type_e_type             is91ep_type; /**< IP-91 type. */
  uint8                              data_len;
    /**< Valid number of bytes in the data array. */
  uint8                              padding_bits;
    /**< Number of invalid bits (0-7) in the last byte of data. This parameter is 
         used for mobile-originated messages only.\n
         There is no way for the API to tell how many padding bits exist in the 
         received message. Instead, the application can find out how many padding 
         bits exist in the user data when decoding the user data. */
  uint8                              data[ WMS_CDMA_USER_DATA_MAX ];
    /**< Raw bits of the user data field of the SMS message. The client software 
         decodes the raw user data according to its supported encoding types and 
         languages. Following are exceptions:
           - CMT-91 user data raw bits are first translated into BD fields 
             (e.g., num_messages and callback).\ The translated user data field in 
             VMN and Short Message is in ASCII characters, each occupying a byte in 
             the resulted data.
           - GSM 7-bit Default characters are decoded such that each byte has one 
             7-bit GSM character. */
  uint8                              number_of_digits;
    /**< Number of digits/characters (7, 8, 16, etc.\ bits) in the raw user data. 
         This data can be used by the client when decoding the user data according 
         to the encoding type and language. */
} wms_cdma_user_data_s_type;

/** @ingroup messaging_group
@{ */
/** User responses for CDMA messages.
*/
typedef uint8           wms_user_response_type;
  /**< This type is used in the SMS User Acknowledgment Message to respond to 
       previously received short messages.\ This element carries the identifier of 
       a predefined response specific to the Message Center. */

/** @ingroup messaging_group
@{ */
/** Cause for an application toolkit busy response is that the Transport Layer 
  destination is busy.
  */
#define WMS_RSP_APPLICATION_TOOLKIT_BUSY   WMS_TL_DESTINATION_BUSY_S

/** @ingroup messaging_group
@{ */
/** Cause for a data download error response is that the Transport Layer has a 
  problem with the other terminal.
*/
#define WMS_RSP_DATA_DOWNLOAD_ERROR        WMS_TL_OTHER_TERMINAL_PROBLEM_S 

/** @ingroup messaging_group
@{ */
/** Reply options for CDMA messages.
*/
typedef struct wms_reply_option_s
{
    boolean          user_ack_requested;     
      /**< User acknowledgment is requested. */
    boolean          delivery_ack_requested; 
      /**< Delivery acknowledgment is requested.\ Set to FALSE for incoming 
           messages. */
    boolean          read_ack_requested;     
      /**< Message originator requests the receiving phone to return a READ_ACK 
           message automatically when the user reads the received message. */
} wms_reply_option_s_type;

/** @ingroup messaging_group
@{ */
/** Maximum number of entries in the broadcast service table.
*/
#define WMS_BC_TABLE_MAX         ( 32 * 4 )

/** @ingroup messaging_group
@{ */
/** Service ID for CDMA messages. The ID is a service/language pair.
*/
typedef struct wms_bc_service_id_s
{
  wms_service_e_type   service;  /**< Specific service. */
  wms_language_e_type  language; /**< Specific language. */
} wms_bc_service_id_s_type;

/** @ingroup messaging_group
@{ */
/** Alert options for receiving broadcast SMS, for CDMA messages.
*/
typedef enum
{
  WMS_BC_ALERT_NONE  = 0,              /**< &nbsp; */
  WMS_BC_ALERT_DEFAULT,                /**< &nbsp; */
  WMS_BC_ALERT_VIBRATE_ONCE,           /**< &nbsp; */
  WMS_BC_ALERT_VIBRATE_REPEAT,         /**< &nbsp; */
  WMS_BC_ALERT_VISUAL_ONCE,            /**< &nbsp; */
  WMS_BC_ALERT_VISUAL_REPEAT,          /**< &nbsp; */
  WMS_BC_ALERT_LOW_PRIORITY_ONCE,      /**< &nbsp; */
  WMS_BC_ALERT_LOW_PRIORITY_REPEAT,    /**< &nbsp; */
  WMS_BC_ALERT_MEDIUM_PRIORITY_ONCE,   /**< &nbsp; */
  WMS_BC_ALERT_MEDIUM_PRIORITY_REPEAT, /**< &nbsp; */
  WMS_BC_ALERT_HIGH_PRIORITY_ONCE,     /**< &nbsp; */
  WMS_BC_ALERT_HIGH_PRIORITY_REPEAT    /**< &nbsp; */
  /* Other values are reserved. */
} wms_bc_alert_e_type;

/** @cond */
/* Max number of services that can be included in a SCPT message */
#define WMS_BC_MM_SRV_LABEL_SIZE   30
#define WMS_BC_SCPT_MAX_SERVICES   5

/* Operation code for SCPT */
typedef enum
{
  WMS_BC_SCPT_OP_DELETE      = 0,
  WMS_BC_SCPT_OP_ADD         = 1,
  WMS_BC_SCPT_OP_DELETE_ALL  = 2,
  WMS_BC_SCPT_OP_MAX32       = 0x10000000
  /* Other values are reserved. */
} wms_bc_scpt_op_code_e_type;

/* Information for an entry in the SCPT Program Data. */
typedef struct wms_bc_scpt_data_entry_s
{
  wms_bc_scpt_op_code_e_type         op_code;

  wms_bc_service_id_s_type           srv_id;

  uint8                              max_messages;
    /* Maximum number of messages to be stored for this service. */
  wms_bc_alert_e_type                bc_alert;
    /* Alert options for this service. */

  uint8                              label_len;
    /* Number of characters in the label.
     ** For Unicode, each character is 2 bytes.
    */
  uint8                              label[WMS_BC_MM_SRV_LABEL_SIZE];
    /*< Name/label for this service */

} wms_bc_scpt_data_entry_s_type;

/* The SCPT Program Data from the network. */
typedef struct wms_bc_scpt_data_s
{
  wms_user_data_encoding_e_type      encoding; /* encoding of service name */
  uint8                              num_entries;
  wms_bc_scpt_data_entry_s_type      entry[WMS_BC_SCPT_MAX_SERVICES];

} wms_bc_scpt_data_s_type;

/* SCPT program result status */
typedef enum
{
  WMS_BC_SCPT_STATUS_OK   = 0,
  WMS_BC_SCPT_STATUS_SERVICE_MEMORY_FULL,
  WMS_BC_SCPT_STATUS_SERVICE_LIMIT_REACHED,
  WMS_BC_SCPT_STATUS_ALREADY_PROGRAMMED,
  WMS_BC_SCPT_STATUS_NOT_YET_PROGRAMMED,
  WMS_BC_SCPT_STATUS_INVALID_MAX_MESSAGES,
  WMS_BC_SCPT_STATUS_INVALID_ALERT_OPTION,
  WMS_BC_SCPT_STATUS_INVALID_SERVICE_NAME,
  WMS_BC_SCPT_STATUS_UNSPECIFIED_ERROR
  /* other values are reserved */
} wms_bc_scpt_status_e_type;

/* Information for an entry in the SCPT program result. */
typedef struct wms_bc_scpt_result_entry_s
{
  wms_bc_service_id_s_type            srv_id;
  wms_bc_scpt_status_e_type           status;

} wms_bc_scpt_result_entry_s_type;

/* SCPT program result to the network. */
typedef struct wms_bc_scpt_result_s
{
  uint8                                    num_entries;
  wms_bc_scpt_result_entry_s_type          entry[WMS_BC_SCPT_MAX_SERVICES];
} wms_bc_scpt_result_s_type;

/** @endcond */ /* SCPT - Internal use only */

/* ===== Multimode Broadcast Definitions ===== */
/** @ingroup mmbroadcast_group
@{ */
/** Commercial Mobile Alert System CBS message identifier for Presidential-level alerts.\ The MMI cannot 
  set this. * 
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID1     4370

/** @ingroup mmbroadcast_group
@{ */
/** CMAS CBS message identifier for extreme alerts with Severity = Extreme, 
  Urgency = Immediate, and Certainty = Observed. 
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID2     4371

/** @ingroup mmbroadcast_group
@{ */
/* CMAS CBS message identifier for extreme alerts with Severity = Extreme, 
  Urgency = Immediate, and Certainty = Likely. 
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID3     4372

/** @ingroup mmbroadcast_group
@{ */
/** CMAS CBS message identifier for extreme alerts with Severity = Extreme, 
  Urgency = Expected, and Certainty = Observed. 
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID4     4373

/** @ingroup mmbroadcast_group
@{ */
/** CMAS CBS message identifier for extreme alerts with Severity = Extreme, 
  Urgency = Expected, and Certainty = Likely.
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID5     4374

/** @ingroup mmbroadcast_group
@{ */
/** CMAS CBS message identifier for Severe Alerts with Severity = Severe, 
  Urgency = Immediate, and Certainty = Observed. 
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID6     4375

/** @ingroup mmbroadcast_group
@{ */
/** CMAS CBS message identifier for Severe Alerts with Severity = Severe, 
  Urgency = Immediate, and Certainty = Likely
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID7     4376

/** @ingroup mmbroadcast_group
@{ */
/** CMAS CBS message identifier for Severe Alerts with Severity = Severe, 
  Urgency = Expected, and Certainty = Observed.
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID8     4377

/** @ingroup mmbroadcast_group
@{ */
/** CMAS CBS message identifier for Severe Alerts with Severity = Severe, 
  Urgency = Expected, and Certainty = Likely.
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID9     4378

/** @ingroup mmbroadcast_group
@{ */
/** CMAS CBS message identifier for a Child Abduction Emergency (or Amber Alert).
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID10    4379

/** @ingroup mmbroadcast_group
@{ */
/** CMAS CBS message identifier for the Required Monthly Test.
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID11    4380

/** @ingroup mmbroadcast_group
@{ */
/** CMAS CBS message identifier for the CMAS Exercise.
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID12    4381

/** @ingroup mmbroadcast_group
@{ */
/** CMAS CBS message identifier for operator-defined use.
*/
#define WMS_GW_CB_SRV_ID_CMAS_MSGID13    4382


/** @ingroup mmbroadcast_group
@{ */
/* Multimode broadcast definitions.
*/
typedef uint16     wms_gw_cb_srv_id_type; /**< Service IDs. */

/** @ingroup mmbroadcast_group
@{ */
/** GSM/WCDMA cell broadcast geographic scope.
*/
typedef enum
{
  WMS_GW_CB_GEO_SCOPE_CELL_IMMEDIATE = 0, /**< Immediate cell. */
  WMS_GW_CB_GEO_SCOPE_PLMN           = 1, /**< Public Land Mobile Network. */
  WMS_GW_CB_GEO_SCOPE_LOCATION_AREA  = 2, /**< Location area defined by the
                                               Location Area Code. */
  WMS_GW_CB_GEO_SCOPE_CELL_NORMAL    = 3  /**< Normal cell. */
} wms_gw_cb_geo_scope_e_type;

/** @ingroup mmbroadcast_group
@{ */
/** GSM/WCDMA cell broadcast languages, as defined in 3GPP TS 23.038.
*/
typedef enum
{
  WMS_GW_CB_LANGUAGE_GERMAN      = 0x00, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_ENGLISH     = 0x01, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_ITALIAN     = 0x02, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_FRENCH      = 0x03, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_SPANISH     = 0x04, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_DUTCH       = 0x05, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_SWEDISH     = 0x06, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_DANISH      = 0x07, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_PORTUGUESE  = 0x08, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_FINNISH     = 0x09, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_NORWEGIAN   = 0x0a, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_GREEK       = 0x0b, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_TURKISH     = 0x0c, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_HUNGARIAN   = 0x0d, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_POLISH      = 0x0e, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_UNSPECIFIED = 0x0f,
    /**< If a language indication is not in the CB message, this enumerator is 
         used. */
  WMS_GW_CB_LANGUAGE_CZECH       = 0x20, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_21 = 0x21, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_22 = 0x22, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_23 = 0x23, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_24 = 0x24, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_25 = 0x25, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_26 = 0x26, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_27 = 0x27, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_28 = 0x28, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_29 = 0x29, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_2A = 0x2a, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_2B = 0x2b, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_2C = 0x2c, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_2D = 0x2d, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_2E = 0x2e, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_RESERVED_2F = 0x2f, /**< &nbsp; */
  WMS_GW_CB_LANGUAGE_ISO_639     = 0x10,
    /**< ISO639 2-letter language code is in the first two GSM 7-bit characters 
         of the CB user data. */
  WMS_GW_CB_LANGUAGE_DUMMY       = 255   /**< &nbsp; */
} wms_gw_cb_language_e_type;

/** @ingroup mmbroadcast_group
@{ */
/** GSM/WCDMA CB Data Coding Scheme groups.
*/
typedef enum
{
  WMS_GW_CB_DCS_GROUP_DEFINED   = 0, /**< &nbsp; */
  WMS_GW_CB_DCS_GROUP_WAP,           /**< &nbsp; */
  WMS_GW_CB_DCS_GROUP_RESERVED       /**< &nbsp; */
} wms_gw_cb_dcs_group_e_type;

/** @ingroup mmbroadcast_group
@{ */
/** GSM/WCDMA CB Data Coding Schemes (DCSs).
*/
typedef struct wms_gw_cb_dcs_s
{
  wms_gw_cb_dcs_group_e_type  group;          /**< Type of DCS group. */

  /* If group==DEFINED:  */
  wms_message_class_e_type    msg_class;       /**< If the group is defined. */
  boolean                     is_compressed;   /**< If the group is defined. */
  wms_gw_alphabet_e_type      alphabet;        /**< If the group is defined. */
  wms_gw_cb_language_e_type   language;        /**< If the group is defined. */
  uint8                       iso_639_lang[3];
    /**< If the group is defined: two GSM characters and one CR character. */
  boolean                     is_udh_present;  /**< If the group is defined. */

  /* If group==WAP or RESERVED: (This is available for other DCS groups too) */
  uint8                       raw_dcs_data;
    /**< If the group is Wireless Application Protocol or Reserved. This can 
         also be used if the group is defined. */
} wms_gw_cb_dcs_s_type;

/** @} */ /* end_ingroup mmbroadcast_group */

/* ===== Messaging Group ===== */
/** @ingroup messaging_group
@{ */
/** GSM/WCDMA cell broadcast decoded page header.
*/
typedef struct wms_gw_cb_page_header_s
{
  wms_gw_cb_srv_id_type        cb_srv_id;    /**< Service ID. */
  wms_gw_cb_dcs_s_type         cb_dcs;       /**< CB data coding scheme. */
  uint8                        total_pages;  /**< Total pages defined. */
    /* [1, 15] */ 
  uint8                        page_number;  /**< Page number. */
    /* [1, 15] */ 

  /* The following are fields in Serial Number as defined in 3GPP 23.041 */
  wms_gw_cb_geo_scope_e_type   geo_scope;
    /**< Geographic scope for a cell broadcast. In the Serial Number as defined 
         in 3GPP 23.041. */
  uint16                       message_code;
    /**< Message code. In the Serial Number as defined in 3GPP 23.041. */
    /* [0, 1023] */ 
  uint8                        update_number;
    /**< Update number.\ In the Serial Number as defined in 3GPP 23.041. */
    /* [0, 15] */ 
  uint16                       raw_serial_number;
    /**< Original serial number raw data */
} wms_gw_cb_page_header_s_type;

/** @ingroup messaging_group
@{ */
/** Size of the IP address.
*/
#define WMS_IP_ADDRESS_SIZE     4 

/** @ingroup messaging_group
@{ */
/** IP address structure.
*/
typedef struct wms_ip_address_s
{
  uint8               address[WMS_IP_ADDRESS_SIZE]; /**< IP address. */
  boolean             is_valid;     /**< Indicates whether the IP address entry 
                                         is valid. */
} wms_ip_address_s_type;

/** @ingroup messaging_group
@{ */
/** Special structure that captures any unrecognized and/or proprietary 
  parameters.
*/
typedef struct wms_other_parm_s
{
  uint8                         input_other_len;
    /**< Length of other parameters. */
  uint8                         desired_other_len;
    /**< Number of bytes needed to properly decode the other parameters. */
  uint8                         * other_data;
  /**< Pointer to the other data. */
  /*~ FIELD wms_other_parm_s.other_data VARRAY LENGTH wms_other_parm_s.input_other_len*/
} wms_other_parm_s_type;

/** @ingroup messaging_group
@{ */
/** CDMA client message structure.
*/
typedef struct wms_cdma_message_s
{
  boolean                      is_mo;
    /**< TRUE -- MO message; FALSE -- MT message. */
  wms_teleservice_e_type       teleservice;
    /**< Type of teleservice.\ If the type is not available, it is set to Unknown. */
  wms_address_s_type           address;
    /**< For incoming messages, this is the origination address.\ Otherwise, this 
         is the destination address. This parameter is mandatory in both incoming 
         and outgoing messages. */
  wms_subaddress_s_type        subaddress;
    /**< (Optional) If subaddress.number_of_digits == 0, this field is not present. */
  boolean                      is_tl_ack_requested;
    /**< Indicates whether the client is to confirm whether the message is 
         received successfully. */
  boolean                      is_service_present;
    /**< Indicates whether the service is present. */
  wms_service_e_type           service;    /**< Type of service. */
  wms_raw_ts_data_s_type       raw_ts;     /**< Raw (un-decoded) bearer data. */
} wms_cdma_message_s_type;

/** @ingroup messaging_group
@{ */
/** CDMA bearer data structure.
*/
typedef struct wms_client_bd_s
{
  uint32                        mask;          /**< Indicates which fields are 
                                                    present in this message. */
  wms_message_id_s_type         message_id;    /**< Type of message. */
  wms_cdma_user_data_s_type     user_data;     /**< User data. */
  wms_user_response_type        user_response; 
    /**< Identifies a predefined response from the Message Center sent in the 
         SMS User Acknowledgment Message in response to previously received short 
         messages. */
  wms_timestamp_s_type          mc_time;
    /**< Message Center timestamp, which can be included with SMS messages sent 
         from the Message Center. */
  wms_timestamp_s_type          validity_absolute;
    /**< Tells the Message Center the absolute time of the validity period, after 
         which the message should be discarded if not delivered to the destination. */
  wms_timestamp_s_type          validity_relative;
    /**< Tells the Message Center the time period, starting from the time the 
         message is received by the Message Center, after which the message 
         should be discarded if not delivered to the destination.
         It can also be used to indicate the time period to retain a message 
         sent to a mobile station. */
  wms_timestamp_s_type          deferred_absolute;
    /**< Absolute time of delivery desired by the originator.  */
  wms_timestamp_s_type          deferred_relative;  
    /**< Relative time of delivery desired by the sender.
         It indicates the time period, starting from the time the message is 
         received by the Message Center, after which the message should be 
         delivered. */
  wms_priority_e_type           priority;  /**< Priority level of the message. */
  wms_privacy_e_type            privacy;   /**< Privacy level of the message. */
  wms_reply_option_s_type       reply_option; 
    /**< Indicates whether SMS acknowledgment is requested or not requested. */
  uint8                         num_messages; 
    /**< Number of messages stored at the Voice Mail System.\ This represents 
         the actual value, not broadcast data. */
  wms_alert_mode_e_type         alert_mode;      
    /**< For pre-IS-637A implementations, alert_mode is either OFF or ON.\ The 
         type of alert used can distinguish different priorities of the message. */
  wms_language_e_type           language;
    /**< Language of the message. */
  wms_address_s_type            callback;
    /**< Call-back number to be dialed in reply to a received SMS message. */
  wms_display_mode_e_type       display_mode;
    /**< Indicates to the mobile station when to display the received message. */
  wms_download_mode_e_type      download_mode;
    /**< Type of download supported. */
  wms_delivery_status_s_type    delivery_status; 
    /**< Indicates the status of a message, or if an SMS error has occurred, the 
         nature of the error and whether the condition is considered temporary or 
         permanent. */
  uint32                        deposit_index;   
    /**< Assigned by the Message Center as a unique index to the contents of the 
         User Data subparameter in each message sent to a particular mobile station.
         When replying to a previously received short message that included a 
         Message Deposit Index subparameter, the mobile station may include the 
         Message Deposit Index of the received message to indicate to the Message 
         Center that the original contents of the message are to be included in 
         the reply. */

    /* NOTE: All SCPT related definitions are for internal use only!  */

/** @cond */
  wms_bc_scpt_data_s_type       * scpt_data_ptr;
  /*~ FIELD wms_client_bd_s.scpt_data_ptr VARRAY LENGTH 0 */

  wms_bc_scpt_result_s_type     * scpt_result_ptr;
  /*~ FIELD wms_client_bd_s.scpt_result_ptr VARRAY LENGTH 0 */
/** @endcond */ /* SCPT - Internal use only */

  wms_ip_address_s_type         ip_address;    /**< IP address. */
  uint8                         rsn_no_notify; /**< Reason of no notify. */

  wms_other_parm_s_type         other; 
    /**< Unrecognized or proprietary parameters included.\ See the function 
         comments for wms_ts_decode() and wms_ts_decode_cdma_bd_with_other() 
         for details about other parameters. */
} wms_client_bd_s_type;

/** @ingroup messaging_group
@{ */
/** Template structure for CDMA messages. 
  If WMS_CDMA_TEMPLATE_MASK_BEARER_REPLY_OPTION is set in the mask, there is no 
  corresponding field for it in this structure.
*/
typedef struct wms_cdma_template_s
{
  wms_teleservice_e_type          teleservice;  /**< Type of teleservice. */
  uint16                          mask; 
    /**< Indicates which of the following fields are present. */
  wms_address_s_type              address;
    /**< Address parameters for the message. */
  wms_subaddress_s_type           subaddress;   /**< Subaddress parameter. */
  wms_user_data_encoding_e_type   dcs;
    /**< Data Coding Scheme for user data. */
  wms_timestamp_s_type            relative_validity;
    /**< Validity Period, relative format. */
  wms_service_e_type              service;      /**< Type of service to use. */
  wms_client_bd_s_type            client_bd;    /**< Client bearer data type. */
} wms_cdma_template_s_type;

/** @ingroup messaging_group
@{ */
/** Acknowledgment information for CDMA messages.
*/
typedef struct wms_cdma_ack_info_s
{
  wms_error_class_e_type      error_class;
    /**< Type of error (temporary or permanent). */
  wms_cdma_tl_status_e_type   tl_status;
    /**< CDMA-specific status code for a Transport Layer error. */
} wms_cdma_ack_info_s_type;

/* ====================== GSM/WCDMA message definitions ==================== */
/** @ingroup messaging_group
@{ */
/** Data Coding Scheme (DCS) for GSM/WCDMA messages.
*/
typedef struct wms_gw_dcs_s
{
  wms_message_class_e_type          msg_class;       /**< Message class. */
  boolean                           is_compressed;  
    /**< Indicates whether the message is compressed. */
  wms_gw_alphabet_e_type            alphabet;
    /**< GSM/WCDMA-specific alphabet set. */  
  wms_gw_msg_waiting_e_type         msg_waiting;
    /**< Type of message waiting action. */
  boolean                           msg_waiting_active;
    /**< Indicates whether the message waiting is active. */
  wms_gw_msg_waiting_kind_e_type    msg_waiting_kind;
    /**< Type of message waiting (e.g., voice mail, email). */ 
  uint8                             raw_dcs_data;    /**< Raw DCS byte. */
} wms_gw_dcs_s_type;

/** @ingroup messaging_group
@{ */
/** User data types for GSM/WCDMA messages. 
  If DCS indicates the default 7-bit alphabet, each byte holds one character 
  with bit 7 equal to 0, and sm_len indicates the number of characters. If DCS 
  indicates otherwise, each character can occupy multiple bytes, and sm_len 
  indicates the total number of bytes.
*/
typedef struct wms_gw_user_data_s
{
  uint8                          num_headers;
    /**< Number of user data headers. */
  wms_udh_s_type                 headers[WMS_MAX_UD_HEADERS];
    /**< Types of user data headers. */
  uint16                         sm_len;
    /**< Length of short message. */
  uint8                          sm_data[WMS_MAX_LEN];
    /**< Short message data. */
} wms_gw_user_data_s_type;

/** @ingroup messaging_group
@{ */
/** Deliver report with acknowledgment TPDU, for GSM/WCDMA messages.
*/
typedef struct wms_gw_deliver_report_ack_s
{
  boolean                         user_data_header_present;
    /**< TP User Data Header Indicator (TP-UDHI). */
  uint32                          mask;
    /**< Indicates which of the optional fields are present. */
  wms_pid_e_type                  pid;
    /**< TP Protocol Identifier (TP-PID). */
  wms_gw_dcs_s_type               dcs;
    /**< TP Data Coding Scheme (TP-DCS). */
  wms_gw_user_data_s_type         user_data;
    /**< TP User Data (TP-UD). */
} wms_gw_deliver_report_ack_s_type;

/** @ingroup messaging_group
@{ */
/** Deliver report with error TPDU, for GSM/WCDMA messages.
*/
typedef struct wms_gw_deliver_report_error_s
{
  boolean                         user_data_header_present;
    /**< TP User Data Header Indicator (TP-UDHI). */
  wms_tp_cause_e_type             tp_cause;
    /**< TP Failure Cause (TP-FCS). */
  uint32                          mask;
    /**< Indicates which of the optional fields are present. */
  wms_pid_e_type                  pid;
    /**< TP Protocol Identifier (TP-PID). */
  wms_gw_dcs_s_type               dcs;
    /**< TP Data Coding Scheme (TP-DCS). */
  wms_gw_user_data_s_type         user_data;
    /**< TP User Data (TP-UD). */
} wms_gw_deliver_report_error_s_type;

/** @ingroup messaging_group
@{ */
/** Submit report with acknowledgment TPDU, for GSM/WCDMA messages.
*/
typedef struct wms_gw_submit_report_ack_s
{
  boolean                        user_data_header_present;
    /**< TP User Data Header Indicator (TP-UDHI). */
  wms_timestamp_s_type           timestamp;
    /**< TP Service Centre Time Stamp (TP-SCTS). */
  uint32                         mask;
    /**< Indicates which of the following optional fields are present. */         
  wms_pid_e_type                 pid;     /**< TP Protocol Identifier (TP-PID). */
  wms_gw_dcs_s_type              dcs;     /**< TP Data Coding Scheme (TP-DCS). */
  wms_gw_user_data_s_type        user_data; /**< TP User Data (TP-UD). */
} wms_gw_submit_report_ack_s_type;

/** @ingroup messaging_group
@{ */
/** Submit report with error TPDU, for GSM/WCDMA messages.
*/
typedef struct wms_gw_submit_report_error_s
{
  boolean                       user_data_header_present;
    /**< TP User Data Header Indicator (TP-UDHI). */
  wms_tp_cause_e_type           tp_cause;
    /**< TP Failure Cause (TP-FCS). */
  wms_timestamp_s_type          timestamp;
    /**< TP Service Centre Time Stamp (TP-SCTS). */
  uint32                        mask;
    /**< Indicates which of the following optional fields are present. */
  wms_pid_e_type                pid;     /**< TP Protocol Identifier (TP-PID). */
  wms_gw_dcs_s_type             dcs;     /**< TP Data Coding Scheme (TP-DCS). */
  wms_gw_user_data_s_type       user_data; /**< TP User Data (TP-UD). */
} wms_gw_submit_report_error_s_type;

/** @ingroup messaging_group
@{ */
/** Alphabetic ID for GSM/WCDMA messages.
*/
typedef struct wms_alpha_id_s
{
  uint8   len;     /**< Length of a 7-bit GSM default alphabet string. */
  uint8   *data;   /**< Pointer to a 7-bit GSM default alphabet string. */
} wms_alpha_id_s_type;
/*~ FIELD wms_alpha_id_s.data VARRAY LENGTH wms_alpha_id_s.len */

/** @ingroup messaging_group
@{ */
/** Template definition for GSM/WCDMA messages.
*/
typedef struct wms_gw_template_s
{
  wms_alpha_id_s_type          alpha_id;  /**< Identifier for a 7-bit default 
                                               alphabet string. */
  uint8                        mask;      /**< Indicates which of the following 
                                               fields are present. */
  wms_address_s_type           dest_addr; /**< Destination address. */
  wms_address_s_type           sc_addr;   /**< Service center address. */
  wms_pid_e_type               pid;       /**< Protocol Identifier Data. */
  wms_gw_dcs_s_type            dcs;       /**< Data Coding Scheme. */
  wms_timestamp_s_type         relative_validity; /**< Validity Period relative 
                                                       format. */
} wms_gw_template_s_type;

/** @ingroup messaging_group
@{ */
/** Message definition for GSM/WCDMA messages.
*/
typedef struct wms_gw_message_s
{
  boolean                       is_broadcast; /**< Indicates whether this a cell 
                                                   broadcast message. */
  wms_address_s_type            sc_address;   /**< Address of the Switching 
                                                   Center. */
  wms_raw_ts_data_s_type        raw_ts_data;  /**< Raw Transport Service data. */
} wms_gw_message_s_type;

/** @ingroup messaging_group
@{ */
/** Acknowledgment information for GSM/WCDMA messages.
*/
typedef struct wms_gw_ack_info_s
{
  boolean                              success; 
    /**< Indicates whether this is an acknowledgment or an error report. */
  wms_rp_cause_e_type                  rp_cause;
    /**< Report the cause of failure.\ This is used when success==FALSE. */
  wms_gw_tpdu_type_e_type              tpdu_type;
    /**< Use WMS_TPDU_NONE if deliver_report TPDUs are not included. */

  /** Used by the wms_gw_ack_info_s_type structure.
  */
  union wms_gw_ack_info_u 
  {
    wms_gw_deliver_report_ack_s_type    ack;   /**< Acknowledgment information. */
     /* if success==TRUE  */
    /*~ IF (_DISC_ == TRUE) wms_gw_ack_info_u.ack */
    wms_gw_deliver_report_error_s_type  error; /**< Error information. */
     /* if success==FALSE */ 
    /*~ IF (_DISC_ == FALSE) wms_gw_ack_info_u.error */

    /*~ DEFAULT wms_gw_ack_info_u.void */
  } u;
   /*~ FIELD wms_gw_ack_info_s.u DISC _OBJ_.success */
} wms_gw_ack_info_s_type;

/* ============ Client message definitions ========== */
/** @ingroup messaging_group
@{ */
/** Header definition for CDMA messages.
*/
typedef struct wms_client_msg_hdr_s
{
  wms_message_mode_e_type   message_mode;
    /**< Message mode (e.g., CDMA or GSM/WDCMA). */
  wms_message_tag_e_type    tag;
    /**< Set to WMS_TAG_NONE if not applicable. */
  wms_memory_store_e_type   mem_store;
    /**< Memory store used for storing this message.\ Set to 
         WMS_MEMORY_STORE_NONE if this message is not to be stored. */
  wms_message_index_type    index;
    /**< Index for a stored message.\ Set to WMS_DUMMY_MESSAGE_INDEX when 
         mem_store==WMS_MEMORY_STORE_NONE. */
} wms_client_msg_hdr_s_type;          

/** @ingroup messaging_group
@{ */
/** Information about a client message of all types.

  Message types include MT, MO, regular message, report, template, CB message, 
  etc. 
*/
typedef struct wms_client_message_s
{
  wms_client_msg_hdr_s_type     msg_hdr; /**< Header definition. */

  /** Used by the wms_client_message_s_type structure. The message_mode and tag 
      together decide which structure to use in this union. */
  union wms_client_message_u
  {
    wms_cdma_message_s_type      cdma_message;  /**< CDMA message type. */
  /*~ IF ( _DISC_.message_mode == WMS_MESSAGE_MODE_CDMA &&
          _DISC_.tag != WMS_TAG_MO_TEMPLATE)
        wms_client_message_u.cdma_message*/

    wms_cdma_template_s_type     cdma_template; /**< CDMA message template. */
  /*~ IF ( _DISC_.message_mode == WMS_MESSAGE_MODE_CDMA &&
          _DISC_.tag == WMS_TAG_MO_TEMPLATE)
       wms_client_message_u.cdma_template*/

    wms_gw_message_s_type        gw_message;    /**< GSM/WCDMA message type. */
  /*~ IF ( _DISC_.message_mode == WMS_MESSAGE_MODE_GW &&
          _DISC_.tag != WMS_TAG_MO_TEMPLATE)
        wms_client_message_u.gw_message*/

    wms_gw_template_s_type       gw_template;   /**< GSM/WCDMA message template. */
  /*~ IF ( _DISC_.message_mode == WMS_MESSAGE_MODE_GW &&
          _DISC_.tag == WMS_TAG_MO_TEMPLATE)
        wms_client_message_u.gw_template*/

    /*~ DEFAULT wms_client_message_u.void */

   } u; /*~ FIELD wms_client_message_s.u DISC _OBJ_.msg_hdr */
} wms_client_message_s_type;

/* ============ Message event definitions =========== */
/** @ingroup messaging_group
@{ */
/** Status information of a message that was sent/stored.
*/
typedef struct wms_message_status_info_s
{
  void                        *user_data; /**< User data pointer passed in the 
                                               original command to the WMS. */
  wms_client_type_e_type      client_id;  /**< Client that issued the original 
                                               command to the WMS. */
  wms_send_mode_e_type        send_mode;  /**< Valid if the event is 
                                               WMS_MSG_EVENT_SEND. */
  wms_write_mode_e_type       write_mode; /**< Valid if the event is 
                                               WMS_MSG_EVENT_WRITE. */
  wms_client_message_s_type   message;    /**< Message/template/status report 
                                               that is operated on. */
  wms_alpha_id_s_type         alpha_id;   /**< Valid if the event is 
                                               WMS_MSG_EVENT_SEND. */
} wms_message_status_info_s_type;

/** @ingroup messaging_group
@{ */
/** Information about a message received from the network.
*/
typedef struct wms_mt_message_info_s
{
  wms_client_type_e_type         client_id; 
     /**< If the client ID is set to WMS_CLIENT_TYPE_MAX, this message was 
          broadcast to all clients.\ Otherwise, the message was delivered to a 
          single client of this particular client_id. */
  wms_route_e_type               route; 
     /**< For TRANSFER_ONLY, the client needs to acknowledge the message. */
  wms_transaction_id_type        transaction_id; 
     /**< Differentiate between multiple incoming messages. */
  wms_client_message_s_type      message; 
     /**< Information about the client message type. */
} wms_mt_message_info_s_type;

/** @ingroup messaging_group
@{ */
/** Information about an error regarding a message received from the network.
*/
typedef struct wms_mt_message_error_s
{
  wms_transaction_id_type        transaction_id; /**< Differentiate between 
                                                      multiple incoming messages. */
  wms_report_status_e_type       report_status;  /**< Success or failure reason. */
} wms_mt_message_error_s_type;

/** @ingroup messaging_group
@{ */
/** Cause information in the submit report; for message events.
*/
typedef struct wms_cause_info_s
{
  wms_error_class_e_type        error_class; 
     /**< Type of error (permanent or temporary). Applicable to CDMA only. */
  wms_cdma_tl_status_e_type     tl_status;
     /**< CDMA-specific status code for Transport Layer error. */
  wms_rp_cause_e_type           cause_value; 
     /**< GSM/WCDMA-specific Relay Protocol (RP) cause types. */                                    
  boolean                       diagnostics_present; 
     /**< Indicates whether diagnostics are present.\ If not, ignore the next 
          member. Applicable to GSM/WCDMA only. */
  uint8                         diagnostics; 
     /**< Diagnostic data. Applicable to GSM/WCDMA only. */
} wms_cause_info_s_type;

/** @ingroup messaging_group
@{ */
/** Submit report information for message events.
*/
typedef struct wms_submit_report_info_s
{
  void                        *user_data;    /**< Pointer to user data. */
  wms_client_type_e_type      client_id;     /**< Client that issued the original 
                                                  command to the WMS. */
  wms_report_status_e_type    report_status; /**< Success or failure reason. */
  wms_message_mode_e_type     message_mode;  /**< System mode to be used for a 
                                                  message. */
  wms_cause_info_s_type       cause_info;    /**< Cause information in the 
                                                  submit report. */                   wms_gw_tpdu_type_e_type     tpdu_type;     /**< GSM/WCDMA-specific TPDU type. */
  wms_alpha_id_s_type         alpha_id;      /**< Alphabetic ID for GSM/WCDMA 
                                                  messages. */

  /** Used by the wms_submit_report_info_s_type structure.
  */
  union wms_submit_report_info_u
  {
    wms_gw_submit_report_ack_s_type    ack;   /**< Acknowledge success. 
                                                   Applicable to GSM/WCDMA only */
    /*~ CASE WMS_RPT_OK wms_submit_report_info_u.ack */
    wms_gw_submit_report_error_s_type  error; /**< Report error. Applicable to 
                                                   GSM/WCDMA only */
    /*~ DEFAULT wms_submit_report_info_u.error */

  } u; /*~ FIELD wms_submit_report_info_s.u DISC _OBJ_.report_status */
} wms_submit_report_info_s_type;

/** @ingroup messaging_group
@{ */
/** Acknowledgment request information for message events.
*/
typedef struct wms_ack_info_s
{
  wms_transaction_id_type          transaction_id;
    /**< Differentiate between multiple incoming messages. */
  wms_message_mode_e_type          message_mode;
    /**< System mode to be used for a message. */

  /** Used by the wms_ack_info_s_type structure.
  */
  union wms_ack_info_u
  {
    wms_cdma_ack_info_s_type       cdma;  /**< Acknowledgment information for 
                                               CDMA messages. */
     /*~ CASE WMS_MESSAGE_MODE_CDMA wms_ack_info_u.cdma */
    wms_gw_ack_info_s_type         gw;    /**< Acknowledgment information for 
                                               GSM/WCDMA messages. */
    /*~ CASE WMS_MESSAGE_MODE_GW wms_ack_info_u.gw */
    /* TBD: check default mode */

    /*~ DEFAULT wms_ack_info_u.void */

  } u; /*~ FIELD wms_ack_info_s.u DISC _OBJ_.message_mode */
} wms_ack_info_s_type;

/** @ingroup messaging_group
@{ */
/** Acknowledgment report information for message events.
*/
typedef struct
{
  wms_client_type_e_type           client_id;
    /**< Type of client application. */
  wms_transaction_id_type          transaction_id;
    /**< Differentiate between multiple incoming messages. */
  wms_message_mode_e_type          message_mode;
    /**< System mode to be used for a message. */
  void                             *user_data;
    /**< Pointer to the user data. */
  boolean                          is_success;
    /**< Indicates whether the acknowledgment was sent successfully. */
} wms_ack_report_info_s_type;

/** @ingroup messaging_group
@{ */
/** Message event types for message events.
*/
typedef enum
{
  /* Command related events */
  WMS_MSG_EVENT_SEND = 0,
    /**< Command-related event: reports the local status of the send request. */
  WMS_MSG_EVENT_ACK,
    /**< Command-related event: reports the local status of the acknowledgment 
         request. */
  WMS_MSG_EVENT_READ,
    /**< Command-related event: reports the result of the read request. */
  WMS_MSG_EVENT_WRITE,
    /**< Command-related event: reports the status of the send request.
         In insert mode, the new message index is returned; otherwise, the 
         original index is returned. */
  WMS_MSG_EVENT_DELETE,
    /**< Command-related event: reports the result of the delete message request. */
  WMS_MSG_EVENT_DELETE_ALL,
    /**< Command-related event: reports the result of the delete all messages 
         request. */
  WMS_MSG_EVENT_MODIFY_TAG,
    /**< Command-related event: reports the result of the modification request. */
  WMS_MSG_EVENT_READ_TEMPLATE,
    /**< Command-related event: reports the result of the read request. */
  WMS_MSG_EVENT_WRITE_TEMPLATE,
    /**< Command-related event: reports the status of the send request.
         In insert mode, the new message index is returned; otherwise, the 
         original index is returned. */
  WMS_MSG_EVENT_DELETE_TEMPLATE,
    /**< Command-related event: reports the result of the delete template 
         request. */
  WMS_MSG_EVENT_DELETE_TEMPLATE_ALL,
    /**< Command-related event: reports the result of the delete all templates 
         request. */
  WMS_MSG_EVENT_READ_STS_REPORT,
    /**< Command-related event: reads the status report event. */
  WMS_MSG_EVENT_WRITE_STS_REPORT,
    /**< Command-related event: reports the status of the send request. 
         In insert mode, the new message index is returned; otherwise, the 
         original index is returned. */
  WMS_MSG_EVENT_DELETE_STS_REPORT,
    /**< Command-related event: reports the result of the Delete Status report 
       request. */
  WMS_MSG_EVENT_DELETE_STS_REPORT_ALL,
    /**< Command-related event: reports the result of the Delete All Status 
         Reports request. */

  /* Events from lower layer  */
  WMS_MSG_EVENT_RECEIVED_MESSAGE,
    /**< Event from the lower layer: message was received at the lower level. */
  WMS_MSG_EVENT_SUBMIT_REPORT,
    /**< Event from the lower layer: submit or status report from the network 
         if there was no local error. The status report can be saved to the SIM. */
  WMS_MSG_EVENT_STATUS_REPORT,
    /**< Event from the lower layer: status report from the lower level. */
  WMS_MSG_EVENT_MT_MESSAGE_ERROR,
    /**< Event from the lower layer: An example is an MT message acknowledgment 
         timeout. */
  WMS_MSG_EVENT_ACK_REPORT,
    /**< Event from the lower layer: status of the client's acknowledgment to 
         an MT message. */
  WMS_MSG_EVENT_DUPLICATE_CB_PAGE,
    /**< Event from the lower layer: duplicate cell broadcast page. */
  WMS_MSG_EVENT_TRANSPORT_REG,
    /**< Event to notify clients about the registration/deregistraton of an 
         SMS Transport Layer */ 

/** @cond */
  WMS_MSG_EVENT_MAX,
  WMS_MSG_EVENT_MAX32 = 0x10000000
/** @endcond */
} wms_msg_event_e_type;
/*~ SENTINEL wms_msg_event_e_type.WMS_MSG_EVENT_MAX */

/** @ingroup messaging_group
@{ */
/** Message event information for each event.
*/
typedef union wms_msg_event_info_u
{
  wms_message_status_info_s_type     status_info;
    /**< Status information of a message that was sent/stored. */

  /*~ DEFAULT wms_msg_event_info_u.status_info */
    /* WMS_MSG_EVENT_SEND
    ** WMS_MSG_EVENT_READ
    ** WMS_MSG_EVENT_WRITE
    ** WMS_MSG_EVENT_DELETE
    ** WMS_MSG_EVENT_DELETE_ALL
    ** WMS_MSG_EVENT_MODIFY_TAG
    ** WMS_MSG_EVENT_READ_TEMPLATE
    ** WMS_MSG_EVENT_WRITE_TEMPLATE
    ** WMS_MSG_EVENT_DELETE_TEMPLATE
    ** WMS_MSG_EVENT_DELETE_TEMPLATE_ALL
    ** WMS_MSG_EVENT_READ_STS_REPORT
    ** WMS_MSG_EVENT_WRITE_STS_REPORT
    ** WMS_MSG_EVENT_DELETE_STS_REPORT
    ** WMS_MSG_EVENT_DELETE_STS_REPORT_ALL    */

  wms_ack_info_s_type                ack_info; 
    /**< Acknowledgment request information for a message event. */
    /* WMS_MSG_EVENT_ACK    */
 /*~ CASE WMS_MSG_EVENT_ACK wms_msg_event_info_u.ack_info */

  wms_mt_message_info_s_type         mt_message_info; 
    /**< Information about an MT message received from the network. */
    /* WMS_MSG_EVENT_RECEIVED_MESSAGE
    ** WMS_MSG_EVENT_STATUS_REPORT    */
 /*~ IF (_DISC_ == WMS_MSG_EVENT_RECEIVED_MESSAGE || _DISC_ == WMS_MSG_EVENT_STATUS_REPORT) 
     wms_msg_event_info_u.mt_message_info */
 
  wms_submit_report_info_s_type      submit_report_info; 
    /**< Submit report information for a message event. */
    /* WMS_MSG_EVENT_SUBMIT_REPORT    */
 /*~ CASE WMS_MSG_EVENT_SUBMIT_REPORT wms_msg_event_info_u.submit_report_info */

  wms_mt_message_error_s_type        mt_message_error; 
    /**< Information about an error regarding an MT message received from the 
         network. */
    /* WMS_MSG_EVENT_MT_MESSAGE_ERROR - Applicable to GW only    */
 /*~ CASE WMS_MSG_EVENT_MT_MESSAGE_ERROR wms_msg_event_info_u.mt_message_error */

  wms_ack_report_info_s_type         ack_report_info; 
    /**< Acknowledgment report information for a message event. */
    /* WMS_MSG_EVENT_ACK_REPORT    */
 /*~ CASE WMS_MSG_EVENT_ACK_REPORT wms_msg_event_info_u.ack_report_info */

  wms_gw_cb_page_header_s_type       cb_page_header; 
    /**< GSM/WCDMA cell broadcast decoded page header. */
    /* WMS_MSG_EVENT_DUPLICATE_CB_PAGE    */
 /*~ CASE WMS_MSG_EVENT_DUPLICATE_CB_PAGE wms_msg_event_info_u.cb_page_header */

  wms_msg_transport_reg_info_s_type  transport_reg_info;
    /**< Transport registration information for a message. */
    /* WMS_MSG_EVENT_TRANSPORT_REG    */
 /*~ CASE WMS_MSG_EVENT_TRANSPORT_REG wms_msg_event_info_u.transport_reg_info */

} wms_msg_event_info_s_type;

/** @ingroup messaging_group
@{ */
/** Information about a message event callback.
*/
typedef void (*wms_msg_event_cb_type)
(
  wms_msg_event_e_type         event,     /**< Event type. */
  wms_msg_event_info_s_type    *info_ptr, /**< Event information. */
  boolean                      *shared    /**< Indicates whether the message can 
                                               be passed to other clients. */
);
/*~ PARAM IN info_ptr POINTER DISC event*/
/*~ PARAM INOUT shared   POINTER */
/*~ CALLBACK wms_msg_event_cb_type
    ONERROR return  */

/** @ingroup messaging_group
@{ */
/** SMS/EMS transport layer network registration status callback
*/
typedef void (*wms_msg_transport_status_cb_type)
(
   wms_msg_transport_status_s_type   *status_ptr
);
/*~ PARAM IN status_ptr POINTER */
/*~ CALLBACK wms_msg_transport_status_cb_type
    ONERROR return  */

/** @ingroup messaging_group
@{ */
/** Information about the message cache size.
*/
#define WMS_MSG_INFO_CACHE_SIZE    30

/** @ingroup messaging_group
@{ */
/** Information about a message event cache callback.
*/
typedef void (*wms_msg_info_cache_cb_type)
(
  const wms_client_message_s_type         * msg_ptr,  /**< Pointer to the message 
                                                           input from the WMS. */
  uint8                                   * cache_ptr /**< Output from the client. */
);
/*~ PARAM IN msg_ptr     POINTER */
/*~ PARAM INOUT cache_ptr ARRAY WMS_MSG_INFO_CACHE_SIZE */ 
/*~ CALLBACK wms_msg_info_cache_cb_type 
    ONERROR return  */

/** @ingroup messaging_group
@{ */
/** Specific parsing criteria provided by the client for message events.\ 
  If a client registers this callback to WMS, this callback is called each time 
  there is a Mobile Terminated message. The client is to examine the message 
  pointed to by msg_ptr and return TRUE or FALSE in the shared parameter.
   - TRUE -- The client does not want to consume the message.\ The WMS 
     continues with normal processing.
   - FALSE -- The client wants to consume the message. The WMS delivers 
     the message only to this client using route TRANSFER_ONLY.\ The client is 
     responsible for an SMS acknowledgment.
*/
typedef void (*wms_cl_parse_msg_cb_type)
(
  const wms_client_message_s_type         *msg_ptr,
    /**< Pointer to the message input from the WMS. */
  boolean                                 *shared 
    /**< Indicates whether the message can be passed to other clients. */
);
/*~ PARAM IN msg_ptr  POINTER */
/*~ PARAM INOUT shared   POINTER */
/*~ CALLBACK wms_cl_parse_msg_cb_type 
    ONERROR return  */

/** @ingroup dc_group 
@{ */
/** Types of Dedicated Channel (DC) events.
*/
typedef enum
{
  WMS_DC_EVENT_INCOMING   = 0,  /**< Incoming call. */
  WMS_DC_EVENT_CONNECTED,       /**< Dedicated channel has been connected. */
  WMS_DC_EVENT_ABORTED,         /**< Transmission was aborted. */
  WMS_DC_EVENT_DISCONNECTED,    /**< Dedicated channel has been disconnected. */
  WMS_DC_EVENT_CONNECTING,      /**< Dedicated channel is connecting. */
  WMS_DC_EVENT_ENABLE_AUTO_DISCONNECT,  /**< Auto-disconnect timer has been set. */
  WMS_DC_EVENT_DISABLE_AUTO_DISCONNECT, /**< Auto-disconnect timer has been 
                                             disabled. */

/** @cond */
  WMS_DC_EVENT_MAX,
  WMS_DC_EVENT_MAX32 = 0x10000000
/** @endcond */
} wms_dc_event_e_type;

/** @ingroup dc_group 
@{ */
/** Dedicated Channel event information.
*/
typedef struct wms_dc_event_info_s
{
  wms_dc_so_e_type    so;               /**< Service option. */
  uint32              idle_timeout;     /**< Idle timeout value. */
} wms_dc_event_info_s_type;

/** @ingroup dc_group 
@{ */
/** DC event callback.
*/
typedef void (*wms_dc_event_cb_type)
(
  wms_dc_event_e_type         event,    /**< Type of dedicated channel event. */
  wms_dc_event_info_s_type    *info_ptr /**< DC event information. */
);
/*~ PARAM IN info_ptr POINTER */
/*~ CALLBACK wms_dc_event_cb_type 
    ONERROR return  */

/** @ingroup dc_group 
@{ */
/** Dedicated Channel disconnect cause type.
*/
typedef enum
{
   WMS_DC_DISCONNECT_CAUSE_NORMAL,           /**< &nbsp; */
   WMS_DC_DISCONNECT_CAUSE_CALL_IN_PROGRESS, /**< Other call is in progress. */
/** @cond */
   WMS_DC_DISCONNECT_CAUSE_MAX,
   WMS_DC_DISCONNECT_CAUSE_MAX32 = 0x10000000
/** @endcond */
} wms_dc_disconnect_cause_e_type;

/** @ingroup dc_group 
@{ */
/** Dedicated Channel disconnect information type.
*/
typedef struct 
{
  wms_dc_so_e_type                so;           /**< Service Option type. */
  uint32                          idle_timeout; /**< Idle timout in milliseconds. */
  wms_dc_disconnect_cause_e_type  disconnect_cause; /**< Disconnect cause. */
} wms_dc_disconnect_info_s_type;   

/** @ingroup dc_group 
@{ */
/** Dedicated Channel disconnect callback.
*/
typedef void (*wms_dc_disconnect_cb_type)
(
   wms_dc_disconnect_info_s_type    *disconnect_info_ptr
     /**< Disconnect information.*/
);
/*~ PARAM IN disconnect_info_ptr POINTER */
/*~ CALLBACK wms_dc_disconnect_cb_type
    ONERROR return  */

/** @ingroup mmbroadcast_group 
@{ */
/** Maximum size of the Multimode Broadcast configuration table. */
#define WMS_BC_MM_TABLE_MAX        190

/* ---------- Data definitions ------------ */
/** @ingroup mmbroadcast_group 
@{ */
/** Carrier's multimode broadcast configuration.
*/
typedef enum
{
  WMS_BC_CONFIG_DISALLOW,    /**< Disallow the carrier's configuration table. */
  WMS_BC_CONFIG_ALLOW_TABLE, /**< Allows the carrier's configuration table. */
  WMS_BC_CONFIG_ALLOW_ALL,   /**< Allows all configurations.\ CDMA only. */
/** @cond */
  WMS_BC_CONFIG_MAX32 = 0x10000000
/** @endcond */
} wms_bc_config_e_type;

/** @ingroup mmbroadcast_group 
@{ */
/** User preferences for multimode broadcast.
*/
typedef enum
{
  WMS_BC_PREF_DEACTIVATE,     /**< Deactivate user language preferences. */
  WMS_BC_PREF_ACTIVATE_TABLE, /**< For GSM/WCDMA CB, use language preferences. */
  WMS_BC_PREF_ACTIVATE_ALL,   /**< For GSM/WCDMA CB, ignore language preferences. */
/** @cond */
  WMS_BC_PREF_MAX,
  WMS_BC_PREF_MAX32 = 0x10000000
/** @endcond */
} wms_bc_pref_e_type;

/** @ingroup mmbroadcast_group 
@{ */
/** Range of services for multimode broadcast. 
*/
typedef struct wms_gw_cb_srv_range_s
{
  /* 'from' is <= 'to' */
  wms_gw_cb_srv_id_type    from; /**< Lower end of range of services. */
  wms_gw_cb_srv_id_type    to;   /**< Upper end of range of services. */
} wms_gw_cb_srv_range_s_type;

/** @ingroup mmbroadcast_group 
@{ */
/** Service ID for multimode broadcast.
*/
typedef struct wms_bc_mm_srv_id_u
{
  wms_bc_service_id_s_type       bc_srv_id;   /**< Multimode broadcast service 
                                                   ID. */
  wms_gw_cb_srv_range_s_type     gw_cb_range; /**< Range of services for 
                                                   GSM/WCDMA. */
} wms_bc_mm_srv_id_type;

/** @ingroup mmbroadcast_group 
@{ */
/** Multimode broadcast service table entry.
*/
typedef struct wms_bc_mm_service_info_s
{
  wms_bc_mm_srv_id_type           srv_id;   
    /**< Service ID for multimode broadcast. */
  boolean                         selected; 
    /**< Indicates whether this entry is selected. */
  wms_priority_e_type             priority; 
    /**< Priority for sending a broadcast message. For GSM/WCDMA CB, it is 
         always Normal. */
  char                            label[WMS_BC_MM_SRV_LABEL_SIZE];
    /**< Service label. */
  wms_user_data_encoding_e_type   label_encoding;
    /**< Encoding type for the service label. If it is ASCII or IA5, each byte 
         of the label has one character. Otherwise, the label has the raw bytes 
         to be decoded by the clients. */
  wms_bc_alert_e_type             alert;  
    /**< Alert options for this broadcast service. */
  uint8                            max_messages; 
    /**< Maximum number of messages that can be stored for this service. */
} wms_bc_mm_service_info_s_type;

/** @ingroup mmbroadcast_group 
@{ */
/** Multimode broadcast service table.
*/
typedef struct wms_bc_mm_table_s
{
  uint16                           size;      /**< Size of the table. */
  wms_bc_mm_service_info_s_type    * entries; /**< Table entries. */
} wms_bc_mm_table_s_type;
/*~ FIELD wms_bc_mm_table_s.entries VARRAY LENGTH wms_bc_mm_table_s.size */

/** @ingroup mmbroadcast_group 
@{ */
/** Array of multimode broadcast service IDs.
*/
typedef struct wms_bc_mm_service_ids_s
{
  uint16                       size;      /**< Size of the array of service IDs. */
  wms_bc_mm_srv_id_type        * entries; /**< Array entries. */
} wms_bc_mm_service_ids_s_type;
/*~ FIELD wms_bc_mm_service_ids_s.entries VARRAY LENGTH wms_bc_mm_service_ids_s.size */

/** @ingroup mmbroadcast_group 
@{ */
/** Reading preferences for multimode broadcast.
*/
typedef struct wms_bc_mm_reading_pref_s
{
  boolean                      reading_advised;   /**< Indicates whether reading 
                                                       is advised. */
  boolean                      reading_optional;  /**< Indicates whether reading 
                                                       is optional. */
} wms_bc_mm_reading_pref_s_type;

/** @ingroup mmbroadcast_group 
@{ */
/** Multimode broadcast events.
*/
typedef enum
{
  WMS_BC_MM_EVENT_CONFIG,      /**< Sets the multimode broadcast configuration. */
  WMS_BC_MM_EVENT_PREF,        /**< Sets the user preferences. */
  WMS_BC_MM_EVENT_TABLE,       /**< Modifies the multimode broadcast service 
                                    table.*/
  WMS_BC_MM_EVENT_SRV_IDS,     /**< Modifies the array of multimode broadcast 
                                    service IDs. */
  WMS_BC_MM_EVENT_SRV_INFO,    /**< Gets the service ID information. */
  WMS_BC_MM_EVENT_SRV_UPDATED, /**< Updates the service ID. */
  WMS_BC_MM_EVENT_ADD_SRVS,    /**< Adds services to the service ID array. */
  WMS_BC_MM_EVENT_DELETE_SRVS, /**< Deletes services from the service ID array. */
  WMS_BC_MM_EVENT_SELECT_ALL_SRVS,       /**< Selects all service IDs. */
  WMS_BC_MM_EVENT_SET_PRIORITY_ALL_SRVS, /**< Sets the priority of all services. */
  WMS_BC_MM_EVENT_READING_PREF,    /**< Sets the reading preference. This 
                                        command executed successfully. */
  WMS_BC_MM_EVENT_DELETE_ALL_SRVS, /**< Deletes all services. No event data is 
                                        needed. */
  WMS_BC_MM_EVENT_ENABLE_FAILURE,  /**< Enables a lower layer (CM and CP) 
                                        failure. */
  WMS_BC_MM_EVENT_DISABLE_FAILURE, /**< Disables a lower layer (CM and CP) 
                                        failure. */
/** @cond */
  WMS_BC_MM_EVENT_MAX,
  WMS_BC_MM_EVENT_MAX32 = 0x10000000
/** @endcond */
} wms_bc_mm_event_e_type;

/** @ingroup mmbroadcast_group 
@{ */
/** Information about multimode broadcast events.
*/
typedef struct wms_bc_mm_event_info_s
{
 wms_message_mode_e_type         message_mode; 
   /**< System mode to be used for a message. */
 wms_bc_mm_event_e_type          event;        
   /**< Multimode broadcast event. */

 /** Used by the wms_bc_mm_event_info_s_type structure.
 */
 union wms_bc_mm_event_info_u
 {
  wms_bc_config_e_type            bc_config;   
    /**< Multimode broadcast configuration. */
    /* WMS_BC_MM_EVENT_CONFIG */
  /*~ IF (_DISC_ == WMS_BC_MM_EVENT_CONFIG) wms_bc_mm_event_info_u.bc_config */

  wms_bc_pref_e_type              bc_pref;     
    /**< User preferences for multimode broadcast. */
    /* WMS_BC_MM_EVENT_PREF */
  /*~ IF (_DISC_ == WMS_BC_MM_EVENT_PREF) wms_bc_mm_event_info_u.bc_pref */

  wms_bc_mm_table_s_type          bc_mm_table; 
    /**< Multimode broadcast service table. */
    /* WMS_BC_MM_EVENT_TABLE */
    /* WMS_BC_MM_EVENT_ADD_SRVS */
    /* WMS_BC_MM_EVENT_SELECT_ALL_SRVS */
    /* WMS_BC_MM_EVENT_SET_PRIORITY_ALL_SRVS */
  /*~ IF ( _DISC_ == WMS_BC_MM_EVENT_TABLE || _DISC_ == WMS_BC_MM_EVENT_ADD_SRVS 
          || _DISC_ == WMS_BC_MM_EVENT_SELECT_ALL_SRVS || _DISC_ == WMS_BC_MM_EVENT_SET_PRIORITY_ALL_SRVS ) 
      wms_bc_mm_event_info_u.bc_mm_table */

  wms_bc_mm_service_ids_s_type    bc_mm_srv_ids; 
    /**< Array of multimode broadcast service IDs. */
    /* WMS_BC_MM_EVENT_SRV_IDS */
    /* WMS_BC_MM_EVENT_DELETE_SRVS */
  /*~ IF (_DISC_ == WMS_BC_MM_EVENT_SRV_IDS || _DISC_ == WMS_BC_MM_EVENT_DELETE_SRVS)
      wms_bc_mm_event_info_u.bc_mm_srv_ids */

  wms_bc_mm_service_info_s_type   bc_mm_srv_info; 
    /**< Multimode broadcast service table entry. */
    /* WMS_BC_MM_EVENT_SRV_INFO */
    /* WMS_BC_MM_EVENT_SRV_UPDATED */
  /*~ IF (_DISC_ == WMS_BC_MM_EVENT_SRV_INFO || _DISC_ == WMS_BC_MM_EVENT_SRV_UPDATED)
      wms_bc_mm_event_info_u.bc_mm_srv_info */

  /* no event data for WMS_BC_MM_EVENT_ALL_SRV_DELETED */

  wms_bc_mm_reading_pref_s_type   bc_mm_reading_pref; 
    /**< Reading preferences for multimode broadcast. */
    /* WMS_BC_MM_EVENT_READING_PREF */
  /*~ IF (_DISC_ == WMS_BC_MM_EVENT_READING_PREF) wms_bc_mm_event_info_u.bc_mm_reading_pref */

  wms_status_e_type               bc_enable_error;  
    /**< Command or request status messages. */
    /* WMS_BC_MM_EVENT_ENABLE_FAILURE */
    /* WMS_BC_MM_EVENT_DISABLE_FAILURE */
  /*~ IF (_DISC_ == WMS_BC_MM_EVENT_ENABLE_FAILURE || _DISC_ == WMS_BC_MM_EVENT_DISABLE_FAILURE)
      wms_bc_mm_event_info_u.bc_enable_error */

  /*~ DEFAULT wms_bc_mm_event_info_u.void */

 } u;  /*~ FIELD wms_bc_mm_event_info_s.u DISC _OBJ_.event */
} wms_bc_mm_event_info_s_type;

/** @ingroup mmbroadcast_group 
@{ */
/** Information about multimode broadcast event callbacks.
*/
typedef void (*wms_bc_mm_event_cb_type)
(
  wms_bc_mm_event_e_type         event,    /**< Multimode broadcast event type. */
  wms_bc_mm_event_info_s_type    *info_ptr /**< Information about a multimode 
                                                broadcast event. */
);
/*~ PARAM IN info_ptr POINTER */
/*~ CALLBACK wms_bc_mm_event_cb_type 
    ONERROR return  */

/** @ingroup debug_group
@{ */
/** Debug events.
*/
typedef enum
{
  WMS_DBG_EVENT_RESET_TL_SEQ_NUM,       /**< Resets the Transport Layer 
                                             sequence number. */
  WMS_DBG_EVENT_SET_SIM_MESSAGE_NUMBER, /**< Sets the SIM message number. */
  WMS_DBG_EVENT_CLEAR_SMMA_FLAG,        /**< Clears the Short Message Memory 
                                             Available flag. */
  WMS_DBG_EVENT_RETRY_INTERVAL,         /**< Retry interval has been retrieved 
                                             or changed. */
  WMS_DBG_EVENT_RETRY_PERIOD,           /**< Retry period has been retrieved 
                                             or changed. */                                           
/** @cond */
  WMS_DBG_EVENT_MAX,
  WMS_DBG_EVENT_MAX32 = 0x10000000
/** @endcond */
} wms_dbg_event_e_type;
/*~ SENTINEL wms_dbg_event_e_type.WMS_DBG_EVENT_MAX */

/** @ingroup debug_group
@{ */
/** Information about a debug event.
*/
typedef union wms_dbg_event_info_u
{
  uint32         retry_interval;   /**< Value is provided with 
                                        WMS_DBG_EVENT_RETRY_INTERVAL event. */

/*~ CASE WMS_DBG_EVENT_RETRY_INTERVAL wms_dbg_event_info_u.retry_interval */
                                  
  
 uint32         retry_period;     /**< Value is provided with 
                                        WMS_DBG_EVENT_RETRY_PERIOD event. */
                                        
/*~ CASE WMS_DBG_EVENT_RETRY_PERIOD   wms_dbg_event_info_u.retry_period   */

/*~ DEFAULT wms_dbg_event_info_u.void */
} wms_dbg_event_info_s_type;

/** @ingroup debug_group
@{ */
/** Information about a debug event callback.
*/
typedef void (*wms_dbg_event_cb_type)
(
  wms_dbg_event_e_type         event,    /**< Type of debug event. */
  wms_dbg_event_info_s_type    *info_ptr /**< Information about the debug event. */
);
/*~ PARAM IN info_ptr POINTER DISC event */
/*~ CALLBACK wms_dbg_event_cb_type 
    ONERROR return  */

/*=========================================================================
FUNCTION wms_client_init
===========================================================================*/
/**
  Initializes a client so the API can allocate resources for the client.

  @param client_type  [IN]  Client type.
  @param *client_id_ptr [OUT] Pointer to the returned client ID.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  None.

  @sideeffects
  Internal data is initialized for the new client.
*/
wms_client_err_e_type wms_client_init
(
  wms_client_type_e_type   client_type,
  wms_client_id_type       *client_id_ptr
);
/*~ FUNCTION wms_client_init
    RELEASE_FUNC wms_client_release(*client_id_ptr)
    ONERROR return WMS_CLIENT_ERR_RPC */
/*~ PARAM INOUT client_id_ptr POINTER */

/*=========================================================================
FUNCTION  wms_client_release
===========================================================================*/
/**
  Releases a client so the API can release the client's resources.

  @param client_id    [IN]  Client ID.

  @return
  Client error code.   
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  None.

  @sideeffects
  Internal data is reset for this client.
*/
wms_client_err_e_type wms_client_release
(
  wms_client_id_type       client_id
);
/*~ FUNCTION wms_client_release 
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_activate
===========================================================================*/
/**
  Activates a client so the client can start using services from the API.

  @param client_id    [IN]  Client ID.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  None.

  @sideeffects
  Internal data is changed.
*/
wms_client_err_e_type wms_client_activate
(
  wms_client_id_type       client_id
);
/*~ FUNCTION wms_client_activate 
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_deactivate
===========================================================================*/
/**
  Deactivates a client so the client can stop using services from the API.

  @param client_id    [IN]  Client ID.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  None.

  @sideeffects
  Internal data is changed.
*/
wms_client_err_e_type wms_client_deactivate
(
  wms_client_id_type       client_id
);
/*~ FUNCTION wms_client_deactivate 
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_reg_cfg_cb
===========================================================================*/
/**
  Registers/de-registers a client's configuration event callback function 
  pointer.

  If a NULL pointer is passed, the callback is de-registered. Otherwise, it is 
  registered.

  @param client_id    [IN]  Client ID.
  @param cfg_event_cb [IN]  Configuration event callback function pointer.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  Client must have been initialized.

  @sideeffects
  Internal data is updated.
*/
wms_client_err_e_type wms_client_reg_cfg_cb
(
  wms_client_id_type       client_id,
  wms_cfg_event_cb_type    cfg_event_cb
);
/*~ FUNCTION wms_client_reg_cfg_cb 
    RELEASE_FUNC wms_client_deactivate(client_id) 
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_reg_msg_cb
===========================================================================*/
/**
  Registers/de-registers the message event callback function pointer.

  If a NULL pointer is passed, the callback is de-registered. Otherwise, it is 
  registered.

  @param client_id    [IN]  Client ID.
  @param msg_event_cb [IN]  Message event callback function pointer.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  Client must have been initialized.

  @sideeffects
  Internal data is updated.
*/
wms_client_err_e_type wms_client_reg_msg_cb
(
  wms_client_id_type       client_id,
  wms_msg_event_cb_type    msg_event_cb
);
/*~ FUNCTION wms_client_reg_msg_cb 
    RELEASE_FUNC wms_client_deactivate(client_id) 
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_reg_transport_status_cb
===========================================================================*/
/**
  Registers/de-registers callback function pointer to get transport layer
  status.

  If a NULL pointer is passed, the callback is de-registered. Otherwise, it is 
  registered.

  @param client_id    [IN]  Client ID.
  @param transport_status_cb [IN]  Transport status callback function pointer.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  Client must have been initialized.

  @sideeffects
  Internal data is updated.
*/
wms_client_err_e_type wms_client_reg_transport_status_cb
(
  wms_client_id_type                 client_id,
  wms_msg_transport_status_cb_type   transport_status_cb
);

/*~ FUNCTION wms_client_reg_transport_status_cb 
    RELEASE_FUNC wms_client_deactivate(client_id) 
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_reg_msg_info_cache_cb
===========================================================================*/
/**
  Registers the message information cache callback function for storing the 
  application's message list display information.

  The client can register its callback function so the client can tell the WMS 
  how to cache the message information for the messages from the memory. If a 
  NULL pointer is passed, the callback is de-registered. No message information 
  is cached.

  @note Only one cache callback is allowed.

  @param msg_info_cache_cb [IN]  Message information cache callback function 
                                   pointer.

  @return
  None.

  @dependencies
  This callback is to be registered in the early stage of the power-up 
  initialization so that WMS can use this callback to cache the information 
  during initialization.

  @sideeffects
  Internal data is updated.
*/
void wms_reg_msg_info_cache_cb
(
  wms_msg_info_cache_cb_type    msg_info_cache_cb
);
/*~ FUNCTION wms_reg_msg_info_cache_cb 
    ONERROR return */

/*=========================================================================
FUNCTION  wms_client_reg_parse_msg_cb
===========================================================================*/
/**
  Registers the message parsing/filter callback function from a client.

  The client can register its callback function so when an incoming message 
  arrives the WMS can query this client and determine whether this client is 
  to consume the message.
   - If TRUE, the incoming message is delivered only to this client.
   - If FALSE, the incoming message is delivered to all clients.

  The client order that is defined in wms_client_type_e_type is used for 
  querying all clients.

  @param client_id    [IN]  Client ID.
  @param msg_parsing_cb [IN]  Message parsing callback function pointer.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  Client must have been initialized.

  @sideeffects
  Depending on the criteria of the parsing being administered to the message, 
  the message cannot be shared amoung other clients.
*/
wms_client_err_e_type wms_client_reg_parse_msg_cb
(
  wms_client_id_type               client_id,
  wms_cl_parse_msg_cb_type         msg_parsing_cb
);
/*~ FUNCTION wms_client_reg_parse_msg_cb 
    RELEASE_FUNC wms_client_deactivate(client_id)
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_reg_dc_cb
===========================================================================*/
/**
  Registers/de-registers the dedicated channel event callback function pointer.
 
  If a NULL pointer is passed, the callback is de-registered. Otherwise, it is 
  registered.

  @param client_id    [IN]  Client ID.
  @param dc_event_cb  [IN]  DC event callback function pointer.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  Client must have been initialized.

  @sideeffects
  Internal data is updated.
*/
wms_client_err_e_type wms_client_reg_dc_cb
(
  wms_client_id_type       client_id,
  wms_dc_event_cb_type     dc_event_cb
);
/*~ FUNCTION wms_client_reg_dc_cb 
    RELEASE_FUNC wms_client_deactivate(client_id) 
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_reg_dc_disconnect_cb
===========================================================================*/
/**
  Registers/de-registers the DC disconnect callback function pointer.\ If a NULL 
  pointer is passed, the callback is de-registered.

  @param client_id    [IN]  Client ID.
  @param dc_disconnect_cb [IN]  DC disconnect callback function pointer.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  Client must have been initialized.

  @sideeffects
  Internal data is updated.
*/
wms_client_err_e_type wms_client_reg_dc_disconnect_cb
(
  wms_client_id_type            client_id,
  wms_dc_disconnect_cb_type     dc_disconnect_cb
);

/*~ FUNCTION wms_client_reg_dc_disconnect_cb
    RELEASE_FUNC wms_client_deactivate(client_id) 
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_reg_bc_mm_cb
===========================================================================*/
/**
  Registers/de-registers a multimode broadcast event callback function pointer.

  If a NULL pointer is passed, the callback is de-registered. Otherwise, it is 
  registered.

  @param client_id    [IN]  Client ID.
  @param bc_event_cb  [IN]  Multimode BROADCAST event callback function pointer.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  Client must have been initialized.

  @sideeffects
  Internal data is updated.
*/
wms_client_err_e_type wms_client_reg_bc_mm_cb
(
  wms_client_id_type          client_id,
  wms_bc_mm_event_cb_type     bc_event_cb
);
/*~ FUNCTION wms_client_reg_bc_mm_cb 
    RELEASE_FUNC wms_client_deactivate(client_id) 
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_reg_dbg_cb
===========================================================================*/
/**
  Registers/de-registers the debug (DBG) event callback function pointer.

  If a NULL pointer is passed, the callback is de-registered. Otherwise, it is 
  registered.

  @param client_id    [IN]  Client ID.
  @param dbg_event_cb [IN]  Debug event callback function pointer.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  Client must have been initialized.

  @sideeffects
  Internal data is updated.
*/
wms_client_err_e_type wms_client_reg_dbg_cb
(
  wms_client_id_type       client_id,
  wms_dbg_event_cb_type    dbg_event_cb
);
/*~ FUNCTION wms_client_reg_dbg_cb 
    RELEASE_FUNC wms_client_deactivate(client_id) 
    ONERROR return WMS_CLIENT_ERR_RPC */

/*===========================================================================
FUNCTION_new wms_client_activate_bearer
===========================================================================*/
/**
  Activates a specified bearer.

  @param client_id    [IN]  Client ID.
  @param bearer       [IN]  Specified bearer.

  @return
  Client error code. 
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  None.

  @sideeffects
  Internal data is updated.
*/
wms_client_err_e_type wms_client_activate_bearer
(
  wms_client_id_type                client_id,
  wms_bearer_e_type                 bearer
);
/*~ FUNCTION wms_client_activate_bearer
    ONERROR return WMS_CLIENT_ERR_RPC */

/*===========================================================================
FUNCTION_new wms_client_deactivate_bearer
===========================================================================*/
/**
  Deactivates a specified bearer. 

  @param client_id    [IN]  Client ID.
  @param bearer       [IN]  Specified bearer.

  @return
  Client error code.
   - WMS_CLIENT_ERR_NONE
   - WMS_CLIENT_ERR_CLIENT_ID

  @dependencies
  None.

  @sideeffects
  Internal data is updated.
*/
wms_client_err_e_type wms_client_deactivate_bearer
(
  wms_client_id_type                client_id,
  wms_bearer_e_type                 bearer
);
/*~ FUNCTION wms_client_deactivate_bearer 
    ONERROR return WMS_CLIENT_ERR_RPC */

/* <EJECT> */
/*===========================================================================

                          Configuration Group

                        API FUNCTION DEFINITIONS

===========================================================================*/
/**
@ingroup config_group
@{ */
/*=========================================================================
FUNCTION   wms_cfg_set_routes
==========================================================================*/
/**
  Changes the message routing configuration so the incoming messages are routed 
  accordingly (e.g., whether to store a message or deliver it to the clients).

  The overlapping parts of the old configuration are replaced with those parts 
  in the new configuration. For example, the old configuration says CLASS 2 
  point-to-point messages are stored in the SIM, but the new configuration can 
  override that, stating that CLASS 2 point-to-point messages are delivered to 
  the clients. 

  The WMS keeps the routing configuration in NV. During powerup, the 
  configuration read from NV is used, until the clients update it. Each time it 
  is changed, it is updated in NV.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param *routes_ptr  [IN]  Pointer to the routing configuration.

  @return
  Status of the request. 
   - WMS_OK_S             
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_CFG_NV_WRITE\n
  WMS_CMD_ERR_CFG_ROUTE\n
  WMS_CMD_ERR_CFG_MEM_STORE\n
  WMS_CMD_ERR_CFG_MSG_CLASS

  @events
  WMS_CFG_EVENT_ROUTES

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_cfg_set_routes
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data,
  const wms_routes_s_type          * routes_ptr
);
/*~ FUNCTION wms_cfg_set_routes 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN routes_ptr POINTER */

/*=========================================================================
FUNCTION  wms_cfg_get_routes
===========================================================================*/
/**
  Retrieves the message routing configuration.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE

  @events
  WMS_CFG_EVENT_ROUTES

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_cfg_get_routes
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data
);
/*~ FUNCTION wms_cfg_get_routes 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_cfg_get_memory_status
===========================================================================*/
/**
  Retrieves the memory usage status of a memory store.

  If tag == WMS_TAG_NONE, all used slots are calculated Otherwise, only messages 
  having this tag value are counted as used_tag_slots.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to get the status.
  @param tag          [IN]  Type of messages for which memory usage status is 
                            returned.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_CFG_MEM_STORE

  @events
  WMS_CFG_EVENT_MEMORY_STATUS

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_cfg_get_memory_status
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data,
  wms_memory_store_e_type          mem_store,
  wms_message_tag_e_type           tag
);
/*~ FUNCTION wms_cfg_get_memory_status 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_cfg_get_message_list
===========================================================================*/
/**
  Retrieves the message list of a memory store for a certain type of message 
  according to the message tag.

  If tag == WMS_TAG_NONE, all types of messages are included in the list. 
  Otherwise, only messages having this tag value are included.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to get the message list.
  @param tag          [IN]  Type of messages to be listed.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE

  @events
  WMS_CFG_EVENT_MEMORY_STATUS

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_cfg_get_message_list
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data,
  wms_memory_store_e_type          mem_store,
  wms_message_tag_e_type           tag
);
/*~ FUNCTION wms_cfg_get_message_list 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_cfg_set_gw_domain_pref
===========================================================================*/
/**
  Sets the preference for GSM/GPRS/UMTS Mobile Originated SMS domains.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                             this transaction of sending a message. The same 
                             pointer is passed to the client's callback function. 
                             A NULL pointer is acceptable.
  @param gw_domain_pref [IN]  New domain preference.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Undetermined.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_CFG_NV_WRITE
  
  @events
  WMS_CFG_EVENT_GW_DOMAIN_PREF

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_cfg_set_gw_domain_pref
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data,
  wms_gw_domain_pref_e_type         gw_domain_pref
);
/*~ FUNCTION wms_cfg_set_gw_domain_pref 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_cfg_get_gw_domain_pref
===========================================================================*/
/**
  Requests the current preference for GSM/GPRS/UMTS Mobile Originated SMS 
  domains.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_cfg_get_gw_domain_pref
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data
);
/*~ FUNCTION wms_cfg_get_gw_domain_pref 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_cfg_set_primary_client
===========================================================================*/
/**
  Sets the client as the primary WMS client for handling incoming messages and 
  acknowledging them if they are not already acknowledged by the WMS.

  The client can also specify if the client memory is to be used, which can be 
  set to Full/Available using wms_cfg_set_memory_full().

  The advantage of being a primary client is that the memory status is checked 
  for memory full and an SMMA Request is sent to the network. Once a primary 
  client is set, no other client can become the primary client. If 
  set_primay == FALSE, the primary client is de-registered.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param set_primary  [IN]  Indicates whether to set/reset the primary client.
  @param use_client_memory [IN]  Indicates whether to use client's memory store.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_CFG_NV_WRITE\n
  WMS_CMD_ERR_NO_RESOURCE

  @events
  WMS_CFG_EVENT_PRIMARY_CLIENT_SET

  @dependencies
  Client must have been initialized.

  @sideeffects
  A command is sent to the WMS task.

@sa  wms_cfg_set_memory_full()
*/
wms_status_e_type wms_cfg_set_primary_client
(
  wms_client_id_type      client_id,
  wms_cmd_cb_type         cmd_cb,
  const void            * user_data,
  boolean                 set_primary,
  boolean                 use_client_memory
);
/*~ FUNCTION wms_cfg_set_primary_client 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_cfg_set_memory_full
===========================================================================*/
/**
  Sets the memory status to full for the primary client memory. 

  The client can set itself as the primary WMS client using 
  wms_cfg_set_primary_client(). Only the primary client can set/reset the 
  memory status.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param memory_full  [IN]  Sets/resets the memory full flag of the client memory.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_CFG_NV_WRITE\n
  WMS_CMD_ERR_NO_RESOURCE

  @events
  WMS_CFG_EVENT_MEMORY_STATUS_SET

  @dependencies
  Client must have been initialized.

  @sideeffects
  A command is sent to the WMS task.

@sa  wms_cfg_set_primary()
*/
wms_status_e_type wms_cfg_set_memory_full
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                     * user_data,
  boolean                          memory_full
);
/*~ FUNCTION wms_cfg_set_memory_full 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_cfg_locate_mo_msg_for_sts_report
===========================================================================*/
/**
  Returns the index for an Mobile Originated SMS message based on the message 
  reference number in the received status report message.

  The function locates the MO message index given a message reference number 
  for that message on the SIM.

  @note This is a synchronous function call.

  @param *rec_id      [OUT] Pointer to the return result.
  @param message_ref  [IN]  Reference number for the MO message.

  @return
  TRUE -- Message index was found.\n 
  FALSE -- Message index was not found.

  @dependencies
  Client must have been initialized.
*/
boolean wms_cfg_locate_mo_msg_for_sts_report
(
  wms_message_index_type  *rec_id,      /* OUTPUT */
  wms_message_number_type  message_ref  /* INPUT */
);
/*~ FUNCTION wms_cfg_locate_mo_msg_for_sts_report 
    ONERROR return FALSE */
/*~ PARAM INOUT rec_id POINTER */

/*=========================================================================
FUNCTION  wms_cfg_get_sts_rpt_index
===========================================================================*/
/**
  Returns the status report's index for an Mobile Originated SMS message given 
  its index on the SIM.

  @note This is a synchronous function call.

  @param sms_index    [IN]  MO SMS message index.
  @param *smsr_index  [OUT] Pointer to the result.

  @return
  TRUE -- Message index was found.\n 
  FALSE -- Message index was not found.

  @dependencies
  Client must have been initialized.
*/
boolean wms_cfg_get_sts_rpt_index
(
  wms_message_index_type sms_index,   /* INPUT */
  wms_message_index_type *smsr_index  /* OUTPUT */
);
/*~ FUNCTION wms_cfg_get_sts_rpt_index 
    ONERROR return FALSE */
/*~ PARAM INOUT smsr_index POINTER */ 

/*=========================================================================
FUNCTION  wms_cfg_check_voicemail_contents
===========================================================================*/
/**
  Examines the content of a CDMA or GSM SMS message and determines whether the 
  message has voice mail information.

  @note This is a synchronous function call.

  @param msg_mode     [IN]  CDMA or GSM/WCDMA message mode.
  @param *message     [IN]  Pointer to the raw bearer data or TPDU.
  @param *is_voicemail_active [OUT] Indicates whether to voice mail status is 
                                     active.
  @param *count       [OUT] How many voice mails are waiting.

  @return
  TRUE -- Message is a voice mail message.\n 
  FALSE -- Message is not a voice mail message.

  @dependencies
  None.

  @sideeffects
  Updates is_voicemail_active and count.
*/
boolean wms_cfg_check_voicemail_contents
(
  wms_message_mode_e_type         msg_mode,             /* INPUT */
  const wms_raw_ts_data_s_type    *message,             /* INPUT */
  boolean                         *is_voicemail_active, /* OUTPUT */
  uint8                           *count  /* OUTPUT: how many voice mails */
);
/*~ FUNCTION wms_cfg_check_voicemail_contents 
    ONERROR return FALSE */
/*~ PARAM IN message POINTER */
/*~ PARAM INOUT is_voicemail_active POINTER */
/*~ PARAM INOUT count POINTER */ 

/*=========================================================================
FUNCTION  wms_cfg_check_mwi_contents
===========================================================================*/
/**
  Checks an SMS message for message waiting indication contents.
  
  This is a synchronous function call. The last parameter indicates the number 
  of message waiting types in the SMS message. The caller can find out if the 
  array is big enough to hold all the message waiting indication information 
  by comparing the size of the passed-in array and the required array size.

  @param *message     [IN]  Pointer to the raw bearer data or TPDU.
  @param *mwi_info_arr [IN]  Pointer to the Message Waiting Indicator 
                                     information array.
  @param mwi_info_arr_size [IN]  MWI information array size.
  @param *mwi_info_arr_size_req [IN]  Pointer to the MWI information array size.

  @return
  TRUE -- Message contains MWI information.\n 
  FALSE -- Message does not contain MWI information, and mwi_info_arr_size_req 
           is set to 0.

  @dependencies
  Undetermined.

  @sideeffects
  Updates *mwi_info_arr and *mwi_info_arr_size_req if they are not NULL.
*/
boolean wms_cfg_check_mwi_contents 
( 
  const wms_raw_ts_data_s_type  *message,              
  wms_gw_mwi_info_s_type        *mwi_info_arr,         
  int                            mwi_info_arr_size,        
  int                           *mwi_info_arr_size_req 
);
/*~ FUNCTION wms_cfg_check_mwi_contents 
    ONERROR return FALSE */
/*~ PARAM IN message POINTER */
/*~ PARAM INOUT mwi_info_arr POINTER */
/*~ PARAM IN mwi_info_arr_size */
/*~ PARAM INOUT mwi_info_arr_size_req POINTER */ 

/*===========================================================================
FUNCTION wms_cfg_check_cphs_msg
===========================================================================*/
/**
  Examines the address field of a GSM SMS message and determines whether it is 
  a CPHS Message Waiting message.\ The function uses TP-OA for a voice Message 
  Waiting indicator.

  @param addr         [IN]  Pointer to the address structure.

  @return
  TRUE -- Message is a CPHS Message Waiting message.\n 
  FALSE -- Message is not a CPHS Message Waiting message.

  @dependencies
  The CPHS feature must be enabled.

  @sideeffects
  Undetermined.
*/
boolean wms_cfg_check_cphs_msg
(
   const wms_address_s_type     *addr
);
/*~ FUNCTION wms_cfg_check_cphs_msg 
    ONERROR return FALSE */
/*~ PARAM IN addr POINTER */ 

/*=========================================================================
FUNCTION  wms_cfg_check_cdma_duplicate
===========================================================================*/
/**
  Examines the content of a CDMA SMS message and determines whether it is a 
  duplicate by comparing messages in R-UIM and NV.

  @note This is a synchronous function call.

  @param *msg_ptr     [IN]  Pointer to the client message structure.

  @return
  TRUE -- Message is a duplicate.\n 
  FALSE -- Message is not a duplicate.

  @dependencies
  None.

  @sideeffects
  Undetermined.
*/
boolean wms_cfg_check_cdma_duplicate
(
  const wms_client_message_s_type     *msg_ptr
);
/*~ FUNCTION wms_cfg_check_cdma_duplicate 
    ONERROR return FALSE */
/*~ PARAM IN msg_ptr POINTER */

/*===========================================================================
FUNCTION  wms_cfg_set_link_control
===========================================================================*/
/**
  Sends a request to the lower layers to disable/enable SMS link control for 
  connection establishment and timer duration.

  Control options include the following:
  - WMS_LINK_CONTROL_DISABLED -- Disable link control so the lower layer can 
    release the link if there is no Mobile Originated SMS to send.
  - WMS_LINK_CONTROL_ENABLED_ONE -- Enable link control once so the the lower 
    layer keeps the link up for a specified time until the next MO SMS is 
    requested, or until the timer expires. After the timer expires, this option 
    is automatically changed to DISABLED.
  - WMS_LINK_CONTROL_ENABLE_ALL -- Always enable link control until this option 
    is changed by the client.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                             this transaction of sending a message. The same 
                             pointer is passed to the client's callback function. 
                             A NULL pointer is acceptable.
  @param control_option [IN]  Link control option.
  @param idle_timer   [IN]  Idle time period in seconds before the link is 
                             released.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S
   - WMS_UNSUPPORTED_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_CFG_LINK_CONTROL\n
  WMS_CMD_ERR_NO_RESOURCE

  @events
  WMS_CFG_EVENT_LINK_CONTROL
*/
wms_status_e_type wms_cfg_set_link_control
(
  wms_client_id_type                client_id,
  wms_cmd_cb_type                   cmd_cb,
  const void                       *user_data,
  wms_cfg_link_control_mode_e_type  control_option,
  uint8                             idle_timer /* in seconds */
);
/*~ FUNCTION wms_cfg_set_link_control 
    ONERROR return WMS_RPC_ERROR_S */

/*===========================================================================
FUNCTION  wms_cfg_get_link_control
===========================================================================*/
/**
  Retrieves the current link control setting.

  The WMS sends a configuration event to all clients to tell them if the link 
  is up/down and what the mode is.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S
   - WMS_UNSUPPORTED_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_NO_RESOURCE

  @events
  WMS_CFG_EVENT_LINK_CONTROL
*/
wms_status_e_type wms_cfg_get_link_control
(
  wms_client_id_type                client_id,
  wms_cmd_cb_type                   cmd_cb,
  const void                       *user_data
);
/*~ FUNCTION wms_cfg_get_link_control 
    ONERROR return WMS_RPC_ERROR_S */

/*===========================================================================
FUNCTION  wms_cfg_locate_voicemail_line2
===========================================================================*/
/**
  Returns the index of the voice mail message in the SIM for subscription line 
  2 for the CPHS Alternative Line feature.

  @param *sms_index   [OUT] Pointer to the returned index.

  @return
  TRUE -- Voice mail is active on line 2.\n 
  FALSE -- Voice mail is not active on line 2.

  @dependencies
  The CPHS feature must be enabled.
*/
boolean wms_cfg_locate_voicemail_line2
(
  wms_message_index_type        *sms_index   /* OUTPUT */
);
/*~ FUNCTION wms_cfg_locate_voicemail_line2 
    ONERROR return FALSE */
/*~ PARAM INOUT sms_index POINTER */ 

/*=========================================================================
FUNCTION  wms_cfg_check_wap_push_message
===========================================================================*/
/**
  Examines the content of an CDMA/GSM/WCDMA SMS message and determines whether 
  it is a Wireless Application Protocol (WAP) Push message.

  @note This is a synchronous function call.

  @param *msg_ptr     [IN]  Pointer to the message structure.

  @return
  TRUE -- Message is a WAP Push message.\n 
  FALSE -- Message is not a WAP Push message.

  @dependencies
  None.
*/
boolean wms_cfg_check_wap_push_message
(
  const wms_client_message_s_type     *msg_ptr
);
/*~ FUNCTION wms_cfg_check_wap_push_message 
    ONERROR return FALSE */
/*~ PARAM IN msg_ptr POINTER */ 

/* <EJECT> */
/*===========================================================================

                              Message Group

                        API FUNCTION DEFINITIONS

===========================================================================*/
/**
@ingroup messaging_group
@{ */
/*=========================================================================
FUNCTION  wms_msg_send
===========================================================================*/
/**
  Sends a Mobile Originated SMS message to the SMS entity as specified by the 
  address in the message.

  This is similar to sending a datagram to another application. The message is 
  an MO message in GSM/WCDMA and CDMA mode, or it is a command in GSM/WCDMA mode.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param send_mode    [IN]  Mode for sending the message.
  @param *message_ptr [IN]  Pointer to the client's SMS message buffer. The A
                            PI must copy the data in this buffer into its own 
                            buffer so the client can reuse it after the function 
                            returns.
  
  @return
  Status of the request.
   - WMS_OK_S 
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE\n
  WMS_CMD_ERR_MSG_INDEX\n
  WMS_CMD_ERR_MSG_EMPTY_MESSAGE\n
  WMS_CMD_ERR_MSG_TAG\n
  WMS_CMD_ERR_MSG_ENCODE\n
  WMS_CMD_ERR_MSG_DECODE\n
  WMS_CMD_ERR_MSG_SEND_MODE\n
  WMS_CMD_ERR_MSG_SIM_WRITE\n
  WMS_CMD_ERR_MSG_SIM_READ\n
  WMS_CMD_ERR_MSG_NV_WRITE\n
  WMS_CMD_ERR_MSG_NV_READ\n
  WMS_CMD_ERR_MSG_RUIM_WRITE\n
  WMS_CMD_ERR_MSG_RUIM_READ\n
  WMS_CMD_ERR_MSG_TPDU_TYPE

  @events 
  WMS_MSG_EVENT_SEND\n
  WMS_MSG_EVENT_SUBMIT_REPORT\n
  WMS_CFG_EVENT_MESSAGE_LIST

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_send
(
  wms_client_id_type                 client_id,
  wms_cmd_cb_type                    cmd_cb,
  const void                         * user_data,
  wms_send_mode_e_type               send_mode,
  const wms_client_message_s_type    * message_ptr
);
/*~ FUNCTION  wms_msg_send 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN message_ptr POINTER */ 

/*=========================================================================
FUNCTION  wms_msg_ack
===========================================================================*/
/**
  Acknowledges a message if the message has not been acknowledged.

  An SMS TL ACK message is sent to the base station. In CDMA mode, this function 
  is to be called only if is_tl_ack_requested in the client message received is 
  TRUE. Otherwise, this ACK request is ignored.

  The client receives WMS_MSG_EVENT_ACK to confirm that the ACK is being sent to 
  the network.

  The client receives WMS_MSG_EVENT_ACK_REPORT to be notified of the status of 
  the ACK (whether the ACK was successful). The application can use this event 
  to ensure that certain events and operations are done in sequence. For example, 
  when the SMS is used as a push mechanism in CDMA mode, the application can 
  wait for this event and then try to set up a data session if the network 
  cannot handle call origination while the SMS ACK is still pending.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                             this transaction of sending a message. The same 
                             pointer is passed to the client's callback function. 
                             A NULL pointer is acceptable.
  @param *ack_info_ptr [IN]  Pointer to the ACK information.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_NO_RESOURCE\n
  WMS_CMD_ERR_MSG_TRANSACTION_ID\n
  WMS_CMD_ERR_MSG_ENCODE

  @events
  WMS_MSG_EVENT_ACK\n
  WMS_MSG_EVENT_ACK_REPORT

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_ack
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  const wms_ack_info_s_type       *ack_info_ptr
);
/*~ FUNCTION  wms_msg_ack 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN ack_info_ptr POINTER */ 

/*=========================================================================
FUNCTION  wms_msg_read
===========================================================================*/
/**
  Reads a message from a memory store.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to read the message.
  @param index        [IN]  Index of the message.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_INDEX\n
  WMS_CMD_ERR_MSG_EMPTY_MESSAGE\n
  WMS_CMD_ERR_MSG_SIM_READ\n
  WMS_CMD_ERR_MSG_RUIM_READ\n
  WMS_CMD_ERR_MSG_NV_READ\n
  WMS_CMD_ERR_MSG_MEMORY_STORE

  @events WMS_MSG_EVENT_READ

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_read
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index
);
/*~ FUNCTION wms_msg_read 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_read_sts_report
===========================================================================*/
/**
  Reads a status report from a memory store.\ Currently, only a SIM memory 
  store is supported.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to read the status report.
  @param index        [IN]  Index of the status report.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE\n
  WMS_CMD_ERR_MSG_INDEX\n
  WMS_CMD_ERR_MSG_SIM_READ

  @events
  WMS_MSG_EVENT_READ_STS_REPORT

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_read_sts_report
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index
);
/*~ FUNCTION  wms_msg_read_sts_report 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_get_cache_info
===========================================================================*/
/**
  Retrieves the message cache information for a message stored in a memory store.

  @note This is a synchronous function call.

  @param mem_store    [IN]  Memory store from which to read the message.
  @param index        [IN]  Index of the message.
  @param tag          [IN]  Type of message, either NONE or TEMPLATE.
  @param *cache_ptr   [IN]  Pointer to the cache buffer in the client space (this 
                           function copies data from the WMS to the client).

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S
 
  @dependencies
  Client must have been initialized.
*/
wms_status_e_type wms_msg_get_cache_info
(
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index,
  wms_message_tag_e_type          tag,
  uint8                           * cache_ptr
);
/*~ FUNCTION  wms_msg_get_cache_info 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM INOUT cache_ptr ARRAY WMS_MSG_INFO_CACHE_SIZE */

/*=========================================================================
FUNCTION  wms_msg_write
===========================================================================*/
/**
  Writes an SMS message to a memory store.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param write_mode   [IN]  Mode for storing the message.
  @param *message_ptr [IN]  Pointer to the client's SMS message buffer. The API 
                            must copy the data in this buffer into its own 
                            buffer so the client can reuse it after the function 
                            returns.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S
   - WMS_CMD_ERR_MSG_TAG
   - WMS_CMD_ERR_MSG_DECODE
   - WMS_CMD_ERR_MSG_NO_MO_MSG

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_TAG\n
  WMS_CMD_ERR_MSG_TPDU_TYPE\n
  WMS_CMD_ERR_MSG_MEMORY_FULL\n
  WMS_CMD_ERR_MSG_INDEX\n
  WMS_CMD_ERR_MSG_WRITE_MODE\n
  WMS_CMD_ERR_MSG_ENCODE\n
  WMS_CMD_ERR_MSG_SIM_WRITE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE

  @events
  WMS_MSG_EVENT_WRITE\n
  WMS_CFG_EVENT_MESSAGE_LIST\n
  WMS_CFG_EVENT_MEMORY_FULL

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_write
(
  wms_client_id_type                 client_id,
  wms_cmd_cb_type                    cmd_cb,
  const void                         * user_data,
  wms_write_mode_e_type              write_mode,
  const wms_client_message_s_type    * message_ptr
);
/*~ FUNCTION wms_msg_write 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN message_ptr POINTER */

/*=========================================================================
FUNCTION  wms_msg_delete
===========================================================================*/
/**
  Deletes a message from a memory store.

  If the memory store is SIM/USIM and a corresponding status report message is 
  stored in the card, the status report message is also deleted.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to read the message.
  @param index        [IN]  Index of the message.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_INDEX\n
  WMS_CMD_ERR_MSG_EMPTY_MESSAGE\n
  WMS_CMD_ERR_MSG_SIM_DELETE\n
  WMS_CMD_ERR_MSG_NV_DELETE\n
  WMS_CMD_ERR_MSG_RUIM_DELETE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE

  @events
  WMS_MSG_EVENT_DELETE \n
  WMS_MSG_EVENT_MESSAGE_LIST

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_delete
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index
);
/*~ FUNCTION wms_msg_delete 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_delete_all
===========================================================================*/
/**
  Deletes all messages from a memory store.

  If the memory store is SIM/USIM and corresponding status report messages are 
  stored in the card, those status report messages are also be deleted.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to read the message.
  @param tag          [IN]  Type of message to be deleted.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
    WMS_CMD_ERR_MSG_SIM_DELETE\n
  WMS_CMD_ERR_MSG_NV_DELETE\n
  WMS_CMD_ERR_MSG_RUIM_DELETE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE

  @events
  WMS_MSG_EVENT_DELETE_ALL\n
  WMS_MSG_EVENT_MESSAGE_LIST

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_delete_all
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_tag_e_type          tag
);
/*~ FUNCTION wms_msg_delete_all 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_modify_tag
===========================================================================*/
/**
  Modifies the tag of a message.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to read the message.
  @param index        [IN]  Index of the message.
  @param tag          [IN]  Type of message for which the tag is to be modified.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE\n
  WMS_CMD_ERR_MSG_INDEX\n
  WMS_CMD_ERR_MSG_TAG\n
  WMS_CMD_ERR_MSG_SIM_WRITE\n
  WMS_CMD_ERR_MSG_NV_WRITE\n
  WMS_CMD_ERR_MSG_RUIM_WRITE

  @events
  WMS_MSG_EVENT_MODIFY_TAG\n
  WMS_CFG_EVENT_MESSAGE_LIST

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_modify_tag
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index,
  wms_message_tag_e_type          tag
);
/*~ FUNCTION  wms_msg_modify_tag 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_read_template
===========================================================================*/
/**
  Reads a template from a memory store.

  If the memory store is NV, the message index is ignored because only one 
  template is in NV.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to read the message.
  @param index        [IN]  Index of the message.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE\n
  WMS_CMD_ERR_MSG_INDEX\n
  WMS_CMD_ERR_MSG_EMPTY_MESSAGE\n
  WMS_CMD_ERR_MSG_DECODE\n
  WMS_CMD_ERR_MSG_SIM_READ\n
  WMS_CMD_ERR_MSG_NV_READ\n
  WMS_CMD_ERR_MSG_RUIM_READ

  @events WMS_MSG_EVENT_READ_TEMPLATE 

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_read_template
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index
);
/*~ FUNCTION  wms_msg_read_template 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_write_template
===========================================================================*/
/**
  Writes a template.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param write_mode   [IN]  Mode for storing the template.
  @param *message_ptr [IN]  Pointer to the client's SMS message buffer. The API 
                            must copy the data in this buffer into its own buffer 
                            so the client can reuse it after the function returns.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_TAG\n
  WMS_CMD_ERR_MSG_TPDU_TYPE\n
  WMS_CMD_ERR_MSG_MEMORY_FULL\n
  WMS_CMD_ERR_MSG_INDEX\n
  WMS_CMD_ERR_MSG_WRITE_MODE\n
  WMS_CMD_ERR_MSG_ENCODE\n
  WMS_CMD_ERR_MSG_SIM_WRITE\n
  WMS_CMD_ERR_MSG_NV_WRITE\n
  WMS_CMD_ERR_MSG_RUIM_WRITE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE

  @events
  WMS_MSG_EVENT_WRITE_TEMPLATE\n
  WMS_MSG_EVENT_MESSAGE_LIST

  @sideeffects
  A command is sent to the WMS task.
*/
/* COMMENTS
  If referenced in the union, set msg_ptr->u.gw_template.alpha_id.data
                              to a Valid Value or NULL
  If referenced in the union, set msg_ptr->u.gw_template.alpha_id.len
                              to a Valid Value or 0
*/
wms_status_e_type wms_msg_write_template
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data,
  wms_write_mode_e_type            write_mode,
  const wms_client_message_s_type  * message_ptr
);
/*~ FUNCTION  wms_msg_write_template 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN message_ptr POINTER */ 

/*=========================================================================
FUNCTION  wms_msg_delete_template
===========================================================================*/
/**
  Deletes a template from an NV or SIM memory store.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to delete the template.
  @param index        [IN]  Index of the template.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_INDEX\n
  WMS_CMD_ERR_MSG_EMPTY_MESSAGE\n
    WMS_CMD_ERR_MSG_SIM_DELETE\n
  WMS_CMD_ERR_MSG_NV_DELETE\n
  WMS_CMD_ERR_MSG_RUIM_DELETE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE

  @events
    WMS_MSG_EVENT_DELETE_TEMPLATE\n
  WMS_MSG_EVENT_MESSAGE_LIST

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_delete_template
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index
);
/*~ FUNCTION  wms_msg_delete_template 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_delete_template_all
===========================================================================*/
/**
  Deletes all templates from an NV or SIM memory store.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to delete the templates.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
    WMS_CMD_ERR_MSG_SIM_DELETE\n
  WMS_CMD_ERR_MSG_NV_DELETE\n
  WMS_CMD_ERR_MSG_RUIM_DELETE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE

  @events
    WMS_MSG_EVENT_DELETE_TEMPLATE_ALL\n
  WMS_MSG_EVENT_MESSAGE_LIST

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_delete_template_all
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store
);
/*~ FUNCTION  wms_msg_delete_template_all 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_write_sts_report
===========================================================================*/
/**
  Writes a status report to a memory store.

  A corresponding Mobile Originated SMS message must be present in the same 
  memory store Otherwise, the write operation of the status report message 
  will fail.

  The status report is stored only if the corresponding SMS Message is also 
  stored in the same memory. If the SMS Message is not found, 
  WMS_CMD_ERR_MSG_NO_MO_MSG is returned in the command callback.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param write_mode   [IN]  Mode for storing the status report.
  @param *message_ptr [IN]  Pointer to the client's SMS message buffer. The API 
                            must copy the data in this buffer into its own 
                            buffer so the client can reuse it after the function 
                            returns.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S
   - WMS_CMD_ERR_MSG_TAG
   - WMS_CMD_ERR_MSG_DECODE
   - WMS_CMD_ERR_MSG_NO_MO_MSG

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_TAG\n
  WMS_CMD_ERR_MSG_TPDU_TYPE \n
  WMS_CMD_ERR_MSG_MEMORY_FULL\n
  WMS_CMD_ERR_MSG_INDEX\n
  WMS_CMD_ERR_MSG_WRITE_MODE\n
  WMS_CMD_ERR_MSG_ENCODE\n
  WMS_CMD_ERR_MSG_SIM_WRITE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE 

  @events
  WMS_MSG_EVENT_WRITE_STS_REPORT\n
    WMS_MSG_EVENT_MESSAGE_LIST\n
  WMS_CFG_EVENT_MEMORY_STATUS

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_write_sts_report
(
  wms_client_id_type                 client_id,
  wms_cmd_cb_type                    cmd_cb,
  const void                         * user_data,
  wms_write_mode_e_type              write_mode,
  const wms_client_message_s_type    * message_ptr
);
/*~ FUNCTION  wms_msg_write_sts_report 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN message_ptr POINTER */ 

/*=========================================================================
FUNCTION  wms_msg_delete_sts_report
===========================================================================*/
/**
  Deletes a status report from a memory store.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to delete the status report.
  @param index        [IN]  Index of the status report.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_INDEX\n
  WMS_CMD_ERR_MSG_EMPTY_MESSAGE\n
  WMS_CMD_ERR_MSG_SIM_DELETE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE

  @events
  WMS_MSG_EVENT_DELETE_TEMPLATE\n
  WMS_MSG_EVENT_MESSAGE_LIST\n
  WMS_CFG_EVENT_MEMORY_STATUS

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_delete_sts_report
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index
);
/*~ FUNCTION  wms_msg_delete_sts_report 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_delete_sts_report_all
===========================================================================*/
/**
  Deletes all status reports from a memory store.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param mem_store    [IN]  Memory store from which to delete the status reports.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_MSG_SIM_DELETE\n
  WMS_CMD_ERR_MSG_MEMORY_STORE

  @events
  WMS_MSG_EVENT_DELETE_TEMPLATE_ALL\n
  WMS_MSG_EVENT_MESSAGE_LIST\n
  WMS_CFG_EVENT_MEMORY_STATUS

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_delete_sts_report_all
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store
);
/*~ FUNCTION  wms_msg_delete_sts_report_all 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_set_retry_period
===========================================================================*/
/**
  Specifies the SMS message retry period.

  If period = nonzero, the retry period is enabled. If period = 0 seconds, the 
  retry period is disabled.

  When the retry period is enabled and an error is received from the lower 
  layers, the WMS waits for five seconds and sends the message again. However, 
  the retry is not performed under the following conditions:
   - If there are errors from the network (CDMA SMS ACK with cause code, or 
     GSM SMS RP/CP errors).
   - If the message is too large.
   - If the message fails the Mobile Originated Control by the SIM card.

  @note This is a synchronous function call.

  @param client_id    [IN]  Client ID.
  @param period       [IN]  Total retry period in seconds.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S
   - WMS_UNSUPPORTED_S
   - WMS_INVALID_PARM_VALUE_S

  @dependencies
  Client must have been initialized.

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_set_retry_period
(
  wms_client_id_type              client_id,
  uint32                          period     /* in seconds */
);
/*~ FUNCTION  wms_msg_set_retry_period 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_get_retry_period
===========================================================================*/
/**
  Retrieves the current SMS message retry period.

  @param client_id    [IN]  Client ID.
  @param *period      [IN]  Pointer to the retry period result.
  
  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S
   - WMS_UNSUPPORTED_S

  @dependencies
  Client must have been initialized.

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_get_retry_period
(
  wms_client_id_type              client_id,
  uint32                          * period     /* in seconds */
);
/*~ FUNCTION  wms_msg_get_retry_period 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM INOUT period POINTER */

/*=========================================================================
FUNCTION  wms_msg_transport_release
===========================================================================*/
/**
  Allows a client to release its messaging transport callback with WMS.

  @param transport_id [IN]  Transport ID.

  @return
  Status of the request:
   - WMS_OK_S
   - WMS_UNSUPPORTED_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type  wms_msg_transport_release
(
  wms_msg_transport_id_type      transport_id
);
/*~ FUNCTION  wms_msg_transport_release 
    ONERROR return WMS_RPC_ERROR_S */
    
/*=========================================================================
FUNCTION  wms_msg_transport_init
===========================================================================*/
/**
  Allows the SMS Transport Layer to register with WMS to handle MO and MT SMS 
  messages with specified capability.
  
  @param transport_type [IN]  Transport type.
  @param transport_cap [IN]  Transport capability.
  @param transport_id_ptr [IN,OUT] Transport ID pointer.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_NULL_PTR_S
   - WMS_UNSUPPORTED_S

  @dependencies
  Client must have been initialized.

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type  wms_msg_transport_init
(
  wms_msg_transport_type_e_type   transport_type,
  wms_msg_transport_cap_type      transport_cap,
  wms_msg_transport_id_type      *transport_id_ptr
);
/*~ FUNCTION  wms_msg_transport_init 
    RELEASE_FUNC wms_msg_transport_release(*transport_id_ptr)
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM INOUT transport_id_ptr POINTER */

/*=========================================================================
FUNCTION  wms_msg_transport_reg_mo_sms_cb
===========================================================================*/
/**
  Allows the SMS Transport Layer to register with WMS with a specified function 
  callback to be used to send SMS over the Transport Layer. If a null pointer 
  is passed, the callback is de-registered.
  
  @param transport_id [IN]  Transport ID.
  @param mo_sms_cb    [IN]  Transport MO SMS callback.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_UNSUPPORTED_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  Client must have been initialized.

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type  wms_msg_transport_reg_mo_sms_cb
(
  wms_msg_transport_id_type         transport_id,
  wms_msg_transport_mo_sms_cb_type  mo_sms_cb
);
/*~ FUNCTION  wms_msg_transport_reg_mo_sms_cb
    RELEASE_FUNC  wms_msg_transport_release(transport_id)
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_transport_nw_reg_status_update
===========================================================================*/
/**
  Allow SMS transport layer to notify WMS when its network registration 
  status change
  
  @param transport_id [IN]  Transport ID.
  @param transport_status [IN]  Transport Status.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_UNSUPPORTED_S

  @dependencies
  Client must have been initialized.

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type  wms_msg_transport_nw_reg_status_update
(
  wms_msg_transport_id_type               transport_id,
  wms_msg_transport_nw_reg_status_e_type  transport_status
);
/*~ FUNCTION  wms_msg_transport_nw_reg_status_update
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_transport_cap_update
===========================================================================*/
/**
  Allows the SMS Transport Layer to notify the WMS when its registered 
  capability changes.

  @param transport_id [IN]  Transport ID.
  @param transport_cap [IN]  Transport Cap.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_UNSUPPORTED_S

  @dependencies
  Client must have been initialized.

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type  wms_msg_transport_cap_update
(
  wms_msg_transport_id_type         transport_id,
  wms_msg_transport_cap_type        transport_cap
);
/*~ FUNCTION  wms_msg_transport_cap_update
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_msg_transport_rpt_ind
===========================================================================*/
/**
  Notifies the WMS when there is a MT SMS or MO status report.

  @param rpt_ind_ptr  [IN]  Transport Report Indication pointer.

  @return
  Status of the request.  
   - WMS_OK_S
   - WMS_UNSUPPORTED_S
   - WMS_NULL_PTR_S

  @dependencies
  Client must have been initialized.

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_msg_transport_rpt_ind
(
  wms_msg_transport_rpt_ind_s_type   *rpt_ind_ptr
);

/*~ FUNCTION  wms_msg_transport_rpt_ind 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN rpt_ind_ptr POINTER */

/* <EJECT> */
/*===========================================================================

                         Dedicated Channel Group

                        API FUNCTION DEFINITIONS

===========================================================================*/
/**
@ingroup dc_group
@{ */
/*=========================================================================
FUNCTION  wms_dc_enable_auto_disconnect
===========================================================================*/
/**
  Submits a request to automatically disconnect a dedicated channel connection 
  if no SMS message is sent or received during the timeout period as specified 
  by the client.

  The client or the base station can still explicitly disconnect the connection 
  after this feature is enabled. The connection release result is delivered to 
  the client through the event callback function.

  The auto-disconnect feature is disabled by default.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param timeout      [IN]  Timeout in seconds.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE

  @events
  WMS_DC_EVENT_ENABLE_AUTO_DISCONNECT\n
  WMS_DC_EVENT_DISCONNECT

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_dc_enable_auto_disconnect
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  uint32                          timeout    /* in seconds */
);
/*~ FUNCTION  wms_dc_enable_auto_disconnect 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_dc_disable_auto_disconnect
==========================================================================*/
/**
  Submits a request to disable the auto-disconnect feature for a dedicated 
  channel connection.

  The auto-disconnect feature is disabled by default.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE

  @events
  WMS_DC_EVENT_DISABLE_AUTO_DISCONNECT

  @sideeffects
  Request is added to the request queue.
*/

wms_status_e_type wms_dc_disable_auto_disconnect
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data
);
/*~ FUNCTION  wms_dc_disable_auto_disconnect 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_dc_connect
==========================================================================*/
/**
  Originates a dedicated channel connection request (CDMA only) to the base 
  station and changes its internal state.

  The connection result is delivered to the client through the event callback 
  function. SMS messages can still be sent or received whether or not there is 
  a DC connection.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param so           [IN]  Service option.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_DC_BAD_STATE

  @events
  WMS_DC_EVENT_CONNECT\n
  WMS_DC_EVENT_DISCONNECT

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_dc_connect
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_dc_so_e_type                so
);
/*~ FUNCTION  wms_dc_connect 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_dc_disconnect
==========================================================================*/
/**
  Submits a request from the client or base station to disconnect a DC 
  connection (CDMA only) and change its internal state.

  An event is issued through the event callback function indicating when the 
  connection is disconnected. SMS messages can still be sent or received 
  regardless of whether there is a DC connection.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE

  @events
  WMS_DC_EVENT_DISCONNECT

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_dc_disconnect
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data
);
/*~ FUNCTION wms_dc_disconnect 
    ONERROR return WMS_RPC_ERROR_S */

/* <EJECT> */
/*===========================================================================

                        Multimode Broadcast Group

                        API FUNCTION DEFINITIONS

===========================================================================*/
/**
@ingroup mmbroadcast_group
@{ */
/*=========================================================================
FUNCTION   wms_bc_mm_get_config
===========================================================================*/
/**
  Retrieves the carrier's configuration for multimode broadcast SMS.

  The result is returned to the user through the event callback function. The 
  command status is returned through the command callback function.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @events
  WMS_BC_MM_EVENT_CONFIG

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_get_config
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode
);
/*~ FUNCTION wms_bc_mm_get_config 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_bc_mm_get_pref
===========================================================================*/
/**
  Retrieves the user's preference for multimode broadcast SMS.

  The result is returned to the user through the event callback function. The 
  command status is returned through the command callback function.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE

  @events
  WMS_BC_MM_EVENT_PREF

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_get_pref
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode
);
/*~ FUNCTION wms_bc_mm_get_pref 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_bc_mm_set_pref
===========================================================================*/
/**
  Changes the user's preference for multimode broadcast SMS.

  The result is returned to the user through the event callback function. The 
  command status is returned through the command callback function. 

  If the enabling status of the broadcast SMS is changed because of the new 
  preference, an enable/disable command is sent to the lower layers.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.
  @param pref         [IN]  New user preference.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_BC_CANNOT_ACTIVATE\n
  WMS_CMD_ERR_BC_CANNOT_ACTIVATE_ALL\n
  WMS_CMD_ERR_BC_BAD_PREF\n
  WMS_CMD_ERR_BC_NV_WRITE

  @events
  WMS_BC_MM_EVENT_PREF

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_set_pref
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode,
  wms_bc_pref_e_type              pref
);
/*~ FUNCTION wms_bc_mm_set_pref 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_bc_mm_get_table
===========================================================================*/
/**
  Retrieves the whole broadcast SMS service table.

  The result is returned to the user through the event callback function. The 
  command status is returned through the command callback function.

  @note To prevent retrieving a large service table, the 
  wms_bc_mm_get_all_service_ids() function can be called to retrieve all service 
  IDs, where each ID is much smaller than a complete table entry. Then the  
  wms_bc_mm_get_service_info() function can be called for each service ID to 
  retrieve the table entries one by one.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE

  @events
  WMS_BC_MM_EVENT_TABLE

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_get_table
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode
);
/*~ FUNCTION wms_bc_mm_get_table 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_bc_mm_select_service
===========================================================================*/
/**
  Selects/deselects a broadcast SMS service in the service table for filtering 
  purposes.

  The broadcast messages for this service can be received when the service is 
  selected or when the current user's preference is All Services.

  For GSM SMS CB, the service IDs of the selected entries are written to the 
  SIM's EF-CBMI and EF-CBMIR. If the enabling status of broadcast SMS is 
  changed  due to the new selection, an enable/disable command is sent to the 
  lower layers.

  @note If the current user preference is table only and no service is selected 
  in the table, broadcast SMS is disabled.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.
  @param *srv_id_ptr  [IN]  Pointer to the service ID for the service.
  @param selected     [IN]  Indicates whether this service is selected.

  @return
  Status of the request.
   - WMS_OK_S
   – WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_BC_BAD_SRV_ID\n
  WMS_CMD_ERR_BC_NV_WRITE\n
  WMS_CMD_ERR_BC_SIM_WRITE

  @events
  WMS_BC_MM_EVENT_SRV_UPDATED

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_select_service
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode,
  const wms_bc_mm_srv_id_type     *srv_id_ptr,
  boolean                         selected
);
/*~ FUNCTION wms_bc_mm_select_service 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN srv_id_ptr POINTER */

/*=========================================================================
FUNCTION  wms_bc_mm_get_all_service_ids
===========================================================================*/
/**
  Retrieves all the service IDs from the broadcast SMS service table and 
  returns the result to the user through the event callback function.
 
  The command status is returned through the command callback function.

  The result does not contain the priorities and labels for each service. The 
  wms_bc_mm_get_service_info() function can be called for each service to 
  retrieve the service information one by one.

  @note The purpose of this function is to reduce the return buffer size.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_NO_RESOURCE

  @events
  WMS_BC_MM_EVENT_SRV_IDS

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_get_all_service_ids
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode
);
/*~ FUNCTION wms_bc_mm_get_all_service_ids 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_bc_mm_get_service_info
===========================================================================*/
/**
  Retrieves a table entry from the broadcast SMS service table.

  The result is returned to the user through the event callback function. The 
  command status is returned through the command callback function.

  By using wms_bc_mm_get_all_service_ids() with this function, the table entries 
  can be retrieved one by one.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.
  @param *srv_id_ptr  [IN]  Pointer to the service ID for the table entry to be 
                            retrieved.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_BC_BAD_SRV_ID

  @events
  WMS_BC_MM_EVENT_SRV_INFO

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_get_service_info
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode,
  const wms_bc_mm_srv_id_type     *srv_id_ptr
);
/*~ FUNCTION wms_bc_mm_get_service_info 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN srv_id_ptr POINTER */ 

/*=========================================================================
FUNCTION  wms_bc_mm_add_services
===========================================================================*/
/**
  Adds a list of table entries to the broadcast SMS service table.
 
  If the enabling status of the broadcast SMS is changed due to the new 
  preference, an enable/disable command is sent to the lower layers.

  For GSM SMS CB, the service IDs of the selected entries are written to the 
  SIM's EF-CBMI and EF-CBMIR.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.
  @param num_entries  [IN]  Number of entries to be added.
  @param *entries     [IN]  Pointer to the list of table entries to be added.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_BC_TABLE_FULL\n
  WMS_CMD_ERR_BC_DUPLICATE_SRV\n
  WMS_CMD_ERR_BC_NV_WRITE\n
  WMS_CMD_ERR_NULL_PTR\n
  WMS_CMD_ERR_BC_SIM_WRITE\n
  WMS_CMD_ERR_NO_RESOURCE 

  @events
  WMS_BC_MM_EVENT_ADD_SRVS 

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_add_services
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode,
  uint8                           num_entries,
  const wms_bc_mm_service_info_s_type   *entries
);
/*~ FUNCTION wms_bc_mm_add_services 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN entries VARRAY LENGTH num_entries */ 

/*=========================================================================
FUNCTION  wms_bc_mm_delete_services
===========================================================================*/
/**
  Deletes a list of table entries from the broadcast SMS service table.
 
  If the enabling status of broadcast SMS is changed due to the new preference, 
  an enable/disable command is sent to the lower layers.

  For GSM SMS CB, the service IDs are also removed from the SIM's EF-CBMI and 
  EF-CBMIR.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.
  @param num_entries  [IN]  Number of entries to be added.
  @param *srv_ids     [IN]  Pointer to the list of service IDs to be removed 
                            from the table.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_BC_BAD_SRV_ID\n
  WMS_CMD_ERR_BC_NV_WRITE\n
  WMS_CMD_ERR_NULL_PTR\n
  WMS_CMD_ERR_BC_SIM_WRITE\n
  WMS_CMD_ERR_NO_RESOURCE 

  @events
  WMS_BC_MM_EVENT_DELETE_SRVS

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_delete_services
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode,
  uint8                           num_entries,
  const wms_bc_mm_srv_id_type     *srv_ids
);
/*~ FUNCTION wms_bc_mm_delete_services 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN srv_ids VARRAY LENGTH num_entries */ 

/*=========================================================================
FUNCTION  wms_bc_mm_change_service_info
===========================================================================*/
/**
  Changes the information of an entry in the broadcast SMS service table, 
  including priority for CDMA, label, etc.

  For GSM SMS CB, the service IDs of the selected entries are written to the 
  SIM's EF-CBMI and EF-CBMIR.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.
  @param *srv_info_ptr [IN]  Pointer to the new service table entry information.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_BC_BAD_SRV_ID\n
  WMS_CMD_ERR_BC_NV_WRITE\n
  WMS_CMD_ERR_BC_SIM_WRITE

  @events
  WMS_BC_MM_EVENT_SRV_UPDATED

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_change_service_info
(
  wms_client_id_type                  client_id,
  wms_cmd_cb_type                     cmd_cb,
  const void                          *user_data,
  wms_message_mode_e_type             message_mode,
  const wms_bc_mm_service_info_s_type *srv_info_ptr
);
/*~ FUNCTION wms_bc_mm_change_service_info 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN srv_info_ptr POINTER */ 

/*=========================================================================
FUNCTION  wms_bc_mm_delete_all_services
===========================================================================*/
/**
  Deletes all entries from the broadcast SMS service table.

  For GSM SMS CB, all service IDs are removed from the SIM's EF-CBMI and 
  EF-CBMIR.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_BC_NV_WRITE\n
  WMS_CMD_ERR_BC_SIM_WRITE 

  @events
  WMS_BC_MM_EVENT_DELETE_ALL_SRVS

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_delete_all_services
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode
);
/*~ FUNCTION wms_bc_mm_delete_all_services 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_bc_mm_select_all_services
===========================================================================*/
/**
  Selects/deselects all entries from the broadcast SMS service table.

  For GSM SMS CB, the service IDs of the selected entries are written to the 
  SIM's EF-CBMI and EF-CBMIR.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.
  @param selected     [IN]  Indicates whether all services are selected or 
                            deselected.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_BC_NV_WRITE\n
  WMS_CMD_ERR_BC_SIM_WRITE\n 
  WMS_CMD_ERR_NO_RESOURCE

  @events
  WMS_BC_MM_EVENT_SELECT_ALL_SRVS

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_select_all_services
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode,
  boolean                         selected
);
/*~ FUNCTION wms_bc_mm_select_all_services 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_bc_mm_set_priority_for_all_services
===========================================================================*/
/**
  Sets the priorities for all entries from the broadcast SMS service table.\ 
  Priority levels are all changed to the same level.

  @note This function is available for CDMA mode only, not for GSM/WCDMA mode.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Indicates CDMA Broadcast SMS or GSM SMS CB.
  @param priority     [IN]  Priority level for all services.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_BC_NV_WRITE\n
  WMS_CMD_ERR_UNSUPPORTED\n
  WMS_CMD_ERR_NO_RESOURCE

  @events
  WMS_BC_MM_EVENT_SET_PRIORITY_ALL_SRVS

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_set_priority_for_all_services
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode,
  wms_priority_e_type             priority
);
/*~ FUNCTION wms_bc_mm_set_priority_for_all_services 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_bc_mm_msg_delete_indication
===========================================================================*/
/**
  Notifies the WMS and lower layers that an SMS-CB Page message has been 
  deleted by the client.

  The WMS and lower layers then remove cached information for this message, 
  ensuring that the repeated message can be received again.

  @note This function is applicable to GSM/WCDMA mode only.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param *cb_page_header [IN]  Pointer to the CB page header buffer.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_UNSUPPORTED\n
  WMS_CMD_ERR_NO_RESOURCE

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_msg_delete_indication
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                     *user_data,
  wms_gw_cb_page_header_s_type   *cb_page_header
);
/*~ FUNCTION wms_bc_mm_msg_delete_indication 
    ONERROR return WMS_RPC_ERROR_S */
/*~ PARAM IN cb_page_header POINTER */ 

/*=========================================================================
FUNCTION  wms_bc_mm_msg_delete_all_indication
===========================================================================*/
/**
  Notifies the WMS and lower layers that all SMS-CB page messages have been 
  deleted by the client.

  The WMS and lower layers then remove cached information for all CB messages, 
  ensuring that the repeated messages can be received again.

  @note This function is applicable to GSM/WCDMA mode only.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S
   - WMS_UNSUPPORTED_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_UNSUPPORTED\n
  WMS_CMD_ERR_NO_RESOURCE

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_msg_delete_all_indication
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                     *user_data
);
/*~ FUNCTION wms_bc_mm_msg_delete_all_indication 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_bc_mm_get_reading_pref
===========================================================================*/
/**
  Retrieves the reading preferences for UMTS BMC (SMS Cell Broadcast in a 
  WCDMA system).

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Message mode (only WMS_MESSAGE_MODE_GW is supported).

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S
   - WMS_UNSUPPORTED_S

  @dependencies
  FEATURE_UMTS_BMC must be turned on.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_UNSUPPORTED\n
  WMS_CMD_ERR_NO_RESOURCE

  @events
  WMS_BC_MM_EVENT_READING_PREF

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_get_reading_pref
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                     *user_data,
  wms_message_mode_e_type         message_mode
);
/*~ FUNCTION wms_bc_mm_get_reading_pref 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_bc_mm_set_reading_pref
===========================================================================*/
/**
  Sets the reading preferences for UMTS BMC (SMS Cell Broadcast in a WCDMA 
  system).

  The schedule message from the network sometimes indicates the reading 
  preferences for certain BMC messages that the network schedules to send at a 
  later time. The mobile can decide to wake up and receive those messages based 
  on these reading preferences.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.
  @param message_mode [IN]  Message mode (only WMS_MESSAGE_MODE_GW is supported).
  @param reading_advised [IN]  BMC reception status for Reading Advised messages.
  @param reading_optional [IN]  BMC reception status for Reading Optional messages.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S
   - WMS_UNSUPPORTED_S

  @dependencies
  FEATURE_UMTS_BMC must be turned on.

  @commandstatus
  WMS_CMD_ERR_NONE\n
  WMS_CMD_ERR_UNSUPPORTED\n
  WMS_CMD_ERR_NO_RESOURCE

  @events
  WMS_BC_MM_EVENT_READING_PREF

  @sideeffects
  Request is added to the request queue.
*/
wms_status_e_type wms_bc_mm_set_reading_pref
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                     *user_data,
  wms_message_mode_e_type         message_mode,
  boolean                         reading_advised,
  boolean                         reading_optional
);
/*~ FUNCTION wms_bc_mm_set_reading_pref 
    ONERROR return WMS_RPC_ERROR_S */

/* <EJECT> */
/*===========================================================================

                            Debugging Group

                        API FUNCTION DEFINITIONS

===========================================================================*/
/** @ingroup debug_group
@{ */
/*=========================================================================
FUNCTION  wms_dbg_reset_tl_seq_num
===========================================================================*/
/**
  Resets the Transport Layer's sequence number so the next message sent has a 
  sequence number 0 for the bearer reply option.

  This is a test interface. Typically, clients do not call this function for 
  normal operation.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_dbg_reset_tl_seq_num
(
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data
);
/*~ FUNCTION wms_dbg_reset_tl_seq_num 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_dbg_set_msg_ref
===========================================================================*/
/**
  Resets the GSM SMS message reference number to 0.

  This is a test interface. Typically, clients do not call this function for 
  normal operation.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_dbg_set_msg_ref
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data
);
/*~ FUNCTION wms_dbg_set_msg_ref 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_dbg_clear_smma_flag
===========================================================================*/
/**
  Resets the GSM SMS SMMA/memory full flag in the SIM's EF-SMSS.

  This is a test interface. Typically, clients do not call this function for 
  normal operation.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S

  @dependencies
  None.

  @sideeffects
  A command is sent to the WMS task.
*/
wms_status_e_type wms_dbg_clear_smma_flag
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data
);
/*~ FUNCTION wms_dbg_clear_smma_flag 
    ONERROR return WMS_RPC_ERROR_S */

/*=========================================================================
FUNCTION  wms_dbg_get_retry_interval
===========================================================================*/
/**
  Retrieves the current retry interval value.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S
   - WMS_UNSUPPORTED_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE

  @events
  WMS_DBG_EVENT_RETRY_INTERVAL

  @sideeffects
  A command is sent to the WMS task.
*/
/*
  Retrieve the SMS Retry Interval. i.e. the interval between the SMS Retry Attempts
  Value is specified in seconds. Command is sent to WMS Task.
  A Debug Event will be posted by WMS its clients with the Retry Interval.
*/
wms_status_e_type wms_dbg_get_retry_interval
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data
); 
/*~ FUNCTION wms_dbg_get_retry_interval 
    ONERROR return WMS_RPC_ERROR_S */

/*
  Retrieve the SMS Retry Interval. i.e. the interval between the SMS Retry Attempts
  Value is specified in seconds. Command is sent to WMS Task.
  A Debug Event will be posted by WMS its clients with the Retry Interval.
*/
wms_status_e_type wms_dbg_set_retry_interval
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data,
  uint32                           retry_interval /* seconds */
);
/*~ FUNCTION wms_dbg_set_retry_interval 
    ONERROR return WMS_RPC_ERROR_S */
    
/*=========================================================================
FUNCTION  wms_dbg_get_retry_period
===========================================================================*/
/**
  Retrieves the current SMS message retry period.

  @param client_id    [IN]  Client ID.
  @param cmd_cb       [IN]  Command callback for reporting the command status.
  @param *user_data   [IN]  Pointer provided by the client to uniquely identify 
                            this transaction of sending a message. The same 
                            pointer is passed to the client's callback function. 
                            A NULL pointer is acceptable.

  @return
  Status of the request.
   - WMS_OK_S
   - WMS_OUT_OF_RESOURCES_S
   - WMS_UNSUPPORTED_S

  @dependencies
  None.

  @commandstatus
  WMS_CMD_ERR_NONE

  @events
  WMS_DBG_EVENT_RETRY_PERIOD

  @sideeffects
  A command is sent to the WMS task.
*/
/*
  Retrieves the current SMS message retry period. 
  A Debug Event will be posted by WMS to its clients with the Retry Period.
*/
wms_status_e_type wms_dbg_get_retry_period
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data
); 
/*~ FUNCTION wms_dbg_get_retry_period 
    ONERROR return WMS_RPC_ERROR_S */

/*
  Specifies the SMS message retry period. Command is sent to WMS Task.
  A Debug Event will be posted by WMS to its clients with the Retry Period.
*/
wms_status_e_type wms_dbg_set_retry_period
(
  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       * user_data,
  uint32                           retry_period /* seconds */
);
/*~ FUNCTION wms_dbg_set_retry_period 
    ONERROR return WMS_RPC_ERROR_S */ 

/** @ingroup client_pm_group
@{ */
/** Filter type.
*/
typedef enum
{
  WMS_PM_NOTIFY_NONE   = 0,     /**< Do not notify any events. */
  WMS_PM_NOTIFY_ALL,            /**< Notify events based on event masks. */
  WMS_PM_NOTIFY_DEFAULT_FILTER, /**< Notify events based on default filtering. */
  WMS_PM_NOTIFY_CUSTOM_FILTER,  /**< Notify events based on customized filtering. */
/** @cond */
  WMS_PM_NOTIFY_MAX
/** @endcond */
} wms_pm_notify_e_type;

/** @ingroup client_pm_group
@{ */
/** Processor type.
*/
typedef enum
{
  WMS_CLIENT_PROCESSOR_NONE   = 0, /**< &nbsp; */
  WMS_CLIENT_PROCESSOR_MODEM,      /**< Modem processor. */
  WMS_CLIENT_PROCESSOR_APP1,       /**< Applications processor 1. */
  WMS_CLIENT_PROCESSOR_APP2,       /**< Applications processor 2. */
/** @cond */
  WMS_CLIENT_PROCESSOR_MAX
/** @endcond */
} wms_client_processor_e_type;

/** @ingroup client_pm_group
@{ */
/* Configuration event masks. */
#define WMS_CFG_EVENT_MASK_GW_READY          (1 << WMS_CFG_EVENT_GW_READY)
#define WMS_CFG_EVENT_MASK_CDMA_READY        (1 << WMS_CFG_EVENT_CDMA_READY)
#define WMS_CFG_EVENT_MASK_ROUTES            (1 << WMS_CFG_EVENT_ROUTES)
#define WMS_CFG_EVENT_MASK_MEMORY_STATUS     (1 << WMS_CFG_EVENT_MEMORY_STATUS)
#define WMS_CFG_EVENT_MASK_MESSAGE_LIST      (1 << WMS_CFG_EVENT_MESSAGE_LIST)
#define WMS_CFG_EVENT_MASK_MEMORY_FULL       (1 << WMS_CFG_EVENT_MEMORY_FULL)
#define WMS_CFG_EVENT_MASK_GW_DOMAN_PREF     (1 << WMS_CFG_EVENT_GW_DOMAIN_PREF)
#define WMS_CFG_EVENT_MASK_CELL_CHANGE       (1 << WMS_CFG_EVENT_CELL_CHANGE)
#define WMS_CFG_EVENT_MASK_PRIMARY_CLIENT_SET (1 << WMS_CFG_EVENT_PRIMARY_CLIENT_SET)
#define WMS_CFG_EVENT_MASK_MEMORY_STATUS_SET (1 << WMS_CFG_EVENT_MEMORY_STATUS_SET)
#define WMS_CFG_EVENT_MASK_LINK_CONTROL      (1 << WMS_CFG_EVENT_LINK_CONTROL)
#define WMS_CFG_EVENT_MASK_CB_ERROR          (1 << WMS_CFG_EVENT_CB_ERROR)

/** @ingroup client_pm_group
@{ */
/* MSG Event Masks */
#define WMS_MSG_EVENT_MASK_SEND                 (1 << WMS_MSG_EVENT_SEND)
#define WMS_MSG_EVENT_MASK_ACK                  (1 << WMS_MSG_EVENT_ACK)
#define WMS_MSG_EVENT_MASK_READ                 (1 << WMS_MSG_EVENT_READ)
#define WMS_MSG_EVENT_MASK_WRITE                (1 << WMS_MSG_EVENT_WRITE)
#define WMS_MSG_EVENT_MASK_DELETE               (1 << WMS_MSG_EVENT_DELETE)
#define WMS_MSG_EVENT_MASK_DELETE_ALL           (1 << WMS_MSG_EVENT_DELETE_ALL)
#define WMS_MSG_EVENT_MASK_MODIFY_TAG           (1 << WMS_MSG_EVENT_MODIFY_TAG)
#define WMS_MSG_EVENT_MASK_READ_TEMPLATE        (1 << WMS_MSG_EVENT_READ_TEMPLATE)
#define WMS_MSG_EVENT_MASK_WRITE_TEMPLATE       (1 << WMS_MSG_EVENT_WRITE_TEMPLATE)
#define WMS_MSG_EVENT_MASK_DELETE_TEMPLATE      (1 << WMS_MSG_EVENT_DELETE_TEMPLATE)
#define WMS_MSG_EVENT_MASK_DELETE_TEMPLATE_ALL  (1 << WMS_MSG_EVENT_DELETE_TEMPLATE_ALL)
#define WMS_MSG_EVENT_MASK_READ_STS_REPORT      (1 << WMS_MSG_EVENT_READ_STS_REPORT)
#define WMS_MSG_EVENT_MASK_WRITE_STS_REPORT     (1 << WMS_MSG_EVENT_WRITE_STS_REPORT)
#define WMS_MSG_EVENT_MASK_DELETE_STS_REPORT    (1 << WMS_MSG_EVENT_DELETE_STS_REPORT)
#define WMS_MSG_EVENT_MASK_DELETE_STS_REPORT_ALL (1 << WMS_MSG_EVENT_DELETE_STS_REPORT_ALL)
#define WMS_MSG_EVENT_MASK_RECEIVED_MESSAGE     (1 << WMS_MSG_EVENT_RECEIVED_MESSAGE)
#define WMS_MSG_EVENT_MASK_SUBMIT_REPORT        (1 << WMS_MSG_EVENT_SUBMIT_REPORT)
#define WMS_MSG_EVENT_MASK_STATUS_REPORT        (1 << WMS_MSG_EVENT_STATUS_REPORT)
#define WMS_MSG_EVENT_MASK_MT_MESSAGE_ERROR     (1 << WMS_MSG_EVENT_MT_MESSAGE_ERROR)
#define WMS_MSG_EVENT_MASK_ACK_REPORT           (1 << WMS_MSG_EVENT_ACK_REPORT)
#define WMS_MSG_EVENT_MASK_DUPLICATE_CB_PAGE    (1 << WMS_MSG_EVENT_DUPLICATE_CB_PAGE)

/** @ingroup client_pm_group
@{ */
/* Dedicated Channel event masks. */
#define WMS_DC_EVENT_MASK_INCOMING             (1 << WMS_DC_EVENT_INCOMING)
#define WMS_DC_EVENT_MASK_CONNECTED            (1 << WMS_DC_EVENT_CONNECTED)
#define WMS_DC_EVENT_MASK_ABORTED              (1 << WMS_DC_EVENT_ABORTED)
#define WMS_DC_EVENT_MASK_DISCONNECTED         (1 << WMS_DC_EVENT_DISCONNECTED)
#define WMS_DC_EVENT_MASK_CONNECTING           (1 << WMS_DC_EVENT_CONNECTING)
#define WMS_DC_EVENT_MASK_ENABLE_AUTO_DISCONNECT  (1 << WMS_DC_EVENT_ENABLE_AUTO_DISCONNECT)
#define WMS_DC_EVENT_MASK_DISABLE_AUTO_DISCONNECT (1 << WMS_DC_EVENT_DISABLE_AUTO_DISCONNECT)

/** @ingroup client_pm_group
@{ */
/* Multimode Broadcas event masks. */
#define WMS_BC_MM_EVENT_MASK_CONFIG          (1 << WMS_BC_MM_EVENT_CONFIG)
#define WMS_BC_MM_EVENT_MASK_PREF            (1 << WMS_BC_MM_EVENT_PREF)
#define WMS_BC_MM_EVENT_MASK_TABLE           (1 << WMS_BC_MM_EVENT_TABLE)
#define WMS_BC_MM_EVENT_MASK_SRV_IDS         (1 << WMS_BC_MM_EVENT_SRV_IDS)
#define WMS_BC_MM_EVENT_MASK_SRV_INFO        (1 << WMS_BC_MM_EVENT_SRV_INFO)
#define WMS_BC_MM_EVENT_MASK_SRV_UPDATED     (1 << WMS_BC_MM_EVENT_SRV_UPDATED)
#define WMS_BC_MM_EVENT_MASK_ADD_SRVS        (1 << WMS_BC_MM_EVENT_ADD_SRVS)
#define WMS_BC_MM_EVENT_MASK_DELETE_SRVS     (1 << WMS_BC_MM_EVENT_DELETE_SRVS)
#define WMS_BC_MM_EVENT_MASK_SELECT_ALL_SRVS (1 << WMS_BC_MM_EVENT_SELECT_ALL_SRVS)
#define WMS_BC_MM_EVENT_MASK_SET_PRIORITY_ALL_SRVS (1 << WMS_BC_MM_EVENT_SET_PRIORITY_ALL_SRVS)
#define WMS_BC_MM_EVENT_MASK_READING_PREF    (1 << WMS_BC_MM_EVENT_READING_PREF)
#define WMS_BC_MM_EVENT_MASK_DELETE_ALL_SRVS (1 << WMS_BC_MM_EVENT_DELETE_ALL_SRVS)
#define WMS_BC_MM_EVENT_MASK_ENABLE_FAILURE  (1 << WMS_BC_MM_EVENT_ENABLE_FAILURE)
#define WMS_BC_MM_EVENT_MASK_DISABLE_FAILURE (1 << WMS_BC_MM_EVENT_DISABLE_FAILURE)

/** @ingroup client_pm_group
@{ */
/* Debug event masks. */
#define WMS_DBG_EVENT_MASK_RESET_TL_SEQ_NUM (1 << WMS_DBG_EVENT_RESET_TL_SEQ_NUM)
#define WMS_DBG_EVENT_MASK_SET_SIM_MESSAGE_NUMBER  (1 << WMS_DBG_EVENT_SET_SIM_MESSAGE_NUMBER)
#define WMS_DBG_EVENT_MASK_CLEAR_SMMA_FLAG  (1 << WMS_DBG_EVENT_CLEAR_SMMA_FLAG)
#define WMS_DBG_EVENT_MASK_RETRY_INTERVAL   (1 << WMS_DBG_EVENT_RETRY_INTERVAL)
#define WMS_DBG_EVENT_MASK_RETRY_PERIOD     (1 << WMS_DBG_EVENT_RETRY_PERIOD)

/** @ingroup client_pm_group
@{ */
/*=========================================================================
FUNCTION  wms_client_init_for_pm
===========================================================================*/
/**
  Allows a client to register itself with the API for the purpose of power 
  management.

  @param client_type  [IN]  Client type.
  @param *client_id_ptr [OUT] Pointer to the client ID.
  @param processor    [IN]  Processor type.

  @return
  Client error code.
   - WMS_CLIENT_ERR_RPC

  @dependencies
  Undetermined.

  @sideeffects
  Internal data is initialized for the new client.
*/
wms_client_err_e_type wms_client_init_for_pm
(
  wms_client_type_e_type       client_type,
  wms_client_id_type           * client_id_ptr,
  wms_client_processor_e_type  processor
);
/*~ FUNCTION wms_client_init_for_pm 
    RELEASE_FUNC wms_client_release(*client_id_ptr)
    ONERROR return WMS_CLIENT_ERR_RPC */
/*~ PARAM INOUT client_id_ptr POINTER */

/*=========================================================================
FUNCTION  wms_client_reg_cfg_cb_for_pm
===========================================================================*/
/**
  Allows a client to register its configuration event callback function with 
  event notification filtering preferences for the purpose of power management.\ 
  If a null pointer is passed, the callback is de-registered.

  @param client_id    [IN]  Client ID.
  @param cfg_notify_type [IN]  Filter type.
  @param cfg_event_mask [IN]  Configuration event mask.
  @param cfg_event_cb [IN]  Configuration event callback.

  @return
  Client error code.
   - WMS_CLIENT_ERR_RPC

  @dependencies
  Undetermined.

  @sideeffects
  Internal data is updated.
*/

wms_client_err_e_type wms_client_reg_cfg_cb_for_pm
(
  wms_client_id_type           client_id,
  wms_pm_notify_e_type         cfg_notify_type,
  uint32                       cfg_event_mask,
  wms_cfg_event_cb_type        cfg_event_cb
);
/*~ FUNCTION wms_client_reg_cfg_cb_for_pm 
    RELEASE_FUNC wms_client_deactivate(client_id)
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_reg_msg_cb_for_pm
===========================================================================*/
/**
  Allows a client to register its event callback function with event notification 
  filtering preferences for the purpose of power management.\ If a null pointer 
  is passed, the callback is de-registered.

  @param client_id    [IN]  Client ID.
  @param msg_notify_type    Filter type.
  @param msg_event_mask     Message event mask.
  @param msg_event_cb       Message event callback callback.

  @return
  Client error code.
  - WMS_CLIENT_ERR_RPC

  @dependencies
  Undetermined.

  @sideeffects
  Internal data is updated.
*/

wms_client_err_e_type wms_client_reg_msg_cb_for_pm
(
  wms_client_id_type           client_id,
  wms_pm_notify_e_type         msg_notify_type,
  uint32                       msg_event_mask,
  wms_msg_event_cb_type        msg_event_cb
);
/*~ FUNCTION wms_client_reg_msg_cb_for_pm 
    RELEASE_FUNC wms_client_deactivate(client_id)
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_reg_dc_cb_for_pm
===========================================================================*/
/**
  Allows a client to register its event callback function with event notification 
  filtering preferences for the purpose of power management.\ If a null pointer 
  is passed, the callback is de-registered.

  @param client_id    [IN]  Client ID.
  @param dc_notify_type [IN]  Filter type.
  @param dc_event_mask [IN]  DC event mask.
  @param dc_event_cb  [IN]  DC event callback callback.

  @return
  Client error code.
  - WMS_CLIENT_ERR_RPC

  @dependencies
  Undetermined.

  @sideeffects
  Internal data is updated.
*/

wms_client_err_e_type wms_client_reg_dc_cb_for_pm
(
  wms_client_id_type           client_id,
  wms_pm_notify_e_type         dc_notify_type,
  uint32                       dc_event_mask,
  wms_dc_event_cb_type         dc_event_cb
);
/*~ FUNCTION wms_client_reg_dc_cb_for_pm 
    RELEASE_FUNC wms_client_deactivate(client_id)
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_reg_bc_mm_cb_for_pm
===========================================================================*/
/**
  Allows a client to register its event callback function with event notification 
  filtering preferences for the purpose of power management.\ If a null pointer 
  is passed, the callback is de-registered.

  @param client_id    [IN]  Client ID.
  @param bc_mm_notify_type [IN]  Filter type.
  @param bc_mm_event_mask [IN]  Cell broadcast event mask.
  @param bc_mm_event_cb [IN]  Cell broadcast event callback callback.

  @return
  Client error code.
  - WMS_CLIENT_ERR_RPC

  @dependencies
  Undetermined.

  @sideeffects
  Internal data is updated.
*/

wms_client_err_e_type wms_client_reg_bc_mm_cb_for_pm
(
  wms_client_id_type           client_id,
  wms_pm_notify_e_type         bc_mm_notify_type,
  uint32                       bc_mm_event_mask,
  wms_bc_mm_event_cb_type      bc_mm_event_cb
);
/*~ FUNCTION wms_client_reg_bc_mm_cb_for_pm 
    RELEASE_FUNC wms_client_deactivate(client_id)
    ONERROR return WMS_CLIENT_ERR_RPC */

/*=========================================================================
FUNCTION  wms_client_reg_dbg_cb_for_pm
===========================================================================*/
/**
  Allows a client to register its event callback function with event notification 
  filtering preferences for the purpose of power management.\ If a null pointer 
  is passed, the callback is de-registered.

  @param client_id    [IN]  Client ID.
  @param dbg_notify_type [IN]  Filter type.
  @param dbg_event_mask [IN]  Cell broadcast event mask.
  @param dbg_event_cb [IN]  Cell broadcast event callback callback.

  @return
  Client error code.
  - WMS_CLIENT_ERR_RPC

  @dependencies
  Undetermined.

  @sideeffects
  Internal data is updated.
*/

wms_client_err_e_type wms_client_reg_dbg_cb_for_pm
(
  wms_client_id_type           client_id,
  wms_pm_notify_e_type         dbg_notify_type,
  uint32                       dbg_event_mask,
  wms_dbg_event_cb_type        dbg_event_cb
);
/*~ FUNCTION wms_client_reg_dbg_cb_for_pm 
    RELEASE_FUNC wms_client_deactivate(client_id)
    ONERROR return WMS_CLIENT_ERR_RPC */

#endif /* WMS_H */

#endif /* ! WMS_H_trimmed */
