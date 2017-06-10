/******************************************************************************

                        D S _ L I S T . C

******************************************************************************/

/******************************************************************************

  @file    ds_list.c
  @brief   List Utility Functions

  DESCRIPTION
  Implementation of list utility functions.

  ---------------------------------------------------------------------------
  Copyright (c) 2007-2008,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id: ds_list.c,v 1.1 2010/02/11 19:07:18 randrew Exp $

when       who        what, where, why
--------   ---        -------------------------------------------------------
03/24/08   vk         Incorporated code review comments
11/26/07   vk         Added function headers and other comments
09/28/07   vk         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <stdlib.h>
#include <assert.h>
#include "ds_util.h"
#include "ds_list.h"

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
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
ds_dll_init (ds_dll_el_t * head)
{
    /* Allocate memory for list head node, if not already allocated */
    if (!head) {
        if ((head = malloc(sizeof(ds_dll_el_t))) == NULL) {
            return NULL;
        }
    }

    /* Initialize head node ptrs to NULL */
    head->next = NULL;
    head->prev = NULL;
    head->data = NULL;

    return head;
}

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
ds_dll_enq (ds_dll_el_t * dlist, ds_dll_el_t * node, const void * data)
{
    ds_dll_el_t * rnode = NULL;


    /* Verify that list ptr is non null. Note that this can be any node in
    ** the list, not necessarily the head.
    */
    if (!dlist) {
        goto error;
    }

    /* Allocate memory for the new list node, if not already allocated */
    if (!node) {
        if ((node = malloc(sizeof(ds_dll_el_t))) == NULL) {
            goto error;
        }
    }

    /* Set data ptr in node to requested value */
    node->data = data;

    /* Set next ptr in node to null, as new node will be the tail ptr */
    node->next = NULL;

    /* Traverse to the tail ptr of the list */
    while (dlist->next) {
        dlist = dlist->next;
    }

    /* Add new node to the end of the list */
    dlist->next = node;
    node->prev = dlist;
    rnode = node;

error:
    /* Return ptr to new node, or NULL if there was a failure */
    return rnode;
}

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
  const int index)
{
    int i=0;
    ds_dll_el_t *temp = NULL;

    /* Verify that list ptr is non null. Note that this can be any node in
    ** the list, not necessarily the head.
    */
    if (!dlist) {
        node = NULL;
        goto error;
    }

    /* Allocate memory for the new list node, if not already allocated */
    if (!node) {
        if ((node = malloc(sizeof(ds_dll_el_t))) == NULL) {
            goto error;
        }
    }

    /* Set data ptr in node to requested value */
    node->data = data;

    while( i<index && dlist->next!=NULL)
    {
        dlist = dlist->next;
        i++;
    }

    temp = dlist->next;

    dlist->next = node;
    node->prev = dlist;

    node->next = temp;
    if ( temp )
    {
      temp->prev = node;
    }

error:
    /* Return ptr to new node, or NULL if there was a failure */
    return node;
}

/*===========================================================================
  FUNCTION  ds_dll_deq
===========================================================================*/
/*!
@brief
  Dequeues a node from the head of the dll.  Caller must invoke
  ds_dll_free() to release memory used by node.

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
ds_dll_deq (ds_dll_el_t * dlist, ds_dll_el_t ** tail, const void ** data)
{
    ds_dll_el_t * node = NULL;

    /* Verify that list node ptr is non null before proceeding. Note that the
    ** list node ptr passed does not have to be the head ptr and it can be any
    ** node in the list.
    */
    if (!dlist) {
        goto error;
    }

    /* Traverse to the head of the list */
    while (dlist->prev) {
        dlist = dlist->prev;
    }

    /* The following check should never fail as head node is a sentinel. We
    ** do this check anyway.
    */
    if ((node = dlist->next)) {
        /* De-link the first node after head from the list */
        dlist->next = node->next;
        if (node->next) {
            /* If there is another node after the dequeued node, change its
            ** prev ptr appropriately to point to the head.
            */
            node->next->prev = dlist;
        } else {
            /* The dequeued node is the current tail. Return new tail ptr
            ** to client.
            */
            if (tail) {
                *tail = dlist;
            }
        }

        /* Initialize node's link ptrs before returning */
        node->next = node->prev = NULL;

        /* Return data ptr in node to client */
        *data = node->data;

        node->data = NULL;
    }

error:
    /* Return ptr to dequeued node, or NULL if operation failed */
    return node;
}

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
)
{
    ds_dll_el_t * node = NULL;

    /* Make sure, before proceeding, that -
    ** a. Head node ptr passed is non null;
    ** b. Pointer to tail node ptr passed is non null;
    ** c. Tail node ptr is non null;
    ** d. Tail node's next ptr is really null, just to be sure.
    */
    if ((!head) || (!tail) || (!*tail) || ((*tail)->next)) {
        goto error;
    }

    /* Set node ptr to the first data node in the list; head is a sentinel */
    node = head->next;

    /* Traverse the list, looking for a match */
    while (node) {
        /* Call client's comparator function to see if there is a match */
        if (comp_f(data, node->data) == 0) {
            break;
        }
        node = node->next;
    }

    /* A match was found */
    if (node) {
        /* If matching node is current tail, update tail ptr of client */
        if (node == *tail) {
            if (NULL != node->next) {
                ds_log_err("ds_dll_delete: already tail! node->next:[0x%x]", node->next);
                return NULL;
            }

            *tail = node->prev;
        } else {
            /* Matching node is not tail, so update prev ptr of node following
            ** the dequeued node.
            */
            node->next->prev = node->prev;
        }

        /* Change next ptr of node preceding dequeued node appropriately */
        node->prev->next = node->next;

        /* Set node's link ptr to NULL before returning */
        node->prev = node->next = NULL;
    }

error:
    /* Return matching node ptr, or NULL if none found */
    return node;
}

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
ds_dll_next (ds_dll_el_t * node, const void ** data)
{
    ds_dll_el_t * next = NULL;

    /* Verify that node ptr passed is not null before proceeding */
    if( NULL == node) {
        ds_log_err("ds_dll_next: Bad Param node NULL");
        return NULL;
    }

    /* If next node ptr is not null, set client's data ptr to next node's
    ** data ptr.
    */
    if ((next = node->next)) {
        *data = next->data;
    }

    /* Return next node ptr, or NULL if no following node */
    return next;
}

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
ds_dll_search (ds_dll_el_t * head, const void * data, ds_dll_comp_f comp_f)
{
    ds_dll_el_t * node = NULL;

    /* Verify that head node ptr is not null before proceeding */
    if(NULL == head) {
        ds_log_err("ds_dll_search: Bad Param head NULL");
        return NULL;
    }

    /* Set first data node ptr as head is a sentinel */
    node = head->next;

    /* Traverse the list, looking for a match */
    while (node) {
        /* Use client's comparator function to find a match */
        if (comp_f(data, node->data) == 0) {
            break;
        }
        node = node->next;
    }

    /* Return matching node ptr, or null if none found */
    return node;
}

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
ds_dll_data (ds_dll_el_t * node)
{
    /* validate input param */
    if(NULL == node) {
        ds_log_err("ds_dll_data: Bad Param node NULL");
        return NULL;
    }

    /* Return data ptr of this node */
    return node->data;
}

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
ds_dll_free (ds_dll_el_t * node)
{
    /* Free memory for the node using standard library's deallocator */
    free(node);
}

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
ds_dll_destroy (ds_dll_el_t * head)
{
    /* Free memory for the list's head node using standard library's
    ** deallocator.
    */
    free(head);
}

