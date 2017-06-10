/*===========================================================================

                    Q M I _ I D L _ L I B . C

  Library for encoding and decoding QMI messages using IDL compiler output
  for defining the message format.

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/
#include <stdint.h>
#include <string.h>

#include "qmi_idl_lib.h"
#include "qmi_idl_lib_internal.h"
#include "qmi_idl_lib_target.h"

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#define QMI_IDL_RESP_TLV_LEN 7

#define QMI_IDL_FLAGS_MAX_SZ_IS_16 ( QMI_IDL_FLAGS_SZ_IS_16 | (QMI_IDL_FLAGS_SZ_IS_32 << 8) )


/***********************************************************************

MACRO: GET_8_OR_16

Description
  Gets 8-bits or 16-bits from a pointer based on a flag advancing
  the pointer appropriately. If the flag is true, gets 16-bits otherwise
  gets 8-bits.

***********************************************************************/
#define GET_8_OR_16( VAR, FLAG, PTR ) \
  do { \
    VAR = *(PTR)++; \
    if( FLAG ) { \
      VAR |= (*(PTR)++) << 8; \
    } \
  } while( 0 )

/***********************************************************************

MACRO: GET_16

Description
  Gets 16-bits from a pointer advancing the pointer appropriately.

***********************************************************************/
#define GET_16( VAR, PTR ) \
  do { \
    VAR = *(PTR)++; \
    VAR |= (*(PTR)++) << 8; \
  } while( 0 )

/***********************************************************************

MACRO: BYTE0, BYTE1

Description
  BYTE0 gets first byte of value (low order byte)
  BYTE1 gets second byte of value

***********************************************************************/
#define BYTE0( VAR ) ((uint8_t)((VAR) & 0xFF))
#define BYTE1( VAR ) ((uint8_t)(((VAR) >> 8) & 0xFF))

/***********************************************************************

MACRO: qmi_idl_print_element_info

Description
  Macro to include/exclude call to debug routine.

***********************************************************************/
#if 0
extern void qmi_idl_print_element_info( char *msg, const uint8_t *p_element );
#else
#define qmi_idl_print_element_info( MSG, P_ELEMENT ) do { } while( 0 )
#endif

/***********************************************************************

MACRO: QMI_IDL_CHECK_UNDEROVERFLOW

Description
  Macro to check for buffer under and overflows

***********************************************************************/
#define QMI_IDL_BUF_RANGE(START,END,BUF,LEN,EXC) if (((START)>(BUF)) || ((END)<((BUF)+(LEN))))\
{ QMI_IDL_HANDLE_ERROR(EXC, QMI_IDL_LIB_LENGTH_INCONSISTENCY,0,0,0); }

/***********************************************************************
A type element is a variable length byte stream that may contain the
following fields:

Flags/type byte - always present
Offset - 1 or 2 bytes - always present
Array size - 1 or 2 bytes - present for arrays, max number of elements
    for strings, max number of chars excluding terminating '\0'
Size offset - 1 byte - present for variable length data structures
Aggregate type - 2 bytes - present for aggregate types

Definition of Flags/type byte:
Flags:  5 bits
  Definition of flag bits:

    OFFSET_IS_16: Indicates size of "Offset" field (2 bytes if set,
        otherwise 1 byte)
    IS_ARRAY:  Indicates array with multiple elements ("Array size" field
        is present)
    SZ_IS_16:  Indicates "Array size" field is 2 bytes if set (otherwise 1
        byte)
    VARIABLE_LENGTH:  Indicates variable length array ("Array size" is
        maximum, actual size from wire stream)
    Type: 3 bits, enumeration with the following values:
        GENERIC_1_BYTE,
        GENERIC_2_BYTE,
        GENERIC_4_BYTE,
        GENERIC_8_BYTE,
        _1_BYTE_ENUM,
        _2_BYTE_ENUM,
        STRING,
        AGGREGATE:  Indicates "Aggregate type" field is present
***********************************************************************/

/*===========================================================================
        Local functions
===========================================================================*/

/***********************************************************************

FUNCTION qmi_idl_next_element

DESCRIPTION
  Function to skip over a type element to allow linear searches for TLV
  decoding data at the top level

ARGUMENTS p_element - pointer to start of type element

RETURN VALUE pointer to start of next type element

***********************************************************************/
static const uint8_t *qmi_idl_next_element(const uint8_t *p_element)
{
  uint32_t flags = *p_element++;

  if ( flags & QMI_IDL_FLAGS_FIRST_EXTENDED) {
    flags |= *p_element++ << 8;
  }
  if( flags & (QMI_IDL_FLAGS_SECOND_EXTENDED << 8))
  {
    flags |= *p_element++ << 16;
  }

  if (QMI_IDL_FLAG_END_VALUE == flags) {
    return p_element;
  }

  ++p_element; /* Offset is always present, at least 8 bits */

  if( flags & QMI_IDL_FLAGS_OFFSET_IS_16 ) {
    ++p_element;
  } else if( flags & (QMI_IDL_FLAGS_EXTENDED_OFFSET << 8) ) {
    p_element+=2;
  }

  if( flags & QMI_IDL_FLAGS_IS_ARRAY ) {
    ++p_element;
  }

  if( flags & QMI_IDL_FLAGS_MAX_SZ_IS_16 ) {
    ++p_element;
  }

  if( flags & (QMI_IDL_FLAGS_SZ_IS_32 << 8) ) {
    p_element += 2;
  }

  if( (flags & QMI_IDL_FLAGS_IS_VARIABLE_LEN) &&
      ((flags & QMI_IDL_FLAGS_TYPE) != QMI_IDL_STRING) &&
      !(flags & (QMI_IDL_FLAGS_UTF16_STRING << 8)) ) {
    ++p_element;
  }

  if( (flags & QMI_IDL_FLAGS_TYPE) == QMI_IDL_AGGREGATE ) {
    p_element += 2;
  }
  if (flags & (QMI_IDL_FLAGS_RANGE_CHECKED << 16))
  {
    p_element+=4;
  }

  return p_element;
} /* qmi_idl_next_element */

/***********************************************************************

FUNCTION qmi_idl_find_msg

DESCRIPTION
  Search list of messages for message with matching message ID.

ARGUMENTS p_service - pointer to service object
          message_type - Request/Response/Indication (different list for
              each message type.
          message_id - ID to search for
          out_type_table - Output param,Pointer to referenced type table object
                           in case the message is defined in the included IDL.
                           Pointer to local type table object in case the
                           message defined is local.

RETURN VALUE pointer to start of message table entry

***********************************************************************/
const qmi_idl_message_table_entry *qmi_idl_find_msg
(
  const qmi_idl_service_object_type p_service,
  qmi_idl_type_of_message_type message_type,
  uint16_t message_id,
  qmi_idl_lib_exception_type *exc,
  const qmi_idl_type_table_object **out_type_table
)
{
  const qmi_idl_service_message_table_entry *p_srvmsg = NULL;
  const qmi_idl_message_table_entry *p_msg;
  qmi_idl_service_object_type inheritance_obj;
  uint32_t table_index;
  uint32_t type_index;
  uint32_t i = 0;
  int imin,imax,imid;

  if (!p_service)
  {
    QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_PARAMETER_ERROR, 0, 0, 0 );
  }
  inheritance_obj = p_service;
  while (inheritance_obj)
  {
    imin=0;
    /* Binary search for the table entry for the message ID */
    p_srvmsg = inheritance_obj->msgid_to_msg[message_type];
    imax = inheritance_obj->n_msgs[message_type] - 1;
    while (imax >= imin)
    {
      imid = (imax+imin) >> 1;
      if (p_srvmsg[imid].qmi_message_id < message_id)
      {
        imin = imid +1;
      }else if(p_srvmsg[imid].qmi_message_id > message_id)
      {
        imax = imid - 1;
      }else
      {
        i=1;
        p_srvmsg = &p_srvmsg[imid];
        break;
      }
    }
    if (i != 0)
    {
      break;
    }
    inheritance_obj = qmi_idl_get_inherited_service_object(inheritance_obj);
  }
  if (0 == i)
  {
    QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_MESSAGE_ID_NOT_FOUND,
                          message_id, 0, 0 );
  }

  /* Found the service message table entry for this message ID.  Now
   * get the table and type index */
  table_index = QMI_IDL_MSG_TYPE_TABLE(p_srvmsg->message_table_message_id);
  type_index = QMI_IDL_MSG_TYPE_TYPE(p_srvmsg->message_table_message_id);
  if (out_type_table)
  {
    *out_type_table = inheritance_obj->p_type_table->
                      p_referenced_tables[table_index];
  }
  /* Use the table and type index to get the message table entry with
   * the actual data needed for decoding */
  p_msg = &(inheritance_obj->p_type_table->p_referenced_tables[table_index]->
            p_messages[type_index]);

