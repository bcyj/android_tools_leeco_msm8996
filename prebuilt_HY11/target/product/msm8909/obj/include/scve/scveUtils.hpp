/**=============================================================================

@file
scveUtils.hpp

@brief
SCVE API Definition for Utils.

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================**/

//=============================================================================
///@defgroup scveUtils Utilities
///@brief Defines API for SCVE-Utilities that are used along with SCVE features.
///@ingroup scve
//=============================================================================

#ifndef SCVE_UTILS_HPP
#define SCVE_UTILS_HPP

#include "scveTypes.hpp"

namespace SCVE
{

#define MAX_VERSION_LEN 16

//------------------------------------------------------------------------------
/// @brief
///    Utils class implements utilities to help in using SCVE features.
///
/// @details
///    Currently includes the following utilities:
///    - Device Memory Allocation and Deallocation.
///
/// @ingroup scveUtils
//------------------------------------------------------------------------------
class SCVE_API Utils
{
   public:
      //------------------------------------------------------------------------------
      /// @brief
      ///    This API allocates memory using ION that is needed for Q6 and GPU. If
      ///    the user does prefers to allocate memory using HEAP, then there will be
      ///    performance impact due to memory copy from HEAP to ION
      //------------------------------------------------------------------------------
      static void* AllocateDeviceMemory (uint32_t nBytes);

      //------------------------------------------------------------------------------
      /// @brief
      ///    This API frees memory that is allocated by AllocateDeviceMemory
      //------------------------------------------------------------------------------
      static void FreeDeviceMemory (void* pPtr);

      //------------------------------------------------------------------------------
      /// @brief
      ///    Get's the current version of SCVE library.
      //------------------------------------------------------------------------------
      static void GetVersion (char *pVersion, uint32_t pBufferLength);

      virtual ~Utils() = 0;

}; //class Utils

} //namespace SCVE

#endif //SCVE_UTILS_HPP