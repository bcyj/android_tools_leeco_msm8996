#ifndef _IS_SNS_LIB_H
#define _IS_SNS_LIB_H
/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
*/

/* Co-ordinate convention */
#include <stdint.h> // definition of fixed width integers

/* Co-ordinate system, units and conventions

   The following crude sketch is that of a device sitting face up say on a table in the portrait orientation. Then

    +X  axis is along the (typical) longer axis of the screen, and points away from the observer.
    +Y  axis is along the (typical) shorter axis of the screen, and points to the observer's right.
    +Z  axis is towards the earth - and points towards the earth's center.

       (+X) _ (+Z - into the page)
   I  / \    /|
   I   |    /
   ----|---/--
   |   |  /   |
   |   | /    |
   |   | ________________\ (+Y)
   |          |          /
   |          |
   |          |
   |          |
   -----------
*/

/* Time stamps are in microseconds as measured by a common clock on the application pocessor, We use CLOCK_REALTIME */
/* Gyro - samples are in radian/second, in Q16 format, and follow the co-ordinate convention below                 */
/* Quaternion -

   are unit quaternions - {w, x, y, z} where
    w = cos(theta/2),
    x = Axis_x * sin(theta/2)
    y = Axis_y * sin(theta/2)
    z = Axis_z * sin(theta/2)

   represents a rotation of theta degrees, about the axis Axis_x*i + Axis_y*j + Axis_z*k
   rotations are positive in the anticlockwise direction.
*/
#define MAX_GYRO_SAMPLES 256

typedef struct {
  int32_t   data[3];  /* 0==x, 1==y, 2==z */
  uint64_t  ts;       /* time stamp in microseconds */
} gyro_sample_s;

typedef struct {
  uint32_t        num_elements;   /* number of elements in the buffer */
  gyro_sample_s*  gyro_data;      /* gyro data */
} gyro_data_s;

typedef struct {
  uint64_t t_start;
  uint64_t t_end;
} time_interval_s;

typedef struct {
  double      data[4];         /* (Quat) = data[0] + data[1]*i + data[2]*j + data[3]*k */
  uint32_t    n_gyro_samples;  /* this is the number of samples used during the
                                  integration - needed for alpha calculation */
} quaternion_s;

typedef struct
{
  char sns_input_file[256];
} sns_lib_input_s;



/* Initialize the EIS 2.0 sensors library */
int sns_eis2_init(sns_lib_input_s *input_params);

/* Stop/flush/release any state/resources held by the EIS 2.0 library */
int sns_eis2_stop(void);


/* Returns applicable gyro samples for the current frame. This API mimics the dsps_get_params that will return a
   set of raw gyro samples, perhaps across frame boundaries. */
int get_gyro_samples(
    const time_interval_s*  ts,    /* in: array of size 2 */
    gyro_data_s*            data   /* out: array of size 2.
                                      Memory for the buffers is allocated by the calling function.
                                      We assume MAX_GYRO_SAMPLES worth of memory to be available for each buffer */
);


/* Update the bias.  This bias is applied to each gyro samples prior when the sensor lib calculates
   the integrated gyro data */
int set_bias(
   const int32_t*   bias      /* in: array of size 3
                                 z data not used.  Rely on autocal for z bias correction */
);


/* Integrate the individual gyro samples given in *data and put the integrated values
   in *x, *y, *z.  This API will be used to get roll_shift and del_offset (refer to MATLAB code) */
int get_integrated_gyro_data(
   const gyro_data_s*  data,  /* in */
   const time_interval_s* ts,
   int32_t*            angle  /* out: array of size 3 --> 0==x, 1==y, 2==z */
);


/* Returns pan (needed for alpha calculation) */
int get_last_pan(
   const gyro_data_s*  data,  /* in */
   int32_t*            angle  /* out: array of size 3 --> 0==x, 1==y, 2==z */
);


/* Update the pan filter coefficient. Use this alpha to correct for pan when calculating the quaternion rotation. */
int set_alpha(
   double alpha     /* in */
);

void pan_filter_reset_after_breach(double alpha);

/* Find the total rotation represented by the gyro samples in the buffer and put it in quaternion form in *rotation.  The rotation should be "pan corrected".
   This means that this function may have to run the pan filter using alpha (from set_alpha) */
int get_quaternion_rotation(
   const gyro_data_s*   data,     /* in */
   const time_interval_s* ts,
   quaternion_s*        quat      /* out */
);


/* Finds the standard deviation of the gyro samples given in *data.  The mean of the standard deviations is filtered and used in
   do determine whether the motion warrants correction. */
int get_gyro_stdev(
   const gyro_data_s*  data,        /* in */
   int32_t*            data_stdev   /* out: array of size 3 --> 0==x, 1==y, 2==z */
);


// #define CANNED_SNS_DATA Uncomment to use canned sensor data
void set_sns_apps_offset(uint64_t t);
#endif
