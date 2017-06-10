/*!
    @file
@verbatim
$Id: //source/divx/drm/sdk1.6/main/latest/inc/DivXInt.h#3 $

Copyright (c) 2006 DivX, Inc. All rights reserved.

This software is the confidential and proprietary information of
DivX, Inc. and may be used only in accordance with the terms of
your license from DivX, Inc.
@endverbatim

    This header defines platform-independent implementations of the most
    often used integer primitive data types. They are provided to allow
    the development of highly portable code and should be used within
    structures, unions, classes and API parameter lists, instead of
    the language's standard primitives:
    - uint32_t
    - uint16_t
    - uint8_t
    - int32_t
    - int16_t
    - divxint8_t

    If the <b>DIVXINT_NATIVE_64BIT</b> symbol is defined then 64-bit
    data types are expected to be supported natively be the compiler
    (i.e. the compiler provides full support for 64-bit maths and pointer
    manipulation) and the following data types are declared in addition:
    - uint64_t
    - int64_t

    For <b>GCC</b> systems, it is possible to define the <b>DIVXINT_USE_STDINT</b>
    symbol, which causes this header to include <b><stdint.h></b>, rather
    than declaring the typedefs locally. The project will need to ensure
    that the standard C libraries are locatable through the project's
    include path.

    The set of typedefs is switched by utilizing macros that are expected
    to be set by the compiler:
    - <b>_MSC_VER</b>
    - <b>__GNUC__</b>

    <br>
    <b>NOTES:</b>
    - The doxygen parsed typedefs, below, have been compiled with
    the __GNUC__ macro set to "3". The actual typedefs used when compiling
    with a Microsoft compiler will differ. Please review the source code
    to see the actual typedefs used for Microsoft compilers.
    - This header should be used in replacement of functionally similar
    headers found within the <b>DivXCommon repository</b>, such as:
    <b>types.h</b>
    <b>portab.h</b>
    <b>DivXPortable.h</b>
    <b>DivXPortableTypes.h</b>
**/

/*  START PROTECTION  */
#ifndef _DIVXINT_H_
#define _DIVXINT_H_

/*!
    No files (other than stdint.h) should be included by this file.
    Nothing but typedefs should be placed in this file.
**/


#ifdef _MSC_VER /* prevent next line creating "symbol undefined" warning */
#if ( _MSC_VER >= 600 )

/*
    _MSC_VER - assuming microsoft compiler, post VS6
                this will accommodate VS6, VS2003 and VS2005 SDKs for WIN32 platforms
*/

/*  typedef unsigned __int32    uint32_t;   */
    typedef unsigned int uint32_t;

    typedef unsigned __int16    uint16_t;

/*  typedef unsigned __int8     uint8_t;    */
    typedef unsigned char uint8_t;

    typedef __int32             int32_t;
    typedef __int16             int16_t;
    typedef __int8              divxint8_t;

#ifdef  DIVXINT_NATIVE_64BIT
    typedef unsigned __int64    uint64_t;
    typedef __int64             int64_t;
#endif  /*DIVXINT_NATIVE_64BIT*/

#define _DIVXINT_DEFINED

#endif  /*_MSC_VER >= 600*/
#endif  /*_MSC_VER*/



#ifdef __GNUC__ /* prevent next line creating "symbol undefined" warning */
#if ( !( defined __SYMBIAN32__ ) && (( __GNUC__ > 2 ) || (( __GNUC__ == 2 ) && ( __GNUC_MINOR__ >= 95 ))) )

/*
    __GNUC__ - assuming GCC, post 2.95 and not SYMBIAN32
               accommodates Linux, MacOS and many CE platforms
*/

#ifdef  _DIVXINT_DEFINED
#error "COMPILER DETECTION WAS AMBIGUOUS, DON'T KNOW WHICH TYPEDEFS TO USE"
#endif  /*_DIVXINT_DEFINED*/


#if defined( DIVXINT_USE_STDINT )


#include <stdint.h>

    /*!
        8-bit signed integer
    **/
    typedef char                divxint8_t;