return p_msg;
} /* qmi_idl_find_msg */

/***********************************************************************

FUNCTION qmi_idl_decode_find_tlv

DESCRIPTION
  Search list of TLVs for a message for a particular TLV

ARGUMENTS p_element - pointer to list of TLVs (elements)
          msg_type - T of TLV to search for
          optional_offset - output
              If 0,TLV mandatory
              If > 0 TLV optional, value is offset to valid flag

RETURN VALUE pointer to start of matching element or NULL if not found

***********************************************************************/
const uint8_t *qmi_idl_decode_find_tlv
(
  const uint8_t *p_element,
  uint8_t msg_type,
  int32_t *optional_offset
)
{
  uint8_t tlv_flags;
  uint8_t msg_type2;    /* "T" value from message element data stream */

  if (!p_element)
  {
    return NULL;
  }
  /* Walk the entire message element data stream until we find a
   * matching T value (msg_type2) */
  for (;;) {
    tlv_flags = *p_element++;

    /* Get the message element data stream "T" value */
    if( QMI_IDL_TLV_FLAGS_OPTIONAL & tlv_flags ) {
      msg_type2 = *p_element++;
      if( msg_type == msg_type2 ) {
        *optional_offset = QMI_IDL_TLV_FLAGS_OFFSET_FOR_OPTIONAL_TLV & tlv_flags;
        return p_element;
      }
    }
    else {
      msg_type2 = QMI_IDL_TLV_FLAGS_TLV_TYPE_FOR_REQUIRED_TLV & tlv_flags;
      if( msg_type == msg_type2 ) {
        *optional_offset = 0;
        return p_element;
      }
    }

    if (QMI_IDL_TLV_FLAGS_LAST_TLV & tlv_flags ) {
      /* We've gone through all the TLV's we know about and didn't
       * find a match. */
      return NULL;
    }

    p_element = qmi_idl_next_element(p_element);
  }
} /* qmi_idl_decode_find_tlv */

/***********************************************************************

FUNCTION qmi_idl_decode_verify_mandatory

DESCRIPTION
  Check to see if all of the mandatory TLVs were present

ARGUMENTS p_element - pointer to list of TLVs (elements)
          found - array of booleans indicating which TLVs were decoded
          exc - longjmp buffer for exception handling

RETURN VALUE NONE

***********************************************************************/
void qmi_idl_decode_verify_mandatory
(
  const uint8_t *p_element,
  uint8_t *found,
  qmi_idl_lib_exception_type *exc
)
{
  uint8_t tlv_flags;
  uint8_t msg_type;

  if(p_element == NULL)
    return;

  /* Walk the encoded message data and verify that all required TLV's
   * were found */
  for (;;) {
    tlv_flags = *p_element++;

    if (! (QMI_IDL_TLV_FLAGS_OPTIONAL & tlv_flags) ) {
      msg_type = QMI_IDL_TLV_FLAGS_TLV_TYPE_FOR_REQUIRED_TLV & tlv_flags;
      if (! found[msg_type]) {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_MISSING_TLV, msg_type, 0, 0 );
      }
    }
    else {
      /* Found an optional - no more mandatories */
      break;
    }

    if (QMI_IDL_TLV_FLAGS_LAST_TLV & tlv_flags ) {
      break;
    }

    p_element = qmi_idl_next_element(p_element);
  }
} /* qmi_idl_decode_verify_mandatory */

/***********************************************************************

FUNCTION qmi_idl_encode_length

DESCRIPTION
  Encode length into the destination buffer

ARGUMENTS see comments inline

RETURN VALUE number of bytes remaining in destination buffer

***********************************************************************/
static uint32_t qmi_idl_encode_length
(
  /* Destination buffer */
  uint8_t **pp_dst,

  /*  Size of destination buffer */
  uint32_t dst_bytes,

  /* element flags */
  uint32_t flags,

  /* array length to be encoded */
  uint32_t length,

  /* For exception handling */
  qmi_idl_lib_exception_type *exc
)
{
  /* Send the number of items */
  if( !(flags & ( QMI_IDL_FLAGS_SZ_IS_16 | (QMI_IDL_FLAGS_SZ_IS_32 << 8))) ) {
    if( dst_bytes == 0 ) {
      QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY, 11, 0, 0 );
    }

    **pp_dst = length & 0xFF;
    (*pp_dst)++;
    dst_bytes--;
  }
  else if( flags & QMI_IDL_FLAGS_SZ_IS_16 ) {
    if( dst_bytes < 2 ) {
      QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY, 12, 0, 0 );
    }

    **pp_dst = length & 0xFF;
    (*pp_dst)++;
    **pp_dst = (length >> 8) & 0xFF;
    (*pp_dst)++;
    dst_bytes -= 2;
  }
  else {
    if( dst_bytes < 4 ) {
      QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY, 13, 0, 0 );
    }

    **pp_dst = length & 0xFF;
    (*pp_dst)++;
    **pp_dst = (length >> 8) & 0xFF;
    (*pp_dst)++;
    **pp_dst = 0;
    (*pp_dst)++;
    **pp_dst = 0;
    (*pp_dst)++;
    dst_bytes -= 4;
  }

  return dst_bytes;
} /* qmi_idl_encode_length */

