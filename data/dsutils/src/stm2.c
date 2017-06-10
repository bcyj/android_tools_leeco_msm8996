/*!
  @file
  stm2.c

  @brief
  This is the core module for the STM state machine framework.

  @detail
  @ref stm_introduction "Click here for an introduction to the STM framework"

*/

/*===========================================================================

  Copyright (c) 2007-2008,2014 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: /local/mnt/workspace/randrew/7x30_1/vendor/qcom-proprietary/data/common/src/RCS/stm2.c,v 1.1 2010/02/17 23:56:34 randrew Exp $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/12/08   trc     Rename to stm2.c to allow legacy coexistence w/ old stm.c
02/26/08   trc     Refactor SM/state entry/exit function calling
08/28/07   trc     Add pre-transition fn debugging hook
08/07/07   trc     Add code for new calling SM param in stm_process_input()
04/23/07   trc     Add STM_ASSERT() macro
04/18/07   trc     Add state parameter to stm_get_child(), don't register
                   unhandled inputs as last input
04/03/07   trc     Add stm_get_uid() for unique SM instance ID retrieval
04/02/07   trc     Refactor state machine structure into non-const, all-instance
                   const, and per-instance const structures for better static
                   initialization and 'reset button' properties
02/28/07   trc     Enforce definition of STM_NULL_CHECK() macro and use it
02/27/07   trc     Lint fixes
02/26/07   trc     Code review changes.  Move variant code to stm_os.h
02/06/07   trc     Initial revision
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

/* Variant target/operating-system specific include file */
#include "stm2_os.h"

/* STM external API include file */
#include "stm2.h"


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/* Make sure that STM_ERROR_MSG() macro is defined */
#ifndef STM_ERROR_MSG
#error STM_ERROR_MSG() must be defined in stm_os.h
#endif

/* Make sure that STM_NULL_CHECK() macro is defined */
#ifndef STM_NULL_CHECK
#error STM_NULL_CHECK() must be defined in stm_os.h
#endif

/* Make sure that STM_ASSERT() macro is defined */
#ifndef STM_ASSERT
#error STM_ASSERT() must be defined in stm_os.h
#endif

/* Make sure that STM_INIT_CRITICAL_SECTION() macro is defined */
#ifndef STM_INIT_CRITICAL_SECTION
#error STM_INIT_CRITICAL_SECTION() must be defined in stm_os.h
#endif

/* Make sure that STM_ENTER_CRITICAL_SECTION() macro is defined */
#ifndef STM_ENTER_CRITICAL_SECTION
#error STM_ENTER_CRITICAL_SECTION() must be defined in stm_os.h
#endif

/* Make sure that STM_EXIT_CRITICAL_SECTION() macro is defined */
#ifndef STM_EXIT_CRITICAL_SECTION
#error STM_EXIT_CRITICAL_SECTION() must be defined in stm_os.h
#endif


/*! @brief Error reporting macro that captures the current file, line number,
           and state machine pointer.  The state machine structure contains
           the error callback function, which is called with the aforementioned
           information.  If the state machine pointer is NULL or the error
           function is NULL, the general STM_ERROR_MSG() macro is invoked.
*/
#define STM_ERROR_NOTIFY(pSM,errormsg)                                        \
  if ( pSM->pi_const_data->const_data->error_fn != NULL )                     \
  {                                                                           \
    pSM->pi_const_data->const_data->error_fn(errormsg,__FILE__,__LINE__,pSM); \
  } else                                                                      \
  {                                                                           \
    STM_ERROR_MSG("NULL error function pointer");                             \
  }

/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/

static stm_status_t stm_acquire_lock(stm_state_machine_t *sm);
static void stm_release_lock(stm_state_machine_t *sm);
static int32 stm_lookup_input(const stm_state_machine_t *sm, stm_input_t input);


/*===========================================================================

                       INTERNAL FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  stm_acquire_lock

===========================================================================*/
/*!
    @brief
    Internal STM function to acquire the lock on a specific state machine
    instance.  Syncronization is performed using OS-specific
    STM_ENTER_CRITICAL_SECTION() and STM_EXIT_CRITICAL_SECTION() macros.

    @return
    Returns stm_status_t success or error code

    @retval STM_SUCCESS
    Lock was successfully acquired

    @retval STM_EBUSY
    State machine instance is busy, lock could not be acquired

    @see STM_ENTER_CRITICAL_SECTION and STM_EXIT_CRITICAL_SECTION

    @see stm_release_lock

*/
/*=========================================================================*/
static stm_status_t stm_acquire_lock
(
  stm_state_machine_t *sm /*!< Pointer to state machine instance to be locked */
)
{

  stm_status_t status = STM_EBUSY;

  STM_INIT_CRITICAL_SECTION();

  /*-----------------------------------------------------------------------*/

  /* Make sure we were handed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);

  /*-----------------------------------------------------------------------*/

  /* Enter critical section */
  STM_ENTER_CRITICAL_SECTION();

  /* See if we can grab the lock */
  if (!sm->is_locked)
  {
    /* Got it! */
    sm->is_locked = TRUE;
    status = STM_SUCCESS;
  }

  /* Leave critical section */
  STM_EXIT_CRITICAL_SECTION();

  /* return our status to the caller */
  return(status);

} /* stm_acquire_lock() */


/*===========================================================================

  FUNCTION:  stm_release_lock

===========================================================================*/
/*!
    @brief
    Internal STM function to release the lock on a specific state machine
    instance acquired via stm_acquire_lock().  Syncronization is performed
    using OS-specific STM_ENTER_CRITICAL_SECTION() and
    STM_EXIT_CRITICAL_SECTION() macros.

    @return
    No return value

    @see STM_ENTER_CRITICAL_SECTION and STM_EXIT_CRITICAL_SECTION

    @see stm_acquire_lock

*/
/*=========================================================================*/
static void stm_release_lock
(
  stm_state_machine_t *sm /*!< Pointer to state mach. instance to be unlocked */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were handed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);

  /*-----------------------------------------------------------------------*/

  /* Enter critical section */
  STM_ENTER_CRITICAL_SECTION();

  /* Release the lock, don't bother checking anything */
  sm->is_locked = FALSE;

  /* Leave critical section */
  STM_EXIT_CRITICAL_SECTION();

  /* return to the caller */
  return;

} /* stm_release_lock() */


