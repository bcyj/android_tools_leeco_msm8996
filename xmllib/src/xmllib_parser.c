/*===========================================================================

                  X M L   P A R S E R   L I B R A R Y
                   
DESCRIPTION
 This file implements the XML parser library.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2005 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllib_parser.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/10/09   jsh     spaces are allowed in end tag before >
06/29/06   ssingh  Added support for UTF-8 encoding - extended 
                   xmllib_decl_const_e_type
12/22/05   clp     code review changes
12/22/05   jsh     fixed xmldecl verification
11/04/05   clp     cleaned up bad indentation
09/12/05   jsh     removed ASSERT(0) from parser_dtd
09/08/05   jsh     parser returns INVALID_ARGUMENTS if error_code is NULL
09/08/05   jsh     fixed parser_comment function: spaces are allowed after
                   <!-- token
09/07/05   jsh     Added case for XMLDECL node type in parser_free function
09/06/05   jsh     Added xmllibi_parser_xmldecl function
09/06/05   jsh     Added case for XMLDECL node type in parser_entity func
03/09/04   ifk     Fixed bug where a NULL pointer was being referenced.
12/11/03   ifk     Reworked xmllibi_parser_attribute() as proposed in code
                   review.
10/30/03   ifk     Created module.

===========================================================================*/
/*===========================================================================

                      INCLUDE FILES FOR MODULE

===========================================================================*/
#include "comdef.h"

#ifdef FEATURE_XMLLIB
#include "xmllib_parser.h"
#include "xmllib_tok.h"
#include "xmllib_decl.h"
#include "err.h"
#include "assert.h"
#include <string.h>

/*===========================================================================
                             Internal type definitions
===========================================================================*/
static const xmllib_decl_const_e_type
xmllib_parser_allowed_encodings[] = {
  XMLLIB_CONST_ENC_ASCII,
  XMLLIB_CONST_ENC_ISO_8859_1,
  XMLLIB_CONST_ENC_UTF8,
  XMLLIB_CONST_MAX
};

static const xmllib_decl_const_e_type
xmllib_parser_allowed_standalone[] = {
  XMLLIB_CONST_STDALONE_YES,
  XMLLIB_CONST_STDALONE_NO,
  XMLLIB_CONST_MAX
};

/*===========================================================================
                        INTERNAL FUNCTION DEFINITIONS
===========================================================================*/
/*===========================================================================
                             Utility Functions 
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_PARSER_NODEALLOC

DESCRIPTION
  Allocate and intitialize a new parse tree node.

DEPENDENCIES
  None.

RETURN VALUE
  NULL in case of error with the error_code argument set
  pointer to the newly allocated node in case of success

SIDE EFFECTS
  Memory is allocated for a parse tree node.
===========================================================================*/
static xmllib_parsetree_node_s_type *xmllibi_parser_nodealloc
(
  xmllib_metainfo_s_type       *metainfo,   /* Meta information structure  */
  xmllib_parsetree_node_e_type  nodetype,   /* Type of node requested      */
  xmllib_error_e_type          *error_code  /* Error code                  */
)
{
  xmllib_parsetree_node_s_type *pnode;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->memalloc );
  XMLLIBI_DEBUG_ASSERT( XMLLIB_PARSETREE_NODE_MAX > nodetype );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code  );

  /*-------------------------------------------------------------------------
    Allocate memory for the parse tree node
  -------------------------------------------------------------------------*/
  pnode = (xmllib_parsetree_node_s_type*)metainfo->memalloc( 
    sizeof(xmllib_parsetree_node_s_type) );

  if( NULL == pnode )
  {
    *error_code = XMLLIB_ERROR_NOMEM;
  }
  else
  {
    /*-----------------------------------------------------------------------
      Initialize the node to 0 and set the node type
    -----------------------------------------------------------------------*/
    memset( pnode, 0, sizeof(xmllib_parsetree_node_s_type) );
    pnode->nodetype = nodetype;
  }

  return pnode;
} /* xmllibi_parser_nodealloc() */


/*===========================================================================
FUNCTION XMLLIBI_PARSER_GETBYTES

DESCRIPTION
  Consume numbytes bytes from the XML text.  If the bytes are to be copied
  allocate memory for them.

DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR   in case of error with error_code set
  XMLLIB_SUCCESS in case of success with the bytes copied to buffer if
                 non-NULL

SIDE EFFECTS
  Memory is allocated for the buffer string if returning XMLLIB_SUCESS.
  The bytes are consumed from the XML text.
===========================================================================*/
static int32 xmllibi_parser_getbytes
(
  xmllib_metainfo_s_type        *metainfo,  /* Meta information structure  */
  xmllib_string_s_type          *strptr,    /* XML string to copy bytes to */
  int32                          numbytes,  /* Number of bytes to consume  */
  xmllib_error_e_type           *error_code /* Error code in case of error */
)
{
  int32 ret = XMLLIB_SUCCESS;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->memalloc );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->memfree );
  XMLLIBI_DEBUG_ASSERT( 0 < numbytes  );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  /*-------------------------------------------------------------------------
    Allocate memory for bytes if a buffer pointer passed in
  -------------------------------------------------------------------------*/
  if( NULL != strptr )
  {
    strptr->len    = numbytes;
    strptr->string = (char*)metainfo->memalloc( numbytes );

    if( NULL == strptr->string )
    {
      *error_code = XMLLIB_ERROR_NOMEM;
      ret         = XMLLIB_ERROR;
    }
  }

  /*---------------------------------------------------------------------
    Copy/skip the bytes.  In case of error free the memory allocated in
    the string buffer.
  ---------------------------------------------------------------------*/
  if( XMLLIB_SUCCESS == ret )
  {
    ret = metainfo->getbytes( metainfo,
                              numbytes,
                              (NULL == strptr ) ? NULL : strptr->string );

    if( XMLLIB_ERROR == ret && NULL != strptr )
    {
      metainfo->memfree( strptr->string );
      strptr->string = NULL;
    }
  }

  return ret;
} /* xmllibi_parser_getbytes() */


