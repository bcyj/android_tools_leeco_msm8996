#ifndef __QMM_LIST_H__
#define __QMM_LIST_H__
/*========================================================================

*//** @file qmmList.h

@par FILE SERVICES:
      This file contains unidirectional linked list interface.

@par DESCRIPTION:

      * User of the linked list shall use QMM_ListLinkType structure as the
        first element in the structures whose objects will be pushed, popped
        or searched in the list.

        e.g.
        typedef struct QMM_ListItem
        {
           QMM_ListLinkType           sListLink;  // link information object
           char                      *pFileName;  // name of the file
           char                      *pDirName;   // name of the directory
           int                        nFileSize;  // size of the file
        } QMM_ListItem;

      * User of the linked list shall implement comparison function of type
        QMM_ListCmpFuncType which is used by the searching function to
        determine if an item is in the list.

      * User of the linked list shall call qmm_ListInit() on every list
        object before using any other API.

@par EXTERNALIZED FUNCTIONS:
      See below.

    Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary. Export of this technology or software is regulated
    by the U.S. Government, Diversion contrary to U.S. law prohibited.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/mm-mux/main/MuxBaseLib/inc/qmmList.h#2 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
04/28/09   dm      Created file.
05/07/09   dm      Added list deinit.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/*===========================================================================

                      DEFINITIONS AND DECLARATIONS

===========================================================================*/