/*===========================================================================

  FUNCTION:  stm_lookup_input

===========================================================================*/
/*!
    @brief
    Internal STM function to look up an arbitrary input value in a given
    state machine's linear table of known inputs.

    @return
    Index of the input value in the state machine instance's input_map_array[]
    table

    @retval STM_INVALID_INDEX
    Upon failure to find the specific input in the state machine instance's
    input_map_array[] table, this invalid index value will be returned.

*/
/*=========================================================================*/
static int32 stm_lookup_input
(
  const stm_state_machine_t *sm,   /*!< State machine instance pointer */
  stm_input_t               input  /*!< State machine input to be indexed */
)
{
  uint32 idx;
  int32  return_idx = (int32)STM_INVALID_INDEX;
  const stm_state_machine_constdata_t *sm_cdata;

  /*-----------------------------------------------------------------------*/

  /* Make sure we were handed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* We'll be using this pointer a lot, so take a local copy */
  sm_cdata = sm->pi_const_data->const_data;

  /* Spin through the input array of the state machine instance and look
     for an input that matches the one passed to us. */
  for (idx = 0; idx < sm_cdata->num_inputs; idx++)
  {
    /* Check if we've got a match */
    if (sm_cdata->input_map_array[idx].input == input)
    {
      /* Got it, save it and bail */
      return_idx = (int32)idx;
      break;
    }
  }

  /* Return what we found, if anything */
  return(return_idx);

} /* stm_lookup_input() */


/*===========================================================================

  FUNCTION:  stm_call_sm_entry

===========================================================================*/
/*!
    @brief
    Call the state machine's entry function, if it exists.

    @return
    None

*/
/*=========================================================================*/
static void stm_call_sm_entry
(
  stm_state_machine_t *sm,      /*!< State machine instance pointer */
  void                *payload  /*!< Payload parameter */
)
{
  const stm_state_machine_constdata_t *sm_cdata;

  /*-----------------------------------------------------------------------*/

  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* We use the const_data structure a lot, keep a local copy */
  sm_cdata = sm->pi_const_data->const_data;

  /* Ensure that the SM has a valid entry function before calling it */
  if (sm_cdata->entry_fn != NULL)
  {
    sm_cdata->entry_fn(sm,payload);

    /* If the debug hook function is valid, call it also */
    if (sm_cdata->debug_hook_fn != NULL)
    {
      sm_cdata->debug_hook_fn(STM_ENTRY_FN,sm,sm->current_state,payload);
    }
  }

} /* stm_call_sm_entry() */


/*===========================================================================

  FUNCTION:  stm_call_sm_exit

===========================================================================*/
/*!
    @brief
    Call the state machine's exit function, if it exists

    @return
    None

*/
/*=========================================================================*/
static void stm_call_sm_exit
(
  stm_state_machine_t *sm,      /*!< State machine instance pointer */
  void                *payload  /*!< Payload parameter */
)
{
  const stm_state_machine_constdata_t *sm_cdata;

  /*-----------------------------------------------------------------------*/

  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* We use the const_data structure a lot, keep a local copy */
  sm_cdata = sm->pi_const_data->const_data;

  /* Ensure that the SM has a valid exit function before calling it */
  if (sm_cdata->exit_fn != NULL)
  {
    sm_cdata->exit_fn(sm,payload);

    /* If the debug hook function is valid, call it also */
    if (sm_cdata->debug_hook_fn != NULL)
    {
      sm_cdata->debug_hook_fn(STM_EXIT_FN,sm,sm->current_state,payload);
    }
  }

} /* stm_call_sm_exit() */


/*===========================================================================

  FUNCTION:  stm_call_state_entry

===========================================================================*/
/*!
    @brief
    Call the state entry function for the passed state machine.

    @return
    None

*/
/*=========================================================================*/
static void stm_call_state_entry
(
  stm_state_machine_t *sm,        /*!< State machine instance pointer */
  stm_state_t         prev_state, /*!< Previous state */
  void                *payload    /*!< Payload parameter */
)
{
  const stm_state_machine_constdata_t *sm_cdata;

  /*-----------------------------------------------------------------------*/

  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* We use the const_data structure a lot, keep a local copy */
  sm_cdata = sm->pi_const_data->const_data;

  /* Ensure that the state has a valid entry function before calling it */
  if (sm_cdata->state_map_array[sm->current_state].entry_fn != NULL)
  {
    sm_cdata->state_map_array[sm->current_state].entry_fn(sm,
                                                          prev_state,
                                                          payload);
    /* If the debug hook function is valid, call it also */
    if (sm_cdata->debug_hook_fn != NULL)
    {
      sm_cdata->debug_hook_fn(STM_STATE_ENTRY_FN,
                              sm,
                              prev_state,
                              payload);
    }
  }

} /* stm_call_state_entry() */


/*===========================================================================

  FUNCTION:  stm_call_state_exit

===========================================================================*/
/*!
    @brief
    Call the state exit function for the passed state machine.

    @return
    None

*/
/*=========================================================================*/
static void stm_call_state_exit
(
  stm_state_machine_t *sm,        /*!< State machine instance pointer */
  stm_state_t         next_state, /*!< Next state */
  void                *payload    /*!< Payload parameter */
)
{
  const stm_state_machine_constdata_t *sm_cdata;

  /*-----------------------------------------------------------------------*/

  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* We use the const_data structure a lot, keep a local copy */
  sm_cdata = sm->pi_const_data->const_data;

  /* Ensure that the state has a valid exit function before calling it */
  if (sm_cdata->state_map_array[sm->current_state].exit_fn != NULL)
  {
    sm_cdata->state_map_array[sm->current_state].exit_fn(sm,
                                                         next_state,
                                                         payload);
    /* If the debug hook function is valid, call it also */
    if (sm_cdata->debug_hook_fn != NULL)
    {
      sm_cdata->debug_hook_fn(STM_STATE_EXIT_FN,
                              sm,
                              next_state,
                              payload);
    }
  }

} /* stm_call_state_exit() */


