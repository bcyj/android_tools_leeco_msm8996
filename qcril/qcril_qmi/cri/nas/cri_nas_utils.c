/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

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

#include "cri_nas_utils.h"
#include "cri_nas_core.h"
#include "cri_core.h"

static const int halfShift  = 10; /* used for shifting by 10 bits */

static const unsigned long halfBase = 0x0010000UL;
static const unsigned long halfMask = 0x3FFUL;

static const unsigned char firstByteMark[ 7 ] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

static int cri_nas_ConvertUTF16toUTF8
(
    const unsigned short ** sourceStart,
    const unsigned short * sourceEnd,
    unsigned char ** targetStart,
    unsigned char * targetEnd,
    int flags
);


#if 0
int nas_client_id;





void cri_nas_init_client_state();

qmi_error_type_v01 cri_nas_utils_init_client(hlos_ind_cb_type hlos_ind_cb)
    {
    qmi_error_type_v01 client_init_error;

    client_init_error = QMI_ERR_INTERNAL_V01;

    nas_client_id = cri_core_create_qmi_service_client(QMI_NAS_SERVICE, hlos_ind_cb);
    if(QMI_INTERNAL_ERR != nas_client_id)
    {
        client_init_error = QMI_ERR_NONE_V01;
        cri_nas_init_client_state();
    }

    return client_init_error;
}

void cri_nas_init_client_state()
{
    nas_set_event_report_req_msg_v01 nas_set_event_report_req_msg;
    nas_set_event_report_resp_msg_v01 nas_set_event_report_resp_msg;

    memset(&nas_set_event_report_req_msg, NIL, sizeof(nas_set_event_report_req_msg_v01));
    memset(&nas_set_event_report_resp_msg, NIL, sizeof(nas_set_event_report_resp_msg_v01));

    cri_core_qmi_send_msg_sync(nas_client_id,
                            QMI_NAS_SET_EVENT_REPORT_REQ_MSG_V01,
                            (void *)&nas_set_event_report_req_msg,
                            sizeof(nas_set_event_report_req_msg_v01),
                            &nas_set_event_report_resp_msg,
                            sizeof(nas_set_event_report_resp_msg_v01),
                            CRI_CORE_MINIMAL_TIMEOUT);

}

void cri_nas_utils_release_client(int qmi_service_client_id)
{
    cri_core_release_qmi_service_client(qmi_service_client_id);
    nas_client_id = NIL;
}


void cri_nas_utils_get_pref_mode(uint32_t *pref_mode)
{
    if ( pref_mode != NULL)
    {
        *pref_mode = current_pref_mode;
    }
}

#endif //0


int cri_nas_is_operator_name_empty_or_white_space ( char * str, int max_len)
{
  int is_empty_or_white_space = TRUE;
  int idx = 0;

  if ( str )
  {
    for ( idx = 0; idx < max_len && str[idx] ; idx++ )
    {
      if ( str[idx] != ' ' )
      {
        is_empty_or_white_space = FALSE;
        break;
      }
    }
  }

  return is_empty_or_white_space;
}