/*===========================================================================
FUNCTION XMLLIBI_PARSER_GET_EXPECTED_TOKEN

DESCRIPTION
  This function is called with a specific expected token in an XML stream
  in a particular state.  If the token is found then the bytes associated
  with the token are consumed; being copied to the passed string buffer
  if a non-NULL buffer is passed.

DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR   in case of error with error_code set.  If the expected
                 token was not seen then the error code is
                 XMLLIB_ERROR_INVALID_TEXT
  XMLLIB_SUCCESS in case of success.  If buffer is non-NULL the bytes
                 associated with the token are copied to it

SIDE EFFECTS
  Memory is allocated for the string in the buffer if a non-NULL buffer is 
  passed.  The bytes associated with the expected token are consumed.
===========================================================================*/
static int32 xmllibi_parser_get_expected_token
(
  xmllib_encoding_e_type         encoding,  /* Encoding of the XML text    */
  xmllib_metainfo_s_type        *metainfo,  /* XML meta information        */
  xmllib_tok_state_e_type        tokstate,  /* Tokenizer state             */
  xmllib_token_e_type            exptok,    /* The expected token          */
  xmllib_string_s_type          *strptr,    /* XML string to copy text to  */
  xmllib_error_e_type           *error_code /* Error code in case of error */
)
{
  int32               ret = XMLLIB_SUCCESS;
  uint32              tokbytes;
  xmllib_token_e_type token;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  
  ret = xmllib_tok_tokenize( encoding,
                             tokstate,
                             metainfo,
                             &token,
                             &tokbytes,
                             error_code );

  if( XMLLIB_SUCCESS == ret )
  {
    if( exptok == token )
    {
      ret = xmllibi_parser_getbytes( metainfo,
                                     strptr,
                                     tokbytes,
                                     error_code );
    }
    else
    {
      if( XMLLIB_TOK_INVALID != token )
      {
        *error_code = XMLLIB_ERROR_INVALID_TEXT;
      }
      ret = XMLLIB_ERROR;
    }
  }

  return ret;
} /* xmllibi_parser_get_expected_token() */

/*===========================================================================
                           Tag Handling Functions 
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_PARSER_ATTRIBUTE

DESCRIPTION
  This function is called to process any attributes present in a tag.  It
  copies the attribute list to the element node passed in without 
  normalizing them and consumes the asssociated bytes.

DEPENDENCIES
  The passed parse tree node should be the element node associated with
  this tag.

RETURN VALUE
  XMLLIB_ERROR   in case of error setting error_code
  XMLLIB_SUCCESS in case of success with the attribute list, if any, copied
                 to the passed element node

SIDE EFFECTS
  Memory is allocated for the attribute list.  The attribute bytes are
  consumed.
===========================================================================*/
static int32 xmllibi_parser_attribute
(
  xmllib_encoding_e_type              encoding,  /* Encoding of XML text   */
  xmllib_metainfo_s_type             *metainfo,  /* XML meta information   */
  xmllib_parsetree_attribute_s_type **attribute, /* Attribute to be filled */
  xmllib_error_e_type                *error_code /* error code if any      */
)
{
  int32 ret = XMLLIB_SUCCESS;
  xmllib_token_e_type token;
  uint32 tokbytes;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    The attributes are in the form - S Name=Value
  -------------------------------------------------------------------------*/
  ret = xmllibi_parser_get_expected_token( encoding,
                                           metainfo,
                                           XMLLIB_TOKSTATE_TAG,
                                           XMLLIB_TOK_SPACE,
                                           NULL,
                                           error_code );

  /*-------------------------------------------------------------------------
    If space seen then allocate memory for an attribute and expect a name.
  -------------------------------------------------------------------------*/
  if( XMLLIB_SUCCESS == ret )
  {
    *attribute = (xmllib_parsetree_attribute_s_type*)metainfo->memalloc(
                   sizeof(xmllib_parsetree_attribute_s_type) );
    if( NULL == *attribute )
    {
      *error_code = XMLLIB_ERROR_NOMEM;
      ret         = XMLLIB_ERROR;
    }
    else
    {
      memset( *attribute, 0, sizeof( xmllib_parsetree_attribute_s_type ) );
      ret = xmllibi_parser_get_expected_token( encoding,
                                               metainfo,
                                               XMLLIB_TOKSTATE_TAG,
                                               XMLLIB_TOK_NAME,
                                               &(*attribute)->name,
                                               error_code );
      if( XMLLIB_SUCCESS == ret )
      {
        do
        {
          ret = xmllib_tok_tokenize( encoding,
                                     XMLLIB_TOKSTATE_TAG,
                                     metainfo,
                                     &token,
                                     &tokbytes,
                                     error_code );
          if(XMLLIB_TOK_SPACE == token)
          {
            /* consume space */
            ret = xmllibi_parser_getbytes( metainfo,
                                           NULL,
                                           tokbytes,
                                           error_code );
          }
        } while( token == XMLLIB_TOK_SPACE ) ;

        ret = xmllibi_parser_get_expected_token( encoding,
                                                 metainfo,
                                                 XMLLIB_TOKSTATE_TAG,
                                                 XMLLIB_TOK_EQ,
                                                 NULL,
                                                 error_code );
        if( XMLLIB_SUCCESS == ret )
        {
          do
          {
            ret = xmllib_tok_tokenize( encoding,
                                       XMLLIB_TOKSTATE_TAG,
                                       metainfo,
                                       &token,
                                       &tokbytes,
                                       error_code );
            if(XMLLIB_TOK_SPACE == token)
            {
              /* consume space */
              ret = xmllibi_parser_getbytes( metainfo,
                                             NULL,
                                             tokbytes,
                                             error_code );
            }
          } while( token == XMLLIB_TOK_SPACE ) ;

          ret = xmllibi_parser_get_expected_token( encoding,
                                                   metainfo,
                                                   XMLLIB_TOKSTATE_TAG,
                                                   XMLLIB_TOK_ATTRIB_VALUE,
                                                   &(*attribute)->value,
                                                   error_code );
        }
      }
    }
  }

  /*-------------------------------------------------------------------------
    Free the memory in the attribute if an error occured
    the attribute list in the passed node
  -------------------------------------------------------------------------*/
  if( XMLLIB_ERROR == ret && NULL != *attribute )
  {
    if( NULL != (*attribute)->name.string )
    {
      metainfo->memfree( (*attribute)->name.string );
    }
    if( NULL != (*attribute)->value.string )
    {
      metainfo->memfree( (*attribute)->value.string );
    }
    metainfo->memfree( *attribute );
    *attribute = NULL;
  }

  return ret;
} /* xmllibi_parser_attribute() */


