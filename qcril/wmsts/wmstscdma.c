/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

          W I R E L E S S    M E S S A G I N G   S E R V I C E S

             ------ CDMA Message Encoding and Decoding Services

GENERAL DESCRIPTION
  This source file contains the encoding and decoding functions of CDMA SMS
  messages.

INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

Copyright (c) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 
2008 by Qualcomm Technologies, Inc.  All Rights Reserved.

Export of this technology or software is regulated by the U.S. Government.
Diversion contrary to U.S. law prohibited.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


/* <EJECT> */
/*===========================================================================

$Header: 

===========================================================================*/


/* <EJECT> */
/*===========================================================================

                             INCLUDE FILES

===========================================================================*/

#include "target.h"
#include "comdef.h"
#include "wms.h"

#include "wmsts.h"
#include "wmstscdma.h"
#include "bit.h"
#include "err.h"
#include <string.h>
#include "tmc.h"

#define LOG_TAG "WMSTSCDMA"
#ifdef FEATURE_UNIT_TEST
#include "Log.h"
#else
#include <utils/Log.h>
#include "common_log.h"
#endif /* FEATURE_UNIT_TEST */

#define SIZE_CHECK  \
         { if( pos > rec_len ) \
           { \
             LOGE("Encode SMSP err: pos=%d>rec_len=%d", pos, rec_len); \
             return FALSE; \
           } \
         }

#define TL_HEADER_SIZE    1
#define TL_PARM_SIZE      2

#define MSG_DUP_PARM LOGE("Duplicate parm: %d", parm_id, 0,0)
static wms_raw_ts_data_s_type  raw_ts;

static wms_cdma_user_data_s_type  cdma_user_data;

/* <EJECT> */
/*===========================================================================

                              FUNCTIONS

===========================================================================*/

/*=========================================================================
FUNCTION
  wms_ts_encode_IS91EP_payload

DESCRIPTION
  This function encodes the IS91 EP data to the raw bearer data structure.
  (see IS-637 4.3.1.4.1)

DEPENDENCIES
  None

RETURN VALUE
  Status of the translation.

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_encode_IS91EP_payload
(
  const wms_client_bd_s_type     * cl_bd_ptr,       /* IN */
  wms_client_bd_s_type           * is91ep_raw_bd    /* OUT */
)
{

  wms_status_e_type     st = WMS_OK_S;
  uint8                  i;
  uint8                  vmn_cnt = 2;


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  if( cl_bd_ptr == NULL || is91ep_raw_bd == NULL )
  {
    LOGE("Null pointer in wms_ts_encode_IS91EP_payload!");
    return WMS_NULL_PTR_S;
  }

  switch( cl_bd_ptr->user_data.is91ep_type )
  {
    case WMS_IS91EP_CLI_ORDER:

      /* Put the callback number back into user_data */
      for( i = 0; i < cl_bd_ptr->callback.number_of_digits; i++ )
      {
        /* (see IS-91 Appendix B) */
        is91ep_raw_bd->user_data.data[i] =
                               cl_bd_ptr->callback.digits[i];
      }
      /* Assign user_data parameters to appropriate values */
      is91ep_raw_bd->user_data.number_of_digits =
                              cl_bd_ptr->callback.number_of_digits;
      is91ep_raw_bd->user_data.data_len =
                               is91ep_raw_bd->user_data.number_of_digits;
      break;

    case WMS_IS91EP_VOICE_MAIL:

      /* Put the num of msgs in the first 2 bytes of the user data */
      is91ep_raw_bd->user_data.data[0] = cl_bd_ptr->num_messages / 10;
      is91ep_raw_bd->user_data.data[1] = cl_bd_ptr->num_messages % 10;

      if(is91ep_raw_bd->user_data.data[0] > 9)
      {
        LOGE( "No resources: Over 100 voice mails %d", cl_bd_ptr->user_data.is91ep_type);
        st = WMS_GENERAL_ERROR_S;
        break;
      }


      if( cl_bd_ptr->mask & WMS_MASK_BD_PRIORITY )
      {
        /* update to 3 bytes of data used including priority */
        vmn_cnt = 3;
        is91ep_raw_bd->user_data.data[2] = cl_bd_ptr->priority ? 1 : 0;
      }

      /* Convert to raw bd format, but don't forget to leave the space for the
         bytes used for num of msgs and priority
      */
      for( i=0; i < cl_bd_ptr->user_data.number_of_digits; i++ )
      {
        /* (see IS-91 Appendix B) */
        is91ep_raw_bd->user_data.data[ i + vmn_cnt ] =
                cl_bd_ptr->user_data.data[i] - 0x20;
      }

      /* Assign user_data parameters to appropriate values */
      is91ep_raw_bd->user_data.number_of_digits =
                               cl_bd_ptr->user_data.number_of_digits + vmn_cnt;
      is91ep_raw_bd->user_data.data_len =
                               is91ep_raw_bd->user_data.number_of_digits;

      break;


    case WMS_IS91EP_SHORT_MESSAGE:
    case WMS_IS91EP_SHORT_MESSAGE_FULL:

      is91ep_raw_bd->user_data.number_of_digits =
                               cl_bd_ptr->user_data.number_of_digits;
      /* convert the digits to raw bearer data format
      */
      for( i = 0; i < cl_bd_ptr->user_data.number_of_digits; i++ )
      {
        /* (see IS-91 Appendix B) */
        is91ep_raw_bd->user_data.data[i] =
                               cl_bd_ptr->user_data.data[i] - 0x20;
      }

      is91ep_raw_bd->user_data.data_len =
                               cl_bd_ptr->user_data.number_of_digits;
      break;


    default:
      /* Should not happen !! */
      LOGI( "wsm_ts_encode_IS91EP_payload: invalid IS91EP type: %d",
                cl_bd_ptr->user_data.is91ep_type);
      st = WMS_GENERAL_ERROR_S;
      break;

  } /* switch */


  /* done */
  return st;

} /* end of uasmsx_encode_IS91EP_payload() */


/* A utility function to return the digit size of the encoding type.
** NOTE: variable digit size encodings are not supported. Only the following
** digit sizes are supported: 7, 8, 16.   8 is returned for unsupported sizes.
*/
uint8 wms_ts_cdma_encoding_digit_size
(
  wms_user_data_encoding_e_type  encoding
)
{
  switch( encoding )
  {
    case WMS_ENCODING_ASCII:
    case WMS_ENCODING_IA5:
    case WMS_ENCODING_GSM_7_BIT_DEFAULT:
      return 7;

    case WMS_ENCODING_UNICODE:
      return 16;

    case WMS_ENCODING_OCTET:
    case WMS_ENCODING_IS91EP:
    case WMS_ENCODING_SHIFT_JIS:
    case WMS_ENCODING_KOREAN:
    case WMS_ENCODING_LATIN_HEBREW:
    case WMS_ENCODING_LATIN:
    default:
      return 8;
  }
}


