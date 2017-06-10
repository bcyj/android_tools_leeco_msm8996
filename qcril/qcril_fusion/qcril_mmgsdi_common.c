/*!
  @file
  qcril_mmgsdi_common.c

  @brief
  Handles RIL basic requests for MMGSDI.

*/

/*===========================================================================

  Copyright (c) 2009 - 2010 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_mmgsdi_common.c#9 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
02/07/11   at      parse_ef_path() to update uim_file_ptr for by-path condition
01/25/11   at      Removed RIL_CardList parameter from card status request
10/14/10   at      Corrected input parameter checking for get_fdn
07/09/10   at      Merged branch changes to qcril_fusion folder
05/02/10   mib     Fixed file access for non supported paths
04/12/10   mib     Added FDN not available state
04/08/10   mib     Fixed incorrect path in qcril_mmgsdi_update_record
03/01/10   fc      Re-architecture to support split modem.
11/02/09   js      Check for valid client before sending command to MMGSDI.
10/24/09   js      QCRIL MMGSDI response function cleanup
08/17/09   js      Provide number of PIN retries in FDN response when incorrect
                   PIN entered
08/13/09   js      Fixed FDN PIN reties not sent in QCRIL response issue
07/23/09   js      Workaround to make SIM/USIM app type to use EF mapping always
                   for qcril_mmgsdi_get_response()
07/20/09   js      Added support to allow PIN2 verification to be performed
                   prior to enable/disable FDN commands
07/17/09   js      Fixed IMSI copy operation in qcril_mmgsdi_common_get_imsi_cnf
07/10/09   tml     Added support to allow PIN2 verification to be performed
                   prior to read/write sim io commands
                   Fixed USIM file access issue
07/08/09   js      Fixed IMSI copy operation in qcril_mmgsdi_common_get_imsi_cnf
06/26/09   fc      Fixed the issue of bogus RIL Request reported in call flow
                   log packet.
06/01/09   js      Fixed condition for appending file id to path in get_response
05/28/09   js      Appended file_id to path information in read_binary, read_record
                   get_response and update_binary processing
05/14/09   pg      Mainlined FEATURE_MULTIMODE_ANDROID.
04/05/09   fc      Cleanup log macros.
03/19/09   tml     Skip select rsp packing if the get file attribute returns
                   failed mmgsdi status.  Added status word for file not found.
                   Fixed IMSI return invalid value when QCRIL Debug is disabled
02/06/09   tml     reconstruct UICC select response into ICC select response
                   map ICC EF ID into corresponding USIM EF enum
01/26/09   fc      Logged assertion info.
01/09/09   tml     Featurize for 1x non ruim build
12/17/08   tml     Fixed FDN access pin2 mismatch issue, externed file mapping,
                   enabled get_imsi to request the right IMSI based on
                   card type and fixed null pointer checkings
10/08/08   tml     Added multimode support
08/04/08   tml     Update debug message.  Uncomment file_Select
                   api.  Fixed enabled/disable values
08/01/08   tml     Fixed file path parsing, get response issues
07/30/08   tml     Added SIM IO Get response support
07/22/08   tml     Fixed compilation on target
07/14/08   tml     Clean up message levels
07/15/08   jod     Lint cleanup for SIM IO and GET IMSI
07/11/08   jod     Added logging information
06/11/08   jod     Added support for GET IMSI and GET SIM STATUS
06/05/08   jod     Following review of libril.so (which does not do so),
                   free returned pointers. Improve out of memory handling.
06/04/08   jod     Bug fixes to parse_hex_ascii() and parse_ef_path()
05/28/08   jod     Add SIM IO functionality
05/23/08   tml     Added FDN enabling/disabling support
05/20/08   tml     Update formatting
05/19/08   tml     initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#if !defined (FEATURE_QCRIL_UIM_QMI)

#ifndef FEATURE_CDMA_NON_RUIM

#include <string.h>

#include "ril.h"
#include "IxErrno.h"
#include "qcrili.h"
#include "qcril_reqlist.h"
#include "qcril_mmgsdii.h"
#include "qcril_map.h"

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/
#if defined(_lint)
/* Lint does not recognise the __FUNCTION__ predefined macro. This is known
   to gcc and VS200x which are the compilers we care about for this code.
   Under Lint, define __FUNCTION__ to have the value of __func__ which is
   the 'official' C99 alternative (not supported by VS200x!!) */

#define __FUNCTION__ (__func__)
#endif

/*! EF maps have size 2^n, where n == EFMAP_SIZE, i.e. 128 buckets */
#define EFMAP_SIZE                 (7)
#define MMGSDI_STATUS_SIZE         (5)

/*! Put an integer value into a qcril_map */
#define MMGSDI_FT_TO_MAP(_n) ((void*) ((int) _n))

/*! Get an mmgsdi_file_type_enum_type out of a qcril_map */
#define MMGSDI_FT_FROM_MAP(_v) ((mmgsdi_file_enum_type)(_v))

/*! Constant (FALSE) used to indicate value in map is not a pointer type */
#define NOT_A_PTR  (1 == 0)
#define IS_A_PTR   (1 == 1)

/* MACROs to extract higher and lower 16 bits in a 32 bit number */
#define LOWORD(l) ((uint16)((uint32)(l) & 0xffff))
#define HIWORD(l) ((uint16)((uint32)(l) >> 16))

/* FDN enable/disable command values set by RIL */
#define QCRIL_GSDI_ENABLE_FDN_CMD        '1'
#define QCRIL_GSDI_DISABLE_FDN_CMD       '0'

/*! Valid command values for RIL_REQUEST_SIM_IO (per 27.007) */
typedef enum
{
  SIM_CMD_READ_BINARY   = 176, /*!< 27.007 +CRSM=READ BINARY */
  SIM_CMD_READ_RECORD   = 178, /*!< 27.007 +CRSM=READ RECORD */
  SIM_CMD_GET_RESPONSE  = 192, /*!< 27.007 +CRSM=GET RESPONSE */
  SIM_CMD_UPDATE_BINARY = 214, /*!< 27.007 +CRSM=UPDATE BINARY */
  SIM_CMD_RETRIEVE_DATA = 203, /*!< 27.007 +CRSM=RETRIEVE DATA */
  SIM_CMD_SET_DATA      = 219, /*!< 27.007 +CRSM=SET DATA */
  SIM_CMD_UPDATE_RECORD = 220, /*!< 27.007 +CRSM=UPDATE RECORD */
  SIM_CMD_STATUS        = 242  /*!< 27.007 +CRSM=STATUS */
} sim_command;

/*! Type used for packing sw1 and sw2 values (per 51.011 */
typedef struct Sw1Sw2
{
  int sw1;
  int sw2_present;
  int sw2;
} Sw1Sw2;

/*! Type used for the data tables defining MMGSDI response to 51.011
    response status conditions
*/
typedef struct StatusMap
{
  mmgsdi_return_enum_type mmgsdi;
  Sw1Sw2                  sw1sw2;
} StatusMap;

/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/*! Map containing mapping from EF value to MMGSDI item enum for the GSM
    SIM application
*/
static qcril_map efmap_gsm  = NULL;

/*! Map containing mapping from EF value to MMGSDI item enum for the UMTS
    USIM application
*/
static qcril_map efmap_umts = NULL;

/*! Map containing lookup of MMGSDI_STATUS to SW1 and SW2 */
static qcril_map status_map = NULL;

/*! Values for GSM SIM application mapping */
EFMap gsm_ef_mapping[] =
{
  { 0x2F00, {MMGSDI_DIR,         UIM_DIR} },

  { 0x2FE2, {MMGSDI_ICCID,       UIM_ICCID} },              /* ICC Identification */

  { 0x6F05, {MMGSDI_GSM_LP,      UIM_GSM_LP} },             /* Language Preference */
  { 0x6F07, {MMGSDI_GSM_IMSI,    UIM_GSM_IMSI} },           /* IMSI */
  { 0x6F20, {MMGSDI_GSM_KC,      UIM_GSM_KC} },             /* Ciphering Key Kc */
  { 0x6F30, {MMGSDI_GSM_PLMN,    UIM_GSM_PLMN} },           /* PLMN selector */
  { 0x6F31, {MMGSDI_GSM_HPLMN,   UIM_GSM_HPLMN} },          /* HPLMN search period */
  { 0x6F37, {MMGSDI_GSM_ACM_MAX, UIM_GSM_ACM_MAX} },        /* ACM Maximum value */
  { 0x6F38, {MMGSDI_GSM_SST,     UIM_GSM_SST} },            /* SIM Service table */
  { 0x6F39, {MMGSDI_GSM_ACM ,    UIM_GSM_ACM} },            /* Accumulated call meter */
  { 0x6F3E, {MMGSDI_GSM_GID1,    UIM_GSM_GID1} },           /* Group Identifier Level 1 */
  { 0x6F3F, {MMGSDI_GSM_GID2,    UIM_GSM_GID2} },           /* Group Identifier Level 2 */

  { 0x6F46, {MMGSDI_GSM_SPN,     UIM_GSM_SPN} },            /* Service Provider Name */
  { 0x6F41, {MMGSDI_GSM_PUCT,    UIM_GSM_PUCT} },           /* Price Per Unit and currency table */
  { 0x6F45, {MMGSDI_GSM_CBMI,    UIM_GSM_CBMI} },           /* Cell broadcast message identifier sel*/
  { 0x6F74, {MMGSDI_GSM_BCCH,    UIM_GSM_BCCH} },           /* Broadcast control channels */
  { 0x6F78, {MMGSDI_GSM_ACC,     UIM_GSM_ACC} },            /* Access control class */
  { 0x6F7B, {MMGSDI_GSM_FPLMN,   UIM_GSM_FPLMN} },          /* Forbidden PLMNs */
  { 0x6F7E, {MMGSDI_GSM_LOCI,    UIM_GSM_LOCI} },           /* Location information */
  { 0x6FAD, {MMGSDI_GSM_AD,      UIM_GSM_AD} },             /* Administrative data */
  { 0x6FAE, {MMGSDI_GSM_PHASE,   UIM_GSM_PHASE} },          /* Phase identification */
  { 0x6FB1, {MMGSDI_GSM_VGCS,    UIM_GSM_VGCS} },           /* Voice Group Call service */

  { 0x6FB2, {MMGSDI_GSM_VGCSS,       UIM_GSM_VGCSS} },          /* Voice Group Call service status */
  { 0x6FB3, {MMGSDI_GSM_VBS,         UIM_GSM_VBS} },            /* Voice Broadcast service  */
  { 0x6FB4, {MMGSDI_GSM_VBSS,        UIM_GSM_VBSS} },           /* Voice Broadcast service status */
  { 0x6FB5, {MMGSDI_GSM_EMLPP,       UIM_GSM_EMLPP} },          /* Enh multi level pre-emption and pri */
  { 0x6FB6, {MMGSDI_GSM_AAEM,        UIM_GSM_AAEM} },           /* Automatic Answer for eMLPP service */
  { 0x6F48, {MMGSDI_GSM_CBMID,       UIM_GSM_CBMID} },          /* Cell Broadcast Msg id for data dload */
  { 0x6FB7, {MMGSDI_GSM_ECC,         UIM_GSM_ECC} },            /* Emergency Call Codes */
  { 0x6F50, {MMGSDI_GSM_CBMIR,       UIM_GSM_CBMIR} },          /* Cell Broadcast Msg id range selection*/
  { 0x6F2C, {MMGSDI_GSM_DCK,         UIM_GSM_DCK} },            /* De-personalization control keys */
  { 0x6F32, {MMGSDI_GSM_CNL,         UIM_GSM_CNL} },            /* Co-operative network list */

  { 0x6F51, {MMGSDI_GSM_NIA,         UIM_GSM_NIA} },            /* Network's indication of alerting */
  { 0x6F52, {MMGSDI_GSM_KCGPRS,      UIM_GSM_KCGPRS} },         /* GPRS ciphering key */
  { 0x6F53, {MMGSDI_GSM_LOCIGPRS,    UIM_GSM_LOCIGPRS} },       /* GPRS location information */
  { 0x6F54, {MMGSDI_GSM_SUME,        UIM_GSM_SUME} },           /* Setup Menu elements */
  { 0x6F60, {MMGSDI_GSM_PLMNWACT,    UIM_GSM_PLMNWACT} },       /* PLMN Selector with Access technology */
  { 0x6F61, {MMGSDI_GSM_OPLMNWACT,   UIM_GSM_OPLMNWACT} },      /* Operator controlled PLMNWACT */
  { 0x6F62, {MMGSDI_GSM_HPLMNACT,    UIM_GSM_HPLMNACT} },       /* HPLNMN Access technology */
  { 0x6F63, {MMGSDI_GSM_CPBCCH,      UIM_GSM_CPBCCH} },         /* CPBCCH information */
  { 0x6F64, {MMGSDI_GSM_INVSCAN,     UIM_GSM_INVSCAN} },        /* Investigation PLMN Scan */
  { 0x6F65, {MMGSDI_GSM_RPLMNAT,     UIM_GSM_RPLMNAT} },        /* RPLMN  Last used Access Technology */

  { 0x6F11, {MMGSDI_GSM_VMWI,        UIM_GSM_VMWI} },           /* CPHS: Voice Mail Waiting Indicator */
  { 0x6F12, {MMGSDI_GSM_SVC_STR_TBL, UIM_GSM_SVC_STR_TBL} },    /* CPHS: Service String Table */
  { 0x6F13, {MMGSDI_GSM_CFF,         UIM_GSM_CFF} },            /* CPHS: Call Forwarding Flag */
  { 0x6F14, {MMGSDI_GSM_ONS,         UIM_GSM_ONS} },            /* CPHS: Operator Name String */
  { 0x6F15, {MMGSDI_GSM_CSP,         UIM_GSM_CSP} },            /* CPHS: Customer Service Profile */
  { 0x6F16, {MMGSDI_GSM_CPHSI,       UIM_GSM_CPHSI} },          /* CPHS: CPHS Information */
  { 0x6F17, {MMGSDI_GSM_MN,          UIM_GSM_MN} },             /* CPHS: Mailbox Number */
  { 0x6FC5, {MMGSDI_GSM_PNN,         UIM_GSM_PNN} },            /* PLMN Network Operator Name */
  { 0x6FC6, {MMGSDI_GSM_OPL,         UIM_GSM_OPL} },            /* Operator PLMN List */

  { 0x4F30, {MMGSDI_GSM_SAI,         UIM_GSM_SAI} },            /* SoLSA access indicator */
  { 0x4F31, {MMGSDI_GSM_SLL,         UIM_GSM_SLL} },            /* SoLSA LSA list */

  { 0x4F40, {MMGSDI_GSM_MEXE_ST,     UIM_GSM_MEXE_ST} },        /* MExE Service table */
  { 0x4F41, {MMGSDI_GSM_ORPK,        UIM_GSM_ORPK} },           /* Operator Root Public Key */
  { 0x4F42, {MMGSDI_GSM_ARPK,        UIM_GSM_ARPK} },           /* Administrator Root Public Key */
  { 0x4F43, {MMGSDI_GSM_TPRPK,       UIM_GSM_TPRPK} },          /* Third party Root public key */
  { 0x6FC7, {MMGSDI_GSM_MBDN,        UIM_GSM_MBDN} },           /* Mail Box Dialing Number */
  { 0x6FC8, {MMGSDI_GSM_EXT6,        UIM_GSM_EXT6} },           /* Extension 6 */
  { 0x6FC9, {MMGSDI_GSM_MBI,         UIM_GSM_MBI} },            /* Mail Box Identifier */
  { 0x6FCA, {MMGSDI_GSM_MWIS,        UIM_GSM_MWIS} },           /* Mail Box Waiting Indication */
  { 0x6FCD, {MMGSDI_GSM_SPDI,        UIM_GSM_SPDI} },           /* Service Provide Display Information */

  { 0x6FCB, {MMGSDI_GSM_CFIS,        UIM_GSM_CFIS} },           /* Call Forwarding Indicator Status */
  { 0x6F3A, {MMGSDI_TELECOM_ADN,     UIM_TELECOM_ADN} },        /* Abbreviated dialing number */
  { 0x6F3B, {MMGSDI_TELECOM_FDN,     UIM_TELECOM_FDN} },        /* Fixed dialing number */
  { 0x6F3C, {MMGSDI_TELECOM_SMS,     UIM_TELECOM_SMS} },        /* Short Message */
  { 0x6F3D, {MMGSDI_TELECOM_CCP,     UIM_TELECOM_CCP} },        /* Capability Configuration Parameters */
  { 0x6F4F, {MMGSDI_TELECOM_ECCP,    UIM_TELECOM_ECCP} },       /* Extended Capability Configuration Parameters */
  { 0x6F40, {MMGSDI_TELECOM_MSISDN,  UIM_TELECOM_MSISDN} },     /* MSISDN */
  { 0x6F42, {MMGSDI_TELECOM_SMSP,    UIM_TELECOM_SMSP} },       /* Short Message Parameters */
  { 0x6F43, {MMGSDI_TELECOM_SMSS,    UIM_TELECOM_SMSS} },       /* Short Message Status */
  { 0x6F44, {MMGSDI_TELECOM_LND,     UIM_TELECOM_LND} },        /* Last number dialled */

  { 0x6F49, {MMGSDI_TELECOM_SDN,     UIM_TELECOM_SDN} },        /* Speed dialling numbers */
  { 0x6F4A, {MMGSDI_TELECOM_EXT1,    UIM_TELECOM_EXT1} },       /* Extension 1 */
  { 0x6F4B, {MMGSDI_TELECOM_EXT2,    UIM_TELECOM_EXT2} },       /* Extension 2 */
  { 0x6F4C, {MMGSDI_TELECOM_EXT3,    UIM_TELECOM_EXT3} },       /* Extension 3 */
  { 0x6F4D, {MMGSDI_TELECOM_BDN,     UIM_TELECOM_BDN} },        /* Barred Dialling Number */
  { 0x6F4E, {MMGSDI_TELECOM_EXT4,    UIM_TELECOM_EXT4} },       /* Extension 4 */
  { 0x6F47, {MMGSDI_TELECOM_SMSR,    UIM_TELECOM_SMSR} },       /* Short Message Report */

  { 0,      {(mmgsdi_file_enum_type) 0, (uim_items_enum_type) 0} }   /* End of table indicator */
};

