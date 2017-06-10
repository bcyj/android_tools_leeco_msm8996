/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef QMI_PROXY_SGLTE_SM_H_INCLUDED
#define QMI_PROXY_SGLTE_SM_H_INCLUDED

#include "qmi_proxy_sm.h"

struct qmi_proxy_sglte_state_machine_type;
typedef struct qmi_proxy_sglte_state_machine_type qmi_proxy_sglte_state_machine_type;


typedef enum
{
  SGLTE_SM_EVENT_INVALID,
  SGLTE_SM_EVENT_ONLINE_IND,
  SGLTE_SM_EVENT_LPM_IND,
  SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE,
  SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE,
  SGLTE_SM_EVENT_NON_SGLTE_PLMN,
  SGLTE_SM_EVENT_SGLTE_MODE,
  SGLTE_SM_EVENT_NON_SGLTE_MODE,
  SGLTE_SM_EVENT_FULL_SVC_TIMER_EXPIRED,
  SGLTE_SM_EVENT_LPM_REQUEST,
  SGLTE_SM_EVENT_LPM_FAIL,
  /* SGLTE Mode events */
  SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL,
  SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL,
  SGLTE_SM_EVENT_NO_SVC_ON_LOCAL,
  SGLTE_SM_EVENT_FULL_SVC_ON_REMOTE,
  SGLTE_SM_EVENT_LIMITED_SVC_ON_REMOTE,
  SGLTE_SM_EVENT_NO_SVC_ON_REMOTE,
  SGLTE_SM_EVENT_PS_TO_G_TIMER_EXPIRED,
  SGLTE_SM_EVENT_CAMP_ONLY_ACTIVE_ON_LOCAL,
  SGLTE_SM_EVENT_UNRESTRICTED_ACTIVE_ON_LOCAL,
  SGLTE_SM_EVENT_PS_ACTIVE_ON_REMOTE,
  SGLTE_SM_EVENT_PS_INACTIVE_ON_REMOTE,
  SGLTE_SM_EVENT_PS_TO_LT_TIMER_EXPIRED,
  SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM,
  SGLTE_SM_EVENT_NO_VOICE_CALL,
  SGLTE_SM_EVENT_MAX
} qmi_proxy_sglte_sm_event_id;

/*
 * Create a new SGLTE state machine. The resulting object must be freed by calling
 * qmi_proxy_sglte_sm_delete()
 */
qmi_proxy_sglte_state_machine_type *qmi_proxy_sglte_sm_new
(
  void
);

/*
 * This function must be called before sending any events to the state machine
 */
int qmi_proxy_sglte_sm_start
(
  qmi_proxy_sglte_state_machine_type *sm
);

void qmi_proxy_sglte_sm_delete
(
  qmi_proxy_sglte_state_machine_type *
);

int qmi_proxy_sglte_sm_queue_event
(
  qmi_proxy_sglte_state_machine_type *sm,
  int evt_id,
  const char *name,
  void *user
);
#endif
