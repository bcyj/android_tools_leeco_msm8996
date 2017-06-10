/*!
  @file
  qcril_sms_util.c

  @brief

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

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_sms_util.c#4 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/02/09   sb      Added support for getting and setting the SMSC address.
04/04/09   fc      Cleanup log macros.
07/15/08   sb      First cut implementation.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "qcril_sms_util.h"
#include <string.h>
#include "qcril_smsi.h"
#include "wms.h"
#include "wmsts.h"
#include "bit.h"

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_sms_ts_convert_from_bcd_address

===========================================================================*/
/*!
    @brief
    Convert address to API structure from BCD structure.  

    @return
    None
*/
/*=========================================================================*/

void qcril_sms_ts_convert_from_bcd_address
(
  const cm_called_party_bcd_no_T    *bcd_addr_ptr,   // INPUT
  wms_address_s_type                *addr_ptr        // OUTPUT
)
{
  uint8   buf[WMS_GW_ADDRESS_MAX + 1];
  uint8 pos;

  if( addr_ptr == NULL || bcd_addr_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s", "Null pointer in qcril_sms_ts_convert_from_bcd_address!\n" );
    return;
  }

  if( 0 == bcd_addr_ptr->length || FALSE == bcd_addr_ptr->present )
  {
    addr_ptr->number_of_digits = 0;
  }
  else
  {
    /*Calculate number of digits */
    buf[0] =(uint8)( 2 * (bcd_addr_ptr->length-1));

    if( (bcd_addr_ptr->data[bcd_addr_ptr->length -1] & 0xF0) == 0xF0)
    {
      buf[0] -= 1;
    }

    /*copy the bytes after the first byte that has the number of digits to buf*/
    memcpy( buf+1, bcd_addr_ptr->data, bcd_addr_ptr->length );

    pos =  wms_ts_decode_address( buf, addr_ptr );

    if (pos != (bcd_addr_ptr->length+1)  )
    {
      QCRIL_LOG_ERROR( "%s", "Invalid BCD address Length in qcril_sms_ts_convert_from_bcd_address\n" );
    }
  }

  /* done */
  return;

} /* qcril_sms_ts_convert_from_bcd_address() */

/*=========================================================================
  FUNCTION:  qcril_sms_ts_convert_to_bcd_address

===========================================================================*/
/*!
    @brief
    Convert address from API structure to a BCD structure.

    @return
    None
*/
/*=========================================================================*/

void qcril_sms_ts_convert_to_bcd_address
(
  const wms_address_s_type          *addr_ptr,      // INPUT
  cm_called_party_bcd_no_T            *bcd_addr_ptr   // OUTPUT
)
{
  /* 1 byte for number type/plan and 10 bytes for number of digits */
  uint8   buf[WMS_GW_ADDRESS_MAX + 1];

  bcd_addr_ptr->present = FALSE;

  if( addr_ptr == NULL || bcd_addr_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s", "Null pointer in qcril_sms_ts_convert_to_bcd_address!\n" );
    return;
  }


  /* buf is the result which has number of digits and the remaining bytes
     bcd_addr_ptr->length does not include the byte for the number of digits (addr_length) */
  bcd_addr_ptr->length = wms_ts_encode_address( addr_ptr, buf );
  bcd_addr_ptr->length -= 1;

  QCRIL_ASSERT( bcd_addr_ptr->length < (WMS_GW_ADDRESS_MAX + 1) );

   //= buf[0];
  //memcpy( buf+1, bcd_addr_ptr->data, bcd_addr_ptr->length -1);
  memcpy( bcd_addr_ptr->data, buf+1, bcd_addr_ptr->length);

  bcd_addr_ptr->present = TRUE;

  return;

} /* qcril_sms_ts_convert_to_bcd_address() */

