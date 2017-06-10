/******************************************************************************
  @file    qcril_cm_util.c
  @brief   qcril qmi - compatibility layer for CM

  DESCRIPTION
    Utilities functions to support QCRIL_CM processing.

  ---------------------------------------------------------------------------

  Copyright (c) 2009-2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/



/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <math.h>
#include <cutils/memory.h>
#include <string.h>
#include <cutils/properties.h>
#include <errno.h>
#include "comdef.h"
#include "qcril_cmi.h"
#include "qcrili.h"
#include "qcril_arb.h"
#include "qcril_cm_util.h"
#include "qcril_cm_ss.h"

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

// backport from cm.h -- start

#define CM_TON_MASK                        0x70
#define MAX_DISPLAY_TEXT_LEN       255


// backport from cm.h -- end


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/




/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/



/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_cm_util_rssi_to_gw_sigal_strength

===========================================================================*/
/*!
    @brief
    Convert the RSSI to signal strength info.

    @return
    None.
*/
/*=========================================================================*/
void qcril_cm_util_rssi_to_gw_signal_strength
(
  uint16 rssi,
  int *signal_strength_ptr
)
{

  if(signal_strength_ptr != NULL)
  {
  /* RSSI dbm is actually a negative value

     RSSI                              Signal Strength
     ------                            ---------------
     -113 or less                      0
     -111                              1
     -109 ... -53                      2 ... 30
     -51 or greater                    31
     not known or not detected         99
  */
  if ( ( QCRIL_CM_RSSI_MIN < rssi ) && ( QCRIL_CM_RSSI_MAX > rssi ) )
  {
    *signal_strength_ptr = (int) ( floor( ( ( rssi * QCRIL_CM_RSSI_SLOPE + QCRIL_CM_RSSI_OFFSET ) *
                                            QCRIL_CM_RSSI_TOOHI_CODE ) / 100 + 0.5 ) );
  }
  else if ( ( QCRIL_CM_RSSI_MAX <= rssi ) && ( QCRIL_CM_RSSI_NO_SIGNAL != rssi ) )
  {
    *signal_strength_ptr = QCRIL_CM_RSSI_TOOLO_CODE;
  }
  else if ( QCRIL_CM_RSSI_MIN >= rssi )
  {
    *signal_strength_ptr = QCRIL_CM_RSSI_TOOHI_CODE;
  }
  else
  {
    *signal_strength_ptr = QCRIL_CM_GW_SIGNAL_STRENGTH_UNKNOWN;
  }
  }
  else
  {
    QCRIL_LOG_FATAL("FATAL : CHECK FAILED");
  }

} /* qcril_cm_util_rssi_to_gw_signal_strength */


/*=========================================================================
  FUNCTION:  qcril_cm_util_convert_2s_complement_to_int

===========================================================================*/
/*!
    @brief
    convert a byte from 2's complement 6 bit number to integer

    @return
    integer

*/
/*=========================================================================*/
int qcril_cm_util_convert_2s_complement_to_int
(
  byte i
)
{
  byte j=0,k=0;
  if (i >= 0x20 )
  {
    j = ((~i + 1) << 2);
    k = ( -1 * (j >> 2) );
  }
  else
  {
    k = i;
  }
  return k;
} /* qcril_cm_util_convert_2s_complement_to_int */


