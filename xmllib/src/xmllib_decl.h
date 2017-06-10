#ifndef _XMLLIB_DECL_H
#define _XMLLIB_DECL_H
/*===========================================================================

            X M L   D E C L   L I B R A R Y   H E A D E R
                   
DESCRIPTION
 This file includes declarations for the XML decl library.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2006 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllib_decl.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/29/06   ssingh  Added support for UTF-8 encoding - extended 
                   xmlllib_decl_const_e_type.
01/04/06   jsh     created file

===========================================================================*/
#include "xmllib_common.h"

/*---------------------------------------------------------------------------
  typedef of XML decl constants
---------------------------------------------------------------------------*/
typedef enum xmlllib_decl_const_e_type
{
  XMLLIB_CONST_VERSION            =  0,      /* 'version'                */
  XMLLIB_CONST_ENCODING           =  1,      /* 'encoding'               */
  XMLLIB_CONST_STDALONE           =  2,      /* 'standalone'             */
  XMLLIB_CONST_XML_VER            =  3,      /* XMLLIB_XML_VER           */
  XMLLIB_CONST_STDALONE_YES       =  4,      /* 'yes'                    */
  XMLLIB_CONST_STDALONE_NO        =  5,      /* 'no'                     */
  XMLLIB_CONST_ENC_ASCII          =  6,      /* 'ascii'                  */
  XMLLIB_CONST_ENC_ISO_8859_1     =  7,      /* 'ISO-8859-1'             */
  XMLLIB_CONST_ENC_UTF8           =  8,      /* 'UTF-8'                  */
  XMLLIB_CONST_MAX                                      
} xmllib_decl_const_e_type ;

/*===========================================================================
                   XML DECL FUNCTIONS
===========================================================================*/
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
);

#endif /* _XMLLIB_DECL_H */