/*===========================================================================

                       EXTERNAL FUNCTIONS

===========================================================================*/


/*===========================================================================

  FUNCTION:  stm_instance_activate

===========================================================================*/
/*!
    @brief
    Activates a specified state machine instance, given a state machine array
    pointer and instance number.

    @see stm_activate for more information

*/
/*=========================================================================*/
stm_status_t stm_instance_activate
(
  stm_state_machine_t *sm,         /*!< State machine instance array pointer */
  uint32              sm_instance, /*!< State machine instance number */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_machine_t *sm_inst;

  /*-----------------------------------------------------------------------*/

  /* Make sure we have a valid state machine instance array pointer */
  STM_NULL_CHECK(sm);

  /* Make sure the instance passed is valid for the given state machine array */
  sm_inst = stm_get_instance(sm,sm_instance);
  if (sm_inst == NULL)
  {
    STM_ERROR_NOTIFY(sm,STM_EBADINSTANCE);
    return(STM_EBADINSTANCE);
  }

  /*-----------------------------------------------------------------------*/

  /* Call stm_process_input() on the instance and return the result */
  return(stm_activate(sm_inst,payload));

} /* stm_instance_activate() */


/*===========================================================================

  FUNCTION:  stm_activate

===========================================================================*/
/*!
    @brief
    Activates a specified state machine instance.

    @detail
    Activation of a deactivated state machine instance happens in a top-down
    manner, in the following sequence.
    -# State machine instance's (optional) entry function is called
    -# State machine instance is set to its initial state and that state's
       (optional) entry function is called
    -# If the initial state has a child state machine instance, stm_activate()
       is called on it, repeating the above steps, ad infinitum until there
       are no more child state machine instances to be activated.

    The payload pointer parameter is passed to all of the state and state
    machine instance's entry functions.

    Upon encountering an error in the activation of a state machine instance,
    everything regarding the activation is 'undone' (ie. deactivated) prior
    to returning the relevant error code to the caller.

    @return
    stm_status_t success/error code

    @retval STM_SUCCESS
    Upon successful state machine instance activation

    @note
    Calling stm_activate on an already activated state machine instance has
    no effect, and simply returns STM_SUCCESS.

    @see stm_deactivate for deactivation details

*/
/*=========================================================================*/
stm_status_t stm_activate
(
  stm_state_machine_t *sm,      /*!< State machine instance pointer */
  void                *payload  /*!< Payload */
)
{
  stm_status_t status = STM_SUCCESS;
  const stm_state_machine_constdata_t *sm_cdata;

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /* Make sure the state machine instance isn't busy/in-transition */
  if ( stm_acquire_lock(sm) != STM_SUCCESS )
  {
    STM_ERROR_NOTIFY(sm,STM_EBUSY);
    return(STM_EBUSY);
  }

  /* Make sure the state machine instance isn't already active */
  if ( sm->current_state != (stm_state_t)STM_DEACTIVATED_STATE )
  {
    /* Unlock it and return success (since it's already active) */
    stm_release_lock(sm);
    return(STM_SUCCESS);
  }

  /*-----------------------------------------------------------------------*/

  /* Note - at this point, we know that the current state machine instance's
     state is STM_DEACTIVATED_STATE, due to the check above */

  /* The strategy here is to first activate this state machine instance,
     then active its initial state.  Since the initial state may also contain
     another state machine, we then repeat the process until we have reached
     the point where there is are no more state machine instances to activate.

     If a child state machine instance fails to activate, something is wrong,
     so we need to roll-back the activation such that we leave nobody,
     including ourselves, active after this call returns.
  */

  /* We'll be using this pointer a lot, so take a local copy */
  sm_cdata = sm->pi_const_data->const_data;

  /* Set the current input index to STM_INVALID_INDEX to represent no input
     received yet */
  sm->curr_input_index = (int32)STM_INVALID_INDEX;

  /* Set the calling SM to NULL to represent no calling SM specified yet */
  sm->curr_calling_sm = NULL;

  /* First, call this state machine's entry function if it exists */
  stm_call_sm_entry(sm, payload);

  /* Set the initial state */
  sm->current_state = sm_cdata->initial_state;

  /* Call the state's entry function, if it exists */
  stm_call_state_entry(sm, (stm_state_t)STM_DEACTIVATED_STATE, payload);

  /* If the initial state has a child state machine instance, activate it now */
  if (sm_cdata->state_map_array[sm->current_state].child_sm != NULL)
  {
    /* Keep track of the status, so we can roll-back the activation if any
       of the children fail to activate */
    status = stm_activate(&sm_cdata->state_map_array[sm->current_state].
                             child_sm[sm->pi_const_data->this_instance],
                          payload);
  }

  /* If we activated a child state machine of our initial state, and it failed,
     we want to undo our activation above, since it's all or nothing */
  if (status != STM_SUCCESS)
  {
    /* That was quick, call the state's exit function if it exists */
    stm_call_state_exit(sm,(stm_state_t)STM_DEACTIVATED_STATE,payload);

    /* Reset our state to the deactivated state */
    sm->current_state = (stm_state_t)STM_DEACTIVATED_STATE;

    /* Finally, call this state machine's exit function, if it exists */
    stm_call_sm_exit(sm,payload);

    /* Call the error hook to let the user know this happened */
    STM_ERROR_NOTIFY(sm,status);
  }

  /* Release the lock on our current state machine */
  stm_release_lock(sm);

  /* Percolate our status back up to the previous caller */
  return(status);

} /* stm_activate() */