/*=========================================================================
  FUNCTION:  qcril_sms_hex_char_to_byte

===========================================================================*/
/*!
    @brief
    Convert a hex character to a byte

    @return
    Byte
*/
/*=========================================================================*/
byte qcril_sms_hex_char_to_byte
(
  char hex_char
)
{
  if (hex_char >= 'A' && hex_char <= 'Z')
  {
    hex_char = hex_char + 'a' - 'A';
  }

  if (hex_char >= 'a' && hex_char <= 'f')
  {
    return (char)(hex_char - 'a' + 10);
  }
  else if (hex_char >= 'A' && hex_char <= 'F')
  {
    return (char)(hex_char - 'A' + 10);
  }
  else if (hex_char >= '0' && hex_char <= '9')
  {
    return (char)(hex_char-'0');
  }
  else
  {
    return 0;
  }

} /* qcril_sms_hex_char_to_byte */

/*=========================================================================
  FUNCTION:  qcril_sms_hex_to_byte

===========================================================================*/
/*!
    @brief
    Convert an SMS PDU from ASCII hex format to a byte array.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_hex_to_byte
(
  const char * hex_pdu,   // INPUT
  byte * byte_pdu,        // OUTPUT
  uint32 num_hex_chars
)
{
  uint16 buf_pos = 0;
  uint32 i;

  for (i=0; i < num_hex_chars; i++)
  {
    b_packb(qcril_sms_hex_char_to_byte(hex_pdu[i]), byte_pdu, buf_pos, (word)4);
    buf_pos += 4;
  }

} /* qcril_sms_hex_to_byte */


/*=========================================================================
  FUNCTION:  qcril_sms_byte_to_hex_char

===========================================================================*/
/*!
    @brief
    Convert a byte to a hex character.

    @return
    Hex char
*/
/*=========================================================================*/
char qcril_sms_byte_to_hex_char (byte val)
{
  if (val <= 9)
  {
    return (char)(val+'0');
  }
  else if (val >= 10 && val <= 15)
  {
    return (char)(val-10+'A');
  }
  else
  {
    return '0';
  }
} /* qcril_sms_byte_to_hex_char */

/*=========================================================================
  FUNCTION:  qcril_sms_byte_to_hex

===========================================================================*/
/*!
    @brief
    Convert a byte array to an SMS PDU in ASCII hex format.

    @return
    None
*/
/*=========================================================================*/
void qcril_sms_byte_to_hex
(
  byte * byte_pdu,   // INPUT
  char * hex_pdu,    // OUTPUT
  uint32 num_bytes
)
{
  uint32 i;
  uint8 nibble;
  uint16 buf_pos = 0;

  for (i=0;i<num_bytes*2;i++)
  {
    nibble = b_unpackb(byte_pdu,buf_pos,4);
    buf_pos += 4;
    hex_pdu[i] = qcril_sms_byte_to_hex_char(nibble);
  }

} /* qcril_sms_byte_to_hex */