/*===========================================================================
FUNCTION XMLLIBI_PARSER_STARTTAG

DESCRIPTION
  This function is called to process a start tag token.  It is passed the
  element node asociated with this tag to fill in.
  
DEPENDENCIES
 currnode should be already allocated.

RETURN VALUE
  XMLLIB_ERROR   in case of error setting error_code
  XMLLIB_SUCCESS in case of success, consuming the XML text to fill in
                 the element node name and attributes

SIDE EFFECTS
  Allocates memory for the name and attribute list.  If the element is an
  empty tag changes the nodetype for the node.  Consumes the bytes in
  the tag.
===========================================================================*/
static int32 xmllibi_parser_starttag
(
  xmllib_encoding_e_type         encoding,  /* Encoding of the XML text    */
  xmllib_metainfo_s_type        *metainfo,  /* XML meta information        */
  xmllib_parsetree_node_s_type **subtree,   /* Sub parse tree generated    */
  xmllib_error_e_type           *error_code /* error code if any           */
)
{
  int32                               ret;
  uint32                              tokbytes;
  xmllib_token_e_type                 token;
  xmllib_parsetree_attribute_s_type **attribute;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
  -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != subtree );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  /*-------------------------------------------------------------------------
    An XML tag consists of <NAME (S Attribute)* S?>.  The < token is already
    consumed.  Expect XMLLIB_TOK_NAME possibly followed by the attribute list
    and terminated by XMLLIB_TOK_TAG_CLOSE.
  -------------------------------------------------------------------------*/
  ret = xmllibi_parser_get_expected_token( encoding,
                                           metainfo,
                                           XMLLIB_TOKSTATE_TAG,
                                           XMLLIB_TOK_NAME,
                                           &(*subtree)->payload.element.name,
                                           error_code );

  /*-------------------------------------------------------------------------
    Check if name copied successfuly.  Check for attributes and use them
    to fill the attribute list, consuming any whitespace as well.
  -------------------------------------------------------------------------*/
  if( XMLLIB_SUCCESS == ret )
  {
    attribute = &(*subtree)->payload.element.attribute_list;
    do {
      ret = xmllibi_parser_attribute( encoding,
                                      metainfo,
                                      attribute,
                                      error_code );
      if( NULL != *attribute )
      {
        attribute = &(*attribute)->next;
      }
    } while( XMLLIB_SUCCESS == ret );

    /*-----------------------------------------------------------------------
      We should have ret set to XMLLIB_ERROR with the error code set to 
      XMLLIB_ERROR_INVALID_TEXT to indicate that we successfully went through
      the attribute list (if present).  Now get the last token,
      XMLLIB_TOK_TAG_CLOSE expected.  Skip the token bytes.  If the last
      token is XMLLIB_TOK_EMPTYTAG_CLOSE change the nodetype to
      XMLLIB_PARSETREE_NODE_EMPTYELEMENT.
    -----------------------------------------------------------------------*/
    if( XMLLIB_ERROR == ret && XMLLIB_ERROR_INVALID_TEXT == *error_code )
    {
      ret = xmllib_tok_tokenize( encoding,
                                 XMLLIB_TOKSTATE_TAG,
                                 metainfo,
                                 &token,
                                 &tokbytes,
                                 error_code );
      if( XMLLIB_SUCCESS == ret )
      {
        switch( token )
        {
          case XMLLIB_TOK_TAG_CLOSE:
            break;

          case XMLLIB_TOK_EMPTYTAG_CLOSE:
            (*subtree)->nodetype = XMLLIB_PARSETREE_NODE_EMPTYELEMENT;
            break;

          default:
            ret = XMLLIB_ERROR;
            break;
        }

        /*-------------------------------------------------------------------
          Consume the bytes in the close tag
        -------------------------------------------------------------------*/
        if( XMLLIB_SUCCESS == ret )
        {
          ret = xmllibi_parser_getbytes( metainfo,
                                         NULL,
                                         tokbytes,
                                         error_code );
        }
      }
    }
  }  

  return ret;
} /* xmllibi_parser_starttag() */


/*===========================================================================
FUNCTION XMLLIBI_PARSER_ENDTAG

DESCRIPTION
  This function is called to process an endtag token.  It uses the passed
  startnode to compare the name of the endtag with the name of the start
  tag and consumes the bytes in the endtag token.
  
DEPENDENCIES
  startnode should be the node corresponding to the start tag of this
  end tag

RETURN VALUE
  XMLLIB_ERROR   in case of error setting error_code
  XMLLIB_SUCCESS in case of success

SIDE EFFECTS
  Consumes the bytes in the end tag.
===========================================================================*/
static int32 xmllibi_parser_endtag
(
  xmllib_encoding_e_type         encoding,  /* Encoding of the XML text    */
  xmllib_metainfo_s_type        *metainfo,  /* XML meta information        */
  xmllib_parsetree_node_s_type **subtree,   /* Sub parse tree generated    */
  xmllib_error_e_type           *error_code /* error code if any           */
)
{
  int32                ret;
  xmllib_string_s_type namestr;
  xmllib_token_e_type  token;
  uint32               tokbytes;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
  -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->memalloc );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->memfree );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext );
  XMLLIBI_DEBUG_ASSERT( NULL != subtree );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  namestr.string = NULL;

  /*-------------------------------------------------------------------------
    An XML endtag consists of </NAME>.  The </ token is consumed by this
    point.  Expect a XMLLIB_TOK_NAME token.
  -------------------------------------------------------------------------*/
  ret = xmllibi_parser_get_expected_token( encoding,
                                           metainfo,
                                           XMLLIB_TOKSTATE_TAG,
                                           XMLLIB_TOK_NAME,
                                           &namestr,
                                           error_code );

  if( XMLLIB_SUCCESS == ret )
  {
    /*---------------------------------------------------------------------
      Compare endtag name with the corresponding open tag name
    ---------------------------------------------------------------------*/
    if( namestr.len == (*subtree)->payload.element.name.len &&
        0 == strncmp( (char *) namestr.string, 
                     (char *) ((*subtree)->payload.element.name.string),
                     namestr.len ) )
    {
      
      do
      {
        /*-------------------------------------------------------------------
          Spaces are allowed before XML_TOK_TAG_CLOSE
          [42]          ETag       ::=          '</' Name  S? '>'
          Consume 0x20, 0x9, 0xD, 0xA
          -------------------------------------------------------------------*/
        ret = xmllib_tok_tokenize( encoding,
                                   XMLLIB_TOKSTATE_TAG,
                                   metainfo,
                                   &token,
                                   &tokbytes,
                                   error_code );
        if(XMLLIB_TOK_SPACE == token)
        {
          /* consume space */
          ret = xmllibi_parser_getbytes( metainfo,
                                         NULL,
                                         tokbytes,
                                         error_code );
        }
      } while( token == XMLLIB_TOK_SPACE ) ;

      /*-------------------------------------------------------------------
        Verify that the next token is XML_TOK_TAG_CLOSE and consume it
      -------------------------------------------------------------------*/
      ret = xmllibi_parser_get_expected_token( encoding,
                                               metainfo,
                                               XMLLIB_TOKSTATE_TAG,
                                               XMLLIB_TOK_TAG_CLOSE,
                                               NULL,
                                               error_code );

    }
    else
    {
      *error_code = XMLLIB_ERROR_INVALID_TEXT;
      ret         = XMLLIB_ERROR;
    }
  }

  /*-------------------------------------------------------------------------
    Free memory allocated to the name string
  -------------------------------------------------------------------------*/
  if( NULL != namestr.string )
  {
    metainfo->memfree( namestr.string );
  }
  
  return ret;
} /* xmllibi_parser_endtag() */


/*===========================================================================
                        DTD Handling Functions
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_PARSER_DTD

DESCRIPTION
  This function is called to process a DTD token.
  
DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR   in case of error
  XMLLIB_SUCCESS in case of success

SIDE EFFECTS
  For now skip
===========================================================================*/
static int32 xmllibi_parser_dtd
(
  xmllib_encoding_e_type         encoding,  /* Encoding of the XML text    */
  xmllib_metainfo_s_type        *metainfo,  /* XML meta information        */
  xmllib_parsetree_node_s_type **subtree,   /* Sub parse tree generated    */
  xmllib_error_e_type           *error_code /* error code if any           */
)
{
  int32 ret = XMLLIB_SUCCESS;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
  -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != subtree );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );


  /* TODO:  Skip DTD for now */

  return ret;
} /* xmllibi_parser_dtd() */

