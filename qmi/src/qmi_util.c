/******************************************************************************
  @file    qmi_util.c
  @brief   The QMI utility functions used in various places in QMI library.

  DESCRIPTION
  QMI interface library common utility functions

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2007-2009, 2013-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_i.h"
#include "qmi_platform.h"
#include "qmi_util.h"


/* Definitions associated with QMI result TLV's */
#define QMI_RESULT_CODE_TYPE_ID   (0x02)
#define QMI_RESULT_CODE_FAILURE    0x0001


/******************************************************************************
******************************************************************************/

/*===========================================================================
  FUNCTION  qmi_util_alloc_and_addref_txn
===========================================================================*/
/*!
@brief
  Routine for allocating a transaction structure

@return
  Pointer to transaction structure or NULL if none available

@note

  - Dependencies
    - None

  - Side Effects
    -

*/
/*=========================================================================*/
qmi_txn_hdr_type *
qmi_util_alloc_and_addref_txn
(
  unsigned long                 txn_size,
  qmi_txn_delete_f_ptr          delete_f_ptr,
  qmi_txn_hdr_type              **list_head,
  QMI_PLATFORM_MUTEX_DATA_TYPE  *list_mutex
)
{
  qmi_txn_hdr_type  *txn;

  /* Make sure passed in size is valid */
  if (txn_size < sizeof (qmi_txn_hdr_type))
  {
    return NULL;
  }

  /* Allocate a transaction structure */
  txn = (qmi_txn_hdr_type *) malloc (txn_size);

  if (!txn)
  {
    return NULL;
  }

  /* Initialize structure to all 0's */
  memset ((void *)txn, 0,txn_size);

  /* Initialize internal header fields */
  txn->ref_count = 1;
  txn->ready_to_delete = FALSE;
  txn->delete_f_ptr = delete_f_ptr;

  /* Lock global txn mutex */
  QMI_PLATFORM_MUTEX_LOCK (list_mutex);

  QMI_SLL_ADD(txn,*list_head);

  /* Unlock global txn mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (list_mutex);
  return txn;
} /* qmi_util_alloc_and_addref_txn */



/*===========================================================================
  FUNCTION  qmi_util_service_delete_txn
===========================================================================*/
/*!
@brief
  Routine for allocating a transaction structure

@return
  Pointer to transaction structure or NULL if none available

@note

  - Dependencies
    - None

  - Side Effects
    -

*/
/*=========================================================================*/
void
qmi_util_service_delete_txn
(
  qmi_txn_hdr_type *txn
)
{
  if (txn)
  {
    if (txn->delete_f_ptr != NULL)
    {
      txn->delete_f_ptr ((void *)txn);
    }
    free (txn);
  }
}

/*===========================================================================
  FUNCTION  qmi_util_release_txn_no_lock
===========================================================================*/
/*!
@brief
  De-allocates a transaction structure from the list. This is a no lock
  version ofqmi_util_release_txn(). The assumption is that the user
  will protect the list with the approprioate list mutex.

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
qmi_util_release_txn_no_lock
(
  qmi_txn_hdr_type   *txn,
  unsigned int       is_delete,
  qmi_txn_hdr_type   **list_head
)
{
  qmi_txn_hdr_type  *del_txn, *prev_txn;

  /* Initialize del_txn */
  del_txn = NULL;

  /* Decrement reference count */
  if (txn->ref_count > 0)
  {
    txn->ref_count--;
  }

  /* If we want to delete the transaction or the transaction is
  ** already marked to be deleted, attempt to delete
  */
  if ((is_delete) || (txn->ready_to_delete == TRUE))
  {
    /* Set flag in transaction that indicates it is ready to delete */
    txn->ready_to_delete = TRUE;

    /* If reference count is 0, delete the transaction
    */
    if (txn->ref_count == 0)
    {
      QMI_SLL_FIND_AND_REMOVE(del_txn,prev_txn,*list_head,(del_txn == txn));
    }
  }

  /* If we found a transaction to delete */
  if (del_txn)
  {
    qmi_util_service_delete_txn (del_txn);
  }
}

