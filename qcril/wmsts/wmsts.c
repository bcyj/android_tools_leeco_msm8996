/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*


            W I R E L E S S   M E S S A G I N G   S E R V I C E S
            -- Translation Services

GENERAL DESCRIPTION
  The WMS module which implements the User API for SMS. This source file
  implements the translation functions for the client applications to call
  and for internal use.

Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
              by Qualcomm Technologies, Inc.
  All Rights Reserved.

Export of this technology or software is regulated by the U.S. Government.
Diversion contrary to U.S. law prohibited.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


/* ^L<EJECT> */
/*===========================================================================

$Header:

===========================================================================*/


/*===========================================================================
========================  INCLUDE FILES =====================================
===========================================================================*/


#include "comdef.h"

#include "wms.h"
#include "wmsts.h"
#include "string.h"
#include "err.h"
#include <pthread.h>

#define LOG_TAG "WMSTS"
#ifdef FEATURE_UNIT_TEST
#include "Log.h"
#else
#include <utils/Log.h>
#include "common_log.h"
#endif /* FEATURE_UNIT_TEST */

#include "wmstscdma.h"

/*===========================================================================
======================== STATIC DATA ========================================
===========================================================================*/

static uint32                      raw_ts_len;

enum { WMS_UDH_OCTETS_PORT8                =2};
enum { WMS_UDH_OCTETS_PORT16               =4};
enum { WMS_UDH_OCTETS_CONCAT8              =3};
enum { WMS_UDH_OCTETS_SPECIAL_SM           =2};
enum { WMS_UDH_OCTETS_CONCAT16             =4};
enum { WMS_UDH_OCTETS_PRE_DEF              =2};
enum { WMS_UDH_OCTETS_TEXT_FORMATTING      =3};
enum { WMS_UDH_OCTETS_RFC822               =1};
enum { WMS_UDH_OCTETS_USER_PROMPT          =1};
enum { WMS_UDH_OCTETS_EO_HEADER            =7};
static const wms_udh_s_type *const_header;

static wms_gw_user_data_s_type   gw_user_data;

static pthread_mutex_t wmsts_mutex = PTHREAD_MUTEX_INITIALIZER; 

/*===========================================================================
============================ FUNCTIONS ======================================
===========================================================================*/


/*=========================================================================
FUNCTION
  wms_ts_bcd_to_int

DESCRIPTION
  Convert a BCD to Integer.
  Valid BCD values altogether are from 0 to 99 and byte arrangement is <MSB, ... ,LSB)

DEPENDENCIES
  None

RETURN VALUE
  Failure if either of the BCD digit is invaild (greater than 9, e.g. 11111010 = 15 and 10)
  Success if both BCD digits are valid

SIDE EFFECTS
  None
=========================================================================*/
boolean wms_ts_bcd_to_int
(
  const uint8 bcd, /*IN*/
  uint8 *result    /*OUT*/
)
{
  if ( (bcd & 0x0F) > 9 || ((bcd & 0xF0) >> 4) > 9)
  {
    LOGE("Invalid BCD digit!");
    *result = 0;
    return FALSE;
  }
  else
  {
    *result = ( (bcd & 0x0F) + (((bcd & 0xF0) >> 4) * 10) );
    return TRUE;
  }
}

/*=========================================================================
FUNCTION
  wms_ts_int_to_bcd

DESCRIPTION
  Convert an Integer to BCD.
  Valid Integer values are from 0 to 99 and byte arrangement is <MSB, ... ,LSB)

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_int_to_bcd
(
  const uint8 i
)
{
  return (uint8)(((i % 10) + ((i / 10) << 4)));
}

/*=========================================================================
FUNCTION
  wms_ts_encode_address

DESCRIPTION
  Encode address structure into format for SIM storage and for interfacing
  with lower layers.

DEPENDENCIES
  None

RETURN VALUE
  0: Failure
  Otherwise: Number of bytes encoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_encode_address
(
  const wms_address_s_type    * addr,
  uint8                       * data
)
{
  uint8   i, pos = 0;

  if( addr->number_of_digits > WMS_GW_ADDRESS_MAX )
  {
    LOGE("Addr len too long: %d", addr->number_of_digits);
    return 0;
  }

  if (addr->number_type == WMS_NUMBER_ALPHANUMERIC )
  {
    data[pos] = (uint8)((addr->number_of_digits*7 + 3)/4);
  }
  else
  {
    /* Len field: */
    data[pos] = addr->number_of_digits;
  }
  pos ++;

  /* TON & NPI: */
  data[pos] = 0x80;
  data[pos] |= (uint8) ((uint8) addr->number_type << 4);
  data[pos] |= (uint8) addr->number_plan;
  pos ++;

  if (addr->number_type == WMS_NUMBER_ALPHANUMERIC )
  {
    /* Alphanumberic Number Type */
    pos += (uint8)wms_ts_pack_gw_7_bit_chars
              (
                addr->digits,
                addr->number_of_digits,
                0,
                WMS_GW_ADDRESS_MAX,
                &data[pos]
              );
  }
  else
  {
    /* the digits: */
    for( i=0; i<addr->number_of_digits; i++ )
    {
      /* pack two digits each time */
      data[pos]  = (uint8) (addr->digits[i++] & 0x0F);

      if( i == addr->number_of_digits )
      {
        data[pos] |= 0xF0;
      }
      else
      {
        data[pos] |= (uint8)(addr->digits[i] << 4);
      }
      pos ++;
    }
  }
  /* done */
  return pos;
} /* wms_ts_encode_address() */

/*=========================================================================
FUNCTION
  wms_ts_decode_address

DESCRIPTION
  Decode address data into a structure.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_decode_address
(
  const uint8               * data,
  wms_address_s_type        * addr
)
{
  uint8   i, pos = 0;

  /* Len field: number of digits */
  i = data[pos];

  if( i > WMS_GW_ADDRESS_MAX )
  {
    /* Address is too long */
    LOGE("Addr len too long: %d", i);
    return 0;
  }

  addr->number_of_digits = i;

  pos ++;

  /* TON & NPI: */
  addr->digit_mode  = WMS_DIGIT_MODE_4_BIT;
  addr->number_type = (wms_number_type_e_type) (( data[pos] & 0x70 ) >> 4);
  addr->number_plan = (wms_number_plan_e_type) (data[pos] & 0x0F);
  pos ++;

  if (addr->number_type == WMS_NUMBER_ALPHANUMERIC )
  {
    uint8 bytes_increment=0;
    /* Alphanumberic Number Type */
    addr->digit_mode = WMS_DIGIT_MODE_8_BIT;

    /* length = number of BCD digits */
    bytes_increment = (addr->number_of_digits+1)/2;

    addr->number_of_digits = (uint8)(addr->number_of_digits*4/7);

    (void)wms_ts_unpack_gw_7_bit_chars
           (
             &data[pos],
             addr->number_of_digits,
             WMS_GW_ADDRESS_MAX,
             0,
             addr->digits
            );

    pos += bytes_increment;
  }
  else
  {
    /* the digits: */
    for( i=0; i<addr->number_of_digits; i++ )
    {
      /* unpack two digits each time */
      addr->digits[i++] = data[pos] & 0x0F;
      addr->digits[i]   = ( data[pos] & 0xF0 ) >> 4;
      pos ++;
    }
  }
  return pos;
} /* wms_ts_decode_address() */

/*=========================================================================
FUNCTION
  wms_ts_pack_gw_7_bit_chars

DESCRIPTION
  Pack GSM 7-bit characters into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes encoded.

SIDE EFFECTS
  None
=========================================================================*/
uint16 wms_ts_pack_gw_7_bit_chars
(
  const uint8     * in,
  uint16          in_len,      /* Number of 7-bit characters */
  uint16          shift,
  uint16          out_len_max, /* Number of 7-bit characters */
  uint8           * out
)
{
  uint16    i=0;
  uint16    pos = 0;

  if (in == NULL || out == NULL)
  {
    LOGE("null pointer in wms_ts_pack_gw_7_bit_chars");
    return 0;
  }
  /* pack the ASCII characters */
  shift %= 7;

  if (shift != 0)
  {
    /* Pack the upper bytes of the first character */
    out[pos] |= (uint8) (in[i] << shift);
    shift = (7 - shift) + 1;
    if (shift == 7)
    {
      shift = 0;
      i++;
    }
    pos++;
  }

  for( ; pos < out_len_max && i < in_len;
       pos++, i++ )
  {
    /* pack the low bits */
    out[pos] = in[i] >> shift;

    if( i+1 < in_len )
    {
      /* pack the high bits using the low bits of the next character */
      out[pos] |= (uint8) (in[i+1] << (7-shift));

      shift ++;

      if( shift == 7 )
      {
        shift = 0;
        i ++;
      }
    }
  }

  /* done */
  return pos;

} /* wms_ts_pack_gw_7_bit_chars() */


/*=========================================================================
FUNCTION
  wms_ts_unpack_gw_7_bit_chars

DESCRIPTION
  Unpack raw data to GSM 7-bit characters.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_unpack_gw_7_bit_chars
(
  const uint8       * in,
  uint8             in_len,        /* Number of 7-bit characters */
  uint8             out_len_max,   /* Number of maximum 7-bit characters after unpacking */
  uint16            shift,
  uint8             * out
)
{
  int      i=0;
  uint16   pos=0;

  if (in == NULL || out == NULL)
  {
    LOGE("null pointer in wms_ts_unpack_gw_7_bit_chars");
    return 0;
  }

  /*If the number of fill bits != 0, then it would cause an additional shift*/
  if (shift != 0)
   pos = pos+1;

  if (shift ==7)
  {
    out[0] = in[0] >> 1; /*7 fillbits*/
    shift =0;            /*First Byte is a special case*/
    i=1;
  }

  for( i=i;
       i < out_len_max && i< in_len;
       i++, pos++ )
  {
    out[i] = ( in[pos] << shift ) & 0x7F;

    if( pos != 0 )
    {
      /* except the first byte, a character contains some bits
      ** from the previous byte.
      */
      out[i] |= in[pos-1] >> (8-shift);
    }

    shift ++;

    if( shift == 7 )
    {
      shift = 0;

      /* a possible extra complete character is available */
      i ++;
      if( i >= out_len_max )
      {
        LOGE("Not enough output buffer for unpacking!");
        break;
      }
      out[i] = in[pos] >> 1; 
    }
  }

  return(uint8)(pos);

} /* wms_ts_unpack_gw_7_bit_chars() */

/*=========================================================================
FUNCTION
  wms_ts_encode_dcs

DESCRIPTION
  Encode Data Coding Scheme into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes encoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_encode_dcs
(
  const wms_gw_dcs_s_type   *dcs,  /* IN */
  uint8                     *data  /* OUT */
)
{
  uint8 pos = 0;

  if( dcs->msg_waiting == WMS_GW_MSG_WAITING_NONE )
  {
    /* Using the pattern 00xx xxxx */

    /* bit 5 */
    data[pos] = dcs->is_compressed ? 0x20 : 0;

    /* bit 4 */
    data[pos] |= (dcs->msg_class != WMS_MESSAGE_CLASS_NONE ) ? 0x10 : 0;

    /* bits 3-2 */
    data[pos] |= dcs->alphabet << 2;

    /* bits 1-0 */
    data[pos] |= dcs->msg_class & 0x03;
  }
  else if (dcs->msg_waiting == WMS_GW_MSG_WAITING_NONE_1111)
  {
    /* Using the pattern 1111 xxxx */

    data[pos] = 0xf0;

    /*bit 2*/
    if (dcs->alphabet == WMS_GW_ALPHABET_8_BIT)
      data[pos] |= 0x04;


    /* bits 1-0 */
    data[pos] |= dcs->msg_class & 0x03;
  }
  else
  {
    /* bits 7-4 */
    if( dcs->msg_waiting == WMS_GW_MSG_WAITING_DISCARD )
    {
      data[pos] = 0xc0;
    }
    else if( dcs->msg_waiting == WMS_GW_MSG_WAITING_STORE &&
             dcs->alphabet    == WMS_GW_ALPHABET_7_BIT_DEFAULT )
    {
      data[pos] = 0xd0;
    }
    else
    {
      data[pos] = 0xe0;
    }

    /* bit 3 */
    data[pos] |= ( dcs->msg_waiting_active == TRUE ) ? 0x08 : 0;

    /* bit 2 is reserved, set to 0 */

    /* bits 1-0 */
    data[pos] |= dcs->msg_waiting_kind & 0x03;
  }

  pos ++;

  return pos;
} /* wms_ts_encode_dcs() */


/*=========================================================================
FUNCTION
  wms_ts_encode_timestamp

DESCRIPTION
  Encode a timestamp into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes encoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_encode_timestamp
(
  const wms_timestamp_s_type    *timestamp, /* IN */
  uint8                           *data     /* OUT */
)
{
  sint7     i;
  uint8     pos = 0, j;

  /* year check */
  if ( !wms_ts_bcd_to_int(timestamp->year, &j) )
  {
    return 0;
  }
  data[pos] = ((timestamp->year & 0x0F) << 4) + ((timestamp->year& 0xF0) >> 4);
  pos ++;

  /* month check */
  if ( wms_ts_bcd_to_int(timestamp->month, &j) )
  {
    if (j > 12 || j < 1)
    {
      LOGE("Month is invalid: %d", j);
      return 0;
    }
  }
  else
  {
    return 0;
  }
  data[pos] = ((timestamp->month & 0x0F) << 4) + ((timestamp->month & 0xF0) >> 4);
  pos ++;

  /* day check */
  if ( wms_ts_bcd_to_int(timestamp->day, &j) )
  {
    if (j > 31 || j < 1)
    {
      LOGE("Day is invalid: %d", j);
      return 0;
    }
  }
  else
  {
    return 0;
  }
  data[pos] = ((timestamp->day & 0x0F) << 4) + ((timestamp->day & 0xF0) >> 4);
  pos ++;

  /* hour check */
  if ( wms_ts_bcd_to_int(timestamp->hour, &j) )
  {
    if (j > 23)
    {
      LOGE("Hour is invalid: %d", j);
      return 0;
    }
  }
  else
  {
    return 0;
  }
  data[pos] = ((timestamp->hour & 0x0F) << 4) + ((timestamp->hour & 0xF0) >> 4);
  pos ++;

  /* minute check */
  if ( wms_ts_bcd_to_int(timestamp->minute, &j) )
  {
    if (j > 59)
    {
      LOGE("Minute is invalid: %d", j);
      return 0;
    }
  }
  else
  {
    return 0;
  }
  data[pos] = ((timestamp->minute & 0x0F) << 4) + ((timestamp->minute & 0xF0) >> 4);
  pos ++;

  /* second check */
  if ( wms_ts_bcd_to_int(timestamp->second, &j) )
  {
    if (j > 59)
    {
      LOGE("Second is invalid: %d", j);
      return 0;
    }
  }
  else
  {
    return 0;
  }
  data[pos] = ((timestamp->second & 0x0F) << 4) + ((timestamp->second & 0xF0) >> 4);
  pos ++;

  /* timezone check */
  i = (sint7)timestamp->timezone;
  if (i > 48|| i < -48)
  {
    LOGE("Timezone is out of bound: %d", i);
    return 0;
  }

  if (i >= 0)
  {
    data[pos] = (uint8) (((uint8)( i % 10 )) << 4);
    data[pos] |= ( i / 10 );
  }
  else
  {
    i *= (-1);
    data[pos] = (uint8) (((uint8)( i % 10 )) << 4);
    data[pos] |= ( i / 10 );
    data[pos] |= 0x08;
  }
  pos ++;

  return pos;

} /* wms_ts_encode_timestamp() */


static int wms_ts_encode_udh_concat_8
(
  uint8 *udh
)
{
  int pos =0;

  if ( const_header->u.concat_8.total_sm == 0 ||
       const_header->u.concat_8.seq_num  == 0 ||
       const_header->u.concat_8.seq_num > const_header->u.concat_8.total_sm)

  {
    LOGE("SMS UDH Header id %d Present with no Data", const_header->header_id);
    return 0;
  }


  udh[pos++] = (uint8) WMS_UDH_CONCAT_8;
  udh[pos++] = (uint8) WMS_UDH_OCTETS_CONCAT8;
  udh[pos++] = const_header->u.concat_8.msg_ref;
  udh[pos++] = const_header->u.concat_8.total_sm;
  udh[pos++] = const_header->u.concat_8.seq_num;

  return pos;
}/*wms_ts_encode_udh_concat_8*/

static int wms_ts_encode_udh_concat16
(
  uint8 *udh
)
{
  int pos=0;

  if (const_header->u.concat_16.total_sm == 0 ||
      const_header->u.concat_16.seq_num == 0 ||
      const_header->u.concat_16.seq_num > const_header->u.concat_16.total_sm)
  {
    LOGE("SMS UDH Header id %d Present with no Data", const_header->header_id);
    return 0;
  }

  udh[pos++] = (uint8) WMS_UDH_CONCAT_16;
  udh[pos++] = (uint8) WMS_UDH_OCTETS_CONCAT16;
  udh[pos++] = (uint8)((const_header->u.concat_16.msg_ref & 0xFF00) >> 8);
  udh[pos++] = (uint8)(const_header->u.concat_16.msg_ref & 0x00FF);
  udh[pos++] = const_header->u.concat_16.total_sm;
  udh[pos++] = const_header->u.concat_16.seq_num;

  return pos;
}/*wms_ts_encode_udh_concat16*/


int wms_ts_encode_udh_special_sm
(
  uint8 *udh
)
{
  int pos=0;
  udh[pos++] = (uint8) WMS_UDH_SPECIAL_SM;
  udh[pos++] = (uint8) WMS_UDH_OCTETS_SPECIAL_SM;

  /* store or discard the message */
  udh[pos] = (const_header->u.special_sm.msg_waiting == WMS_GW_MSG_WAITING_STORE) ? 0x80 : 0;
  udh[pos] |= (uint8) const_header->u.special_sm.msg_waiting_kind;
  pos++;
  udh[pos++] = const_header->u.special_sm.message_count;

  return pos;
}/*wms_ts_encode_udh_special_sm*/

static int wms_ts_encode_udh_port_8
(
  uint8 *udh
)
{
  int pos =0;

  udh[pos++] = (uint8) WMS_UDH_PORT_8;
  udh[pos++] = WMS_UDH_OCTETS_PORT8;
  udh[pos++] = const_header->u.wap_8.dest_port;
  udh[pos++] = const_header->u.wap_8.orig_port;

  return pos;
}/*wms_ts_encode_udh_port_8*/

