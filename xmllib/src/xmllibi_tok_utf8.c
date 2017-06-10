/*===========================================================================

                  X M L   T O K E N I Z E R   L I B R A R Y
                        U T F - 8   E N C O D I N G
                   
DESCRIPTION
 This file implements the XML library's UTF-8 encoded text tokenizer.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2006 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllibi_tok_utf8.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/29/06   ssingh  Created module.

===========================================================================*/
/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include "comdef.h"

#ifdef FEATURE_XMLLIB
#include "xmllib_common.h"
#include "xmllib_tok.h"
#include "xmllibi_tok_utf8.h"
#include "xmllibi_utf8_util.h"
#include "assert.h"
#include <string.h>

/*===========================================================================
                      INTERNAL FUNCTION DEFINITIONS
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_TOK_UTF8_CHECKVALUE

DESCRIPTION
  This function is called with a UCS (Unicode Character Set) value to check 
  if it is present in the stream at the specified offset.
  
DEPENDENCIES
  None.

RETURN VALUE
  TRUE   if token found at the specified offset
  FALSE  if the ucsvalue is not found at the offset

SIDE EFFECTS
  None
===========================================================================*/
static int32 xmllibi_tok_utf8_checkvalue
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  uint32                  offset,           /* Offset in XML text          */
  uint32                  expvalue          /* Expected UCS value         */
)
{
  int32                 ret;
  uint32                ucsvalue;
  uint32                utf8_bytes;
  xmllib_error_e_type   errorcode;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );

  ret = ( XMLLIB_SUCCESS == xmllibi_utf8_peekbytes( metainfo,
                                                    offset,
                                                    &ucsvalue,
                                                    &utf8_bytes,
                                                    &errorcode ) &&
          ucsvalue == expvalue ) ?
          TRUE                    :
          FALSE;

  return ret;
} /* xmllib_tok_utf8_checkvalue() */


/*===========================================================================
FUNCTION XMLLIBI_TOK_UTF8_START_TOKEN

DESCRIPTION
  This function is called to determine what kind of token is prsent next in
  the stream.
  
DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR    In case of an error with the error code set in error_code
                  argument.
  XMLLIB_SUCCESS  In case of success with the token set in token  argument
                  and the number of bytes in the token returned in
                  token_bytes argument.

SIDE EFFECTS
  None
===========================================================================*/
int32 xmllibi_tok_utf8_start_token
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  int32                 ret;
  uint32                ucsvalue;
  uint32                utf8_bytes;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  /*-------------------------------------------------------------------------
    Verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != token );
  XMLLIBI_DEBUG_ASSERT( NULL != token_bytes );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  *token_bytes = 0;
  *token       = XMLLIB_TOK_INVALID;

  /*-------------------------------------------------------------------------
    Peek at the next byte in the stream.  If XMLLIB_ERROR is returned then
    set token to XMLLIB_TOK_EOF if no token has been acquired yet.
    -------------------------------------------------------------------------*/
  ret = xmllibi_utf8_peekbytes( metainfo,
                                *token_bytes,
                                &ucsvalue,
                                &utf8_bytes,
                                error_code );
  *token_bytes += utf8_bytes;

  if( XMLLIB_ERROR == ret && *error_code != XMLLIB_ERROR_INVALID_ENCODING)
  {
      *token = XMLLIB_TOK_EOF;
      ret    = XMLLIB_SUCCESS;
  }
  else
  {
    switch( ucsvalue )
    {
    case '<':
      /*-------------------------------------------------------------------
        '<' is used to start a tag
        -------------------------------------------------------------------*/
      *token = XMLLIB_TOK_TAG_OPEN;
      break;

    case '>':
      /*-------------------------------------------------------------------
        '>' is used to end the tag
        -------------------------------------------------------------------*/
      *token = XMLLIB_TOK_TAG_CLOSE;
      break;

    case ' ':
    case '\t':
    case '\n':
    case '\r':
      /*-----------------------------------------------------------------
        For whitespace set token to XMLLIB_TOK_SPACE and find number of
        bytes of whitespace
        -----------------------------------------------------------------*/
      *token = XMLLIB_TOK_SPACE;
      do
      {
        ret = xmllibi_utf8_peekbytes( metainfo,
                                      *token_bytes,
                                      &ucsvalue,
                                      &utf8_bytes,
                                      error_code );
        *token_bytes += utf8_bytes;

      } while( XMLLIB_SUCCESS == ret &&
               (' '  == ucsvalue || '\t' == ucsvalue ||
                '\r' == ucsvalue || '\n' == ucsvalue ));
      *token_bytes -= utf8_bytes; /* decrement last byte which is not space */

      ret = XMLLIB_SUCCESS;
      break;

    default:
      /*-------------------------------------------------------------------
        Set token to XMLLIB_TOK_CONTENT and return.
        -------------------------------------------------------------------*/
      *token = XMLLIB_TOK_CONTENT;
      break;
    } /* switch */
  }

  return ret;
} /* xmllibi_tok_utf8_start_token() */