/*===========================================================================

  FUNCTION:  stm_instance_deactivate

===========================================================================*/
/*!
    @brief
    Deactivates a specified state machine instance, given a state machine array
    pointer and instance number.

    @see stm_deactivate for more information

*/
/*=========================================================================*/
stm_status_t stm_instance_deactivate
(
  stm_state_machine_t *sm,         /*!< State machine instance array pointer */
  uint32              sm_instance, /*!< State machine instance number */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_machine_t *sm_inst;

  /*-----------------------------------------------------------------------*/

  /* Make sure we have a valid state machine instance array pointer */
  STM_NULL_CHECK(sm);

  /* Make sure the instance passed is valid for the given state machine array */
  sm_inst = stm_get_instance(sm,sm_instance);
  if (sm_inst == NULL)
  {
    STM_ERROR_NOTIFY(sm,STM_EBADINSTANCE);
    return(STM_EBADINSTANCE);
  }

  /*-----------------------------------------------------------------------*/

  /* Call stm_process_input() on the instance and return the result */
  return(stm_deactivate(sm_inst,payload));

} /* stm_instance_deactivate() */

/*===========================================================================

  FUNCTION:  stm_deactivate

===========================================================================*/
/*!
    @brief
    Deactivates a specified state machine instance.

    @detail
    Deactivation of an active state machine instance happens in a bottom-up
    manner, in the following sequence.
    -# The state machine instance is traversed to the lowest-level active child
       state machine instance's current state (that has no child state machine
       instance)
    -# The lowest-level active child state machine's state's (optional) exit
       function is called and enters the STM_DEACTIVATED_STATE
    -# The lowest-level active child state machine's (optional) exit function
       is called
    -# Step 2 is repeated using the above child state machine instance's
       parent state.  The process repeats until the top-level state machine
       instance passed to stm_deactivate() has been deactivated

    The payload pointer parameter is passed to all of the state and state
    machine instance's exit functions.

    Upon encountering an error in the deactivation of a state machine instance
    (which by-design shouldn't be able to happen), the deactivation roll-up
    ceases and the relevant error code is returned to the caller.

    @return
    stm_status_t success/error code

    @retval STM_SUCCESS
    Upon successful state machine instance deactivation

    @note
    Calling stm_deactivate on an already inactivate state machine instance has
    no effect, and simply returns STM_SUCCESS.

    @see stm_activate for activation details

*/
/*=========================================================================*/
stm_status_t stm_deactivate
(
  stm_state_machine_t *sm,      /*!< State machine instance pointer */
  void                *payload  /*!< Payload */
)
{
  stm_status_t status = STM_SUCCESS;
  const stm_state_machine_constdata_t *sm_cdata;

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /* Make sure the state machine instance isn't busy/in-transition */
  if ( stm_acquire_lock(sm) != STM_SUCCESS )
  {
    STM_ERROR_NOTIFY(sm,STM_EBUSY);
    return(STM_EBUSY);
  }

  /* Make sure the state machine instance isn't already deactivated */
  if ( sm->current_state == (stm_state_t)STM_DEACTIVATED_STATE )
  {
    /* Unlock it and return that the job (from this point on downwards)
       is done */
    stm_release_lock(sm);
    return(STM_SUCCESS);
  }

  /*-----------------------------------------------------------------------*/

  /* The strategy here is to do the exact reverse of the stm_activate()
     function.  We must first descend to the lowest-level active state
     machine instance, and deactivate from it there on upward.

     Like stm_activate(), if we run into errors along the way, we stop at
     the point where we encountered the error and return that error to the
     caller.  This somewhat violates the 'kill switch will always work'
     mantra, but is absolutely necessary.  The only real case where this
     might occur is if a child state machine is in transition (very unlikely),
     where resetting everything could be disastrous.  The fact that inputs
     are processed from the top-level state machine on downward (in which
     all the parent state machines would be locked) should prevent this from
     ever occurring by-design, but to be programmatically correct we'll make
     the code behave properly.
  */

  /* We'll be using this pointer a lot, so take a local copy */
  sm_cdata = sm->pi_const_data->const_data;

  /* If the current state of this state machine has a child state machine,
     call deactivate on it prior to deactivating this state machine */
  if (sm_cdata->state_map_array[sm->current_state].child_sm != NULL)
  {
    /* Keep track of the status, so we can percolate any failures upward */
    status = stm_deactivate(&sm_cdata->state_map_array[sm->current_state].
                               child_sm[sm->pi_const_data->this_instance],
                            payload);
  }

  /* Upon success, follow-through and perform the remaining exit processing */
  if (status == STM_SUCCESS)
  {
    /* Call the state's exit function if it exists */
    stm_call_state_exit(sm,(stm_state_t)STM_DEACTIVATED_STATE,payload);

    /* Reset our state to the deactivated state */
    sm->current_state = (stm_state_t)STM_DEACTIVATED_STATE;

    /* Call this state machine's exit function, if it exists */
    stm_call_sm_exit(sm,payload);
  }
  else
  {
    /* Call the error hook to let the user know this happened */
    STM_ERROR_NOTIFY(sm,status);
  }

  /* Release the lock on our current state machine */
  stm_release_lock(sm);

  /* Percolate our status back up to the previous caller */
  return(status);

} /* stm_deactivate() */



/*===========================================================================

  FUNCTION:  stm_instance_process_input

===========================================================================*/
/*!
    @brief
    Sends an input to a specified state machine instance, given a state machine
    array pointer and instance number.

    @see stm_process_input for more information

*/
/*=========================================================================*/
stm_status_t stm_instance_process_input
(
  const stm_state_machine_t
                      *calling_sm, /*!< Optional calling state machine
                                        instance pointer */
  stm_state_machine_t *sm,         /*!< State machine instance array pointer */
  uint32              sm_instance, /*!< State machine instance number */
  stm_input_t         input,       /*!< State machine input to process */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_machine_t *sm_inst;

  /*-----------------------------------------------------------------------*/

  /* Make sure we have a valid state machine instance array pointer */
  STM_NULL_CHECK(sm);

  /* Make sure the instance passed is valid for the given state machine array */
  sm_inst = stm_get_instance(sm,sm_instance);
  if (sm_inst == NULL)
  {
    STM_ERROR_NOTIFY(sm,STM_EBADINSTANCE);
    return(STM_EBADINSTANCE);
  }

  /*-----------------------------------------------------------------------*/

  /* Call stm_process_input() on the proper instance and return the result */
  return(stm_process_input(calling_sm,sm_inst,input,payload));

} /* stm_instance_process_input() */


