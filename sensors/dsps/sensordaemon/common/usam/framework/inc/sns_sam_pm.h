#ifndef SNS_SAM_PM_H
#define SNS_SAM_PM_H

/*============================================================================
  @file sns_sam_pm.h

  @brief
  Data structures and constants used only by the SAM Framework.

  @note
  File line length should generally be limited to <= 80 columns.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/
#include "sns_common.h"
#include "sns_debug_str.h"
#include "sns_em.h"
#include "sns_osa.h"
#include "sns_memmgr.h"
#include "sns_pm.h"
#include "sns_sam_pm.h"
#include "sns_sam_algo_api.h"


/*==============================================================================
  MACROS
==============================================================================*/
#ifdef SNS_USES_ISLAND
#define SNS_SAM_UIMAGE_CODE __attribute__((section ("text.uSAM")))
#define SNS_SAM_UIMAGE_DATA __attribute__((section ("data.uSAM")))
#else
#define SNS_SAM_UIMAGE_CODE
#define SNS_SAM_UIMAGE_DATA
#endif /* USES_ISLAND */

#define SNS_SAM_UIMAGE_ALGOINST_ISSUPPORTED(algoInst) \
( SNS_SAM_ALGOINST_UIMAGE == algoInst->imageMode )

/* Maximum number of preallocated algorithm input and output data blocks*/
#define SNS_SAM_ALGO_MAX_IO 5

// When SAM queue count exceeds this threshold, SAM votes for higher MIPS - Turbo
#define SNS_SAM_Q_UPPER_THRESH  8    // PEND: Set the upper threshold after profiling
// When SAM queue count goes below this threshold, SAM votes for lower MIPS - Normal
#define SNS_SAM_Q_LOWER_THRESH  2    // PEND: Set the lower threshold after profiling

/*==============================================================================
  INTERNAL DEFINITIONS AND TYPES
==============================================================================*/

/**
*  SAM can be in one of two states: operating entirely from within TCM memory
* (uImage mode) or requiring access to DDR memory for its operation
* (Big Image mode).
*/
typedef enum
{
/** SAM can operate in uImage mode*/
  SNS_SAM_MODE_UIMAGE,
/** SAM can operate only in Big Image mode*/
  SNS_SAM_MODE_BIGIMAGE,
} sns_sam_uimage_mode_type;

typedef enum
{
/**
*  SAM framework can not enter uImage mode. To be used only by the SAM
*  framework for all operations that requires SAM to be in Big Image Mode unless
*  an operation specific enum is specified below.
*/
   SNS_SAM_UIMAGE_BLOCK_SAM_BUSY = 0,

/**
*  Algorithm Instance cannot be run in uImage mode. Exit uImage to service the
*  algorithm.
*/
   SNS_SAM_UIMAGE_BLOCK_ALGO,

/**
*  SAM has to be outside uImage mode to process a  duty cycle timer event.
*/
   SNS_SAM_UIMAGE_BLOCK_TIMER,

/**
*  SAM has to be outside uImage mode to process a message.
*/
   SNS_SAM_UIMAGE_BLOCK_MSG,

/**
*  There was insufficient memory to process a malloc in uImage. SAM will exit
*  uImage to reattempt the malloc operation in big image mode.
*/
   SNS_SAM_UIMAGE_BLOCK_NOMEM,

/**
*  SAM is waiting for registry to become available before switching into uImage mode
*/
   SNS_SAM_UIMAGE_BLOCK_WAIT_REG,


/**
*  This shall be the last item of the enum
*/
   SNS_SAM_UIMAGE_BLOCK_COUNT
} sns_sam_uimage_blocking_type;

typedef enum
{
/**
*  The algorithm code is statically placed in DDR, or it was determined that
*  this algorithm should not (for some reason) run in uImage
*/
  SNS_SAM_ALGOINST_BIGIMAGE,

/**
* This algorithm instance can run in uImage mode indinitely
*/
  SNS_SAM_ALGOINST_UIMAGE,

/**
* This algorithm instance may be moved into uImage when TCM memory becomes
* available.
*/
  SNS_SAM_ALGOINST_NOMEM_UIMAGE

} sns_sam_algoinst_uimage_mode;

/**
* SAM's internal representation of the current CPU speed
* Default is SNS_SAM_MIPS_NORMAL
*/
typedef enum
{
  SNS_SAM_MIPS_NORMAL,
  SNS_SAM_MIPS_TURBO,
  SNS_SAM_MIPS_MAX = SNS_SAM_MIPS_TURBO
}sns_sam_internal_mips_type;

