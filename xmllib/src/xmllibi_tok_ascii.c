/*===========================================================================

                  X M L   T O K E N I Z E R   L I B R A R Y
           A S C I I   E N C O D I N G
                   
DESCRIPTION
 This file implements the XML library's ascii encoded text tokenizer.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2005 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllibi_tok_ascii.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/10/05   clp     fixed indentation
09/15/05   jsh     added tok_gen_ascii for generator
09/09/05   jsh     added more cases to ascii_cdata function
09/08/05   jsh     added TOK_SPACE case in ascii_comment function
09/08/05   jsh     fixed comparestr pointer increment 
                   fixed token_bytes pointer value increment 
09/08/05   jsh     fixed token_bytes increment in tok_ascii_comment function
09/06/05   jsh     fixed ascii_pi function case '?'
09/06/05   jsh     fixed ascii_pi function case TOK_CONTENT
09/06/05   jsh     Added XMLLIB_TOK_XMLDECL_CLOSE as one of the valid tokens
                   returned by XMLLIBI_TOK_ASCII_TAG function
09/02/05   jsh     Added xmllibi_tok_ascii_xmldecl
09/02/05   jsh     Added xmllib_tok_ascii_check_value_const function
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
#include "assert.h"
#include <string.h>

/*===========================================================================
                      INTERNAL FUNCTION DEFINITIONS
===========================================================================*/
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
static int32 xmllibi_tok_ascii_checkvalue
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  uint32                  offset,           /* Offset in XML text          */
  char                    expvalue          /* Expected bytevalue          */
)
{
  int32 ret;
  uint8 bytevalue;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );

  ret = ( XMLLIB_SUCCESS == metainfo->peekbytes( metainfo,
                                                 offset,
                                                 sizeof(bytevalue),
                                                 &bytevalue ) &&
          bytevalue == expvalue ) ?
          TRUE                    :
          FALSE;

  return ret;
} /* xmllib_tok_ascii_checkvalue() */


/*===========================================================================
FUNCTION XMLLIBI_TOK_ASCII_START_TOKEN

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
int32 xmllibi_tok_ascii_start_token
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  int32   ret;
  uint8   bytevalue;
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
  ret = metainfo->peekbytes( metainfo,
                             (*token_bytes)++,
                             sizeof(bytevalue),
                             &bytevalue );

  if( XMLLIB_ERROR == ret )
  {
    *token = XMLLIB_TOK_EOF;
    ret    = XMLLIB_SUCCESS;
  }
  else
  {
    switch( bytevalue )
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
        ret = metainfo->peekbytes( metainfo,
                                   (*token_bytes)++,
                                   sizeof(bytevalue),
                                   &bytevalue );
      } while( XMLLIB_SUCCESS == ret &&
               (' '  == bytevalue || '\t' == bytevalue ||
                '\r' == bytevalue || '\n' == bytevalue ));
      (*token_bytes)--;  /* decrement last byte which is not space    */
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
} /* xmllibi_tok_ascii_start_token() */


/*===========================================================================
                      EXTERNAL FUNCTION DEFINITIONS
===========================================================================*/
/*===========================================================================
                      TOKEN PARSER FUNCTIONS
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_TOK_ASCII_DTD

DESCRIPTION
  This function is called to handle DTD tag in ASCII encoding.
  
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
int32 xmllibi_tok_ascii_dtd
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint8                bytevalue;
  int32                ret        = XMLLIB_SUCCESS;
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
    Call xmllibi_tok_ascii_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_ascii_start_token( metainfo,
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
      ret = metainfo->peekbytes( metainfo,
                                 (*token_bytes)++,
                                 sizeof(bytevalue),
                                 &bytevalue );
      if( XMLLIB_SUCCESS == ret )
      {
        switch( bytevalue )
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
          if( ('A' <= bytevalue && 'Z' >= bytevalue) ||
              ('a' <= bytevalue && 'z' >= bytevalue) ||
              '_' == bytevalue || ':' == bytevalue )
          {
            do
            {
              ret = metainfo->peekbytes( metainfo,
                                         (*token_bytes)++,
                                         sizeof(bytevalue),
                                         &bytevalue );
            }
            while( XMLLIB_SUCCESS == ret &&
                   (('A' <= bytevalue && 'Z' >= bytevalue) ||
                    ('a' <= bytevalue && 'z' >= bytevalue) ||
                    ('0' <= bytevalue && '9' >= bytevalue) ||
                    '.' == bytevalue || '-' == bytevalue   ||
                    '_' == bytevalue) );

#if 0
            /*-----------------------------------------------------------
              The delimiter for a name is either space or the '>'
              character.  Check for the delimiter.
              -----------------------------------------------------------*/
            if( XMLLIB_SUCCESS == ret && ('>'  != bytevalue &&
                                          ' '  != bytevalue &&
                                          '\t' != bytevalue &&
                                          '\n' != bytevalue &&
                                          '\r' != bytevalue ) )
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
        } /* switch( bytevalue ) */
      }
      break;

    default:
      *token = XMLLIB_TOK_INVALID;
      break;
    } /* switch( *token ) */
  }

  return ret;
} /* xmllibi_tok_ascii_dtd() */


