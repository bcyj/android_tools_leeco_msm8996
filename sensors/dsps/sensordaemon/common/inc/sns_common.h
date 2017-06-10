#ifndef SNS_COMMON_H
#define SNS_COMMON_H

/*============================================================================

  @file sns_common.h

  @brief
  This file contains common definitions for Sensors framework

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*============================================================================

                                INCLUDE FILES

============================================================================*/
#include "sensor1.h"
#include "sns_osa.h"
#include <stdlib.h>
#include <stdbool.h>
#if defined(SNS_DSPS_BUILD) || defined(SNS_BLAST)
# include <err.h>
# include "customer.h"
#endif
#if defined(SNS_LA) || defined(SNS_LA_SIM)
# include "cutils/log.h"
# include <common_log.h>
#endif /* defined(SNS_LA) || defined(SNS_LA_SIM) */

#if defined(SNS_DSPS_BUILD)
# include <qmi_csi.h>
#endif /* defined(SNS_DSPS_BUILD) */

/*============================================================================

                                  Constants

============================================================================*/
/* The version definition */
#define SNS_MAJOR_VER             0x00
#define SNS_MINOR_VER             0x01

#ifdef QDSP6
#undef SNS_UNIT_TEST            /* For testing PM, undefine if not required */
#undef SNS_SAM_QMI_UNIT_TEST    /* For testing SAM, undefine if not required */
#undef SNS_SMGR_QMI_UNIT_TEST   /* For testing SMGR, undefine if not required */
#undef ADSP_STANDALONE
#endif //QDSP6

/* -----------------------------------------------------------
 *  Module ID definition
 * ---------------------------------------------------------*/
/* Processor ID definition */
#define SNS_MODULE_GRP_MASK       0xE0    /* 3 MSBs for Processor identification field */
#if defined(SNS_LA_SIM) || defined(SNS_PCSIM) || defined(SNS_MDM_SIM)
/* For simulation environments, "flatten" all of the module IDs into one list starting
 * from 0 */
# define SNS_MODULE_APPS           00
# define SNS_MODULE_DSPS           (SNS_MODULE_APPS + SNS_APPS_MODULE_CNT)
# define SNS_MODULE_MDM            (SNS_MODULE_DSPS + SNS_DSPS_MODULE_CNT)
# define SNS_MODULE_UNDEFINED      0xFF
#else
# define SNS_MODULE_APPS           0x00
# define SNS_MODULE_DSPS           0x20
# define SNS_MODULE_MDM            0x60
# define SNS_MODULE_UNDEFINED      0xFF
#endif

/* Module definition - For APPS */
/* NOTE : PRIO 1 to 9 ARE RESERVED !!!!
 * DALSys UCOS Shim allocates these priorities to mutexes at run time
 * during creation of DALSys Sync objects.
 * MAKE SURE THESE PRIORITIES ARE NOT USED IN SEONSORS CODE !!!
 */
#define SNS_APPS_MODULE_CNT       13
#define SNS_MODEM_MODULE_CNT      9

#define SNS_APPS_PRI_MUTEX_BASE   13
#define SNS_APPS_MODULE_PRI_BASE  20

#define SNS_MODEM_PRI_MUTEX_BASE   SNS_APPS_PRI_MUTEX_BASE
#define SNS_MODEM_MODULE_PRI_BASE  SNS_APPS_MODULE_PRI_BASE

#define SNS_MODULE_APPS_PP        (SNS_MODULE_APPS + 00)    /* Power Proxy */
#define SNS_MODULE_APPS_SAM       (SNS_MODULE_APPS + 01)    /* Legacy Sensor Algorithm Manager */
#define SNS_MODULE_APPS_SMR       (SNS_MODULE_APPS + 02)    /* Sensors Message Router */
#define SNS_MODULE_APPS_DIAG      (SNS_MODULE_APPS + 03)    /* Sensors diag agent module */
#define SNS_MODULE_APPS_ACM       (SNS_MODULE_APPS + 04)    /* Sensors application client module */
#define SNS_MODULE_APPS_REG       (SNS_MODULE_APPS + 05)    /* Sensors Registry Module */
#define SNS_MODULE_APPS_EM        (SNS_MODULE_APPS + 06)    /* Sensors Event Manager Module */
#define SNS_MODULE_APPS_INIT      (SNS_MODULE_APPS + 07)    /* Initialization module */
#define SNS_MODULE_APPS_PWR       (SNS_MODULE_APPS + 8)     /* Power module */
#define SNS_MODULE_APPS_TIME      (SNS_MODULE_APPS + 9)     /* Sensors Time Module */
#define SNS_MODULE_APPS_SCM       (SNS_MODULE_APPS + 10)    /* Sensors Calibration Module on Apps */
#define SNS_MODULE_APPS_FILE      (SNS_MODULE_APPS + 11)    /* Sensors File internal module */
#define SNS_MODULE_APPS_SAM2      (SNS_MODULE_APPS + 12)    /* Updated SAM Framework */

