/*========================================================================

*//** @file qmmList.c

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

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/mm-mux/main/MuxBaseLib/src/qmmList.c#2 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
03/15/10   dm      Return ERROR_NONE on push and popelement.
05/07/09   dm      Added list deinit.
04/28/09   dm      Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "qmmList.h"

/*===========================================================================

                      DEFINITIONS AND DECLARATIONS

===========================================================================*/
static QMM_ListErrorType   qmm_ValidateData
(
   QMM_ListHandleType  *pList,
   QMM_ListLinkType    *pLink,
   QMM_BooleanType      bValidateLink
);

/* -----------------------------------------------------------------------
** Constant and Macros
** ----------------------------------------------------------------------- */

#define QMM_LIST_LOCK()  //INTLOCK()
#define QMM_LIST_FREE()  //INTFREE()

/* -----------------------------------------------------------------------
** Variables
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Typedefs
** ----------------------------------------------------------------------- */

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
)
{
   QMM_ListErrorType   eRetVal;

   /* aquire lock */
   QMM_LIST_LOCK();
   
   /* Initialization */
   eRetVal = QMM_LIST_ERROR_NONE;

   if ( QMM_NULL == pList )
   {
      /* Return Error Bad Parameter */
      eRetVal = QMM_LIST_ERROR_BAD_PARM;
   }
   else
   {
      /* initializations */ 
      pList->pFront  = QMM_NULL;
      pList->pRear   = QMM_NULL;
      pList->nSize   = 0;
      pList->bInit   = QMM_TRUE;
   }

   /* release lock */
   QMM_LIST_FREE();

   return ( eRetVal );

} /* qmm_ListInit */

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
)
{
   QMM_ListErrorType   eRetVal;

   /* aquire lock */
   QMM_LIST_LOCK();
   
   /* Initialization */
   eRetVal = QMM_LIST_ERROR_NONE;

   if ( QMM_NULL == pList )
   {
      /* Return Error Bad Parameter */
      eRetVal = QMM_LIST_ERROR_BAD_PARM;
   }
   else if ( QMM_FALSE == pList->bInit )
   {
      /* Return Error Not Initialized */
      eRetVal = QMM_LIST_ERROR_NOT_INITIALIZED;
   }
   else
   {
      /* initializations */ 
      pList->pFront  = QMM_NULL;
      pList->pRear   = QMM_NULL;
      pList->nSize   = 0;
      pList->bInit   = QMM_FALSE;

   }

   /* release lock */
   QMM_LIST_FREE();

   return ( eRetVal );

} /* qmm_ListDeInit */

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
)
{
   QMM_ListErrorType   eRetVal;

   /* aquire lock */
   QMM_LIST_LOCK();

   /* Initialization */
   eRetVal = QMM_LIST_ERROR_NONE;

   /* validate list and the link */
   eRetVal = qmm_ValidateData( pList, pPushLink, QMM_TRUE );

   if ( QMM_LIST_ERROR_NOT_PRESENT == eRetVal )
   {
      /* Assign list front link pointer to the next of this link */
      pPushLink->pNext = pList->pFront;

      /* Make this link as the front of the list */
      pList->pFront = pPushLink;

      /* set rear to this link if there was no link earlier */
      if ( 0 == pList->nSize )
      {
         pList->pRear = pPushLink;
      }

      /* Increment list size by one */
      pList->nSize++;

      /* update return status */
      eRetVal = QMM_LIST_ERROR_NONE;

   }

   /* release lock */
   QMM_LIST_FREE();

   return ( eRetVal );

} /* qmm_ListPushFront */

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
)
{
   QMM_ListErrorType   eRetVal;

   /* aquire lock */
   QMM_LIST_LOCK();

   /* Initialization */
   eRetVal = QMM_LIST_ERROR_NONE;

   /* validate list and the link */
   eRetVal = qmm_ValidateData( pList, pPushLink, QMM_TRUE );

   if ( QMM_LIST_ERROR_NOT_PRESENT == eRetVal )
   {
      /* Assign QMM_NULL to the next of this link */
      pPushLink->pNext = QMM_NULL;

      /* if list size is 0, make this link as front of the list 
      ** else change next pointer of rear link to point to this link
      */
      if( 0 == pList->nSize )
      {
         pList->pFront = pPushLink;
      }
      else
      {
         pList->pRear->pNext = pPushLink;
      }

      /* Make this link as the rear of the list */
      pList->pRear = pPushLink;

      /* Increment list size by one */
      pList->nSize++;

      /* update return status */
      eRetVal = QMM_LIST_ERROR_NONE;

   }

   /* release lock */
   QMM_LIST_FREE();

   return ( eRetVal );

} /* qmm_ListPushRear */

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
)
{
   QMM_ListErrorType   eRetVal;

   /* aquire lock */
   QMM_LIST_LOCK();
   
   /* Initialization */
   eRetVal = QMM_LIST_ERROR_NONE;

   /* validate list */
   eRetVal = qmm_ValidateData( pList, QMM_NULL, QMM_FALSE );

   if ( QMM_NULL == ppPopLink )
   {
      /* Bad parameter Error */
      eRetVal = QMM_LIST_ERROR_BAD_PARM;
   }
   else if ( QMM_LIST_ERROR_NONE == eRetVal )
   {
      if ( 0 == pList->nSize )
      {
         /* error : list is empty */
         eRetVal = QMM_LIST_ERROR_EMPTY_LIST;
      }
      else if ( 1 == pList->nSize )
      {
         /* Assign list front link pointer to the popped link pointer */
         ( *ppPopLink ) = pList->pFront;

         /* Assign NULL to front and rear and 0 to list size */
         pList->pFront  = QMM_NULL;
         pList->pRear   = QMM_NULL;
         pList->nSize   = 0;

      }
      else
      {
         /* Assign list front link pointer to the popped link pointer */
         ( *ppPopLink ) = pList->pFront;

         /* Assign list front link pointer to the next of the front link */
         pList->pFront = ( *ppPopLink )->pNext;

         /* Decrement list size by one */
         pList->nSize--;

      }
   }

   /* release lock */
   QMM_LIST_FREE();

   return ( eRetVal );

} /* qmm_ListPopFront */

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
)
{
   QMM_ListErrorType   eRetVal;
   QMM_ListLinkType   *pCurrentLink;
   QMM_ListSizeType    i;

   /* aquire lock */
   QMM_LIST_LOCK();

   /* validate list */
   eRetVal = qmm_ValidateData( pList, QMM_NULL, QMM_FALSE );

   if ( QMM_NULL == ppPopLink )
   {
      /* Bad parameter Error */
      eRetVal = QMM_LIST_ERROR_BAD_PARM;
   }
   else if ( QMM_LIST_ERROR_NONE == eRetVal )
   {
      if ( 0 == pList->nSize )
      {
         /* error : list is empty */
         eRetVal = QMM_LIST_ERROR_EMPTY_LIST;
      }
      else if ( 1 == pList->nSize )
      {
         /* Assign list rear link pointer to the popped link pointer */
         ( *ppPopLink ) = pList->pRear;

         /* Assign NULL to front and rear and 0 to list size */
         pList->pFront  = QMM_NULL;
         pList->pRear   = QMM_NULL;
         pList->nSize   = 0;

      }
      else
      {
         /* Assign list front link pointer to the current link pointer */
         pCurrentLink = pList->pFront;

         /* Move current link pointer to point to (N-1)th link */
         for( i = 1; i < ( pList->nSize - 1 ); i++)
         {
            pCurrentLink = pCurrentLink->pNext;
         }

         /* Assign list rear link pointer to the popped link pointer */
         ( *ppPopLink ) = pList->pRear;

         /* Assign current link's next pointer to NULL */
         pCurrentLink->pNext = QMM_NULL;

         /* Move rear pointer to current link */
         pList->pRear = pCurrentLink;

         /* Decrement list size by one */
         pList->nSize--;

      }
   }

   /* release lock */
   QMM_LIST_FREE();

   return ( eRetVal );
  
} /* qmm_ListPopRear */

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
)
{
   QMM_ListErrorType   eRetVal;
   QMM_ListLinkType   *pCurrentLink;

   /* aquire lock */
   QMM_LIST_LOCK();

   /* validate list */
   eRetVal = qmm_ValidateData( pList, pPopLink, QMM_TRUE );

   if( QMM_LIST_ERROR_PRESENT == eRetVal )
   {
      /* There will be minimum 1 link in the list since given link was found
      ** during validation
      */
      if ( 1 == pList->nSize )
      {
         /* Assign NULL to front and rear and 0 to list size */
         pList->pFront  = QMM_NULL;
         pList->pRear   = QMM_NULL;
      }
      else
      {
         /* Assign list front link pointer to the current link pointer */
         pCurrentLink = pList->pFront;

         /* check if link being popped is front link */
         if( pPopLink == pList->pFront )
         {
            /* bring next link pointer to the front */
            pList->pFront = pList->pFront->pNext;
         }
         else
         {
            /* Move current link pointer to point to previous link of the link 
            ** to be popped
            */
            while( pCurrentLink->pNext != pPopLink )
            {
               pCurrentLink = pCurrentLink->pNext;
            }

            /* assign popped link next to current link's next */
            pCurrentLink->pNext = pPopLink->pNext;            

            /* check if link being popped is rear link */
            if( pPopLink == pList->pRear )
            {
               /* update rear */
               pList->pRear = pCurrentLink;
            }
         }
      }
      
      /* Decrement list size by one */
      pList->nSize--;

      /* update return status */
      eRetVal = QMM_LIST_ERROR_NONE;

   }

   /* release lock */
   QMM_LIST_FREE();

   return ( eRetVal );
  
} /* qmm_ListPopElement */

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
)
{
   QMM_ListErrorType   eRetVal;

   /* aquire lock */
   QMM_LIST_LOCK();

   /* validate list */
   eRetVal = qmm_ValidateData( pList, QMM_NULL, QMM_FALSE );

   if ( QMM_NULL == ppPeekLink )
   {
      /* Bad parameter Error */
      eRetVal = QMM_LIST_ERROR_BAD_PARM;
   }
   else if ( QMM_LIST_ERROR_NONE == eRetVal )
   {
      if ( 0 == pList->nSize )
      {
         /* error : list is empty */
         eRetVal = QMM_LIST_ERROR_EMPTY_LIST;
      }
      else
      {
         /* Assign list front link pointer to the popped link pointer */
         ( *ppPeekLink ) = pList->pFront;

      }
   }

   /* release lock */
   QMM_LIST_FREE();

   return ( eRetVal );

} /* qmm_ListPeekFront */

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
)
{
   QMM_ListErrorType   eRetVal;

   /* aquire lock */
   QMM_LIST_LOCK();

   /* validate list */
   eRetVal = qmm_ValidateData( pList, QMM_NULL, QMM_FALSE );

   if ( QMM_NULL == ppPeekLink )
   {
      /* Bad parameter Error */
      eRetVal = QMM_LIST_ERROR_BAD_PARM;
   }
   else if ( QMM_LIST_ERROR_NONE == eRetVal )
   {
      if ( 0 == pList->nSize )
      {
         /* error : list is empty */
         eRetVal = QMM_LIST_ERROR_EMPTY_LIST;
      }
      else
      {
         /* Assign list rear link pointer to the popped link pointer */
         ( *ppPeekLink ) = pList->pRear;

      }
   }

   /* release lock */
   QMM_LIST_FREE();

   return ( eRetVal );
  
} /* qmm_ListPeekRear */

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
)
{
   QMM_ListErrorType   eRetVal;

   /* aquire lock */
   QMM_LIST_LOCK();

   /* validate list */
   eRetVal = qmm_ValidateData( pList, QMM_NULL, QMM_FALSE );

   if ( QMM_NULL == pSize )
   {
      /* Bad parameter Error */
      eRetVal = QMM_LIST_ERROR_BAD_PARM;
   }
   else
   {
      /* assign list size */
      ( *pSize ) = pList->nSize;
   }

   /* release lock */
   QMM_LIST_FREE();

   return ( eRetVal );

} /* qmm_ListSize */

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
            @param[in]  fnCmp       - Function which implements comparison
                                      logic for the given comparison
                                      value.
            @param[in] pCmpValue    - Pointer to the value which is used for 
                                      comparison against the given element's
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
)
{
   QMM_ListErrorType   eRetVal;
   QMM_ListLinkType   *pCurrentLink;
   QMM_ListSizeType    i;

   /* aquire lock */
   QMM_LIST_LOCK();

   /* validate list */
   eRetVal = qmm_ValidateData( pList, QMM_NULL, QMM_FALSE );

   if( ( QMM_NULL == fnCmp ) || ( QMM_NULL == pCmpValue )
      || ( QMM_NULL == ppSearchLink )
      )
   {
      /* Bad parameter Error */
      eRetVal = QMM_LIST_ERROR_BAD_PARM;
   }
   else if( QMM_LIST_ERROR_NONE == eRetVal )
   {
      /* assign front link pointer to current link */
      pCurrentLink = pList->pFront;

      /* traverse the entire list */
      for( i = 0; i < pList->nSize; i++)
      {
         eRetVal = fnCmp( pCurrentLink, pCmpValue );
         
         if ( QMM_LIST_ERROR_PRESENT == eRetVal )
         {
            /* assign searched link pointer */
            ( *ppSearchLink ) = pCurrentLink;

            /* stop the search */
            break;
         }
    
         /* move to next link */
         pCurrentLink = pCurrentLink->pNext;
      }
   }

   /* release lock */
   QMM_LIST_FREE();

   return ( eRetVal );

} /* qmm_ListSearch */

