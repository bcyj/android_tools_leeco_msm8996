/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  dload_protocol.h : Interface to the DMSS-DL "DLOAD" protocol
 *
 */
#ifndef DLOAD_PROTOCOL_H
#define DLOAD_PROTOCOL_H

#include "common.h"

#define DLOAD_CMD_PARAM_REQ_SIZE              1
#define DLOAD_CMD_NOP_SIZE                    1
#define DLOAD_CMD_RESET_SIZE                  1
#define DLOAD_CMD_MDM_DBG_QUERY_SIZE          1
#define DLOAD_CMD_MEM_READ_REQ_SIZE           7
#define DLOAD_CMD_GO_SIZE                     5
#define DLOAD_CMD_UNFRAMED_MEM_READ_REQ_SIZE 12

/* Memory read parameters */
#define PROTOCOL_VERSION_UNFRAMED_MEMORY_READ 7
#define MAX_FRAMED_READ_LENGTH                2040
#define MAX_UNFRAMED_READ_LENGTH              4080

#define DLOAD_WRITE_SIZE_THRESHOLD 1536

enum dload_cmd_id
{
    DLOAD_CMD_ACK           = 0x02,
    DLOAD_CMD_NAK           = 0x03,
    DLOAD_CMD_GO            = 0x05,
    DLOAD_CMD_NOP,
    DLOAD_CMD_PARAM_REQ,
    DLOAD_CMD_PARAM_RESP,
    DLOAD_CMD_RESET         = 0x0A,
    DLOAD_CMD_WRITE_32BIT   = 0x0F,
    DLOAD_CMD_MEM_DBG_QUERY = 0x10,
    DLOAD_CMD_MEM_DBG_INFO_RESP,
    DLOAD_CMD_MEM_READ_REQ,
    DLOAD_CMD_MEM_READ_RESP,
    DLOAD_CMD_MEM_UNFRAMED_READ_REQ,
    DLOAD_CMD_MEM_UNFRAMED_READ_RESP,
    DLOAD_LAST_CMD_ID
};

boolean dload_init_buffers ();
boolean dload_ping_target ();
boolean collect_ram_dumps ();
boolean transfer_hex_file (const char* filename);

#endif