/*===========================================================================
                      EXTERNAL FUNCTION DEFINITIONS
===========================================================================*/
/*===========================================================================
                      TOKEN PARSER FUNCTIONS
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_TOK_UTF8_DTD

DESCRIPTION
  This function is called to handle DTD tag in UTF-8 encoding.
  
DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR    In case of an error with the error code set in error_code
                  argument.
  XMLLIB_SUCCESS  In case of success with the token set in token  argument
                  and the number of bytes in the token returned in
                  token_bytes argument.

SIDE EFFECTS
  None
===========================================================================*/
int32 xmllibi_tok_utf8_dtd
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint32              ucsvalue;
  uint32              utf8_bytes;
  int32               ret        = XMLLIB_SUCCESS;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /* TODO: incomplete for now */
  /* TODO: Handle PEReference, conditional sections, external ID */
  /*-------------------------------------------------------------------------
    Verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != token );
  XMLLIBI_DEBUG_ASSERT( NULL != token_bytes );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  *token_bytes = 0;

  /*-------------------------------------------------------------------------
    Call xmllibi_tok_utf8_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_utf8_start_token( metainfo,
                                      token,
                                      token_bytes,
                                      error_code );

  if( XMLLIB_SUCCESS == ret )
  {
    switch( *token )
    {
      /*---------------------------------------------------------------------
        For these returned tokens the token is already complete, return it.
        ---------------------------------------------------------------------*/
    case XMLLIB_TOK_SPACE:
    case XMLLIB_TOK_TAG_OPEN:
    case XMLLIB_TOK_TAG_CLOSE:
      break;

      /*---------------------------------------------------------------------
        A return of XMLLIB_TOK_CONTENT requires further processing
        ---------------------------------------------------------------------*/
    case XMLLIB_TOK_CONTENT:
      *token_bytes = 0;
      ret = xmllibi_utf8_peekbytes( metainfo,
                                    *token_bytes,
                                    &ucsvalue,
                                    &utf8_bytes,
                                    error_code );
      *token_bytes += utf8_bytes;

      if( XMLLIB_SUCCESS == ret )
      {
        switch( ucsvalue )
        {
        case '[':
          /*-------------------------------------------------------------
            '[' is used to start the markup declaration section
            -------------------------------------------------------------*/
          *token = XMLLIB_TOK_DTD_MUP_OPEN;
          break;

        case '\'':
        case '"':
          /*-------------------------------------------------------------
            The quotation marks open the system or pubid literal
            -------------------------------------------------------------*/
          /* TODO:  Add this to list of tokens */
          //*token = XMLLIB_TOK_LITERAL;
          break;

        default:
          /*-------------------------------------------------------------
            This token is a name.  Check that the name is validly formed
            [XML 1.0 spec production 5].
            -------------------------------------------------------------*/
          *token = XMLLIB_TOK_NAME;
          if( ('A' <= ucsvalue && 'Z' >= ucsvalue) ||
              ('a' <= ucsvalue && 'z' >= ucsvalue) ||
              '_' == ucsvalue || ':' == ucsvalue )
          {
            do
            {
              ret = xmllibi_utf8_peekbytes( metainfo,
                                            *token_bytes,
                                            &ucsvalue,
                                            &utf8_bytes,
                                            error_code );
              *token_bytes += utf8_bytes;

            }
            while( XMLLIB_SUCCESS == ret &&
                   (('A' <= ucsvalue && 'Z' >= ucsvalue) ||
                    ('a' <= ucsvalue && 'z' >= ucsvalue) ||
                    ('0' <= ucsvalue && '9' >= ucsvalue) ||
                    '.' == ucsvalue || '-' == ucsvalue   ||
                    '_' == ucsvalue || 0xB7 == ucsvalue) );
//PJB this is broken   There is a bunch of stuff other than 0xB7 that is valid for UTF-8.

#if 0
            /*-----------------------------------------------------------
              The delimiter for a name is either space or the '>'
              charachter.  Check for the delimiter.
              -----------------------------------------------------------*/
            if( XMLLIB_SUCCESS == ret && ('>'  != ucsvalue &&
                                          ' '  != ucsvalue &&
                                          '\t' != ucsvalue &&
                                          '\n' != ucsvalue &&
                                          '\r' != ucsvalue ) )
            {
              *token = XMLLIB_TOK_INVALID;
            }
            /* Possible TODO:  Add handling of SYSTEM or PUBLIC */
#endif
          }
          else
          {
            *token = XMLLIB_TOK_INVALID;
          }
          break;
        } /* switch( ucsvalue ) */
      }
      break;

    default:
      *token = XMLLIB_TOK_INVALID;
      break;
    } /* switch( *token ) */
  }

  return ret;
} /* xmllibi_tok_utf8_dtd() */


