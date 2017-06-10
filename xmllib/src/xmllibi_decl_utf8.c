/*===========================================================================

                  X M L    D E C L      L I B R A R Y
                   U T F - 8   E N C O D I N G
                   
DESCRIPTION
 This file implements the XML decl library in UTF-8 encoding.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2006 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllibi_decl_utf8.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/29/06   ssingh  Created module.

===========================================================================*/
/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include "comdef.h"

#ifdef FEATURE_XMLLIB
#include "xmllibi_decl_utf8.h"
#include "xmllib_decl.h"
#include <string.h>

/*===========================================================================
                         local definitions
===========================================================================*/
/*---------------------------------------------------------------------------
  The UCS codepoint values for single quotes and double quotes.
  Aside: ASCII encoding also has the same code point values as UCS encoding.
---------------------------------------------------------------------------*/
#define XMLLIB_SINGLE_QUOTE_UCS 39
#define XMLLIB_DOUBLE_QUOTE_UCS 34

/*---------------------------------------------------------------------------
  enumerates quote options
---------------------------------------------------------------------------*/
typedef enum
{
  XMLLIB_TEXT_QUOTE_REQ = 0,
  XMLLIB_TEXT_QUOTE_OPT,
  XMLLIB_TEXT_QUOTE_NOT_REQ,
  XMLLIB_TEXT_QUOTE_MAX
} xmllib_quote_req_e_type;


/*---------------------------------------------------------------------------
  typedef of xmllib_tok_utf8 const-->string mapping table
---------------------------------------------------------------------------*/
typedef struct
{ 
  char                     * str;
  xmllib_quote_req_e_type    quote_req;
} xmllib_tok_utf8_const_tbl_type ;

/*---------------------------------------------------------------------------
  static table containing const-->string mappings ( used by check_value_const
  function ) LOWER CASE
---------------------------------------------------------------------------*/
#define XMLLIB_XML_VER_ATTRIB_VALUE XMLLIB_XML_VER
static const xmllib_tok_utf8_const_tbl_type const_tbl[XMLLIB_CONST_MAX] =
{
  /* mapped from XMLLIB_CONST_VERSION */
  { "version",                   XMLLIB_TEXT_QUOTE_NOT_REQ },
  /* mapped from XMLLIB_CONST_ENCODING */
  { "encoding",                  XMLLIB_TEXT_QUOTE_NOT_REQ },          
  /* mapped from XMLLIB_CONST_STDALONE */
  { "standalone",                XMLLIB_TEXT_QUOTE_NOT_REQ },          
  /* mapped from XMLLIB_CONST_XML_VER */
  { XMLLIB_XML_VER_ATTRIB_VALUE, XMLLIB_TEXT_QUOTE_REQ     }, 
  /* mapped from XMLLIB_CONST_STDALONE_YES */                               
  { "yes",                       XMLLIB_TEXT_QUOTE_REQ     },
  /* mapped from XMLLIB_CONST_STDALONE_NO */
  { "no",                        XMLLIB_TEXT_QUOTE_REQ     },
  /* mapped from XMLLIB_CONST_ENC_ASCII */
  { "ascii",                     XMLLIB_TEXT_QUOTE_REQ     },
  /* mapped from XMLLIB_CONST_ENC_ISO_8859_1 */
  { "iso-8859-1",                XMLLIB_TEXT_QUOTE_REQ     },
  /* mapped from XMLLIB_CONST_ENC_UTF8 */
  { "UTF-8",                     XMLLIB_TEXT_QUOTE_REQ     }
} ;


/*===========================================================================
                     XML DECL FUNCTIONS
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_DECL_UTF8_CHECK_VALUE_CONST

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
int32 xmllibi_decl_utf8_check_value_const
(
  xmllib_string_s_type     *source,      /* source string being compared*/
  xmllib_decl_const_e_type  expval,      /* Expected value const        */
  xmllib_error_e_type      *error_code
)
{
  int32  ret           = XMLLIB_SUCCESS;
  char * string        = NULL;
  uint32 source_len    = 0;
  boolean quotes_exist = FALSE;

  /*---------------------------------------------------------------------
    verify parameters received
  ---------------------------------------------------------------------*/
  if( NULL == source     || 
      NULL == error_code || 
      XMLLIB_CONST_MAX <= expval )
  {
    ret = XMLLIB_ERROR;
    if(NULL != error_code)
    {
      *error_code = XMLLIB_ERROR_INVALID_ARGUMENTS;
    }
    return ret;
  }

  string     = source->string;
  source_len = source->len;
  /*---------------------------------------------------------------------
    Quote handling
  ---------------------------------------------------------------------*/
  switch(const_tbl[expval].quote_req)
  {
    case XMLLIB_TEXT_QUOTE_REQ:
      if( XMLLIB_DOUBLE_QUOTE_UCS != string[0] && 
          XMLLIB_SINGLE_QUOTE_UCS != string[0] )
      {
        ret = XMLLIB_ERROR;
        *error_code = XMLLIB_ERROR_UNEXPECTED_TEXT ;
        return ret;
      }
      else
      {
        quotes_exist = TRUE;
      }
      break;
    case XMLLIB_TEXT_QUOTE_OPT:
      if( XMLLIB_DOUBLE_QUOTE_UCS == string[0] || 
          XMLLIB_SINGLE_QUOTE_UCS == string[0] ) 
      {
        quotes_exist = TRUE;
      }
      break;
    case XMLLIB_TEXT_QUOTE_NOT_REQ:
      break;
    default:
      XMLLIBI_DEBUG_ASSERT(0);
      break;
  }
  
  if( TRUE == quotes_exist )
  {
    string++;
    source_len -= 2;
    if( XMLLIB_DOUBLE_QUOTE_UCS != source->string[source->len-1] && 
        XMLLIB_SINGLE_QUOTE_UCS != source->string[source->len-1] )
    {
      ret = XMLLIB_ERROR;
      *error_code = XMLLIB_ERROR_UNEXPECTED_TEXT ;
    }
  }

  /*---------------------------------------------------------------------
    compare
  ---------------------------------------------------------------------*/
  if ( source_len != strlen(const_tbl[expval].str) || 
       0 != strncmp( const_tbl[expval].str, string, source_len) ) 
  {
    ret = XMLLIB_ERROR;
    *error_code = XMLLIB_ERROR_UNEXPECTED_TEXT ;
  }

  return ret;
} /* xmllibi_decl_utf8_check_value_const() */

#endif  /* FEATURE_XMLLIB */
