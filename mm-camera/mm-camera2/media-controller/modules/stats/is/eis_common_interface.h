/* eis_common_interface.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __EIS_COMMOM_INTERFACE_H__
#define __EIS_COMMOM_INTERFACE_H__
#include "stats_event.h"

/** _eis_kalman_type:
 *    @A: relation between previous state and current state
 *    @Q: variance of process noise
 *    @R: variance of measurement noise
 *    @H: relation between state and measurement
 *    @x: discrete time controlled process
 *    @P: error covariance
 *    @B: relation between state and control input
 *    @u: control input
 *    @z: measurement
 *
 * This structure maintains the state for the Kalman filter defined by the
 * following equations:
 * State equation:        x_k = A*x_(k-1) + B*u_(k-l) + w_k
 * Measurement equation:  z_k = H*x_k + v_k
 * Estimation error:      e = x_est_k - x_k
 * Error covariance:      P = E[e*e']
 **/
typedef struct
{
  long A;
  long Q;
  long R;
  long H;
  long x;
  long P;
  long B;
  long u;
  long z;
} eis_kalman_type;


/** _eis_init_type:
 *    @margin_x: margin in x direction
 *    @margin_y: margin in y direction
 *    @width:
 *    @height:
 *    @sensor_mount_angle:
 *    @sensor_mount_angle:
 **/
typedef struct {
  unsigned short margin_x;
  unsigned short margin_y;
  unsigned short width;
  unsigned short height;
  unsigned int sensor_mount_angle;
  enum camb_position_t camera_position;
} eis_init_type;
#endif