/* Module definition - For MODEM */
#define SNS_MODULE_MDM_PP        (SNS_MODULE_MDM + 00)    /* Power Proxy */
#define SNS_MODULE_MDM_SAM       (SNS_MODULE_MDM + 01)    /* Sensor Algorithm Manager */
#define SNS_MODULE_MDM_SMR       (SNS_MODULE_MDM + 02)    /* Sensors Message Router */
#define SNS_MODULE_MDM_DIAG      (SNS_MODULE_MDM + 03)    /* Sensors diag agent module */
#define SNS_MODULE_MDM_ACM       (SNS_MODULE_MDM + 04)    /* Sensors application client module */
#define SNS_MODULE_MDM_REG       (SNS_MODULE_MDM + 05)    /* Sensors Registry Module */
#define SNS_MODULE_MDM_EM        (SNS_MODULE_MDM + 06)    /* Sensors Event Manager Module */
#define SNS_MODULE_MDM_INIT      (SNS_MODULE_MDM + 07)    /* Initialization module */
#define SNS_MODULE_MDM_PWR       (SNS_MODULE_MDM + 8)     /* Power module */
#define SNS_MODULE_MDM_TIME      (SNS_MODULE_MDM + 9)     /* Sensors Time Module */
#define SNS_MODULE_MDM_FILE      (SNS_MODULE_MDM + 10)   /* Sensors File internal module */

/* Priority definition for mutex and modules for APPS*/
#define SNS_MEMMGR_APPS_MUTEX           (SNS_APPS_PRI_MUTEX_BASE + 00)
#define SNS_MODULE_PRI_APPS_EM_MUTEX    (SNS_APPS_PRI_MUTEX_BASE + 01)
#define SNS_MODULE_PRI_APPS_ACM_MUTEX   (SNS_APPS_PRI_MUTEX_BASE + 02)
#define SNS_SMR_APPS_SMDL_MUTEX         (SNS_APPS_PRI_MUTEX_BASE + 03)
#define SNS_SMR_APPS_QUE_MUTEX          (SNS_APPS_PRI_MUTEX_BASE + 04)
#define SNS_PWR_APPS_MUTEX              (SNS_APPS_PRI_MUTEX_BASE + 05)
#define SNS_REG_APPS_MUTEX              (SNS_APPS_PRI_MUTEX_BASE + 06)
#define SNS_TIME_APPS_MUTEX             (SNS_APPS_PRI_MUTEX_BASE + 07)
#define SNS_SAM_APPS_MSG_QUE_MUTEX      (SNS_APPS_PRI_MUTEX_BASE + 8)
#define SNS_SAM_APPS_CLI_QUE_MUTEX      (SNS_APPS_PRI_MUTEX_BASE + 9)
#define SNS_FILE_APPS_MUTEX             (SNS_APPS_PRI_MUTEX_BASE + 10)

#define SNS_MODULE_PRI_APPS_EM     (SNS_APPS_MODULE_PRI_BASE + 0)
#define SNS_MODULE_PRI_APPS_PP     (SNS_APPS_MODULE_PRI_BASE + 1)
#define SNS_MODULE_PRI_APPS_SCM    (SNS_APPS_MODULE_PRI_BASE + 2)
#define SNS_MODULE_PRI_APPS_SAM    (SNS_APPS_MODULE_PRI_BASE + 3)
#define SNS_MODULE_PRI_APPS_ACM    (SNS_APPS_MODULE_PRI_BASE + 4)
#define SNS_MODULE_PRI_APPS_SMR    (SNS_APPS_MODULE_PRI_BASE + 5)
#define SNS_MODULE_PRI_APPS_DIAG   (SNS_APPS_MODULE_PRI_BASE + 6)
#define SNS_MODULE_PRI_APPS_REG    (SNS_APPS_MODULE_PRI_BASE + 7)
#define SNS_MODULE_PRI_APPS_TIME   (SNS_APPS_MODULE_PRI_BASE + 8)
#define SNS_MODULE_PRI_APPS_TIME2  (SNS_APPS_MODULE_PRI_BASE + 9)
#define SNS_MODULE_PRI_APPS_SAM_MR (SNS_APPS_MODULE_PRI_BASE + 10)
#define SNS_MODULE_PRI_APPS_FILE   (SNS_APPS_MODULE_PRI_BASE + 11)

/* Priority definition for mutex and modules for Modem, currently same as Apps and it's not used */
#define SNS_MEMMGR_MDM_MUTEX           SNS_MEMMGR_APPS_MUTEX
#define SNS_MODULE_PRI_MDM_EM_MUTEX    SNS_MODULE_PRI_APPS_EM_MUTEX
#define SNS_MODULE_PRI_MDM_ACM_MUTEX   SNS_MODULE_PRI_APPS_ACM_MUTEX
#define SNS_SMR_MDM_SMDL_MUTEX         SNS_SMR_APPS_SMDL_MUTEX
#define SNS_SMR_MDM_QUE_MUTEX          SNS_SMR_APPS_QUE_MUTEX
#define SNS_PWR_MDM_MUTEX              SNS_PWR_APPS_MUTEX
#define SNS_REG_MDM_MUTEX              SNS_REG_APPS_MUTEX
#define SNS_TIME_MDM_MUTEX             SNS_TIME_APPS_MUTEX