/*=========================================================================
FUNCTION
  wms_ts_encode_CDMA_bd

DESCRIPTION
  This function encodes the bearer data from the client structure to
  its raw format in CDMA mode.

DEPENDENCIES
  None

RETURN VALUE
  Status of the translation

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_encode_CDMA_bd
(
  const wms_client_bd_s_type                * cl_bd_ptr,       /* IN */
  wms_raw_ts_data_s_type                    * raw_bd_ptr       /* OUT */
)
{
  uint16                    pos;      /* running position in bytes */
  uint16                    bit_pos;  /* position in bits */
  uint8                    parm_len;
  uint8                    * parm_len_ptr;
  uint8                    i;
  uint8                    digit_size = 8;
  uint8                   data_length = 0;

  static wms_client_bd_s_type    is91ep_raw_bd;

  wms_status_e_type       st = WMS_OK_S;
  uint8                    * data;
  uint8                   padding_bits = 0;
  int                     num_digits =0;

  uint16                    j, k;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    /*---- checking ---- */
  if ( cl_bd_ptr == NULL || raw_bd_ptr == NULL)
  {
    LOGE("wms_ts_encode_CDMA_bd null ptr!");
    return WMS_NULL_PTR_S;
  }

  data = raw_bd_ptr->data;

  /* encoding for only CDMA format is needed at this time */
  raw_bd_ptr->format = WMS_FORMAT_CDMA;

  pos = 0;

  if( cl_bd_ptr->mask & WMS_MASK_BD_MSG_ID )
  {
    data[pos] = (uint8) WMS_BD_MSG_ID; /* parm id */
    pos++;

    data[pos] = parm_len = 3;
    pos++; /* skip parm len */

    bit_pos = (uint16)(8 * pos);

    b_packb( (uint8) cl_bd_ptr->message_id.type,
             data,
             (uint16)(bit_pos),
             4 );
    bit_pos += 4;

    b_packw( (uint16) cl_bd_ptr->message_id.id_number,
             data,
             (uint16)(bit_pos),
             16 );
    bit_pos += 16;

    b_packb( (uint8) cl_bd_ptr->message_id.udh_present,
             data,
             bit_pos,
             1 );
    bit_pos += 1;

    /* pad reserved bits with 0
    */
    b_packb( 0,
             data,
             bit_pos,
             3 );
    bit_pos += 3;
    pos += parm_len; /* skip parm data */

  } /* end of MsgId */


  if( (cl_bd_ptr->mask & WMS_MASK_BD_USER_DATA) ||
      ( cl_bd_ptr->user_data.encoding == WMS_ENCODING_IS91EP &&
       cl_bd_ptr->user_data.is91ep_type == WMS_IS91EP_CLI_ORDER) )
  {
    data[pos] = (uint8) WMS_BD_USER_DATA; /* parm id */
    pos++;

    parm_len_ptr = data+pos;
    pos++; /* skip parm len */

    bit_pos = (uint16)(pos * 8);

    b_packb( (uint8) cl_bd_ptr->user_data.encoding,
             data,
             bit_pos,
             5 );
    bit_pos += 5;


    if( cl_bd_ptr->user_data.encoding == WMS_ENCODING_IS91EP )
    {
      /* verify that number_of_digits == data_len
      */
      if( cl_bd_ptr->user_data.number_of_digits !=
          cl_bd_ptr->user_data.data_len )
      {
        LOGI("enc UD sz err: %d!=%d", cl_bd_ptr->user_data.number_of_digits,
                  cl_bd_ptr->user_data.data_len);
        st = WMS_INVALID_USER_DATA_SIZE_S;
      }

      if( st == WMS_OK_S )
      {
        b_packb( (uint8) cl_bd_ptr->user_data.is91ep_type,
                 data,
                 bit_pos,
                 8 );
        bit_pos += 8;

        if( cl_bd_ptr->user_data.is91ep_type == WMS_IS91EP_CLI_ORDER )
        {
          digit_size = 4;
        }
        else
        {
          digit_size = 6;
        }

        /* Must encode is91ep first before packing user_data */
        st = wms_ts_encode_IS91EP_payload(cl_bd_ptr, & is91ep_raw_bd);
      } /* if st==OK */
    }
    else /* all the other encoding types */
    {
      /* will pack the raw bits */
      digit_size = 8;
    }


    if( st == WMS_OK_S )
    {

      /* pack the num_fields parameter
      */
      if( cl_bd_ptr->message_id.udh_present == TRUE )
      {
        uint8   udhl = 0;
        uint8   fill_bits = 0;
        int     position =0;
        uint8   shift_bits = 0;

        num_digits = cl_bd_ptr->user_data.number_of_digits;

        memset(raw_ts.data,0,WMS_MAX_LEN);

        /* Check for Memory Corruption - raw_ts.data[WMS_MAX_LEN] */
        if (wms_ts_compute_user_data_header_length(cl_bd_ptr->user_data.num_headers, cl_bd_ptr->user_data.headers) <= WMS_CDMA_USER_DATA_MAX)
        {
          /* Use raw_ts as a temporay buffer of the result */
          udhl = wms_ts_encode_user_data_header(
                                         cl_bd_ptr->user_data.num_headers,
                                         cl_bd_ptr->user_data.headers,
                                         raw_ts.data ); /* OUT */

        }
        else
        {
          LOGE("Encode User Data Header Exceeds Capacity - Skipping UDH");
          st = WMS_INVALID_USER_DATA_SIZE_S;
        }

        data_length = udhl+1;
        if( cl_bd_ptr->user_data.encoding == WMS_ENCODING_GSM_7_BIT_DEFAULT ||
          cl_bd_ptr->user_data.encoding == WMS_ENCODING_ASCII || cl_bd_ptr->user_data.encoding == WMS_ENCODING_IA5)
        {
          uint32 total_bits_occupied = (udhl+1)*8;
          fill_bits = (uint8)total_bits_occupied % 7;

          if (fill_bits != 0)
          {
            fill_bits = 7 - fill_bits;
          }

          // Calculate the new value of number of septets that make the User data
          num_digits = (int) (cl_bd_ptr->user_data.number_of_digits * 7
                       + total_bits_occupied + fill_bits) / 7;

          position = udhl+ 1;
          shift_bits = fill_bits;
        }
        else if ( cl_bd_ptr->user_data.encoding == WMS_ENCODING_UNICODE )
        {
          /* if not an even number, add fill bits */
          if ( (udhl+1) % 2 != 0)
          {
            fill_bits = 8;
          }
          num_digits = (cl_bd_ptr->user_data.number_of_digits * 16 + (udhl + 1)*8 + fill_bits)/16;
          shift_bits = fill_bits;
        }
        else
        {
          num_digits += udhl + 1;
        }

        /* Pack in the num_fields */
        b_packb( (uint8)num_digits,
                 data,
                 bit_pos,
                 8 );

       /* Concatenate the result of encoding headers with the Characters */
        if( cl_bd_ptr->user_data.encoding == WMS_ENCODING_GSM_7_BIT_DEFAULT )
        {
          /*Returns the number of bytes used*/
          data_length +=(uint8) wms_ts_pack_gw_7_bit_chars(
                                       cl_bd_ptr->user_data.data,
                                       cl_bd_ptr->user_data.number_of_digits,
                                       shift_bits,
                                       (uint16) (WMS_MAX_LEN - (bit_pos/8+position)),
                                       raw_ts.data + position); /* OUT */

          padding_bits = 3;//Encoding=5 bits

        }
        else if(cl_bd_ptr->user_data.encoding == WMS_ENCODING_ASCII ||
          cl_bd_ptr->user_data.encoding == WMS_ENCODING_IA5){
          
          for(i = 0; i<cl_bd_ptr->user_data.data_len ; i++)
          {
            b_packb((uint8)cl_bd_ptr->user_data.data[i], 
                  (uint8 *)raw_ts.data,
                  (uint16)(udhl+1)*8+fill_bits + (i*digit_size), 
                  digit_size);
          }
          data_length += cl_bd_ptr->user_data.data_len;
        }
        else if ( cl_bd_ptr->user_data.encoding == WMS_ENCODING_UNICODE )
        {
          memcpy( raw_ts.data + udhl + 1 + shift_bits/8,
                  cl_bd_ptr->user_data.data,
                  cl_bd_ptr->user_data.data_len);
          data_length = cl_bd_ptr->user_data.data_len + udhl + 1 + shift_bits/8;
        }
        else
        {
          memcpy( raw_ts.data+udhl+1,
                  cl_bd_ptr->user_data.data,
                  cl_bd_ptr->user_data.data_len );
          data_length = cl_bd_ptr->user_data.data_len + udhl + 1;
          /* data_length will be used below in the for loop */
        }

      }
      else /* ELSE for if (cl_bd_ptr->message_id.udh_present == TRUE )*/
      if( cl_bd_ptr->user_data.encoding == WMS_ENCODING_IS91EP )
      {
        b_packb( (uint8) is91ep_raw_bd.user_data.number_of_digits,
                 data,
                 bit_pos,
                 8 );
      }
      else
      {
        b_packb( cl_bd_ptr->user_data.number_of_digits,
                 data,
                 bit_pos,
                 8 );
      }
      bit_pos += 8;


      /* Use the correct data_len */
      if( cl_bd_ptr->user_data.encoding == WMS_ENCODING_IS91EP )
      {
        data_length = is91ep_raw_bd.user_data.data_len;
      }
      else
      {
       if( cl_bd_ptr->message_id.udh_present == TRUE )
       {
         // do nothing
       }
       else
         if(cl_bd_ptr->user_data.encoding == WMS_ENCODING_GSM_7_BIT_DEFAULT)
         {
               data_length = (uint8) wms_ts_pack_gw_7_bit_chars(
                                       cl_bd_ptr->user_data.data,
                                       cl_bd_ptr->user_data.number_of_digits,
                                       0,
                                       (uint16) WMS_MAX_LEN - (bit_pos/8),
                                       raw_ts.data);
         }
         else
         {
           data_length = cl_bd_ptr->user_data.data_len;
         }
      }

      /* pack the raw data bits
      */
      for( i=0;
           i < data_length;
           i++, bit_pos += digit_size )
      {
        if( i >= WMS_CDMA_USER_DATA_MAX )
        {
          LOGI( "encoding: UD too big: %d",
                    cl_bd_ptr->user_data.data_len);
          st = WMS_INVALID_USER_DATA_SIZE_S;
          break; /* out of for loop */
        }

        if( cl_bd_ptr->message_id.udh_present == TRUE )
        {
          b_packb( raw_ts.data[i],
                   data,
                   bit_pos,
                   digit_size );
        }
        else /* ELSE */
        if( cl_bd_ptr->user_data.encoding == WMS_ENCODING_IS91EP )
        {
          b_packb( is91ep_raw_bd.user_data.data[i],
                 data,
                 bit_pos,
                 digit_size );
        }
        else if(cl_bd_ptr->user_data.encoding == WMS_ENCODING_GSM_7_BIT_DEFAULT)
        {
          b_packb( raw_ts.data[i],
                   data,
                   bit_pos,
                   digit_size );
        }
        else
        {
          b_packb( cl_bd_ptr->user_data.data[i],
                   data,
                   bit_pos,
                   digit_size );
        }
      } /* for */

    } /* if st==OK */


    if( st == WMS_OK_S )
    {
      /*
      ** The 'data' field has padded with bits of 'padding_bits', which should
      ** be subtracted from the final padding.
      */
      if (cl_bd_ptr->message_id.udh_present == TRUE)
      {
        // Add The Padding bits to make an octet boundary
        bit_pos += padding_bits;
      }
      else
      {
        bit_pos -= cl_bd_ptr->user_data.padding_bits;
      }

      if( bit_pos % 8 != 0 )
      {
        /* pad 0 to remaining bits
        */
        digit_size = 8 - bit_pos % 8;
        b_packb( 0, data, bit_pos,  digit_size );
        bit_pos += digit_size;
      }

      * parm_len_ptr = (uint8)(bit_pos / 8 - pos);

      pos += * parm_len_ptr; /* skip parm data */
    }

  } /* end of UserData encoding */


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_USER_RESP )
  {
    data[pos] = (uint8) WMS_BD_USER_RESP; /* parm id */
    pos++;

    data[pos] = parm_len = 1;
    pos++; /* skip parm len */

    data[pos] = cl_bd_ptr->user_response;

    pos += parm_len; /* skip parm data */

  }


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_MC_TIME )
  {
    data[pos] = (uint8) WMS_BD_MC_TIME; /* parm id */
    pos++;

    data[pos] = 6;
    pos++; /* skip parm len */

    data[pos++] = cl_bd_ptr->mc_time.year;
    data[pos++] = cl_bd_ptr->mc_time.month;
    data[pos++] = cl_bd_ptr->mc_time.day;
    data[pos++] = cl_bd_ptr->mc_time.hour;
    data[pos++] = cl_bd_ptr->mc_time.minute;
    data[pos++] = cl_bd_ptr->mc_time.second;

    /* pos: parm data already skipped */
  }


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_VALID_ABS )
  {
    data[pos] = (uint8) WMS_BD_VALID_ABS; /* parm id */
    pos++;

    data[pos] = 6;
    pos++; /* skip parm len */

    data[pos++] = cl_bd_ptr->validity_absolute.year;
    data[pos++] = cl_bd_ptr->validity_absolute.month;
    data[pos++] = cl_bd_ptr->validity_absolute.day;
    data[pos++] = cl_bd_ptr->validity_absolute.hour;
    data[pos++] = cl_bd_ptr->validity_absolute.minute;
    data[pos++] = cl_bd_ptr->validity_absolute.second;

    /* pos: parm data already skipped */
  }


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_VALID_REL )
  {
    data[pos] = (uint8) WMS_BD_VALID_REL; /* parm id */
    pos++;

    data[pos] = parm_len = 1;
    pos++; /* skip parm len */

    data[pos] = wms_ts_encode_relative_time( & cl_bd_ptr->validity_relative );

    pos += parm_len; /* skip parm data */
  }


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_DEFER_ABS )
  {
    data[pos] = (uint8) WMS_BD_DEFER_ABS; /* parm id */
    pos++;

    data[pos] = 6;
    pos++; /* skip parm len */

    data[pos++] = cl_bd_ptr->deferred_absolute.year;
    data[pos++] = cl_bd_ptr->deferred_absolute.month;
    data[pos++] = cl_bd_ptr->deferred_absolute.day;
    data[pos++] = cl_bd_ptr->deferred_absolute.hour;
    data[pos++] = cl_bd_ptr->deferred_absolute.minute;
    data[pos++] = cl_bd_ptr->deferred_absolute.second;

    /* pos: parm data already skipped */

  }


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_DEFER_REL )
  {
    data[pos] = (uint8) WMS_BD_DEFER_REL; /* parm id */
    pos++;

    data[pos] = parm_len = 1;
    pos++; /* skip parm len */

    data[pos] = wms_ts_encode_relative_time( & cl_bd_ptr->deferred_relative );

    pos += parm_len; /* skip parm data */
  }

  /* if is91ep_type is _VOICE_MAIL then the priority has been packed
  ** in the user data
  */
  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_PRIORITY  &&
      !(cl_bd_ptr->user_data.encoding == WMS_ENCODING_IS91EP &&
       cl_bd_ptr->user_data.is91ep_type == WMS_IS91EP_VOICE_MAIL) )
  {
    data[pos] = (uint8) WMS_BD_PRIORITY; /* parm id */
    pos++;

    data[pos] = parm_len = 1;
    pos++; /* skip parm len */

    b_packb( (uint8)(cl_bd_ptr->priority), data, (uint16)(pos * 8), (uint16)2 );
    b_packb( 0, data, (uint16)(pos * 8 + 2), 8 ); /* padding */

    pos += parm_len; /* skip parm data */
  }


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_PRIVACY )
  {
    data[pos] = (uint8) WMS_BD_PRIVACY; /* parm id */
    pos++;

    data[pos] = parm_len = 1;
    pos++; /* skip parm len */

    b_packb( (uint8) cl_bd_ptr->privacy, data, (uint16)(pos * 8), 2 );
    b_packb( 0, data, (uint16)(pos * 8 + 2), 8 ); /* padding */

    pos += parm_len; /* skip parm data */
  }


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_REPLY_OPTION )
  {
    data[pos] = (uint8) WMS_BD_REPLY_OPTION; /* parm id */
    pos++;

    data[pos] = parm_len = 1;
    pos++; /* skip parm len */

    bit_pos = (uint16)(pos * 8);

    data[pos] = 0; /* reset all 8 bits to 0 */

    b_packb( cl_bd_ptr->reply_option.user_ack_requested, data, bit_pos, 1 );
    bit_pos ++;

    b_packb( cl_bd_ptr->reply_option.delivery_ack_requested, data, bit_pos, 1 );
    bit_pos ++;

    b_packb( cl_bd_ptr->reply_option.read_ack_requested, data, bit_pos, 1 );
    bit_pos ++;

    pos += parm_len; /* skip parm data */
  }

  /* if is91ep_type is _VOICE_MAIL then the num of msgs has been packed
  ** in the user data
  */
  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_NUM_OF_MSGS &&
      !(cl_bd_ptr->user_data.encoding == WMS_ENCODING_IS91EP &&
       cl_bd_ptr->user_data.is91ep_type == WMS_IS91EP_VOICE_MAIL) )
  {
    data[pos] = (uint8) WMS_BD_NUM_OF_MSGS; /* parm id */
    pos++;

    data[pos] = parm_len = 1;
    pos++; /* skip parm len */

    /* two BCD digits ( e.g. 93 -> 1001 0011 ) */
    b_packb( cl_bd_ptr->num_messages / 10, data+pos, 0, 4 );
    b_packb( cl_bd_ptr->num_messages % 10, data+pos, 4, 4 );


    if( (cl_bd_ptr->num_messages / 10) > 9)
    {
        LOGE("Param not in range in wms_ts_encode_CDMA_bd (num_of_msgs)");
        st = WMS_INVALID_PARM_VALUE_S;
    }

    pos += parm_len; /* skip parm data */
  }


  if( st == WMS_OK_S &&
      cl_bd_ptr->mask & WMS_MASK_BD_ALERT )
  {
    data[pos] = (uint8) WMS_BD_ALERT; /* parm id */
    pos++;

    if ( (cl_bd_ptr->alert_mode == WMS_ALERT_MODE_OFF)
         ||(cl_bd_ptr->alert_mode == WMS_ALERT_MODE_DEFAULT) )
    {
      data[pos] = parm_len = 0;
      pos++; /* skip parm len */
    }
    else
    {
      data[pos] = parm_len = 1;
      pos++; /* skip parm len */

      data[pos] = 0x00;
      /* pack the 2-bit alert mode */
      b_packb( (uint8) cl_bd_ptr->alert_mode, data, (uint16)(pos*8), 2 );
    }

    pos += parm_len;
  }


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_LANGUAGE )
  {
    data[pos] =  (uint8) WMS_BD_LANGUAGE; /* parm id */
    pos++;

    data[pos] = parm_len = 1;
    pos++; /* skip parm len */

    data[pos] = (uint8) (cl_bd_ptr->language);

    pos += parm_len;
  }


  /* if is91ep_type is _CLI_ORDER then the callback number has been packed
  ** in the user data
  */
  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_CALLBACK &&
      !(cl_bd_ptr->user_data.encoding == WMS_ENCODING_IS91EP &&
       cl_bd_ptr->user_data.is91ep_type == WMS_IS91EP_CLI_ORDER) )
  {
    data[pos] =  (uint8) WMS_BD_CALLBACK; /* parm id */
    pos++;

    parm_len_ptr = data + pos; /* remember where is the len */
    pos++; /* skip parm len */

    bit_pos = (uint16) (pos * 8);

    b_packb( (uint8) cl_bd_ptr->callback.digit_mode,
             data,
             bit_pos,
             1 );
    bit_pos ++;

    if( cl_bd_ptr->callback.digit_mode == WMS_DIGIT_MODE_8_BIT )
    {
      digit_size = 8;

      b_packb( (uint8) cl_bd_ptr->callback.number_type,
               data,
               bit_pos,
               3 );
      bit_pos += 3;

      b_packb( (uint8) cl_bd_ptr->callback.number_plan,
               data,
               bit_pos,
               4 );
      bit_pos += 4;
    }
    else
    {
      digit_size = 4;
    }

    b_packb( cl_bd_ptr->callback.number_of_digits,
             data,
             bit_pos,
             8 );
    bit_pos += 8;

    for( i= 0; i < cl_bd_ptr->callback.number_of_digits; i ++ )
    {
      b_packb( cl_bd_ptr->callback.digits[i],
               data,
               bit_pos,
               digit_size );
      bit_pos += digit_size;
    }

    if( bit_pos % 8 != 0 )
    {
      /* pad 0 to remaining bits
      */
      digit_size = 8 - bit_pos % 8;
      b_packb( 0, data, bit_pos,  digit_size );
      bit_pos += digit_size;
    }

    /* pos points to beginning parm data. */
    * parm_len_ptr = (uint8) (bit_pos / 8 - pos);

    pos += * parm_len_ptr; /* skip parm data */

  } /* end of Callback Number */


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_DISPLAY_MODE )
  {
    data[pos] =  (uint8) WMS_BD_DISPLAY_MODE; /* parm id */
    pos++;

    data[pos] = parm_len = 1;
    pos++; /* skip parm len */

    data[pos] = 0x00;
    /* pack the 2-bit display mode */
    b_packb( (uint8) cl_bd_ptr->display_mode, data, (uint16) (pos*8), 2 );

    if( cl_bd_ptr->display_mode == WMS_DISPLAY_MODE_RESERVED )
    {
      /* pack the remaining 6 bits */
      b_packb( (uint8) cl_bd_ptr->download_mode, data, pos*8+2, 6 );
    }

    pos += parm_len;
  }


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_SCPT_DATA )
  {
    data[pos] = (uint8) WMS_BD_SCPT_DATA; /* parm id */
    pos++;

    parm_len_ptr = data + pos;
    pos++; /* skip parm len */

    bit_pos = (uint8)pos * 8;

    /* Encoding is 5 bits. After this is one or more of the service
    ** specific information
    */
    b_packb( (uint8)cl_bd_ptr->scpt_data_ptr->encoding, data, bit_pos, 5 );
    bit_pos += 5;

    for( i = 0; i < cl_bd_ptr->scpt_data_ptr->num_entries; i ++ )
    {
      /* Operation code
      */
      b_packb( (uint8)cl_bd_ptr->scpt_data_ptr->entry[i].op_code, data, bit_pos, 4 );
      bit_pos += 4;

      /* Service category
      */
      b_packb( (uint8)cl_bd_ptr->scpt_data_ptr->entry[i].srv_id.service, data, bit_pos, 16);
      bit_pos += 16;

      /* Language
      */
      b_packb( (uint8) cl_bd_ptr->scpt_data_ptr->entry[i].srv_id.language, data, bit_pos, 8);
      bit_pos += 8;

      /* Max messages
      */
      b_packb( cl_bd_ptr->scpt_data_ptr->entry[i].max_messages, data, bit_pos, 8 );
      bit_pos += 8;

      /* Alert options
      */
      b_packb( (uint8) cl_bd_ptr->scpt_data_ptr->entry[i].bc_alert, data, bit_pos, 4 );
      bit_pos += 4;

      /* Label length
      */
      b_packb( cl_bd_ptr->scpt_data_ptr->entry[i].label_len, data, bit_pos, 8 );
      bit_pos += 8;

      /* NOTE: SHIFT-JIS or Korean are not supported here, since
      ** they have variable digit sizes.
      */
      digit_size =
       wms_ts_cdma_encoding_digit_size(cl_bd_ptr->scpt_data_ptr->encoding);

      if( digit_size == 16 )
      {
        for( j=0, k=0; j < cl_bd_ptr->scpt_data_ptr->entry[i].label_len; j++ )
        {
          b_packb( cl_bd_ptr->scpt_data_ptr->entry[i].label[k], data, bit_pos, 8 );
          bit_pos += 8;
          k ++;

          b_packb( cl_bd_ptr->scpt_data_ptr->entry[i].label[k], data, bit_pos, 8 );
          bit_pos += 8;
          k ++;
        } /* for */
      }
      else
      {
        for( j=0; j < cl_bd_ptr->scpt_data_ptr->entry[i].label_len; j++ )
        {
          b_packb( cl_bd_ptr->scpt_data_ptr->entry[i].label[j],
                   data,
                   bit_pos,
                   digit_size );
          bit_pos += digit_size;
        } /* for */
      }

    } /* for( i = 0; i < cl_bd_ptr->scpt_data_ptr->num_entries; i ++ ) */


    if( bit_pos % 8 != 0 )
    {
      /* pad 0 to remaining bits
      */
      digit_size = 8 - bit_pos % 8;
      b_packb( 0, data, bit_pos,  digit_size );
      bit_pos += digit_size;
    }

    /* pos is pointing to the beginning of the parm data. */
    /* Now fill in the parm len field */
    * parm_len_ptr = (uint8)(bit_pos / 8 - pos);

    pos += * parm_len_ptr; /* skip parm data */

  } /* end of SCPT data */


  if( st == WMS_OK_S && ( cl_bd_ptr->mask & WMS_MASK_BD_SCPT_RESULT ) )
  {
    uint16   old_pos;

    data[pos] = (uint8) WMS_BD_SCPT_RESULT; /* parm id */
    pos++;

    parm_len_ptr = data + pos;
    pos++; /* skip parm len */

    old_pos = pos; /* remember the start of the parm data */

    for( i = 0; i < cl_bd_ptr->scpt_result_ptr->num_entries; i ++ )
    {
      data[pos] = (uint8)((uint32)cl_bd_ptr->scpt_result_ptr->entry[i].srv_id.service >> 8);
      pos ++;

      data[pos] = (uint8) ((uint32)cl_bd_ptr->scpt_result_ptr->entry[i].srv_id.service & 0xFF);
      pos ++;

      data[pos] = (uint8)(cl_bd_ptr->scpt_result_ptr->entry[i].srv_id.language);
      pos ++;

      data[pos] = (uint8) ((uint32)cl_bd_ptr->scpt_result_ptr->entry[i].status << 4);
      pos ++;
    }

    /* Now fill in the parm len field */
    * parm_len_ptr = (uint8)(pos - old_pos);

  } /* end of SCPT result */


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_DEPOSIT_INDEX )
  {
    data[pos] = (uint8) WMS_BD_DEPOSIT_INDEX; /* parm id */
    pos++;

    data[pos] = parm_len = 2;
    pos++; /* skip parm len */

    /* Pack 16 bits of data for the deposit index field */
    b_packw( (uint16)cl_bd_ptr->deposit_index, data, (uint16) (pos * 8),  parm_len * 8 );

    pos += parm_len; /* skip parm data */
  }


  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_DELIVERY_STATUS )
  {
    data[pos] = (uint8) WMS_BD_DELIVERY_STATUS; /* parm id */
    pos++;

    data[pos] = parm_len = 1;
    pos++; /* skip parm len */

    /* Pack 2 bits of data for the error class field */
    b_packb( (uint8) cl_bd_ptr->delivery_status.error_class,
             data,
             (uint16) (pos * 8),
             2 );

    /* Pack 6 bits of data for the message status field */
    b_packb( (uint8) cl_bd_ptr->delivery_status.status,
             data,
             (uint16) (pos * 8 + 2),
             6 );

    pos += parm_len; /* skip parm data */
  }

  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_IP_ADDRESS )
  {
    data[pos] = (uint8) WMS_BD_IP_ADDRESS; /* parm id */
    pos++;

    data[pos] = parm_len = 5;
    pos++; /* skip parm len */

    {
      uint16 old_pos = pos;

      for(i=0; i < WMS_IP_ADDRESS_SIZE; i++)
      {
        b_packb( (uint8) cl_bd_ptr->ip_address.address[i],
                 data,
                 old_pos*8,
                 8 );
        old_pos++;
      }

      b_packb( cl_bd_ptr->ip_address.is_valid,
               data,
               old_pos*8,
               8 );
    }
    pos += parm_len;

  } /* end of ip address */

  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_RSN_NO_NOTIFY )
  {
    data[pos] = (uint8) WMS_BD_RSN_NO_NOTIFY; /* parm id */
    pos++;


    data[pos] = parm_len = 1;
    pos++; /* skip parm len */

    /* Pack pp and ss */
    b_packb( (uint8) cl_bd_ptr->rsn_no_notify,
            data,
            pos*8,
            8 );

    pos += parm_len; /* skip parm data */

  } /* end of reason no notify */

  /* Encode generic other bearer data parameters */
  if( st == WMS_OK_S && cl_bd_ptr->mask & WMS_MASK_BD_OTHER )
  {
    if( NULL == cl_bd_ptr->other.other_data )
    {
      st = WMS_NULL_PTR_S;
      LOGE("other_data=NULL in wms_ts_encode_CDMA_bd!");
    }
    else
    {
      memcpy( data+pos,
              cl_bd_ptr->other.other_data,
              cl_bd_ptr->other.input_other_len );
      pos += cl_bd_ptr->other.input_other_len;
    }
  }


  raw_bd_ptr->len = pos;

  if( st == WMS_OK_S && pos > WMS_MAX_LEN )
  {
    // encoded data has too many bytes
    st = WMS_INVALID_PARM_SIZE_S;
  }

  /* done */
  return st;

} /* end of wms_ts_encode_CDMA_bd() */


