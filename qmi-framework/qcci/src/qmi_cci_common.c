/******************************************************************************
  @file    qmi_cci_common.c
  @brief   The QMI common client interface common module

  DESCRIPTION
  QMI common client routines.  All client will be build on top of these
  routines for initializing, sending messages and receiving responses/
  indications.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_client_init() needs to be called before sending or receiving of any
  service specific messages

  ---------------------------------------------------------------------------
  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_target.h"
#include "qmi_cci_common.h"
#include "qmi_fw_debug.h"
#include <string.h>

/*===========================================================================
                      MACROS AND DEFINES
===========================================================================*/
/* Backward Compatibilty Macro */

/* By default there is no OS Parameters to copy
 * (This is the case for all OSes except REX). Note
 * this is NOT supposed to copy the entire OS parameters
 * variable but must copy just the user provided parameters
 * which will be required to call QMI_CCI_OS_SIGNAL_INIT* */
#ifndef QMI_CCI_COPY_OS_PARAMS
#define QMI_CCI_COPY_OS_PARAMS(dest, src)
#endif

#ifndef QMI_CCI_OS_SIGNAL_INIT_SELF
#define QMI_CCI_OS_SIGNAL_INIT_SELF(ptr, os_params) QMI_CCI_OS_SIGNAL_INIT(ptr, os_params)
#endif

#define QMI_CCI_FREE_TXN(txn) do {\
  QMI_CCI_OS_SIGNAL_DEINIT(&txn->signal); \
  LOCK_DEINIT(&txn->lock);  \
  FREE(txn);  \
}while(0)

/* Use only when txn_list_lock is held */
#define QMI_CCI_INC_TXN(txn) do { \
  (txn)->ref_count++; \
} while(0)

#define QMI_CCI_DEC_TXN(txn) do {  \
  if((txn)->ref_count > 0) { \
    (txn)->ref_count--; \
    if((txn)->ref_count == 0) { \
      QMI_CCI_FREE_TXN(txn);  \
    } \
  } \
  else { \
    QMI_FW_LOGE("%s: Invalid ref_count %d - txn %p\n", \
                 __func__, (txn)->ref_count, txn); \
  } \
} while(0)

#define QMI_CCI_INC_TXN_SAFE(clnt,txn) do {  \
  LOCK(&(clnt)->txn_list_lock); \
  QMI_CCI_INC_TXN(txn); \
  UNLOCK(&(clnt)->txn_list_lock); \
} while(0)


#define QMI_CCI_DEC_TXN_SAFE(clnt,txn) do {  \
  LOCK(&(clnt)->txn_list_lock); \
  QMI_CCI_DEC_TXN(txn);  \
  UNLOCK(&(clnt)->txn_list_lock); \
} while(0)

#define INVALIDATE_TXN_RX_BUF_SAFE(txn) do { \
  LOCK(&(txn)->lock); \
  (txn)->rx_buf = NULL; \
  (txn)->rx_buf_len = 0;  \
  (txn)->rx_cb_data = NULL; \
  UNLOCK(&(txn)->lock); \
} while(0)

#define INVALIDATE_TXN_TX_BUF(txn) do { \
  if((txn)->tx_buf) \
    FREE((txn)->tx_buf);  \
  (txn)->tx_buf = NULL; \
  (txn)->tx_buf_len = 0;  \
} while(0)

/* 0 is a surprisingly common 'random' number. So consider it as invalid */
#define INVALID_CLID 0
#define INVALID_HANDLE CAST_CLID_TO_HANDLE(INVALID_CLID)

#define CAST_CLID_TO_HANDLE(clid) ((qmi_client_type)(uintptr_t)(clid))
#define CAST_HANDLE_TO_CLID(handle) ((uint32_t)(uintptr_t)(handle))
#define CLIENT_HANDLE(clnt) CAST_CLID_TO_HANDLE((clnt)->priv.clid)

#define CLIENT_TBL_COUNT (16)
#define CLID2IDX(clid) ((clid) & (CLIENT_TBL_COUNT - 1))


#define QCCI_MIN(a, b) ((a) > (b) ? (b) : (a))

#define CLIENT_XPORT_VALID(clnt) \
          (LIST_HEAD((clnt)->xport)->ops != NULL && \
           LIST_HEAD((clnt)->xport)->handle != NULL)

/* Allow compilation env to set this differently
 * for memory constrained systems */
#ifndef MAX_XPORTS
#define MAX_XPORTS 21
#endif

struct qmi_cci_xport_tbl_s
{
  qmi_cci_xport_ops_type *ops;
  void *xport_data;
  uint8_t uid;
};

static struct qmi_cci_xport_tbl_s xport_tbl[MAX_XPORTS];
static unsigned int num_xports = 0;
static uint8_t next_xport_uid = 0;
static qmi_cci_lock_type xport_tbl_lock;

static qmi_cci_lock_type qcci_tbl_lock;
static uint32_t next_clid = 1;
static LIST(qmi_cci_client_type, qcci_client_tbl)[CLIENT_TBL_COUNT];
static int qcci_fw_inited = 0;

static void handle_txn_error
(
 qmi_cci_client_type *clnt,
 qmi_cci_txn_type *txn,
 int error
);
/*===========================================================================
    FUNCTION  qmi_cci_init
 ===========================================================================*/
/*!
 * @brief
 *
 *   One time initialization of the QCCI stack.
 *
 *   @return
 *     None
 *
 *     @note
 *     This function is NOT re-enterable or thread safe. The only safe place
 *     to call this is during init.
 */
/*=========================================================================*/
void qmi_cci_init(void)
{
  if(qcci_fw_inited == 0)
  {
    LOCK_INIT(&qcci_tbl_lock);
    LOCK_INIT(&xport_tbl_lock);
    qcci_fw_inited = 1;
  }
}

/*===========================================================================
    FUNCTION  qmi_cci_deinit
 ===========================================================================*/
/*!
 * @brief
 *
 *   de-initialization of the QCCI stack.
 *
 *   @return
 *     None
 *
 *     @note
 *     This function is NOT re-enterable or thread safe. The only safe place
 *     to call this is during library deinit.
 */
/*=========================================================================*/
void qmi_cci_deinit(void)
{
  if(qcci_fw_inited)
  {
    qcci_fw_inited = 0;
    LOCK_DEINIT(&qcci_tbl_lock);
    LOCK_DEINIT(&xport_tbl_lock);
  }
}

/*===========================================================================
    FUNCTION  qmi_cci_client_lookup
 ===========================================================================*/
/*!
 * @brief
 *
 *   Lookup and return the client structure by taking in the client ID as a key
 *
 *   @return
 *
 *   pointer to the client handle upon success.
 *
 *   @note
 *   qcci_tbl_lock must be held by the caller
 */
/*=========================================================================*/
static qmi_cci_client_type *qmi_cci_client_lookup
(
  uint32_t clid
)
{
  qmi_cci_client_type *clnt;

  LIST_FIND(qcci_client_tbl[CLID2IDX(clid)], clnt, priv.link,
                      clnt->priv.clid == clid);
  return clnt;
}

/*===========================================================================
    FUNCTION  qmi_cci_client_alloc
  ===========================================================================*/
/*!
 * @brief
 *
 *   Allocate a client handle and return its pointer
 *
 *   @return
 *
 *   pointer to the client handle upon success.
 *
 *   @note
 *   the client handle will be inserted into the client list.
 *   the pointer MUST be freed using qmi_cci_client_free() only.
 */
/*=========================================================================*/
static qmi_cci_client_type *qmi_cci_client_alloc
(
  qmi_idl_service_object_type service_obj,
  qmi_client_os_params *os_params,
  qmi_client_os_params *ext_os_params,
  qmi_client_ind_cb ind_cb,
  void *ind_cb_data
)
{
  qmi_cci_client_type *clnt;
  unsigned int idx;
 /* This is NOT thread safe, but coming down to it,
  * it is better than nothing protecting locking an
  * uninitialized lock */
  if(!qcci_fw_inited)
    return NULL;

  clnt = CALLOC(1, sizeof(*clnt));
  if(!clnt)
    return NULL;

  QMI_CCI_OS_SIGNAL_INIT(&clnt->signal, os_params);
  #ifdef QMI_CCI_OS_SIGNAL_VALID
  if(!QMI_CCI_OS_SIGNAL_VALID(&clnt->signal))
  {
    FREE(clnt);
    return NULL;
  }
  #endif

  if(ext_os_params)
  {
    QMI_CCI_OS_EXT_SIGNAL_INIT(clnt->ext_signal, ext_os_params);
    #ifdef QMI_CCI_OS_EXT_SIGNAL_VALID
    if(!QMI_CCI_OS_EXT_SIGNAL_VALID(clnt->ext_signal))
    {
      QMI_CCI_OS_SIGNAL_DEINIT(&clnt->signal);
      FREE(clnt);
      return NULL;
    }
    #endif
  }
  LOCK_INIT(&clnt->priv.ref_count_lock);
  LOCK_INIT(&clnt->info_lock);
  LOCK_INIT(&clnt->txn_list_lock);
  LOCK_INIT(&clnt->tx_q_lock);
  LOCK_INIT(&clnt->server_addr_valid_lock);
  LIST_INIT(clnt->xport);
  clnt->service_obj = service_obj;
  clnt->next_txn_id = 1;
  clnt->accepting_txns = 1;
  clnt->release_cb = NULL;
  clnt->info.client.err_cb = NULL;
  clnt->info.client.err_cb_data = NULL;
  clnt->info.client.err_pending = 0;
  LIST_INIT(clnt->txn_list);
  LIST_INIT(clnt->tx_q);

  clnt->ind_cb = ind_cb;
  clnt->ind_cb_data = ind_cb_data;
  /* Allocate one ref for this call */
  clnt->priv.ref_count = 1;
  LINK_INIT(clnt->priv.link);

  LOCK(&qcci_tbl_lock);
  clnt->priv.clid = INVALID_CLID;
  /* Get a new unused and valid clid */
  while((clnt->priv.clid == INVALID_CLID))
  {
    clnt->priv.clid = next_clid++;
    if(qmi_cci_client_lookup(clnt->priv.clid) != NULL)
    {
      clnt->priv.clid = INVALID_CLID;
    }
  }
  idx = CLID2IDX(clnt->priv.clid);
  LIST_ADD(qcci_client_tbl[idx], clnt, priv.link);
  UNLOCK(&qcci_tbl_lock);
  return clnt;
}