#elif defined( _DIVX_DOXYGEN_PREPROCESSOR )


    /*
        This section purely for doxygen's benefit.
    */
    /*!
        64-bit signed integer; defined only if DIVXINT_NATIVE_64BIT is defined
    **/
    typedef long long           int64_t;

    /*!
        32-bit signed integer
    **/
    typedef int                 int32_t;

    /*!
        16-bit signed integer
    **/
    typedef short               int16_t;

    /*!
        8-bit signed integer
    **/
    typedef char                divxint8_t;

    /*!
        64-bit unsigned integer; defined only if DIVXINT_NATIVE_64BIT is defined
    **/
    typedef unsigned long long  uint64_t;

    /*!
        32-bit unsigned integer
    **/
    typedef unsigned int        uint32_t;

    /*!
        16-bit unsigned integer
    **/
    typedef unsigned short      uint16_t;

    /*!
        8-bit unsigned integer
    **/
    typedef unsigned char       uint8_t;


#else   /* !DIVXINT_USE_STDINT && !_DIVX_DOXYGEN_PREPROCESSOR */


#include <sys/types.h>

    /*
        <sys/types.h> has performed intN_t typedefs
        now we just need to do C99 style uintN_t
    */

    /*!
        32-bit unsigned integer
    **/
#ifndef __uint32_t_defined
#define __uint32_t_defined
    typedef u_int32_t           uint32_t;
#endif
    /*!
        16-bit unsigned integer
    **/
#ifndef __uint16_t_defined
#define __uint16_t_defined
    typedef u_int16_t           uint16_t;
#endif
    /*!
        8-bit unsigned integer
    **/
#ifndef __uint8_t_defined
#define __uint8_t_defined
    typedef u_int8_t            uint8_t;
#endif

#ifdef  DIVXINT_NATIVE_64BIT

    /*!
        64-bit unsigned integer
    **/
    typedef u_int64_t           uint64_t;

#endif  /*DIVXINT_NATIVE_64BIT*/

#endif  /*DIVXINT_USE_STDINT*/

#define _DIVXINT_DEFINED

#endif  /*__GNUC__ > 2.95 */


#if ( ( defined __SYMBIAN32__ ) && (( __GNUC__ > 2 ) || (( __GNUC__ == 2 ) && ( __GNUC_MINOR__ >= 9 ))) )

/*
    __GNUC__ - assuming GCC, post 2.9 and SYMBIAN define
               accommodates Symbian platforms, S60, S60 3rd, UIQ 3.0 and UIQ 2.0
*/

#ifdef  _DIVXINT_DEFINED
#error "COMPILER DETECTION WAS AMBIGUOUS, DON'T KNOW WHICH TYPEDEFS TO USE"
#endif  /*_DIVXINT_DEFINED*/


    /*
        This section purely for doxygen's benefit.
    */
    /*!
        64-bit signed integer; always defined
    **/
    typedef long long           int64_t;

    /*!
        32-bit signed integer
    **/
    typedef int                 int32_t;

    /*!
        16-bit signed integer
    **/
    typedef short               int16_t;

    /*!
        8-bit signed integer
    **/
    typedef char                divxint8_t;

    /*!
        64-bit unsigned integer; always defined
    **/
    typedef unsigned long long  uint64_t;

    /*!
        32-bit unsigned integer
    **/
    typedef unsigned long       uint32_t;

    /*!
        16-bit unsigned integer
    **/
    typedef unsigned short      uint16_t;

    /*!
        8-bit unsigned integer
    **/
    typedef unsigned char       uint8_t;

#define _DIVXINT_DEFINED

#endif  /*__GNUC__ >= 2.9 && __SYMBIAN32__ */

#endif  /*__GNUC__*/



/*
    ALL
*/

#ifndef _DIVXINT_DEFINED
  //#error  "THE COMPILER YOU ARE USING WAS NOT RECOGNIZED"
  #define _DIVXINT_DEFINED
  typedef unsigned char  uint8_t;
  typedef unsigned short uint16_t;
  typedef unsigned int   uint32_t;
  typedef char           divxint8_t;  
  typedef short          int16_t;
  typedef int            int32_t;
  #ifdef  DIVXINT_NATIVE_64BIT
    typedef unsigned long long    uint64_t;
    typedef long long             int64_t;
  #endif  /*DIVXINT_NATIVE_64BIT*/

#else   /*_DIVXINT_DEFINED*/
  #undef  _DIVXINT_DEFINED  /* remove this label, it's purpose fulfilled */
#endif  /*_DIVXINT_DEFINED*/



#endif /* _DIVXINT_H_ */

/****** END OF FILE *********************************************************/