/* <EJECT> */
/*=========================================================================
FUNCTION
  wms_ts_encode_bearer_data

DESCRIPTION
  This function encodes the SMS bearer data from the client
  structure to the raw format.

DEPENDENCIES
  None

RETURN VALUE
  Encoding result. WMS_OK_S means encoding succeeded; otherwise it failed.

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_encode_bearer_data
(
  const wms_client_bd_s_type                * cl_bd_ptr,       /* IN */
  wms_raw_ts_data_s_type                    * raw_bd_ptr       /* OUT */
)
{

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    /*---- checking ---- */
  if ( cl_bd_ptr == NULL || raw_bd_ptr == NULL)
  {
    LOGE("wms_ts_encode_bearer_data null ptr!");
    return WMS_NULL_PTR_S;
  }


  if( cl_bd_ptr->mask == 0 )
  {
    raw_bd_ptr->len = 0;
    return WMS_OK_S;
  }


  /* right now, only CDMA mode has M.O. requirements */
  return wms_ts_encode_CDMA_bd( cl_bd_ptr, raw_bd_ptr );
}

/* <EJECT> */
/*=========================================================================
FUNCTION
  wms_ts_decode_IS91EP_payload

DESCRIPTION
  This function decodes the IS91 EP data to the client's bearer data structure.
  (see IS-637 4.3.1.4.1)

DEPENDENCIES
  None

RETURN VALUE
  Status of the translation.

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_decode_IS91EP_payload
(
  const wms_IS91EP_type_e_type         is91ep_type,
  uint8                          number_of_digits,
  const uint8                    is91ep_digits[],
  wms_client_bd_s_type           * cl_bd_ptr     /* OUT */
)
{

  wms_status_e_type     st = WMS_OK_S;
  uint8                  i;
  uint8                  vmn_cnt = 2;
  uint8                 d1, d2;
  boolean               urgent; /*  voice mail priority */


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  if (cl_bd_ptr == NULL || is91ep_digits == NULL)
  {
    LOGE("wms_ts_decode_IS91EP_payload null ptr!");
    return WMS_NULL_PTR_S;
  }
  else if (number_of_digits > WMS_ADDRESS_MAX)
  {
    LOGE("wms_ts_decode_IS91EP_payload, number of digits exceeded max value: %d", WMS_ADDRESS_MAX);
    return WMS_INVALID_PARM_SIZE_S;
  }
  else
  {
    cl_bd_ptr->mask = 0;

    /* Alert on delivery is implictly set */
    cl_bd_ptr->mask |= WMS_MASK_BD_ALERT;
    cl_bd_ptr->alert_mode = WMS_ALERT_MODE_DEFAULT;

    switch( is91ep_type )
    {
      case WMS_IS91EP_CLI_ORDER:

        cl_bd_ptr->mask |= WMS_MASK_BD_CALLBACK;
        cl_bd_ptr->callback.digit_mode = WMS_DIGIT_MODE_4_BIT;
        cl_bd_ptr->callback.number_of_digits = number_of_digits;
 
        for( i = 0; i < number_of_digits && i < WMS_ADDRESS_MAX; i++ )
        {
          cl_bd_ptr->callback.digits[i] = is91ep_digits[i];
        }

        break;


      case WMS_IS91EP_VOICE_MAIL:

        cl_bd_ptr->mask |= WMS_MASK_BD_NUM_OF_MSGS;
  
        /* extract low 4 bits of the first two digits and then
           convert them to a number
        */
        d1 = is91ep_digits[0] & 0x0f;
        d2 = is91ep_digits[1] & 0x0f;
        cl_bd_ptr->num_messages = (uint8) (d1 * 10 + d2);

        // continue decoding if there are more digits left
        if( number_of_digits > 2 )
        {
          /* The 3rd digit may be the priority
          */
          urgent = is91ep_digits[2];
          if( urgent == 1)
          {
            cl_bd_ptr->mask |= WMS_MASK_BD_PRIORITY;
            cl_bd_ptr->priority = WMS_PRIORITY_URGENT;
            vmn_cnt = 3;
          }  
          else if( urgent == 0 )
          {
            cl_bd_ptr->mask |= WMS_MASK_BD_PRIORITY;
            cl_bd_ptr->priority = WMS_PRIORITY_NORMAL;
            vmn_cnt = 3;
          }  
          else
          {
            /* the 3rd bit is not a priority */
            vmn_cnt = 2;
          }  
        } // if number_of_digits>2
  

        if( number_of_digits > vmn_cnt )
        {
          /* extract user data text
          */
          cl_bd_ptr->mask |= WMS_MASK_BD_USER_DATA;
          cl_bd_ptr->user_data.encoding    = WMS_ENCODING_IS91EP;
          cl_bd_ptr->user_data.is91ep_type = WMS_IS91EP_VOICE_MAIL;
          cl_bd_ptr->user_data.number_of_digits = number_of_digits - vmn_cnt;
  
          /* convert remaining digits to ASCII characters
          */
          for( i=vmn_cnt; i < number_of_digits; i++ )
          {
            if( i >= WMS_CDMA_USER_DATA_MAX || number_of_digits > WMS_CDMA_USER_DATA_MAX)
            {
              LOGI( "decoding: BD user data too big. len=%d", i);
              st = WMS_INVALID_USER_DATA_SIZE_S;
              break; /* out of for loop */
            }
            /* (see IS-91 Appendix B) */
            cl_bd_ptr->user_data.data[ i - vmn_cnt ] = is91ep_digits[i] + 0x20;
          }
  
          cl_bd_ptr->user_data.data_len     = number_of_digits - vmn_cnt;
  
        }

        break;


      case WMS_IS91EP_SHORT_MESSAGE:
      case WMS_IS91EP_SHORT_MESSAGE_FULL:

        cl_bd_ptr->mask |= WMS_MASK_BD_USER_DATA;

        cl_bd_ptr->user_data.encoding    = WMS_ENCODING_IS91EP;
        cl_bd_ptr->user_data.is91ep_type = WMS_IS91EP_SHORT_MESSAGE;
        cl_bd_ptr->user_data.number_of_digits = number_of_digits;

        /* convert the digits to ASCII characters
        */
        for( i = 0; i < number_of_digits; i++ )
        {
          if( i >= WMS_CDMA_USER_DATA_MAX )
          {
             LOGI( "decoding: BD user data too big. len=%d", i);
             st = WMS_INVALID_USER_DATA_SIZE_S;
             break; /* out of for loop */
          }
          /* (see IS-91 Appendix B) */
          cl_bd_ptr->user_data.data[i] = is91ep_digits[i] + 0x20;
        }

        cl_bd_ptr->user_data.data_len     = number_of_digits;
  
        break;


      default:
 
        /* Should not happen !! */
        LOGI( "wms_ts_decode_IS91EP_payload: invalid IS91EP type: %d",
                  is91ep_type);
        st = WMS_GENERAL_ERROR_S;
 
        break;

    } /* switch */
  }

  /* done */
  return st;

} /* end of wms_ts_decode_IS91EP_payload() */