static int wms_ts_encode_udh_port16
(
  uint8 *udh
)
{
  int pos =0;

  udh[pos++] = (uint8) WMS_UDH_PORT_16;
  udh[pos++] = (uint8) WMS_UDH_OCTETS_PORT16;
  udh[pos++] = (uint8)((const_header->u.wap_16.dest_port & 0xFF00) >> 8);
  udh[pos++] = (uint8)(const_header->u.wap_16.dest_port & 0x00FF);
  udh[pos++] = (uint8)((const_header->u.wap_16.orig_port & 0xFF00) >> 8);
  udh[pos++] = (uint8)(const_header->u.wap_16.orig_port & 0x00FF);

  return pos;

}/*wms_ts_encode_udh_port16*/

int wms_ts_encode_udh_text_formatting
(
  uint8 *udh
)
{
  int pos=0;

  udh[pos++] = (uint8) WMS_UDH_TEXT_FORMATING;

  if(const_header->u.text_formating.is_color_present)
  {
    udh[pos++] = (uint8) WMS_UDH_OCTETS_TEXT_FORMATTING + 1;
  }
  else
  {
    udh[pos++] = (uint8) WMS_UDH_OCTETS_TEXT_FORMATTING;
  }

  udh[pos++] = const_header->u.text_formating.start_position;
  udh[pos++] = const_header->u.text_formating.text_formatting_length;
  udh[pos]   = (uint8) const_header->u.text_formating.alignment_type;
  /*bit 1 and  bit 0*/
  udh[pos]   = (uint8) (((uint8)const_header->u.text_formating.font_size <<2) | udh[pos]);
  /*bit 3 and  bit 2*/
  udh[pos]   = (uint8)(const_header->u.text_formating.style_bold <<4)         | udh[pos];
  /*bit 4 */
  udh[pos]   = (uint8)(const_header->u.text_formating.style_italic <<5)       | udh[pos];
  /*bit 5 */
  udh[pos]   = (uint8)(const_header->u.text_formating.style_underlined<<6)    | udh[pos];
  /*bit 6 */
  udh[pos]   = (uint8)(const_header->u.text_formating.style_strikethrough<<7) | udh[pos];
  pos++;
  /*bit 7 */

  if(const_header->u.text_formating.is_color_present)
  {
    udh[pos] = 0;
    udh[pos] = (uint8) (const_header->u.text_formating.text_color_foreground)| udh[pos];
    /* bit 0-3 */
    udh[pos] = (uint8) ((uint8) const_header->u.text_formating.text_color_background<<4 | udh[pos]);
    /* bit 4-7 */
    pos++;
  }



  return pos;
}/*wms_ts_encode_udh_text_formatting*/

int wms_ts_encode_udh_pre_def_sound
(
  uint8 *udh
)
{
  int pos=0;

  udh[pos++] = (uint8) WMS_UDH_PRE_DEF_SOUND;
  udh[pos++] = (uint8) WMS_UDH_OCTETS_PRE_DEF;
  udh[pos++] = const_header->u.pre_def_sound.position;
  udh[pos++] = const_header->u.pre_def_sound.snd_number;

  return pos;
}/*wms_ts_encode_udh_pre_def_sound*/

int wms_ts_encode_udh_user_def_sound
(
  uint8 *udh
)
{
  int pos=0,j;

  if (const_header->u.user_def_sound.data_length == 0)
  {
     LOGE("SMS UDH Header id %d Present with no Data", const_header->header_id);
  }


  udh[pos++] = (uint8) WMS_UDH_USER_DEF_SOUND;
  udh[pos++] = const_header->u.user_def_sound.data_length+1;
  udh[pos++] = const_header->u.user_def_sound.position;

  for (j=0;j<const_header->u.user_def_sound.data_length;j++)
    udh[pos++] = const_header->u.user_def_sound.user_def_sound[j];

  return pos;
}/*wms_ts_encode_udh_user_def_sound*/


int wms_ts_encode_udh_pre_def_anim
(
  uint8 *udh
)
{
  int pos=0;

  udh[pos++] = (uint8) WMS_UDH_PRE_DEF_ANIM;
  udh[pos++] = (uint8) WMS_UDH_OCTETS_PRE_DEF;
  udh[pos++] = const_header->u.pre_def_anim.position;
  udh[pos++] = const_header->u.pre_def_anim.animation_number;

  return pos;
}/*wms_ts_encode_udh_pre_def_anim*/

int wms_ts_encode_udh_large_anim
(
  uint8 *udh
)
{
  int pos=0,j,k;

  udh[pos++] = (uint8) WMS_UDH_LARGE_ANIM;
  udh[pos++] = (uint8) WMS_UDH_LARGE_BITMAP_SIZE * (uint8) WMS_UDH_ANIM_NUM_BITMAPS  + 1;
  udh[pos++] = const_header->u.large_anim.position;

  for (j=0;j<WMS_UDH_ANIM_NUM_BITMAPS;j++)
    for (k=0;k<WMS_UDH_LARGE_BITMAP_SIZE;k++)
      udh[pos++] = const_header->u.large_anim.data[j][k];

  return pos;
}/*wms_ts_encode_udh_large_anim*/


int wms_ts_encode_udh_small_anim
(
  uint8 *udh
)
{
  int pos=0,j,k;

  udh[pos++] = (uint8) WMS_UDH_SMALL_ANIM;
  udh[pos++] = (uint8) (WMS_UDH_SMALL_BITMAP_SIZE * WMS_UDH_ANIM_NUM_BITMAPS) + 1;
  udh[pos++] = const_header->u.small_anim.position;


  for (j=0;j<WMS_UDH_ANIM_NUM_BITMAPS;j++)
    for (k=0;k<WMS_UDH_SMALL_BITMAP_SIZE;k++)
      udh[pos++] = const_header->u.small_anim.data[j][k];

  return pos;
}/*wms_ts_encode_udh_small_anim*/

int wms_ts_encode_udh_large_picture
(
  uint8 *udh
)
{
  int pos=0,j;

  udh[pos++] = (uint8) WMS_UDH_LARGE_PICTURE;
  udh[pos++] = (uint8) WMS_UDH_LARGE_PIC_SIZE +1;
  udh[pos++] = const_header->u.large_picture.position;

  for (j=0;j<WMS_UDH_LARGE_PIC_SIZE;j++)
    udh[pos++] = const_header->u.large_picture.data[j];

  return pos;
}/*wms_ts_encode_udh_large_picture*/


int wms_ts_encode_udh_small_picture
(
  uint8 *udh
)
{
  int pos=0,j;

  udh[pos++] = (uint8) WMS_UDH_SMALL_PICTURE;
  udh[pos++] = (uint8) WMS_UDH_SMALL_PIC_SIZE +1;
  udh[pos++] = const_header->u.small_picture.position;

  for (j=0;j<WMS_UDH_SMALL_PIC_SIZE;j++)
    udh[pos++] = const_header->u.small_picture.data[j];

  return pos;
}/*wms_ts_encode_udh_small_picture*/



int wms_ts_encode_udh_var_picture
(
  uint8 *udh
)
{
  int pos=0,j;

  udh[pos++] = (uint8) WMS_UDH_VAR_PICTURE;
  udh[pos++] = (uint8)((const_header->u.var_picture.height * const_header->u.var_picture.width/8) + 3);
  udh[pos++] = const_header->u.var_picture.position;
  udh[pos++] = const_header->u.var_picture.width/8;
  udh[pos++] = const_header->u.var_picture.height;

  for (j=0;j<(const_header->u.var_picture.height * const_header->u.var_picture.width/8);j++)
    udh[pos++] = const_header->u.var_picture.data[j];


  return pos;
}/*wms_ts_encode_udh_var_picture*/


int wms_ts_encode_udh_user_prompt
(
  uint8 *udh
)
{
  int pos=0;

  udh[pos++] = (uint8) WMS_UDH_USER_PROMPT;
  udh[pos++] = (uint8) WMS_UDH_OCTETS_USER_PROMPT; /* only 1 byte for the udh data */
  udh[pos++] = const_header->u.user_prompt.number_of_objects;

  return pos;
} /* wms_ts_encode_udh_user_prompt() */


int wms_ts_encode_udh_eo
(
  uint8 *udh
)
{
  int    pos = 0;
  uint8  udh_length;


  udh[pos++] = (uint8) WMS_UDH_EXTENDED_OBJECT;

  /* Pack UDH length */
  if( const_header->u.eo.first_segment == TRUE )
  {
    udh_length = WMS_UDH_OCTETS_EO_HEADER + const_header->u.eo.content.length;
  }
  else
  {
    udh_length = const_header->u.eo.content.length;
  }

  if( udh_length > WMS_UDH_EO_DATA_SEGMENT_MAX )
  {
    LOGE("UDH EO data too long: %d", udh_length);
    return 0;
  }

  udh[pos++] = udh_length;

  /* Pack EO header for the first segment */
  if( const_header->u.eo.first_segment == TRUE )
  {
    udh[pos++] = const_header->u.eo.reference;
    udh[pos++] = const_header->u.eo.length >> 8;
    udh[pos++] = const_header->u.eo.length & 0xFF;
    udh[pos++] = const_header->u.eo.control;
    udh[pos++] = (uint8) const_header->u.eo.type;
    udh[pos++] = const_header->u.eo.position >> 8;
    udh[pos++] = const_header->u.eo.position & 0xFF;
  }

  /* Pack content data */
  memcpy( udh+pos,
          const_header->u.eo.content.data,
          const_header->u.eo.content.length );
  pos += const_header->u.eo.content.length;

  return pos;

} /* wms_ts_encode_udh_eo() */

int wms_ts_encode_udh_rfc822
(
  uint8 *udh
)
{
  int pos=0;

  udh[pos++] = (uint8) WMS_UDH_RFC822;
  udh[pos++] = (uint8) WMS_UDH_OCTETS_RFC822;
  udh[pos++] = const_header->u.rfc822.header_length;

  return pos;
}/*wms_ts_encode_udh_rfc822*/


int wms_ts_encode_udh_other
(
  uint8 *udh,
  wms_udh_id_e_type header_id
)
{
  int i=0;
  int pos =0;

  udh[pos++] = (uint8) const_header->u.other.header_id;
  udh[pos++] = const_header->u.other.header_length;

  for(i = 0; i< const_header->u.other.header_length;i++)
  {
    udh[pos++] = const_header->u.other.data[i];
  }

  return pos;
}


/*=========================================================================
FUNCTION
  wms_ts_encode_user_data_header

DESCRIPTION
  Encode User Data structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes encoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_encode_user_data_header
(
  uint8                           num_headers, /* IN */
  const wms_udh_s_type            * headers,   /* IN */
  uint8                           *data        /* OUT */
)
{
   int i,pos=0;

   if (num_headers == 0)
     return 0;

   ++pos; /*Fill the user data header length later*/


   for (i=0;i<WMS_MAX_UD_HEADERS && i< num_headers;i++)
   {
     const_header = &headers[i];
     switch(const_header->header_id)
     {
       case WMS_UDH_CONCAT_8:
         pos+= wms_ts_encode_udh_concat_8(data+pos);
         break;

       case WMS_UDH_CONCAT_16:
         pos+= wms_ts_encode_udh_concat16(data+pos);
         break;

       case WMS_UDH_SPECIAL_SM:
         pos+= wms_ts_encode_udh_special_sm(data+pos);
         break;

       case WMS_UDH_PORT_8:
         pos+= wms_ts_encode_udh_port_8(data+pos);
         break;

       case WMS_UDH_PORT_16:
         pos+= wms_ts_encode_udh_port16(data+pos);
         break;

       case WMS_UDH_TEXT_FORMATING:
         pos+= wms_ts_encode_udh_text_formatting(data+pos);
         break;


       case WMS_UDH_PRE_DEF_SOUND:
         pos+= wms_ts_encode_udh_pre_def_sound(data+pos);
         break;

       case WMS_UDH_USER_DEF_SOUND:
         pos+= wms_ts_encode_udh_user_def_sound(data+pos);
         break;

       case WMS_UDH_PRE_DEF_ANIM:
         pos+= wms_ts_encode_udh_pre_def_anim(data+pos);
         break;

       case WMS_UDH_LARGE_ANIM:
         pos+= wms_ts_encode_udh_large_anim(data+pos);
         break;

       case WMS_UDH_SMALL_ANIM:
         pos+= wms_ts_encode_udh_small_anim(data+pos);
         break;

       case WMS_UDH_LARGE_PICTURE:
         pos+= wms_ts_encode_udh_large_picture(data+pos);
         break;

       case WMS_UDH_SMALL_PICTURE:
         pos+= wms_ts_encode_udh_small_picture(data+pos);
         break;

       case WMS_UDH_VAR_PICTURE:
         pos+= wms_ts_encode_udh_var_picture(data+pos);
         break;

       case WMS_UDH_USER_PROMPT:
         pos+= wms_ts_encode_udh_user_prompt(data+pos);
         break;

       case WMS_UDH_EXTENDED_OBJECT:
         pos+= wms_ts_encode_udh_eo(data+pos);
         break;

       case WMS_UDH_RFC822:
         pos+= wms_ts_encode_udh_rfc822(data+pos);
         break;

       default:
         pos+= wms_ts_encode_udh_other(data+pos,const_header->header_id);
     }

   }
   data[0] = (uint8)(pos-1); /*User Data Header Length*/
   return ((uint8)(pos-1));

} /*wms_ts_encode_user_data_header*/

/*=========================================================================
FUNCTION
  wms_ts_encode_gw_user_data

DESCRIPTION
  Encode User Data structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes encoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_encode_gw_user_data
(
  const wms_gw_dcs_s_type         *dcs,
  const wms_gw_user_data_s_type   *user_data,
  uint8                           *data
)
{
  uint16 i, pos=0;
  uint8 fill_bits = 0;
  uint16 total_bits_occupied;

  uint8 user_data_header_length;
  uint16 user_data_length; /*User Data Length with the header*/

  data[pos] = (uint8)user_data->sm_len; /*User Data length*/
  pos++;
  if( dcs->alphabet == WMS_GW_ALPHABET_7_BIT_DEFAULT )
  {
    /* pack GSM default characters */
    if (user_data->num_headers > 0)
    {
      /* Check for Memory Corruption - data[WMS_MAX_LEN] */
      if (wms_ts_compute_user_data_header_length(user_data->num_headers, user_data->headers) <= WMS_SMS_UDL_MAX_8_BIT)
      {

        user_data_header_length = wms_ts_encode_user_data_header( user_data->num_headers,
                                              user_data->headers,
                                              data+pos);

        pos += user_data_header_length + 1;

        total_bits_occupied = (user_data_header_length + 1) * 8;
        fill_bits = (total_bits_occupied % 7);    /* fill_bits vary from 0 to 6 */

        if (fill_bits != 0)
        {
          fill_bits = 7 - fill_bits;
        }

        user_data_length = (total_bits_occupied + fill_bits + (user_data->sm_len * 7)) / 7;

        data[0] = (uint8)user_data_length;   /* UDL */
        data[1] = user_data_header_length;   /* UDHL */
      }
      else
      {
        LOGE("Encode User Data Header Exceeds Capacity - Skipping UDH");
      }
    }

    i = wms_ts_pack_gw_7_bit_chars( user_data->sm_data,
                                    user_data->sm_len,
                                    fill_bits,
                                   (uint16) (WMS_MAX_LEN - pos),
                                    & data[pos] );
    pos += i;
  }
  else
  {
    if (user_data->num_headers > 0)
    {
      /* Check for Memory Corruption - data[WMS_MAX_LEN] */
      if (wms_ts_compute_user_data_header_length(user_data->num_headers, user_data->headers) <= WMS_SMS_UDL_MAX_8_BIT)
      {
        user_data_header_length =
              wms_ts_encode_user_data_header(user_data->num_headers,
                                             user_data->headers,
                                             data+pos);

        /*TP-UDL is modified to actual user data + udhl */
        data[0] = (uint8)(user_data->sm_len + user_data_header_length +1);
        pos += user_data_header_length+1;
      }
      else
      {
        LOGE("Encode User Data Header Exceeds Capacity - Skipping UDH");
      }
    }

    memcpy( & data[pos],
            user_data->sm_data,
            user_data->sm_len );
    pos += user_data->sm_len;
  }

  return (uint8)pos;

} /* wms_ts_encode_gw_user_data() */


/*=========================================================================
FUNCTION
  wms_ts_encode_gw_validity

DESCRIPTION
  Encode Validity structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes encoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_encode_gw_validity
(
  const wms_gw_validity_s_type      *validity,
  uint8                               *data
)
{
  uint8 i, pos = 0;

  switch( validity->format )
  {
    case WMS_GW_VALIDITY_NONE:
      break;

    case WMS_GW_VALIDITY_RELATIVE:
      data[pos] = wms_ts_encode_relative_time( & validity->u.time );
      pos ++;
      break;

    case WMS_GW_VALIDITY_ABSOLUTE:
      i = wms_ts_encode_timestamp( & validity->u.time, data+pos );
      if( i == 0 )
      {
        LOGE("Error while Decoding Absolute Validity Timestamp");
      }
      pos += i;
      break;

    case WMS_GW_VALIDITY_ENHANCED:
      // TBD
      break;

    default:
      break;
  } /* switch */

  return pos;

} /* wms_ts_encode_gw_validity() */


