#ifndef _XMLLIB_GENERATOR_H
#define _XMLLIB_GENERATOR_H
/*===========================================================================

          X M L   G E N E R A T O R   L I B R A R Y   H E A D E R
                   
DESCRIPTION
 This file includes declarations for the XML generator library.
 
EXTERNALIZED FUNCTIONS
 xmllib_generator_generate
 xmllib_generator_free

Copyright (c) 2005 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllib_generator.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/26/05   jsh     added xmllibi_generator_fptr_type
09/13/05   jsh     initial creation

===========================================================================*/
/*===========================================================================

                      INCLUDE FILES FOR MODULE

===========================================================================*/
#include "comdef.h"

#ifdef FEATURE_XMLLIB
#include "xmllib_common.h"
#include "xmllib_tok.h"


/*===========================================================================
                        GENERATOR DATA TYPES
===========================================================================*/
/*-------------------------------------------------------------------------
  TYPEDEF XMLLIB_GENERATOR_GENERATE_FPTR_TYPE
  This type is used by generator functions table.
-------------------------------------------------------------------------*/
typedef int32 (* xmllibi_generator_fptr_type)
(
  xmllib_encoding_e_type         encoding,  /* Encoding used in XML text   */
  xmllib_parsetree_node_s_type  *parse_tree,/* parse tree                  */
  xmllib_gen_metainfo_s_type    *metainfo,  /* XML meta info structure     */
  uint32                        *bytesgen,  /* num of xml bytes generated  */
  xmllib_error_e_type           *error_code /* Any error                   */
);

/*===========================================================================
                           FUNCTION DECLARATIONS
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
);

#endif  /* FEATURE_XMLLIB */
#endif /* #ifndef _XMLLIB_GENERATOR_H */
