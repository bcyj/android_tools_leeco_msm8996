/*!
  @file
  qcril_cm_ss.c

  @brief
  Contains information related to supplimentary services.

*/

/*===========================================================================

  Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc. All Rights Reserved

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

/*
 * Copyright 2001-2004 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.


when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/16/09   fc      Fix the incorrect service class checking for BAOC and CFW.
05/14/09   nrn     Adding fix for resolving the call forwarding issue for
                   allbearerservices
04/05/09   fc      Cleanup log macros.
03/16/09   pg      Mainlined FEATURE_CM_UTIL_RPC_AVAIL.
02/04/09   pg      Call cmutil RPC APIs when FEATURE_CM_UTIL_RPC_AVAIL is
                   defined.
7/24/2008  ranganat Initial Framework for Supplementary services

===========================================================================*/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <string.h>       
#include "IxErrno.h"
#include "comdef.h"
#include "cm.h"
#include "qcril_cmi.h"
#include "qcril_cm_ss.h"  
#include "qcril_cm_util.h"  


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/
byte qcril_cm_ss_ref; 

static const int halfShift  = 10; /* used for shifting by 10 bits */

static const UTF32 halfBase = 0x0010000UL;
static const UTF32 halfMask = 0x3FFUL;


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/* Mapping of unescaped GSM 7 bit characters to ANSI Codepage Unicode */
static const uint16 gsm_def_alpha_to_utf8_table[] =
{
  /* DEC   0       1       2       3       4       5       6       7
     HEX 0X00    0X01    0X02    0X03    0X04    0X05    0X06    0X07
          @       £       $     Yen      e\       e/      u\      i\.         */
      0x0040,    0xC2A3, 0x0024, 0xC2A5, 0xC3A8, 0xC3A9, 0xC3B9, 0xC3AC,

  /* DEC  8       9       10      11      12      13      14      15
     HEX 0X08    0X09    0X0A    0X0B    0X0C    0X0D    0X0E    0X0F
          o\     C,      LF      O|     o|      CR       A0      a0           */
      0xC3B2,    0xC387, 0x000A, 0xC398, 0xC3B8, 0x000D, 0xC385, 0xC3A5,

  /* DEC  16      17      18      19      20      21      22      23
     HEX 0X10    0X11    0X12    0X13    0X14    0X15    0X16    0X17
         Delta    _     ... Greek ...                                         */
      0xCE94,    0x005F, 0xCEA6, 0xCE93, 0xCE9B, 0xCEA9, 0xCEA0, 0xCEA8,

  /* DEC  24      25      26      27      28      29      30      31
     HEX 0X18    0X19    0X1A    0X1B    0X1C    0X1D    0X1E    0X1F
         ... Greek ...           Esc     AE      ae    GrossS     E/          */
      0xCEA3,    0xCE98, 0xCE9E, 0x0020, 0xC386, 0xC3A6, 0xC39F, 0xC389,

  /* DEC  32      33      34      35      36      37      38      39
     HEX 0X20    0X21    0X22    0X23    0X24    0X25    0X26    0X27
          SPC      !       "       #      OX       %       &       '         */
      0x0020,    0x0021, 0x0022, 0x0023, 0xC2A4, 0x0025, 0x0026, 0x0027,

  /* DEC  40      41      42      43      44      45      46      47
     HEX 0X28    0X29    0X2A    0X2B    0X2C    0X2D    0X2E    0X2F
          (       )       *       +       ,       -       .       /          */
  0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,

  /* DEC  48      49      50      51      52      53      54      55
     HEX 0X30    0X31    0X32    0X33    0X34    0X35    0X36    0X37       */
  0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,

  /* DEC  56      57      58      59      60      61      62      63
     HEX 0X38    0X39    0X3A    0X3B    0X3C    0X3D    0X3E    0X3F
          8       9       :       ;       <       =       >       ?         */
  0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,

  /* DEC  64      65      66      67      68      69      70      71
     HEX 0X40    0X41    0X42    0X43    0X44    0X45    0X46    0X47       
          .|      A       B       C       D       E       F       G         */
      0xC2A1,    0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,

  /* DEC  72      73      74      75      76      77      78      79
     HEX 0X48    0X49    0X4A    0X0X4B  0X4C    0X4D    0X4E    0X4F       */
  0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,

  /* DEC  80      81      82      83      84      85      86      87
     HEX 0X50    0X51    0X52    0X53    0X54    0X55    0X56    0X57       */
  0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,

  /* DEC  88      89      90      91      92      93      94      95
     HEX 0X58    0X59    0X5A    0X5B    0X5C    0X5D    0X5E    0X5F
          X       Y       Z       A"      0"      N~      U"     S\S       */
      0x0058,    0x0059, 0x005A, 0xC384, 0xC396, 0xC391, 0xC39C, 0xC2A7,

  /* DEC  96      97      98      99      100     101     102     103
     HEX 0X60    0X61    0X62    0X63    0X64    0X65    0X66    0X67
         ';      a       b       c       d       e       f       g         */
      0xC2BF,    0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,

  /* DEC  104     105     106     107     108     109     110     111
     HEX 0X68    0X69    0X6A    0X6B    0X6C    0X6D    0X6E    0X6F      */
  0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,

  /* DEC  112     113     114     115     116     117     118     119
     HEX 0X70    0X71    0X72    0X73    0X74    0X75    0X76    0X77     */
  0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,

  /* DEC  120     121     122     123     124     125     126     127
     HEX 0X78    0X79    0X7A    0X7B    0X7C    0X7D    0X7E    0X7F
          x       y       z       a"      o"      n~      u"      a      */
      0x0078,    0x0079, 0x007A, 0xC3A4, 0xC3B6, 0xC3B1, 0xC3BC, 0xC3A0
};


/*===========================================================================

                         GLOBAL VARIABLES

===========================================================================*/

/* mapping table for mapping service class to bearer type and code */
qcril_cm_ss_bs_mapping_s_type qcril_cm_ss_bs_mapping_table[] =
{
  { QCRIL_CM_SS_CLASS_ALL,                 (uint8) BS_CODE,   qcril_cm_ss_allBearerServices},
  { QCRIL_CM_SS_CLASS_DATA_ALL,            (uint8) BS_CODE,   qcril_cm_ss_allBearerServices},
  { QCRIL_CM_SS_CLASS_ALL_DATA_ASYNC,      (uint8) BS_CODE,   qcril_cm_ss_allAsynchronousServices },
  { QCRIL_CM_SS_CLASS_ALL_DATA_SYNC,       (uint8) BS_CODE,   qcril_cm_ss_allSynchronousServices },
  { QCRIL_CM_SS_CLASS_DATA_PKT,            (uint8) BS_CODE,   qcril_cm_ss_allDataPDS_Services},
  { QCRIL_CM_SS_CLASS_ALL_DATA_SYNC_ASYNC, (uint8) BS_CODE,   qcril_cm_ss_allBearerServices},
  { QCRIL_CM_SS_CLASS_DATA_ASYNC,          (uint8) BS_CODE,   qcril_cm_ss_allDataCircuitAsynchronous},
  { QCRIL_CM_SS_CLASS_DATA_SYNC,           (uint8) BS_CODE,   qcril_cm_ss_allDataCircuitSynchronous},
  { QCRIL_CM_SS_ALL_TELE_SERV,             (uint8) TS_CODE,   qcril_cm_ss_allTeleservices},
  { QCRIL_CM_SS_CLASS_ALL_TS_DATA,         (uint8) TS_CODE,   qcril_cm_ss_allDataTeleservices},
  { QCRIL_CM_SS_CLASS_SMS,                 (uint8) TS_CODE,   qcril_cm_ss_allShortMessageServices},
  { QCRIL_CM_SS_ALL_TELE_SERV_EX_SMS,      (uint8) TS_CODE,   qcril_cm_ss_allTeleservices_ExeptSMS},
  { QCRIL_CM_SS_CLASS_FAX,                 (uint8) TS_CODE,   qcril_cm_ss_allFacsimileTransmissionServices},
  { QCRIL_CM_SS_CLASS_DATA,                (uint8) TS_CODE,   qcril_cm_ss_allBearerServices},
  { QCRIL_CM_SS_CLASS_VOICE,               (uint8) TS_CODE,   qcril_cm_ss_allSpeechTransmissionservices},
  { QCRIL_CM_SS_CLASS_DATA_PAD,            (uint8) BS_CODE,   qcril_cm_ss_allPadAccessCA_Services},
  { QCRIL_CM_SS_CLASS_MAX,                 (uint8) MAX_CODE,  0xFF }
};

