/*==============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
 *============================================================================*/

#ifndef _EIS_INTERFACE_H_
#define _EIS_INTERFACE_H_

/*=============================================================================
                                    eis_interface.h
=============================================================================*/

/*=============================================================================
  Include Files
=============================================================================*/

/*=============================================================================
  Macro Definitions
=============================================================================*/

/* EIS error codes */
#define EIS_SUCCESS             0
#define EIS_ERR_BASE            0x00
#define EIS_ERR_FAILED          (EIS_ERR_BASE + 1)
#define EIS_ERR_NO_MEMORY       (EIS_ERR_BASE + 2)
#define EIS_ERR_BAD_HANDLE      (EIS_ERR_BASE + 3)
#define EIS_ERR_BAD_POINTER     (EIS_ERR_BASE + 4)
#define EIS_ERR_BAD_PARAM       (EIS_ERR_BASE + 5)

/* Number of samples needed to for correlation calculation */
#define CORR_WINDOW_LEN 15


/*=============================================================================
  Kalman filter input structure
  State equation:        x_k = A*x_(k-1) + B*u_(k-l) + w_k
  Measurement equation:  z_k = H*x_k + v_k
  Estimation error:      e = x_est_k - x_k
  Error covariance:      P = E[e*e']
=============================================================================*/
typedef struct
{
  long A;    // Relation between previous state and current state
  long Q;    // Variance of process noise
  long R;    // Variance of measurement noise
  long H;    // Relation between state and measurement
  long x;    // Discrete time controlled process
  long P;    // Error covariance
  long B;    // Relation between state and control input
  long u;    // Control input
  long z;    // Measurement
} eis_kalman_type;


typedef struct {
  long x;
  long y;
} eis_position_type;


typedef struct {
  /* margin in x, y direction to allow image stabilization */
  unsigned short margin_x;
  unsigned short margin_y;
  unsigned short width;
  unsigned short height;
} eis_init_type;


/*=============================================================================
  The main EIS structure
=============================================================================*/
typedef struct
{
  /* Current frame number */
  unsigned long frame_num;

  /* Correlation data */
  eis_position_type past_dis_offsets[CORR_WINDOW_LEN];
  /* A sliding window of past DIS offset values used for
   * correlation calculation */
  eis_position_type past_eis_offsets[CORR_WINDOW_LEN];
  /* A sliding window of past EIS offset values used for
   * correlation calculation */

  /* Scale factors in the */
  float w_scale;
  float h_scale;

  eis_position_type prev_bias; /* Bias from previous frame */
  eis_kalman_type kalman_params_x; /* Kalman filter parameters for x axis */
  eis_kalman_type kalman_params_y; /* Kalman filter parameters for y axis */

  /* Pan data */
  eis_position_type pan_offset; /* Current frame's pan value */
  eis_position_type alpha; /* Current frame's pan parameter alpha */
  eis_position_type int_offset; /* Integrated pixel offset */

  /* Output offset */
  eis_position_type eis_frame_offset;

  unsigned long margin_x;
  unsigned long margin_y;

  eis_init_type init_data;
} eis_context_type;


/*=============================================================================

FUNCTION  eis_init

DESCRIPTION
  EIS initialization routine that must be called prior to any other EIS APIs.

ARGUMENTS IN
  p_init_param - EIS initialization structure pointer
  p_eis_context - EIS context pointer (Allocated by client)

ARGUMENTS OUT
  None

RETURN VALUE
  None

=============================================================================*/
int eis_init
(
    const eis_init_type *p_init_param,
    eis_context_type *p_eis_context
);


/*=============================================================================

FUNCTION  eis_stabilize_frame

DESCRIPTION
  This function is called per frame to perform EIS.

ARGUMENTS IN
  p_eis_context - EIS context pointer
  p_gyro_offset - gyro offset pointer
  p_dis_offset - DIS offset pointer

ARGUMENTS OUT
  p_eis_offset - EIS offset pointer
  p_offset_valid - offset valid flag pointer

RETURN VALUE
  EIS_SUCCESS on success, EIS error code otherwise.

=============================================================================*/
int eis_stabilize_frame
(
    eis_context_type *p_eis_context,
    eis_position_type *p_gyro_offset,
    eis_position_type *p_dis_offset,
    eis_position_type *p_eis_offset,
    unsigned long *p_offset_valid
);


/*=============================================================================

FUNCTION  eis_exit

DESCRIPTION
  EIS termination routine.

ARGUMENTS IN
  p_eis_context - EIS context pointer

ARGUMENTS OUT
  None

RETURN VALUE
  None

=============================================================================*/
int eis_exit
(
    eis_context_type *p_eis_context
);
#endif
