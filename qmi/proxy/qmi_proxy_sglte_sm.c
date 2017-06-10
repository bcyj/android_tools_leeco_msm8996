/*
 * Copyright (c) 2012, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

//#define TEST_QMI_PROXY_SM
#ifdef TEST_QMI_PROXY_SM
#include <pthread.h>
#include <qmi.h>
#include <qmi_i.h>
#include <qmi_platform.h>
#endif

#include "qmi_proxy_sglte_sm.h"
//#include "qmi_proxy.c"
#include "qmi_proxy_queue.h"


#define STRINGIZE(x) (#x)

struct qmi_proxy_sglte_state_machine_type
{
  qmi_proxy_state_machine_type *sm;
  qmi_proxy_queue *msg_queue;
  QMI_PLATFORM_MUTEX_DATA_TYPE msg_queue_lock;
  QMI_PLATFORM_SIGNAL_DATA_TYPE queue_signal;
  pthread_t sm_thread;
};

#ifdef TEST_QMI_PROXY_SM
#define sglte_sm_power_up_enter test_sglte_sm_power_up_enter
#define sglte_sm_sglte_mode_enter test_sglte_sm_sglte_mode_enter
#define sglte_sm_in_china_non_sglte_mode_enter test_sglte_sm_in_china_non_sglte_mode_enter
#define sglte_sm_non_sglte_coverage_enter test_sglte_sm_non_sglte_coverage_enter
#define sglte_sm_complete_oos_enter test_sglte_sm_complete_oos_enter
#define sglte_mode_substate_active_enter test_sglte_mode_substate_active_enter
#define sglte_mode_substate_trans_ps_to_g_enter test_sglte_mode_substate_trans_ps_to_g_enter
#define sglte_mode_substate_trans_ps_to_g_leave test_sglte_mode_substate_trans_ps_to_g_leave
#define sglte_mode_substate_ps_on_g_enter test_sglte_mode_substate_ps_on_g_enter
#define sglte_mode_substate_trans_ps_to_lt_enter test_sglte_mode_substate_trans_ps_to_lt_enter
#define sglte_mode_substate_trans_ps_to_lt_leave test_sglte_mode_substate_trans_ps_to_lt_leave
#define sglte_mode_substate_no_svc_on_qsc_enter test_sglte_mode_substate_no_svc_on_qsc_enter
#include <stdio.h>
#include <stdlib.h>

void test_sglte_sm_power_up_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);

void test_sglte_sm_sglte_mode_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);

void test_sglte_sm_in_china_non_sglte_mode_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);

void test_sglte_sm_non_sglte_coverage_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);

void test_sglte_sm_complete_oos_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);

void test_sglte_mode_substate_active_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);

void test_sglte_mode_substate_trans_ps_to_g_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);

void test_sglte_mode_substate_trans_ps_to_g_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);

void test_sglte_mode_substate_ps_on_g_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);

void test_sglte_mode_substate_trans_ps_to_lt_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);

void test_sglte_mode_substate_trans_ps_to_lt_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);

void test_sglte_mode_substate_no_svc_on_qsc_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
);
#else

void sglte_sm_power_up_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void* user
);

void sglte_sm_proc_lpm_req_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void sglte_sm_sglte_coverage_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void sglte_sm_sglte_coverage_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void sglte_sm_sglte_mode_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void sglte_sm_sglte_coverage_non_sglte_mode_enter
(  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_sm_sglte_coverage_non_sglte_mode_waiting_svc_enter
(  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_sm_sglte_coverage_non_sglte_mode_waiting_svc_leave
(  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_sm_non_sglte_coverage_enter
(  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_sm_non_sglte_coverage_waiting_svc_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void sglte_sm_non_sglte_coverage_waiting_svc_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void sglte_sm_complete_oos_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_mode_substate_active_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_mode_substate_active_waiting_svc_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_mode_substate_active_waiting_svc_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_mode_substate_active_no_svc_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_mode_substate_active_no_svc_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_mode_substate_trans_ps_to_g_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_mode_substate_trans_ps_to_g_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_mode_substate_ps_on_g_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
   void *user
);

void sglte_mode_substate_trans_ps_to_lt_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void sglte_mode_substate_trans_ps_to_lt_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void trans_ps_to_lt_timer_started_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void trans_ps_to_lt_timer_started_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void trans_ps_to_lte_moving_ps_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void trans_ps_to_lt_removing_camp_only_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void sglte_mode_substate_no_svc_on_qsc_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void trans_ps_to_g_timer_started_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);
void trans_ps_to_g_timer_started_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);
void trans_ps_to_g_timer_started_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);
void trans_ps_to_g_setting_camp_only_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);
void trans_ps_to_g_setting_camp_only_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);
void trans_ps_to_g_moving_ps_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);
void trans_ps_to_g_moving_ps_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);
int qmi_proxy_force_srv_domain_pref
(
  qmi_proxy_conn_type conn_type,
  nas_srv_domain_pref_enum_type_v01 srv_domain_pref,
  boolean persistent
);

int qmi_proxy_force_srv_restriction
(
  qmi_proxy_conn_type conn_type,
  nas_srv_reg_restriction_enum_v01 srv_restriction,
  boolean persistent
);

void sglte_mode_substate_no_svc_on_local_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);

void sglte_mode_voice_on_remote_no_dtm_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
);
#endif

void sglte_validate_service_and_call_status
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id
);

typedef enum
{
  SGLTE_STATE_INVALID = 0,
  SGLTE_STATE_POWER_UP,
  SGLTE_STATE_SGLTE_COVERAGE,
    /* SGLTE_COVERAGE Substates */
    SGLTE_STATE_SGLTE_MODE,
      /* SGLTE_MODE Substates */
      SGLTE_STATE_SGLTE_MODE_ACTIVE,
        /* SGLTE_MODE_ACTIVE Substates */
        SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC,
          SGLTE_STATE_SGLTE_MODE_ACTIVE_TO_NON_SGLTE_WAITING_SVC,
        SGLTE_STATE_SGLTE_MODE_ACTIVE_NO_SVC,
      SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_G,
        SGLTE_STATE_TRANS_PS_TO_G_TIMER_STARTED,
        SGLTE_STATE_TRANS_PS_TO_G_SETTING_CAMP_ONLY,
        SGLTE_STATE_TRANS_PS_TO_G_MOVING_PS,
      SGLTE_STATE_SGLTE_MODE_PS_ON_G,
      SGLTE_STATE_SGLTE_MODE_PS_ON_G_VOICE_CALL_ON_REMOTE_NO_DTM,
      SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_LT,
        SGLTE_SGLTE_STATE_TRANS_PS_TO_LT_TIMER_STARTED,
        SGLTE_STATE_TRANS_PS_TO_LT_MOVING_PS,
        SGLTE_STATE_TRANS_PS_TO_LT_REMOVING_CAMP_ONLY,
      SGLTE_STATE_SGLTE_MODE_NO_SVC_ON_REMOTE,
      SGLTE_STATE_SGLTE_MODE_VOICE_CALL_ON_REMOTE_NO_DTM,
      SGLTE_STATE_SGLTE_MODE_NO_SVC_ON_LOCAL,
    SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE,
      SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
        SGLTE_STATE_SGLTE_COV_NON_SGLTE_MODE_TO_NON_SGLTE_WAITING_SVC,
  SGLTE_STATE_NON_SGLTE_COVERAGE,
    SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC,
      SGLTE_STATE_NON_SGLTE_TO_SGLTE_COV_SGLTE_MODE_WAITING_SVC,
      SGLTE_STATE_NON_SGLTE_TO_SGLTE_COV_NON_SGLTE_MODE_WAITING_SVC,
  SGLTE_STATE_COMPLETE_OOS,
  SGLTE_STATE_PROC_LPM_REQ,
  SGLTE_STATE_MAX /* This must be the last entry. Not valid as state */
} qmi_proxy_sglte_state_id;