#ifndef FEATURE_ANDROID_SUPPORT_USIM
/*! Values for UMTS USIM application mapping - Simulating a 2G EF ID to the closest possible 3G EF enum */
EFMap umts_ef_mapping[] =
{
  { 0x2F00, {MMGSDI_DIR,                       UIM_DIR} },
  { 0x2FE2, {MMGSDI_ICCID,                     UIM_ICCID} },             /* ICC Identification */

  { 0x6F05, {MMGSDI_USIM_LI,                   UIM_USIM_LI} },           /* Language Indication */
  { 0x6F07, {MMGSDI_USIM_IMSI,                 UIM_USIM_IMSI} },         /* IMSI */
  { 0x6F20, {MMGSDI_USIM_KEYS,                 UIM_USIM_KEYS} },         /* Ciphering and Integrity keys - GSM_KC */
  { 0x6F08, {MMGSDI_USIM_KEYS,                 UIM_USIM_KEYS} },         /* Ciphering and Integrity keys */
  { 0x6F09, {MMGSDI_USIM_KEYSPS,               UIM_USIM_KEYSPS} },       /* C and I keys for pkt switched domain */
  { 0x6F60, {MMGSDI_USIM_PLMNWACT,             UIM_USIM_PLMNWACT} },     /* User controlled PLMN selector w/Acc Tech */
  { 0x6F30, {MMGSDI_USIM_PLMNWACT,             UIM_USIM_PLMNWACT} },     /* User controlled PLMN selector w/Acc Tech - GSM_PLMN */
  { 0x6F31, {MMGSDI_USIM_HPLMN,                UIM_USIM_HPLMN} },        /* HPLMN search period */

  { 0x6F37, {MMGSDI_USIM_ACM_MAX,              UIM_USIM_ACM_MAX} },      /* ACM maximum value */
  { 0x6F38, {MMGSDI_USIM_UST,                  UIM_USIM_UST} },          /* USIM Service table */
  { 0x6F39, {MMGSDI_USIM_ACM,                  UIM_USIM_ACM} },          /* Accumulated Call meter */
  { 0x6F3E, {MMGSDI_USIM_GID1,                 UIM_USIM_GID1} },         /* Group Identifier Level  */
  { 0x6F3F, {MMGSDI_USIM_GID2,                 UIM_USIM_GID2} },         /* Group Identifier Level 2 */
  { 0x6F46, {MMGSDI_USIM_SPN ,                 UIM_USIM_SPN} },          /* Service Provider Name */
  { 0x6F41, {MMGSDI_USIM_PUCT,                 UIM_USIM_PUCT} },         /* Price Per Unit and Currency table */
  { 0x6F45, {MMGSDI_USIM_CBMI,                 UIM_USIM_CBMI} },         /* Cell Bcast Msg identifier selection */
  { 0x6F78, {MMGSDI_USIM_ACC,                  UIM_USIM_ACC} },          /* Access control class */
  { 0x6F7B, {MMGSDI_USIM_FPLMN,                UIM_USIM_FPLMN} },        /* Forbidden PLMNs */

  { 0x6F7E, {MMGSDI_USIM_LOCI,                 UIM_USIM_LOCI} },         /* Location information */
  { 0x6FAD, {MMGSDI_USIM_AD,                   UIM_USIM_AD} },           /* Administrative data */
  { 0x6F48, {MMGSDI_USIM_CBMID,                UIM_USIM_CBMID} },        /* Cell Bcast msg id for data download */
  { 0x6FB7, {MMGSDI_USIM_ECC,                  UIM_USIM_ECC} },          /* Emergency call codes */
  { 0x6F50, {MMGSDI_USIM_CBMIR,                UIM_USIM_CBMIR} },        /* Cell bcast msg id range selection */
  { 0x6F73, {MMGSDI_USIM_PSLOCI,               UIM_USIM_PSLOCI} },       /* Packet switched location information */
  { 0x6F3B, {MMGSDI_USIM_FDN,                  UIM_USIM_FDN} },          /* Fixed dialling numbers */
  { 0x6F3C, {MMGSDI_USIM_SMS,                  UIM_USIM_SMS} },          /* Short messages */
  { 0x6F40, {MMGSDI_USIM_MSISDN,               UIM_USIM_MSISDN} },       /* MSISDN */
  { 0x6F42, {MMGSDI_USIM_SMSP,                 UIM_USIM_SMSP} },         /* SMS parameters */

  { 0x6F43, {MMGSDI_USIM_SMSS,                 UIM_USIM_SMSS} },         /* SMS Status */
  { 0x6F49, {MMGSDI_USIM_SDN,                  UIM_USIM_SDN} },          /* Service dialling numbers */
  { 0x6F4B, {MMGSDI_USIM_EXT2,                 UIM_USIM_EXT2} },         /* Extension 2 */
  { 0x6F4C, {MMGSDI_USIM_EXT3,                 UIM_USIM_EXT3} },         /* Extension 3 */
  { 0x6F47, {MMGSDI_USIM_SMSR,                 UIM_USIM_SMSR} },         /* SMS reports */
  { 0x6F80, {MMGSDI_USIM_ICI,                  UIM_USIM_ICI} },          /* Incoming call information */
  { 0x6F81, {MMGSDI_USIM_OCI,                  UIM_USIM_OCI} },          /* Outgoing call information */
  { 0x6F82, {MMGSDI_USIM_ICT,                  UIM_USIM_ICT} },          /* Incoming call timer */
  { 0x6F83, {MMGSDI_USIM_OCT,                  UIM_USIM_OCT} },          /* Outgoing call timer */
  { 0x6F4E, {MMGSDI_USIM_EXT5,                 UIM_USIM_EXT5} },         /* Extension 5 */

  { 0x6F4F, {MMGSDI_USIM_CCP2,                 UIM_USIM_CCP2} },         /* Capability Config Parameters 2 */
  { 0x6FB5, {MMGSDI_USIM_EMLPP,                UIM_USIM_EMLPP} },        /* Enh Multi Level Precedence and Pri */
  { 0x6FB6, {MMGSDI_USIM_AAEM,                 UIM_USIM_AAEM} },         /* Automatic answer for eMLPP service */
  { 0x6FC2, {MMGSDI_USIM_GMSI,                 UIM_USIM_GMSI} },         /* Group identity */
  { 0x6FC3, {MMGSDI_USIM_HIDDENKEY,            UIM_USIM_HIDDENKEY} },    /* Key for hidden phonebook entries */
  { 0x6F4D, {MMGSDI_USIM_BDN,                  UIM_USIM_BDN} },          /* Barred dialling numbers */
  { 0x6F55, {MMGSDI_USIM_EXT4,                 UIM_USIM_EXT4} },         /* Extension 4 */
  { 0x6F4E, {MMGSDI_USIM_EXT4,                 UIM_USIM_EXT4} },       /* Extension 4 - GSM-EXT4*/
  { 0x6F58, {MMGSDI_USIM_CMI,                  UIM_USIM_CMI} },          /* Comparison Method information */
  { 0x6F56, {MMGSDI_USIM_EST,                  UIM_USIM_EST} },          /* Enabled services table */

  { 0x6F57, {MMGSDI_USIM_ACL,                  UIM_USIM_ACL} },          /* Access Point Name Control List */
  { 0x6F2C, {MMGSDI_USIM_DCK,                  UIM_USIM_DCK} },          /* De-personalization Control Keys */
  { 0x6F32, {MMGSDI_USIM_CNL,                  UIM_USIM_CNL} },          /* Co-operative network list */
  { 0x6F5B, {MMGSDI_USIM_START_HFN,            UIM_USIM_START_HFN} },    /* Hyperframe number */
  { 0x6F5C, {MMGSDI_USIM_THRESHOLD,            UIM_USIM_THRESHOLD} },    /* Maximum value of Hyperframe number */
  { 0x6F61, {MMGSDI_USIM_OPLMNWACT,            UIM_USIM_OPLMNWACT} },    /* OPLMN selector with access tech*/
  { 0x6F5D, {MMGSDI_USIM_OPLMNSEL,             UIM_USIM_OPLMNSEL} },     /* OPLMN selector */
  { 0x6F62, {MMGSDI_USIM_HPLMNWACT,            UIM_USIM_HPLMNWACT} },    /* HPLMN selector with access technology */
  { 0x6F06, {MMGSDI_USIM_ARR,                  UIM_USIM_ARR} },          /* Access Rule reference */
  { 0x6F65, {MMGSDI_USIM_RPLMNACT,             UIM_USIM_RPLMNACT} },     /* RPLMN last used access tech */

  { 0x6FC4, {MMGSDI_USIM_NETPAR,               UIM_USIM_NETPAR} },       /* Network Parameters */
  { 0x6F11, {MMGSDI_USIM_VMWI,                 UIM_USIM_VMWI} },         /* CPHS: Voice Mail Waiting Indicator */
  { 0x6F12, {MMGSDI_USIM_SVC_STR_TBL,          UIM_USIM_SVC_STR_TBL} },  /* CPHS: Service String Table */
  { 0x6F13, {MMGSDI_USIM_CFF,                  UIM_USIM_CFF} },          /* CPHS: Call Forwarding Flag */
  { 0x6F14, {MMGSDI_USIM_ONS,                  UIM_USIM_ONS} },          /* CPHS: Operator Name String */
  { 0x6F15, {MMGSDI_USIM_CSP,                  UIM_USIM_CSP} },          /* CPHS: Customer Service Profile */
  { 0x6F16, {MMGSDI_USIM_CPHSI,                UIM_USIM_CPHSI} },        /* CPHS: CPHS Information */
  { 0x6F17, {MMGSDI_USIM_MN,                   UIM_USIM_MN} },           /* CPHS: Mailbox Number */
  { 0x6FC5, {MMGSDI_USIM_PNN,                  UIM_USIM_PNN} },          /* PLMN Network Name  */
  { 0x6FC6, {MMGSDI_USIM_OPL,                  UIM_USIM_OPL} },          /* Operator PLMN List */

  { 0x6F9F, {MMGSDI_USIM_7F40_PROP1_DFS,       UIM_USIM_ORANGE_DFS} },   /* Dynamic Flags Status */
  { 0x6F92, {MMGSDI_USIM_7F40_PROP1_D2FS,      UIM_USIM_ORANGE_D2FS} },  /* Dynamic2 Flag Setting */
  { 0x6F98, {MMGSDI_USIM_7F40_PROP1_CSP2,      UIM_USIM_ORANGE_CSP2} },  /* Customer Service Profile Line2*/
  { 0x6F9B, {MMGSDI_USIM_7F40_PROP1_PARAMS ,   UIM_USIM_ORANGE_PARAMS} },/* EF PARAMS - Welcome Message */
  { 0x4F30, {MMGSDI_TELECOM_PBR,               UIM_TELECOM_PBR} },           /* Was UIM_USIM_PBR - Phone book reference file */
  { 0x4F22, {MMGSDI_USIM_PSC,                  UIM_USIM_PSC} },          /* Phone book synchronization center */
  { 0x4F23, {MMGSDI_USIM_CC,                   UIM_USIM_CC} },           /* Change counter */
  { 0x4F24, {MMGSDI_USIM_PUID,                 UIM_USIM_PUID} },         /* Previous Unique Identifier */
  { 0x4F20, {MMGSDI_USIM_KC,                   UIM_USIM_KC} },           /* GSM ciphering key Kc */
  { 0x4F52, {MMGSDI_USIM_KCGPRS,               UIM_USIM_KCGPRS} },       /* GPRS ciphering key */

  { 0x4F63, {MMGSDI_USIM_CPBCCH,               UIM_USIM_CPBCCH} },       /* CPBCCH information */
  { 0x4F64, {MMGSDI_USIM_INVSCAN,              UIM_USIM_INVSCAN} },      /* Investigation scan */
  { 0x4F40, {MMGSDI_USIM_MEXE_ST,              UIM_USIM_MEXE_ST} },      /* MExE Service table */
  { 0x4F41, {MMGSDI_USIM_ORPK,                 UIM_USIM_ORPK} },         /* Operator Root Public Key */
  { 0x4F42, {MMGSDI_USIM_ARPK,                 UIM_USIM_ARPK} },         /* Administrator Root Public Key */
  { 0x4F43, {MMGSDI_USIM_TPRPK,                UIM_USIM_TPRPK} },        /* Third party Root public key */
  { 0x6FC7, {MMGSDI_USIM_MBDN,                 UIM_USIM_MBDN} },         /* Mail Box Dialing Number*/
  { 0x6FC8, {MMGSDI_USIM_EXT6,                 UIM_USIM_EXT6} },         /* Extension 6 */
  { 0x6FC9, {MMGSDI_USIM_MBI,                  UIM_USIM_MBI} },          /* Mailbox Identifier */
  { 0x6FCA, {MMGSDI_USIM_MWIS,                 UIM_USIM_MWIS} },         /* Message Waiting Indication Status */

  { 0x6FCD, {MMGSDI_USIM_SPDI,                 UIM_USIM_SPDI} },         /* Service Provider Display Information */
  { 0x6FD2, {MMGSDI_USIM_7F66_PROP1_SPT_TABLE, UIM_USIM_SPT_TABLE} },    /* Cingular TST */
  { 0x6FD9, {MMGSDI_USIM_EHPLMN ,              UIM_USIM_EHPLMN} },       /* Equivalent HPLMN  */
  { 0x6FCB, {MMGSDI_USIM_CFIS,                 UIM_USIM_CFIS} },         /* Call Forwarding Indicator Status */
  /* fake USIM file mapping */
  { 0x6F3A, {MMGSDI_TELECOM_ADN,               UIM_TELECOM_ADN} },        /* Abbreviated dialing number GSM_ADN */
  { 0x6F3D, {MMGSDI_TELECOM_CCP,               UIM_TELECOM_CCP} },        /* Capability Configuration Parameters GSM_CCP */
  { 0x6F4A, {MMGSDI_TELECOM_EXT1,              UIM_TELECOM_EXT1} },       /* Extension 1 GSM_EXT1 */

  { 0,      {(mmgsdi_file_enum_type) 0,        (uim_items_enum_type) 0} } /* End of table indicator */
};
#else
/*! Values for UMTS USIM application mapping */
EFMap umts_ef_mapping[] =
{
  { 0x2F00, {MMGSDI_DIR,                       UIM_DIR} },

  { 0x2FE2, {MMGSDI_ICCID,                     UIM_ICCID} },             /* ICC Identification */

  { 0x6F05, {MMGSDI_USIM_LI,                   UIM_USIM_LI} },           /* Language Indication */
  { 0x6F07, {MMGSDI_USIM_IMSI,                 UIM_USIM_IMSI} },         /* IMSI */
  { 0x6F08, {MMGSDI_USIM_KEYS,                 UIM_USIM_KEYS} },         /* Ciphering and Integrity keys */
  { 0x6F09, {MMGSDI_USIM_KEYSPS,               UIM_USIM_KEYSPS} },       /* C and I keys for pkt switched domain */
  { 0x6F60, {MMGSDI_USIM_PLMNWACT,             UIM_USIM_PLMNWACT} },     /* User controlled PLMN selector w/Acc Tech */
  { 0x6F30, {MMGSDI_USIM_UPLMNSEL,             UIM_USIM_UPLMNSEL} },     /* User controlled PLMN selector */
  { 0x6F31, {MMGSDI_USIM_HPLMN,                UIM_USIM_HPLMN} },        /* HPLMN search period */
  { 0x6F37, {MMGSDI_USIM_ACM_MAX,              UIM_USIM_ACM_MAX} },      /* ACM maximum value */
  { 0x6F38, {MMGSDI_USIM_UST,                  UIM_USIM_UST} },          /* USIM Service table */
  { 0x6F39, {MMGSDI_USIM_ACM,                  UIM_USIM_ACM} },          /* Accumulated Call meter */

  { 0x6F3E, {MMGSDI_USIM_GID1,                 UIM_USIM_GID1} },         /* Group Identifier Level  */
  { 0x6F3F, {MMGSDI_USIM_GID2,                 UIM_USIM_GID2} },         /* Group Identifier Level 2 */
  { 0x6F46, {MMGSDI_USIM_SPN ,                 UIM_USIM_SPN} },          /* Service Provider Name */
  { 0x6F41, {MMGSDI_USIM_PUCT,                 UIM_USIM_PUCT} },         /* Price Per Unit and Currency table */
  { 0x6F45, {MMGSDI_USIM_CBMI,                 UIM_USIM_CBMI} },         /* Cell Bcast Msg identifier selection */
  { 0x6F78, {MMGSDI_USIM_ACC,                  UIM_USIM_ACC} },          /* Access control class */
  { 0x6F7B, {MMGSDI_USIM_FPLMN,                UIM_USIM_FPLMN} },        /* Forbidden PLMNs */
  { 0x6F7E, {MMGSDI_USIM_LOCI,                 UIM_USIM_LOCI} },         /* Location information */
  { 0x6FAD, {MMGSDI_USIM_AD,                   UIM_USIM_AD} },           /* Administrative data */
  { 0x6F48, {MMGSDI_USIM_CBMID,                UIM_USIM_CBMID} },        /* Cell Bcast msg id for data download */

  { 0x6FB7, {MMGSDI_USIM_ECC,                  UIM_USIM_ECC} },          /* Emergency call codes */
  { 0x6F50, {MMGSDI_USIM_CBMIR,                UIM_USIM_CBMIR} },        /* Cell bcast msg id range selection */
  { 0x6F73, {MMGSDI_USIM_PSLOCI,               UIM_USIM_PSLOCI} },       /* Packet switched location information */
  { 0x6F3B, {MMGSDI_USIM_FDN,                  UIM_USIM_FDN} },          /* Fixed dialling numbers */
  { 0x6F3C, {MMGSDI_USIM_SMS,                  UIM_USIM_SMS} },          /* Short messages */
  { 0x6F40, {MMGSDI_USIM_MSISDN,               UIM_USIM_MSISDN} },       /* MSISDN */
  { 0x6F42, {MMGSDI_USIM_SMSP,                 UIM_USIM_SMSP} },         /* SMS parameters */
  { 0x6F43, {MMGSDI_USIM_SMSS,                 UIM_USIM_SMSS} },         /* SMS Status */
  { 0x6F49, {MMGSDI_USIM_SDN,                  UIM_USIM_SDN} },          /* Service dialling numbers */
  { 0x6F4B, {MMGSDI_USIM_EXT2,                 UIM_USIM_EXT2} },         /* Extension 2 */

  { 0x6F4C, {MMGSDI_USIM_EXT3,                 UIM_USIM_EXT3} },         /* Extension 3 */
  { 0x6F47, {MMGSDI_USIM_SMSR,                 UIM_USIM_SMSR} },         /* SMS reports */
  { 0x6F80, {MMGSDI_USIM_ICI,                  UIM_USIM_ICI} },          /* Incoming call information */
  { 0x6F81, {MMGSDI_USIM_OCI,                  UIM_USIM_OCI} },          /* Outgoing call information */
  { 0x6F82, {MMGSDI_USIM_ICT,                  UIM_USIM_ICT} },          /* Incoming call timer */
  { 0x6F83, {MMGSDI_USIM_OCT,                  UIM_USIM_OCT} },          /* Outgoing call timer */
  { 0x6F4E, {MMGSDI_USIM_EXT5,                 UIM_USIM_EXT5} },         /* Extension 5 */
  { 0x6F4F, {MMGSDI_USIM_CCP2,                 UIM_USIM_CCP2} },         /* Capability Config Parameters 2 */
  { 0x6FB5, {MMGSDI_USIM_EMLPP,                UIM_USIM_EMLPP} },        /* Enh Multi Level Precedence and Pri */
  { 0x6FB6, {MMGSDI_USIM_AAEM,                 UIM_USIM_AAEM} },         /* Automatic answer for eMLPP service */

  { 0x6FC2, {MMGSDI_USIM_GMSI,                 UIM_USIM_GMSI} },         /* Group identity */
  { 0x6FC3, {MMGSDI_USIM_HIDDENKEY,            UIM_USIM_HIDDENKEY} },    /* Key for hidden phonebook entries */
  { 0x6F4D, {MMGSDI_USIM_BDN,                  UIM_USIM_BDN} },          /* Barred dialling numbers */
  { 0x6F55, {MMGSDI_USIM_EXT4,                 UIM_USIM_EXT4} },         /* Extension 4 */
  { 0x6F58, {MMGSDI_USIM_CMI,                  UIM_USIM_CMI} },          /* Comparison Method information */
  { 0x6F56, {MMGSDI_USIM_EST,                  UIM_USIM_EST} },          /* Enabled services table */
  { 0x6F57, {MMGSDI_USIM_ACL,                  UIM_USIM_ACL} },          /* Access Point Name Control List */
  { 0x6F2C, {MMGSDI_USIM_DCK,                  UIM_USIM_DCK} },          /* De-personalization Control Keys */
  { 0x6F32, {MMGSDI_USIM_CNL,                  UIM_USIM_CNL} },          /* Co-operative network list */
  { 0x6F5B, {MMGSDI_USIM_START_HFN,            UIM_USIM_START_HFN} },    /* Hyperframe number */

  { 0x6F5C, {MMGSDI_USIM_THRESHOLD,            UIM_USIM_THRESHOLD} },    /* Maximum value of Hyperframe number */
  { 0x6F61, {MMGSDI_USIM_OPLMNWACT,            UIM_USIM_OPLMNWACT} },    /* OPLMN selector with access tech*/
  { 0x6F5D, {MMGSDI_USIM_OPLMNSEL,             UIM_USIM_OPLMNSEL} },     /* OPLMN selector */
  { 0x6F62, {MMGSDI_USIM_HPLMNWACT,            UIM_USIM_HPLMNWACT} },    /* HPLMN selector with access technology */
  { 0x6F06, {MMGSDI_USIM_ARR,                  UIM_USIM_ARR} },          /* Access Rule reference */
  { 0x6F65, {MMGSDI_USIM_RPLMNACT,             UIM_USIM_RPLMNACT} },     /* RPLMN last used access tech */
  { 0x6FC4, {MMGSDI_USIM_NETPAR,               UIM_USIM_NETPAR} },       /* Network Parameters */
  { 0x6F11, {MMGSDI_USIM_VMWI,                 UIM_USIM_VMWI} },         /* CPHS: Voice Mail Waiting Indicator */
  { 0x6F12, {MMGSDI_USIM_SVC_STR_TBL,          UIM_USIM_SVC_STR_TBL} },  /* CPHS: Service String Table */
  { 0x6F13, {MMGSDI_USIM_CFF,                  UIM_USIM_CFF} },          /* CPHS: Call Forwarding Flag */

  { 0x6F14, {MMGSDI_USIM_ONS,                  UIM_USIM_ONS} },          /* CPHS: Operator Name String */
  { 0x6F15, {MMGSDI_USIM_CSP,                  UIM_USIM_CSP} },          /* CPHS: Customer Service Profile */
  { 0x6F16, {MMGSDI_USIM_CPHSI,                UIM_USIM_CPHSI} },        /* CPHS: CPHS Information */
  { 0x6F17, {MMGSDI_USIM_MN,                   UIM_USIM_MN} },           /* CPHS: Mailbox Number */
  { 0x6FC5, {MMGSDI_USIM_PNN,                  UIM_USIM_PNN} },          /* PLMN Network Name  */
  { 0x6FC6, {MMGSDI_USIM_OPL,                  UIM_USIM_OPL} },          /* Operator PLMN List */
  { 0x6F9F, {MMGSDI_USIM_7F40_PROP1_DFS,       UIM_USIM_ORANGE_DFS} },   /* Dynamic Flags Status */
  { 0x6F92, {MMGSDI_USIM_7F40_PROP1_D2FS,      UIM_USIM_ORANGE_D2FS} },  /* Dynamic2 Flag Setting */
  { 0x6F98, {MMGSDI_USIM_7F40_PROP1_CSP2,      UIM_USIM_ORANGE_CSP2} },  /* Customer Service Profile Line2*/
  { 0x6F9B, {MMGSDI_USIM_7F40_PROP1_PARAMS ,   UIM_USIM_ORANGE_PARAMS} },/* EF PARAMS - Welcome Message */

  { 0x4F30, {MMGSDI_TELECOM_PBR,               UIM_TELECOM_PBR} },           /* Was UIM_USIM_PBR - Phone book reference file */
  { 0x4F22, {MMGSDI_USIM_PSC,                  UIM_USIM_PSC} },          /* Phone book synchronization center */
  { 0x4F23, {MMGSDI_USIM_CC,                   UIM_USIM_CC} },           /* Change counter */
  { 0x4F24, {MMGSDI_USIM_PUID,                 UIM_USIM_PUID} },         /* Previous Unique Identifier */
  { 0x4F20, {MMGSDI_USIM_KC,                   UIM_USIM_KC} },           /* GSM ciphering key Kc */
  { 0x4F52, {MMGSDI_USIM_KCGPRS,               UIM_USIM_KCGPRS} },       /* GPRS ciphering key */
  { 0x4F63, {MMGSDI_USIM_CPBCCH,               UIM_USIM_CPBCCH} },       /* CPBCCH information */
  { 0x4F64, {MMGSDI_USIM_INVSCAN,              UIM_USIM_INVSCAN} },      /* Investigation scan */
  { 0x4F40, {MMGSDI_USIM_MEXE_ST,              UIM_USIM_MEXE_ST} },      /* MExE Service table */
  { 0x4F41, {MMGSDI_USIM_ORPK,                 UIM_USIM_ORPK} },         /* Operator Root Public Key */

  { 0x4F42, {MMGSDI_USIM_ARPK,                 UIM_USIM_ARPK} },         /* Administrator Root Public Key */
  { 0x4F43, {MMGSDI_USIM_TPRPK,                UIM_USIM_TPRPK} },        /* Third party Root public key */
  { 0x6FC7, {MMGSDI_USIM_MBDN,                 UIM_USIM_MBDN} },         /* Mail Box Dialing Number*/
  { 0x6FC8, {MMGSDI_USIM_EXT6,                 UIM_USIM_EXT6} },         /* Extension 6 */
  { 0x6FC9, {MMGSDI_USIM_MBI,                  UIM_USIM_MBI} },          /* Mailbox Identifier */
  { 0x6FCA, {MMGSDI_USIM_MWIS,                 UIM_USIM_MWIS} },         /* Message Waiting Indication Status */
  { 0x6FCD, {MMGSDI_USIM_SPDI,                 UIM_USIM_SPDI} },         /* Service Provider Display Information */
  { 0x6FD2, {MMGSDI_USIM_7F66_PROP1_SPT_TABLE, UIM_USIM_SPT_TABLE} },    /* Cingular TST */
  { 0x6FD9, {MMGSDI_USIM_EHPLMN ,              UIM_USIM_EHPLMN} },       /* Equivalent HPLMN  */
  { 0x6FCB, {MMGSDI_USIM_CFIS,                 UIM_USIM_CFIS} },         /* Call Forwarding Indicator Status */
  { 0,      {(mmgsdi_file_enum_type) 0,        (uim_items_enum_type) 0} } /* End of table indicator */
};
#endif /* FEATURE_ANDROID_SUPPORT_USIM */

