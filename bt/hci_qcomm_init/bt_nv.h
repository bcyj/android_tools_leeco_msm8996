#ifndef BT_NV_H
#define BT_NV_H
/*============================================================================
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  FILE: bt_nv.h

  OVERVIEW: Public declarations for the NV parser module.

*==============================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

when        who  what, where, why
----------  ---  -----------------------------------------------------------
2010-02-15   ss  Intial version
2011-09-28  rrr  Moved the implementation to CPP, for having BD address being
                 programmed twice if previous BD address was random generated.
2012-02-22  rrr  Added macro related to persistent file path.
============================================================================*/
#ifdef __cplusplus
extern "C" {
#endif

#include <linux/limits.h>

/* To Enable the logging define the BTNV_DEBUG */
//#define BTNV_DEBUG

#define TRUE 1
#define FALSE 0
#define NV_BD_ADDR_SIZE 6
#define PERSISTENCE_PATH "/persist"
#define BT_NV_FILE_NAME ".bt_nv.bin"

/* BTS Master Clock Reference Value */
typedef long int nv_ps_bt_soc_refclock_enum_type;
#define NV_PS_BT_SOC_REFCLOCK_32MHZ ((nv_ps_bt_soc_refclock_enum_type)0)
#define NV_PS_BT_SOC_REFCLOCK_19P2MHZ ((nv_ps_bt_soc_refclock_enum_type)1)

/* Provides Information of BTS clk sharing */
typedef long int nv_ps_bt_soc_clock_sharing_enum_type;
#define  NV_PS_BT_SOC_CLOCK_SHARING_DISABLED ((nv_ps_bt_soc_clock_sharing_enum_type)0)
#define  NV_PS_BT_SOC_CLOCK_SHARING_ENABLED ((nv_ps_bt_soc_clock_sharing_enum_type)1)


typedef union{
  /* Provides the BT Address */
  unsigned char bd_addr[NV_BD_ADDR_SIZE];
  /* Provides the BTS Master Reference Clock */
  nv_ps_bt_soc_refclock_enum_type bt_soc_refclock_type;
  /* Provides Information of BTS Clk sharing  */
  nv_ps_bt_soc_clock_sharing_enum_type bt_soc_clk_sharing_type;
} nv_persist_item_type;

typedef enum {
  NV_BT_ITEM_MIN,
  NV_BD_ADDR_I,
  NV_BT_SOC_REFCLOCK_TYPE_I,
  NV_BT_SOC_CLK_SHARING_TYPE_I,
  NV_BT_ITEM_MAX
} nv_persist_items_enum_type;


/*  Command codes when command is issued to the NV task.                   */
typedef enum {
  NV_READ_F,          /* Read item */
  NV_WRITE_F          /* Write item */
} nv_persist_func_enum_type;

/*  Returned status codes for requested operation.                         */
typedef enum {
  NV_SUCCESS=0,          /* Request completed okay */
  NV_FAILURE,          /* Command failed, reason other than NVM was full */
  NV_BADCMD,        /* Unrecognizable command field */
  NV_READONLY,      /* Parameter is write-protected and thus read only */
} nv_persist_stat_enum_type;


/*==============================================================
FUNCTION:  bt_nv_cmd
==============================================================*/
/**
  This function is to do the read and write the NV item.

  @see  nv_persist_func_enum_type -> For Read or Write (nv_persist_func_enum_type)
    nv_persist_items_enum_type  -> NV ID number (refer the nv_persist_items_enum_type enum)
    nv_persist_item_type    -> Structure which contains the variables to read or write.
    bIsRandom -> Used only in case of BD address NV item being written.
      bIsRandom = 1 implies that BD address programmed is random generated
      bIsRandom = 0 (default) implies it to be user programmed

  @return  Returns the nagative value on failure.

  @sideeffects Undetermined.
*/
int bt_nv_cmd(nv_persist_func_enum_type nvReadWriteFunc,  nv_persist_items_enum_type nvitem,
                nv_persist_item_type *my_nv_item, int bIsRandom = 0);


#ifdef __cplusplus
}
#endif

#endif
