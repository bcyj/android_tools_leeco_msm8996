/******************************************************************************
  @file    qmi_service.c
  @brief   The QMI common service layer.

  DESCRIPTION
  QMI common service routines.  All services will be build on top of these
  routines for initializing, sending messages and receiving responses/
  indications.  File also contains some common utility routines for decoding
  message headers and TLV's.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_service_init() needs to be called before sending or receiving of any
  service specific messages

  ---------------------------------------------------------------------------
  Copyright (c) 2007-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi_i.h"
#include "qmi_service.h"
#include "qmi_qmux_if.h"
#include "qmi_util.h"
#include "qmi_client.h"
#include "qmi_proxy.h"

#include <unistd.h>

/******************************************************************************
******************************************************************************/


#define QMI_STD_SRVC_REQUEST_CONTROL_FLAG 0x00
#define QMI_STD_SRVC_RESPONSE_CONTROL_FLAG 0x02
#define QMI_STD_SRVC_INDICATION_CONTROL_FLAG 0x04


/* Implicit assumption here is that we will never have more than 127 connection
** ID's (SMD QMI ports).  This should be fine since 3 is the current max
*/

#define QMI_SRVC_CREATE_CLIENT_HANDLE(conn_id,client_id,service_id) \
  (qmi_client_handle_type) ((((qmi_client_handle_type)conn_id & 0x7F) << 24) | \
                            (((qmi_client_handle_type)client_id & 0xFF) << 16) | \
                            (((qmi_client_handle_type)service_id & 0xFF) << 8) \
                           )


/* Static two dimensional array of transactions lists.
*/
static qmi_service_txn_info_type *qmi_service_txn_table [QMI_MAX_CONN_IDS][QMI_MAX_SERVICES];

/* Global mutex used during allocation/de-allocation of transactions */
static QMI_PLATFORM_MUTEX_DATA_TYPE   qmi_service_txn_mutex_table [QMI_MAX_CONN_IDS][QMI_MAX_SERVICES];



/*****************************************************************************
** Type declarations
*****************************************************************************/

static int qmi_qcci_internal_public_service_id_to_bookkeeping_service_id(int public_service_id);
static int qmi_qcci_internal_bookkeeping_service_id_to_public_service_id(int bookkeeping_service_id);

typedef struct qmi_srvc_fn_info_type
{
  qmi_service_ind_rx_hdlr             srvc_rx_ind_msg_hdlr;
} qmi_srvc_fn_info_type;

static qmi_srvc_fn_info_type qmi_srvc_fn_info [QMI_MAX_SERVICES];

/* Each service of each connection will have one of these data structures
** associated with it.
*/
typedef struct qmi_srvc_client_info_type
{
  struct qmi_srvc_client_info_type    *next;
  int                                 ref_count;
  unsigned int                        ready_to_delete;
  qmi_connection_id_type              conn_id;
  qmi_service_id_type                 service_id;
  qmi_client_id_type                  client_id;
  unsigned long                       next_txn_id;
  void                                *user_ind_msg_hdlr;
  void                                *user_ind_msg_hdlr_user_data;
  void                                *user_decode_handle;
  QMI_PLATFORM_MUTEX_DATA_TYPE                     mutex;
} qmi_srvc_client_info_type;


/* Static array of service structures */
static qmi_srvc_client_info_type *qmi_srvc_client_info_table
                                [QMI_MAX_CONN_IDS][QMI_MAX_SERVICES];

/* Mutexs used during allocation/de-allocation/lookup of service data */
static QMI_PLATFORM_MUTEX_DATA_TYPE   qmi_srvc_list_mutex_table
                                [QMI_MAX_CONN_IDS][QMI_MAX_SERVICES];


/* Definitions/declaration associated with individual service init/release
** callbacks
*/

extern int qmi_wds_srvc_init (void);
extern int qmi_wds_srvc_release (void);

extern int qmi_nas_srvc_init (void);
extern int qmi_nas_srvc_release (void);

extern int qmi_qos_srvc_init (void);
extern int qmi_qos_srvc_release (void);

extern int qmi_eap_srvc_init (void);
extern int qmi_eap_srvc_release (void);

extern int qmi_atcop_srvc_init (void);
extern int qmi_atcop_srvc_release (void);

extern int qmi_uim_srvc_init (void);
extern int qmi_uim_srvc_release (void);

extern int qmi_cat_srvc_init (void);
extern int qmi_cat_srvc_release (void);

typedef int (*qmi_service_init_release_fn_type) (void);

typedef struct qmi_init_release_cb_table_type
{
  qmi_service_init_release_fn_type init_f_ptr;
  qmi_service_init_release_fn_type release_f_ptr;
} qmi_init_release_cb_table_type;

static qmi_init_release_cb_table_type qmi_init_release_cb_table [QMI_MAX_SERVICES] =
{
  {qmi_nas_srvc_init, qmi_nas_srvc_release},
  {qmi_qos_srvc_init, qmi_qos_srvc_release},
  {qmi_eap_srvc_init, qmi_eap_srvc_release},
  {qmi_wds_srvc_init, qmi_wds_srvc_release},
  {qmi_atcop_srvc_init, qmi_atcop_srvc_release},
  {qmi_uim_srvc_init, qmi_uim_srvc_release},
  {qmi_cat_srvc_init, qmi_cat_srvc_release}
};

#define QMI_SERVICE_INIT_RELEASE_TABLE_SIZE (sizeof (qmi_init_release_cb_table) / sizeof (qmi_init_release_cb_table_type))

static int qmi_service_initialization_done = 0;
static qmi_sys_event_rx_hdlr qmi_service_sys_event_hdlr_f = NULL;

static qmi_qmux_if_hndl_t  qmi_service_qmux_if_handle = QMI_QMUX_IF_INVALID_HNDL;

_qmi_service_hook_indication_passthrough_type _qmi_service_hook_indication_passthrough;

static void qmi_service_complete_txn (
  qmi_service_txn_info_type   *txn,
  unsigned char       *rx_msg,
  int                 rx_msg_len,
  int                 rsp_rc,
  int                 qmi_err_code
);

typedef enum
{
  QMI_SERVICE_CONN_STATE_ACTIVE,
  QMI_SERVICE_CONN_STATE_INACTIVE
} qmi_service_conn_state_t;

static qmi_service_conn_state_t  qmi_service_conn_state_tbl[QMI_MAX_CONN_IDS];

/*===========================================================================
  FUNCTION  qmi_service_set_srvc_functions
===========================================================================*/
/*!
@brief
  Routine that should be called once by each service to set the calback
  called to recieve indications, and service-specific write/read transaction
  header routines if they exist

@return
  QMI_NO_ERR if success, negative value if not

@note

  - Dependencies
    - None

  - Side Effects
    -

*/
/*=========================================================================*/
int qmi_service_set_srvc_functions (
  qmi_service_id_type                 service_id,
  qmi_service_ind_rx_hdlr             srvc_rx_ind_msg_hdlr
)
{
  /* Make sure passed in service ID is valid */
  if ( qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id )  >= QMI_MAX_SERVICES)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Set fields in table */
  qmi_srvc_fn_info[ qmi_qcci_internal_public_service_id_to_bookkeeping_service_id( service_id ) ].srvc_rx_ind_msg_hdlr = srvc_rx_ind_msg_hdlr;
  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_service_cmp_txn
===========================================================================*/
/*!
@brief
  Routine that should be called once by each service to set the calback
  called to recieve indications, and service-specific write/read transaction
  header routines if they exist

@return
  QMI_NO_ERR if success, negative value if not

@note

  - Dependencies
    - None

  - Side Effects
    -

*/
/*=========================================================================*/
static int qmi_service_cmp_txn (
  qmi_txn_hdr_type  *txn_data,
  void              *cmp_data
)
{
  qmi_service_txn_cmp_type *cmp = (qmi_service_txn_cmp_type *) cmp_data;
  qmi_service_txn_info_type  *txn = (qmi_service_txn_info_type *) txn_data;
  int rc = 0;

  if ((txn != NULL) && (cmp != NULL))
  {
    if ((txn->client_id == cmp->client_id) &&
        (txn->txn_id == cmp->txn_id))
    {
      rc = 1;
    }
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_service_delete_async_txn
===========================================================================*/
/*!
@brief
  Deletes an asynchronous transaction so that it will free resources
  associated with transaction

@return
   QMI_NO_ERR if successful, negative otherwise
@note

  - Dependencies
    - None

  - Side Effects
    - Async response will not be delivered
*/
/*=========================================================================*/
int qmi_service_delete_async_txn (
  int user_handle,
  int async_txn_handle
)
{
  qmi_service_txn_info_type       *txn;
  qmi_connection_id_type    conn_id;
  qmi_client_id_type        client_id;
  qmi_service_id_type       service_id;
  qmi_service_txn_cmp_type  cmp_data;
  int book_keep_srvc_id;

  conn_id = QMI_SRVC_CLIENT_HANDLE_TO_CONN_ID (user_handle);
  client_id = QMI_SRVC_CLIENT_HANDLE_TO_CLIENT_ID (user_handle);
  service_id = QMI_SRVC_CLIENT_HANDLE_TO_SERVICE_ID(user_handle);
  book_keep_srvc_id = qmi_qcci_internal_public_service_id_to_bookkeeping_service_id (service_id);

  if ( conn_id >= QMI_MAX_CONN_IDS ||
       service_id >= QMI_MAX_SERVICES ||
       book_keep_srvc_id >= QMI_MAX_SERVICES )
  {
    QMI_ERR_MSG_2("qmi_service_delete_async_txn: conn_id or service_id is invalid"
                  "conn_id: %d, service_id:%d", conn_id, service_id);
    return QMI_INTERNAL_ERR;
  }

  cmp_data.client_id = client_id;
  cmp_data.txn_id = (unsigned long) async_txn_handle;

  txn = (qmi_service_txn_info_type *)
        qmi_util_find_and_addref_txn ((void *)&cmp_data,
                                      qmi_service_cmp_txn,
                                      (qmi_txn_hdr_type **) &qmi_service_txn_table[conn_id][ book_keep_srvc_id ],
                                      &qmi_service_txn_mutex_table[conn_id][ book_keep_srvc_id ]);

  if (txn)
  {
    qmi_util_release_txn ((qmi_txn_hdr_type *)txn,
                          TRUE,
                          (qmi_txn_hdr_type **) &qmi_service_txn_table[conn_id][ book_keep_srvc_id ],
                          &qmi_service_txn_mutex_table[conn_id][ book_keep_srvc_id ]);
  }

  return QMI_NO_ERR;

}


/*===========================================================================
  FUNCTION  qmi_alloc_srvc_data
===========================================================================*/
/*!
@brief
  Allocates and initializes a service data structure.

@return
   Pointer to transaction structure or NULL if no matching transaction found

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static
qmi_srvc_client_info_type *qmi_alloc_srvc_data (
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      client_id
)
{
  qmi_srvc_client_info_type *srvc;
  int book_keep_srvc_id = qmi_qcci_internal_public_service_id_to_bookkeeping_service_id (service_id);

  if ( conn_id >= QMI_MAX_CONN_IDS    ||
       service_id >= QMI_MAX_SERVICES ||
       book_keep_srvc_id >= QMI_MAX_SERVICES )
  {
    QMI_ERR_MSG_3("qmi_alloc_srvc_data: conn_id or service_id "
                  "invalid, conn_id:%d, service_id:%d, book_keep_srvc_id:%d",
                  conn_id, service_id, book_keep_srvc_id);
    return NULL;
  }

  /* Dynamically allocate a service data structure */
  srvc =  malloc (sizeof (qmi_srvc_client_info_type));

  if (!srvc)
  {
    return NULL;
  }

  /* Initialize structure to all 0's */
  memset ((void *)srvc, 0,sizeof (qmi_srvc_client_info_type));

  QMI_PLATFORM_MUTEX_INIT (&srvc->mutex);

  /* Fill in conn_id, service_id and client_id fields */
  srvc->conn_id    = conn_id;
  srvc->service_id = service_id;
  srvc->client_id  = client_id;

  /* Lock global service access mutex */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_srvc_list_mutex_table[conn_id][book_keep_srvc_id]);

  QMI_SLL_ADD(srvc,qmi_srvc_client_info_table [conn_id][book_keep_srvc_id]);

  /* Unlock global service access mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_srvc_list_mutex_table[conn_id][book_keep_srvc_id]);

  /* return allocated service pointer */
  return srvc;
}