/*! Mapping of MMGSDI status values to SW1 and SW2 valies for RIL_SIM_IO */
static StatusMap mmgsdi_to_sw1sw2[] =
{
  { MMGSDI_SUCCESS,                           { 0x90,  TRUE, 0x00 } },
  { MMGSDI_NO_EF_SELECTED,                    { 0x94,  TRUE, 0x00 } },
  { MMGSDI_INCORRECT_PARAMS,                  { 0x94,  TRUE, 0x02 } },
  { MMGSDI_EF_INCONSISTENT,                   { 0x94,  TRUE, 0x08 } },
  { MMGSDI_ACCESS_DENIED,                     { 0x98,  TRUE, 0x04 } },
  { MMGSDI_NOT_FOUND,                         { 0x94,  TRUE, 0x04 } },
  { MMGSDI_INVALIDATION_CONTRADICTION_STATUS, { 0x98,  TRUE, 0x10 } },
  { MMGSDI_INCORRECT_LENGTH,                  { 0x67,  TRUE, 0x00 } },
  { MMGSDI_UNKNOWN_INST_CLASS,                { 0x6E,  TRUE, 0x00 } },
  { MMGSDI_SIM_TECHNICAL_PROBLEMS,            { 0x6F,  TRUE, 0x00 } },
  { (mmgsdi_return_enum_type) 0,              {    0,     0,    0 } }
};

/*! List of supported paths for access by enum */
static const char *qcril_mmgsdi_supported_paths[] =
{
  /* Paths without MF */
  "",
  "7F10",
  "7F20",
  "7F205F70",
  "7F205F3C",
  "7FFF",
  "7FFF5F3A",
  "7FFF5F3B",
  "7FFF5F3C",
  /* Paths with MF */
  "3F00",
  "3F007F10",
  "3F007F20",
  "3F007F205F70",
  "3F007F205F3C",
  "3F007FFF",
  "3F007FFF5F3A",
  "3F007FFF5F3B",
  "3F007FFF5F3C"
};

uint32 umts_ef_map_size = sizeof(umts_ef_mapping)/sizeof(EFMap);
uint32 gsm_ef_map_size = sizeof(gsm_ef_mapping)/sizeof(EFMap);

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/



/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/



/*=========================================================================*/
/*! @brief convert hex ASCII into 4 bits of binary data

    @pre   @a ch must be in the range for ascii representation for digits
              and a - f/A - F.   Any other value will result in an QCRIL_ASSERT.
    @param  ch Value to convert to 4 bits.
    @return Character in range [0x00 - 0x0F]
 */
/*=========================================================================*/
static uint8 hexchar_to_bin(char ch)
{

  if (ch >= 0x30 && ch <= 0x39)
  {
    return (ch - '0');
  }
  else if (ch >= 0x41 && ch <= 0x46)  /* A - F */
  {
    return (ch - 'A' + 10);
  }
  else if (ch >= 0x61 && ch <= 0x66)  /* a - f */
  {
    return (ch - 'a' + 10);
  }
  else
  {
    QCRIL_ASSERT(0);
  }
  return 0;
}


/*=========================================================================*/
/*! @brief Convert an array of hex string into binary bytes.

    @note If memory allocation fails, will return NULL. Caller must be
          able to deal with this case.

    @param str        Pointer to allocated area of null terminated char.
                      Must not be NULL.
    @param length_ptr Length (in bytes) of the data returned.  Must not be
                      NULL.
    @return           An uint8 pointer containing binary representation.
 */
/*=========================================================================*/
uint8* alloc_hexstring_to_bin
(
  const char* const str,
  mmgsdi_len_type *length_ptr
)
{
  int    i;
  uint8* out_ptr = NULL;

  QCRIL_ASSERT(str != NULL);
  QCRIL_ASSERT(length_ptr != NULL);

  *length_ptr = strlen(str)/2 + strlen(str)%2;
  if (*length_ptr == 0)
  {
    return out_ptr;
  }

  out_ptr = malloc(*length_ptr);

  if (out_ptr != NULL)
  {
    memset(out_ptr, 0, *length_ptr);

    for (i = 0; i < (*length_ptr - 1); i++)
    {
      out_ptr[i] = hexchar_to_bin(str[2*i]) << 4;
      out_ptr[i] = out_ptr[i] | (hexchar_to_bin(str[(2*i)+1]) & 0x0F);
    }
    if (strlen(str)%2 == 0)
    {
      out_ptr[i] = hexchar_to_bin(str[2*i]) << 4;
      out_ptr[i] = out_ptr[i] | (hexchar_to_bin(str[(2*i)+1]) & 0x0F);
    }
    else
    {
      out_ptr[i] = hexchar_to_bin(str[2*i]) << 4;
    }
  }
  return out_ptr;
} /* alloc_hexstring_to_bin */


/*=========================================================================*/
/*! @brief Determine the length of a string up to a maximum, with no
    side effects.

    @pre @a str is not NULL, and should point to a NULL terminated string.
    @param str Pointer to the string to scan.
    @param max_len Maximum permissible length of string.
    @return Number of characters in string excluding NULL character,
            or 0 if the string exceeds the maximum permitted length.
 */
/*=========================================================================*/
size_t safe_strlen(const char* const str, size_t max_len)
{
  const char* elem = str;
  size_t i = 0;

  QCRIL_ASSERT(str != NULL);

  while ((i < max_len) && (*elem != 0))
  {
    i++;
    elem++;
  }

  if (i < max_len)
    return i;
  else
    return 0;
}


/*=========================================================================*/
/*! @brief convert the lower four bits of a byte into hex ASCII.

    @pre   @a ch must be less than 0x10. This is QCRIL_ASSERTed in code.
    @param  ch Value to convert to hex ASCII.
    @return Character in range [0-9a-f]
 */
/*=========================================================================*/
static char bin_to_hexchar(const uint8 ch)
{
  QCRIL_ASSERT(ch < 0x10);

  if (ch < 0x0a)
    return (ch + '0');
  else
    return (ch + 'a' - 10);
}


/*=========================================================================*/
/*! @brief Convert an array of binary bytes into a hex string.

    @note If memory allocation fails, will return NULL. Caller must be
          able to deal with this case.

    @param data   Pointer to allocated area of bytes. Must not be NULL.
    @param length Length (in bytes) of the data pointed to by @a data
    @return       A NULL terminated string containing a hex ASCII
                  representation of the bytes in @a data.
 */
/*=========================================================================*/
char* bin_to_hexstring
(
 const uint8* const data,
 const mmgsdi_len_type length
)
{
  int   i;
  char* str;

  QCRIL_ASSERT(data != NULL);
  str = malloc((2 * ((size_t) length)) + 1);

  if (str != NULL)
  {
    memset(str, 0, (2 * ((size_t) length)) + 1);

    for (i = 0; i < length; i++)
    {
      str[2*i]   = bin_to_hexchar(data[i] >> 4);
      str[(2*i)+1] = bin_to_hexchar(data[i] & 0x0f);
    }
  }
  return str;
}


/*=========================================================================*/
/*! @brief Parse a C string consisting of only hexadecimal characters.

    @param path [in] Pointer to the value in a hex ASCII string
                which we wish to parse. This is either a sequence of
                four digits or a \0 character (in which case we return
                NULL.
    @param dir  [out] Pointer to the value parsed. This is only valid
                if a non-NULL value is returned.
    @return Pointer to the next position in the hex string to parse, or
            NULL if parse failed for some reason.
 */
/*=========================================================================*/
static const char* parse_hex_ascii
(
 const char* path,
 uint16*     dir
)
{
  uint16 i;
  size_t   check_len;
  QCRIL_ASSERT(path != NULL);

  QCRIL_LOG_DEBUG( "parse_hex_ascii(path=%s)\n", path);

  /* Base case: we have consumed the whole string*/
  if (*path == '\0')
    return NULL;

  /* Normal case: we expect at least four characters matching regex [0-9a-fA-F] */
  check_len = safe_strlen(path, MAX_STR);
  QCRIL_ASSERT(check_len >= 4);
  *dir = 0;

  for (i = 0; i < 4; i++)
  {
    if ((path[i] >= '0') && (path[i] <= '9'))
      *dir |= ((uint16) (path[i] - '0')) << ((3-i) << 2);
    else if ((path[i] >= 'a') && (path[i] <= 'f'))
      *dir |= ((uint16) (path[i] - 'a' + 10)) << ((3-i) << 2);
    else if ((path[i] >= 'A') && (path[i] <= 'F'))
      *dir |= ((uint16) (path[i] - 'A' + 10)) << ((3-i) << 2);
    else
    {
      QCRIL_LOG_ERROR( "%s: Illegal character\n", __FUNCTION__);
      return NULL;
    }
  }

  /* Fall through means success - return pointer to next unread character */
  return &path[i];
}


/*=========================================================================*/
/*! @brief Translate path from Android RIL format to MMGSDILIB format.

    Android requests information in a format dervied from the AT+CRSM
    command, which is described in detail in 3GPP TS 27.007. The components
    @a path parameter is passed to this function to be translated into
    the format used by MMGSDI.

    The rule followed is that if @a path is not NULL, the MMGSDI path will
    be defined using the MMGSDI_BY_PATH_ACCESS approach, assuming that <b>
    the entire path from MF is provided</b>, which is per specification.
    In this format, the file identify of the MF <b>shall not</b> be provided
    in @a path, but the remainder of the path shall be fully specified.

    @param fileid          [in] EF identifier.
    @param path_ptr        [in] Path to EF expressed in the form "select by
                                path from MF" as per 27.007 section 8.18
                                and ETSI TS 102 221 section 8.4.2.
    @param access_type_ptr [out] Constructed mmgsdi_access_type structure.
                                 This is only valid on return if function
                                 call returned TRUE.
    @return            TRUE if function completed successfully, else FALSE.
 */
/*=========================================================================*/
static boolean qcril_mmgsdi_convert_path
(
  const int           fileid,
  const char         *path_ptr,
  mmgsdi_access_type *access_type_ptr
)
{

  size_t      len_check;
  uint16      dir;
  const char* cur_path_ptr = NULL;

  if ((access_type_ptr == NULL) || (path_ptr == NULL))
  {
    QCRIL_LOG_ERROR( "%s: NULL access_type_ptr or path_ptr\n", __FUNCTION__);
    return FALSE;
  }

  len_check = safe_strlen(path_ptr, MAX_STR);
  if((len_check / 4) <= 0)
  {
    /* Path at least 4 digits */
    QCRIL_LOG_ERROR( "%s: Path too short len: 0x%x\n", __FUNCTION__, len_check);
    return FALSE;
  }
  if((len_check % 4) != 0)
  {
    /* Path length is multiple of 4 digits */
    QCRIL_LOG_ERROR( "%s: Path not divisible by 4 len: 0x%x\n", __FUNCTION__, len_check);
    return FALSE;
  }

  /* First path_buf entry is always MF for MMGSDI call */
  access_type_ptr->access_method = MMGSDI_BY_PATH_ACCESS;
  access_type_ptr->file.path_type.path_len = 0;

  /* Iterate over the path string, filling in path_buf entries */
  QCRIL_LOG_VERBOSE( "%s", "Path: \n");
  cur_path_ptr = path_ptr;
  while ((cur_path_ptr != NULL) &&
         (access_type_ptr->file.path_type.path_len < MMGSDI_MAX_PATH_LEN))
  {
    cur_path_ptr = parse_hex_ascii(cur_path_ptr, &dir);
    if (cur_path_ptr != NULL)
    {
      QCRIL_LOG_VERBOSE( " %4hX \n", dir);
      access_type_ptr->file.path_type.path_buf[access_type_ptr->file.path_type.path_len++] = dir;
    }
  }

  return TRUE;
} /* qcril_mmgsdi_convert_path */


/*=========================================================================*/
/*! @brief Translate pathid from Android RIL format to MMGSDILIB format.

    Android requests information in a format dervied from the AT+CRSM
    command, which is described in detail in 3GPP TS 27.007. The components
    of this information whcih determine the file to be accessed on the
    card are the @a fileid and @a path parameters, which arepassed to this
    function to be translated into the format used by MMGSDI.

    The rule followed is that if @a path is not NULL, the MMGSDI path will
    be defined using the MMGSDI_BY_PATH_ACCESS approach, assuming that <b>
    the entire path from MF is provided</b>, which is per specification.
    In this format, the file identify of the MF <b>shall not</b> be provided
    in @a path, but the remainder of the path shall be fully specified.

    If @a path is NULL, we will use MMGSDI_EF_ENUM_ACCESS. In this case, EF
    identifiers are translated to mmgsdi_file_enum_type using one of the
    efmap_gsm or efmap_umts hash tables.

    @param fileid          [in] EF identifier.
    @param path_ptr        [in] Path to EF expressed in the form "select by
                                path from MF" as per 27.007 section 8.18
                                and ETSI TS 102 221 section 8.4.2.
    @param access_type_ptr [out] Constructed mmgsdi_access_type structure.
                                 This is only valid on return if function
                                 call returned TRUE.
    @param uim_file_ptr    [out] Optional parameter if caller wants to
                                 receive the uim_item_enum_type value.
                                 This is only valid on return if function
                                 call returned TRUE.
    @return            TRUE if function completed successfully, else FALSE.
 */
/*=========================================================================*/
static int parse_ef_path
(
  const int            fileid,
  const char*          path_ptr,
  mmgsdi_access_type*  access_type_ptr,
  uim_items_enum_type* uim_file_ptr
)
{
  qcril_map   app_map;
  FileEnums*  ef_ptr   = NULL;
  int         i        = 0;
  boolean     use_path = TRUE;

  QCRIL_LOG_DEBUG( "parse_ef_path(fileid=0x%x, path_ptr=%s)\n", fileid, path_ptr);

  if (access_type_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s: NULL access_type_ptr\n", __FUNCTION__);
    return FALSE;
  }

  /* If path is present, make sure it is one of the supported paths,
     otherwise jump directly to access by path. This is done to avoid
     problems in case of files with same file ids */
  if (path_ptr != NULL)
  {
    boolean supported_path = FALSE;

    for (i = 0; i < (int)QCRIL_ARR_SIZE(qcril_mmgsdi_supported_paths); i++)
    {
      if (strcasecmp(qcril_mmgsdi_supported_paths[i], path_ptr) == 0)
      {
        supported_path = TRUE;
        break;
      }
    }
    if (!supported_path)
    {
      /* The path is not supported for access by enaum... directly jump
         to path conversion, to avoid possible conflicts */
      QCRIL_LOG_DEBUG( "parse_ef_path: unsupported path: %s\n", path_ptr);
      goto Convert_path;
    }
  }

  if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_TRUE)
  {
    /* Try to find if there is a SIM app, if so, proceed with the SIM app mapping
       table.
       This is done so that RUIM support will be preserved by doing the path copying
       route */
    for(i=0; i<qcril_mmgsdi.curr_card_status.num_applications; i++)
    {
      if(qcril_mmgsdi.curr_card_status.applications[i].app_type == RIL_APPTYPE_SIM)
      {
        app_map  = efmap_gsm;
        use_path = FALSE;
      }
    }
  }
  else
  {
    app_map  = efmap_umts;
    use_path = FALSE;
  }

  memset(access_type_ptr, 0, sizeof(mmgsdi_access_type));

  /* Go through mapping table first regardless of whether path is provided or not
     This is true only for non RUIM case.
  */
  if(!use_path)
  {
    /* Simple case: attempt to use the mapping table */
    access_type_ptr->access_method  = MMGSDI_EF_ENUM_ACCESS;
    ef_ptr = (FileEnums*)(qcril_map_find(app_map, (const unsigned int) fileid));

    if (ef_ptr == QCRIL_MAP_NOT_PRESENT)
    {
      QCRIL_LOG_ERROR( "%s: ef_ptr is Null\n", __FUNCTION__);
      if(path_ptr == NULL)
      {
        return FALSE;
      }
      else
      {
        /* attempt with the whole path now */
        goto Convert_path;
      }
    }

    access_type_ptr->file.file_enum = ef_ptr->gsdi_enum;

    /* If lookup failed, invalidate returned result */
    if (access_type_ptr->file.file_enum == ((int) MMGSDI_NO_FILE_ENUM))
    {
      QCRIL_LOG_VERBOSE( "%s:requested EF: %d not present\n",
                        __FUNCTION__, fileid);
      if(path_ptr == NULL)
      {
        return FALSE;
      }
      else
      {
        /* attempt with the whole path now */
        goto Convert_path;
      }
    }

    if (uim_file_ptr != NULL)
    {
      /* populating the uim file ptr as well */
      *uim_file_ptr = ef_ptr->uim_enum;
    }
    return TRUE;
  }

Convert_path:
  if (path_ptr != NULL)
  {
    /* populating the uim file ptr as well */
    if (uim_file_ptr != NULL)
    {
      *uim_file_ptr = UIM_EF_BY_PATH;
    }

    /* Path information provided - a sequence of 16 bit hex ASCII values */
    return qcril_mmgsdi_convert_path(fileid, path_ptr, access_type_ptr);
  }
  return FALSE;
} /* parse_ef_path */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_read_transparent_ext