/* <EJECT> */
/*=========================================================================
FUNCTION
  wms_ts_decode_CDMA_bd

DESCRIPTION
  This function decodes the raw CDMA bearer data into the client's structure.

DEPENDENCIES
  None

RETURN VALUE
  Status of the translation.

SIDE EFFECTS
  None

COMMENTS
  If is_unpacked_user_data = TRUE, then there should be no UDH.

=========================================================================*/
wms_status_e_type  wms_ts_decode_CDMA_bd
(
  const wms_raw_ts_data_s_type       * raw_bd_ptr,   /* IN */
  boolean                            is_unpacked_user_data, /* IN */
  boolean                            decode_other,     /* IN */
  wms_client_bd_s_type               * cl_bd_ptr       /* OUT */
)
{
  wms_status_e_type       st = WMS_OK_S;
  uint16                    pos;      /* running position in bytes */
  uint16                    bit_pos;  /* position in bits */
  wms_bd_sub_parm_id_e_type  parm_id = WMS_BD_DUMMY;
  uint8                    parm_len;
  uint8                    digit_size;
  uint8                    i;

  uint8                    bd_len;
  uint8                    * bd_data;
  uint8                    desired_other_len = 0;

  static wms_client_bd_s_type    is91ep_bd;  /* static for saving stack */


  int                     j, k;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*---- checking ---- */
  if ( raw_bd_ptr == NULL || cl_bd_ptr == NULL )
  {
    LOGE("wms_ts_decode_CDMA_bd null ptr!");
    return WMS_NULL_PTR_S;
  }

  bd_len = (uint8) MIN(raw_bd_ptr->len,WMS_MAX_LEN);
  bd_data = (uint8*) raw_bd_ptr->data;

  cl_bd_ptr->mask = 0;

  /* in case of null bearer data, return successful */
  if( raw_bd_ptr->len == 0 )
  {
    return WMS_OK_S;  /* SHORT-RETURN */
  }

  pos = 0;

  /*---- checking ----*/

  if( bd_len < 2 )   /* the 2 bytes are parm id + parm len */
  {
    LOGI("** BD Msg too short: %d", bd_len);
    return WMS_INVALID_PARM_SIZE_S;  /* SHORT-CIRCUIT RETURN */
  }

  /* bd_len is uint8, so it won't be greater than WMS_MAX_LEN */


  /*------ start decoding ------*/

  /* the remain data has one or more of the following:
     - PARAMETER_ID    8 bits
     - PARAMETER_LEN   8 bits
     - Parameter Data  8 x PARAMETER_LEN
  */

  while( st == WMS_OK_S  )
  {

    if( pos == bd_len )
    {
      /* Good. Done with parameter processing successfully */
      break; /* out of while loop */
    }
    else if( pos + 2 > bd_len )   // 1 byte for parm_id, 1 for parm_len
    {
      /* Current position goes beyond the bearer data size. */
      LOGI( "decoding: parameter (id=%d) extends beyond bd size %d",
                parm_id, bd_len);
      st = WMS_INVALID_PARM_SIZE_S;
      break; /* out of while loop */
    }

    parm_id = (wms_bd_sub_parm_id_e_type) bd_data[ pos ];
    pos ++;  /* skip parm id */

    parm_len = bd_data[ pos ];
    pos ++;  /* skip parm len */


    if( pos + parm_len > bd_len )
    {
      /* parm data passes the max length of bearer data
      */
      LOGI( "decoding: parameter (id=%d) extends beyond bd size %d",
                parm_id, bd_len);
      st = WMS_INVALID_PARM_SIZE_S;
      break; /* out of while loop */
    }


    /* Now pos should point to the parm data */
    /* After each case below, pos should point to the next parm Id */

    switch( parm_id )
    {

      case WMS_BD_MSG_ID:

        if( cl_bd_ptr->mask & WMS_MASK_BD_MSG_ID )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_MSG_ID;

        if ( pos + parm_len > bd_len || parm_len < 3 )
        {
          LOGE( "decoding: BD msg id len is not 3: %d. Reset to 3.",
                    parm_len);
          return WMS_INVALID_PARM_SIZE_S;
        }

        bit_pos = (uint16) (pos * 8); /* prepare for using bit_pos */

        cl_bd_ptr->message_id.type = (wms_bd_message_type_e_type)
                                     b_unpackb( bd_data, bit_pos, 4 );
        bit_pos += 4;

        cl_bd_ptr->message_id.id_number = b_unpackw( bd_data, bit_pos, 16 );
        bit_pos += 16;

        cl_bd_ptr->message_id.udh_present = b_unpackb( bd_data, bit_pos, 1 );
        bit_pos += 1;

        pos += parm_len; /* skip parm data */

        break;


      case WMS_BD_USER_DATA:

        if( cl_bd_ptr->mask & WMS_MASK_BD_USER_DATA )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_USER_DATA;

        /* Now start using bit positions instead of byte positions
        */
        bit_pos = (uint16) (pos * 8);

        cl_bd_ptr->user_data.encoding = (wms_user_data_encoding_e_type)
                                        b_unpackb( bd_data, bit_pos, 5 );
        bit_pos += 5;

        /* -------------------------------------------------------------------
          If the encoding is IS91EP, unpack the digits into user_data.data
          and set user_data.data_len to the number of digits. Then at the
          end of this function. these digits are further translated into
          corresponding BD fields (num_digits, callback, user_data). The final
          user_data is in the form of ASCII characters, each occupying a byte.

          OTHERWISE, copy the raw bits into user_data.data and set
          user_data.data_len to the number of bytes of raw data, but not
          the number of digits, since we don't know/care the digit sizes
          of the other and potentially new user data encoding types.
        ---------------------------------------------------------------------*/

        if( cl_bd_ptr->user_data.encoding == WMS_ENCODING_IS91EP )
        {
          cl_bd_ptr->user_data.is91ep_type = (wms_IS91EP_type_e_type)
                                             b_unpackb( bd_data, bit_pos, 8 );
          bit_pos += 8;

          if( cl_bd_ptr->user_data.is91ep_type == WMS_IS91EP_CLI_ORDER )
          {
            digit_size = 4;
          }
          else
          {
            digit_size = 6;
          }
        }
        else /* for all the other encoding tpyes, copy the raw bits */
        {
          digit_size = 8;
        }

        /* extract the number of digits field. this is only useful
           for IS91EP user data, since for other encoding types, we don't
           know the digit sizes.
        */
        cl_bd_ptr->user_data.number_of_digits = b_unpackb(bd_data,bit_pos,8 );
        bit_pos += 8;


        /* Extract each digits according to parm_len: */

        for( i=0;
             bit_pos < (pos+parm_len)*8;
             i++, bit_pos += digit_size )
        {
          if( i >= WMS_CDMA_USER_DATA_MAX )
          {
            LOGI( "decoding: BD user data too big. len=%d", parm_len);
            st = WMS_INVALID_USER_DATA_SIZE_S;
            break; /* out of for loop */
          }
          cl_bd_ptr->user_data.data[i] = b_unpackb( bd_data,
                                                    bit_pos,
                                                    digit_size );
        } /* for */


        if( st == WMS_OK_S )
        {
          if( cl_bd_ptr->user_data.encoding == WMS_ENCODING_IS91EP )
          {
            /* since we extracted each digit into a byte, hence the
               following two are the same to the client:
            */
            cl_bd_ptr->user_data.data_len =
                                    cl_bd_ptr->user_data.number_of_digits;

            /* 'i' might be greater than num_digits because of padding, but
               it cannot be smaller:
            */
            if( i < cl_bd_ptr->user_data.number_of_digits )
            {
              LOGI("decoding: UD sz error(%d<%d)",
                        i, cl_bd_ptr->user_data.number_of_digits);
              st = WMS_INVALID_USER_DATA_SIZE_S;
            }
          }
          else if( cl_bd_ptr->user_data.encoding == WMS_ENCODING_ASCII ||
                   cl_bd_ptr->user_data.encoding == WMS_ENCODING_IA5 )
          {
            /* Reply on the number of characters/digits to set the
            ** data_len correctly.
            */
            cl_bd_ptr->user_data.data_len = (uint8)
                        (( cl_bd_ptr->user_data.number_of_digits * 7 + 7 ) / 8);
            cl_bd_ptr->user_data.padding_bits =(uint8) 
              ((cl_bd_ptr->user_data.data_len * 8) -
              (cl_bd_ptr->user_data.number_of_digits * 7));
          }
          else if ( cl_bd_ptr->user_data.encoding == WMS_ENCODING_UNICODE )
          {
            cl_bd_ptr->user_data.data_len = (uint8) (cl_bd_ptr->user_data.number_of_digits * 2);
            cl_bd_ptr->user_data.padding_bits = 0;
          }
          else /* all the other encoding types */
          {
            /* in many cases, the len is not equal to num of digits */
            /* The last byte unpacked does not have useful data */
            cl_bd_ptr->user_data.data_len = i - 1; /* ignore the last byte */
            cl_bd_ptr->user_data.padding_bits = 0;
          }

        } /* if st==OK */

        pos += parm_len;  /* skip parm data */

        break;


      case WMS_BD_USER_RESP:

        if( cl_bd_ptr->mask & WMS_MASK_BD_USER_RESP )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_USER_RESP;

        if( pos + parm_len > bd_len || parm_len < 1)
        {
          /* bd data too short , or bad len field */
          st = WMS_INVALID_PARM_SIZE_S;
        }
        else
        {
          cl_bd_ptr->user_response = b_unpackb( bd_data,
                                              (uint16) (pos*8),
                                              parm_len*8 );
          pos += parm_len; /* skip parm data */
        }

        break;


      case WMS_BD_MC_TIME:

        if( cl_bd_ptr->mask & WMS_MASK_BD_MC_TIME )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_MC_TIME;

        cl_bd_ptr->mc_time.year   = bd_data[pos++];
        cl_bd_ptr->mc_time.month  = bd_data[pos++];
        cl_bd_ptr->mc_time.day    = bd_data[pos++];
        cl_bd_ptr->mc_time.hour   = bd_data[pos++];
        cl_bd_ptr->mc_time.minute = bd_data[pos++];
        cl_bd_ptr->mc_time.second = bd_data[pos++];
        break;


      case WMS_BD_VALID_ABS:

        if( cl_bd_ptr->mask & WMS_MASK_BD_VALID_ABS )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_VALID_ABS;

        cl_bd_ptr->validity_absolute.year   = bd_data[pos++];
        cl_bd_ptr->validity_absolute.month  = bd_data[pos++];
        cl_bd_ptr->validity_absolute.day    = bd_data[pos++];
        cl_bd_ptr->validity_absolute.hour   = bd_data[pos++];
        cl_bd_ptr->validity_absolute.minute = bd_data[pos++];
        cl_bd_ptr->validity_absolute.second = bd_data[pos++];
        break;


      case WMS_BD_VALID_REL:

        if( cl_bd_ptr->mask & WMS_MASK_BD_VALID_REL )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_VALID_REL;

        wms_ts_decode_relative_time( bd_data[pos++],
                                     & cl_bd_ptr->validity_relative );
        break;


      case WMS_BD_DEFER_ABS:

        if( cl_bd_ptr->mask & WMS_MASK_BD_DEFER_ABS )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_DEFER_ABS;

        cl_bd_ptr->deferred_absolute.year   = bd_data[pos++];
        cl_bd_ptr->deferred_absolute.month  = bd_data[pos++];
        cl_bd_ptr->deferred_absolute.day    = bd_data[pos++];
        cl_bd_ptr->deferred_absolute.hour   = bd_data[pos++];
        cl_bd_ptr->deferred_absolute.minute = bd_data[pos++];
        cl_bd_ptr->deferred_absolute.second = bd_data[pos++];
        break;


      case WMS_BD_DEFER_REL:

        if( cl_bd_ptr->mask & WMS_MASK_BD_DEFER_REL )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_DEFER_REL;

        wms_ts_decode_relative_time( bd_data[pos++],
                                     & cl_bd_ptr->deferred_relative );

        break;


      case WMS_BD_PRIORITY:

        if( cl_bd_ptr->mask & WMS_MASK_BD_PRIORITY )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        if( parm_len == 0)
        {
          /* bd data too short , or bad len field */
          return WMS_INVALID_PARM_SIZE_S;
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_PRIORITY;

        cl_bd_ptr->priority = (wms_priority_e_type)
                              b_unpackb( bd_data, (uint16) (pos * 8), 2 );

        pos += parm_len;

        break;


      case WMS_BD_PRIVACY:

        if( cl_bd_ptr->mask & WMS_MASK_BD_PRIVACY )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }
        cl_bd_ptr->mask |= WMS_MASK_BD_PRIVACY;

        cl_bd_ptr->privacy = (wms_privacy_e_type)
                              b_unpackb( bd_data, (uint16)(pos * 8), 2 );

        pos += parm_len;

        break;


      case WMS_BD_REPLY_OPTION:

        if( cl_bd_ptr->mask & WMS_MASK_BD_REPLY_OPTION )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }
        cl_bd_ptr->mask |= WMS_MASK_BD_REPLY_OPTION;

        cl_bd_ptr->reply_option.user_ack_requested =
                       b_unpackb( bd_data, (uint16)(pos * 8), 1 );
        cl_bd_ptr->reply_option.delivery_ack_requested =
                       b_unpackb( bd_data, (uint16)(pos * 8 + 1), 1 );
        cl_bd_ptr->reply_option.read_ack_requested =
                       b_unpackb( bd_data, (uint16)(pos * 8 + 2), 1 );

        pos += parm_len;

        break;


      case WMS_BD_NUM_OF_MSGS:

        if( cl_bd_ptr->mask & WMS_MASK_BD_NUM_OF_MSGS )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_NUM_OF_MSGS;

        if( pos + parm_len > bd_len || parm_len < 1 )
        {
          /* bd data too short , or bad len field */
          LOGE("Bearer data too short or bad len field in wms_ts_decode_CDMA_bd (num_of_msgs)");
          st = WMS_INVALID_PARM_SIZE_S;
        }
        else
        {
          /* two BCD digits ( e.g. 93 -> 1001 0011 ) */

          cl_bd_ptr->num_messages  = 0;

          /* extract the first BCD digit */
          i = b_unpackb( bd_data, (uint16)(pos*8), 4 );
          if( i < 10 )
          {
            cl_bd_ptr->num_messages  = (uint8)(i * 10);
          }
          else
          {
            LOGE("Param not in range in wms_ts_decode_CDMA_bd; first BCD digit in num_of_msgs");
            st = WMS_INVALID_PARM_VALUE_S;
            break;
          }

          /* extract the second BCD digit */
          i = b_unpackb( bd_data, (uint16)(pos*8 + 4), 4 );
          if( i < 10 )
          {
            cl_bd_ptr->num_messages  += i;
          }
          else
          {
            LOGE("Param not in range in wms_ts_decode_CDMA_bd; second BCD digit in num_of_msgs");
            st = WMS_INVALID_PARM_VALUE_S;
            break;
          }

          pos += parm_len; /* skip parm data */
        }

        break;


      case WMS_BD_ALERT:

        if( cl_bd_ptr->mask & WMS_MASK_BD_ALERT )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_ALERT;

        if( parm_len == 0 )
        {
          /* The network still uses 637, not 637A/B yet */
          cl_bd_ptr->alert_mode = WMS_ALERT_MODE_DEFAULT;
        }
        else
        {
          cl_bd_ptr->alert_mode = (wms_alert_mode_e_type)b_unpackb( bd_data, (uint16)(pos*8), 2 );
        }

        pos += parm_len;   /* skip parm data */

        break;


      case WMS_BD_LANGUAGE:

        if( cl_bd_ptr->mask & WMS_MASK_BD_LANGUAGE )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_LANGUAGE;

        if( pos + parm_len > bd_len || parm_len < 1 )
        {
          /* bd data too short */
          LOGE("Bearer data too short in wms_ts_decode_CDMA_bd (language)");
          st = WMS_INVALID_PARM_SIZE_S;
        }
        else
        {
          cl_bd_ptr->language = (wms_language_e_type)
                              b_unpackb( bd_data, (uint16)(pos*8), parm_len*8 );
          pos += parm_len; /* skip parm data */
        }

        break;


      case WMS_BD_CALLBACK:

        if( cl_bd_ptr->mask & WMS_MASK_BD_CALLBACK )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_CALLBACK;

        /* Now start using bit positions instead of byte positions
        */
        bit_pos = (uint16) (pos * 8);

        cl_bd_ptr->callback.digit_mode = (wms_digit_mode_e_type)
                                         b_unpackb( bd_data, bit_pos, 1 );
        bit_pos ++;

        if( cl_bd_ptr->callback.digit_mode == WMS_DIGIT_MODE_8_BIT )
        {
          cl_bd_ptr->callback.number_type = (wms_number_type_e_type)
                                            b_unpackb(bd_data,bit_pos,3);
          bit_pos += 3;

          cl_bd_ptr->callback.number_plan = (wms_number_plan_e_type)
                                            b_unpackb(bd_data, bit_pos, 4);
          bit_pos += 4;

          digit_size = 8;
        }
        else
        {
          cl_bd_ptr->callback.number_type = WMS_NUMBER_UNKNOWN;
          cl_bd_ptr->callback.number_plan = WMS_NUMBER_PLAN_UNKNOWN;
          digit_size = 4;
        }

        cl_bd_ptr->callback.number_of_digits = b_unpackb( bd_data, bit_pos, 8 );
        bit_pos += 8;


        /* extract all digits. if too long, truncate it.
        */
        if( cl_bd_ptr->callback.number_of_digits > WMS_ADDRESS_MAX )
        {
          LOGI( "decoding: callback number too big: %d",
                   cl_bd_ptr->callback.number_of_digits);
          cl_bd_ptr->callback.number_of_digits = WMS_ADDRESS_MAX;
        }

        for( i=0; i < cl_bd_ptr->callback.number_of_digits; i++ )
        {
          cl_bd_ptr->callback.digits[i] = b_unpackb( bd_data,
                                                     bit_pos,
                                                     digit_size );
          bit_pos += digit_size;
        }

        pos += parm_len;  /* skip parm data */

        break;


      case WMS_BD_DISPLAY_MODE:

        if( cl_bd_ptr->mask & WMS_MASK_BD_DISPLAY_MODE )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_DISPLAY_MODE;

        if( pos + parm_len > bd_len || parm_len < 1 )
        {
          /* bd data too short , or bad len field */
          LOGE("Bearer data too short or bad len field in wms_ts_decode_CDMA_bd (display_mode)");
          st = WMS_INVALID_PARM_SIZE_S;
        }
        else
        {
          cl_bd_ptr->display_mode = (wms_display_mode_e_type)
                                b_unpackb( bd_data, (uint16)(pos*8), 2 );
          if( cl_bd_ptr->display_mode == WMS_DISPLAY_MODE_RESERVED )
          {
            cl_bd_ptr->download_mode = (wms_download_mode_e_type)
                                b_unpackb( bd_data, pos*8+2, 6 );
          }
          else
          {
            cl_bd_ptr->download_mode = WMS_DOWNLOAD_MODE_NONE;
          }
          pos += parm_len; /* skip parm data */
        }

        break;


      case WMS_BD_SCPT_DATA:
        if( cl_bd_ptr->mask & WMS_MASK_BD_SCPT_DATA )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_SCPT_DATA;

        /* Now start using bit positions instead of byte positions
        */
        bit_pos = (uint16) (pos * 8);

        /* Encoding is 5 bits. After this is one or more of the service
        ** specific information
        */

        if (cl_bd_ptr->scpt_data_ptr == NULL)
        {
          LOGE("null pointer in wms_ts_decode_CDMA_bd (scpt_data)");
          return WMS_NULL_PTR_S;
        }
        else
        {
          cl_bd_ptr->scpt_data_ptr->encoding = (wms_user_data_encoding_e_type)
                                           b_unpackb( bd_data, bit_pos, 5 );
          bit_pos += 5;

          for( i = 0; i < WMS_BC_SCPT_MAX_SERVICES; i ++ )
          {
            /* Operation code
            */
            cl_bd_ptr->scpt_data_ptr->entry[i].op_code =
                  (wms_bc_scpt_op_code_e_type) b_unpackb( bd_data, bit_pos, 4 );
            bit_pos += 4;

            /* Service category
            */
            cl_bd_ptr->scpt_data_ptr->entry[i].srv_id.service =
                    (wms_service_e_type) b_unpackw( bd_data, bit_pos, 16 );
            bit_pos += 16;

            /* Language
            */
            cl_bd_ptr->scpt_data_ptr->entry[i].srv_id.language =
                    (wms_language_e_type) b_unpackb( bd_data, bit_pos, 8 );
            bit_pos += 8;

            /* Max messages
            */
            cl_bd_ptr->scpt_data_ptr->entry[i].max_messages =
                                            b_unpackb( bd_data, bit_pos, 8 );
            bit_pos += 8;

            /* Alert options
            */
            cl_bd_ptr->scpt_data_ptr->entry[i].bc_alert = (wms_bc_alert_e_type)
                                              b_unpackb( bd_data, bit_pos, 4 );
            bit_pos += 4;

            /* Label length
            */
            cl_bd_ptr->scpt_data_ptr->entry[i].label_len =
                                               b_unpackb( bd_data, bit_pos, 8 );
            bit_pos += 8;

            /* NOTE: SHIFT-JIS or Korean are not supported here, since
            ** they have variable digit sizes.
            */
            digit_size =
            wms_ts_cdma_encoding_digit_size(cl_bd_ptr->scpt_data_ptr->encoding);

            /* Clear the label before assigning its new value */
            memset(cl_bd_ptr->scpt_data_ptr->entry[i].label, 0,
                   WMS_BC_MM_SRV_LABEL_SIZE);


            if( digit_size == 16 )
            {
              for( j=0, k=0; j < cl_bd_ptr->scpt_data_ptr->entry[i].label_len; j++ )
              {
                /* skip the digits if max label size has been reached */
                if( k < WMS_BC_MM_SRV_LABEL_SIZE )
                {
                  cl_bd_ptr->scpt_data_ptr->entry[i].label[k]
                                 = b_unpackb( bd_data, bit_pos, 8 );
                  k ++;
                }
                bit_pos += 8;

                if( k < WMS_BC_MM_SRV_LABEL_SIZE )
                {
                  cl_bd_ptr->scpt_data_ptr->entry[i].label[k]
                                 = b_unpackb( bd_data, bit_pos, 8 );
                  k ++;
                }
                bit_pos += 8;
              }
            }
            else
            {
              /* Handle 8-bit or 7-bit digit sizes
              */
              for( j=0; j < cl_bd_ptr->scpt_data_ptr->entry[i].label_len; j++ )
              {
                /* skip the digits if max label size has been reached */
                if( j < WMS_BC_MM_SRV_LABEL_SIZE )
                {
                  cl_bd_ptr->scpt_data_ptr->entry[i].label[j]
                       = b_unpackb( bd_data, bit_pos, digit_size );
                }
                bit_pos += digit_size;
              } /* for */

            } /* if( digit_size == 16 ) */

            /* Check if it is the end of this parameter:
            ** (max padding bits is 7)
            */
            if( bit_pos+7 >= (pos+parm_len) * 8 )
            {
              i ++; /* 'i' will be used as the number of entries */
              break;
            }

          } /* for( i = 0; i < WMS_SCPT_MAX_SERVICES; i ++ ) */

          cl_bd_ptr->scpt_data_ptr->num_entries = i;

          pos += parm_len;  /* skip parm data */
        }

        break;

      case WMS_BD_SCPT_RESULT:
        if( cl_bd_ptr->mask & WMS_MASK_BD_SCPT_RESULT )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_SCPT_RESULT;

        /* Now start using bit positions instead of byte positions
        */
        bit_pos = (uint16) (pos * 8);

        if (cl_bd_ptr->scpt_result_ptr == NULL)
        {
          LOGE("null pointer in wms_ts_decode_CDMA_bd (scpt_result)");
          return WMS_NULL_PTR_S;
        }
        else
        {
          for( i = 0; i < WMS_BC_SCPT_MAX_SERVICES; i ++ )
          {
            cl_bd_ptr->scpt_result_ptr->entry[i].srv_id.service =
                   (wms_service_e_type ) b_unpackw( bd_data, bit_pos, 16 );
            bit_pos += 16;

            cl_bd_ptr->scpt_result_ptr->entry[i].srv_id.language =
                   (wms_language_e_type ) b_unpackb( bd_data, bit_pos, 8 );
            bit_pos += 8;

            cl_bd_ptr->scpt_result_ptr->entry[i].status = (wms_bc_scpt_status_e_type )
                                    b_unpackb( bd_data, bit_pos, 4 );
            bit_pos += 4;

            bit_pos += 4; /* skip reserved bits */

            /* Check if it is the end of this parameter:
            */
            if( bit_pos >= (pos+parm_len) * 8 )
            {
              i ++; /* 'i' will be used as the number of entries */
              break;
            }

          } /* for( i = 0; i < WMS_BC_SCPT_MAX_SERVICES; i ++ ) */

          cl_bd_ptr->scpt_result_ptr->num_entries = i;

          pos += parm_len;  /* skip parm data */
        }

        break;

      case WMS_BD_DEPOSIT_INDEX:
        if( cl_bd_ptr->mask & WMS_MASK_BD_DEPOSIT_INDEX )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_DEPOSIT_INDEX;

        /* Unpack message deposit index field, which has 16 bits */
        cl_bd_ptr->deposit_index = b_unpackw( bd_data, (uint16)(pos * 8), 16 );

        pos += parm_len;  /* skip parm data */
        break;

      case WMS_BD_DELIVERY_STATUS:
        if( cl_bd_ptr->mask & WMS_MASK_BD_DELIVERY_STATUS )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_DELIVERY_STATUS;

        /* Unpack error class field, which has 2 bits */
        cl_bd_ptr->delivery_status.error_class = (wms_error_class_e_type)
                                     b_unpackb( bd_data, (uint16)(pos * 8), 2 );

        /* Unpack message status field, which has 6 bits */
        cl_bd_ptr->delivery_status.status = (wms_delivery_status_e_type) b_unpackb( bd_data, (uint16)(pos*8 + 2), 6 );

        pos += parm_len;  /* skip parm data */

        break;

      case WMS_BD_IP_ADDRESS:
        if( cl_bd_ptr->mask & WMS_MASK_BD_IP_ADDRESS )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        {
          uint16 old_pos = pos;

          cl_bd_ptr->mask |= WMS_MASK_BD_IP_ADDRESS;

          for( i = 0; i < WMS_IP_ADDRESS_SIZE; i++ )
          {
            cl_bd_ptr->ip_address.address[i] =
                  (uint8) b_unpackb( bd_data, old_pos*8, 8 );
            old_pos++;
          }

          cl_bd_ptr->ip_address.is_valid =
                  (boolean) b_unpackb( bd_data, old_pos*8, 8 );
        }
        pos += parm_len;  /* skip parm data */

        break;

      case WMS_BD_RSN_NO_NOTIFY:
        if( cl_bd_ptr->mask & WMS_MASK_BD_RSN_NO_NOTIFY )
        {
          LOGI("Duplicate parm: %d", parm_id);
          pos += parm_len;    /* skip parm data */
          break;              /* continue with next parm */
        }

        cl_bd_ptr->mask |= WMS_MASK_BD_RSN_NO_NOTIFY;

        /* Unpack pp and ss*/
        cl_bd_ptr->rsn_no_notify = b_unpackb( bd_data, pos*8, 8 );


        pos += parm_len; /* skip parm data */

        break;


      default:
        LOGI( "Decoding BD: unrecognized parm_id: %d",  parm_id);
        LOGI( "Decoding BD: unrecognized parm_len: %d", parm_len);
        LOGI( "Decoding BD: new desired_other_len=%d", 
                  desired_other_len + parm_len + 2);
        cl_bd_ptr->mask |= WMS_MASK_BD_OTHER;

        if( TRUE == decode_other )
        {
          LOGI("Decoding other parameters");
          if( cl_bd_ptr->other.input_other_len < desired_other_len+parm_len+2 )
          {
            LOGE( "input_other_len %d < desired_other_len!", cl_bd_ptr->other.input_other_len);
          }
          else if( NULL == cl_bd_ptr->other.other_data )
          {
            LOGE( "other_data==NULL!");
          }
          else
          {
            cl_bd_ptr->other.other_data[desired_other_len]   = parm_id;
            cl_bd_ptr->other.other_data[desired_other_len+1] = parm_len;
            memcpy( cl_bd_ptr->other.other_data + desired_other_len + 2,
                    bd_data+pos,
                    parm_len );
          }
        } /* if TRUE == decode_other */

        desired_other_len += parm_len + 2;

        pos += parm_len;    /* skip parm data */
        break;              /* continue with next parm */

    } /* switch */

  } /* while loop */

  cl_bd_ptr->other.desired_other_len = desired_other_len;


  /**** decode embedded IS91EP message *****/

  if( st == WMS_OK_S &&
      cl_bd_ptr->mask & WMS_MASK_BD_USER_DATA &&
      cl_bd_ptr->user_data.encoding == WMS_ENCODING_IS91EP )
  {
    /* To avoid memory conflict, copy the bearer data to a different place.
    */
    is91ep_bd = * cl_bd_ptr;

    st = wms_ts_decode_IS91EP_payload
            (
              is91ep_bd.user_data.is91ep_type,
              is91ep_bd.user_data.number_of_digits,
              (uint8*) is91ep_bd.user_data.data,
              cl_bd_ptr   /* OUT */
            );

    if( st == WMS_OK_S )
    {
      /* Restore the previously decoded msg id field:
      */
      if( is91ep_bd.mask & WMS_MASK_BD_MSG_ID )
      {
        cl_bd_ptr->mask |= WMS_MASK_BD_MSG_ID;
        cl_bd_ptr->message_id = is91ep_bd.message_id;
      }
    }
    else
    {
      LOGI("error in function wms_ts_decode_IS91EP_payload");
      return WMS_GENERAL_ERROR_S;
    }
  }  /* if */
  /* end of embedded IS91EP msg processing */


  cl_bd_ptr->user_data.num_headers = 0;

  /* Decode GSM 7 bit characters and User Data Headers */
  {
    uint8    udhl = 0;
    uint8    fill_bits = 0;


    /* Decode UDH's first
    */
    if ((cl_bd_ptr->message_id.udh_present == TRUE) && (!is_unpacked_user_data))
    {
      /* To avoid memory conflict, copy the user data to a different place.
      */
      cdma_user_data = cl_bd_ptr->user_data;

      udhl = wms_ts_decode_user_data_header( cdma_user_data.number_of_digits,
                                             cdma_user_data.data,
                                             & cl_bd_ptr->user_data.num_headers,/*OUT*/
                                             cl_bd_ptr->user_data.headers );    /*OUT*/

      if( udhl+1 > cdma_user_data.data_len )
      {
        st = WMS_INVALID_PARM_SIZE_S;
      }
      else if( cdma_user_data.encoding == WMS_ENCODING_GSM_7_BIT_DEFAULT )
      {
        /* Unpack GSM 7-bit characters */

       if( udhl > 0 )
       {
         /*The number of fill bits required to make a septet boundary*/
         fill_bits =( (cl_bd_ptr->user_data.number_of_digits * 7) - ((udhl+1)*8) ) % 7;

         cl_bd_ptr->user_data.data_len = (uint8) (( (cl_bd_ptr->user_data.number_of_digits * 7) -
                                             ((udhl+1)*8) ) / 7);

         if (fill_bits !=0)
         {
           fill_bits = 8-fill_bits;
         }
       }

       cl_bd_ptr->user_data.number_of_digits =cl_bd_ptr->user_data.data_len;

       (void)wms_ts_unpack_gw_7_bit_chars
              ( cdma_user_data.data + udhl + 1,
              cl_bd_ptr->user_data.data_len,
              WMS_CDMA_USER_DATA_MAX,
              fill_bits,
              cl_bd_ptr->user_data.data /* OUT */
              );
      }
      else if( cdma_user_data.encoding == WMS_ENCODING_ASCII || cdma_user_data.encoding == WMS_ENCODING_IA5)
      {
        if( udhl > 0 )
        {
          fill_bits =( (cl_bd_ptr->user_data.number_of_digits * 7) - ((udhl+1)*8) ) % 7;
          if (fill_bits == 7)
          {
            fill_bits = 0;
          }
        }

        cl_bd_ptr->user_data.number_of_digits = (uint8) (( (cl_bd_ptr->user_data.number_of_digits * 7) - ((udhl+1)*8) ) / 7);

        cl_bd_ptr->user_data.padding_bits = fill_bits;

        digit_size = 8;
        bit_pos = ((udhl+1) * 8) + fill_bits;
        cl_bd_ptr->user_data.data_len = cl_bd_ptr->user_data.number_of_digits;

        for (i = 0; i<cl_bd_ptr->user_data.number_of_digits; i++)
        {
          cl_bd_ptr->user_data.data[i] = b_unpackb((uint8 *)cdma_user_data.data,
                                                     ( ((udhl+1) * 8) + fill_bits) + (i*digit_size), digit_size);
        }
      }
      else if( cdma_user_data.encoding == WMS_ENCODING_UNICODE)
      {
        if ( (udhl+1) % 2 == 0)
        {
          fill_bits = 0;
        }
        else
        {
          fill_bits = 8;
        }
        cl_bd_ptr->user_data.data_len = cdma_user_data.data_len - (udhl + 1 + fill_bits/8);
        cl_bd_ptr->user_data.number_of_digits   = cl_bd_ptr->user_data.data_len / 2;
        cl_bd_ptr->user_data.padding_bits = fill_bits;
        memcpy( cl_bd_ptr->user_data.data,
                cdma_user_data.data + udhl + 1 + fill_bits/8,
                cdma_user_data.data_len - (udhl + 1 + fill_bits/8));
      }
      else // 8 Bit Encoding
      {
        /* Unpack 8-bit encoding
        */
        /* Copy the user data bits to the result by skipping the headers */
        cl_bd_ptr->user_data.data_len = MIN (cdma_user_data.data_len - (udhl+1), WMS_CDMA_USER_DATA_MAX-1);
        cl_bd_ptr->user_data.number_of_digits =
                                        cdma_user_data.number_of_digits - (udhl+1);
        memcpy( cl_bd_ptr->user_data.data,
                  cdma_user_data.data + udhl+1,
                  cl_bd_ptr->user_data.data_len );
      }
    }
    else
    {
      if ((cl_bd_ptr->user_data.encoding == WMS_ENCODING_GSM_7_BIT_DEFAULT) && (!is_unpacked_user_data))
      {
        /* Unpack GSM 7-bit characters */

        cdma_user_data = cl_bd_ptr->user_data;

        /* Make sure 'data_len' means the number of digits for GSM-7-bit
        ** when this function returns
        */
        cl_bd_ptr->user_data.data_len = cl_bd_ptr->user_data.number_of_digits;

        (void)wms_ts_unpack_gw_7_bit_chars
            ( cdma_user_data.data ,
              cl_bd_ptr->user_data.data_len, /* number of 7-bit chars */
              WMS_MAX_LEN,
              fill_bits,
              cl_bd_ptr->user_data.data /* OUT */
            );
       }
    }
  }

  /* done */
  return st;

} /* end of wms_ts_decode_CDMA_bd() */