/*===========================================================================
  FUNCTION  qmi_service_wait_for_sync_txn_completion
===========================================================================*/
/*!
@brief
  Completes all transactions for the given client_id and waits for the
  completion of SYNC transactions

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void
qmi_service_wait_for_sync_txn_completion
(
  qmi_connection_id_type  conn_id,
  int                     srvc_id,
  qmi_client_id_type      client_id
)
{
  qmi_txn_hdr_type  *txn = NULL, *prev_txn = NULL, *next_txn = NULL, *head_txn = NULL;
  int anything_left;

  QMI_DEBUG_MSG_0("qmi_service_wait_for_sync_txn_completion : ENTRY");

  QMI_PLATFORM_MUTEX_LOCK (&qmi_service_txn_mutex_table[conn_id][srvc_id]);

  head_txn = (qmi_txn_hdr_type *) qmi_service_txn_table [conn_id][srvc_id];

  QMI_SLL_FIND(txn,
               prev_txn,
               head_txn,
               ((((qmi_service_txn_info_type *)txn)->client_id == client_id) && (((qmi_service_txn_info_type *)txn)->srvc_txn_info.txn_type == QMI_TXN_SYNC)));

  while (txn)
  {
    QMI_ERR_MSG_4("qmi_service_wait_for_sync_txn_completion : completing txn conn_id=%d, service=%d, client=%d, txn=0x%x",
                  conn_id,
                  srvc_id,
                  client_id,
                  ((qmi_service_txn_info_type *)txn)->txn_id);

    /* Complete transactions with QMI_TIMEOUT_ERR return code for the given client */
    qmi_service_complete_txn ((qmi_service_txn_info_type *)txn,NULL,0,QMI_TIMEOUT_ERR,QMI_SERVICE_ERR_NONE);
    head_txn = txn->next;
    QMI_SLL_FIND(txn,
                 prev_txn,
                 head_txn,
                 ((((qmi_service_txn_info_type *)txn)->client_id == client_id) && (((qmi_service_txn_info_type *)txn)->srvc_txn_info.txn_type == QMI_TXN_SYNC)));
  }

  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_service_txn_mutex_table[conn_id][srvc_id]);

  // wait for sync transaction completion
  QMI_DEBUG_MSG_0("qmi_service_wait_for_sync_txn_completion : Wait for sync transactions to complete");
  do
  {
    QMI_PLATFORM_MUTEX_LOCK (&qmi_service_txn_mutex_table[conn_id][srvc_id]);
    anything_left = FALSE;
    txn = (qmi_txn_hdr_type *) qmi_service_txn_table [conn_id][srvc_id];

    while ( txn && !anything_left )
    {
      next_txn = txn->next;
      if (((qmi_service_txn_info_type *)txn)->client_id == client_id &&
          ((qmi_service_txn_info_type *)txn)->srvc_txn_info.txn_type == QMI_TXN_SYNC)
      {
        anything_left = TRUE;
      }
      txn = next_txn;
    }
    QMI_PLATFORM_MUTEX_UNLOCK (&qmi_service_txn_mutex_table[conn_id][srvc_id]);

    if ( anything_left )
    {
      usleep( 10000 ); // 10 msec, yield to allow completion of any pending sync requests
    }
  } while ( anything_left );
  QMI_DEBUG_MSG_0("qmi_service_wait_for_sync_txn_completion : EXIT");
}

/*===========================================================================
  FUNCTION  qmi_service_delete_client_txns
===========================================================================*/
/*!
@brief
  Delete all the transactions associated with the given service client on a
  given connection

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void qmi_service_delete_client_txns
(
  qmi_connection_id_type  conn_id,
  int                     srvc_id,
  qmi_client_id_type      client_id
)
{
  qmi_txn_hdr_type  *txn = NULL, *next = NULL;

  QMI_DEBUG_MSG_3("qmi_service_delete_client_txns : ENTRY - conn_id=%d, service=%d, client=%d",
                  conn_id,
                  srvc_id,
                  client_id);

  /* Cleanup txns for the given client_id */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_service_txn_mutex_table[conn_id][srvc_id]);

  for (txn = (qmi_txn_hdr_type *) qmi_service_txn_table[conn_id][srvc_id]; txn != NULL; txn = next)
  {
    next = txn->next;

    if (((qmi_service_txn_info_type *)txn)->client_id == client_id)
    {
      qmi_util_addref_txn_no_lock(&txn);

      /* Skip if the txn is already marked for deletion */
      if (txn == NULL)
      {
        continue;
      }

      QMI_ERR_MSG_1("releasing txn type=%s",
                    ((qmi_service_txn_info_type *) txn)->srvc_txn_info.txn_type == QMI_TXN_SYNC ?
                    "SYNC" :
                    "ASYNC");

      QMI_ERR_MSG_4("releasing txn conn_id=%d, service=%d, client=%d, txn=0x%x",
                    conn_id,
                    srvc_id,
                    client_id,
                    ((qmi_service_txn_info_type *)txn)->txn_id);

      qmi_util_release_txn_no_lock(txn,
                                   TRUE,
                                   (qmi_txn_hdr_type **)&qmi_service_txn_table[conn_id][srvc_id]);
    }
  }

  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_service_txn_mutex_table[conn_id][srvc_id]);
  QMI_DEBUG_MSG_0("qmi_service_delete_client_txns : EXIT");
}

/*===========================================================================
  FUNCTION  qmi_free_srvc_data
===========================================================================*/
/*!
@brief
  Removes and frees service data from list

@return
   QMI_NO_ERR if service was found, QMI_INTERNAL_ERR otherwise.

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static
int qmi_free_srvc_data (
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      client_id,
  int                     lock_list_mutex
)
{
  int rc;
  qmi_srvc_client_info_type *srvc, *prev;
  int book_keep_srvc_id = qmi_qcci_internal_public_service_id_to_bookkeeping_service_id (service_id);

  QMI_DEBUG_MSG_0("qmi_free_srvc_data : ENTRY");

  if ( conn_id >= QMI_MAX_CONN_IDS    ||
       service_id >= QMI_MAX_SERVICES ||
       book_keep_srvc_id >= QMI_MAX_SERVICES )
  {
    QMI_ERR_MSG_3("qmi_free_srvc_data: conn_id or service_id "
                  "invalid, conn_id:%d, service_id:%d, book_keep_srvc_id:%d",
                  conn_id, service_id, book_keep_srvc_id);
    return QMI_INTERNAL_ERR;
  }

  /* Lock global service list mutex */
  if (lock_list_mutex)
  {
    QMI_PLATFORM_MUTEX_LOCK (&qmi_srvc_list_mutex_table[conn_id][book_keep_srvc_id]);
  }

  /* Initialize pointer to first element in table */
  QMI_SLL_FIND(srvc,
               prev,
               qmi_srvc_client_info_table [conn_id][book_keep_srvc_id],
               (srvc->client_id == client_id));

  /* If we find matching service, proceed.... */
  if (srvc)
  {
    /* Set return code */
    rc = QMI_NO_ERR;

    /* Lock service mutex */
    QMI_PLATFORM_MUTEX_LOCK (&srvc->mutex);

    /* Set the ready for delete status flag */
    srvc->ready_to_delete = TRUE;

    /* If reference count is 0, delete service object */
    if (srvc->ref_count <= 0)
    {
      QMI_SLL_REMOVE(srvc,prev,qmi_srvc_client_info_table [conn_id][book_keep_srvc_id]);

      /* Unlock the service mutex... destroying a locked mutex
      ** results in undefined behavior
      */
      QMI_PLATFORM_MUTEX_UNLOCK (&srvc->mutex);

      qmi_service_wait_for_sync_txn_completion(conn_id, book_keep_srvc_id, client_id);

      qmi_service_delete_client_txns(conn_id, book_keep_srvc_id, client_id);

      /* Destroy mutex and free the service data */
      QMI_PLATFORM_MUTEX_DESTROY (&srvc->mutex);

      free (srvc);

      srvc = NULL;
    }
    else
    {
      /* Unlock the service mutex, ref_count != 0 */
      QMI_PLATFORM_MUTEX_UNLOCK (&srvc->mutex);
    }
  }
  else
  {
    rc = QMI_INTERNAL_ERR;
  }

  /* Unlock global service list mutex */
  if (lock_list_mutex)
  {
    QMI_PLATFORM_MUTEX_UNLOCK (&qmi_srvc_list_mutex_table[conn_id][book_keep_srvc_id]);
  }

  QMI_DEBUG_MSG_0("qmi_free_srvc_data : EXIT");
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_alloc_srvc_data
===========================================================================*/
/*!
@brief
  Allocates and initializes a service data structure.

@return
   Pointer to transaction structure or NULL if no matching transaction found

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
qmi_srvc_client_info_type *qmi_service_addref (
  qmi_srvc_client_info_type *srvc
)
{
  qmi_srvc_client_info_type *rc = NULL;

  QMI_PLATFORM_MUTEX_LOCK (&srvc->mutex);
  if (!(srvc->ready_to_delete))
  {
    srvc->ref_count++;
    rc = srvc;
  }
  QMI_PLATFORM_MUTEX_UNLOCK (&srvc->mutex);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_find_and_addref_srvc_data
===========================================================================*/
/*!
@brief
  Allocates and initializes a service data structure.

@return
   Pointer to transaction structure or NULL if no matching transaction found

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static
qmi_srvc_client_info_type *qmi_find_and_addref_srvc_data (
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      client_id,
  int                     do_add_ref
)
{
  qmi_srvc_client_info_type *srvc, *prev, *rc;
  int book_keep_srvc_id = qmi_qcci_internal_public_service_id_to_bookkeeping_service_id (service_id);

  if ( conn_id >= QMI_MAX_CONN_IDS    ||
       service_id >= QMI_MAX_SERVICES ||
       book_keep_srvc_id >= QMI_MAX_SERVICES )
  {
    QMI_ERR_MSG_3("qmi_find_and_addref_srvc_data: conn_id or service_id "
                  "invalid, conn_id:%d, service_id:%d, book_keep_srvc_id:%d",
                  conn_id, service_id, book_keep_srvc_id);
    return NULL;
  }

  /* Lock global service access mutex */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_srvc_list_mutex_table[conn_id][book_keep_srvc_id]);

  QMI_SLL_FIND (srvc,
                prev,
                qmi_srvc_client_info_table [conn_id][book_keep_srvc_id],
                (srvc->client_id == client_id));

  /* Set return code pointer to the service we found (or didn't) */
  rc = srvc;

  /* If we found the service and we want to add a reference, do so */
  if (srvc && do_add_ref)
  {
    rc = qmi_service_addref (srvc);
  }

  /* Unlock global service access mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_srvc_list_mutex_table[conn_id][book_keep_srvc_id]);

  (void)prev; /* Keep lint happy */

  /* Return the service pointer */
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_service_add_decode_handle
===========================================================================*/
/*!
@brief
  Find and adds the client decode handle to the client information structure

@return
   - None

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
qmi_service_add_decode_handle
(
    int  user_handle,
    void *user_decode_handle
)
{
  int rc = QMI_NO_ERR;
  qmi_connection_id_type    conn_id;
  qmi_client_id_type        client_id;
  qmi_service_id_type       service_id;
  qmi_srvc_client_info_type *srvc_data;

  conn_id = QMI_SRVC_CLIENT_HANDLE_TO_CONN_ID (user_handle);
  client_id = QMI_SRVC_CLIENT_HANDLE_TO_CLIENT_ID (user_handle);
  service_id = QMI_SRVC_CLIENT_HANDLE_TO_SERVICE_ID(user_handle);

  srvc_data = qmi_find_and_addref_srvc_data(conn_id,
                                            service_id,
                                            client_id,
                                            FALSE);

 if (srvc_data != NULL) {
     srvc_data->user_decode_handle = user_decode_handle;
 }
 else
 {
     rc = QMI_INTERNAL_ERR;
 }

 return rc;
}

/*===========================================================================
  FUNCTION  qmi_alloc_srvc_data
===========================================================================*/
/*!
@brief
  Allocates and initializes a service data structure.

@return
   Pointer to transaction structure or NULL if no matching transaction found

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static
void qmi_release_srvc_data (
  qmi_srvc_client_info_type **srvc
)
{
  int delete_srvc;
  qmi_srvc_client_info_type *tmp_srvc;

  /* Error condition */
  if ((srvc == NULL) || (*srvc == NULL))
  {
    return;
  }

  /* Initialize tmp_srvc.  Use tmp_srvc to avoid extra dereference each
  ** time service is accessed
  */
  tmp_srvc = *srvc;

  /* Set srvc pointer to NULL */
  *srvc = NULL;

  /* Lock the service mutex */
  QMI_PLATFORM_MUTEX_LOCK (&tmp_srvc->mutex);

  /* Decrement the reference count */
  if (tmp_srvc->ref_count)
  {
    tmp_srvc->ref_count--;
  }

  /* Check the ready for delete flag... if set, and reference count is 0,
  ** delete it
  */
  delete_srvc = ((tmp_srvc->ready_to_delete) &&
                 (tmp_srvc->ref_count == 0)) ? TRUE : FALSE;

  /* Unlock service mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&tmp_srvc->mutex);

  /* Delete the service if ready */
  if (delete_srvc)
  {
    if (qmi_free_srvc_data (tmp_srvc->conn_id,
                            tmp_srvc->service_id,
                            tmp_srvc->client_id,
                            TRUE) != QMI_NO_ERR)
    {
      QMI_ERR_MSG_3 ("Failed to free service data for conn=%d,srvc=%d,client=%d",
                                                           (int) tmp_srvc->conn_id,
                                                           (int) tmp_srvc->service_id,
                                                           (int) tmp_srvc->client_id);
    }
  }
}






