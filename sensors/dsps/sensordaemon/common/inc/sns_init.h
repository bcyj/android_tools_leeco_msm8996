#ifndef _SNS_INIT_H_
#define _SNS_INIT_H_

/*============================================================================
  @file sns_init.h

  @brief
  This file defines "sns_init()" and a list of modules to be initialized on
  each processor.

  sns_init() is a function which is called to initialize all of the sensors
  modules.

  This is a common header file; however, each processor & target OS may have
  a specific implementation of sns_init() for that platform.

  <br><br>

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  Include Files
  ============================================================================*/
#include "sns_common.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/
/*
 SNS_INIT_FUNCTIONS is an initializer for an array used during initialization.
 Each function will be called in turn.

 These functions may not block, but may spawn threads which may block during
 initialization. When the module init is done, each module shall call
 sns_init_done().
*/
#if defined(QDSP6) /* ADSP-based Builds */
# if defined(SNS_QDSP_SIM)
#  define SNS_INIT_FUNCTIONS   \
   {  sns_init_dsps,           \
      sns_em_init,             \
      sns_smgr_init,           \
      sns_sam_init,            \
      sns_scm_init,            \
      sns_ssm_init,            \
      sns_debug_init,          \
      NULL }
# elif defined(VIRTIO_BRINGUP)
#  define SNS_INIT_FUNCTIONS   \
   {  sns_init_dsps,           \
      sns_em_init,             \
      sns_smgr_init,           \
      sns_sam_init,            \
      sns_usam_init,           \
      sns_scm_init,            \
      sns_pm_init,             \
      sns_ssm_init,            \
      sns_debug_init,          \
      NULL }
# else
#  define SNS_INIT_FUNCTIONS   \
   {  sns_init_dsps,           \
      sns_em_init,             \
      sns_smgr_init,           \
      sns_sam_init,            \
      sns_scm_init,            \
      sns_ssm_init,            \
      sns_debug_init,          \
      smgr_qmi_ping_client_start, \
      NULL }
# endif /* defined(QDSP6) */
#elif defined(SNS_PCSIM)  /* PCSIM-based Builds */
# define SNS_INIT_FUNCTIONS    \
  {   sns_memmgr_init,         \
      sns_init_dsps,           \
      sns_em_init,             \
      sns_smr_init,            \
      sns_reg_init,            \
      sns_time_init,           \
      sns_smgr_init,           \
      sns_sam_init,            \
      sns_playback_init,       \
      sns_test_init,           \
      sns_pm_test_task_init,   \
      NULL }
#elif defined(_WIN32) /* Windows Phone-based Builds */
# define SNS_INIT_FUNCTIONS                           \
  { { sns_memmgr_init, "sns_memmgr_init" },           \
    { sns_debug_test_init1, "sns_debug_test_init1" }, \
    { sns_em_init, "sns_em_init" },                   \
    { sns_pwr_init, "sns_pwr_init" },                 \
    { sns_reg_init, "sns_reg_init" },                 \
    { sns_time_init, "sns_time_init" },               \
    { sns_debug_test_init2, "sns_debug_test_init2" }, \
    { sns_file_init, "sns_file_init" },               \
    { sns_acm_init, "sns_acm_init" },                 \
    {  NULL, ""} }
# define SNS_DEINIT_FUNCTIONS                         \
  { { sns_acm_deinit, "sns_acm_deinit" },             \
    /* WIN32 - kill ACM, SMR, then rest */            \
    { sns_file_deinit, "sns_file_deinit" },           \
    { sns_debug_test_deinit2, "sns_debug_test_deinit2" }, \
    { sns_time_deinit, "sns_time_deinit" },           \
    { sns_reg_deinit, "sns_reg_deinit" },             \
    { sns_pwr_deinit, "sns_pwr_deinit" },             \
    { sns_em_deinit, "sns_em_deinit" },               \
    { sns_memmgr_deinit, "sns_memmgr_deinit" },       \
    {  NULL, ""} }
#else /* LA-based Builds */
# define SNS_INIT_FUNCTIONS                          \
  { { sns_memmgr_init, "sns_memmgr_init" },           \
    { sns_debug_test_init1, "sns_debug_test_init1" }, \
    { sns_pwr_init, "sns_pwr_init" },                 \
    { sns_em_init, "sns_em_init" },                   \
    { sns_reg_init, "sns_reg_init" },                 \
    { sns_time_init, "sns_time_init" },               \
    { sns_sam_init, "sns_sam_init" },                 \
    { sns_usam_init, "sns_usam_init" },               \
    { sns_debug_test_init2, "sns_debug_test_init2" }, \
    { sns_file_init, "sns_file_init" },               \
    { sns_acm_init, "sns_acm_init" },                 \
    {  NULL, ""} }