/***********************************************************************
FUNCTION      qmi_idl_decode_element

DESCRIPTION   Local function that decodes a TLV or (recursively) an element
              within a nested aggregate type

ARGUMENTS     See inline comments

RETURN VALUE  Returns the number of bytes remaining in the destination buffer

SIDE EFFECTS  None
***********************************************************************/
static uint32_t qmi_idl_decode_element(
  /* Pointer to the beginning of the C-struct containing the element in
     question */
  uint8_t *p_dst,

  /* Pointer to the location of the element in the wire stream */
  const uint8_t **pp_src,

  /*  Size of source buffer */
  uint32_t src_len,

  /* If -1, not top level
   * If 0, top level - TLV mandatory
   * If > 0, top level - TLV optional, value is offset to valid flag */
  int32_t optional_offset,

  /* Pointer to element description stream */
  const uint8_t **pp_element,

  /* Type table object that type values are relative to */
  const qmi_idl_type_table_object *p_type_table,

  /* For exception handling */
  qmi_idl_lib_exception_type *exc,

  /* For Range Checking */
  uint8_t *range_error_ignore,

  uint8_t *p_dst_start,

  uint8_t *p_dst_end
)
{
  const uint8_t *p_element = *pp_element;
  const uint8_t *p_element2;
  uint8_t  *valid_flag = NULL;
  uint32_t flags;
  uint32_t array_max_size;
  uint32_t length;
  uint32_t i;
  uint32_t ag_type;
  uint32_t table_index;
  uint32_t type_index;
  uint32_t c_struct_sz;
  uint32_t range_index = 0;

  qmi_idl_print_element_info( "qmi_idl_decode_element", p_element );

  /* decode p_element information - when done we have:
   * flags = flag byte of p_element info
   * length = number of items to copy
   * p_dst advanced to the start of the data field
   * the size field updated if required
   * the valid field, if any, has been updated
   * the number of array items validated
   */
  flags = *p_element++;
  if( flags & QMI_IDL_FLAGS_FIRST_EXTENDED) {
    flags |= *p_element++ << 8;
  }
  if( flags & (QMI_IDL_FLAGS_SECOND_EXTENDED << 8))
  {
    flags |= *p_element++ << 16;
  }

  /* Update C struct pointer to point to element to decode */
  p_dst += *p_element++;
  if( flags & QMI_IDL_FLAGS_OFFSET_IS_16 ) {
    p_dst += (uint8_t)(*p_element++) << 8;
  } else if(flags & (QMI_IDL_FLAGS_EXTENDED_OFFSET << 8)) {
    p_dst += (uint8_t)(*p_element++) << 8;
    p_dst += (uint8_t)(*p_element++) << 16;
  }

  /* TODO - possible optimization is to have multiple entries to
   * decode element - one for toplevel and one for recursion. Maybe
   * even one for toplevel mandatory, one for toplevel optional and one
   * for recursion. */
  if( optional_offset > 0 ) {
    /* Test for buffer under/overflow */
    /* If optional_offset is > 0 p_dst should have already been incremented */
    QMI_IDL_BUF_RANGE(p_dst_start,p_dst_end,(p_dst - optional_offset),0,exc);
    *(uint8_t *)(p_dst - optional_offset) = TRUE;
    valid_flag = (p_dst - optional_offset);
  }

  if( flags & QMI_IDL_FLAGS_IS_ARRAY ) {
    GET_8_OR_16( array_max_size, (flags & QMI_IDL_FLAGS_MAX_SZ_IS_16), p_element );
    if (flags & (QMI_IDL_FLAGS_SZ_IS_32 << 8)) {
      p_element += 2;
    }
    if( flags & QMI_IDL_FLAGS_IS_VARIABLE_LEN ) {
      if( optional_offset != -1 && (flags & QMI_IDL_FLAGS_TYPE) == QMI_IDL_STRING ) {
        /* Top level and string - length, in bytes, is src_len */
        length = src_len;
        /* TODO - adjust this based on the size of items */
      }
      else {
        /* Not top level or not string - length, in items, in input stream */
        if (!(flags & (QMI_IDL_FLAGS_SZ_IS_16 | (QMI_IDL_FLAGS_SZ_IS_32 << 8) |
                      (QMI_IDL_FLAGS_ARRAY_DATA_ONLY << 8) | (QMI_IDL_FLAGS_ARRAY_IS_LENGTHLESS << 8)))) {
          if (src_len < 1 ) {
            QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                                  14, 0, 0 );
          }
          length = *(*pp_src)++;
          src_len--;
        }
        else if (!(flags & ((QMI_IDL_FLAGS_SZ_IS_32 << 8) | (QMI_IDL_FLAGS_ARRAY_DATA_ONLY << 8)
                             | (QMI_IDL_FLAGS_ARRAY_IS_LENGTHLESS << 8)))) {
          if (src_len < 2 ) {
            QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                                  14, 0, 0 );
          }
          length = *(*pp_src)++;
          length |=  (*(*pp_src)++) << 8;
          src_len -= 2;
        }
        else if (!(flags & ((QMI_IDL_FLAGS_ARRAY_DATA_ONLY << 8) | (QMI_IDL_FLAGS_ARRAY_IS_LENGTHLESS << 8)))){
          if (src_len < 4 ) {
            QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                                  14, 0, 0 );
          }
          length = *(*pp_src)++;
          length |=  (*(*pp_src)++) << 8;
          *pp_src += 2;
          src_len -= 4;
        }
        else if  (!(flags & (QMI_IDL_FLAGS_ARRAY_IS_LENGTHLESS << 8))) {
          length = *(uint32_t *)(p_dst - *p_element);
        }
        else
        {
          length = src_len;
          switch( flags & QMI_IDL_FLAGS_TYPE ) {
            case QMI_IDL_GENERIC_1_BYTE:
            case QMI_IDL_GENERIC_2_BYTE:
            case QMI_IDL_GENERIC_4_BYTE:
            case QMI_IDL_GENERIC_8_BYTE:
              length = length >> (flags & QMI_IDL_FLAGS_TYPE);
              break;
            case QMI_IDL_2_BYTE_ENUM:
              length = length >> 1;
              break;
          }
        }

      }

      if( ((flags & QMI_IDL_FLAGS_TYPE) != QMI_IDL_STRING) && !(flags & (QMI_IDL_FLAGS_UTF16_STRING << 8))) {
        /* Test for buffer under/overflow */
        QMI_IDL_BUF_RANGE(p_dst_start,p_dst_end,(p_dst - *p_element),4,exc);
        *(uint32_t *)(p_dst - *p_element++) = length;
      }

      if( length > array_max_size ) {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_ARRAY_TOO_BIG,
                              length, array_max_size, 0 );
      }

      if (flags & (QMI_IDL_FLAGS_ARRAY_LENGTH_ONLY << 8)) {
        if ((flags & QMI_IDL_FLAGS_TYPE) == QMI_IDL_AGGREGATE) {
          p_element += 2;
        }
        *pp_element = p_element;
        return src_len;
      }
    }
    else {
      length = array_max_size;
    }
  }
  else {
    length = 1;
  }
  /* Range Checking */
  if (flags & (QMI_IDL_FLAGS_RANGE_CHECKED << 16))
  {
    range_index =  (*p_element++);
    range_index |= (*p_element++) << 8;
    range_index |= (*p_element++) << 16;
    range_index |= (*p_element++) << 24;
  }

  /* Done parsing element, set return value to start of next element */
  *pp_element = p_element;

  switch( flags & QMI_IDL_FLAGS_TYPE ) {
    case QMI_IDL_GENERIC_1_BYTE:
    case QMI_IDL_GENERIC_2_BYTE:
    case QMI_IDL_GENERIC_4_BYTE:
    case QMI_IDL_GENERIC_8_BYTE:
      /* Handle simple primitive types */
      /* convert length in items to bytes - depends on item size == 2 ^ type */
      length = length << (flags & QMI_IDL_FLAGS_TYPE);

      if( length > src_len ) {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                              3, length, src_len );
      }
      src_len -= length;
      /* Test for buffer under/overflow */
      QMI_IDL_BUF_RANGE(p_dst_start,p_dst_end,p_dst,length,exc);
      memcpy( p_dst, *pp_src, length );
      /* Range Checking */
      if (flags & (QMI_IDL_FLAGS_RANGE_CHECKED << 16))
      {
        int8_t ret_val = 0;
        uint64_t tmp_val = 0;
        length = length >> (flags & QMI_IDL_FLAGS_TYPE);
        for(i=0; i<length; i++)
        {
          tmp_val = 0;
          /* Test for buffer under/overflow */
          if ((flags & QMI_IDL_FLAGS_TYPE) > 3)
          {
            QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                                  0, 0, 0 );
          }
          memcpy( &tmp_val, *pp_src, (1 << (flags & QMI_IDL_FLAGS_TYPE)));
          ret_val = p_type_table->p_ranges[range_index].range_check(&tmp_val);
          if (ret_val == QMI_IDL_RANGE_RESPONSE_IGNORE)
          {
            if( optional_offset > 0 )
            {
              *valid_flag = FALSE;
            }else
            {
              *range_error_ignore = TRUE;
            }
          }else if(ret_val > 0)
          {
            QMI_IDL_HANDLE_ERROR( exc, ret_val, 0, 0, 0);
          }else if(ret_val == QMI_IDL_RANGE_RESPONSE_DEFAULT)
          {
            uint64_t offset = i*(1 << (flags & QMI_IDL_FLAGS_TYPE));
            /* Test for buffer under/overflow */
            QMI_IDL_BUF_RANGE(p_dst_start,p_dst_end,(p_dst+offset),(1 << (flags & QMI_IDL_FLAGS_TYPE)),exc);
            memcpy(p_dst+offset,&tmp_val,(1 << (flags & QMI_IDL_FLAGS_TYPE)));
          }
          *pp_src += (1 << (flags & QMI_IDL_FLAGS_TYPE));
        }
      }else
      {
      *pp_src = *pp_src + length;
      }
      if (flags & (QMI_IDL_FLAGS_UTF16_STRING << 8))
      {
        /* Test for buffer under/overflow */
        QMI_IDL_BUF_RANGE(p_dst_start,p_dst_end,p_dst,length,exc);
        *(p_dst + length) = 0;
      }
      break;

    case QMI_IDL_1_BYTE_ENUM:
      if( length > src_len )
      {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY, 0, 0, 0 );
      }
      src_len -= length;
      /* Test for buffer under/overflow */
      /* Length is never more than 64k (only read 2 bytes to fill out length)
         so length*4 will never overflow */
      QMI_IDL_BUF_RANGE(p_dst_start,p_dst_end,p_dst,(length*4),exc);
      if (flags & (QMI_IDL_FLAGS_ENUM_IS_UNSIGNED << 8))
      {
        for( i = 0; i < length; i++ )
        {
          *(int32_t *)p_dst = (int32_t)(uint8_t) *(*pp_src)++;
          p_dst += 4;
        }
      }else
      {
        for( i = 0; i < length; i++ )
        {
          *(int32_t *)p_dst = (int32_t)(int8_t) *(*pp_src)++;
          p_dst += 4;
        }
      }
      if (flags & (QMI_IDL_FLAGS_RANGE_CHECKED << 16))
      {
        int8_t ret_val = 0;
        int32_t tmp_val = 0;
        p_dst -= 4*length;
        for(i=0; i<length; i++)
        {
          tmp_val = 0;
          memcpy( &tmp_val, p_dst, 4);
          ret_val = p_type_table->p_ranges[range_index].range_check(&tmp_val);
          if (ret_val == QMI_IDL_RANGE_RESPONSE_IGNORE)
          {
            if( optional_offset > 0 )
            {
              *valid_flag = FALSE;
            }else
            {
              *range_error_ignore = TRUE;
            }
          }else if(ret_val > 0)
          {
            QMI_IDL_HANDLE_ERROR( exc, ret_val, 0, 0, 0);
          }else if(ret_val == QMI_IDL_RANGE_RESPONSE_DEFAULT)
          {
            uint64_t offset = i*4;
            memcpy(p_dst+offset,&tmp_val,4);
          }

          p_dst += 4;
        }
      }
      break;

    case QMI_IDL_2_BYTE_ENUM:
      if( 2*length > src_len ) {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                              5, length, src_len );
      }
      src_len -= 2*length;
      /* Test for buffer under/overflow */
      /* Length is never more than 64k (only read 2 bytes to fill out length)
         so length*4 will never overflow */
      QMI_IDL_BUF_RANGE(p_dst_start,p_dst_end,p_dst,(length*4),exc);
      if (flags & (QMI_IDL_FLAGS_ENUM_IS_UNSIGNED << 8))
      {
        for( i = 0; i < length; i++ )
        {
          *(int32_t *)p_dst =
            (int32_t) (uint16_t) ( (uint16_t) **pp_src |
                                  (((uint16_t) (*(*pp_src + 1))) << 8) );
          p_dst += 4;
          *pp_src += 2;
        }
      }
      else
      {
        for( i = 0; i < length; i++ )
        {
          *(int32_t *)p_dst =
            (int32_t) (int16_t) ( (uint16_t) **pp_src |
                                (((uint16_t) (*(*pp_src + 1))) << 8) );
          p_dst += 4;
          *pp_src += 2;
        }
      }
      if (flags & (QMI_IDL_FLAGS_RANGE_CHECKED << 16))
      {
        int8_t ret_val = 0;
        int32_t tmp_val = 0;
        p_dst -= 4*length;
        for(i=0; i<length; i++)
        {
          tmp_val = 0;
          memcpy( &tmp_val, p_dst, 4);
          ret_val = p_type_table->p_ranges[range_index].range_check(&tmp_val);
          if (ret_val == QMI_IDL_RANGE_RESPONSE_IGNORE)
          {
            if( optional_offset > 0 )
            {
              *valid_flag = FALSE;
            }else
            {
              *range_error_ignore = TRUE;
            }
          }else if(ret_val > 0)
          {
            QMI_IDL_HANDLE_ERROR( exc, ret_val, 0, 0, 0);
          }else if(ret_val == QMI_IDL_RANGE_RESPONSE_DEFAULT)
          {
            uint64_t offset = i*4;
            memcpy(p_dst+offset,&tmp_val,4);
          }
          p_dst += 4;
        }
      }
      break;

  case QMI_IDL_STRING:
      /* Handle string type */
      /* length in bytes = length in items - assumes 1-byte chars */
      if( length > src_len ) {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                              6, length, src_len );
      }
      src_len -= length;
      /* Test for buffer under/overflow */
      QMI_IDL_BUF_RANGE(p_dst_start,p_dst_end,p_dst,length,exc);
      memcpy( p_dst, *pp_src, length );
      *(p_dst + length) = 0;
      *pp_src += length;
      break;

    case QMI_IDL_AGGREGATE:
      /* Handle aggregate type
       * use pp_element because it's already been updated */
      GET_16( ag_type, *pp_element );
      table_index = QMI_IDL_TYPE_TABLE( ag_type );
      p_type_table = p_type_table->p_referenced_tables[table_index];
      type_index = QMI_IDL_TYPE_TYPE( ag_type );
      p_element = p_type_table->p_types[type_index].p_encoded_type_data;
      c_struct_sz = p_type_table->p_types[type_index].c_struct_sz;
      for( i = 0; i < length; i++ ) {
        p_element2 = p_element;

        do {
          src_len = qmi_idl_decode_element( p_dst,
                                            pp_src,
                                            src_len,
                                            -1,
                                            &p_element2,
                                            p_type_table,
                                            exc,
                                            range_error_ignore,
                                            p_dst_start,
                                            p_dst_end );
          if (*range_error_ignore && valid_flag)
          {
            *valid_flag = FALSE;
          }
        } while( *p_element2 != QMI_IDL_FLAG_END_VALUE );

        p_dst += c_struct_sz;
      }
      break;
  }

  return src_len;
} /* qmi_idl_decode_element */