/*===========================================================================
  FUNCTION  qmi_service_write_std_txn_hdr_and_inc_txn_id
===========================================================================*/
/*!
@brief
  Adds a "standard" QMI service header to a message payload.  It then
  increments the service structure's 'next_txn_id' field

@return
  None.

@note

  - Dependencies
    - None

  - Side Effects
    - This routine "backs up" message buffer pointer to add header as well
    as changing the passed in message buffer size parameter.
    Assumption is that space before message has been properly pre-allocated.
    Function also changes the next_txn_id value of the service info record

*/
/*=========================================================================*/
static
unsigned long qmi_service_write_std_txn_hdr_and_inc_txn_id (
  qmi_srvc_client_info_type  *srvc,
  unsigned char          **msg_buf,
  int                    *msg_buf_size
)
{

  unsigned char *tmp_msg_buf;
  unsigned long curr_txn_id = srvc->next_txn_id;

  /* Back up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  *msg_buf -= QMI_SRVC_STD_TXN_HDR_SIZE;
  *msg_buf_size += QMI_SRVC_STD_TXN_HDR_SIZE;

  tmp_msg_buf = *msg_buf;
  /* Standard service header consists of a 1 byte control flags
  ** field of message type (request/response/indication),
  ** followed by a 2-byte transaction
  ** ID
  */
  WRITE_8_BIT_VAL (tmp_msg_buf,(unsigned char)QMI_STD_SRVC_REQUEST_CONTROL_FLAG);
  WRITE_16_BIT_VAL (tmp_msg_buf,curr_txn_id);

  /* Increment to next transaction ID and truncate to a 16-bit value  */
  /* According to documentation, 0 is not allowed, so when counter rolls
  ** over, make new value a 1
  */
  srvc->next_txn_id = (srvc->next_txn_id == 0xFFFF) ? 1 : (srvc->next_txn_id + 1);

  return curr_txn_id;
} /* qmi_service_write_std_txn_hdr */


/*===========================================================================
  FUNCTION  qmi_service_read_std_txn_hdr
===========================================================================*/
/*!
@brief
  Removes a "standard" QMI service header from a message payload.

@return
  0 if function is successful, negative value if not.

@note

  - Dependencies
    - None

  - Side Effects
    - Moves msg_buf pointer forward the appropriate number of bytes and
    decrements the message buffer size value accordingly.  The RX message
    type and transaction ID's are returned in their respective parameters.
*/
/*=========================================================================*/
static
int qmi_service_read_std_txn_hdr (
  unsigned char          **msg_buf,
  int                    *msg_buf_size,
  unsigned long          *rx_txn_id,
  qmi_service_msg_type   *rx_msg_type
)
{

  unsigned char *tmp_msg_buf;
  unsigned char  tmp_msg_type;

  /* Make sure buffer size is correct */
  if (*msg_buf_size < QMI_SRVC_STD_TXN_HDR_SIZE)
  {
    QMI_ERR_MSG_1("invalid header size %d\n", *msg_buf_size);
    return QMI_INTERNAL_ERR;
  }

  /* Move up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  tmp_msg_buf = *msg_buf;

  { /*lint --e{774} */
  /* Read the message type and translate into the generic message
  ** type enum
  */
  READ_8_BIT_VAL (tmp_msg_buf,tmp_msg_type);

  if (tmp_msg_type == QMI_STD_SRVC_REQUEST_CONTROL_FLAG)
  {
    *rx_msg_type = QMI_SERVICE_REQUEST_MSG;
  }
  else if (tmp_msg_type == QMI_STD_SRVC_RESPONSE_CONTROL_FLAG)
  {
    *rx_msg_type = QMI_SERVICE_RESPONSE_MSG;
  }
  else if (tmp_msg_type == QMI_STD_SRVC_INDICATION_CONTROL_FLAG)
  {
    *rx_msg_type = QMI_SERVICE_INDICATION_MSG;
  }
  else
  {
    QMI_ERR_MSG_1("invalid control flag 0x%02x\n", tmp_msg_type);
    return QMI_INTERNAL_ERR;
  }
  }


  /* Read 16-bit transaction ID */
  READ_16_BIT_VAL (tmp_msg_buf,*rx_txn_id);

  /* Set output pointer and size values */
  *msg_buf += QMI_SRVC_STD_TXN_HDR_SIZE;
  *msg_buf_size -= QMI_SRVC_STD_TXN_HDR_SIZE;

  return 0;
} /* qmi_service_read_std_txn_hdr */




/*===========================================================================
  FUNCTION  qmi_service_write_std_srvc_msg_hdr
===========================================================================*/
/*!
@brief
  Encodes a standard QMI message header (ID & length fields).

@return
  Number of bytes written (4) if success, value < 0 if failure.

@note

  - Dependencies
    - None

  - Side Effects
    - moves the msg_buf pointer back in the array, increments msg_buf_size by
    appropriate amount of header (4 bytes).
*/
/*=========================================================================*/
static
int qmi_service_write_std_srvc_msg_hdr (
  unsigned char **msg_buf,
  int           *msg_buf_size,
  unsigned long msg_id,
  int           length
)
{
  unsigned char *tmp_msg_buf;

  /* Back pointer up by 4 bytes */
  *msg_buf -= QMI_SRVC_STD_MSG_HDR_SIZE;
  *msg_buf_size += QMI_SRVC_STD_MSG_HDR_SIZE;

  tmp_msg_buf = *msg_buf;

  /* Write the message ID field (16 bits) */
  WRITE_16_BIT_VAL (tmp_msg_buf,msg_id);
  /* Write the length field */
  WRITE_16_BIT_VAL (tmp_msg_buf,length);

  return 0;
} /* qmi_service_put_std_msg_type */


/*===========================================================================
  FUNCTION  qmi_service_get_std_msg_type
===========================================================================*/
/*!
@brief
  Decodes a QMI message header (ID & length fields).

@return
  0 if succees, negative value if error occurs

@note

  - Dependencies
    - None

  - Side Effects
    - moves the msg_buf pointer ahead in array, decrements msg_buf_size by
    appropriate amount of header (4 bytes).
*/
/*=========================================================================*/
static
int qmi_service_read_std_srvc_msg_hdr (
  unsigned char **msg_buf,
  int           *msg_buf_size,
  unsigned long *msg_id,
  unsigned long *length
)
{
  unsigned char *tmp_msg_buf;

  /* Make sure buffer size is correct */
  if (*msg_buf_size < QMI_SRVC_STD_MSG_HDR_SIZE)
  {
    return QMI_INTERNAL_ERR;
  }
  /* Move up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  tmp_msg_buf = *msg_buf;
  *msg_buf += QMI_SRVC_STD_MSG_HDR_SIZE;
  *msg_buf_size -= QMI_SRVC_STD_MSG_HDR_SIZE;

  /* Get the message ID field (16 bits) */
  READ_16_BIT_VAL (tmp_msg_buf,*msg_id);

  /* Get the length field (16 bits) */
  READ_16_BIT_VAL (tmp_msg_buf,*length);

  return 0;
} /* qmi_service_get_std_msg_type */




/*===========================================================================
  FUNCTION  qmi_service_validate_client_handle
===========================================================================*/
/*!
@brief
  Validates that a user handle is valid and belongs to the passed
  in service type

@return
  TRUE if handle is valid, FALSE if not

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_service_validate_client_handle (
  int                  user_handle,
  qmi_service_id_type  service_id
)
{
  qmi_connection_id_type   handle_conn_id;
  qmi_client_id_type       handle_client_id;
  qmi_service_id_type      handle_service_id;

  handle_conn_id = QMI_SRVC_CLIENT_HANDLE_TO_CONN_ID(user_handle);
  handle_client_id = QMI_SRVC_CLIENT_HANDLE_TO_CLIENT_ID(user_handle);
  handle_service_id = QMI_SRVC_CLIENT_HANDLE_TO_SERVICE_ID(user_handle);

  /* Verify that service ID and handle service ID match */
  if (handle_service_id != service_id)
  {
    return FALSE;
  }

  /* Make sure we have a record of this service,client,connection */
  if (qmi_find_and_addref_srvc_data (handle_conn_id,
                                     handle_service_id,
                                     handle_client_id,
                                     FALSE) == NULL)
  {
    return FALSE;
  }

  return TRUE;

} /* qmi_service_validate_client_handle */