/* Not a state machine event. This type signals queue reader to stop receiving events */
#define QMI_PROXY_SGLTE_EVT_STOP 0xffff1457

#define QMI_PROXY_SM_STATE_ENTRY(idarg, ...) \
  [idarg] = { \
                               .id = idarg, \
                               .name = #idarg, \
                               __VA_ARGS__ \
  }
qmi_proxy_sm_state_type qmi_proxy_sglte_state_table[SGLTE_STATE_MAX] =
{
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_POWER_UP,
                           .enter = sglte_sm_power_up_enter
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_COVERAGE,
                               .enter = sglte_sm_sglte_coverage_enter,
                               .leave = sglte_sm_sglte_coverage_leave,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE,
                               .enter = sglte_sm_sglte_mode_enter,
                               .initial_substate = SGLTE_STATE_SGLTE_MODE_ACTIVE,
                               .parent_id = SGLTE_STATE_SGLTE_COVERAGE
  ),
  /* SGLTE MODE Substates */
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE_ACTIVE,
                               .enter = sglte_mode_substate_active_enter,
                               .parent_id = SGLTE_STATE_SGLTE_MODE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC,
                               .enter = sglte_mode_substate_active_waiting_svc_enter,
                               .leave = sglte_mode_substate_active_waiting_svc_leave,
                               .parent_id = SGLTE_STATE_SGLTE_MODE_ACTIVE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE_ACTIVE_TO_NON_SGLTE_WAITING_SVC,
                               .parent_id = SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE_ACTIVE_NO_SVC,
                               .enter = sglte_mode_substate_active_no_svc_enter,
                               .leave = sglte_mode_substate_active_no_svc_leave,
                               .parent_id = SGLTE_STATE_SGLTE_MODE_ACTIVE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_G,
                               .enter = sglte_mode_substate_trans_ps_to_g_enter,
                               .leave = sglte_mode_substate_trans_ps_to_g_leave,
                               .initial_substate = SGLTE_STATE_TRANS_PS_TO_G_TIMER_STARTED,
                               .parent_id = SGLTE_STATE_SGLTE_MODE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_TRANS_PS_TO_G_TIMER_STARTED,
                               .enter = trans_ps_to_g_timer_started_enter,
                               .leave = trans_ps_to_g_timer_started_leave,
                               .parent_id = SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_G,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_TRANS_PS_TO_G_SETTING_CAMP_ONLY,
                               .enter = trans_ps_to_g_setting_camp_only_enter,
                               .leave = trans_ps_to_g_setting_camp_only_leave,
                               .parent_id = SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_G,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_TRANS_PS_TO_G_MOVING_PS,
                               .enter = trans_ps_to_g_moving_ps_enter,
                               .leave = trans_ps_to_g_moving_ps_leave,
                               .parent_id = SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_G,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE_PS_ON_G,
                               .enter = sglte_mode_substate_ps_on_g_enter,
                               .parent_id = SGLTE_STATE_SGLTE_MODE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE_PS_ON_G_VOICE_CALL_ON_REMOTE_NO_DTM,
                               .parent_id = SGLTE_STATE_SGLTE_MODE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_LT,
                               .enter = sglte_mode_substate_trans_ps_to_lt_enter,
                               .leave = sglte_mode_substate_trans_ps_to_lt_leave,
                               .parent_id = SGLTE_STATE_SGLTE_MODE,
                               .initial_substate = SGLTE_SGLTE_STATE_TRANS_PS_TO_LT_TIMER_STARTED,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_SGLTE_STATE_TRANS_PS_TO_LT_TIMER_STARTED,
                               .enter = trans_ps_to_lt_timer_started_enter,
                               .leave = trans_ps_to_lt_timer_started_leave,
                               .parent_id = SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_LT,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_TRANS_PS_TO_LT_MOVING_PS,
                               .enter = trans_ps_to_lte_moving_ps_enter,
                               .parent_id = SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_LT,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_TRANS_PS_TO_LT_REMOVING_CAMP_ONLY,
                               .enter = trans_ps_to_lt_removing_camp_only_enter,
                               .parent_id = SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_LT,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE_NO_SVC_ON_REMOTE,
                               .enter = sglte_mode_substate_no_svc_on_qsc_enter,
                               .parent_id = SGLTE_STATE_SGLTE_MODE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE_NO_SVC_ON_LOCAL,
                               .enter = sglte_mode_substate_no_svc_on_local_enter,
                               .parent_id = SGLTE_STATE_SGLTE_MODE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_MODE_VOICE_CALL_ON_REMOTE_NO_DTM,
                               .enter = sglte_mode_voice_on_remote_no_dtm_enter,
                               .parent_id = SGLTE_STATE_SGLTE_MODE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE,
                               .enter = sglte_sm_sglte_coverage_non_sglte_mode_enter,
                               .parent_id = SGLTE_STATE_SGLTE_COVERAGE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
                               .enter = sglte_sm_sglte_coverage_non_sglte_mode_waiting_svc_enter,
                               .leave = sglte_sm_sglte_coverage_non_sglte_mode_waiting_svc_leave,
                               .parent_id = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_SGLTE_COV_NON_SGLTE_MODE_TO_NON_SGLTE_WAITING_SVC,
                               .parent_id = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_NON_SGLTE_COVERAGE,
                               .enter = sglte_sm_non_sglte_coverage_enter,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC,
                               .enter = sglte_sm_non_sglte_coverage_waiting_svc_enter,
                               .leave = sglte_sm_non_sglte_coverage_waiting_svc_leave,
                               .parent_id = SGLTE_STATE_NON_SGLTE_COVERAGE,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_NON_SGLTE_TO_SGLTE_COV_SGLTE_MODE_WAITING_SVC,
                               .parent_id = SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_NON_SGLTE_TO_SGLTE_COV_NON_SGLTE_MODE_WAITING_SVC,
                               .parent_id = SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC,
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_COMPLETE_OOS,
                               .enter = sglte_sm_complete_oos_enter
  ),
  QMI_PROXY_SM_STATE_ENTRY(SGLTE_STATE_PROC_LPM_REQ,
                           .enter = sglte_sm_proc_lpm_req_enter
  ),
};

