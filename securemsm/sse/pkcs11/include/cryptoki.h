#ifndef ___CRYPTOKI_H_INC___
#define ___CRYPTOKI_H_INC___

/**
  @file cryptoki.h
  @brief Include file for PKCS #11; Revision: 1.4.
  This is a sample file containing the top level include directives
  for building Android Cryptoki libraries and applications.

*/

/*=============================================================================
NOTE: The @brief description above does not appear in the PDF.

      The pkcs11_mainpage.dox file contains the group/module descriptions that
      are displayed in the output PDF generated using Doxygen and LaTeX. To
      edit or update any of the group/module text in the PDF, edit the
      pkcs11_mainpage.dox file or contact Tech Pubs.
=============================================================================*/

/*=============================================================================
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  Notifications and licenses are retained for attribution purposes only

  License to copy and use this software is granted provided that it is
  identified as "RSA Security Inc. PKCS #11 Cryptographic Token Interface
  (Cryptoki)" in all material mentioning or referencing this software.

  License is also granted to make and use derivative works provided that
  such works are identified as "derived from the RSA Security Inc. PKCS #11
  Cryptographic Token Interface (Cryptoki)" in all material mentioning or
  referencing the derived work.

  RSA Security Inc. makes no representations concerning either the
  merchantability of this software or the suitability of this software for
  any particular purpose. It is provided "as is" without express or implied
  warranty of any kind.

=============================================================================*/

#pragma pack(push, cryptoki, 1)

/** @addtogroup pkcs11_constants
 @{ */

/** @cond
*/

#ifdef _WIN32
/** Specifies that the function is a DLL entry point. */
#define CK_IMPORT_SPEC __declspec(dllimport)
#else
/** Specifies that the function is a DLL entry point. */
#define CK_IMPORT_SPEC
#endif

/* Defines CRYPTOKI_EXPORTS during the build of cryptoki libraries. Do
 * not define it in applications.
 */
#ifdef CRYPTOKI_EXPORTS
/* Specifies that the function is an exported DLL entry point. */
#ifdef _WIN32
/** Specifies that the function is an exported DLL entry point. */
#define CK_EXPORT_SPEC __declspec(dllexport)
#else
/** Comment needed here. */
#define CK_EXPORT_SPEC
#endif
#else
/** Comment needed here. */
#define CK_EXPORT_SPEC CK_IMPORT_SPEC
#endif

/* Ensures the calling convention for Win32 builds. */
#ifdef _WIN32
/** Ensures the calling convention for Win32 builds. */
#define CK_CALL_SPEC __cdecl
#else
/** Comment needed here. */
#define CK_CALL_SPEC
#endif
/** @endcond */

/** @} */ /* end_pkcs11_constants */

/** @addtogroup ps_macros
  @{ */

/** Makes a pointer into an object.

  @codeexample

  @code
  typedef CK_BYTE CK_PTR CK_BYTE_PTR;
  @endcode

  In a typical UNIX environment, it can be defined by:
  @code
  #define CK_PTR *
  @endcode
*/
#define CK_PTR *

/** @cond
*/
/** Makes an exportable Cryptoki library function definition out of a return
  type and a function name.

  @codeexample

  @code
  CK_DEFINE_FUNCTION(CK_RV, C_Initialize)(
    CK_VOID_PTR pReserved
  )
  {
    ...
  }
  @endcode

  In a UNIX environment, it can be defined by:
  @code
  #define CK_DEFINE_FUNCTION(returnType, name) \
    returnType name
  @endcode
*/
#define CK_DEFINE_FUNCTION(returnType, name) \
  returnType CK_EXPORT_SPEC CK_CALL_SPEC name

/** Makes a Cryptoki API function pointer declaration or
  function pointer type declaration out of a return type and a
  function name.

  @codeexample

  @code
  extern CK_DECLARE_FUNCTION(CK_RV, C_Initialize)(
    CK_VOID_PTR pReserved
  );
  @endcode

  In a UNIX environment, it can be defined by:
  @code
  #define CK_DECLARE_FUNCTION(returnType, name) \
    returnType
  @endcode
*/
#define CK_DECLARE_FUNCTION(returnType, name) \
  returnType CK_EXPORT_SPEC CK_CALL_SPEC name

/** Makes a Cryptoki API function pointer declaration or function pointer type
  declaration out of a return type and a function name.

  @codeexample{1}

  Define funcPtr to be a pointer to a Cryptoki API function that takes
  arguments args and returns CK_RV.
  @code
  CK_DECLARE_FUNCTION_POINTER(CK_RV, funcPtr)(args);
  @endcode

  @codeexample{2}

  Define funcPtrType to be the type of a pointer to a Cryptoki API function
  that takes arguments args and returns CK_RV, and then define funcPtr to be a
  variable of type funcPtrType.
  @code
  typedef CK_DECLARE_FUNCTION_POINTER(CK_RV, funcPtrType)(args);
  funcPtrType funcPtr;
  @endcode

  In a UNIX environment, it can be defined by:
  @code
  #define CK_DECLARE_FUNCTION_POINTER(returnType, name) \
    returnType (* name)
  @endcode
*/
#define CK_DECLARE_FUNCTION_POINTER(returnType, name) \
  returnType CK_IMPORT_SPEC (CK_CALL_SPEC CK_PTR name)

/** Makes a function pointer type for an application callback out of a return
  type for the callback and a name for the callback.

  @codeexample

  @code
  CK_CALLBACK_FUNCTION(CK_RV, myCallback)(args);
  @endcode

  To declare a function pointer, myCallback, to a callback that takes arguments
  args and returns CK_RV, it can be defined by:
  @code
  typedef CK_CALLBACK_FUNCTION(CK_RV, myCallbackType)(args);
  myCallbackType myCallback;
  @endcode

  In a UNIX environment, it can be defined by:
  @code
  #define CK_CALLBACK_FUNCTION(returnType, name) \
    returnType (* name)
  @endcode
*/
#define CK_CALLBACK_FUNCTION(returnType, name) \
  returnType (CK_CALL_SPEC CK_PTR name)
/** @endcond */

/** NULL pointer value.

  @codeexample

  In any ANSI/ISO C environment (and many other environments), it is best
  defined by:
  @code
  #ifndef NULL_PTR
  #define NULL_PTR 0
  #endif
  @endcode
*/
#ifndef NULL_PTR
#define NULL_PTR 0
#endif

/** @} */ /* end_addtogroup ps_macros */

#include "pkcs11.h"

#pragma pack(pop, cryptoki)

#endif /* ___CRYPTOKI_H_INC___ */
