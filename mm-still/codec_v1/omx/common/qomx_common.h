/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#ifndef __QOMX_COMMON_H_
#define __QOMX_COMMON_H_

#include <stdio.h>
#include <qomx_core.h>
#include "QOMX_JpegExtensions.h"

/**
 *  MACROS and CONSTANTS
 **/
#define OMX_MAX_NUM_PLANES 3

#define DEFAULT_Q_FACTOR 75
#define DEFAULT_THUMB_Q_FACTOR 75

/** qomx_message_t
 *  OMX_MESSAGE_CHANGE_STATE:
 *  OMX_MESSAGE_FLUSH:
 *  OMX_MESSAGE_ABORT:
 *  OMX_MESSAGE_DEINIT:
 *  OMX_MESSAGE_PORT_ENABLE:
 *  OMX_MESSAGE_PORT_DISABLE:
 *  OMX_MESSAGE_START_MAIN_ENCODE:
 *  OMX_MESSAGE_ETB_DONE:
 *  OMX_MESSAGE_FTB_DONE:
 *  OMX_MESSAGE_EVENT_ERROR:
 *  OMX_MESSAGE_CHANGE_STATE_DONE:
 *
 *  OMX message type
 **/
typedef enum {
  OMX_MESSAGE_CHANGE_STATE = 0,
  OMX_MESSAGE_FLUSH,
  OMX_MESSAGE_ABORT,
  OMX_MESSAGE_DEINIT,
  OMX_MESSAGE_PORT_ENABLE,
  OMX_MESSAGE_PORT_DISABLE,
  OMX_MESSAGE_START_MAIN_ENCODE,
  OMX_MESSAGE_START_MAIN_DECODE,
  OMX_MESSAGE_ETB_DONE,
  OMX_MESSAGE_FTB_DONE,
  OMX_MESSAGE_EVENT_ERROR,
  OMX_MESSAGE_CHANGE_STATE_DONE,
  OMX_MESSAGE_PORT_SETTINGS_CHANGED,
  OMX_MESSAGE_PORT_ENABLE_DONE,
  OMX_MESSAGE_START_NEW_ENCODE,
} qomx_message_t;

/** qomx_intermediate_comp_state_t:
 *  OMX_StateLoaded_Pending:
 *  OMX_StateIdle_Pending:
 *  OMX_StatePause_Pending:
 *  OMX_StateExecuting_Pending:
 *  OMX_StateNone:
 *
 *  OMX intermediate states
 **/
typedef enum {
  OMX_StateLoaded_Pending = 0,
  OMX_StateIdle_Pending,
  OMX_StatePause_Pending,
  OMX_StateExecuting_Pending,
  OMX_StateNone
} qomx_intermediate_comp_state_t;

/** qomx_intermediate_port_state_t:
 *  OMX_PORT_ENABLE_PENDING:
 *  OMX_PORT_DISABLE_PENDING:
 *  OMX_PORT_NONE:
 *
 *  OMX port states
 **/
typedef enum {
  OMX_PORT_ENABLE_PENDING = 0,
  OMX_PORT_DISABLE_PENDING,
  OMX_PORT_NONE
} qomx_intermediate_port_state_t;

/** QOMX_Buffer_Data_t:
 *  mHeader: buffer header
 *  mInfo: buffer info
 *  valid: check if the data is valid
 *
 *  OMX port states
 **/
typedef struct _QOMX_Buffer_Data {
  OMX_BUFFERHEADERTYPE mHeader;
  QOMX_BUFFER_INFO mInfo;
  OMX_BOOL valid;
} QOMX_Buffer_Data_t;

#endif //__QOMX_COMMON_H_