/*==========================================================================
     
         FUNCTION:      qmm_ValidateData
                                                                        
         DESCRIPTION:                                                       
*//**       This function will check if the current list is valid. It will
            also check if the passed element is present in list.
            This is an internal function.

@par     DEPENDENCIES:   
               None
*//*                                                                        
         PARAMETERS:
*//**       @param[in]  pList       - Pointer to the list object.
            @param[in]  pLink       - Pointer to the link to be checked.
                                      ( It can be null if only list shall be
                                      checked, bValidateLink shall mandatorily
                                      be false if this is null )
            @param[in]  bValidateLink
                                    - True if link shall be validated.
**//*     RETURN VALUE:
*//**       @return    QMM Error code.

@par     SIDE EFFECTS:   
                        None
                                                                        
===========================================================================*/
static QMM_ListErrorType   qmm_ValidateData
(
   QMM_ListHandleType  *pList,
   QMM_ListLinkType    *pLink,
   QMM_BooleanType      bValidateLink
)
{
   QMM_ListErrorType   eRetVal;
   QMM_ListLinkType    *pCurrentLink;
   QMM_ListLinkType    *pPreviousLink;
   QMM_ListSizeType    nCurSize;

   /* Initialization */
   eRetVal        = QMM_LIST_ERROR_NONE;
   nCurSize       = 0;
   pPreviousLink  = QMM_NULL;
   pCurrentLink   = QMM_NULL;

   if( ( QMM_NULL == pList )
       ||
       ( ( QMM_TRUE == bValidateLink ) && ( QMM_NULL == pLink ) )
      )
   {
      /* Bad parameter Error */
      eRetVal = QMM_LIST_ERROR_BAD_PARM;
   }
   else if ( QMM_FALSE == pList->bInit )
   {
      /* Initialization Error */
      eRetVal = QMM_LIST_ERROR_NOT_INITIALIZED;
   }
   else
   {
      /* Initialization */
      pCurrentLink   = pList->pFront;

   }

   /* if there is no error, traverse through entire list */
   while ( ( QMM_LIST_ERROR_NONE == eRetVal ) && ( QMM_NULL != pCurrentLink ) )
   {
      /* increment size by 1 */
      nCurSize++;

      /* there is list error if current total number of links is 
      ** greater than total list size
      ** also, if current link and the link being checked are same
      ** return the status
      */
      if( nCurSize > pList->nSize )
      {
         /* list corruption */
         eRetVal = QMM_LIST_ERROR_CORRUPTION;
      }
      else if ( ( QMM_TRUE == bValidateLink ) && ( pCurrentLink == pLink ) )
      {
         /* element is present in the list */
         eRetVal = QMM_LIST_ERROR_PRESENT;
      }

      /* save current link in previous link pointer */
      pPreviousLink = pCurrentLink;

      /* move to next link */
      pCurrentLink = pCurrentLink->pNext;

   } /* end of while loop */

   /* validate rear pointer */
   if( QMM_LIST_ERROR_NONE == eRetVal ) 
   {
      if( pPreviousLink != pList->pRear )
      {
         /* rear pointer is not correct */
         eRetVal = QMM_LIST_ERROR_CORRUPTION;
      }
      else if( QMM_TRUE == bValidateLink )
      {
         /* element is not present in the list */
         eRetVal = QMM_LIST_ERROR_NOT_PRESENT;
      }         
   }

   return ( eRetVal );

} /* END qmm_ListIsValid */

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
int qmm_ListIsEmpty
(
   QMM_ListHandleType  *pList
)
{
   QMM_ListSizeType size = 0;
   qmm_ListSize(pList, &size);
   return size == 0;
}

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
)
{
   QMM_ListLinkType *pQueueLink = (void*)0;
   (void)qmm_ListPopFront(pList, &pQueueLink);
   return (void *)pQueueLink;
}