/*===========================================================================
  FUNCTION  qmi_service_process_rx_indication
===========================================================================*/
/*!
@brief
  Takes input indication message buffer and calls associated callbacks for
  all clients of the service/connection.

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    Client indication callbacks will be called.
*/
/*=========================================================================*/
void qmi_service_process_rx_indication (
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      srvc_client_id,
  unsigned long           srvc_msg_id,
  unsigned char           *rx_msg,
  int                     rx_msg_len
)
{
  qmi_srvc_client_info_type *srvc = NULL, *next_srvc = NULL;
  qmi_srvc_fn_info_type *srvc_fns = NULL;
  int index = 0;
  int book_keep_srvc_id = qmi_qcci_internal_public_service_id_to_bookkeeping_service_id (service_id);



  if ( conn_id >= QMI_MAX_CONN_IDS    ||
       service_id >= QMI_MAX_SERVICES ||
       book_keep_srvc_id >= QMI_MAX_SERVICES )
  {
    QMI_ERR_MSG_3("qmi_service_process_rx_indication: conn_id or service_id "
                  "invalid, conn_id:%d, service_id:%d, book_keep_srvc_id:%d",
                  conn_id, service_id, book_keep_srvc_id);
    return;
  }

  /* Get pointer to the appropriate service function data */
  if (qmi_srvc_fn_info[book_keep_srvc_id].srvc_rx_ind_msg_hdlr != NULL)
  {
      srvc_fns = &qmi_srvc_fn_info[book_keep_srvc_id];
  }

  /* Lock service list access mutex */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_srvc_list_mutex_table[conn_id][book_keep_srvc_id]);

  /* Get pointer to head of list */
  srvc = qmi_srvc_client_info_table [conn_id][book_keep_srvc_id];

  /* Loop through all service clients and report indication */
  while (srvc)
  {
    /* Do an addref to the service client structure... to make sure it is
    ** not deleted while we are accessing it, (although we have the
    ** list mutex locked, in theory, the indication callback could
    ** attempt to delete the service client)
    */
    if (((srvc_client_id == QMI_QMUX_BROADCAST_QMI_CLIENT_ID) ||
           (srvc->client_id == srvc_client_id)) && (qmi_service_addref (srvc) != NULL))
    {
      qmi_client_handle_type client_handle;

      /* Create user_handle */
      client_handle = QMI_SRVC_CREATE_CLIENT_HANDLE(conn_id, srvc->client_id,service_id);

      QMI_DEBUG_MSG_1 ("Client %x gets indication callback\n",(unsigned int) client_handle);

      /* Call service-specific registered callback */
      if (srvc_fns != NULL && srvc_fns->srvc_rx_ind_msg_hdlr != NULL) {
          srvc_fns->srvc_rx_ind_msg_hdlr (client_handle,
                                          service_id,
                                          srvc_msg_id,
                                          srvc->user_ind_msg_hdlr,
                                          srvc->user_ind_msg_hdlr_user_data,
                                          rx_msg,
                                          rx_msg_len);
      }
      else {
          qmi_client_ind_cb func_ptr =  srvc->user_ind_msg_hdlr;
          if ( NULL != _qmi_service_hook_indication_passthrough )
          {
            _qmi_service_hook_indication_passthrough( book_keep_srvc_id, (int)srvc_msg_id, rx_msg, rx_msg_len );
          }
          if (*func_ptr != NULL && srvc->user_decode_handle != NULL  ) {
              (*func_ptr)(srvc->user_decode_handle,
                          srvc_msg_id,
                          rx_msg,
                          rx_msg_len,
                          srvc->user_ind_msg_hdlr_user_data
                         );
          }
          else
          {
              QMI_ERR_MSG_1 ("qmi_service.c RX indication with no registered "
                             "handler for service=%d\n",service_id);
          }
      }

      next_srvc = srvc->next;

      /* Release service client structure */
      qmi_release_srvc_data (&srvc);

      if (srvc_client_id != QMI_QMUX_BROADCAST_QMI_CLIENT_ID)
      {
        break;
      }
    }
    else
    {
      next_srvc = srvc->next;
    }
    srvc = next_srvc;
  }

  /* unlock service list access mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_srvc_list_mutex_table[conn_id][book_keep_srvc_id]);

} /* qmi_service_process_rx_indication */


/*===========================================================================
  FUNCTION  qmi_service_process_rx_indication
===========================================================================*/
/*!
@brief
  Takes input indication buffer, and runs processes one indication at a time.

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    Client indication callbacks will be called.
*/
/*=========================================================================*/
void qmi_service_process_all_rx_indications (
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      client_id,
  unsigned char           *rx_msg,
  int                     rx_msg_len
)
{
  unsigned long srvc_msg_id, srvc_msg_len;
  int indications_left_to_process;

  /*if (qmi_srvc_fn_info[service_id].srvc_rx_ind_msg_hdlr == NULL)
  {
    QMI_ERR_MSG_1 ("qmi_service.c RX indication with no registered handler for service=%d\n",service_id);
    return;
  }*/


  QMI_DEBUG_MSG_2 ("qmi_service.c RX indication for conn=%d, srvc=%d\n",conn_id,service_id);

  /* Read first indication message header */
  if (qmi_service_read_std_srvc_msg_hdr (&rx_msg,
                                         &rx_msg_len,
                                         &srvc_msg_id,
                                         &srvc_msg_len) < 0)
  {
    QMI_ERR_MSG_0("qmi_service.c RX indication bad message header, returning\n");
    return;
  }

    /* Process the indication */
    QMI_DEBUG_MSG_2("Processing indication: Mesage4 ID=  %d, "
                    "Service ID = %d\n", (int)srvc_msg_id, service_id);

    qmi_service_process_rx_indication (conn_id,
                                       service_id,
                                       client_id,
                                       srvc_msg_id,
                                       rx_msg,
                                       (int)srvc_msg_len);
} /* qmi_service_process_all_rx_indications */




/*===========================================================================
  FUNCTION  qmi_service_send_msg
===========================================================================*/
/*!
@brief
  Sends a QMI service message on the specified connection.

@return
  0 if function is successful, negative value if not.

@note

  - Dependencies
    - None

  - Side Effects
    - If the transaction information passed with message indicates that
    it should be a synchronous transaction, the calling thread will block
    until response is received.
*/
/*=========================================================================*/
int qmi_service_send_msg (
  qmi_connection_id_type    conn_id,
  qmi_service_id_type       service_id,
  qmi_client_id_type        client_id,
  unsigned long             msg_id,
  unsigned char             *msg_buf,
  int                       msg_buf_size,
  qmi_service_txn_info_type         *txn
)
{
  qmi_srvc_client_info_type   *srvc;
  int                         rc;

  /* Validate connection ID and service ID values */
  if ((int)conn_id >= (int)QMI_MAX_CONN_IDS)
  {
    return QMI_INTERNAL_ERR;
  }
  if ( qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) >= QMI_MAX_SERVICES)
  {
    return QMI_INTERNAL_ERR;
  }

  if (!qmi_service_initialization_done)
  {
    QMI_ERR_MSG_0 ("Unable to send message, initialization not completed\n");
    return QMI_INTERNAL_ERR;
  }
  else if (QMI_SERVICE_CONN_STATE_ACTIVE != qmi_service_conn_state_tbl[conn_id])
  {
    QMI_ERR_MSG_1 ("Unable to send message, conn_id=%d is not active\n", conn_id);
    return QMI_INTERNAL_ERR;
  }

  /* If connection isn't open, no need to proceed */
  if (!(qmi_qmux_if_is_conn_active(qmi_service_qmux_if_handle, conn_id)))
  {
    QMI_ERR_MSG_0 ("Unable to send message, connection not active\n");
    return QMI_INTERNAL_ERR;
  }

  /* Put on message ID and length header */
  if (qmi_service_write_std_srvc_msg_hdr (&msg_buf,
                                          &msg_buf_size,
                                          msg_id,
                                          msg_buf_size) < 0)
  {
    QMI_ERR_MSG_0("qmi_service_write_std_srvc_msg_hdr failed\n");
    return QMI_INTERNAL_ERR;
  }

  /* Get pointer to the appropriate service data */
  srvc = qmi_find_and_addref_srvc_data (conn_id, service_id, client_id, TRUE);

  /* Make sure we got a valid service data pointer */
  if (!srvc)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Lock service mutex */
  QMI_PLATFORM_MUTEX_LOCK (&srvc->mutex);

  /* Put TXN ID header on message, and save in txn */
  txn->txn_id = qmi_service_write_std_txn_hdr_and_inc_txn_id (srvc,
                                                              &msg_buf,
                                                              &msg_buf_size);


  /* Send to PDU to QMUX.*/
  rc = qmi_qmux_if_send_qmi_msg (qmi_service_qmux_if_handle,
                                 conn_id,
                                 service_id,
                                 client_id,
                                 msg_buf,
                                 msg_buf_size);


  /* Note:  we don't unlock mutex and release handle to service until after
  ** we have sent message to QMUX layer.  This will prevent us from possibly
  ** sending service transaction ID's out of order
  */
  QMI_PLATFORM_MUTEX_UNLOCK (&srvc->mutex);

  /* Release reference to service data */
  qmi_release_srvc_data (&srvc);

  return rc;
} /* qmi_service_send_msg */



/*===========================================================================
  FUNCTION  qmi_service_sync_txn_delete
===========================================================================*/
/*!
@brief
  Function to delete a synchronous transaction.

@return
  None.

@note

  - Dependencies
    - None

  - Side Effects
    - Deletes reply buffer and destroys signal data
*/
/*=========================================================================*/
static
void qmi_service_sync_txn_delete (
  void *del_txn
)
{
  qmi_service_txn_info_type *txn = (qmi_service_txn_info_type *) del_txn;
  if (txn->srvc_txn_info.sync_async.sync.user_reply_buf != NULL)
  {
    free (txn->srvc_txn_info.sync_async.sync.user_reply_buf);
  }

  QMI_PLATFORM_DESTROY_SIGNAL_DATA (&txn->srvc_txn_info.sync_async.sync.signal_data);
}


/*===========================================================================
  FUNCTION  qmi_service_send_msg_sync
===========================================================================*/
/*!
@brief
  Sends a synchronous QMI service message on the specified connection.

@return
  0 if function is successful, negative value if not.

@note

  - Dependencies
    - None

  - Side Effects
    - If the transaction information passed with message indicates that
    it should be a synchronous transaction, the calling thread will block
    until response is received.
*/
/*=========================================================================*/
int qmi_service_send_msg_sync (
  int                       user_handle,
  qmi_service_id_type       service_id,
  unsigned long             msg_id,
  unsigned char             *msg_buf,
  int                       num_bytes_in_msg_buf,
  unsigned char             *reply_buf,
  int                       *reply_buf_size_ptr,
  int                       reply_buf_size,
  int                       timeout_secs,
  int                       *qmi_err_code
)
{
    int timeout_milli_secs = timeout_secs * 1000;
    /* printing the message buffer */


    return qmi_service_send_msg_sync_millisec(user_handle,
                                             service_id,
                                             msg_id,
                                             msg_buf,
                                             num_bytes_in_msg_buf,
                                             reply_buf,
                                             reply_buf_size_ptr,
                                             reply_buf_size,
                                             timeout_milli_secs,
                                             QMI_OLD_APIS,
                                             qmi_err_code);
}


