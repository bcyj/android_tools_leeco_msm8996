/*****************************************************************************/
/*                                                                           */
/*                               PROJECT NAME                                */
/*                     ITTIAM SYSTEMS PVT LTD, BANGALORE                     */
/*                             COPYRIGHT(C) 2005                             */
/*                                                                           */
/*  This program  is  proprietary to  Ittiam  Systems  Private  Limited  and */
/*  is protected under Indian  Copyright Law as an unpublished work. Its use */
/*  and  disclosure  is  limited by  the terms  and  conditions of a license */
/*  agreement. It may not be copied or otherwise  reproduced or disclosed to */
/*  persons outside the licensee's organization except in accordance with the*/
/*  terms  and  conditions   of  such  an  agreement.  All  copies  and      */
/*  reproductions shall be the property of Ittiam Systems Private Limited and*/
/*  must bear this notice in its entirety.                                   */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*  File Name         : datatypedef.h                                        */
/*                                                                           */
/*  Description       : This file contains all the necessary data type       */
/*						definitions.										 */
/*                                                                           */
/*  List of Functions : None					          		             */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes (Describe the changes made)  */
/*         29 12 2006  Rajendra C Y          Draft                           */
/*                                                                           */
/*****************************************************************************/

#ifndef DATATYPEDEF_H
#define DATATYPEDEF_H

/*****************************************************************************/
/* Typedefs                                                                  */
/*****************************************************************************/

typedef int             WORD32;
typedef unsigned int    UWORD32;

typedef short           WORD16;
typedef unsigned short  UWORD16;

typedef char            WORD8;
typedef unsigned char   UWORD8;

#ifndef NULL
#define NULL            ((void *)0)

#endif

typedef enum
{
    IT_FALSE,
    IT_TRUE
} IT_BOOL;


typedef enum
{
    IT_OK,
    IT_ERROR = -1
} IT_STATUS;

#ifdef MSVC
typedef _int64          WORD64;
typedef int             FILE_TYPE;
#else
typedef long long int   WORD64;
typedef void*           FILE_TYPE;
#endif

typedef struct{
    UWORD32   lsw;
    UWORD32   msw;
}UWORD64;

//typedef UWORD32 time_t;

/*****************************************************************************/
/* Input and Output Parameter identifiers                                    */
/*****************************************************************************/
#define                 IT_IN           
#define                 IT_OUT


#endif /* DATATYPEDEF_H */