/***********************************************************************

FUNCTION qmi_idl_message_decode_v6

DESCRIPTION
  Decodes a QMI message body from the wire format to the C structure
  based on verison #1 or #2  of the database.

ARGUMENTS     See inline comments

RETURN VALUE  None, on error - does longjump, does not return

***********************************************************************/
void qmi_idl_message_decode_v6
(
  /* Pointer to service object, from auto-generated header from IDL */
  const qmi_idl_service_object_type p_service,

  /* See enumeration above */
  qmi_idl_type_of_message_type req_resp_ind,

  /* Message ID from IDL */
  uint16_t message_id,

  /* Pointer to BEGINNING OF FIRST TLV IN MESSAGE */
  const void *p_src_void,

  /* Length field from QMI header */
  uint32_t src_len,

  /* Pointer to C structure for decoded data */
  void *p_dst_void,

  /* Size of above structure, for sanity check */
  uint32_t dst_len,

  /* For exception handling */
  qmi_idl_lib_exception_type *exc
)
{
  /* Pointer to the message table entry for this message */
  const qmi_idl_message_table_entry *p_msg;
  /* Pointer in case message is defined in included IDL*/
  const qmi_idl_type_table_object *msg_type_table;
  /* Array of flags for which TLV's have been found */
  uint8_t found[256];
  /* Pointer to message format data stream */
  const uint8_t *p_element;
  /* Change type of arg from void pointer to byte pointer */
  const uint8_t *p_src = p_src_void;
  uint8_t *p_dst = p_dst_void;
  uint8_t *p_dst_start,*p_dst_end;

  /* T & L of TLV */
  uint8_t msg_type;
  uint32_t msg_len;
  /* resp message error TLV reports an error */
  uint8_t resp_error = 0;
  uint8_t range_err = FALSE;
  uint32_t bytes_remaining;
  int32_t optional_offset;  /* Relative offset for the optional flag */

  /* Start with no TLV's found */
  memset(found, 0, 256);

  p_msg = qmi_idl_find_msg( p_service, req_resp_ind, message_id, exc,
                            &msg_type_table );

  /* Validate that the decode buffer is large enough
     it is valid for dst_len to be greater than c_struct_sz */
  if (dst_len < p_msg->c_struct_sz) {
    QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_BUFFER_TOO_SMALL,
                          1, dst_len, p_msg->c_struct_sz );
  }
  /* Set start & end ranges for p_dst */
  p_dst_start = p_dst;
  p_dst_end = p_dst_start + p_msg->c_struct_sz;
  /* Initially zero the message C-struct so that valid flags will have
   * reasonable values */
  if (p_dst)
    memset(p_dst, 0, p_msg->c_struct_sz);

  /* Loop over all the TLV's found in the encoded message. */
  while (src_len) {
    if (src_len < 3) {
      QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                            13, 3, src_len );
    }

    msg_type  = *p_src++;
    GET_16(msg_len, p_src);
    src_len -= 3;

    /* If the length of a TLV exceeds the number of bytes remaining in
     * the whole message, it's a bad message */
    if (msg_len > src_len) {
      QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                            14, msg_len, src_len );
    }

    /* Find index into p_msg->p_elements[] array based on T value from
     * TLV header */
    p_element = qmi_idl_decode_find_tlv( p_msg->p_encoded_tlv_data,
                                         msg_type,
                                         &optional_offset );

    if( p_element != NULL ) {
      /* Use the message element data (pointed to by p_element) to decode
       * the message.
       * If a TLV is duplicated, it's a bad message */
      if (found[msg_type]) {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_TLV_DUPLICATED, msg_type, 0, 0 );
      }

      found[msg_type] = TRUE;
      /* Check to see if the mandatory response field reports an error */
      if ( req_resp_ind == QMI_IDL_RESPONSE ) {
        /* If the T is 2 in a response, it is the standard response msg */
        if ( msg_type == 2 ) {
          resp_error = p_src[0];
        }
      }
      bytes_remaining = qmi_idl_decode_element( p_dst,
                                                &p_src,
                                                msg_len,
                                                optional_offset,
                                                &p_element,
                                                msg_type_table,
                                                exc,
                                                &range_err,
                                                p_dst_start,
                                                p_dst_end );

      /* If the length of the TLV doesn't match the actual length of
       * the value, it's a bad message */
      if( bytes_remaining != 0 ) {
        QMI_IDL_HANDLE_ERROR(exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                             15, bytes_remaining, 0);
      }

      src_len -= msg_len;
    }
    else if( msg_type < 0x10 ) {
      /* Unknown mandatory TLV */
      QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_UNKNOWN_MANDATORY_TLV,
                            msg_type, 0, 0 );
    }
    else {
      /* Unrecognized TLV's are to be ignored.
       * Increment encoded message data pointer and decrement size */
      p_src += msg_len;
      src_len -= msg_len;
    }
  }

  if (!resp_error) {
    /* Verify that all required TLV's were found */
    qmi_idl_decode_verify_mandatory( p_msg->p_encoded_tlv_data, found, exc );
  }
} /* qmi_idl_message_decode_v6 */