qmi_proxy_sglte_state_id qmi_proxy_sglte_transition_table[SGLTE_STATE_MAX][SGLTE_SM_EVENT_MAX] =
{
  [SGLTE_STATE_POWER_UP] = {
                                 [SGLTE_SM_EVENT_ONLINE_IND]             = SGLTE_STATE_COMPLETE_OOS,
  },
  [SGLTE_STATE_SGLTE_COVERAGE] = {
                                 [SGLTE_SM_EVENT_FULL_SVC_TIMER_EXPIRED]     = SGLTE_STATE_COMPLETE_OOS,
                                 [SGLTE_SM_EVENT_LPM_IND]                    = SGLTE_STATE_POWER_UP,
                                 [SGLTE_SM_EVENT_LPM_REQUEST]                = SGLTE_STATE_PROC_LPM_REQ,
  },
  [SGLTE_STATE_SGLTE_MODE] = {
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE]  = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_NON_SGLTE_PLMN]             = SGLTE_STATE_NON_SGLTE_COVERAGE,
                                 [SGLTE_SM_EVENT_NON_SGLTE_MODE]             = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE,
  },
  [SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE] = {
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE]      = SGLTE_STATE_SGLTE_MODE,
                                 [SGLTE_SM_EVENT_NON_SGLTE_PLMN]             = SGLTE_STATE_NON_SGLTE_COVERAGE,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_LOCAL]            = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_SGLTE_MODE]                 = SGLTE_STATE_SGLTE_MODE,
  },
  [SGLTE_STATE_NON_SGLTE_COVERAGE] = {
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE]      = SGLTE_STATE_SGLTE_MODE,
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE]  = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_LOCAL]            = SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_FULL_SVC_TIMER_EXPIRED]     = SGLTE_STATE_COMPLETE_OOS,
                                 [SGLTE_SM_EVENT_LPM_IND]                    = SGLTE_STATE_POWER_UP,
                                 [SGLTE_SM_EVENT_LPM_REQUEST]                = SGLTE_STATE_PROC_LPM_REQ,
  },
  [SGLTE_STATE_COMPLETE_OOS] = {
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE]     = SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE] = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_NON_SGLTE_PLMN]            = SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_LPM_IND]                   = SGLTE_STATE_POWER_UP,
                                 [SGLTE_SM_EVENT_LPM_REQUEST]               = SGLTE_STATE_PROC_LPM_REQ,
  },
  [SGLTE_STATE_SGLTE_MODE_NO_SVC_ON_REMOTE] = {
                                 [SGLTE_SM_EVENT_NO_SVC_ON_LOCAL]           = SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_REMOTE]        = SGLTE_STATE_SGLTE_MODE_ACTIVE,
  },
  [SGLTE_STATE_SGLTE_MODE_NO_SVC_ON_LOCAL] = {
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL]         = SGLTE_STATE_SGLTE_MODE_ACTIVE,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_REMOTE]          = SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_NO_VOICE_CALL]             = SGLTE_STATE_SGLTE_MODE_ACTIVE,
  },
  [SGLTE_STATE_SGLTE_MODE_VOICE_CALL_ON_REMOTE_NO_DTM] = {
                                 [SGLTE_SM_EVENT_NO_SVC_ON_LOCAL]           = SGLTE_STATE_SGLTE_MODE_NO_SVC_ON_LOCAL,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_REMOTE]          = SGLTE_STATE_SGLTE_MODE_NO_SVC_ON_REMOTE,
                                 [SGLTE_SM_EVENT_NO_VOICE_CALL]             = SGLTE_STATE_SGLTE_MODE_ACTIVE,
  },
  [SGLTE_STATE_SGLTE_MODE_ACTIVE] = {
                                 [SGLTE_SM_EVENT_NO_SVC_ON_LOCAL]           = SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_G,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_REMOTE]          = SGLTE_STATE_SGLTE_MODE_NO_SVC_ON_REMOTE,
                                 [SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM]  = SGLTE_STATE_SGLTE_MODE_VOICE_CALL_ON_REMOTE_NO_DTM,
  },
  [SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_G] = {
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL]          = SGLTE_STATE_SGLTE_MODE_ACTIVE,
                                 [SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL]     = SGLTE_STATE_SGLTE_MODE_ACTIVE,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_REMOTE]           = SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM]   = SGLTE_STATE_SGLTE_MODE_VOICE_CALL_ON_REMOTE_NO_DTM,
  },
  [SGLTE_STATE_TRANS_PS_TO_G_TIMER_STARTED] = {
                                 [SGLTE_SM_EVENT_PS_TO_G_TIMER_EXPIRED]      = SGLTE_STATE_TRANS_PS_TO_G_SETTING_CAMP_ONLY,
  },
  [SGLTE_STATE_TRANS_PS_TO_G_SETTING_CAMP_ONLY] = {
                                 [SGLTE_SM_EVENT_CAMP_ONLY_ACTIVE_ON_LOCAL]  = SGLTE_STATE_TRANS_PS_TO_G_MOVING_PS,
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL]          = SGLTE_STATE_SGLTE_MODE_ACTIVE,
                                 [SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL]     = SGLTE_STATE_SGLTE_MODE_ACTIVE,
  },
  [SGLTE_STATE_TRANS_PS_TO_G_MOVING_PS] = {
                                 [SGLTE_SM_EVENT_PS_ACTIVE_ON_REMOTE]        = SGLTE_STATE_SGLTE_MODE_PS_ON_G,
                                 [SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL]     = SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_LT,
                                 [SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM]   = SGLTE_STATE_TRANS_PS_TO_G_MOVING_PS,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_REMOTE]           = SGLTE_STATE_TRANS_PS_TO_LT_MOVING_PS,
  },
  [SGLTE_STATE_SGLTE_MODE_PS_ON_G] = {
                                 [SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL]     = SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_LT,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_REMOTE]           = SGLTE_STATE_SGLTE_MODE_ACTIVE_NO_SVC,
                                 [SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM]   = SGLTE_STATE_SGLTE_MODE_PS_ON_G_VOICE_CALL_ON_REMOTE_NO_DTM,
  },
  [SGLTE_STATE_SGLTE_MODE_PS_ON_G_VOICE_CALL_ON_REMOTE_NO_DTM] = {
                                 [SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL]     = SGLTE_STATE_SGLTE_MODE_PS_ON_G_VOICE_CALL_ON_REMOTE_NO_DTM,
                                 [SGLTE_SM_EVENT_NO_VOICE_CALL]              = SGLTE_STATE_SGLTE_MODE_PS_ON_G,
  },
  [SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_LT] = {
                                 [SGLTE_SM_EVENT_NO_SVC_ON_LOCAL]            = SGLTE_STATE_SGLTE_MODE_PS_ON_G,
                                 [SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM]   = SGLTE_STATE_SGLTE_MODE_PS_ON_G_VOICE_CALL_ON_REMOTE_NO_DTM,
  },
  [SGLTE_SGLTE_STATE_TRANS_PS_TO_LT_TIMER_STARTED] = {
                                 [SGLTE_SM_EVENT_PS_TO_LT_TIMER_EXPIRED]      = SGLTE_STATE_TRANS_PS_TO_LT_MOVING_PS,
  },
  [SGLTE_STATE_TRANS_PS_TO_LT_MOVING_PS] = {
                                 [SGLTE_SM_EVENT_NO_SVC_ON_LOCAL]             = SGLTE_STATE_TRANS_PS_TO_G_MOVING_PS,
                                 [SGLTE_SM_EVENT_PS_INACTIVE_ON_REMOTE]       = SGLTE_STATE_TRANS_PS_TO_LT_REMOVING_CAMP_ONLY,
                                 [SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM]    = SGLTE_STATE_TRANS_PS_TO_LT_MOVING_PS,
  },
  [SGLTE_STATE_TRANS_PS_TO_LT_REMOVING_CAMP_ONLY] = {
                                 [SGLTE_SM_EVENT_UNRESTRICTED_ACTIVE_ON_LOCAL]  = SGLTE_STATE_SGLTE_MODE_ACTIVE,
                                 [SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM]      = SGLTE_STATE_TRANS_PS_TO_LT_REMOVING_CAMP_ONLY,
  },
  [SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC] = {
                                 [SGLTE_SM_EVENT_NO_SVC_ON_LOCAL]            = SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_REMOTE]           = SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_NON_SGLTE_PLMN]             = SGLTE_STATE_SGLTE_MODE_ACTIVE_TO_NON_SGLTE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE]  = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL]          = SGLTE_STATE_SGLTE_MODE_NO_SVC_ON_REMOTE,
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_REMOTE]         = SGLTE_STATE_SGLTE_MODE_ACTIVE,
  },
  [SGLTE_STATE_SGLTE_MODE_ACTIVE_TO_NON_SGLTE_WAITING_SVC] = {
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL]          = SGLTE_STATE_NON_SGLTE_COVERAGE,
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_REMOTE]         = SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE]      = SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC,
  },
  [SGLTE_STATE_SGLTE_MODE_ACTIVE_NO_SVC] = {
                                 [SGLTE_SM_EVENT_NO_SVC_ON_LOCAL]            = SGLTE_STATE_SGLTE_MODE_ACTIVE_NO_SVC,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_REMOTE]           = SGLTE_STATE_SGLTE_MODE_ACTIVE_NO_SVC,
                                 [SGLTE_SM_EVENT_NON_SGLTE_PLMN]             = SGLTE_STATE_SGLTE_MODE_ACTIVE_TO_NON_SGLTE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE]  = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_REMOTE]         = SGLTE_STATE_SGLTE_MODE_PS_ON_G,
                                 [SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL]     = SGLTE_STATE_SGLTE_MODE_TRANS_PS_TO_LT,
  },
  [SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC] = {
                                 [SGLTE_SM_EVENT_NO_SVC_ON_LOCAL]            = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_REMOTE]           = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE]  = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_NON_SGLTE_PLMN]             = SGLTE_STATE_SGLTE_COV_NON_SGLTE_MODE_TO_NON_SGLTE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE]      = SGLTE_STATE_SGLTE_MODE_ACTIVE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL]          = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE,
  },
  [SGLTE_STATE_SGLTE_COV_NON_SGLTE_MODE_TO_NON_SGLTE_WAITING_SVC] = {
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL]          = SGLTE_STATE_NON_SGLTE_COVERAGE,
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE]  = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE_WAITING_SVC,
  },
  [SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC] = {
                                 [SGLTE_SM_EVENT_NO_SVC_ON_LOCAL]            = SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_NO_SVC_ON_REMOTE]           = SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE]      = SGLTE_STATE_NON_SGLTE_TO_SGLTE_COV_SGLTE_MODE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE]  = SGLTE_STATE_NON_SGLTE_TO_SGLTE_COV_NON_SGLTE_MODE_WAITING_SVC,
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL]          = SGLTE_STATE_NON_SGLTE_COVERAGE,
  },
  [SGLTE_STATE_NON_SGLTE_TO_SGLTE_COV_NON_SGLTE_MODE_WAITING_SVC] = {
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL]          = SGLTE_STATE_IN_SGLTE_COVERAGE_NON_SGLTE_MODE,
                                 [SGLTE_SM_EVENT_NON_SGLTE_PLMN]             = SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC,
  },
  [SGLTE_STATE_NON_SGLTE_TO_SGLTE_COV_SGLTE_MODE_WAITING_SVC] = {
                                 [SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL]          = SGLTE_STATE_SGLTE_MODE_NO_SVC_ON_REMOTE,
                                 [SGLTE_SM_EVENT_NON_SGLTE_PLMN]             = SGLTE_STATE_NON_SGLTE_COVERAGE_WAITING_SVC,
  },
  [SGLTE_STATE_PROC_LPM_REQ] = {
                                 [SGLTE_SM_EVENT_ONLINE_IND]             = SGLTE_STATE_COMPLETE_OOS,
                                 [SGLTE_SM_EVENT_LPM_IND]                    = SGLTE_STATE_POWER_UP,
                                 [SGLTE_SM_EVENT_LPM_FAIL]                   = SGLTE_STATE_COMPLETE_OOS,
  },
};

