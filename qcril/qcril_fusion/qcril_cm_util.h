/*!
  @file
  qcril_cm_util.h

  @brief
  Utilities functions to support QCRIL_CM processing.

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_cm_util.h#10 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
11/12/09   sb      Added sanity timer to info rec processing.
09/21/09   pg      Fixed Registration State info when phone is in Hybrid mode 
                   and only HDR is available.                 
07/17/09   pg      Added default support for CDMA registration reject cause.
06/15/09   nd      Added support to check the CDMA emergency flash number in ECC property file.
06/15/09   nd      Added support for CDMA Time of Day.
06/06/09   nrn     Adding support for Authentication and Registration Reject
05/14/09   pg      Added support for CDMA phase II under FEATURE_MULTIMODE_ANDROID_2.
02/25/09   fc      Moved all ONS support to qcril_cm_ons.c.
                   Mainlined FEATURE_CM_UTIL_RPC_AVAIL.
02/04/09   pg      Call cmutil RPC APIs when FEATURE_CM_UTIL_RPC_AVAIL is
                   defined.
05/21/08   jar     Featurized for off target platform compilation
05/08/08   fc      First cut implementation.


===========================================================================*/

#ifndef QCRIL_CM_UTIL_H
#define QCRIL_CM_UTIL_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "comdef.h"
#include "cm.h"
#include "qcril_arb.h"
#include "qcril_cm.h"


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/* For Off-target testing in MS-VS2005 strtok_r is not supported so using equivalent strtok_s */
#ifdef _MSC_VER
#define strtok_r strtok_s
#endif


/*===========================================================================

                   EXTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/*
** defines needed for CM_UTIL functions being transferred to qcril_cm_util.c
*/
#define QCRIL_CM_NUM_TYPE_INTERNATIONAL 1
#define QCRIL_BCD_LEN 0
#define QCRIL_BCD_NT_NPI 1
#define QCRIL_BCD_NUM 2
#define QCRIL_BCD_STAR  0x0A
#define QCRIL_BCD_HASH  0x0B

#define QCRIL_INRANGE( val, min, max ) (((int32_t)val) >= ((int32_t)min) && ((int32_t)val) <= ((int32_t)max))
#define QCRIL_ISDIGIT(X) ((X >= '0') && (X <= '9'))

static const char qcril_def_alpha_to_ascii_table[] =
{
 '@',  0xA3, '$',  0xA5, 0xE8, 0xE9, 0xF9, 0xEC,
 0xF2, 0xC7, 0x0A, 0xD8, 0xF8, 0x0D, 0xC5, 0xE5,
 '?',  '_',  '?',  '?',  '?',  '?',  0xB6, '?',
 '?',  '?',  '?',  0x1B, 0xC6, 0xE6, 0xDF, 0xC9,
 ' ',  '!',  0x22, '#',  0xA4,  '%',  '&', 0x27,
 '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
 '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
 '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
 0xA1, 'A',  'B',  'C',  'D',  'E',  'F',  'G',
 'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
 'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',
 'X',  'Y',  'Z',  0xC4, 0xD6, 0xD1, 0xDC, 0xA7,
 0xBF, 'a',  'b',  'c',  'd',  'e',  'f',  'g',
 'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
 'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
 'x',  'y',  'z',  0xE4, 0xF6, 0xF1, 0xFC, 0xE0
};

static const byte qcril_ascii_to_def_alpha_table[] =
{
0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x0A,0x2E,0x2E,0x0D,0x2E,0x2E,
0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,0x2E,
0x20,0x21,0x22,0x23,0x02,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x00,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x28,0x2F,0x29,0x2E,0x11,
0x2F,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x21,0x2F,0x29,0x2D,0x2E
};

