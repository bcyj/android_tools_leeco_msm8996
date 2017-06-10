/******************************************************************************

                        D S _ L I S T . H

******************************************************************************/

/******************************************************************************

  @file    ds_list.h
  @brief   List Utility Functions Header File

  DESCRIPTION
  Header file for List utility functions.

  ---------------------------------------------------------------------------
  Copyright (c) 2007,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id: ds_list.h,v 1.1 2010/02/11 19:09:07 randrew Exp $

when       who        what, where, why
--------   ---        -------------------------------------------------------
11/26/07   vk         Added function headers and other comments
09/28/07   vk         Initial version

******************************************************************************/

#ifndef __DS_LIST_H__
#define __DS_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/*---------------------------------------------------------------------------
   Type representing a dll node - clients must never access the fields of
   this structure directly
---------------------------------------------------------------------------*/
typedef struct ds_dll_el_s {
    struct ds_dll_el_s * next;
    struct ds_dll_el_s * prev;
    const void         * data;
} ds_dll_el_t;

/*---------------------------------------------------------------------------
   Type of comparison function registered by clients used in search and
   delete operations
---------------------------------------------------------------------------*/
typedef long int (* ds_dll_comp_f) (const void * first, const void * second);

/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  ds_dll_init
===========================================================================*/
/*!
@brief
  Initializes a dll and returns the head pointer.

@return
  ds_dll_el_t * - pointer to list's head

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
ds_dll_el_t *
ds_dll_init (ds_dll_el_t * head);

/*===========================================================================
  FUNCTION  ds_dll_enq
===========================================================================*/
/*!
@brief
  Enqueues given node/data to the tail of the dll.

@return
  ds_dll_el_t * - pointer to newly added node

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
ds_dll_el_t *
ds_dll_enq (ds_dll_el_t * dlist, ds_dll_el_t * node, const void * data);

/*===========================================================================
  FUNCTION  ds_dll_insert
===========================================================================*/
/*!
@brief
  Enqueues given node/data at a given index in a dll

@return
  ds_dll_el_t * - pointer to newly added node

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
ds_dll_el_t *
ds_dll_insert (ds_dll_el_t * dlist, ds_dll_el_t * node, const void * data,
  const int index);

/*===========================================================================
  FUNCTION  ds_dll_deq
===========================================================================*/
/*!
@brief
  Dequeues a node from the head of the dll.

@return
  ds_dll_el_t * - pointer to dequeued node if available, NULL otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
ds_dll_el_t *
ds_dll_deq (ds_dll_el_t * dlist, ds_dll_el_t ** tail, const void ** data);

/*===========================================================================
  FUNCTION  ds_dll_delete
===========================================================================*/
/*!
@brief
  Searches for a node in the given dll and removes it from the list.

@return
  ds_dll_el_t * - pointer to node removed if found, NULL otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
ds_dll_el_t *
ds_dll_delete
(
    ds_dll_el_t * head,
    ds_dll_el_t ** tail,
    const void * data,
    ds_dll_comp_f comp_f
);

/*===========================================================================
  FUNCTION  ds_dll_next
===========================================================================*/
/*!
@brief
  Returns the next node in a dll for the given node.

@return
  ds_dll_el_t * - pointer to the next node if one exists, NULL otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
ds_dll_el_t *
ds_dll_next (ds_dll_el_t * node, const void ** data);

/*===========================================================================
  FUNCTION  ds_dll_search
===========================================================================*/
/*!
@brief
  Searches for a node in the given dll. Note that the node is not removed
  from the list.

@return
  ds_dll_el_t * - pointer to node if found, NULL otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
ds_dll_el_t *
ds_dll_search (ds_dll_el_t * head, const void * data, ds_dll_comp_f comp_f);

/*===========================================================================
  FUNCTION  ds_dll_data
===========================================================================*/
/*!
@brief
  Returns the client data pointer for a given node.

@return
  void * - pointer to client data

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
const void *
ds_dll_data (ds_dll_el_t * node);

/*===========================================================================
  FUNCTION  ds_dll_free
===========================================================================*/
/*!
@brief
  Frees memory allocated for a node.

@return
  void

@note

  - Dependencies
    - The node must not be currently enqueued in a list.
    - The memory for the node must have been allocated by ds_dll module.

  - Side Effects
    - None
*/
/*=========================================================================*/
void
ds_dll_free (ds_dll_el_t * node);

/*===========================================================================
  FUNCTION  ds_dll_destroy
===========================================================================*/
/*!
@brief
  Dll list destructor.

@return
  void

@note

  - Dependencies
    - The list must be empty
    - The memory for the node must have been allocated by ds_dll module.

  - Side Effects
    - None
*/
/*=========================================================================*/
void
ds_dll_destroy (ds_dll_el_t * head);

#ifdef __cplusplus
}
#endif

#endif /* __DS_LIST_H__ */
