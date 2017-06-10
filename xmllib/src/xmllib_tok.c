/*===========================================================================

                  X M L   T O K E N I Z E R   L I B R A R Y
                   
DESCRIPTION
 This file implements the XML tokenizer library.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2005-2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllib_tok.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/29/06   ssingh  Added support for UTF-8 encoding - extended 
                   xmllibi_tok_hdlr_tbl, xmllibi_tok_gen_tbl
09/01/05   jsh     Added xmllib_tok_check_value function
10/30/03   ifk     Created module.

===========================================================================*/
/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include "comdef.h"

#ifdef FEATURE_XMLLIB
#include "xmllib_common.h"
#include "xmllib_tok.h"
#include "xmllibi_tok_ascii.h"
#include "xmllibi_tok_utf8.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


/*===========================================================================
                      LOCAL DATA STRUCTURE DEFINITIONS 
===========================================================================*/
/*===========================================================================
FUNCTION POINTER TYPEDEF XMLLIBI_TOK_HDLR_FPTR_TYPE

DESCRIPTION
  This is the prototype of the function called to handle specific XML
  sub-structures.
  
DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR    in case of an error with the error code set in the
                  error_code argument.
  XMLLIB_SUCCESS  in case of success with the relevant token set in the
                  token argument and the bytes in the token returned in the
                  token_bytes argument.

SIDE EFFECTS
  None
===========================================================================*/
typedef int32 (*xmllibi_tok_hdlr_fptr_type)
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* The token returned          */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
);

/*---------------------------------------------------------------------------
  Array of function pointers to handle XML sub-structures for different
  encodings.  The array is of type [encoding][tokenizer state]
---------------------------------------------------------------------------*/
static xmllibi_tok_hdlr_fptr_type
  xmllibi_tok_hdlr_tbl[][XMLLIB_TOKSTATE_MAX] =
{
  { /* ASCII encoding */
    xmllibi_tok_ascii_entity,
    xmllibi_tok_ascii_tag,
    xmllibi_tok_ascii_cdata,
    xmllibi_tok_ascii_dtd,
    xmllibi_tok_ascii_comment,
    xmllibi_tok_ascii_pi,
    xmllibi_tok_ascii_xmldecl
  },
  { /* UTF-8 encoding */
    xmllibi_tok_utf8_entity,
    xmllibi_tok_utf8_tag,
    xmllibi_tok_utf8_cdata,
    xmllibi_tok_utf8_dtd,
    xmllibi_tok_utf8_comment,
    xmllibi_tok_utf8_pi,
    xmllibi_tok_utf8_xmldecl
  }
};

/*---------------------------------------------------------------------------
  Array of function pointers to handle XML tokens (check the value) for 
  different  encodings.  The array is of type [encoding]
---------------------------------------------------------------------------*/
static const xmllibi_tok_gen_table_type *
  xmllibi_tok_gen_tbl[XMLLIB_ENCODING_MAX] =
{
    xmllibi_tok_gen_ascii_table,
    xmllibi_tok_gen_utf8_table
} ;


/*===========================================================================
                       EXTERNAL FUNCTION DEFINITIONS
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIB_TOK_TOKENIZE

DESCRIPTION
  This function is passed XML text which it tokenizes and returns the
  token.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR    in case of an error with the error code set in the
                  error_code argument.
  XMLLIB_SUCCESS  in case of success with the relevant token set in the
                  token argument and the bytes in the token returned in the
                  token_bytes argument.

SIDE EFFECTS
  None
===========================================================================*/
int32 xmllib_tok_tokenize
(
  xmllib_encoding_e_type   encoding,        /* Encoding used               */
  xmllib_tok_state_e_type  tokstate,        /* Tokenizer state             */
  xmllib_metainfo_s_type  *metainfo,        /* XML meta information        */
  xmllib_token_e_type     *token,           /* Token being returned        */
  uint32                  *token_bytes,     /* Number of bytes in token    */
  xmllib_error_e_type     *error_code       /* Error code                  */
)
{
  int32 ret = XMLLIB_SUCCESS;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify arguments
    a) encoding is supported
    b) tokstate is in valid range
    c) metainfo structure is not NULL
    d) metainfo->xmltext is not NULL
    e) metainfo->peekbytes function is not NULL
    f) token cannot be NULL
    g) token_bytes cannot be NULL
    h) error_code cannot be NULL
    i) there is a handler function for the encpding and the state
  -------------------------------------------------------------------------*/
  if( XMLLIB_ENCODING_MAX <= encoding            ||
      XMLLIB_TOKSTATE_MAX <= tokstate            ||
      NULL                == metainfo            ||
      NULL                == metainfo->xmltext   ||
      NULL                == metainfo->peekbytes ||
      NULL                == token               ||
      NULL                == token_bytes         ||
      NULL                == error_code          ||
      NULL                == xmllibi_tok_hdlr_tbl[encoding][tokstate] )
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
      Call the appropriate function based on the encoding and tokenizer state
      values passed
    -----------------------------------------------------------------------*/
    ret = xmllibi_tok_hdlr_tbl[encoding][tokstate]( metainfo,
                                                    token,
                                                    token_bytes,
                                                    error_code );
  }

  return ret;
} /* xmllib_tok_tokenize */