/* <EJECT> */
/*=========================================================================
FUNCTION
  wms_ts_decode_IS91EP_bd

DESCRIPTION
  This function decodes the IS91 EP bearer data into the client structure.
  to the internal message structure.

DEPENDENCIES
  None

RETURN VALUE
  Status of the translation.

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_decode_IS91EP_bd
(
  const wms_raw_ts_data_s_type    * raw_bd_ptr,  /* IN */
  wms_client_bd_s_type            * cl_bd_ptr    /* OUT */
)
{
  wms_IS91EP_type_e_type       is91ep_type;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  if (raw_bd_ptr == NULL || cl_bd_ptr == NULL )
  {
    LOGE( "NULL pointer in wms_ts_decode_IS91EP_bd");
    return WMS_NULL_PTR_S;
  }
  else if( raw_bd_ptr->format == WMS_FORMAT_ANALOG_CLI )
  {
    is91ep_type = WMS_IS91EP_CLI_ORDER;
  }
  else if( raw_bd_ptr->format == WMS_FORMAT_ANALOG_VOICE_MAIL )
  {
    is91ep_type = WMS_IS91EP_VOICE_MAIL;
  }
  else
  {
    is91ep_type = WMS_IS91EP_SHORT_MESSAGE;
  }


  return wms_ts_decode_IS91EP_payload
           (
             is91ep_type,
             (uint8) MIN(raw_bd_ptr->len,WMS_MAX_LEN),          /* number of digits */
             (uint8*) raw_bd_ptr->data, /* the digits */
             cl_bd_ptr     /* OUT */
           );

} /* end of wms_ts_decode_IS91EP_bd() */


