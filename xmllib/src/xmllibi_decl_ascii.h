#ifndef _XMLLIBI_DECL_ASCII
#define _XMLLIBI_DECL_ASCII
/*===========================================================================

                  X M L    D E C L    L I B R A R Y
                        A S C I I   E N C O D I N G
                   
DESCRIPTION
 This file implements the XML decl library's ascii encodng functions.
 
Copyright (c) 2006 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllibi_decl_ascii.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/29/06   ssingh  Moved typedefs and macros to xmllibi_decl_ascii.c
01/04/06   jsh     created file
===========================================================================*/
#include "xmllib_common.h"
#include "xmllib_decl.h"

/*===========================================================================
                   XML DECL ASCII FUNCTIONS
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_DECL_ASCII_CHECK_VALUE_CONST

DESCRIPTION
  This function is called with a string to be checked against the given
  string constant.
  
DEPENDENCIES
  None

RETURN VALUE
  TRUE   if matches
  FALSE  if not

SIDE EFFECTS
  None
===========================================================================*/
int32 xmllibi_decl_ascii_check_value_const
(
  xmllib_string_s_type     *source,      /* source string being compared*/
  xmllib_decl_const_e_type  expval,      /* Expected value const        */
  xmllib_error_e_type      *error_code
);

#endif /* _XMLLIBI_DECL_ASCII */