/*===========================================================================
    FUNCTION  qmi_cci_client_unlink
 ===========================================================================*/
/*!
 * @brief
 *
 *   Unlinks a client handle from the client list.
 *
 *   @return
 *
 *   None
 *
 *   @note
 *   Needs qcci_tbl_lock to be held by the caller.
 */
/*=========================================================================*/
static void qmi_cci_client_unlink(qmi_cci_client_type *clnt)
{
  qmi_cci_client_type *i;
  unsigned int idx;
  if(!clnt)
    return;
  idx = CLID2IDX(clnt->priv.clid);
  LIST_FIND(qcci_client_tbl[idx], i, priv.link,
                i->priv.clid == clnt->priv.clid);
  if(i)
  {
    LIST_REMOVE(qcci_client_tbl[idx], clnt, priv.link);
  }
}

/*===========================================================================
    FUNCTION  qmi_cci_client_free
  ===========================================================================*/
/*!
 * @brief
 *
 *   Frees a client handle
 *
 *   @return
 *
 *   None
 *
 *   @note
 *   The client handle will be unlinked if required.
 */
/*=========================================================================*/
static void qmi_cci_client_free(qmi_cci_client_type *clnt)
{
    struct qmi_cci_xport_s *xport;

   /* Unlink just to be sure, in most cases this should do nothing
    * as qmi_cci_get_ref(handle, 1) would have removed this
    * from the list */
    LOCK(&qcci_tbl_lock);
    qmi_cci_client_unlink(clnt);
    UNLOCK(&qcci_tbl_lock);

    QMI_CCI_OS_SIGNAL_DEINIT(&clnt->signal);
    if(clnt->ext_signal)
    {
      QMI_CCI_OS_SIGNAL_DEINIT(clnt->ext_signal);
    }
    LOCK_DEINIT(&clnt->priv.ref_count_lock);
    LOCK_DEINIT(&clnt->info_lock);
    LOCK_DEINIT(&clnt->txn_list_lock);
    LOCK_DEINIT(&clnt->tx_q_lock);
    LOCK_DEINIT(&clnt->server_addr_valid_lock);

    if(clnt->release_cb)
    {
      clnt->release_cb(clnt->release_cb_data);
    }

    /* Now it is safe to modify the xport list */
    while(NULL != (xport = LIST_HEAD(clnt->xport)))
    {
      LIST_REMOVE(clnt->xport, xport, link);
      FREE(xport);
    }
    FREE(clnt);
}

/*===========================================================================
 *   FUNCTION  qmi_cci_get_ref
 *   ===========================================================================*/
/*!
 * @brief
 *
 *   Gets a reference on the client. If the unlink parameter is set,
 *   then the client handle will be removed from the global table
 *   so future calls to qmi_cci_get_ref() will fail.
 *
 *   @return
 *
 *   Pointer to the client handle if successful. NULL otherwise.
 *
 *   @note
 *   The client's reference must be released when one is done with it by
 *   calling qmi_cci_put_ref
 */
/*=========================================================================*/
static qmi_cci_client_type *qmi_cci_get_ref
(
  qmi_client_type client_handle,
  int unlink
)
{
  uint32_t clid = CAST_HANDLE_TO_CLID(client_handle);
  qmi_cci_client_type *clnt;

  LOCK(&qcci_tbl_lock);
  clnt = qmi_cci_client_lookup(clid);
  if(!clnt)
  {
    UNLOCK(&qcci_tbl_lock);
    return NULL;
  }
  /* This implies that no one shall ever try to acquire qcci_tbl_lock
   * while holding ref_count_lock of any client */

  LOCK(&clnt->priv.ref_count_lock);
  if(clnt->priv.ref_count < 0)
  {
    UNLOCK(&clnt->priv.ref_count_lock);
    UNLOCK(&qcci_tbl_lock);
    return NULL;
  }
  clnt->priv.ref_count++;
  UNLOCK(&clnt->priv.ref_count_lock);

  /* Unlink the client from the global table so future get_ref's fail */
  if(unlink)
    qmi_cci_client_unlink(clnt);
  UNLOCK(&qcci_tbl_lock);
  return clnt;
}

/*===========================================================================
    FUNCTION  qmi_cci_put_ref
 ===========================================================================*/
/*!
 * @brief
 *
 *   Releases a reference on the client.
 *
 *   @return
 *
 *  Current reference count after releasing one's reference.
 *
 *  @note
 *  The client structure must NOT be accessed after calling this function.
 */
/*=========================================================================*/
static int qmi_cci_put_ref(qmi_cci_client_type *clnt)
{
  int ref = 0;

  if(!clnt)
    return 0;

  LOCK(&clnt->priv.ref_count_lock);
  ref = --clnt->priv.ref_count;
  UNLOCK(&clnt->priv.ref_count_lock);

  if(ref == 0)
  {
    qmi_cci_client_free(clnt);
  }
  return ref;
}

/*===========================================================================
  FUNCTION  remove_txn
===========================================================================*/
/*!
@brief

  Find, remove a transaction from client's transaction list and release
  the list's reference count on the txn if found.

@return
  None

@note
  Caller must have a reference to the client structure.
*/
/*=========================================================================*/
static qmi_client_error_type remove_txn
(
  qmi_cci_client_type *clnt,
  qmi_cci_txn_type *txn
)
{
  qmi_cci_txn_type *i;
  qmi_client_error_type rc = QMI_INTERNAL_ERR;

  if(!clnt || !txn)
    return rc;

  LOCK(&clnt->txn_list_lock);
  LIST_FIND(clnt->txn_list, i, link, i == txn);
  if(i)
  {
    LIST_REMOVE(clnt->txn_list, i, link);
    QMI_CCI_DEC_TXN(i);
    rc = QMI_NO_ERR;
  }
  UNLOCK(&clnt->txn_list_lock);
  return rc;
}