/*=========================================================================
  FUNCTION:  qcril_sms_lookup_cmd_name

===========================================================================*/
/*!
    @brief
    Look up the name of a WMS command.

    @return
    The string representing the name of a WMS command.    
*/
/*=========================================================================*/
const char *qcril_sms_lookup_cmd_name
(
  wms_cmd_id_e_type cmd_id
)
{
  switch (cmd_id)
  {
    case WMS_CMD_CFG_SET_ROUTES:
      return "WMS_CMD_CFG_SET_ROUTES";

    case WMS_CMD_CFG_SET_PRIMARY_CLIENT:
      return "WMS_CMD_CFG_SET_PRIMARY_CLIENT";

    case WMS_CMD_MSG_SEND:
      return "WMS_CMD_MSG_SEND";

    case WMS_CMD_CFG_SET_LINK_CONTROL:
      return "WMS_CMD_CFG_SET_LINK_CONTROL";

    case WMS_CMD_MSG_ACK:
      return "WMS_CMD_MSG_ACK";

    case WMS_CMD_MSG_WRITE:
      return "WMS_CMD_MSG_WRITE";

    case WMS_CMD_MSG_DELETE:
      return "WMS_CMD_MSG_DELETE";

    case WMS_CMD_BC_MM_SET_PREF:
      return "WMS_CMD_BC_MM_SET_PREF";

    case WMS_CMD_BC_MM_GET_TABLE:
      return "WMS_CMD_BC_MM_GET_TABLE";

    case WMS_CMD_BC_MM_DELETE_ALL_SERVICES:
      return "WMS_CMD_BC_MM_DELETE_ALL_SERVICES";

    case WMS_CMD_BC_MM_ADD_SRV:
      return "WMS_CMD_BC_MM_ADD_SRV";

    case WMS_CMD_CFG_SET_MEMORY_FULL:
      return "WMS_CMD_CFG_SET_MEMORY_FULL";

    case WMS_CMD_CFG_GET_MESSAGE_LIST:
      return "WMS_CMD_CFG_GET_MESSAGE_LIST";

    case WMS_CMD_MSG_READ_TEMPLATE:
      return "WMS_CMD_MSG_READ_TEMPLATE";

    case WMS_CMD_MSG_WRITE_TEMPLATE:
      return "WMS_CMD_MSG_WRITE_TEMPLATE";

    default: 
      return "Unknown WMS command";
  }

} /* qcril_sms_lookup_cmd_name */


/*===========================================================================

FUNCTION    qcril_sms_strip_quotes

DESCRIPTION
  Strips out quotes from the string that is wrapped in quotes.
  Resultant string will be placed in the out pointer.
  Gives an error if the string does not begin or end with a quote
  
DEPENDENCIES
  None

RETURN VALUE
  Boolean : 
    TRUE : if successful in stripping the quotes out
    FALSE : if there is any error

SIDE EFFECTS
  None

===========================================================================*/
boolean qcril_sms_strip_quotes
(
  const byte * in_ptr,   /* Pointer to the string to be processed  */
  byte * out_ptr         /* Pointer to the resultant string buffer */
) 
{
  if (*in_ptr != '\0' )
  {
    if ( *in_ptr++ == '"' )
    {
      while ( *in_ptr != '"' && *in_ptr != '\0' )
      {
        *out_ptr++ = *in_ptr++;
      }
      /* Check to see if the string ends with a null */
      if ( *in_ptr == '\0' )
      {
        /* We got a string without ending quotes */
        return FALSE;
      }
      else
      {
        /* Everything is happy */
        *out_ptr = '\0';
        return TRUE;
      }
    }
    else
    {
      /* We got a string with out quotes */
      return FALSE;
    }    
  }
  else
  {
    return FALSE;
  }
} /* qcril_sms_strip_quotes */


/*===========================================================================

FUNCTION    qcril_sms_strip_plus_sign

DESCRIPTION
  Strips out the plus sign from the beginning of the string
  Resultant string will be placed in the out pointer.
  Gives an error if the string does not begin with a plus sign
  
DEPENDENCIES
  None

RETURN VALUE
  Boolean : 
    TRUE : if successful in stripping the plus sign
    FALSE : if there is any error

SIDE EFFECTS
  None

===========================================================================*/
boolean qcril_sms_strip_plus_sign
(
  const byte * in_ptr,   /* Pointer to the string to be processed  */
  byte * out_ptr         /* Pointer to the resultant string buffer */
) 
{
  if (*in_ptr != '\0' )
  {
    if ( *in_ptr++ == '+' )
    {
      while ( *in_ptr != '\0' )
      {
        *out_ptr++ = *in_ptr++;
      }
      *out_ptr = '\0';
      return TRUE;
    }
    else
    {
      /* String did not start with a plus sign*/
      return FALSE;
    }    
  }
  else
  {
    /* Input string was null */
    return FALSE;
  }

} /* qcril_sms_strip_plus_sign */