/*=========================================================================
FUNCTION
  wms_ts_encode_deliver

DESCRIPTION
  Encode Deliver TPDU into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Encoding status.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode_deliver
(
  const wms_gw_deliver_s_type   *deliver,
  wms_raw_ts_data_s_type        *raw_ts_data_ptr
)
{
  wms_status_e_type     st = WMS_OK_S;
  uint8                *data;
  uint8                 pos = 0, i;

  if( deliver == NULL || raw_ts_data_ptr == NULL )
  {
    LOGE("Null pointer in wms_ts_encode_deliver!");
    return WMS_NULL_PTR_S;
  }
  
  data = raw_ts_data_ptr->data;

  /* Setting Data to 0 */
  (void)memset(data, 0, WMS_MAX_LEN);

  /* TP-MTI, TP-MMS, TP-SRI, TP_UDHI, TP-RP:
  */
  data[pos] = 0x00; /* DELIVER: bits 0, 1 */
  data[pos] |= deliver->more ? 0 : 0x04; /* bit 2 */
  /* bits 3, 4 are not used */
  data[pos] |= deliver->status_report_enabled ? 0x20 : 0; /* bit 5 */
  data[pos] |= deliver->user_data_header_present ? 0x40 : 0; /* bit 6 */
  data[pos] |= deliver->reply_path_present ? 0x80 : 0;    /* bit 7 */
  pos ++;

  /* TP-OA
  */
  i = wms_ts_encode_address( & deliver->address, & data[pos] );
  if( i==0 )
  {
    return WMS_INVALID_PARM_SIZE_S;
  }
  pos += i;

  /* TP-PID
  */
  data[pos] = deliver->pid;
  pos ++;

  /* TP-DCS
  */
  pos += wms_ts_encode_dcs( & deliver->dcs, data+pos );

  /* TP-SCTS
  */
  i = wms_ts_encode_timestamp( & deliver->timestamp, data+pos );
  if( i==0 )
  {
    return WMS_INVALID_PARM_VALUE_S;
  }
  pos += i;

  /* TP-UDL
  */
 /* data[pos] = (uint8)deliver->user_data.sm_len;
    pos ++;*/

 /*TP_UDL is filled in encode_gw_user_data function*/
  /* TP-UD
  */
  if (wms_ts_compute_gw_user_data_length( &deliver->dcs, &deliver->user_data) > WMS_SMS_UDL_MAX_8_BIT)
  {
    LOGE("User Data Length has exceeded capacity");
    st = WMS_INVALID_USER_DATA_SIZE_S;
  }
  else
  {
    i = wms_ts_encode_gw_user_data( & deliver->dcs,
                                    & deliver->user_data,
                                      data+pos );
    pos += i;
  }

  raw_ts_data_ptr->tpdu_type  = WMS_TPDU_DELIVER;
  raw_ts_data_ptr->len        = pos;

  return st;

} /* wms_ts_encode_deliver() */


/*=========================================================================
FUNCTION
  wms_ts_encode_deliver_report_ack

DESCRIPTION
  Encode Deliver-Report-Ack structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of encoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode_deliver_report_ack
(
  const wms_gw_deliver_report_ack_s_type    *deliver_report_ack,
  wms_raw_ts_data_s_type                    *raw_ts_data_ptr
)
{
  wms_status_e_type     st = WMS_OK_S;
  uint8                 *data;
  uint8                 i, pos = 0;

  if( deliver_report_ack == NULL || raw_ts_data_ptr == NULL )
  {
    LOGE("Null pointer in wms_ts_encode_deliver_report_ack!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* Setting Data to 0 */
  (void)memset(data, 0, WMS_MAX_LEN);

  /* TP-MTI, TP_UDHI:
  */
  data[pos] = 0x00; /* DELIVER-REPORT: bits 0, 1 */
  data[pos] |= deliver_report_ack->user_data_header_present ? 0x40 : 0;
               /* bit 6 */
  pos ++;


  /* TP-PI
  */
  data[pos] = (uint8) deliver_report_ack->mask;
  pos ++;

  /* TP-PID
  */
  if( deliver_report_ack->mask & WMS_TPDU_MASK_PID )
  {
    data[pos] = deliver_report_ack->pid;
    pos ++;
  }

  /* TP-DCS
  */
  if( deliver_report_ack->mask & WMS_TPDU_MASK_DCS )
  {
    pos += wms_ts_encode_dcs( & deliver_report_ack->dcs, data+pos );
  }

  if( deliver_report_ack->mask & WMS_TPDU_MASK_USER_DATA )
  {
    /* TP-UDL
    */
    /*data[pos] =(uint8) deliver_report_ack->user_data.sm_len;
      pos ++;*/

    /*TP_UDL is filled in encode_gw_user_data function*/

    /* TP-UD
    */
    if (wms_ts_compute_gw_user_data_length( &deliver_report_ack->dcs, &deliver_report_ack->user_data) > WMS_SMS_UDL_MAX_8_BIT)
    {
      LOGE("User Data Length has exceeded capacity");
      st = WMS_INVALID_USER_DATA_SIZE_S;
    }
    else
    {
      i = wms_ts_encode_gw_user_data( & deliver_report_ack->dcs,
                                      & deliver_report_ack->user_data,
                                        data+pos );
      pos += i;
    }
  }

  raw_ts_data_ptr->tpdu_type  = WMS_TPDU_DELIVER_REPORT_ACK;
  raw_ts_data_ptr->len        = pos;

  return st;

} /* wms_ts_encode_deliver_report_ack() */


/*=========================================================================
FUNCTION
  wms_ts_encode_deliver_report_error

DESCRIPTION
  Encode Deliver-Report-Error structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of encoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode_deliver_report_error
(
  const wms_gw_deliver_report_error_s_type    *deliver_report_error,
  wms_raw_ts_data_s_type                      *raw_ts_data_ptr
)
{
  wms_status_e_type     st = WMS_OK_S;
  uint8                 *data;
  uint8                 i, pos = 0;

  if( deliver_report_error == NULL || raw_ts_data_ptr == NULL )
  {
    LOGE("Null pointer in wms_ts_encode_deliver_report_error!");
    return WMS_NULL_PTR_S;
  }
  
  data = raw_ts_data_ptr->data;

  /* Setting Data to 0 */
  (void)memset(data, 0, WMS_MAX_LEN);

  /* TP-MTI, TP_UDHI:
  */
  data[pos] = 0x00; /* DELIVER-REPORT: bits 0, 1 */
  data[pos] |= deliver_report_error->user_data_header_present ? 0x40 : 0;
               /* bit 6 */
  pos ++;

  /* TP-FCS
  */
  data[pos] = deliver_report_error->tp_cause;
  pos ++;

  /* TP-PI
  */
  data[pos] = (uint8) deliver_report_error->mask;
  pos ++;

  /* TP-PID
  */
  if( deliver_report_error->mask & WMS_TPDU_MASK_PID )
  {
    data[pos] = deliver_report_error->pid;
    pos ++;
  }

  /* TP-DCS
  */
  if( deliver_report_error->mask & WMS_TPDU_MASK_DCS )
  {
    pos += wms_ts_encode_dcs( & deliver_report_error->dcs, data+pos );
  }

  if( deliver_report_error->mask & WMS_TPDU_MASK_USER_DATA)
  {
    /* TP-UDL
    */
    /*data[pos] =(uint8) deliver_report_error->user_data.sm_len;
      pos ++;*/

    /*TP_UDL is filled in encode_gw_user_data function*/

    /* TP-UD
    */
    if (wms_ts_compute_gw_user_data_length( &deliver_report_error->dcs, &deliver_report_error->user_data) > WMS_SMS_UDL_MAX_8_BIT)
    {
      LOGE("User Data Length has exceeded capacity");
      st = WMS_INVALID_USER_DATA_SIZE_S;
    }
    else
    {
      i = wms_ts_encode_gw_user_data( & deliver_report_error->dcs,
                                      & deliver_report_error->user_data,
                                        data+pos );
      pos += i;
    }
  }

  raw_ts_data_ptr->tpdu_type  = WMS_TPDU_DELIVER_REPORT_ERROR;
  raw_ts_data_ptr->len        = pos;

  return st;

} /* wms_ts_encode_deliver_report_error() */

/*=========================================================================
FUNCTION
  wms_ts_encode_status_report

DESCRIPTION
  Encode Status Report TPDU structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of encoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode_status_report
(
  const wms_gw_status_report_s_type    *status_report,
  wms_raw_ts_data_s_type               *raw_ts_data_ptr
)
{
  wms_status_e_type     st = WMS_OK_S;
  uint8                 *data;
  uint8                 pos = 0, i;

  if( status_report == NULL || raw_ts_data_ptr == NULL )
  {
    LOGE("Null pointer in wms_ts_encode_status_report!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* Setting Data to 0 */
  (void)memset(data, 0, WMS_MAX_LEN);

  /* TP-MTI, TP_UDHI:
  */
  data[pos] = 0x10; /* Status Report bits 0, 1 */
  data[pos] |= status_report->more ? 0 : 0x04;/*bit 2*/
  data[pos] |= status_report->status_report_qualifier ? 0x20 : 0; /*bit 5*/
  data[pos] |= status_report->user_data_header_present ? 0x40 : 0;
               /* bit 6 */
  pos ++;

  /* TP-MR
  */
  data[pos] = (uint8) status_report->message_reference;
  pos ++;

  /* TP-RA
  */
  i = wms_ts_encode_address( & status_report->address, & data[pos] );
  if( i==0 )
  {
    return WMS_INVALID_PARM_SIZE_S;
  }
  pos += i;


  /* TP-SCTS
  */
  i = wms_ts_encode_timestamp( & status_report->timestamp, data+pos );
  if( i==0 )
  {
    return WMS_INVALID_PARM_VALUE_S;
  }
  pos += i;

  /* TP-DT*/
  i = wms_ts_encode_timestamp( & status_report->discharge_time, data+pos );
  if( i==0 )
  {
    return WMS_INVALID_PARM_VALUE_S;
  }
  pos += i;

  /*TP-ST*/
  data[pos] = status_report->tp_status;
  pos++;

  /*TP-PI*/
  data[pos] = (uint8) status_report->mask;
  pos ++;

  /*TP-PID*/
  if( status_report->mask & WMS_TPDU_MASK_PID )
  {
    data[pos] = status_report->pid;
    pos ++;
  }

  /* TP-DCS
  */
  if( status_report->mask & WMS_TPDU_MASK_DCS )
  {
    pos += wms_ts_encode_dcs( & status_report->dcs, data+pos );
  }

  if( status_report->mask & WMS_TPDU_MASK_USER_DATA)
  {
    /* TP-UDL
    */
    /*data[pos] =(uint8) deliver_report_error->user_data.sm_len;
      pos ++;*/

    /*TP_UDL is filled in encode_gw_user_data function*/

    /* TP-UD
    */
    if (wms_ts_compute_gw_user_data_length( &status_report->dcs, &status_report->user_data) > WMS_SMS_UDL_MAX_8_BIT)
    {
      LOGE("User Data Length has exceeded capacity");
      st = WMS_INVALID_USER_DATA_SIZE_S;
    }
    else
    {
      i = wms_ts_encode_gw_user_data( & status_report->dcs,
                                      & status_report->user_data,
                                        data+pos );
      pos += i;
    }
  }

  raw_ts_data_ptr->tpdu_type  = WMS_TPDU_STATUS_REPORT;
  raw_ts_data_ptr->len        = pos;

  return st;

} /* wms_ts_encode_status_report() */

/*=========================================================================
FUNCTION
  wms_ts_encode_command

DESCRIPTION
  Encode Command TPDU structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of encoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode_command
(
  const wms_gw_command_s_type          *command,
  wms_raw_ts_data_s_type               *raw_ts_data_ptr
)
{
  wms_status_e_type   st = WMS_OK_S;
  uint8                 *data;
  uint8                 pos = 0, i;

  if( command == NULL || raw_ts_data_ptr == NULL )
  {
    LOGE("Null pointer in wms_ts_encode_command!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* Setting Data to 0 */
  (void)memset(data, 0, WMS_MAX_LEN);

  /* TP-MTI, TP_UDHI:
  */
  data[pos] = 0x2; /* SMS Command bits 0, 1 */
  data[pos] |= command->status_report_enabled ? 0x20 :0; /*bit 5*/
  data[pos] |= command->user_data_header_present ? 0x40 : 0;
               /* bit 6 */
  pos ++;

  /* TP-MR
  */
  data[pos] = (uint8) command->message_reference;
  pos ++;

  /*TP-PID
  */
  data[pos] = command->pid;
  pos ++;

  /*TP-CT
  */
  data[pos] = command->command;
  pos ++;

  /*TP-MN
  */
  data[pos] = (uint8) command->message_number;
  pos ++;

  /* TP-DA
  */
  i = wms_ts_encode_address( & command->address, & data[pos] );
  if( i==0 )
  {
    return WMS_INVALID_PARM_SIZE_S;
  }
  pos += i;


  /* TP-CDL
  */
  /* Guard against Buffer Overflow */
  if (WMS_GW_COMMAND_DATA_MAX < command->command_data_len)
  {
     LOGE("Command Data Length %d Exceeds Command Data Buffer!",
          command->command_data_len);
     data[pos] = WMS_GW_COMMAND_DATA_MAX;
  }
  else
  {
     data[pos] = command->command_data_len;
  }

  pos ++;

  /*TP_CD
  */
  memcpy(data+pos, command->command_data, MIN(data[pos-1], WMS_MAX_LEN-pos));

  pos += command->command_data_len;

  raw_ts_data_ptr->tpdu_type  = WMS_TPDU_COMMAND;
  raw_ts_data_ptr->len        = pos;

  return st;

} /* wms_ts_encode_command() */

/*=========================================================================
FUNCTION
  wms_ts_encode_submit

DESCRIPTION
  Encode Submit TPDU structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of encoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode_submit
(
  const wms_gw_submit_s_type    *submit,
  wms_raw_ts_data_s_type        *raw_ts_data_ptr
)
{
  wms_status_e_type     st = WMS_OK_S;
  uint8                *data;
  uint8                 pos = 0, i;

  if( submit == NULL || raw_ts_data_ptr == NULL )
  {
    LOGE("Null pointer in wms_ts_encode_submit!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* Setting Data to 0 */
  (void)memset(data, 0, WMS_MAX_LEN);

  /* TP-MTI, TP-RD, TP-VPF, TP-SRR, TP_UDHI, TP-RP:
  */
  data[pos] = 0x01; /* SUBMIT: bits 0, 1 */
  data[pos] |= submit->reject_duplicates ? 0x04 : 0; /* bit 2 */

  if (submit->validity.format > 3 )
  {
    return st = WMS_INVALID_VALIDITY_FORMAT_S;
  }
  data[pos] |= submit->validity.format << 3;                /* bits 3, 4 */

  data[pos] |= submit->status_report_enabled ? 0x20 : 0;    /* bit 5 */
  data[pos] |= submit->user_data_header_present ? 0x40 : 0; /* bit 6 */
  data[pos] |= submit->reply_path_present ? 0x80 : 0;       /* bit 7 */
  pos ++;

  /* TP-MR
  */
  data[pos] = (uint8) submit->message_reference;
  pos ++;

  /* TP-DA
  */
  i = wms_ts_encode_address( & submit->address, & data[pos] );
  if( i==0 )
  {
    return WMS_INVALID_PARM_SIZE_S;
  }
  pos += i;


  /* TP-PID
  */
  data[pos] = submit->pid;
  pos ++;

  /* TP-DCS
  */
  pos += wms_ts_encode_dcs( & submit->dcs, data+pos );

  /* TP-VP
  */
  pos += wms_ts_encode_gw_validity( & submit->validity, data+pos );

  /* TP-UDL
  */
  /*data[pos] =(uint8) submit->user_data.sm_len;
    pos ++;*/

  /*TP_UDL is filled in encode_gw_user_data function*/

  /* TP-UD
  */

  if (wms_ts_compute_gw_user_data_length( &submit->dcs, &submit->user_data) > WMS_SMS_UDL_MAX_8_BIT)
  {
    LOGE("User Data Length has exceeded capacity");
    st = WMS_INVALID_USER_DATA_SIZE_S;
  }
  else
  {
    i = wms_ts_encode_gw_user_data( & submit->dcs,
                                    & submit->user_data,
                                      data+pos );
    pos += i;
  }

  raw_ts_data_ptr->tpdu_type  = WMS_TPDU_SUBMIT;
  raw_ts_data_ptr->len        = pos;

  return st;

} /* wms_ts_encode_submit() */


/*=========================================================================
FUNCTION
  wms_ts_encode_submit_report_ack

DESCRIPTION
  Encode Submit-Report-Ack structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of encoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode_submit_report_ack
(
  const wms_gw_submit_report_ack_s_type     *submit_report_ack,
  wms_raw_ts_data_s_type                    *raw_ts_data_ptr
)
{
  wms_status_e_type   st = WMS_OK_S;
  uint8                 *data;
  uint8                 pos = 0, i;

  if( submit_report_ack == NULL || raw_ts_data_ptr == NULL )
  {
    LOGE("Null pointer in wms_ts_encode_submit_report_ack!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* Setting Data to 0 */
  (void)memset(data, 0, WMS_MAX_LEN);

  /* TP-MTI, TP_UDHI:
  */
  data[pos] = 0x01; /* SUBMIT-REPORT: bits 0, 1 */
  data[pos] |= submit_report_ack->user_data_header_present ? 0x40 : 0;
               /* bit 6 */
  pos ++;

  /* TP-PI
  */
  data[pos] = (uint8) submit_report_ack->mask;
  pos ++;

  /* TP-SCTS
  */
  i = wms_ts_encode_timestamp( & submit_report_ack->timestamp, data+pos );

  if( i==0 )
  {
    return WMS_INVALID_PARM_VALUE_S;
  }
  pos += i;

  /* TP-PID
  */
  if( submit_report_ack->mask & WMS_TPDU_MASK_PID )
  {
    data[pos] = submit_report_ack->pid;
    pos ++;
  }

  /* TP-DCS
  */
  if( submit_report_ack->mask & WMS_TPDU_MASK_DCS )
  {
    pos += wms_ts_encode_dcs( & submit_report_ack->dcs, data+pos );
  }

  /* TP-UDL
  */
  if( submit_report_ack->mask & WMS_TPDU_MASK_USER_DATA )
  {
    /*data[pos] =(uint8) submit_report_ack->user_data.sm_len;
      pos ++;*/

    /*TP_UDL is filled in encode_gw_user_data function*/
    /* TP-UD
    */
    if (wms_ts_compute_gw_user_data_length( &submit_report_ack->dcs, &submit_report_ack->user_data) > WMS_SMS_UDL_MAX_8_BIT)
    {
      LOGE("User Data Length has exceeded capacity");
      st = WMS_INVALID_USER_DATA_SIZE_S;
    }
    else
    {
      i = wms_ts_encode_gw_user_data( & submit_report_ack->dcs,
                                      & submit_report_ack->user_data,
                                        data+pos );
      pos += i;
    }
  }

  raw_ts_data_ptr->tpdu_type  = WMS_TPDU_SUBMIT_REPORT_ACK;
  raw_ts_data_ptr->len        = pos;

  return st;

} /* wms_ts_encode_submit_report_ack() */