qmi_proxy_sglte_state_machine_type *qmi_proxy_sglte_sm_new
(
  void
)
{
  qmi_proxy_sglte_state_machine_type *sm;
  int rc;

  sm = calloc(1, sizeof(*sm));

  if (sm)
  {
    sm->sm = qmi_proxy_sm_new(
            qmi_proxy_sglte_state_table,
            sizeof(qmi_proxy_sglte_state_table) / sizeof(*qmi_proxy_sglte_state_table),
            &qmi_proxy_sglte_transition_table[0][0],
            SGLTE_SM_EVENT_MAX,
            SGLTE_STATE_POWER_UP
            );
    if (sm->sm)
    {
      sm->sm_thread = 0;
      QMI_PLATFORM_INIT_SIGNAL_DATA(&sm->queue_signal);
      sm->msg_queue = qmi_proxy_queue_new();
      if (sm->msg_queue)
      {
        QMI_PLATFORM_MUTEX_INIT(&sm->msg_queue_lock);
      }
      else
      {
        QMI_PROXY_ERR_MSG("%s: Unable to allocate memory for queue\n", __FUNCTION__);
        qmi_proxy_sm_delete(sm->sm);
        QMI_PLATFORM_DESTROY_SIGNAL_DATA(&sm->queue_signal);
        free(sm);
        sm = NULL;
      }
    }
    else
    {
      QMI_PROXY_ERR_MSG("%s: Unable to initialize state machine\n", __FUNCTION__);
      free(sm);
      sm = NULL;
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s: Unable to allocate memory for sglte state machine\n", __FUNCTION__);
  }
  return sm;
}
#define QMI_PLATFORM_WAIT_FOR_SIGNAL_NO_UNLOCK(conn_id, signal_ptr, timeout_milli_secs) \
        qmi_linux_wait_for_sig_with_timeout_no_unlock(signal_ptr, timeout_milli_secs)
#define QMI_PLATFORM_FINISH_SIGNAL_WAIT(cond_ptr) \
  do { \
    pthread_mutex_unlock(&(cond_ptr)->cond_mutex); \
  } while (0);
#define NANO_SEC 1000000000

#define QMI_PLATFORM_INIT_SIGNAL_FOR_SEND(conn_id,signal_ptr) \
do \
{ \
  pthread_mutex_lock (&(signal_ptr)->cond_mutex); \
} while (0)

#define QMI_PLATFORM_FINISH_SIGNAL_SEND(cond_ptr) \
  do { \
    pthread_mutex_unlock(&(cond_ptr)->cond_mutex); \
  } while (0);

#define QMI_PLATFORM_SEND_SIGNAL_LOCKED(conn_id,signal_ptr) \
do \
{ \
  (signal_ptr)->cond_predicate = TRUE; \
  pthread_cond_signal (&(signal_ptr)->cond_var); \
} while (0)


int qmi_linux_wait_for_sig_with_timeout_no_unlock
(
  qmi_linux_signal_data_type  *signal_ptr,
  int                         timeout_milli_secs
)
{
  int rc = QMI_NO_ERR;
  struct timeval curr_time;
  struct timespec wait_till_time;

  /* Get current time of day */
  gettimeofday (&curr_time,NULL);

  /* Set wait time seconds to current + the number of seconds needed for timeout */
  wait_till_time.tv_sec =  curr_time.tv_sec + (timeout_milli_secs/1000);
  wait_till_time.tv_nsec = (curr_time.tv_usec * 1000) +  ((timeout_milli_secs % 1000) * 1000 * 1000);

  /* Check the nano sec overflow */
  if (wait_till_time.tv_nsec >= NANO_SEC ) {

      wait_till_time.tv_sec +=  wait_till_time.tv_nsec/NANO_SEC;
      wait_till_time.tv_nsec %= NANO_SEC;
  }

  while ((signal_ptr)->cond_predicate == FALSE)
  {
    if (pthread_cond_timedwait (&(signal_ptr)->cond_var,
                                &(signal_ptr)->cond_mutex,
                                &wait_till_time) == ETIMEDOUT)
    {
      rc = QMI_TIMEOUT_ERR;
      break;
    }
  }

  return rc;
}

static void *qmi_proxy_sglte_sm_thread_function
(
  void *ptr
)
{
  qmi_proxy_sglte_state_machine_type *sm = (qmi_proxy_sglte_state_machine_type *)ptr;
  int rc;
  int is_empty;
  qmi_proxy_sm_event_type *evt;
  QMI_PROXY_DEBUG_MSG("%s: Thread entry point\n", __FUNCTION__);

  for(;;)
  {
    QMI_PLATFORM_INIT_SIGNAL_FOR_WAIT(0, &sm->queue_signal);
    if (qmi_proxy_queue_is_empty(sm->msg_queue)) {
      rc = QMI_PLATFORM_WAIT_FOR_SIGNAL_NO_UNLOCK(0, &sm->queue_signal, 0x7fffffff);
      if (rc == QMI_TIMEOUT_ERR)
      {
        QMI_PROXY_DEBUG_MSG("%s: Timeout in receiving events. Retrying\n", __FUNCTION__);
        QMI_PLATFORM_FINISH_SIGNAL_WAIT(&sm->queue_signal);
        continue;
      }
    }

    QMI_PROXY_DEBUG_MSG("%s: Dequeuing message\n", __FUNCTION__);
    evt = qmi_proxy_queue_pop(sm->msg_queue);
    QMI_PLATFORM_FINISH_SIGNAL_WAIT(&sm->queue_signal);
    if (evt)
    {
      if (evt->id == QMI_PROXY_SGLTE_EVT_STOP)
      {
        QMI_PROXY_DEBUG_MSG("%s: Ending thread\n", __FUNCTION__);
        while (evt != NULL)
        {
          free(evt);
          evt = qmi_proxy_queue_pop(sm->msg_queue);
          if (evt)
          {
            QMI_PROXY_DEBUG_MSG("%s: Dequed pending event %s(%d)\n", __FUNCTION__, evt->name, evt->id);
          }
        }
        break;
      }
      else
      {
        qmi_proxy_sm_event_handler(sm->sm, evt);
        free(evt);
      }
    }
    else
    {
      QMI_PROXY_ERR_MSG("%s: Dequeued null message", __FUNCTION__);
    }
  }

  QMI_PROXY_DEBUG_MSG("%s: Thread ending\n", __FUNCTION__);
  return NULL;
}
int qmi_proxy_sglte_sm_start
(
  qmi_proxy_sglte_state_machine_type *sm
)
{
  int rc;

  QMI_PROXY_DEBUG_MSG("%s: Starting SGLTE SM\n", __FUNCTION__);
  QMI_PLATFORM_INIT_SIGNAL_FOR_SEND(0, &sm->queue_signal);
  rc = pthread_create(&sm->sm_thread, NULL, qmi_proxy_sglte_sm_thread_function, sm);
  QMI_PLATFORM_FINISH_SIGNAL_SEND(&sm->queue_signal);

  if (rc)
  {
    QMI_PROXY_ERR_MSG("%s: Unable to start thread for state machine\n", __FUNCTION__);
  }
  return rc;
}

static int qmi_proxy_sglte_sm_queue_event_locked
(
  qmi_proxy_sglte_state_machine_type *sm,
  int evt_id,
  const char *name,
  void *user
)
{

  qmi_proxy_sm_event_type *evt;
  int ret = QMI_NO_ERR;

  evt = calloc(1, sizeof(*evt));
  if (evt)
  {
    evt->id = evt_id;
    evt->name = name;
    evt->user_data = user;
    qmi_proxy_queue_push(sm->msg_queue, evt);
    QMI_PLATFORM_SEND_SIGNAL_LOCKED(0, &sm->queue_signal);
    //usleep(10);
  }
  else
  {
    QMI_PROXY_DEBUG_MSG("%s: Unable to allocate memory for event %d\n", __FUNCTION__, evt_id);
    ret = QMI_INTERNAL_ERR;
  }
  return ret;
}

int qmi_proxy_sglte_sm_queue_event
(
  qmi_proxy_sglte_state_machine_type *sm,
  int evt_id,
  const char *name,
  void *user
)
{
  int ret = QMI_NO_ERR;

  QMI_PROXY_DEBUG_MSG("%s: Queueing event %s (%d)\n", __FUNCTION__, name, evt_id);
  QMI_PLATFORM_INIT_SIGNAL_FOR_SEND(0,&sm->queue_signal);
  qmi_proxy_sglte_sm_queue_event_locked(sm, evt_id, name, user);
  QMI_PLATFORM_FINISH_SIGNAL_SEND(&sm->queue_signal);

  return ret;
}

int qmi_proxy_sglte_sm_stop
(
  qmi_proxy_sglte_state_machine_type *sm
)
{
  pthread_t thread;
  int ret = QMI_NO_ERR;
  QMI_PROXY_DEBUG_MSG("%s: Stopping state machine\n", __FUNCTION__);
  QMI_PLATFORM_INIT_SIGNAL_FOR_SEND(0, &sm->queue_signal);
  thread = sm->sm_thread;
  if (thread)
  {
    qmi_proxy_sglte_sm_queue_event_locked(sm, QMI_PROXY_SGLTE_EVT_STOP, "STOP", NULL);
    sm->sm_thread = 0;
    QMI_PLATFORM_FINISH_SIGNAL_SEND(&sm->queue_signal);
    pthread_join(thread, NULL);
    QMI_PROXY_DEBUG_MSG("%s: State machine stopped\n", __FUNCTION__);
  }
  else
  {
    QMI_PLATFORM_FINISH_SIGNAL_SEND(&sm->queue_signal);
    QMI_PROXY_DEBUG_MSG("%s: State machine was already stopped\n", __FUNCTION__);
  }
  return ret;
}
static void qmi_proxy_sglte_sm_destroy
(
  qmi_proxy_sglte_state_machine_type *sm
)
{
  if (sm)
  {
    qmi_proxy_sglte_sm_stop(sm);

    qmi_proxy_sm_delete(sm->sm);
    sm->sm = NULL;
    QMI_PLATFORM_DESTROY_SIGNAL_DATA(&sm->queue_signal);
    qmi_proxy_queue_delete(sm->msg_queue);
    sm->msg_queue = NULL;
    QMI_PLATFORM_MUTEX_DESTROY(&sm->msg_queue_lock);
  }
}

void qmi_proxy_sglte_sm_delete
(
  qmi_proxy_sglte_state_machine_type *sm
)
{
  if (sm)
  {
    qmi_proxy_sglte_sm_destroy(sm);
    free(sm);
  }
}

#ifdef TEST_QMI_PROXY_SM
void test_sglte_sm_power_up_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
}

