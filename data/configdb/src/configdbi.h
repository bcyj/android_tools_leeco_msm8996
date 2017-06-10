/*!
  @file
  configdbi.h

  @brief
  This file declares the configuration parser internal definitions.

*/
/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/07/11   sg      initial version

===========================================================================*/

#ifndef _CONFIGDBI_H
#define _CONFIGDBI_H

#include "comdef.h"


/* Type definition for a configdb aggregate node */
typedef struct configdb_node_t configdb_node_t;
struct configdb_node_t
{
   char *name;                /* Name of the node */
   char *type;                /* Type of the node */
   char *value;               /* Value associated with the node */
   uint32 child_count;        /* Number of children */
   configdb_node_t *parent;   /* Pointer to the parent */
   configdb_node_t *child;    /* Pointer to the first child */
   configdb_node_t *sibling;  /* Pointer to the next sibling */
};


/* Type definition for a type converter node */
typedef struct configdb_converter_node_t configdb_converter_node_t;
struct configdb_converter_node_t
{
   const char *type;                      /* Type handled by the converter */
   configdb_type_converter_t converter;   /* Converter function */
   configdb_converter_node_t *next;       /* Pointer to the next node */
};

#endif /* _CONFIGDBI_H */