/*===========================================================================
  FUNCTION  get_dest_addr
===========================================================================*/
/*!
@brief

  Gets the destination address of the service.

@return

  QMI_NO_ERR if server exists and is valid (addr will be populated with the
             correct address)
  QMI_INTERNAL_ERR - Invalid parameters
  QMI_SERVICE_ERR - Address to the service is not valid (Service does not
  exist)

@note
  The caller must have taken a reference count.
*/
/*=========================================================================*/
static qmi_client_error_type get_dest_addr(qmi_cci_client_type *clnt, uint8_t *addr)
{
  if(!clnt || !addr)
    return QMI_INTERNAL_ERR;
  LOCK(&clnt->server_addr_valid_lock);
  if(!clnt->server_addr_valid)
  {
    UNLOCK(&clnt->server_addr_valid_lock);
    return QMI_SERVICE_ERR;
  }
  memcpy(addr, clnt->server_addr, MAX_ADDR_LEN);
  UNLOCK(&clnt->server_addr_valid_lock);
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_client_get_txn
===========================================================================*/
/*!
@brief

  Create a transaction and return the handle

@return
  qmi_client_error_type

@note
  The transaction is added to the client's outstanding transaction list
*/
/*=========================================================================*/
static qmi_client_error_type qmi_client_get_txn
(
 qmi_cci_client_type              *clnt,
 qmi_cci_txn_enum_type             type,
 unsigned int                      msg_id,
 void                              *resp_buf,
 unsigned int                      resp_buf_len,
 qmi_client_recv_raw_msg_async_cb  raw_async_rx_cb,
 qmi_client_recv_msg_async_cb      msg_async_rx_cb,
 void                              *rx_cb_data,
 qmi_cci_txn_type                  **txn_handle
 )
{
  qmi_cci_txn_type *txn;

  txn = (qmi_cci_txn_type *)CALLOC(1, sizeof(qmi_cci_txn_type));
  if(!txn)
    return QMI_INTERNAL_ERR;

  LINK_INIT(txn->link);
  LINK_INIT(txn->tx_link);
  LOCK(&clnt->txn_list_lock);
  txn->type = type;
  txn->msg_id = msg_id;
  while((txn->txn_id = clnt->next_txn_id++) == 0);
  txn->raw_async_rx_cb = raw_async_rx_cb;
  txn->msg_async_rx_cb = msg_async_rx_cb;
  txn->rx_cb_data = rx_cb_data;
  txn->rx_buf = resp_buf;
  txn->rx_buf_len = resp_buf_len;
  txn->tx_buf = NULL;
  txn->client = clnt;
  LOCK_INIT(&txn->lock);
  txn->ref_count = 1; /* The txn_list takes a reference.
                         The txn_list's reference will be
                         released only when it is removed
                         from the list */

  QMI_CCI_OS_SIGNAL_INIT_SELF(&txn->signal, &clnt->signal);
  LIST_ADD(clnt->txn_list, txn, link);
  UNLOCK(&clnt->txn_list_lock);

  *txn_handle = txn;
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  cleanup_client_lists
===========================================================================*/
/*!
@brief

  Cleans up all client lists.

@return
  None

@note
  The caller must have acquired a reference to the client structure.
*/
/*=========================================================================*/
static void cleanup_client_lists
(
  qmi_cci_client_type *clnt,
  int error
)
{
  qmi_cci_txn_type *txn;

  LOCK(&clnt->tx_q_lock);
  txn = LIST_HEAD(clnt->tx_q);
  clnt->tx_q.head = clnt->tx_q.tail = NULL;
  clnt->tx_q.count = 0;
  UNLOCK(&clnt->tx_q_lock);
  while(txn)
  {
    qmi_cci_txn_type *to_free = txn;
    txn = txn->tx_link.next;
    /* No need for lock as it is no longer in the list */
    INVALIDATE_TXN_TX_BUF(to_free);
    /* Give up tx_q list reference */
    QMI_CCI_DEC_TXN_SAFE(clnt, to_free);
  }

  LOCK(&clnt->txn_list_lock);
  /* The refcount for the txn_list gets transferred
   * into handle_txn_error */
  txn = LIST_HEAD(clnt->txn_list);
  clnt->txn_list.head = clnt->txn_list.tail = NULL;
  clnt->txn_list.count = 0;
  UNLOCK(&clnt->txn_list_lock);

  /* Handle error on each txn */
  while(txn)
  {
    qmi_cci_txn_type *to_free = txn;
    txn = txn->link.next;

    /* handle transaction error base on its type */
    handle_txn_error(clnt, to_free, error);
    INVALIDATE_TXN_RX_BUF_SAFE(to_free);
    QMI_CCI_DEC_TXN_SAFE(clnt, to_free);
  }
}

/*===========================================================================
  FUNCTION  qmi_cci_flush_tx_q
===========================================================================*/
/*!
@brief

  Tries and transmits all pending transactions in the tx queue.

@return
  None

@note
  The caller must have acquired a reference to the client structure.
*/
/*=========================================================================*/
void qmi_cci_flush_tx_q
(
  qmi_cci_client_type *clnt
)
{
  int rc;
  qmi_cci_txn_type *txn;
  uint8_t dest_addr[MAX_ADDR_LEN];

  rc = get_dest_addr(clnt, dest_addr);
  if(rc != QMI_NO_ERR)
  {
    cleanup_client_lists(clnt, QMI_SERVICE_ERR);
    return;
  }

  LOCK(&clnt->tx_q_lock);
  while(LIST_CNT(clnt->tx_q) > 0 && NULL != (txn = LIST_HEAD(clnt->tx_q)))
  {
    struct qmi_cci_xport_s *xport = LIST_HEAD(clnt->xport);

    if(clnt->accepting_txns && xport->handle &&
        xport->ops && txn->tx_buf)
    {
      /* Implies that xport cannot do up calls in send. If it does,
       * the up call should not try to acquire the tx_q_lock */
      rc = xport->ops->send(xport->handle, dest_addr,
          txn->tx_buf, txn->tx_buf_len);
    }
    else
    {
      rc = QMI_INTERNAL_ERR;
    }

    /* xport is flow controlled, try again later */
    if(rc == QMI_XPORT_BUSY_ERR)
      break;

    LIST_REMOVE(clnt->tx_q, txn, tx_link);
    INVALIDATE_TXN_TX_BUF(txn);
    UNLOCK(&clnt->tx_q_lock);

    /* Release tx_q ref count */
    QMI_CCI_DEC_TXN_SAFE(clnt, txn);

    /* Error sending txn */
    if(rc != QMI_NO_ERR)
    {
      qmi_cci_txn_type *to_find = txn;

      LOCK(&clnt->txn_list_lock);
      LIST_FIND(clnt->txn_list, txn, link, txn == to_find);
      if(txn)
      {
          LIST_REMOVE(clnt->txn_list, txn, link);
        /* Txn_list's ref count is transferred
         * into handle_txn_error */
        UNLOCK(&clnt->txn_list_lock);
        handle_txn_error(clnt, txn, rc);
        QMI_CCI_DEC_TXN_SAFE(clnt, txn);
      }
      else
      {
        UNLOCK(&clnt->txn_list_lock);
      }
      if (rc == QMI_SERVICE_ERR)
      {
	qmi_cci_xport_event_server_error(clnt, dest_addr, QMI_SERVICE_ERR);
      }
    }
    /* Lock and check the tx Q again */
    LOCK(&clnt->tx_q_lock);
  }
  UNLOCK(&clnt->tx_q_lock);
}

/*===========================================================================
  FUNCTION  qmi_cci_send
===========================================================================*/
/*!
@brief

  Transmit a message

@return
  None

@note
  The caller must have obtained a reference to the client handle.
  The caller should free the buffer only if this function returns error.
  The caller should provide buffers which are allocated on the heap only.
*/
/*=========================================================================*/
static qmi_client_error_type qmi_cci_send
(
  qmi_cci_client_type *clnt,
  qmi_cci_txn_type *txn,
  void *msg,
  uint32_t len
)
{
  if(!txn || !len || !msg)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Check server addr validity once before sending. */
  LOCK(&clnt->server_addr_valid_lock);
  if(!clnt->server_addr_valid)
  {
    UNLOCK(&clnt->server_addr_valid_lock);
    return QMI_SERVICE_ERR;
  }
  UNLOCK(&clnt->server_addr_valid_lock);

  /* Take a reference for the tx queue */
  QMI_CCI_INC_TXN_SAFE(clnt, txn);

  LOCK(&clnt->tx_q_lock);
  if(!clnt->accepting_txns)
  {
    UNLOCK(&clnt->tx_q_lock);
    /* Give up reference */
    QMI_CCI_DEC_TXN_SAFE(clnt, txn);
    return QMI_INTERNAL_ERR;
  }
  txn->tx_buf = msg;
  txn->tx_buf_len = len;
  LIST_ADD(clnt->tx_q, txn, tx_link);
  UNLOCK(&clnt->tx_q_lock);

  qmi_cci_flush_tx_q(clnt);

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  encode_and_send
===========================================================================*/
/*!
@brief

  Encode and send a message to the client

@return
  qmi_client_error_type

@note
*/
/*=========================================================================*/
static qmi_client_error_type encode_and_send
(
 qmi_cci_client_type *clnt,
 qmi_idl_type_of_message_type msg_type,
 qmi_cci_txn_type *txn,
 uint16_t msg_id,
 void *c_struct,
 int c_struct_len
)
{
  int rc;
  uint32_t max_msg_len;
  uint32_t out_len, idl_c_struct_len;
  unsigned char *msg;
  uint8_t cntl_flag;
  uint16_t txn_id = txn ? txn->txn_id : 0;

  rc = qmi_idl_get_message_c_struct_len(clnt->service_obj, msg_type, msg_id,
      &idl_c_struct_len);
  if(rc != QMI_IDL_LIB_NO_ERR)
      return rc;

/* Allow users to pass c_stuct_len == 0. This is useful in cases when the c
   structure has only optional members(thus idl_c_stuct_len would be non-zero)
   and the user requires to send the message with all options turned off */
  if(c_struct_len != 0 && c_struct_len != (int)idl_c_struct_len)
    return QMI_INTERNAL_ERR;

  if(c_struct && c_struct_len)
  {
    rc = qmi_idl_get_max_message_len(clnt->service_obj, msg_type, msg_id,
                &max_msg_len);
    if(rc != QMI_IDL_LIB_NO_ERR)
      return rc;

    msg = (unsigned char *)MALLOC(max_msg_len + QMI_HEADER_SIZE);
    if(!msg)
      return QMI_INTERNAL_ERR;

    rc = qmi_idl_message_encode(
        clnt->service_obj,
        msg_type,
        msg_id,
        c_struct,
        c_struct_len,
        msg + QMI_HEADER_SIZE,
        max_msg_len,
        (uint32_t *)&out_len);

    if(rc != QMI_IDL_LIB_NO_ERR)
    {
      FREE(msg);
      return rc;
    }
  }
  else
  {
    /* Empty message */
    out_len = 0;
    msg = (unsigned char *)MALLOC(QMI_HEADER_SIZE);
    if(!msg)
      return QMI_INTERNAL_ERR;
  }

  switch(msg_type)
  {
    case QMI_IDL_INDICATION:
      cntl_flag = QMI_INDICATION_CONTROL_FLAG;
      break;
    case QMI_IDL_RESPONSE:
      cntl_flag = QMI_RESPONSE_CONTROL_FLAG;
      break;
    default:
      cntl_flag = QMI_REQUEST_CONTROL_FLAG;
      break;
  }

  /* fill in header */
  encode_header(msg, cntl_flag, txn_id, msg_id, (uint16_t)out_len);

  QMI_CCI_OS_LOG_TX(msg);

  out_len += QMI_HEADER_SIZE;

  rc = qmi_cci_send(clnt, txn, msg, out_len);
  if(rc != QMI_NO_ERR)
  {
    FREE(msg);
  }

  return rc;
}

/*===========================================================================
  FUNCTION  handle_txn_error
===========================================================================*/
/*!
@brief
  Handle transaction error base on its type and set return code to error

@note
  Transaction is freed in the async case. The thread waiting on a sync response
  will free the transation after waking up.
*/
/*=========================================================================*/
static void handle_txn_error
(
 qmi_cci_client_type *clnt,
 qmi_cci_txn_type *txn,
 int error
 )
{
  if (!txn)
    return;

  txn->rc = error;

  switch(txn->type)
  {
    case TXN_SYNC_MSG:
    case TXN_SYNC_RAW:
      /* txn freed by the waiting function */
      QMI_CCI_OS_SIGNAL_SET(&txn->signal);
      break;

    case TXN_ASYNC_MSG:
      if(txn->msg_async_rx_cb)
      {
        txn->msg_async_rx_cb(CLIENT_HANDLE(clnt), txn->msg_id, txn->rx_buf,
            0, txn->rx_cb_data, txn->rc);
      }
      break;

    case TXN_ASYNC_RAW:
      if(txn->raw_async_rx_cb)
        txn->raw_async_rx_cb(CLIENT_HANDLE(clnt), txn->msg_id, txn->rx_buf,
            0, txn->rx_cb_data, txn->rc);
      break;
    default:
      break;
  }
}

/*===========================================================================
 *   FUNCTION  qmi_cci_response_wait_loop
===========================================================================*/
/*!
 * @brief
 *
 * wait for a transaction response message.
 *
 * @return
 *  error code
 *
 * @note
 * The caller is required to hold a reference to the client structure.
 */
/*=========================================================================*/
static qmi_client_error_type qmi_cci_response_wait_loop
(
  qmi_cci_client_type *clnt,
  qmi_cci_txn_type *txn,
  unsigned int timeout_msecs
)
{
  qmi_client_error_type ret = QMI_NO_ERR;

  if(!clnt)
    return QMI_INTERNAL_ERR;

  do
  {
    QMI_CCI_OS_SIGNAL_WAIT(&txn->signal, timeout_msecs);
    QMI_CCI_OS_SIGNAL_CLEAR(&txn->signal);

    if(QMI_CCI_OS_SIGNAL_TIMED_OUT(&txn->signal))
    {
      ret = QMI_TIMEOUT_ERR;
      break;
    }

    /* Not a stray wake-up break out */
    if(txn->rc != QMI_TIMEOUT_ERR)
    {
      ret = txn->rc;
      break;
    }
  } while(1);

  return ret;
}

/*===========================================================================
  FUNCTION  release_cb_internal
===========================================================================*/
/*!
@brief

  Internal callback function used by qmi_client_release()

@return
  None

@note
  unblocks qmi_client_release()
*/
/*=========================================================================*/
static void release_cb_internal(void *cb_data)
{
  QMI_CCI_OS_SIGNAL *signal = (QMI_CCI_OS_SIGNAL *)cb_data;
  if(signal)
  {
    QMI_CCI_OS_SIGNAL_SET(signal);
  }
}



/*===========================================================================
                   EXPORTED FUNCTIONS RELATING TO XPORT
===========================================================================*/

/*===========================================================================
  FUNCTION  qmi_cci_xport_closed
===========================================================================*/
void qmi_cci_xport_closed
(
 qmi_cci_client_type *clnt
 )
{
  /* Release the reference taken for this transport */
  (void)qmi_cci_put_ref(clnt);
}

/*===========================================================================
  FUNCTION  qmi_cci_xport_event_new_server
===========================================================================*/
void qmi_cci_xport_event_new_server
(
 qmi_cci_client_type *clnt,
 void *addr
 )
{
  qmi_client_notify_cb notify_cb = NULL;
  void *cb_data;

  if(!clnt || !clnt->ext_signal)
    return;
  LOCK(&clnt->info_lock);
  notify_cb = clnt->info.notifier.notify_cb;
  cb_data   = clnt->info.notifier.notify_cb_data;
  if(!notify_cb)
    clnt->info.notifier.notify_pending = 1;
  UNLOCK(&clnt->info_lock);

  QMI_CCI_OS_SIGNAL_SET(clnt->ext_signal);

  if(notify_cb)
  {
    notify_cb(CLIENT_HANDLE(clnt), clnt->service_obj, QMI_CLIENT_SERVICE_COUNT_INC, cb_data);
  }
}

/*===========================================================================
  FUNCTION  qmi_cci_xport_event_server_error
===========================================================================*/
void qmi_cci_xport_event_server_error
(
 qmi_cci_client_type *clnt,
 void *addr,
 int error
 )
{
  qmi_client_error_cb err_cb = NULL;
  qmi_client_notify_cb notify_cb = NULL;
  void *cb_data;

  if(!clnt)
    return;

  /* signal notifier of the event */
  if(clnt->ext_signal)
  {
    QMI_CCI_OS_SIGNAL_SET(clnt->ext_signal);
  }
  else
  {
    /* This is a connected client and the service is
     * not the one we are connected to */
    if(memcmp(addr, clnt->server_addr, LIST_HEAD(clnt->xport)->addr_len))
    {
      return;
    }
  }

  /* zero out the server's address so no future transaction can occur */
  LOCK(&clnt->server_addr_valid_lock);
  clnt->server_addr_valid = 0;
  memset(clnt->server_addr, 0, LIST_HEAD(clnt->xport)->addr_len);
  UNLOCK(&clnt->server_addr_valid_lock);

  cleanup_client_lists(clnt, error);

  /* notify error callback */
  LOCK(&clnt->info_lock);
  if(clnt->ext_signal)
  {
    notify_cb = clnt->info.notifier.notify_cb;
    cb_data = clnt->info.notifier.notify_cb_data;
  }
  else
  {
    err_cb = clnt->info.client.err_cb;
    cb_data = clnt->info.client.err_cb_data;
    if (!err_cb)
      clnt->info.client.err_pending = 1;
  }
  UNLOCK(&clnt->info_lock);

  if(err_cb)
  {
    err_cb(CLIENT_HANDLE(clnt), error, cb_data);
  }
  else if(notify_cb)
  {
    notify_cb(CLIENT_HANDLE(clnt), clnt->service_obj,
                            QMI_CLIENT_SERVICE_COUNT_DEC, cb_data);
  }
}

/*===========================================================================
  FUNCTION  qmi_cci_xport_event_remove_server
===========================================================================*/
void qmi_cci_xport_event_remove_server
(
 qmi_cci_client_type *clnt,
 void *addr
 )
{
  qmi_cci_xport_event_server_error(clnt, addr, QMI_SERVICE_ERR);
}

/*=============================================================================
  FUNCTION qmi_cci_xport_resume
=============================================================================*/
void qmi_cci_xport_resume
(
 qmi_cci_client_type *clnt
)
{

  if(!clnt)
    return;

  /* xport already has a ref count, we dont need one */
  qmi_cci_flush_tx_q(clnt);
}

/*===========================================================================
  FUNCTION  qmi_cci_xport_recv
===========================================================================*/
qmi_client_error_type qmi_cci_xport_recv
(
 qmi_cci_client_type *clnt,
 void *addr,
 uint8_t *buf,
 uint32_t len
 )
{
  uint8_t cntl_flag;
  uint16_t txn_id, msg_id, msg_len;
  qmi_cci_txn_type *txn;

  if(!clnt || len < QMI_HEADER_SIZE)
    return QMI_INTERNAL_ERR;

  QMI_CCI_OS_LOG_RX(buf);

  /* decode message header and find the transaction */
  decode_header(buf, &cntl_flag, &txn_id, &msg_id, &msg_len);
  buf += QMI_HEADER_SIZE;
  len -= QMI_HEADER_SIZE;

  ASSERT(msg_len == len);

  if(cntl_flag != QMI_RESPONSE_CONTROL_FLAG &&
      cntl_flag != QMI_INDICATION_CONTROL_FLAG)
    return QMI_INTERNAL_ERR;

  /* process indication */
  if(cntl_flag == QMI_INDICATION_CONTROL_FLAG)
  {
    if(clnt->ind_cb)
    {
      clnt->ind_cb(CLIENT_HANDLE(clnt), msg_id, msg_len ? buf : NULL, msg_len,
          clnt->ind_cb_data);
    }
    return QMI_NO_ERR;
  }

  /* process transaction */
  LOCK(&clnt->txn_list_lock);
  LIST_FIND(clnt->txn_list, txn, link, txn->txn_id == txn_id);
  if(txn)
    {
    /* Txn list lock's reference is transferred to xport_recv */
      LIST_REMOVE(clnt->txn_list, txn, link);
    }
  UNLOCK(&clnt->txn_list_lock);

  /* transaction not found */
  if(!txn)
    return QMI_INTERNAL_ERR;

  /* mismatched msg_id, something went wrong - bail */
  if(txn->msg_id != msg_id)
  {
    handle_txn_error(clnt, txn, QMI_INVALID_TXN);
    QMI_CCI_DEC_TXN_SAFE(clnt, txn);
    return QMI_INTERNAL_ERR;
  }

  LOCK(&txn->lock);
  if(txn->rx_buf)
  {
    switch(txn->type)
    {
      case TXN_SYNC_MSG:
        txn->rc = qmi_idl_message_decode(
          clnt->service_obj,
          QMI_IDL_RESPONSE,
          msg_id,
          buf,
          len,
          txn->rx_buf,
          txn->rx_buf_len
          );
        QMI_CCI_OS_SIGNAL_SET(&txn->signal);
        break;

      case TXN_SYNC_RAW:
        memcpy(txn->rx_buf, buf, QCCI_MIN(len, txn->rx_buf_len));
        txn->reply_len = QCCI_MIN(len, txn->rx_buf_len);
        txn->rc = QMI_NO_ERR;
        QMI_CCI_OS_SIGNAL_SET(&txn->signal);
        break;

      case TXN_ASYNC_MSG:
        if(txn->msg_async_rx_cb)
        {
          txn->rc = qmi_idl_message_decode(
            clnt->service_obj,
            QMI_IDL_RESPONSE,
            msg_id,
            buf,
            len,
            txn->rx_buf,
            txn->rx_buf_len
            );
          txn->msg_async_rx_cb(CLIENT_HANDLE(clnt), msg_id, txn->rx_buf,
            txn->rx_buf_len, txn->rx_cb_data, txn->rc);
        }
        break;

      case TXN_ASYNC_RAW:
        memcpy(txn->rx_buf, buf, QCCI_MIN(len, txn->rx_buf_len));
        if(txn->raw_async_rx_cb)
          txn->raw_async_rx_cb(CLIENT_HANDLE(clnt), msg_id, txn->rx_buf,
              QCCI_MIN(len, txn->rx_buf_len), txn->rx_cb_data, QMI_NO_ERR);
        break;
      default:
        break;
    }
  }

  UNLOCK(&txn->lock);

  QMI_CCI_DEC_TXN_SAFE(clnt, txn);

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_cci_xport_start
===========================================================================*/
void qmi_cci_xport_start
(
 qmi_cci_xport_ops_type *ops,
 void *xport_data
 )
{
  unsigned int i;

  if(!ops)
  {
    return;
  }

  LOCK(&xport_tbl_lock);
  for (i = 0; i < num_xports; i++)
  {
    if ((xport_tbl[i].ops == ops) && (xport_tbl[i].xport_data == xport_data))
    {
      UNLOCK(&xport_tbl_lock);
      return;
    }
  }

  if(num_xports < MAX_XPORTS)
  {
    xport_tbl[num_xports].ops = ops;
    xport_tbl[num_xports].xport_data = xport_data;
    xport_tbl[num_xports].uid = next_xport_uid++;
    num_xports++;
  }
  UNLOCK(&xport_tbl_lock);
}

/*===========================================================================
  FUNCTION  qmi_cci_xport_stop
===========================================================================*/
void qmi_cci_xport_stop
(
 qmi_cci_xport_ops_type *ops,
 void *xport_data
 )
{
  unsigned int i, j;

  LOCK(&xport_tbl_lock);
  for(i = 0; i < num_xports; i++)
  {
    if(xport_tbl[i].ops == ops && xport_tbl[i].xport_data == xport_data)
    {
      /* Left shift all the xports */
      for(j = i; j < num_xports - 1; j++)
      {
        xport_tbl[j].ops = xport_tbl[j + 1].ops;
        xport_tbl[j].xport_data = xport_tbl[j + 1].xport_data;
      }
      num_xports--;
      break;
    }
  }
  UNLOCK(&xport_tbl_lock);
}

/*===========================================================================
                   PUBLIC EXPORTED FUNCTIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qmi_client_notifier_init
===========================================================================*/
qmi_client_error_type
qmi_client_notifier_init
(
 qmi_idl_service_object_type               service_obj,
 qmi_client_os_params                      *os_params,
 qmi_client_type                           *user_handle
 )
{
  qmi_cci_client_type *clnt;
  qmi_client_type ret_user_handle;
  uint32_t service_id, idl_version, i;
  int rc;

  if(!user_handle)
    return QMI_INTERNAL_ERR;

  *user_handle = INVALID_HANDLE;

  if (!service_obj || !os_params)
    return QMI_INTERNAL_ERR;

  /* Extract service id */
  rc =  qmi_idl_get_service_id(service_obj, &service_id);
  if (rc !=  QMI_IDL_LIB_NO_ERR)
    return QMI_INTERNAL_ERR;

  /* Get IDL version */
  rc =  qmi_idl_get_idl_version(service_obj, &idl_version);
  if (rc !=  QMI_IDL_LIB_NO_ERR)
    return QMI_INTERNAL_ERR;

  clnt = qmi_cci_client_alloc(service_obj, os_params, os_params, NULL, NULL);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  ret_user_handle = CLIENT_HANDLE(clnt);
  rc = QMI_INTERNAL_ERR;

  LOCK(&xport_tbl_lock);
  /* go through all the xports and find the service */
  for(i = 0; i < num_xports; i++)
  {
     struct qmi_cci_xport_s *xport;

     xport = CALLOC(1, sizeof(*xport));
     if(!xport)
     {
       continue;
     }

     clnt = qmi_cci_get_ref(ret_user_handle, 0);
     if(!clnt)
     {
       FREE(xport);
       return QMI_INTERNAL_ERR; /* Should never happen */
     }
     /* opening up the xport */
     LINK_INIT(xport->link);
     xport->ops = xport_tbl[i].ops;
     xport->addr_len = QCCI_MIN(MAX_ADDR_LEN, xport->ops->addr_len());
     xport->handle = xport->ops->open(xport_tbl[i].xport_data,
        clnt, service_id, idl_version, NULL, 0);
    /* unable to open xport, go to the next one */
    if(!xport->handle)
    {
      (void)qmi_cci_put_ref(clnt);
      FREE(xport);
      continue;
    }
    LIST_ADD(clnt->xport, xport, link);

    /* if server exists, signal notifier */
    if(xport->ops->lookup(xport_tbl[i].xport_data, (uint8_t)xport_tbl[i].uid, service_id,
        idl_version, NULL, NULL))
    {
      QMI_CCI_OS_SIGNAL_SET(clnt->ext_signal);
      clnt->info.notifier.notify_pending = 1;
    }

    rc = QMI_NO_ERR;
  }
  UNLOCK(&xport_tbl_lock);

  if(rc == QMI_NO_ERR)
  {
    *user_handle = ret_user_handle;
  }
  qmi_cci_put_ref(clnt);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_client_init
===========================================================================*/
qmi_client_error_type
qmi_client_init
(
 qmi_service_info                          *service_info,
 qmi_idl_service_object_type               service_obj,
 qmi_client_ind_cb                         ind_cb,
 void                                      *ind_cb_data,
 qmi_client_os_params                      *os_params,
 qmi_client_type                           *user_handle
 )
{
  qmi_cci_service_info *svc = (qmi_cci_service_info *)service_info;
  qmi_cci_client_type *clnt;
  struct qmi_cci_xport_s *xport;
  qmi_client_type ret_user_handle;
  uint32_t service_id, idl_version;
  uint32_t max_msg_len, i;
  int rc;
  unsigned int num_entries = 0, num_services;
  qmi_service_info *service_array = NULL;
  void *xport_data;

  if(!svc || !user_handle)
    return QMI_INTERNAL_ERR;

  *user_handle = INVALID_HANDLE;

  LOCK(&xport_tbl_lock);
  for(i = 0; i < num_xports; i++)
  {
    if(xport_tbl[i].uid == svc->xport)
    {
      break;
    }
  }
  if(i == num_xports)
  {
    UNLOCK(&xport_tbl_lock);
    return QMI_INTERNAL_ERR;
  }
  xport = CALLOC(1, sizeof(*xport));
  if(!xport)
  {
    UNLOCK(&xport_tbl_lock);
    return QMI_INTERNAL_ERR;
  }
  xport_data = xport_tbl[i].xport_data;
  xport->ops = xport_tbl[i].ops;
  xport->addr_len = QCCI_MIN(MAX_ADDR_LEN, xport->ops->addr_len());
  xport->handle = NULL;
  LINK_INIT(xport->link);
  UNLOCK(&xport_tbl_lock);

  /* Extract service id */
  if(qmi_idl_get_service_id(service_obj, &service_id) !=  QMI_IDL_LIB_NO_ERR ||
     qmi_idl_get_idl_version(service_obj, &idl_version) !=  QMI_IDL_LIB_NO_ERR ||
     qmi_idl_get_max_service_len(service_obj, &max_msg_len) != QMI_IDL_LIB_NO_ERR)
  {
    FREE(xport);
    return QMI_INTERNAL_ERR;
  }

  clnt = qmi_cci_client_alloc(service_obj, os_params, NULL, ind_cb, ind_cb_data);
  if(!clnt)
  {
    FREE(xport);
    return QMI_INTERNAL_ERR;
  }

  ret_user_handle = CLIENT_HANDLE(clnt);

  /* fill in server address */
  clnt->server_addr_valid = 1;
  memcpy(clnt->server_addr, svc->addr, xport->addr_len);

 clnt = qmi_cci_get_ref(ret_user_handle, 0);
  if(!clnt)
  {
    FREE(xport);
    return QMI_INTERNAL_ERR; /* Should never be here */
  }

  xport->handle = xport->ops->open(xport_data,
                          clnt, service_id, idl_version, clnt->server_addr, max_msg_len);
  /* unable to open xport, bail */
  if(!xport->handle)
  {
    /* Release the xport reference */
    FREE(xport);
    (void)qmi_cci_put_ref(clnt);
    rc = QMI_INTERNAL_ERR;
    goto release_init_bail;
  }
  LIST_ADD(clnt->xport, xport, link);

  /* redo lookup to make sure the server actually exits */
  while(1)
  {
    rc = qmi_client_get_service_list(service_obj, service_array, &num_entries,
        &num_services);

    if(rc != QMI_NO_ERR)
    {
      if(service_array)
      {
        FREE(service_array);
        service_array = NULL;
      }
      break;
    }

    if(num_entries == num_services)
      break;

    if(service_array)
      FREE(service_array);

    service_array = (qmi_service_info *)MALLOC(sizeof(qmi_service_info) * num_services);
    if(!service_array)
    {
      rc = QMI_INTERNAL_ERR;
      goto init_close_bail;
    }

    num_entries = num_services;
  }
  rc = QMI_SERVICE_ERR;
  if(service_array)
  {
    for(i = 0; i < num_entries; i++)
    {
      qmi_cci_service_info *s = (qmi_cci_service_info *)&service_array[i];
      if(!memcmp(s->addr, svc->addr, xport->addr_len))
      {
        *user_handle = ret_user_handle;
        rc = QMI_NO_ERR;
        break;
      }
    }
    FREE(service_array);
  }

init_close_bail:
  if(rc != QMI_NO_ERR)
  {
    /* Freed when the reference is released */
    xport->ops->close(xport->handle);
  }
release_init_bail:
  (void)qmi_cci_put_ref(clnt);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_client_send_raw_msg_async
===========================================================================*/
qmi_client_error_type
qmi_client_send_raw_msg_async
(
 qmi_client_type                   user_handle,
 unsigned int                      msg_id,
 void                              *req_buf,
 unsigned int                      req_buf_len,
 void                              *resp_buf,
 unsigned int                      resp_buf_len,
 qmi_client_recv_raw_msg_async_cb  resp_cb,
 void                              *resp_cb_data,
 qmi_txn_handle                    *txn_handle
 )
{
  qmi_cci_client_type *clnt;
  qmi_cci_txn_type *txn = NULL;
  unsigned char *buf;
  int rc;

  if(!resp_buf)
    return QMI_INTERNAL_ERR;

  if(req_buf_len > 0 && req_buf == NULL)
    return QMI_INTERNAL_ERR;

  if(txn_handle)
    *txn_handle = (qmi_txn_handle)NULL;

  clnt = qmi_cci_get_ref(user_handle, 0);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  if(!CLIENT_XPORT_VALID(clnt))
  {
     rc = QMI_INTERNAL_ERR;
     goto bail;
  }

  /* allocate output buffer */
  buf = (unsigned char *)MALLOC(req_buf_len + QMI_HEADER_SIZE);
  if(!buf)
  {
    rc = QMI_INTERNAL_ERR;
    goto bail;
  }

  rc = qmi_client_get_txn(clnt, TXN_ASYNC_RAW, msg_id, resp_buf, resp_buf_len,
      resp_cb, NULL, resp_cb_data, &txn);

  if(rc != QMI_NO_ERR)
  {
    FREE(buf);
    rc = QMI_INTERNAL_ERR;
    goto bail;
  }

  /* encode header and copy payload */
  encode_header(buf, QMI_REQUEST_CONTROL_FLAG, (uint16_t)txn->txn_id,
      (uint16_t)msg_id, (uint16_t)req_buf_len);

  if(req_buf_len > 0)
    memcpy(buf + QMI_HEADER_SIZE, req_buf, req_buf_len);

  QMI_CCI_OS_LOG_TX(buf);
  /* send message and return */
  rc = qmi_cci_send(clnt, txn, buf, req_buf_len + QMI_HEADER_SIZE);

  if(rc != QMI_NO_ERR)
  {
    FREE(buf);
    if(remove_txn(clnt, txn) != QMI_NO_ERR)
    {
      /* The transaction is already removed meaning
      * the resp cb has already been called */
      rc = QMI_NO_ERR;
      txn = NULL;
    }
  }
  if(rc == QMI_NO_ERR && txn_handle)
  {
    *txn_handle = (qmi_txn_handle)txn;
  }
bail:
   (void)qmi_cci_put_ref(clnt);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_client_send_msg_async
===========================================================================*/
qmi_client_error_type
qmi_client_send_msg_async
(
    qmi_client_type                 user_handle,
    unsigned int                    msg_id,
    void                            *req_c_struct,
    unsigned int                    req_c_struct_len,
    void                            *resp_c_struct,
    unsigned int                    resp_c_struct_len,
    qmi_client_recv_msg_async_cb    resp_cb,
    void                            *resp_cb_data,
    qmi_txn_handle                  *txn_handle
)
{
  qmi_cci_client_type *clnt;
  qmi_cci_txn_type *txn = NULL;
  int rc;

  if(!resp_c_struct)
    return QMI_INTERNAL_ERR;

  if(req_c_struct_len > 0 && req_c_struct == NULL)
    return QMI_INTERNAL_ERR;

  if(txn_handle)
    *txn_handle = (qmi_txn_handle)NULL;

  clnt = qmi_cci_get_ref(user_handle, 0);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  if(!CLIENT_XPORT_VALID(clnt))
  {
    rc = QMI_INTERNAL_ERR;
    goto bail;
  }

  rc = qmi_client_get_txn(clnt, TXN_ASYNC_MSG, msg_id, resp_c_struct,
      resp_c_struct_len, NULL, resp_cb, resp_cb_data, &txn);

  if(rc != QMI_NO_ERR)
  {
    rc = QMI_INTERNAL_ERR;
    goto bail;
  }

  /* encode message */
  rc = encode_and_send(clnt, QMI_IDL_REQUEST, txn, (uint16_t)msg_id,
      req_c_struct, req_c_struct_len);

  if(rc != QMI_NO_ERR)
  {
    if(remove_txn(clnt, txn) != QMI_NO_ERR)
    {
      /* The transaction is already removed meaning
       * the resp cb has already been called */
      rc = QMI_NO_ERR;
      txn = NULL;
    }
  }
  if(rc == QMI_NO_ERR && txn_handle)
  {
    *txn_handle = (qmi_txn_handle)txn;
  }
bail:
  (void)qmi_cci_put_ref(clnt);
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_client_delete_async_txn
===========================================================================*/
qmi_client_error_type
qmi_client_delete_async_txn
(
  qmi_client_type  user_handle,
  qmi_txn_handle   async_txn_handle
)
{
  qmi_cci_client_type *clnt;
  qmi_cci_txn_type *txn;
  qmi_cci_txn_type *find_txn = (qmi_cci_txn_type *)async_txn_handle;
  int rc = QMI_INVALID_TXN;

  if(!async_txn_handle)
    return QMI_INTERNAL_ERR;

  clnt = qmi_cci_get_ref(user_handle, 0);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  /* Find and remove txn from tx queue first */
  LOCK(&clnt->tx_q_lock);
  LIST_FIND(clnt->tx_q, txn, tx_link, txn == find_txn);
  if(txn)
  {
    INVALIDATE_TXN_TX_BUF(txn);
    LIST_REMOVE(clnt->tx_q, txn, tx_link);
    UNLOCK(&clnt->tx_q_lock);
    /* Release the tx_q ref count */
    QMI_CCI_DEC_TXN_SAFE(clnt, txn);
  }
  else
  {
    UNLOCK(&clnt->tx_q_lock);
  }

  /* lock list and look for txn */
  LOCK(&clnt->txn_list_lock);
  LIST_FIND(clnt->txn_list, txn, link, txn == find_txn);
  if(txn)
    {
      LIST_REMOVE(clnt->txn_list, txn, link);
  }
  UNLOCK(&clnt->txn_list_lock);

  if(txn)
  {
    INVALIDATE_TXN_RX_BUF_SAFE(txn);
    QMI_CCI_DEC_TXN_SAFE(clnt, txn);
    rc = QMI_NO_ERR;
  }

  (void)qmi_cci_put_ref(clnt);
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_client_send_raw_msg_sync
===========================================================================*/
qmi_client_error_type
qmi_client_send_raw_msg_sync
(
  qmi_client_type           user_handle,
  unsigned int              msg_id,
  void                      *req_buf,
  unsigned int              req_buf_len,
  void                      *resp_buf,
  unsigned int              resp_buf_len,
  unsigned int              *resp_buf_recv_len,
  unsigned int              timeout_msecs
)
{
  unsigned char *buf;
  qmi_cci_client_type *clnt;
  qmi_cci_txn_type *txn = NULL;
  int rc;

  if(!resp_buf || !resp_buf_recv_len)
    return QMI_INTERNAL_ERR;

  if(req_buf_len > 0 && req_buf == NULL)
    return QMI_INTERNAL_ERR;

  clnt = qmi_cci_get_ref(user_handle, 0);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  if(!CLIENT_XPORT_VALID(clnt))
  {
    rc = QMI_INTERNAL_ERR;
    goto bail;
  }

  /* allocate output buffer */
  buf = (unsigned char *)MALLOC(req_buf_len + QMI_HEADER_SIZE);
  if(!buf)
  {
    rc = QMI_INTERNAL_ERR;
    goto bail;
  }

  rc = qmi_client_get_txn(clnt, TXN_SYNC_RAW, msg_id, resp_buf, resp_buf_len,
      NULL, NULL, NULL, &txn);

  if(rc != QMI_NO_ERR)
  {
    FREE(buf);
    rc = QMI_INTERNAL_ERR;
    goto bail;
  }

  /* This function is going to maintain a reference */
  QMI_CCI_INC_TXN_SAFE(clnt, txn);

  /* encode header and copy payload */
  encode_header(buf, QMI_REQUEST_CONTROL_FLAG, (uint16_t)txn->txn_id,
      (uint16_t)msg_id, (uint16_t)req_buf_len);

  if(req_buf_len > 0)
  {
    memcpy(buf + QMI_HEADER_SIZE, req_buf, req_buf_len);
  }

  /* clear signal */
  QMI_CCI_OS_SIGNAL_CLEAR(&txn->signal);
  txn->rc = QMI_TIMEOUT_ERR;

  QMI_CCI_OS_LOG_TX(buf);

  /* send message */
  rc = qmi_cci_send(clnt, txn, buf, req_buf_len + QMI_HEADER_SIZE);

  if(rc == QMI_NO_ERR)
  {
    rc = qmi_cci_response_wait_loop(clnt, txn, timeout_msecs);
    if(rc == QMI_NO_ERR)
    {
      *resp_buf_recv_len = txn->reply_len;
    }
  }
  else
  {
    FREE(buf);
  }

  INVALIDATE_TXN_RX_BUF_SAFE(txn);
  (void)remove_txn(clnt, txn);
  QMI_CCI_DEC_TXN_SAFE(clnt, txn);

bail:
  (void)qmi_cci_put_ref(clnt);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_client_send_msg_sync
===========================================================================*/
qmi_client_error_type
qmi_client_send_msg_sync
(
    qmi_client_type    user_handle,
    unsigned int       msg_id,
    void               *req_c_struct,
    unsigned int       req_c_struct_len,
    void               *resp_c_struct,
    unsigned int       resp_c_struct_len,
    unsigned int       timeout_msecs
)
{
  qmi_cci_client_type *clnt;
  qmi_cci_txn_type *txn = NULL;
  int rc;

  if(!resp_c_struct)
    return QMI_INTERNAL_ERR;

  if(req_c_struct_len > 0 && req_c_struct == 0)
    return QMI_INTERNAL_ERR;

  clnt = qmi_cci_get_ref(user_handle, 0);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  if(!CLIENT_XPORT_VALID(clnt))
  {
    rc = QMI_INTERNAL_ERR;
    goto bail;
  }

  rc = qmi_client_get_txn(clnt, TXN_SYNC_MSG, msg_id, resp_c_struct,
      resp_c_struct_len, NULL, NULL, NULL, &txn);

  if(rc != QMI_NO_ERR)
  {
    rc = QMI_INTERNAL_ERR;
    goto bail;
  }

  QMI_CCI_INC_TXN_SAFE(clnt, txn);

  /* clear signal */
  QMI_CCI_OS_SIGNAL_CLEAR(&txn->signal);
  txn->rc = QMI_TIMEOUT_ERR;

  /* encode message */
  rc = encode_and_send(clnt, QMI_IDL_REQUEST, txn, (uint16_t)msg_id,
      req_c_struct, req_c_struct_len);

  if(rc == QMI_NO_ERR)
  {
    rc = qmi_cci_response_wait_loop(clnt, txn, timeout_msecs);
  }

  INVALIDATE_TXN_RX_BUF_SAFE(txn);
    (void)remove_txn(clnt, txn);
    QMI_CCI_DEC_TXN_SAFE(clnt, txn);

bail:
  (void)qmi_cci_put_ref(clnt);
    return rc;
}

/*===========================================================================
  FUNCTION  qmi_client_release_async
===========================================================================*/
qmi_client_error_type qmi_client_release_async
(
  qmi_client_type user_handle,
  qmi_client_release_cb release_cb,
  void *release_cb_data
)
{
  qmi_cci_client_type *clnt;
  struct qmi_cci_xport_s *xport;

  clnt = qmi_cci_get_ref(user_handle, 1);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  LOCK(&clnt->tx_q_lock);
  /* From now on all calls to qmi_cci_send will fail */
  clnt->accepting_txns = 0;
  UNLOCK(&clnt->tx_q_lock);

  cleanup_client_lists(clnt, QMI_INTERNAL_ERR);

  clnt->release_cb = release_cb;
  clnt->release_cb_data = release_cb_data;

  /* close each of the opened xports allowing each to free the
   * reference allocated to it. */
  for(xport = LIST_HEAD(clnt->xport); xport; xport = xport->link.next)
  {
    void *handle = xport->handle;
    /* From now, all down calls (initiated before release()) will fail */
    xport->handle = NULL;
    xport->ops->close(handle);
  }

  (void)qmi_cci_put_ref(clnt);

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_client_release
===========================================================================*/
qmi_client_error_type
qmi_client_release
(
  qmi_client_type     user_handle
)
{
  qmi_cci_client_type *clnt;
  qmi_client_error_type rc;
  QMI_CCI_OS_SIGNAL signal;

  clnt = qmi_cci_get_ref(user_handle, 0);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  QMI_CCI_OS_SIGNAL_INIT_SELF(&signal, &clnt->signal);
  QMI_CCI_OS_SIGNAL_CLEAR(&signal);

  /* Release this call's reference */
  (void)qmi_cci_put_ref(clnt);

  rc = qmi_client_release_async(user_handle, release_cb_internal, (void *)&signal);

  if(rc == QMI_NO_ERR)
  {
    QMI_CCI_OS_SIGNAL_WAIT(&signal, 0);
  }
  QMI_CCI_OS_SIGNAL_DEINIT(&signal);
  return rc;
}



/*===========================================================================
  FUNCTION  qmi_client_message_encode
===========================================================================*/
qmi_client_error_type
qmi_client_message_encode
(
    qmi_client_type                      user_handle,
    qmi_idl_type_of_message_type         req_resp_ind,
    unsigned int                         message_id,
    const void                           *p_src,
    unsigned int                         src_len,
    void                                 *p_dst,
    unsigned int                         dst_len,
    unsigned int                         *dst_encoded_len
)
{
  qmi_cci_client_type *clnt;
  qmi_idl_service_object_type service_obj;

  clnt = qmi_cci_get_ref(user_handle, 0);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  service_obj = clnt->service_obj;
  (void)qmi_cci_put_ref(clnt);

  return (
      qmi_idl_message_encode(
        service_obj,
        req_resp_ind,
        (uint16_t)message_id,
        p_src,
        src_len,
        p_dst,
        dst_len,
        (uint32_t*)dst_encoded_len)
      );
}

/*===========================================================================
  FUNCTION  qmi_client_message_decode
===========================================================================*/
qmi_client_error_type
qmi_client_message_decode
(
    qmi_client_type                         user_handle,
    qmi_idl_type_of_message_type            req_resp_ind,
    unsigned int                            message_id,
    const void                              *p_src,
    unsigned int                            src_len,
    void                                    *p_dst,
    unsigned int                            dst_len
)
{
  qmi_cci_client_type *clnt;
  qmi_idl_service_object_type service_obj;

  clnt = qmi_cci_get_ref(user_handle, 0);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  service_obj = clnt->service_obj;
  (void)qmi_cci_put_ref(clnt);

  return (
      qmi_idl_message_decode(
        service_obj,
        req_resp_ind,
        (uint16_t)message_id,
        p_src,
        src_len,
        p_dst,
        dst_len)
      );
}

/*===========================================================================
  FUNCTION  qmi_client_get_service_list
===========================================================================*/
qmi_client_error_type
qmi_client_get_service_list
(
 qmi_idl_service_object_type   service_obj,
 qmi_service_info       *service_info_array,
 unsigned int *num_entries,
 unsigned int *num_services
 )
{
  qmi_cci_service_info *svc = (qmi_cci_service_info *)service_info_array;
  unsigned int i, to_fill = 0, filled = 0;
  uint32_t service_id, idl_version;
  int rc;

  if(!num_services)
    return QMI_INTERNAL_ERR;

  *num_services = 0;

  if(num_entries && *num_entries && svc)
  {
    to_fill = *num_entries;
    *num_entries = 0;
  }

  /* Extract service id */
  rc =  qmi_idl_get_service_id(service_obj, &service_id);
  if (rc !=  QMI_IDL_LIB_NO_ERR)
    return QMI_INTERNAL_ERR;

  /* Get IDL version */
  rc =  qmi_idl_get_idl_version(service_obj, &idl_version);
  if (rc !=  QMI_IDL_LIB_NO_ERR)
    return QMI_INTERNAL_ERR;

  /* go through all the xports and find the service */
  LOCK(&xport_tbl_lock);
  for(i = 0; i < num_xports; i++)
  {
    /* look up server address */
    unsigned int entries = to_fill - filled;
    unsigned int service_xport;
    (*num_services) += service_xport = xport_tbl[i].ops->lookup(xport_tbl[i].xport_data,
        (uint8_t)xport_tbl[i].uid, service_id, idl_version, entries ? &entries : NULL,
        entries ? &svc[filled] : NULL);
    /* only increment if lookup returns non-zero */
    filled += service_xport ? entries : 0;
  }
  UNLOCK(&xport_tbl_lock);

  if (num_entries)
    (*num_entries) = filled;

  return *num_services ? QMI_NO_ERR : QMI_SERVICE_ERR;
}

#ifndef QMI_CLIENT_INSTANCE_ANY
#define QMI_CLIENT_INSTANCE_ANY 0xffff
#endif
/*===========================================================================
  FUNCTION  qmi_client_get_any_service
===========================================================================*/
qmi_client_error_type
qmi_client_get_any_service
(
 qmi_idl_service_object_type service_obj,
 qmi_service_info *service_info
 )
{
  return qmi_client_get_service_instance(service_obj, QMI_CLIENT_INSTANCE_ANY, service_info);
}

/*===========================================================================
  FUNCTION  qmi_client_get_service_instance
===========================================================================*/
qmi_client_error_type
qmi_client_get_service_instance
(
 qmi_idl_service_object_type service_obj,
 qmi_service_instance instance_id,
 qmi_service_info *service_info
 )
{
  unsigned int num_entries = 1, num_services, i;
  qmi_client_error_type rc;
  qmi_service_info *service_array;

  if(!service_info)
    return QMI_INTERNAL_ERR;

  if(instance_id == QMI_CLIENT_INSTANCE_ANY)
  {
    return qmi_client_get_service_list(service_obj, service_info, &num_entries, &num_services);
  }

  rc = qmi_client_get_service_list(service_obj, NULL, NULL, &num_services);
  if(rc != QMI_NO_ERR)
    return rc;

  service_array = (qmi_service_info *)MALLOC(sizeof(qmi_service_info) * num_services);
  if(!service_array)
    return QMI_INTERNAL_ERR;

  num_entries = num_services;
  rc = qmi_client_get_service_list(service_obj, service_array, &num_entries,
      &num_services);
  if(rc != QMI_NO_ERR)
    goto free_bail;

  rc = QMI_SERVICE_ERR;
  for(i = 0; i < num_entries; i++)
  {
    qmi_cci_service_info *svc = (qmi_cci_service_info *)&service_array[i];
    if(svc->instance == instance_id)
    {
      memcpy(service_info, svc, sizeof(qmi_service_info));
      rc = QMI_NO_ERR;
      break;
    }
  }

free_bail:
  FREE(service_array);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_client_get_instance_id
===========================================================================*/
qmi_client_error_type
qmi_client_get_instance_id
(
 qmi_service_info *service_info,
 qmi_service_instance *instance_id
 )
{
  qmi_cci_service_info *svc = (qmi_cci_service_info *)service_info;

  if(!svc || !instance_id)
    return QMI_INTERNAL_ERR;

  *instance_id = svc->instance;
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_client_register_error_cb
===========================================================================*/
qmi_client_error_type qmi_client_register_error_cb
(
 qmi_client_type user_handle,
 qmi_client_error_cb err_cb,
 void *err_cb_data
 )
{
  qmi_cci_client_type *clnt;
  unsigned int err_pending;
  int rc = QMI_NO_ERR;

  if(!err_cb)
    return QMI_INTERNAL_ERR;

  clnt = qmi_cci_get_ref(user_handle, 0);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  /* Do not allow error callback registration
   * on the notifier as it is not connected to any
   * physical service */
  if(clnt->ext_signal)
  {
    rc = QMI_PORT_NOT_OPEN_ERR;
    goto bail;
  }

  LOCK(&clnt->info_lock);
  clnt->info.client.err_cb = err_cb;
  clnt->info.client.err_cb_data = err_cb_data;
  err_pending = clnt->info.client.err_pending;
  UNLOCK(&clnt->info_lock);

  if(err_pending)
  {
    err_cb(CLIENT_HANDLE(clnt), QMI_SERVICE_ERR, err_cb_data);
    rc = QMI_SERVICE_ERR;
  }

bail:
  (void)qmi_cci_put_ref(clnt);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_client_register_notify_cb
===========================================================================*/
qmi_client_error_type qmi_client_register_notify_cb
(
 qmi_client_type user_handle,
 qmi_client_notify_cb notify_cb,
 void *notify_cb_data
)
{
  qmi_cci_client_type *clnt;
  unsigned int notify_pending;
  int rc = QMI_NO_ERR;

  if(!notify_cb)
    return QMI_INTERNAL_ERR;

  clnt = qmi_cci_get_ref(user_handle, 0);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  /* Do not allow notify callback registration on a connected client */
  if(!clnt->ext_signal)
  {
     rc = QMI_PORT_NOT_OPEN_ERR;
     goto bail;
  }

  LOCK(&clnt->info_lock);
  clnt->info.notifier.notify_cb = notify_cb;
  clnt->info.notifier.notify_cb_data = notify_cb_data;
  notify_pending = clnt->info.notifier.notify_pending;
  UNLOCK(&clnt->info_lock);

  if(notify_pending)
  {
    notify_cb(CLIENT_HANDLE(clnt), clnt->service_obj,
                 QMI_CLIENT_SERVICE_COUNT_INC, notify_cb_data);
  }
bail:
  (void)qmi_cci_put_ref(clnt);
  return rc;
}


/*==========================================================================
  FUNCTION  qmi_client_get_async_txn_id
===========================================================================*/
/*!
@brief

  Gets a transaction id from the transaction handle

@return
  qmi_client_error_type

@note
  This API is added to support the legacy messages that needs access to the
  transcation ID.
*/
/*=========================================================================*/
qmi_client_error_type
qmi_client_get_async_txn_id
(
 qmi_client_type  user_handle,
 qmi_txn_handle   async_txn_handle,
 uint32_t *txn_id
)
{
  qmi_cci_txn_type *i;
  qmi_cci_client_type *clnt;
  qmi_cci_txn_type *txn = (qmi_cci_txn_type *)async_txn_handle;
  qmi_client_error_type rc = QMI_INVALID_TXN;

  if (!txn_id || !txn )
    return QMI_INTERNAL_ERR;

  clnt = qmi_cci_get_ref(user_handle, 0);
  if(!clnt)
    return QMI_INTERNAL_ERR;

  *txn_id = 0;

  LOCK(&clnt->txn_list_lock);
  LIST_FIND(clnt->txn_list, i, link, i == txn);
  if(i)
  {
    *txn_id = i->txn_id;
    rc = QMI_NO_ERR;
  }
  UNLOCK(&clnt->txn_list_lock);

  (void)qmi_cci_put_ref(clnt);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_client_init_instance
===========================================================================*/
qmi_client_error_type
qmi_client_init_instance
(
 qmi_idl_service_object_type service_obj,
 qmi_service_instance        instance_id,
 qmi_client_ind_cb           ind_cb,
 void                        *ind_cb_data,
 qmi_client_os_params        *os_params,
 uint32_t                    timeout,
 qmi_client_type             *user_handle
)
{
  int rc;
  qmi_client_type notifier;
  qmi_service_info info;
  qmi_client_os_params notifier_os_params;

  if(!user_handle || !service_obj)
  {
    return QMI_INTERNAL_ERR;
  }

  /* The common case when we do not have to wait for the service,
   * avoid creation of the notifier */
  rc = qmi_client_get_service_instance(service_obj, instance_id, &info);
  if(rc == QMI_NO_ERR)
  {
    rc = qmi_client_init(&info, service_obj, ind_cb, ind_cb_data, os_params, user_handle);
    if(rc == QMI_NO_ERR || rc != QMI_SERVICE_ERR)
    {
      return rc;
    }
  }

  memset(&notifier_os_params, 0, sizeof(notifier_os_params));
  QMI_CCI_COPY_OS_PARAMS(&notifier_os_params, os_params);

  rc = qmi_client_notifier_init(service_obj, &notifier_os_params, &notifier);
  if(rc)
  {
    return rc;
  }

  while(1)
  {
    QMI_CCI_OS_SIGNAL_CLEAR(&notifier_os_params);
    rc = qmi_client_get_service_instance(service_obj, instance_id, &info);
    if(rc == QMI_NO_ERR)
    {
      rc = qmi_client_init(&info, service_obj, ind_cb, ind_cb_data, os_params, user_handle);
      if(rc == QMI_NO_ERR || rc != QMI_SERVICE_ERR)
      {
        /* Success or a generic error occured */
        break;
      }
    }
    QMI_CCI_OS_SIGNAL_WAIT(&notifier_os_params, timeout);
    if(QMI_CCI_OS_SIGNAL_TIMED_OUT(&notifier_os_params))
    {
      rc = QMI_TIMEOUT_ERR;
      break;
    }
  }
  qmi_client_release(notifier);
  return rc;
}