#if 0
/*===========================================================================
FUNCTION XMLLIBI_TOK_UTF8_DTD_MARKUP

DESCRIPTION
  This function is called to handle markup section in DTD in UTF-8 encoding.
  
DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR    In case of an error with the error code set in error_code
                  argument.
  XMLLIB_SUCCESS  In case of success with the token set in token  argument
                  and the number of bytes in the token returned in
                  token_bytes argument.

SIDE EFFECTS
  None
===========================================================================*/
int32 xmllibi_tok_utf8_dtd_markup
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint32               ucsvalue;
  uint32               tmpucsvalue;
  int32                ret;
  uint8               *comparestr;
  int32                index;
  int32                loopflag;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /* TODO: incomplete for now */
  /*-------------------------------------------------------------------------
    Verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != token );
  XMLLIBI_DEBUG_ASSERT( NULL != token_bytes );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  *token_bytes = 0;

  /*-------------------------------------------------------------------------
    Look at the XML text byte at a time and acquire the token.
    -------------------------------------------------------------------------*/
  do
  {
    /*-----------------------------------------------------------------------
      Peek at the next byte in the stream.  If XMLLIB_ERROR is returned
      then set token to XMLLIB_TOK_EOF if no token has been acquired yet.
      -----------------------------------------------------------------------*/
    ret = xmllib_tok_peekbyte( xmltext, (*bytes)++, &ucsvalue );

    if( XMLLIB_ERROR == ret )
    {
      if( XMLLIB_TOK_INVALID == token )
      {
        token = XMLLIB_TOK_EOF;
      }
      break;
    }

    /* TODO: Handle PEReference, conditional sections, external ID */
    switch( ucsvalue )
    {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      /*-------------------------------------------------------------------
        For whitespace, if it is not inside content set token to
        XMLLIB_TOK_SPACE and find number of bytes of whitespace.  Otherwise
        white space is inside content and no special processing is done.
        --------------------------------------------------------------------*/
      if( XMLLIB_TOK_INVALID == token )
      {
        token = XMLLIB_TOK_SPACE;
        do
        {
          ret = xmllib_tok_peekbyte( xmltext,
                                     (*bytes)++,
                                     &ucsvalue);
          if( XMLLIB_ERROR == ret )
          {
            loopflag = FALSE; 
          }
          else
          {
            if( ' ' != ucsvalue && '\t' != ucsvalue &&
                '\r' != ucsvalue && '\n' != ucsvalue )
            {
              loopflag = FALSE; 
            }
          }
        } while( TRUE == loopflag );
        break;
      }
      /* else fallthrough */

    case '>':
      /*-------------------------------------------------------------------
        '>' is used to end the tag
        --------------------------------------------------------------------*/
      if( XMLLIB_TOK_INVALID == token )
      {
        token = XMLLIB_TOK_TAG_CLOSE;
      }
      loopflag = FALSE; 
      break;

    case ']':
      /*-------------------------------------------------------------------
        ']' is used to end the markup declaration section
        --------------------------------------------------------------------*/
      if( XMLLIB_TOK_INVALID == token )
      {
        token = XMLLIB_TOK_DTD_MUP_CLOSE;
      }
      loopflag = FALSE; 
      break;

    case '<':
      /*-------------------------------------------------------------------
        One of <!ELEMENT, <!ATTLIST, <!ENTITY or <!NOTATION
        -------------------------------------------------------------------*/
      if( XMLLIB_TOK_INVALID == token )
      {
        if( TRUE == xmllibi_tok_utf8_checkvalue( xmltext,
                                                  (*bytes)++,
                                                  '!' ) &&
            XMLLIB_SUCCESS == xmllib_tok_peekbyte( xmltext,
                                                   (*bytes)++,
                                                   &tmpbytevalue ))
        {
          switch( tmpbytevalue )
          {
          case 'E':
            if( TRUE == xmllibi_tok_utf8_checkvalue( xmltext,
                                                      *bytes,
                                                      'L' ) )
            {
              comparestr = "ELEMENT";
              token      = XMLLIB_TOK_DTD_OPEN;
            }
            else
            {
              comparestr = "ENTITY";
              token      = XMLLIB_TOK_DTD_OPEN;
            }
            break;

          case 'A':
            comparestr = "ATTLIST";
            token      = XMLLIB_TOK_CDATA_OPEN;
            break;

          case '-':
            comparestr = "--";
            token      = XMLLIB_TOK_COMMENT_OPEN;
            break;

          case 'N':
            comparestr = "NOTATION";
            token      = XMLLIB_TOK_COMMENT_OPEN;
            break;

          case '?':
            comparestr = NULL;
            token      = XMLLIB_TOK_PI_OPEN;
            break;

          default:
            loopflag = FALSE; 
            break;
          }

          while( TRUE == loopflag && NULL != *comparestr )
          {
            loopflag = xmllibi_tok_utf8_checkvalue( xmltext,
                                                     *bytes + index,
                                                     *comparestr );
            index++; comparestr++;
          } /* while */

          /* TODO: Also check for comparestr being NULL */
          if( NULL == *comparestr )
          {
            *bytes += index+1;
          }
          else
          {
            token       = XMLLIB_TOK_INVALID;
            *error_code = XMLLIB_ERROR_INVALID_TEXT;
          }
        }
        else
        {
          token       = XMLLIB_TOK_INVALID;
          *error_code = XMLLIB_ERROR_INVALID_TEXT;
        }
      }
      loopflag = FALSE;
      break;

    default:
      if( XMLLIB_TOK_INVALID == token )
      {
        token = XMLLIB_TOK_CONTENT;
      }

      /*-------------------------------------------------------------------
        Check reference contents for valid charachters.
        TODO:  The char reference cannot have a-f | A-F if it doesn't
        start with &#x.  Pass x in and use it to set a flag
        -------------------------------------------------------------------*/
      if( !(('0' <= ucsvalue && '9' >= ucsvalue) ||
            ('a' <= ucsvalue && 'f' >=ucsvalue)  ||
            ('A' <= ucsvalue && 'F' >=ucsvalue)) )
      {
        token       = XMLLIB_TOK_INVALID;
        *error_code = XMLLIB_ERROR_INVALID_TEXT;
        loopflag = FALSE;
      }
      break;
    } /* switch */
  } while( TRUE == loopflag );

  return ret;
} /* xmllibi_tokenize_utf8_dtd_markup() */
#endif /* if 0 */