/* See Spec 3GPP 29.002,Section 17.7.10. */
qcril_cm_ss_bs_mapping_s_type qcril_cm_ss_extra_bs_mapping_table[] =
{
  { QCRIL_CM_SS_CLASS_VOICE,               (uint8) TS_CODE,    qcril_cm_ss_telephony},
  { QCRIL_CM_SS_CLASS_VOICE,               (uint8) TS_CODE,   qcril_cm_ss_emergencyCalls},
  { QCRIL_CM_SS_CLASS_FAX,                 (uint8) TS_CODE,   qcril_cm_ss_facsimileGroup3AndAlterSpeech},
  { QCRIL_CM_SS_CLASS_FAX,                 (uint8) TS_CODE,   qcril_cm_ss_automaticFacsimileGroup3},
  { QCRIL_CM_SS_CLASS_FAX,                 (uint8) TS_CODE,   qcril_cm_ss_facsimileGroup4},
  { QCRIL_CM_SS_CLASS_SMS,                 (uint8) TS_CODE,   qcril_cm_ss_shortMessageMT_PP},
  { QCRIL_CM_SS_CLASS_SMS,                 (uint8) TS_CODE,   qcril_cm_ss_shortMessageMO_PP},
  { QCRIL_CM_SS_CLASS_DATA_SYNC,           (uint8) BS_CODE,   qcril_cm_ss_allDataCDS_Services},
  { QCRIL_CM_SS_CLASS_DATA_SYNC,           (uint8) BS_CODE,   qcril_cm_ss_dataCDS_1200bps},
  { QCRIL_CM_SS_CLASS_DATA_SYNC,           (uint8) BS_CODE,   qcril_cm_ss_dataCDS_2400bps},
  { QCRIL_CM_SS_CLASS_DATA_SYNC,           (uint8) BS_CODE,   qcril_cm_ss_dataCDS_4800bps},
  { QCRIL_CM_SS_CLASS_DATA_SYNC,           (uint8) BS_CODE,   qcril_cm_ss_dataCDS_9600bps},
  { QCRIL_CM_SS_CLASS_DATA_SYNC,           (uint8) BS_CODE,   qcril_cm_ss_allAlternateSpeech_DataCDS},
  { QCRIL_CM_SS_CLASS_DATA_SYNC,           (uint8) BS_CODE,   qcril_cm_ss_allSpeechFollowedByDataCDS},
  { QCRIL_CM_SS_CLASS_DATA_ASYNC,          (uint8) BS_CODE,   qcril_cm_ss_allDataCDA_Services},
  { QCRIL_CM_SS_CLASS_DATA_ASYNC,          (uint8) BS_CODE,   qcril_cm_ss_dataCDA_300bps},
  { QCRIL_CM_SS_CLASS_DATA_ASYNC,          (uint8) BS_CODE,   qcril_cm_ss_dataCDA_1200bps},
  { QCRIL_CM_SS_CLASS_DATA_ASYNC,          (uint8) BS_CODE,   qcril_cm_ss_dataCDA_1200_75bps},
  { QCRIL_CM_SS_CLASS_DATA_ASYNC,          (uint8) BS_CODE,   qcril_cm_ss_dataCDA_2400bps},
  { QCRIL_CM_SS_CLASS_DATA_ASYNC,          (uint8) BS_CODE,   qcril_cm_ss_dataCDA_4800bps},
  { QCRIL_CM_SS_CLASS_DATA_ASYNC,          (uint8) BS_CODE,   qcril_cm_ss_dataCDA_9600bps},
  { QCRIL_CM_SS_CLASS_DATA_ASYNC,          (uint8) BS_CODE,   qcril_cm_ss_allAlternateSpeech_DataCDA},
  { QCRIL_CM_SS_CLASS_DATA_ASYNC,          (uint8) BS_CODE,   qcril_cm_ss_allSpeechFollowedByDataCDA},
  { QCRIL_CM_SS_CLASS_DATA_PKT,            (uint8) BS_CODE,   qcril_cm_ss_dataPDS_2400bps},
  { QCRIL_CM_SS_CLASS_DATA_PKT,            (uint8) BS_CODE,   qcril_cm_ss_dataPDS_4800bps},
  { QCRIL_CM_SS_CLASS_DATA_PKT,            (uint8) BS_CODE,   qcril_cm_ss_dataPDS_9600bps},
  { QCRIL_CM_SS_CLASS_DATA_PAD,            (uint8) BS_CODE,   qcril_cm_ss_padAccessCA_300bps},
  { QCRIL_CM_SS_CLASS_DATA_PAD,            (uint8) BS_CODE,   qcril_cm_ss_padAccessCA_1200bps},
  { QCRIL_CM_SS_CLASS_DATA_PAD,            (uint8) BS_CODE,   qcril_cm_ss_padAccessCA_1200_75bps},
  { QCRIL_CM_SS_CLASS_DATA_PAD,            (uint8) BS_CODE,   qcril_cm_ss_padAccessCA_2400bps},
  { QCRIL_CM_SS_CLASS_DATA_PAD,            (uint8) BS_CODE,   qcril_cm_ss_padAccessCA_4800bps},
  { QCRIL_CM_SS_CLASS_DATA_PAD,            (uint8) BS_CODE,   qcril_cm_ss_padAccessCA_9600bps},
  { QCRIL_CM_SS_CLASS_MAX,                 (uint8) MAX_CODE,  0xFF }
};

/*MMI to Network value Mapping*/
const qcril_cm_ss_sc_table_s_type qcril_cm_ss_sc_conversion_table [] =
{
  { "",        qcril_cm_ss_all },
  { "002",     qcril_cm_ss_allForwardingSS },
  { "004",     qcril_cm_ss_allCondForwardingSS },
  { "21",      qcril_cm_ss_cfu },
  { "67",      qcril_cm_ss_cfb },
  { "61",      qcril_cm_ss_cfnry },
  { "62",      qcril_cm_ss_cfnrc },
  { "30",      qcril_cm_ss_clip },
  { "31",      qcril_cm_ss_clir },
  { "76",      qcril_cm_ss_colp },
  { "77",      qcril_cm_ss_colr },
  { "43",      qcril_cm_ss_cw },
  { "330",     qcril_cm_ss_allCallRestrictionSS },
  { "333",     qcril_cm_ss_barringOfOutgoingCalls },
  { "353",     qcril_cm_ss_barringOfIncomingCalls },
  { "33",      qcril_cm_ss_baoc },
  { "331",     qcril_cm_ss_boic },
  { "332",     qcril_cm_ss_boicExHC },
  { "35",      qcril_cm_ss_baic },
  { "351",     qcril_cm_ss_bicRoam },
  { "300",     qcril_cm_ss_cnap },
  { "37",      qcril_cm_ss_ccbs }
};

#define QCRIL_CM_SS_MAX_SC_ENTRY   ( sizeof( qcril_cm_ss_sc_conversion_table ) / sizeof( qcril_cm_ss_sc_table_s_type ) )

/* MMI to Service class Mapping*/
qcril_cm_ss_bsg_table_s_type qcril_cm_ss_bsg_conversion_table[] =
{
  { "11",        QCRIL_CM_SS_CLASS_VOICE },
  { "16",        QCRIL_CM_SS_CLASS_SMS },
  { "13",        QCRIL_CM_SS_CLASS_FAX },
  { "10",        QCRIL_CM_SS_ALL_TELE_SERV },
  { "12",        QCRIL_CM_SS_CLASS_ALL_TS_DATA },
  { "19",        QCRIL_CM_SS_ALL_TELE_SERV_EX_SMS },
  { "20",        QCRIL_CM_SS_CLASS_ALL_DATA_SYNC_ASYNC },
  { "21",        QCRIL_CM_SS_CLASS_ALL_DATA_ASYNC },
  { "22",        QCRIL_CM_SS_CLASS_ALL_DATA_SYNC },
  { "24",        QCRIL_CM_SS_CLASS_DATA_SYNC },
  { "25",        QCRIL_CM_SS_CLASS_DATA_ASYNC },
  { "26",        QCRIL_CM_SS_CLASS_ALL_DATA_PDS }
};

#define QCRIL_CM_SS_MAX_BSG_ENTRY (sizeof(qcril_cm_ss_bsg_conversion_table)/sizeof(qcril_cm_ss_bsg_table_s_type))

/* Sups mode type string to Mode mapping */
qcril_cm_ss_mode_table_s_type qcril_cm_ss_mode_input[] =
{
  { "**",       QCRIL_CM_SS_MODE_REG },
  { "*#",       QCRIL_CM_SS_MODE_QUERY },
  { "*",        QCRIL_CM_SS_MODE_ENABLE },
  { "##",       QCRIL_CM_SS_MODE_ERASURE },
  { "#",        QCRIL_CM_SS_MODE_DISABLE },
  { "**03*",    QCRIL_CM_SS_MODE_REG_PASSWD },
  { NULL,       QCRIL_CM_SS_MODE_MAX }
};

/* Possible class values for corresponding supplementary services 
Per 22.004 Table A.1 the incoming, waiting call can be of any kind */                                              
uint32 qcril_cm_ss_cfw_allowed_classes = (uint32)( QCRIL_CM_SS_CLASS_VOICE |
                                                   QCRIL_CM_SS_CLASS_DATA | 
                                                   QCRIL_CM_SS_CLASS_FAX | 
                                                   QCRIL_CM_SS_CLASS_DATA_SYNC | 
                                                   QCRIL_CM_SS_CLASS_DATA_PAD |
                                                   QCRIL_CM_SS_CLASS_DATA_ASYNC |
                                                   QCRIL_CM_SS_CLASS_DATA_PKT );

uint32 qcril_cm_ss_cb_allowed_classes =  (uint32) QCRIL_CM_SS_CLASS_ALL;

uint32 qcril_cm_ss_cw_allowed_classes =  (uint32) QCRIL_CM_SS_CLASS_ALL;

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
static const char trailingBytesForUTF8[ 256 ] = 
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
static const UTF32 offsetsFromUTF8[ 6 ] = 
{ 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
static const UTF8 firstByteMark[ 7 ] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };


/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_cm_ss_get_ss_ref

===========================================================================*/
/*!
    @brief
    Assigning reference to supplimentary services for identifying it uniquely

    @return
    retuns SS reference value
*/
/*=========================================================================*/
uint8 qcril_cm_ss_get_ss_ref
(
  void
)
{
  qcril_cm_ss_ref++;

  return qcril_cm_ss_ref;

} /* qcril_cm_ss_get_ss_ref */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_req_set_call_wait_is_valid