===========================================================================*/
/*!
    @brief utility function to invoke mmgsdi read transparent ext function call

    @param access [in]  the access type that the read record is to perform on.
    @param offset [in]  the offset to read from
    @param length [in]  the length of data to be read.
    @param token  [in]  the token for the command request list.
*/
/*=========================================================================*/
static void qcril_mmgsdi_common_read_transparent_ext(
  qcril_instance_id_e_type        instance_id,
  qcril_modem_id_e_type           modem_id,
  mmgsdi_access_type              access,
  mmgsdi_len_type                 offset,
  mmgsdi_len_type                 length,
  mmgsdi_client_data_type         token)
{
  char                    details[80];
  int                     i           = 0;
  mmgsdi_return_enum_type status      = MMGSDI_SUCCESS;

  memset(details, 0x00, sizeof(details));
  QCRIL_SNPRINTF( details, sizeof( details ), "offset=%d, length=%d",
                  (int) offset, (int) length );
  QCRIL_LOG_RPC2( modem_id, "mmgsdi_read_transparent_ext()", details );

  status = mmgsdi_read_transparent_ext(qcril_mmgsdi.client_id,
                                       MMGSDI_SLOT_1,
                                       access,
                                       offset,
                                       length,
                                       qcril_mmgsdi_command_callback,
                                       token);
  if (status != MMGSDI_SUCCESS)
  {
    QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, status);
    qcril_mmgsdi_response( instance_id, (RIL_Token)token, status, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_common_read_transparent_ext */


/*=========================================================================*/
/*! @brief Read a string of bytes from a transparent EF.

    This function is called from qcril_sim_io_request(), and performs read
    of transparent EF. Per ETSI TS 102 221 and 3GPP TS 27.007, the values
    in the RIL_SIM_IO structure pointed to by @a req are interpreted as
    follows:

    <table>
    <tr><th>Parameter</th><th>Interpretation</th></tr>
    <tr><td>fileid</td><td>EF identifier. May be modified by path</td></tr>
    <tr><td>path</td><td>Path from MF to requested EF. Optional</td></tr>
    <tr><td>p1</td><td>High part of offset to first byte to read</td></tr>
    <tr><td>p2</td><td>Low part of offset to first byte to read</td></tr>
    <tr><td>p3</td><td>Number of bytes to read</td></tr>
    </table>

    @pre @a req is not NULL (verified by caller).
    @post Radio state unchanged. MMGSDI callback expected.

    @param req [in]  params_ptr->data points to a RIL_SIM_IO structure.
 */
/*=========================================================================*/
static void qcril_mmgsdi_read_binary
(
 const qcril_request_params_type* const req
)
{
  /* Parameters checked (by QCRIL_ASSERT) in caller... */
  qcril_instance_id_e_type    instance_id  = req->instance_id;
  qcril_modem_id_e_type       modem_id     = req->modem_id;
  const RIL_SIM_IO_v6* const  request      = req->data;
  mmgsdi_return_enum_type     status       = MMGSDI_SUCCESS;
  mmgsdi_offset_type          offset       = (request->p1) << 8 | (request->p2);
  mmgsdi_len_type             length       = request->p3;
  mmgsdi_access_type          access;
  uint16                      fileid_low   = 0;

  /* Top bit of p1 shall be 0, per 3GPP TS 51.011 */
  QCRIL_ASSERT((request->p1 & 0x80) == 0);

  fileid_low = LOWORD(request->fileid);

  if (!parse_ef_path(request->fileid, request->path, &access, NULL))
  {
    QCRIL_LOG_ERROR( "%s failed to parse EF path\n", __FUNCTION__);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  /* Append EF ID to the path information */
  if((access.file.path_type.path_len < MMGSDI_MAX_PATH_LEN) &&
     (access.access_method == MMGSDI_BY_PATH_ACCESS))
  {
    memcpy((void *)&access.file.path_type.path_buf[access.file.path_type.path_len],
           (void *)&fileid_low,
           sizeof(uint16));
    access.file.path_type.path_len++;
  }

  if (request->pin2 != NULL)
  {
    mmgsdi_data_type                            pin_data;
    qcril_mmgsdi_internal_sim_data_type        *internal_data_ptr = NULL;

    /* Queue a verify PIN2 command first */

    /* Populate the mmgsdi required members */
    pin_data.data_len = (mmgsdi_len_type)(strlen(request->pin2));
    pin_data.data_ptr = (uint8*)request->pin2;

    /* malloc and populate internal data for subsequent processing */
    internal_data_ptr = malloc(sizeof(qcril_mmgsdi_internal_sim_data_type));
    if(internal_data_ptr == NULL)
    {
      QCRIL_LOG_ERROR( "%s: failed alloc internal_data_ptr\n", __FUNCTION__);
      qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
      return;
    }

    memset(internal_data_ptr, 0x00, sizeof(qcril_mmgsdi_internal_sim_data_type));
    internal_data_ptr->cmd = QCRIL_MMGSDI_ORIG_SIM_IO_READ_BINARY;
    memcpy(&internal_data_ptr->access, &access, sizeof(mmgsdi_access_type));
    internal_data_ptr->offset        = offset;
    internal_data_ptr->length        = length;
    internal_data_ptr->token         = req->t;
    internal_data_ptr->do_pin_verify = TRUE;

    QCRIL_LOG_RPC2( modem_id, "mmgsdi_verify_pin PIN2()", "" );
    qcril_mmgsdi_print_byte_data(pin_data.data_len, pin_data.data_ptr);
    status = mmgsdi_verify_pin (qcril_mmgsdi.client_id,
                                MMGSDI_SLOT_1,
                                MMGSDI_PIN2,
                                pin_data,
                                qcril_mmgsdi_internal_verify_pin_command_callback,
                                (mmgsdi_client_data_type)internal_data_ptr);
    if (status != MMGSDI_SUCCESS)
    {
      free(internal_data_ptr);
      QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, status);
      qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
    }
    /* set current pin to PIN2 */
    qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN2);
    return;
  }

  /* No pin2 was provided */
  qcril_mmgsdi_common_read_transparent_ext( instance_id,
                                            modem_id,
                                            access,
                                            offset,
                                            length,
                                            (mmgsdi_client_data_type)req->t );
} /* qcril_mmgsdi_read_binary */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_read_record_ext

===========================================================================*/
/*!
    @brief utility function to invoke mmgsdi read record ext function call

    @param access [in]  the access type that the read record is to perform on.
    @param record [in]  the record number to read
    @param length [in]  the length of data to be read.
    @param token  [in]  the token for the command request list.
*/
/*=========================================================================*/
static void qcril_mmgsdi_common_read_record_ext(
  qcril_instance_id_e_type        instance_id,
  qcril_modem_id_e_type           modem_id,
  mmgsdi_access_type              access,
  mmgsdi_rec_num_type             record,
  mmgsdi_len_type                 length,
  mmgsdi_client_data_type         token)
{
  char                    details[80];
  int                     i           = 0;
  mmgsdi_return_enum_type status      = MMGSDI_SUCCESS;

  memset(details, 0x00, sizeof(details));
  QCRIL_SNPRINTF( details, sizeof( details ), "record=%d, length=%d",
                  (int) record, (int) length );
  QCRIL_LOG_RPC2( modem_id, "mmgsdi_read_record_ext()", details );

  status = mmgsdi_read_record_ext(qcril_mmgsdi.client_id,
                                  MMGSDI_SLOT_1,
                                  access,
                                  record,
                                  length,
                                  qcril_mmgsdi_command_callback,
                                  token);
  if (status != MMGSDI_SUCCESS)
  {
    QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, status);
    qcril_mmgsdi_response( instance_id, (RIL_Token)token, status, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_common_read_record_ext */


/*=========================a================================================*/
/*! @brief Read a complete record in a linear or cyclic EF.

    This function is called from qcril_sim_io_request(), and performs read
    of a complete record. Per ETSI TS 102 221 and 3GPP TS 27.007, the values
    in the RIL_SIM_IO structure pointed to by @a req are interpreted as
    follows:

    <table>
    <tr><th>Parameter</th><th>Interpretation</th></tr>
    <tr><td>fileid</td><td>EF identifier. May be modified by path</td></tr>
    <tr><td>path</td><td>Path from MF to requested EF. Optional</td></tr>
    <tr><td>p1</td><td>Record number</td></tr>
    <tr><td>p2</td><td>Read mode</td></tr>
    <tr><td>p3</td><td>Bytes to read</td></tr>
    </table>

    @pre @a req is not NULL (verified by caller).
    @post Radio state unchanged. MMGSDI callback expected.

    @note Only 'absolute/current' mode for reading records is supported
          by MMGSDILIB, and thus by qcril_mmgsdi_read_record().

    @param req [in]  params_ptr->data points to a RIL_SIM_IO structure.
 */
/*=========================================================================*/
static void qcril_mmgsdi_read_record
(
 const qcril_request_params_type* const req
)
{
  qcril_instance_id_e_type    instance_id  = req->instance_id;
  qcril_modem_id_e_type       modem_id     = req->modem_id;
  const RIL_SIM_IO_v6* const  request      = req->data;
  mmgsdi_return_enum_type     status       = MMGSDI_SUCCESS;
  mmgsdi_len_type             length       = request->p3;
  mmgsdi_access_type          access;
  mmgsdi_rec_num_type         record       = request->p1;
  uint16                      fileid_low   = 0;

  /* p2 can take values 2, 3, 4, per 3GPP TS 51.011, however MMGSDI
   * does not support next record (2) or previous record (3) reads */
  if (request->p2 != 4)
  {
    QCRIL_LOG_ERROR( "%s: unsupported case P2 = %d\n", __FUNCTION__, request->p2);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_NOT_SUPPORTED, NULL, 0, TRUE, NULL );
    return;
  }

  fileid_low = LOWORD(request->fileid);

  if (!parse_ef_path(request->fileid, request->path, &access, NULL))
  {
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  /* Append EF ID to the path information */
  if((access.file.path_type.path_len < MMGSDI_MAX_PATH_LEN) &&
     (access.access_method == MMGSDI_BY_PATH_ACCESS))
  {
    memcpy((void *)&access.file.path_type.path_buf[access.file.path_type.path_len],
           (void *)&fileid_low,
           sizeof(uint16));
    access.file.path_type.path_len++;
  }

  if (request->pin2 != NULL)
  {
    mmgsdi_data_type                            pin_data;
    qcril_mmgsdi_internal_sim_data_type        *internal_data_ptr = NULL;

    /* Queue a verify PIN2 command first */

    /* Populate the mmgsdi required members */
    pin_data.data_len = (mmgsdi_len_type)(strlen(request->pin2));
    pin_data.data_ptr = (uint8*)request->pin2;

    /* malloc and populate internal data for subsequent processing */
    internal_data_ptr = malloc(sizeof(qcril_mmgsdi_internal_sim_data_type));
    if(internal_data_ptr == NULL)
    {
      QCRIL_LOG_ERROR( "%s: failed alloc internal_data_ptr \n", __FUNCTION__);
      qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
      return;
    }

    memset(internal_data_ptr, 0x00, sizeof(qcril_mmgsdi_internal_sim_data_type));
    internal_data_ptr->cmd = QCRIL_MMGSDI_ORIG_SIM_IO_READ_RECORD;
    memcpy(&internal_data_ptr->access, &access, sizeof(mmgsdi_access_type));
    internal_data_ptr->record        = record;
    internal_data_ptr->length        = length;
    internal_data_ptr->token         = req->t;
    internal_data_ptr->do_pin_verify = TRUE;

    QCRIL_LOG_RPC2( modem_id, "mmgsdi_verify_pin PIN2()", "" );
    qcril_mmgsdi_print_byte_data(pin_data.data_len, pin_data.data_ptr);
    status = mmgsdi_verify_pin (qcril_mmgsdi.client_id,
                                MMGSDI_SLOT_1,
                                MMGSDI_PIN2,
                                pin_data,
                                qcril_mmgsdi_internal_verify_pin_command_callback,
                                (mmgsdi_client_data_type)internal_data_ptr);
    if (status != MMGSDI_SUCCESS)
    {
      free(internal_data_ptr);
      QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, status);
      qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
    }
    /* set current pin to PIN2 */
    qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN2);
    return;
  }

  qcril_mmgsdi_common_read_record_ext( instance_id,
                                       modem_id,
                                       access,
                                       record,
                                       length,
                                       (mmgsdi_client_data_type)req->t );

} /* qcril_mmgsdi_read_record */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_write_transparent_ext

===========================================================================*/
/*!
    @brief utility function to invoke mmgsdi write record ext function call

    @param access [in]  the access type that the write record is to perform on.
    @param offset [in]  the offset from where the write should start
    @param data   [in]  the data to be written.
    @param token  [in]  the token for the command request list.
*/
/*=========================================================================*/
static void qcril_mmgsdi_common_write_transparent_ext(
  qcril_instance_id_e_type        instance_id,
  qcril_modem_id_e_type           modem_id,
  mmgsdi_access_type              access,
  mmgsdi_offset_type              offset,
  mmgsdi_data_type                data,
  mmgsdi_client_data_type         token)
{
  char                    details[80];
  int                     i           = 0;
  mmgsdi_return_enum_type status      = MMGSDI_SUCCESS;

  memset(details, 0x00, sizeof(details));
  QCRIL_SNPRINTF( details, sizeof( details ), "offset=%d", (int) offset);
  QCRIL_LOG_RPC2( modem_id, "mmgsdi_write_transparent_ext()", details );

  if(data.data_ptr && data.data_len > 0)
  {
    QCRIL_LOG_VERBOSE( "%s", "with data: [\n");
    for (i = 0; i < data.data_len; i++)
    {
      QCRIL_LOG_VERBOSE( "  0x%x,\n", data.data_ptr[i]);
    }
    QCRIL_LOG_VERBOSE( "%s", "]\n");
  }

  status = mmgsdi_write_transparent_ext(qcril_mmgsdi.client_id,
                                        MMGSDI_SLOT_1,
                                        access,
                                        offset,
                                        data,
                                        qcril_mmgsdi_command_callback,
                                        token);

  if (status != MMGSDI_SUCCESS)
  {
    QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, status);
    qcril_mmgsdi_response( instance_id, (RIL_Token)token, status, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_common_write_transparent_ext */


/*=========================================================================*/
/*! @brief Update a transparent EF with a string of bytes

    This function is called from qcril_sim_io_request(), and performs update
    of transparent EF. Per ETSI TS 102 221 and 3GPP TS 27.007, the values
    in the RIL_SIM_IO structure pointed to by @a req are interpreted as
    follows:

    <table>
    <tr><th>Parameter</th><th>Interpretation</th></tr>
    <tr><td>fileid</td><td>EF identifier. May be modified by path</td></tr>
    <tr><td>path</td><td>Path from MF to requested EF. Optional</td></tr>
    <tr><td>p1</td><td>High part of offset to first byte to write</td></tr>
    <tr><td>p2</td><td>Low part of offset to first byte to write</td></tr>
    <tr><td>p3</td><td>Number of bytes to write</td></tr>
    <tr><td>data</td><td>Data bytes to write</td></tr>
    </table>

    @pre @a req is not NULL (verified by caller).
    @post Radio state unchanged. MMGSDI callback expected.

    @param req [in]  params_ptr->data points to a RIL_SIM_IO structure.
 */
/*=========================================================================*/
static void qcril_mmgsdi_update_binary
(
 const qcril_request_params_type* const req
)
{
  qcril_instance_id_e_type    instance_id = req->instance_id;
  qcril_modem_id_e_type       modem_id    = req->modem_id;
  const RIL_SIM_IO_v6* const  request     = req->data;
  mmgsdi_return_enum_type     status      = MMGSDI_SUCCESS;
  mmgsdi_access_type          access;
  mmgsdi_data_type            data;
  mmgsdi_offset_type          offset      = (request->p1) << 8 | (request->p2);
  uint16                      fileid_low  = 0;

  /* Top bit of p1 shall be 0, per 3GPP TS 51.011 */
  QCRIL_ASSERT((request->p1 & 0x80) == 0);
  QCRIL_ASSERT(request->data != NULL);

  fileid_low = LOWORD(request->fileid);

  if (!parse_ef_path(request->fileid, request->path, &access, NULL))
  {
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  /* Append EF ID to the path information */
  if((access.file.path_type.path_len < MMGSDI_MAX_PATH_LEN) &&
     (access.access_method == MMGSDI_BY_PATH_ACCESS))
  {
    memcpy((void *)&access.file.path_type.path_buf[access.file.path_type.path_len],
           (void *)&fileid_low,
           sizeof(uint16));
    access.file.path_type.path_len++;
  }

  /* Construct data field */
  data.data_ptr = alloc_hexstring_to_bin(request->data, &data.data_len);
  QCRIL_ASSERT( data.data_ptr != NULL );

  if (data.data_len != request->p3)
  {
    if (data.data_ptr)
      free(data.data_ptr);
    QCRIL_LOG_ERROR( "%s: convert 0x%x and requested 0x%x data len mismatch \n",
                     __FUNCTION__, (unsigned int)data.data_len, request->p3);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  if (request->pin2 != NULL)
  {
    mmgsdi_data_type                            pin_data;
    qcril_mmgsdi_internal_sim_data_type        *internal_data_ptr = NULL;

    /* Queue a verify PIN2 command first */

    /* Populate the mmgsdi required members */
    pin_data.data_len = (mmgsdi_len_type)(strlen(request->pin2));
    pin_data.data_ptr = (uint8*)request->pin2;

    /* malloc and populate internal data for subsequent processing */
    internal_data_ptr = malloc(sizeof(qcril_mmgsdi_internal_sim_data_type));
    if(internal_data_ptr == NULL)
    {
      if (data.data_ptr)
        free(data.data_ptr);
      QCRIL_LOG_ERROR( "%s: failed allocate internal_data_ptr\n", __FUNCTION__);
      qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
      return;
    }

    memset(internal_data_ptr, 0x00, sizeof(qcril_mmgsdi_internal_sim_data_type));
    internal_data_ptr->cmd = QCRIL_MMGSDI_ORIG_SIM_IO_UPDATE_BINARY;
    memcpy(&internal_data_ptr->access, &access, sizeof(mmgsdi_access_type));
    internal_data_ptr->offset = offset;
    if(data.data_len > 0)
    {
      internal_data_ptr->data.data_ptr = malloc(data.data_len);
      if(internal_data_ptr->data.data_ptr == NULL)
      {
        free(internal_data_ptr);
        if (data.data_ptr)
          free(data.data_ptr);
        QCRIL_LOG_ERROR( "%s: failed alloc internal_data_ptr's data_ptr\n",
                         __FUNCTION__);
        qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
        return;
      }
      memcpy(internal_data_ptr->data.data_ptr, data.data_ptr, data.data_len);
      internal_data_ptr->data.data_len = data.data_len;
    }
    internal_data_ptr->token         = req->t;
    internal_data_ptr->do_pin_verify = TRUE;

    QCRIL_LOG_RPC2( modem_id, "mmgsdi_verify_pin PIN2()", "" );
    qcril_mmgsdi_print_byte_data(pin_data.data_len, pin_data.data_ptr);
    status = mmgsdi_verify_pin (qcril_mmgsdi.client_id,
                                MMGSDI_SLOT_1,
                                MMGSDI_PIN2,
                                pin_data,
                                qcril_mmgsdi_internal_verify_pin_command_callback,
                                (mmgsdi_client_data_type)internal_data_ptr);
    if (status != MMGSDI_SUCCESS)
    {
      if(internal_data_ptr->data.data_ptr)
        free(internal_data_ptr->data.data_ptr);
      free(internal_data_ptr);
      QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, status);
      qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
    }
    if (data.data_ptr)
      free(data.data_ptr);

    /* set current pin to PIN2 */
    qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN2);
    return;
  }

  /* proceed with write transparent */
  qcril_mmgsdi_common_write_transparent_ext( instance_id,
                                             modem_id,
                                             access,
                                             offset,
                                             data,
                                             (mmgsdi_client_data_type)req->t );
  if (data.data_ptr)
    free(data.data_ptr);
} /* qcril_mmgsdi_update_binary */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_write_record_ext

===========================================================================*/
/*!
    @brief utility function to invoke mmgsdi write record ext function call

    @param access [in]  the access type that the write record is to perform on.
    @param record [in]  the record number for the write
    @param data   [in]  the data to be written.
    @param token  [in]  the token for the command request list.
*/
/*=========================================================================*/
static void qcril_mmgsdi_common_write_record_ext(
  qcril_instance_id_e_type        instance_id,
  qcril_modem_id_e_type           modem_id,
  mmgsdi_access_type              access,
  mmgsdi_rec_num_type             record,
  mmgsdi_data_type                data,
  mmgsdi_client_data_type         token)
{
  char                    details[80];
  int                     i           = 0;
  mmgsdi_return_enum_type status      = MMGSDI_SUCCESS;

  memset(details, 0x00, sizeof(details));
  QCRIL_SNPRINTF( details, sizeof( details ), "record=%d", (int) record );
  QCRIL_LOG_RPC2(  modem_id, "mmgsdi_write_record_ext()", details );

  if(data.data_ptr && data.data_len > 0)
  {
    QCRIL_LOG_VERBOSE( "%s", "with data: [\n");
    for (i = 0; i < data.data_len; i++)
    {
      QCRIL_LOG_VERBOSE( "  0x%x,\n", data.data_ptr[i]);
    }
    QCRIL_LOG_VERBOSE( "%s", "]\n");
  }

  status = mmgsdi_write_record_ext(qcril_mmgsdi.client_id,
                                   MMGSDI_SLOT_1,
                                   access,
                                   MMGSDI_LINEAR_FIXED_FILE,
                                   record,
                                   data,
                                   qcril_mmgsdi_command_callback,
                                   token);

  if (status != MMGSDI_SUCCESS)
  {
    QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, status);
    qcril_mmgsdi_response( instance_id, (RIL_Token)token, status, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_common_write_record_ext */


/*=========================a================================================*/
/*! @brief Update a complete record in a linear or cyclic EF.

    This function is called from qcril_sim_io_request(), and performs update
    of a complete record. Per ETSI TS 102 221 and 3GPP TS 27.007, the values
    in the RIL_SIM_IO structure pointed to by @a req are interpreted as
    follows:

    <table>
    <tr><th>Parameter</th><th>Interpretation</th></tr>
    <tr><td>fileid</td><td>EF identifier. May be modified by path</td></tr>
    <tr><td>path</td><td>Path from MF to requested EF. Optional</td></tr>
    <tr><td>p1</td><td>Record number</td></tr>
    <tr><td>p2</td><td>Update mode</td></tr>
    <tr><td>p3</td><td>Bytes to update</td></tr>
    <tr><td>data</td><td>Data bytes to write</td></tr>
    </table>

    @pre @a req is not NULL (verified by caller).
    @post Radio state unchanged. MMGSDI callback expected.

    @note Only 'absolute/current' and 'previous' mode for updating records is
          supported by MMGSDILIB, and thus by qcril_mmgsdi_update_record().

    @param req [in]  params_ptr->data points to a RIL_SIM_IO structure.
 */
/*=========================================================================*/
static void qcril_mmgsdi_update_record
(
 const qcril_request_params_type* const req
)
{
  qcril_instance_id_e_type    instance_id  = req->instance_id;
  qcril_modem_id_e_type       modem_id     = req->modem_id;
  const RIL_SIM_IO_v6* const  request      = req->data;
  mmgsdi_return_enum_type     status       = MMGSDI_SUCCESS;
  mmgsdi_access_type          access;
  mmgsdi_rec_num_type         record       = request->p1;
  mmgsdi_data_type            data;
  uint16                      fileid_low   = 0;

  /* p2 can take values 2, 3, 4, per 3GPP TS 51.011, however MMGSDI
   * does not support next record (2) updates */
  QCRIL_ASSERT(request->data != NULL);

  if ((request->p2 != 4) && (request->p2 != 3))
  {
    QCRIL_LOG_ERROR( "%s: unsupported case P2 = %d\n", __FUNCTION__, request->p2);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_NOT_SUPPORTED, NULL, 0, TRUE, NULL );
    return;
  }

  fileid_low = LOWORD(request->fileid);

  if (!parse_ef_path(request->fileid, request->path, &access, NULL))
  {
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  /* Append EF ID to the path information */
  if((access.file.path_type.path_len < MMGSDI_MAX_PATH_LEN) &&
     (access.access_method == MMGSDI_BY_PATH_ACCESS))
  {
    memcpy((void *)&access.file.path_type.path_buf[access.file.path_type.path_len],
           (void *)&fileid_low,
           sizeof(uint16));
    access.file.path_type.path_len++;
  }

  /* Construct data field */
  data.data_ptr = alloc_hexstring_to_bin(request->data, &data.data_len);
  QCRIL_ASSERT( data.data_ptr != NULL );

  if (data.data_len != request->p3)
  {
    if (data.data_ptr)
      free(data.data_ptr);
    QCRIL_LOG_ERROR( "%s: convert 0x%x and requested 0x%x data len mismatch \n",
                     __FUNCTION__, (unsigned int)data.data_len, request->p3);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  if (request->pin2 != NULL)
  {
    mmgsdi_data_type                            pin_data;
    qcril_mmgsdi_internal_sim_data_type        *internal_data_ptr = NULL;

    /* Queue a verify PIN2 command first */

    /* Populate the mmgsdi required members */
    pin_data.data_len = (mmgsdi_len_type)(strlen(request->pin2));
    pin_data.data_ptr = (uint8*)request->pin2;

    /* malloc and populate internal data for subsequent processing */
    internal_data_ptr = malloc(sizeof(qcril_mmgsdi_internal_sim_data_type));
    if(internal_data_ptr == NULL)
    {
      if (data.data_ptr)
        free(data.data_ptr);
      QCRIL_LOG_ERROR( "%s: failed allocate internal_data_ptr\n", __FUNCTION__);
      qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
      return;
    }

    memset(internal_data_ptr, 0x00, sizeof(qcril_mmgsdi_internal_sim_data_type));
    internal_data_ptr->cmd = QCRIL_MMGSDI_ORIG_SIM_IO_UPDATE_RECORD;
    memcpy(&internal_data_ptr->access, &access, sizeof(mmgsdi_access_type));
    internal_data_ptr->record = record;
    if(data.data_len > 0)
    {
      internal_data_ptr->data.data_ptr = malloc(data.data_len);
      if(internal_data_ptr->data.data_ptr == NULL)
      {
        free(internal_data_ptr);
        if (data.data_ptr)
          free(data.data_ptr);
        QCRIL_LOG_ERROR( "%s: failed alloc internal_data_ptr's data_ptr\n",
                         __FUNCTION__);
        qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
        return;
      }
      memcpy(internal_data_ptr->data.data_ptr, data.data_ptr, data.data_len);
      internal_data_ptr->data.data_len = data.data_len;
    }
    internal_data_ptr->token         = req->t;
    internal_data_ptr->do_pin_verify = TRUE;

    QCRIL_LOG_RPC2( modem_id, "mmgsdi_verify_pin PIN2()", "" );
    qcril_mmgsdi_print_byte_data(pin_data.data_len, pin_data.data_ptr);
    status = mmgsdi_verify_pin (qcril_mmgsdi.client_id,
                                MMGSDI_SLOT_1,
                                MMGSDI_PIN2,
                                pin_data,
                                qcril_mmgsdi_internal_verify_pin_command_callback,
                                (mmgsdi_client_data_type)internal_data_ptr);
    if (status != MMGSDI_SUCCESS)
    {
      if(internal_data_ptr->data.data_ptr)
        free(internal_data_ptr->data.data_ptr);
      free(internal_data_ptr);
      QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, status);
      qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
    }

    if (data.data_ptr)
      free(data.data_ptr);

    /* set current pin to PIN2 */
    qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN2);
    return;
  }

  /* proceed with write record */
  qcril_mmgsdi_common_write_record_ext( instance_id,
                                        modem_id,
                                        access,
                                        record,
                                        data,
                                        (mmgsdi_client_data_type)req->t );
  if (data.data_ptr)
    free(data.data_ptr);
} /* qcril_mmgsdi_update_record */


/*=========================a================================================*/
/*! @brief Return information about the current directory.

    This function is called from qcril_sim_io_request(), and retrieves status
    information. Per ETSI TS 102 221 and 3GPP TS 27.007, the values
    in the RIL_SIM_IO structure pointed to by @a req are interpreted as
    follows:

    <table>
    <tr><th>Parameter</th><th>Interpretation</th></tr>
    <tr><td>fileid</td><td>Ignored</td></tr>
    <tr><td>path</td><td>Ignored</td></tr>
    <tr><td>p1</td><td>No indication (0)</td></tr>
    <tr><td>p2</td><td>No indication (0)</td></tr>
    <tr><td>p3</td><td>0</td></tr>
    <tr><td>data</td><td>Data bytes to write</td></tr>
    </table>

    @pre @a req and @a ret are not NULL (verified by caller).
    @post Radio state unchanged. MMGSDI callback expected.

    @param req [in]  params_ptr->data points to a RIL_SIM_IO structure.
 */
/*=========================================================================*/
static void qcril_mmgsdi_get_status
(
 const qcril_request_params_type* const req
)
{
  qcril_instance_id_e_type  instance_id = req->instance_id;
  qcril_modem_id_e_type     modem_id    = req->modem_id;
  mmgsdi_return_enum_type   status;

  QCRIL_LOG_RPC2( modem_id, "mmgsdi_send_card_status()", "");

  QCRIL_MMGSDI_RETURN_ERR_RSP_IF_INVALID_CLIENT( instance_id,
                                                 qcril_mmgsdi.client_id,
                                                 req->t, TRUE );

  status = mmgsdi_send_card_status(qcril_mmgsdi.client_id,
                                   MMGSDI_SLOT_1,
                                   MMGSDI_STATUS_APP_NONE,
                                   MMGSDI_STATUS_DATA_SELECT,
                                   qcril_mmgsdi_command_callback,
                                   (mmgsdi_client_data_type) req->t);

  if (status != MMGSDI_SUCCESS)
  {
    QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, status);
    qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
  }

} /* qcril_mmgsdi_get_status */


/*==========================================================================*/
/*! @brief Select file ID.

    This function is called from qcril_sim_io_request(), and performs select
    of the file ID/path provided. Per ETSI TS 102 221 and 3GPP TS 27.007,
    the values in the RIL_SIM_IO structure pointed to by @a req are
    interpreted as follows:

    <table>
    <tr><th>Parameter</th><th>Interpretation</th></tr>
    <tr><td>fileid</td><td>EF identifier. May be modified by path</td></tr>
    <tr><td>path</td><td>Path from MF to requested EF. Optional</td></tr>
    <tr><td>p1</td><td>Selection method. 0 for ICC.</td></tr>
    <tr><td>p2</td><td>Selection criteria.  0 for ICC.</td></tr>
    <tr><td>p3</td><td>Length of the input parameters, i.e., fileid or path len</td></tr>
    </table>

    @pre @a req is not NULL (verified by caller).
    @post Radio state unchanged. MMGSDI callback expected.

    @param req [in]  params_ptr->data points to a RIL_SIM_IO structure.
 */
/*=========================================================================*/
static void qcril_mmgsdi_get_response
(
 const qcril_request_params_type* const req_ptr
)
{
  qcril_instance_id_e_type         instance_id     = req_ptr->instance_id;
  qcril_modem_id_e_type            modem_id        = req_ptr->modem_id;
  const RIL_SIM_IO_v6*             request_ptr;
  gsdi_returns_T                   gsdi_status     = GSDI_SUCCESS;
  mmgsdi_return_enum_type          mmgsdi_status   = MMGSDI_SUCCESS;
  mmgsdi_access_type               access;
  uim_items_enum_type              uim_file        = UIM_NO_SUCH_ITEM;
  uim_file_type                    file_type       = UIM_EF;
  uint8                            ret_data_len    = 0;
  uint8                            ret_data        = 0;
  uint16                           fileid_low      = 0;
  boolean                          use_path        = TRUE;
  int                              i               = 0;
  char details[80];

  memset((void *)details, 0x00, sizeof(details));
  memset((void *)&access, 0x00, sizeof(mmgsdi_access_type));

  QCRIL_ASSERT( req_ptr != NULL );

  request_ptr = req_ptr->data;

  if (request_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s: null request_ptr \n", __FUNCTION__);
    qcril_mmgsdi_response( instance_id, req_ptr->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  /* We will not use path for USIM and SIM. However for RUIM we will
    use path */
  if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_TRUE)
  {
    /* Try to find if there is a SIM app */
    /* Todo - find based on slot and AID */
    for(i=0; i<qcril_mmgsdi.curr_card_status.num_applications; i++)
    {
      if(qcril_mmgsdi.curr_card_status.applications[i].app_type == RIL_APPTYPE_SIM)
      {
        use_path = FALSE;
      }
    }
  }
  else
  {
    use_path = FALSE;
  }

  fileid_low = LOWORD(request_ptr->fileid);

  /* Check if the file ID is EF or not */
  if ((request_ptr->path == NULL) || (use_path == FALSE))
  {
    switch(request_ptr->fileid & 0xFF00)
    {
    case 0x3F00:
      file_type = UIM_MF;
      uim_file  = UIM_EF_BY_PATH;
      break;
    case 0x7F00:
      if (request_ptr->fileid == 0x7FFF)
      {
        file_type = UIM_ADF;
      }
      else
      {
        file_type = UIM_DF;
      }
      uim_file  = UIM_EF_BY_PATH;
      break;
    case 0x5F00:
      file_type = UIM_DF;
      uim_file  = UIM_EF_BY_PATH;
      break;
    default:
      /* EF */
      break;
    }
    if (!parse_ef_path(request_ptr->fileid, request_ptr->path, &access, &uim_file))
    {
      qcril_mmgsdi_response( instance_id, req_ptr->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
      return;
    }
  }
  else if((request_ptr->path != NULL) && (use_path == TRUE))
  {
    uim_file  = UIM_EF_BY_PATH;
    /* Assume EF */
    /* Path information provided - a sequence of 16 bit hex ASCII values */
    if (!qcril_mmgsdi_convert_path(request_ptr->fileid, request_ptr->path, &access))
    {
      qcril_mmgsdi_response( instance_id, req_ptr->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
      return;
    }
  }

  /* Append EF ID to the path information */
  if((access.file.path_type.path_len < MMGSDI_MAX_PATH_LEN) &&
     (access.access_method == MMGSDI_BY_PATH_ACCESS))
  {
    memcpy((void *)&access.file.path_type.path_buf[access.file.path_type.path_len],
           (void *)&fileid_low,
           sizeof(uint16));
    access.file.path_type.path_len++;
  }

  if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_TRUE)
  {
    if ((uim_file != UIM_EF_BY_PATH) && (uim_file != UIM_NO_SUCH_ITEM))
    {
      memset(&access, 0x00, sizeof(mmgsdi_access_type));
    }

    QCRIL_SNPRINTF( details, sizeof( details ), "uim_file 0x%x, file_type 0x%x, file_id 0x%x",
                    uim_file, file_type, request_ptr->fileid );

    QCRIL_LOG_RPC2( modem_id, "gsdi_file_select() ICC card", details);
    gsdi_status = gsdi_file_select(uim_file,
                                   file_type,
                                   access.file.path_type.path_buf,
                                   (uint8)access.file.path_type.path_len,
                                   &ret_data_len,
                                   &ret_data,
                                   (uint32)req_ptr->t,
                                   qcril_mmgsdi_gsdi_command_callback);

    if (GSDI_SUCCESS != gsdi_status)
    {
      mmgsdi_status = qcril_mmgsdi_convert_from_gsdi_status(gsdi_status);
    }
  }
  else
  {
    QCRIL_LOG_RPC2( modem_id, "UICC card: mmgsdi_get_file_attr()", "");
    mmgsdi_status = mmgsdi_get_file_attr(qcril_mmgsdi.client_id,
                                         MMGSDI_SLOT_1,
                                         access,
                                         qcril_mmgsdi_command_callback,
                                         (mmgsdi_client_data_type) req_ptr->t);
  }
  if (mmgsdi_status != MMGSDI_SUCCESS)
  {
    QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, mmgsdi_status);
    qcril_mmgsdi_response( instance_id, req_ptr->t, mmgsdi_status, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_get_response */


/*=========================================================================*/
/*! @brief Perform common processing for SIM IO responses.

    The common processing components performed here are:
     - Allocate space for response
     - Determine values for SW1 and SW2 from mmgsdi_to_sw1sw2.

    In some cases, indicated in mmgsdi_to_sw1sw2 by setting field
    sw2_present to TRUE, the value of SW2 is known simply based on lookup
    given the response from MMGSDI. If this is the case, the value is
    filled, and the address given in @a sw2_set is set TRUE, otherwise it
    is FALSE, and the caller needs to fill in this value appropriately.

    Provided that the returned value is not NULL, SW1 is giaranteed to
    be set.

    @param req     [in] Request object pointer.
    @param sw2_set [out] Points to 'TRUE' if SW2 filed has been filled.

    @return Pointer to a RIL_SIM_IO_Response object in which at least the
            SW1 field is valid. Returned pointer may be NULL in the case of
            out of memory.
 */
/*=========================================================================*/
static RIL_SIM_IO_Response* qcril_mmgsdi_common_simio_common_cnf
(
  mmgsdi_return_enum_type                mmgsdi_status,
  int                                   *sw2_set_ptr
)
{
  Sw1Sw2*                             status_ptr = NULL;
  RIL_SIM_IO_Response*                rsp_ptr    = NULL;

  if (sw2_set_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s: Null input sw2_set_ptr\n", __FUNCTION__);
    return rsp_ptr;
  }

  /* Allocate space for response */
  rsp_ptr = malloc(sizeof(RIL_SIM_IO_Response));
  if (rsp_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s: Out of memory\n", __FUNCTION__);
    return rsp_ptr;
  }
  memset(rsp_ptr, 0, sizeof(RIL_SIM_IO_Response));

  /* Determine values for sw1 and sw2 */
  status_ptr = (Sw1Sw2*) qcril_map_find(status_map, (int) mmgsdi_status);

  if (status_ptr == QCRIL_MAP_NOT_PRESENT)
  {
    QCRIL_LOG_ERROR( "%s: Unknown status: %d\n", __FUNCTION__, mmgsdi_status );
    *sw2_set_ptr = FALSE;
  }
  else
  {
    rsp_ptr->sw1 = status_ptr->sw1;
    /* Where sw2 is not defined, we set sw2_set FALSE */
    if (status_ptr->sw2_present)
    {
      rsp_ptr->sw2 = status_ptr->sw2;
      *sw2_set_ptr = TRUE;
    }
    else
    {
      *sw2_set_ptr = FALSE;
    }
  }
  return rsp_ptr;
}


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_convert_mmgsdi_sec_attr_to_icc_select_rsp_value

===========================================================================*/
/*!
    @brief
    Convert the MMGSDI structure for security attribute retrieved from
    get file attribute into the ICC defined values in the select response's
    access condition.

    @return
    None.
*/
/*=========================================================================*/
static boolean qcril_mmgsdi_common_convert_mmgsdi_sec_attr_to_icc_select_rsp_value
(
  const mmgsdi_file_security_type *sec_ptr,
  uint8                           *icc_value_ptr
)
{
  if ((sec_ptr == NULL) || (icc_value_ptr == NULL))
  {
    QCRIL_LOG_ERROR( "%s: null sec_ptr 0x%x or icc_value_ptr 0x%x \n",
      __FUNCTION__, (unsigned int)sec_ptr, (unsigned int)icc_value_ptr);
    return FALSE;
  }

  switch(sec_ptr->protection_method)
  {
  case MMGSDI_ALWAYS_ALLOWED:
    *icc_value_ptr = 0x00;
    break;
  case MMGSDI_NEVER_ALLOWED:
    *icc_value_ptr = 0x0F;
    break;
  case MMGSDI_AND_ALLOWED:
  case MMGSDI_OR_ALLOWED:
  case MMGSDI_SINGLE_ALLOWED:
    if (sec_ptr->protection_pin_ptr == NULL)
    {
      QCRIL_LOG_ERROR( "%s: null sec_ptr->protection_pin_ptr\n", __FUNCTION__);
      return FALSE;
    }
    switch(*(sec_ptr->protection_pin_ptr))
    {
    case MMGSDI_PIN1:
    case MMGSDI_UNIVERSAL_PIN: /* simulating UPin as PIN1 */
      if (*(sec_ptr->protection_pin_ptr) == MMGSDI_UNIVERSAL_PIN)
      {
        QCRIL_LOG_INFO( "%s", "Treat MMGSDI_UNIVERSAL_PIN as PIN1\n");
      }
      *icc_value_ptr = 0x01;
      break;
    case MMGSDI_PIN2:
      *icc_value_ptr = 0x02;
      break;
    case MMGSDI_ADM1:
      *icc_value_ptr = 0x04;
      break;
    case MMGSDI_ADM2:
      *icc_value_ptr = 0x05;
      break;
    case MMGSDI_ADM3:
      *icc_value_ptr = 0x06;
      break;
    case MMGSDI_ADM4:
      *icc_value_ptr = 0x07;
      break;
    case MMGSDI_ADM5:
      *icc_value_ptr = 0x08;
      break;
    case MMGSDI_ADM6:
      *icc_value_ptr = 0x09;
      break;
    case MMGSDI_ADM7:
      *icc_value_ptr = 0x0A;
      break;
    case MMGSDI_ADM8:
      *icc_value_ptr = 0x0B;
      break;
    case MMGSDI_ADM9:
      *icc_value_ptr = 0x0C;
      break;
    case MMGSDI_ADM10:
      *icc_value_ptr = 0x0D;
      break;
    case MMGSDI_ADM11:
      *icc_value_ptr = 0x0E;
      break;
    case MMGSDI_ADM12:
    case MMGSDI_ADM13:
    case MMGSDI_ADM14:
    case MMGSDI_ADM15:
      /* Simulating NEVER */
        QCRIL_LOG_INFO( "Treat MMGSDI_ADM12+ 0x%x as NEVER\n", *(sec_ptr->protection_pin_ptr));
      *icc_value_ptr = 0x0F;
      break;
    case MMGSDI_MAX_PIN_ENUM:
    default:
      QCRIL_LOG_ERROR( "Unhandled pin enum 0x%x \n", *(sec_ptr->protection_pin_ptr));
      return FALSE;
    }
    break;
  default:
    QCRIL_LOG_ERROR( "Unhandled protection_method 0x%x \n", sec_ptr->protection_method);
    return FALSE;
  }
  QCRIL_LOG_INFO( "Converted icc access cond select rsp value 0x%x\n", *icc_value_ptr);
  return TRUE;
} /* qcril_mmgsdi_common_convert_mmgsdi_sec_attr_to_icc_select_rsp_value*/


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_reconstruct_icc_select_rsp_cnf

===========================================================================*/
/*!
    @brief
    Reconstruct ICC select rsp for get file attribute.  Used for UICC
    card because Android application does not understand UICC select response

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_common_reconstruct_icc_select_rsp_cnf
(
  qcril_instance_id_e_type          instance_id,
  const mmgsdi_get_file_attr_cnf_type* cnf_ptr
)
{
  uint32                            ril_token;
  RIL_SIM_IO_Response*              rsp_ptr            = NULL;
  int                               sw2_set            = FALSE;
  uint8                             temp_icc_rsp_buf[15];
  mmgsdi_file_security_access_type *security_ptr       = NULL;
  uint8                             access_cond        = 0;
  boolean                           error_encountered  = FALSE;

  /*-----------------------------------------------------------------------*/
  memset(temp_icc_rsp_buf, 0x00, sizeof(temp_icc_rsp_buf));

  if (cnf_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s: null cnf_ptr\n", __FUNCTION__);
    return;
  }

  QCRIL_LOG_DEBUG( "%s: status = 0x%x\n", __FUNCTION__, cnf_ptr->response_header.mmgsdi_status);

  ril_token = cnf_ptr->response_header.client_data;

  /* Perform common processing */
  rsp_ptr = qcril_mmgsdi_common_simio_common_cnf(
    cnf_ptr->response_header.mmgsdi_status,
    &sw2_set);

  if (rsp_ptr != NULL)
  {
    if (cnf_ptr->response_header.mmgsdi_status == MMGSDI_SUCCESS)
    {

    /* per 51.011
       0 - 1  RFU
       2 - 3  File size
       4 - 5  File ID
       6      Type of file
       7      For Cyclic file: b7 = 1 indicates increased allowed
       8 - 10 Access conditions
       11     File Status
       12     Length of following data (byte 14 to end)
       13     Structure of EF
       14     Length of a record
       > 15   RFU */
    temp_icc_rsp_buf[2] = (uint8)(cnf_ptr->file_attrib.file_size >> 8);
    temp_icc_rsp_buf[3] = (uint8)(cnf_ptr->file_attrib.file_size & 0xFF);
    temp_icc_rsp_buf[4] = cnf_ptr->file_attrib.file_id[0];
    temp_icc_rsp_buf[5] = cnf_ptr->file_attrib.file_id[1];
    temp_icc_rsp_buf[6] = 0x04; /* Type of file: EF */
    switch (cnf_ptr->file_attrib.file_type)
    {
    case MMGSDI_LINEAR_FIXED_FILE:
      temp_icc_rsp_buf[13] = 0x01;
      temp_icc_rsp_buf[14] =
        (uint8)(cnf_ptr->file_attrib.file_info.linear_fixed_file.rec_len);
      security_ptr =
        (mmgsdi_file_security_access_type*)&cnf_ptr->file_attrib.file_info.linear_fixed_file.file_security;
      break;
    case MMGSDI_CYCLIC_FILE:
      temp_icc_rsp_buf[13] = 0x03;
      temp_icc_rsp_buf[7] =
        cnf_ptr->file_attrib.file_info.cyclic_file.increase_allowed;
      temp_icc_rsp_buf[14] =
        (uint8)(cnf_ptr->file_attrib.file_info.cyclic_file.rec_len);
      security_ptr =
        (mmgsdi_file_security_access_type*)&cnf_ptr->file_attrib.file_info.cyclic_file.file_security;
      break;
    case MMGSDI_TRANSPARENT_FILE:
      temp_icc_rsp_buf[13] = 0x00;
      security_ptr =
        (mmgsdi_file_security_access_type*)&cnf_ptr->file_attrib.file_info.transparent_file.file_security;
      break;
    default:
      /* non EF! */
      QCRIL_LOG_ERROR( "Not supporting Non EF file type 0x%x, build error response \n", cnf_ptr->file_attrib.file_type);
      error_encountered = TRUE;
      break;
    }

    if (!error_encountered)
    {
      if (security_ptr == NULL)
      {
        QCRIL_LOG_ERROR( "%s", "security_ptr is Null, build error response \n");
        error_encountered = TRUE;
      }
      else
      {
        /* Read: byte 8 bit 5 - 8 */
        if(qcril_mmgsdi_common_convert_mmgsdi_sec_attr_to_icc_select_rsp_value(
             &security_ptr->read,
             &access_cond) != TRUE)
        {
          QCRIL_LOG_ERROR( "%s", "Cannot convert access condition for Read, build error response \n");
          error_encountered = TRUE;
        }
        else
        {
          temp_icc_rsp_buf[8] = access_cond << 4;
        }

        if (!error_encountered)
        {
          /* Write: byte 8 bit 1 - 4 */
          if(qcril_mmgsdi_common_convert_mmgsdi_sec_attr_to_icc_select_rsp_value(
               &security_ptr->write,
               &access_cond) != TRUE)
          {
            QCRIL_LOG_ERROR( "%s", "Cannot convert access condition for Write, build error response \n");
            error_encountered = TRUE;
          }
          else
          {
            temp_icc_rsp_buf[8] |= access_cond;
          }
        }

        if (!error_encountered)
        {
          /* Increase: byte 9 bit 5 - 8 */
          if(qcril_mmgsdi_common_convert_mmgsdi_sec_attr_to_icc_select_rsp_value(
               &security_ptr->increase,
               &access_cond) != TRUE)
          {
            QCRIL_LOG_ERROR( "%s", "Cannot convert access condition for Increase, build error response \n");
            error_encountered = TRUE;
          }
          else
          {
            temp_icc_rsp_buf[9] = access_cond << 4;
          }
        }

        if (!error_encountered)
        {
          /* Rehabilitate: byte 10 bit 5 - 8 */
          if(qcril_mmgsdi_common_convert_mmgsdi_sec_attr_to_icc_select_rsp_value(
               &security_ptr->rehabilitate_activate,
               &access_cond) != TRUE)
          {
            QCRIL_LOG_ERROR( "%s", "Cannot convert access condition for Rehabilitate, build error response\n");
            error_encountered = TRUE;
          }
          else
          {
            temp_icc_rsp_buf[10] = access_cond << 4;
          }
        }

        if (!error_encountered)
        {
          /* Invalidate: byte 10 bit 1 - 4 */
          if(qcril_mmgsdi_common_convert_mmgsdi_sec_attr_to_icc_select_rsp_value(
               &security_ptr->invalidate_deactivate,
               &access_cond) != TRUE)
          {
            QCRIL_LOG_ERROR( "%s", "Cannot convert access condition for Invalidate, build error response \n");
            error_encountered = TRUE;
          }
          else
          {
            temp_icc_rsp_buf[10] |= access_cond;
          }
        } /* security_ptr != NULL */
      } /* no error_encountered */

      if (!error_encountered)
      {
        temp_icc_rsp_buf[11] = 0x05; /* hardcoded to b1 = 1 -> not invalidated,
                                         b3 = 1 -> readable and updatable
                                         when invalidated */
        temp_icc_rsp_buf[12] = 2;  /* Hardcoded to 2 because we only simulate
                                      byte 13 and 14 */


        /* Transform returned data into ASCII hex */
        rsp_ptr->simResponse = bin_to_hexstring(temp_icc_rsp_buf,
                                                15);

        QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d data=%s\n",
                         rsp_ptr->sw1, rsp_ptr->sw2,
                         rsp_ptr->simResponse != NULL ? rsp_ptr->simResponse : "NULL");

        /* Generate response */
        qcril_mmgsdi_response( instance_id, (RIL_Token) ril_token,
                               cnf_ptr->response_header.mmgsdi_status,
                               rsp_ptr,
                               sizeof(RIL_SIM_IO_Response),
                               TRUE,
                               NULL );
      }
    }  /* !error_encountered */

    if (error_encountered)
    {
      QCRIL_LOG_ERROR( "%s: failed to generate RIL_SIM_IO_Response\n", __FUNCTION__);
      qcril_mmgsdi_response( instance_id, (RIL_Token)ril_token, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    }
    }
    else
    {
      /* When mmgsdi status != MMGSDI SUCCESS, there is no need to build
         any additional data other than the sw */
      QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d data=%s\n",
                       rsp_ptr->sw1, rsp_ptr->sw2,
                       rsp_ptr->simResponse != NULL ? rsp_ptr->simResponse : "NULL");

      /* Generate response */
      qcril_mmgsdi_response( instance_id, (RIL_Token) ril_token,
                             cnf_ptr->response_header.mmgsdi_status,
                             rsp_ptr,
                             sizeof(RIL_SIM_IO_Response),
                             TRUE,
                             NULL );
    }
    /* Free rsp after call completes */
    free(rsp_ptr->simResponse);
    free(rsp_ptr);
  }
  else
  {
    /* Error case: rsp allocation failed */
    QCRIL_LOG_ERROR( "%s: failed to generate RIL_SIM_IO_Response\n", __FUNCTION__);
    qcril_mmgsdi_response( instance_id, (RIL_Token)ril_token, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_common_reconstruct_icc_select_rsp_cnf() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_process_cnf

===========================================================================*/
/*!
    @brief
    General callback function for MMGSDI general operations

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_common_process_cnf
(
  qcril_instance_id_e_type    instance_id,
  mmgsdi_return_enum_type     status,
  mmgsdi_cnf_enum_type        cnf,
  const mmgsdi_cnf_type       *cnf_ptr
)
{
  QCRIL_LOG_INFO( "qcril_mmgsdi_common_process_cnf: cnf = 0x%x, status = 0x%x\n", cnf, status);

  switch(cnf)
  {
  case MMGSDI_GET_FILE_ATTR_CNF:
    if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_TRUE)
    {
      /* ignore */
      QCRIL_LOG_INFO( "%s", "Received GET FILE ATTR CNF for ICC card, not handled\n");
    }
    else
    {
      /* Reconstruct "ICC Select response */
      if (cnf_ptr != NULL)
      {
        qcril_mmgsdi_common_reconstruct_icc_select_rsp_cnf( instance_id, &cnf_ptr->get_file_attr_cnf );
      }
    }
    break;
  default:
    break;
  }
} /* qcril_mmgsdi_common_process_cnf() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_process_fdn_status_cnf

===========================================================================*/
/*!
    @brief
    Handle FDN enabling/disabling confirmation

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_common_process_fdn_status_cnf
(
  qcril_instance_id_e_type    instance_id,
  gsdi_returns_T              status,
  uint32                      client_ref
)
{
  mmgsdi_return_enum_type               mmgsdi_status     = MMGSDI_SUCCESS;
  qcril_mmgsdi_internal_sim_data_type * internal_data_ptr = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "qcril_mmgsdi_common_process_fdn_status_cnf: status = 0x%x\n",
                     status);

  mmgsdi_status = qcril_mmgsdi_convert_from_gsdi_status(status);

  internal_data_ptr = (qcril_mmgsdi_internal_sim_data_type *) client_ref;

  if (internal_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s, Null internal_data_ptr, can not send response \n",
                      __FUNCTION__);
    return;
  }

  if(internal_data_ptr->do_pin_verify)
  {
    QCRIL_LOG_INFO( "%s - num_retries = 0x%x\n",__FUNCTION__,
                    internal_data_ptr->pin_info.num_retries);
    qcril_mmgsdi_response(
      instance_id,
      (RIL_Token)internal_data_ptr->token,
      mmgsdi_status,
      &internal_data_ptr->pin_info.num_retries,
      sizeof(int32),
      TRUE,
      NULL);
  }
  else
  {
    qcril_mmgsdi_response( instance_id, (RIL_Token)internal_data_ptr->token,
                           mmgsdi_status, NULL, 0, TRUE, NULL );
  }

  if(internal_data_ptr->data.data_ptr)
    free(internal_data_ptr->data.data_ptr);
  free(internal_data_ptr);

  switch (mmgsdi_status)
  {
  case MMGSDI_SUCCESS:
    QCRIL_LOG_DEBUG( "%s", "status: MMGDSI_SUCCESS\n");
    qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN1);
    break;
  case MMGSDI_ACCESS_DENIED:
    QCRIL_LOG_DEBUG( "%s", "status: MMGSDI_ACCESS_DENIED\n");
    qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN2);
    break;
  default:
    QCRIL_LOG_DEBUG( "mmgsdi status: 0x%x -> No update to curr pin ptr\n", mmgsdi_status);
    break;
  }
} /* qcril_mmgsdi_common_process_fdn_status_cnf() */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_get_sim_status

===========================================================================*/
/*!
    @brief Handler for RIL_REQUEST_SIM_STATUS

    The function implementation simply returns the current state of the
    MMGSDI object current_card_state field. This is updated from a number
    of different event handlers (listed below).

    The function relies on the contents of qcril_mmgsdi.curr_card_state
    being initialized to QCRIL_SIM_STATE_NOT_READY before the RIL starts operations.
    This is performed in qcril_mmgsdi_init().

    @note While the function implementation is very straightforward, the
          sequencing operations required to support correct operation are
          more complex. Changes to any of the functions referenced below
          should be verified against this implementation.

    @see qcril_mmgsdi_process_pin_evts()
    @see qcril_mmgsdi_process_card_init_completed_evt()
    @see qcril_mmgsdi_process_event_callback()
    @see qcril_mmgsdi_process_perso_event_callback()
    @see qcril_mmgsdi_init()

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_get_sim_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type      instance_id;
  qcril_modem_id_e_type         modem_id = QCRIL_DEFAULT_MODEM_ID;
  char                         *card_state_name;
  qcril_reqlist_public_type     reqlist_entry;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_request_get_sim_status \n");

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id,
                               modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                               QCRIL_EVT_NONE, NULL, 
                               &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  switch ( qcril_mmgsdi.curr_card_status.card_state )
  {
    case RIL_CARDSTATE_ABSENT:
      card_state_name = "Card Absent";
      break;

    case RIL_CARDSTATE_PRESENT:
      card_state_name = "Card Present";
      break;

    case RIL_CARDSTATE_ERROR:
      card_state_name = "Card Error";
      break;

    default:
      card_state_name = "Unknown";
      break;
  }

  /* Create a meaningful short string expressing SIM status and change to
      qcril_mmgsdi_response call. */
  qcril_mmgsdi_response( instance_id,
                         params_ptr->t,
                         MMGSDI_SUCCESS,
                         &qcril_mmgsdi.curr_card_status,
                         sizeof(RIL_CardStatus_v6),
                         TRUE,
                         card_state_name );

} /* qcril_mmgsdi_request_get_sim_status() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_get_imsi

===========================================================================*/
/*!
    @brief Handler for RIL_REQUEST_GET_IMSI.

    IMSI is stored as a transparent file on the (U)SIM

    @note Currently support only the GSM application. This is in line with
          available support in Android.

    @param req [in]  Request object
    @param ret [out] Updated radio state.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_get_imsi
(
  const qcril_request_params_type *const req,
  qcril_request_return_type       *const ret /*!< Output parameter */
)
{
  qcril_instance_id_e_type    instance_id;
  qcril_modem_id_e_type       modem_id;
  mmgsdi_return_enum_type     status    = MMGSDI_ERROR;
  mmgsdi_file_enum_type       imsi_file = MMGSDI_GSM_IMSI;
  qcril_reqlist_public_type   reqlist_entry; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( req != NULL );
  instance_id = req->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = req->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_request_get_imsi \n");

  QCRIL_MMGSDI_RETURN_ERR_RSP_IF_INVALID_CLIENT( instance_id, qcril_mmgsdi.client_id, req->t, FALSE );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( req->t, req->event_id, modem_id,
                               QCRIL_REQ_AWAITING_CALLBACK,
                               QCRIL_EVT_NONE, NULL, 
                               &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_NOT_INIT)
  {
    QCRIL_LOG_DEBUG("Unknown card mode for Get IMSI request", "" );
    qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
    return;
  }

  /* Perform IMSI read */
  if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_FALSE)
  {
    QCRIL_LOG_RPC2A( modem_id, "mmgsdi_read_transparent()", "MMGSDI_USIM_IMSI" );
    imsi_file = MMGSDI_USIM_IMSI;
  }
  else
  {
    QCRIL_LOG_RPC2A( modem_id, "mmgsdi_read_transparent()", "MMGSDI_GSM_IMSI" );
  }

  status = mmgsdi_read_transparent(qcril_mmgsdi.client_id,
                                   MMGSDI_SLOT_1,
                                   imsi_file,
                                   0,               /* Offset */
                                   0,               /* Length - get all */
                                   qcril_mmgsdi_imsi_command_callback,
                                   (mmgsdi_client_data_type) req->t);

  if (status != MMGSDI_SUCCESS)
  {
    QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, status);
    qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
  }

} /* qcril_mmgsdi_request_get_imsi() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_get_imsi_cnf

===========================================================================*/
/*!
    @brief Callback processing for RIL_REQUEST_GET_IMSI

    IMSI is stored as a transparent file on the (U)SIM. This information
    is contained within the data field returned in callback from MMGSDI.

    The EF structure is defined in 31.102 section 4.2.2:

    <TABLE>
      <TR>
        <TH> Byte(s) </TH>
        <TH> Description </TH>
      </TR>
      <TR>
        <TH> 1 </TH>
        <TD> IMSI length (bytes) </TD>
      </TR>
      <TR>
        <TH> 2 </TH>
        <TD> Bits 0-3: Unused  <BR> Bits 4-7: Digit 1   </TD>
      </TR>
      <TR>
        <TH> 3 to 9  </TH>
        <TD> Bits 0-3: Digit n <BR> Bits 4-7: Digit n+1 </TD>
      </TR>
    </TABLE>

    @param req [in]  Request object containing raw data returned by MMGSDI.
    @param ret [out] Updated radio state.
*/
/*=========================================================================*/
void qcril_mmgsdi_common_get_imsi_cnf
(
 const qcril_request_params_type* const req,
       qcril_request_return_type* const ret
)
{
  qcril_instance_id_e_type instance_id;
  qcril_mmgsdi_command_callback_params_type* cnf_raw;
  mmgsdi_read_cnf_type*                      cnf;
  char*                                      imsi   = NULL;
  int                                        imsi_length, src, dst;
  mmgsdi_return_enum_type                    status = MMGSDI_SUCCESS;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( req != NULL );
  instance_id = req->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_VERBOSE( "%s", "qcril_mmgsdi_common_get_imsi_cnf()");

  cnf_raw = (qcril_mmgsdi_command_callback_params_type*) req->data;
  if (cnf_raw == NULL)
  {
    status = MMGSDI_ERROR;
    QCRIL_LOG_ERROR( "%s: NULL cnf_raw\n", __FUNCTION__);
    goto Error;
  }

  cnf = (mmgsdi_read_cnf_type*) cnf_raw->cnf_ptr;
  if (cnf == NULL)
  {
    status = MMGSDI_ERROR;
    QCRIL_LOG_ERROR( "%s: NULL cnf\n", __FUNCTION__);
    goto Error;
  }

  /* Check that file was read OK */
  if (cnf->response_header.mmgsdi_status != MMGSDI_SUCCESS)
  {
    status = cnf->response_header.mmgsdi_status;
    QCRIL_LOG_ERROR( "%s: failed to read IMSI\n", __FUNCTION__);
    goto Error;
  }

  imsi_length = cnf->read_data.data_ptr[0];
  if (imsi_length >= cnf->read_data.data_len)
  {
    status = MMGSDI_ERROR;
    QCRIL_LOG_ERROR( "%s: invalid data length %d\n", __FUNCTION__, imsi_length);
    goto Error;
  }

  /* Construct return string - IMSI coded BCD - two characters per byte.
     However for byte #2 we copy only lower 4 bits to char buffer and
     one extra byte for null termination */
  imsi = malloc((2 * imsi_length));

  if (imsi == NULL)
  {
    status = MMGSDI_ERROR;
    QCRIL_LOG_ERROR( "%s: memory allocation failed\n", __FUNCTION__);
    goto Error;
  }

  /* Hurrah - we will actually return successfully */
  memset(imsi, 0x00, (2 * imsi_length));

  for (src = 1, dst = 0;
       (src <= imsi_length) && (dst < (imsi_length * 2));
       src++)
  {
    QCRIL_LOG_VERBOSE( "IMSI[%d] src=%4x, dst=", src,
                          cnf->read_data.data_ptr[src]);
    /* Only process lower part of byte for second and subsequent bytes */
    if (src > 1)
    {
      imsi[dst] = bin_to_hexchar(cnf->read_data.data_ptr[src] & 0x0F);
      QCRIL_LOG_VERBOSE( "%c", imsi[dst]);
      dst++;
    }
    /* Process upper part of byte for all bytes */
    imsi[dst] = bin_to_hexchar(cnf->read_data.data_ptr[src] >> 4);
    QCRIL_LOG_VERBOSE( "%c\n", imsi[dst]);
    dst++;
  }

  /* Generate response */
  qcril_mmgsdi_response( instance_id,
                         req->t,
                         cnf->response_header.mmgsdi_status,
                         imsi,
                         (2 * imsi_length),
                         TRUE,
                         NULL );

  /* Free IMSI after call completes */
  free(imsi);
  return;

  /* Exception case - generate error response */
 Error:
    qcril_mmgsdi_response( instance_id, req->t, status, NULL, 0, TRUE, NULL );
}


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_sim_io

===========================================================================*/
/*!
    @brief Handler for RIL_REQUEST_SIM_IO.

    Function is passed a RIL_SIM_IO structure pointer in the data field of @a
    params_ptr. This contains the basic fields of a 27.007 +CRSM command.
    This has the following fields:

    <table>
    <tr><th>Field</th>
        <th>Applicablity</th>
        <th>Purpose</th>
    </tr>
    <tr><td>command</td>
        <td>M</td>
        <td><dl>
            <dt>176</dt><dd>READ BINARY</dd>
            <dt>178</dt><dd>READ RECORD</dd>
            <dt>192</dt><dd>GET RESPONSE</dd>
            <dt>203</dt><dd>RETRIEVE DATA</dd>
            <dt>214</dt><dd>UPDATE BINARY</dd>
            <dt>219</dt><dd>SET DATA</dd>
            <dt>220</dt><dd>UPDATE RECORD</dd>
            <dt>242</dt><dd>STATUS</dd>
            </dl>
        </td>
    </tr>
    <tr><td>fileid</td>
        <td>M (O if command == STATUS)</td>
        <td>Identifier of an elementary data file</td>
    </tr>
    <tr><td>p1, p2, p3</td>
        <td>M (O if command == GET RESPONSE || command == STATUS)</td>
        <td>Parameter values sent with commands, semantics described in 51.011.</td>
    </tr>
    <tr><td>data</td>
        <td>O (Mandatory for commands writing data to SIM)</td>
        <td>Data to write to the selected record on the SIM. NULL if no data provided</td>
    </tr>
    <tr><td>path</td>
        <td>M</td>
        <td>Hex ASCII representation of path to the selected fileid.</td>
    </tr>
    <tr><td>PIN2</td>
        <td>O</td>
        <td>PIN2 value to send for locked fileids.</td>
    </tr>
    </table>

    Access to files via MMGSDI is always performed using access method
    MMGSDI_BY_PATH_ACCESS as the Android RIL is always passed complete path
    information. Path data is transformed into a uint16 array as required by
    MMGSDI.

    Radio state is not (directly) changed by any of the possible commands
    to qcril_mmgsdi_request_sim_io().

    @param req [in]  params_ptr->data points to a RIL_SIM_IO structure.
    @param ret [out] Updated radio state.
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_sim_io
(
  const qcril_request_params_type *const req,
  qcril_request_return_type       *const ret /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type    modem_id = QCRIL_DEFAULT_MODEM_ID;
  const RIL_SIM_IO_v6*        request;
  qcril_reqlist_public_type   reqlist_entry; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( req != NULL );
  instance_id = req->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret != NULL );

  /*-----------------------------------------------------------------------*/
  request = (RIL_SIM_IO_v6*) req->data;
  QCRIL_ASSERT(request != NULL);

  QCRIL_LOG_INFO( "qcril_mmgsdi_request_sim_io(%d, %d, %s, %d, %d, %d, %s, %s)\n",
                  request->command,
                  request->fileid,
                  request->path,
                  request->p1,
                  request->p2,
                  request->p3,
                  request->data != NULL ? request->data : "NULL",
                  request->pin2 != NULL ? request->pin2 : "NULL");

  QCRIL_MMGSDI_RETURN_ERR_RSP_IF_INVALID_CLIENT( req->instance_id,
                                                 qcril_mmgsdi.client_id,
                                                 req->t, FALSE );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( req->t, req->event_id, modem_id,
                               QCRIL_REQ_AWAITING_CALLBACK, 
                               QCRIL_EVT_NONE, NULL,
                               &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  switch (request->command)
  {
  case SIM_CMD_READ_BINARY:
    qcril_mmgsdi_read_binary(req);
    break;

  case SIM_CMD_READ_RECORD:
    qcril_mmgsdi_read_record(req);
    break;

  case SIM_CMD_GET_RESPONSE:
    qcril_mmgsdi_get_response(req);
    break;

  case SIM_CMD_UPDATE_BINARY:
    qcril_mmgsdi_update_binary(req);
    break;

  case SIM_CMD_RETRIEVE_DATA:
    // Not implemented
    QCRIL_LOG_ERROR( "%s", "NOTIMPL: qcril_mmgsdi_request_sim_io SIM_CMD_RETRIEVE_DATA\n");
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_NOT_SUPPORTED, NULL, 0, TRUE, NULL );
    break;

  case SIM_CMD_SET_DATA:
    // Not implemented
    QCRIL_LOG_ERROR( "%s", "NOTIMPL: qcril_mmgsdi_request_sim_io SIM_CMD_SET_DATA\n");
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_NOT_SUPPORTED, NULL, 0, TRUE, NULL );
    break;

  case SIM_CMD_UPDATE_RECORD:
    qcril_mmgsdi_update_record(req);
    break;

  case SIM_CMD_STATUS:
    qcril_mmgsdi_get_status(req);
    break;

  default:
    QCRIL_LOG_ERROR( "%s", "ILLEGAL: qcril_mmgsdi_request_sim_io unknown cmd\n");
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_NOT_SUPPORTED, NULL, 0, TRUE, NULL );
    break;
  }
} /* qcril_mmgsdi_request_sim_io() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_set_fdn_status_ext

===========================================================================*/
/*!
    @brief
    Utility function that calls  qcril_mmgsdi_request_set_fdn_status

    @param req_type     [in]  req type if enable/disable FDN.
    @param aid_len      [in]  length of aid data
    @param aid_data_ptr [in]  aid data buffer.
    @param token        [in]  the token for the command request list.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_set_fdn_status_ext
(
  qcril_instance_id_e_type              instance_id,
  qcril_modem_id_e_type                 modem_id,
  char                                  req_type,
  uint8                                 aid_len,
  uint8                               * aid_data_ptr,
  qcril_mmgsdi_internal_sim_data_type * internal_data_ptr
)
{
  int                     i                = 0;
  gsdi_returns_T          gsdi_status      = GSDI_ERROR;
  mmgsdi_return_enum_type mmgsdi_status    = MMGSDI_ERROR;
  char                    details[80];
  gsdi_app_id_type        aid;

  memset(details, 0x00, sizeof(details));
  memset(&aid, 0x00, sizeof(gsdi_app_id_type));

  if(internal_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s: failed null input param int_data_ptr can not send response\n",
                      __FUNCTION__);
    return;
  }

  if((aid_data_ptr == NULL) || (aid_len > GSDI_MAX_APP_ID_LEN))
  {
    QCRIL_LOG_ERROR( "%s: failed null input param aid_data_ptr\n",
                     __FUNCTION__);
    qcril_mmgsdi_response( instance_id, (RIL_Token)internal_data_ptr->token, mmgsdi_status, NULL, 0, TRUE, NULL );
    return;
  }

  QCRIL_SNPRINTF( details, sizeof( details ), "req_type=%d, aid_len=%d",
                  (int) req_type, (int) aid_len );
  QCRIL_LOG_RPC2( modem_id, "qcril_mmgsdi_request_set_fdn_status_ext()", details );

  memcpy((void *)aid.aid_data,(void *)aid_data_ptr,aid_len);

  if(req_type == QCRIL_GSDI_ENABLE_FDN_CMD)
  {
    QCRIL_LOG_RPC2( modem_id, "gsdi2_enable_fdn()", "" );
    gsdi_status = gsdi2_enable_fdn (GSDI_SLOT_1,
                                    FALSE,
                                    NULL,
                                    &aid,
                                    (uint32)internal_data_ptr,
                                    qcril_mmgsdi_gsdi_command_callback);
  }
  else if(req_type == QCRIL_GSDI_DISABLE_FDN_CMD)
  {
    QCRIL_LOG_RPC2( modem_id, "gsdi2_disable_fdn()", "" );
    gsdi_status = gsdi2_disable_fdn (GSDI_SLOT_1,
                                     FALSE,
                                     NULL,
                                     &aid,
                                     (uint32)internal_data_ptr,
                                     qcril_mmgsdi_gsdi_command_callback);
  }
  else
  {
    QCRIL_LOG_ERROR(
      "Invalid Input Param [1]: 0x%x for QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS \n",
      req_type);
    gsdi_status = GSDI_ERROR;
  }
  if (gsdi_status != GSDI_SUCCESS)
  {
    mmgsdi_status = qcril_mmgsdi_convert_from_gsdi_status(gsdi_status);
    QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, mmgsdi_status);
    qcril_mmgsdi_response( instance_id, (RIL_Token)internal_data_ptr->token, mmgsdi_status, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_request_set_fdn_status_ext */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_proceed_internal_cmd_processing