/*===========================================================================
  FUNCTION: qcril_cm_util_ussd_pack

===========================================================================*/
/*!
    @brief
    Pack 7-bit GSM characters into bytes (8-bits)

    @return
    packed_data length

*/
/*=========================================================================*/
byte qcril_cm_util_ussd_pack(

    byte *packed_data,
    const byte *str,
    byte num_chars
)
{
  byte stridx=0;
  byte pckidx=0;
  byte shift;

  if(packed_data != NULL && str != NULL)
  {
  /* Loop through the 7-bit string till the last but one character.
  */
  while(stridx < (num_chars-1))
  {
   shift = stridx  & 0x07;

   /* A byte of packed data is always made up of only 2 7-bit characters. The
   ** shift of the two characters always depends on their index in the string.
   */
   packed_data[pckidx++] = (str[stridx] >> shift) |
                           (str[stridx+1] << (7-shift)); /*lint !e734 */

   /* If the second characters fits inside the current packed byte, then skip
   ** it for the next iteration.
   */
   if(shift==6) stridx++;
   stridx++;
  }

  /* Special case for the last 7-bit character.
  */
  if(stridx < num_chars)
  {
    shift = stridx & 0x07;
    /* The tertiary operator (?:) takes care of the special case of (8n-1)
    ** 7-bit characters which requires padding with CR (0x0D).
    */
    packed_data[pckidx++] = ((shift == 6) ? (CHAR_CR << 1) : 0) |
                          (str[stridx] >> shift);
  }

  /* Takes care of special case when there are 8n 7-bit characters and the last
  ** character is a CR (0x0D).
  */
  if((num_chars & 0x07) == 0 && str[num_chars - 1] == CHAR_CR)
  {
    packed_data[pckidx++] = CHAR_CR;
  }
  }
  else
  {
    QCRIL_LOG_FATAL("FATAL : CHECK FAILED");
  }

  return pckidx;
} /* qcril_cm_util_ussd_pack */


/*===========================================================================
  FUNCTION: qcril_cm_util_ussd_unpack

===========================================================================*/
/*!
    @brief
    Unpack the bytes (8-bit) into 7-bit GSM characters

    @return
    str length

*/
/*=========================================================================*/
byte qcril_cm_util_ussd_unpack
(
    byte *str,
    const byte *packed_data,
    byte num_bytes
)
{

  byte stridx = 0;
  byte pckidx = 0;
  byte prev = 0;
  byte curr = 0;
  byte shift;

  if(packed_data != NULL && str != NULL)
  {
  while(pckidx < num_bytes)
  {
    shift = stridx & 0x07;
    curr = packed_data[pckidx++];

    /* A 7-bit character can be split at the most between two bytes of packed
    ** data.
    */
    str[stridx++] = ( (curr << shift) | (prev >> (8-shift)) ) & 0x7F;

    /* Special case where the whole of the next 7-bit character fits inside
    ** the current byte of packed data.
    */
    if(shift == 6)
    {
      /* If the next 7-bit character is a CR (0x0D) and it is the last
      ** character, then it indicates a padding character. Drop it.
      */

      if((curr >> 1) == CHAR_CR && pckidx == num_bytes)
      {
        break;
      }
      str[stridx++] = curr >> 1;
    }

    prev = curr;
  }
  }
  else
  {
    QCRIL_LOG_FATAL("FATAL : CHECK FAILED");
  }


  return stridx;

} /* qcril_cm_util_ussd_unpack */


/*===========================================================================
  FUNCTION: qcril_cm_util_bcd_to_ascii

===========================================================================*/
/*!
    @brief
    Convert the phone number from BCD to ASCII

    @return
    None

*/
/*=========================================================================*/
void qcril_cm_util_bcd_to_ascii
(
  const byte *bcd_number,
  byte *ascii_number
)
{
  int bcd_index = 0;
  int ascii_index = 0;
  byte bcd_length;
  uint8 asc_1 = 0;
  uint8 asc_2 = 0;
  boolean presentation_indicator_absent = TRUE;

  if((bcd_number   != NULL) && (ascii_number != NULL))
  {
  bcd_length = bcd_number[bcd_index++];

  /*****************************/
  /* International call or not */
  /*****************************/
  /*lint -save -e641*/
  if ((bcd_number[bcd_index] & CM_TON_MASK) >> 4 == QCRIL_CM_NUM_TYPE_INTERNATIONAL)
  {
     ascii_number[ascii_index++] = '+';
  }
  /*lint -restore */

  /* check if there is an extra byte or not (screening indicator,
  ** presentation_indicator)
  */

  presentation_indicator_absent =
     ((boolean)(bcd_number[bcd_index] & 0x80) >> 7);

  bcd_index++;

  /**************************/
  /* presentation_indicator */
  /**************************/

  if (presentation_indicator_absent == FALSE)
  {
     bcd_index++;
  }


  /*************************/
  /* Decode BCD into ascii */
  /*************************/

  for( ; bcd_index <= bcd_length; bcd_index++)
  {

     asc_1 = (bcd_number[bcd_index] & 0x0F);
     asc_2 = (bcd_number[bcd_index] & 0xF0) >> 4;

     ascii_number[ascii_index++] = (asc_1==QCRIL_BCD_STAR)? '*' :
                                    (asc_1==QCRIL_BCD_HASH)? '#' :
                                    QCRIL_INRANGE(asc_1, 0x0C, 0x0E)? (asc_1 - 0x0C) + 'a':
                                    asc_1 + '0';

     ascii_number[ascii_index++] = (asc_2==QCRIL_BCD_STAR)? '*' :
                                    (asc_2==QCRIL_BCD_HASH)? '#' :
                                    QCRIL_INRANGE(asc_2, 0x0C, 0x0E)? (asc_2 - 0x0C) + 'a':
                                    (asc_2==0x0F)? '\0' :
                                    asc_2 + '0';
  }

  /* Null terminate the ascii string */
  if (asc_2 != 0x0f)
  {
    ascii_number[ascii_index] = '\0';
  }
  }
  else
  {
    QCRIL_LOG_FATAL("FATAL : CHECK FAILED");
  }
} /* qcril_cm_util_bcd_to_ascii */