#ifndef PLATFORM_LTK
/*
** Enumeration of system band classes and band classes' sub-bands.
** If there is a new band class, append to the end of list.
*/
typedef enum {
  SYS_SBAND_BC0_A = 0,
    /* Band Class 0, A-System */

  SYS_SBAND_BC0_B = 1,
    /* Band Class 0, B-System */

    /* Band Class 0 AB , GSM 850 Band*/

  SYS_SBAND_BC1 = 2,
    /* Band Class 1, all blocks */

  SYS_SBAND_BC2 = 3,
    /* Band Class 2 place holder */

  SYS_SBAND_BC3 = 4,
    /* Band Class 3, A-System */

  SYS_SBAND_BC4 = 5,
    /* Band Class 4, all blocks */

  SYS_SBAND_BC5 = 6,
    /* Band Class 5, all blocks */

  SYS_SBAND_GSM_DCS_1800 = 7,
    /* GSM DCS band */

  SYS_SBAND_GSM_EGSM_900 = 8,
    /* GSM Extended GSM (E-GSM) band */

  SYS_SBAND_GSM_PGSM_900 = 9,
    /* GSM Primary GSM (P-GSM) band */

  SYS_SBAND_BC6 = 10,
    /* Band Class 6 */

  SYS_SBAND_BC7 = 11,
    /* Band Class 7 */

  SYS_SBAND_BC8 = 12,
    /* Band Class 8 */

  SYS_SBAND_BC9 = 13,
    /* Band Class 9*/

  SYS_SBAND_BC10 = 14,
    /* Band Class 10 */

  SYS_SBAND_BC11 = 15,
    /* Band Class 11 */

  SYS_SBAND_GSM_450 = 16,
    /* GSM 450 band */

  SYS_SBAND_GSM_480 = 17,
    /* GSM 480 band */

  SYS_SBAND_GSM_750 = 18,
    /* GSM 750 band */

  SYS_SBAND_GSM_850 = 19,
    /* GSM 850 band */

  SYS_SBAND_GSM_RGSM_900 = 20,
    /* GSM Railways GSM Band */

  SYS_SBAND_GSM_PCS_1900 = 21,
    /* GSM PCS band */

  SYS_SBAND_WCDMA_I_IMT_2000 = 22,
    /* WCDMA EUROPE JAPAN & CHINA IMT 2100 band */

  SYS_SBAND_WCDMA_II_PCS_1900 = 23,
    /* WCDMA US PCS 1900 band */

  SYS_SBAND_WCDMA_III_1700 = 24,
    /* WCDMA EUROPE & CHINA DCS 1800 band */

  SYS_SBAND_WCDMA_IV_1700 = 25,
    /* WCDMA US 1700 band */

  SYS_SBAND_WCDMA_V_850 = 26,
    /* WCDMA US850 band */

  SYS_SBAND_WCDMA_VI_800 = 27,
    /* WCDMA JAPAN 800 band */

  SYS_SBAND_BC12 = 28,
    /* Band Class 12 */

  SYS_SBAND_BC14 = 29,
   /* Band Class 14 */

  SYS_SBAND_RESERVED_2 = 30,
   /* Reserved 2 */

  SYS_SBAND_BC15 = 31,
   /* Band Class 15 */


  SYS_SBAND_WLAN_US_2400 = 32,
    /* WLAN US 2400 band */

  SYS_SBAND_WLAN_EUROPE_2400 = 33,
    /* WLAN ETSI 2400 band */

  SYS_SBAND_WLAN_FRANCE_2400 = 34,
    /* WLAN FRANCE 2400 band */

  SYS_SBAND_WLAN_SPAIN_2400 = 35,
    /* WLAN SPAIN 2400 band */

  SYS_SBAND_WLAN_JAPAN_2400 = 36,
    /* WLAN JAPAN 2400 band */

  SYS_SBAND_WLAN_US_5000 = 37,
    /* WLAN US 2400 band */

  SYS_SBAND_WLAN_EUROPE_5000 = 38,
    /* WLAN EUROPE 5000 band */

  SYS_SBAND_WLAN_FRANCE_5000 = 39,
    /* WLAN FRANCE 5000 band */

  SYS_SBAND_WLAN_SPAIN_5000 = 40,
    /* WLAN SPAIN 5000 band */

  SYS_SBAND_WLAN_JAPAN_5000 = 41,
    /* WLAN JAPAN 5000 band */

  SYS_SBAND_WCDMA_VII_2600 = 48,
    /* WCDMA EUROPE 2600 band*/

  SYS_SBAND_WCDMA_VIII_900 = 49,
    /* WCDMA EUROPE & JAPAN 900 band */

  SYS_SBAND_WCDMA_IX_1700 = 50,
    /* WCDMA JAPAN 1700 band */

  /* 51-55 reserved for WLAN */
  SYS_SBAND_BC16 = 56,
   /* Band Class 16 */

  SYS_SBAND_PERSISTENT = 62,
    /* Persistent value from NV */

  SYS_SBAND_MAX = 63
    /* FOR INTERNAL USE OF CM ONLY! */
} sys_sband_e_type;
#endif /* !PLATFORM_LTK */