#endif
/*============================================================================
  Type Declarations
  ============================================================================*/

/** This type defines the initialization signature */
typedef sns_err_code_e (*sns_init_fcn) ( void );

/*============================================================================
  Function Declarations
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_init_done

  ===========================================================================*/
/*!
  @brief Called by each module after its initialization has been completed.

  This will allow the initialization procedure to continue onto the next
  module.

*/
/*=========================================================================*/
void
sns_init_done( void );

/*===========================================================================

  FUNCTION:   sns_init

  ===========================================================================*/
/*!
  @brief Performs all one-time initialization.

  This function will call all of the modules' initialization functions. It
  shall be called before any sensors functionality is used.

  @sideeffects
  None directly -- although each module's init function may have side effects.

*/
/*=========================================================================*/
sns_err_code_e
sns_init( void );

/*============================================================================
  Module Initializaiton Function Declarations -- should only be used by init
  ============================================================================*/
#ifdef DAL_UCOS_TEST
/*===========================================================================

  FUNCTION:   sns_init_dsps

  ===========================================================================*/
/**
  @brief Performs init of various DSPS services which are required to be ready
  before the SNS modules are initialized.

  @return SNS_SUCCESS if init succeeds
*/
/*=========================================================================*/
sns_err_code_e
sns_init_tests( void );
#endif

/*===========================================================================

  FUNCTION:   sns_init_dsps

  ===========================================================================*/
/**
  @brief Performs init of various DSPS services which are required to be ready
  before the SNS modules are initialized.

  @return SNS_SUCCESS if init succeeds
*/
/*=========================================================================*/
sns_err_code_e
sns_init_dsps( void );


/*=====================================================================================

  FUNCTION:  sns_em_init

=====================================================================================*/
/**
Initializes the event manager and the underlying timer hardware. This API should be
called once at system initialization.

@retval sns_err_code_e

*/
/*=========================================================================*/
sns_err_code_e
sns_em_init( void );


/*===========================================================================

  FUNCTION:  sns_smr_init

===========================================================================*/
/**
  @brief
  Initialize all the necessary data structures of SMR

  @detail

  @return None
*/
/*=========================================================================*/
sns_err_code_e
sns_smr_init(void);

/*===========================================================================

  FUNCTION:   sns_memmgr_init

  ===========================================================================*/
/**
  @brief Initializes the memory manager.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_memmgr_init( void );


/*===========================================================================

  FUNCTION:   sns_sam_init

  ===========================================================================*/
/**
  @brief Initializes the "legacy" SAM Framework.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_sam_init(void);

/*===========================================================================

  FUNCTION:   sns_usam_init

  ===========================================================================*/
/**
  @brief Initializes new SAM Framework.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_usam_init(void);


/*===========================================================================

  FUNCTION:  sns_smgr_init

===========================================================================*/
/**
  @brief
  Initialize all the necessary data structure of SMGR task

  @detail

  @return None

*/
/*=========================================================================*/
sns_err_code_e
sns_smgr_init (void);


/*===========================================================================

  FUNCTION:  sns_scm_init

===========================================================================*/
/**
  @brief Initializes SCM

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_scm_init(void);

/*===========================================================================

  FUNCTION:  sns_ssm_init

===========================================================================*/
/**
  @brief Initializes Sensors Service Manager

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_ssm_init(void);

/*===========================================================================

  FUNCTION:   sns_debug_test_init1

===========================================================================*/
/**
  @brief
  Entry point to the sns_debug module. Registers and initializes with DIAG.
  This would help to see debug messages during sensors initialization in
  QXDM.

  @param[i]
  No input parameters.

  @return
  sns_err_code_e: Error code indicating if init was successful.
*/
/*=========================================================================*/
sns_err_code_e
sns_debug_test_init1( void );

/*===========================================================================

  FUNCTION:   sns_debug_test_init2

===========================================================================*/
/**
  @brief
  Second entry point to the sns_debug module. Creates and initializes
  the debug thread.

  @param[i]
  No input parameters.

  @return
  sns_err_code_e: Error code indicating if init was successful.
*/
/*=========================================================================*/
sns_err_code_e
sns_debug_test_init2( void );


/*===========================================================================

  FUNCTION:   sns_acm_init

  ===========================================================================*/
/**
  @brief Initializes ACM. Creates ACM thread & prepares data structures.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_acm_init( void );

/*===========================================================================

  FUNCTION:   sns_pwr_init

  ===========================================================================*/
/**
  @brief Initializes and boots DSPS.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_pwr_init( void );

/*===========================================================================

  FUNCTION:   sns_reg_init

  ===========================================================================*/
