#ifndef _LIBSENSOR1_H_
#define _LIBSENSOR1_H_
/*============================================================================
  @file libsensor1.h

  @brief Defines the types used in the sensor1 remoting library for socket
  communication.

  <br><br>

  DEPENDENCIES:

  Copyright (c) 2010,2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================

  INCLUDE FILES

  ============================================================================*/

#include "sensor1.h"

#include <pthread.h>
#include <stddef.h>

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

/* server socket path for control socket */
#if defined(SNS_LA)
#define SENSOR_CTL_PATH "/dev/socket/"
#elif defined (SNS_LA_SIM)
#define SENSOR_CTL_PATH "/tmp/"
#endif /* SNS_LA */

#define SENSOR_CTL_FILENAME "sensor_ctl_socket"
#define SENSOR_CTL_SOCKET SENSOR_CTL_PATH SENSOR_CTL_FILENAME

/* Maximum size of the QMI encoded data in a message, in bytes */
#define SENSOR_MAX_MSG_SIZE 2048

#ifndef UNREFERENCED_PARAMETER
# define UNREFERENCED_PARAMETER(x) (void)x;
#endif /* UNREFERENCED_PARAMETER */

/*============================================================================
  Type Declarations
  ============================================================================*/

/* Data format for write message on ctl socket */
typedef struct __attribute((__packed__)) libsensor1_ctl_write_s {
  uint32_t svc_num;
  int32_t  msg_id;
  uint8_t  txn_id;
  uint8_t  socket_cmd;
  uint16_t reserved;

  /* Variable sized data */
  uint8_t  data[1];
} libsensor_ctl_write_s;

/* Data format for read message on ctl socket */
typedef struct __attribute((__packed__)) libsensor1_ctl_read_s {
  uint32_t svc_num;
  int32_t  msg_id;
  uint8_t  txn_id;
  uint8_t  msg_type;
  uint8_t  socket_cmd;
  uint8_t  reserved;

  /* Variable sized data */
  uint8_t  data[1];
} libsensor_ctl_read_s;

/* Data format for read message on data socket */
typedef struct __attribute((__packed__)) libsensor1_data_read_s {
  uint32_t svc_num;
  int32_t  msg_id;
  uint8_t  txn_id;
  uint8_t  msg_type;
  uint8_t  socket_cmd;
  uint8_t  reserved;

  /* Variable sized data */
  uint8_t  data[1];
} libsensor_data_read_s;


/* Types of sensor commands send over a socket */
typedef enum libsensor_socket_cmd_e {
  LIBSENSOR_SOCKET_CMD_WRITE_QMI,
  LIBSENSOR_SOCKET_CMD_WRITE_RAW,
  LIBSENSOR_SOCKET_CMD_DISCON_CTL,
  LIBSENSOR_SOCKET_CMD_OPEN_BLOCK,
  LIBSENSOR_SOCKET_CMD_OPEN_SUCCESS
} libsensor_socket_cmd_e;
#endif /* _LIBSENSOR1_H_ */
