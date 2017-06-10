/*===========================================================================

                X M L   G E N E R A T O R   L I B R A R Y
                   
DESCRIPTION
 This file implements the XML generator library.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2005, 2006, 2007 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllib_generator.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/29/07   jkl     Fixed possible overrun of static array "xmllibi_gen_fptr_tbl
09/26/05   jsh     added xmllibi_gen_fptr_tbl
09/13/05   jsh     initial creation

===========================================================================*/
/*===========================================================================

                      INCLUDE FILES FOR MODULE

===========================================================================*/
#include "comdef.h"

#ifdef FEATURE_XMLLIB
#include "xmllib_decl.h"
#include "xmllib_generator.h"
#include "xmllib_tok.h"
#include "err.h"
#include "assert.h"

/*===========================================================================
                             Internal type definitions
===========================================================================*/
/*---------------------------------------------------------------------------
  Enum of expected xmldecl consts *IN ORDER*
  These constants are originally defined in xmllib_tok.h that are used by
  tok_ascii_check_value function. They are re-used here *in specific order*
  by parser_xmldecl INTERNAL.
---------------------------------------------------------------------------*/
typedef enum xmllibi_parser_xmldecl_const_type
{
  /* ORDER OF LISTING SHOULD MATCH THAT IN PRODUCTION 23 */
  XMLLIB_PARSER_XMLDECL_VERSION  = XMLLIB_CONST_VERSION,
  XMLLIB_PARSER_XMLDECL_ENCODING = XMLLIB_CONST_ENCODING,
  XMLLIB_PARSER_XMLDECL_STDALONE = XMLLIB_CONST_STDALONE,
  XMLLIB_PARSER_XMLDECL_MAX
} xmllibi_parser_xmldecl_const_type ;

/*---------------------------------------------------------------------------
  table of expected xmldecl values *in order*
---------------------------------------------------------------------------*/
static const xmllibi_parser_xmldecl_const_type 
  xmldecl_const_tbl[XMLLIB_PARSER_XMLDECL_MAX] = 
{ 
  XMLLIB_PARSER_XMLDECL_VERSION,
  XMLLIB_PARSER_XMLDECL_ENCODING,
  XMLLIB_PARSER_XMLDECL_STDALONE 
} ;

/*===========================================================================
                        Forward Declarations 
===========================================================================*/
static int32 xmllibi_generator_node
(
  xmllib_encoding_e_type           encoding,    /* Encoding of the XML text*/
  xmllib_parsetree_node_s_type    *node,        /* parse tree comment      */
  xmllib_gen_metainfo_s_type      *metainfo,    /* XML meta information    */
  uint32                          *bytesgen,    /* num of bytes generated  */
  xmllib_error_e_type             *error_code   /* error code if any       */
);

/*===========================================================================
                        INTERNAL FUNCTION DEFINITIONS
===========================================================================*/
/*===========================================================================
                           Tag Handling Functions 
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_GENERATOR_ATTRIBUTE 

DESCRIPTION
  This function is called to process attribute list and generate 
  corresponding  tokens. These tokens are inserted at the location provided 
  by metainfo.

DEPENDENCIES
  None  

RETURN VALUE
  XMLLIB_ERROR   in case of error setting error_code
  XMLLIB_SUCCESS in case of success with the attribute bytes (name,value)
                 inserted at the location provided by metainfo

SIDE EFFECTS
  bytes generated are inserted at the location provided by metainfo
===========================================================================*/
static int32 xmllibi_generator_attribute
(
  xmllib_encoding_e_type             encoding,   /* Encoding of XML text   */
  xmllib_parsetree_attribute_s_type *attrib,     /* parse tree attribute   */
  xmllib_gen_metainfo_s_type        *metainfo,   /* XML meta information   */
  uint32                            *bytesgen,   /* num bytes generated    */
  xmllib_error_e_type               *error_code  /* error code if any      */
)
{
  int32 ret = XMLLIB_SUCCESS ;

  /*-------------------------------------------------------------------------
    verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding );
  XMLLIBI_DEBUG_ASSERT( NULL != attrib );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext );
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->putbytes );
  XMLLIBI_DEBUG_ASSERT( NULL != bytesgen );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  /*-------------------------------------------------------------------------
    generate tokens
    XMLLIB_TOK_SPACE
    XMLLIB_TOK_NAME
    XMLLIB_TOK_EQ
    XMLLIB_TOK_ATTRIB_VALUE
    according to productions [40], [41]
    -------------------------------------------------------------------------*/
  ret = xmllib_tok_generate( encoding,
                             XMLLIB_TOK_SPACE,
                             NULL,
                             metainfo,
                             bytesgen,
                             error_code );
  if( XMLLIB_SUCCESS == ret )
  {
    ret = xmllib_tok_generate( encoding,
                               XMLLIB_TOK_NAME,
                               &attrib->name,
                               metainfo,
                               bytesgen,
                               error_code );
    if( XMLLIB_SUCCESS == ret )
    {
      ret = xmllib_tok_generate( encoding,
                                 XMLLIB_TOK_EQ,
                                 NULL,
                                 metainfo,
                                 bytesgen,
                                 error_code );
      if( XMLLIB_SUCCESS == ret )
      {
        ret = xmllib_tok_generate( encoding,
                                   XMLLIB_TOK_ATTRIB_VALUE,
                                   &attrib->value,
                                   metainfo,
                                   bytesgen,
                                   error_code );
        if( XMLLIB_SUCCESS == ret && NULL != attrib->next )
        {
          ret = xmllibi_generator_attribute( encoding,
                                             attrib->next,
                                             metainfo,
                                             bytesgen,
                                             error_code );
        }
      }
    }
  }

  return ret ;
} /* xmllibi_generator_attribute() */