/*=========================================================================
FUNCTION
  wms_ts_encode_submit_report_error

DESCRIPTION
  Encode Submit-Report-Error structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of encoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode_submit_report_error
(
  const wms_gw_submit_report_error_s_type     *submit_report_error,
  wms_raw_ts_data_s_type                      *raw_ts_data_ptr
)
{
  wms_status_e_type   st = WMS_OK_S;
  uint8                 *data;
  uint8                 pos = 0, i;

  if( submit_report_error == NULL || raw_ts_data_ptr == NULL )
  {
    LOGE("Null pointer in wms_ts_encode_submit_report_error!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* Setting Data to 0 */
  (void)memset(data, 0, WMS_MAX_LEN);

  /* TP-MTI, TP_UDHI:
  */
  data[pos] = 0x01; /* SUBMIT-REPORT */
  data[pos] |= submit_report_error->user_data_header_present ? 0x40 : 0;
               /* bit 6 */
  pos ++;

  /* TP-FCS
  */
  data[pos] = submit_report_error->tp_cause;
  pos ++;

  /* TP-PI
  */
  data[pos] = (uint8) submit_report_error->mask;
  pos ++;

  /* TP-SCTS
  */
  i = wms_ts_encode_timestamp( & submit_report_error->timestamp, data+pos );
  if( i==0 )
  {
    return WMS_INVALID_PARM_VALUE_S;
  }
  pos += i;

  /* TP-PID
  */
  if( submit_report_error->mask & WMS_TPDU_MASK_PID )
  {
    data[pos] = submit_report_error->pid;
    pos ++;
  }

  /* TP-DCS
  */
  if( submit_report_error->mask & WMS_TPDU_MASK_DCS )
  {
    pos += wms_ts_encode_dcs( & submit_report_error->dcs, data+pos );
  }

  /* TP-UDL
  */
  if( submit_report_error->mask & WMS_TPDU_MASK_USER_DATA )
  {
    /*data[pos] =(uint8) submit_report_error->user_data.sm_len;
      pos ++;*/

    /*TP_UDL is filled in encode_gw_user_data function*/
    /* TP-UD
    */
    if (wms_ts_compute_gw_user_data_length( &submit_report_error->dcs, &submit_report_error->user_data) > WMS_SMS_UDL_MAX_8_BIT)
    {
      LOGE("User Data Length has exceeded capacity");
      st = WMS_INVALID_USER_DATA_SIZE_S;
    }
    else
    {
      i = wms_ts_encode_gw_user_data( & submit_report_error->dcs,
                                      & submit_report_error->user_data,
                                        data+pos );
      pos += i;
    }
  }

  raw_ts_data_ptr->tpdu_type  = WMS_TPDU_SUBMIT_REPORT_ERROR;
  raw_ts_data_ptr->len        = pos;

  return st;

} /* wms_ts_encode_submit_report_error() */

uint8 wms_ts_encode_gw_cb_page_header
(
  const wms_gw_cb_page_header_s_type  * page_header_ptr,
  uint8                               * data
)
{
  uint8             pos = 0;

  if ( page_header_ptr == NULL || data == NULL)
  {
    return 0;
  }

  /* Serial Number */
  data[pos] = (uint8)((page_header_ptr->raw_serial_number & 0xFF00)>>8);
  pos++;
  data[pos] = (uint8)(page_header_ptr->raw_serial_number & 0x00FF);
  pos++;

  /* Message Identifier */
  data[pos] = (uint8)((page_header_ptr->cb_srv_id & 0xFF00)>>8);
  pos++;
  data[pos] = (uint8)(page_header_ptr->cb_srv_id & 0x00FF);
  pos++;

  data[pos] = page_header_ptr->cb_dcs.raw_dcs_data;
  pos++;

  data[pos] = (uint8)(((page_header_ptr->page_number & 0x0F)<<4)|(page_header_ptr->total_pages & 0x0F));
  pos++;

  return pos;

} /* wms_ts_encode_gw_cb_page_header() */

wms_status_e_type wms_ts_encode_gw_cb
(
  const wms_gw_cb_ts_data_s_type    * client_ts_ptr,   /* IN */
  wms_raw_ts_data_s_type            * raw_ts_data_ptr  /* OUT */
)
{
  wms_status_e_type   st = WMS_OK_S;

  if( client_ts_ptr == NULL || raw_ts_data_ptr == NULL )
  {
    st = WMS_NULL_PTR_S;
  }
  else if( wms_ts_encode_gw_cb_page_header(&client_ts_ptr->cb_page_hdr, raw_ts_data_ptr->data) == 0 )
  {
    st = WMS_INVALID_CB_DATA_S;
  }
  else
  {
    /* Process user data part */
    switch( client_ts_ptr->cb_page_hdr.cb_dcs.alphabet )
    {
      case WMS_GW_ALPHABET_7_BIT_DEFAULT:
        raw_ts_data_ptr->len = WMS_GW_CB_PAGE_HEADER_LEN;
        gw_user_data.num_headers = 0;
        gw_user_data.sm_len = 0;
        if (client_ts_ptr->cb_page_hdr.cb_dcs.language == WMS_GW_CB_LANGUAGE_ISO_639)
        {
          /* First 3 characters of user data are Language */
          memcpy(gw_user_data.sm_data, client_ts_ptr->cb_page_hdr.cb_dcs.iso_639_lang, 3);
          gw_user_data.sm_len += 3;
        }
        memcpy( gw_user_data.sm_data + gw_user_data.sm_len,
                client_ts_ptr->user_data.sm_data,
                client_ts_ptr->user_data.sm_len);

        gw_user_data.sm_len += client_ts_ptr->user_data.sm_len;

        raw_ts_data_ptr->len += wms_ts_pack_gw_7_bit_chars( gw_user_data.sm_data,
                                    gw_user_data.sm_len,
                                    0, /* shift */
                                    WMS_MAX_LEN,
                                    raw_ts_data_ptr->data + WMS_GW_CB_PAGE_HEADER_LEN
                                  );
        break;

      default:
        raw_ts_data_ptr->len = WMS_GW_CB_PAGE_HEADER_LEN;
        if (client_ts_ptr->cb_page_hdr.cb_dcs.language == WMS_GW_CB_LANGUAGE_ISO_639)
        {
          /* First 2 characters of user data are Language */
          memcpy(raw_ts_data_ptr->data, client_ts_ptr->cb_page_hdr.cb_dcs.iso_639_lang, 2);
          raw_ts_data_ptr->len += 2;
        }
        memcpy( raw_ts_data_ptr->data + raw_ts_data_ptr->len,
                client_ts_ptr->user_data.sm_data,
                client_ts_ptr->user_data.sm_len);

        raw_ts_data_ptr->len += client_ts_ptr->user_data.sm_len;
        break;

    }
    raw_ts_data_ptr->format = WMS_FORMAT_GW_CB;
  }
  return st;
} /* wms_ts_encode_gw_cb */


/*=========================================================================
FUNCTION
  wms_ts_encode

DESCRIPTION
  Encode Transport Service structure into raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of encoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_encode
(
  const wms_client_ts_data_s_type         * client_ts_data_ptr,
  wms_raw_ts_data_s_type                  * raw_ts_data_ptr
)
{
  wms_status_e_type   st = WMS_OK_S;
  const wms_gw_pp_ts_data_s_type *msg;

  if( client_ts_data_ptr == NULL || raw_ts_data_ptr == NULL )
  {
    LOGE("Null pointer in wms_ts_encode!");
    return WMS_NULL_PTR_S;
  }

  msg = & client_ts_data_ptr->u.gw_pp;

  pthread_mutex_lock( &wmsts_mutex );
  LOGI(">>>>>>>>>> pthread_mutex_lock() : wmsts_mutex >>>>>>>>>>\n" );

  switch( client_ts_data_ptr->format )
  {
    case WMS_FORMAT_CDMA:
    case WMS_FORMAT_ANALOG_AWISMS:
    case WMS_FORMAT_ANALOG_CLI:
    case WMS_FORMAT_ANALOG_VOICE_MAIL:
    case WMS_FORMAT_ANALOG_SMS:
    case WMS_FORMAT_MWI:
      st = wms_ts_encode_bearer_data( & client_ts_data_ptr->u.cdma,
                                      raw_ts_data_ptr );
      break;

    case WMS_FORMAT_GW_PP:
      /* ---------- Start encoding --------- */

      raw_ts_data_ptr->tpdu_type    = msg->tpdu_type;

      switch( msg->tpdu_type )
      {
        case WMS_TPDU_DELIVER:
          st = wms_ts_encode_deliver( & msg->u.deliver, raw_ts_data_ptr );
          break;


        case WMS_TPDU_DELIVER_REPORT_ACK:
          st = wms_ts_encode_deliver_report_ack( & msg->u.deliver_report_ack,
                                                   raw_ts_data_ptr );
          break;

        case WMS_TPDU_DELIVER_REPORT_ERROR:
          st = wms_ts_encode_deliver_report_error( & msg->u.deliver_report_error,
                                                     raw_ts_data_ptr );
          break;

        case WMS_TPDU_SUBMIT:
          st = wms_ts_encode_submit( & msg->u.submit, raw_ts_data_ptr );
          break;


        case WMS_TPDU_SUBMIT_REPORT_ACK:
          st = wms_ts_encode_submit_report_ack( & msg->u.submit_report_ack,
                                                  raw_ts_data_ptr );
          break;


        case WMS_TPDU_SUBMIT_REPORT_ERROR:
          st = wms_ts_encode_submit_report_error( & msg->u.submit_report_error,
                                                    raw_ts_data_ptr );
          break;


        case WMS_TPDU_STATUS_REPORT:
          st = wms_ts_encode_status_report( & msg->u.status_report,
                                              raw_ts_data_ptr );
          break;

        case WMS_TPDU_COMMAND:
          st = wms_ts_encode_command( & msg->u.command,
                                        raw_ts_data_ptr );
          break;

        default:
          LOGE("Invalid TPDU type %d", msg->tpdu_type);
          st = WMS_INVALID_TPDU_TYPE_S;
          break;
      }

      /* ---------- End of encoding --------- */
      break;

      case WMS_FORMAT_GW_CB:
        st = wms_ts_encode_gw_cb(&client_ts_data_ptr->u.gw_cb,
                                  raw_ts_data_ptr);
        break;

      default:
        st = WMS_INVALID_FORMAT_S;
        ERR("Invalid format: %d", client_ts_data_ptr->format, 0,0);
        break;
  }

  raw_ts_data_ptr->format = client_ts_data_ptr->format;

  LOGI(">>>>>>>>>> pthread_mutex_unlock() : wmsts_mutex >>>>>>>>>>\n" );
  pthread_mutex_unlock( &wmsts_mutex );

  return st;

} /* wms_ts_encode() */

/*=========================================================================
FUNCTION
  wms_ts_decode_dcs

DESCRIPTION
  Decode Data Coding Scheme from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_decode_dcs
(
  const uint8           *data,
  wms_gw_dcs_s_type     *dcs
)
{
  uint8 pos = 0;
  uint8 i;
  /* initialize the values */

  if (data == NULL || dcs == NULL)
  {
    LOGE("null pointer in wms_ts_decode_dcs");
    return 0;
  }

  dcs->msg_class          = WMS_MESSAGE_CLASS_NONE;
  dcs->msg_waiting    = WMS_GW_MSG_WAITING_NONE;
  dcs->alphabet       = WMS_GW_ALPHABET_7_BIT_DEFAULT;
  dcs->is_compressed  = FALSE;


  /* bits 7-6 */
  i = ( data[pos] & 0xC0 ) >> 6;
  switch( i )
  {
    case 0:
      /* pattern 00xx xxxx */
      dcs->is_compressed = data[pos] & 0x20;
      if( data[pos] & 0x10 )
      {
        dcs->msg_class = (wms_message_class_e_type) (data[pos] & 0x03);
      }
      else
      {
        /* no class information */
        dcs->msg_class = WMS_MESSAGE_CLASS_NONE;
      }
      dcs->alphabet = (wms_gw_alphabet_e_type) (( data[pos] & 0x0C ) >> 2);
      break;

    case 3:
      /* bits 5-4 */
      if( (data[pos] & 0x30) == 0x30 )
      {
        /* pattern 1111 xxxx */

        /* bit 3 is reserved */

        /* bit 2 */
        dcs->alphabet = (data[pos] & 0x04 ) ? WMS_GW_ALPHABET_8_BIT:
                            WMS_GW_ALPHABET_7_BIT_DEFAULT;

        /* bits 1-0 */
        dcs->msg_class = (wms_message_class_e_type) (data[pos] & 0x03);

        /* set remaining fields */
        dcs->is_compressed  = FALSE;
        dcs->msg_waiting    = WMS_GW_MSG_WAITING_NONE_1111;
      }
      else
      {
        /* Message waiting groups
        */
        dcs->is_compressed  = FALSE;
        dcs->msg_class          = WMS_MESSAGE_CLASS_NONE;

        /* bits 5-4 */
        if( (data[pos] & 0x30) == 0x00 )
        {
          dcs->msg_waiting  = WMS_GW_MSG_WAITING_DISCARD;
          dcs->alphabet     = WMS_GW_ALPHABET_7_BIT_DEFAULT;
        }
        else if( (data[pos] & 0x30) == 0x10 )
        {
          dcs->msg_waiting  = WMS_GW_MSG_WAITING_STORE;
          dcs->alphabet     = WMS_GW_ALPHABET_7_BIT_DEFAULT;
        }
        else
        {
          dcs->msg_waiting  = WMS_GW_MSG_WAITING_STORE;
          dcs->alphabet     = WMS_GW_ALPHABET_UCS2;
        }

        /* bit 3 */
        dcs->msg_waiting_active = ( data[pos] & 0x08 ) ? TRUE : FALSE;

        /* bit 2 is reserved */

        /* bits 1-0 */
        dcs->msg_waiting_kind = (wms_gw_msg_waiting_kind_e_type) (data[pos] & 0x03);
      }
      break;

    default:
      // reserved values
      LOGE("Invalid DCS: %x", data[pos]);
      dcs->alphabet       = WMS_GW_ALPHABET_7_BIT_DEFAULT;
      dcs->is_compressed  = FALSE;
      dcs->msg_waiting    = WMS_GW_MSG_WAITING_NONE;
      dcs->msg_class          = WMS_MESSAGE_CLASS_NONE;
      break;
  }

  if ( dcs->alphabet > WMS_GW_ALPHABET_UCS2 )
  {
    dcs->alphabet = WMS_GW_ALPHABET_7_BIT_DEFAULT;
  }

  dcs->raw_dcs_data = data[pos];

  pos ++;

  return pos;

} /* wms_ts_decode_dcs() */

/*=========================================================================
FUNCTION
  wms_ts_decode_timestamp

DESCRIPTION
  Decode a timestamp from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_decode_timestamp
(
  const uint8             *data,
  wms_timestamp_s_type  *timestamp
)
{
  uint8 pos = 0, i, j;

  if (data == NULL || timestamp == NULL)
  {
    LOGE("null pointer in wms_ts_decode_timestamp");
    return 0;
  }

  /*year check*/

  /* in GW, swap the order of LSB, MSB */
  i = ((data[pos] & 0x0F) << 4) + ((data[pos] & 0xF0) >> 4);
  if ( !wms_ts_bcd_to_int(i, &j) )
  {
    LOGE("Invalid BCD Digits in Encoded Timestamp Year : %d",data[pos]);
    i = 0;  /* Modifying it to a Good Value */
  }
  timestamp->year = i;
  pos ++;

  /*month check*/
  i = ((data[pos] & 0x0F) << 4) + ((data[pos] & 0xF0) >> 4);
  if ( wms_ts_bcd_to_int(i, &j) )
  {
    if (j > 12 || j < 1)
    {
      LOGE("Month is invalid: %d", j);
      i = 1;  /* Modifying it to a Good Value */
    }
  }
  else
  {
    LOGE("Invalid BCD Digits in Encoded Timestamp Month : %d",data[pos]);
    i = 1;  /* Modifying it to a Good Value */
  }
  timestamp->month = i;
  pos ++;

  /*day check*/
  i = ((data[pos] & 0x0F) << 4) + ((data[pos] & 0xF0) >> 4);
  if ( wms_ts_bcd_to_int(i, &j) )
  {
    if (j > 31 || j < 1)
    {
      LOGE("Day is invalid: %d", j);
      i = 1;  /* Modifying it to a Good Value */
    }
  }
  else
  {
    LOGE("Invalid BCD Digits in Encoded Timestamp Day : %d",data[pos]);
    i = 1;  /* Modifying it to a Good Value */
  }
  timestamp->day = i;
  pos ++;

  /*hour check*/
  i = ((data[pos] & 0x0F) << 4) + ((data[pos] & 0xF0) >> 4);
  if ( wms_ts_bcd_to_int(i, &j) )
  {
    if (j > 23)
    {
      LOGE("Hour is too large: %d", j);
      i = 0;  /* Modifying it to a Good Value */
    }
  }
  else
  {
    LOGE("Invalid BCD Digits in Encoded Timestamp Hour : %d",data[pos]);
    i = 0;  /* Modifying it to a Good Value */
  }
  timestamp->hour = i;
  pos ++;

  /*minute check*/
  i = ((data[pos] & 0x0F) << 4) + ((data[pos] & 0xF0) >> 4);
  if ( wms_ts_bcd_to_int(i, &j) )
  {
    if (j > 59)
    {
      LOGE("Minute is too large: %d", j);
      i = 0;  /* Modifying it to a Good Value */
    }
  }
  else
  {
    LOGE("Invalid BCD Digits in Encoded Timestamp Minute : %d",data[pos]);
    i = 0;  /* Modifying it to a Good Value */
  }
  timestamp->minute = i;
  pos ++;

  /*seconds check*/
  i = ((data[pos] & 0x0F) << 4) + ((data[pos] & 0xF0) >> 4);
  if ( wms_ts_bcd_to_int(i, &j) )
  {
    if (j > 59)
    {
      LOGE("Second is too large: %d", i);
      i = 0;  /* Modifying it to a Good Value */
    }
  }
  else
  {
    LOGE("Invalid BCD Digits in Encoded Timestamp Second : %d",data[pos]);
    i = 0;  /* Modifying it to a Good Value */
  }
  timestamp->second = i;
  pos ++;

  /*timezone, special case where timestamp->timezone is an integer value*/
  if (data[pos] & 0x08)
  {
    timestamp->timezone = (data[pos] & 0x07) * 10 + ((data[pos] & 0xF0)>>4);
    timestamp->timezone *= (-1);
  }
  else
  {
    timestamp->timezone = (sint7)((data[pos] & 0x0F) * 10 + ((data[pos] & 0xF0) >> 4 ));
  }

  if (timestamp->timezone > 48 || timestamp->timezone < -48)
  {
    LOGE("Timezone is out of bound: %d", timestamp->timezone);
    timestamp->timezone = 0;  /* Modifying it to a Good Value */
  }
  pos ++;

  return pos;

} /* wms_ts_decode_timestamp() */