/*===========================================================================
                        XMLDECL HANDLING FUNCTIONS
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_PARSER_XMLDECL

DESCRIPTION
  this function is called to process a xmldecl token.
  
DEPENDENCIES
  none.

RETURN VALUE
  XMLLIB_ERROR   in case of error
  XMLLIB_SUCCESS in case of success

SIDE EFFECTS
  consumes the corresponding bytes, does not copy them to the subtree arg
===========================================================================*/
static int32 xmllibi_parser_xmldecl
(
  xmllib_encoding_e_type         encoding,  /* encoding of the xml text    */
  xmllib_metainfo_s_type        *metainfo,  /* xml meta information        */
  xmllib_parsetree_node_s_type **subtree,   /* sub parse tree generated    */
  xmllib_error_e_type           *error_code /* error code if any           */
)
{
  int32                              ret = XMLLIB_SUCCESS;
  int32                              status = XMLLIB_SUCCESS;
  uint32                             tokbytes;
  xmllib_token_e_type                token;
  xmllib_parsetree_attribute_s_type  **attribute;
  xmllib_decl_const_e_type     const *allowed;

  /*-------------------------------------------------------------------------
    verify the arguments passed.
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != subtree );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext );

  /*-------------------------------------------------------------------------
    allocate xmldecl node
    -------------------------------------------------------------------------*/
  *subtree = xmllibi_parser_nodealloc( metainfo,
                                       XMLLIB_PARSETREE_NODE_XMLDECL,
                                       error_code );
  if( NULL == *subtree )
  {
    return XMLLIB_ERROR ;
  }

  /*-------------------------------------------------------------------------
    xmldecl start with <?xml... of which, <?xml is already consumed.
    production 23
    expect xmllib_tok_name=version followed by 
    mandatory xmllib_tok_eq and xmllib_tok_attrib_value='1.0' followed by
    2) optional xmllib_tok_name=encoding, xmllib_tok_eq and 
    xmllib_tok_attrib_value followed by
    3) optional xmllib_tok_name=standalone, xmllib_tok_eq and 
    xmllib_tok_attrib_value = 'yes' | 'no' followed by
    4) optional xmllib_tok_space followed by
    5) mandatory xmllib_tok_pi_close
    -------------------------------------------------------------------------*/
  attribute = &(*subtree)->payload.element.attribute_list;
  do {
    ret = xmllibi_parser_attribute( encoding,
                                    metainfo,
                                    attribute,
                                    error_code );
    if( NULL != *attribute )
    {
      attribute = &(*attribute)->next;
    }
  } while( XMLLIB_SUCCESS == ret );

  if( XMLLIB_ERROR == ret && XMLLIB_ERROR_INVALID_TEXT == *error_code )
  {
    ret = xmllib_tok_tokenize( encoding,
                               XMLLIB_TOKSTATE_XMLDECL,
                               metainfo,
                               &token,
                               &tokbytes,
                               error_code );

    if( XMLLIB_SUCCESS == ret )
    {
      switch( token )
      {
      case XMLLIB_TOK_XMLDECL_CLOSE:
        break;

      default:
        ret = XMLLIB_ERROR;
        break;
      }
      /*-------------------------------------------------------------------
        Consume the bytes in the close tag
        -------------------------------------------------------------------*/
      if( XMLLIB_SUCCESS == ret )
      {
        ret = xmllibi_parser_getbytes( metainfo,
                                       NULL,
                                       tokbytes,
                                       error_code );
      }
    }
  }

  /*-------------------------------------------------------------------
    verify xmldecl node
    -------------------------------------------------------------------*/
  attribute = &(*subtree)->payload.element.attribute_list;

  /*---------------------------------------------------------------------
    check for name/value of version attribute
    ---------------------------------------------------------------------*/
  if( XMLLIB_SUCCESS == ret && NULL != *attribute )  
  {
    ret = xmllib_decl_check_value( encoding,
                                  &(*attribute)->name,
                                  XMLLIB_CONST_VERSION,
                                  error_code );
    if(XMLLIB_SUCCESS == ret)
    {
      ret = xmllib_decl_check_value( encoding,
                                    &(*attribute)->value,
                                    XMLLIB_CONST_XML_VER,
                                    error_code );
    }
  }
  else
    /*------------------------------------------------------------------------
      version is a mandatory attribute 
      ------------------------------------------------------------------------*/
  {
    ret = XMLLIB_ERROR;
    *error_code = XMLLIB_ERROR_INVALID_TEXT;
  }

  /*---------------------------------------------------------------------
    check for name/value of encoding attribute (if exists)
    ---------------------------------------------------------------------*/
  if( XMLLIB_SUCCESS == ret && NULL != (*attribute)->next )
  {
    attribute = &(*attribute)->next;

    /*---------------------------------------------------------------------
      encoding attribute is optional, don't use ret as return value
      ---------------------------------------------------------------------*/
    status = xmllib_decl_check_value( encoding,
                                     &(*attribute)->name,
                                     XMLLIB_CONST_ENCODING,
                                     error_code );

    if(XMLLIB_SUCCESS == status)
    {
      allowed = xmllib_parser_allowed_encodings;
      do
      {
        ret = xmllib_decl_check_value( encoding,
                                      &(*attribute)->value,
                                      *allowed++,
                                      error_code );
      }
      while ( ret != XMLLIB_SUCCESS && XMLLIB_CONST_MAX != *allowed );
      attribute = &(*attribute)->next;
    }

    /*---------------------------------------------------------------------
      Check for standalone
      ---------------------------------------------------------------------*/
    if( XMLLIB_SUCCESS == ret && NULL != *attribute)
    {
      ret = xmllib_decl_check_value( encoding,
                                    &(*attribute)->name,
                                    XMLLIB_CONST_STDALONE,
                                    error_code );

      allowed = xmllib_parser_allowed_standalone;
      do
      {
        ret = xmllib_decl_check_value( encoding,
                                      &(*attribute)->value,
                                      *allowed++,
                                      error_code );
      }
      while ( ret != XMLLIB_SUCCESS && XMLLIB_CONST_MAX != *allowed );
    }
  }

  return ret;
} /* xmllibi_parser_xmldecl() */