void test_sglte_sm_sglte_mode_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
}

void test_sglte_sm_in_china_non_sglte_mode_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
}

void test_sglte_sm_non_sglte_coverage_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
}

void test_sglte_sm_complete_oos_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
}

void test_sglte_mode_substate_active_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
}

void test_sglte_mode_substate_trans_ps_to_g_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
}

void test_sglte_mode_substate_trans_ps_to_g_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
  if (evt_id == SGLTE_SM_EVENT_NO_SVC_ON_REMOTE)
  {
    /* Cancel any action we may have mode for ps movement */
    /* - Remote to CS Only */
    qmi_proxy_force_srv_domain_pref(QMI_PROXY_LOCAL_CONN_TYPE,
           QMI_SRV_DOMAIN_PREF_CS_ONLY_V01,
           TRUE);
    /* - Remove registration restriction on Local */
    qmi_proxy_force_srv_restriction(QMI_PROXY_LOCAL_CONN_TYPE,
           NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01);
  }
}

void test_sglte_mode_substate_ps_on_g_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
}

void test_sglte_mode_substate_trans_ps_to_lt_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
}

void test_sglte_mode_substate_trans_ps_to_lt_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
}

void test_sglte_mode_substate_no_svc_on_qsc_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *arg
)
{
  QMI_PROXY_DEBUG_MSG("%s\n", __FUNCTION__);
}