#if 0
/*===========================================================================
FUNCTION XMLLIBI_TOK_ASCII_DTD_MARKUP

DESCRIPTION
  This function is called to handle markup section in DTD in ASCII encoding.
  
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
int32 xmllibi_tok_ascii_dtd_markup
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint8                bytevalue;
  uint8                tmpbytevalue;
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
    ret = xmllib_tok_peekbyte( xmltext, (*bytes)++, &bytevalue );

    if( XMLLIB_ERROR == ret )
    {
      if( XMLLIB_TOK_INVALID == token )
      {
        token = XMLLIB_TOK_EOF;
      }
      break;
    }

    /* TODO: Handle PEReference, conditional sections, external ID */
    switch( bytevalue )
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
                                     &bytevalue);
          if( XMLLIB_ERROR == ret )
          {
            loopflag = FALSE; 
          }
          else
          {
            if( ' ' != bytevalue && '\t' != bytevalue &&
                '\r' != bytevalue && '\n' != bytevalue )
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
        if( TRUE == xmllibi_tok_ascii_checkvalue( xmltext,
                                                  (*bytes)++,
                                                  '!' ) &&
            XMLLIB_SUCCESS == xmllib_tok_peekbyte( xmltext,
                                                   (*bytes)++,
                                                   &tmpbytevalue ))
        {
          switch( tmpbytevalue )
          {
          case 'E':
            if( TRUE == xmllibi_tok_ascii_checkvalue( xmltext,
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
            loopflag = xmllibi_tok_ascii_checkvalue( xmltext,
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
      if( !(('0' <= bytevalue && '9' >= bytevalue) ||
            ('a' <= bytevalue && 'f' >=bytevalue)  ||
            ('A' <= bytevalue && 'F' >=bytevalue)) )
      {
        token       = XMLLIB_TOK_INVALID;
        *error_code = XMLLIB_ERROR_INVALID_TEXT;
        loopflag = FALSE;
      }
      break;
    } /* switch */
  } while( TRUE == loopflag );

  return ret;
} /* xmllibi_tokenize_ascii_dtd_markup() */
#endif /* if 0 */


/*===========================================================================
FUNCTION XMLLIBI_TOK_ASCII_ENTITY

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
int32 xmllibi_tok_ascii_entity
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint8 bytevalue;
  int32 ret;
  char  *comparestr = NULL;
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
    Call xmllibi_tok_ascii_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_ascii_start_token( metainfo,
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
      ret = metainfo->peekbytes( metainfo,
                                 *token_bytes,
                                 sizeof(bytevalue),
                                 &bytevalue );
      if( XMLLIB_SUCCESS == ret )
      {
        switch( bytevalue )
        {
        case '/':
          *token = XMLLIB_TOK_ENDTAG_OPEN;
          (*token_bytes)++;
          break;

        case '?':
          *token = XMLLIB_TOK_PI_OPEN;
          (*token_bytes)++;

          if( TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
                                                    *token_bytes,
                                                    'x' ) &&
              TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
                                                    *token_bytes + 1,
                                                    'm' ) &&
              TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
                                                    *token_bytes + 2,
                                                    'l' ) )
          {
            ret = metainfo->peekbytes( metainfo,
                                       *token_bytes + 3,
                                       sizeof(bytevalue),
                                       &bytevalue );
            
            if( XMLLIB_SUCCESS == ret &&
                !(('A' <= bytevalue && 'Z' >= bytevalue) ||
                  ('a' <= bytevalue && 'z' >= bytevalue) ||
                  ('0' <= bytevalue && '9' >= bytevalue) ||
                  '.' == bytevalue || '-' == bytevalue   ||
                  ':' == bytevalue || '_' == bytevalue))
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
          ret = metainfo->peekbytes( metainfo,
                                     ++(*token_bytes),
                                     sizeof(bytevalue),
                                     &bytevalue );
          if( XMLLIB_SUCCESS == ret )
          {
            switch( bytevalue )
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
                     TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
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
        } /* switch( bytevalue ) */
      }
      break;

    case XMLLIB_TOK_CONTENT:
      /*-------------------------------------------------------------------
        Find the length of content verifying that it doesn't contain ']]>'
        [XML spec 1.0 production 14]
        -------------------------------------------------------------------*/
      do
      {
        ret = metainfo->peekbytes( metainfo,
                                   (*token_bytes)++,
                                   sizeof(bytevalue),
                                   &bytevalue );
        if( XMLLIB_SUCCESS == ret && ']' == bytevalue &&
            TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
                                                  *token_bytes,
                                                  ']' ) &&
            TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
                                                  *token_bytes + 1,
                                                  '>' ) )

        {
          *token = XMLLIB_TOK_INVALID;
          break;
        }
      } while( XMLLIB_SUCCESS == ret && '<' != bytevalue );
      *token_bytes -= 1;
      break;

    default:
      *token = XMLLIB_TOK_INVALID;
      break;
    } /* switch( *token ) */
  }

  return ret;
} /* xmllib_tok_ascii_entity() */