/*===========================================================================
                   XML Sub-component Handling Functions 
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIBI_GENERATOR_XMLDECL

DESCRIPTION
  This function is called to process an xml declaration  and generate 
  corresponding tokens. Bytes generated are inserted at the location provided
  by metainfo.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error code set in error_code
  XMLLIB_SUCESS  in case of success with the xmldecl tokens inserted
                 at the location provided by metainfo

SIDE EFFECTS
  bytes generated are inserted at the location provided by metainfo
===========================================================================*/
static int32 xmllibi_generator_xmldecl
(
  xmllib_encoding_e_type         encoding,    /* Encoding of the XML text  */
  xmllib_parsetree_node_s_type  *xmldecl_node,/* parse tree xmldecl node   */
  xmllib_gen_metainfo_s_type    *metainfo,    /* XML meta information      */
  uint32                        *bytesgen,    /* num bytes generated       */
  xmllib_error_e_type           *error_code   /* error code if any         */
)
{
  int32 ret = XMLLIB_SUCCESS;

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
  -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != xmldecl_node ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->putbytes ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != bytesgen ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != error_code ) ;
  XMLLIBI_DEBUG_ASSERT( XMLLIB_PARSETREE_NODE_XMLDECL == 
                        xmldecl_node->nodetype ) ;

  /*-------------------------------------------------------------------------
    [23] XMLDecl ::= '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
  -------------------------------------------------------------------------*/
  /*-------------------------------------------------------------------------
    generate xmldecl start token
  -------------------------------------------------------------------------*/
  ret = xmllib_tok_generate( encoding,
                             XMLLIB_TOK_XMLDECL_OPEN,
                             NULL,
                             metainfo,
                             bytesgen,
                             error_code );
  if( XMLLIB_SUCCESS == ret && 
      NULL != xmldecl_node->payload.element.attribute_list )
  {
    /*-----------------------------------------------------------------------
      generate attributes
    -----------------------------------------------------------------------*/
    ret = xmllibi_generator_attribute( encoding,
                              xmldecl_node->payload.element.attribute_list,
                                       metainfo,
                                       bytesgen,
                                       error_code );
  }
  
  if( XMLLIB_SUCCESS == ret )
  {
    /*-------------------------------------------------------------------------
      generate xmldecl close token
    -------------------------------------------------------------------------*/
    ret = xmllib_tok_generate( encoding,
                               XMLLIB_TOK_XMLDECL_CLOSE,
                               NULL,
                               metainfo,
                               bytesgen,
                               error_code );
  }

  /*-------------------------------------------------------------------------
    error if child is not null
  -------------------------------------------------------------------------*/
  if( NULL != xmldecl_node->payload.element.child )
  {
    *error_code = XMLLIB_ERROR_MALFORMED_TREE;
    XMLLIB_LOG_ERR("xmldecl node with child %p",
                   xmldecl_node->payload.element.child);
    ret = XMLLIB_ERROR;
  }

  /*-------------------------------------------------------------------------
    generate sibling
  -------------------------------------------------------------------------*/
  if( NULL != xmldecl_node->sibling )
  {
    ret = xmllibi_generator_node( encoding,
                                  xmldecl_node->sibling,
                                  metainfo,
                                  bytesgen,
                                  error_code );
  }
  return ret;
  
} /* xmllibi_generator_xmldecl() */