/*===========================================================================
FUNCTION XMLLIBI_TOK_UTF8_ENTITY

DESCRIPTION
  This function tokenizes XML entities.  Valid tokens returned include:
  
  XMLLIB_TOK_SPACE
  XMLLIB_TOK_INVALID
  XMLLIB_TOK_TAG_OPEN
  XMLLIB_TOK_ENDTAG_OPEN
  XMLLIB_TOK_XMLDECL_OPEN
  XMLLIB_TOK_PI_OPEN
  XMLLIB_TOK_DTD_OPEN
  XMLLIB_TOK_COMMENT_OPEN
  XMLLIB_TOK_CDATA_OPEN
  XMLLIB_TOK_CONTENT
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR    In case of an error with the error code set in error_code
                  argument.
  XMLLIB_SUCCESS  In case of success with the token set in token  argument
                  and the number of bytes in the token returned in
                  token_bytes argument.

SIDE EFFECTS
  None.
===========================================================================*/
int32 xmllibi_tok_utf8_entity
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint32  ucsvalue;
  uint32  utf8_bytes;
  int32   ret;
  char    *comparestr     = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != token );
  XMLLIBI_DEBUG_ASSERT( NULL != token_bytes );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  *token_bytes = 0;

  /*-------------------------------------------------------------------------
    Call xmllibi_tok_utf8_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_utf8_start_token( metainfo,
                                      token,
                                      token_bytes,
                                      error_code );

  if( XMLLIB_SUCCESS == ret )
  {
    switch( *token )
    {
      /*---------------------------------------------------------------------
        For these returned tokens the token is already complete, return it.
        ---------------------------------------------------------------------*/
    case XMLLIB_TOK_SPACE:
    case XMLLIB_TOK_EOF:
      break;

    case XMLLIB_TOK_TAG_OPEN:
      /* TODO: Process further */
      ret = xmllibi_utf8_peekbytes( metainfo,
                                    *token_bytes,
                                    &ucsvalue,
                                    &utf8_bytes,
                                    error_code );

      if( XMLLIB_SUCCESS == ret )
      {
        switch( ucsvalue )
        {
        case '/':
          *token = XMLLIB_TOK_ENDTAG_OPEN;
          (*token_bytes)++;
          break;

        case '?':
          *token = XMLLIB_TOK_PI_OPEN;
          (*token_bytes)++;

          if( TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                   *token_bytes,
                                                   'x' ) &&
              TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                   *token_bytes + 1,
                                                   'm' ) &&
              TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                   *token_bytes + 2,
                                                   'l' ) )
          {
            ret = xmllibi_utf8_peekbytes( metainfo,
                                          *token_bytes + 3,
                                          &ucsvalue,
                                          &utf8_bytes,
                                          error_code);
            if( XMLLIB_SUCCESS == ret &&
                !(('A' <= ucsvalue && 'Z' >= ucsvalue) ||
                  ('a' <= ucsvalue && 'z' >= ucsvalue) ||
                  ('0' <= ucsvalue && '9' >= ucsvalue) ||
                  '.' == ucsvalue || '-' == ucsvalue   ||
                  ':' == ucsvalue || '_' == ucsvalue))
              // PJB there is other chars that are valid here
            {
              *token = XMLLIB_TOK_XMLDECL_OPEN;
              *token_bytes += 3;
              break;
            }

          }
          break;

        case '!':
          /*-----------------------------------------------------------
            If a <! found then the only valid tags should be <![CDATA[,
            <!DOCTYPE or <!-- otherwise malformed text.
            -----------------------------------------------------------*/
          ret = xmllibi_utf8_peekbytes( metainfo,
                                        *token_bytes + 1,
                                        &ucsvalue,
                                        &utf8_bytes,
                                        error_code );
          *token_bytes += utf8_bytes;

          if( XMLLIB_SUCCESS == ret )
          {
            switch( ucsvalue )
            {
            case 'D':
              comparestr = "DOCTYPE";
              *token     = XMLLIB_TOK_DTD_OPEN;
              break;

            case '[':
              comparestr = "[CDATA[";
              *token     = XMLLIB_TOK_CDATA_OPEN;
              break;

            case '-':
              comparestr = "--";
              *token     = XMLLIB_TOK_COMMENT_OPEN;
              break;

            default:
              *token = XMLLIB_TOK_INVALID;
              break;
            }

            if( NULL != comparestr )
            {
              while( 0 != *comparestr &&
                     TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                          (*token_bytes)++,
                                                          *(comparestr++) ) );

              if( 0 != *comparestr )
              {
                *token = XMLLIB_TOK_INVALID;
              }
            }
          }
          break;

        default:
          break;
        } /* switch( ucsvalue ) */
      }
      break;

    case XMLLIB_TOK_CONTENT:
      /*-------------------------------------------------------------------
        Find the length of content verifying that it doesn't contain ']]>'
        [XML spec 1.0 production 14]
        -------------------------------------------------------------------*/
      do
      {
          ret = xmllibi_utf8_peekbytes( metainfo,
                                        *token_bytes,
                                        &ucsvalue,
                                        &utf8_bytes,
                                        error_code );
          *token_bytes += utf8_bytes;

        if( XMLLIB_SUCCESS == ret && ']' == ucsvalue &&
            TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                 *token_bytes,
                                                 ']' ) &&
            TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                 *token_bytes + 1,
                                                 '>' ) )

        {
          *token = XMLLIB_TOK_INVALID;
          break;
        }
      } while( XMLLIB_SUCCESS == ret && '<' != ucsvalue );

      /* *token_bytes -= 1; */
      *token_bytes -= utf8_bytes;
      
      break;

    default:
      *token = XMLLIB_TOK_INVALID;
      break;
    } /* switch( *token ) */
  }

  return ret;
} /* xmllib_tok_utf8_entity() */