/*===========================================================================
                   XML Sub-component Handling Functions 
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_PARSER_CDATA

DESCRIPTION
  This function is called to process a cdata token.  It allocates a node,
  copies the cdata bytes and consumes the corresponding XML text.
  
DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error code set in error_code
  XMLLIB_SUCESS  in case of success with the cdata copied to the allocted
                 node returned in subtree

SIDE EFFECTS
  Memory is allocated for the new node and all associated data structures.
  The corresponding bytes in the XML text are consumed.
===========================================================================*/
static int32 xmllibi_parser_cdata
(
  xmllib_encoding_e_type         encoding,  /* Encoding of the XML text    */
  xmllib_metainfo_s_type        *metainfo,  /* XML meta information        */
  xmllib_parsetree_node_s_type **subtree,   /* Sub parse tree generated    */
  xmllib_error_e_type           *error_code /* error code if any           */
)
{
  int32 ret = XMLLIB_SUCCESS;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
  -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->memfree );
  XMLLIBI_DEBUG_ASSERT( NULL != subtree );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  /*-------------------------------------------------------------------------
    Allocate a node of type content.  In case of error return XMLLIB_ERROR,
    the error_code is set in the callto xmllibi_parser_nodealloc()
  -------------------------------------------------------------------------*/
  *subtree = xmllibi_parser_nodealloc( metainfo,
                                       XMLLIB_PARSETREE_NODE_CONTENT,
                                       error_code );

  if( NULL == *subtree )
  {
    return XMLLIB_ERROR;
  }

  /*-------------------------------------------------------------------------
    CDATA section consists of <![CDATA[ data ]]> , <![CDATA{ is already
    consumed  therefore expect XMLLIB_TOK_CONTENT followed by 
    XMLLIB_TOK_CDATA_CLOSE token
  -------------------------------------------------------------------------*/
  ret = xmllibi_parser_get_expected_token( encoding,
                                           metainfo,
                                           XMLLIB_TOKSTATE_CDATA,
                                           XMLLIB_TOK_CONTENT,
                                           &((*subtree)->payload.content),
                                           error_code );

  if( XMLLIB_SUCCESS == ret )
  {
    ret = xmllibi_parser_get_expected_token( encoding,
                                             metainfo,
                                             XMLLIB_TOKSTATE_CDATA,
                                             XMLLIB_TOK_CDATA_CLOSE,
                                             NULL,
                                             error_code );
  }

  /*-------------------------------------------------------------------------
    In case of error free the node
  -------------------------------------------------------------------------*/
  if( XMLLIB_ERROR == ret )
  {
    xmllib_parser_free( metainfo->memfree, *subtree );
    *subtree = NULL;
  }
  
  return ret;
} /* xmllibi_parser_cdata() */


/*===========================================================================
FUNCTION XMLLIBI_PARSER_COMMENT

DESCRIPTION
  This function is called to process a comment token.  It allocates a node
  and copies the comment to it returning it in the subtree argument.
  
DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error code set in error_code
  XMLLIB_SUCCESS in case of success with the comment node returned in
                 subtree

SIDE EFFECTS
  Allocates memory for the comment node and consumes the corresponding
  XML text.
===========================================================================*/
static int32 xmllibi_parser_comment
(
  xmllib_encoding_e_type         encoding,  /* Encoding of the XML text    */
  xmllib_metainfo_s_type        *metainfo,  /* XML meta information        */
  xmllib_parsetree_node_s_type **subtree,   /* Sub parse tree generated    */
  xmllib_error_e_type           *error_code /* error code if any           */
)
{
  int32 ret = XMLLIB_SUCCESS;
  uint32              tokbytes;
  xmllib_token_e_type token;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
  -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->memfree );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext );
  XMLLIBI_DEBUG_ASSERT( NULL != subtree );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  /*-------------------------------------------------------------------------
    Allocate a node of type comment.  In case of error return XMLLIB_ERROR,
    the error_code is set in the callto xmllibi_parser_nodealloc()
  -------------------------------------------------------------------------*/
  *subtree = xmllibi_parser_nodealloc( metainfo,
                                       XMLLIB_PARSETREE_NODE_COMMENT,
                                       error_code );

  if( NULL == *subtree )
  {
    return XMLLIB_ERROR;
  }

  /*-----------------------------------------------------------------------
    [15]  Comment ::= '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->'
    skip initial spaces S ::=   (#x20 | #x9 | #xD | #xA)+ (production 3)
    -----------------------------------------------------------------------*/
  do
  {
    /*---------------------------------------------------------------------
      Comments consist of <!-- comment -->.  The <!-- token is already
      consumed.  Expect XMLLIB_TOK_CONTENT followed by
      XMLLIB_TOK_COMMENT_CLOSE
      ---------------------------------------------------------------------*/
    ret = xmllib_tok_tokenize( encoding,
                               XMLLIB_TOKSTATE_COMMENT,
                               metainfo,
                               &token,
                               &tokbytes,
                               error_code );
    switch( token )
    {
    case XMLLIB_TOK_SPACE:
      /* consume space */
      ret = xmllibi_parser_getbytes( metainfo,
                                     NULL,
                                     tokbytes,
                                     error_code );
      break ;
    case XMLLIB_TOK_COMMENT_VALUE:
      /* consume comment value */
      ret = xmllibi_parser_getbytes( metainfo,
                                     &(*subtree)->payload.comment,
                                     tokbytes,
                                     error_code );
      ret = xmllibi_parser_get_expected_token( encoding,
                                               metainfo,
                                               XMLLIB_TOKSTATE_COMMENT,
                                               XMLLIB_TOK_COMMENT_CLOSE,
                                               NULL,
                                               error_code ) ;
      break ;
    default:
      break ;
    }
  } while( token == XMLLIB_TOK_SPACE ) ;

  /*-------------------------------------------------------------------------
    In case of error free the node
  -------------------------------------------------------------------------*/
  if( XMLLIB_ERROR == ret )
  {
    xmllib_parser_free( metainfo->memfree, *subtree );
    *subtree = NULL;
  }
  
  return ret;
} /* xmllibi_parser_comment() */