qmi_proxy_sm_event_type test_events_no_change_from_power_up[] =
{
  { .name = STRINGIZE(SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL),
    .id = SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL,
  },
  {
    .name = STRINGIZE(SGLTE_SM_EVENT_FULL_SVC_ON_REMOTE),
    .id = SGLTE_SM_EVENT_FULL_SVC_ON_REMOTE,
  },
};

#define TEST_EVENT_ENTRY(evt_id) \
  {  .name = STRINGIZE(evt_id), \
     .id = evt_id, \
  }
qmi_proxy_sm_event_type test_events_full[] =
{
  /* Test switching major modes */
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_NON_SGLTE_PLMN),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_NON_SGLTE_PLMN),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_SGLTE_PLMN_NON_SGLTE_MODE),
  /* Test SGLTE Mode */
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL),
  /* - Test Hysteresis timers */
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_NO_SVC_ON_LOCAL),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_NO_SVC_ON_LOCAL),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_PS_TO_LT_TIMER_EXPIRED),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_PS_TO_LT_TIMER_EXPIRED),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_PS_TO_G_TIMER_EXPIRED),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_PS_TO_G_TIMER_EXPIRED),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_PS_TO_LT_TIMER_EXPIRED),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_NO_SVC_ON_REMOTE),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_NO_SVC_ON_LOCAL),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_PS_TO_LT_TIMER_EXPIRED),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_NO_SVC_ON_LOCAL),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_NON_SGLTE_PLMN),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_SGLTE_PLMN_SGLTE_MODE),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_NO_SVC_ON_LOCAL),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_PS_TO_G_TIMER_EXPIRED),
  TEST_EVENT_ENTRY(SGLTE_SM_EVENT_NO_SVC_ON_REMOTE),
};

int main()
{
  int i;
  int events_to_send;
  qmi_proxy_sglte_state_machine_type *sm = qmi_proxy_sglte_sm_new();

  qmi_proxy_sglte_sm_start(sm);

  events_to_send = sizeof(test_events_no_change_from_power_up) / sizeof(test_events_no_change_from_power_up[0]);
  for (i = 0; i < events_to_send; i++)
  {
    qmi_proxy_sglte_sm_queue_event(sm,
            test_events_no_change_from_power_up[i].id,
            test_events_no_change_from_power_up[i].name,
            test_events_no_change_from_power_up[i].user_data);
  }

  QMI_PROXY_DEBUG_MSG("%s: Full test\n", __FUNCTION__);
  events_to_send = sizeof(test_events_full) / sizeof(test_events_full[0]);
  for (i = 0; i < events_to_send; i++)
  {
    qmi_proxy_sglte_sm_queue_event(sm,
            test_events_full[i].id,
            test_events_full[i].name,
            test_events_full[i].user_data);
  }
  pthread_join(sm->sm_thread, NULL);

  return 0;
}


#else

void sglte_sm_power_up_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
}

void sglte_sm_proc_lpm_req_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
}
void sglte_sm_sglte_coverage_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
}
void sglte_sm_sglte_coverage_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);

}
void sglte_sm_sglte_mode_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  int is_in_sglte_coverage = TRUE;
  int is_user_request = FALSE;
  int send_async = FALSE;
  qmi_proxy_conn_type cs_active_modem, ps_active_modem;
  int is_mode_pref_required = (evt_id != SGLTE_SM_EVENT_SGLTE_MODE);

  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) user;

  QMI_PLATFORM_MUTEX_LOCK(&qmi_proxy_internal_info.cache_mutex);
  qmi_proxy_update_cs_active_ps_active_sglte();
  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_proxy_internal_info.cache_mutex);
  qmi_proxy_get_cs_active_ps_active(&cs_active_modem, &ps_active_modem);
  qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode(
          cs_active_modem,
          ps_active_modem,
          QMI_PROXY_MODE_PREF_LTE_GSM_WCDMA,
          is_in_sglte_coverage,
          is_user_request,
          is_mode_pref_required,
          send_async);
}

void sglte_sm_sglte_coverage_non_sglte_mode_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  qmi_proxy_mode_pref_type sglte_user_mode;
  int is_in_sglte_coverage = TRUE;
  int is_user_request = FALSE;
  int send_async = FALSE;
  qmi_proxy_conn_type cs_active_modem, ps_active_modem;
  int is_mode_pref_required = (evt_id != SGLTE_SM_EVENT_NON_SGLTE_MODE);

  /* Supress compiler warnings for unused variables */
  (void) user;
  
  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);

  QMI_PLATFORM_MUTEX_LOCK(&qmi_proxy_internal_info.cache_mutex);
  qmi_proxy_update_cs_active_ps_active_sglte();
  qmi_proxy_get_cs_active_ps_active_locked(&cs_active_modem, &ps_active_modem);
  sglte_user_mode = qmi_proxy_internal_info.sglte_user_mode_pref;
  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_proxy_internal_info.cache_mutex);

  qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode(
          cs_active_modem,
          ps_active_modem,
          sglte_user_mode,
          is_in_sglte_coverage,
          is_user_request,
          is_mode_pref_required,
          send_async);

  sglte_validate_service_and_call_status(sm, evt_id);
}

void sglte_sm_full_svc_timer_expired
(
  void
)
{
  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_sglte_sm_queue_event(
          qmi_proxy_internal_info.sglte_sm,
          SGLTE_SM_EVENT_FULL_SVC_TIMER_EXPIRED,
          "SGLTE_SM_EVENT_FULL_SVC_TIMER_EXPIRED",
          NULL);
}

void sglte_sm_ps_to_g_timer_expired
(
  void
)
{
  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_sglte_sm_queue_event(
		  qmi_proxy_internal_info.sglte_sm,
          SGLTE_SM_EVENT_PS_TO_G_TIMER_EXPIRED,
          "SGLTE_SM_EVENT_PS_TO_G_TIMER_EXPIRED",
          NULL);
}

void sglte_sm_ps_to_lt_timer_expired
(
  void
)
{
  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_sglte_sm_queue_event(
		  qmi_proxy_internal_info.sglte_sm,
          SGLTE_SM_EVENT_PS_TO_LT_TIMER_EXPIRED,
          "SGLTE_SM_EVENT_PS_TO_LT_TIMER_EXPIRED",
          NULL);
}

void sglte_sm_sglte_coverage_non_sglte_mode_waiting_svc_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) user;
  (void) evt_id;
  (void) sm;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_create_timer(qmi_proxy_sglte_hystersis_timer_info.sglte_rat_timer_value,
          sglte_sm_full_svc_timer_expired);

}

void sglte_sm_sglte_coverage_non_sglte_mode_waiting_svc_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) user;
  (void) evt_id;
  (void) sm;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_cancel_timer();
}

void sglte_sm_non_sglte_coverage_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  qmi_proxy_mode_pref_type sglte_user_mode;
  int is_in_sglte_coverage = FALSE;
  int is_user_request = FALSE;
  int send_async = FALSE;
  qmi_proxy_conn_type cs_active_modem, ps_active_modem;
  int is_mode_pref_required = TRUE;

  /* Supress compiler warnings for unused variables */
  (void) user;
  (void) evt_id;
  (void) sm;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);

  QMI_PLATFORM_MUTEX_LOCK(&qmi_proxy_internal_info.cache_mutex);
  qmi_proxy_update_cs_active_ps_active_sglte();
  sglte_user_mode = qmi_proxy_internal_info.sglte_user_mode_pref;
  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_proxy_internal_info.cache_mutex);
  qmi_proxy_get_cs_active_ps_active(&cs_active_modem, &ps_active_modem);
  qmi_proxy_force_sglte_sys_sel_pref_and_oprt_mode(
          cs_active_modem,
          ps_active_modem,
          sglte_user_mode,
          is_in_sglte_coverage,
          is_user_request,
          is_mode_pref_required,
          send_async);
}

