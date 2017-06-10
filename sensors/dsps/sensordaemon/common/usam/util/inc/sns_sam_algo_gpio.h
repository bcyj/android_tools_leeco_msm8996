#ifndef SNS_SAM_ALGO_GPIO
#define SNS_SAM_ALGO_GPIO
/*============================================================================
  @file sns_sam_algo_gpio.h

  GPIO related functions for SAM algorithms.

  This is for using SNS_TEST GPIO pins to measure algo latency. It will be
  enabled only when FEATURE_TEST_ALGO_GPIO flag is set in Sensors.scons file

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/

/*============================================================================
  Function Declarations
  ===========================================================================*/

/**
 * Initialize SNS_TEST GPIO pins
 *
 */
void
sns_sam_init_gpio( void );

/**
 * Set GPIO state on a particular GPIO pin. This function can only be used by
 * one algorithm each time, since all sensors algos are sharing the same GPIO
 * pins for this test.
 *
 * @param[i] gpioIndex: index of GPIO array to indicate which GPIO is to be set
 * @param[i] setToHigh: whether set this GPIO to high
 *
 * @return None
*/
void
sns_sam_algo_set_gpio(uint32_t gpioIndex, bool setToHigh);

#endif /* SNS_SAM_ALGO_GPIO */