/*===========================================================================
FUNCTION XMLLIBI_PARSER_PI

DESCRIPTION
  This function is called to process a processing instruction token.  It
  allocates a node for the processing instruction and consumes the bytes
  associated with the PI returning the filled node in the subtree argument.
  
DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR   in case of error with error_code set
  XMLLIB_SUCCESS in case of success with the PI returned in subtree

SIDE EFFECTS
  Allocates memory for the PI node and consumes the corresponding
  XML text.
===========================================================================*/
static int32 xmllibi_parser_pi
(
  xmllib_encoding_e_type         encoding,  /* Encoding of the XML text    */
  xmllib_metainfo_s_type        *metainfo,  /* XML meta information        */
  xmllib_parsetree_node_s_type **subtree,   /* Sub parse tree generated    */
  xmllib_error_e_type           *error_code /* error code if any           */
)
{
  int32               ret = XMLLIB_SUCCESS;
  uint32              tokbytes;
  xmllib_token_e_type token;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
  -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->memalloc );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->memfree );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext );
  XMLLIBI_DEBUG_ASSERT( NULL != subtree );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  /*-------------------------------------------------------------------------
    Allocate a node of type PI.  In case of error return XMLLIB_ERROR,
    the error_code is set in the callto xmllibi_parser_nodealloc()
  -------------------------------------------------------------------------*/
  *subtree = xmllibi_parser_nodealloc( metainfo,
                                       XMLLIB_PARSETREE_NODE_PI,
                                       error_code );

  if( NULL == *subtree )
  {
    return XMLLIB_ERROR;
  }

  /*-------------------------------------------------------------------------
    A processing instruction consists of <? PITARGET (S instruction)? ?>.
    <? is already consumed, expect an XMLLIB_TOK_NAME followed by
    optional XMLLIB_TOK_PI_VALUE followed by XMLLIB_TOK_PI_CLOSE.
  -------------------------------------------------------------------------*/
  ret = xmllibi_parser_get_expected_token( encoding,
                                           metainfo,
                                           XMLLIB_TOKSTATE_TAG,
                                           XMLLIB_TOK_NAME,
                                           &((*subtree)->payload.pi.target),
                                           error_code );

  /*-------------------------------------------------------------------------
    If the PITARGET was successfully acquired then get the next token
  -------------------------------------------------------------------------*/
  if( XMLLIB_SUCCESS == ret )
  {
    ret = xmllib_tok_tokenize( encoding,
                               XMLLIB_TOKSTATE_PI,
                               metainfo,
                               &token,
                               &tokbytes,
                               error_code );
    if( XMLLIB_ERROR == ret )
    {
      return ret;
    }

    /*-----------------------------------------------------------------------
      This token can be the instruction or the close tag.  If it is the 
      instruction then consume it.
    -----------------------------------------------------------------------*/
    switch( token )
    {
      case XMLLIB_TOK_PI_VALUE:
        ret = xmllibi_parser_getbytes( metainfo,
                                       &(*subtree)->payload.pi.instruction,
                                       tokbytes,
                                       error_code );
        break;

      case XMLLIB_TOK_PI_CLOSE:
        break;

      case XMLLIB_TOK_INVALID:
        ret = XMLLIB_ERROR;
        break;

      default:
        ret         = XMLLIB_ERROR;
        *error_code = XMLLIB_ERROR_INVALID_TEXT;
        break;
    } /* switch( token ) */

    /*-----------------------------------------------------------------------
      Consume the processing instruction close tag.
    -----------------------------------------------------------------------*/
    if( XMLLIB_SUCCESS == ret )
    {
      ret = xmllibi_parser_get_expected_token( encoding,
                                               metainfo,
                                               XMLLIB_TOKSTATE_PI,
                                               XMLLIB_TOK_PI_CLOSE,
                                               NULL,
                                               error_code );
    }
  }

  /*-------------------------------------------------------------------------
    Free the node if an error occured
  -------------------------------------------------------------------------*/
  if( XMLLIB_ERROR == ret )
  {
    xmllib_parser_free( metainfo->memfree, *subtree );
    *subtree = NULL;
  }

  return ret;
} /* xmllibi_parser_pi() */


/*===========================================================================
FUNCTION XMLLIBI_PARSER_ELEMENT

DESCRIPTION
  This function is called to process an XML element.  It returns the subtree
  formed by the element in the subtree argument.
  
DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error_code returned in error_code
  XMLLIB_SUCCESS in case of success with the subtree returned in the subtree
                 argument

SIDE EFFECTS
  Memory is allocated for the subtree and the XML text parsed is consumed
===========================================================================*/
static int32 xmllibi_parser_element
(
  xmllib_encoding_e_type         encoding,  /* Encoding of the XML text    */
  xmllib_metainfo_s_type        *metainfo,  /* XML meta information        */
  xmllib_parsetree_node_s_type **subtree,   /* Sub parse tree generated    */
  xmllib_error_e_type           *error_code /* error code if any           */
)
{
  int32                         ret;
  xmllib_token_e_type           token;
  uint32                        tokbytes;
  int32                         endtag_seen = FALSE;
  xmllib_parsetree_node_s_type *pnode       = NULL;
  xmllib_parsetree_node_s_type *pnextnode   = NULL;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
  -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->memalloc );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->memfree );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext );
  XMLLIBI_DEBUG_ASSERT( NULL != subtree );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  /*-------------------------------------------------------------------------
    Allocate a node for this element
  -------------------------------------------------------------------------*/
  *subtree = pnode = xmllibi_parser_nodealloc( metainfo,
                                               XMLLIB_PARSETREE_NODE_ELEMENT,
                                               error_code );

  if( NULL == pnode )
  {
    return XMLLIB_ERROR;
  }

  /*-------------------------------------------------------------------------
    Parse the element tag and get the parse tree node corresponding to it.
  -------------------------------------------------------------------------*/
  ret = xmllibi_parser_starttag( encoding,
                                 metainfo,
                                 &pnode,
                                 error_code );

  /*-------------------------------------------------------------------------
    If an error occured in parsing the start tag or if the tag is an empty
    tag then do not process further.  Otherwise parse the element.
  -------------------------------------------------------------------------*/
  if( XMLLIB_SUCCESS == ret &&
      XMLLIB_PARSETREE_NODE_EMPTYELEMENT != pnode->nodetype )
  {
    do
    {
      /*---------------------------------------------------------------------
        Get the next token using the ENTITY state.  Skip the bytes in the 
        token if the token is not content, EOF or an error case.
      ---------------------------------------------------------------------*/
      ret = xmllib_tok_tokenize( encoding,
                                 XMLLIB_TOKSTATE_ENTITY,
                                 metainfo,
                                 &token,
                                 &tokbytes,
                                 error_code );
      if( XMLLIB_ERROR == ret )
      {
        break;
      }

      if( XMLLIB_TOK_INVALID != token &&
          XMLLIB_TOK_EOF     != token &&
          XMLLIB_TOK_CONTENT != token )
      {
        ret = xmllibi_parser_getbytes( metainfo,
                                       NULL,
                                       tokbytes,
                                       error_code );
      }

      /*---------------------------------------------------------------------
        Process the stream based on the token encountered
      ---------------------------------------------------------------------*/
      switch( token )
      {
        /*-------------------------------------------------------------------
          No action taken
        -------------------------------------------------------------------*/
        case XMLLIB_TOK_SPACE:
          break;

        case XMLLIB_TOK_TAG_OPEN:
          /*-----------------------------------------------------------------
            If a new start tag is encountered then this is a child element
          -----------------------------------------------------------------*/
          ret = xmllibi_parser_element( encoding,
                                        metainfo,
                                        &pnextnode,
                                        error_code );

          break;

        case XMLLIB_TOK_PI_OPEN:
          ret = xmllibi_parser_pi( encoding,
                                   metainfo,
                                   &pnextnode,
                                   error_code );
          break;

        case XMLLIB_TOK_COMMENT_OPEN:
          ret = xmllibi_parser_comment( encoding,
                                        metainfo,
                                        &pnextnode,
                                        error_code );
          break;

        case XMLLIB_TOK_CDATA_OPEN:
          ret = xmllibi_parser_cdata( encoding,
                                      metainfo,
                                      &pnextnode,
                                      error_code );
          break;

        case XMLLIB_TOK_CONTENT:
          pnextnode = xmllibi_parser_nodealloc( metainfo,
                                                XMLLIB_PARSETREE_NODE_CONTENT,
                                                error_code );
          if( NULL != pnextnode )
          {
            ret = xmllibi_parser_getbytes( metainfo,
                                           &pnextnode->payload.content,
                                           tokbytes,
                                           error_code );
          }
          else
          {
            ret = XMLLIB_ERROR;
          }
          break;

        case XMLLIB_TOK_ENDTAG_OPEN:
          /*-----------------------------------------------------------------
            Verify that the endtag is for this element (i.e. the name in the
            endtag matches the name in the start tag.  Set the endtag_seen
            flag to TRUE
          -----------------------------------------------------------------*/
          endtag_seen = TRUE;
          ret = xmllibi_parser_endtag( encoding,
                                       metainfo,
                                       subtree,
                                       error_code );
          break;

        default:
          ret         = XMLLIB_ERROR;
          *error_code = XMLLIB_ERROR_INVALID_TEXT;
          break;

      } /* switch( token ) */

      /*---------------------------------------------------------------------
        In case of success parsing the token leading to a subtree link 
        the subtree into the parse tree.  Only case in which this subtree is
        a child of the previous node is if the previous node is the node
        corresponding to the element being parsed.
      ---------------------------------------------------------------------*/
      if( XMLLIB_SUCCESS == ret && NULL != pnextnode )
      {
        if( *subtree == pnode )
        {
          pnode->payload.element.child = pnextnode;
        }
        else
        {
          pnode->sibling = pnextnode;
        }
        pnode     = pnextnode;
        pnextnode = NULL;
      }

    } while( FALSE == endtag_seen && XMLLIB_SUCCESS == ret );

    /*-----------------------------------------------------------------------
      Free the parse tree in case of error
    -----------------------------------------------------------------------*/
    if( FALSE == endtag_seen || XMLLIB_ERROR == ret )
    {
      xmllib_parser_free( metainfo->memfree, *subtree );
      *subtree = NULL;
    }
  }

  return ret;
} /* xmllibi_parser_element() */