/*===========================================================================

  FUNCTION:  stm_process_input

===========================================================================*/
/*!
    @brief
    Sends an input to an active state machine instance for processing.

    @detail
    Inputs are processed by a state machine instance in a bottom-up manner. An
    input is processed by matching it to a state machine instance's array of
    transition function - input mappings, calling the transition function, and
    using the returned stm_state_t value as the new (potentially the same)
    state of the affected state machine.

    Processing an input happens in the following sequence:
    -# The state machine instance is traversed to the lowest-level active child
       state machine instance's current state.
    -# If the lowest-level active child state machine's state has a transition
       function for the input, it is then called.
       - If the transition function returns STM_SAME_STATE or the state value
         of the currently occupied state, nothing else occurs other than the
         code that was executed in the transition function
       - If the transition function returns a different state than the current
         one occupied by the child state machine instance, the following steps
         occur:
         -# If the child state machine instance's state contains a child state
            machine instance, stm_deactivate() is called on it.
         -# The (optional) exit function for the current state is called, with
            parameters indicating the next state and user passed payload
            parameter.
         -# The (optional) entry function for the new state is called, with
            parameters indicating the previous state and user passed payload
            parameter.
         -# If the child state machine instance's newstate contains a child
            state machine instance, stm_activate() is called on it.
       - If the transition function returns STM_DEACTIVATED_STATE, the state
         machine instance will be deactivated via the following steps:
         -# The (optional) exit function for the current state is called, with
            parameters indicating the next state is STM_DEACTIVATED_STATE
            and the user passed payload parameter.
         -# The (optional) state machine instance exit function will be called
            with the user passed payload parameter.
         -# The state machine instance is now considered deactivated, and must
            be reactivated before being able to further process any inputs.
    -# Upon successfully calling a transition function for the input, control
       will be returned to the caller with an STM_SUCCESS return value.
       - If a transition function for a child state machine instance was called,
         and the transition function indicated that it wanted the input passed
         up to the parent via a call to stm_propagate(sm,TRUE), the parent state
         machine instance's current state will be given the opportunity to
         process the input, as well.
    -# If a transition function is NOT found for a child state machine
       instance's current state, the parent state is given a chance to process
       the input.  If the input is not processed by anyone by the time it
       reaches the passed in state machine instance parameter, control is
       returned to the caller with a return value of STM_ENOTPROCESSED.

    The payload pointer parameter is passed to all of the traversed state and
    state machine instances' exit, transition, and entry functions.  It is
    also passed to the child state machine instance activation and deactivation
    functions if a child state machine is involved.

    @return
    stm_status_t success/error code

    @retval STM_SUCCESS
    Upon successful processing of the input by the state machine instance

    @retval STM_ENOTPROCESSED
    The input was not processed by the state machine instance or any of its
    children

    @note If a child state machine instance handles an input and allows it
          to propagate to its parent, but the parent does not handle the
          input, STM_SUCCESS is still returned to the caller, since at least
          one state machine instance in the family was able to process it.

    @see stm_propagate for details of handling an input in multiple levels
         of a hierarchical state machine instance

*/
/*=========================================================================*/
stm_status_t stm_process_input
(
  const stm_state_machine_t
                      *calling_sm, /*!< Optional calling state machine
                                        instance pointer */
  stm_state_machine_t *sm,         /*!< State machine instance pointer */
  stm_input_t         input,       /*!< State machine input to process */
  void                *payload     /*!< Payload pointer */
)
{
  stm_status_t        status = STM_ENOTPROCESSED,
                      temp_status = STM_SUCCESS;
  int32               input_idx;
  stm_transition_fn_t trans_fn;
  stm_state_t         new_state,prev_state;
  const stm_state_machine_constdata_t *sm_cdata;

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /* Make sure the state machine instance isn't busy/in-transition */
  if ( stm_acquire_lock(sm) != STM_SUCCESS )
  {
    STM_ERROR_NOTIFY(sm,STM_EBUSY);
    return(STM_EBUSY);
  }

  /* Make sure the state machine instance isn't deactivated */
  if ( sm->current_state == (stm_state_t)STM_DEACTIVATED_STATE )
  {
    /* Unlock it and return that the input wasn't processed */
    stm_release_lock(sm);
    STM_ERROR_NOTIFY(sm,STM_EINACTIVE);
    return(STM_EINACTIVE);
  }

  /*-----------------------------------------------------------------------*/

  /* The strategy of processing an input to a state machine instance is to
     descend to the lowest possible child state machine instance and attempt
     to process the input.  If the lower level state machine doesn't process
     the input, the next level up's state machine tries it, etc.  Once a given
     state machine instance processes the message, the default behavior is
     to return to the caller.  If a state machine does process an input, it
     may also opt to not 'consume' the input and let the above state machine
     instance try to process it, and so forth.

     The transition function returns the next state of the state machine
     instance.  If this state is different than the current state, the current
     state is exited (and all child state machines are deactivated) prior to
     moving to the new state.

  */

  /* We'll be using this pointer a lot, so take a local copy */
  sm_cdata = sm->pi_const_data->const_data;

  /* Set the propagate flag to FALSE here.  The transition function will change
     it to TRUE if it wants our parent to also process the input */
  sm->propagate_input = FALSE;

  /* Descend to the lowest level child state machine instance */
  if (sm_cdata->state_map_array[sm->current_state].child_sm != NULL)
  {
    /* Keep track of the status, so we can percolate any failures upward */
    status = stm_process_input(calling_sm,
                               &sm_cdata->state_map_array[sm->current_state].
                                  child_sm[sm->pi_const_data->this_instance],
                               input,
                               payload);
  }

  do
  {
    /* Bail out now if there were any non-standard errors */
    if ( (status != STM_ENOTPROCESSED) &&
         (status != STM_SUCCESS) &&
         (status != STM_EINACTIVE) )
    {
      STM_ERROR_NOTIFY(sm,status);
      break;
    }

    /* Bail out if our child handled the input and didn't propagate it up */
    if ( (status == STM_SUCCESS) &&
         !sm_cdata->state_map_array[sm->current_state].
            child_sm[sm->pi_const_data->this_instance].
            propagate_input )
    {
      break;
    }

    /* Try to look up the input in our input array */
    input_idx = stm_lookup_input(sm,input);

    /* Bail if we don't handle this input */
    if (input_idx == (int32)STM_INVALID_INDEX)
    {
      break;
    }

    /* Try to look up the transition function, given the input index and
       our current state */
    trans_fn = sm_cdata->
      trans_map_array[ ((uint32)sm->current_state * sm_cdata->num_inputs) +
                        (uint32)input_idx ];

    /* Bail if it's NULL (input known, but not handled in this state) */
    if (trans_fn == NULL)
    {
      break;
    }

    /* Set the status to indicate we processed the input (it's inevitable) */
    status = STM_SUCCESS;

    /* Record the input index for debug usage */
    sm->curr_input_index = input_idx;

    /* Record the calling SM for debug usage */
    sm->curr_calling_sm = calling_sm;

    /* If the debug hook function is valid, call it before the trans. fn */
    if (sm_cdata->debug_hook_fn != NULL)
    {
      sm_cdata->debug_hook_fn(STM_PRE_TRANS_FN,sm,sm->current_state,payload);
    }

    /* Call the transition function and record what state it returns */
    new_state = trans_fn(sm,payload);

    /* If the debug hook function is valid, call it also */
    if (sm_cdata->debug_hook_fn != NULL)
    {
      sm_cdata->debug_hook_fn(STM_POST_TRANS_FN,sm,new_state,payload);
    }

    /* If the state didn't change, we're done */
    if ( (new_state == (stm_state_t)STM_SAME_STATE) ||
         (new_state == sm->current_state) )
    {
      break;
    }

    /* Just to be paranoid, check the range of the state we got back */
    if ( ((new_state < 0) && (new_state != (stm_state_t)STM_DEACTIVATED_STATE)) ||
         (new_state >= (stm_state_t)sm_cdata->num_states) )
    {
      status = STM_EBADSTATE;
      STM_ERROR_NOTIFY(sm,status);
      break;
    }

    /* If the current state of this state machine has a child state machine,
       call deactivate on it prior to changing state */
    if (sm_cdata->state_map_array[sm->current_state].child_sm != NULL)
    {
      /* Keep track of the status, so we can send any failure upward */
      temp_status =
        stm_deactivate(&sm_cdata->state_map_array[sm->current_state].
                          child_sm[sm->pi_const_data->this_instance],
                       payload);
    }

    /* If our temp_status (the deactivation of our prev state's state machine
       instance) is bad, send that back instead of our standard status */
    if (temp_status != STM_SUCCESS)
    {
      status = temp_status;
      STM_ERROR_NOTIFY(sm,status);
      break;
    }

    /* If the current state has an exit function, call it */
    stm_call_state_exit(sm,new_state,payload);

    /* Save the current state and update the SM to reflect the new state */
    prev_state = sm->current_state;
    sm->current_state = new_state;

    /* If the new state is STM_DEACTIVATED_STATE, proceed by deactivating this
       state machine instance */
    if (sm->current_state == (stm_state_t)STM_DEACTIVATED_STATE)
    {
      /* Call this state machine's exit function, if it exists */
      stm_call_sm_exit(sm,payload);

      /* We're done now, since we just deactivated ourself */
      break;
    }

    /* If we get here, see if the new state has an entry function to call */
    stm_call_state_entry(sm,prev_state,payload);

    /* Finally, activate the child state machine of the new state if it
       has one */
    if (sm_cdata->state_map_array[sm->current_state].child_sm != NULL)
    {
      /* Keep track of the status, so we can send any failure upward */
      temp_status = stm_activate(&sm_cdata->state_map_array[sm->current_state].
                                    child_sm[sm->pi_const_data->this_instance],
                                 payload);
    }

    /* If our temp_status (the activation of our new state's state machine
       instance) is bad, send it back instead of our standard status */
    if (temp_status != STM_SUCCESS)
    {
      status = temp_status;
      STM_ERROR_NOTIFY(sm,status);
      break;
    }

  } while ( 0 );

  /* Release the lock on our current state machine */
  stm_release_lock(sm);

  /* Return our status to the caller */
  return(status);

} /* stm_process_input() */