/*===========================================================================
  FUNCTION  qmi_util_release_txn
===========================================================================*/
/*!
@brief
  De-allocates a transaction structure

@return
  None.

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
qmi_util_release_txn
(
  qmi_txn_hdr_type *txn,
  unsigned int      is_delete,
  qmi_txn_hdr_type              **list_head,
  QMI_PLATFORM_MUTEX_DATA_TYPE  *list_mutex
)
{
  /* Check input value */
  if (txn == NULL)
  {
    return;
  }

  /* Lock  txn mutex */
  QMI_PLATFORM_MUTEX_LOCK (list_mutex);

  qmi_util_release_txn_no_lock(txn, is_delete, list_head);

  /* Unlock the mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (list_mutex);
} /* qmi_delete_txn */

/*===========================================================================
  FUNCTION  qmi_util_find_and_addref_txn
===========================================================================*/
/*!
@brief
  Returns a pointer to an active transaction structure based on input
  data.

@return
   Pointer to transaction structure or NULL if no matching transaction found

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
qmi_txn_hdr_type *
qmi_util_find_and_addref_txn
(
  void                          *cmp_data,
  qmi_txn_find_txn_f_ptr        cmp_f_ptr,
  qmi_txn_hdr_type              **list_head,
  QMI_PLATFORM_MUTEX_DATA_TYPE  *list_mutex
)
{
  qmi_txn_hdr_type *tmp_txn, *prev_txn;

  /* Lock global txn mutex */
  QMI_PLATFORM_MUTEX_LOCK (list_mutex);

  /* Start at beginning of list */
  QMI_SLL_FIND(tmp_txn,
               prev_txn,
               *list_head,
               (cmp_f_ptr (tmp_txn,cmp_data)));

  qmi_util_addref_txn_no_lock(&tmp_txn);

  /* UnLock global txn mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (list_mutex);

  (void) prev_txn; /* Keep lint happy */

  /* Return the txn */
  return tmp_txn;
}/* qmi_util_find_and_addref_txn */

/*===========================================================================
  FUNCTION  qmi_util_addref_txn_no_lock
===========================================================================*/
/*!
@brief
  Increments the reference count of a transaction.

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void qmi_util_addref_txn_no_lock
(
  qmi_txn_hdr_type **txn
)
{
  /* Make sure transaction isn't ready to be deleted.  If it is, then
  ** just return NULL.  Otherwise increment it's reference count
  */
  if (NULL != txn &&
       NULL != (*txn))
  {
    if ((*txn)->ready_to_delete)
    {
      (*txn) = NULL;
    }
    else
    {
      (*txn)->ref_count++;
    }
  }
} /* qmi_util_addref_txn */

/*===========================================================================
  FUNCTION  qmi_util_read_std_tlv
===========================================================================*/
/*!
@brief
  Decodes a TLV and moves the buffer pointer appropriately.

@return
  0 if succees, negative value if error occurs

@note

  - Dependencies
    - None

  - Side Effects
    - moves the msg_buf pointer ahead in array, decrements msg_buf_size by
    appropriate amount of whole TLV.
*/
/*=========================================================================*/
int
qmi_util_read_std_tlv
(
  unsigned char **msg_buf,
  int           *msg_buf_size,
  unsigned long *type,
  unsigned long *length,
  unsigned char **value
)
{
  unsigned char *tmp_msg_buf = *msg_buf;
  unsigned long  tmp_length;

  /* Get the "type" field (8 bits) */
  if (*msg_buf_size < QMI_TLV_HDR_SIZE)
  {
    return QMI_INTERNAL_ERR;
  }
  READ_8_BIT_VAL (tmp_msg_buf,*type);
  READ_16_BIT_VAL (tmp_msg_buf,tmp_length);

  /* Adjust msg_buf and msg_buf_size */
  *msg_buf_size -= QMI_TLV_HDR_SIZE;
  *msg_buf += QMI_TLV_HDR_SIZE;

  /* temp_length now contains the length of the data, get pointer to the
  ** "value" field
  */
  if ((unsigned long)*msg_buf_size >= tmp_length)
  {
    /* Everything looks good, adjust real values */
    *length = tmp_length;
    *value = tmp_msg_buf;
    *msg_buf += tmp_length;
    *msg_buf_size -= (int)tmp_length;
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }
  return ((int)tmp_length + QMI_TLV_HDR_SIZE);
} /* qmi_util_read_std_tlv */