static uint8 wms_ts_decode_udh_concat_8
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr /* OUT */
)
{
  uint8 pos=0;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if (udh[pos] < 3)  /*Length of information element*/
  {
    LOGE("UDH Header Concat 8 Present with invalid data length = %d", udh[pos]);
    return 0; /*Return 0*/
  }


  /* if the maximum number of messages is 0     Or*/
  /* if the sequence number of the message is 0 Or*/
  /* if the sequence number of the current message is greater than the max messages*/
  if (udh[pos +2] == 0 ||
      udh[pos +3] == 0 ||
      udh[pos +3] > udh[pos +2])
  {
    LOGE("UDH Header Contact 8 with out of bound max messages");
    return 0;
  }

  pos++;
  header_ptr->header_id          = WMS_UDH_CONCAT_8;
  header_ptr->u.concat_8.msg_ref = udh[pos++];
  header_ptr->u.concat_8.total_sm= udh[pos++];
  header_ptr->u.concat_8.seq_num = udh[pos++];

  return (udh[0] + 1);

}/*wms_ts_decode_udh_concat_8*/

static uint8 wms_ts_decode_udh_concat16
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] < 4) /*Length of information element*/
  {
    LOGE("SMS UDH Header Concat16 Present with invalid data length = %d", udh[pos]);
    return 0;
  }

  /* if the maximum number of messages is 0     Or*/
  /* if the sequence number of the message is 0 Or*/
  /* if the sequence number of the current message is greater than the max messages*/
  if (udh[pos +3] == 0 ||
      udh[pos +4] == 0 ||
      udh[pos +4] > udh[pos +3])
    return 0;

  header_ptr->header_id           = WMS_UDH_CONCAT_16;
  pos++;
  header_ptr->u.concat_16.msg_ref = udh[pos++];
  header_ptr->u.concat_16.msg_ref = (uint16)(header_ptr->u.concat_16.msg_ref << 8) | udh[pos++];
  header_ptr->u.concat_16.total_sm= udh[pos++];
  header_ptr->u.concat_16.seq_num = udh[pos++];

  return (udh[0] + 1);
}/*wms_ts_decode_udh_concat16*/


/*=========================================================================
FUNCTION
  wms_ts_udh_decode_first_seg_check

DESCRIPTION

  Local helper function used during decoding to check if message segment
  is first segment or not. 

DEPENDENCIES
  None

RETURN VALUE
  TRUE/FALSE depending on if function is successful or not.

SIDE EFFECTS
  Modifies is_first_segment_ptr with result
=========================================================================*/
static boolean wms_ts_udh_decode_first_seg_check
(
  const uint8  len,                /* user_data_length*/
  const uint8  *data,              /* first byte of user data */
  boolean      *is_first_segment_ptr      /* OUT */
)
{
  uint8 pos = 0;
  uint8 num_headers = 0;
  uint8 udhl = 0;
  uint8 iedl = 0;
  uint8 iei = 0;

  // Default to true - it might not have concat header present
  *is_first_segment_ptr = TRUE;
      
  if (data == NULL || data[pos] == 0 || len == 0 )
  {
    LOGE("null in wms_ts_udh_decode_first_seg_check");
    return FALSE;
  }

  // First byte is the UDH Length
  udhl = data[pos];

  // Move pos to first user data header
  pos++;

  while ((pos < udhl) && (num_headers < WMS_MAX_UD_HEADERS))
  {
      // First byte is UDH, next byte is length
      iei  = data[pos];
      iedl = data[pos+1];

      if ( iei == WMS_UDH_CONCAT_16 )
      {
          // For concat16 element, peek and see seq#
          // -------------------------------------------
          // || IEI | IEDL | Ref# | Ref# |Max# | Seq# ||
          // -------------------------------------------
          if ( data[pos+5] != 1 )
          {
              LOGI("WMS_UDH_CONCAT_16 not first segment!");
              *is_first_segment_ptr = FALSE;
              return TRUE;
          }
          else
          {
              return TRUE;
          }
      }
      // Not a concat header, so we dont care, skip over it's length
      else 
      {
          num_headers++;
          pos += (2 + iedl); // IEI + IEDL + Actual Data Length
      }
  } // while

  return TRUE;
}


static uint8 wms_ts_decode_udh_special_sm
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] < 2) /*Length of information element*/
  {
    LOGE("SMS UDH Header Special SM Present with invalid data length = %d", udh[pos]);
    return 0;
  }

  pos++;
  header_ptr->header_id                             = WMS_UDH_SPECIAL_SM;

  /* Bit 7 of octet 1 indicates if the message shall be stored or not.
     Bit 6..0 of octet 1 indicate which indicator is to be updated.
          000 0000 = Voice Message Waiting
          000 0001 = Fax Message Waiting
          000 0010 = Email Message Waiting
          000 0011 = Other Message Waiting
    Octet 2 represents the number of waiting messages
          The number can range from 0 to 255 with the value of 255 meaning
          that there are 255 or more messages waiting
  */

  header_ptr->u.special_sm.msg_waiting =  (wms_gw_msg_waiting_e_type) ((udh[pos] >> 7 == 0) ? WMS_GW_MSG_WAITING_DISCARD : WMS_GW_MSG_WAITING_STORE);
  header_ptr->u.special_sm.msg_waiting_kind         = (wms_gw_msg_waiting_kind_e_type) (udh[pos++] & 0x7f);
  header_ptr->u.special_sm.message_count            = udh[pos++];

  return (udh[0] + 1);
}/*wms_ts_decode_udh_special_sm*/

static uint8 wms_ts_decode_udh_port_8
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }


  if (udh[pos] < 2)  /*Length of information element*/
  {
    LOGE("UDH Header Port 8 Present with invalid data length = %d", udh[pos]);
    return 0; /*Return 0*/
  }


  pos++;
  header_ptr->header_id          = WMS_UDH_PORT_8;
  header_ptr->u.wap_8.dest_port  = udh[pos++];
  header_ptr->u.wap_8.orig_port  = udh[pos++];

  return (udh[0] + 1);
}/*wms_ts_decode_udh_port_8*/

static uint8 wms_ts_decode_udh_port16
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] < 4) /*Length of information element*/
  {
    LOGE("SMS UDH Header Port16 Present with invalid data length = %d", udh[pos]);
    return 0;
  }

  header_ptr->header_id           = WMS_UDH_PORT_16;
  pos++;
  header_ptr->u.wap_16.dest_port = udh[pos++];
  header_ptr->u.wap_16.dest_port = (uint16)(header_ptr->u.wap_16.dest_port << 8) | udh[pos++];
  header_ptr->u.wap_16.orig_port = udh[pos++];
  header_ptr->u.wap_16.orig_port = (uint16)(header_ptr->u.wap_16.orig_port << 8) | udh[pos++];

  return (udh[0] + 1);
}/*wms_ts_decode_udh_port16*/


static uint8 wms_ts_decode_udh_text_formatting
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] < 3 ) /*Length of information element*/
  {
    LOGE("SMS UDH Header Text Formatting Present with invalid data length = %d", udh[pos]);
    return 0;
  }

  if(udh[pos] >= 4)
  {
    header_ptr->u.text_formating.is_color_present = TRUE;
  }
  else
  {
    header_ptr->u.text_formating.is_color_present = FALSE;
  }
  pos++;


  header_ptr->header_id                                 = WMS_UDH_TEXT_FORMATING;
  header_ptr->u.text_formating.start_position           = udh[pos++];
  header_ptr->u.text_formating.text_formatting_length   = udh[pos++];
  header_ptr->u.text_formating.alignment_type           = (wms_udh_alignment_e_type) (udh[pos] & 0x03 ); /*bit 0 and  bit 1*/
  header_ptr->u.text_formating.font_size                = (wms_udh_font_size_e_type) ((udh[pos] & 0x0c) >> 2); /*bit 3 and  bit 2*/

  header_ptr->u.text_formating.style_bold               = (udh[pos] & 0x10) >> 4; /*bit 4 */
  header_ptr->u.text_formating.style_italic             = (udh[pos] & 0x20) >> 5; /*bit 5 */
  header_ptr->u.text_formating.style_underlined         = (udh[pos] & 0x40) >> 6; /*bit 6 */
  header_ptr->u.text_formating.style_strikethrough      = (udh[pos] & 0x80) >> 7; /*bit 7 */
  pos++;

  if(header_ptr->u.text_formating.is_color_present)
  {
    header_ptr->u.text_formating.text_color_foreground   = (wms_udh_text_color_e_type) (udh[pos] & 0x0F);  /* bit 0-3 */
    header_ptr->u.text_formating.text_color_background   = (wms_udh_text_color_e_type) ((udh[pos] & 0xF0) >> 4);  /* bit 4-7 */
    pos++;
  }
  return (udh[0] + 1);
}/*wms_ts_decode_udh_text_formatting*/

static uint8 wms_ts_decode_udh_pre_def_sound
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] < 2) /*Length of information element*/
  {
    LOGE("SMS UDH Header Pre Defined Sound Present with invalid data length = %d",
               udh[pos]);
    return 0;
  }

  pos++;
  header_ptr->header_id                                 = WMS_UDH_PRE_DEF_SOUND;
  header_ptr->u.pre_def_sound.position                  = udh[pos++];
  header_ptr->u.pre_def_sound.snd_number                = udh[pos++];

  return (udh[0] + 1);
}/*wms_ts_decode_udh_pre_def_sound*/

static uint8 wms_ts_decode_udh_user_def_sound
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0,j;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] == 0) /*Length of information element*/
  {
    LOGE("SMS UDH Header User Defined Sound Present with no Data");
    return 0;
  }

  header_ptr->header_id                                 = WMS_UDH_USER_DEF_SOUND;
  header_ptr->u.user_def_sound.data_length              = udh[pos++]-1;
  header_ptr->u.user_def_sound.position                 = udh[pos++];


  if (header_ptr->u.user_def_sound.data_length > WMS_UDH_MAX_SND_SIZE)
  {
    LOGE("Max Size Exceed Header id %d ", header_ptr->header_id);
    //data[pos] += data[pos]; /*Skip the bytes*/
    return 0;
  }

  //pos++;
  memset (header_ptr->u.user_def_sound.user_def_sound, 0xff,WMS_UDH_MAX_SND_SIZE);

  for(j=0;j<header_ptr->u.user_def_sound.data_length;j++)
    header_ptr->u.user_def_sound.user_def_sound[j]      = udh[pos++];

  return pos;
}/*wms_ts_decode_udh_user_def_sound*/


static uint8 wms_ts_decode_udh_pre_def_anim
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] != 2) /*Length of information element*/
  {
    LOGE("SMS UDH Header Pre Defined Animation Present with invalid data length = %d",
         udh[pos]);
    return 0;
  }

  pos++;
  header_ptr->header_id                                 = WMS_UDH_PRE_DEF_ANIM;
  header_ptr->u.pre_def_anim.position                   = udh[pos++];
  header_ptr->u.pre_def_anim.animation_number           = udh[pos++];

  return pos;
}/*wms_ts_decode_udh_pre_def_anim*/

static uint8 wms_ts_decode_udh_large_anim
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0,j,k;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] != (WMS_UDH_ANIM_NUM_BITMAPS * WMS_UDH_LARGE_BITMAP_SIZE + 1) ) /*Length of information element*/
  {
    LOGE("SMS UDH Header Large Defined Animation Present with invalid data length = %d",
               udh[pos]);
    return 0;
  }

  header_ptr->header_id                             = WMS_UDH_LARGE_ANIM;
  pos++; /*Skip the Size*/
  header_ptr->u.large_anim.position                 = udh[pos++];

  for(j=0;j<WMS_UDH_ANIM_NUM_BITMAPS;j++)
    for (k=0;k<WMS_UDH_LARGE_BITMAP_SIZE; k++)
      header_ptr->u.large_anim.data[j][k] = udh[pos++];

  return pos;
}/*wms_ts_decode_udh_large_anim*/


static uint8 wms_ts_decode_udh_small_anim
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0,j,k;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] != (WMS_UDH_ANIM_NUM_BITMAPS * WMS_UDH_SMALL_BITMAP_SIZE + 1) ) /*Length of information element*/
  {
    LOGE("SMS UDH Header Large Defined Animation Present with invalid data length = %d",
               udh[pos]);
    return 0;
  }

  header_ptr->header_id                             = WMS_UDH_SMALL_ANIM;

  pos++; /*Skip the Size*/
  header_ptr->u.small_anim.position                 = udh[pos++];

  for(j=0;j<WMS_UDH_ANIM_NUM_BITMAPS;j++)
    for (k=0;k<WMS_UDH_SMALL_BITMAP_SIZE; k++)
      header_ptr->u.small_anim.data[j][k] = udh[pos++];

  return pos;
}/*wms_ts_decode_udh_small_anim*/

static uint8 wms_ts_decode_udh_large_picture
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0,j;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] != WMS_UDH_LARGE_PIC_SIZE + 1) /*Length of information element*/
  {
    LOGE("SMS UDH Header Large Picture Present with invalid data length = %d",
               udh[pos]);
    return 0;
  }


  header_ptr->header_id                                 = WMS_UDH_LARGE_PICTURE;
  pos++; /*Skip the Size*/
  header_ptr->u.large_picture.position                  = udh[pos++];

  for(j=0;j<WMS_UDH_LARGE_PIC_SIZE;j++)
    header_ptr->u.large_picture.data[j]    = udh[pos++];

  return pos;
}/*wms_ts_decode_udh_large_picture*/


static uint8 wms_ts_decode_udh_small_picture
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0,j;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] != WMS_UDH_SMALL_PIC_SIZE + 1) /*Length of information element*/
  {
    LOGE("SMS UDH Header Small Picture Present with invalid data legnth = %d",
               udh[pos]);
    return 0;
  }

  header_ptr->header_id                                 = WMS_UDH_SMALL_PICTURE;
  pos++; /*Skip the size*/
  header_ptr->u.small_picture.position                  = udh[pos++];

  for(j=0;j<WMS_UDH_SMALL_PIC_SIZE;j++)
    header_ptr->u.small_picture.data[j]           = udh[pos++];

  return pos;
}/*wms_ts_decode_udh_small_picture*/


static uint8 wms_ts_decode_udh_var_picture
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0,j,pic_size;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] > WMS_UDH_VAR_PIC_SIZE + 3) /*Length of information element*/
  {
    LOGE("SMS UDH Header Var Picture Present with invalid data length = %d",
               udh[pos]);
    return 0;
  }

  if ( (udh[pos] - 3) != (udh[pos+2] * udh[pos+3]) )
  {
    LOGE("SMS UDH Header Var Picture, pic size value mismatch with heigt and weight");
    return 0;
  }

  pic_size                                          = udh[pos++] -3;
  header_ptr->header_id                                 = WMS_UDH_VAR_PICTURE;
  header_ptr->u.var_picture.position                    = udh[pos++];
  header_ptr->u.var_picture.width                       = (uint8) (udh[pos++] * 8);
  header_ptr->u.var_picture.height                      = udh[pos++];

  for(j=0;j<pic_size && j<WMS_UDH_VAR_PIC_SIZE; j++)
    header_ptr->u.var_picture.data[j]             = udh[pos++];


  return pos;
}/*wms_ts_decode_udh_var_picture*/


static uint8 wms_ts_decode_udh_user_prompt
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] < 1) /*Length of information element*/
  {
    LOGE("SMS UDH User Prompt present with invalid data length = %d",
               udh[pos]);
    return 0;
  }

  pos ++; /* Skip udh length */

  header_ptr->header_id                                 = WMS_UDH_USER_PROMPT;
  header_ptr->u.user_prompt.number_of_objects           = udh[pos++];

  return (udh[0] + 1);

} /* wms_ts_decode_udh_user_prompt() */


/* Decoding UDH Extended Object
*/
static uint8 wms_ts_decode_udh_eo
(
  const uint8 *udh,
  boolean     first_segment,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0, udh_length;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] == 0) /*Length of information element*/
  {
    LOGE("SMS UDH Extended Object present with no Data");
    return 0;
  }

  /* Get the length of this UDH */
  udh_length = udh[pos++];

  header_ptr->header_id                  = WMS_UDH_EXTENDED_OBJECT;
  header_ptr->u.eo.first_segment         = first_segment;

  if( first_segment == TRUE )
  {
    /* The following fields in the first segment occupy 7 bytes */
    if( udh_length < WMS_UDH_OCTETS_EO_HEADER )
    {
      return 0;
    }

    header_ptr->u.eo.reference           = udh[pos++];
    header_ptr->u.eo.length              = udh[pos++] << 8;
    header_ptr->u.eo.length              |= udh[pos++];
    header_ptr->u.eo.control             = udh[pos++];
    header_ptr->u.eo.type                = (wms_udh_eo_id_e_type) udh[pos++];
    header_ptr->u.eo.position            = udh[pos++] << 8;
    header_ptr->u.eo.position            |= udh[pos++];
  }

  /* Decode EO content */
  header_ptr->u.eo.content.length = (udh_length - pos) + 1;
  memcpy( header_ptr->u.eo.content.data,
          udh + pos,
          header_ptr->u.eo.content.length );

  pos += header_ptr->u.eo.content.length;

  return pos;

} /* wms_ts_decode_udh_eo() */

static uint8 wms_ts_decode_udh_rfc822
(
  const uint8 *udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if(udh[pos] <  1) /*Length of information element*/
  {
    LOGE("SMS UDH Header Rfc822 Present with invalid data length = %d", udh[pos]);
    return 0;
  }

  pos++;
  header_ptr->header_id                                 = WMS_UDH_RFC822;
  header_ptr->u.rfc822.header_length                    = udh[pos++];

  return (udh[0] + 1);
}/*wms_ts_decode_udh_rfc822*/

static uint8 wms_ts_decode_udh_other
(
  const uint8* udh,
  wms_udh_s_type *header_ptr
)
{
  uint8 pos=0, i=0;

  if (udh == NULL || header_ptr == NULL )
  {
    LOGE("udh is NULL");
    return 0;
  }

  if (udh[pos+1] > WMS_UDH_OTHER_SIZE)
  {
    LOGE("SMS UDH Header Other data length exceeding 226");
    return 0;
  }

  header_ptr->header_id                                 = (wms_udh_id_e_type) udh[pos];
  header_ptr->u.other.header_id                         = (wms_udh_id_e_type) udh[pos++];
  header_ptr->u.other.header_length                     = udh[pos++];

  for(i=0;i<header_ptr->u.other.header_length;i++)
  {
    header_ptr->u.other.data[i] = udh[pos++];
  }

  return pos;
}

