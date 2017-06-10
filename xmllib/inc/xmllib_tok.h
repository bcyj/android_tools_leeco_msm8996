#ifndef _XMLLIB_TOK_H
#define _XMLLIB_TOK_H
/*===========================================================================

           X M L   T O K E N I Z E R   L I B R A R Y   H E A D E R
                   
DESCRIPTION
 This file includes declarations for the XML tokenizer library.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2005 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/inc/xmllib_tok.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/02/05   jsh     Added xmllib_tok_check_value function prototype
09/02/05   jsh     Added xmllib_const_e_type
10/30/03   ifk     Created module.

===========================================================================*/
/*===========================================================================
                            INCLUDE FILES
===========================================================================*/
#include "comdef.h"

#ifdef FEATURE_XMLLIB
#include "xmllib_common.h"


/*===========================================================================
                            TYPE DECLARATIONS
===========================================================================*/
/*---------------------------------------------------------------------------
  typedef of tokens returned by XML tokenizer
---------------------------------------------------------------------------*/
typedef enum xmllib_token_e_type
{
  XMLLIB_TOK_INVALID        = -1,      /* Error                            */
  XMLLIB_TOK_EOF            =  0,      /* End of XML text                  */
  XMLLIB_TOK_SPACE          =  1,      /* White space charachters          */
  XMLLIB_TOK_TAG_OPEN       =  2,      /* '<'                              */
  XMLLIB_TOK_PI_OPEN        =  3,      /* '<?'                             */
  XMLLIB_TOK_XMLDECL_OPEN   =  4,      /* '<?xml'                          */
  XMLLIB_TOK_COMMENT_OPEN   =  5,      /* '<!--'                           */
  XMLLIB_TOK_CDATA_OPEN     =  6,      /* '<![CDATA['                      */
  XMLLIB_TOK_DTD_OPEN       =  7,      /* '<!DOCTYPE S'                    */
  XMLLIB_TOK_ENDTAG_OPEN    =  8,      /* '</'                             */
  XMLLIB_TOK_TAG_CLOSE      =  9,      /* '>'                              */
  XMLLIB_TOK_PI_CLOSE       =  10,     /* '?>'                             */
  XMLLIB_TOK_XMLDECL_CLOSE  =  10,     /* '?>'                             */
  XMLLIB_TOK_CDATA_CLOSE    =  11,     /* ']]>'                            */
  XMLLIB_TOK_EMPTYTAG_CLOSE =  12,     /* '/>'                             */
  XMLLIB_TOK_COMMENT_CLOSE  =  13,     /* '-->'                            */
  XMLLIB_TOK_ENTITY_REF     =  14,     /* '& Name ;'                       */
  XMLLIB_TOK_CHAR_REF       =  15,     /* '&# [0-9]+ ;' | '&#x [0-9]+ ;'   */
  XMLLIB_TOK_CONTENT        =  16,     /* uninterpreted content data       */
  XMLLIB_TOK_EQ             =  17,     /* =                                */
  XMLLIB_TOK_NAME           =  18,     /* Name                             */
  XMLLIB_TOK_ATTRIB_VALUE   =  19,     /* Attribute value                  */
  XMLLIB_TOK_PI_VALUE       =  20,     /* PI value                         */
  XMLLIB_TOK_COMMENT_VALUE  =  21,     /* Comment content                  */
  XMLLIB_TOK_CREF_VALUE     =  22,     /* Charachter reference value       */
  XMLLIB_TOK_REF_CLOSE      =  24,     /* ';'                              */
  XMLLIB_TOK_DTD_MUP_OPEN   =  25,     /* DTD markup declaration section   */
  XMLLIB_TOK_DTD_MUP_CLOSE  =  26,     /* DTD markup section close         */
  XMLLIB_TOK_MAX
} xmllib_token_e_type;

/*---------------------------------------------------------------------------
  typedef of XML substructures (states) processed by XML tokenizer
---------------------------------------------------------------------------*/
typedef enum xmllib_tok_state_e_type
{
  XMLLIB_TOKSTATE_ENTITY    =  0,  /* Tokenizing entity                    */
  XMLLIB_TOKSTATE_TAG       =  1,  /* Tokenizing contents of a tag         */
  XMLLIB_TOKSTATE_CDATA     =  2,  /* Tokenizing CDATA section             */
  XMLLIB_TOKSTATE_DTD       =  3,  /* Tokenizing DTD                       */
  XMLLIB_TOKSTATE_COMMENT   =  4,  /* Tokenizing comment                   */
  XMLLIB_TOKSTATE_PI        =  5,  /* Tokenizing processing instruction    */
  XMLLIB_TOKSTATE_XMLDECL   =  6,  /* Tokenizing xmldecl element           */
  XMLLIB_TOKSTATE_MAX
} xmllib_tok_state_e_type;

/*---------------------------------------------------------------------------
  typedef of XML generator encoding specific token node.
---------------------------------------------------------------------------*/
typedef struct 
{
  boolean   tok_supported; /* This token supported?          */
  char    * prefix;        /* Constant bytes, token specific */
  uint32    size;          /* Size of prefix                 */
  boolean   bytes_used;    /* Are passed in bytes used?      */
} xmllibi_tok_gen_table_type;

/*===========================================================================
                          FUNCTION DECLARATIONS
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
);

/*===========================================================================
FUNCTION XMLLIB_TOK_GENERATE

DESCRIPTION
  This function generates the required token that consists of token tags and
  token text. Argument xmlbytes contains the text while token (type) decides 
  what tags to use.
  
DEPENDENCIES
  None

RETURN VALUE
  XMLLIB_ERROR    in case of an error with the error code set in the
                  error_code argument.
  XMLLIB_SUCCESS  in case of success with the relevant token inserted at the
                  xmltext location specified in metainfo structure.

SIDE EFFECTS
  None
===========================================================================*/
int32 xmllib_tok_generate
(
  xmllib_encoding_e_type      encoding,     /* encoding                    */
  xmllib_token_e_type         token,        /* token to be generated       */
  xmllib_string_s_type       *bytes,        /* token bytes                 */
  xmllib_gen_metainfo_s_type *metainfo,     /* XML meta information        */
  uint32                     *bytesgen,     /* bytes generated             */
  xmllib_error_e_type        *error_code    /* error code, if any          */
);

#endif  /* FEATURE_XMLLIB */
#endif /* #ifndef _XMLLIB_TOK_H */
