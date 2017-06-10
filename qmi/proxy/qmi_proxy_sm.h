/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */
#ifndef QMI_PROXY_SM_H_INCLUDED
#define QMI_PROXY_SM_H_INCLUDED

/*
 * Hierarchical state machine implementation
 */
#ifdef TEST_QMI_PROXY_SM
#include <stdio.h>
#include <stdlib.h>
#include <qmi.h>
#define QMI_PROXY_DEBUG_MSG printf
#define QMI_PROXY_ERR_MSG printf
#define TRUE 1
#define FALSE 0
#endif

#define QMI_PROXY_SM_STATE_VALID(s,max) ((s) > 0 && (s) < (max))

#define QMI_PROXY_SM_MAX_STATE_DEPTH 2

/* Forward declarations */
struct qmi_proxy_state_machine_type;
typedef struct qmi_proxy_state_machine_type qmi_proxy_state_machine_type;
struct qmi_proxy_sm_state_type;
typedef struct qmi_proxy_sm_state_type qmi_proxy_sm_state_type;

/* Type definitions */
typedef void (*qmi_proxy_sm_cb)(qmi_proxy_state_machine_type *, unsigned evt_id, void *);
typedef struct
{
  const char *name;
  unsigned id;
  void *user_data;
} qmi_proxy_sm_event_type;

struct qmi_proxy_sm_state_type
{
  unsigned id;
  const char *name;
  qmi_proxy_sm_cb enter;
  qmi_proxy_sm_cb leave;
  unsigned parent_id;
  int nsubstates;
  unsigned *substates;
  unsigned initial_substate;
};

/* Function prototypes */

void qmi_proxy_sm_event_handler
(
  qmi_proxy_state_machine_type *sm,
  qmi_proxy_sm_event_type *evt
);

int qmi_proxy_sm_state_config_valid(
  qmi_proxy_state_machine_type *sm
);

int qmi_proxy_sm_get_state_depth
(
  qmi_proxy_state_machine_type *sm,
  unsigned id
);

unsigned qmi_proxy_sm_find_common_ancestor
(
  qmi_proxy_state_machine_type *sm,
  unsigned id1,
  int depth1,
  unsigned id2,
  int depth2
);

void qmi_proxy_sm_event_handler
(
  qmi_proxy_state_machine_type *sm,
  qmi_proxy_sm_event_type *evt
);

qmi_proxy_state_machine_type *qmi_proxy_sm_new
(
  qmi_proxy_sm_state_type *state_table,
  int nstates,
  unsigned *transition_table,
  int nevents,
  unsigned initial_state
);

int qmi_proxy_sm_init
(
  qmi_proxy_state_machine_type *sm,
  const qmi_proxy_sm_state_type *state_table,
  int nstates,
  const unsigned *transition_table,
  int nevents,
  unsigned initial_state
);

void qmi_proxy_sm_delete
(
  qmi_proxy_state_machine_type *sm
);
#endif
