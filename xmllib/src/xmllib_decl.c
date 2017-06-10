/*===========================================================================

                  X M L   D E C L   L I B R A R Y
                   
DESCRIPTION
 This file implements the XML decl library.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2006 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllib_decl.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/29/06   ssingh  Added support for UTF-8 encoding - extended 
                   xmllibi_decl_check_value_tbl.
01/04/06   jsh     created module

===========================================================================*/
#include "comdef.h"

#ifdef FEATURE_XMLLIB
#include "xmllib_decl.h"
#include "xmllibi_decl_ascii.h"
#include "xmllibi_decl_utf8.h"

/*===========================================================================
FUNCTION XMLLIBI_TOK_ASCII_CHECKVALUE

DESCRIPTION
  This function is called with a byte value to check if it is present in
  the stream at the specified offset.
  
DEPENDENCIES
  None.

RETURN VALUE
  TRUE   if token found at the specified offset
  FALSE  if the bytevalue is not found at the offset

SIDE EFFECTS
  None
===========================================================================*/
typedef int32 (* xmllibi_decl_check_value_fptr_type)
(
  xmllib_string_s_type    * string,       /* source string being compared*/
  xmllib_decl_const_e_type  expvalue,     /* Expected const value        */
  xmllib_error_e_type     * error_code
) ;

/*===========================================================================
  encoding specific check value functions
===========================================================================*/
static xmllibi_decl_check_value_fptr_type
  xmllibi_decl_check_value_tbl[XMLLIB_ENCODING_MAX] =
{
  xmllibi_decl_ascii_check_value_const,
  xmllibi_decl_utf8_check_value_const
};

/*===========================================================================
FUNCTION XMLLIB_DECL_CHECK_VALUE

DESCRIPTION
  This function is passed XML decl string, which it compares against the
  value referred by given string constant.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR    in case of an error with the error code set in the
                  error_code argument.
  XMLLIB_SUCCESS  token matches the constant

SIDE EFFECTS
  None
===========================================================================*/
int32 xmllib_decl_check_value
(
  xmllib_encoding_e_type   encoding,        /* Encoding used               */
  xmllib_string_s_type    *string,          /* source string being compared*/
  xmllib_decl_const_e_type expval,          /* value being matched         */
  xmllib_error_e_type     *error_code       /* Error code                  */
)
{
  int32  ret = XMLLIB_SUCCESS;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify arguments
    a) encoding is supported
    b) string is not NULL
    c) error_code cannot be NULL
    d) there is a check value function for the encpding
  -------------------------------------------------------------------------*/
  if( XMLLIB_ENCODING_MAX <= encoding            ||
      NULL                == string              ||
      NULL                == error_code          ||
      NULL                == xmllibi_decl_check_value_tbl[encoding] )
  {
    if( NULL != error_code )
    {
      *error_code = (XMLLIB_ENCODING_MAX <= encoding ) ? 
                      XMLLIB_ERROR_UNSUP_ENCODING      :
                      XMLLIB_ERROR_INVALID_ARGUMENTS;
    }
    ret = XMLLIB_ERROR;
  }
  else
  {
    /*-----------------------------------------------------------------------
      Call the appropriate function based on the encoding 
    -----------------------------------------------------------------------*/
    ret = xmllibi_decl_check_value_tbl[encoding]( string,
                                                 expval,
                                                 error_code ) ;
                                          
  }

  return ret;
} /* xmllib_decl_check_value() */
#endif /* FEATURE_XMLLIB */