int qmi_service_send_msg_sync_millisec (
  int                       user_handle,
  qmi_service_id_type       service_id,
  unsigned long             msg_id,
  unsigned char             *msg_buf,
  int                       num_bytes_in_msg_buf,
  unsigned char             *reply_buf,
  int                       *reply_buf_size_ptr,
  int                       reply_buf_size,
  int                       timeout_milli_secs,
  int                       api_flag,
  int                       *qmi_err_code
)
{

  qmi_service_txn_info_type         *txn;
  qmi_connection_id_type    conn_id;
  qmi_client_id_type        client_id;
  int                       rc;
  int                       tmo_ms = timeout_milli_secs;
  int book_keep_srvc_id;

  /* Initialize QMI error code */
  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  conn_id = QMI_SRVC_CLIENT_HANDLE_TO_CONN_ID (user_handle);
  client_id = QMI_SRVC_CLIENT_HANDLE_TO_CLIENT_ID (user_handle);

  if (service_id != QMI_SRVC_CLIENT_HANDLE_TO_SERVICE_ID(user_handle))
  {
    return QMI_INTERNAL_ERR;
  }

  book_keep_srvc_id = qmi_qcci_internal_public_service_id_to_bookkeeping_service_id (service_id);

  if ( conn_id >= QMI_MAX_CONN_IDS    ||
       service_id >= QMI_MAX_SERVICES ||
       book_keep_srvc_id >= QMI_MAX_SERVICES )
  {
    QMI_ERR_MSG_3("qmi_service_send_msg_sync_millisec: conn_id or service_id "
                  "invalid, conn_id:%d, service_id:%d, book_keep_srvc_id:%d",
                  conn_id, service_id, book_keep_srvc_id);
    return QMI_INTERNAL_ERR;
  }

  if ( QMI_CONN_ID_IS_PROXY( conn_id ) )
  {
  /* QMI proxy service can involve multiple modems' QMI services. So use QMI Proxy sanity timeout instead.*/

  /* Set the timeout value to min of QMI_PROXY_SYNC_REQ_SANITY_TIMEOUT but allow larger timeout values */

    if(tmo_ms < QMI_PROXY_SYNC_REQ_SANITY_TIMEOUT)
    {
      tmo_ms = QMI_PROXY_SYNC_REQ_SANITY_TIMEOUT;
    }

  }

  /* Allocate a transaction structure */
  txn = (qmi_service_txn_info_type *)
        qmi_util_alloc_and_addref_txn (sizeof (qmi_service_txn_info_type),
                                       qmi_service_sync_txn_delete,
                                       (qmi_txn_hdr_type **) &qmi_service_txn_table[conn_id][book_keep_srvc_id],
                                       &qmi_service_txn_mutex_table[conn_id][book_keep_srvc_id]);

  /* Make sure we allocated transaction */
  if (txn == NULL)
  {
    QMI_ERR_MSG_0 ("qmi_service_send_msg_sync: Unable to alloc TXN\n");
    return QMI_INTERNAL_ERR;
  }

  /* Initialize Id fields */
  txn->conn_id    = conn_id;
  txn->service_id = service_id;
  txn->client_id = client_id;
  txn->msg_id = msg_id;
  txn->api_flag = api_flag;
  /* Initialize fields */
  txn->srvc_txn_info.txn_type = QMI_TXN_SYNC;
  txn->srvc_txn_info.sync_async.sync.user_reply_buf = NULL;
  txn->srvc_txn_info.sync_async.sync.user_reply_buf_size = 0;
  txn->srvc_txn_info.sync_async.sync.rsp_rc = QMI_NO_ERR;
  txn->srvc_txn_info.sync_async.sync.qmi_err_code = QMI_SERVICE_ERR_NONE;
  QMI_PLATFORM_INIT_SIGNAL_DATA(&txn->srvc_txn_info.sync_async.sync.signal_data);

  QMI_DEBUG_MSG_1("Setting the api flag to : %d\n",txn->api_flag);
  /* Prepare to wait for signal for synchronous transaction */
  QMI_PLATFORM_INIT_SIGNAL_FOR_WAIT (conn_id,
                                     &txn->srvc_txn_info.sync_async.sync.signal_data);


  /* Send the message.  When this function returns, we should have reply data in
  ** the tmp_msg buffer
  */
  rc = qmi_service_send_msg (conn_id,
                             service_id,
                             client_id,
                             msg_id,
                             msg_buf,
                             num_bytes_in_msg_buf,
                             txn);

  if (rc == QMI_NO_ERR)
  {
    /* Blocks waiting for signal from QMUX */
    /* If we get a timeout, indicate so accordingly */
    if (QMI_PLATFORM_WAIT_FOR_SIGNAL (conn_id,
                                      &txn->srvc_txn_info.sync_async.sync.signal_data,
                                      tmo_ms) == QMI_TIMEOUT_ERR)
    {
      QMI_ERR_MSG_3("qmi_service_send_msg: timeout error for conn_id%d, service_id=%d, client_id=%d\n",
                    conn_id, service_id, client_id);

      if( reply_buf_size_ptr )
      {
      *reply_buf_size_ptr = 0;
      }
      rc = QMI_TIMEOUT_ERR;
      *qmi_err_code = QMI_SERVICE_ERR_NONE;
    }
    else
    {
      /* Make sure that the reply size isn't greater than that allocated by receiver */
      if (txn->srvc_txn_info.sync_async.sync.user_reply_buf_size > reply_buf_size)
      {
        if ( reply_buf_size_ptr )
        {
          *reply_buf_size_ptr = 0;
        }
        rc = QMI_INTERNAL_ERR;
        *qmi_err_code = QMI_SERVICE_ERR_NONE;
      }
      else
      {
        /* Make sure all looks good for copy to receiver */
        if ((txn->srvc_txn_info.sync_async.sync.user_reply_buf != NULL) &&
            (reply_buf != NULL) &&
            (txn->srvc_txn_info.sync_async.sync.user_reply_buf_size > 0))
        {
          memcpy (reply_buf,
                  txn->srvc_txn_info.sync_async.sync.user_reply_buf,
                  (size_t)txn->srvc_txn_info.sync_async.sync.user_reply_buf_size);
        }
        if ( reply_buf_size_ptr )
        {
          *reply_buf_size_ptr  = txn->srvc_txn_info.sync_async.sync.user_reply_buf_size;
        }
        rc                   = txn->srvc_txn_info.sync_async.sync.rsp_rc;
        *qmi_err_code        = txn->srvc_txn_info.sync_async.sync.qmi_err_code;
      }
    }
  }
  else
  {
    QMI_PLATFORM_MUTEX_UNLOCK (&txn->srvc_txn_info.sync_async.sync.signal_data.cond_mutex);
  }

  /* Release and delete the transaction */
  qmi_util_release_txn ((qmi_txn_hdr_type *)txn,
                        TRUE,
                        (qmi_txn_hdr_type **) &qmi_service_txn_table[conn_id][book_keep_srvc_id],
                        &qmi_service_txn_mutex_table[conn_id][book_keep_srvc_id]);

  return rc;

} /* qmi_service_send_msg_sync_millisec */



/*===========================================================================
  FUNCTION  qmi_service_send_msg_async
===========================================================================*/
/*!
@brief
  Sends an asynchronous QMI service message on the specified connection.

@return
  0 if function is successful, negative value if not.

@note

  - Dependencies
    - None

  - Side Effects
    - If the transaction information passed with message indicates that
    it should be a synchronous transaction, the calling thread will block
    until response is received.
*/
/*=========================================================================*/
int qmi_service_send_msg_async (
  int                       user_handle,
  qmi_service_id_type       service_id,
  unsigned long             msg_id,
  unsigned char             *msg_buf,
  int                       msg_buf_size,
  srvc_async_cb_fn_type     srvc_cb,
  void                      *srvc_cb_data,
  void                      *user_cb,
  void                      *user_cb_data
)
{
  qmi_service_txn_info_type         *txn;
  qmi_connection_id_type    conn_id;
  qmi_client_id_type        client_id;
  int                       rc;
  unsigned long                     tran_id;

  conn_id = QMI_SRVC_CLIENT_HANDLE_TO_CONN_ID (user_handle);
  client_id = QMI_SRVC_CLIENT_HANDLE_TO_CLIENT_ID (user_handle);

  if ((rc = qmi_service_setup_txn(user_handle,
                                 service_id,
                                 msg_id,
                                 srvc_cb,
                                 srvc_cb_data,
                                 user_cb,
                                 user_cb_data,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 QMI_OLD_APIS,
                                 &txn) < 0 ))
  {
      return rc;
  }


  rc = qmi_service_send_msg (conn_id,
                             service_id,
                             client_id,
                             msg_id,
                             msg_buf,
                             msg_buf_size,
                             txn);

  qmi_service_release_txn(user_handle,
                          txn,
                          &tran_id,
                          rc);

  if (rc == QMI_NO_ERR ) {
      rc = (int)tran_id;
  }

  return rc;
} /* qmi_service_send_msg_async */


/*===========================================================================
  FUNCTION  qmi_service_setup_txn
===========================================================================*/
/*!
@brief
  Setup the transaction structure for the async calls
@return
  None.

@note

  - Dependencies
    - None

  - Side Effects
    - If the transaction information passed with message indicates that
    it should be a synchronous transaction, the calling thread will block
    until response is received.
*/
/*=========================================================================*/

int  qmi_service_setup_txn
(
  int                                user_handle,
  qmi_service_id_type                service_id,
  unsigned long                      msg_id,
  srvc_async_cb_fn_type              srvc_cb,
  void                               *srvc_cb_data,
  void                               *user_cb,
  void                               *user_cb_data,
  qmi_client_recv_raw_msg_async_cb   user_rsp_raw_cb,
  qmi_client_decode_msg_async_cb     user_decode_cb,
  void                               *user_decode_handle,
  void                               *user_buf,
  int                                user_buf_len,
  int                                api_flag,
  qmi_service_txn_info_type          **txn
)
{
    qmi_connection_id_type    conn_id;
    qmi_client_id_type        client_id;
    int                       rc = 0;
    int book_keep_srvc_id;

  conn_id = QMI_SRVC_CLIENT_HANDLE_TO_CONN_ID (user_handle);
  client_id = QMI_SRVC_CLIENT_HANDLE_TO_CLIENT_ID (user_handle);
  if (service_id != QMI_SRVC_CLIENT_HANDLE_TO_SERVICE_ID(user_handle) ||
        txn == NULL)
  {
    QMI_ERR_MSG_0 ("qmi_service_send_msg_async: Bad Input Params\n");
    return QMI_INTERNAL_ERR;
  }

  book_keep_srvc_id = qmi_qcci_internal_public_service_id_to_bookkeeping_service_id (service_id);

  if ( conn_id >= QMI_MAX_CONN_IDS    ||
       service_id >= QMI_MAX_SERVICES ||
       book_keep_srvc_id >= QMI_MAX_SERVICES )
  {
    QMI_ERR_MSG_3("qmi_service_setup_txn: conn_id or service_id "
                  "invalid, conn_id:%d, service_id:%d, book_keep_srvc_id:%d",
                  conn_id, service_id, book_keep_srvc_id);
    return QMI_INTERNAL_ERR;
  }

   /* Allocate a transaction structure */
    *txn = (qmi_service_txn_info_type *)
        qmi_util_alloc_and_addref_txn (sizeof (qmi_service_txn_info_type),
                                       NULL,
                                       (qmi_txn_hdr_type **) &qmi_service_txn_table[conn_id][book_keep_srvc_id],
                                       &qmi_service_txn_mutex_table[conn_id][book_keep_srvc_id]);

  /* Make sure we allocated transaction */
  if (*txn == NULL)
  {
    QMI_ERR_MSG_0 ("qmi_service_send_msg_async: Unable to alloc TXN\n");
    return QMI_INTERNAL_ERR;
  }

  /* Initialize Id fields */
    (*txn)->conn_id    = conn_id;
    (*txn)->service_id = service_id;
    (*txn)->client_id = client_id;
    (*txn)->msg_id = msg_id;
    (*txn)->api_flag = api_flag;

    QMI_DEBUG_MSG_1(" Message ID .....................%d\n", (*txn)->msg_id);
    QMI_DEBUG_MSG_1(" Setting up api flag ........... %d\n", (*txn)->api_flag);

  /* Set up transaction type info */
    (*txn)->srvc_txn_info.txn_type = QMI_TXN_ASYNC;
    (*txn)->srvc_txn_info.sync_async.async.service_async_cb_fn = srvc_cb;
    (*txn)->srvc_txn_info.sync_async.async.service_async_cb_data = srvc_cb_data;
    (*txn)->srvc_txn_info.sync_async.async.user_async_cb_fn = user_cb;
    (*txn)->srvc_txn_info.sync_async.async.user_async_cb_data = user_cb_data;
    (*txn)->srvc_txn_info.sync_async.async.user_rsp_raw_cb = user_rsp_raw_cb;
    (*txn)->srvc_txn_info.sync_async.async.user_decode_cb = user_decode_cb;
    (*txn)->srvc_txn_info.sync_async.async.user_decode_handle = user_decode_handle;
    (*txn)->srvc_txn_info.sync_async.async.user_buf = user_buf;
    (*txn)->srvc_txn_info.sync_async.async.user_buf_len = user_buf_len;

    return rc;
}
/*===========================================================================
  FUNCTION  qmi_service_release_txn
===========================================================================*/
/*!
@brief
  Release the transaction structure for the async calls
@return
  None.

@note
  In case err_code is QMI_NO_ERR , release the transaction  but doesn't  delete...
  function that receives response will always delete txn's, otherwise release and
  delete the transaction as we will never get a response.

  - Dependencies
    - None

  - Side Effects
    - None
    */
