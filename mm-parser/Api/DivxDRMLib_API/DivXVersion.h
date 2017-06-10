/*!

@file
@verbatim
$Id: //source/divx/drm/sdk1.6/main/latest/inc/DivXVersion.h#1 $

Copyright (c) 2006 DivX, Inc. All rights reserved.

This software is the confidential and proprietary information of DivX, Inc. 
and may be used only in accordance with the terms of your license from
DivX, Inc.

@endverbatim

 */

#ifndef _DIVXVERSION_H
#define _DIVXVERSION_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "DivXInt.h"

/*! @brief Standard DivX component version structure 
    Example Output: LibraryA 1.0.2 Build: 234F ("Banana") 
    <pre>
    static DivXVersion *version = 
    {
        L"LibraryA",
        1, 0, 2, 
        0x234F, 
        L"Banana"
    };
    </pre>
    
*/
typedef struct _DivXVersion
{
    /*! A string signifying the exact name of the library/component*/
    const char  *officialName; 
    /*! An absolute integer value. 
     *  API Changes may or may not be backward compatible.
     */
    uint32_t        major;
    /*! An absolute integer value is set back to zero once major.
     *  API Changes must be backwards compatible.
     */
    uint32_t        minor;
    /*! An absolute integer value.
     *  Changes do not affect the API in any way.
     */
    uint32_t        fix;
    /*! An absolute integer value.
     *  Represents a unique number that identifies the build.
     */
    uint32_t        build;
    /*! A string given to a development effort. */
    const char  *codeName;
}
DivXVersion;

/*!
 Function prototype for the API function called at any time to return
 the version of the implementing library.
  
  @note the char * members of the struct do not need
        memory allocated; the pointed to location will be updated.

  @param pVersion (IN/OUT) pointer to a library structure into which values will be copied
  @return DIVXVERSION_OK for success, otherwise returns a DIVXVERSION_ error code

 */
typedef int ( *DivXVersionGetVersionType )
    (
        DivXVersion     *pVersion
    );

/*! @name DivXVersionGetVersionType Funtion Return Codes */
/*! @{ */
#define DIVXVERSION_OK                  0
#define DIVXVERSION_UNKNOWN_ERROR       1
#define DIVXVERSION_INVALID_PARAM       2
#define DIVXVERSION_INVALID_LABEL       3
/*! @} */

/*! To be used to separate the fields substituted */
#define DIVXVERSION_LABEL_SEPARATOR     '_'
/*! Default placeholder to be substituted by during release builds */
#define DIVXVERSION_LABEL_PLACEHOLDER  "03_05_01_00002_RepoMen"

#ifdef __cplusplus
};
#endif

#endif