/*===========================================================================
FUNCTION XMLLIBI_GENERATOR_CONTENT

DESCRIPTION
  This function is called to process parse_tree content.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error code set in error_code
  XMLLIB_SUCESS  in case of success with the content tokens inserted
                 at the location provided by metainfo

SIDE EFFECTS
  bytes generated are inserted at the location provided by metainfo
===========================================================================*/
static int32 xmllibi_generator_content
(
  xmllib_encoding_e_type         encoding,    /* Encoding of the XML text  */
  xmllib_parsetree_node_s_type  *content_node,/* parse tree content node   */
  xmllib_gen_metainfo_s_type    *metainfo,    /* XML meta information      */
  uint32                        *bytesgen,    /* num bytes generated       */
  xmllib_error_e_type           *error_code   /* error code if any         */
)
{
  int32 ret = XMLLIB_SUCCESS;

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != content_node ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->putbytes ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != bytesgen ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != error_code ) ;
  XMLLIBI_DEBUG_ASSERT( XMLLIB_PARSETREE_NODE_CONTENT == 
                        content_node->nodetype ) ;


  ret = xmllib_tok_generate( encoding,
                             XMLLIB_TOK_CONTENT,
                             &content_node->payload.content,
                             metainfo,
                             bytesgen,
                             error_code );
  if( XMLLIB_SUCCESS == ret && NULL != content_node->sibling )
  {
    ret = xmllibi_generator_node( encoding,
                                  content_node->sibling,
                                  metainfo,
                                  bytesgen,
                                  error_code );
  }
  return ret;

} /* xmllibi_generator_content */

/*===========================================================================
  FUNCTION XMLLIBI_GENERATOR_PI

DESCRIPTION
  This function is called to process a processing instruction and generate 
  corresponding tokens. Bytes generated are inserted at the location provided
  by metainfo.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error code set in error_code
  XMLLIB_SUCESS  in case of success with the pi tokens inserted
                 at the location provided by metainfo

SIDE EFFECTS
  bytes generated are inserted at the location provided by metainfo
===========================================================================*/
static int32 xmllibi_generator_pi
(
  xmllib_encoding_e_type         encoding,  /* Encoding of the XML text    */
  xmllib_parsetree_node_s_type  *pi_node,   /* parse tree pi node          */
  xmllib_gen_metainfo_s_type    *metainfo,  /* XML meta information        */
  uint32                        *bytesgen,  /* num bytes generated         */
  xmllib_error_e_type           *error_code /* error code if any           */
)
{
  int32 ret = XMLLIB_SUCCESS ;

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != pi_node ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->putbytes ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != bytesgen ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != error_code ) ;
  XMLLIBI_DEBUG_ASSERT( XMLLIB_PARSETREE_NODE_PI == 
                        pi_node->nodetype ) ;

  /*-------------------------------------------------------------------------
    [16] PI ::='<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
    [17] PITarget ::= Name - (('X' | 'x') ('M' | 'm') ('L' | 'l'))
    -------------------------------------------------------------------------*/
  /*-------------------------------------------------------------------------
    generate pi open token
    -------------------------------------------------------------------------*/
  ret = xmllib_tok_generate( encoding,
                             XMLLIB_TOK_PI_OPEN,
                             NULL,
                             metainfo,
                             bytesgen,
                             error_code );
  if( XMLLIB_SUCCESS == ret )
  {
    /*-------------------------------------------------------------------------
      generate pi target
      -------------------------------------------------------------------------*/
    ret = xmllib_tok_generate( encoding,
                               XMLLIB_TOK_CONTENT,
                               &pi_node->payload.pi.target,
                               metainfo,
                               bytesgen,
                               error_code );
    if( XMLLIB_SUCCESS == ret )
    {
      /*---------------------------------------------------------------------
        generate space
        ---------------------------------------------------------------------*/
      ret = xmllib_tok_generate( encoding,
                                 XMLLIB_TOK_SPACE,
                                 NULL,
                                 metainfo,
                                 bytesgen,
                                 error_code );
      if( XMLLIB_SUCCESS == ret )
      {
        /*------------------------------------------------------------------
          generate pi instruction
          ------------------------------------------------------------------*/
        ret = xmllib_tok_generate( encoding,
                                   XMLLIB_TOK_CONTENT,
                                   &pi_node->payload.pi.instruction,
                                   metainfo,
                                   bytesgen,
                                   error_code );
        if( XMLLIB_SUCCESS == ret )
        {
          /*---------------------------------------------------------------
            generate pi close token
            ---------------------------------------------------------------*/
          ret = xmllib_tok_generate( encoding,
                                     XMLLIB_TOK_PI_CLOSE,
                                     NULL,
                                     metainfo,
                                     bytesgen,
                                     error_code );
        }
      }
    }
  }

  if( XMLLIB_SUCCESS == ret && NULL != pi_node->sibling )
  {
    ret = xmllibi_generator_node( encoding,
                                  pi_node->sibling,
                                  metainfo,
                                  bytesgen,
                                  error_code );
  }
  return ret ;

} /* xmllibi_generator_pi() */

