#ifndef _XMLLIBI_TOK_UTF8
#define _XMLLIBI_TOK_UTF8
/*===========================================================================

                  X M L   T O K E N I Z E R   L I B R A R Y
                        U T F 8   E N C O D I N G
                   
DESCRIPTION
 This file implements the XML tokenizer library's UTF-8 encodng functions.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2006 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllibi_tok_utf8.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/29/06   ssingh  Created module.

===========================================================================*/
/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include "comdef.h"

#ifdef FEATURE_XMLLIB
/*===========================================================================
                     TOKEN GENERATOR UTF-8 TOKEN TABLE 
===========================================================================*/
extern const xmllibi_tok_gen_table_type 
xmllibi_tok_gen_utf8_table[XMLLIB_TOK_MAX];
/*===========================================================================
                          FUNCTION DECLARATIONS
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
);


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
);


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
);


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
);


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
);

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
);


/*===========================================================================
FUNCTION XMLLIBI_TOK_UTF8_COMMENT

DESCRIPTION
  This function is called to tokenize an XML comment tag.  Valid tokens
  are:

  XMLLIB_TOK_COMMENT_CLOSE
  XMLLIB_TOK_INVALID
  XMLLIB_TOK_COMMENT_VALUE
  
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
);

#endif /* #ifdef FEATURE_XMLLIB */
#endif /* #ifndef _XMLLIBI_TOK_UTF8 */