/**
  @brief Initializes Sensors Registry. Creates Sensors Registry thread.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_reg_init( void );

/*===========================================================================

  FUNCTION:   sns_file_init

  ===========================================================================*/
/**
  @brief Initializes file service. Creates file thread & prepares data structures.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_file_init( void );

/*===========================================================================

  FUNCTION:   sns_time_init

  ===========================================================================*/
/**
  @brief Initializes the time service. Creates a time service thread.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_time_init( void );

/*===========================================================================

  FUNCTION:   sns_playback_init

  ===========================================================================*/
/**
  @brief Initializes PLAYBACK.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_playback_init( void );

/*===========================================================================

  FUNCTION:   sns_test_init

  ===========================================================================*/
/**
  @brief Initializes Unit Test

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_test_init( void );

#ifndef VIRTIO_BRINGUP
/*===========================================================================

  FUNCTION:   sns_pm_init

  ===========================================================================*/
/*!
  @brief Performs initialization of the Sensors Power Manager.

  @sideeffects

*/
/*=========================================================================*/
sns_err_code_e
sns_pm_init( void );
#endif

#ifdef QDSP6
/*===========================================================================

  FUNCTION:   sns_debug_init

===========================================================================*/
/**
  @brief
  Entry point to the sns_debug module. Registers and initializes with DIAG.
  This would help to see debug messages during sensors initialization in
  QXDM.

  @param[i]
  No input parameters.

  @return
  sns_err_code_e: Error code indicating if init was successful.
*/
/*=========================================================================*/
sns_err_code_e
sns_debug_init( void );
#endif
/*===========================================================================

  FUNCTION:   sns_idle_main

  ===========================================================================*/
/**
  @brief
  This function is called from the OSIdleHook as registered with the uCOS idle
  thread.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_idle_main( void );

sns_err_code_e sam_qmi_ping_client_start(void);
sns_err_code_e debug_qmi_ping_client_start(void);
sns_err_code_e smgr_qmi_ping_client_start(void);

/*===========================================================================

  FUNCTION:   sns_deinit

  ===========================================================================*/
/*!
  @brief Performs all deinitialization.

  This function will call all of the modules' deinitialization functions. It
  shall be called to cleanup all sensor resources.

  @sideeffects
  None directly -- although each module's deinit function may have side effects.

*/
/*=========================================================================*/
sns_err_code_e
sns_deinit( void );

/*=====================================================================================

  FUNCTION:  sns_em_deinit

=====================================================================================*/
/**
Deinitializes the event manager and the underlying timer hardware. This API should be
called once at service/driver shutdown.

@retval sns_err_code_e

*/
/*=========================================================================*/
sns_err_code_e
sns_em_deinit( void );

/*===========================================================================

  FUNCTION:  sns_smr_deinit

===========================================================================*/
/**
  @brief
  Remove all the necessary data structures of SMR

  @detail

  @return None
*/
/*=========================================================================*/
sns_err_code_e
sns_smr_deinit(void);

/*===========================================================================

  FUNCTION:   sns_memmgr_deinit

  ===========================================================================*/
/**
  @brief Deinitializes the memory manager.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_memmgr_deinit( void );

/*===========================================================================

  FUNCTION:   sns_sam_deinit

  ===========================================================================*/
/**
  @brief Deinitializes SAM

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_sam_deinit(void);

/*===========================================================================

  FUNCTION:   sns_debug_test_deinit2

===========================================================================*/
/**
  @brief
  Cleanup debug resources.

  @param[i]
  No input parameters.

  @return
  sns_err_code_e: Error code indicating if deinit was successful.
*/
/*=========================================================================*/
sns_err_code_e
sns_debug_test_deinit2( void );

/*===========================================================================

  FUNCTION:   sns_acm_deinit

  ===========================================================================*/
/**
  @brief Cleanup ACM. Remove ACM thread & resources.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_acm_deinit( void );

/*===========================================================================

  FUNCTION:   sns_pwr_deinit

  ===========================================================================*/
/**
  @brief Deinitializes and boots DSPS.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_pwr_deinit( void );

/*===========================================================================

  FUNCTION:   sns_reg_deinit

  ===========================================================================*/
/**
  @brief Deinitializes Sensors Registry. Destroys Sensors Registry thread.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_reg_deinit( void );

/*===========================================================================

  FUNCTION:   sns_file_deinit

  ===========================================================================*/
/**
  @brief Deinitializes file service. Destroys file service thread.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_file_deinit( void );


/*===========================================================================

  FUNCTION:   sns_time_deinit

  ===========================================================================*/
/**
  @brief Deinitializes the time service. Destroys the time service thread.

  @return
  SNS_SUCCESS if no error.

*/
/*=========================================================================*/
sns_err_code_e
sns_time_deinit( void );

#endif /* _SNS_INIT_H_ */