/*===========================================================================
FUNCTION XMLLIBI_PARSER_ENTITY

DESCRIPTION
  This function is called to parse an XML entity which is the physical unit
  of XML text storage.  Each XML document must have at least one document
  entity.  It generates a parse tree from the entity and returns it in the
  subtree argument.
  
DEPENDENCIES
  None.

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error_code set to appropriate
                 value
  XMLLIB_SUCCESS in case of success with the parse tree returned in the
                 subtree argument

SIDE EFFECTS
  Parses the XML text passed, forms a parse treee and returns it
===========================================================================*/
static int32 xmllibi_parser_entity
(
  xmllib_encoding_e_type         encoding,  /* Encoding of the XML text    */
  xmllib_metainfo_s_type        *metainfo,  /* XML meta information        */
  xmllib_parsetree_node_s_type **subtree,   /* Sub parse tree generated    */
  xmllib_error_e_type           *error_code /* error code if any           */
)
{
  xmllib_parsetree_node_s_type *pnode      = NULL;
  xmllib_parsetree_node_s_type *pnextnode  = NULL;
  int32                         ret        = XMLLIB_SUCCESS;
  xmllib_token_e_type           token;
  uint32                        tokbytes;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    The do-while loop used to tokenize and parse the xml text stream.
  -------------------------------------------------------------------------*/
  do
  {
    /*-----------------------------------------------------------------------
      Call the toeknizer on the stream, if the token is neither content
      nor an error condition consume the token bytes.
    ------------------------------------------------------------------------*/
  ret = xmllib_tok_tokenize( encoding,
                               XMLLIB_TOKSTATE_ENTITY,
                               metainfo,
                               &token,
                               &tokbytes,
                               error_code );

    if( XMLLIB_ERROR == ret )
    {
      break;
    }

    if( XMLLIB_TOK_INVALID != token &&
        XMLLIB_TOK_EOF     != token &&
        XMLLIB_TOK_CONTENT != token )
    {
      xmllibi_parser_getbytes( metainfo,
                               NULL,
                               tokbytes,
                               error_code );
    }

    /*-----------------------------------------------------------------------
      Parse the stream based on the token returned
    -----------------------------------------------------------------------*/
    switch( token )
    {
      case XMLLIB_TOK_INVALID:
        ret = XMLLIB_ERROR;
        break;

      /*---------------------------------------------------------------------
        No action taken
      ---------------------------------------------------------------------*/
      case XMLLIB_TOK_EOF:
      case XMLLIB_TOK_SPACE:
        break;

      case XMLLIB_TOK_TAG_OPEN:
        ret = xmllibi_parser_element( encoding,
                                      metainfo,
                                      &pnextnode,
                                      error_code );
        break;

      case XMLLIB_TOK_PI_OPEN:
        ret = xmllibi_parser_pi( encoding,
                                 metainfo,
                                 &pnextnode,
                                 error_code );
        break;

      case XMLLIB_TOK_COMMENT_OPEN:
        ret = xmllibi_parser_comment( encoding,
                                      metainfo,
                                      &pnextnode,
                                      error_code );
        break;

      case XMLLIB_TOK_CDATA_OPEN:
        ret = xmllibi_parser_cdata( encoding,
                                    metainfo,
                                    &pnextnode,
                                    error_code );
        break;

      case XMLLIB_TOK_CONTENT:
        pnextnode = xmllibi_parser_nodealloc( metainfo,
                                              XMLLIB_PARSETREE_NODE_CONTENT,
                                              error_code );
        if( NULL != pnextnode )
        {
          ret = xmllibi_parser_getbytes( metainfo,
                                         &pnextnode->payload.content,
                                         tokbytes,
                                         error_code );
        }
        else
        {
          ret = XMLLIB_ERROR;
        }
        break;

      case XMLLIB_TOK_DTD_OPEN:
        ret = xmllibi_parser_dtd( encoding,
                                  metainfo,
                                  &pnextnode,
                                  error_code );
        break;
      case XMLLIB_TOK_XMLDECL_OPEN:
        ret = xmllibi_parser_xmldecl( encoding,
                                      metainfo,
                                      &pnextnode,
                                      error_code ) ;
        break ;
      /*---------------------------------------------------------------------
        Any other token is unexpected and causes an error
      ---------------------------------------------------------------------*/
      default:
        XMLLIB_LOG_ERR( "Invalid occurance of token %d", token);
        ret         = XMLLIB_ERROR;
        *error_code = XMLLIB_ERROR_INVALID_TEXT;
        break;
    } /* switch( token ) */

    /*-----------------------------------------------------------------------
      If a new node was allocated add that to the parse tree
    -----------------------------------------------------------------------*/
    if( XMLLIB_SUCCESS == ret && NULL != pnextnode )
    {
      if( NULL != pnode )
      {
        pnode->sibling = pnextnode;
      }
      pnode = pnextnode;
      pnode->sibling = NULL ;
      pnextnode = NULL;
    }

    /*-----------------------------------------------------------------------
      Set the parse tree head node
    -----------------------------------------------------------------------*/
    if( NULL == *subtree && NULL != pnode )
    {
      *subtree = pnode;
    }
  } while( XMLLIB_TOK_EOF != token && XMLLIB_SUCCESS == ret );

  /*-------------------------------------------------------------------------
    In case of error free the parse tree
  -------------------------------------------------------------------------*/
  if( XMLLIB_ERROR == ret )
  {
    xmllib_parser_free( metainfo->memfree, *subtree );
    *subtree = NULL;
  }

  return ret;
} /* xmllibi_parser_entity() */