===========================================================================*/
/*!
    @brief
    To check whether RIL_REQUEST_SET_CALL_WAITING is received with valid 
    parameters

    @return
    returns TRUE is parameters are valid else FALSE
*/
/*=========================================================================*/
boolean qcril_cm_ss_req_set_call_wait_is_valid
(
  unsigned int length, 
  int status, 
  int service_class
)
{

 if ( ( length == 0 ) || !( ( status == (int) QCRIL_CM_SS_MODE_DISABLE ) || 
      ( status == (int) QCRIL_CM_SS_MODE_ENABLE ) ) || 
      ( qcril_cm_ss_cw_allowed_classes != ((uint32) service_class | qcril_cm_ss_cw_allowed_classes ) ) )
 {
   return FALSE;
 }
 else
   return TRUE;    

} /* qcril_cm_ss_req_set_call_wait_is_valid */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_req_set_call_fwd_is_valid

===========================================================================*/
/*!
    @brief
    To check whether RIL_REQUEST_SET_CALL_FORWARD is received with valid 
    parameters

    @return
    Returns TRUE or FALSE based on whether received parameters 
    are valid or not
*/
/*=========================================================================*/
boolean qcril_cm_ss_req_set_call_fwd_is_valid
(
  unsigned int in_data_len, 
  qcril_cm_ss_callforwd_info_param_u_type *in_data_ptr
)
{
  int status = in_data_ptr->status;
  char *digit;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( in_data_ptr != NULL );

  /*-----------------------------------------------------------------------*/
  
  if ( in_data_len == 0 )
    return FALSE;

  if ( !( ( status >= (int) QCRIL_CM_SS_MODE_DISABLE ) && 
       ( status < (int) QCRIL_CM_SS_MODE_MAX ) && ( status != (int) QCRIL_CM_SS_MODE_QUERY ) ) )
    return FALSE;

  if ( !( ( in_data_ptr->reason >= (int) QCRIL_CM_SS_CCFC_REASON_UNCOND ) && 
       ( in_data_ptr->reason <  (int) QCRIL_CM_SS_CCFC_REASON_MAX ) ) )
    return FALSE ;    
      
  if ( qcril_cm_ss_cfw_allowed_classes != ( (uint32) in_data_ptr->service_class | qcril_cm_ss_cfw_allowed_classes ) )
  {
    /* since service class is optional for all call forwarding ignoring it if incorrect value is received */
    QCRIL_LOG_ERROR( "%s", "invalid service class received\n");
  } 

  if ( in_data_ptr->number != NULL )
  {
    /* max number of charecters is 80, 24.008 10.5.4.7 */
    if ( QCRIL_CM_SS_CCFC_NUM_MAX_LEN < strlen( in_data_ptr->number ) )
      return FALSE;
  
    /* verifying whether digits are valid or not */
    
    digit = in_data_ptr->number;
    /* cm_num_fill does nothing to verify number */
    if ( '+' == *digit )
    {
      digit++;
    }
    /* per GSM 04.08, table 10.81 */
    while ( *digit )
    {
      if ( ( ( *digit < '0' ) || ( *digit > '9' ) ) &&
           ( *digit != '*' ) && ( *digit != '#' ) && 
           ( *digit != 'a' ) && ( *digit != 'b' ) && ( *digit != 'c' ) )
      {
        return FALSE;
      }       

      digit++;
    }
  }
  else
  {
    QCRIL_LOG_DEBUG( "%s", "Received CF Number as NULL\n");
  }
    
  if(in_data_ptr->reason == (int)QCRIL_CM_SS_CCFC_REASON_NOREPLY)
  {
    if ( in_data_ptr->no_reply_timer != 0 )
    {
      if ( !( ( in_data_ptr->no_reply_timer >= 5 ) && ( in_data_ptr->no_reply_timer <= 30 ) ) )
      {
        QCRIL_LOG_ERROR( "invalid no_reply_timer value received = %d \n", in_data_ptr->no_reply_timer );
        return FALSE;
      }
    }
  }

  return TRUE;
  
} /* qcril_cm_ss_req_set_call_fwd_is_valid */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_req_set_fac_lck_is_valid

===========================================================================*/
/*!
    @brief
    To check whether RIL_REQUEST_SET_FACILITY_LOCK is received with 
    valid parameters

    @return
    Returns TRUE or FALSE based on whether received parameters are 
    valid or not
*/
/*=========================================================================*/
boolean qcril_cm_ss_req_set_fac_lck_is_valid
(
  unsigned int in_data_len, 
  int facility, 
  int status,  
  const char *password, 
  int service_class
)
{
  if ( in_data_len == 0 )
    return FALSE;

  if ( !( ( status == 0 ) || ( status == 1 ) ) )
    return FALSE;
 
  if (!qcril_cm_ss_facility_value_is_valid( facility, status ) )
  { 
    return FALSE; 
  }  

  if ( qcril_cm_ss_cb_allowed_classes != ( (uint32) service_class | qcril_cm_ss_cb_allowed_classes ) )
    return FALSE;

  /* removing the check here as password is optional, if NW asks for password
     then reject the activation request for call barring */
  /* if( (password == NULL) || (strlen(password) > MAX_PWD_CHAR))
     return(FALSE);*/

  return TRUE;

} /* qcril_cm_ss_req_set_fac_lck_is_valid */


/*=========================================================================
  FUNCTION:  qcril_req_change_cb_pwd_is_valid

===========================================================================*/
/*!
    @brief
    To check whether RIL_REQUEST_CHANGE_BARRING_PASSWORD is received with 
    valid parameters

    @return
    Returns TRUE or FALSE based on whether received parameters are 
    valid or not
*/
/*=========================================================================*/
boolean qcril_cm_ss_req_change_cb_pwd_is_valid
(
  unsigned int in_data_len, 
  int facility, 
  const char *old_password, 
  const char *new_password
)
{
  if ( in_data_len == 0 )
    return FALSE;

  if ( !( qcril_cm_ss_facility_value_is_valid( facility, FALSE ) ||
       ( qcril_cm_ss_facility_value_is_valid( facility, TRUE ) &&
       ( facility != (int) QCRIL_CM_SS_LOCK_FD ) && ( facility != (int) QCRIL_CM_SS_LOCK_SC ) ) ) )
  {
    return FALSE;
  }
 
  if ( ( old_password == NULL ) || ( strlen( old_password ) > MAX_PWD_CHAR ) )
    return FALSE;

  if ( ( new_password == NULL ) || ( strlen( new_password ) > MAX_PWD_CHAR ) )
    return FALSE;

 return TRUE;
 
} /* qcril_cm_ss_req_change_cb_pwd_is_valid */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_query_facility_lock_is_valid

===========================================================================*/
/*!
    @brief
    To check whether RIL_REQUEST_QUERY_FACILITY_LOCK is received with
    valid parameters

    @return
    Returns TRUE or FALSE based on whether received parameters are 
    valid or not
*/
/*=========================================================================*/
boolean qcril_cm_ss_query_facility_lock_is_valid
(
  unsigned int in_data_len, 
  int facility, 
  const char *password, 
  int service_class
)
{
  if ( in_data_len == 0 )
    return FALSE;

  if ( !( qcril_cm_ss_facility_value_is_valid( facility, TRUE ) ||
       qcril_cm_ss_facility_value_is_valid( facility, FALSE ) ) )
    return FALSE;

  /* removing the check here as password is optional, if NW asks for password
      then reject the activation request for call barring */ 
  if ( ( password == NULL ) || ( strlen(password) > MAX_PWD_CHAR ) )
    QCRIL_LOG_ERROR( "%s", "Password is NULL or length is > MAX_PWD_CHAR\n" );

  /* ignoring the service class with invalid value as it is optional */
  if ( qcril_cm_ss_cb_allowed_classes != ( (uint32) service_class | qcril_cm_ss_cb_allowed_classes ) )
    QCRIL_LOG_ERROR( "%s", "Invalid service class received\n" );
  
  return TRUE;
 
} /* qcril_cm_ss_query_facility_lock_is_valid */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_get_cfw_ss_code

===========================================================================*/
/*!
    @brief
    Returns the appropriate supplimentary services code for call forwarding
    based on the reason value received from ANDROID RIL which is as per
    spec 27.007 section 7.11

    @return
    Returns SS code for call forwarding
*/
/*=========================================================================*/
uint8 qcril_cm_ss_get_cfw_ss_code
(
  int reason, 
  char **cf_reason_name
)
{
  qcril_cm_ss_operation_code_e_type cfw_ss_code = qcril_cm_ss_all;
  
  switch ( reason )
  {
    case QCRIL_CM_SS_CCFC_REASON_UNCOND:
      cfw_ss_code = qcril_cm_ss_cfu;
      *cf_reason_name = "CFU";
      break;
 
    case QCRIL_CM_SS_CCFC_REASON_BUSY:
      cfw_ss_code = qcril_cm_ss_cfb;
      *cf_reason_name = "CFB";
      break;
 
    case QCRIL_CM_SS_CCFC_REASON_NOREPLY:
      cfw_ss_code = qcril_cm_ss_cfnry;
      *cf_reason_name = "CFNRY";
      break;
     
    case QCRIL_CM_SS_CCFC_REASON_NOTREACH:
      cfw_ss_code = qcril_cm_ss_cfnrc;
      *cf_reason_name = "CFNRC";
      break;
     
    case QCRIL_CM_SS_CCFC_REASON_ALLCALL:
      cfw_ss_code = qcril_cm_ss_allForwardingSS;
      *cf_reason_name = "ALLCALL";
      break;
     
    case QCRIL_CM_SS_CCFC_REASON_ALLCOND:
      cfw_ss_code = qcril_cm_ss_allCondForwardingSS;    
      *cf_reason_name = "ALLCOND";
      break;
 
    default:
        break;
        /*none as of now as the error values are already checked before */
     
  }

  return ( (uint8) cfw_ss_code );

} /* qcril_cm_ss_get_cfw_ss_code */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_get_cfw_reason

