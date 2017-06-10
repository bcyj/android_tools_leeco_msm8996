#ifndef _SNS_PWR_H_
#define _SNS_PWR_H_
/*============================================================================
  @file sns_pwr.h

  @brief
  Declares functions used for booting and turning on/off the DSPS.

  <br><br>

  DEPENDENCIES: None.

  Copyright (c) 2010,2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //source/qcom/qct/core/sensors/dsps/apps/common/main/latest/pwr/inc/sns_pwr.h#6 $ 
  $DateTime: 2011/11/21 15:35:20 $

  when       who    what, where, why 
  ---------- --- -----------------------------------------------------------
  2011-11-08 gju Add sns_pwr_get_msm_type function declaration.
  2011-05-24  hm Used the PM QOS interface to specify the minimum latency 
                 required by the sensors subsytem to balance between power 
                 utilization and performance
  2010-11-09 jtl Initial revision

  ============================================================================*/


/*============================================================================

  INCLUDE FILES

  ============================================================================*/
#include "sns_common.h"

#include <stdbool.h>
#include <stdint.h>


/*============================================================================
  Typedefs
  ============================================================================*/

/* Voting masks for entities interested in voting for power on/off */
#define SNS_PWR_VOTE_ACM 0x00000001

/*============================================================================
 * Global Data Definitions
 ============================================================================*/

/*============================================================================
  Externalized Function Declarations
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_pwr_get_msm_type

  ===========================================================================*/
/*!
  @brief Returns the msm_type object.  Contains chipset and platform.
*/
sns_msm_type
sns_pwr_get_msm_type( );

/*===========================================================================

  FUNCTION:   sns_pwr_set_cpu_latency

  ===========================================================================*/
/*!
  @brief Sets the required CPU latency (in the apps processor) using the Linux 
         PM QOS interface

  This would potentially prevent and/or allow the MSM from entering certain 
  power states (based on the latency set in the apps processor). As an example, 
  if we have a client streaming data at 20Hz, the system should possibly not 
  go into full suspend but can go into power collapse. Based on the latency 
  specified the kernel will determine which power state it should go into without 
  compromising system performance.

  For details refer to kernel\Documentation\power\pm_qos_interface.txt
*/
void 
sns_pwr_set_cpu_latency( int32_t hz );


/*===========================================================================

  FUNCTION:   sns_pwr_boot

  ===========================================================================*/
/*!
  @brief Boots the DSPS.

  This has no effect if the DSPS is already booted. This is used for one-time
  initialization at startup.

  @return
  SNS_SUCCESS: Booted the DSPS.
  Any other error code: DSPS not booted.
*/
/*=========================================================================*/
sns_err_code_e
sns_pwr_boot( void );

/*===========================================================================

  FUNCTION:   sns_pwr_on

  ===========================================================================*/
/*!
  @brief Votes to turn on the DSPS.

  If anyone votes to turn on the DSPS, it will be turned on.

  @param[i] vote_mask: Mask of entities voting for power on.

  @dependencies
  DSPS should have been booted.

  @return
  SNS_SUCCESS: Booted the DSPS.
  Any other error code: DSPS not booted.
*/
/*=========================================================================*/
sns_err_code_e
sns_pwr_on( uint32_t vote_mask );


/*===========================================================================

  FUNCTION:   sns_pwr_off

  ===========================================================================*/
/*!
  @brief Turns off the DSPS

  If everyone votes to turn off the DSPS, it will be turned off.

  @param[i] vote_mask: Mask of entities voting for power off.

  @dependencies
  The DSPS must have been booted first.

  @return
  SNS_SUCCESS: Booted the DSPS.
  Any other error code: DSPS not booted.
*/
/*=========================================================================*/
sns_err_code_e
sns_pwr_off( uint32_t vote_mask );

/*===========================================================================

  FUNCTION:   sns_pwr_get_pil_fd

  ===========================================================================*/
/*!
  @brief Accessor function for msm_dsps file descriptor.

  @return File descriptor.
*/
/*=========================================================================*/
int
sns_pwr_get_pil_fd( void );

/*===========================================================================

  FUNCTION:   sns_pwr_set_wake_lock

  ===========================================================================*/
/*!
  @brief Grabs a wakelock for the sensor code.

  Calling this function multiple times will to lock the wakelock will only
  lock it once. That is: only one call to disable the wakelock is necessary.

  @param[i] lock true: lock the wakelock. false: unlock.
*/
/*=========================================================================*/
void
sns_pwr_set_wake_lock( boolean lock );

/*===========================================================================

  FUNCTION:   sns_pwr_crash_shutdown

  ===========================================================================*/
/*!
  @brief Called in case of a crash shutdown.

  Cleans up all power-related activites before the sensor daemon shuts down.

  @dependencies
  None.

*/
/*=========================================================================*/
void
sns_pwr_crash_shutown( void );

#endif /* _SNS_PWR_H_ */
