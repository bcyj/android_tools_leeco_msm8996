/******************************************************************************
  @file    sap-api.pb.c
  @brief   SAP api proto for uim remote server

  DESCRIPTION
    Handles uim_remote_server messages

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "sap-api.pb.h"



const pb_field_t MsgHeader_fields[6] = {
    PB_FIELD2(  1, FIXED32 , REQUIRED, STATIC  , FIRST, MsgHeader, token, token, 0),
    PB_FIELD2(  2, ENUM    , REQUIRED, STATIC  , OTHER, MsgHeader, type, token, 0),
    PB_FIELD2(  3, ENUM    , REQUIRED, STATIC  , OTHER, MsgHeader, id, type, 0),
    PB_FIELD2(  4, ENUM    , REQUIRED, STATIC  , OTHER, MsgHeader, error, id, 0),
    PB_FIELD2(  5, BYTES   , REQUIRED, CALLBACK, OTHER, MsgHeader, payload, error, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_CONNECT_REQ_fields[2] = {
    PB_FIELD2(  1, INT32   , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_CONNECT_REQ, max_message_size, max_message_size, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_CONNECT_RSP_fields[3] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_CONNECT_RSP, response, response, 0),
    PB_FIELD2(  2, INT32   , OPTIONAL, STATIC  , OTHER, RIL_SIM_SAP_CONNECT_RSP, max_message_size, response, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_DISCONNECT_REQ_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_DISCONNECT_RSP_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_DISCONNECT_IND_fields[2] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_DISCONNECT_IND, disconnectType, disconnectType, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_APDU_REQ_fields[3] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_APDU_REQ, type, type, 0),
    PB_FIELD2(  2, BYTES   , REQUIRED, POINTER , OTHER, RIL_SIM_SAP_APDU_REQ, command, type, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_APDU_RSP_fields[4] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_APDU_RSP, type, type, 0),
    PB_FIELD2(  2, ENUM    , REQUIRED, STATIC  , OTHER, RIL_SIM_SAP_APDU_RSP, response, type, 0),
    PB_FIELD2(  3, BYTES   , OPTIONAL, POINTER , OTHER, RIL_SIM_SAP_APDU_RSP, apduResponse, response, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_TRANSFER_ATR_REQ_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_TRANSFER_ATR_RSP_fields[3] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_TRANSFER_ATR_RSP, response, response, 0),
    PB_FIELD2(  2, BYTES   , OPTIONAL, POINTER , OTHER, RIL_SIM_SAP_TRANSFER_ATR_RSP, atr, response, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_POWER_REQ_fields[2] = {
    PB_FIELD2(  1, BOOL    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_POWER_REQ, state, state, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_POWER_RSP_fields[2] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_POWER_RSP, response, response, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_RESET_SIM_REQ_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_RESET_SIM_RSP_fields[2] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_RESET_SIM_RSP, response, response, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_STATUS_IND_fields[2] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_STATUS_IND, statusChange, statusChange, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_REQ_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP_fields[3] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP, response, response, 0),
    PB_FIELD2(  2, INT32   , OPTIONAL, STATIC  , OTHER, RIL_SIM_SAP_TRANSFER_CARD_READER_STATUS_RSP, CardReaderStatus, response, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_ERROR_RSP_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ_fields[2] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_REQ, protocol, protocol, 0),
    PB_LAST_FIELD
};

const pb_field_t RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP_fields[2] = {
    PB_FIELD2(  1, ENUM    , REQUIRED, STATIC  , FIRST, RIL_SIM_SAP_SET_TRANSFER_PROTOCOL_RSP, response, response, 0),
    PB_LAST_FIELD
};