/*===========================================================================

  FUNCTION:  stm_propagate_input

===========================================================================*/
/*!
    @brief
    Called within a transition function to allow a child state machine
    instance's parent to also handle the input

    @return
    Nothing

    @see stm_process_input for details on how input processing is done

*/
void stm_propagate_input
(
  stm_state_machine_t *sm,      /*!< State machine instance pointer */
  boolean             prop_flag /*!< Flag to enable/disable propagation of input
                                     to parent state machine. Default behavior
                                     is FALSE */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);

  /*-----------------------------------------------------------------------*/

  /* Set the value appropriately */
  sm->propagate_input = (prop_flag ? TRUE : FALSE);

} /* stm_propagate_input() */

/*===========================================================================

                       HELPER FUNCTIONS

===========================================================================*/


/*===========================================================================

  FUNCTION:  stm_get_error_string

===========================================================================*/
/*!
    @brief
    Helper function to get a human readable description of the stm_status_t
    codes used by the STM framework

    @return
    Returns a friendly string describing the stm_status_t code

*/
/*=========================================================================*/
const char *stm_get_error_string
(
  stm_status_t  error_code  /*!< Status code for which to retrieve a string */
)
{
  const char *err_string;

  /*-----------------------------------------------------------------------*/

  /* Convert error code to a representative string */
  switch (error_code)
  {
  case STM_SUCCESS:
    err_string = "General Success/Input Processed";
    break;

  case STM_EBUSY:
    err_string = "State Machine Busy";
    break;

  case STM_EBADINSTANCE:
    err_string = "Nonexistent Instance";
    break;

  case STM_ENOTPROCESSED:
    err_string = "Input Not Processed";
    break;

  case STM_EINACTIVE:
    err_string = "State Machine Inactive";
    break;

  case STM_EBADSTATE:
    err_string = "Invalid State Returned from Transition Function";
    break;

  default:
    err_string = "Unknown error code";
    break;
  }

  /* Return the string */
  return(err_string);

} /* stm_get_error_string() */