/*=========================================================================
  FUNCTION:  qcril_sms_convert_smsc_address_to_wms_format

===========================================================================*/
/*!
    @brief
    Converts an SMSC address from RIL format to WMS format.
    Note that the RIL format comes from the CSCA ATCOP command:
    "+358501234567",145

    @return
    TRUE if the SMSC address was successfully translated to WMS format.
    FALSE otherwise.    
*/
/*=========================================================================*/
boolean qcril_sms_convert_smsc_address_to_wms_format
(
  const char * input_smsc_address_ptr,
  wms_address_s_type * output_smsc_address_ptr
)
{
  uint8 asciiDigits[WMS_ADDRESS_MAX];
  char toa[WMS_ADDRESS_MAX];
  uint32 i = 0;
  uint32 j = 0;
  uint32 type_of_address = 0;
  boolean number_type_is_international = FALSE;

  if (*input_smsc_address_ptr == '\0')
  {
    QCRIL_LOG_ERROR("%s", "Input SMSC address is empty!\n");
    return FALSE;
  }

  /* Copy everything up to the first comma into the SMSC address */
  while ((*input_smsc_address_ptr != '\0') && (*input_smsc_address_ptr != ','))
  {
    asciiDigits[i++] = *input_smsc_address_ptr++;
  }

  asciiDigits[i] = '\0';

  if (*input_smsc_address_ptr == ',')
  {
    /* Increment past the comma */
    input_smsc_address_ptr++;
    while (*input_smsc_address_ptr != '\0')
    {
      if (*input_smsc_address_ptr == ',')
      {
        QCRIL_LOG_ERROR("%s", "More than one comma in SMSC address!\n");
        return FALSE;
      }

      /* Copy the rest of the string into the type of address */
      toa[j++] = *input_smsc_address_ptr++;
    }
  }

  toa[j] = '\0';

  /* Strip the quotes from the SMSC address */
  if (!qcril_sms_strip_quotes(asciiDigits, asciiDigits))
  {
    QCRIL_LOG_ERROR("%s", "SMSC address either did not begin or end with quotes!\n");
    return FALSE;
  }

  if (asciiDigits[0] == '+')
  {
    /* If the number starts with a "+", this is an international number */
    number_type_is_international = TRUE;

    /* Strip out the plus sign */
    if (!qcril_sms_strip_plus_sign(asciiDigits, asciiDigits))
    {
      QCRIL_LOG_ERROR("%s", "Unable to strip plus sign from SMSC address!\n");
      return FALSE;
    }
  }

  if (strlen((char*)asciiDigits) == 0)
  {
    QCRIL_LOG_ERROR("%s", "SMSC address is empty!\n");
    return FALSE;
  }

  /* Initialize the digit mode and number mode */
  output_smsc_address_ptr->digit_mode = WMS_DIGIT_MODE_8_BIT;
  output_smsc_address_ptr->number_mode = WMS_NUMBER_MODE_NONE_DATA_NETWORK;

  if (strlen(toa) == 0)
  {
    /* No type of address information was given. */
    output_smsc_address_ptr->number_type = WMS_NUMBER_UNKNOWN;
    output_smsc_address_ptr->number_plan = WMS_NUMBER_PLAN_TELEPHONY;

    if (number_type_is_international)
    {
      output_smsc_address_ptr->number_type = WMS_NUMBER_INTERNATIONAL;
    }
  }
  else
  {
    /* Fill in the number_type and number_plan based on the type of address. */
    type_of_address = atoi(toa);
    if ((type_of_address == 0) || (type_of_address > 255))
    {
      QCRIL_LOG_ERROR("%s", "Type of address is out of range!\n");
      return FALSE;
    }

    output_smsc_address_ptr->number_type = 
    (wms_number_type_e_type)((type_of_address & 0x70)>>4);
    output_smsc_address_ptr->number_plan = 
    (wms_number_plan_e_type)(type_of_address & 0x0F);

    if (number_type_is_international)
    {
      /* The address starts with a "+", so it is an international number. */
      output_smsc_address_ptr->number_type = WMS_NUMBER_INTERNATIONAL;
    }
  }

  output_smsc_address_ptr->number_of_digits = strlen((char*)asciiDigits);

  wms_ts_ascii_to_bcd(asciiDigits, output_smsc_address_ptr->number_of_digits,
                      output_smsc_address_ptr->digits);

  return TRUE;

} /* qcril_sms_convert_smsc_address_to_wms_format */