/* <EJECT> */
/*=========================================================================
FUNCTION
  wms_ts_decode_MWI_bd

DESCRIPTION
  This function decodes the MWI (Message Waiting Indication) bearer data
  into the client structure.  to the internal message structure.

DEPENDENCIES
  None

RETURN VALUE
  Status of the translation.

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_decode_MWI_bd
(
  const wms_raw_ts_data_s_type            * raw_bd_ptr,   /* IN */
  wms_client_bd_s_type                    * cl_bd_ptr     /* OUT */
)
{

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  if (raw_bd_ptr == NULL || cl_bd_ptr == NULL)
  {
    LOGE( "NULL pointers in wms_ts_decode_MWI_bd");
    return WMS_NULL_PTR_S;
  }
  else
  {
    cl_bd_ptr->mask = WMS_MASK_BD_NUM_OF_MSGS;

    cl_bd_ptr->num_messages         = raw_bd_ptr->data[0];

    /* done */
    return WMS_OK_S;
  }
} /* end of wms_ts_decode_MWI_bd() */


/* <EJECT> */
/*=========================================================================
FUNCTION
  wms_ts_decode_bearer_data

DESCRIPTION
  This function decodes the SMS bearer data from the raw format
  to the client structure.

DEPENDENCIES
  None

RETURN VALUE
  Encoding result. WMS_OK_S means encoding succeeded; otherwise it failed.

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_decode_bearer_data
(
  const wms_raw_ts_data_s_type            * raw_bd_ptr,   /* IN */
  wms_client_bd_s_type                    * cl_bd_ptr     /* OUT */
)
{
  wms_status_e_type                st = WMS_OK_S;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  if (raw_bd_ptr == NULL || cl_bd_ptr == NULL)
  {
    LOGE( "NULL pointers in wms_ts_decode_bearer_data");
    return WMS_NULL_PTR_S;
  }
  else
  {
    cl_bd_ptr->mask = 0;

    if( raw_bd_ptr->len == 0 )
    {
      return WMS_OK_S;  /* SHORT-RETURN */
    }


    switch( raw_bd_ptr->format )
    {
      case WMS_FORMAT_CDMA:
      case WMS_FORMAT_ANALOG_AWISMS:

        st = wms_ts_decode_CDMA_bd( raw_bd_ptr, FALSE, FALSE, cl_bd_ptr );
        break;


      case WMS_FORMAT_ANALOG_CLI:
      case WMS_FORMAT_ANALOG_VOICE_MAIL:
      case WMS_FORMAT_ANALOG_SMS:
        /* Analog messages */
        st = wms_ts_decode_IS91EP_bd( raw_bd_ptr, cl_bd_ptr );
        break;


      case WMS_FORMAT_MWI:
        st = wms_ts_decode_MWI_bd( raw_bd_ptr, cl_bd_ptr );
        break;


      default:
        LOGE( "**Invalid msg fmt when decoding: %d",
                  raw_bd_ptr->format);
        st = WMS_INVALID_FORMAT_S;
        break;
    }

    /* done: */
    return st;
  }

} /* end of wms_ts_decode_bearer_data() */

/*=========================================================================
FUNCTION
  wms_ts_decode_cdma_bd_with_other

DESCRIPTION
  Allow the client to decode other CDMA BD parameters. See also function 
  comments for wms_ts_decode().

  The client shall make sure 'input_other_len' reflects the buffer size of
  'other_data' which should have enough room for decoding the other parameters.
  The size of the memory allocation in 'other_data' can be determined
  from the 'desired_other_len' after calling wms_ts_decode().

DEPENDENCIES
  None

RETURN VALUE
  Decoding status

SIDE EFFECTS
  None
=========================================================================*/
wms_status_e_type wms_ts_decode_cdma_bd_with_other
(
  const wms_raw_ts_data_s_type            * raw_ts_data_ptr,
  wms_client_ts_data_s_type               * client_ts_data_ptr
)
{
  wms_status_e_type  st;

  if( NULL == raw_ts_data_ptr ||
      NULL == client_ts_data_ptr )
  {
    st = WMS_NULL_PTR_S;
  }
  else
  {
    st = wms_ts_decode_CDMA_bd( raw_ts_data_ptr,
                                FALSE, /* is_unpacked? */
                                TRUE,  /* decode_other? */
                                & client_ts_data_ptr->u.cdma );
  }
  return st;
} /* wms_ts_decode_cdma_bd_with_other() */


/*=========================================================================
FUNCTION
  wms_ts_convert_cl2tl

DESCRIPTION
  This function converts the SMS TL data from the client structure
  to the internal structure.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

=========================================================================*/
void wms_ts_convert_cl2tl
(
  const wms_cdma_message_s_type     * cl_ptr,    /* IN */
  wms_tl_message_type               * tl_ptr    /* OUT */
)
{
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

   /*---- checking ---- */
  if ( cl_ptr == NULL || tl_ptr == NULL )
  {
    LOGE("wms_ts_convert_cl2tl null ptr!");
  }
  else
  {
    tl_ptr->mask = 0;

    tl_ptr->is_mo   = cl_ptr->is_mo;

    if( cl_ptr->teleservice == WMS_TELESERVICE_BROADCAST )
    {
      tl_ptr->tl_message_type = WMS_TL_TYPE_BROADCAST;
    }
    else
    {
      tl_ptr->tl_message_type = WMS_TL_TYPE_POINT_TO_POINT;
    }

    tl_ptr->mask |= WMS_MASK_TL_TELESERVICE_ID;
    tl_ptr->teleservice = cl_ptr->teleservice;

    if( cl_ptr->is_service_present == TRUE )
    {
      tl_ptr->mask |= WMS_MASK_TL_BC_SRV_CATEGORY;
      tl_ptr->service = cl_ptr->service;
    }

    // address should always be present
    tl_ptr->mask |= WMS_MASK_TL_ADDRESS;
    tl_ptr->address = cl_ptr->address;

    if( cl_ptr->subaddress.number_of_digits != 0 )
    {
      tl_ptr->mask |= WMS_MASK_TL_SUBADDRESS;
      tl_ptr->subaddress = cl_ptr->subaddress;
    }

    if( cl_ptr->is_tl_ack_requested )
    {
      tl_ptr->mask |= WMS_MASK_TL_BEARER_REPLY_OPTION;
      tl_ptr->bearer_reply_seq_num = 255;   /* dummy */
    }

    /* there is no cause code since it is not an Ack msg */

    if( cl_ptr->raw_ts.len != 0 )
    {
      tl_ptr->mask |= WMS_MASK_TL_BEARER_DATA;
    }
  }

  /* done */
  return;

} /* end of wms_ts_convert_cl2tl() */


/*
*/
wms_status_e_type wms_ts_cdma_encode_address
(
  const wms_address_s_type  * address_ptr,
  uint8               * parm_len_ptr,
  uint8               * data
)
{
  uint32       bit_pos = 0;
  wms_status_e_type  st = WMS_OK_S;
  uint8        digit_size;
  uint32       i;

  /*---- checking ---- */
  if ( address_ptr == NULL || parm_len_ptr == NULL || data == NULL)
  {
    LOGE("wms_ts_encode_address null ptr!");
    return WMS_NULL_PTR_S;
  }


  /* Digit Mode */
  b_packb( (uint8) address_ptr->digit_mode,
           data,
           (uint16) (bit_pos),
           1 );
  bit_pos++;

  /* Number mode */
  b_packb( (uint8) address_ptr->number_mode,
           data,
           (uint16) (bit_pos),
           1 );
  bit_pos++;

  /* Number type */
  if( address_ptr->digit_mode == WMS_DIGIT_MODE_8_BIT )
  {
    digit_size = 8;

    b_packb( (uint8) address_ptr->number_type,
             data,
             (uint16) (bit_pos),
             3 );
    bit_pos += 3;

    if( address_ptr->number_mode ==
          WMS_NUMBER_MODE_NONE_DATA_NETWORK )
    {
      b_packb( (uint8) address_ptr->number_plan,
               data,
               (uint16) (bit_pos),
               4 );
      bit_pos += 4;
    }
  }
  else
  {
    digit_size = 4;
  }

  /* Address size */
  b_packb( address_ptr->number_of_digits,
           data,
           (uint16) (bit_pos),
           8 );
  bit_pos += 8;

  /* pack the digits */
  for( i= 0; i < address_ptr->number_of_digits; i ++ )
  {
    b_packb( address_ptr->digits[i],
             data,
             (uint16) (bit_pos),
             digit_size );
    bit_pos += digit_size;
  }

  if( bit_pos % 8 != 0 )
  {
    /* pad 0 to remaining bits
    */
    digit_size = (uint8)(8 - bit_pos % 8);
    b_packb( 0, data, (uint16) (bit_pos),  digit_size );
    bit_pos += digit_size;
  }

  *parm_len_ptr = (uint8) (bit_pos/8);

  return st;

} /* wms_ts_cdma_encode_address() */


/*
*/
wms_status_e_type wms_ts_cdma_encode_subaddress
(
  const wms_subaddress_s_type  * address_ptr,
  uint8                  * parm_len_ptr,
  uint8                  * data
)
{
  uint32             bit_pos = 0;
  wms_status_e_type  st = WMS_OK_S;
  uint8              digit_size;
  uint32       i;

  /*---- checking ---- */
  if ( address_ptr == NULL || parm_len_ptr == NULL || data == NULL)
  {
    LOGE("wms_ts_encode_subaddress null ptr!");
    return WMS_NULL_PTR_S;
  }

  /* subaddress type */
  b_packb( (uint8) address_ptr->type,
           data,
           (uint16) (bit_pos),
           3 );
  bit_pos += 3;

  /* Odd flag */
  b_packb( address_ptr->odd,
           data,
           (uint16) (bit_pos),
           1 );
  bit_pos += 1;

  /* Address size */
  b_packb( address_ptr->number_of_digits,
           data,
           (uint16) (bit_pos),
           8 );
  bit_pos += 8;

  /* Digit size */
  digit_size = 8;  /* always 8 */

  /* Pack the digits */
  for( i=0; i < address_ptr->number_of_digits; i++ )
  {
    b_packb( address_ptr->digits[i],
             data,
             (uint16) (bit_pos),
             digit_size );
    bit_pos += digit_size;
  }

  /* pad last 4 bits with 0 */
  b_packb( 0,
           data,
           (uint16) (bit_pos),
           4 );
  bit_pos += 4;

  /* return len */
  * parm_len_ptr = (uint8) (bit_pos/8);

  return st;

} /* wms_ts_cdma_encode_subaddress() */

