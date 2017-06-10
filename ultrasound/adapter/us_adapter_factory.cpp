/*===========================================================================
                           us_adapter_factory.cpp

DESCRIPTION: Implement a factory for adapters.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include "us_adapter_factory.h"
#include <stdlib.h>
#include <dlfcn.h>

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
 * Holds reference to the opened dynamic library
 */
static void *adapterlib = NULL;

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  Function implementation
------------------------------------------------------------------------------*/
/*============================================================================
  FUNCTION:  create_adapter
============================================================================*/
/**
 * See function definition in header file
 */
int create_adapter(FrameworkAdapter **adapter,
                   char *lib_path)
{
  if (NULL == adapter ||
      NULL == lib_path)
  {
    LOGE("%s: Invalid parameters",
         __FUNCTION__);
    return -1;
  }

  if (0 == strcmp(lib_path,
                  ""))
  { // No library is chosen, standalone gesture service should run
    LOGI("%s: lib_path is empty, standalone gesture service will run",
         __FUNCTION__);
    *adapter = NULL;
    return 0;
  }

  // Open dynamic library
  adapterlib = dlopen(lib_path,
                      RTLD_LAZY);
  if (NULL == adapterlib)
  {
    LOGE("%s: Cannot load library dynamically: %s",
         __FUNCTION__,
         lib_path);
    *adapter = NULL;
    return 1;
  }

  // Reset errors
  dlerror();

  // Load the symbols
  get_adapter_t *get_adapter = (get_adapter_t*) dlsym(adapterlib,
                                                      "get_adapter");
  const char *dlsym_error = dlerror();
  if (dlsym_error || NULL == get_adapter)
  {
    LOGE("%s: Cannot load symbol get_adapter %s or get_adapter is null",
         __FUNCTION__,
         dlsym_error);
    *adapter = NULL;
    return 1;
  }

  // Create an instance to the class
  LOGI("%s: Running Gesture Framework",
         __FUNCTION__);
  *adapter =  get_adapter();
  if (NULL == *adapter ||
      FAILURE == (*adapter)->get_status())
  {
    LOGE("%s: Could not init adapter",
         __FUNCTION__);
    *adapter = NULL;
    return 1;
  }
  return 0;
}

/*============================================================================
  FUNCTION:  destroy_adapter
============================================================================*/
/**
 * See function definition in header file
 */
void destroy_adapter()
{
  dlclose(adapterlib);
}

