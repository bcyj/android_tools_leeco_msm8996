/******************************************************************************
  @file    qcril_cm_util.h
  @brief   qcril qmi - compatibility layer for CM

  DESCRIPTION
    Utilities functions to support QCRIL_CM processing.

  ---------------------------------------------------------------------------

  Copyright (c) 2009-2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef QCRIL_CM_UTIL_H
#define QCRIL_CM_UTIL_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "comdef.h"
#include "qcril_arb.h"


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
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x28,0x2F,0x29,0x2E,0x2D,
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


/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

void qcril_cm_util_rssi_to_gw_signal_strength( uint16 rssi, int *signal_strength_ptr );


int qcril_cm_util_convert_2s_complement_to_int(byte);

byte qcril_cm_util_ussd_pack(byte *packed_data, const byte *str, byte num_chars);

byte qcril_cm_util_ussd_unpack(byte *str, const byte *packed_data, byte num_bytes);

void qcril_cm_util_bcd_to_ascii(const byte *bcd_number, byte * ascii_number);

byte qcril_cm_util_ascii_to_gsm_alphabet(byte *gsm_alphabet_string, const byte *ascii_string, byte num_chars);

byte qcril_cm_util_gsm_alphabet_to_ascii(byte * ascii_string, const byte *gsm_alphabet_string, byte num_bytes);

#endif /* QCRIL_CM_UTIL_H */