/*===========================================================================
FUNCTION XMLLIBI_TOK_ASCII_TAG

DESCRIPTION
  This function tokenizes contents of an XML tag encoded in ASCII.  Valid
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
int32 xmllibi_tok_ascii_tag
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint8               bytevalue;
  char                quotevalue;
  int32               ret;
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
    Call xmllibi_tok_ascii_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_ascii_start_token( metainfo,
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
      ret = metainfo->peekbytes( metainfo,
                                 (*token_bytes)++,
                                 sizeof(bytevalue),
                                 &bytevalue );

      /*-------------------------------------------------------------------
        Find the length of content verifying that it doesn't contain ']]>'
        [XML spec 1.0 production 14]
        -------------------------------------------------------------------*/
      switch( bytevalue )
      {
      case '/':
        /*---------------------------------------------------------------
          Check if this is the empty tag close '/>'
          ---------------------------------------------------------------*/
        *token = (TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
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
        quotevalue = bytevalue;
        *token     = XMLLIB_TOK_ATTRIB_VALUE; 
        do
        {
          ret = metainfo->peekbytes( metainfo,
                                     (*token_bytes)++,
                                     sizeof(bytevalue),
                                     &bytevalue );
        } while( XMLLIB_SUCCESS == ret && quotevalue != bytevalue );
        break;
      case '?':
        /*---------------------------------------------------------------
          This could be either XMLDECL_CLOSE or PI_CLOSE token. As both 
          constants have same value (10), the caller function will
          receive success for any of those tokens.
          ----------------------------------------------------------------*/
        *token = (TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
                                                        (*token_bytes)++,
                                                        '>' )) ?
          XMLLIB_TOK_XMLDECL_CLOSE : XMLLIB_TOK_INVALID;
        break;

      default:
        /*---------------------------------------------------------------
          This is a name.  A name can start with a letter or an '_' or
          a ':'. [XML spec 1.0 production 5]
          ---------------------------------------------------------------*/
        *token = ( ('A' <= bytevalue && 'Z' >= bytevalue) ||
                   ('a' <= bytevalue && 'z' >= bytevalue) ||
                   '_' == bytevalue || ':' == bytevalue ) ?
          XMLLIB_TOK_NAME : XMLLIB_TOK_INVALID;

        if( XMLLIB_TOK_NAME == *token )
        {
          /*-------------------------------------------------------------
            A name can contain a letter, a digit, '.', '-', '_' or an
            extender 0xB7 => TODO: figure out
            -------------------------------------------------------------*/
          do
          {
            ret = metainfo->peekbytes( metainfo,
                                       (*token_bytes)++,
                                       sizeof(bytevalue),
                                       &bytevalue );
          } while( XMLLIB_SUCCESS == ret &&
                   (('A' <= bytevalue && 'Z' >= bytevalue) ||
                    ('a' <= bytevalue && 'z' >= bytevalue) ||
                    ('0' <= bytevalue && '9' >= bytevalue) ||
                    '.' == bytevalue || '-' == bytevalue   ||
                    ':' == bytevalue ||
                    '_' == bytevalue) );

          *token_bytes -= 1;

#if 0
          /*-------------------------------------------------------------
            The delimiter for a name is either space, '>' or '/>'
            charachters.  Check for the delimiter.
            -------------------------------------------------------------*/
          if( XMLLIB_SUCCESS == ret && ('>'  != bytevalue &&
                                        '/'  != bytevalue &&
                                        ' '  != bytevalue &&
                                        '='  != bytevalue &&
                                        '\t' != bytevalue &&
                                        '\n' != bytevalue &&
                                        '\r' != bytevalue ) )
          {
            *token = XMLLIB_TOK_INVALID;
          }
#endif
        }
        break;
      } /* switch( bytevalue ) */
      break;

    default:
      *token = XMLLIB_TOK_INVALID;
      break;
    } /* switch( *token ) */
  }

  return ret;
} /* xmllib_tok_ascii_tag() */


