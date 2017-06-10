/******************************************************************************
  @file    uim_remote_client.pb.c
  @brief

  DESCRIPTION
    Handles uim_remote_client message nanopb encode/decode related functions.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include "uim_remote_client.pb.h"



const pb_field_t com_qualcomm_uimremoteclient_MessageTag_fields[6] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, com_qualcomm_uimremoteclient_MessageTag, type, type, 0),
    PB_FIELD2(  2, ENUM    , REQUIRED, STATIC  , OTHER, com_qualcomm_uimremoteclient_MessageTag, id, type, 0),
    PB_FIELD2(  3, ENUM    , OPTIONAL, STATIC  , OTHER, com_qualcomm_uimremoteclient_MessageTag, error, id, 0),
    PB_FIELD2(  4, FIXED32 , OPTIONAL, STATIC  , OTHER, com_qualcomm_uimremoteclient_MessageTag, token, error, 0),
    PB_FIELD2(  5, BYTES   , OPTIONAL, CALLBACK, OTHER, com_qualcomm_uimremoteclient_MessageTag, payload, token, 0),
    PB_LAST_FIELD
};

const pb_field_t com_qualcomm_uimremoteclient_UimRemoteEventReq_fields[5] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, com_qualcomm_uimremoteclient_UimRemoteEventReq, event, event, 0),
    PB_FIELD2(  2, BYTES   , OPTIONAL, CALLBACK, OTHER, com_qualcomm_uimremoteclient_UimRemoteEventReq, atr, event, 0),
    PB_FIELD2(  3, BOOL    , OPTIONAL, STATIC  , OTHER, com_qualcomm_uimremoteclient_UimRemoteEventReq, wakeup_support, atr, 0),
    PB_FIELD2(  4, ENUM    , OPTIONAL, STATIC  , OTHER, com_qualcomm_uimremoteclient_UimRemoteEventReq, error_code, wakeup_support, 0),
    PB_LAST_FIELD
};

const pb_field_t com_qualcomm_uimremoteclient_UimRemoteEventResp_fields[2] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, com_qualcomm_uimremoteclient_UimRemoteEventResp, response, response, 0),
    PB_LAST_FIELD
};

const pb_field_t com_qualcomm_uimremoteclient_UimRemoteApduReq_fields[3] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, com_qualcomm_uimremoteclient_UimRemoteApduReq, status, status, 0),
    PB_FIELD2(  2, BYTES   , OPTIONAL, CALLBACK, OTHER, com_qualcomm_uimremoteclient_UimRemoteApduReq, apduResponse, status, 0),
    PB_LAST_FIELD
};

const pb_field_t com_qualcomm_uimremoteclient_UimRemoteApduResp_fields[2] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, com_qualcomm_uimremoteclient_UimRemoteApduResp, status, status, 0),
    PB_LAST_FIELD
};

const pb_field_t com_qualcomm_uimremoteclient_UimRemoteApduInd_fields[2] = {
    PB_FIELD2(  1, BYTES   , REQUIRED, CALLBACK, FIRST, com_qualcomm_uimremoteclient_UimRemoteApduInd, apduCommand, apduCommand, 0),
    PB_LAST_FIELD
};

const pb_field_t com_qualcomm_uimremoteclient_UimRemotePowerUpInd_fields[3] = {
    PB_FIELD2(  1, INT32   , OPTIONAL, STATIC  , FIRST, com_qualcomm_uimremoteclient_UimRemotePowerUpInd, timeout, timeout, 0),
    PB_FIELD2(  2, ENUM    , OPTIONAL, STATIC  , OTHER, com_qualcomm_uimremoteclient_UimRemotePowerUpInd, voltageclass, timeout, 0),
    PB_LAST_FIELD
};

const pb_field_t com_qualcomm_uimremoteclient_UimRemotePowerDownInd_fields[2] = {
    PB_FIELD2(  1, ENUM    , OPTIONAL, STATIC  , FIRST, com_qualcomm_uimremoteclient_UimRemotePowerDownInd, mode, mode, 0),
    PB_LAST_FIELD
};