#if defined( __cplusplus )
extern "C"
{
#endif /* end of macro __cplusplus */


/* -----------------------------------------------------------------------
** Constant and Macros
** ----------------------------------------------------------------------- */

/**
 * Macros for TRUE and FALSE
 */
#define  QMM_TRUE      1
#define  QMM_FALSE     0

/**
 * Macros NULL
 */
#define  QMM_NULL      0

/* -----------------------------------------------------------------------
** Variables
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Typedefs
** ----------------------------------------------------------------------- */

/**
 * Enumeration defines the standard List Errors.
 */
typedef enum
{
   /**< No Error */
   QMM_LIST_ERROR_NONE = 0,

   /**< Bad Parameters detected */
   QMM_LIST_ERROR_BAD_PARM,

   /**< List is not initialized */
   QMM_LIST_ERROR_NOT_INITIALIZED,

   /**< List is already initialized */
   QMM_LIST_ERROR_ALREADY_INITIALIZED,

   /**< List is corrupted */
   QMM_LIST_ERROR_CORRUPTION,

   /**< Link being inserted is already present */
   QMM_LIST_ERROR_PRESENT,

   /**< Link being searched is not present */
   QMM_LIST_ERROR_NOT_PRESENT,

   /**< List is empty */
   QMM_LIST_ERROR_EMPTY_LIST,

}QMM_ListErrorType;

/**
 *
 * Typedef for linked list element size
 *
 */
typedef  unsigned char  QMM_BooleanType;

/**
 *
 * Typedef for linked list element size
 *
 */
typedef  unsigned int   QMM_ListSizeType;

/**
 *
 * Following stucture holds the next element information in the list.
 *
 */
typedef struct QMM_ListLinkType
{
   struct QMM_ListLinkType     *pNext;  /**<
                                          * Pointer to next element.
                                          */
} QMM_ListLinkType;

/**
 *
 * Following stucture holds the linked list information.
 *
 */
typedef struct
{
   QMM_ListLinkType             *pFront; /**<
                                          * Pointer to first element of the
                                          * list i.e. Start of the list.
                                          */
   QMM_ListLinkType             *pRear;  /**<
                                          * Pointer to last element of the
                                          * list i.e. End of the list.
                                          */
   QMM_ListSizeType              nSize;  /**<
                                          * Size of the list.
                                          */
   QMM_BooleanType               bInit;  /**<
                                          * true if list is initialized.
                                          */
} QMM_ListHandleType;

/**
 *
 * Typedef for List Comparasion Function which is used by the searching
 * functions to determine if an item is in the list
 * Must return QMM_LIST_ERROR_PRESENT if the element is found and the search
 * should be terminated, QMM_LIST_ERROR_NOT_PRESENT otherwise.
 *
 */
typedef QMM_ListErrorType ( *QMM_ListCmpFuncType )
(
   void *pElement,
   void *pCmpValue
);

/* ------------------------------------------------------------------------
** Functions
** ------------------------------------------------------------------------ */

/*==========================================================================

         FUNCTION:      qmm_ListInit

         DESCRIPTION:
*//**       This function will initialize the given linked list.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.

**//*     RETURN VALUE:
*//**       @return    QMM Error code

@par     SIDE EFFECTS:
                        None

===========================================================================*/
QMM_ListErrorType   qmm_ListInit
(
   QMM_ListHandleType  *pList
);

/*==========================================================================

         FUNCTION:      qmm_ListDeInit

         DESCRIPTION:
*//**       This function will de-initialize the given linked list.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.

**//*     RETURN VALUE:
*//**       @return    QMM Error code

@par     SIDE EFFECTS:
                        None

===========================================================================*/
QMM_ListErrorType   qmm_ListDeInit
(
   QMM_ListHandleType  *pList
);

/*==========================================================================

         FUNCTION:      qmm_ListPushFront

         DESCRIPTION:
*//**       This function will insert the given element at the front of the
            given list.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.
            @param[in]  pPushLink   - Pointer to the link to be inserted.

**//*     RETURN VALUE:
*//**       @return    QMM Error code

@par     SIDE EFFECTS:
                        None

===========================================================================*/
QMM_ListErrorType   qmm_ListPushFront
(
   QMM_ListHandleType  *pList,
   QMM_ListLinkType    *pPushLink
);

/*==========================================================================

         FUNCTION:      qmm_ListPushRear

         DESCRIPTION:
*//**       This function will insert the given element at the rear of the
            given list.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.
            @param[in]  pPushLink   - Pointer to the link to be inserted.

**//*     RETURN VALUE:
*//**       @return    QMM Error code

@par     SIDE EFFECTS:
                        None

===========================================================================*/
QMM_ListErrorType   qmm_ListPushRear
(
   QMM_ListHandleType  *pList,
   QMM_ListLinkType    *pPushLink
);

/*==========================================================================

         FUNCTION:      qmm_ListPopFront

         DESCRIPTION:
*//**       This function will pop a element from the front of the given list
            and return the popped link.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.
            @param[out] ppPopLink   - Pointer to a variable of link type which
                                      will be filled with pointer to the popped
                                      element.

**//*     RETURN VALUE:
*//**       @return    QMM Error code

@par     SIDE EFFECTS:
                        None

===========================================================================*/
QMM_ListErrorType   qmm_ListPopFront
(
   QMM_ListHandleType  *pList,
   QMM_ListLinkType   **ppPopLink
);

/*==========================================================================

         FUNCTION:      qmm_ListPopRear

         DESCRIPTION:
*//**       This function will pop a element from the rear of the given list
            and return the popped link.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.
            @param[out] ppPopLink   - Pointer to a pointer to the variable of
                                      link type which will be filled with
                                      pointer to the popped element.

**//*     RETURN VALUE:
*//**       @return    QMM Error code

@par     SIDE EFFECTS:
                        None

===========================================================================*/
QMM_ListErrorType   qmm_ListPopRear
(
   QMM_ListHandleType  *pList,
   QMM_ListLinkType   **ppPopLink
);

/*==========================================================================

         FUNCTION:      qmm_ListPopElement

         DESCRIPTION:
*//**       This function will pop the given element from the given list.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.
            @param[out] pPopLink    - Pointer to the variable of link type
                                      which need to be popped from the list.

**//*     RETURN VALUE:
*//**       @return    QMM Error code

@par     SIDE EFFECTS:
                        None

===========================================================================*/
QMM_ListErrorType   qmm_ListPopElement
(
   QMM_ListHandleType  *pList,
   QMM_ListLinkType    *pPopLink
);

/*==========================================================================

         FUNCTION:      qmm_ListPeekFront

         DESCRIPTION:
*//**       This function will return a element from the front of the given
            list.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.
            @param[out] ppPeekLink  - Pointer to a pointer to the variable of
                                      link type which will be filled with
                                      pointer to the picked element.

**//*     RETURN VALUE:
*//**       @return    QMM Error code

@par     SIDE EFFECTS:
                        None

===========================================================================*/
QMM_ListErrorType   qmm_ListPeekFront
(
   QMM_ListHandleType  *pList,
   QMM_ListLinkType   **ppPeekLink
);

/*==========================================================================

         FUNCTION:      qmm_ListPeekRear

         DESCRIPTION:
*//**       This function will return a element from the rear of the given
            list.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.
            @param[out] ppPeekLink  - Pointer to a pointer to the variable of
                                      link type which will be filled with
                                      pointer to the picked element.
**//*     RETURN VALUE:
*//**       @return    QMM Error code

@par     SIDE EFFECTS:
                        None

===========================================================================*/
QMM_ListErrorType   qmm_ListPeekRear
(
   QMM_ListHandleType  *pList,
   QMM_ListLinkType   **ppPeekLink
);

/*==========================================================================

         FUNCTION:      qmm_ListSize

         DESCRIPTION:
*//**       This function will return the total number of nods in the
            given list.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.
            @param[out] pSize       - Pointer to a variable of list size type
                                      which will be assigned with size of the
                                      list.

**//*     RETURN VALUE:
*//**       @return    QMM Error code

@par     SIDE EFFECTS:
                        None

===========================================================================*/
QMM_ListErrorType   qmm_ListSize
(
   QMM_ListHandleType  *pList,
   QMM_ListSizeType    *pSize
);

/*==========================================================================

         FUNCTION:      qmm_ListSize

         DESCRIPTION:
*//**       This function will return the total number of nods in the
            given list.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.
            @param[in]  pfnCmp      - Pointer to the function which implements
                                      comparison logic for the given comparison
                                      value.
            @param[in]  fnCmp       - Function which implements comparison
                                      logic for the given comparison
                                      value.
            @param[out] ppSearchLink -
                                      Pointer to a pointer to the variable of
                                      link type which will be filled with
                                      pointer to the searched element.

**//*     RETURN VALUE:
*//**       @return    QMM Error code

@par     SIDE EFFECTS:
                        None

===========================================================================*/
QMM_ListErrorType   qmm_ListSearch
(
   QMM_ListHandleType  *pList,
   QMM_ListCmpFuncType  fnCmp,
   void                *pCmpValue,
   QMM_ListLinkType   **ppSearchLink
);

/*==========================================================================

         FUNCTION:      qmm_ListIsEmpty

         DESCRIPTION:
*//**       Returns 1 if list is empty, otherwise returns 0.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.

===========================================================================*/
int   qmm_ListIsEmpty
(
   QMM_ListHandleType  *pList
);

/*==========================================================================

         FUNCTION:      qmm_ListPop

         DESCRIPTION:
*//**       Returns first entry on the list, NULL if the list is empty.

@par     DEPENDENCIES:
               None
*//*
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.

===========================================================================*/
void *qmm_ListPop
(
   QMM_ListHandleType  *pList
);

#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */


#endif /* __QMM_LIST_H__ */