/*=========================================================================
FUNCTION
  wms_ts_decode_user_data_header

DESCRIPTION
  Decode User Data from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_decode_user_data_header
(
  const uint8               len, /* user_data_length*/
  const uint8               *data, /* first byte of user data */
  uint8                     * num_headers_ptr, /* OUT */
  wms_udh_s_type            * udh_ptr          /* OUT */
)
{
  uint8 pos =0;
  uint8 header_length =0, num_headers=0;
  uint8 udhl;

  boolean    first_segment = TRUE; /* Used for Extended Object decoding */

  if (data == NULL || len == 0 || data[pos] == 0 || num_headers_ptr == NULL || udh_ptr == NULL )
  {
    LOGE("null pointer in wms_ts_decode_user_data_header");
   return 0;
  }

  udhl = data[pos];

  pos++;

   while ((pos < udhl)&&(num_headers<WMS_MAX_UD_HEADERS))
   {
     /*first byte - header id*/
     /*second byte - header length*/
     switch(data[pos++])
     {
       case WMS_UDH_CONCAT_8:
         header_length = wms_ts_decode_udh_concat_8(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_CONCAT_16:
         header_length = wms_ts_decode_udh_concat16(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_SPECIAL_SM:
         header_length = wms_ts_decode_udh_special_sm(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_PORT_8:
         header_length = wms_ts_decode_udh_port_8(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_PORT_16:
         header_length = wms_ts_decode_udh_port16(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_TEXT_FORMATING:
         header_length = wms_ts_decode_udh_text_formatting(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_PRE_DEF_SOUND:
         header_length = wms_ts_decode_udh_pre_def_sound(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_USER_DEF_SOUND:
         header_length = wms_ts_decode_udh_user_def_sound(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_PRE_DEF_ANIM:
         header_length = wms_ts_decode_udh_pre_def_anim(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_LARGE_ANIM:
         header_length = wms_ts_decode_udh_large_anim(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_SMALL_ANIM:
         header_length = wms_ts_decode_udh_small_anim(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_LARGE_PICTURE:
         header_length = wms_ts_decode_udh_large_picture(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_SMALL_PICTURE:
         header_length = wms_ts_decode_udh_small_picture(data+pos, &udh_ptr[num_headers]);
           break;

       case WMS_UDH_VAR_PICTURE:
         header_length = wms_ts_decode_udh_var_picture(data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_USER_PROMPT:
         header_length = wms_ts_decode_udh_user_prompt( data+pos, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_EXTENDED_OBJECT:
         if ( wms_ts_udh_decode_first_seg_check ( len, data, &first_segment ) == FALSE )
         {
           LOGE("wms_ts_udh_decode_first_seg_check failed");
           return 0;
         }
         header_length = wms_ts_decode_udh_eo( data+pos, first_segment, &udh_ptr[num_headers]);
         break;

       case WMS_UDH_RFC822:
         header_length = wms_ts_decode_udh_rfc822(data+pos, &udh_ptr[num_headers]);
         break;

       default:
         pos --;
         header_length = wms_ts_decode_udh_other(data+pos, &udh_ptr[num_headers]);
         break;
     } /* End of Switch */

     if ( (uint16)pos + (uint16)header_length  > WMS_MAX_LEN )
     {
       LOGE("number of bytes decoded has exceeded UDHL value of %d",udhl);
       return 0;
     }
     else if(header_length != 0)
     {
       pos += header_length;
       num_headers++;
     }
     else
     {
       LOGE("Bad UDH: pos=%d, data[pos]=%d", pos, data[pos]);
       * num_headers_ptr = 0;
       return 0;  /* SHORT-RETURN */
     }
   } /* End of While loop */


   if (num_headers >= WMS_MAX_UD_HEADERS)
   {
     /* Num Headers has Exceeded Max */
     LOGE("decode_udh: Num Headers has exceeded WMS_MAX_UD_HEADERS");

     /* Placing the correct value */
     pos = udhl+1;
   }

   if (pos!= (udhl+1))
   {
     LOGE("SMS UDH could not be decoded");
     num_headers =0;
     udhl =0;
   }

   if (num_headers >0)
   {
     * num_headers_ptr = num_headers;
   }

   return udhl;
}
/*wms_ts_decode_user_data_header*/

/*=========================================================================
FUNCTION
  wms_ts_decode_gw_user_data

DESCRIPTION
  Decode User Data from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_decode_gw_user_data
(
  const wms_gw_dcs_s_type   *dcs,
  const uint8               len, // user data length
  const uint8               *data,
  const boolean             user_data_header_present,
  wms_gw_user_data_s_type   *user_data
)
{
  uint8 i, pos=0;
  uint8   fill_bits =0;
  uint8   user_data_length;

  uint8   user_data_header_length = 0; /* only the user data header length*/

  if (dcs == NULL || data == NULL || user_data == NULL)
  {
    LOGE("null pointer in wms_ts_decode_gw_user_data");
    return 0;
  }

  /* Defaulting to all zeroes */
  (void)memset(user_data, 0, sizeof(wms_gw_user_data_s_type));

  if (len == 0)
  {
    return 0;
  }

  if( dcs->alphabet == WMS_GW_ALPHABET_7_BIT_DEFAULT )
  {
    if (len > WMS_SMS_UDL_MAX_7_BIT)
    {
      LOGE("user data length > max value for gw 7-bit alphabet");
      return 0;
    }
    user_data_length = len;

    if(user_data_header_present)
    {
      user_data_header_length = wms_ts_decode_user_data_header( data[pos], data+pos,
                                                                & user_data->num_headers, user_data->headers );
    }
    if (user_data_header_length > len)
    {
       LOGE("user data header length > total length");
       return 0;
    }
    /*len would be the number of septets*/
    if(user_data_header_length > 0)
    {
      /*The number of fill bits required to make a septet boundary*/
      fill_bits =( (len * 7) - ((user_data_header_length+1)*8) ) % 7;
      user_data_length = (uint8)(( (len * 7) - ((user_data_header_length+1)*8) ) / 7);
      pos = user_data_header_length + 1;

      if (fill_bits != 0)
      {
        fill_bits = 8-fill_bits;
      }
    }

    i = wms_ts_unpack_gw_7_bit_chars
        (
          & data[pos],
          user_data_length,
          WMS_MAX_LEN,
          fill_bits,
          user_data->sm_data
        );
    user_data->sm_len = user_data_length;
  }
  else
  {
    if (len > WMS_SMS_UDL_MAX_8_BIT)
    {
      LOGE("user data length > max value for 8-bit chararacters");
      return 0;
    }
    user_data_length = len;
    if(user_data_header_present)
    {
       user_data_header_length = wms_ts_decode_user_data_header( data[pos],
                                                                 data+pos,
                                                       & user_data->num_headers,
                                                       user_data->headers );
       if (user_data_header_length > len)
       {
          LOGE("user data header length > total length");
          return 0;
       }
       pos += user_data_header_length +1;
       user_data_length = len - user_data_header_length - 1;
    }

    memcpy( user_data->sm_data, data+pos, user_data_length );
    user_data->sm_len = user_data_length;
    i = (uint8)user_data->sm_len;
  }

  pos += i;

  return pos;

} /* wms_ts_decode_gw_user_data() */


/*=========================================================================
FUNCTION
  wms_ts_decode_gw_validity

DESCRIPTION
  Decode Validity from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  Number of bytes decoded.

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_decode_gw_validity
(
  const uint8                 * data,
  wms_gw_validity_s_type    * validity
)
{
  uint8 i, pos = 0;

  if (data == NULL || validity == NULL)
  {
    LOGE("null pointer in wms_ts_decode_gw_validity");
    return 0;
  }
  else
  {
    switch( validity->format )
    {
      case WMS_GW_VALIDITY_NONE:
      memset( validity, 0, sizeof(wms_gw_validity_s_type) );
        break;

      case WMS_GW_VALIDITY_RELATIVE:
        wms_ts_decode_relative_time( data[pos], & validity->u.time );
        pos ++;
        break;

      case WMS_GW_VALIDITY_ABSOLUTE:
        i = wms_ts_decode_timestamp( data+pos, & validity->u.time );
        if( i == 0 )
        {
          LOGE("Error while Decoding Absolute Validity Timestamp");
        }
        pos += i;
        break;

      case WMS_GW_VALIDITY_ENHANCED:
        // TBD
        break;

      default:
        break;
    } /* switch */
    return pos;
  }

} /* wms_ts_decode_gw_validity() */



/*=========================================================================
FUNCTION
  wms_ts_decode_deliver

DESCRIPTION
  Decode Deliver TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_deliver
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_gw_deliver_s_type                   * deliver
)
{
  wms_status_e_type     st = WMS_OK_S;
  uint32                pos = 0, i;
  const uint8           *data = raw_ts_data_ptr->data;

  /* TP-MTI, TP-MMS, TP-SRI, TP-UDHI, TP-RP
  */
  if (raw_ts_data_ptr == NULL || deliver == NULL)
  {
    LOGE("null pointer in wms_ts_decode_deliver");
    return WMS_NULL_PTR_S;
  }
  else if ( (data[pos] & 0x03) != 0x00) /* check bit 0 and bit 1 for TP-MTI for deliver */
  {
    LOGE("invalid tpdu type in wms_ts_decode_deliver");
    return WMS_INVALID_TPDU_TYPE_S;
  }
  else
  {
    deliver->more                     = (data[pos] & 0x04) ? FALSE : TRUE;
                                        /* bit 2 */
    /* bits 3, 4 are not used */
    deliver->status_report_enabled    = (data[pos] & 0x20) ? TRUE : FALSE;
                                        /* bit 5 */
    deliver->user_data_header_present = (data[pos] & 0x40) ? TRUE : FALSE;
                                        /* bit 6 */
    deliver->reply_path_present       = (data[pos] & 0x80) ? TRUE : FALSE;
                                        /* bit 7 */
    pos ++;

    /* TP-OA
    */
    i = wms_ts_decode_address( & data[pos], & deliver->address);
    if( i==0 )
    {
      LOGE("invalid param size in wms_ts_decode_deliver");
      return WMS_INVALID_PARM_SIZE_S;
    }
    pos += i;

    /* TP-PID
    */
    deliver->pid = (wms_pid_e_type) data[pos];
    pos ++;

    /* TP-DCS
    */
    pos += wms_ts_decode_dcs( data+pos, & deliver->dcs );

    if (deliver->dcs.msg_waiting_kind != WMS_GW_MSG_WAITING_VOICEMAIL)
    {
      if (deliver->pid == WMS_PID_RETURN_CALL)
      {
        deliver->dcs.msg_waiting        = WMS_GW_MSG_WAITING_STORE;
        deliver->dcs.msg_waiting_active = TRUE;
        deliver->dcs.msg_waiting_kind   = WMS_GW_MSG_WAITING_VOICEMAIL;
      }
    }

    /* TP-SCTS
    */
    i = wms_ts_decode_timestamp( data+pos, & deliver->timestamp );
    if ( i==0 )
    {
      LOGE("invalid param value in wms_ts_decode_deliver");
      return WMS_INVALID_PARM_VALUE_S;
    }
    pos += i;


    /* TP-UDL
    */
  //  deliver->user_data_len = data[pos];
    pos ++;

    /* TP-UD
    */
    i = wms_ts_decode_gw_user_data( & deliver->dcs,
                                         data[pos-1],
                                         data+pos,
                                         deliver->user_data_header_present,
                                       & deliver->user_data );

    if (i > WMS_SMS_UDL_MAX_8_BIT)
    {
      LOGE("User Data Length has exceeded capacity: UDL = %lu", i);
      st = WMS_INVALID_USER_DATA_SIZE_S;
    }

    pos += i;

    /* Set the global raw ts data len
    */
    raw_ts_len = pos;

    return st;
  }
} /* wms_ts_decode_deliver() */



/*=========================================================================
FUNCTION
  wms_ts_decode_deliver_report_ack

DESCRIPTION
  Decode Deliver-Report-Ack TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_deliver_report_ack
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_gw_deliver_report_ack_s_type        * deliver_report_ack
)
{
  wms_status_e_type     st = WMS_OK_S;
  uint32                pos = 0, i;
  const uint8           *data;

  if( raw_ts_data_ptr == NULL || deliver_report_ack == NULL )
  {
    LOGE("Null pointer in wms_ts_decode_deliver_report_ack!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* TP-MTI, TP-UDHI
  */
  deliver_report_ack->user_data_header_present =
          (data[pos] & 0x40) ? TRUE : FALSE; /* bit 6 */
  pos ++;

  /* TP-PI
  */
  deliver_report_ack->mask = data[pos];
  pos ++;

  if( deliver_report_ack->mask & WMS_TPDU_MASK_PID )
  {
    /* TP-PID
    */
    deliver_report_ack->pid = (wms_pid_e_type) data[pos];
    pos ++;
  }

  if( deliver_report_ack->mask & WMS_TPDU_MASK_DCS )
  {
    /* TP-DCS
    */
    pos += wms_ts_decode_dcs( data+pos, & deliver_report_ack->dcs );
  }

  if( deliver_report_ack->mask & WMS_TPDU_MASK_USER_DATA )
  {
    /* TP-UDL
    */
//    deliver_report_ack->user_data_len = data[pos];
    pos ++;

    /* TP-UD
    */
    i = wms_ts_decode_gw_user_data( & deliver_report_ack->dcs,
                                         data[pos-1],
                                         data+pos,
                                         deliver_report_ack->user_data_header_present,
                                       & deliver_report_ack->user_data );

    if (i > WMS_SMS_UDL_MAX_8_BIT)
    {
      LOGE("User Data Length has exceeded capacity: UDL = %lu", i);
      st = WMS_INVALID_USER_DATA_SIZE_S;
    }

    pos += i;
  }

  /* Set the global raw ts data len
  */
  raw_ts_len = pos;


  return st;

} /* wms_ts_decode_deliver_report_ack() */


/*=========================================================================
FUNCTION
  wms_ts_decode_deliver_report_error

DESCRIPTION
  Decode Deliver-Report-Error TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_deliver_report_error
(
  const wms_raw_ts_data_s_type              * raw_ts_data_ptr,
  wms_gw_deliver_report_error_s_type        * deliver_report_error
)
{
  wms_status_e_type     st = WMS_OK_S;
  uint32                pos = 0, i;
  const uint8           *data;

  if( raw_ts_data_ptr == NULL || deliver_report_error == NULL )
  {
    LOGE("Null pointer in wms_ts_decode_deliver_report_error!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* TP-MTI, TP-UDHI
  */
  deliver_report_error->user_data_header_present =
         (data[pos] & 0x40) ? TRUE : FALSE; /* bit 6 */
  pos ++;

  /* TP-FCS
  */
  deliver_report_error->tp_cause = (wms_tp_cause_e_type) data[pos];
  pos ++;

  /* TP-PI
  */
  deliver_report_error->mask = data[pos];
  pos ++;

  if( deliver_report_error->mask & WMS_TPDU_MASK_PID )
  {
    /* TP-PID
    */
    deliver_report_error->pid = (wms_pid_e_type) data[pos];
    pos ++;
  }

  if( deliver_report_error->mask & WMS_TPDU_MASK_DCS )
  {
    /* TP-DCS
    */
    pos += wms_ts_decode_dcs( data+pos, & deliver_report_error->dcs );
  }

  if( deliver_report_error->mask & WMS_TPDU_MASK_USER_DATA )
  {
    /* TP-UDL
    */
//    deliver_report_error->user_data_len = data[pos];
    pos ++;

    /* TP-UD
    */
    i = wms_ts_decode_gw_user_data( & deliver_report_error->dcs,
                                         data[pos-1],
                                         data+pos,
                                         deliver_report_error->user_data_header_present,
                                       & deliver_report_error->user_data );

    if (i > WMS_SMS_UDL_MAX_8_BIT)
    {
      LOGE("User Data Length has exceeded capacity: UDL = %lu", i);
      st = WMS_INVALID_USER_DATA_SIZE_S;
    }

    pos += i;
  }

  /* Set the global raw ts data len
  */
  raw_ts_len = pos;


  return st;

} /* wms_ts_decode_deliver_report_error() */

/*=========================================================================
FUNCTION
  wms_ts_decode_status_report

DESCRIPTION
  Decode Status Report TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_status_report
(
  const wms_raw_ts_data_s_type              * raw_ts_data_ptr,
  wms_gw_status_report_s_type               * status_report
)
{
  wms_status_e_type     st = WMS_OK_S;
  uint32                pos = 0, i;
  const uint8           *data;

  if( raw_ts_data_ptr == NULL || status_report == NULL )
  {
    LOGE("Null pointer in wms_ts_decode_status_report!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* TP-MTI, TP_UDHI:
  */
  status_report->more = data[pos] & 0x04? FALSE:TRUE;/*bit 2*/
  status_report->status_report_qualifier = data[pos]  & 0x20 ? TRUE:FALSE;
      /*bit 5*/
  status_report->user_data_header_present =
         (data[pos] & 0x40) ? TRUE : FALSE; /* bit 6 */
  pos ++;

  /* TP-MR
  */
  status_report->message_reference = data[pos];
  pos ++;

  /* TP-RA
  */
  i = wms_ts_decode_address( & data[pos], & status_report->address);
  if( i==0 )
  {
    return WMS_INVALID_PARM_SIZE_S;
  }
  pos += i;

  /* TP-SCTS
  */
  i = wms_ts_decode_timestamp( data+pos, & status_report->timestamp);
  if( i==0 )
  {
    return WMS_INVALID_PARM_VALUE_S;
  }
  pos += i;

  /* TP-DT*/
  i = wms_ts_decode_timestamp( data+pos, & status_report->discharge_time);
  if( i==0 )
  {
    return WMS_INVALID_PARM_VALUE_S;
  }
  pos += i;

  /*TP-ST*/
  status_report->tp_status = (wms_tp_status_e_type) data[pos];
  pos++;

  /*TP-PI*/
  status_report->mask = data[pos];

  /*TP-PID*/
  status_report->pid = (wms_pid_e_type) 0;

  status_report->user_data.sm_len = 0;

  /* to extract last byte */
  status_report->mask &= 0xFF;

  /* to check for valid mask */
  if ((status_report->mask != 0xFF)
      && (status_report->mask != 0))
  {
    /* Increment only if mask is valid */
    pos ++;

    if( status_report->mask & WMS_TPDU_MASK_PID )
    {
       status_report->pid = (wms_pid_e_type) data[pos];
       pos ++;
    }

    /* TP-DCS
    */
    if( status_report->mask & WMS_TPDU_MASK_DCS )
    {
      pos += wms_ts_decode_dcs( data+pos, & status_report->dcs );
    }

    if( status_report->mask & WMS_TPDU_MASK_USER_DATA )
    {
      /* TP-UDL
      */
      pos ++;
      /* TP-UD
      */
      i = wms_ts_decode_gw_user_data( & status_report->dcs,
                                         data[pos-1],
                                         data+pos,
                                         status_report->user_data_header_present,
                                         & status_report->user_data );

      if (i > WMS_SMS_UDL_MAX_8_BIT)
      {
        LOGE("User Data Length has exceeded capacity: UDL = %lu", i);
        st = WMS_INVALID_USER_DATA_SIZE_S;
      }

      pos += i;
    }
  }
  else
  {
    status_report->mask = 0;
  }
  /* Set the global raw ts data len
  */
  raw_ts_len = pos;

  return st;

} /* wms_ts_decode_status_report() */

/*=========================================================================
FUNCTION
  wms_ts_decode_command

DESCRIPTION
  Decode Command TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_command
(
  const wms_raw_ts_data_s_type              * raw_ts_data_ptr,
  wms_gw_command_s_type                     * command
)
{
  wms_status_e_type   st = WMS_OK_S;
  uint32                pos = 0, i;
  const uint8           *data;

  if( raw_ts_data_ptr == NULL || command == NULL )
  {
    LOGE("Null pointer in wms_ts_decode_command!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* TP-MTI, TP_UDHI:
  */
  command ->status_report_enabled  =
         (data[pos] & 0x20) ? TRUE : FALSE; /* bit 5 */

  command->user_data_header_present =
         (data[pos] & 0x40) ? TRUE : FALSE; /* bit 6 */
  pos ++;

  /* TP-MR
  */
  command->message_reference = data[pos];
  pos ++;

  /* TP-PID
  */
  command->pid = (wms_pid_e_type) data[pos];
  pos++;

  /*TP-CT
  */
  command->command = (wms_gw_command_e_type) data[pos];
  pos ++;

  /*TP-MN
  */
  command->message_number = data[pos];
  pos ++;

  /* TP-DA
  */
  i = wms_ts_decode_address( & data[pos], & command->address);
  if( i==0)
  {
    return WMS_INVALID_PARM_SIZE_S;
  }

  pos += i;

  /* TP-CDL
  */
  command->command_data_len = MIN (data[pos], WMS_GW_COMMAND_DATA_MAX);
  pos ++;

  /*TP-CD
  */
  memcpy(command->command_data,data+pos,command->command_data_len);

  /* Set the global raw ts data len
  */
  raw_ts_len = pos;

  return st;

} /* wms_ts_decode_command() */

/*=========================================================================
FUNCTION
  wms_ts_decode_submit

DESCRIPTION
  Decode Submit TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_submit
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_gw_submit_s_type                    * submit
)
{
  wms_status_e_type     st = WMS_OK_S;
  uint32                pos = 0, i;
  const uint8           *data;

  if( raw_ts_data_ptr == NULL || submit == NULL )
  {
    LOGE("Null pointer in wms_ts_decode_submit!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* TP-MTI, TP-RD, TP-VPF, TP-SRR, TP_UDHI, TP-RP:
  */
  submit->reject_duplicates         = (data[pos] & 0x04) ? TRUE : FALSE;
                                      /* bit 2 */
  submit->validity.format           = (wms_gw_validity_format_e_type) (( data[pos] & 0x18 ) >> 3); /* bits 3, 4 */
  submit->status_report_enabled     = (data[pos] & 0x20) ? TRUE : FALSE;
                                      /* bit 5 */
  submit->user_data_header_present  = (data[pos] & 0x40) ? TRUE : FALSE;
                                      /* bit 6 */
  submit->reply_path_present        = (data[pos] & 0x80) ? TRUE : FALSE;
                                      /* bit 7 */
  pos ++;


  /* TP-MR
  */
  submit->message_reference = data[pos];
  pos ++;

  /* TP-DA
  */
  i = wms_ts_decode_address( & data[pos], & submit->address );
  if( i == 0 )
  {
    return WMS_INVALID_PARM_SIZE_S;
  }

  pos += i;

  /* TP-PID
  */
  submit->pid = (wms_pid_e_type) data[pos];
  pos ++;

  /* TP-DCS
  */
  pos += wms_ts_decode_dcs( data+pos, & submit->dcs );

  /* TP-VP
  */
  i = wms_ts_decode_gw_validity( data+pos, & submit->validity );
  if ((submit->validity.format != WMS_GW_VALIDITY_NONE) && ( i == 0 ))
  {
    return WMS_INVALID_PARM_VALUE_S;
  }

  pos += i;

  /* TP-UDL
  */
//  submit->user_data_len = data[pos];
  pos ++;

  /* TP-UD
  */
  i = wms_ts_decode_gw_user_data( & submit->dcs,
                                       data[pos-1],
                                       data+pos,
                                       submit->user_data_header_present,
                                     & submit->user_data );

  if (i > WMS_SMS_UDL_MAX_8_BIT)
  {
    LOGE("User Data Length has exceeded capacity: UDL = %lu", i);
    st = WMS_INVALID_USER_DATA_SIZE_S;
  }

  pos += i;

  /* Set the global raw ts data len
  */
  raw_ts_len = pos;

  return st;

} /* wms_ts_decode_submit() */


/*=========================================================================
FUNCTION
  wms_ts_decode_submit_report_ack

DESCRIPTION
  Decode Submit-Report-Ack TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_submit_report_ack
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_gw_submit_report_ack_s_type         * submit_report_ack
)
{
  wms_status_e_type     st = WMS_OK_S;
  uint32                pos = 0, i;
  const uint8          *data;

  if( raw_ts_data_ptr == NULL || submit_report_ack == NULL )
  {
    LOGE("Null pointer in wms_ts_decode_submit_report_ack!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;

  /* TP-MTI, TP_UDHI:
  */
  submit_report_ack->user_data_header_present  =
            (data[pos] & 0x40) ? TRUE : FALSE; /* bit 6 */
  pos ++;

  /* TP-PI
  */
  submit_report_ack->mask = data[pos];
  pos ++;

  /* TP-SCTS
  */

  i = wms_ts_decode_timestamp( data+pos, & submit_report_ack->timestamp );
  if ( i==0 )
  {
    return WMS_INVALID_PARM_SIZE_S;
  }
  pos += i;

  if( submit_report_ack->mask & WMS_TPDU_MASK_PID )
  {
    /* TP-PID
    */
    submit_report_ack->pid = (wms_pid_e_type) data[pos];
    pos ++;
  }

  if( submit_report_ack->mask & WMS_TPDU_MASK_DCS )
  {
    /* TP-DCS
    */
    pos += wms_ts_decode_dcs( data+pos, & submit_report_ack->dcs );
  }

  if( submit_report_ack->mask & WMS_TPDU_MASK_USER_DATA )
  {
    /* TP-UDL
    */
//    submit_report_ack->user_data_len = data[pos];
    pos ++;

    /* TP-UD
    */
    i = wms_ts_decode_gw_user_data( & submit_report_ack->dcs,
                                         data[pos-1],
                                         data+pos,
                                         submit_report_ack->user_data_header_present,
                                       & submit_report_ack->user_data );
    if (i > WMS_SMS_UDL_MAX_8_BIT)
    {
      LOGE("User Data Length has exceeded capacity: UDL = %lu", i);
      st = WMS_INVALID_USER_DATA_SIZE_S;
    }

    pos += i;
  }

  /* Set the global raw ts data len
  */
  raw_ts_len = pos;

  return st;

} /* wms_ts_decode_submit_report_ack() */


/*=========================================================================
FUNCTION
  wms_ts_decode_submit_report_error

DESCRIPTION
  Decode Submit-Report-Error TPDU from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_submit_report_error
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_gw_submit_report_error_s_type       * submit_report_error
)
{
  wms_status_e_type   st = WMS_OK_S;
  uint32              pos = 0, i;
  const uint8        *data;

  if( raw_ts_data_ptr == NULL || submit_report_error == NULL )
  {
    LOGE("Null pointer in wms_ts_decode_submit_report_error!");
    return WMS_NULL_PTR_S;
  }

  data = raw_ts_data_ptr->data;


  /* TP-MTI, TP_UDHI:
  */
  submit_report_error->user_data_header_present  =
         (data[pos] & 0x40) ? TRUE : FALSE; /* bit 6 */
  pos ++;

  /* TP-FCS
  */
  submit_report_error->tp_cause = (wms_tp_cause_e_type) data[pos];
  pos ++;

  /* TP-PI
  */
  submit_report_error->mask = data[pos];
  pos ++;

  /* TP-SCTS
  */
  i = wms_ts_decode_timestamp( data+pos, & submit_report_error->timestamp );
  if( i==0 )
  {
    return WMS_INVALID_PARM_VALUE_S;
  }
  pos += i;

  if( submit_report_error->mask & WMS_TPDU_MASK_PID )
  {
    /* TP-PID
    */
    submit_report_error->pid = (wms_pid_e_type) data[pos];
    pos ++;
  }

  if( submit_report_error->mask & WMS_TPDU_MASK_DCS )
  {
    /* TP-DCS
    */
    pos += wms_ts_decode_dcs( data+pos, & submit_report_error->dcs );
  }

  if( submit_report_error->mask & WMS_TPDU_MASK_USER_DATA )
  {
    /* TP-UDL
    */
//    submit_report_error->user_data_len = data[pos];
    pos ++;

    /* TP-UD
    */
    i = wms_ts_decode_gw_user_data( & submit_report_error->dcs,
                                         data[pos-1],
                                         data+pos,
                                         submit_report_error->user_data_header_present,
                                       & submit_report_error->user_data );

    if (i > WMS_SMS_UDL_MAX_8_BIT)
    {
      LOGE("User Data Length has exceeded capacity: UDL = %lu", i);
      st = WMS_INVALID_USER_DATA_SIZE_S;
    }

    pos += i;
  }

  /* Set the global raw ts data len
  */
  raw_ts_len = pos;

  return st;

} /* wms_ts_decode_submit_report_error() */


wms_status_e_type wms_ts_decode_gw_cb
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr, /* IN */
  wms_gw_cb_ts_data_s_type                * client_ts_ptr    /* OUT */
)
{
  wms_status_e_type   st = WMS_OK_S;

  if( raw_ts_data_ptr == NULL || client_ts_ptr == NULL )
  {
    st = WMS_NULL_PTR_S;
  }
  else if( raw_ts_data_ptr->format != WMS_FORMAT_GW_CB || raw_ts_data_ptr->len > WMS_GW_CB_PAGE_SIZE )
  {
    st = WMS_INVALID_CB_DATA_S;
  }
  else if( wms_ts_decode_gw_cb_page_header( (const uint8*)raw_ts_data_ptr->data,
                                          & client_ts_ptr->cb_page_hdr ) == 0 )
  {
    st = WMS_INVALID_CB_DATA_S;
  }
  else
  {
    /* Process user data part */
    client_ts_ptr->user_data.num_headers = 0;

    switch( client_ts_ptr->cb_page_hdr.cb_dcs.alphabet )
    {
      case WMS_GW_ALPHABET_7_BIT_DEFAULT:
        gw_user_data.sm_len = wms_ts_unpack_gw_7_bit_chars
                        (
                          raw_ts_data_ptr->data + WMS_GW_CB_PAGE_HEADER_LEN,
                          (uint8)(((raw_ts_data_ptr->len - WMS_GW_CB_PAGE_HEADER_LEN) * 8) / 7),
                          WMS_MAX_LEN,
                          0, /* fill bits */
                          gw_user_data.sm_data
                        );

        if (gw_user_data.sm_len > WMS_GW_CB_MAX_PAGE_USER_DATA_LEN)
        {
          st = WMS_INVALID_CB_DATA_S;
        }
        else
        {
          /* Convert it from Number of Bytes to Number of Characters */
          gw_user_data.sm_len = (gw_user_data.sm_len * 8) / 7;

          client_ts_ptr->user_data.sm_len = gw_user_data.sm_len;

          if( client_ts_ptr->cb_page_hdr.cb_dcs.language ==
              WMS_GW_CB_LANGUAGE_ISO_639 )
          {
            /* The first 3 chars of user data is Language */
            client_ts_ptr->user_data.sm_len -= 3;
            memcpy( client_ts_ptr->user_data.sm_data,
                    gw_user_data.sm_data + 3,
                    client_ts_ptr->user_data.sm_len );
          }
          else
          {
            memcpy( client_ts_ptr->user_data.sm_data,
                    gw_user_data.sm_data,
            client_ts_ptr->user_data.sm_len );
          }
        }
        break;

      default:
        /* For 8-bit and 16-bit alphabets, no further decoding done */
        client_ts_ptr->user_data.sm_len
                         = (uint16)(raw_ts_data_ptr->len - WMS_GW_CB_PAGE_HEADER_LEN);

        if( client_ts_ptr->cb_page_hdr.cb_dcs.language ==
            WMS_GW_CB_LANGUAGE_ISO_639 )
        {
          client_ts_ptr->user_data.sm_len -= 2;
          memcpy( client_ts_ptr->user_data.sm_data,
                  raw_ts_data_ptr->data + WMS_GW_CB_PAGE_HEADER_LEN + 2,
          client_ts_ptr->user_data.sm_len );
        }
        else
        {
          memcpy( client_ts_ptr->user_data.sm_data,
                  raw_ts_data_ptr->data + WMS_GW_CB_PAGE_HEADER_LEN,
          client_ts_ptr->user_data.sm_len );
        }
        break;
    }
  }

  return st;

} /* wms_ts_decode_gw_cb() */


/*=========================================================================
FUNCTION
  wms_ts_decode

DESCRIPTION
  Decode Transport Service structure from raw bytes.

DEPENDENCIES
  None

RETURN VALUE
  status of decoding.

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_client_ts_data_s_type               * client_ts_data_ptr
)
{
  // TBD: GW format only
  wms_status_e_type   st = WMS_OK_S;
  wms_gw_pp_ts_data_s_type *msg;

  if( raw_ts_data_ptr == NULL || client_ts_data_ptr == NULL )  
  {
    LOGE("Null pointer in wms_ts_decode!");
    return WMS_NULL_PTR_S;
  }

  msg = & client_ts_data_ptr->u.gw_pp;

  pthread_mutex_lock( &wmsts_mutex );
  LOGI(">>>>>>>>>> pthread_mutex_lock() : wmsts_mutex >>>>>>>>>>\n" );

  switch( raw_ts_data_ptr->format )
  {
    case WMS_FORMAT_CDMA:
    case WMS_FORMAT_ANALOG_AWISMS:
    case WMS_FORMAT_ANALOG_CLI:
    case WMS_FORMAT_ANALOG_VOICE_MAIL:
    case WMS_FORMAT_ANALOG_SMS:
    case WMS_FORMAT_MWI:
      st = wms_ts_decode_bearer_data( raw_ts_data_ptr,
                                      & client_ts_data_ptr->u.cdma );
      break;

    case WMS_FORMAT_GW_PP:

      /* ---------- Start decoding --------- */

      msg->tpdu_type = raw_ts_data_ptr->tpdu_type;

      switch( msg->tpdu_type )
      {
        case WMS_TPDU_DELIVER:
          st = wms_ts_decode_deliver( raw_ts_data_ptr, & msg->u.deliver );
          break;


        case WMS_TPDU_DELIVER_REPORT_ACK:
          st = wms_ts_decode_deliver_report_ack( raw_ts_data_ptr,
                                                   & msg->u.deliver_report_ack );
          break;

        case WMS_TPDU_DELIVER_REPORT_ERROR:
          st = wms_ts_decode_deliver_report_error( raw_ts_data_ptr,
                                                     & msg->u.deliver_report_error );
          break;

        case WMS_TPDU_SUBMIT:
          st = wms_ts_decode_submit( raw_ts_data_ptr, & msg->u.submit );
          break;


        case WMS_TPDU_SUBMIT_REPORT_ACK:
          st = wms_ts_decode_submit_report_ack( raw_ts_data_ptr,
                                                  & msg->u.submit_report_ack );
          break;


        case WMS_TPDU_SUBMIT_REPORT_ERROR:
          st = wms_ts_decode_submit_report_error( raw_ts_data_ptr,
                                                    & msg->u.submit_report_error );

          break;


        case WMS_TPDU_STATUS_REPORT:
          st = wms_ts_decode_status_report( raw_ts_data_ptr,
                                            & msg->u.status_report);
          break;

        case WMS_TPDU_COMMAND:
          st = wms_ts_decode_command( raw_ts_data_ptr,
                                    & msg->u.command);
          break;

        default:
          LOGE("Invalid TPDU type %d", msg->tpdu_type);
          st = WMS_INVALID_TPDU_TYPE_S;
          break;
      }

      /* ---------- End of decoding --------- */
      break;

      case WMS_FORMAT_GW_CB:
        st = wms_ts_decode_gw_cb( raw_ts_data_ptr,
                                  & client_ts_data_ptr->u.gw_cb );
        break;

      default:
        st = WMS_INVALID_FORMAT_S;
        ERR("Invalid format: %d", raw_ts_data_ptr->format, 0,0);
        break;
  }

  client_ts_data_ptr->format = raw_ts_data_ptr->format;

  LOGI(">>>>>>>>>> pthread_mutex_unlock() : wmsts_mutex >>>>>>>>>>\n" );
  pthread_mutex_unlock( &wmsts_mutex );

  return st;

} /* wms_ts_decode() */