===========================================================================*/
/*!
    @brief
    Returns the appropriate call forwarding reason(27.007 7.11) based
    on the supplimentary services code

    @return
    Returns call forwarding reason 
*/
/*=========================================================================*/
int qcril_cm_ss_get_cfw_reason
(
  int ss_code
)
{
  qcril_cm_ss_ccfc_reason_e_type cfw_reason = QCRIL_CM_SS_CCFC_REASON_UNCOND;
  
  switch ( ss_code )
  {
    case qcril_cm_ss_cfu:
      cfw_reason = QCRIL_CM_SS_CCFC_REASON_UNCOND;
      break;
 
    case qcril_cm_ss_cfb:
      cfw_reason = QCRIL_CM_SS_CCFC_REASON_BUSY;
      break;
 
    case qcril_cm_ss_cfnry:
      cfw_reason = QCRIL_CM_SS_CCFC_REASON_NOREPLY;
      break;
     
    case qcril_cm_ss_cfnrc:
      cfw_reason = QCRIL_CM_SS_CCFC_REASON_NOTREACH;
      break;
     
    case qcril_cm_ss_allForwardingSS:
      cfw_reason = QCRIL_CM_SS_CCFC_REASON_ALLCALL;
      break;
     
    case qcril_cm_ss_allCondForwardingSS :
      cfw_reason = QCRIL_CM_SS_CCFC_REASON_ALLCOND;    
      break;
 
    default:
      break;
      /*none as of now as the error values are already checked before */
  
  }

  return ( (int) cfw_reason );

} /* qcril_cm_ss_get_cfw_reason */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_get_facility_value

