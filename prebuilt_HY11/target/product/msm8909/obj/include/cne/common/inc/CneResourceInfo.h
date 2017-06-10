#ifndef CNE_RESOURCE_INFO_H
#define CNE_RESOURCE_INFO_H

/*============================================================================
  @file CneResourceInfo.h

  Skeleton documentation example

Provide a description of the functionality implemented in this file. Give a
detailed enough overview that a user browsing this file, having never seen
it before, would have a good idea of whether or not this file provides what
the user is looking for.

The Class Definitions section is applicable only to C++ code and should be
removed for C.

               Copyright (c) 2009,2010 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Confidential and Proprietary
============================================================================*/



/*============================================================================
  EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.  Please
  use ISO format for dates.

  $Header: //depot/asic/sandbox/projects/cne/common/core/inc/CneResourceInfo.h#7 $
  $DateTime: 2009/11/10 15:59:14 $
  $Author: chinht $
  $Change: 1080815 $

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2009-09-13  cht  First revision.

============================================================================*/


/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "CneDefs.h"
#include "CneSrmDefs.h"
#include <vector>
#include <iterator>
#include <list>

using namespace std;

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Class Definitions
 * -------------------------------------------------------------------------*/
class CneResourceInfo
{
  public:
  virtual ~CneResourceInfo() {};
  virtual CneRetType putResourceToTable (void *res ) = 0;
  virtual int getListResourceFromTable (int numItems, void *res) = 0;
};

#endif /* CNE_RESOURCE_INFO_H */