/*===========================================================================
FUNCTION XMLLIBI_TOK_UTF8_TAG

DESCRIPTION
  This function tokenizes contents of an XML tag encoded in UTF-8.  Valid
  tokens returned include:

  XMLLIB_TOK_SPACE
  XMLLIB_TOK_INVALID
  XMLLIB_TOK_TAG_CLOSE
  XMLLIB_TOK_EMPTYTAG_CLOSE
  XMLLIB_TOK_EQ
  XMLLIB_TOK_ATTRIBUTE_VALUE
  XMLLIB_TOK_NAME
  XMLLIB_TOK_XMLDECL_CLOSE
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR    In case of an error with the error code set in error_code
                  argument.
  XMLLIB_SUCCESS  In case of success with the token set in token  argument
                  and the number of bytes in the token returned in
                  token_bytes argument.

SIDE EFFECTS
  None.
===========================================================================*/
int32 xmllibi_tok_utf8_tag
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint32              ucsvalue;
  uint8                quotevalue;
  int32               ret;
  uint32              utf8_bytes;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  /*-------------------------------------------------------------------------
    Verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != token );
  XMLLIBI_DEBUG_ASSERT( NULL != token_bytes );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  *token_bytes = 0;

  /*-------------------------------------------------------------------------
    Call xmllibi_tok_utf8_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_utf8_start_token( metainfo,
                                      token,
                                      token_bytes,
                                      error_code );

  if( XMLLIB_SUCCESS == ret )
  {
    switch( *token )
    {
      /*---------------------------------------------------------------------
        For these returned tokens the token is already complete, return it.
        ---------------------------------------------------------------------*/
    case XMLLIB_TOK_SPACE:
    case XMLLIB_TOK_TAG_CLOSE:
      break;

    case XMLLIB_TOK_CONTENT:
      *token_bytes = 0;
      ret = xmllibi_utf8_peekbytes( metainfo,
                                    *token_bytes,
                                    &ucsvalue,
                                    &utf8_bytes,
                                    error_code );
      *token_bytes += utf8_bytes;

      /*-------------------------------------------------------------------
        Find the length of content verifying that it doesn't contain ']]>'
        [XML spec 1.0 production 14]
        -------------------------------------------------------------------*/
      switch( ucsvalue )
      {
      case '/':
        /*---------------------------------------------------------------
          Check if this is the empty tag close '/>'
          ---------------------------------------------------------------*/
        *token = (TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                       (*token_bytes)++,
                                                       '>' )) ?
          XMLLIB_TOK_EMPTYTAG_CLOSE : XMLLIB_TOK_INVALID;
        break;

      case '=':
        /*---------------------------------------------------------------
          = is used to seperate attribute from its value
          ----------------------------------------------------------------*/
        *token = XMLLIB_TOK_EQ;
        break;

      case '\'':
      case '"':
        /*---------------------------------------------------------------
          An attribute value is enclosed in either ' or ".  An attribute
          value can contain a quotation mark of a type different from
          the one being used to delimit the attribute value.
          ----------------------------------------------------------------*/
        quotevalue = ucsvalue;
        *token     = XMLLIB_TOK_ATTRIB_VALUE; 
        do
        {
          ret = xmllibi_utf8_peekbytes( metainfo,
                                        *token_bytes,
                                        &ucsvalue,
                                        &utf8_bytes,
                                        error_code );
          *token_bytes += utf8_bytes;

        } while( XMLLIB_SUCCESS == ret && quotevalue != ucsvalue );
        break;
      case '?':
        /*---------------------------------------------------------------
          This could be either XMLDECL_CLOSE or PI_CLOSE token. As both 
          constants have same value (10), the caller function will
          receive success for any of those tokens.
          ----------------------------------------------------------------*/
        *token = (TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                       (*token_bytes)++,
                                                       '>' )) ?
          XMLLIB_TOK_XMLDECL_CLOSE : XMLLIB_TOK_INVALID;
        break;

      default:
        /*---------------------------------------------------------------
          This is a name.  A name can start with a letter or an '_' or
          a ':'. [XML spec 1.0 production 5]
          ---------------------------------------------------------------*/
        *token = ( ('A' <= ucsvalue && 'Z' >= ucsvalue) ||
                   ('a' <= ucsvalue && 'z' >= ucsvalue) ||
                   '_' == ucsvalue || ':' == ucsvalue ) ?
          XMLLIB_TOK_NAME : XMLLIB_TOK_INVALID;

        if( XMLLIB_TOK_NAME == *token )
        {
          /*-------------------------------------------------------------
            A name can contain a letter, a digit, '.', '-', '_' or an
            extender 0xB7 => TODO: figure out
            -------------------------------------------------------------*/
          do
          {
            ret = xmllibi_utf8_peekbytes( metainfo,
                                          *token_bytes,
                                          &ucsvalue,
                                          &utf8_bytes,
                                          error_code );
            *token_bytes += utf8_bytes;

          } while( XMLLIB_SUCCESS == ret &&
                   (('A' <= ucsvalue && 'Z' >= ucsvalue) ||
                    ('a' <= ucsvalue && 'z' >= ucsvalue) ||
                    ('0' <= ucsvalue && '9' >= ucsvalue) ||
                    '.' == ucsvalue || '-' == ucsvalue   ||
                    ':' == ucsvalue ||
                    '_' == ucsvalue || 0xB7 == ucsvalue) );
//PJB this is broken   There is a bunch of stuff other than 0xB7 that is valid for UTF-8.


          /* *token_bytes -= 1;*/
          *token_bytes -= utf8_bytes;

#if 0
          /*-------------------------------------------------------------
            The delimiter for a name is either space, '>' or '/>'
            charachters.  Check for the delimiter.
            -------------------------------------------------------------*/
          if( XMLLIB_SUCCESS == ret && ('>'  != ucsvalue &&
                                        '/'  != ucsvalue &&
                                        ' '  != ucsvalue &&
                                        '='  != ucsvalue &&
                                        '\t' != ucsvalue &&
                                        '\n' != ucsvalue &&
                                        '\r' != ucsvalue ) )
          {
            *token = XMLLIB_TOK_INVALID;
          }
#endif
        }
        break;
      } /* switch( ucsvalue ) */
      break;

    default:
      *token = XMLLIB_TOK_INVALID;
      break;
    } /* switch( *token ) */
  }

  return ret;
} /* xmllib_tok_utf8_tag() */