/*===========================================================================
FUNCTION XMLLIBI_GENERATOR_COMMENT

DESCRIPTION
  This function is called to process a comment element and generate 
  corresponding tokens. Bytes generated are inserted at the location provided
  by metainfo.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error code set in error_code
  XMLLIB_SUCCESS in case of success with the comment node tokens
                 inserted at the location provided by metainfo

SIDE EFFECTS
  bytes generated are inserted at the location provided by metainfo
===========================================================================*/
static int32 xmllibi_generator_comment
(
  xmllib_encoding_e_type           encoding,    /* Encoding of the XML text*/
  xmllib_parsetree_node_s_type    *comment_node,/* parse tree comment      */
  xmllib_gen_metainfo_s_type      *metainfo,    /* XML meta information    */
  uint32                          *bytesgen,    /* num of bytes generated  */
  xmllib_error_e_type             *error_code   /* error code if any       */
)
{
  int32 ret = XMLLIB_SUCCESS ;

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != comment_node ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->putbytes ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != bytesgen ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != error_code ) ;
  XMLLIBI_DEBUG_ASSERT( XMLLIB_PARSETREE_NODE_COMMENT == 
                        comment_node->nodetype ) ;

  /*-------------------------------------------------------------------------
    [15] Comment ::= '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->' 
    XMLLIB_TOK_COMMENT_OPEN
    XMLLIB_TOK_CONTENT
    XMLLIB_TOK_COMMENT_CLOSE
    -------------------------------------------------------------------------*/
  ret = xmllib_tok_generate( encoding,
                             XMLLIB_TOK_COMMENT_OPEN,
                             NULL,
                             metainfo,
                             bytesgen,
                             error_code );
  if( XMLLIB_SUCCESS == ret )
  {
    ret = xmllib_tok_generate( encoding,
                               XMLLIB_TOK_CONTENT,
                               &comment_node->payload.comment,
                               metainfo,
                               bytesgen,
                               error_code );
    if( XMLLIB_SUCCESS == ret )
    {
      ret = xmllib_tok_generate( encoding,
                                 XMLLIB_TOK_COMMENT_CLOSE,
                                 NULL,
                                 metainfo,
                                 bytesgen,
                                 error_code );
    }
  }

  /*-------------------------------------------------------------------------
    generate sibling
    -------------------------------------------------------------------------*/
  if( NULL != comment_node->sibling )
  {
    ret = xmllibi_generator_node( encoding,
                                  comment_node->sibling,
                                  metainfo,
                                  bytesgen,
                                  error_code );
  }

  return ret;
} /* xmllibi_generator_comment() */