/*=========================================================================*/
void qmi_service_release_txn
(
    int                            user_handle,
    qmi_service_txn_info_type      *txn,
    unsigned long                  *tran_id,
    int                            err_code
)
{
  unsigned int              delete_flag;
  qmi_connection_id_type    conn_id;
  qmi_service_id_type       service_id;
  int book_keep_srvc_id;

  if (err_code == QMI_NO_ERR ) {
    *tran_id = txn->txn_id;
    delete_flag = FALSE;
  }
  else
  {
    delete_flag = TRUE;
    *tran_id = 0;
  }

  conn_id = QMI_SRVC_CLIENT_HANDLE_TO_CONN_ID (user_handle);
  service_id = QMI_SRVC_CLIENT_HANDLE_TO_SERVICE_ID(user_handle);
  book_keep_srvc_id = qmi_qcci_internal_public_service_id_to_bookkeeping_service_id (service_id);

  if ( conn_id >= QMI_MAX_CONN_IDS    ||
       service_id >= QMI_MAX_SERVICES ||
       book_keep_srvc_id >= QMI_MAX_SERVICES )
  {
    QMI_ERR_MSG_3("qmi_service_release_txn: conn_id or service_id "
                  "invalid, conn_id:%d, service_id:%d, book_keep_srvc_id:%d",
                  conn_id, service_id, book_keep_srvc_id);
    return;
  }

  qmi_util_release_txn ((qmi_txn_hdr_type *)txn,
                        delete_flag,
                        (qmi_txn_hdr_type **) &qmi_service_txn_table[conn_id][book_keep_srvc_id],
                        &qmi_service_txn_mutex_table[conn_id][book_keep_srvc_id]);

}






/*===========================================================================
  FUNCTION  qmi_service_receive_msg_hdlr
===========================================================================*/
/*!
@brief
  Callback that is registered with QMUX layer that handles QMI service
  response/indication messages

@return
  None.

@note

  - Dependencies
    - None

  - Side Effects
    - If the transaction information passed with message indicates that
    it should be a synchronous transaction, the calling thread will block
    until response is received.
*/
/*=========================================================================*/
static
void qmi_service_complete_txn (
  qmi_service_txn_info_type   *txn,
  unsigned char       *rx_msg,
  int                 rx_msg_len,
  int                 rsp_rc,
  int                 qmi_err_code
)
{
    int qmi_complete_txn_err = 0;
    if (rx_msg == NULL)
    {
      rx_msg_len = 0;
    }

  /* Handle synchronous transaction.  Note the lack of elegance.  This needed
  ** to be structured like this to make a brain-dead tool (prefast) happy,
  ** so please don't clean it up
  */

  if ((txn->srvc_txn_info.txn_type == QMI_TXN_SYNC) &&
     (rx_msg_len > 0))
  {
    /* If there is return data, allocate memory for it and return it */
    void *p = malloc ((size_t)rx_msg_len);
    if (p != NULL)
    {

      memcpy (p,
              rx_msg,
              (size_t)rx_msg_len );

      txn->srvc_txn_info.sync_async.sync.user_reply_buf = p;
      txn->srvc_txn_info.sync_async.sync.user_reply_buf_size = rx_msg_len;
      txn->srvc_txn_info.sync_async.sync.rsp_rc = rsp_rc;
      txn->srvc_txn_info.sync_async.sync.qmi_err_code = qmi_err_code;
    }

    else
    {
      QMI_ERR_MSG_1 ("qmi_service_complete_txn:  Unable to allocate dynamic memory of size %d\n",rx_msg_len);
      txn->srvc_txn_info.sync_async.sync.user_reply_buf = NULL;
      txn->srvc_txn_info.sync_async.sync.user_reply_buf_size = 0;
      txn->srvc_txn_info.sync_async.sync.rsp_rc = QMI_INTERNAL_ERR;
      txn->srvc_txn_info.sync_async.sync.qmi_err_code = 0;
    }

    /* Send signal to calling thread to unblock */
    QMI_PLATFORM_SEND_SIGNAL (txn->conn_id,
                              &txn->srvc_txn_info.sync_async.sync.signal_data);
  }
  else if ((txn->srvc_txn_info.txn_type == QMI_TXN_SYNC) &&
           (rx_msg_len <= 0))
  {

    /* Copy response return code and qmi error code to sync transaction */
    txn->srvc_txn_info.sync_async.sync.user_reply_buf = NULL;
    txn->srvc_txn_info.sync_async.sync.user_reply_buf_size = rx_msg_len;
    txn->srvc_txn_info.sync_async.sync.rsp_rc = rsp_rc;
    txn->srvc_txn_info.sync_async.sync.qmi_err_code = qmi_err_code;

    /* Send signal to calling thread to unblock */
    QMI_PLATFORM_SEND_SIGNAL (txn->conn_id,
                              &txn->srvc_txn_info.sync_async.sync.signal_data);
  }

  else /* Async callback */
  {
    qmi_client_handle_type client_handle;

    client_handle = QMI_SRVC_CREATE_CLIENT_HANDLE(txn->conn_id,
                                                txn->client_id,
                                                txn->service_id);
    if (txn->srvc_txn_info.sync_async.async.service_async_cb_fn != NULL ) {
        QMI_DEBUG_MSG_0(" Calling old async Callback \n");
    txn->srvc_txn_info.sync_async.async.service_async_cb_fn
                              (client_handle,
                               txn->service_id,
                               txn->msg_id,
                               rsp_rc,
                               qmi_err_code,
                               rx_msg,
                               rx_msg_len,
                               txn->srvc_txn_info.sync_async.async.service_async_cb_data,
                               txn->srvc_txn_info.sync_async.async.user_async_cb_fn,
                               txn->srvc_txn_info.sync_async.async.user_async_cb_data);

    }
    else if (txn->srvc_txn_info.sync_async.async.user_rsp_raw_cb != NULL ) {
        QMI_DEBUG_MSG_0(" calling raw async Callback \n");
        /* Memcopy the data into user buffer registered when making an async call */
        /* Make sure the rx_msg_len is less than provided by the user */
        if (txn->srvc_txn_info.sync_async.async.user_buf_len >= rx_msg_len) {
            memcpy(txn->srvc_txn_info.sync_async.async.user_buf,
                   rx_msg,
                   (size_t)rx_msg_len);
        }
        else {
            /* Populate the user buffer with 0 */
            memset(txn->srvc_txn_info.sync_async.async.user_buf,
                   0,
                   (size_t)txn->srvc_txn_info.sync_async.async.user_buf_len);

            qmi_complete_txn_err = QMI_MEMCOPY_ERROR;
        }
        txn->srvc_txn_info.sync_async.async.user_rsp_raw_cb(txn->srvc_txn_info.sync_async.async.user_decode_handle,
                                                            txn->msg_id,
                                                            txn->srvc_txn_info.sync_async.async.user_buf,
                                                            (int)txn->srvc_txn_info.sync_async.async.user_buf_len,
                                                            txn->srvc_txn_info.sync_async.async.user_async_cb_data,
                                                            qmi_complete_txn_err);



    }
    else if (txn->srvc_txn_info.sync_async.async.user_decode_cb != NULL ) {
        QMI_DEBUG_MSG_0(" Calling cooked async Callback \n");
        txn->srvc_txn_info.sync_async.async.user_decode_cb(txn->srvc_txn_info.sync_async.async.user_decode_handle,
                                                           txn->msg_id,
                                                           rx_msg,
                                                           rx_msg_len,
                                                           txn->srvc_txn_info.sync_async.async.user_buf,
                                                           txn->srvc_txn_info.sync_async.async.user_buf_len,
                                                           txn->srvc_txn_info.sync_async.async.user_async_cb_fn,
                                                           txn->srvc_txn_info.sync_async.async.user_async_cb_data);
    }
  }
}




/*===========================================================================
  FUNCTION  qmi_service_receive_msg_hdlr
===========================================================================*/
/*!
@brief
  Callback that is registered with QMUX layer that handles QMI service
  response/indication messages

@return
  None.

@note

  - Dependencies
    - None

  - Side Effects
    - If the transaction information passed with message indicates that
    it should be a synchronous transaction, the calling thread will block
    until response is received.
*/
/*=========================================================================*/
static
void qmi_service_receive_msg_hdlr (
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      client_id,
  unsigned char           control_flags,
  unsigned char           *rx_msg,
  int                     rx_msg_len
)
{
  qmi_service_txn_info_type       *txn;
  qmi_service_msg_type    rx_msg_type;
  unsigned long           rx_txn_id;
  unsigned long           srvc_msg_id;
  unsigned long           srvc_msg_len;

  int                       rsp_rc = 0;
  qmi_service_txn_cmp_type  cmp_data;
  int                       qmi_err_code = QMI_SERVICE_ERR_NONE;

  /* Validate connection ID and service ID values */
  if( !QMI_CONN_ID_IS_VALID( conn_id ) )
  {
    QMI_ERR_MSG_1 ("qmi_service.c RX: bad conn ID=%d \n",conn_id);
    return;
  }
  if( qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) >= QMI_MAX_SERVICES)
  {
    QMI_ERR_MSG_1 ("qmi_service.c RX: bad service ID=%d \n",service_id);
    return;
  }

  /* Control flags should be set to 0x80 (service is sender) or
  ** something may be wrong
  */
  if (control_flags != 0x80)
  {
    QMI_ERR_MSG_1 ("qmi_service.c RX: Invalid control flags=%x \n",(int)control_flags);
    return;
  }

  if (qmi_service_read_std_txn_hdr (&rx_msg,
                                    &rx_msg_len,
                                    &rx_txn_id,
                                    &rx_msg_type) < 0)
  {
    QMI_ERR_MSG_0 ("qmi_service.c RX: qmi_service_read_std_txn_hdr failed\n");
    return;
  }


  /* Process indication if we get one and there is an indication message
  ** handler registered with the service
  */
  if (rx_msg_type == QMI_SERVICE_INDICATION_MSG)
  {
    /* Call function to process indication messages and return */
    qmi_service_process_all_rx_indications (conn_id,
                                            service_id,
                                            client_id,
                                            rx_msg,
                                            rx_msg_len);
    return;
  }

  /* Should be a response if not an indication */
  else if (rx_msg_type != QMI_SERVICE_RESPONSE_MSG)
  {
    QMI_ERR_MSG_1 ("qmi_service.c RX: rx_msg_type (%d) != QMI_SERVICE_RESPONSE_MSG\n",rx_msg_type);
    return;
  }

  /* Read the service message header */
  if (qmi_service_read_std_srvc_msg_hdr (&rx_msg,
                                         &rx_msg_len,
                                         &srvc_msg_id,
                                         &srvc_msg_len) < 0)
  {
    QMI_ERR_MSG_0 ("qmi_service.c RX: qmi_service_read_std_srvc_msg_hdr failed\n");
    return;
  }

  /* Make sure that the message length specified in the QMI message
  ** matches the message length that we received from SMD
  */
  if (srvc_msg_len != (unsigned long) rx_msg_len)
  {
    QMI_ERR_MSG_2 ("qmi_service.c RX: srvc_msg_len (%d) != rx_msg_len (%d)\n",
           (int)srvc_msg_len,(int)rx_msg_len);
    return;
  }


  /* Get the transaction.  If we can't find it, it is possible that
  ** it has been deleted due to aborting the command, so just
  ** return
  */
  cmp_data.client_id = client_id;
  cmp_data.txn_id = rx_txn_id;

  if ((txn = (qmi_service_txn_info_type *)
        qmi_util_find_and_addref_txn ((void *)&cmp_data,
                                      qmi_service_cmp_txn,
                                      (qmi_txn_hdr_type **) &qmi_service_txn_table[conn_id][ qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) ],
                                      &qmi_service_txn_mutex_table[conn_id][ qmi_qcci_internal_public_service_id_to_bookkeeping_service_id( service_id) ])) == NULL)
  {
    QMI_ERR_MSG_4 ("qmi_service.c RX: Can't find txn for conn=%d,srvc=%d,clnt=%d,txn_id=%d\n",
              (int)conn_id,(int)service_id,(int)client_id,(int)rx_txn_id);
    return;
  }

  /* Now get the QMI return code and error code.  The assumption here is that this
  ** will be the first TLV in every response.  This assumption was confirmed by the
  ** QMI team
  */

  /* Validate that we got correct response ID */
  if (srvc_msg_id != txn->msg_id)
  {
    rsp_rc = QMI_INTERNAL_ERR;
  }
  else
  {   /* Strip off the standard result code only in case of OLD APIs */
      QMI_DEBUG_MSG_1(" API Flag .............. %d \n",txn->api_flag );
      QMI_DEBUG_MSG_1(" Message ID ............... %d \n",txn->msg_id);
      if (txn->api_flag == QMI_OLD_APIS )
      {
          QMI_DEBUG_MSG_0(" Striping off the standard result code \n");
#ifdef FEATURE_QMI_TEST
          /* The below logic is incorrect, the response may contain
           * result at any index. Ignoring this for off-target right now */
          rsp_rc = qmi_err_code = 0;
#else
          rsp_rc = qmi_util_get_std_result_code (&rx_msg,
                                           &rx_msg_len,
                                           &qmi_err_code);
#endif
      }
  }

  /* Complete the transaction */
  qmi_service_complete_txn (txn,
                            rx_msg,
                            rx_msg_len,
                            rsp_rc,
                            qmi_err_code);


  /* Free the transaction and delete it.  Note that we have to delete it to handle case
  ** of ASYNC transactions.... but this will be OK for SYNC transactions because
  ** a reference is always kept to the transaction by the calling context, so the
  ** transaction won't be deleted until done so by calling context
  */
  qmi_util_release_txn ((qmi_txn_hdr_type *)txn,
                         TRUE,
                         (qmi_txn_hdr_type **) &qmi_service_txn_table[conn_id][  qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) ],
                         &qmi_service_txn_mutex_table[conn_id][ qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) ]);

} /* qmi_service_rx_msg */