/*===========================================================================
FUNCTION XMLLIBI_TOK_UTF8_CDATA

DESCRIPTION
  This function is called to tokenize the CDATA section.  Valid tokens
  include:
  
  XMLLIB_TOK_CDATA_CLOSE
  XMLLIB_TOK_INVALID
  XMLLIB_TOK_CONTENT
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR    In case of an error with the error code set in error_code
                  argument.
  XMLLIB_SUCCESS  In case of success with the token set in token  argument
                  and the number of bytes in the token returned in
                  token_bytes argument.

SIDE EFFECTS
  None.
===========================================================================*/
int32 xmllibi_tok_utf8_cdata
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint32    ucsvalue;
  int32     ret;
  uint32    utf8_bytes;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != token );
  XMLLIBI_DEBUG_ASSERT( NULL != token_bytes );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  *token_bytes = 0;

  /*-------------------------------------------------------------------------
    Call xmllibi_tok_utf8_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_utf8_start_token( metainfo,
                                      token,
                                      token_bytes,
                                      error_code );

  /* Any token is content inside <![CDATA[ tag, adding remaining cases
     for all possible token types returned by start_token function */
  if( XMLLIB_SUCCESS == ret )
  {
    switch( *token )
    {
      /*---------------------------------------------------------------------
        return of any tag requires further processing (no mark-up here - its 
        CDATA ) any thing and every thing is plain character data
        ---------------------------------------------------------------------*/
    case XMLLIB_TOK_TAG_OPEN:
    case XMLLIB_TOK_TAG_CLOSE:
    case XMLLIB_TOK_SPACE:
    case XMLLIB_TOK_CONTENT:
      *token_bytes = 0;
      ret = xmllibi_utf8_peekbytes( metainfo,
                                    *token_bytes,
                                    &ucsvalue,
                                    &utf8_bytes,
                                    error_code );
      *token_bytes += utf8_bytes;

      if( XMLLIB_SUCCESS == ret )
      {
        if( ']' == ucsvalue &&
            TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                 *token_bytes,
                                                 ']' ) &&
            TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                 *token_bytes + 1,
                                                 '>' ) )
        {
          /*---------------------------------------------------------------
            ']]>' is used to end the CDATA section
            ---------------------------------------------------------------*/
          *token        = XMLLIB_TOK_CDATA_CLOSE;
          *token_bytes += 2;
        }
        else
        {
          /*---------------------------------------------------------------
            This token is CDATA.  The delimiter is ']]>'
            ---------------------------------------------------------------*/
          *token = XMLLIB_TOK_CONTENT;
          do
          {
            ret = xmllibi_utf8_peekbytes( metainfo,
                                          *token_bytes,
                                          &ucsvalue,
                                          &utf8_bytes,
                                          error_code );
          

            if( XMLLIB_SUCCESS == ret &&
                ']' == ucsvalue &&
                TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                     *token_bytes + 1,
                                                     ']' ) &&
                TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                     *token_bytes + 2,
                                                     '>' ) )
            {
              break;
            }

            /* (*token_bytes)++ ; */
            *token_bytes += utf8_bytes;
          }
          while( XMLLIB_SUCCESS == ret );
        }
      }
      break;

    default:
      *token = XMLLIB_TOK_INVALID;
      break;
    } /* switch( *token ) */
  }

  return ret;
} /* xmllib_tok_utf8_cdata() */