#define SNS_MODULE_PRI_MDM_EM       SNS_MODULE_PRI_APPS_EM
#define SNS_MODULE_PRI_MDM_PP       SNS_MODULE_PRI_APPS_PP
#define SNS_MODULE_PRI_MDM_SAM      SNS_MODULE_PRI_APPS_SAM
#define SNS_MODULE_PRI_MDM_ACM      SNS_MODULE_PRI_APPS_ACM
#define SNS_MODULE_PRI_MDM_SMR      SNS_MODULE_PRI_APPS_SMR
#define SNS_MODULE_PRI_MDM_DIAG     SNS_MODULE_PRI_APPS_DIAG
#define SNS_MODULE_PRI_MDM_REG      SNS_MODULE_PRI_APPS_REG
#define SNS_MODULE_PRI_MDM_TIME     SNS_MODULE_PRI_APPS_TIME
#define SNS_MODULE_PRI_MDM_SAM_MR   SNS_MODULE_PRI_APPS_SAM_MR

/* Stack size definition */
#define SNS_MODULE_STK_SIZE_APPS_PP     (0x200)
#define SNS_MODULE_STK_SIZE_APPS_SAM    (0x200)
#define SNS_MODULE_STK_SIZE_APPS_SMR    (0x200)
#define SNS_MODULE_STK_SIZE_APPS_DIAG   (0x200)
#define SNS_MODULE_STK_SIZE_APPS_ACM    (0x200)
#define SNS_MODULE_STK_SIZE_APPS_REG    (0x200)
#define SNS_MODULE_STK_SIZE_APPS_TIME   (0x200)
#define SNS_MODULE_STK_SIZE_APPS_SCM    (0x200)
#define SNS_MODULE_STK_SIZE_APPS_FILE   (0x200)

#if defined(_WIN32)
/* Win32 doesn't use SAM or SMR */
#undef SNS_MODULE_STK_SIZE_APPS_SAM
#define SNS_MODULE_STK_SIZE_APPS_SAM    (1)
#undef SNS_MODULE_STK_SIZE_APPS_SMR
#define SNS_MODULE_STK_SIZE_APPS_SMR    (1)
#endif /* _WIN32 */

#define SNS_MODULE_STK_SIZE_MODEM_PP     SNS_MODULE_STK_SIZE_APPS_PP
#define SNS_MODULE_STK_SIZE_MODEM_SAM    SNS_MODULE_STK_SIZE_APPS_SAM
#define SNS_MODULE_STK_SIZE_MODEM_SMR    SNS_MODULE_STK_SIZE_APPS_SMR
#define SNS_MODULE_STK_SIZE_MODEM_DIAG   SNS_MODULE_STK_SIZE_APPS_DIAG
#define SNS_MODULE_STK_SIZE_MODEM_ACM    SNS_MODULE_STK_SIZE_APPS_ACM
#define SNS_MODULE_STK_SIZE_MODEM_REG    SNS_MODULE_STK_SIZE_APPS_REG
#define SNS_MODULE_STK_SIZE_MODEM_TIME   SNS_MODULE_STK_SIZE_APPS_TIME


/* Module definition - For DSPS */
#ifdef  SNS_UNIT_TEST
#ifdef SNS_PM_TEST
  #define SNS_DSPS_MODULE_CNT       9
#else
  #define SNS_DSPS_MODULE_CNT       8
#endif
#elif defined(SNS_REG_TEST)
#ifdef SNS_PM_TEST
  #define SNS_DSPS_MODULE_CNT       8
#else
  #define SNS_DSPS_MODULE_CNT       7
#endif
#elif defined(SNS_PCSIM) || defined(SNS_LA_SIM) || defined(QDSP6)
#ifdef SNS_PM_TEST
  #define SNS_DSPS_MODULE_CNT       9
#else
  #define SNS_DSPS_MODULE_CNT       8
#endif
#else
#ifdef SNS_PM_TEST
/* Module count is made as Power test priority + 1
 * This is done so that SMR register does not fail for the
 * power test thread. SMR register can fail because
 * SMR asserts if module count <= module id
 * In this case pm test thread priority is 8 and module count has
 * to be greater than 8 for the check to pass.
 * SMR checks need to be fixed later.
 */
  #define SNS_DSPS_MODULE_CNT       9
#else
  #define SNS_DSPS_MODULE_CNT       6
#endif
#endif
#if defined(SNS_PCSIM) || defined(SNS_LA_SIM) || defined(SNS_BLAST)
# define SNS_DSPS_MUTEX_PRI_BASE  (SNS_APPS_PRI_MUTEX_BASE+10)
# define SNS_DSPS_MODULE_PRI_BASE (SNS_APPS_MODULE_PRI_BASE+20)
#else
# define SNS_DSPS_MUTEX_PRI_BASE  (OS_MAX_DAL_RESERVED_PRIO + 2)
# define SNS_DSPS_MODULE_PRI_BASE (SNS_DSPS_MUTEX_PRI_BASE + 10)
#endif
#define SNS_MODULE_DSPS_IST       (SNS_MODULE_DSPS + 00)    /* Sensors Interrupt Service Task */
#define SNS_MODULE_DSPS_SMGR      (SNS_MODULE_DSPS + 01)    /* Sensor Manager */
#define SNS_MODULE_DSPS_PM        (SNS_MODULE_DSPS + 02)    /* Power Manager */
#define SNS_MODULE_DSPS_SAM       (SNS_MODULE_DSPS + 03)    /* Sensors Algorithm Manager */
#define SNS_MODULE_DSPS_SMR       (SNS_MODULE_DSPS + 04)    /* Sensors Message Router */
#define SNS_MODULE_DSPS_SCM       (SNS_MODULE_DSPS + 05)    /* Sensors Calibration Manager */
#define SNS_MODULE_DSPS_DEBUG     (SNS_MODULE_DSPS + 06)    /* Diag/debug */

