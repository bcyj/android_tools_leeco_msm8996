/***************************************************************************
* Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/
#ifndef __EZTUNE_H__
#define __EZTUNE_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#define EZTUNE_FORMAT_MAX 128
#define EZTUNE_FORMAT_STR 128
#define EZTUNE_MAX_ITEM_VALUE_LEN 128
#define MAX_FD_PER_PROCESS 250

#include <inttypes.h>
#include <sys/types.h>

#include "eztune_items_0301.h"

#include "eztune_diagnostics.h"
#include "eztune_vfe_diagnostics.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "actuator_driver.h"
#include "af_algo_tuning.h"
#include "sensor_common.h"

typedef int boolean;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define Q10 1024

typedef enum {
  EZTUNE_FORMAT_JPG = 0,
  EZTUNE_FORMAT_YUV_422,
  EZTUNE_FORMAT_YUV_420,
  EZTUNE_FORMAT_YVU_422,
  EZTUNE_FORMAT_YVU_420,
  EZTUNE_FORMAT_YCrCb_422,
  EZTUNE_FORMAT_YCrCb_420,
  EZTUNE_FORMAT_YCbCr_422,
  EZTUNE_FORMAT_YCbCr_420,
  EZTUNE_FORMAT_INVALID
} eztune_prev_format_t;

typedef enum {
    TUNING_SET_RELOAD_CHROMATIX,
    TUNING_SET_RELOAD_AFTUNE,
    TUNING_SET_AUTOFOCUS_TUNING,
    TUNING_SET_VFE_COMMAND,
    TUNING_SET_POSTPROC_COMMAND,
    TUNING_SET_3A_COMMAND,
    TUNING_SET_ACTION_JPEGSNAPSHOT,
    TUNING_SET_ACTION_RAWSNAPSHOT,
    TUNING_SET_AEC_LOCK,
    TUNING_SET_AEC_UNLOCK,
    TUNING_SET_AWB_LOCK,
    TUNING_SET_AWB_UNLOCK,
    TUNING_SET_MAX
} tune_set_t;

typedef enum {
    EZTUNE_MISC_GET_VERSION,
    EZTUNE_MISC_APPLY_CHANGES,
    EZTUNE_MISC_WRITE_INI,
    EZTUNE_MISC_READ_INI,
    EZTUNE_MISC_LIST_INI,
} eztune_misc_message_t;

struct eztune_set_val_t {
    int item_num;
    int table_index;
    char value_string[EZTUNE_MAX_ITEM_VALUE_LEN];
};

struct eztune_t {
    chromatix_parms_type *chromatixptr;
    chromatix_parms_type *snapchromatixptr;
    chromatix_VFE_common_type *common_chromatixptr;
    actuator_driver_params_t *af_driver_ptr;
    af_algo_tune_parms_t *af_tune_ptr;
    cam_metadata_info_t *metadata;
    ez_af_tuning_params_t *af_tuning;
};

typedef enum {
    EZT_D_FLOAT = 1,
    EZT_D_INT8,
    EZT_D_INT16,
    EZT_D_INT32,
    EZT_D_UINT8,
    EZT_D_UINT16,
    EZT_D_UINT32,
    EZT_D_DOUBLE,
    EZT_D_EXPENT,
    EZT_D_INVALID
} eztune_item_data_t;

typedef enum {
    EZT_T_CHROMATIX,
    EZT_T_DIGANOSTIC,
    EZT_T_AUTOFOCUS,
    EZTUNE_TYPE_INVALID
} eztune_item_type_t;

typedef enum {
    EZT_WRITE_FLAG = 0,
    EZT_READ_FLAG = 1,
    EZT_ACTION_FLAG = (1<<2),
    EZT_CHROMATIX_FLAG = (1<<3),
    EZT_3A_FLAG = (1<<4),
    EZT_AUTOFOCUS_FLAG = (1<<5),
} eztune_item_reg_flag_t;

typedef enum {
    EZT_SIZE_DIAG,
    EZT_SIZE_ONE
} eztune_item_size;


typedef struct {
    eztune_parms_list_t id;
    char name[EZTUNE_MAX_ITEM_VALUE_LEN];
    eztune_item_data_t data;
    eztune_item_type_t type;
    eztune_item_reg_flag_t reg_flag;
    uint32_t offset;
    uint16_t size;
    /*eztune_item_action_t req_action; */
} eztune_item_t;

#if defined(__cplusplus)
extern "C" {
#endif

void tuning_set_vfe(void *ctrl, vfemodule_t module, optype_t optype, int32_t value);

void tuning_set_pp(void *ctrl, pp_module_t module, optype_t optype, int32_t value);

void tuning_set_3a(void *ctrl, aaa_set_optype_t optype, int32_t value);

void tuning_set_autofocus(void *ctrl, aftuning_optype_t optype, uint8_t value);

int eztune_get_diagnostic_item(struct eztune_t *ezctrl, char *output_buf,
                               int offset, eztune_parms_list_t id, uint16_t table_index);

int eztune_get_af_item(struct eztune_t *ezctrl, char *output_buf, int offset,
                       eztune_parms_list_t id, uint16_t table_index);

void eztune_change_item(eztune_item_t *item, struct eztune_set_val_t *item_data, struct eztune_t *ezctrl);

eztune_item_t eztune_get_item (int i);

#if defined(__cplusplus)
}
#endif

#endif /* __EZTUNE_H__ */
