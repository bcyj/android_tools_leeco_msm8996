/*!
  @file
  configdb_xml.h

  @brief
  This file declares the xml specific parsing functionality.

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

#ifndef _CONFIGDB_XML_H
#define _CONFIGDB_XML_H

#include "comdef.h"
#include "configdbi.h"


/*===========================================================================
  FUNCTION:  configdb_xml_init
  ===========================================================================*/
  /*!
      @brief
      This function parses the given xml_file, constructs a configdb tree and
      returns a pointer to the root of the tree via cfgdb_root

      @params
      xml_file   [in]  - XML file from which to construct the configdb tree
      cfgdb_root [out] - Root of the configdb tree

      @return
      CFGDB_ENOTFOUND
      CFGDB_EBADPARAM
      CFGDB_EMEMORY
      CFGDB_EFORMAT
      CFGDB_EFAIL
      CFGDB_SUCCESS
  */
/*=========================================================================*/
int32 configdb_xml_init(
   const char *xml_file,
   configdb_node_t **cfgdb_root
);

#endif /* _CONFIGDB_XML_H */