unsigned char cri_nas_ussd_pack(
    unsigned char *packed_data,
    const unsigned char *str,
    unsigned char num_chars
)
{
    unsigned char stridx=0;
    unsigned char pckidx=0;
    unsigned char shift;

    if(packed_data != NULL && str != NULL)
    {
        /* Loop through the 7-bit string till the last but one character.
        */
        while(stridx < (num_chars-1))
        {
            shift = stridx  & 0x07;

            /* A unsigned char of packed data is always made up of only 2 7-bit characters. The
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
        UTIL_LOG_MSG("FATAL : CHECK FAILED");
    }

    return pckidx;
}





unsigned char cri_nas_ussd_unpack
(
    unsigned char *str,
    const unsigned char *packed_data,
    unsigned char num_bytes
)
{

    unsigned char stridx = 0;
    unsigned char pckidx = 0;
    unsigned char prev = 0;
    unsigned char curr = 0;
    unsigned char shift;

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
        UTIL_LOG_MSG("FATAL : CHECK FAILED");
    }

    return stridx;

}



void cri_nas_ons_decode_packed_7bit_gsm_string
(
    char *dest,
    const unsigned char *src,
    unsigned char src_length
)
{
    unsigned char dest_length = 0;

    if(  dest != NULL && src != NULL )
    {
        dest_length = cri_nas_convert_gsm_def_alpha_string_to_utf8( ( const char * ) src, src_length, dest );

        /* Spare bits is set to '0' as documented in 3GPP TS24.008 Section 10.5.3.5a, and
        the CM util function unpacks it assuming USSD packing (packing for 7 spare bits is carriage return = 0x0D).
        Thus, an '@' is appended when there are 7 spare bits. So remove it. */
        if ( !( src_length % 7 ) && !( src[ src_length - 1 ] & 0xFE ) && ( dest[ dest_length - 1 ] == '@' ) )
        {
            UTIL_LOG_MSG( "Detected 7 spare bits in network name, removing trailing @\n");
            dest[ dest_length - 1 ] = '\0';
        }
    }
    else
    {
        UTIL_LOG_MSG("FATAL : CHECK FAILED\n");
    }

}




int cri_nas_convert_gsm_def_alpha_string_to_utf8
(
    const char *gsm_data,
    unsigned char gsm_data_len,
    char *utf8_buf
)
{
    int  num_chars;
    unsigned char *temp_buf;
    int ret_value = 0;

    do
    {
        if ( ( gsm_data == NULL ) || ( gsm_data_len == 0 ) || ( utf8_buf == NULL ) )
        {
            UTIL_LOG_MSG( "Invalid parameters for GSM alphabet to UTF8 conversion, input len = %d\n", gsm_data_len );
            break;
        }

        /* Allocate buffer */
        temp_buf = (unsigned char *) util_memory_alloc( gsm_data_len * 2 );
        if ( temp_buf == NULL )
        {
            UTIL_LOG_MSG( "Fail to allocate buffer for GSM alphabet to UTF8 conversion\n" );
            break;
        }

        /* Unpack the string from 7-bit format into 1 char per byte format */
        num_chars = cri_nas_ussd_unpack( temp_buf, (const unsigned char *) gsm_data, gsm_data_len );
        ret_value = cri_nas_convert_gsm8bit_alpha_string_to_utf8((const char *)temp_buf, num_chars, utf8_buf);
        free( temp_buf );

    }while(0);

    return ret_value;

}


int cri_nas_convert_ucs2_to_utf8
(
    char *ussData,
    int size,
    char *ussd_str
)
{
    unsigned char utf8_buf[ MAX_USS_CHAR * 2 ];
    unsigned short *utf16SourceStart, *utf16SourceEnd;
    unsigned char *utf8SourceStart, *utf8SourceEnd;
    int result;
    unsigned char length;

    utf8SourceStart = utf8_buf;
    utf8SourceEnd = &utf8_buf[ MAX_USS_CHAR * 2 ];

    utf16SourceStart = ( unsigned short * ) ussData;
    utf16SourceEnd = ( unsigned short * ) ( ussData + size );

    result = cri_nas_ConvertUTF16toUTF8( (const unsigned short **) &utf16SourceStart, utf16SourceEnd,
                                            &utf8SourceStart, utf8SourceEnd, 1 );

    if ( result != 0 )
    {
        UTIL_LOG_MSG( "Error in decoding the USSD response string" );
    }

    length = (unsigned int) ( utf8SourceStart - utf8_buf );

    memcpy( ussd_str, utf8_buf, length );
    ussd_str[ length ] = '\0';

    return length;

}



int cri_nas_ConvertUTF16toUTF8
(
    const unsigned short ** sourceStart,
    const unsigned short * sourceEnd,
    unsigned char ** targetStart,
    unsigned char * targetEnd,
    int flags
)
{
    int result = 0;
    const unsigned short * source = *sourceStart;
    unsigned char * target = *targetStart;

    while ( source < sourceEnd )
    {
        unsigned long ch;
        unsigned short bytesToWrite = 0;
        const unsigned long byteMask = 0xBF;
        const unsigned long byteMark = 0x80;
        const unsigned short

        * oldSource = source; /* In case we have to back up because of target overflow. */

        ch = *source++;

        /* If we have a surrogate pair, convert to unsigned long first. */
        if ( ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END )
        {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            if ( source < sourceEnd )
            {
                unsigned long ch2 = *source;
                /* If it's a low surrogate, convert to unsigned long. */
                if ( ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END )
                {
                ch = ( ( ch - UNI_SUR_HIGH_START ) << halfShift ) + ( ch2 - UNI_SUR_LOW_START ) + halfBase;
                ++source;
                }
                /* it's an unpaired high surrogate */
                else if ( flags == 0 )
                {
                --source; /* return to the illegal value itself */
                result = 3;
                break;
                }
            }
            /* We don't have the 16 bits following the high surrogate. */
            else
            {
            --source; /* return to the high surrogate */
            result = 2;
            break;
            }
        }
        else if ( flags == 0 )
        {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if ( ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END )
            {
            --source; /* return to the illegal value itself */
            result = 3;
            break;
            }
        }

        /* Figure out how many bytes the result will require */
        if ( ch < ( unsigned long ) 0x80 )
        {
        bytesToWrite = 1;
        }
        else if ( ch < ( unsigned long )0x800 )
        {
        bytesToWrite = 2;
        }
        else if ( ch < ( unsigned long ) 0x10000 )
        {
        bytesToWrite = 3;
        }
        else if (ch < ( unsigned long ) 0x110000 )
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
        target -= bytesToWrite; result = 2;
        break;
        }

        switch (bytesToWrite)
        {
        /* note: everything falls through. */
        case 4: *--target = ( unsigned char ) ( ( ch | byteMark ) & byteMask ); ch >>= 6;
        case 3: *--target = ( unsigned char ) ( ( ch | byteMark ) & byteMask ); ch >>= 6;
        case 2: *--target = ( unsigned char ) ( ( ch | byteMark ) & byteMask ); ch >>= 6;
        case 1: *--target = ( unsigned char ) ( ch | firstByteMark[ bytesToWrite ] );
        }

        target += bytesToWrite;

    }

    *sourceStart = source;
    *targetStart = target;

    return result;

}