/*===========================================================================

  FUNCTION:  stm_get_name

===========================================================================*/
/*!
    @brief
    Helper function to retrieve state machine instance name

    @return
    Returns a friendly name string of the passed state machine instance

*/
const char *stm_get_name
(
  const stm_state_machine_t *sm /*!< State machine instance pointer */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);

  /*-----------------------------------------------------------------------*/

  /* Return the name of the state machine */
  return( sm->pi_const_data->name );
} /* stm_get_name() */


/*===========================================================================

  FUNCTION:  stm_get_uid

===========================================================================*/
/*!
    @brief
    Helper function to retrieve state machine instance unique id.  The
    unique id of a state machine instance is computed by taking the lower (LSBs)
    32 bits of the standard MD5 digest of the state machine instance name.

    @return
    32 bit unsigned value made up of 32 LSBs of MD5 digest of SM inst name

*/
/*=========================================================================*/
uint32 stm_get_uid
(
  const stm_state_machine_t *sm /*!< State machine instance pointer */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);

  /*-----------------------------------------------------------------------*/

  /* Return the uid of the state machine */
  return( sm->pi_const_data->uid );
} /* stm_get_uid() */


/*===========================================================================

  FUNCTION:  stm_get_num_instances

===========================================================================*/
/*!
    @brief
    Helper function to retrieve the total number of state machine instances
    for a state machine

    @return
    Returns the total number of instances of this state machine

    @note
    This function takes either a state machine instance pointer or a
    state machine instance array pointer

*/
uint32 stm_get_num_instances
(
  const stm_state_machine_t *sm /*!< State machine instance or array pointer */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* Return the # of instances of this state machine */
  return( sm->pi_const_data->const_data->num_instances );
} /* stm_get_num_instances() */


/*===========================================================================

  FUNCTION:  stm_get_instance_num

===========================================================================*/
/*!
    @brief
    Helper function to retrieve state machine instance index for the
    passed in state machine instance pointer

    @return
    Returns the instance (0 thru n-1) for an n-instance state machine

    @retval 0
    Returns 0 upon NULL state machine instance pointer

*/
uint32 stm_get_instance_num
(
  const stm_state_machine_t *sm /*!< State machine instance pointer */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);

  /*-----------------------------------------------------------------------*/

  /* Return the instance of this state machine */
  return( sm->pi_const_data->this_instance );
} /* stm_get_instance_num() */


/*===========================================================================

  FUNCTION:  stm_get_instance

===========================================================================*/
/*!
    @brief
    Helper function to retrieve the state machine instance pointer for the
    provided state machine instance index

    @return
    Returns the state machine instance pointer for the given instance index

    @retval NULL
    Returns NULL if the specified instance is invalid

*/
stm_state_machine_t *stm_get_instance
(
  stm_state_machine_t *sm,     /*!< State machine instance array pointer */
  uint32              instance /*!< Instance index */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* Handle invalid instance by returning NULL */
  if ( instance >= sm[0].pi_const_data->const_data->num_instances )
  {
    return(NULL);
  }

  /* Return the state machine instance pointer for the passed instance, */
  /* regardless of which SM array element it is called on. */
  return(&sm[instance - sm->pi_const_data->this_instance]);
} /* stm_get_instance() */


/*===========================================================================

  FUNCTION:  stm_get_state

===========================================================================*/
/*!
    @brief
    Helper function to retrieve the current state of a given state machine
    instance

    @return
    Returns the stm_state_t value for the state machine instance's current
    state

*/
stm_state_t stm_get_state
(
  const stm_state_machine_t *sm /*!< State machine instance pointer */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);

  /*-----------------------------------------------------------------------*/

  /* Return the state machine's current state */
  return( sm->current_state );
} /* stm_get_state() */


/*===========================================================================

  FUNCTION:  stm_get_num_states

===========================================================================*/
/*!
    @brief
    Helper function to retrieve the number of states in a given state machine
    instance

    @return
    Number of states in the given state machine instance

*/
/*=========================================================================*/
uint32 stm_get_num_states
(
  const stm_state_machine_t *sm  /*!< State machine instance pointer */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* Return the number of states in this state machine */
  return( sm->pi_const_data->const_data->num_states );

} /* stm_get_num_states() */


/*===========================================================================

  FUNCTION:  stm_get_state_name

===========================================================================*/
/*!
    @brief
    Helper function to retrieve the state name string of a given state machine
    instance pointer and valid state.

    @return
    Returns a string representing the friendly name of the specified state in
    the specified state machine instance

*/
const char *stm_get_state_name
(
  const stm_state_machine_t *sm,  /*!< State machine instance pointer */
  stm_state_t               state /*!< State for which string is desired */
)
{
  const char *state_name;
  const char *invalid_str = "Invalid State";
  const stm_state_machine_constdata_t *sm_cdata;

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* We'll be using this pointer a lot, so take a local copy */
  sm_cdata = sm->pi_const_data->const_data;

  /* Handle a invalid states by returning "Invalid" */
  if ( (state <= (stm_state_t)STM_INVALID_STATE) ||
       (state >= (stm_state_t)sm_cdata->num_states) )
  {
    return(invalid_str);
  }

  /* Handle the easy cases */
  switch(state)
  {
  case STM_DEACTIVATED_STATE:
    state_name = "DEACTIVATED";
    break;
  case STM_SAME_STATE:
    /* This should never happen, but since we may recurse, ensure that it
       only occurs once before returning "Invalid" */
    if( sm->current_state == (stm_state_t)STM_SAME_STATE)
    {
      return(invalid_str);
    }
    else
    {
      /* Get the state name, based upon current state */
      state_name = stm_get_state_name(sm,sm->current_state);
    }
    break;
  default:
    state_name = sm_cdata->state_map_array[state].name;
    break;
  }

  /* Return with what we got */
  return(state_name);

} /* stm_get_state_name() */