/*===========================================================================
  FUNCTION  qmi_service_reset_all
===========================================================================*/
/*!
@brief
  Resets all internal data.  All services data structure are freed and
  the client ID's are released on modem if release_services_on_modem is
  TRUE.  Also, all pending transactions are completed with return of
  QMI_TIMEOUT_ERR.

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    - Will potentially communicate with QMUX/modem if release_services_on_modem
    is TRUE
*/
/*=========================================================================*/
static
int qmi_service_reset_all (
  int                     release_services_on_modem,
  qmi_connection_id_type  conn_id
)
{
  qmi_service_id_type     service_id;
  qmi_srvc_client_info_type *srvc, *next_srvc;
  int qmi_err_code;

  /* Delete all services */
  QMI_DEBUG_MSG_2 ("Called qmi_service_reset_all, release on modem=%d, conn_id=%d\n",
                   release_services_on_modem,
                   conn_id);

  for (service_id = QMI_FIRST_SERVICE; service_id < QMI_MAX_SERVICES; service_id++)
  {
    /* Lock global service access mutex */
    QMI_PLATFORM_MUTEX_LOCK (&qmi_srvc_list_mutex_table[conn_id][service_id]);

    srvc = qmi_srvc_client_info_table[conn_id][service_id];

    while (srvc)
    {
      qmi_client_id_type client_id = srvc->client_id;
      /* Get a handle on the next service in the list */
      next_srvc = srvc->next;

      /* Release service on modem if necessary */
      if ((qmi_free_srvc_data (conn_id, qmi_qcci_internal_bookkeeping_service_id_to_public_service_id ( service_id ), srvc->client_id, FALSE) == QMI_NO_ERR) &&
          (release_services_on_modem == TRUE))
      {
        (void) qmi_qmux_if_release_service_client (qmi_service_qmux_if_handle,
                                                   conn_id,
                                                   qmi_qcci_internal_bookkeeping_service_id_to_public_service_id ( service_id ),
                                                   client_id,
                                                   &qmi_err_code);
      }

      srvc = next_srvc;
    }

    /* Set service list pointer to NULL */
    qmi_srvc_client_info_table[conn_id][service_id] = NULL;

    /* Unlock global service access mutex */
    QMI_PLATFORM_MUTEX_UNLOCK (&qmi_srvc_list_mutex_table[conn_id][service_id]);


    /* TODO:  This is no good now that txn is basically it's own subsystem... fix
    ** this in the future */

  } /* for service_id */

  return QMI_NO_ERR;

}

/*===========================================================================
  FUNCTION  qmi_service_sys_event_handler
===========================================================================*/
/*!
@brief
  qmi_service layer system event handler.  If system event is modem restart
  then all pending transactions are completed and deleted.  Then calls upper
  layer system event handler

@return
  QMI_NO_ERR if success, negative value if not

@note

  - Dependencies
    - None

  - Side Effects
    -

*/
/*=========================================================================*/
static void qmi_service_sys_event_handler
(
  qmi_sys_event_type              event_id,
  const qmi_sys_event_info_type   *event_info,
  void                            *user_data
)
{
  int conn_id;

  if (NULL == event_info)
  {
    QMI_ERR_MSG_0 ("qmi_service_sys_event_handler: bad parameter\n");
    return;
  }

  QMI_DEBUG_MSG_1 ("qmi_service_sys_event_handler: sys event_id=%d\n",event_id);

  if (QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND == event_id ||
      QMI_SYS_EVENT_MODEM_IN_SERVICE_IND == event_id)
  {
    conn_id = event_info->qmi_modem_service_ind.conn_id;

    if (!QMI_CONN_ID_IS_VALID(conn_id))
    {
      QMI_ERR_MSG_1 ("qmi_service_sys_event_handler: invalid conn_id=%d\n",
                     conn_id);
      return;
    }

    if (event_id == QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND)
    {
      QMI_DEBUG_MSG_1 ("qmi_service_sys_event_handler: marking conn_id=%d inactive\n", conn_id);
      qmi_service_conn_state_tbl[conn_id] = QMI_SERVICE_CONN_STATE_INACTIVE;
      qmi_service_reset_all (FALSE, conn_id);
    }
    else if (event_id == QMI_SYS_EVENT_MODEM_IN_SERVICE_IND)
    {
      QMI_DEBUG_MSG_1 ("qmi_service_sys_event_handler: marking conn_id=%d active\n", conn_id);
      qmi_service_conn_state_tbl[conn_id] = QMI_SERVICE_CONN_STATE_ACTIVE;
    }
  }

  if (qmi_service_sys_event_hdlr_f != NULL)
  {
    qmi_service_sys_event_hdlr_f (event_id, event_info, user_data);
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_service_sys_event_handler: qmi_service_sys_event_hdlr_f is NULL\n");
  }
}



/*===========================================================================
  FUNCTION  qmi_service_pwr_up_init
===========================================================================*/
/*!
@brief
  Initializes a QMI connection

@return
  0 if successful, negative number if not successful

@note

  - Dependencies
    - None

  - Side Effects
    - QMI connection is opened
*/
/*=========================================================================*/
int qmi_service_pwr_up_init (
  qmi_sys_event_rx_hdlr   event_rx_hdlr,
  void                    *event_user_data
)
{
  int i,j,rc;
  rc = QMI_NO_ERR;

  /* If initialization hasn't been done, do it now */
  if (!qmi_service_initialization_done)
  {
    /* Call lower layer powerup init routines */
    rc = qmi_qmux_if_pwr_up_init(qmi_service_receive_msg_hdlr,
                                 qmi_service_sys_event_handler,
                                 event_user_data,
                                 &qmi_service_qmux_if_handle);

    /* Return failure if the power-up init fails at the lower layers */
    if (QMI_NO_ERR != rc)
    {
      return rc;
    }

    /* Initialize service and transaction info structures */
    for (i=0; i<QMI_MAX_CONN_IDS; i++)
    {
      qmi_service_conn_state_tbl[i] = QMI_SERVICE_CONN_STATE_ACTIVE;

      for (j=0; j<(int)QMI_MAX_SERVICES; j++)
      {
        qmi_srvc_client_info_table[i][j] = NULL;
        qmi_service_txn_table[i][j] = NULL;
        QMI_PLATFORM_MUTEX_INIT (&qmi_service_txn_mutex_table[i][j]);
        QMI_PLATFORM_MUTEX_INIT (&qmi_srvc_list_mutex_table[i][j]);
      }
    }

    qmi_service_sys_event_hdlr_f = event_rx_hdlr;

    /* Call all of the service-specific initialization callbacks */
    for (i=0; i < (int)QMI_SERVICE_INIT_RELEASE_TABLE_SIZE; i++)
    {
      if (qmi_init_release_cb_table[i].init_f_ptr != NULL)
      {
        if ((rc = qmi_init_release_cb_table[i].init_f_ptr()) != QMI_NO_ERR)
        {
          QMI_ERR_MSG_1 ("Service init failed for index = %d\n",i);
          return rc;
        }
      }
    }

    /* Mark initialization as complete */
    qmi_service_initialization_done = 1;
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_service_pwr_up_init failed... already initialized");
  }

  return rc;
} /* qmi_service_pwr_up_init */



/*===========================================================================
  FUNCTION  qmi_service_pwr_down_release
===========================================================================*/
/*!
@brief
  Initializes a QMI connection

@return
  0 if successful, negative number if not successful

@note

  - Dependencies
    - None

  - Side Effects
    - QMI connection is opened
*/
/*=========================================================================*/
int qmi_service_pwr_down_release (
  void
)
{
  int i,j,rc;

  rc = QMI_NO_ERR;

  if (qmi_service_initialization_done)
  {
    for (i=0; i<QMI_MAX_CONN_IDS; i++)
    {
      qmi_service_conn_state_tbl[i] = QMI_SERVICE_CONN_STATE_INACTIVE;

      /* clean up service transactions/data structures */
      (void) qmi_service_reset_all(TRUE, i);
    }

    /* Call all of the service-specific release callbacks */
    for (i=0; i < (int)QMI_SERVICE_INIT_RELEASE_TABLE_SIZE; i++)
    {
      if (qmi_init_release_cb_table[i].release_f_ptr != NULL)
      {
        if ((rc = qmi_init_release_cb_table[i].release_f_ptr()) != QMI_NO_ERR)
        {
          QMI_ERR_MSG_1 ("Service release failed for index = %d\n",i);
          return rc;
        }
      }
    }
    /* Tell QMUX and QMUX IF to clean up */
    (void) qmi_qmux_if_pwr_down_release(qmi_service_qmux_if_handle);
    qmi_service_qmux_if_handle = QMI_QMUX_IF_INVALID_HNDL;

    /* Destroy service and transaction mutexes */
    for (i=0; i<QMI_MAX_CONN_IDS; i++)
    {
      for (j=0; j<(int)QMI_MAX_SERVICES; j++)
      {
        QMI_PLATFORM_MUTEX_DESTROY (&qmi_service_txn_mutex_table[i][j]);
        QMI_PLATFORM_MUTEX_DESTROY (&qmi_srvc_list_mutex_table[i][j]);
      }
    }

    /* Take out of initialized state */
    qmi_service_initialization_done = FALSE;
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_service_pwr_down_release failed... not initialized");
    rc = QMI_INTERNAL_ERR;
  }

  return rc;
} /* qmi_service_pwr_down_release */




int
qmi_service_connection_init
(
  qmi_connection_id_type  conn_id,
  int                     *qmi_err_code
)
{
  int rc;

  /* Initialize QMI error code */
  *qmi_err_code = QMI_SERVICE_ERR_NONE;

  if (!qmi_service_initialization_done)
  {
    QMI_ERR_MSG_0 ("Unable to open connection, initialization not completed\n");
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    /* Initialize the error code */
    *qmi_err_code = QMI_SERVICE_ERR_NONE;

    /* Try to bring up the connection.... */
    if ((rc = qmi_qmux_if_open_connection (qmi_service_qmux_if_handle, conn_id)) != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1 ("qmi_service_init: open connection failed, rc=%d\n",rc);
    }
  }


  return rc;
}