/** defining the ENUM as the values defined in CM are
       different from the ones used in QXDM **/
typedef enum qcril_cm_mode_pref_e {

  /** @cond  */
  QCRIL_CM_MODE_PREF_NONE=-1,   /* FOR INTERNAL CM USE ONLY! */
  /** @endcond */

  QCRIL_CM_MODE_PREF_AMPS_ONLY=0,
    /**< = NV_MODE_ANALOG_ONLY, service is limited to analog only. */

  QCRIL_CM_MODE_PREF_DIGITAL_ONLY=1,
    /**< = NV_MODE_DIGITAL_ONLY, service is limited to digital
         (CDMA, HDR, GW) only. */

  QCRIL_CM_MODE_PREF_AUTOMATIC=2,
    /**< = NV_MODE_AUTOMATIC, determine the mode automatically. */

/** @cond */
  QCRIL_CM_MODE_PREF_EMERGENCY=3,
    /* = NV_MODE_EMERGENCY, Emergency mode.\ This is intended to be used internally
         by the CM. The client is not allowed to use it as a parameter to change a
         system selection preference command. */
/** @endcond */

  /* For compatibility with QPST, do not change values or
     order. We start with NV_MODE_CELL_CDMA_ONLY+1 (i.e. 9). */

  QCRIL_CM_MODE_PREF_CDMA_ONLY = 9,
    /**< Service is limited to CDMA only. */

  QCRIL_CM_MODE_PREF_HDR_ONLY=10,
    /**< Service is limited to HDR only. */

  QCRIL_CM_MODE_PREF_CDMA_AMPS_ONLY=11,
    /**< Service is limited to CDMA and AMPS only. */

  QCRIL_CM_MODE_PREF_GPS_ONLY=12,
    /**< Service is limited to GPS only. */

  /* The following enums are the radio access technologies for
     WCDMA and GSM. */

  QCRIL_CM_MODE_PREF_GSM_ONLY=13,
    /**< Service is limited to GSM only. */

  QCRIL_CM_MODE_PREF_WCDMA_ONLY=14,
    /**< Service is limited to WCMDA only. */

  QCRIL_CM_MODE_PREF_WLAN_ONLY=15,
    /**< Acquire WLAN systems only. */
  /* Others */

  QCRIL_CM_MODE_PREF_ANY_BUT_HDR=16,
    /**< Use any service but HDR. */

  QCRIL_CM_MODE_PREF_GSM_WCDMA_ONLY=17,
    /**< Service is limited to GSM and WCDMA only. */

  QCRIL_CM_MODE_PREF_DIGITAL_LESS_HDR_ONLY=18,
    /**< Acquire digital, non-HDR mode systems only (CDMA, GSM, or WCDMA). */

  QCRIL_CM_MODE_PREF_CDMA_HDR_ONLY=19,
    /**< Acquire CDMA or HDR systems only. */

  QCRIL_CM_MODE_PREF_CDMA_AMPS_HDR_ONLY=20,
    /**< Acquire CDMA or AMPS or HDR systems only. */

  QCRIL_CM_MODE_PREF_CDMA_WLAN=21,
    /**< Acquire CDMA and WLAN systems only. */

  QCRIL_CM_MODE_PREF_HDR_WLAN=22,
    /**< Acquire HDR and WLAN systems only. */

  QCRIL_CM_MODE_PREF_CDMA_HDR_WLAN=23,
    /**< Acquire CDMA, HDR, and WLAN systems only. */

  QCRIL_CM_MODE_PREF_GSM_WLAN=24,
    /**< Acquire GSM and WLAN systems only. */

  QCRIL_CM_MODE_PREF_WCDMA_WLAN=25,
    /**< Acquire WCDMA and WLAN systems only. */

  QCRIL_CM_MODE_PREF_GW_WLAN=26,
    /**< Acquire GSM/WCDMA and WLAN systems only. */

  QCRIL_CM_MODE_PREF_CDMA_AMPS_HDR_WLAN_ONLY = 27,
    /**< Acquire CDMA, AMPS, HDR, and WLAN systems. */

  QCRIL_CM_MODE_PREF_CDMA_AMPS_WLAN_ONLY = 28,
    /**< Acquire CDMA, AMPS, and WLAN systems. */

  QCRIL_CM_MODE_PREF_ANY_BUT_HDR_WLAN = 29,
    /**< Use any service except HDR and WLAN. */

    /**< @internal */

  QCRIL_CM_MODE_PREF_MAX
    /* FOR INTERNAL USE OF CM ONLY!
    */
} qcril_cm_mode_pref_e_type;
/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