/*===========================================================================

  FUNCTION:  stm_get_child

===========================================================================*/
/*!
    @brief
    Helper function to retrieve the child state machine instance of a
    state machine instance and a state.

    @return
    Returns the child state machine instance pointer belonging to the
    corresponding state.

    @retval NULL
    Upon no child state machine instance pointer or invalid state.

*/
stm_state_machine_t *stm_get_child
(
  const stm_state_machine_t *sm,    /*!< State machine instance pointer */
  const stm_state_t          state  /*!< State machine state */
)
{
  const stm_state_machine_constdata_t *sm_cdata;

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* We'll be using this pointer a lot, so take a local copy */
  sm_cdata = sm->pi_const_data->const_data;

  /* Handle invalid states or nonexistent child SM by returning NULL */
  if ( (state < 0) ||
       (state >= (stm_state_t)sm_cdata->num_states) ||
       (sm_cdata->state_map_array[state].child_sm == NULL) )
  {
    return(NULL);
  }

  /* Return the state machine's state's child or NULL if sm is NULL */
  return(&sm_cdata->state_map_array[state].
            child_sm[sm->pi_const_data->this_instance]);
} /* stm_get_child() */


/*===========================================================================

  FUNCTION:  stm_get_last_input

===========================================================================*/
/*!
    @brief
    Helper function to get the current input for a state machine instance.

    @return
    Returns the latest input value processed by the passed in state machine
    instance.

    @retval STM_INVALID_INPUT
    Upon no input being processed since state machine instance activation

*/
stm_input_t stm_get_last_input
(
  const stm_state_machine_t *sm /*!< State machine instance pointer */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* Handle an unset current_input_index by returning STM_INVALID_INDEX */
  if ( sm->curr_input_index == (int32)STM_INVALID_INDEX )
  {
    return(stm_state_t)(STM_INVALID_INPUT);
  }

  /* Extract the input value and return it */
  return(sm->pi_const_data->const_data->
           input_map_array[sm->curr_input_index].input);

} /* stm_get_last_input() */


/*===========================================================================

  FUNCTION:  stm_get_last_input_name

===========================================================================*/
/*!
    @brief
    Helper function to get the current input name for a state machine instance.

    @return
    Returns the name for the latest input value processed by the passed in
    state machine instance.

*/
const char *stm_get_last_input_name
(
  const stm_state_machine_t *sm /*!< State machine instance pointer */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);
  STM_NULL_CHECK(sm->pi_const_data);
  STM_NULL_CHECK(sm->pi_const_data->const_data);

  /*-----------------------------------------------------------------------*/

  /* Handle no input yet received */
  if ( sm->curr_input_index == (int32)STM_INVALID_INDEX )
  {
    return("No Input Received");
  }

  /* Extract the input name and return it */
  return(sm->pi_const_data->const_data->
           input_map_array[sm->curr_input_index].name);

} /* stm_get_last_input_name() */


/*===========================================================================

  FUNCTION:  stm_get_last_calling_sm

===========================================================================*/
/*!
    @brief
    Helper function to get the last calling state machine instance
    pointer for a given state machine instance.

    @return
    Returns the calling state machine instance pointer last passed to
    stm_process_input() or stm_process_input_instance() for the
    state machine instance provided.

*/
/*=========================================================================*/
const stm_state_machine_t *stm_get_last_calling_sm
(
  const stm_state_machine_t *sm /*!< State machine instance pointer */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);

  /*-----------------------------------------------------------------------*/

  /* Extract the calling SM pointer */
  return(sm->curr_calling_sm);

} /* stm_get_last_calling_sm() */


/*===========================================================================

  FUNCTION:  stm_get_debug_mask

===========================================================================*/
/*!
    @brief
    Helper function to get the debug mask for a passed-in state
    machine instance.  This is intended to be used from within a state
    machine debug hook function.

    @return
    Returns the debug mask that was previously set by the user

    @see stm_set_debug_mask

    @note
    The debug mask is persistent across state machine instance
    activation and deactivation.

*/
uint32 stm_get_debug_mask
(
  const stm_state_machine_t *sm /*!< State machine instance pointer */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);

  /*-----------------------------------------------------------------------*/

  /* Return the value */
  return( sm->debug_mask );
} /* stm_get_debug_mask() */


/*===========================================================================

  FUNCTION:  stm_set_debug_mask

===========================================================================*/
/*!
    @brief
    Helper function to set the debug mask for a passed-in state
    machine instance.  This is generally done prior to state machine
    activation to set a user-defined logging or profiling mask, but can
    be dynamically changed at any time during state machine operation.

    @see stm_get_debug_mask

    @note
    The debug mask is persistent across state machine instance
    activation and deactivation.

*/
void stm_set_debug_mask
(
  stm_state_machine_t *sm,
  uint32               debug_mask
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);

  /*-----------------------------------------------------------------------*/

  /* Set the value */
  sm->debug_mask = debug_mask;
} /* stm_set_debug_mask() */


/*===========================================================================

  FUNCTION:  stm_get_user_data

===========================================================================*/
/*!
    @brief
    Helper function to get the user-defined data for a passed-in state
    machine instance.

    @return
    Returns the user-defined data pointer that was previously set by the
    user

    @see stm_set_user_data

    @note
    The user-defined data pointer is persistent across state machine instance
    activation and deactivation.

*/
void *stm_get_user_data
(
  const stm_state_machine_t *sm /*!< State machine instance pointer */
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);

  /*-----------------------------------------------------------------------*/

  /* Return the user data ptr */
  return( sm->user_data );
} /* stm_get_user_data() */


/*===========================================================================

  FUNCTION:  stm_set_user_data

===========================================================================*/
/*!
    @brief
    Helper function to set the user-defined data for a passed-in state
    machine instance.

    @see stm_get_user_data

    @note
    The user-defined data pointer is persistent across state machine instance
    activation and deactivation.

*/
void stm_set_user_data
(
  stm_state_machine_t *sm,
  void                *user_data
)
{

  /*-----------------------------------------------------------------------*/

  /* Make sure we were passed a valid state machine instance pointer */
  STM_NULL_CHECK(sm);

  /*-----------------------------------------------------------------------*/

  /* Set the value */
  sm->user_data = user_data;
} /* stm_set_user_data() */