/***********************************************************************

FUNCTION qmi_idl_encode_element

DESCRIPTION
  Encodes a TLV or (recursively) an element within a nested aggregate type

ARGUMENTS     See inline comments

RETURN VALUE  number of bytes remaining in destination buffer

***********************************************************************/
static uint32_t qmi_idl_encode_element
(
  /* Destination buffer */
  uint8_t **pp_dst,

  /*  Size of destination buffer */
  uint32_t dst_bytes,

  /* Pointer to beginning of C-struct representing message or nested
   * aggregate type */
  const uint8_t *p_src,

  /* Pointer to type element description stream */
  const uint8_t **pp_element,

  /* Pointer to type table object that nested aggregate types refer to */
  const qmi_idl_type_table_object *p_type_table,

  /* If -1, not top level
   * If 0, top level - TLV mandatory
   * If > 0, top level - TLV optional, value is offset to valid flag */
  int32_t optional_offset,

  /* For exception handling */
  qmi_idl_lib_exception_type *exc
)
{
  const uint8_t *p_element = *pp_element;
  const uint8_t *p_element2;
  uint32_t flags;
  uint32_t array_max_size;
  uint32_t length;                /* Number of array elements */
  uint32_t i;
  uint32_t ag_type;
  uint32_t table_index;
  uint32_t type_index;
  uint32_t c_struct_sz;
  uint32_t range_index = 0;

  qmi_idl_print_element_info( "qmi_idl_encode_element", p_element );

  /* decode p_element information - when done we have:
   * flags = flag byte of p_element info
   * length = number of items to copy
   * p_src advanced to the start of the data field
   * the size field has been encoded, if required
   * the number of array items validated
   */
  flags = *p_element++;
  if( flags & QMI_IDL_FLAGS_FIRST_EXTENDED) {
      flags |= *p_element++ << 8;
  }
  if( flags & (QMI_IDL_FLAGS_SECOND_EXTENDED << 8))
  {
    flags |= *p_element++ << 16;
  }

  /* Update C struct pointer to point to element to encode */
  p_src += *p_element++;
  if( flags & QMI_IDL_FLAGS_OFFSET_IS_16 ) {
    p_src += (uint32_t)(*p_element++) << 8;
  } else if(flags & (QMI_IDL_FLAGS_EXTENDED_OFFSET << 8)) {
    p_src += (uint32_t)(*p_element++) << 8;
    p_src += (uint32_t)(*p_element++) << 16;
  }

  if( optional_offset > 0 ) {
    /* Top level optional TLV */
    if( !*(uint8_t *)(p_src - optional_offset) ) {
      /* Valid flag not set - skip rest of element description and return */
      if( flags & QMI_IDL_FLAGS_IS_ARRAY ) {
        if( flags & QMI_IDL_FLAGS_MAX_SZ_IS_16 ) {
          p_element += 2;
        }
        else {
          p_element += 1;
        }

        if( (flags & QMI_IDL_FLAGS_IS_VARIABLE_LEN) &&
            ((flags & QMI_IDL_FLAGS_TYPE) != QMI_IDL_STRING) &&
            !(flags & (QMI_IDL_FLAGS_UTF16_STRING << 8)) ) {
          p_element += 1;
        }
        }

      if( (flags & QMI_IDL_FLAGS_TYPE) == QMI_IDL_AGGREGATE ) {
        p_element += 2;
      }
      if (flags & (QMI_IDL_FLAGS_RANGE_CHECKED << 16))
      {
        p_element += 4;
      }
      *pp_element = p_element;
      return dst_bytes;
    }
  }

  if( optional_offset != -1 ) {
    /* At top level - leave room for type and length value */

    /* If less than 3 bytes, not even enough room for a minimal TLV
     * A TLV with a zero-length value is possible, for example an empty string
     */
    if( dst_bytes < 3 ) {
      QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_BUFFER_TOO_SMALL, 2, dst_bytes, 3 );
    }

    *pp_dst = *pp_dst + 3;
    dst_bytes -= 3;
  }

  if( (flags & QMI_IDL_FLAGS_TYPE) == QMI_IDL_STRING ) {
    /* Handle strings separately */
    GET_8_OR_16( array_max_size, (flags & QMI_IDL_FLAGS_MAX_SZ_IS_16), p_element );
    if (flags & (QMI_IDL_FLAGS_SZ_IS_32 << 8)) {
      p_element += 2;
    }
    /* TODO - possible optimization is to make single pass over string */
    length = strlen((char *)p_src);
    if( length > array_max_size) {
      QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_ARRAY_TOO_BIG,
                            length, array_max_size, 0 );
    }

    if( optional_offset == -1 && (!(flags & (QMI_IDL_FLAGS_ARRAY_DATA_ONLY << 8)))) {
      dst_bytes = qmi_idl_encode_length( pp_dst, dst_bytes,
                                         flags, length, exc );
      if (flags & (QMI_IDL_FLAGS_ARRAY_LENGTH_ONLY << 8)) {
        *pp_element = p_element;
        return dst_bytes;
      }
    }

    if( length > dst_bytes ) {
      QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                            7, length, dst_bytes );
    }
    dst_bytes -= length;

    memcpy( *pp_dst, p_src, length );
    *pp_dst = *pp_dst + length;
  }else if (flags & (QMI_IDL_FLAGS_UTF16_STRING << 8))
  {
    uint16_t character;
    GET_8_OR_16( array_max_size, (flags & QMI_IDL_FLAGS_MAX_SZ_IS_16), p_element );
    if (flags & (QMI_IDL_FLAGS_SZ_IS_32 << 8)) {
      p_element += 2;
    }
    character = (*p_src++);
    character |= (*p_src++ << 8);
    length = 0;
    while(character != 0)
    {
      length ++;
      character = (*p_src++);
      character |= (*p_src++ << 8);
    }
    if( length > array_max_size ) {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_ARRAY_TOO_BIG,
                              length, array_max_size, 0 );
    }
    p_src -= (length*2 + 2);
  }else if( flags & QMI_IDL_FLAGS_IS_ARRAY ) {
    GET_8_OR_16( array_max_size, (flags & QMI_IDL_FLAGS_MAX_SZ_IS_16), p_element );
    if (flags & (QMI_IDL_FLAGS_SZ_IS_32 << 8)) {
      p_element += 2;
    }
    if( flags & QMI_IDL_FLAGS_IS_VARIABLE_LEN ) {
      length = *(uint32_t *)(p_src - *p_element++);

      if( length > array_max_size ) {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_ARRAY_TOO_BIG,
                              length, array_max_size, 0 );
      }
    }
    else {
      length = array_max_size;
    }
  }
  else {
    length = 1;
  }
  /* Range Checking */
  if (flags & (QMI_IDL_FLAGS_RANGE_CHECKED << 16))
  {
    range_index = (*p_element++);
    range_index |= ((*p_element++) << 8);
    range_index |= ((*p_element++) << 16);
    range_index |= ((*p_element++) << 24);
  }

  /* Done parsing element, set return value to start of next element */
  *pp_element = p_element;

  switch( flags & QMI_IDL_FLAGS_TYPE ) {
    case QMI_IDL_GENERIC_1_BYTE:
    case QMI_IDL_GENERIC_2_BYTE:
    case QMI_IDL_GENERIC_4_BYTE:
    case QMI_IDL_GENERIC_8_BYTE:
      /* Handle simple primitive types */
      /* if variable length array send number of items */
      if( flags & QMI_IDL_FLAGS_IS_ARRAY && flags & QMI_IDL_FLAGS_IS_VARIABLE_LEN
          && (!(flags & ((QMI_IDL_FLAGS_ARRAY_DATA_ONLY << 8) |
                         (QMI_IDL_FLAGS_ARRAY_IS_LENGTHLESS << 8))))) {
        dst_bytes = qmi_idl_encode_length( pp_dst, dst_bytes,
                                           flags, length, exc );
        if (flags & (QMI_IDL_FLAGS_ARRAY_LENGTH_ONLY << 8)) {
          return dst_bytes;
        }
      }

      /* convert length in items to bytes - depends on item size == 2 ^ type */
      length = length << (flags & QMI_IDL_FLAGS_TYPE);

      if( length > dst_bytes ) {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                              8, length, dst_bytes );
      }
      dst_bytes -= length;

      memcpy( *pp_dst, p_src, length );
      /* Range Checking */
      if (flags & (QMI_IDL_FLAGS_RANGE_CHECKED << 16))
      {
        int8_t ret_val = 0;
        uint64_t tmp_val = 0;
        length = length >> (flags & QMI_IDL_FLAGS_TYPE);
        for(i=0; i<length; i++)
        {
          tmp_val = 0;
          memcpy( &tmp_val, p_src, (1 << (flags & QMI_IDL_FLAGS_TYPE)));
          ret_val = p_type_table->p_ranges[range_index].range_check(&tmp_val);
          if (ret_val != QMI_IDL_RANGE_RESPONSE_SUCCESS)
          {
            QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_RANGE_FAILURE, 0, 0, 0 );
          }
          *pp_dst += (1 << (flags & QMI_IDL_FLAGS_TYPE));
        }
      }else
      {
      *pp_dst = *pp_dst + length;
      }
      break;

    case QMI_IDL_1_BYTE_ENUM:
      /* if variable length array send number of items */
      if( flags & QMI_IDL_FLAGS_IS_ARRAY && flags & QMI_IDL_FLAGS_IS_VARIABLE_LEN
          && (!(flags & ((QMI_IDL_FLAGS_ARRAY_DATA_ONLY << 8) |
                         (QMI_IDL_FLAGS_ARRAY_IS_LENGTHLESS << 8))))) {
        dst_bytes = qmi_idl_encode_length( pp_dst, dst_bytes,
                                           flags, length, exc );
        if (flags & (QMI_IDL_FLAGS_ARRAY_LENGTH_ONLY << 8)) {
          return dst_bytes;
        }
      }

      if( length > dst_bytes ) {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                              9, length, dst_bytes );
      }
      dst_bytes -= length;

      for( i =0; i < length; i++ ) {
        *(*pp_dst)++ = *p_src;
        p_src += 4;
      }
      if (flags & (QMI_IDL_FLAGS_RANGE_CHECKED << 16))
      {
        int8_t ret_val = 0;
        uint32_t tmp_val = 0;
        p_src -= 4*length;
        for(i=0; i<length; i++)
        {
          tmp_val = 0;
          memcpy( &tmp_val, p_src, 4);
          ret_val = p_type_table->p_ranges[range_index].range_check(&tmp_val);
          if (ret_val != QMI_IDL_RANGE_RESPONSE_SUCCESS)
          {
            QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_RANGE_FAILURE, 0, 0, 0 );
          }
          p_src+=4;
        }
      }
      break;

    case QMI_IDL_2_BYTE_ENUM:
      /* if variable length array send number of items */
      if( flags & QMI_IDL_FLAGS_IS_ARRAY && flags & QMI_IDL_FLAGS_IS_VARIABLE_LEN
          && (!(flags & ((QMI_IDL_FLAGS_ARRAY_DATA_ONLY << 8) |
                         (QMI_IDL_FLAGS_ARRAY_IS_LENGTHLESS << 8))))) {
        dst_bytes = qmi_idl_encode_length( pp_dst, dst_bytes,
                                           flags, length, exc );
        if (flags & (QMI_IDL_FLAGS_ARRAY_LENGTH_ONLY << 8)) {
          return dst_bytes;
        }
      }

      if( 2*length > dst_bytes ) {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY,
                              10, length, dst_bytes );
      }
      dst_bytes -= 2*length;

      for( i =0; i < length; i++ ) {
        /* TODO - depends on little endian */
        *(*pp_dst)++ = *p_src++;
        *(*pp_dst)++ = *p_src;
        p_src += 3;
      }
      if (flags & (QMI_IDL_FLAGS_RANGE_CHECKED << 16))
      {
        int8_t ret_val = 0;
        uint32_t tmp_val = 0;
        p_src -= 4*length;
        for(i=0; i<length; i++)
        {
          tmp_val = 0;
          memcpy( &tmp_val, p_src, 4);
          ret_val = p_type_table->p_ranges[range_index].range_check(&tmp_val);
          if (ret_val != QMI_IDL_RANGE_RESPONSE_SUCCESS)
          {
            QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_RANGE_FAILURE, 0, 0, 0 );
          }
          p_src += 4;
        }
      }
      break;

    case QMI_IDL_STRING:
      /* String type handled above - do nothing */
      break;

    case QMI_IDL_AGGREGATE:
      /* Handle aggregate type */
      /* if variable length array send number of items */
      if( flags & QMI_IDL_FLAGS_IS_ARRAY && flags & QMI_IDL_FLAGS_IS_VARIABLE_LEN
          && (!(flags & ((QMI_IDL_FLAGS_ARRAY_DATA_ONLY << 8) |
                         (QMI_IDL_FLAGS_ARRAY_IS_LENGTHLESS << 8))))) {
        dst_bytes = qmi_idl_encode_length( pp_dst, dst_bytes,
                                           flags, length, exc );
        if (flags & (QMI_IDL_FLAGS_ARRAY_LENGTH_ONLY << 8)) {
          *pp_element += 2;
          return dst_bytes;
        }
      }

      /* use pp_element because it's already been updated */
      GET_16( ag_type, *pp_element );
      table_index = QMI_IDL_TYPE_TABLE( ag_type );
      p_type_table = p_type_table->p_referenced_tables[table_index];

      type_index = QMI_IDL_TYPE_TYPE( ag_type );
      p_element = p_type_table->p_types[type_index].p_encoded_type_data;
      c_struct_sz = p_type_table->p_types[type_index].c_struct_sz;


      for( i = 0; i < length; i++ ) {
        p_element2 = p_element;

        do {
          dst_bytes = qmi_idl_encode_element( pp_dst,
                                              dst_bytes,
                                              p_src,
                                              &p_element2,
                                              p_type_table,
                                              -1,
                                              exc );
        } while( *p_element2 != QMI_IDL_FLAG_END_VALUE );

        p_src += c_struct_sz;
      }
      break;
  }

  return dst_bytes;
} /* qmi_idl_encode_element */

