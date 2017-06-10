/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "qmi_proxy_sm.h"

struct qmi_proxy_state_machine_type
{
  unsigned initial_state;
  unsigned *current_state_config;
  int max_state_depth;
  int current_state_depth;
  unsigned nstates;
  const qmi_proxy_sm_state_type *state_table;
  unsigned nevents;
  const unsigned **transition_table;
};

int qmi_proxy_sm_state_config_valid(
  qmi_proxy_state_machine_type *sm
)
{
  int i;
  int ret = TRUE;
  if (sm->current_state_depth < sm->max_state_depth)
  {
    for (i = 0; i < sm->current_state_depth; i++)
    {
      if (!QMI_PROXY_SM_STATE_VALID(sm->current_state_config[i],sm->nstates))
      {
        ret = FALSE;
      }
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s: Invalid depth current state depth %d",__FUNCTION__, sm->current_state_depth);
    ret = FALSE;
  }

  return ret;
}

int qmi_proxy_sm_get_state_depth
(
  qmi_proxy_state_machine_type *sm,
  unsigned id
)
{
  int ret = -1;
  const qmi_proxy_sm_state_type *tmp;
  unsigned tmp_id;

  tmp_id = id;
  tmp = &sm->state_table[tmp_id];
  for (; QMI_PROXY_SM_STATE_VALID(tmp_id, sm->nstates) ;
          tmp_id = tmp->parent_id, tmp = &sm->state_table[tmp_id])
  {
    ret ++;
  }

  return ret;
}

unsigned qmi_proxy_sm_find_common_ancestor
(
  qmi_proxy_state_machine_type *sm,
  unsigned id1,
  int depth1,
  unsigned id2,
  int depth2
)
{
  unsigned tmp_id1, tmp_id2;
  unsigned ret = 0;
  const qmi_proxy_sm_state_type *tmp1, *tmp2;

  if (depth1 >= 0 && depth2 >= 0 &&
          QMI_PROXY_SM_STATE_VALID(id1, sm->nstates) &&
          QMI_PROXY_SM_STATE_VALID(id2, sm->nstates))
  {
    tmp_id1 = id1;
    tmp1 = &sm->state_table[tmp_id1];
    tmp_id2 = id2;
    tmp2 = &sm->state_table[tmp_id2];

    if (depth1 > depth2)
    {
      for (; depth1 > depth2; depth1--)
      {
        tmp_id1 = tmp1->parent_id;
        tmp1 = &sm->state_table[tmp_id1];
      }
    }
    else if (depth1 < depth2)
    {
      for (; depth2 > depth1; depth2 --)
      {
        tmp_id2 = tmp2->parent_id;
        tmp2 = &sm->state_table[tmp_id2];
      }
    }

    for (; depth1 >= 0 && tmp_id1 != tmp_id2; depth1--)
    {
      tmp_id1 = tmp1->parent_id;
      tmp1 = &sm->state_table[tmp_id1];
      tmp_id2 = tmp2->parent_id;
      tmp2 = &sm->state_table[tmp_id2];
    }
    ret = tmp_id1;
  }

  return ret;
}
void qmi_proxy_sm_event_handler
(
  qmi_proxy_state_machine_type *sm,
  qmi_proxy_sm_event_type *evt
)
{
  unsigned current_id;
  const qmi_proxy_sm_state_type *current;
  unsigned next_id;
  const qmi_proxy_sm_state_type *next;
  int next_depth;
  unsigned common_ancestor_id;
  const qmi_proxy_sm_state_type *tmp;
  unsigned tmp_id;
  int i,j;
  int inconsistent = FALSE;

  if (sm && evt && evt->id > 0 && evt->id < sm->nevents)
  {
    QMI_PROXY_DEBUG_MSG("%s: State %s(%d) (depth %d) Processing Event %s (%d)\n",
            __FUNCTION__,
            sm->state_table[sm->current_state_config[sm->current_state_depth]].name,
            sm->current_state_config[sm->current_state_depth],
            sm->current_state_depth,
            evt->name,
            evt->id);
    if (qmi_proxy_sm_state_config_valid(sm))
    {
      tmp_id = current_id = sm->current_state_config[sm->current_state_depth];
      tmp = current = &sm->state_table[current_id];

      /* Find the next state as per the transition table */
      next_id = sm->transition_table[sm->current_state_config[sm->current_state_depth]][evt->id];
      next = &sm->state_table[next_id];

      while (QMI_PROXY_SM_STATE_VALID(tmp->parent_id, sm->nstates) && !QMI_PROXY_SM_STATE_VALID(next_id, sm->nstates))
      {
        /* State wouldn't process event. Give parents a chance */
        tmp_id = tmp->parent_id;
        tmp = &sm->state_table[tmp_id];
        next_id = sm->transition_table[tmp_id][evt->id];
        next = &sm->state_table[next_id];
      }
      while (QMI_PROXY_SM_STATE_VALID(next_id, sm->nstates) && QMI_PROXY_SM_STATE_VALID(next->initial_substate, sm->nstates))
      {
        /* State has a valid initial_substate. Use that as next state */
        next_id = next->initial_substate;
        next = &sm->state_table[next_id];
      }

      /* Process the transition only if the next state is valid,
       * and different from current state*/
      if (QMI_PROXY_SM_STATE_VALID(next_id, sm->nstates) && next_id != current_id)
      {
        next_depth = qmi_proxy_sm_get_state_depth(sm, next_id);
        common_ancestor_id = qmi_proxy_sm_find_common_ancestor(sm, current_id, sm->current_state_depth, next_id, next_depth);
        /* Leave current state, up to but excluding the common ancestor */
        tmp = current;
        tmp_id = current_id;
        for (i = sm->current_state_depth; i >= 0 && tmp_id != common_ancestor_id; i --,tmp_id = tmp->parent_id, tmp = &sm->state_table[tmp_id])
        {
          QMI_PROXY_DEBUG_MSG("%s: Leaving state %d. depth: %d\n", __FUNCTION__, tmp_id, sm->current_state_depth);
          if (tmp->leave)
          {
            tmp->leave(sm, evt->id, (void *)((unsigned long)next->id));
          }
          sm->current_state_config[i] = 0;
          sm->current_state_depth --;
        }
        /* Update state configuration */
        tmp_id = next_id;
        tmp = next;
        for (j = next_depth; j > i; j--)
        {
          /* Sanity test. current_state_config[j] must have been cleared by the leave process */
          if (sm->current_state_config[j] == 0)
          {
            sm->current_state_config[j] = tmp_id;
            tmp_id = tmp->parent_id;
            tmp = &sm->state_table[tmp_id];
          }
          else
          {
            QMI_PROXY_ERR_MSG("%s: Inconsistency found. Leave seems to have not cleared the states", __FUNCTION__);
            break;
          }
        }

        if (!inconsistent)
        {
          sm->current_state_depth = next_depth;
          /* Enter new state */
          for (j++; j<= next_depth; j++)
          {
            tmp = &sm->state_table[sm->current_state_config[j]];
            QMI_PROXY_DEBUG_MSG("%s: Entering state %d\n", __FUNCTION__, sm->current_state_config[j]);
            if (tmp->enter)
            {
              tmp->enter(sm, evt->id, evt->user_data);
            }
          }
        }
      }

    }
    else
    {
      QMI_PROXY_ERR_MSG("%s: Invalid current state config [depth: %d]\n", __FUNCTION__, sm->current_state_depth);
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s: Invalid arguments passed\n", __FUNCTION__);
  }
}

qmi_proxy_state_machine_type *qmi_proxy_sm_new
(
  qmi_proxy_sm_state_type *state_table,
  int nstates,
  unsigned *transition_table,
  int nevents,
  unsigned initial_state
)
{
  qmi_proxy_state_machine_type *ret = NULL;
  int rc;

  ret = calloc(1, sizeof(qmi_proxy_state_machine_type));
  if (ret)
  {
    rc = qmi_proxy_sm_init(ret, state_table, nstates, transition_table, nevents, initial_state);
    if (rc != QMI_NO_ERR)
    {
      QMI_PROXY_ERR_MSG("%s: Error initializing state machine", __FUNCTION__);
      free(ret);
      ret = NULL;
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s: Unable to allocate memory for state machine", __FUNCTION__);
  }

  return ret;
}

void qmi_proxy_sm_delete
(
  qmi_proxy_state_machine_type *sm
)
{
  free(sm->transition_table);
  free(sm);
}

int qmi_proxy_sm_init
(
  qmi_proxy_state_machine_type *sm,
  const qmi_proxy_sm_state_type *state_table,
  int nstates,
  const unsigned *transition_table,
  int nevents,
  unsigned initial_state
)
{
  const unsigned **transition_ptrs;
  int i;
  int init_depth;
  unsigned tmp_id;
  const qmi_proxy_sm_state_type *tmp;
  int max_depth = -1;
  int cur_depth = -1;;
  int ret = QMI_NO_ERR;

  transition_ptrs = calloc(nstates, sizeof(*transition_ptrs));

  if (sm && transition_ptrs)
  {
    sm->nstates = nstates;
    sm->state_table = state_table;
    sm->nevents = nevents;
    sm->transition_table = transition_ptrs;
    sm->initial_state = initial_state;

    for (i = 0; i < nstates; i++)
    {
      transition_ptrs[i] = &transition_table[i * nevents];
      cur_depth = qmi_proxy_sm_get_state_depth(sm, i);
      if ( cur_depth > max_depth)
      {
        max_depth = cur_depth;
      }
    }

    sm->max_state_depth = ++max_depth;

    QMI_PROXY_DEBUG_MSG("%s: Max depth for state machine: %d\n", __FUNCTION__, max_depth);
    sm->current_state_config = calloc(max_depth, sizeof(unsigned));
    if (sm->current_state_config)
    {
      init_depth = qmi_proxy_sm_get_state_depth(sm, initial_state);
      tmp_id = initial_state;
      tmp = &sm->state_table[initial_state];
      for (i = init_depth; i >= 0; i--)
      {
        sm->current_state_config[i] = tmp_id;
        tmp_id = tmp->parent_id;
        tmp = &sm->state_table[tmp_id];
      }
      /* Enter initial state */
      for (i = 0; i <= init_depth; i++)
      {
        tmp = &sm->state_table[sm->current_state_config[i]];
        QMI_PROXY_DEBUG_MSG("%s: Entering state %d\n", __FUNCTION__, sm->current_state_config[i]);
        if (tmp->enter)
        {
          tmp->enter(sm, 0, NULL);
        }
      }
    }
    else
    {
      QMI_PROXY_ERR_MSG("%s: Unable to allocate memory for state machine", __FUNCTION__);
      ret = QMI_INTERNAL_ERR;
    }
  }
  else
  {
    QMI_PROXY_ERR_MSG("%s: Unable to allocate memory for state machine", __FUNCTION__);
    ret = QMI_INTERNAL_ERR;
  }
  return ret;
}
