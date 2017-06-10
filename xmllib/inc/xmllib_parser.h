#ifndef _XMLLIB_PARSER_H
#define _XMLLIB_PARSER_H
/*===========================================================================

              X M L   P A R S E R   L I B R A R Y   H E A D E R
                   
DESCRIPTION
 This file includes declarations for the XML parser library.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2005 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/inc/xmllib_parser.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/09/05   jsh     Moved common data structures to xmllib_common.h
09/02/05   jsh     Added XMLLIB_XML_VER
09/02/05   jsh     Added XMLLIB_PARSETREE_NODE_XMLDECL
10/30/03   ifk     Created module.

===========================================================================*/
/*===========================================================================

                      INCLUDE FILES FOR MODULE

===========================================================================*/
#include "comdef.h"

#ifdef FEATURE_XMLLIB
#include "xmllib_common.h"
#include "xmllib_tok.h"

/*===========================================================================
                           FUNCTION DECLARATIONS
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
extern int32 xmllib_parser_parse
(
  xmllib_encoding_e_type         encoding,  /* Encoding used in XML text   */
  xmllib_metainfo_s_type        *metainfo,  /* XML meta info structure     */
  xmllib_parsetree_node_s_type **parse_tree,/* The parse tree formed       */
  xmllib_error_e_type           *error_code /* Any error                   */
);

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
extern void xmllib_parser_free
(
  xmllib_memfree_fptr_type      memfree,
  xmllib_parsetree_node_s_type *pnode
);
#endif  /* FEATURE_XMLLIB */
#endif /* #ifndef _XMLLIB_PARSER_H */