void sglte_sm_non_sglte_coverage_waiting_svc_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  qmi_proxy_mode_pref_type sglte_user_mode;

  /* Supress compiler warnings for unused variables */
  (void) user;
  (void) evt_id;
  (void) sm;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_create_timer(qmi_proxy_sglte_hystersis_timer_info.non_sglte_rat_timer_value,
          sglte_sm_full_svc_timer_expired);
}

void sglte_sm_non_sglte_coverage_waiting_svc_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user)
{
  /* Supress compiler warnings for unused variables */
  (void) user;
  (void) evt_id;
  (void) sm;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_cancel_timer();

}

void sglte_sm_complete_oos_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  qmi_proxy_mode_pref_type sglte_user_mode;
  nas_set_system_selection_preference_req_msg_v01 *local_set_sys_sel_pref_req;
  nas_set_system_selection_preference_resp_msg_v01 *local_set_sys_sel_pref_resp;
  nas_set_system_selection_preference_req_msg_v01 *remote_set_sys_sel_pref_req;
  nas_set_system_selection_preference_resp_msg_v01 *remote_set_sys_sel_pref_resp;
  qmi_proxy_conn_type cs_active;
  qmi_proxy_conn_type ps_active;
  int in_sglte_coverage;
  int rc;

  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) user;
  
  QMI_PROXY_DEBUG_MSG("%s (evt_id: %d)\n", __FUNCTION__, evt_id);

  QMI_PLATFORM_MUTEX_LOCK(&qmi_proxy_internal_info.cache_mutex);
  sglte_user_mode = qmi_proxy_internal_info.sglte_user_mode_pref;
  qmi_proxy_sglte_invalidate_last_plmns_locked();
  qmi_proxy_update_cs_active_ps_active_sglte();
  cs_active = qmi_proxy_internal_info.cs_active_modem;
  ps_active = qmi_proxy_internal_info.ps_active_modem;
  in_sglte_coverage = qmi_proxy_internal_info.is_in_sglte_coverage;
  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_proxy_internal_info.cache_mutex);

  if (in_sglte_coverage < 0)
  {
    local_set_sys_sel_pref_req = calloc(1, sizeof(*local_set_sys_sel_pref_req));
    local_set_sys_sel_pref_resp = calloc(1, sizeof(*local_set_sys_sel_pref_resp));

    remote_set_sys_sel_pref_req = calloc(1, sizeof(*remote_set_sys_sel_pref_req));
    remote_set_sys_sel_pref_resp = calloc(1, sizeof(*remote_set_sys_sel_pref_resp));

    if (local_set_sys_sel_pref_req && local_set_sys_sel_pref_resp &&
            remote_set_sys_sel_pref_req && remote_set_sys_sel_pref_resp)
    {
      rc = qmi_proxy_set_sys_sel_pref_per_sglte_mode_pref(
              sglte_user_mode,
              in_sglte_coverage,
              FALSE,
              local_set_sys_sel_pref_req,
              remote_set_sys_sel_pref_req );

      if (rc == QMI_NO_ERR)
      {
        if (cs_active == QMI_PROXY_LOCAL_CONN_TYPE || ps_active == QMI_PROXY_LOCAL_CONN_TYPE)
        {
          if (evt_id != SGLTE_SM_EVENT_ONLINE_IND)
          {
            qmi_proxy_force_online(QMI_PROXY_LOCAL_CONN_TYPE);
          }
          qmi_proxy_force_sys_sel_pref_sync(QMI_PROXY_LOCAL_CONN_TYPE,
                local_set_sys_sel_pref_req, local_set_sys_sel_pref_resp, 0x7fffffff);
        }

        if (cs_active == QMI_PROXY_REMOTE_CONN_TYPE ||
                  ps_active == QMI_PROXY_REMOTE_CONN_TYPE)
        {
          if (evt_id != SGLTE_SM_EVENT_ONLINE_IND)
          {
            qmi_proxy_force_online(QMI_PROXY_REMOTE_CONN_TYPE);
          }
          qmi_proxy_force_sys_sel_pref_sync(QMI_PROXY_REMOTE_CONN_TYPE,
                  remote_set_sys_sel_pref_req, remote_set_sys_sel_pref_resp, 0x7fffffff);
        }

      }
      else
      {
        QMI_PROXY_ERR_MSG("%s: Unexpected/Invalid mode pref: %d\n", __FUNCTION__, sglte_user_mode);
      }
    }
    else
    {
      QMI_PROXY_ERR_MSG("%s: Failed to allocate memory for requests and responses\n", __FUNCTION__);
    }
    free(local_set_sys_sel_pref_req);
    free(local_set_sys_sel_pref_resp);
    free(remote_set_sys_sel_pref_req);
    free(remote_set_sys_sel_pref_resp);
  }
  else
  {
    qmi_proxy_sglte_update_in_sglte_coverage();
  }
}

void sglte_validate_service_and_call_status
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);

  if (qmi_proxy_internal_info.is_dtm_supported[QMI_PROXY_REMOTE_CONN_TYPE] <= 0 &&
      qmi_proxy_internal_info.number_of_calls > 0)
  {
    qmi_proxy_sglte_sm_queue_event(qmi_proxy_internal_info.sglte_sm,
            SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM,
            "SGLTE_SM_EVENT_VOICE_CALL_ACTIVE",
            NULL);
  }

  if (!qmi_proxy_internal_info.is_remote_svc)
  {
    qmi_proxy_sglte_sm_queue_event(
            qmi_proxy_internal_info.sglte_sm,
            SGLTE_SM_EVENT_NO_SVC_ON_REMOTE,
            "SGLTE_SM_EVENT_NO_SVC_ON_REMOTE",
            NULL);
  }

  if (!qmi_proxy_internal_info.is_local_svc)
  {
    qmi_proxy_sglte_sm_queue_event(
            qmi_proxy_internal_info.sglte_sm,
            SGLTE_SM_EVENT_NO_SVC_ON_LOCAL,
            "SGLTE_SM_EVENT_NO_SVC_ON_LOCAL",
            NULL);
  }

}

void sglte_mode_substate_active_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  sglte_validate_service_and_call_status(sm, evt_id);
}

void sglte_mode_substate_active_waiting_svc_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_create_timer(
          qmi_proxy_sglte_hystersis_timer_info.sglte_rat_timer_value,
          sglte_sm_full_svc_timer_expired);
}

void sglte_mode_substate_active_waiting_svc_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_cancel_timer();
  sglte_validate_service_and_call_status(sm, evt_id);
}

void sglte_mode_substate_active_no_svc_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_create_timer(
          qmi_proxy_sglte_hystersis_timer_info.sglte_rat_timer_value,
          sglte_sm_full_svc_timer_expired);
}

void sglte_mode_substate_active_no_svc_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  if (SGLTE_SM_EVENT_FULL_SVC_TIMER_EXPIRED == evt_id)
  {
    /* Detach - This removes PS from domain pref as well */
    qmi_proxy_force_srv_domain_pref(QMI_PROXY_REMOTE_CONN_TYPE,
          QMI_SRV_DOMAIN_PREF_PS_DETACH_V01,
          FALSE);
  }

  qmi_proxy_cancel_timer();
}

void sglte_mode_substate_trans_ps_to_g_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
}

void sglte_mode_substate_trans_ps_to_g_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
}