/* Priority definition for mutex and modules for DSPS */
#define SNS_MEMMGR_DSPS_MUTEX     (SNS_DSPS_MUTEX_PRI_BASE  + 00)
#define SNS_SMR_SMDL_MUTEX        (SNS_DSPS_MUTEX_PRI_BASE  + 01)
#define SNS_SMR_QUE_MUTEX         (SNS_DSPS_MUTEX_PRI_BASE  + 02)
#define SNS_SMGR_QUE_MUTEX        (SNS_DSPS_MUTEX_PRI_BASE  + 03)
#define SNS_SAM_QUE_MUTEX         (SNS_DSPS_MUTEX_PRI_BASE  + 04)
#define SNS_PM_QUE_MUTEX          (SNS_DSPS_MUTEX_PRI_BASE  + 05)

#define SNS_MODULE_PRI_DSPS_SMR   (SNS_DSPS_MODULE_PRI_BASE + 03)

// TODO: decide the thread priorities
#ifndef QDSP6
#define SNS_MODULE_PRI_DSPS_IST   (SNS_DSPS_MODULE_PRI_BASE + 00)
#define SNS_MODULE_PRI_DSPS_PM    (SNS_DSPS_MODULE_PRI_BASE + 01)
#define SNS_MODULE_PRI_DSPS_SMGR  (SNS_DSPS_MODULE_PRI_BASE + 02)
#define SNS_MODULE_PRI_DSPS_SAM   (SNS_DSPS_MODULE_PRI_BASE + 04)
#define SNS_MODULE_PRI_DSPS_SCM   (SNS_DSPS_MODULE_PRI_BASE + 05)
#define SNS_MODULE_PRI_DSPS_DEBUG (SNS_DSPS_MODULE_PRI_BASE + 06)

#if defined(SNS_LA_SIM) || defined(SNS_PCSIM)
#define SNS_MODULE_DSPS_PLAYBACK     (SNS_MODULE_DSPS + 07)    /* Playback Module */
#define SNS_MODULE_PRI_DSPS_PLAYBACK (SNS_DSPS_MODULE_PRI_BASE + 07)
#define SNS_MODULE_STK_SIZE_DSPS_PLAYBACK (0x200)
#endif
#else //QDSP6
#define SNS_MODULE_STK_SIZE_DSPS_EVMGR    (0x100)
#define SNS_MODULE_PRI_DSPS_EVMGR (SNS_DSPS_MODULE_PRI_BASE + 07)
#define SNS_MODULE_PRI_DSPS_SMGR  (SNS_DSPS_MODULE_PRI_BASE + 06)
#define SNS_MODULE_PRI_DSPS_SAM   (SNS_DSPS_MODULE_PRI_BASE + 05)
#define SNS_MODULE_PRI_DSPS_SCM   (SNS_DSPS_MODULE_PRI_BASE + 04)
#define SNS_MODULE_PRI_DSPS_PM    (SNS_DSPS_MODULE_PRI_BASE + 03)
#define SNS_MODULE_PRI_DSPS_DEBUG (SNS_DSPS_MODULE_PRI_BASE + 02)
#define SNS_MODULE_PRI_DSPS_PLAYBACK (SNS_DSPS_MODULE_PRI_BASE + 01)
#define SNS_MODULE_DSPS_PLAYBACK     (SNS_MODULE_DSPS + 07)    /* Playback Module */
#define SNS_MODULE_STK_SIZE_DSPS_PLAYBACK (0x300)
#endif //QDSP6

#ifdef  SNS_UNIT_TEST
#define SNS_MODULE_DSPS_TEST     (SNS_MODULE_DSPS + 07)
#define SNS_MODULE_PRI_DSPS_TEST (SNS_DSPS_MODULE_PRI_BASE + 07)
#define SNS_MODULE_STK_SIZE_DSPS_TEST 0x100      /* Minimum stack size required for a thread to create */
#endif

#ifdef SNS_REG_TEST /* Sensors registry unit test code */
#define SNS_MODULE_DSPS_REG_TEST     (SNS_MODULE_DSPS + 06)
#define SNS_MODULE_PRI_DSPS_REG_TEST (SNS_DSPS_MODULE_PRI_BASE + 06)
#endif /* SNS_REG_TEST */

#ifdef SNS_PM_TEST
/* Sensors Power manager test thread code used for target bringup */
/* Note change PM Test module number for use on PCSIM. Reason is that
 * on PC SIM playaback module uses same module number
 */