void wms_ts_decode_relative_time
(
  uint8                     v,
  wms_timestamp_s_type    * timestamp
)
{
  uint32                  i;

  if (timestamp != NULL)
  {
    memset( (void*) timestamp, 0, sizeof(wms_timestamp_s_type) );

    if( v < 144 )
    {
      i = ( v + 1 ) * 5; /* minutes */
      timestamp->hour    = (uint8)wms_ts_int_to_bcd((uint8)(i/60));
      timestamp->minute  = (uint8)wms_ts_int_to_bcd((uint8)(i % 60));
    }
    else if (v < 167)
    {
      i = ( v - 143 ) * 30; /* minutes */
      timestamp->hour    = (uint8)wms_ts_int_to_bcd((uint8)(12 + i/60));
      timestamp->minute  = (uint8)wms_ts_int_to_bcd((uint8)(i % 60));
    }
    else if (v < 197)
    {
      i = v - 166; /* days */
      timestamp->month    = (uint8)wms_ts_int_to_bcd((uint8)(i/30));
      timestamp->day      = (uint8)wms_ts_int_to_bcd((uint8)(i % 30));
    }
    else
    {
      i = ( v - 192 ) * 7; /* days */
      timestamp->year     = (uint8)wms_ts_int_to_bcd((uint8)(i/365));
      timestamp->month    = (uint8)wms_ts_int_to_bcd((uint8)((i%365)/30));
      timestamp->day      = (uint8)wms_ts_int_to_bcd((uint8)(( i % 365 ) % 30));
    }

  }
  else
  {
    LOGE("null pointer in wms_ts_decode_relative_time");
  }
  /* done */
  return;

} /* wms_ts_decode_relative_time() */