/*===========================================================================
FUNCTION XMLLIBI_GENERATOR_EMPTYELEMENT 

DESCRIPTION
  This function processes given parse tree empty element and generates 
  corresponding tokens. 
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error_code set to appropriate
                 value
  XMLLIB_SUCCESS in case of success with the element tokens inserted at
                 the location provided by metainfo

SIDE EFFECTS
  bytes generated are inserted at the location provided by metainfo
===========================================================================*/
static int32 xmllibi_generator_emptyelement
(
  xmllib_encoding_e_type        encoding,    /* Encoding of the XML text   */
  xmllib_parsetree_node_s_type *element_node,/* parse tree element         */
  xmllib_gen_metainfo_s_type   *metainfo,    /* XML meta information       */
  uint32                       *bytesgen,    /* number of bytes generated  */
  xmllib_error_e_type          *error_code   /* error code if any          */
)
{
  int32 ret = XMLLIB_SUCCESS ;

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
  -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != element_node ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->putbytes ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != bytesgen ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != error_code ) ;
  XMLLIBI_DEBUG_ASSERT( XMLLIB_PARSETREE_NODE_EMPTYELEMENT == 
                        element_node->nodetype );

  /*-------------------------------------------------------------------------
    generate tag open 
  -------------------------------------------------------------------------*/
  ret = xmllib_tok_generate( encoding,
                             XMLLIB_TOK_TAG_OPEN,
                             NULL,
                             metainfo,
                             bytesgen,
                             error_code ) ;
  /*-------------------------------------------------------------------------
    generate name
  -------------------------------------------------------------------------*/
  if( XMLLIB_SUCCESS == ret )
  {
    ret = xmllib_tok_generate( encoding,
                               XMLLIB_TOK_NAME,
                               &element_node->payload.element.name,
                               metainfo,
                               bytesgen,
                               error_code ) ;
    /*---------------------------------------------------------------------
      generate attribute pairs (name,value)
      ---------------------------------------------------------------------*/
    if( XMLLIB_SUCCESS == ret )
    {
      /*--------------------------------------------------------------------
        generate attribute pairs (name,value) 
        --------------------------------------------------------------------*/
      if( NULL != element_node->payload.element.attribute_list ) 
      {
        ret = xmllibi_generator_attribute( encoding,
                               element_node->payload.element.attribute_list,
                                           metainfo,
                                           bytesgen,
                                           error_code ) ;
      }
      /*--------------------------------------------------------------------
        generate tag close
        ----------------------------------------------------------------------*/
      if( XMLLIB_SUCCESS == ret ) 
      {
        ret = xmllib_tok_generate( encoding,
                                   XMLLIB_TOK_EMPTYTAG_CLOSE,
                                   NULL,
                                   metainfo,
                                   bytesgen,
                                   error_code ) ;
      }
    }
  }

  if( NULL != element_node->sibling )
  {
    ret = xmllibi_generator_node( encoding,
                                  element_node->sibling,
                                  metainfo,
                                  bytesgen,
                                  error_code );
  }
  return ret ;

} /* xmllibi_generator_emptyelement() */