void trans_ps_to_g_timer_started_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  /* Start timer */
  qmi_proxy_create_timer(
          qmi_proxy_sglte_hystersis_timer_info.hyst_ps_to_g_timer_value,
          sglte_sm_ps_to_g_timer_expired);
}
void trans_ps_to_g_timer_started_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  /* Stop Timer */
  qmi_proxy_cancel_timer();
}

int qmi_proxy_force_srv_domain_pref
(
  qmi_proxy_conn_type conn_type,
  nas_srv_domain_pref_enum_type_v01 srv_domain_pref,
  boolean persistent
)
{
  nas_set_system_selection_preference_req_msg_v01 *set_sys_sel_pref_req;
  nas_set_system_selection_preference_resp_msg_v01 *set_sys_sel_pref_resp;
  int rc = QMI_INTERNAL_ERR;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);

  set_sys_sel_pref_req = calloc(1, sizeof(*set_sys_sel_pref_req));
  set_sys_sel_pref_resp = calloc(1, sizeof(*set_sys_sel_pref_resp));

  if (set_sys_sel_pref_req && set_sys_sel_pref_resp)
  {
    /* Send camp-only request to local */
    set_sys_sel_pref_req->srv_domain_pref_valid = TRUE;
    set_sys_sel_pref_req->srv_domain_pref = srv_domain_pref;

    /* If change duration is not present, qmi_nas will set it to PERMANENT */
    set_sys_sel_pref_req->change_duration_valid = !persistent;
    set_sys_sel_pref_req->change_duration = NAS_POWER_CYCLE_V01;

    rc = qmi_proxy_force_sys_sel_pref_sync(conn_type,
            set_sys_sel_pref_req,
            set_sys_sel_pref_resp,
            QMI_PROXY_SYNC_REQ_TIMEOUT);

    if (rc != QMI_NO_ERR)
    {
      QMI_PROXY_ERR_MSG("Error sending request: %d", rc);
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s: Allocation failed\n", __FUNCTION__);
  }
  free(set_sys_sel_pref_req);
  free(set_sys_sel_pref_resp);

  return rc;
}

int qmi_proxy_force_srv_restriction
(
  qmi_proxy_conn_type conn_type,
  nas_srv_reg_restriction_enum_v01 srv_restriction,
  boolean persistent
)
{
  nas_set_system_selection_preference_req_msg_v01 *set_sys_sel_pref_req;
  nas_set_system_selection_preference_resp_msg_v01 *set_sys_sel_pref_resp;
  int rc = QMI_INTERNAL_ERR;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);

  set_sys_sel_pref_req = calloc(1, sizeof(*set_sys_sel_pref_req));
  set_sys_sel_pref_resp = calloc(1, sizeof(*set_sys_sel_pref_resp));

  if (set_sys_sel_pref_req && set_sys_sel_pref_resp)
  {
    /* Send camp-only request to local */
    set_sys_sel_pref_req->srv_reg_restriction_valid = TRUE;
    set_sys_sel_pref_req->srv_reg_restriction = srv_restriction;

    /* If change duration is not present, qmi_nas will set it to PERMANENT */
    set_sys_sel_pref_req->change_duration_valid = !persistent;
    set_sys_sel_pref_req->change_duration = NAS_POWER_CYCLE_V01;

    rc = qmi_proxy_force_sys_sel_pref_sync(conn_type,
            set_sys_sel_pref_req,
            set_sys_sel_pref_resp,
            QMI_PROXY_SYNC_REQ_TIMEOUT);

    if (rc != QMI_NO_ERR)
    {
      QMI_PROXY_ERR_MSG("Error sending request: %d", rc);
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s: Allocation failed\n", __FUNCTION__);
  }
  free(set_sys_sel_pref_req);
  free(set_sys_sel_pref_resp);

  return rc;
}

void trans_ps_to_g_setting_camp_only_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  int rc;

  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_force_srv_restriction(QMI_PROXY_LOCAL_CONN_TYPE,
          NAS_SRV_REG_RESTRICTION_CAMPED_ONLY_V01,
          FALSE);
}

void trans_ps_to_g_setting_camp_only_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  int rc;

  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  if (evt_id == SGLTE_SM_EVENT_FULL_SVC_ON_LOCAL ||
      evt_id == SGLTE_SM_EVENT_CAMP_ONLY_SVC_ON_LOCAL ||
      evt_id == SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM)
  {
    qmi_proxy_force_srv_restriction(QMI_PROXY_LOCAL_CONN_TYPE,
            NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01,
            FALSE);
  }
}

void trans_ps_to_g_moving_ps_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);

  /* Set CS+PS on remote - not persistently*/
  qmi_proxy_force_srv_domain_pref(QMI_PROXY_REMOTE_CONN_TYPE,
          QMI_SRV_DOMAIN_PREF_CS_PS_V01,
          FALSE);

  /* Perform local detach on LOCAL */
  qmi_proxy_force_srv_domain_pref(QMI_PROXY_LOCAL_CONN_TYPE,
          QMI_SRV_DOMAIN_PREF_PS_DETACH_NO_PREF_CHANGE_V01,
          FALSE);
}

void trans_ps_to_g_moving_ps_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  if (evt_id == SGLTE_SM_EVENT_VOICE_CALL_ACTIVE_NO_DTM)
  {
    qmi_proxy_force_srv_domain_pref(QMI_PROXY_REMOTE_CONN_TYPE,
            QMI_SRV_DOMAIN_PREF_PS_DETACH_V01,
            FALSE);
    qmi_proxy_force_srv_restriction(QMI_PROXY_REMOTE_CONN_TYPE,
            NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01,
            FALSE);
  }
}

void sglte_mode_substate_ps_on_g_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  //qmi_proxy_sglte_ps_to_g_timer_exp_handler();
}

void sglte_mode_substate_trans_ps_to_lt_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
}

void sglte_mode_substate_trans_ps_to_lt_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
}

void trans_ps_to_lt_timer_started_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_create_timer(
          qmi_proxy_sglte_hystersis_timer_info.hyst_ps_to_lt_timer_value,
          sglte_sm_ps_to_lt_timer_expired);
}

void trans_ps_to_lt_timer_started_leave
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_cancel_timer();
}

void trans_ps_to_lte_moving_ps_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  /* Detach - This removes PS from domain pref as well */
  qmi_proxy_force_srv_domain_pref(QMI_PROXY_REMOTE_CONN_TYPE,
          QMI_SRV_DOMAIN_PREF_PS_DETACH_V01,
          FALSE);
  if (!qmi_proxy_internal_info.is_remote_svc)
  {
    qmi_proxy_sglte_sm_queue_event(
            qmi_proxy_internal_info.sglte_sm,
            SGLTE_SM_EVENT_PS_INACTIVE_ON_REMOTE,
            "SGLTE_SM_EVENT_PS_INACTIVE_ON_REMOTE",
            NULL);
  }
}

void trans_ps_to_lt_removing_camp_only_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
  qmi_proxy_force_srv_restriction(QMI_PROXY_LOCAL_CONN_TYPE,
          NAS_SRV_REG_RESTRICTION_UNRESTRICTED_V01,
          FALSE);
}

void sglte_mode_substate_no_svc_on_qsc_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
}

void sglte_mode_substate_no_svc_on_local_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
}

void sglte_mode_voice_on_remote_no_dtm_enter
(
  qmi_proxy_state_machine_type *sm,
  unsigned evt_id,
  void *user
)
{
  /* Supress compiler warnings for unused variables */
  (void) sm;
  (void) evt_id;
  (void) user;

  QMI_PROXY_DEBUG_MSG("%s", __FUNCTION__);
}
#endif
