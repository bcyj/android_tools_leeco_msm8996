/*===========================================================================
                           usf_dynamic_lib_proxy.cpp

DESCRIPTION: Implement a dynamic library proxy.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include "usf_dynamic_lib_proxy.h"
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

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  Function implementation
------------------------------------------------------------------------------*/

/*============================================================================
  DESTRUCTOR:
============================================================================*/
/**
 * See function definition in header file
 */
UsfDynamicLibProxy::~UsfDynamicLibProxy()
{
  dlclose(m_dynamic_lib);
}

/*============================================================================
  FUNCTION:  open_lib
============================================================================*/
/**
 * See function definition in header file
 */
bool UsfDynamicLibProxy::open_lib(const char *lib_path)
{
  if (NULL == lib_path)
  {
    LOGE("%s: Invalid parameters",
         __FUNCTION__);
    return false;
  }

  // Open dynamic library
  m_dynamic_lib = dlopen(lib_path,
                         RTLD_LAZY);
  if (NULL == m_dynamic_lib)
  {
    LOGE("%s: Cannot load library dynamically: %s",
         __FUNCTION__,
         lib_path);
    return false;
  }

  // Load all methods
  if (!load_all_methods())
  {
    LOGE("%s: Cannot load library methods",
         __FUNCTION__);
    return false;
  }

  return true;
}

/*============================================================================
  FUNCTION:  get_method
============================================================================*/
/**
 * See function definition in header file
 */
void *UsfDynamicLibProxy::get_method(const char *method_name)
{
  if (NULL == method_name)
  {
    LOGE("%s: Invalid parameters",
         __FUNCTION__);
    return NULL;
  }

  // Reset errors
  dlerror();

  // Load the symbols
  void *method = dlsym(m_dynamic_lib,
                       method_name);

  const char *dlsym_error = dlerror();
  if (dlsym_error || NULL == method)
  {
    LOGW("%s: Cannot load symbol - %s %s",
         __FUNCTION__,
         dlsym_error,
         method_name);
    return NULL;
  }

  return method;
}