uint8 wms_ts_encode_relative_time
(
  const wms_timestamp_s_type  *timestamp
)
{
  uint32    i;
  uint8     v = 0, j;
  /* round up to the next time unit boundary*/

  if (timestamp != NULL)
  {
    if ( !wms_ts_bcd_to_int(timestamp->year, &j) )
    {
      LOGE("Year is invalid: %d", j);
    }
    i = j * 365;

    if ( !wms_ts_bcd_to_int(timestamp->month, &j) )
    {
      LOGE("Month is invalid: %d", j);
    }
    i = i + j * 30;

    if ( !wms_ts_bcd_to_int(timestamp->day, &j) )
    {
      LOGE("Day is invalid: %d", j);
    }
    i += j;

    if( i > 30 )
    {
      /* 197 - 255: (TP-VP - 192) weeks */
      v = (uint8) ( (i+6) / 7 + 192 );
    }
    else if( i >= 1 )
    {
      /* 168 to 196: (TP-VP - 166 ) days */
      v = (uint8) ( i + 166 );
    }
    else
    {
      if ( !wms_ts_bcd_to_int(timestamp->day, &j) )
      {
        LOGE("Day is invalid: %d", j);
      }
      i = j * 24 * 60;

      if ( !wms_ts_bcd_to_int(timestamp->hour, &j) )
      {
        LOGE("Hour is invalid: %d", j);
      }
      i = i + j * 60;

      if ( !wms_ts_bcd_to_int(timestamp->minute, &j) )
      {
        LOGE("Minute is invalid: %d", j);
      }
      i += j;

      if( i > 12 * 60 ) /* greater than 12 hours */
      {
        /* 144 - 167: 12 hours + ( (TP-VP - 143) * 30 minutes ) */
        v = (uint8) ( ( i - ((12 * 60) + 29) ) / 30 + 143 );
      }
      else
      {
        /* 0 - 143: (TP-VP + 1) * 5 minutes */
        v = (uint8) ( ( i + 4 ) / 5 - 1 );
      }
    }
  }
  else
  {
    LOGE("null pointer in wms_ts_encode_relative_time");
  }
  /* done */
  return v;

} /* wms_ts_encode_relative_time() */

uint8 wms_ts_decode_gw_cb_dcs
(
  const uint8                  data,            /* INPUT */
  const uint8                  * user_data_ptr, /* INPUT */
  wms_gw_cb_dcs_s_type         * dcs_ptr        /* OUTPUT */
)
{
  uint32  i;
  uint8 pos = 0;

  if( user_data_ptr == NULL || dcs_ptr == NULL )
  {
    LOGE("Null pointer in wms_ts_decode_gw_cb_dcs!");
    return 0;
  }


  dcs_ptr->group         = WMS_GW_CB_DCS_GROUP_RESERVED;
  dcs_ptr->msg_class         = WMS_MESSAGE_CLASS_NONE;
  dcs_ptr->is_compressed = FALSE;
  dcs_ptr->alphabet      = WMS_GW_ALPHABET_7_BIT_DEFAULT;
  dcs_ptr->language      = WMS_GW_CB_LANGUAGE_UNSPECIFIED;
  memset( dcs_ptr->iso_639_lang, 0, sizeof(dcs_ptr->iso_639_lang) );
  dcs_ptr->is_udh_present = FALSE;

  i = (data & 0xF0) >> 4;

  switch( i )
  {
    case 0:
    case 2:
    case 3:
      dcs_ptr->group         = WMS_GW_CB_DCS_GROUP_DEFINED;
      dcs_ptr->language      = (wms_gw_cb_language_e_type) data;
      break;

    case 1:
      if( data == 0x10 || data == 0x11 )
      {
        dcs_ptr->group         = WMS_GW_CB_DCS_GROUP_DEFINED;
        dcs_ptr->alphabet      = (data & 0x01) ?
                        WMS_GW_ALPHABET_UCS2 : WMS_GW_ALPHABET_7_BIT_DEFAULT;
        dcs_ptr->language      = WMS_GW_CB_LANGUAGE_ISO_639;

        if (user_data_ptr != NULL)
        {
          dcs_ptr->iso_639_lang[0]  = user_data_ptr[0] & 0x7F;
          dcs_ptr->iso_639_lang[1]  = ( user_data_ptr[0] & 0x80 ) >> 7;
          dcs_ptr->iso_639_lang[1] |= ( user_data_ptr[1] & 0x3F ) << 1;
          dcs_ptr->iso_639_lang[2]  = 13; /* CR char in GSM 7-bit Alphabet */
        }
        else
        {
          LOGE("No User Data Ptr for decoding ISO 639 language");

          /* Default it to English if user_data_ptr is not passed */
          dcs_ptr->iso_639_lang[0] = 0x45;  /* E */
          dcs_ptr->iso_639_lang[1] = 0x4E;  /* N */
          dcs_ptr->iso_639_lang[2] = 13;    /* CR */
        }
      }
      break;

    case 4:
    case 5:
    case 6:
    case 7:
      dcs_ptr->group         = WMS_GW_CB_DCS_GROUP_DEFINED;
      if( data & 0x10 )
      {
        dcs_ptr->msg_class         = (wms_message_class_e_type) (data & 0x03);
      }
      else
      {
        dcs_ptr->msg_class         = WMS_MESSAGE_CLASS_NONE;
      }
      dcs_ptr->is_compressed = (data & 0x20) >> 5;
      dcs_ptr->alphabet      = (wms_gw_alphabet_e_type) ((data & 0x0C) >> 2);
      break;

    case 14:
      dcs_ptr->group = WMS_GW_CB_DCS_GROUP_WAP;
      break;

    case 15:
      dcs_ptr->group         = WMS_GW_CB_DCS_GROUP_DEFINED;
      dcs_ptr->msg_class         = (wms_message_class_e_type) (data & 0x03);
      if( dcs_ptr->msg_class == 0 )
      {
        dcs_ptr->msg_class = WMS_MESSAGE_CLASS_NONE;
      }
      dcs_ptr->alphabet      = (wms_gw_alphabet_e_type) ((data & 0x04) >> 2);
      break;

    default:
      /* case 8, 9, 10, 11, 12, 13, reserved coding groups */
      LOGI("gw cb dcs, reserved coding groups");
      break;
  }

  if( dcs_ptr->alphabet > WMS_GW_ALPHABET_UCS2 )
  {
    dcs_ptr->alphabet = WMS_GW_ALPHABET_7_BIT_DEFAULT;
  }

  dcs_ptr->raw_dcs_data = data;
  pos ++;

  return pos;

} /* wms_ts_decode_gw_cb_dcs() */

uint8 wms_ts_decode_gw_cb_page_header
(
  const uint8                   * data,
  wms_gw_cb_page_header_s_type  * page_header_ptr
)
{
  uint8             pos = 0;

  if( data == NULL || page_header_ptr == NULL )
  {
    LOGE("Null pointer in wms_ts_decode_gw_cb_page_header!");
    return 0;
  }


  /* Copy the original Serial Number data */
  page_header_ptr->raw_serial_number = (data[pos] << 8) | data[pos+1];

  page_header_ptr->geo_scope      =
                    (wms_gw_cb_geo_scope_e_type) ( (data[pos] & 0xC0) >> 6 );

  page_header_ptr->message_code   = ( data[pos] & 0x30 ) << 4;
  page_header_ptr->message_code  |= ( data[pos] & 0x0F ) << 4;
  pos ++;
  page_header_ptr->message_code  |= ( data[pos] & 0xF0 ) >> 4;

  page_header_ptr->update_number  = data[pos] & 0x0F;
  pos ++;

  page_header_ptr->cb_srv_id      = data[pos] << 8;
  pos ++;
  page_header_ptr->cb_srv_id     |= data[pos];
  pos ++;

  if( wms_ts_decode_gw_cb_dcs( data[pos],
                               data+WMS_GW_CB_PAGE_HEADER_LEN,
                               & page_header_ptr->cb_dcs ) != 0 )
  {
    pos ++;
    page_header_ptr->page_number  = (data[pos] & 0xF0) >> 4;
    page_header_ptr->total_pages  = data[pos] & 0x0F;
    pos ++;

    /* treat as single message if either field is 0000 */
    if (page_header_ptr->page_number == 0 || page_header_ptr->total_pages == 0)
    {
      page_header_ptr->page_number = 0x0001;
      page_header_ptr->total_pages = 0x0001;
    }
    if (page_header_ptr->page_number > page_header_ptr->total_pages)
    {
      LOGE("Invalid GW CB DCS, page number > total pages, %d > %d", page_header_ptr->page_number, page_header_ptr->total_pages);
      pos = 0;
    }
  }
  else
  {
    pos = 0;
  }

  return pos;

} /* wms_ts_decode_gw_cb_page_header() */


/*=========================================================================
FUNCTION
  wms_ts_get_udh_length

DESCRIPTION
  Allow the client to get the header length for the given user data header.

DEPENDENCIES
  None

RETURN VALUE
  User Data Header Length

SIDE EFFECTS
  None
=========================================================================*/
uint8 wms_ts_get_udh_length
(
  const wms_udh_s_type                    *udh
)
{
  uint8 length = 0;
  if (udh != NULL)
  {
    switch(udh->header_id)
    {
      /* 0x00 */
      case WMS_UDH_CONCAT_8:
        /* Header ID Octet + Header Length Octet + Header Length */
        length = 1 + 1 + WMS_UDH_OCTETS_CONCAT8;
        break;

      /* 0x08 */
      case WMS_UDH_CONCAT_16:
        /* Header ID Octet + Header Length Octet + Header Length */
        length = 1 + 1 + WMS_UDH_OCTETS_CONCAT16;
        break;

      /* 0x01 */
      case WMS_UDH_SPECIAL_SM:
        /* Header ID Octet + Header Length Octet + Header Length */
        length = 1 + 1 + WMS_UDH_OCTETS_SPECIAL_SM;
        break;

      /* 0x02 - 0x03 Reserved */

      /* 0x04 */
      case WMS_UDH_PORT_8:
        /* Header ID Octet + Header Length Octet + Header Length */
        length = 1 + 1 + WMS_UDH_OCTETS_PORT8;
        break;

      /* 0x05 */
      case WMS_UDH_PORT_16:
        /* Header ID Octet + Header Length Octet + Header Length */
        length = 1 + 1 + WMS_UDH_OCTETS_PORT16;
        break;

      /* 0x06 */
      case WMS_UDH_SMSC_CONTROL:
        /* Header ID Octet + Header Length Octet + Header Length */
        length = 1 + 1 + udh->u.other.header_length;
        break;

      /* 0x07 */
      case WMS_UDH_SOURCE:
        /* Header ID Octet + Header Length Octet + Header Length */
        length = 1 + 1 + udh->u.other.header_length;
        break;

      /* 0x09 */
      case WMS_UDH_WCMP:
        /* Header ID Octet + Header Length Octet + Header Length */
        length = 1 + 1 + udh->u.other.header_length;
        break;

      /* 0x0A */
      case WMS_UDH_TEXT_FORMATING:
        /* Header ID Octet + Header Length Octet + Header Length */
        if(!udh->u.text_formating.is_color_present) {
          length = 1 + 1 + WMS_UDH_OCTETS_TEXT_FORMATTING;
        }
        else {
          /* Header ID Octet + Header Length Octet + Header Length + text color octet */
          length = 1 + 1 + WMS_UDH_OCTETS_TEXT_FORMATTING + 1;
        }
        break;

      /* 0x0B */
      case WMS_UDH_PRE_DEF_SOUND:
        /* Header ID Octet + Header Length Octet + Header Length */
        length = 1 + 1 + WMS_UDH_OCTETS_PRE_DEF;
        break;

      /* 0x0C */
      case WMS_UDH_USER_DEF_SOUND:
        /* Header ID Octet + Header Length Octet + Header Length + 1 (for position octet)*/
        length = 1 + 1 + udh->u.user_def_sound.data_length + 1;
        break;

      /* 0x0D */
      case WMS_UDH_PRE_DEF_ANIM:
        /* Header ID Octet + Header Length Octet + Header Length */
        length = 1 + 1 + WMS_UDH_OCTETS_PRE_DEF;
        break;

      /* 0x0E */
      case WMS_UDH_LARGE_ANIM:
        /* Header ID Octet + Header Length Octet + Header Length + 1 (for position octet)*/
        length = 1 + 1 + WMS_UDH_LARGE_BITMAP_SIZE * WMS_UDH_ANIM_NUM_BITMAPS + 1;
        break;

      /* 0x0F */
      case WMS_UDH_SMALL_ANIM:
        /* Header ID Octet + Header Length Octet + Header Length + 1 (for position octet)*/
        length = 1 + 1 + WMS_UDH_SMALL_BITMAP_SIZE * WMS_UDH_ANIM_NUM_BITMAPS + 1;
        break;

      /* 0x10 */
      case WMS_UDH_LARGE_PICTURE:
        /* Header ID Octet + Header Length Octet + Header Length + 1 (for position octet)*/
        length = 1 + 1 + WMS_UDH_LARGE_PIC_SIZE + 1;
        break;

      /* 0x11 */
      case WMS_UDH_SMALL_PICTURE:
        /* Header ID Octet + Header Length Octet + Header Length + 1 (for position octet)*/
        length = 1 + 1 + WMS_UDH_SMALL_PIC_SIZE + 1;
        break;

      /* 0x12 */
      case WMS_UDH_VAR_PICTURE:
        /* Header ID Octet + Header Length Octet + Header Length + 3 (for height, width and position octets)*/
        length = 1 + 1 + (uint8)(udh->u.var_picture.height * udh->u.var_picture.width/8) + 3;
        break;

      /* 0x13 - 0x1F Reserved for future EMS*/

      /* 0x20 */
      case WMS_UDH_RFC822:
        /* Header ID Octet + Header Length Octet + Header Length*/
        length = 1 + 1 + WMS_UDH_OCTETS_RFC822;
        break;

     /*  0x21 - 0x6F Reserved for future use*/
     /*  0x70 - 0x7f Reserved for (U)SIM Toolkit Security Headers */
     /*  0x80 - 0x9F SME to SME specific use*/
     /*  0xA0 - 0xBF Reserved for future use*/
     /*  0xC0 - 0xDF SC specific use*/
     /*  0xE0 - 0xFF Reserved for future use*/
      case WMS_UDH_USER_PROMPT:
        length = 1 + 1 + WMS_UDH_OCTETS_USER_PROMPT;
        break;

      case WMS_UDH_EXTENDED_OBJECT:
        length = 1 + 1 + udh->u.eo.content.length;
        if( udh->u.eo.first_segment == TRUE )
        {
          length += WMS_UDH_OCTETS_EO_HEADER;
        }
        break;

      default:
        /* Header ID Octet + Header Length Octet + Header Length */
        length = 1 + 1 + udh->u.other.header_length;
        break;

    }
  }
  return length;
}

/*=========================================================================
FUNCTION
  wms_ts_compute_user_data_header_length

DESCRIPTION
  Computes the User Data Header Length

DEPENDENCIES
  None

RETURN VALUE
  User Data Header Length in bytes

SIDE EFFECTS
  None
=========================================================================*/
uint32 wms_ts_compute_user_data_header_length
(
  const uint8           num_headers,
  const wms_udh_s_type *headers
)
{
  uint32 length = 0;

  uint32 i;

  if( headers == NULL )
  {
    LOGE("Null pointer in wms_ts_compute_user_data_header_length!");
    return 0;
  }

  if (num_headers > 0)
  {
    length += 1; /* 1 byte for UDHL byte */

    /* User Data Headers Length */
    for ( i=0 ; i<num_headers && i<WMS_MAX_UD_HEADERS ; i++)
    {
      length += (uint32)wms_ts_get_udh_length(&headers[i]);
    }
  }

  return length;
}


/*=========================================================================
FUNCTION
  wms_ts_compute_gw_user_data_length

DESCRIPTION
  Computes the GW User Data Length

DEPENDENCIES
  None

RETURN VALUE
  GW User Data Length in bytes

SIDE EFFECTS
  None
=========================================================================*/
uint32 wms_ts_compute_gw_user_data_length
(
  const wms_gw_dcs_s_type         *dcs,
  const wms_gw_user_data_s_type   *user_data
)
{
  uint32 length = 0;

  if( dcs == NULL || user_data == NULL )
  {
    LOGE("Null pointer in wms_ts_compute_gw_user_data_length!");
    return 0;
  }

  length += wms_ts_compute_user_data_header_length(user_data->num_headers, user_data->headers);

  if( dcs->alphabet == WMS_GW_ALPHABET_7_BIT_DEFAULT )
  {
    length += ((user_data->sm_len * 7) + 7)/8;
  }
  else
  {
    length += user_data->sm_len;
  }

  return length;

} /* wms_ts_compute_gw_user_data_length */


/*=========================================================================
FUNCTION
  wms_ts_ascii_to_bcd

DESCRIPTION
  Convert an ASCII number string to BCD digits.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
=========================================================================*/
void wms_ts_ascii_to_bcd
(
  const uint8     * ascii,
  uint8           len,
  uint8           * out
)
{
  uint8   i;

  for ( i=0; i<len; i++ )
  {
    switch ( ascii[i] )
    {
    case '*':
      out[i] = 0xA;
      break;
    case '#':
      out[i] = 0xB;
      break;
    default:
      out[i] = ascii[i] - '0';
      break;
    }
  }

  /* done */
  return;

} /* wms_ts_ascii_to_bcd() */


/*=========================================================================
FUNCTION
  wms_ts_bcd_to_ascii

DESCRIPTION
  Convert BCD digits to an ASCII number string.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
=========================================================================*/
void wms_ts_bcd_to_ascii
(
  const uint8         * addr,
  uint8               len,
  uint8               * out
)
{
  uint8   i;

  for ( i=0; i<len; i++ )
  {
    uint8   digit; /* to store stripped digit */

    digit = addr[i] & 0x0F;

    switch ( digit )
    {
    case 0xA:
      out[i] = '*';
      break;
    case 0xB:
      out[i] = '#';
      break;
    case 0xC:
    case 0xD:
    case 0xE:
      out[i] = (digit - 0xC) + 'a';
      break;
    case 0xF:
      /* Ignore it */
      out[i] = ' ';
      break;
    default:
      out[i] = digit + '0';
      break;
    }
  }

  /* done */
  return;

} /* wms_ts_bcd_to_ascii() */