/*===========================================================================
FUNCTION XMLLIBI_TOK_UTF8_PI

DESCRIPTION
  This function is called to tokenize processing instruction.  Valid tokens
  are:
  
  XMLLIB_TOK_PI_CLOSE
  XMLLIB_TOK_INVALID
  XMLLIB_TOK_PI_VALUE
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR    In case of an error with the error code set in error_code
                  argument.
  XMLLIB_SUCCESS  In case of success with the token set in token  argument
                  and the number of bytes in the token returned in
                  token_bytes argument.

SIDE EFFECTS
  None.
===========================================================================*/
int32 xmllibi_tok_utf8_pi
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint32    ucsvalue;
  int32     ret;
  uint32    utf8_bytes;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != token );
  XMLLIBI_DEBUG_ASSERT( NULL != token_bytes );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  *token_bytes = 0;
  *token = XMLLIB_TOK_PI_VALUE;

  /*---------------------------------------------------------------
    This token is processing instruction.  The delimiter is '?>' 
  ---------------------------------------------------------------*/
  do
  {
    ret = xmllibi_utf8_peekbytes( metainfo,
                                  *token_bytes,
                                  &ucsvalue,
                                  &utf8_bytes,
                                  error_code );
    
    if( XMLLIB_SUCCESS == ret) 
    {
      if ('?' == ucsvalue &&
           TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                 *token_bytes + 1,
                                                 '>' ))
      {
        if(0 == *token_bytes) 
        {
 	  *token = XMLLIB_TOK_PI_CLOSE;
	  *token_bytes = 2;
        }
        break;
      } 

      (*token_bytes) += utf8_bytes;
    }

  } while( XMLLIB_SUCCESS == ret );

  return ret;
} /* xmllib_tok_utf8_pi() */


/*===========================================================================
FUNCTION XMLLIBI_TOK_UTF8_COMMENT

DESCRIPTION
  This function is called to tokenize an XML comment tag.  Valid tokens
  are:

  XMLLIB_TOK_COMMENT_CLOSE
  XMLLIB_TOK_INVALID
  XMLLIB_TOK_COMMENT_VALUE
  XMLLIB_TOK_SPACE
  
DEPENDENCIES
  Both DSM item pointer and the text string pointer cannot be simultaneously
  non-NULL.

RETURN VALUE
  XMLLIB_ERROR    In case of an error with the error code set in error_code
                  argument.
  XMLLIB_SUCCESS  In case of success with the token set in token  argument
                  and the number of bytes in the token returned in
                  token_bytes argument.

SIDE EFFECTS
  Bytes are removed from the DSM item chain and the item_ptr may change
===========================================================================*/
int32 xmllibi_tok_utf8_comment
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint32    ucsvalue;
  int32     ret;
  uint32    utf8_bytes;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != token );
  XMLLIBI_DEBUG_ASSERT( NULL != token_bytes );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  *token_bytes = 0;

  /*-------------------------------------------------------------------------
    Call xmllibi_tok_utf8_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_utf8_start_token( metainfo,
                                      token,
                                      token_bytes,
                                      error_code );

  if( XMLLIB_SUCCESS == ret )
  {
    switch( *token )
    {
    case XMLLIB_TOK_SPACE:
      break ;
      /*---------------------------------------------------------------------
        A return of XMLLIB_TOK_CONTENT requires further processing
        ---------------------------------------------------------------------*/
    case XMLLIB_TOK_CONTENT:
      *token_bytes = 0;
      ret = xmllibi_utf8_peekbytes( metainfo,
                                    *token_bytes,
                                    &ucsvalue,
                                    &utf8_bytes,
                                    error_code );
          

      if( XMLLIB_SUCCESS == ret )
      {
        if( '-' == ucsvalue &&
            TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                 *token_bytes + 1,
                                                 '-' ) &&
            TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                 *token_bytes + 2,
                                                 '>' ) )
        {
          /*---------------------------------------------------------------
            '-->' is used to end the comment tag 
            ---------------------------------------------------------------*/
          *token        = XMLLIB_TOK_COMMENT_CLOSE;
          *token_bytes += 3;
        }
        else
        {
          /*---------------------------------------------------------------
            This token is comment data.  The delimiter is '-->'.  The 
            string "--" is not allowed.  [XML spec 1.0 production 15]
            ---------------------------------------------------------------*/
          *token = XMLLIB_TOK_COMMENT_VALUE;
          do
          {
            ret = xmllibi_utf8_peekbytes( metainfo,
                                          *token_bytes,
                                          &ucsvalue,
                                          &utf8_bytes,
                                          error_code );


            if( XMLLIB_SUCCESS == ret && '-' == ucsvalue &&
                TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                                     *token_bytes + 1,
                                                     '-' ) )
            {
              if( TRUE != xmllibi_tok_utf8_checkvalue( metainfo,
                                                       *token_bytes + 2,
                                                       '>' ) )
              {
                *token = XMLLIB_TOK_INVALID;
              }
              break;
            }

            /* *token_bytes += 1; */
            *token_bytes += utf8_bytes;

          } while( XMLLIB_SUCCESS == ret );
        }
      }
      break;

    default:
      *token = XMLLIB_TOK_INVALID;
      break;
    } /* switch( *token ) */
  }

  return ret;
} /* xmllib_tok_utf8_comment() */

