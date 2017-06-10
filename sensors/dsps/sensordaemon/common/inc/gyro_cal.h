#ifndef GYRO_CAL_H
#define GYRO_CAL_H

/*============================================================================
  @file gyro_cal.h

  Gyroscope calibration algorithm header file

  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
============================================================================*/

/* $Header: //source/qcom/qct/core/pkg/dsps/dev/kaushiks.accelautocal_v1/core/sensors/scm/algo/inc/gyro_cal.h#1 $ */
/* $DateTime: 2012/06/19 14:40:08 $ */
/* $Author: coresvc $ */

/*============================================================================
  EDIT HISTORY

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. 

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2011-07-05  sc   Re-formated with UNIX newline ending
  2011-06-21  ad   updated gyro bias calibration algorithm
  2011-02-11  ad   initial version  

============================================================================*/

#include <stdint.h>

#define GYRO_CAL_COL (3)

/*gyro calibration motion state*/
typedef enum
{
  GYRO_CAL_MOT_STATE_UNKNOWN = 0,
  GYRO_CAL_MOT_STATE_REST    = 1,
  GYRO_CAL_MOT_STATE_MOVING  = 2
} gyro_cal_mot_state_e;

/*gyro calibration configuration*/
typedef struct
{
   int32_t sampleRate;         // sample rate in Hz, Q16
   int32_t numSamples;         // number of samples
   int64_t varianceThold;      // stationary detect variance threshold
} gyro_cal_config_s;

/*gyro calibration input*/
typedef struct
{
   int32_t data[GYRO_CAL_COL]; // gyro measurement, rad/s, Q16
} gyro_cal_input_s;

/*gyro calibration output*/
typedef struct
{
   int32_t bias[GYRO_CAL_COL]; // estimated gyro bias
   int32_t motionState;        // unknown=0, rest=1, motion=2
} gyro_cal_output_s;

/*gyro calibration state*/
typedef struct
{
   gyro_cal_config_s config;           // gyro calibration configuration
   int64_t variance[GYRO_CAL_COL];     // gyro variance
   int64_t sampleSqSum[GYRO_CAL_COL];  // sum of square of gyro sample data
   int32_t sampleSum[GYRO_CAL_COL];    // sum of gyro sample data
   uint8_t sampleCount;                // count of gyro samples
   uint8_t numSamplesPow2;             // number of samples as power of 2
} gyro_cal_state_s;

/*=========================================================================
  FUNCTION:  gyro_cal_scm_mem_req
  =========================================================================*/
/*!
  @brief
  Provides the algorithm state memory requirement for the specified config

  @param[i] config: pointer to the specified algorithm configuration

  @return
  algorithm state memory requirement in bytes if successful, 0 if error
*/
/*=======================================================================*/
int32_t gyro_cal_scm_mem_req(
   gyro_cal_config_s *config);

/*=========================================================================
  FUNCTION:  gyro_cal_scm_state_reset
  =========================================================================*/
/*!
  @brief
  Reset the algorithm state

  @param[i] config: pointer to the specified algorithm configuration
  @param[i] mem: pointer to the memory allocated for algorithm state

  @return
  pointer to algorithm state
*/
/*=======================================================================*/
gyro_cal_state_s* gyro_cal_scm_state_reset(
   gyro_cal_config_s *config,
   void *mem);

/*=========================================================================
  FUNCTION:  gyro_cal_scm_update
  =========================================================================*/
/*!
  @brief
  Execute the algorithm in the specified state using specified input
  to generate the output at the specified location

  @param[i] state: pointer to algorithm state
  @param[i] input: pointer to algorithm input
  @param[i] output: pointer to algorithm output

  @return None
*/
/*=======================================================================*/
void gyro_cal_scm_update(
   gyro_cal_state_s *state,
   gyro_cal_input_s *input,
   gyro_cal_output_s *output);

#endif /* GYRO_CAL_H */
