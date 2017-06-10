#ifndef QMI_UTIL_H
#define QMI_UTIL_H
/******************************************************************************
  @file    qmi_util.c
  @brief   Definitions and declarations for the 
           QMI utility functions used in various places in QMI library.

  DESCRIPTION
  QMI interface library common utility functions definitions/declarations

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2007-2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_i.h"

#define QMI_INVALID_TXN_ID  0

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

/* Function that will be called on transaction just prior to deleting */ 
typedef void (*qmi_txn_delete_f_ptr) (void *txn_ptr);

typedef struct qmi_txn_hdr_type
{
  /* Works with QMI_SLL macros */
  struct qmi_txn_hdr_type       *next;
  unsigned int                  ref_count;
  unsigned int                  ready_to_delete;
  qmi_txn_delete_f_ptr          delete_f_ptr;

} qmi_txn_hdr_type;


typedef int  (*qmi_txn_find_txn_f_ptr)  
(
  qmi_txn_hdr_type  *txn,
  void              *cmp_data
);




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
extern qmi_txn_hdr_type *
qmi_util_alloc_and_addref_txn 
(
  unsigned long                 txn_size,
  qmi_txn_delete_f_ptr          delete_f_ptr,
  qmi_txn_hdr_type              **list_head,
  QMI_PLATFORM_MUTEX_DATA_TYPE  *list_mutex
);

/*===========================================================================
  FUNCTION  qmi_service_delete_txn
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
extern void 
qmi_util_service_delete_txn
(
  qmi_txn_hdr_type *txn
);

/*===========================================================================
  FUNCTION  qmi_delete_txn
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
extern void
qmi_util_release_txn
(
  qmi_txn_hdr_type *txn,
  unsigned int      is_delete,
  qmi_txn_hdr_type              **list_head,
  QMI_PLATFORM_MUTEX_DATA_TYPE  *list_mutex
);

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
);

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
extern qmi_txn_hdr_type *
qmi_util_find_and_addref_txn 
(
  void                          *cmp_data,
  qmi_txn_find_txn_f_ptr        cmp_f_ptr,
  qmi_txn_hdr_type              **list_head,
  QMI_PLATFORM_MUTEX_DATA_TYPE  *list_mutex
);


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
);


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
extern int 
qmi_util_read_std_tlv 
(
  unsigned char **msg_buf,
  int           *msg_buf_size,
  unsigned long *type,
  unsigned long *length,
  unsigned char **value
);



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
extern int 
qmi_util_write_std_tlv 
(
  unsigned char **msg_buf,
  int           *msg_buf_size,
  unsigned long type,
  unsigned long length,
  void          *value
);


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
extern int 
qmi_util_get_std_result_code 
(
  unsigned char **msg_buf,
  int           *msg_buf_size,
  int           *qmi_err_code
);

#endif /* QMI_UTIL_H */