/***********************************************************************

FUNCTION qmi_idl_message_encode_v6

DESCRIPTION
  Encodes the body (TLV's) of a QMI message based on C data structure
  based on verison #1 of the database. On error does long

ARGUMENTS     See inline comments

RETURN VALUE  Success, number of bytes encoded (too be filled in as the
              length field of the QMI header)
              Error - does longjump, does not return

***********************************************************************/
static uint32_t qmi_idl_message_encode_v6
(
  /* Pointer to service object, from auto-generated header from IDL */
  const qmi_idl_service_object_type p_service,

  /* See enumeration above */
  qmi_idl_type_of_message_type req_resp_ind,

  /* Message ID from IDL */
  uint16_t message_id,

  /* Pointer to C structure containing message data */
  const void *p_src,

  /* Length of C structure containing message data */
  uint32_t src_len,

  /* Pointer to buffer for encoded message data */
  void *p_dst,

  /* Length of above buffer */
  uint32_t dst_len,

  /* For exception handling */
  qmi_idl_lib_exception_type *exc
)
{
  /* Pointer to the message table entry for this message */
  const qmi_idl_message_table_entry *p_msg;
  const uint8_t *p_element;     /* Pointer to message format data stream */
  const qmi_idl_type_table_object *msg_type_table; /* Pointer in case message
                                                      is defined in included IDL*/
  uint8_t *p_dst2 = p_dst;
  int32_t optional_offset;      /* Relative offset for the valid flag */
  uint8_t msg_type;                    /* TLV "T" value */
  uint8_t *p_hdr;
  uint32_t new_dst_len;
  uint32_t length;
  uint32_t total_length;

  /* Variables used for TLV encoding */
  uint8_t tlv_flags;

  /* Lookup message ID */
  p_msg = qmi_idl_find_msg( p_service, req_resp_ind, message_id, exc, &msg_type_table );

  p_element = p_msg->p_encoded_tlv_data;
  total_length = 0;

  /* Validate that the source buffer is the correct length
     it is valid for src_len to be greater than c_struct_sz */
  if (src_len < p_msg->c_struct_sz) {
    QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_BUFFER_TOO_SMALL,
                          1, dst_len, p_msg->c_struct_sz );
  }else if(p_msg->c_struct_sz == 0 || dst_len == 0 || p_element == NULL)
  {
    /* If the c_struct or p_dst is 0 length, return */
    return total_length;
  }
  /* Iterate over each TLV in the decoded structure */
  do {
    tlv_flags = *p_element++;

    /* Get the message element data stream "T" value */
    if (QMI_IDL_TLV_FLAGS_OPTIONAL & tlv_flags ) {
      msg_type = *p_element++;
      optional_offset = QMI_IDL_TLV_FLAGS_OFFSET_FOR_OPTIONAL_TLV & tlv_flags;
    } else {
      msg_type = QMI_IDL_TLV_FLAGS_TLV_TYPE_FOR_REQUIRED_TLV & tlv_flags;
      optional_offset = 0;
    }

    /* Save pointer to TLV header */
    p_hdr = p_dst2;

    /* Encode the TLV "V", will leave room for T & L */
    new_dst_len = qmi_idl_encode_element( &p_dst2,
                                          dst_len,
                                          p_src,
                                          &p_element,
                                          msg_type_table,
                                          optional_offset,
                                          exc );

    if( new_dst_len != dst_len ) {
      /* V encoded, set T & L */
      /* Check for integer underflow */
      if (dst_len < (3+new_dst_len))
      {
        QMI_IDL_HANDLE_ERROR( exc, QMI_IDL_LIB_LENGTH_INCONSISTENCY, 0, 0, 0 );
      }
      length = dst_len - 3 - new_dst_len;
      *p_hdr++ = msg_type;
      *p_hdr++ = BYTE0( length );
      *p_hdr++ = BYTE1( length );
      total_length += length + 3;
      dst_len = new_dst_len;
    }
  } while (! (QMI_IDL_TLV_FLAGS_LAST_TLV & tlv_flags) );

  /* Return number of encoded bytes and success */
  return total_length;
} /* qmi_idl_message_encode_v6 */


