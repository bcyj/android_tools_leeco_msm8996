/*****************************************************************************/
/*                                                                           */
/*                             Standard Files                                */
/*                  ITTIAM SYSTEMS PVT LTD, BANGALORE                        */
/*                          COPYRIGHT(C) 2006                                */
/*                                                                           */
/*  This program  is  proprietary to  Ittiam  Systems  Private  Limited  and */
/*  is protected under Indian  Copyright Law as an unpublished work. Its use */
/*  and  disclosure  is  limited by  the terms  and  conditions of a license */
/*  agreement. It may not be copied or otherwise  reproduced or disclosed to */
/*  persons outside the licensee's organization except in accordance with the*/
/*  the  terms  and  conditions   of  such  an  agreement.  All  copies  and */
/*  reproductions shall be the property of Ittiam Systems Private Limited and*/
/*  must bear this notice in its entirety.                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  File Name         : ittiam_datatypes.h                                   */
/*                                                                           */
/*  Description       : This file has the definitions of the data types used */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes (Describe the changes made)  */
/*         06 04 2006   Malavika        Draft                                */
/*                                                                           */
/*****************************************************************************/

#ifndef _ITTIAM_DATATYPES_H_
#define _ITTIAM_DATATYPES_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************/
/* Unsigned data types                                                       */
/*****************************************************************************/
typedef unsigned char   UWORD8;
typedef unsigned short  UWORD16;
typedef unsigned int    UWORD32;
typedef unsigned long long   UWORD64;

typedef unsigned char   UWord8;
typedef unsigned short  UWord16;
typedef unsigned int    UWord32;

/*****************************************************************************/
/* Signed data types                                                         */
/*****************************************************************************/
typedef signed char     WORD8;
typedef short           WORD16;
typedef int             WORD32;

typedef signed char     Word8;
typedef short           Word16;
typedef int             Word32;

/*****************************************************************************/
/* Miscellaneous data types                                                  */
/*****************************************************************************/
typedef char            BOOL;
typedef char           *pSTRING;

#ifdef __cplusplus
}
#endif

#endif /*_ITTIAM_DATATYPES_H_*/