/*=========================================================================
FUNCTION
  wms_ts_encode_CDMA_tl

DESCRIPTION
  This function encodes the SMS TL fields in CDMA mode.
  If the BD mask does not include Bearer Data, the raw_bd_ptr can be NULL.

DEPENDENCIES
  None

RETURN VALUE
  Status of the translation.

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_encode_CDMA_tl
(
  const wms_tl_message_type        * tl_msg_ptr,   /* IN */
  const wms_raw_ts_data_s_type     * raw_bd_ptr,   /* IN */
  wms_OTA_message_type             * OTA_msg_ptr   /* OUT */
)
{
  uint16                       pos = 0;
  uint8                        parm_len;
  uint8                        * parm_len_ptr;
  uint8                        * data;

  wms_status_e_type   st = WMS_OK_S;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*---- checking ---- */
  if ( tl_msg_ptr == NULL || OTA_msg_ptr == NULL)
  {
    LOGE("wms_ts_encode_CDMA_tl null ptr!");
    return WMS_NULL_PTR_S;
  }

  /* Check whether mandatory srv_category is present */
  if( WMS_TL_TYPE_BROADCAST == tl_msg_ptr->tl_message_type )
  {
    if( FALSE == (tl_msg_ptr->mask & WMS_MASK_TL_BC_SRV_CATEGORY) )
    {
       return WMS_MISSING_PARM_S;
    }
  }


  data = (uint8*) OTA_msg_ptr->data;

  {
    /* For the following special cases, there is TL information and raw_bd_ptr
    ** has the OTA data ready
    */
    if( tl_msg_ptr->mask & WMS_MASK_TL_TELESERVICE_ID )
    {
      if( tl_msg_ptr->teleservice == WMS_TELESERVICE_MWI ||
          tl_msg_ptr->teleservice == WMS_TELESERVICE_IS91_PAGE ||
          tl_msg_ptr->teleservice == WMS_TELESERVICE_IS91_VOICE_MAIL ||
          tl_msg_ptr->teleservice == WMS_TELESERVICE_IS91_SHORT_MESSAGE )
      {
        if( raw_bd_ptr == NULL )
        {
          return WMS_NULL_PTR_S;
        }
        else
        {  
          OTA_msg_ptr->format   = raw_bd_ptr->format;
          OTA_msg_ptr->data_len = (uint16) MIN (raw_bd_ptr->len, WMS_MAX_LEN);
          memcpy( (uint8 *)OTA_msg_ptr->data, raw_bd_ptr->data, OTA_msg_ptr->data_len );
          return WMS_OK_S; /* RETURN */
        }
      }
    }

    /* msg type
    */
    data[0] = (uint8) tl_msg_ptr->tl_message_type;
    pos ++;

    /* the remaining parameters have one or more of the following:
       - PARAMETER_ID    8 bits
       - PARAMETER_LEN   8 bits
       - Parameter Data  8 x PARAMETER_LEN
    */

    if( tl_msg_ptr->mask & WMS_MASK_TL_TELESERVICE_ID )
    {
      data[pos] = (uint8) WMS_TL_TELESERVICE_ID;
      pos++; /* skip parm id */

      data[pos] = parm_len = 2;
      pos++; /* skip len */

      b_packw( (uint16) tl_msg_ptr->teleservice,
               data,
               (uint16) (pos * 8),
               parm_len * 8 );

      pos += parm_len;  /* skip parm data */
    }

    if( tl_msg_ptr->mask & WMS_MASK_TL_BC_SRV_CATEGORY )
    {
      data[pos] = (uint8) WMS_TL_BC_SRV_CATEGORY;
      pos++; /* skip parm id */

      data[pos] = parm_len = 2;
      pos++; /* skip len */

      b_packw( (uint16) (tl_msg_ptr->service),
               data,
               (uint16) (pos * 8),
               parm_len * 8 );

      pos += parm_len;  /* skip parm data */
    }

    if( tl_msg_ptr->mask & WMS_MASK_TL_ADDRESS )
    {
      data[pos] = (uint8) (tl_msg_ptr->is_mo ? WMS_TL_DEST_ADDRESS : WMS_TL_ORIG_ADDRESS);
      pos++; /* skip parm id */

      parm_len_ptr = data + pos;   /* will set value later */
      pos ++; /* skip parm len */

      st = wms_ts_cdma_encode_address( & tl_msg_ptr->address,
                                       parm_len_ptr,
                                       data+pos );
      pos += * parm_len_ptr; /* skip parm data */
    }

    if( tl_msg_ptr->mask & WMS_MASK_TL_SUBADDRESS )
    {
      data[pos] = (uint8) (tl_msg_ptr->is_mo ? WMS_TL_DEST_SUBADDRESS : WMS_TL_ORIG_SUBADDRESS);
      pos++; /* skip parm id */

      parm_len_ptr = data + pos;   /* will set value later */
      pos ++; /* skip parm len */

      st = wms_ts_cdma_encode_subaddress( & tl_msg_ptr->subaddress,
                                          parm_len_ptr,
                                          data+pos );
      pos += * parm_len_ptr;
    }

    if( tl_msg_ptr->mask & WMS_MASK_TL_BEARER_REPLY_OPTION )
    {
      data[pos] = (uint8) WMS_TL_BEARER_REPLY_OPTION;
      pos++; /* skip parm id */

      data[pos] = parm_len = 1;
      pos++;

      b_packb( tl_msg_ptr->bearer_reply_seq_num,
               data,
             (uint16) (pos * 8),
               6 );  /* high 6 bits */

      b_packb( 0,
               data,
             (uint16) (pos * 8 + 6),
               2 );  /* low 2 bits: reserved, set to 0 */

      pos += parm_len;  /* skip parm data */
    }

    if( tl_msg_ptr->mask & WMS_MASK_TL_CAUSE_CODES )
    {
      data[pos] = (uint8) WMS_TL_CAUSE_CODES;
      pos++; /* skip parm id */

      data[pos] = (tl_msg_ptr->cause_code.error_class==WMS_ERROR_NONE)?
                  1 : 2;
      pos++; /* skip parm len */

      b_packb( tl_msg_ptr->cause_code.reply_seq_num,
               data,
             (uint16) (pos * 8),
               6 );  /* high 6 bits */

      /* TBD: b_packb() has a bug if error_class > 3 */
      b_packb( (uint8)( (uint8)tl_msg_ptr->cause_code.error_class & 0x3),
               data,
               (uint16) (pos * 8 + 6),
               2 );  /* low 2 bits */

      pos++; /* skip first parm data byte */

      if( tl_msg_ptr->cause_code.error_class != WMS_ERROR_NONE )
      {
        data[pos] = (uint8) (tl_msg_ptr->cause_code.tl_status);
        pos++; /* skip second parm data byte */
      }
    }

  
    if( tl_msg_ptr->mask & WMS_MASK_TL_BEARER_DATA )
    {
      if ( raw_bd_ptr == NULL)
      {
        LOGE("wms_ts_encode_CDMA_tl null ptr!");
        st = WMS_NULL_PTR_S;
      }
      else if( raw_bd_ptr->len == 0 || raw_bd_ptr->len > WMS_MAX_LEN )
      {
        LOGE( "Invalid bd len: %d", (int)raw_bd_ptr->len);
        st = WMS_INVALID_PARM_VALUE_S;
      }
      else
      {
        data[pos] = (uint8) WMS_TL_BEARER_DATA;
        pos++; /* skip parm id */

        data[pos] = (uint8) MIN (raw_bd_ptr->len, WMS_MAX_LEN);
        pos++; /* skip parm len */

        memcpy( data+pos, raw_bd_ptr->data, MIN((uint8)raw_bd_ptr->len, (uint8)WMS_MAX_LEN-pos) );
        pos += (uint16) raw_bd_ptr->len; /* skip parm data */
      }
    }

    OTA_msg_ptr->format   = WMS_FORMAT_CDMA;
    OTA_msg_ptr->data_len = pos;
      /* data was filled in the above */

    if( st == WMS_OK_S && pos > WMS_TL_MAX_LEN )
    {
      // encoded data has too many bytes
      LOGE( "Invalid parm size: %d", pos);
      st = WMS_INVALID_PARM_SIZE_S;
    }

    /* done */
    return st;
  }
} /* end of wms_ts_encode_CDMA_tl() */

/*===========================================================================

FUNCTION    wms_ts_cdma_cl2OTA

DESCRIPTION
  Convert the client message format to OTA format.

DEPENDENCIES
  None

RETURN VALUE
  status of the operation.

SIDE EFFECTS
 

===========================================================================*/
wms_status_e_type wms_ts_cdma_cl2OTA
(
  wms_client_message_s_type   *cl_msg,
  uint8                       *data_buf,
  uint16                       data_buf_len,
  uint16                      *ota_data_len
)
{
  wms_status_e_type             st = WMS_OK_S;
  static wms_OTA_message_type   ota_data;
  static wms_tl_message_type    tl_msg;
 
   /*---- checking ---- */
  if (NULL == cl_msg)
  {
    LOGE( "wms_ts_cdma_cl2OTA() null ptr!");
    return WMS_NULL_PTR_S;
  }
  
  /* reset static structures */
  (void) memset((uint8 *)ota_data.data, 0, sizeof(ota_data.data));
  (void) memset(&tl_msg, 0, sizeof(tl_msg));
 
  wms_ts_convert_cl2tl(&(cl_msg->u.cdma_message), &tl_msg);
  st = wms_ts_encode_CDMA_tl(&tl_msg,  &(cl_msg->u.cdma_message.raw_ts) , &ota_data);
  if (WMS_OK_S == st) 
  {   
     if (NULL != ota_data_len) 
     {
        *ota_data_len = ota_data.data_len;
     }     
     if (NULL != data_buf && data_buf_len > 0)
     {
       (void) memcpy((uint8 *)data_buf, (uint8 *)ota_data.data, MIN(data_buf_len, ota_data.data_len));
     }  
  } else 
  {
     LOGE( "wms_ts_encode_CDMA_tl()  failed w/st = %d", st);
  }
 
  return st;
}

wms_status_e_type wms_ts_cdma_decode_address
(
  const uint8                 * data,
  uint8                 parm_len,
  wms_address_s_type    * address_ptr
)
{
  uint32     bit_pos = 0;
  uint8      digit_size;
  wms_status_e_type   st = WMS_OK_S;
  uint32       i;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    /*---- checking ---- */
  if ( address_ptr == NULL || data == NULL)
  {
    LOGE("wms_ts_decode_address null ptr!");
    return WMS_NULL_PTR_S;
  }


  /* Digit mode */
  address_ptr->digit_mode = (wms_digit_mode_e_type)
                  b_unpackb( (uint8*) data, (uint16) (bit_pos), 1 );
  bit_pos ++;

  /* Number mode */
  address_ptr->number_mode = (wms_number_mode_e_type)
               b_unpackb( (uint8*) data, (uint16) (bit_pos), 1 );
  bit_pos ++;


  /* Number type, and determine digit size */
  if( address_ptr->digit_mode == WMS_DIGIT_MODE_8_BIT )
  {
    address_ptr->number_type = (wms_number_type_e_type)
                    b_unpackb( (uint8*) data, (uint16) (bit_pos), 3 );
    bit_pos += 3;

    digit_size = 8;
  }
  else
  {
    address_ptr->number_type = WMS_NUMBER_UNKNOWN;
    digit_size = 4;
  }


  /* Number plan */
  if( address_ptr->digit_mode  == WMS_DIGIT_MODE_8_BIT &&
      address_ptr->number_mode == WMS_NUMBER_MODE_NONE_DATA_NETWORK )
  {
    address_ptr->number_plan = (wms_number_plan_e_type)
                b_unpackb( (uint8*) data, (uint16) (bit_pos), 4);
    bit_pos += 4;
  }
  else
  {
    /* number plan is not used */
    address_ptr->number_plan = WMS_NUMBER_PLAN_UNKNOWN;
  }

  /* Address size */
  address_ptr->number_of_digits=
                    b_unpackb( (uint8*) data, (uint16) (bit_pos), 8 );
  bit_pos += 8;


  /* Extract all digits:
  */
  if( address_ptr->number_of_digits > WMS_ADDRESS_MAX )
  {
    LOGE( "decoding: address digits number too big: %d",
              address_ptr->number_of_digits);
    st = WMS_INVALID_PARM_SIZE_S;
  }
  else
  {
    for( i=0; i < address_ptr->number_of_digits; i++ )
    {
      address_ptr->digits[i] = b_unpackb( (uint8*) data,
                                                   (uint16) (bit_pos),
                                                   digit_size );
      bit_pos += digit_size;
    }
  }

  /* Parm len checking */
  if( bit_pos > (uint32)(parm_len*8) )
  {
    LOGE( "decoding: address too long: %d>%d",
              bit_pos, parm_len*8);
    st = WMS_INVALID_PARM_SIZE_S;
  }

  return st;

} /* wms_ts_cdma_decode_address() */



/*
*/
wms_status_e_type wms_ts_cdma_decode_subaddress
(
  const uint8                 * data,
  uint8                 parm_len,
  wms_subaddress_s_type    * address_ptr
)
{
  uint32     bit_pos = 0;
  uint8      digit_size;
  wms_status_e_type   st = WMS_OK_S;
  uint32       i;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*---- checking ---- */
  if ( address_ptr == NULL || data == NULL)
  {
    LOGE("wms_ts_decode_subaddress null ptr!");
    return WMS_NULL_PTR_S;
  }


  /* address type */
  address_ptr->type = (wms_subaddress_type_e_type)
                  b_unpackb( (uint8*) data, (uint16) (bit_pos), 3 );
  bit_pos += 3;

  /* Odd flag */
  address_ptr->odd = b_unpackb( (uint8*) data, (uint16) (bit_pos), 1 );
  bit_pos += 1;

  /* Address size */
  address_ptr->number_of_digits = b_unpackb( (uint8*) data, (uint16) (bit_pos), 8 );
  bit_pos += 8;

  /* Extract all digits:
  */
  if( address_ptr->number_of_digits > WMS_SUBADDRESS_MAX )
  {
    LOGE( "decoding: subaddress digits number too big: %d",
              address_ptr->number_of_digits);
    st = WMS_INVALID_PARM_SIZE_S;
  }
  else
  {
    digit_size = 8;  /* always 8 bits so far */

    for( i=0; i < address_ptr->number_of_digits; i++ )
    {
      address_ptr->digits[i] = b_unpackb( (uint8*) data,
                                                  (uint16) (bit_pos),
                                                  digit_size );
      bit_pos += digit_size;
    }
  }

  /* parm len check */
  if( bit_pos > (uint32)(parm_len*8) )
  {
    LOGE( "decoding: subaddress too long: %d>%d",
              bit_pos, parm_len*8);
    st = WMS_INVALID_PARM_SIZE_S;
  }

  /* Done */
  return st;

} /* wms_ts_cdma_decode_subaddress() */

