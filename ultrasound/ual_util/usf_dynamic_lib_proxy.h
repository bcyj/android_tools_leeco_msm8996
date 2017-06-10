/*===========================================================================
                           usf_dynamic_lib_proxy.h

DESCRIPTION: Provide a dynamic library proxy.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef __USF_DYNAMIC_LIB_PROXY__
#define __USF_DYNAMIC_LIB_PROXY__

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Classes
-----------------------------------------------------------------------------*/

class UsfDynamicLibProxy
{
private:
  /**
   * Holds reference to the opened dynamic library
   */
  void *m_dynamic_lib;

public:
  /*============================================================================
    DESTRUCTOR:  ~UsfDynamicLibProxy
  ============================================================================*/
  /**
   * closes the dynamic library.
   */
  virtual ~UsfDynamicLibProxy();

  /*============================================================================
    FUNCTION:  open_lib
  ============================================================================*/
  /**
   * Opens the desired dynamic library.
   *
   * @param lib_path The library path.
   *
   * @return bool - true success
   *                false failure
   */
  bool open_lib(const char *lib_path);

protected:
  /*============================================================================
    FUNCTION:  get_method
  ============================================================================*/
  /**
   * Returns the address where that method symbol is loaded into.
   * This method should only be called by inherited class which
   * implements a specific cast to the returned method.
   *
   * @param method_name The method name.
   *
   * @return address where that symbol is loaded into memory or null for failure
   */
  void* get_method(const char *method_name);

  /*============================================================================
    FUNCTION:  load_all_methods
  ============================================================================*/
  /**
   * Loads all the methods from the library.
   *
   * @return bool - true success
   *                false failure
   */
  virtual bool load_all_methods() = 0;

};

#endif //__USF_DYNAMIC_LIB_PROXY__