#define SNS_MODULE_DSPS_POWER_TEST     (SNS_MODULE_DSPS + 8)
#define SNS_MODULE_PRI_DSPS_POWER_TEST (SNS_DSPS_MODULE_PRI_BASE + 8)
/* Define a smaller stack for the test code */
#define SNS_MODULE_STK_SIZE_DSPS_PM_TEST     (0x180)
#endif /* SNS_PM_TEST */

/* Stack size difinition */
#define SNS_MODULE_STK_SIZE_DSPS_IST    (0x300)
#define SNS_MODULE_STK_SIZE_DSPS_SMGR   (0x600)
#define SNS_MODULE_STK_SIZE_DSPS_PM     (0x300)
#define SNS_MODULE_STK_SIZE_DSPS_SAM    (0x300)
#define SNS_MODULE_STK_SIZE_DSPS_SMR    (0x300)
#define SNS_MODULE_STK_SIZE_DSPS_SCM    (0x300)
#define SNS_MODULE_STK_SIZE_DSPS_DEBUG    (0x300)

#if defined(SNS_LA) || defined (SNS_LA_SIM) || defined(_WIN32)
#define SNS_MEMMGR_MUTEX SNS_MEMMGR_APPS_MUTEX
#else
#define SNS_MEMMGR_MUTEX SNS_MEMMGR_DSPS_MUTEX
#endif

#if defined(SNS_LA_SIM) || defined(SNS_PCSIM) ||  defined(SNS_MDM_SIM)
# define SNS_MODULE_CNT       (SNS_APPS_MODULE_CNT + SNS_DSPS_MODULE_CNT)
# define SNS_THIS_MODULE_GRP  0x00
#elif defined (SNS_LA) || defined(_WIN32)
# define SNS_MODULE_CNT       SNS_APPS_MODULE_CNT
# define SNS_THIS_MODULE_GRP  SNS_MODULE_APPS
#elif defined (SNS_DSPS_BUILD)
# define SNS_MODULE_CNT       SNS_DSPS_MODULE_CNT
# define SNS_THIS_MODULE_GRP  SNS_MODULE_DSPS
#elif defined (SNS_BLAST)
# define SNS_MODULE_CNT       SNS_MODEM_MODULE_CNT
# define SNS_THIS_MODULE_GRP  SNS_MODULE_MDM
#endif
/*============================================================================

                                  Typedefs

============================================================================*/
/*
 * The error code definition within the sensor framework
 */
typedef enum {
  SNS_SUCCESS   = 0,
  SNS_ERR_BUFFER,       /* Value in the buffer is wrong */
  SNS_ERR_NOMEM,        /* Insufficient memory to process the request */
  SNS_ERR_INVALID_HNDL, /* Invalid handle */
  SNS_ERR_UNKNOWN,      /* Unknown error */
  SNS_ERR_FAILED,       /* Failure in general */
  SNS_ERR_NOTALLOWED,   /* Not allowed operation */
  SNS_ERR_BAD_PARM,     /* One or more parameters have invalid value */
  SNS_ERR_BAD_PTR,      /* Invalid pointer. This may be due to NULL values */
  SNS_ERR_BAD_MSG_ID,   /* The message ID is not supported */
  SNS_ERR_BAD_MSG_SIZE, /* The message size of the message ID does not match */
  SNS_ERR_WOULDBLOCK,   /* Indicating the data is tentatively unavailable */
  SNS_ERR_NOTSUPPORTED  /* Indicating that API is not supported */
} sns_err_code_e;

/*
 * MSM types
 */
typedef enum
{
  SNS_MSM_UNKNOWN,
  SNS_MSM_7x30,
  SNS_MSM_8660,
  SNS_MSM_8960,
  SNS_MSM_8974,
  SNS_MSM_8226,
  SNS_APQ_8084,
  SNS_MSM_8962,
  SNS_PLUTONIUM
} sns_msm_id;

typedef enum
{
  SNS_PLATFORM_FLUID,
  SNS_PLATFORM_MTP,
  SNS_PLATFORM_UNKNOWN,
  SNS_PLATFORM_CDP,
  SNS_PLATFORM_LIQUID,
  SNS_PLATFORM_QRD,
  SNS_PLATFORM_SKUF
} sns_platform;

typedef struct
{
  sns_msm_id msm_id;
  sns_platform platform;
} sns_msm_type;

#if defined(SNS_DSPS_BUILD)
typedef struct {
  qmi_client_handle client_handle;
} client_info_type;
#endif /* defined(SNS_DSPS_BUILD) */

/*============================================================================

                       MACRO definition

============================================================================*/
/*
 * Define SNS_ASSERT related macro definitions
 */
