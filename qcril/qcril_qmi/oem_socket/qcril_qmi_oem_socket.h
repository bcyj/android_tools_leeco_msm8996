/*!
  @file
  qcril_qmi_ims_socket.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#ifdef  __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "ril.h"
#include "qcril_qmi_client.h"
#include "qcril_other.h"
#include "qcril_arb.h"

#define QCRIL_QMI_OEM_SOCKET_MAX_BUF_SIZE              1024

#define OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE    4
#define OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE       4
#define OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE             2
#define OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE             2

#define OEM_HOOK_QMI_TUNNELING_REQ_MODEM_ID_SIZE       8
#define OEM_HOOK_QMI_TUNNELING_REQ_MSG_SIZE            4
#define OEM_HOOK_QMI_TUNNELING_ERROR_CODE_SIZE         2

#define OEM_HOOK_QMI_TUNNELING_RESP_OVERHEAD_SIZE      ( OEM_HOOK_QMI_TUNNELING_RESP_REQUEST_ID_SIZE +              \
                                                         OEM_HOOK_QMI_TUNNELING_RESP_RESP_SZ_SIZE +                    \
                                                         OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE +                               \
                                                         OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE +                               \
                                                         OEM_HOOK_QMI_TUNNELING_ERROR_CODE_SIZE )

#define OEM_HOOK_QMI_TUNNELING_RESULT_SIZE             7

#define OEM_HOOK_QMI_TUNNELING_IND_Q_STR_LEN_SIZE      (QCRIL_OTHER_OEM_NAME_LENGTH)
#define OEM_HOOK_QMI_TUNNELING_IND_EVENT_ID_SIZE       4
#define OEM_HOOK_QMI_TUNNELING_IND_PLD_LEN_SIZE        4

#define OEM_HOOK_QMI_TUNNELING_IND_OVERHEAD_SIZE       ( OEM_HOOK_QMI_TUNNELING_IND_Q_STR_LEN_SIZE + OEM_HOOK_QMI_TUNNELING_IND_EVENT_ID_SIZE + \
                                                         OEM_HOOK_QMI_TUNNELING_IND_PLD_LEN_SIZE + OEM_HOOK_QMI_TUNNELING_SVC_ID_SIZE + OEM_HOOK_QMI_TUNNELING_MSG_ID_SIZE )

typedef enum _Oem__MsgType {
    OEM__MSG_TYPE__RESPONSE       = 0,
    OEM__MSG_TYPE__UNSOL_RESPONSE = 1
} Oem_MsgType;

typedef struct
{
    int                                       is_oem_hook;
    uint32                                    hook_req;
    uint32                                    hook_req_len;
    int                                       is_qmi_tunneling;
    uint16                                    ril_idl_service_id; // applicable only when is_qmi_tunneling is TRUE
    uint16                                    ril_idl_message_id; // applicable only when is_qmi_tunneling is TRUE
} qmi_ril_oem_hook_request_details_type;

typedef struct qmi_ril_oem_hook_response_context_type
{
  RIL_Token     original_token;
  uint16_t      ril_idl_service_id;
  uint16_t      ril_idl_message_id;
  uint32_t      ril_request_id;
  struct qmi_ril_oem_hook_response_context_type *next;
} qmi_ril_oem_hook_response_context_type;

typedef struct
{
  qmi_ril_oem_hook_response_context_type     *qmi_ril_oem_hook_qmi_tunneling_reqs_root;
  pthread_mutex_t                             overview_lock_mutex;
  pthread_mutexattr_t                         overview_lock_mtx_atr;
} qmi_ril_oem_hook_overview_type;

RIL_Errno qmi_ril_oem_hook_get_request_id
(
  uint16   service_id,
  uint16   message_id,
  uint32  *request_id,
  char   **log_ind
);

RIL_Errno qmi_ril_parse_oem_hook_header
(
  unsigned char *data,
  qmi_ril_oem_hook_request_details_type *outcome
);

IxErrnoType qcril_dispatch_event
(
  qcril_dispatch_table_entry_type *entry_ptr,
  qcril_request_params_type *params_ptr
);

IxErrnoType qcril_hash_table_lookup
(
  uint32 id,
  qcril_dispatch_table_entry_type **entry_ptr_ptr /*!< OUT table entry ptr, if found */
);

void qcril_qmi_oem_socket_init
(
  void
);
void qcril_qmi_oem_socket_send
(
  qcril_instance_id_e_type oem_socket_instance_id,
  RIL_Token   token,
  int         message_id,
  RIL_Errno   error,
  const void* msg,
  int         msg_len
);

void qcril_qmi_oem_socket_send_unsol
(
  const void *msg,
  int         msg_len
);

boolean qmi_ril_parse_oem_req_tunnelled_message
(
  qmi_ril_oem_hook_request_details_type  *oem_hook_req_details,
  RIL_Errno                              *audit_result,
  qcril_request_params_type              *param
);

boolean qmi_ril_get_req_details_from_oem_req
(
  qmi_ril_oem_hook_request_details_type  *oem_hook_req_details,
  RIL_Errno                              *audit_result,
  unsigned char                          *data,
  qcril_request_params_type              *param,
  int                                     recv_byte_num
);

qmi_ril_oem_hook_overview_type qmi_ril_oem_hook_overview;
#ifdef  __cplusplus
}
#endif