/*===========================================================================
  FUNCTION  qmi_service_init
===========================================================================*/
/*!
@brief
  Initializes a specified service on a specified connection.  Upon
  successful return, the service may be used on the connection.

@return
  0 if function is successful, negative value if not.

@note

  - Dependencies
    - None

  - Side Effects
    - Thread may block waiting for client ID to be returned by lower layers
*/
/*=========================================================================*/
qmi_client_handle_type qmi_service_init (
  qmi_connection_id_type             conn_id,
  qmi_service_id_type                service_id,
  void                               *user_ind_msg_hdlr,
  void                               *user_ind_msg_hdlr_user_data,
  int                                *qmi_err_code
)
{
  qmi_srvc_client_info_type  *srvc;
  qmi_client_id_type     client_id;
  int                    rc;

  /* Make sure initialization has been done */
  if (!qmi_service_initialization_done)
  {
    return (qmi_client_handle_type) QMI_INTERNAL_ERR;
  }

  /* Check to make sure that the connection has been enabled */
  /* Validate connection ID and service ID values */
  if ((int)conn_id >= (int)QMI_MAX_CONN_IDS)
  {
    return (qmi_client_handle_type) QMI_INTERNAL_ERR;
  }
  if ( qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) >= QMI_MAX_SERVICES)
  {
    return (qmi_client_handle_type) QMI_INTERNAL_ERR;
  }

  /* If connection isn't open, no need to proceed */
  if (!(qmi_qmux_if_is_conn_active(qmi_service_qmux_if_handle, conn_id)))
  {
    return (qmi_client_handle_type) QMI_INTERNAL_ERR;
  }

  /* Get a client ID */
  if ((rc = qmi_qmux_if_alloc_service_client (qmi_service_qmux_if_handle,
                                              conn_id,
                                              service_id,
                                              &client_id,
                                              qmi_err_code)) < 0)
  {
    return (qmi_client_handle_type) rc;
  }

  /* Set up pointer to the appropriate service data */
  srvc = qmi_alloc_srvc_data (conn_id, service_id, client_id);
  if (srvc == NULL)
  {
    QMI_ERR_MSG_0 ("qmi_service_init: Unable to alloc SRVC data\n");
    return (qmi_client_handle_type) QMI_INTERNAL_ERR;
  }

  srvc->next_txn_id = 1;
  srvc->user_ind_msg_hdlr = user_ind_msg_hdlr;
  srvc->user_ind_msg_hdlr_user_data = user_ind_msg_hdlr_user_data;

  /* Construct a user service ID handle from the connection ID and
  ** the client ID
  */
  return QMI_SRVC_CREATE_CLIENT_HANDLE (conn_id, client_id, service_id);

}  /* qmi_service_init */


/*===========================================================================
  FUNCTION  qmi_service_release
===========================================================================*/
/*!
@brief
  Deletes the service by deleting internal data structures and
  releasing the service in the qmux_if layer.

@return
  0 if function is successful, negative value if not.

@note

  - Dependencies
    - None

  - Side Effects
    - Thread may block waiting for client ID to be returned by lower layers
*/
/*=========================================================================*/
int qmi_service_release (
  int  user_handle,
  int  *qmi_err_code
)
{
  qmi_connection_id_type    conn_id;
  qmi_client_id_type        client_id;
  qmi_service_id_type       service_id;
  int                       rc = QMI_INTERNAL_ERR;

  QMI_DEBUG_MSG_1 ("qmi_service_release called, user_handle=%x\n",user_handle);

  conn_id   = QMI_SRVC_CLIENT_HANDLE_TO_CONN_ID (user_handle);
  client_id = QMI_SRVC_CLIENT_HANDLE_TO_CLIENT_ID (user_handle);
  service_id = QMI_SRVC_CLIENT_HANDLE_TO_SERVICE_ID (user_handle);

  if ((int)conn_id >= (int)QMI_MAX_CONN_IDS)
  {
    QMI_ERR_MSG_1 ("qmi_service_release invalid conn_id=%d\n",conn_id);
    return QMI_INTERNAL_ERR;
  }

  if ((int)service_id >= (int)QMI_MAX_SERVICES)
  {
    QMI_ERR_MSG_1 ("qmi_service_release invalid service_id=%d\n",service_id);
    return QMI_INTERNAL_ERR;
  }

  /* Release the service information */
  rc = qmi_free_srvc_data (conn_id, service_id, client_id, TRUE);

  /* If service info was found, release it in QMUX */
  if (rc == QMI_NO_ERR)
  {
    rc = qmi_qmux_if_release_service_client (qmi_service_qmux_if_handle,
                                             conn_id,
                                             service_id,
                                             client_id,
                                             qmi_err_code);
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_service_set_data_format
===========================================================================*/
/*!
@brief
  Implements the CTL functionality of set data format, which will set
  which format the data channel will use (Ethernet, raw IP, etc).

@return
  0 if function is successful, negative value if not.

@note

  - Dependencies
    - None

  - Side Effects
    - Thread may block waiting for client ID to be returned by lower layers
*/
/*=========================================================================*/
int qmi_service_set_data_format (
  qmi_connection_id_type                conn_id,
  qmi_data_format_qos_hdr_state_type    qos_hdr_state,
  qmi_link_layer_protocol_type          *link_protocol,
  int                                   *qmi_err_code
)
{
  /* Do some error checking */

  /* Make sure only the bits we know about are set in link_protocol */
  if ((*link_protocol & ~(QMI_DATA_FORMAT_LINK_PROTOCOL_ALL)) != 0)
  {
    QMI_ERR_MSG_1 ("qmi_service_set_data_format: bad link protocol = %x\n",(int)*link_protocol);
    return QMI_INTERNAL_ERR;
  }

  if ((qos_hdr_state != QMI_DATA_FORMAT_WITHOUT_QOS_HDR) &&
      (qos_hdr_state != QMI_DATA_FORMAT_WITH_QOS_HDR))
  {
    QMI_ERR_MSG_1 ("qmi_service_set_data_format: bad qos hdr state = %x\n",(int)qos_hdr_state);
    return QMI_INTERNAL_ERR;
  }

  return qmi_qmux_if_set_data_format (qmi_service_qmux_if_handle,
                                      conn_id,
                                      qos_hdr_state,
                                      link_protocol,
                                      qmi_err_code);
}


/*===========================================================================
  FUNCTION  qmi_service_reg_pwr_save_mode
===========================================================================*/
/*!
@brief
  This function is used to register/de-register for power state change
  events.  Calls relevant QMI_QMUX function


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
int
qmi_service_reg_pwr_save_mode
(
  qmi_pwr_report_type   report_state,
  int                   *qmi_err_code
)
{
  return qmi_qmux_if_reg_pwr_save_mode (qmi_service_qmux_if_handle,
                                        report_state,
                                        qmi_err_code);
}

/*===========================================================================
  FUNCTION  qmi_service_config_pwr_save_settings
===========================================================================*/
/*!
@brief
  Configures the power state indication filter for each connection.
  Calls relevant QMI QMUX function.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
int
qmi_service_config_pwr_save_settings
(
  int                  pwr_state_hndl,
  qmi_service_id_type  service_id,
  int                  num_indication_ids,
  unsigned short       indication_ids[],
  int                  *qmi_err_code
)
{
  return qmi_qmux_if_config_pwr_save_settings (qmi_service_qmux_if_handle,
                                               pwr_state_hndl,
                                               service_id,
                                               num_indication_ids,
                                               indication_ids,
                                               qmi_err_code);
}

/*===========================================================================
  FUNCTION  qmi_service_set_pwr_state
===========================================================================*/
/*!
@brief
  Sets power state.  Calls relevant QMI QMUX function to do so.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    - Talks to modem processor
    - Modem will not send filtered indications until later power state change.
*/
/*=========================================================================*/
int
qmi_service_set_pwr_state
(
  unsigned long        pwr_state,
  int                  *qmi_err_code
)
{
  return qmi_qmux_if_set_pwr_state (qmi_service_qmux_if_handle, pwr_state, qmi_err_code);
}

/*===========================================================================
  FUNCTION  qmi_service_get_pwr_state
===========================================================================*/
/*!
@brief
  Gets power state.  Calls relevant QMI QMUX function to do so.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
int
qmi_service_get_pwr_state
(
  const char       *dev_id,
  unsigned long    *pwr_state,
  int              *qmi_err_code
)
{
  qmi_connection_id_type conn_id;

  if ((conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID(dev_id)) == QMI_CONN_ID_INVALID)
  {
    return QMI_INTERNAL_ERR;
  }

  return qmi_qmux_if_get_pwr_state (qmi_service_qmux_if_handle,
                                    conn_id,
                                    pwr_state,
                                    qmi_err_code);
}

/*=========================================================================
 FUNCTION qmi_service_get_version
=========================================================================*/
/*!
@brief
  Gets the major and minor version identifier for specified service.

@return
  QMI_NO_ERR if function is successful, error code otherwise. If
  return code is QMI_SERVICE_ERR, then the qmi_err_code will be valid
  and will indicate which QMI error occurred.

@note

  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
int
qmi_service_get_version
(
  const char                    *dev_id,
  qmi_service_id_type            service_id,
  qmi_service_version_info      *service_version_info,
  int                           *qmi_err_code
)
{
  qmi_connection_id_type conn_id;
  unsigned short         major_ver;    /*  Major version number */
  unsigned short         minor_ver;    /*  Minor version number */
  int                    rc = QMI_NO_ERR;
  int                    err_code;

  if (!dev_id || !service_version_info || !qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_service_get_version: Bad Input received");
    return QMI_INTERNAL_ERR;
  }

  if ((conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID(dev_id)) == QMI_CONN_ID_INVALID)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_qmux_if_get_version_list( qmi_service_qmux_if_handle,
                                     conn_id,
                                     service_id,
                                     &major_ver,
                                     &minor_ver,
                                     qmi_err_code );

  if (rc == QMI_NO_ERR && *qmi_err_code == QMI_SERVICE_ERR_NONE ) {
    service_version_info->major_ver = major_ver;
    service_version_info->minor_ver = minor_ver;
  }

  return rc;
}


// ---------------------------------------------------------------------------------------
// qmi_qcci_internal_public_service_id_to_bookkeeping_service_id
// ---------------------------------------------------------------------------------------
int qmi_qcci_internal_public_service_id_to_bookkeeping_service_id(int public_service_id)
{
   int res;

   if ( public_service_id < QMI_FIRST_VS_SERVICE )
   {
     res = public_service_id;
   }
   else
   {
     switch ( public_service_id )
     {
       case 0xE3: // RF SAR
         res = QMI_RF_SAR_SERVICE;
         break;

       default:
         res = QMI_MAX_SERVICES;
         break;
     }
   }

   return res;
} // qmi_qcci_internal_public_service_id_to_bookkeeping_service_id


// ---------------------------------------------------------------------------------------
// qmi_qcci_internal_bookkeeping_service_id_to_public_service_id
// ---------------------------------------------------------------------------------------
int qmi_qcci_internal_bookkeeping_service_id_to_public_service_id(int bookkeeping_service_id)
{
   int res;

   if ( bookkeeping_service_id < QMI_FIRST_VS_SERVICE )
   {
     res = bookkeeping_service_id;
   }
   else
   {
     switch ( bookkeeping_service_id )
     {
       case QMI_RF_SAR_SERVICE:
         res = 0xE3; // RF SAR
         break;

       default:
         res = QMI_MAX_SERVICES;
         break;
     }
   }

   return res;
} // qmi_qcci_internal_bookkeeping_service_id_to_public_service_id


/*===========================================================================
  FUNCTION  qmi_service_get_qmux_if_client_handle
===========================================================================*/
/*!
@brief
  Gets the QMUX IF client handle

@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
qmi_qmux_if_hndl_t
qmi_service_get_qmux_if_handle(void)
{
  return qmi_service_qmux_if_handle;
}

/* internal logging extensions */
/*=========================================================================
 FUNCTION _qmi_client_set_passthrough_hook_indication
 =========================================================================*/
/*!
@brief

  installs internal hooks for QMI indication bytestream traffic
@return
  none

@note

  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
void _qmi_client_set_passthrough_hook_indication( _qmi_service_hook_indication_passthrough_type indication_passthrough )
{
  _qmi_service_hook_indication_passthrough = indication_passthrough;
}

