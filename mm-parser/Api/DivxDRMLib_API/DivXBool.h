/*!
    @file
@verbatim
$Id: //source/divx/drm/sdk1.6/main/latest/inc/DivXBool.h#1 $

Copyright (c) 2006 DivX, Inc. All rights reserved.

This software is the confidential and proprietary information of
DivX, Inc. and may be used only in accordance with the terms of
your license from DivX, Inc.
@endverbatim

    This file defines a data type for boolean values. This data type
    should be used instead of standard language types to ensure greatest
    portability:<br>
    - DivXBool
    - with valid values DIVX_TRUE, and DIVX_FALSE
**/

/*  START PROTECTION  */
#ifndef _DIVXBOOL_H_
#define _DIVXBOOL_H_



/*!
    No files should be included by this file.
    Nothing but the typedef and macros should be placed in this file.
**/



/*!
    Platform independent boolean type.
**/
typedef int DivXBool;



/*!
    Platform independent boolean TRUE value.
**/
#define DIVX_TRUE       ( (DivXBool) 1 )


/*!
    Platform independent boolean FALSE value.
**/
#define DIVX_FALSE      ( (DivXBool) 0 )



#endif /* _DIVXBOOL_H_ */

/****** END OF FILE *********************************************************/