/*===========================================================================
  FUNCTION  qmi_util_write_std_tlv
===========================================================================*/
/*!
@brief
  Encodes a TLV and moves the buffer pointer/counter appropriately.

@return
  Number of bytes written if success, value < 0 if failure.

@note

  - Dependencies
    - None

  - Side Effects
    - moves the msg_buf pointer ahead in array, decrements msg_buf_size by
    appropriate amount of whole TLV.
*/
/*=========================================================================*/
int
qmi_util_write_std_tlv
(
  unsigned char **msg_buf,
  int           *msg_buf_size,
  unsigned long type,
  unsigned long length,
  void          *value
)
{
  unsigned char *tmp_msg_buf = *msg_buf;
  int           total_length = (int) length + QMI_TLV_HDR_SIZE;

  if (*msg_buf_size < total_length)
  {
    return QMI_INTERNAL_ERR;
  }

  *msg_buf += total_length;
  *msg_buf_size -= total_length;

  /* Write the "type" field (8 bits) */
  WRITE_8_BIT_VAL (tmp_msg_buf,type);

  /* Write the length field (16 bits) */
  WRITE_16_BIT_VAL (tmp_msg_buf,length);

  /* Copy the data into the buffer */
  memcpy (tmp_msg_buf, value, length);

  /* Return the total length of the TLV */
  return total_length;
} /* qmi_util_write_std_tlv */


/*===========================================================================
  FUNCTION  qmi_util_get_std_result_code
===========================================================================*/
/*!
@brief
  Retrieves the standard QMI result code TLV.  QMI error code is
  returned in qmi_err_code parameter

@return
  0 if operation was successful, negative value if not.  If 0 is returned,
  the qmi_err_code value is valid but it is not if negative value is
  returned

@note

  - Dependencies
    - None

  - Side Effects
    - if successful, moves the msg_buf pointer ahead in array, decrements
      msg_buf_size by appropriate amount of whole TLV
*/
/*=========================================================================*/
int
qmi_util_get_std_result_code
(
  unsigned char **msg_buf,
  int           *msg_buf_size,
  int           *qmi_err_code
)
{
  unsigned char *tmp_msg_buf = *msg_buf;
  int           tmp_msg_buf_size = *msg_buf_size;
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  int           temp;
  int           rc;


  /* Initialize the qmi error code and return code */
  *qmi_err_code = QMI_SERVICE_ERR_NONE;
  rc = QMI_NO_ERR;

  /* Read the TLV... */
  if (qmi_util_read_std_tlv (&tmp_msg_buf,
                                &tmp_msg_buf_size,
                                &type,
                                &length,
                                &value_ptr) < 0)
  {
    rc = QMI_INTERNAL_ERR;
  }
  else if (type != QMI_RESULT_CODE_TYPE_ID)
  {
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    /* Read whether or not operation was successful, if not, get
    ** the QMI error code and set appropriate error code
    */
    READ_16_BIT_VAL (value_ptr,temp);

    if (temp == QMI_RESULT_CODE_FAILURE)
    {
      READ_16_BIT_VAL (value_ptr,temp);
      *qmi_err_code = temp;
      rc = QMI_SERVICE_ERR;
    }

    /* Adjust pointer and size values to reflect that we consumed this TLV */
    *msg_buf = tmp_msg_buf;
    *msg_buf_size = tmp_msg_buf_size;
  }

  return rc;
} /* qmi_util_get_std_result_code */
