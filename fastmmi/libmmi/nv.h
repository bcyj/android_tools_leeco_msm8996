/*
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SYSTEM_CORE_MMI_DIAGNV__
#define __SYSTEM_CORE_MMI_DIAGNV__
#include "msg.h"
#include "diag_lsm.h"
#include "diagcmd.h"
#include "diagpkt.h"

#define MAX_PKT_SIZE                1200
#define NV_PKT_SIZE                   134
#define BYTE_PTR( var )               ( (byte *) &(var) )

typedef struct {
    byte data[MAX_PKT_SIZE];
    int item_id;
    int length;
} rsp_t;

typedef enum {
    NV_ESN_I = 0,
    NV_UE_IMEI_I = 550,
    NV_FACTORY_DATA_1_I = 2497,
    NV_FACTORY_DATA_2_I = 2498,
    NV_FACTORY_DATA_3_I = 2499,
    NV_FACTORY_DATA_4_I = 2500,
    NV_OEM_ITEM_1_I = 6853,
    NV_OEM_ITEM_2_I = 6854,
    NV_OEM_ITEM_3_I = 6855,
    NV_OEM_ITEM_4_I = 6856,
    NV_OEM_ITEM_5_I = 6857,
    NV_OEM_ITEM_6_I = 6858,
    NV_OEM_ITEM_7_I = 6859,
    NV_OEM_ITEM_8_I = 6860,
} nv_items_enum_type;

typedef struct {
    byte cmd_code;
    byte msm_hw_version_format;
    byte reserved[2];           /* for alignment / future use */

    uint32 msm_hw_version;
    uint32 mobile_model_id;

    /* The following character array contains 2 NULL terminated strings:
       'build_id' string, followed by 'model_string' */
    char ver_strings[1];

} __attribute__ ((packed)) diag_ext_build_id_rsp_type;

typedef struct {
    uint8_t cmd_code;
    uint8_t subsys_id;
    uint16_t subsys_cmdcode;
} __attribute__ ((packed)) diagpkt_subsys_header_t;

typedef struct {
    diagpkt_subsys_header_t hdr;
    uint16_t sequence_number;
    char path[2];
} __attribute__ ((packed)) efs2_diag_sync_no_wait_req_type;

typedef struct {
    diagpkt_subsys_header_t hdr;
    uint16_t sequence_number;
    uint32_t sync_token;
    int32_t diag_errno;         /* errno if any error or 0 otherwise. */
} __attribute__ ((packed)) efs2_diag_sync_no_wait_rsp_type;

/*!
  @ingroup diag_misc
  @brief
    This procedure encapsulates an NV read operation for the
    diag task.
    Assumes that diag_init has set up the NV command buffer,
    ncmd.

    @param[in] item nv_items_enum_type
    @param[out] buf_ptr to put the read data

    @dependencies
    diag_init

    @sideeffects
    None

    @sa
    None

*/
int diag_nv_read(nv_items_enum_type item,   /*!< Which NV item to read */
                 unsigned char *buf_ptr,    /*!< buffer pointer to put the read data */
                 int len);


/*!
    @ingroup diag_misc
    @brief
    This procedure encapsulates an NV write operation for the
    diag task.
    Assumes that diag_init has set up the NV command buffer,
    ncmd.

    @param[in] item  enum nv_items_enum_type
    @param[in] buf_ptr buffer pointer pointing to the data to be
          written

    @dependencies
    diag_init

    @sideeffects
    None

    @sa
    None

*/
int diag_nv_write(nv_items_enum_type item,  /*!< Which NV item to write */
                  unsigned char *buf_ptr,   /*!< buffer pointer pointing to the data to be written */
                  int len);

void register_callback(void);
int diag_send_cmd(unsigned char cmd, unsigned char *data_ptr, int len);

#endif