/*===========================================================================
FUNCTION XMLLIBI_TOK_UTF8_XMLDECL

DESCRIPTION
  This function is called to tokenize xml declaration.  Valid tokens
  are:
  
  XMLLIB_TOK_XMLDECL_CLOSE
  XMLLIB_TOK_INVALID
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR    In case of an error with the error code set in error_code
                  argument.
  XMLLIB_SUCCESS  In case of success with the token set in token  argument
                  and the number of bytes in the token returned in
                  token_bytes argument.

SIDE EFFECTS
  None.
===========================================================================*/
int32 xmllibi_tok_utf8_xmldecl
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint32    ucsvalue;
  int32     ret;
  uint32    utf8_bytes;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != token );
  XMLLIBI_DEBUG_ASSERT( NULL != token_bytes );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  *token_bytes = 0;

  /*-------------------------------------------------------------------------
    Call xmllibi_tok_utf8_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_utf8_start_token( metainfo,
                                      token,
                                      token_bytes,
                                      error_code );

  if( XMLLIB_SUCCESS == ret )
  {
    switch( *token )
    {
    case XMLLIB_TOK_SPACE:
      break;

      /*---------------------------------------------------------------------
        A return of XMLLIB_TOK_CONTENT requires further processing
        ---------------------------------------------------------------------*/
    case XMLLIB_TOK_CONTENT:
      *token_bytes = 0;
      ret = xmllibi_utf8_peekbytes( metainfo,
                                    *token_bytes,
                                    &ucsvalue,
                                    &utf8_bytes,
                                    error_code );
      *token_bytes += utf8_bytes;
          

      if( XMLLIB_SUCCESS == ret &&
          '?' == ucsvalue &&
          TRUE == xmllibi_tok_utf8_checkvalue( metainfo,
                                               (*token_bytes)++,
                                               '>' ) )
      {
        /*---------------------------------------------------------------
          '?>' is used to end the processing instruction
          ---------------------------------------------------------------*/
        *token        = XMLLIB_TOK_XMLDECL_CLOSE;
      }
      break;
    default:
      *token = XMLLIB_TOK_INVALID;
      break;
    } /* switch( *token ) */
  }

  return ret;
} /* xmllib_tok_utf8_xmldecl() */

/*===========================================================================
                           TOKEN GENERATOR TOKEN TABLE
===========================================================================*/

const xmllibi_tok_gen_table_type 
xmllibi_tok_gen_utf8_table[XMLLIB_TOK_MAX] = 
{
  /* XMLLIB_TOK_EOF            */
  {
    FALSE,
    NULL,
    0,
    FALSE
  },
  /* XMLLIB_TOK_SPACE          */
  {
    TRUE,
    " ",
    1,
    FALSE
  },
  /* XMLLIB_TOK_TAG_OPEN       */
  {
    TRUE,
    "<",
    1,
    FALSE
  },
  /* XMLLIB_TOK_PI_OPEN        */
  {
    TRUE,
    "<?",
    2,
    FALSE
  },
  /* XMLLIB_TOK_XMLDECL_OPEN   */
  {
    TRUE,
    "<?xml",
    5,
    FALSE
  },
  /* XMLLIB_TOK_COMMENT_OPEN   */
  {
    TRUE,
    "<!--",
    4,
    FALSE
  },
  /* XMLLIB_TOK_CDATA_OPEN     */
  {
    TRUE,
    "<![CDATA[",
    9,
    FALSE
  },
  /* XMLLIB_TOK_DTD_OPEN       */
  {
    FALSE,
    NULL,
    0,
    FALSE
  },
  /* XMLLIB_TOK_ENDTAG_OPEN    */
  {
    TRUE,
    "</",
    2,
    FALSE
  },
  /* XMLLIB_TOK_TAG_CLOSE      */
  {
    TRUE,
    ">",
    1,
    FALSE
  },
  /* XMLLIB_TOK_PI_CLOSE       */
  {
    TRUE,
    "?>",
    2,
    FALSE
  },
  /* XMLLIB_TOK_CDATA_CLOSE    */
  {
    TRUE,
    "]]>",
    3,
    FALSE
  },
  /* XMLLIB_TOK_EMPTYTAG_CLOSE */
  {
    TRUE,
    "/>",
    2,
    FALSE
  },
  /* XMLLIB_TOK_COMMENT_CLOSE  */
  {
    TRUE,
    "-->",
    3,
    FALSE
  },
  /* XMLLIB_TOK_ENTITY_REF     */
  {
    TRUE,
    "&",
    1,
    TRUE
  },
  /* XMLLIB_TOK_CHAR_REF       */
  {
    TRUE,
    "&#",
    2,  
    FALSE  /* FIXME Check this */
  },
  /* XMLLIB_TOK_CONTENT        */
  {
    TRUE,
    NULL,
    0,  
    TRUE
  },
  /* XMLLIB_TOK_EQ             */
  {
    TRUE,
    "=",
    1,  
    FALSE
  },
  /* XMLLIB_TOK_NAME           */
  {
    TRUE,
    NULL,
    0,  
    TRUE
  },
  /* XMLLIB_TOK_ATTRIB_VALUE   */
  {
    TRUE,
    NULL,
    0,  
    TRUE
  },
  /* XMLLIB_TOK_PI_VALUE       */
  {
    TRUE,
    NULL,
    0,  
    TRUE
  },
  /* XMLLIB_TOK_COMMENT_VALUE  */
  {
    TRUE,
    NULL,
    0,  
    TRUE
  },
  /* XMLLIB_TOK_CREF_VALUE     */
  {
    TRUE,
    NULL,
    0,  
    TRUE
  },
  /* XMLLIB_TOK_REF_CLOSE      */
  {
    TRUE,
    ";",
    1,  
    FALSE
  },
  /* XMLLIB_TOK_DTD_MUP_OPEN   */
  {
    FALSE,
    NULL,
    0,
    FALSE
  },
  /* XMLLIB_TOK_DTD_MUP_CLOSE  */
  {
    FALSE,
    NULL,
    0,
    FALSE
  }
};

#endif  /* FEATURE_XMLLIB */