/*=========================================================================
  FUNCTION:  qcril_sms_get_type_of_address

===========================================================================*/
/*!
    @brief
    Uses the number_type and number_plan fields to determine the type
    of address.

    @return
    None.

*/
/*=========================================================================*/
void qcril_sms_get_type_of_address
(
  const wms_address_s_type * address_ptr,
  uint8 * type_of_addr
)
{
  uint8 temp = 0;

  /* Calculate the type of address */
  *type_of_addr  = 0;

  temp = (uint8)((uint32)address_ptr->number_type & 0x07);
  *type_of_addr = (uint8)((*type_of_addr | temp ) << 4);
  temp = (uint8)((uint32)address_ptr->number_plan & 0x07);
  *type_of_addr = *type_of_addr | temp ;
  *type_of_addr = *type_of_addr | 0x80;

} /* qcril_sms_get_type_of_address */


/*=========================================================================
  FUNCTION:  qcril_sms_convert_smsc_address_to_ril_format

===========================================================================*/
/*!
    @brief
    Converts an SMSC address from WMS format to RIL format.
    Note that the RIL format comes from the CSCA ATCOP command:
    "+358501234567",145

    @return
    TRUE if the SMSC address was successfully translated to RIL format.
    FALSE otherwise.    
*/
/*=========================================================================*/
boolean qcril_sms_convert_smsc_address_to_ril_format
(
  const wms_address_s_type * input_smsc_address_ptr,
  char * output_smsc_address_ptr
)
{
  /* Current position within the output SMSC address. */
  uint32 curr_pos;
  uint8 type_of_address;
  char smsc_address[WMS_ADDRESS_MAX];

  if (input_smsc_address_ptr->number_of_digits == 0)
  {
    QCRIL_LOG_ERROR("%s", "SMSC address is empty!\n");
    return FALSE;
  }

  /* Add quote to the beginning of the SMSC address */
  smsc_address[0] = '"';
  curr_pos = 1;

  if (input_smsc_address_ptr->number_type == WMS_NUMBER_INTERNATIONAL)
  {
    /* It's an international number, so add a "+" to the beginning */
    smsc_address[curr_pos] = '+';
    curr_pos++;
  }

  /* Copy the digits into the output SMSC address */
  wms_ts_bcd_to_ascii(input_smsc_address_ptr->digits,
                      input_smsc_address_ptr->number_of_digits,
                      (uint8 *)&smsc_address[curr_pos]);

  curr_pos += input_smsc_address_ptr->number_of_digits;
  QCRIL_ASSERT( curr_pos < (WMS_ADDRESS_MAX - 3) );

  /* Add a quote to the end of the SMSC address, and a comma */
  smsc_address[curr_pos++] = '"';
  smsc_address[curr_pos++] = ',';
  smsc_address[curr_pos++] = '\0';

  /* Calculate the type of address */
  qcril_sms_get_type_of_address(input_smsc_address_ptr,
                                &type_of_address);

  /* Append the type of address to the SMSC address */
  QCRIL_SNPRINTF( output_smsc_address_ptr, WMS_ADDRESS_MAX, "%s%d", 
                  smsc_address, type_of_address );

  return TRUE;

} /* qcril_sms_convert_smsc_address_to_ril_format */
