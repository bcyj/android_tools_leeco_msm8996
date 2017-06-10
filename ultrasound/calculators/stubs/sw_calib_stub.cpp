/*===========================================================================
                           sw_calib_stub.cpp

DESCRIPTION: Provide stub to SW calib lib.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "SW_Calib_stub"

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include "dpencalib.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define STUB_MSG_INTERVAL 5000
/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/

/*============================================================================
  FUNCTION:  dpencalib_init
============================================================================*/
DPC_ERR DLL_EXPORT dpencalib_init(void *workspace)
{
  LOGW("%s: Stub.",
       __FUNCTION__);

  return DPENCALIB_SUCCESS;
}

/*============================================================================
  FUNCTION:  dpencalib_get_size
============================================================================*/
DPC_ERR DLL_EXPORT dpencalib_get_size(unsigned int *size)
{
  LOGW("%s: Stub.",
       __FUNCTION__);

  return DPENCALIB_SUCCESS;
}

/*============================================================================
  FUNCTION:  dpencalib_version
============================================================================*/
DPC_ERR DLL_EXPORT dpencalib_version(int *major, int *minor, int *subminor)
{
  LOGW("%s: Stub.",
       __FUNCTION__);

  *major = 0;
  *minor = 0;
  *subminor = 0;
  return DPENCALIB_SUCCESS;
}

/*============================================================================
  FUNCTION:  dpencalib_dump
============================================================================*/
DPC_ERR DLL_EXPORT dpencalib_dump(int enable)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return DPENCALIB_SUCCESS;
}

/*============================================================================
  FUNCTION:  dpencalib_calib
============================================================================*/
DPC_ERR DLL_EXPORT dpencalib_calib(void *workspace,
                                   const void *calib_packet, int length)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return DPENCALIB_SUCCESS;
}

/*============================================================================
  FUNCTION:  dpencalib_process_frame
============================================================================*/
DPC_ERR DLL_EXPORT dpencalib_process_frame(void *workspace,
                                           const struct dpencalib_frame *frame)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return DPENCALIB_SUCCESS;
}

/*============================================================================
  FUNCTION:  dpencalib_get_calib_packet
============================================================================*/
DPC_ERR DLL_EXPORT dpencalib_get_calib_packet(void *workspace,
                                              void *calib_packet, int *length)
{
  LOGW("%s: Stub.",
       __FUNCTION__);
  return DPENCALIB_SUCCESS;
}

/*============================================================================
  FUNCTION:  dpencalib_get_status
============================================================================*/
DPC_ERR DLL_EXPORT dpencalib_get_status(void *workspace,
    struct dpencalib_status *status)
{
  status->status = DPCSTATUS_NO_PEN;
  LOGW("%s: Stub.",
       __FUNCTION__);
  return DPENCALIB_SUCCESS;
}