/* <EJECT> */
/*=========================================================================
FUNCTION
  wms_ts_decode_CDMA_tl

DESCRIPTION
  This function decodes the SMS TL fields in CDMA mode from the OTA format.

DEPENDENCIES
  None

RETURN VALUE
  Status of the translation.

SIDE EFFECTS
  None

=========================================================================*/
wms_status_e_type wms_ts_decode_CDMA_tl
(
  const wms_OTA_message_type     * OTA_msg_ptr,   /* IN */
  wms_tl_message_type            * tl_ptr,        /* OUT */
  wms_raw_ts_data_s_type         * raw_bd_ptr     /* OUT */
)
{
  wms_status_e_type       st = WMS_OK_S;
  uint16                    pos;      /* running position in bytes */
  wms_tl_parm_id_e_type parm_id = WMS_TL_DUMMY;
  uint8                    parm_len;
  uint16                    tl_len;  /* len of TL data */

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*---- checking ---- */
  if ( NULL == OTA_msg_ptr || NULL == tl_ptr || NULL == raw_bd_ptr || 0 == OTA_msg_ptr->data_len )
  {
    LOGE("wms_ts_decode_CDMA_tl null ptr or OTA data len is zero!");
    return WMS_NULL_PTR_S;
  }
  else if ( OTA_msg_ptr->data_len <= TL_HEADER_SIZE && WMS_FORMAT_MWI != OTA_msg_ptr->format )
  {
    LOGE("** TL Msg too short: %d", OTA_msg_ptr->data_len);
    return WMS_INVALID_PARM_SIZE_S;  /* SHORT-CIRCUIT RETURN */
  }
  else if ( OTA_msg_ptr->data_len > WMS_MAX_LEN )
  {
    LOGE("** TL Msg len too long: %d", OTA_msg_ptr->data_len);
    return WMS_INVALID_PARM_SIZE_S;  /* SHORT-CIRCUIT RETURN */
  }
  else
  {
      /* For the following special cases, TL has special values */
    if( OTA_msg_ptr->format == WMS_FORMAT_ANALOG_CLI ||
        OTA_msg_ptr->format == WMS_FORMAT_ANALOG_VOICE_MAIL ||
        OTA_msg_ptr->format == WMS_FORMAT_ANALOG_SMS ||
        OTA_msg_ptr->format == WMS_FORMAT_MWI )
    {
      tl_ptr->is_mo = FALSE;
      tl_ptr->address.number_of_digits = 0;

      tl_ptr->mask |= WMS_MASK_TL_TELESERVICE_ID;
      switch( OTA_msg_ptr->format )
      {
        case WMS_FORMAT_ANALOG_CLI:
          tl_ptr->teleservice = WMS_TELESERVICE_IS91_PAGE;
          break;

        case WMS_FORMAT_ANALOG_VOICE_MAIL:
          tl_ptr->teleservice = WMS_TELESERVICE_IS91_VOICE_MAIL;
          break;

        case WMS_FORMAT_ANALOG_SMS:
          tl_ptr->teleservice = WMS_TELESERVICE_IS91_SHORT_MESSAGE;
          break;

        case WMS_FORMAT_MWI:
          tl_ptr->teleservice = WMS_TELESERVICE_MWI;
          break;

        /* Default Condition will never be called*/
        default:
          break;
      }

      raw_bd_ptr->format = OTA_msg_ptr->format;
      raw_bd_ptr->len    = MIN (OTA_msg_ptr->data_len, WMS_MAX_LEN);
      memcpy( raw_bd_ptr->data, (uint8 *)OTA_msg_ptr->data, raw_bd_ptr->len );
      return WMS_OK_S; /* RETURN */
    }

    tl_len = OTA_msg_ptr->data_len;
    pos = 0;

    /* ---- start decoding ---- */

    tl_ptr->mask = 0;

    /* the first byte is msg type */

    if (OTA_msg_ptr->data[pos] > (uint8) WMS_TL_TYPE_MAX )
    {
      LOGE("WARNING: Invalid TL msg type: %d", OTA_msg_ptr->data[pos]);
      return WMS_INVALID_PARM_SIZE_S;
    }
    else
    {
      tl_ptr->tl_message_type = (wms_tl_message_type_e_type)(OTA_msg_ptr->data[pos]);
      pos ++;
    }

    /* the remain data has one or more of the following:
       - PARAMETER_ID    8 bits
       - PARAMETER_LEN   8 bits
       - Parameter Data  8 x PARAMETER_LEN
    */

    while( st == WMS_OK_S  )
    {

      if( pos == tl_len )
      {
        /* Good. Done with parameter processing successfully */
        break; /* out of while loop */
      }
      else if( pos + TL_PARM_SIZE > tl_len )
      {
        /* Current position goes beyond the msg size. */
        LOGE( "decoding: msg size %d incorrect", tl_len);
        st = WMS_INVALID_PARM_SIZE_S;
        break; /* out of while loop */
      }

      parm_id = (wms_tl_parm_id_e_type) ( OTA_msg_ptr->data[ pos ] );
      pos ++;  /* skip parm id */

      parm_len = OTA_msg_ptr->data[ pos ];
      pos ++;  /* skip parm len */

      if( pos + parm_len > tl_len )
      {
        /* parm data passes the max length of bearer data
        */
        LOGE( "decoding: parameter (id=%d) extends beyond msg size %d",
                  parm_id, tl_len);
        st = WMS_INVALID_PARM_SIZE_S;
        break; /* out of while loop */
      }


      /* Now pos should point to the parm data */
      /* After each case below, pos should point to the next parm Id */

      switch( parm_id )
      {

        case WMS_TL_TELESERVICE_ID:

          if( tl_ptr->mask & WMS_MASK_TL_TELESERVICE_ID )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }

          tl_ptr->mask |= WMS_MASK_TL_TELESERVICE_ID;

          if( pos  + 2 > tl_len ||
              parm_len != 2
            )
          {
            /* tl data too short , or bad len field */
            st = WMS_INVALID_PARM_SIZE_S;
          }
          else
          {
            tl_ptr->teleservice = (wms_teleservice_e_type)
                                b_unpackw( (uint8*) OTA_msg_ptr->data,
                                           (uint16) (pos*8),
                                             parm_len*8 );
            pos += parm_len; /* skip parm data */
          }

          break;


        case WMS_TL_BC_SRV_CATEGORY:

          if( tl_ptr->mask & WMS_MASK_TL_BC_SRV_CATEGORY )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= WMS_MASK_TL_BC_SRV_CATEGORY;

          if( pos  + 2 > tl_len ||
              parm_len != 2
            )
          {
            /* tl data too short , or bad len field */
            st = WMS_INVALID_PARM_SIZE_S;
          }
          else
          {
            tl_ptr->service = (wms_service_e_type)
                            b_unpackw( (uint8*) OTA_msg_ptr->data,
                                       (uint16) (pos*8),
                                         parm_len*8 );
            pos += parm_len; /* skip parm data */
          }

          break;

        case WMS_TL_ORIG_ADDRESS:
        case WMS_TL_DEST_ADDRESS:

          if( tl_ptr->mask & WMS_MASK_TL_ADDRESS )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= WMS_MASK_TL_ADDRESS;

          /* Set the MO/MT tag. */
          tl_ptr->is_mo = ( parm_id == WMS_TL_DEST_ADDRESS ) ?
                            TRUE : FALSE;

          st = wms_ts_cdma_decode_address( (uint8*) OTA_msg_ptr->data+pos,
                                           parm_len,
                                           & tl_ptr->address );
          pos += parm_len;
          break;

        case WMS_TL_ORIG_SUBADDRESS:
        case WMS_TL_DEST_SUBADDRESS:

          if( tl_ptr->mask & WMS_MASK_TL_SUBADDRESS )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= WMS_MASK_TL_SUBADDRESS;

          /* Set the MO/MT tag. */
          tl_ptr->is_mo = ( parm_id == WMS_TL_DEST_SUBADDRESS ) ?
                                TRUE : FALSE;

          st = wms_ts_cdma_decode_subaddress( (uint8*) OTA_msg_ptr->data+pos,
                                              parm_len,
                                              & tl_ptr->subaddress );
          pos += parm_len;  /* skip parm data */
          break;

        case WMS_TL_BEARER_REPLY_OPTION:

          if( tl_ptr->mask & WMS_MASK_TL_BEARER_REPLY_OPTION )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= WMS_MASK_TL_BEARER_REPLY_OPTION;

          if( parm_len != 1 )
          {
            LOGE("decoding: bearer reply option len is not 1: %d. Reset to 1.", parm_len);
            return WMS_INVALID_PARM_SIZE_S;
          }

          tl_ptr->bearer_reply_seq_num = OTA_msg_ptr->data[pos] >> 2;
            /* get high 6 bits */

          pos += parm_len; /* skip parm data */

          break;

        case WMS_TL_CAUSE_CODES:

          if( tl_ptr->mask & WMS_MASK_TL_CAUSE_CODES )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          tl_ptr->mask |= WMS_MASK_TL_CAUSE_CODES;

          tl_ptr->cause_code.reply_seq_num = OTA_msg_ptr->data[pos] >> 2;
            /* get high 6 bits */
          tl_ptr->cause_code.error_class   = (wms_error_class_e_type)
                                                 (OTA_msg_ptr->data[pos] & 0x03);
            /* get low 2 bits */

          pos ++; /* skip first parm data byte */

          if( parm_len == 1 )
          {
            /* If parm_len==1, error_class shall be NONE */
            if( tl_ptr->cause_code.error_class != WMS_ERROR_NONE)
            {
              LOGE("decoding: invalid parm len = 1 with valid error cause identifier",parm_len);
              st = WMS_INVALID_PARM_SIZE_S;
            }
          }
          else if( parm_len == 2 )
          {
            tl_ptr->cause_code.tl_status = (wms_cdma_tl_status_e_type)
                                                      OTA_msg_ptr->data[pos];
            pos ++; /* skip cause code in parm data */
          }
          else
          {
            LOGE("decoding: invalid parm len for CauseCodes: %d",parm_len);
            st = WMS_INVALID_PARM_SIZE_S;
          }

          break;


        case WMS_TL_BEARER_DATA:

          if( tl_ptr->mask & WMS_MASK_TL_BEARER_DATA )
          {
            MSG_DUP_PARM;
            pos += parm_len;    /* skip parm data */
            break;              /* continue with next parm */
          }


          /* bearer data is not decoded here */
          /* it will be copied to the output */

          tl_ptr->mask |= WMS_MASK_TL_BEARER_DATA;

          if( parm_len == 0 )
          {
            LOGE("decoding: bearer data len is 0");
            st = WMS_INVALID_PARM_SIZE_S;
          }
          /* parm_len is uint8, so it won't be greater than WMS_MAX_LEN */
          else if( pos + parm_len > tl_len )
          {
            LOGE("decoding: bearer data len %d extends beyond msg size %d",
                     parm_len, tl_len);
            st = WMS_INVALID_PARM_SIZE_S;
          }
          else
          {
            /* copy bearer data to output
            */
            raw_bd_ptr->format = WMS_FORMAT_CDMA;
            raw_bd_ptr->len = MIN (parm_len, WMS_MAX_LEN);
            memcpy( raw_bd_ptr->data, (uint8*)OTA_msg_ptr->data + pos, raw_bd_ptr->len );
          }

          pos += parm_len; /* skip parm data */

          break;

        default:
          LOGE( "decoding: Invalid parm id: %d", parm_id);
          pos += parm_len; /* skip parm data */
          break;           /* continue with next parm */

      } /* switch */

    } /* while */


    /* Null the raw bd */
    if( ! ( tl_ptr->mask & WMS_MASK_TL_BEARER_DATA ) )
    {
      raw_bd_ptr->len = 0;
    }


    /* -------- check CDMA TL mandatory fields -------- */
    if( st == WMS_OK_S && OTA_msg_ptr->format == WMS_FORMAT_CDMA )
    {
      switch( tl_ptr->tl_message_type )
      {
        case WMS_TL_TYPE_POINT_TO_POINT:

          if( ! ( tl_ptr->mask & WMS_MASK_TL_ADDRESS ) )
          {
            LOGE("No address present in msg!");
            st = WMS_MISSING_PARM_S;
          }

          break;

        case WMS_TL_TYPE_BROADCAST:

          if( ! ( tl_ptr->mask & WMS_MASK_TL_BC_SRV_CATEGORY ) )
          {
            LOGE( "decoding: broadcast category is not present!");
            st = WMS_MISSING_PARM_S;
          }

          break;

        case WMS_TL_TYPE_ACK:

          if( ! ( tl_ptr->mask & WMS_MASK_TL_CAUSE_CODES ) )
          {
            LOGE( "decoding: cause code is not present in Ack!");
            st = WMS_MISSING_PARM_S;
          }

          break;

        default:

          LOGE( "decoding: invalid TL msg type: %d",
                    tl_ptr->tl_message_type);
          st = WMS_INVALID_PARM_VALUE_S;
          break;

      } /* switch */

    } /* end of CDMA TL fields checking */
  }
  /* done */
  return st;

} /* end of wms_ts_decode_CDMA_tl() */

/* <EJECT> */
/*=========================================================================
FUNCTION
  wms_ts_convert_tl2cl

DESCRIPTION
  This function converts the SMS TL data from the internal structure
  to the client structure.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

=========================================================================*/
void wms_ts_convert_tl2cl
(
  const wms_tl_message_type       * tl_ptr,   /* IN */
  wms_cdma_message_s_type         * cl_ptr    /* OUT */
)
{

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  if ( tl_ptr == NULL || cl_ptr == NULL )
  {
    LOGE("wms_ts_convert_tl2cl null ptr!");
    return;
  }
  else
  {
    cl_ptr->is_mo  = tl_ptr->is_mo;

    /* msg type
    */
    if( tl_ptr->tl_message_type == WMS_TL_TYPE_BROADCAST )
    {
      cl_ptr->teleservice  = WMS_TELESERVICE_BROADCAST;
    }
    else if( tl_ptr->tl_message_type == WMS_TL_TYPE_POINT_TO_POINT )
    {
      /* teleservice id
      */
      if( tl_ptr->mask & WMS_MASK_TL_TELESERVICE_ID )
      {
        cl_ptr->teleservice = tl_ptr->teleservice;
      }
      else
      {
        cl_ptr->teleservice = WMS_TELESERVICE_UNKNOWN;
      }
    }
    else
    {
      /* it is an ACK msg; not applicable for client msg */
      return;  /* SHORT-RETURN */
    }


    /* address
    */
    if( tl_ptr->mask & WMS_MASK_TL_ADDRESS )
    {
      cl_ptr->address = tl_ptr->address;
    }
    else
    {
      cl_ptr->address.number_of_digits = 0;
    }

    /* sub-address
    */
    if( tl_ptr->mask & WMS_MASK_TL_SUBADDRESS )
    {
      cl_ptr->subaddress = tl_ptr->subaddress;
    }
    else
    {
      cl_ptr->subaddress.number_of_digits = 0;
    }

    /* TL ack flag
    */
    if( tl_ptr->mask & WMS_MASK_TL_BEARER_REPLY_OPTION )
    {
      cl_ptr->is_tl_ack_requested = TRUE;
    }
    else
    {
      cl_ptr->is_tl_ack_requested = FALSE;
    }

    /* broadcast category
    */
    if( tl_ptr->mask & WMS_MASK_TL_BC_SRV_CATEGORY )
    {
      cl_ptr->is_service_present = TRUE;
      cl_ptr->service = tl_ptr->service;
    }
    else
    {
      cl_ptr->is_service_present = FALSE;
      cl_ptr->service = WMS_SRV_UNKOWN;  /* 0 means unspecified */
    }
    /* cause code is only in ACK msg; N/A for client msg */
  }

  /* done */
  return;

} /* wms_ts_convert_tl2cl() */

/*===========================================================================

FUNCTION    wms_ts_cdma_OTA2cl

DESCRIPTION
  Convert the OTA data to client message format.

DEPENDENCIES
  None

RETURN VALUE
  status of the operation.

SIDE EFFECTS
  *cl_msg modified directly.

===========================================================================*/
wms_status_e_type wms_ts_cdma_OTA2cl
(
  const uint8                 *data,
  uint16                      length,
  wms_client_message_s_type   *cl_msg
)
{
  wms_status_e_type             st = WMS_OK_S;
  static wms_OTA_message_type   ota_data;
  static wms_tl_message_type    tl_msg;
  static wms_raw_ts_data_s_type raw_ts_data;

  /*---- checking ---- */
  if ( data == NULL || cl_msg == NULL)
  {
    LOGE("wms_ts_cdma_OTA2cl() null ptr!");
    return WMS_NULL_PTR_S;
  }

  /* reset static structures */
  (void) memset((uint8 *)ota_data.data, 0, sizeof(ota_data.data));
  (void) memset(&tl_msg, 0, sizeof(tl_msg));
  (void) memset(&raw_ts_data, 0, sizeof(raw_ts_data));

  /* fill in the required fields of an OTA CDMA SMS */
  ota_data.format    = WMS_FORMAT_CDMA;
  ota_data.data_len  = MIN(length, WMS_MAX_LEN);
  (void) memcpy((uint8 *)ota_data.data, (uint8 *)data, ota_data.data_len);

  /* Decode to CDMA tl and convert to cl to get to the client format */
  if((st = wms_ts_decode_CDMA_tl(&ota_data, &tl_msg, &raw_ts_data)) != WMS_OK_S)
  {
    LOGE("decode CDMA tl failed w/st = %d",st);
  }
  else
  {
    wms_ts_convert_tl2cl(&tl_msg,
                         (wms_cdma_message_s_type*)&cl_msg->u.cdma_message);
    cl_msg->u.cdma_message.raw_ts = raw_ts_data;
  }
  return st;
}