/*===========================================================================
FUNCTION XMLLIBI_TOK_ASCII_CDATA

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
int32 xmllibi_tok_ascii_cdata
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint8 bytevalue;
  int32 ret;
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
    Call xmllibi_tok_ascii_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_ascii_start_token( metainfo,
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
      ret = metainfo->peekbytes( metainfo,
                                 (*token_bytes)++,
                                 sizeof(bytevalue),
                                 &bytevalue );
      if( XMLLIB_SUCCESS == ret )
      {
        if( ']' == bytevalue &&
            TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
                                                  *token_bytes,
                                                  ']' ) &&
            TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
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
            ret = metainfo->peekbytes( metainfo,
                                       (*token_bytes),
                                       sizeof(bytevalue),
                                       &bytevalue );

            if( XMLLIB_SUCCESS == ret &&
                ']' == bytevalue &&
                TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
                                                      *token_bytes + 1,
                                                      ']' ) &&
                TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
                                                      *token_bytes + 2,
                                                      '>' ) )
            {
              break;
            }
            (*token_bytes)++ ;
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
} /* xmllib_tok_ascii_cdata() */


/*===========================================================================
FUNCTION XMLLIBI_TOK_ASCII_PI

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
int32 xmllibi_tok_ascii_pi
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint8 bytevalue;
  int32 ret;
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
    ret = metainfo->peekbytes( metainfo,
                               (*token_bytes),
                               sizeof(bytevalue),
                               &bytevalue );

    if( XMLLIB_SUCCESS == ret) 
    {
      if ('?' == bytevalue &&
           TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
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

      (*token_bytes)++;
    }

  } while( XMLLIB_SUCCESS == ret );

  return ret;
} /* xmllib_tok_ascii_pi() */


/*===========================================================================
FUNCTION XMLLIBI_TOK_ASCII_COMMENT

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
int32 xmllibi_tok_ascii_comment
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint8 bytevalue;
  int32 ret;
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
    Call xmllibi_tok_ascii_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_ascii_start_token( metainfo,
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
      ret = metainfo->peekbytes( metainfo,
                                 *token_bytes,
                                 sizeof(bytevalue),
                                 &bytevalue );
      if( XMLLIB_SUCCESS == ret )
      {
        if( '-' == bytevalue &&
            TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
                                                  *token_bytes + 1,
                                                  '-' ) &&
            TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
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
            ret = metainfo->peekbytes( metainfo,
                                       *token_bytes,
                                       sizeof(bytevalue),
                                       &bytevalue );

            if( XMLLIB_SUCCESS == ret && '-' == bytevalue &&
                TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
                                                      *token_bytes + 1,
                                                      '-' ) )
            {
              if( TRUE != xmllibi_tok_ascii_checkvalue( metainfo,
                                                        *token_bytes + 2,
                                                        '>' ) )
              {
                *token = XMLLIB_TOK_INVALID;
              }
              break;
            }
            *token_bytes += 1;
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
} /* xmllib_tok_ascii_comment() */

/*===========================================================================
FUNCTION XMLLIBI_TOK_ASCII_XMLDECL

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
int32 xmllibi_tok_ascii_xmldecl
(
  xmllib_metainfo_s_type *metainfo,         /* XML meta information        */
  xmllib_token_e_type    *token,            /* XML text token              */
  uint32                 *token_bytes,      /* Bytes in token              */
  xmllib_error_e_type    *error_code        /* Error code if any           */
)
{
  uint8 bytevalue;
  int32 ret;
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
    Call xmllibi_tok_ascii_start_token() to start forming the token
    -------------------------------------------------------------------------*/
  ret = xmllibi_tok_ascii_start_token( metainfo,
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
      ret = metainfo->peekbytes( metainfo,
                                 (*token_bytes)++,
                                 sizeof(bytevalue),
                                 &bytevalue );
      if( XMLLIB_SUCCESS == ret &&
          '?' == bytevalue &&
          TRUE == xmllibi_tok_ascii_checkvalue( metainfo,
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
} /* xmllib_tok_ascii_xmldecl() */

/*===========================================================================
                           TOKEN GENERATOR TOKEN TABLE
===========================================================================*/

const xmllibi_tok_gen_table_type 
xmllibi_tok_gen_ascii_table[XMLLIB_TOK_MAX] = 
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