===========================================================================*/
/*!
    @brief Callback processing for RIL_REQUEST's internal
    PIN2 verification

    @param req_ptr [in]  Request object containing raw data returned by MMGSDI.
    @param ret_ptr [out] Updated radio state.
*/
/*=========================================================================*/
void qcril_mmgsdi_common_proceed_internal_cmd_processing
(
  const qcril_request_params_type* const req_ptr,
        qcril_request_return_type* const ret_ptr
)
{
  qcril_instance_id_e_type                    instance_id;
  qcril_modem_id_e_type                       modem_id;
  qcril_mmgsdi_command_callback_params_type*  cnf_raw_ptr       = NULL;
  mmgsdi_pin_operation_cnf_type*              cnf_ptr           = NULL;
  qcril_mmgsdi_internal_sim_data_type       * internal_data_ptr = NULL;
  mmgsdi_return_enum_type                     status            = MMGSDI_SUCCESS;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( req_ptr != NULL );
  instance_id = req_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = req_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_VERBOSE( "%s", "qcril_mmgsdi_common_proceed_internal_cmd_processing()");

  cnf_raw_ptr = (qcril_mmgsdi_command_callback_params_type*) req_ptr->data;
  if (cnf_raw_ptr == NULL)
  {
    status = MMGSDI_ERROR;
    QCRIL_LOG_ERROR( "%s: NULL cnf_raw_ptr\n", __FUNCTION__);
    goto Error;
  }

  cnf_ptr = (mmgsdi_pin_operation_cnf_type*) cnf_raw_ptr->cnf_ptr;
  if (cnf_ptr == NULL)
  {
    status = MMGSDI_ERROR;
    QCRIL_LOG_ERROR( "%s: NULL cnf_ptr\n", __FUNCTION__);
    goto Error;
  }

  internal_data_ptr =
    (qcril_mmgsdi_internal_sim_data_type *)cnf_ptr->response_header.client_data;
  if (internal_data_ptr == NULL)
  {
    status = MMGSDI_ERROR;
    QCRIL_LOG_ERROR( "%s: NULL internal_data_ptr\n", __FUNCTION__);
    goto Error;
  }

  if((cnf_ptr->response_header.mmgsdi_status == MMGSDI_SUCCESS) ||
     (cnf_ptr->response_header.mmgsdi_status == MMGSDI_INCORRECT_CODE) ||
     (cnf_ptr->response_header.mmgsdi_status == MMGSDI_CODE_BLOCKED))
  {
    /* Store the pin information */
    memcpy((void *)&internal_data_ptr->pin_info, (void *)&cnf_ptr->pin_info,
           sizeof(mmgsdi_pin_info_type));
  }

  /* Check that pin verification was OK */
  if (cnf_ptr->response_header.mmgsdi_status != MMGSDI_SUCCESS)
  {
    status = cnf_ptr->response_header.mmgsdi_status;
    QCRIL_LOG_ERROR( "%s: failed to verify PIN\n", __FUNCTION__);
    goto Error;
  }

  /* Proceed with the rest of SIM IO operation */
  switch(internal_data_ptr->cmd)
  {
    case QCRIL_MMGSDI_ORIG_SIM_IO_UPDATE_BINARY:
      qcril_mmgsdi_common_write_transparent_ext(
        instance_id,
        modem_id,
        internal_data_ptr->access,
        internal_data_ptr->offset,
        internal_data_ptr->data,
        (mmgsdi_client_data_type)internal_data_ptr->token);
      break;
    case QCRIL_MMGSDI_ORIG_SIM_IO_UPDATE_RECORD:
      qcril_mmgsdi_common_write_record_ext(
        instance_id,
        modem_id,
        internal_data_ptr->access,
        internal_data_ptr->record,
        internal_data_ptr->data,
        (mmgsdi_client_data_type)internal_data_ptr->token);
      break;
    case QCRIL_MMGSDI_ORIG_SIM_IO_READ_BINARY:
      qcril_mmgsdi_common_read_transparent_ext(
        instance_id,
        modem_id,
        internal_data_ptr->access,
        internal_data_ptr->offset,
        internal_data_ptr->length,
        (mmgsdi_client_data_type)internal_data_ptr->token);
      break;
    case QCRIL_MMGSDI_ORIG_SIM_IO_READ_RECORD:
      qcril_mmgsdi_common_read_record_ext(
        instance_id, 
        modem_id,
        internal_data_ptr->access,
        internal_data_ptr->record,
        internal_data_ptr->length,
        (mmgsdi_client_data_type)internal_data_ptr->token);
      break;
      /* For enable/disable FDN requests we will still need PIN retry info
        to be passed in the response. We shall free internal_data_ptr
        in the callback handler for enable/disable FDN function */
    case QCRIL_MMGSDI_ORIG_ENABLE_FDN:
      qcril_mmgsdi_request_set_fdn_status_ext(
        instance_id,
        modem_id,
        QCRIL_GSDI_ENABLE_FDN_CMD,
        internal_data_ptr->aid_len,
        internal_data_ptr->aid_data,
        internal_data_ptr );
      qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN1);
      return;
    case QCRIL_MMGSDI_ORIG_DISABLE_FDN:
      qcril_mmgsdi_request_set_fdn_status_ext(
        instance_id,
        modem_id,
        QCRIL_GSDI_DISABLE_FDN_CMD,
        internal_data_ptr->aid_len,
        internal_data_ptr->aid_data,
        internal_data_ptr );
      qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN1);
      return;
    default:
      QCRIL_LOG_ERROR( "%s: invalid orig cmd for internal pin2 verification\n",
                        __FUNCTION__);
      break;
  }

  if(internal_data_ptr->data.data_ptr)
    free(internal_data_ptr->data.data_ptr);
  free(internal_data_ptr);

  /* reset curr pin back to PIN1.  If we arrive here, it means then PIN2 verification
     was successful, so even if there is a failure for access later on, it should not
     be due to PIN2 error hence, no need to wait to reset current pin until then */
  qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN1);
  return;

  /* Exception case - generate error response */
 Error:
  if(((internal_data_ptr != NULL) &&
      ((internal_data_ptr->cmd == QCRIL_MMGSDI_ORIG_ENABLE_FDN) ||
       (internal_data_ptr->cmd == QCRIL_MMGSDI_ORIG_DISABLE_FDN))) &&
     ((status == MMGSDI_INCORRECT_CODE) ||
      (status == MMGSDI_CODE_BLOCKED)))
  {
    /* For FDN Enable/Disable commands send pin retries in response
       if PIN2 verification failed. For general failures send
       simple response */
    QCRIL_LOG_DEBUG( "%s - num_retries = 0x%x \n",__FUNCTION__,
                     internal_data_ptr->pin_info.num_retries);
    qcril_mmgsdi_response(
      req_ptr->instance_id,
      (RIL_Token)req_ptr->t,
      status,
      &internal_data_ptr->pin_info.num_retries,
      sizeof(int32),
      TRUE,
      NULL);
  }
  else
  {
    qcril_mmgsdi_response( instance_id, req_ptr->t, status, NULL, 0, TRUE, NULL );
  }

  if(internal_data_ptr)
  {
    if(internal_data_ptr->data.data_ptr)
      free(internal_data_ptr->data.data_ptr);
    free(internal_data_ptr);
  }

  /* reset curr pin back to PIN1.  This should be set after response is being
     sent */
  qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN1);
} /* qcril_mmgsdi_common_proceed_sim_io_processing */