int cri_nas_convert_gsm8bit_alpha_string_to_utf8
(
    const char *gsm_data,
    int gsm_data_len,
    char *utf8_buf
)
{
    int i, j;
    int hi_utf8, lo_utf8;
    int unicode;
    int ret_value = 0;

    unsigned int gsm_char;

    do
    {
        if ( ( gsm_data == NULL ) || ( gsm_data_len == 0 ) || ( utf8_buf == NULL ) )
        {
            UTIL_LOG_MSG( "Invalid parameters for GSM alphabet to UTF8 conversion, input len = %d", gsm_data_len );
            break;
        }

        for ( i = 0, j = 0; i < gsm_data_len; i++ )
        {
            if(gsm_data[i] == 0x0D)
            {
                /* ignoring the non-standard representation of new line (carriage return charecter is also included along with new line)*/
                UTIL_LOG_MSG( "ignored CR charecter at index = %d\n", i);
                continue;
            }

            gsm_char = (unsigned int)gsm_data[ i ];
            if ( gsm_char <= 127 )
            {
                unicode = gsm_def_alpha_to_utf8_table[ gsm_char ];
                hi_utf8 = ( int ) ( ( unicode & 0xFF00 ) >> 8 );
                lo_utf8 = ( int ) unicode & 0x00FF;
                if ( hi_utf8 > 0 )
                {
                utf8_buf[ j++ ] = hi_utf8;
                }

                utf8_buf[ j++ ] = lo_utf8;
            } // if ( gsm_char <= 127 )
            else
            {
                utf8_buf[ j++ ] = (char) gsm_char;
            }
        }

        utf8_buf[ j ] = '\0';

        ret_value =  j;
    } while(0);

    return ret_value;

}



void cri_nas_decode_operator_name_in_little_endian
(
    char *dest,
    unsigned short max_dest_length,
    int coding_scheme,
    const unsigned char *src,
    unsigned short src_length
)
{
    unsigned char data_length;

    if ( dest!= NULL && src != NULL && src_length > NIL )
    {
        data_length = ( src_length > max_dest_length ) ? max_dest_length : src_length;

        switch ( coding_scheme )
        {
            case QMI_CODING_SCHEME_CELL_BROADCAST_DATA:
                UTIL_LOG_MSG( "7-bit coding scheme for NITZ ONS\n" );
                cri_nas_ons_decode_packed_7bit_gsm_string( dest, src, data_length );
                UTIL_LOG_MSG( "NITZ 7-bit GSM str: %s\n", dest );
            break;

            case QMI_CODING_SCHEME_UCS2:
                UTIL_LOG_MSG( "UC2 coding scheme for NITZ ONS, len %d\n", data_length );
                if ( ( data_length % 2 ) != 0 )
                {
                UTIL_LOG_MSG( "Invalid UCS length %d\n", data_length );
                break;
                }
                (void) cri_nas_convert_ucs2_to_utf8( (char *)src, data_length, dest );
                UTIL_LOG_MSG( "NITZ UCS str: %s\n", dest );
            break;

            default:
                UTIL_LOG_MSG( "Unknown coding scheme %d for NITZ ONS\n", coding_scheme );
            break;
        }
    }
    else
    {
        UTIL_LOG_MSG("cri_nas_decode_operator_name_in_little_endian CHECK FAILED\n");
    }

} // qcril_qmi_util_decode_operator_name_in_little_endian