/***********************************************************************

FUNCTION qmi_idl_message_decode

DESCRIPTION
  Decodes a QMI message body from the wire format to the C structure

ARGUMENTS     See inline comments

RETURN VALUE  =0:  Success
              <0:  Error, see enumeration in header file

***********************************************************************/
int32_t qmi_idl_message_decode
(
  /* Pointer to service object, from auto-generated header from IDL */
  const qmi_idl_service_object_type p_service,

  /* See enumeration above */
  qmi_idl_type_of_message_type req_resp_ind,

  /* Message ID from IDL */
  uint16_t message_id,

  /* Pointer to BEGINNING OF FIRST TLV IN MESSAGE */
  const void *p_src,

  /* Length field from QMI header */
  uint32_t src_len,

  /* Pointer to C structure for decoded data */
  void *p_dst,

  /* Size of above structure, for sanity check */
  uint32_t dst_len
)
{
  /* Exception handling variables */
  qmi_idl_lib_exception_type exc;

  /* Set up exception handling */
  QMI_IDL_LIB_TRY(&exc) {
    if( p_service == NULL ||
      req_resp_ind >= QMI_IDL_NUM_MSG_TYPES ||
        (p_src == NULL && src_len > 0) ||
        (p_dst == NULL && dst_len > 0) ) {
      QMI_IDL_HANDLE_ERROR( &exc, QMI_IDL_LIB_PARAMETER_ERROR, 0, 0, 0 );
    }

    switch( p_service->library_version ) {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
        qmi_idl_message_decode_v6( p_service,
            req_resp_ind,
            message_id,
            p_src,
            src_len,
            p_dst,
            dst_len,
            &exc );
        break;


      default:
        QMI_IDL_HANDLE_ERROR( &exc, QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION,
                          p_service->library_version, 0, 0 );
        break;
    }
  } QMI_IDL_LIB_CATCH(&exc) {
    return QMI_IDL_LIB_GET_ERROR(&exc);
  }


  return QMI_IDL_LIB_NO_ERR;
} /* qmi_idl_message_decode */