/*
*  Numeric value of the MIPS rates used by SAM. SAM will always attempt to be at
*  the lowest possible MIPS rate.
*/
static const uint32_t sns_sam_mips_table[SNS_SAM_MIPS_MAX+1] = {40, 200};

/**
*  The uImage state of SAM.
*/
typedef struct
{
  /** The uimage vote array. Every time we exit uImage, the corresponding array
      element is incremented once. Everytime we vote to enter uImage, the element
      is decremented. All the elements of this array should be 0 in order to enter
      uImage.
  */
  uint8_t                    sns_sam_uimage_block[SNS_SAM_UIMAGE_BLOCK_COUNT];
  /** The current SAM vote for uImage*/
  sns_sam_uimage_mode_type   sam_status;
  /** Timestamp at the last time we changed the Image mode in ticks*/
sns_sam_timestamp            timestamp;
  /** A running average of the time spent in uImage mode, in ticks*/
  uint64_t                   uimage_avg_run_length;
  /** A running average of the time spent in Bigimage mode, in ticks*/
  uint64_t                   bigimage_avg_run_length;
  /** Set to true when the uimage_hysterisis_timer is registered */
  bool                       uimage_timer_active;
} sns_sam_uimage_status;

/*==============================================================================
  FUNCTION:  sns_sam_uimage_exit
==============================================================================*/
/**
*  Exits the uImage mode. This function is can be called at any part of the
*  SAM framework, including from within algorithms.
*
* The caller of this function is also responsible for calling sns_sam_uimage_vote_enter
*
* @param[i] blockType  Reason for exiting uImage mode
*
* @return SAM_EFAILED  If operation failed
*         SAM_ENONE    If operation was successful
*/
SNS_SAM_UIMAGE_CODE sns_sam_err
  sns_sam_uimage_exit( sns_sam_uimage_blocking_type blockType);

/*==============================================================================
  FUNCTION:  sns_sam_uimage_vote_enter
==============================================================================*/
/**
*  This function call is used to unblock SAM from entering uImage mode. It is the
*  responsibility of the caller of sns_sam_uimage_exit to call this function when
*  it is possible to re-enter uImage mode.
*
* @param[i] blockType  Reason for exiting uImage mode
*/
  void
  sns_sam_uimage_vote_enter( sns_sam_uimage_blocking_type blockType);

/*==============================================================================
  FUNCTION:  sns_sam_pm_vote_mips
==============================================================================*/
/**
*  Vote for higher/lower MIPS
*
*  @param[i] newMips  Vote for Normal or Turbo MIPS
*/
SNS_SAM_UIMAGE_CODE void
  sns_sam_pm_vote_mips( sns_sam_internal_mips_type newMips );

/*==============================================================================
  FUNCTION:  sns_sam_pm_adjust_mips
==============================================================================*/
/**
*  Adjust the SAM MIPS vote based on queue size
*/
SNS_SAM_UIMAGE_CODE void sns_sam_pm_adjust_mips( void );

/*==============================================================================
  FUNCTION:  sns_sam_pm_enter_sleep
==============================================================================*/
/**
*  Vote to enter the low power sleep mode. SAM should use this only when it is expected to be in an idle state for a long period
*  of time. This is not related to sns_sam_uimage_exit or sns_sam_uimage_vote_enter.
*/
void sns_sam_pm_enter_sleep( void );

/*==============================================================================
  FUNCTION:  sns_sam_pm_exit_sleep
==============================================================================*/
/**
*  Vote to exit the sleep mode and resume SAM's latest image mode.
*/
void sns_sam_pm_exit_sleep( void );

/*==============================================================================
  FUNCTION:  sns_sam_pm_init
==============================================================================*/
/**
*  To be called during SAM initializations. This function is responsible
*  for initializing the data structures to be maintained by SAM for uImage.
*
* @return SAM_EFAILED  If operation failed
*         SAM_ENONE    If operation was successful
*/
sns_sam_err sns_sam_pm_init( void );

/*==============================================================================
  FUNCTION:  sns_sam_pm_ap_suspend
==============================================================================*/
/**
 * Determine whether the AP is in suspend.  Used when determining whether to
 * send indications, and at what rate to send batched indications.
 *
 * @return true if AP in suspend, false otherwise
 */
SNS_SAM_UIMAGE_CODE bool
sns_sam_pm_ap_suspend( void );

#endif /* SNS_SAM_PM_H*/
