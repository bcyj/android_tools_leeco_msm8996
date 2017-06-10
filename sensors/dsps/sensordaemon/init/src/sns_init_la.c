/*============================================================================
  @file sns_init_la.c

  @brief
  This implements the initialization of the sensors subsystem required for
  Linux Android builds.

  Provides an "sns_init()" function which is called from sensor1_init() in
  the ACM.

  <br><br>

  DEPENDENCIES: This uses OS services defined in sns_osa.h.
  It initializes all of the modules on the apps processor, so it depends on each
  module's init function.

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  INCLUDE FILES
  ============================================================================*/

#include "sensor1.h"

#define LOG_TAG "Sensors"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "sns_common.h"
#include "sns_debug_str.h"
#include "sns_diagproxy.h" /* For sns_diag_deinit */
#include "sns_debug_api.h" /* For Debug module ID */
#include "sns_init.h"
#include "sns_osa.h"
#include "sns_pwr.h"

#include <utils/Log.h>
#include <common_log.h>
#include <linux/msm_dsps.h>

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

#define SNS_MODULE SNS_MODULE_APPS_INIT

/* Retry a standard library command if the error code is EINTR */
#define  RETRY_ON_EINTR(ret,cond) \
    do { \
        ret = (cond); \
    } while (ret < 0 && errno == EINTR)

/* Name of the Android debugger socket */
#define ANDROID_DEBUG_SOCKET_NAME "android:debuggerd"

/*============================================================================
  Type Declarations
  ============================================================================*/

extern int g_pil_fd;

/*============================================================================
 * Global Data Definitions
 ============================================================================*/

/** Semaphore used for synchronizing initialization. Each "init" function
    posts to the semaphore when its initialization is complete */
OS_EVENT *sns_init_sem_ptr;

/*============================================================================
  Static Variable Definitions
  ============================================================================*/

static pthread_once_t   sns_init_once_ctl = PTHREAD_ONCE_INIT;
static sns_err_code_e   sns_init_sns_err;
static sem_t            sns_init_sem;
static struct sigaction old_action[NSIG];

/*============================================================================
  Static Function Definitions and Documentation
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_init_fault_hndlr

  ===========================================================================*/
/*!
  @brief Does necessary cleanup after faults are detected.

  This function does not return

*/
/*=========================================================================*/
static void sns_init_fault_hndlr( int signum )
{
  static volatile bool err_in_progress = false;

  if( SIGHUP == signum ) {
    /* Reset the DSPS on SIGHUP */
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_INIT,
                               "Resetting DSPS on SIGHUP (fd %d)",
                               g_pil_fd );
    ioctl( g_pil_fd, DSPS_IOCTL_RESET, 0 );
    return;
  }

  /* LOGE("%s: signal %s (%d)", __FUNCTION__, strsignal(signum), signum); */

  if( err_in_progress != false ) {
    // Immediately exit
    signal( signum, SIG_IGN );
    raise( signum );
    exit( signum );
  }

  err_in_progress = true;

  /* Perform any process cleanup operations: */
  sns_pwr_crash_shutown();
  sns_diag_deinit();

  sigaction( signum, &old_action[signum], NULL );
  raise( signum );
}

/*===========================================================================

  FUNCTION:   sns_init_once

  ===========================================================================*/
/*!
  @brief Performs all one-time initialization & sets up the fault handler.

  This function is called via "pthread_once()" to insure that it can only be
  called once.

  This function will call all of the modules' initialization functions.


  @sideeffects
  Updates sns_init_sns_err with any error code.
  Each module's init function may have side effects.

*/
/*=========================================================================*/
static void sns_init_once( void )
{
  int              i;
  struct sigaction action;
  sigset_t         set;

  static const struct init_ptrs
  {
    sns_init_fcn fcn;
    const char * fcn_name;
  } init_ptrs[] = SNS_INIT_FUNCTIONS;

  /* Mask of SIGALRM for use by EM */
  sigemptyset( &set );
  sigaddset( &set, SIGALRM );
  sigprocmask( SIG_SETMASK, &set, NULL );

  /* Set up the fault handler */
  action.sa_handler = sns_init_fault_hndlr;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  for( i = 1; i < NSIG ; i++ )
  {
    sigaction( i, &action, &old_action[i] );
  }

  (void)sem_init( &sns_init_sem, 0, 0 );

  /* Call each module's init function */
  for( i = 0; NULL != init_ptrs[i].fcn; i++ ) {
    SNS_PRINTF_STRING_HIGH_1(SNS_DBG_MOD_APPS_INIT,
                             "initializing %s",
                             (intptr_t)init_ptrs[i].fcn_name);

    sns_init_sns_err = init_ptrs[i].fcn();
    if( SNS_SUCCESS != sns_init_sns_err ) {
      SNS_PRINTF_STRING_ERROR_2(SNS_DBG_MOD_APPS_INIT,
                                "Error %d initializing %s",
                                sns_init_sns_err,
                                (intptr_t)init_ptrs[i].fcn_name);

      return;
    }
    SNS_PRINTF_STRING_HIGH_1(SNS_DBG_MOD_APPS_INIT,
                             "Waiting for %s...",
                             (intptr_t)init_ptrs[i].fcn_name);
    sem_wait( &sns_init_sem );
    SNS_PRINTF_STRING_HIGH_1(SNS_DBG_MOD_APPS_INIT,
                             "Done waiting for %s",
                             (intptr_t)init_ptrs[i].fcn_name);
  }

  (void)sem_destroy( &sns_init_sem );
  SNS_PRINTF_STRING_HIGH_0(SNS_DBG_MOD_APPS_INIT,
                           "All modules initializied" );
}
/*============================================================================
  Externalized Function Definitions
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_init_done
  - Documented in sns_init.h

  ===========================================================================*/
void sns_init_done( void )
{
  sem_post( &sns_init_sem );
}


/*===========================================================================

  FUNCTION:   sns_init
  - Documented in sns_init.h

  ===========================================================================*/
sns_err_code_e sns_init( void )
{
  sns_init_sns_err = SNS_SUCCESS;
  if( 0 == pthread_once( &sns_init_once_ctl, sns_init_once ) ) {
    /* sns_init_once will update the error code, if necessary */
    return sns_init_sns_err;
  } else {
    return SNS_ERR_FAILED;
  }
}