cm_rtre_config_e_type qcril_cm_util_map_nv_to_cm_rtre( uint32 nv_rtre );

void qcril_cm_util_rssi_to_gw_signal_strength( uint16 rssi, int *signal_strength_ptr );

void qcril_cm_util_srv_sys_info_to_reg_state( boolean reporting_data_reg_state, qcril_arb_pref_data_tech_e_type pref_data_tech,
                                              char **reg_state_ptr, const qcril_cm_ss_info_type *ssi_ptr, cm_mode_pref_e_type mode_pref,
                                              qcril_cm_reg_reject_info_type *reg_reject_info_ptr );

void qcril_cm_util_srv_sys_info_to_avail_radio_tech( boolean reporting_data_reg_state, cm_mode_pref_e_type mode_pref,
                                                     char **reg_state_ptr, char **avail_radio_tech_ptr, const qcril_cm_ss_info_type *ssi_ptr );

void qcril_cm_util_srv_sys_info_to_gw_sys_info( char **reg_state_ptr, char **lac_ptr, char **cid_ptr, char **psc_ptr,
                                                char *buf_lac_ptr, char *buf_cid_ptr, char *buf_psc_ptr, const qcril_cm_ss_info_type *ssi_ptr );

void qcril_cm_util_srv_sys_info_to_1xevdo_sys_info( qcril_cm_registration_state_type *payload_ptr, 
                                                    const qcril_cm_ss_info_type *ssi_ptr,
                                                    cm_mode_pref_e_type mode_pref );

void qcril_cm_util_srv_sys_info_to_rej_cause( char **reg_state_ptr, char **rej_cause_ptr, 
                                              qcril_cm_reg_reject_info_type *reg_reject_info_ptr, 
                                              const qcril_cm_ss_info_type *ssi_ptr,
                                             boolean reporting_data_reg_state);

const char *qcril_cm_util_lookup_reg_status( char *reg_state_ptr );

const char *qcril_cm_util_lookup_radio_tech( char *radio_tech_ptr );

void qcril_cm_util_process_cnap_info( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id,
                                      const cm_mm_call_info_s_type *call_info_ptr, boolean *unsol_call_state_changed_ptr );

int qcril_cm_util_convert_2s_complement_to_int(byte);

IxErrnoType qcril_cm_util_is_emer_number(char *flash_num);

byte qcril_cm_util_ussd_pack(byte *packed_data, const byte *str, byte num_chars);

byte qcril_cm_util_ussd_unpack(byte *str, const byte *packed_data, byte num_bytes);

void qcril_cm_util_bcd_to_ascii(const byte *bcd_number, byte * ascii_number);

void qcril_cm_util_number_to_bcd(const cm_num_s_type *number, byte *bcd_number);

byte qcril_cm_util_ascii_to_gsm_alphabet(byte *gsm_alphabet_string, const byte *ascii_string, byte num_chars);

byte qcril_cm_util_gsm_alphabet_to_ascii(byte * ascii_string, const byte *gsm_alphabet_string, byte num_bytes);

boolean  qcril_cm_util_subs_mode_pref( qcril_cm_mode_pref_e_type  modem_mode_pref, qcril_subs_mode_pref *mode_pref);
#endif /* QCRIL_CM_UTIL_H */