/*===========================================================================
FUNCTION XMLLIBI_GENERATOR_ELEMENT

DESCRIPTION
  This function processes given parse tree element and generates 
  corresponding tokens. 
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error_code set to appropriate
                 value
  XMLLIB_SUCCESS in case of success with the element tokens inserted at
                 the location provided by metainfo

SIDE EFFECTS
  bytes generated are inserted at the location provided by metainfo
===========================================================================*/
static int32 xmllibi_generator_element
(
  xmllib_encoding_e_type        encoding,    /* Encoding of the XML text   */
  xmllib_parsetree_node_s_type *element_node,/* parse tree element         */
  xmllib_gen_metainfo_s_type   *metainfo,    /* XML meta information       */
  uint32                       *bytesgen,    /* number of bytes generated  */
  xmllib_error_e_type          *error_code   /* error code if any          */
)
{
  int32 ret = XMLLIB_SUCCESS ;

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
  -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != element_node ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->putbytes ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != bytesgen ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != error_code ) ;
  XMLLIBI_DEBUG_ASSERT( XMLLIB_PARSETREE_NODE_ELEMENT == 
                        element_node->nodetype );

  /*-------------------------------------------------------------------------
    a non-empty xml element is 
    [39] element := sTag content eTag
    start tag contains '<', name, attribute list, '>'
    end tag contains '\>'   
  -------------------------------------------------------------------------*/
  /*-------------------------------------------------------------------------
    generate start tag - tag open, name, attribute list, tag close
  -------------------------------------------------------------------------*/
  /*-------------------------------------------------------------------------
    generate tag open 
  -------------------------------------------------------------------------*/
  ret = xmllib_tok_generate( encoding,
                             XMLLIB_TOK_TAG_OPEN,
                             NULL,
                             metainfo,
                             bytesgen,
                             error_code ) ;
  /*-------------------------------------------------------------------------
    generate name
  -------------------------------------------------------------------------*/
  if( XMLLIB_SUCCESS == ret )
  {
    ret = xmllib_tok_generate( encoding,
                               XMLLIB_TOK_NAME,
                               &element_node->payload.element.name,
                               metainfo,
                               bytesgen,
                               error_code ) ;
    /*---------------------------------------------------------------------
      generate attribute pairs (name,value)
      ---------------------------------------------------------------------*/
    if( XMLLIB_SUCCESS == ret )
    {
      /*--------------------------------------------------------------------
        generate attribute pairs (name,value) 
        --------------------------------------------------------------------*/
      if( NULL != element_node->payload.element.attribute_list ) 
      {
        ret = xmllibi_generator_attribute( encoding,
                               element_node->payload.element.attribute_list,
                                           metainfo,
                                           bytesgen,
                                           error_code ) ;
      }
      /*--------------------------------------------------------------------
        generate tag close
        ----------------------------------------------------------------------*/
      if( XMLLIB_SUCCESS == ret ) 
      {
        ret = xmllib_tok_generate( encoding,
                                   XMLLIB_TOK_TAG_CLOSE,
                                   NULL,
                                   metainfo,
                                   bytesgen,
                                   error_code ) ;
      }
    }
  }

  /*-------------------------------------------------------------------------
    generate child
    child could be any of the following:
    character data
    element
    emptyelement
    cdata
    comment 
    pi
    reference
    -------------------------------------------------------------------------*/
  if( NULL != element_node->payload.element.child )
  {
      ret = xmllibi_generator_node( encoding,
                                    element_node->payload.element.child,
                                    metainfo,
                                    bytesgen,
                                    error_code );
  }

  /*-------------------------------------------------------------------------
    generate end tag - end tag open, name, tag close
    -------------------------------------------------------------------------*/
  if( XMLLIB_SUCCESS == ret )
  {
    /*---------------------------------------------------------------------
      generate end tag open 
      ---------------------------------------------------------------------*/
    ret = xmllib_tok_generate( encoding,
                               XMLLIB_TOK_ENDTAG_OPEN,
                               NULL,
                               metainfo,
                               bytesgen,
                               error_code ) ;
    /*---------------------------------------------------------------------
      generate name
      ----------------------------------------------------------------------*/
    if( XMLLIB_SUCCESS == ret )
    {
      ret = xmllib_tok_generate( encoding,
                                 XMLLIB_TOK_NAME,
                                 &element_node->payload.element.name,
                                 metainfo,
                                 bytesgen,
                                 error_code ) ;
      /*----------------------------------------------------------------------
        generate tag close
        ----------------------------------------------------------------------*/
      if( XMLLIB_SUCCESS == ret )
      {
        ret = xmllib_tok_generate( encoding,
                                   XMLLIB_TOK_TAG_CLOSE,
                                   NULL,
                                   metainfo,
                                   bytesgen,
                                   error_code ) ;
      }
    }
  }

  /*-------------------------------------------------------------------------
    generate sibling
    sibling could be any of the following:
    character data
    element
    emptyelement
    cdata
    comment 
    pi
    reference
    -------------------------------------------------------------------------*/
  if( NULL != element_node->sibling )
  {
    ret = xmllibi_generator_node( encoding,
                                  element_node->sibling,
                                  metainfo,
                                  bytesgen,
                                  error_code );
  }
  return ret ;

} /* xmllibi_generator_element() */

/*===========================================================================
                        Internal type definitions
===========================================================================*/
/*---------------------------------------------------------------------------
  table of generator internal functions
---------------------------------------------------------------------------*/
xmllibi_generator_fptr_type
xmllibi_gen_fptr_tbl[XMLLIB_PARSETREE_NODE_MAX] =
{
  /* XMLLIB_PARSETREE_NODE_ELEMENT      = 0 */
  xmllibi_generator_element, 
  /* XMLLIB_PARSETREE_NODE_EMPTYELEMENT = 1 */
  xmllibi_generator_emptyelement,
  /* XMLLIB_PARSETREE_NODE_CONTENT      = 2 */
  xmllibi_generator_content,
  /* XMLLIB_PARSETREE_NODE_PI           = 3 */
  xmllibi_generator_pi,
  /* XMLLIB_PARSETREE_NODE_COMMENT      = 4 */
  xmllibi_generator_comment,
  /* XMLLIB_PARSETREE_NODE_XMLDECL      = 5 */
  xmllibi_generator_xmldecl
};