/*===========================================================================
  FUNCTION: qcril_cm_util_ascii_to_gsm_alphabet

===========================================================================*/
/*!
    @brief
    Convert the ASCII string to GSM default alphabets string and packs it
    into bytes.

    @return
    None

*/
/*=========================================================================*/
byte qcril_cm_util_ascii_to_gsm_alphabet
(
    byte          *gsm_alphabet_string,
    const byte          *ascii_string,
    byte           num_chars
)
{

   byte temp_buffer[MAX_DISPLAY_TEXT_LEN];
   byte n = 0,ret=0;

   if(ascii_string != NULL && gsm_alphabet_string != NULL)
   {
   /*
   ** convert from ascii coding into GSM default-alphabet coding with
   ** 1 char per byte
   */
   for( n = 0; n < num_chars; n++ )
   {
      temp_buffer[n] = qcril_ascii_to_def_alpha_table[ascii_string[n]];
   }

   /*
   ** now pack the string down to 7-bit format
   */
     ret = qcril_cm_util_ussd_pack( gsm_alphabet_string, temp_buffer, num_chars);
   }
   else
   {
     QCRIL_LOG_FATAL("FATAL : CHECK FAILED");
     ret = 0;
   }
   return ret;

} /* qcril_cm_util_ascii_to_gsm_alphabet */


/*===========================================================================
  FUNCTION: qcril_cm_util_gsm_alphabet_to_ascii

===========================================================================*/
/*!
    @brief
    Unpacks bytes of data into 7-bit GSM default alphabet string and then
    converts it to an ASCII string.

    @return
    Number of characters written into the output buffer.

*/
/*=========================================================================*/
byte qcril_cm_util_gsm_alphabet_to_ascii
(
    byte    *ascii_string,
    const byte    *gsm_alphabet_string,
    byte     num_bytes
)
{

   byte temp_buffer[MAX_DISPLAY_TEXT_LEN];
   byte n = 0;
   byte num_chars;

   if(ascii_string != NULL && gsm_alphabet_string != NULL)
   {
   /*
   ** unpack the string from 7-bit format into 1 char per byte format
   */
   num_chars = qcril_cm_util_ussd_unpack(temp_buffer, gsm_alphabet_string, num_bytes);

   /*
   ** now convert from GSM default alphabet coding into ascii coding
   */
   for( n = 0; n < num_chars; n++ )
   {
      ascii_string[n] = qcril_def_alpha_to_ascii_table[temp_buffer[n]];
   }
   ascii_string[num_chars] = '\0';
   }
   else
   {
     num_chars = 0;
     QCRIL_LOG_FATAL("FATAL : CHECK FAILED");
   }

   return num_chars;

} /* qcril_cm_util_gsm_alphabet_to_ascii */