/*=========================================================================*/
/*! @brief Handler for SIM IO read response callback from MMGSDI

    Construction of an appropriate response requires the following to be
    performed:
     - Set next radio state (unchanged)
     - Construct RIL_SIM_IO_Response with SW1 and SW2 fields determined
       depending on the status returned by MMGSDI.
     - Turn (binary) data returned by MMGSDI into ASCII hex string
     - Return response object and call RIL_onRequestComplete().

    @param req [in]  Request object containing raw data returned by MMGSDI.
    @param ret [out] Updated radio state.
 */
/*=========================================================================*/
void qcril_mmgsdi_common_simio_read_cnf
(
 const qcril_request_params_type* const req,
       qcril_request_return_type* const ret
)
{
  qcril_instance_id_e_type                   instance_id = req->instance_id;
  qcril_mmgsdi_command_callback_params_type* cnf_raw;
  mmgsdi_read_cnf_type*                      cnf;
  RIL_SIM_IO_Response*                       rsp;
  int                                        sw2_set = FALSE;

  cnf_raw = (qcril_mmgsdi_command_callback_params_type*) req->data;

  if (cnf_raw == NULL)
  {
    QCRIL_LOG_ERROR( "%s: cnf_raw is NULL\n", __FUNCTION__);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  cnf     = (mmgsdi_read_cnf_type*) cnf_raw->cnf_ptr;

  if (cnf == NULL)
  {
    QCRIL_LOG_ERROR( "%s: cnf is NULL\n",
                       __FUNCTION__);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Perform common processing */
  rsp = qcril_mmgsdi_common_simio_common_cnf(
    cnf->response_header.mmgsdi_status,
    &sw2_set);

  if (rsp != NULL)
  {
    /* Normal case - rsp allocation succeeded */

    /* Transform returned data into ASCII hex */
    rsp->simResponse = bin_to_hexstring(cnf->read_data.data_ptr,
                                        cnf->read_data.data_len);

    QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d data=%s\n",
                     rsp->sw1, rsp->sw2,
                     rsp->simResponse != NULL ? rsp->simResponse : "NULL");

    /* Generate response */
    qcril_mmgsdi_response( instance_id,
                           req->t,
                           cnf->response_header.mmgsdi_status,
                           rsp,
                           sizeof(RIL_SIM_IO_Response),
                           TRUE,
                           NULL );

    /* Free rsp after call completes */
    free(rsp->simResponse);
    free(rsp);
  }
  else
  {
    /* Error case: rsp allocation failed */
    QCRIL_LOG_ERROR( "%s: failed to generate RIL_SIM_IO_Response\n",
                     __FUNCTION__);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
  }

} /* qcril_mmgsdi_common_simio_read_cnf */


/*=========================================================================*/
/*! @brief Handler for SIM IO update response callback from MMGSDI

    Construction of an appropriate response requires the following to be
    performed:
     - Set next radio state (unchanged)
     - Construct RIL_SIM_IO_Response with SW1 and SW2 fields determined
       depending on the status returned by MMGSDI.
     - Return response object and call RIL_onRequestComplete(). Note that,
       per 27.007, there is no response data sent up with update responses.

    @param req [in]  Request object containing raw data returned by MMGSDI.
    @param ret [out] Updated radio state.
 */
/*=========================================================================*/
void qcril_mmgsdi_common_simio_update_cnf
(
 const qcril_request_params_type* const req,
       qcril_request_return_type* const ret
)
{
  qcril_instance_id_e_type                   instance_id = req->instance_id;
  qcril_modem_id_e_type                      modem_id = req->modem_id;
  qcril_mmgsdi_command_callback_params_type* cnf_raw;
  RIL_SIM_IO_Response*                       rsp;
  int                                        sw2_set = FALSE;
  IxErrnoType                                err_no;
  mmgsdi_write_cnf_type                      *w_cnf;

  cnf_raw = (qcril_mmgsdi_command_callback_params_type*) req->data;

  if (cnf_raw == NULL)
  {
    QCRIL_LOG_ERROR( "%s: cnf_raw is NULL\n", __FUNCTION__);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  w_cnf = (mmgsdi_write_cnf_type *)cnf_raw->cnf_ptr;
  if ( (w_cnf != NULL) && (qcril_mmgsdi.fdn_enable == QCRIL_MMGSDI_FDN_ENABLED) &&
       (cnf_raw->status == MMGSDI_SUCCESS) && 
        (((w_cnf->access.access_method == MMGSDI_EF_ENUM_ACCESS) && 
        ((w_cnf->access.file.file_enum == MMGSDI_TELECOM_FDN) ||
        (w_cnf->access.file.file_enum == MMGSDI_USIM_FDN)))
        || ((w_cnf->access.access_method == MMGSDI_BY_PATH_ACCESS)
        && (w_cnf->access.file.path_type.path_buf[w_cnf->access.file.path_type.path_len-1] == 0x6F3B))) )
  {
    /* Toggle the FDN status only when it is enabled. If disabled, user will anyway have to 
    enable the status while activating FDN on the phone. This will inturn trigger PBM to 
    update its cache */
    QCRIL_LOG_RPC2( modem_id, "gsdi2_disable_fdn()", "" );
    gsdi2_disable_fdn (GSDI_SLOT_1,
                             FALSE,
                              NULL,
                              NULL,
                                 0,
                              qcril_mmgsdi_gsdi_command_callback);

    QCRIL_LOG_RPC2( modem_id, "gsdi2_enable_fdn()", "" );
    gsdi2_enable_fdn (GSDI_SLOT_1,
                            FALSE,
                             NULL,
                             NULL,
                                0,
                             qcril_mmgsdi_gsdi_command_callback);

    /* Now wait for FDN record update from PBM */
    qcril_reqlist_update_pending_event_id( instance_id, req->modem_id, req->t, QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE );
    err_no = qcril_reqlist_update_state( instance_id, req->modem_id, req->t, QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS );
    QCRIL_ASSERT( err_no == E_SUCCESS );
  }
  else
  {
    /* Perform common processing */
    rsp = qcril_mmgsdi_common_simio_common_cnf(
              cnf_raw->status,
              &sw2_set);

    if (rsp != NULL)
    {
      /* Normal case - rsp allocation succeeded */

      /* data field is always NULL (strictly not required as rsp was cleared
       to all zero value on creation, but clearer to understand...)
      */
      rsp->simResponse = NULL;

      QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d data=%s\n",
                     rsp->sw1, rsp->sw2, "NULL");

      /* Generate response */
      qcril_mmgsdi_response( instance_id,
                             req->t,
                             cnf_raw->status,
                             rsp,
                             sizeof(RIL_SIM_IO_Response),
                             TRUE,
                             NULL );
      /* Free rsp after call completes (rsp->simResponse is always NULL here */
      free(rsp);
    }
    else
    {
      /* Error case: rsp allocation failed */
      QCRIL_LOG_ERROR( "%s: failed to generate RIL_SIM_IO_Response\n",
                       __FUNCTION__);
      qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    }
  }
} /* qcril_mmgsdi_common_simio_update_cnf */


/*=========================================================================*/
/*! @brief Handler for SIM IO status response callback from MMGSDI

    Construction of an appropriate response requires the following to be
    performed:
     - Set next radio state (unchanged)
     - Construct RIL_SIM_IO_Response with SW1 and SW2 fields determined
       depending on the status returned by MMGSDI.
     - Return response object and call RIL_onRequestComplete(). Note that,
       per 27.007, there is no response data sent up with update responses.

    @param req [in]  Request object containing raw data returned by MMGSDI.
    @param ret [out] Updated radio state.
 */
/*=========================================================================*/
void qcril_mmgsdi_common_simio_get_status_cnf
(
 const qcril_request_params_type* const req,
       qcril_request_return_type* const ret
)
{
  qcril_instance_id_e_type                   instance_id = req->instance_id;
  qcril_mmgsdi_command_callback_params_type* cnf_raw;
  mmgsdi_status_cnf_type*                    cnf;
  RIL_SIM_IO_Response*                       rsp;
  int                                        sw2_set;

  cnf_raw = (qcril_mmgsdi_command_callback_params_type*) req->data;

  if (cnf_raw == NULL)
  {
    QCRIL_LOG_ERROR( "%s: cnf_raw is NULL\n",
                     __FUNCTION__);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  cnf = (mmgsdi_status_cnf_type*) &cnf_raw->cnf_ptr;

  if (cnf == NULL)
  {
    QCRIL_LOG_ERROR( "%s: cnf is NULL\n",
                     __FUNCTION__);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  /* Perform common processing */
  rsp = qcril_mmgsdi_common_simio_common_cnf(
    cnf->response_header.mmgsdi_status,
    &sw2_set);

  if (rsp != NULL)
  {
    /* Normal case - rsp allocation succeeded */

    /* Transform returned data into ASCII hex */
    rsp->simResponse = bin_to_hexstring(cnf->status_data.data_ptr,
                                        cnf->status_data.data_len);

    QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d data=%s\n",
                     rsp->sw1, rsp->sw2,
                     rsp->simResponse != NULL ? rsp->simResponse : "NULL");

    /* Generate response */
    qcril_mmgsdi_response( instance_id,
                           req->t,
                           cnf->response_header.mmgsdi_status,
                           rsp,
                           sizeof(RIL_SIM_IO_Response),
                           TRUE,
                           NULL );

    /* Free rsp after call completes */
    free(rsp->simResponse);
    free(rsp);
  }
  else
  {
    /* Error case: rsp allocation failed */
    QCRIL_LOG_ERROR( "%s: failed to generate RIL_SIM_IO_Response\n",
                     __FUNCTION__);
    qcril_mmgsdi_response( instance_id, req->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_common_simio_get_status_cnf() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_process_select_cnf

===========================================================================*/
/*!
    @brief
    Handle Select confirmation (for SIM IO Get Response Solicited command)

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_common_process_select_cnf
(
  qcril_instance_id_e_type instance_id,
  const gsdi_select_cnf_T*    select_rsp_ptr
)
{
  uint32                  ril_token;
  RIL_SIM_IO_Response*    rsp_ptr       = NULL;
  int                     sw2_set       = FALSE;
  mmgsdi_return_enum_type mmgsdi_status = MMGSDI_SUCCESS;

  /*-----------------------------------------------------------------------*/

  if (select_rsp_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s: null select_rsp_ptr\n", __FUNCTION__);
    return;
  }

  QCRIL_LOG_DEBUG( "%s: status = 0x%x\n", __FUNCTION__, select_rsp_ptr->message_header.gsdi_status);

  ril_token = select_rsp_ptr->message_header.client_ref;

  mmgsdi_status = qcril_mmgsdi_convert_from_gsdi_status(
                    select_rsp_ptr->message_header.gsdi_status);

  /* Perform common processing */
  rsp_ptr = qcril_mmgsdi_common_simio_common_cnf(
    mmgsdi_status,
    &sw2_set);

  if (rsp_ptr != NULL)
  {

    /* Transform returned data into ASCII hex */
    rsp_ptr->simResponse = bin_to_hexstring(select_rsp_ptr->data,
                                            select_rsp_ptr->returned_data_len);

    QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d data=%s\n",
                     rsp_ptr->sw1, rsp_ptr->sw2,
                     rsp_ptr->simResponse != NULL ? rsp_ptr->simResponse : "NULL");

    /* Generate response */
    qcril_mmgsdi_response( instance_id,
                           (RIL_Token) ril_token,
                           mmgsdi_status,
                           rsp_ptr,
                           sizeof(RIL_SIM_IO_Response),
                           TRUE,
                           NULL );

    /* Free rsp after call completes */
    free(rsp_ptr->simResponse);
    free(rsp_ptr);
  }
  else
  {
    /* Error case: rsp allocation failed */
    QCRIL_LOG_ERROR( "%s: failed to generate RIL_SIM_IO_Response\n", __FUNCTION__);
    qcril_mmgsdi_response( instance_id, (RIL_Token)ril_token, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
  }
} /* qcril_mmgsdi_common_process_select_cnf() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_set_fdn_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS
    (RIL request: RIL_REQUEST_SET_FACILITY_LOCK - FD)

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_set_fdn_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type            instance_id;
  qcril_modem_id_e_type               modem_id;
  mmgsdi_return_enum_type             mmgsdi_status     = MMGSDI_ERROR;
  gsdi_returns_T                      gsdi_status       = GSDI_SUCCESS;
  uint8                               **in_data_ptr       = NULL;
  int32                               pin_len           = 0;
  uint8                               pin2[GSDI_PIN_LEN];
  gsdi_app_id_type                    aid;
  qcril_mmgsdi_internal_sim_data_type *internal_data_ptr = NULL;
  qcril_reqlist_public_type reqlist_entry; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  memset(pin2, 0x00, GSDI_PIN_LEN);
  aid.aid_len = 0;
  memset(aid.aid_data, 0x00, GSDI_MAX_APP_ID_LEN);

  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_request_set_fdn_status\n" );
  QCRIL_MMGSDI_RETURN_ERR_RSP_IF_INVALID_CLIENT( instance_id, qcril_mmgsdi.client_id, params_ptr->t, FALSE );

  /* Obtain input pin info:
     ((const char **)data)[0] = facility string code from TS 27.007 7.4
     (eg "AO" for BAOC)
     ((const char **)data)[1] = "0" for "unlock" and "1" for "lock"
     ((const char **)data)[2] = password
     ((const char **)data)[3] = string representation of decimal TS 27.007
                                service class bit vector. Eg, the string
                                "1" means "set this facility for voice services"  */
  in_data_ptr = (uint8 **)(params_ptr->data);

  /* Null pointer check for extracted input data */
  QCRIL_ASSERT(  in_data_ptr    != NULL );
  QCRIL_ASSERT(  in_data_ptr[0] != NULL );
  QCRIL_ASSERT(  in_data_ptr[1] != NULL );
  QCRIL_ASSERT(  in_data_ptr[2] != NULL );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id,
                               QCRIL_REQ_AWAITING_CALLBACK, QCRIL_EVT_NONE,
                               NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  if (memcmp(in_data_ptr[0], "FD", 2) != 0)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid Input Parameter [0] for QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS\n" );
    qcril_mmgsdi_response( instance_id, params_ptr->t, mmgsdi_status, NULL, 0, TRUE, NULL );
    return;
  }

  /* Populate GSDI required function memeber type */
  pin_len = (int32)strlen((const char*) in_data_ptr[2]);
  if (pin_len > GSDI_PIN_LEN)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid Pin2 length for QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS\n" );
    qcril_mmgsdi_response( instance_id, params_ptr->t, mmgsdi_status, NULL, 0, TRUE, NULL );
    return;
  }

  /* malloc and populate internal data for subsequent processing */
  internal_data_ptr = malloc(sizeof(qcril_mmgsdi_internal_sim_data_type));
  if(internal_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s: failed alloc internal_data_ptr\n", __FUNCTION__);
    qcril_mmgsdi_response( instance_id, params_ptr->t, mmgsdi_status, NULL, 0, TRUE, NULL );
    return;
  }

  memset(internal_data_ptr, 0x00, sizeof(qcril_mmgsdi_internal_sim_data_type));
  internal_data_ptr->token  = params_ptr->t;
  internal_data_ptr->do_pin_verify = FALSE;

  if (pin_len > 0)
  {
    mmgsdi_data_type   pin_data;

    /* Queue a verify PIN2 command first */

    /* Populate the mmgsdi required members */
    memcpy(pin2, in_data_ptr[2], pin_len);
    pin_data.data_len = pin_len;
    pin_data.data_ptr = (uint8*)pin2;

    internal_data_ptr->do_pin_verify = TRUE;

    if ('1' == *in_data_ptr[1])
    {
      internal_data_ptr->cmd = QCRIL_MMGSDI_ORIG_ENABLE_FDN;
    }
    else if ('0' == *in_data_ptr[1])
    {
      internal_data_ptr->cmd = QCRIL_MMGSDI_ORIG_DISABLE_FDN;
    }
    internal_data_ptr->aid_len = aid.aid_len;
    memcpy((void *)internal_data_ptr->aid_data,
           (void *)aid.aid_data,aid.aid_len);

    QCRIL_LOG_RPC2( modem_id, "mmgsdi_verify_pin PIN2()", "");
    qcril_mmgsdi_print_byte_data(pin_data.data_len, pin_data.data_ptr);
    mmgsdi_status = mmgsdi_verify_pin (qcril_mmgsdi.client_id,
                                MMGSDI_SLOT_1,
                                MMGSDI_PIN2,
                                pin_data,
                                qcril_mmgsdi_internal_verify_pin_command_callback,
                                (mmgsdi_client_data_type)internal_data_ptr);
    if (mmgsdi_status != MMGSDI_SUCCESS)
    {
      free(internal_data_ptr);
      QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, mmgsdi_status);
      qcril_mmgsdi_response( instance_id, params_ptr->t, mmgsdi_status, NULL, 0, TRUE, NULL );
    }
    /* set current pin to PIN2 */
    qcril_mmgsdi_set_curr_pin_ptr(MMGSDI_PIN2);
    return;
  }

  /* If PIN2 is not provided send enable/disable request directly to
     modem. 
     Note - *in_data_ptr[1] = indicates whether operation is enable or disable
     values are mapped as follows
      - QCRIL_GSDI_ENABLE_FDN_CMD        '1'
      - QCRIL_GSDI_DISABLE_FDN_CMD       '0' */
  QCRIL_LOG_INFO( "FDN Request without PIN2 verification", "");
  qcril_mmgsdi_request_set_fdn_status_ext(
    instance_id,
    modem_id,
    *in_data_ptr[1],
    aid.aid_len,
    aid.aid_data,
    internal_data_ptr );

  /* We free internal_data_ptr in function 
    qcril_mmgsdi_common_process_fdn_status_cnf() */

} /* qcril_mmgsdi_request_set_fdn_status() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_request_get_fdn_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS
    (RIL request: RIL_REQUEST_QUERY_FACILITY_LOCK - FD)

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_request_get_fdn_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type  instance_id;
  qcril_modem_id_e_type     modem_id = QCRIL_DEFAULT_MODEM_ID;
  uint8                        **in_data_ptr     = NULL;
  mmgsdi_return_enum_type      mmgsdi_status   = MMGSDI_SUCCESS;
  int                          enable          = 0;
  qcril_reqlist_public_type    reqlist_entry; 

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_request_get_fdn_status\n" );

  /* Obtain input pin info:
     ((const char **)data)[1] is the facility string code from TS 27.007 7.4
                              (eg "AO" for BAOC, "SC" for SIM lock)
     ((const char **)data)[2] is the password, or "" if not required
     ((const char **)data)[3] is the TS 27.007 service class bit vector of
                              services to query  */
  in_data_ptr = (uint8 **)(params_ptr->data);

  /* Null pointer check for extracted input data */
  QCRIL_ASSERT(  in_data_ptr    != NULL );
  QCRIL_ASSERT(  in_data_ptr[0] != NULL );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id,
                               QCRIL_REQ_AWAITING_CALLBACK, QCRIL_EVT_NONE,
                               NULL, &reqlist_entry ); 
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
    return;
  }

  if (memcmp(in_data_ptr[0], "FD", 2) != 0)
  {
    QCRIL_LOG_ERROR( "%s", "Invalid Input Parameter[0] for QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS\n" );
    qcril_mmgsdi_response( instance_id, params_ptr->t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
    return;
  }

  switch (qcril_mmgsdi.fdn_enable)
  {
  case QCRIL_MMGSDI_FDN_DISABLED:
    QCRIL_LOG_INFO( "%s", "FDN disable\n" );
    enable = 0;
    break;

  case QCRIL_MMGSDI_FDN_ENABLED:
    QCRIL_LOG_INFO( "%s", "FDN enable\n" );
    enable = 1;
    break;

  case QCRIL_MMGSDI_FDN_NOT_AVAILABLE:
    QCRIL_LOG_INFO( "%s", "FDN not available\n" );
    enable = 2;
    break;

  case QCRIL_MMGSDI_FDN_NOT_INIT:
    QCRIL_LOG_INFO( "%s", "FDN status not initialized\n" );
    mmgsdi_status = MMGSDI_ERROR;
    break;

  default:
    QCRIL_LOG_ERROR( "FDN status unknown 0x%x\n", qcril_mmgsdi.fdn_enable );
    mmgsdi_status = MMGSDI_ERROR;
    break;

  }

  qcril_mmgsdi_response( instance_id, params_ptr->t, mmgsdi_status, &enable, sizeof(int), TRUE, NULL );

} /* qcril_mmgsdi_request_get_fdn_status() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_init_ef_maps

===========================================================================*/
void qcril_mmgsdi_init_maps()
{
  int i;

  /* Construct the GSM EF map */
  efmap_gsm = qcril_map_constructor(EFMAP_SIZE);
  for (i = 0; gsm_ef_mapping[i].EF != 0; i++)
  {
    qcril_map_add(efmap_gsm, gsm_ef_mapping[i].EF,
                  &gsm_ef_mapping[i].file, IS_A_PTR);
  }

  /* Construct the UMTS EF map */
  efmap_umts = qcril_map_constructor(EFMAP_SIZE);
  for (i = 0; umts_ef_mapping[i].EF != 0; i++)
  {
    qcril_map_add(efmap_umts, umts_ef_mapping[i].EF,
                  &umts_ef_mapping[i].file, IS_A_PTR);
  }

  /* Construct the MMGSDI status map. NB: Do not attempt to call
     qcril_map_destructor() on status_map as it contains aliases to
     values stored in a static table, and you'd get an access violation.
     (in any case, there should *never* be a need to call destructor).
   */
  status_map = qcril_map_constructor(MMGSDI_STATUS_SIZE);
  for (i = 0; (int) mmgsdi_to_sw1sw2[i].sw1sw2.sw1 != 0; i++)
  {
    qcril_map_add(status_map, (int) mmgsdi_to_sw1sw2[i].mmgsdi,
                  &mmgsdi_to_sw1sw2[i].sw1sw2, IS_A_PTR);
  }
}
/*lint -restore */

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_fdn_record_update_from_pbm

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_process_fdn_record_update_from_pbm
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type    modem_id;
  qcril_reqlist_public_type    req_info;
  RIL_SIM_IO_Response*         rsp;
  int                          sw2_set = FALSE;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_process_fdn_record_update_from_pbm\n" );

  if ( qcril_reqlist_query_by_event( instance_id, modem_id,
                                     QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE, 
                                     &req_info ) == E_SUCCESS )
  {
    if ( req_info.request == RIL_REQUEST_SIM_IO )
    {
      /* Perform common processing */
      rsp = qcril_mmgsdi_common_simio_common_cnf( MMGSDI_SUCCESS, &sw2_set);
      if (rsp != NULL)
      {
        /* Normal case - rsp allocation succeeded */
        /* data field is always NULL (strictly not required as rsp was cleared
        to all zero value on creation, but clearer to understand...)
        */
        rsp->simResponse = NULL;

        QCRIL_LOG_DEBUG( "RIL_SIM_IO_Response: sw1=%d sw2=%d data=%s\n",
                     rsp->sw1, rsp->sw2, "NULL");

        /* Generate response */
        qcril_mmgsdi_response( instance_id,
                               req_info.t,
                               MMGSDI_SUCCESS,
                               rsp,
                               sizeof(RIL_SIM_IO_Response),
                               TRUE,
                               NULL );
        
        /* Free rsp after call completes (rsp->simResponse is always NULL here */
        free(rsp);
      }
      else
      {
        /* Error case: rsp allocation failed */
        QCRIL_LOG_ERROR( "%s: failed to generate RIL_SIM_IO_Response\n",
                        __FUNCTION__);
        qcril_mmgsdi_response( instance_id, req_info.t, MMGSDI_ERROR, NULL, 0, TRUE, NULL );
      }
    }
  }
} /* qcril_mmgsdi_process_fdn_record_update_from_pbm() */
#endif /* FEATURE_CDMA_NON_RUIM */

#endif /* !defined (FEATURE_QCRIL_UIM_QMI) */