/*===========================================================================
FUNCTION XMLLIB_TOK_GENERATE

DESCRIPTION
  This function generates a specified token and inserts at the desired
  location.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR if any error occured while generating the token, error code
               is set in error_code argument
  XMLLIB_SUCCESS with bytes inserted at the location specified by metainfo

SIDE EFFECTS
  token bytes are inserted at the location provided by metainfo
===========================================================================*/
int32 xmllib_tok_generate
(
  xmllib_encoding_e_type      encoding,     /* encoding                    */
  xmllib_token_e_type         token,        /* token to be generated       */
  xmllib_string_s_type       *bytes,        /* token bytes                 */
  xmllib_gen_metainfo_s_type *metainfo,     /* XML meta information        */
  uint32                     *bytesgen,     /* bytes generated             */
  xmllib_error_e_type        *error_code    /* error code, if any          */
)
{
  int32                        ret = XMLLIB_SUCCESS ;
  xmllibi_tok_gen_table_type const * tok_ent;
  /*-------------------------------------------------------------------------
    Verify arguments
    a) encoding is supported
    b) metainfo structure is not NULL
    c) metainfo->xmltext is not NULL
    d) metainfo->putbytes function is not NULL
    e) error_code cannot be NULL
    f) there is a tok gen function for the encpding
  -------------------------------------------------------------------------*/
  if ( XMLLIB_ENCODING_MAX <= encoding ||
       NULL == metainfo               ||
       NULL == metainfo->xmltext      ||
       NULL == metainfo->putbytes     ||
       NULL == error_code             ||
       NULL == xmllibi_tok_gen_tbl[encoding] )
  {
    if( NULL != error_code )
    {
      *error_code = ( XMLLIB_ENCODING_MAX <= encoding ) ? 
                      XMLLIB_ERROR_UNSUP_ENCODING :
                      ( XMLLIB_TOK_MAX <= token ) ? 
                        XMLLIB_ERROR_INVALID_TOKEN :
                        XMLLIB_ERROR_INVALID_ARGUMENTS;
    }
    ret = XMLLIB_ERROR;
  }
  else
  {
    tok_ent = &xmllibi_tok_gen_tbl[encoding][token];
    if ( FALSE == tok_ent->tok_supported )
    {
      *error_code = XMLLIB_ERROR_INVALID_TOKEN;
      ret = XMLLIB_ERROR;
    }
    else if ( NULL != tok_ent->prefix && 0 != tok_ent->size )
    {
      ret = metainfo->putbytes(metainfo,
                               tok_ent->size,
                               tok_ent->prefix,
                               bytesgen);
      if ( ret != XMLLIB_SUCCESS )
      {
        *error_code = XMLLIB_ERROR_NOMEM;
      }
    }
    if ( XMLLIB_SUCCESS == ret && TRUE == tok_ent->bytes_used )
    {
      if(NULL != bytes && NULL != bytes->string)
      {
        ret = metainfo->putbytes(metainfo,
                                 bytes->len,
                                 bytes->string,
                                 bytesgen);
        if ( ret != XMLLIB_SUCCESS )
        {
          *error_code = XMLLIB_ERROR_NOMEM;
        }
      }
      else
      {
        XMLLIBI_DEBUG_ASSERT(0);
        ret = XMLLIB_ERROR;
      }
    }
  }

  return ret ;
} /* xmllib_tok_generate() */

/*===========================================================================
FUNCTION XMLLIB_COMMON_FORMAT_LOG_MSG

DESCRIPTION
  Format debug message for logging.
  
DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
void xmllib_common_format_log_msg
(
  char *buf_ptr,
  int buf_size,
  char *fmt,
  ...
)
{
  va_list ap;

  /*-----------------------------------------------------------------------*/

  XMLLIBI_DEBUG_ASSERT( buf_ptr != NULL );
  XMLLIBI_DEBUG_ASSERT( buf_size > 0 );

  /*-----------------------------------------------------------------------*/

  va_start( ap, fmt );

  vsnprintf( buf_ptr, buf_size, fmt, ap );

  va_end( ap );

} /* xmllib_common_format_log_msg */

#endif  /* FEATURE_XMLLIB */