/***********************************************************************

FUNCTION qmi_idl_message_encode

DESCRIPTION
  Encodes the body (TLV's) of a QMI message based on C data structure

ARGUMENTS     See inline comments

RETURN VALUE  >0: Success, number of bytes encoded (too be filled in
                  as the length field of the QMI header)

              <0:  Error, see enumeration above

***********************************************************************/
int32_t qmi_idl_message_encode
(
  /* Pointer to service object, from auto-generated header from IDL */
  const qmi_idl_service_object_type p_service,

  /* See enumeration above */
  qmi_idl_type_of_message_type req_resp_ind,

  /* Message ID from IDL */
  uint16_t message_id,

  /* Pointer to C structure containing message data */
  const void *p_src,

  /* Length of above buffer */
  uint32_t src_len,

  /* Pointer to buffer for encoded message data */
  void *p_dst,

  /* Length of above buffer */
  uint32_t dst_len,

  /* Length of the encoded message */
  uint32_t *dst_encoded_len
)
{
  /* Exception handling variables */
  qmi_idl_lib_exception_type exc;

  QMI_IDL_LIB_TRY(&exc) {
    if( p_service == NULL ||
      req_resp_ind >= QMI_IDL_NUM_MSG_TYPES ||
        (p_src == NULL && src_len > 0) ||
        (p_dst == NULL && dst_len > 0) ) {
      QMI_IDL_HANDLE_ERROR( &exc, QMI_IDL_LIB_PARAMETER_ERROR, 0, 0, 0 );
    }
    switch( p_service->library_version ) {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
        *dst_encoded_len = qmi_idl_message_encode_v6( p_service,
            req_resp_ind,
            message_id,
            p_src,
            src_len,
            p_dst,
            dst_len,
            &exc );
        break;


      default:
        QMI_IDL_HANDLE_ERROR( &exc, QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION,
                          p_service->library_version, 0, 0 );
        break;
    }
  } QMI_IDL_LIB_CATCH(&exc) {
    return QMI_IDL_LIB_GET_ERROR(&exc);
  }
  return QMI_IDL_LIB_NO_ERR;
} /* qmi_idl_message_encode  */

/***********************************************************************

FUNCTION qmi_idl_encode_resp_tlv

DESCRIPTION
  Encodes the TLV of the standard response message

ARGUMENTS     See inline comments

RETURN VALUE  >0: Success, number of bytes encoded (too be filled in
                  as the length field of the QMI header)

              <0:  Error, see enumeration above

***********************************************************************/
int32_t qmi_idl_encode_resp_tlv
(
  /* Result value for the response TLV */
  uint16_t result,

  /* Error value for the response TLV */
  uint16_t error,

  /* Pointer to buffer for encoded message data */
  void *p_dst,

  /* Length of above buffer */
  uint32_t dst_len
)
{
  uint8_t *p_dst2 = p_dst;

  if (!p_dst2 || dst_len < QMI_IDL_RESP_TLV_LEN) {
    return QMI_IDL_LIB_PARAMETER_ERROR;
  }
  memset(p_dst2,0,QMI_IDL_RESP_TLV_LEN);
  /* Response TLV T is always 2 */
  memset(p_dst2,2,1);
  ++p_dst2;

  /* Encode the len in the high bits of the L field */
  memset(p_dst2,QMI_IDL_RESP_TLV_LEN-3,1);
  p_dst2 += 2;

  /* Encode the result and errors */
  memcpy(p_dst2,&result,2);
  p_dst2 +=2;
  memcpy(p_dst2,&error,2);

  return QMI_IDL_LIB_NO_ERR;
} /* qmi_idl_encode_resp_tlv */

/*===========================================================================
  FUNCTION  qmi_idl_get_std_resp_tlv_len
===========================================================================*/
/*!
@brief
  Returns the length of a standard response message

@retval    length of the standard response

*/
/*=========================================================================*/
uint32_t qmi_idl_get_std_resp_tlv_len(void)
{
  return QMI_IDL_RESP_TLV_LEN;
}