/*===========================================================================
                        EXTERNAL FUNCTION DEFINITIONS
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIB_PARSER_PARSE

DESCRIPTION
  This function is the main parse routine which is passed XML text to parse.
  The function parses the text, forms a parse tree and returns that back to
  the caller.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error code set in error_code
  XMLLIB_SUCCESS in case of success with the parse tree returned in the
                 parse_tree argument

SIDE EFFECTS
  Parses the XML text passed and forms a parse treee out of it.
===========================================================================*/
int32 xmllib_parser_parse
(
  xmllib_encoding_e_type         encoding,  /* Encoding used in XML text   */
  xmllib_metainfo_s_type        *metainfo,  /* XML meta info structure     */
  xmllib_parsetree_node_s_type **parse_tree,/* The parse tree formed       */
  xmllib_error_e_type           *error_code /* Any error                   */
)
{
  int32 ret = XMLLIB_SUCCESS;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
  -------------------------------------------------------------------------*/
  if( XMLLIB_ENCODING_MAX <= encoding ||
      NULL == metainfo                ||
      NULL == metainfo->memalloc      ||
      NULL == metainfo->memfree       ||
      NULL == metainfo->xmltext       ||
      NULL == metainfo->peekbytes     ||
      NULL == metainfo->getbytes      ||
      NULL == parse_tree              ||
      NULL == error_code )
  {
    if( NULL != error_code )
    {
      *error_code = (XMLLIB_ENCODING_MAX <= encoding) ?
                      XMLLIB_ERROR_UNSUP_ENCODING     :
                      XMLLIB_ERROR_INVALID_ARGUMENTS;
    }
        
    /* Return appropriate error code */
    return XMLLIB_ERROR_INVALID_ARGUMENTS ;
  }

  *parse_tree = NULL;

#ifdef FEATURE_XMLLIB_LOG_QXDM
  /* Initialize diag */
  if (TRUE != Diag_LSM_Init(NULL))
  {
    XMLLIB_LOG_ERR("xmllib_parser_parse: Diag init failed");
  }
#endif

  /*-------------------------------------------------------------------------
    Call xmllibi_parser_entity() to parse the top level document entity and
    form the parse tree.  This is the first pass.
  -------------------------------------------------------------------------*/
  ret = xmllibi_parser_entity( encoding,
                               metainfo,
                               parse_tree,
                               error_code );

  if( XMLLIB_SUCCESS == ret )
  {
    /*-----------------------------------------------------------------------
      TODO: process the parsetree, 2nd pass, to normalize attributes,
            replace references etc.
    -----------------------------------------------------------------------*/
  }

  return ret;
} /* xmllib_parser_parse() */


/*===========================================================================
FUNCTION XMLLIB_PARSER_FREE

DESCRIPTION
  This function is called to free up an XML parse tree and all allocated
  memory.
  
DEPENDENCIES
  The memory should have been allocated by the memalloc function
  corresponding to the memfree passed

RETURN VALUE
  None.

SIDE EFFECTS
  Frees up the parse tree and all allocated memory.
===========================================================================*/
void xmllib_parser_free
(
  xmllib_memfree_fptr_type      memfree,
  xmllib_parsetree_node_s_type *pnode
)
{
  xmllib_parsetree_node_s_type       *ptmpnode   = NULL;
  xmllib_parsetree_attribute_s_type  *pattrib;
  xmllib_parsetree_attribute_s_type  *ptmpattrib;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify parameters
  -------------------------------------------------------------------------*/
  if( NULL == pnode || NULL == memfree )
  {
    return;
  }

  /*-------------------------------------------------------------------------
    Loop through the parse tree freeing memory allocated in nodes
  -------------------------------------------------------------------------*/
  while( NULL != pnode )
  {
    switch( pnode->nodetype )
    {
      case XMLLIB_PARSETREE_NODE_ELEMENT:
        /*-------------------------------------------------------------------
          If the node has children then recursively call
          xmllib_parser_free().  The empty element doesn't have children
        -------------------------------------------------------------------*/
        if( NULL != pnode->payload.element.child )
        {
          xmllib_parser_free( memfree, pnode->payload.element.child );
        }
        /* fall through */

      case XMLLIB_PARSETREE_NODE_EMPTYELEMENT:
        /*-------------------------------------------------------------------
          Free memory allocated in name and attribute list
        -------------------------------------------------------------------*/
        memfree( pnode->payload.element.name.string );
        pattrib = pnode->payload.element.attribute_list;

        while( NULL != pattrib )
        {
          memfree( pattrib->name.string );  
          memfree( pattrib->value.string );  
          ptmpattrib = pattrib->next;
          memfree( pattrib );
          pattrib = ptmpattrib;
        }
        break;

      case XMLLIB_PARSETREE_NODE_COMMENT:
        /*-------------------------------------------------------------------
          Free memory allocated in comment string
        -------------------------------------------------------------------*/
        memfree( pnode->payload.comment.string );
        break;

      case XMLLIB_PARSETREE_NODE_CONTENT:
        /*-------------------------------------------------------------------
          Free memory allocated in content string
        -------------------------------------------------------------------*/
        memfree( pnode->payload.content.string );
        break;

      case XMLLIB_PARSETREE_NODE_PI:
        /*-------------------------------------------------------------------
          Free memory allocated in processing instruction and target
        -------------------------------------------------------------------*/
        if( NULL != pnode->payload.pi.target.string )
        {
          memfree( pnode->payload.pi.target.string );
        }
        if( NULL != pnode->payload.pi.instruction.string )
        {
          memfree( pnode->payload.pi.instruction.string );
        }
        break;
          case XMLLIB_PARSETREE_NODE_XMLDECL:
        /*-------------------------------------------------------------------
          Free memory allocated in  attribute list
        -------------------------------------------------------------------*/
        pattrib = pnode->payload.element.attribute_list;

        while( NULL != pattrib )
        {
          memfree( pattrib->name.string );  
          memfree( pattrib->value.string );  
          ptmpattrib = pattrib->next;
          memfree( pattrib );
          pattrib = ptmpattrib;
        }
        break;
                
      default:
        /* confused node */
        XMLLIB_LOG_ERR("Unexpected node-type: %d in node: 0x%p",
                       pnode->nodetype,
                       pnode);
        break;
    } /* switch( pnode->nodetype ) */

    /*-----------------------------------------------------------------------
      Set node to its sibling and free memory in the node.
    -----------------------------------------------------------------------*/
    ptmpnode = pnode->sibling;
    memfree( pnode );
    pnode = ptmpnode;
  } /* while( NULL != pnode ) */

  return;
} /* xmllib_parser_free() */
#endif  /* FEATURE_XMLLIB */