/*===========================================================================
FUNCTION XMLLIBI_GENERATOR_NODE

DESCRIPTION
  This function is called to process an xml parsetree node.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error code set in error_code
  XMLLIB_SUCCESS in case of success 

SIDE EFFECTS
  bytes generated are inserted at the location provided by metainfo
===========================================================================*/
static int32 xmllibi_generator_node
(
  xmllib_encoding_e_type           encoding,    /* Encoding of the XML text*/
  xmllib_parsetree_node_s_type    *node,        /* parse tree comment      */
  xmllib_gen_metainfo_s_type      *metainfo,    /* XML meta information    */
  uint32                          *bytesgen,    /* num of bytes generated  */
  xmllib_error_e_type             *error_code   /* error code if any       */
)
{
  int32 ret = XMLLIB_SUCCESS;

  /*-------------------------------------------------------------------------
    Verify the arguments passed.
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( XMLLIB_ENCODING_MAX > encoding ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != node ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->xmltext ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo->putbytes ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != bytesgen ) ;
  XMLLIBI_DEBUG_ASSERT( NULL != error_code ) ; 
  if( XMLLIB_PARSETREE_NODE_MAX <= node->nodetype )
  {
    *error_code = XMLLIB_ERROR_MALFORMED_TREE;
    ret = XMLLIB_ERROR;
    XMLLIB_LOG_ERR("Invalid node type %d",node->nodetype);
  }

  /*-------------------------------------------------------------------------
    call appropriate function based on nodetype
  -------------------------------------------------------------------------*/
  ret = xmllibi_gen_fptr_tbl[node->nodetype]( encoding,
                                              node,
                                              metainfo,
                                              bytesgen,
                                              error_code );
  return ret;

} /* xmllibi_generator_node() */

/*===========================================================================
                        EXTERNAL FUNCTION DEFINITIONS
===========================================================================*/
/*===========================================================================
FUNCTION XMLLIB_GENERATOR_GENERATE

DESCRIPTION
  This function is the main generator routing which is passed XML parse
  tree as a base to generate corresponding XML string. The function 
  traverses the given parse tree, generates XML string and returns the
  number of bytes generated to the caller.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR   in case of error with the error code set in error_code
  XMLLIB_SUCCESS in case of success with number of bytes returned in 
                 bytesgen argument

SIDE EFFECTS
  None
===========================================================================*/
extern int32 xmllib_generator_generate
(
  xmllib_encoding_e_type         encoding,  /* Encoding used in XML text   */
  xmllib_parsetree_node_s_type  *parse_tree,/* parse tree                  */
  xmllib_gen_metainfo_s_type    *metainfo,  /* XML meta info structure     */
  uint32                        *bytesgen,  /* num of xml bytes generated  */
  xmllib_error_e_type           *error_code /* Any error                   */
)
{
  int32 ret = XMLLIB_SUCCESS;

  /*-------------------------------------------------------------------------
    verify arguments
    -------------------------------------------------------------------------*/
  if ( XMLLIB_ENCODING_MAX < encoding ||
       NULL == parse_tree             ||
       NULL == metainfo               ||
       NULL == metainfo->xmltext      ||
       NULL == metainfo->putbytes     ||
       NULL == bytesgen               ||
       NULL == error_code              )
  {
    if( NULL != error_code )
    {
      *error_code = XMLLIB_ERROR_INVALID_ARGUMENTS ;
    }
    return XMLLIB_ERROR ;
  }

  *bytesgen = 0;
  /*-------------------------------------------------------------------------
    generate entity
    entity could be any of the following:
    character data
    element
    emptyelement
    cdata
    comment 
    pi
  -------------------------------------------------------------------------*/
  ret = xmllibi_generator_node( encoding,
                                parse_tree,
                                metainfo,
                                bytesgen,
                                error_code );    
  return ret;

} /* xmllib_generator_generate() */

#endif  /* FEATURE_XMLLIB */