===========================================================================*/
/*!
    @brief
    Converts the facility string to a numberical value

    @return
    Returns numerical value corresponding to facility string
*/
/*=========================================================================*/
int  qcril_cm_ss_get_facility_value
(
  const char * facility, 
  char* facility_name )
{
  char temp[ 3 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( facility != NULL );

  /*-----------------------------------------------------------------------*/
   
  if ( strlen( facility ) != sizeof( char ) * 2 )
    return ( (int) QCRIL_CM_SS_LOCK_MAX );

  memcpy( &temp[ 0 ], facility, sizeof( char ) * 2 );
  temp[ 2 ] = '\0';

  temp[ 0 ] = QCRIL_CM_SS_UPCASE( temp[ 0 ] );
  temp[ 1 ] = QCRIL_CM_SS_UPCASE( temp[ 1 ] );

  memcpy( facility_name, temp, 3 );

  if ( strcmp( temp, "SC" ) == FALSE )
   return ( (int) QCRIL_CM_SS_LOCK_SC );
  
  if ( strcmp( temp, "AO" ) == FALSE )
    return ( (int) QCRIL_CM_SS_LOCK_AO );
   
  if ( strcmp( temp, "OI" ) == FALSE )
    return ( (int) QCRIL_CM_SS_LOCK_OI );
   
  if ( strcmp( temp, "OX" ) == FALSE )
    return ( (int) QCRIL_CM_SS_LOCK_OX );

  if ( strcmp( temp, "AI") == FALSE )
    return ( (int) QCRIL_CM_SS_LOCK_AI );

  if ( strcmp( temp, "IR" ) == FALSE )
    return ( (int) QCRIL_CM_SS_LOCK_IR );

  if ( strcmp( temp, "AB" ) == FALSE )
    return( (int) QCRIL_CM_SS_LOCK_AB );
  
  if ( strcmp( temp, "AG" ) == FALSE )
    return ( (int) QCRIL_CM_SS_LOCK_AG );

  if ( strcmp( temp, "AC" ) == FALSE )
    return ( (int) QCRIL_CM_SS_LOCK_AC );

  if (strcmp( temp, "FD" ) == FALSE)
    return ( (int) QCRIL_CM_SS_LOCK_FD );  

  return ( (int) QCRIL_CM_SS_LOCK_MAX );

} /* qcril_cm_ss_get_facility_value */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_facility_value_is_valid

===========================================================================*/
/*!
    @brief
    Checks whether the facility value is valid as per mode or not

    @return
    TRUE if the facility value is valid
*/
/*=========================================================================*/
boolean qcril_cm_ss_facility_value_is_valid
(
  int facility, 
  int status
)
{

 if ( ( status == FALSE ) && ( ( (int) QCRIL_CM_SS_LOCK_AB <= facility && (int) QCRIL_CM_SS_LOCK_AC>= facility )||
                               ( (int) QCRIL_CM_SS_LOCK_SC <= facility && (int) QCRIL_CM_SS_LOCK_IR >= facility ) ||
                               ( (int) QCRIL_CM_SS_LOCK_FD == facility ) ) )
 {
   return TRUE;
 }
 else if ( (status == TRUE ) && ( ( (int) QCRIL_CM_SS_LOCK_SC <= facility && (int) QCRIL_CM_SS_LOCK_IR >= facility ) ||
                                  ( (int) QCRIL_CM_SS_LOCK_FD == facility ) ) )  
 {
   return TRUE ;
 }
 else
 {
   return FALSE ;
 }  

} /* qcril_cm_ss_facility_value_is_valid */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_add_service_class_based_on_number

===========================================================================*/
/*!
    @brief
    Adds the Service class based on the number provided

    @return
    Returns None, and updates the array and length.
*/
/*=========================================================================*/
void qcril_cm_ss_add_service_class_based_on_number
(
  qcril_cm_ss_callforwd_info_param_u_type *a[], 
  int *LEN
)
{
  qcril_cm_ss_callforwd_info_param_u_type arr[ 7 ];
  int l = 0;
  int arr_len = 0, orig_len = *LEN;
  int i = 0, j = 0;

  memset( arr, 0, 7 * sizeof( qcril_cm_ss_callforwd_info_param_u_type ) );
  for ( i = 0; i < ( *LEN ) ; i++ )
  {
    memcpy( &arr[ i ], a[ i ], sizeof( arr[ i ] ) );
  }

  for( i = 0; i < ( *LEN ) ; i++ )
  {
    for ( j = i + 1 ; j < ( *LEN ) ; j++ )
    {
      if ( ( arr[ i ].number != (char *) 0 ) && ( arr[ j ].number != (char *) 0 ) )
      {
        if ( memcmp( arr[ i ].number, arr[ j ].number, strlen( arr[ j ].number ) ) == 0 )
        {
          arr[ i ].service_class += arr[ j ].service_class;
          memset( &arr[ j ], 0, sizeof( arr[ j ] ) );
        }
      }
     }
   }   

  for ( i = 0; i < ( *LEN ) ; i++ )
  {
    if ( arr[ i ].status != 0 )
    {
      arr_len = arr_len + 1;
      memcpy( a[ l ], &arr[ i ], sizeof( qcril_cm_ss_callforwd_info_param_u_type ) );
      l++;
    }
  }

  *LEN = l;
  
  for( j = l; j < orig_len ; j++ )
  {
    a[ l ] = NULL;
    l++;
  }
      
} /* qcril_cm_ss_add_service_class_based_on_number */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_get_cb_ss_code

===========================================================================*/
/*!
    @brief
    Returns the appropriate supplimentary services code for call barring 
    supplimentary services based on the facility value received from ANDROID RIL 
    which is as per spec 27.007 section 7.11

    @return
    Returns SS code for call barring
*/
/*=========================================================================*/
int  qcril_cm_ss_get_cb_ss_code
(
  int facility
)
{
  qcril_cm_ss_operation_code_e_type cb_ss_code = qcril_cm_ss_all;
  
  switch ( facility )
  {
    case QCRIL_CM_SS_LOCK_AO:
      cb_ss_code = qcril_cm_ss_baoc;
      break;
 
    case QCRIL_CM_SS_LOCK_OI:
      cb_ss_code = qcril_cm_ss_boic;
      break;
 
    case QCRIL_CM_SS_LOCK_OX:
      cb_ss_code = qcril_cm_ss_boicExHC;
      break;
   
    case QCRIL_CM_SS_LOCK_AI:
      cb_ss_code = qcril_cm_ss_baic;
      break;
   
    case QCRIL_CM_SS_LOCK_IR:
      cb_ss_code = qcril_cm_ss_bicRoam;
      break;
   
    case QCRIL_CM_SS_LOCK_AB:
      cb_ss_code = qcril_cm_ss_allCallRestrictionSS;  
      break;

     case QCRIL_CM_SS_LOCK_AG:
      cb_ss_code = qcril_cm_ss_barringOfOutgoingCalls;
      break;
   
    case QCRIL_CM_SS_LOCK_AC:
      cb_ss_code = qcril_cm_ss_barringOfIncomingCalls;
      break;
      
    default:
      break;
      /*none as of now as the error values are already checked before */
  }

  return ( (int) cb_ss_code );

} /* qcril_cm_ss_get_cb_ss_code */

/*=========================================================================
  FUNCTION:  qcril_cm_ss_get_service_type

===========================================================================*/
/*!
    @brief
    finds the appropriate service type that matches the input

    INPUT:        ss code

    @return
    Returns service type
*/
/*=========================================================================*/
RIL_SsServiceType qcril_cm_ss_get_service_type
(
int ss_code
)
{
    RIL_SsServiceType ss_service_type;
    switch ( ss_code )
    {
         case qcril_cm_ss_cfu:
         ss_service_type = SS_CFU;
         break;

         case qcril_cm_ss_cfb:
         ss_service_type = SS_CF_BUSY;
         break;

         case qcril_cm_ss_cfnry:
         ss_service_type = SS_CF_NO_REPLY;
         break;

         case qcril_cm_ss_cfnrc:
         ss_service_type = SS_CF_NOT_REACHABLE;
         break;

         case qcril_cm_ss_allForwardingSS:
         ss_service_type = SS_CF_ALL;
         break;

         case qcril_cm_ss_allCondForwardingSS:
         ss_service_type = SS_CF_ALL_CONDITIONAL;
         break;

         case qcril_cm_ss_baoc:
         ss_service_type = SS_BAOC;
         break;

         case qcril_cm_ss_boic:
         ss_service_type = SS_BAOIC;
         break;

         case qcril_cm_ss_boicExHC:
         ss_service_type = SS_BAOIC_EXC_HOME;
         break;

         case qcril_cm_ss_baic:
         ss_service_type = SS_BAIC;
         break;

         case qcril_cm_ss_bicRoam:
         ss_service_type = SS_BAIC_ROAMING;
         break;

         case qcril_cm_ss_allCallRestrictionSS:
         ss_service_type = SS_ALL_BARRING;
         break;

         case qcril_cm_ss_barringOfOutgoingCalls:
         ss_service_type = SS_OUTGOING_BARRING;
         break;

         case qcril_cm_ss_barringOfIncomingCalls:
         ss_service_type = SS_INCOMING_BARRING;
         break;

         case qcril_cm_ss_cw:
         ss_service_type = SS_WAIT;
         break;

         case qcril_cm_ss_clip:
         ss_service_type = SS_CLIP;
         break;

         case qcril_cm_ss_clir:
         ss_service_type = SS_CLIR;
         break;

         default:
         break;
  }
  return ss_service_type;
}

/*=========================================================================
  FUNCTION:  qcril_cm_ss_get_service_class

===========================================================================*/
/*!
    @brief
    finds the appropriate service class that matches the input input 
    basic service group and type

    INPUT:        basic service type and basic service code   
    
    @return
    Returns service class
*/
/*=========================================================================*/
uint32 qcril_cm_ss_get_service_class
(
  bsg_code_type bsg_type, 
  int bsg_code
)
{
  qcril_cm_ss_class_e_type bs_class = QCRIL_CM_SS_CLASS_MAX;
  uint8 index = 0;

  /* Lookup for Class parameter */
  while ( QCRIL_CM_SS_CLASS_MAX != qcril_cm_ss_bs_mapping_table[index].bs_class )
  {
    if ((qcril_cm_ss_bs_mapping_table[ index ].bs_code == (uint8)bsg_code) &&
        (qcril_cm_ss_bs_mapping_table[ index ].bs_type == (uint8)bsg_type))
    {
      bs_class = qcril_cm_ss_bs_mapping_table[ index ].bs_class;
      break;
    }
    index++;
  }

  if ( QCRIL_CM_SS_CLASS_MAX == bs_class )
  {
    index = 0;

    /* Lookup BSG extra mapping table for Class parameter */
    while ( QCRIL_CM_SS_CLASS_MAX != qcril_cm_ss_extra_bs_mapping_table[ index ].bs_class )
    {
      if ( ( qcril_cm_ss_extra_bs_mapping_table[ index ].bs_code == (uint8) bsg_code ) &&
           ( qcril_cm_ss_extra_bs_mapping_table[ index ].bs_type == (uint8) bsg_type ) )
      {
        bs_class = qcril_cm_ss_extra_bs_mapping_table[ index ].bs_class;
        break;
      }
      index++;
    }
  }

  if ( QCRIL_CM_SS_CLASS_MAX == bs_class )
  {
    QCRIL_LOG_ERROR( "Could not decode BSG type,code: %d,%d \n", bsg_type, bsg_code );
    bs_class = QCRIL_CM_SS_CLASS_MIN;
  }
  else
  {
    QCRIL_LOG_DEBUG( "BSG type=%d, code=%d and class=%d\n", bsg_type, bsg_code,bs_class );
  }

  return (uint32) bs_class;

} /* qcril_cm_ss_get_service_class */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_set_bsg_info

===========================================================================*/
/*!
    @brief
    finds and sets appropriate bsg type and bsg code that matches the received service class

    INPUT:   service class

    @return
    Returns none
*/
/*=========================================================================*/
void qcril_cm_ss_set_bsg_info
(
  int service_class,
  basic_service_s_type *bsg_info
)
{
  uint32 index = 0;

  while ( QCRIL_CM_SS_CLASS_MAX != qcril_cm_ss_bs_mapping_table[ index ].bs_class)
  {
    /* This same comparison will handle aggregate or single class */
    if ( ( service_class & ( int ) qcril_cm_ss_bs_mapping_table[ index ].bs_class ) ==
         ( int ) qcril_cm_ss_bs_mapping_table[ index ].bs_class )
    {
      bsg_info->code_type = qcril_cm_ss_bs_mapping_table[ index ].bs_type;
      bsg_info->bs_code = ( bsg_code_type ) qcril_cm_ss_bs_mapping_table[ index ].bs_code;
      bsg_info->present = TRUE;
      break;
    }

    index++;
  }

} /* end of qcril_cm_ss_set_bsg_info */

/*=========================================================================
  FUNCTION:  qcril_cm_ss_convert_utf8_to_ucs2

===========================================================================*/
/*!
    @brief
    convets the USSD message received in UTF8 format to ASCII or UCS2 format which
    are the only format recognized by protocol layers.

    INPUT:   USSD string in UTF8 format  
    
    @return
    None (stores the converted USSD charecters in ussd_str)
*/
/*=========================================================================*/
void qcril_cm_ss_convert_utf8_to_ucs2
(
  const char *rcvd_ussd_str, 
  char *ussd_str, 
  int *ussd_str_length
)
{
  UTF16 *utf16TargetStart;
  UTF8 *utf8SourceStart;
  ConversionResult result = conversionOK;
  int length = 0;
 
  length = ( int ) strlen( rcvd_ussd_str );
  utf8SourceStart = ( UTF8 * ) rcvd_ussd_str;
  utf16TargetStart = ( UTF16 *) ussd_str;
 
  result = qcril_cm_ss_ConvertUTF8toUTF16( ( const UTF8 ** ) &utf8SourceStart, 
                                           ( UTF8 * ) ( rcvd_ussd_str + length ),
                                           &utf16TargetStart, 
                                           ( UTF16 * ) ( ussd_str + MAX_USS_CHAR ),
                                           strictConversion );
  if ( result != sourceIllegal )
  {
    /* multiplying by 2 as UCS2 occupies 2 bytes */
    *ussd_str_length = ( ( int ) ( utf16TargetStart - ( UTF16 * ) ussd_str ) ) * 2;
  }
  else
  {
    *ussd_str_length = 0;
  }

} /* qcril_cm_ss_convert_utf8_to_ucs2 */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_convert_ucs2_to_utf8

===========================================================================*/
/*!
    @brief
    convets the USSD message received in ASCII or UCS2 format to 
    UTF8 format which is the only format recognized by RIL layers.

    INPUT:   USSD string ASCII or UCS2 format  
    
    @return
    Length of UTF8 string
*/
/*=========================================================================*/
uint8 qcril_cm_ss_convert_ucs2_to_utf8
(
  char *ussData, 
  int size, 
  char *ussd_str
)
{
  UTF8 utf8_buf[ MAX_USS_CHAR ];
  UTF16 *utf16SourceStart, *utf16SourceEnd;
  UTF8 *utf8SourceStart, *utf8SourceEnd;
  ConversionResult result;
  uint8 length;
 
  utf8SourceStart = utf8_buf;
  utf8SourceEnd = &utf8_buf[ MAX_USS_CHAR ];

  utf16SourceStart = ( UTF16 * ) ussData;
  utf16SourceEnd = ( UTF16 * ) ( ussData + size );

  result = qcril_cm_ss_ConvertUTF16toUTF8( (const UTF16 **) &utf16SourceStart, utf16SourceEnd, 
                                           &utf8SourceStart, utf8SourceEnd, lenientConversion );

  if ( result != conversionOK )
  {
    QCRIL_LOG_ERROR( "%s", "Error in decoding the USSD response string\n" );
  }

  length = (unsigned int) ( utf8SourceStart - utf8_buf );

  memcpy( ussd_str, utf8_buf, length );
  ussd_str[ length ] = '\0';
 
  return length;

} /* qcril_cm_ss_convert_ucs2_to_utf8 */


/*=========================================================================
  FUNCTION:  cm_ss_isLegalUTF8

===========================================================================*/
/*!
    @brief
    * Utility routine to tell whether a sequence of bytes is legal UTF-8.
    * This must be called with the length pre-determined by the first byte.
    * If not calling this from ConvertUTF8to*, then the length can be set by:
    *  length = trailingBytesForUTF8[*source]+1;
    * and the sequence is illegal right away if there aren't that many bytes
    * available.
    * If presented with a length > 4, this returns false.  The Unicode
    * definition of UTF-8 goes up to 4-byte sequences.

    INPUT:   USSD string ASCII or UCS2 format 
    
    @return
    None (stores the converted USSD charecters in ussd_str)
*/
/*=========================================================================*/
static boolean qcril_cm_ss_isLegalUTF8
(
  const UTF8 *source, 
  int length
)
{
  UTF8 a;
  const UTF8 *srcptr = source+length;

  switch (length) 
  {
    default: 
      return FALSE;

    /* Everything else falls through when "true"... */
    case 4: 
      if ( ( a = ( *--srcptr ) ) < 0x80 || a > 0xBF ) return FALSE;

    case 3: 
      if ( ( a = ( *--srcptr ) ) < 0x80 || a > 0xBF ) return FALSE;

    case 2: 
      if ( ( a = ( *--srcptr ) ) > 0xBF ) return FALSE;

    switch (*source) 
    {
      /* no fall-through in this inner switch */
      case 0xE0: 
        if ( a < 0xA0 ) return FALSE; 
        break;

      case 0xED: 
        if ( a > 0x9F ) return FALSE; 
        break;

      case 0xF0: 
        if ( a < 0x90 ) return FALSE; 
        break;

      case 0xF4: 
        if ( a > 0x8F ) return FALSE; 
        break;

      default:   
        if ( a < 0x80 ) return FALSE;
    }

    case 1: 
      if ( *source >= 0x80 && *source < 0xC2 ) return FALSE;
  }

  if ( *source > 0xF4 ) return FALSE;

  return TRUE;

} /* qcril_cm_ss_isLegalUTF8 */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_isLegalUTF8Sequence

===========================================================================*/
/*!
    @brief
    Exported function to return whether a UTF-8 sequence is legal or not.
    This is not used here; it's just exported.

    INPUT:        USSD string ASCII or UCS2 format    
    
    @return
    None (stores the converted USSD charecters in ussd_str)
*/
/*=========================================================================*/
boolean qcril_cm_ss_isLegalUTF8Sequence
(
  const UTF8 *source, 
  const UTF8 *sourceEnd
) 
{
  int length = trailingBytesForUTF8[ *source ] + 1;

  if ( source+length > sourceEnd ) 
  {
    return FALSE;
  }

  return qcril_cm_ss_isLegalUTF8( source, length );

} /* qcril_cm_ss_isLegalUTF8Sequence */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_ConvertUTF8toUTF16

===========================================================================*/
/*!
    @brief
    convets the USSD message received in ASCII or UCS2 format to
    UTF8 format which is the only format recognized by RIL layers.

    INPUT:        USSD string in UTF8 format  
    
    @return
    USSD string in UCS2 format
*/
/*=========================================================================*/
ConversionResult qcril_cm_ss_ConvertUTF8toUTF16 
(
  const UTF8** sourceStart, 
  const UTF8* sourceEnd, 
  UTF16** targetStart, 
  UTF16* targetEnd, 
  ConversionFlags flags
) 
{
  ConversionResult result = conversionOK;
  const UTF8* source = *sourceStart;
  UTF16* target = *targetStart;

  while ( source < sourceEnd ) 
  {
    UTF32 ch = 0;
    unsigned short extraBytesToRead = trailingBytesForUTF8[ *source ];

    if ( source + extraBytesToRead >= sourceEnd ) 
    {
      result = sourceExhausted; 
      break;
    }

    /* Do this check whether lenient or strict */
    if ( !qcril_cm_ss_isLegalUTF8( source, extraBytesToRead + 1 ) ) 
    {
      result = sourceIllegal;
      break;
    }

    /*
     * The cases all fall through. See "Note A" below.
     */
    switch ( extraBytesToRead ) 
    {
      case 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
      case 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
      case 3: ch += *source++; ch <<= 6;
      case 2: ch += *source++; ch <<= 6;
      case 1: ch += *source++; ch <<= 6;
      case 0: ch += *source++;
    }

    ch -= offsetsFromUTF8[ extraBytesToRead ];

    if ( target >= targetEnd ) 
    {
      source -= ( extraBytesToRead + 1 ); /* Back up source pointer! */
      result = targetExhausted; 
      break;
    }

    if ( ch <= UNI_MAX_BMP ) 
    { 
      /* Target is a character <= 0xFFFF */
      /* UTF-16 surrogate values are illegal in UTF-32 */
      if ( ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END ) 
      {
        if ( flags == strictConversion ) 
        {
          source -= ( extraBytesToRead + 1 ); /* return to the illegal value itself */
          result = sourceIllegal;
          break;
        } 
        else
        {
          *target++ = UNI_REPLACEMENT_CHAR;
        }
      } 
      else 
      {
        *target++ = ( UTF16 ) ch; /* normal case */
      }
    } 
    else if ( ch > UNI_MAX_UTF16 ) 
    {
      if ( flags == strictConversion ) 
      {
        result = sourceIllegal;
        source -= ( extraBytesToRead + 1 ); /* return to the start */
        break; /* Bail out; shouldn't continue */
      } 
      else 
      {
        *target++ = UNI_REPLACEMENT_CHAR;
      }
    } 
    else 
    {
      /* target is a character in range 0xFFFF - 0x10FFFF. */
      if ( target + 1 >= targetEnd ) 
      {
        source -= ( extraBytesToRead + 1 ); /* Back up source pointer! */
        result = targetExhausted; break;
      }

      ch -= halfBase;
      *target++ = ( UTF16 ) ( ( ch >> halfShift ) + UNI_SUR_HIGH_START );
      *target++ = ( UTF16 ) ( ( ch & halfMask ) + UNI_SUR_LOW_START );
    }
  }

  *sourceStart = source;
  *targetStart = target;

  return result;

} /* qcril_cm_ss_ConvertUTF8toUTF16 */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_ConvertUTF16toUTF8

===========================================================================*/
/*!
    @brief
    * The interface converts a whole buffer to avoid function-call overhead.
    * Constants have been gathered. Loops & conditionals have been removed as
    * much as possible for efficiency, in favor of drop-through switches.
    * (See "Note A" at the bottom of the file for equivalent code.)
    * If your compiler supports it, the "qcril_cm_ss_isLegalUTF8" call can be turned
    * into an inline function.

    INPUT:    USSD string in UCS2 format
    
    @return
    USSD string in UTF8 format
*/
/*=========================================================================*/
ConversionResult qcril_cm_ss_ConvertUTF16toUTF8
(
  const UTF16** sourceStart,
  const UTF16* sourceEnd, 
  UTF8** targetStart, 
  UTF8* targetEnd, 
  ConversionFlags flags
) 
{
  ConversionResult result = conversionOK;
  const UTF16* source = *sourceStart;
  UTF8* target = *targetStart;

  while ( source < sourceEnd ) 
  {
    UTF32 ch;
    unsigned short bytesToWrite = 0;
    const UTF32 byteMask = 0xBF;
    const UTF32 byteMark = 0x80; 
    const UTF16* oldSource = source; /* In case we have to back up because of target overflow. */

    ch = *source++;

    /* If we have a surrogate pair, convert to UTF32 first. */
    if ( ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END ) 
    {
      /* If the 16 bits following the high surrogate are in the source buffer... */
      if ( source < sourceEnd ) 
      {
        UTF32 ch2 = *source;

        /* If it's a low surrogate, convert to UTF32. */
        if ( ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END ) 
        {
          ch = ( ( ch - UNI_SUR_HIGH_START ) << halfShift ) + ( ch2 - UNI_SUR_LOW_START ) + halfBase;
          ++source;
        } 
        /* it's an unpaired high surrogate */
        else if ( flags == strictConversion ) 
        { 
          --source; /* return to the illegal value itself */
          result = sourceIllegal;
          break;
        }
      } 
      /* We don't have the 16 bits following the high surrogate. */
      else 
      {
        --source; /* return to the high surrogate */
        result = sourceExhausted;
        break;
      }
    } 
    else if ( flags == strictConversion ) 
    {
      /* UTF-16 surrogate values are illegal in UTF-32 */
      if ( ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END ) 
      {
        --source; /* return to the illegal value itself */
        result = sourceIllegal;
        break;
      }
    }

    /* Figure out how many bytes the result will require */
    if ( ch < ( UTF32 ) 0x80 ) 
    { 
      bytesToWrite = 1;
    } 
    else if ( ch < ( UTF32 )0x800 ) 
    { 
      bytesToWrite = 2;
    } 
    else if ( ch < ( UTF32 ) 0x10000 ) 
    {   
      bytesToWrite = 3;
    } 
    else if (ch < ( UTF32 ) 0x110000 ) 
    {  
      bytesToWrite = 4;
    } 
    else 
    { 
      bytesToWrite = 3;
      ch = UNI_REPLACEMENT_CHAR;
    }

    target += bytesToWrite;
    if ( target > targetEnd ) 
    {
      source = oldSource; /* Back up source pointer! */
      target -= bytesToWrite; result = targetExhausted; 
      break;
    }

    switch (bytesToWrite) 
    { 
      /* note: everything falls through. */
      case 4: *--target = ( UTF8 ) ( ( ch | byteMark ) & byteMask ); ch >>= 6;
      case 3: *--target = ( UTF8 ) ( ( ch | byteMark ) & byteMask ); ch >>= 6;
      case 2: *--target = ( UTF8 ) ( ( ch | byteMark ) & byteMask ); ch >>= 6;
      case 1: *--target = ( UTF8 ) ( ch | firstByteMark[ bytesToWrite ] );
    }

    target += bytesToWrite;

  }

  *sourceStart = source;
  *targetStart = target;

  return result;

} /* qcril_cm_ss_ConvertUTF16toUTF8 */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_write_token

===========================================================================*/
/*!
    @brief
    Writes the Sups token to the dest character buffer.

    INPUT:    Sups string tokens one at a time
    
    @return
    IxErrnoType Result of Sups construction
    (Also stores the Sups MMI characters in dest buffer)
*/
/*=========================================================================*/
IxErrnoType qcril_cm_ss_write_token
(
  char *dest,
  char *src, 
  int *idx, 
  int max_dest_len
)
{
  int src_len = 0;

  /* check if we have valid input */
  if ( ( NULL == src ) || ( '\0' == *src ) || ( NULL == dest ) || ( NULL == idx ) )
  {
    return E_NO_DATA;
  }

  src_len = strlen( src );

  /* dest buffer bounds check */
  if ( ( *idx + src_len ) >= max_dest_len )
  {
    QCRIL_LOG_ERROR( "%s", "dest buffer full!\n" );
    return E_OUT_OF_RANGE;
  }

  /* Copy the Sups token to the destination buffer */
  strncpy( &dest[ *idx ], src, src_len );
  *idx += src_len;

  return E_SUCCESS;

}/* qcril_cm_ss_write_token */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_write_sups_tokens

===========================================================================*/
/*!
    @brief
    Build the Sups string from the sups tokens.
    One of **, ##, *#, *, #, **03* is say Sups Type (ST)
    Service Code is (SC).
    Supplementary information (SI) is made of SIA, SIB, SIC
    Sups String constructed will be one among the many forms given below
    STSC*SIA*SIB*SIC#
    STSC*SIA*SIB#
    STSC*SIA**SIC#
    STSC*SIA#
    STSC**SIB*SIC#
    STSC**SIB#
    STSC***SIC#
    STSC#

    INPUT:    Sups string in tokenized form
              ss_tokens[0]  is  ST
              ss_tokens[1]  is  SC
              ss_tokens[2]  is  SIA
              ss_tokens[3]  is  SIB
              ss_tokens[4]  is  SIC

    @return
    IxErrnoType  Result of Sups construction
    (Also buf will have the final constructed sups string)
*/
/*=========================================================================*/
IxErrnoType qcril_cm_ss_write_sups_tokens
(
  char *ss_tokens[], 
  char *buf, 
  int max_buf_len
)
{
  int i = 0;
  int max_len = max_buf_len - 2;
  boolean is_sia_present = FALSE, is_sib_present = FALSE, is_sic_present = FALSE;

  if ( NULL == buf )
  {
    QCRIL_LOG_ERROR( "%s", "Input buf is NULL\n");
    return E_FAILURE;
  }

  /* Write the mode type string. This is mandatory */
  if ( E_SUCCESS != qcril_cm_ss_write_token( buf, ss_tokens[ 0 ], &i, max_len ) )
  {
    return E_FAILURE;
  }

  /* Write the Service Code(SC). SC is not mandatory in all cases. */
  if ( E_OUT_OF_RANGE == qcril_cm_ss_write_token( buf, ss_tokens[ 1 ], &i, max_len ) )
  {
    return E_FAILURE;
  }

  is_sia_present = ( ( ss_tokens[ 2 ] != NULL ) && ( *ss_tokens[ 2 ] != '\0' ) );
  is_sib_present = ( ( ss_tokens[ 3 ] != NULL ) && ( *ss_tokens[ 3 ] != '\0' ) );
  is_sic_present = ( ( ss_tokens[ 4 ] != NULL ) && ( *ss_tokens[ 4 ] != '\0' ) );

  if ( is_sia_present )
  {
    buf[i++] = '*';
    /* Write SIA */
    if ( E_SUCCESS != qcril_cm_ss_write_token( buf, ss_tokens[ 2 ], &i, max_len ) )
    {
      return E_FAILURE;
    }
  }
  else
  {
    if ( is_sib_present || is_sic_present )
    {
      /* Extra * for not having SIA. Check function description 
      for more information */
      buf[ i++ ] = '*';
    }
  }

  if ( is_sib_present )
  {
    buf[ i++ ] = '*';
    /* Write SIB */
    if ( E_SUCCESS != qcril_cm_ss_write_token( buf, ss_tokens[ 3 ], &i, max_len  ))
    {
      return E_FAILURE;
    }
  }
  else
  {
    if ( is_sic_present )
    {
      /* Extra * for not having SIB */
      buf[ i++ ] = '*';
    }
  }

  if ( is_sic_present )
  {
    buf[ i++ ] = '*';
    /* Write SIC */
    if ( E_SUCCESS != qcril_cm_ss_write_token( buf, ss_tokens[ 4 ], &i, max_len ) )
    {
      return E_FAILURE;
    }
  }

  buf[ i++ ] = '#';
  buf[ i ] = '\0';

  return E_SUCCESS;

}/* qcril_cm_ss_write_sups_tokens */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_build_sups_string

===========================================================================*/
/*!
    @brief
    Prepares the Sups tokens and constructs the Sups string.

    INPUT:    Sups information required to construct the string
    
    @return
    IxErrnoType  Result of the sups construction.
    (Also stores the constructed Sups string in buf)
*/
/*=========================================================================*/
IxErrnoType qcril_cm_ss_build_sups_string
(
  qcril_cm_ss_sups_params_s_type *ss_params,
  char *buf,
  int max_len
)
{
  char *ss_tokens[ 5] ;
  char *ss_bsg = NULL;
  char ss_nr_timer[ QCRIL_CM_SS_MAX_INT_DIGITS + 1 ];  /* include nul terminating char */
  uint8 i = 0;
  qcril_cm_ss_mode_table_s_type *curr_ss_mode = qcril_cm_ss_mode_input;

  QCRIL_LOG_INFO( "%s \n", __FUNCTION__);

  if ( ( NULL == buf ) || ( NULL == ss_params ) )
  {
    QCRIL_LOG_ERROR( "%s \n", "Invalid Input parameters" );
    return E_FAILURE;
  }

  /* Derive the sups type string */
  while ( curr_ss_mode->sups_mode_str != NULL )
  {
    if ( curr_ss_mode->sups_mode == ss_params->mode )
    {
      /* Store mode type */
      ss_tokens[ 0 ] = curr_ss_mode->sups_mode_str;
      break;
    }
    curr_ss_mode++;
  }

  /* Derive the MMI Service Code */
  ss_tokens[ 1 ] = NULL;
  for ( i = 0; i < QCRIL_CM_SS_MAX_SC_ENTRY; i++ )
  {
    if ( ss_params->code == qcril_cm_ss_sc_conversion_table[ i ].net_sc ) 
    {
      /* Store service code in [1] */
      ss_tokens[ 1 ] = qcril_cm_ss_sc_conversion_table[ i ].mmi_sc;
      break;
    }
  }

  /* check if sups request is for change password */
  if ( QCRIL_CM_SS_MODE_REG_PASSWD == ss_params->mode )
  {
    /* Copy the password in the tokens */
    ss_tokens[ 2 ] = ss_params->req.passwd.old_passwd;
    ss_tokens[ 3 ] = ss_params->req.passwd.new_passwd;
    ss_tokens[ 4 ] = ss_params->req.passwd.new_passwd_again;
    return qcril_cm_ss_write_sups_tokens( ss_tokens, buf, max_len );
  }

  /* Derive the Basic Service Group MMI from the Service class */
  for ( i = 0; i < QCRIL_CM_SS_MAX_BSG_ENTRY; i++ ) 
  {
    if ( ss_params->service_class == qcril_cm_ss_bsg_conversion_table[ i ].service_class ) 
    {
      ss_bsg = qcril_cm_ss_bsg_conversion_table[ i ].mmi_bsg;
      break;
    }
  }

  if ( ( ss_params->code & 0xF0 ) == qcril_cm_ss_allForwardingSS )
  {
    /* Here SIA will be Directory number */
    /* Store SIA */
    ss_tokens[ 2 ] = ss_params->req.reg.number;
    /* In case of Call forwarding, BSG will be in SIB */
    /* Store SIB */
    ss_tokens[ 3 ] = ss_bsg;
    ss_tokens[ 4 ] = NULL;
  }
  else if ( ( ss_params->code & 0xF0 ) == qcril_cm_ss_allCallRestrictionSS )
  {
    ss_tokens[ 2 ] = ss_params->req.passwd.old_passwd;
    /* In case of Call Barring, BSG will be in SIB */
    /* Store SIB in [3]. SIC will not present */
    ss_tokens[ 3 ] = ss_bsg;
    ss_tokens[ 4 ] = NULL;
  }
  else if ( qcril_cm_ss_cw == ss_params->code )
  {
    /* In case of Call Waiting, BSG will be in SIA */
    /* Store SIA.  SIB, SIC are not applicable */
    ss_tokens[ 2 ] = ss_bsg;
    ss_tokens[ 3 ] = NULL;
    ss_tokens[ 4 ] = NULL;
  }
  else
  {
    /* SIA, SIB, SIC will not be present*/
    ss_tokens[ 2 ] = NULL;
    ss_tokens[ 3 ] = NULL;
    ss_tokens[ 4 ] = NULL;
  }

  if ( QCRIL_CM_SS_MODE_REG == ss_params->mode && 
       ( ( ss_params->code & 0xF0 ) == qcril_cm_ss_allForwardingSS ) ) 
  {
    if ( 0 != ss_params->req.reg.nr_timer ) 
    {
      QCRIL_SNPRINTF( ss_nr_timer, QCRIL_CM_SS_MAX_INT_DIGITS + 1, "%d", ss_params->req.reg.nr_timer );
      ss_tokens[ 4 ] = ss_nr_timer;
    }
  }

  return qcril_cm_ss_write_sups_tokens( ss_tokens, buf, max_len );

}/* qcril_cm_ss_build_sups_string */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_ascii_to_utf8

===========================================================================*/
/*!
    @brief
    converts the USSD string in ASCII format to UTF8 format
    INPUT:        USSD string in ASCII format

    @return
    the lenth of USSD string in UTF8 format.
*/
/*=========================================================================*/
uint8 qcril_cm_ss_ascii_to_utf8
( 
  char* ussd_ascii_str,  
  char* ussd_utf_str, 
  uint8 ussd_ascii_len
)
{
  uint8 index = 0, ussd_utf8_len = 0;

  while ( index < ussd_ascii_len )
  {
     /* for charecters in extended ASCII set */
     if ( ussd_ascii_str[index] > 127 )
     {
       ussd_utf_str[ ussd_utf8_len++ ] = 0xc0 | ( ussd_ascii_str[ index ] >> 6 );
       ussd_utf_str[ ussd_utf8_len++ ] = 0x80 | ( ussd_ascii_str[ index++ ] & 0x3F );
     }
     /* for ignoring the carriage return charecter */
     else if ( ussd_ascii_str[ index ]  == 0x0D )
     {
       index ++;
       QCRIL_LOG_ERROR( "ignored charecter at index = %d \n", index - 1 );
     }
     else
     {
        ussd_utf_str [ussd_utf8_len++ ] = ussd_ascii_str[ index++ ];
     }
  }

  ussd_utf_str[ ussd_utf8_len ] = '\0';

  return ussd_utf8_len;

} /* qcril_cm_ss_ascii_to_utf8 */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_ConvertUTF8toUTF16

===========================================================================*/
/*!
    @brief
    checks whether the received USSD string in UTF8 format has only ASCII charecters or not
    INPUT:        USSD string in UTF8 format

    @return
    SUCCESS if all charecters of the USSD strings are in ASCII format
    FALSE otherwise
*/
/*=========================================================================*/
boolean qcril_cm_ss_UssdStringIsAscii
(
  const char *string
)
{
  int i = 0;
  boolean flag = TRUE;

  while ( ( string[ i ] != '\0' ) && ( flag == TRUE ) )
  {
    if ( string[ i ] & 0x80 )
    {
      flag = FALSE;
    }
    i++;
  }

  return flag;

} /* qcril_cm_ss_UssdStringIsAscii */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_convert_gsm_def_alpha_string_to_utf8

===========================================================================*/
/*!
    @brief
    Convert the input GSM 7-bit Default Alphabet string to UTF8 format.

    @return
    Length of UTF8 string.

*/
/*=========================================================================*/
uint16 qcril_cm_ss_convert_gsm_def_alpha_string_to_utf8
(
  const char *gsm_data,
  byte gsm_data_len,
  char *utf8_buf
)
{
  uint16 i, j, num_chars;
  byte *temp_buf;
  uint8 hi_utf8, lo_utf8;
  uint16 unicode;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( gsm_data != NULL );
  QCRIL_ASSERT( utf8_buf != NULL );
  
  /*-----------------------------------------------------------------------*/

  if ( ( gsm_data == NULL ) || ( gsm_data_len == 0 ) || ( utf8_buf == NULL ) )
  {
    QCRIL_LOG_ERROR( "Invalid parameters for GSM alphabet to UTF8 conversion, input len = %d\n", gsm_data_len ); 
    return 0;
  }

  /* Allocate buffer */
  temp_buf = (byte *) qcril_malloc( gsm_data_len * 2 );
  if ( temp_buf == NULL )
  {
    QCRIL_LOG_ERROR( "%s\n", "Fail to allocate buffer for GSM alphabet to UTF8 conversion" ); 
    return 0;
  }

  /* Unpack the string from 7-bit format into 1 char per byte format */
  num_chars = qcril_cm_util_ussd_unpack( temp_buf, (const byte *) gsm_data, gsm_data_len );

  for ( i = 0, j = 0; i < num_chars; i++ )
  {

    if(temp_buf[i] == 0x0D)
    {
      /* ignoring the non-standard representation of new line (carriage return charecter is also included along with new line)*/
      QCRIL_LOG_DEBUG( "ignored CR charecter at index = %d \n", i);
      continue;
    }

    unicode = gsm_def_alpha_to_utf8_table[ temp_buf[ i ] ];
    hi_utf8 = ( uint8 ) ( ( unicode & 0xFF00 ) >> 8 );
    lo_utf8 = ( uint8 ) unicode & 0x00FF;
    if ( hi_utf8 > 0 )
    {
      utf8_buf[ j++ ] = hi_utf8;
    }

    utf8_buf[ j++ ] = lo_utf8;
  }

  utf8_buf[ j ] = '\0';

  qcril_free( temp_buf );

  return j;

} /* qcril_cm_ss_convert_gsm_def_alpha_string_to_utf8() */


/*=========================================================================
  FUNCTION:  qcril_cm_ss_convert_ussd_string_to_utf8

===========================================================================*/
/*!
    @brief
    Convert the input USSD string to a UTF8 format string.

    @return
    Length of UTF8 string.

*/
/*=========================================================================*/
uint16 qcril_cm_ss_convert_ussd_string_to_utf8
(
  byte uss_data_coding_scheme,
  byte uss_data_len,
  byte *uss_data,
  char *utf8_buf
)
{
  byte hi_dcs, lo_dcs;
  uint16 utf8_len = 0;
  byte i;
  byte temp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( uss_data != NULL );
  QCRIL_ASSERT( utf8_buf != NULL );
  
  /*-----------------------------------------------------------------------*/

  hi_dcs = ( ( uss_data_coding_scheme & 0xF0 ) >> 4 );
  lo_dcs = ( uss_data_coding_scheme & 0x0F );

  /* Refer to 3GPP TS 23.038 V6.1.0 , Section 5 : CBS Data Coding Scheme */
  /* GSM 7-bit alphabet */
  if ( ( hi_dcs == QCRIL_CM_SS_USSD_DCS_7_BIT ) || 
       ( ( hi_dcs == 0x01 ) && ( lo_dcs == QCRIL_CM_SS_USSD_DCS_7_BIT_MASK ) ) ||
       ( hi_dcs == 0x02 ) || 
       ( hi_dcs == 0x03 ) || 
       ( ( hi_dcs & 0x04 ) && ( ( lo_dcs & 0x0C ) == QCRIL_CM_SS_USSD_DCS_7_BIT_MASK ) ) ||
       ( ( hi_dcs == 0x09 ) && ( ( lo_dcs & 0x0C ) == QCRIL_CM_SS_USSD_DCS_7_BIT_MASK ) ) ||
       ( ( hi_dcs == 0x0F ) && ( ( lo_dcs & 0x04 ) == QCRIL_CM_SS_USSD_DCS_7_BIT_MASK ) ) )
  { 
    /* Convert from GSM 7-bit alphabet to UTF8 */
    utf8_len = qcril_cm_ss_convert_gsm_def_alpha_string_to_utf8( (const char *) uss_data, uss_data_len, utf8_buf );
    QCRIL_LOG_DEBUG( "USSD DCS 7-bit str, hi %d lo %d: %s\n", hi_dcs, lo_dcs, utf8_buf ); 
  }
  /* 8-bit */
  else if ( ( ( hi_dcs & 0x04 ) && ( ( lo_dcs & 0x0C ) == QCRIL_CM_SS_USSD_DCS_8_BIT_MASK ) ) ||
            ( ( hi_dcs == 0x09 ) && ( ( lo_dcs & 0x0C ) == QCRIL_CM_SS_USSD_DCS_8_BIT_MASK ) ) ||
            ( ( hi_dcs == 0x0F ) && ( ( lo_dcs & 0x04 ) == QCRIL_CM_SS_USSD_DCS_8_BIT_MASK ) ) )
  {
    utf8_len = uss_data_len;
    memcpy( (void*) utf8_buf, uss_data, utf8_len );
    utf8_buf[ utf8_len ] = '\0';
    QCRIL_LOG_DEBUG( "USSD DCS 8-bit str, hi %d lo %d: %s\n", hi_dcs, lo_dcs, utf8_buf ); 
  }
  /* UCS2 preceded by two GSM 7-bit alphabet */
  else if ( ( hi_dcs == 0x01 ) && ( lo_dcs == QCRIL_CM_SS_USSD_DCS_UCS2_LANG_IND_MASK ) )  
  {
    QCRIL_LOG_DEBUG( "ussd string size preceded by language = %d", uss_data_len );

    for( i = 2; i< uss_data_len; i += 2 )
    {
      temp = uss_data[ i ];
      uss_data[ i ] = uss_data[ i + 1 ];
      uss_data[ i + 1 ] = temp;
    }

    utf8_len = qcril_cm_ss_convert_ucs2_to_utf8( (char *) &uss_data[2], uss_data_len - 2, utf8_buf );
    QCRIL_LOG_DEBUG( "USSD DCS UCS2 str, hi %d lo %d: %s\n", hi_dcs, lo_dcs, utf8_buf ); 
  }
  /* UCS2 */
  else if ( ( ( hi_dcs & 0x04 ) && ( ( lo_dcs & 0x0C ) == QCRIL_CM_SS_USSD_DCS_UCS2_MASK ) ) || 
            ( ( hi_dcs == 0x09 ) && ( ( lo_dcs & 0x0C ) == QCRIL_CM_SS_USSD_DCS_UCS2_MASK ) ) )
  {
    QCRIL_LOG_DEBUG( "ussd string size = %d", uss_data_len );

    for( i = 0; i< uss_data_len; i += 2 )
    {
      temp = uss_data[ i ];
      uss_data[ i ] = uss_data[ i + 1 ];
      uss_data[ i + 1 ] = temp;
    }

    utf8_len = qcril_cm_ss_convert_ucs2_to_utf8( (char *) uss_data, uss_data_len, utf8_buf );
    QCRIL_LOG_DEBUG( "USSD DCS UCS2 str, hi %d lo %d: %s\n", hi_dcs, lo_dcs, utf8_buf ); 
  }

  return utf8_len;

} /* qcril_cm_util_convert_ussd_string_to_utf8 */