#if defined(SNS_LA) || defined(SNS_LA_SIM)
extern void sns_print_heap_summary();
# define SNS_ASSERT(value) if((value)!=true)                          \
  {                                                                   \
    while (1)                                                         \
    {                                                                 \
      SNS_PRINTF_STRING_FATAL_1(SNS_DBG_MOD_APPS_SMR,                 \
                                "!!!SNS ASSERT!!! line %d",           \
                                (int)__LINE__);                       \
      LOGE("!!!SNS ASSERT:%s!!!File %s line %d",                      \
           #value, __FILE__, (int)__LINE__);                          \
      sns_print_heap_summary();                             \
      abort();                                                        \
    }                                                                 \
  }
#elif defined(QDSP6)
# ifdef SNS_DEBUG
#   define SNS_ASSERT(value) if((value)!=TRUE)                          \
      ERR_FATAL( #value ,0,0,0)
# else /* SNS_DEBUG */
#   define SNS_ASSERT(value) if((value)!=TRUE)                          \
      ERR_FATAL( "" ,0,0,0)
# endif /* SNS_DEBUG */
#elif defined(SNS_DSPS_BUILD)
# ifdef SNS_DEBUG
#   define SNS_ASSERT(value) if((value)!=true)                          \
      ERR_FATAL( #value ,0,0,0)
# else /* SNS_DEBUG */
#   define SNS_ASSERT(value) if((value)!=true)                          \
      ERR_FATAL( "" ,0,0,0)
# endif /* SNS_DEBUG */
#elif defined( SNS_BLAST )
# define SNS_ASSERT(value) if((value)!=TRUE)                          \
    ERR_FATAL( #value ,0,0,0)
#elif defined (_WIN32)
#  ifdef DBG
extern void sns_print_heap_summary();
//WIN32 TODO - every SNS_ASSERT should probably be SNS_ASSERT(FALSE). Linux
//  will abort in them, even on release builds. Some asserts require return,
//  return <value>, break or continue to be gracefully handled, because we
//  cannot use one function like abort().
#    define SNS_ASSERT(value)                                         \
        __pragma(warning(disable:4127)) /* conditional expression is constant */ \
        if((value)!=TRUE) {                                           \
            SNS_PRINTF_STRING_FATAL_1(SNS_DBG_MOD_APPS_SMR,           \
                                      "!!!SNS ASSERT!!! line %d",     \
                                      (int)__LINE__);                 \
            sns_print_heap_summary();                                 \
            DbgBreakPoint();                                          \
        }
#  else
#    define SNS_ASSERT(value) /* Do nothing */
#  endif /* SNS_DEBUG */
#else
# define SNS_ASSERT(value) if((value)!=true)                          \
    while (1){}
#endif

#define SNS_ASSERT_DBG(value) SNS_ASSERT(value)

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)x;
#endif /* UNREFERENCED_PARAMETER */

/*============================================================================

                        External Variable Declarations

============================================================================*/
/*
 * Each module shall sem_post to this semaphore after module initialization
 * is complete.
 */
extern OS_EVENT *sns_init_sem_ptr;
/*
 * Boolean variable to hold the kernel state information that is notified by SMSM
 */
extern bool linux_kernel_suspend;

/*============================================================================

                        Initialized data definition

============================================================================*/
/*
 * Define the static routing table
 * This table shall be sorted by QMI service number(ascending)
 * Also, QMI service number shall be in sequence(i.e No gaps)
 */
#ifdef SNS_SMR_C
#include "sns_common_v01.h"
#include "sns_smgr_api_v01.h"
#include "sns_sam_amd_v01.h"
#include "sns_sam_vmd_v01.h"
#include "sns_sam_rmd_v01.h"
#include "sns_sam_fns_v01.h"
#include "sns_sam_bte_v01.h"
#include "sns_sam_quaternion_v01.h"
#include "sns_sam_gravity_vector_v01.h"
#include "sns_sam_rotation_vector_v01.h"
#include "sns_debug_interface_v01.h"
#include "sns_diag_dsps_v01.h"
#include "sns_reg_api_v01.h"
#include "sns_reg_api_v02.h"
#include "sns_smgr_internal_api_v01.h"
#include "sns_pm_api_v01.h"
#include "sns_sam_mag_cal_v01.h"
#include "sns_sam_filtered_mag_v01.h"
#include "sns_sam_sensor_thresh_v01.h"
#include "sns_time_api_v01.h"
#include "sns_sam_orientation_v01.h"
#include "sns_time_api_v02.h"
#include "sns_sam_basic_gestures_v01.h"
#include "sns_sam_tap_v01.h"
#include "sns_sam_facing_v01.h"
#include "sns_sam_integ_angle_v01.h"
#include "sns_sam_gyro_tap2_v01.h"
#include "sns_sam_gyrobuf_v01.h"
#include "sns_sam_gyroint_v01.h"
#include "sns_file_internal_v01.h"
#include "sns_sam_ped_v01.h"
#include "sns_sam_pam_v01.h"
#include "sns_sam_cmc_v02.h"
#include "sns_sam_distance_bound_v01.h"
#include "sns_sam_smd_v01.h"
#include "sns_sam_game_rotation_vector_v01.h"
#include "sns_oem_1_v01.h"
#include "sns_sam_tilt_detector_v01.h"
#include "sns_smgr_restricted_api_v01.h"
#include "sns_sam_dpc_v01.h"
#include "sns_sam_event_gated_sensor_v01.h"

/* constant definitions for qmi accessor */
#define SNS_GET_SVC_OBJ( svc_name, version )                            \
  {   SNS_##svc_name##_SVC_get_service_object_internal_v##version,      \
      SNS_##svc_name##_SVC_V##version##_IDL_MAJOR_VERS,                 \
      SNS_##svc_name##_SVC_V##version##_IDL_MINOR_VERS,                 \
      SNS_##svc_name##_SVC_V##version##_IDL_TOOL_VERS }

/* Type definition for the routing table */
typedef struct sns_svc_accessor_s
{
  qmi_idl_service_object_type (*get_svc_obj)(int32_t, int32_t, int32_t);
  int32_t maj_ver;
  int32_t min_ver;
  int32_t tool_ver;
} sns_qmi_svc_accessor_s;

/* Type definition for the routing table */
typedef struct sns_rtb_s
{
  uint32_t                      qmi_svc_num;  /* QMI service number */
  uint8_t                       module_id;    /* The module id bound to the QMI service number*/
  const sns_qmi_svc_accessor_s  svc_map;      /* The svc accessor to get the QMI service object */
  qmi_idl_service_object_type   qmi_svc_obj;  /* The QMI service object. This field will be set when initializing */
} sns_rtb_s;

/*
 *  Define Sensor Routing Table which maps QMI service numbers to module IDs.
 *  Note: The table shall be sorted(ASC) by QMI SVC_ID and shall not have a gap.
 *        Also, it is assumed that the SVC_ID of the first entry in the table to be 0
 */

#ifdef _WIN32
#define SAM_ON_APPS_SVC_OBJ(svc, ver) {NULL, 0, 0, 0}
#else
#define SAM_ON_APPS_SVC_OBJ(svc, ver) SNS_GET_SVC_OBJ(svc,ver)
#endif

sns_rtb_s sns_rtb[] = {
    { SNS_SMGR_SVC_ID_V01,                SNS_MODULE_DSPS_SMGR, SNS_GET_SVC_OBJ(SMGR, 01),                    NULL }, /* 0 */
    { SNS_PM_SVC_ID_V01,                  SNS_MODULE_DSPS_PM,   SNS_GET_SVC_OBJ(PM,01),                       NULL }, /* 1 */
    { SNS_SMR_DSPS_SVC_ID_V01,            SNS_MODULE_DSPS_SMR,  {NULL, 0, 0, 0},                              NULL }, /* 2 */
    { SNS_REG_SVC_ID_V01,                 SNS_MODULE_APPS_REG,  {NULL, 0, 0, 0},                              NULL }, /* 3 - DEPRECATED */
    { SNS_SAM_AMD_SVC_ID_V01,             SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_AMD, 01),                 NULL }, /* 4 */
    { SNS_SAM_RMD_SVC_ID_V01,             SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_RMD, 01),                 NULL }, /* 5 */
    { SNS_SAM_VMD_SVC_ID_V01,             SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_VMD, 01),                 NULL }, /* 6 */
    { SNS_DEBUG_SVC_ID_V01,               SNS_MODULE_APPS_DIAG, SNS_GET_SVC_OBJ(DEBUG, 01),                   NULL }, /* 7 */
    { SNS_DIAG_DSPS_SVC_ID_V01,           SNS_MODULE_DSPS_SMR,  SNS_GET_SVC_OBJ(DIAG_DSPS, 01),               NULL }, /* 8 */
    { SNS_SAM_FNS_SVC_ID_V01,             SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_FNS, 01),             NULL }, /* 9 */
    { SNS_SAM_BTE_SVC_ID_V01,             SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_BTE, 01),             NULL }, /* 10 */
    { SNS_SAM_QUAT_SVC_ID_V01,            SNS_MODULE_APPS_SAM,  { NULL, 0, 0, 0},                             NULL }, /* 11 - DEPRECATED */
    { SNS_SAM_GRAVITY_SVC_ID_V01,         SNS_MODULE_APPS_SAM,  { NULL, 0, 0, 0},                             NULL }, /* 12 - DEPRECATED */
    { SNS_SMGR_INTERNAL_SVC_ID_V01,       SNS_MODULE_DSPS_SMGR, SNS_GET_SVC_OBJ(SMGR_INTERNAL, 01),           NULL }, /* 13 */
    { SNS_DEBUG_INTERNAL_SVC_ID_V01,      SNS_MODULE_DSPS_SMR,  {NULL, 0, 0, 0},                              NULL }, /* 14 */
    { SNS_REG2_SVC_ID_V01,                SNS_MODULE_APPS_REG,  SNS_GET_SVC_OBJ(REG2, 02),                    NULL }, /* 15 */
    { SNS_SAM_MAG_CAL_SVC_ID_V01,         SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_MAG_CAL, 01),         NULL }, /* 16 */
    { SNS_SAM_FILTERED_MAG_SVC_ID_V01,    SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_FILTERED_MAG, 01),    NULL }, /* 17 */
    { SNS_SAM_ROTATION_VECTOR_SVC_ID_V01, SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_ROTATION_VECTOR, 01), NULL }, /* 18 */
    { SNS_SAM_QUATERNION_SVC_ID_V01,      SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_QUATERNION, 01),      NULL }, /* 19 */
    { SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01,  SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_GRAVITY_VECTOR, 01),  NULL }, /* 20 */
    { SNS_SAM_SENSOR_THRESH_SVC_ID_V01,   SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_SENSOR_THRESH, 01),       NULL }, /* 21 */
    { SNS_TIME_SVC_ID_V01,                SNS_MODULE_APPS_TIME, { NULL, 0, 0, 0},                             NULL }, /* 22 - DEPRECATED */
    { SNS_SAM_ORIENTATION_SVC_ID_V01,     SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_ORIENTATION, 01),     NULL }, /* 23 */
    { SNS_TIME2_SVC_ID_V01,               SNS_MODULE_APPS_TIME, SNS_GET_SVC_OBJ(TIME2, 02),                   NULL }, /* 24 */
    { SNS_SAM_BASIC_GESTURES_SVC_ID_V01,  SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_BASIC_GESTURES, 01),  NULL }, /* 25 */
    { SNS_SAM_TAP_SVC_ID_V01,             SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_TAP, 01),             NULL }, /* 26 */
    { SNS_SAM_FACING_SVC_ID_V01,          SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_FACING, 01),          NULL }, /* 27 */
    { SNS_SAM_INTEG_ANGLE_SVC_ID_V01,     SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_INTEG_ANGLE, 01),     NULL }, /* 28 */
    { SNS_SAM_GYRO_TAP_SVC_ID_V01,        SNS_MODULE_APPS_SAM,  { NULL, 0, 0, 0},                             NULL }, /* 29  - DEPRECATED */
    { SNS_SAM_GYRO_TAP2_SVC_ID_V01,       SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_GYRO_TAP2, 01),       NULL }, /* 30 */
    { SNS_OEM_1_SVC_ID_V01,               SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(OEM_1, 01),                   NULL }, /* 31  - OEM Use Only */
    { SNS_OEM_2_SVC_ID_V01,               SNS_MODULE_UNDEFINED, { NULL, 0, 0, 0},                             NULL }, /* 32  - OEM Use Only */
    { SNS_OEM_3_SVC_ID_V01,               SNS_MODULE_UNDEFINED, { NULL, 0, 0, 0},                             NULL }, /* 33  - OEM Use Only */
    { SNS_SAM_GYROBUF_SVC_ID_V01,         SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_GYROBUF, 01),             NULL }, /* 34 */
    { SNS_SAM_GYROINT_SVC_ID_V01,         SNS_MODULE_APPS_SAM,  SAM_ON_APPS_SVC_OBJ(SAM_GYROINT, 01),         NULL }, /* 35 */
    { SNS_FILE_INTERNAL_SVC_ID_V01,       SNS_MODULE_APPS_FILE, SNS_GET_SVC_OBJ(FILE_INTERNAL, 01),           NULL }, /* 36 */
    { SNS_SAM_PED_SVC_ID_V01,             SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_PED, 01),                 NULL }, /* 37 */
    { SNS_SAM_PAM_SVC_ID_V01,             SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_PAM, 01),                 NULL }, /* 38 */
    { SNS_SAM_MODEM_SCN_SVC_ID_V01,       SNS_MODULE_DSPS_SAM,  { NULL, 0, 0, 0},                             NULL }, /* 39 */
    { SNS_SAM_SMD_SVC_ID_V01,             SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_SMD, 01),                 NULL }, /* 40 */
    { SNS_SAM_CMC_SVC_ID_V01,             SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_CMC, 02),                 NULL }, /* 41 */
    { SNS_SAM_DISTANCE_BOUND_SVC_ID_V01,  SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_DISTANCE_BOUND, 01),      NULL }, /* 42 */
    { SNS_SAM_GAME_ROTATION_VECTOR_SVC_ID_V01, SNS_MODULE_APPS_SAM, SAM_ON_APPS_SVC_OBJ(SAM_GAME_ROTATION_VECTOR,01), NULL }, /* 43 */
    { SNS_SMGR_RESTRICTED_SVC_ID_V01,     SNS_MODULE_DSPS_SMGR, SNS_GET_SVC_OBJ(SMGR_RESTRICTED, 01),         NULL }, /* 44 */
    { SNS_SAM_PED_ALIGNMENT_SVC_ID_V01,   SNS_MODULE_DSPS_SAM,  { NULL, 0, 0, 0},       NULL }, /* 45 */
    { SNS_SAM_TILT_DETECTOR_SVC_ID_V01,   SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_TILT_DETECTOR, 01),       NULL }, /* 46 */
    { SNS_SAM_INTERNAL_PED_SVC_ID_V01,    SNS_MODULE_DSPS_SAM,  { NULL, 0, 0, 0},                             NULL }, /* 47 */
    { SNS_SAM_DPC_SVC_ID_V01,             SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_DPC, 01),                 NULL }, /* 48 */
    { SNS_SAM_EVENT_GATED_SENSOR_SVC_ID_V01, SNS_MODULE_DSPS_SAM,  SNS_GET_SVC_OBJ(SAM_EVENT_GATED_SENSOR, 01),    NULL }, /* 49 */
};

#define SNS_SMR_RTB_SIZE (sizeof(sns_rtb)/sizeof(sns_rtb_s))
#endif /* SNS_SMR_C */

#endif /* SNS_COMMON_H */
