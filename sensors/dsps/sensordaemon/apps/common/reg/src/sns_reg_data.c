/*============================================================================
  @file sns_reg_data.c

  @brief
    This file defines the structure of the contents the Sensors Registry. It
    also includes utility functions for initializing the registry. The
    contents of this file need to be updated when modifying the structure of
    the registry.

  <br><br>

  DEPENDENCIES:

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential
  ============================================================================*/

/*============================================================================

  To add new Data Items and Groups to the Sensors Registry:

  1. Add new data item ID and update item count in sns_reg_common.h
  2. Add data item default values in sns_reg-common.h
  3. Add new data group ID and update group count in sns_reg_common.h
  4. Add new data group structure below (example: sns_reg_alp_data_group_s)
  5. Add data item info (data type and offset in registry file) in the
     sns_reg_item_info array below
  6. Add data group info (group size and offset in registry file) in the
     sns_reg_group_info array below
  7. Add a case for the new data element in sns_reg_get_default() below to
     return the default value of the data item
  8. Add a case for the new data group in sns_group_data_init() below to return
     the default values of the date items in the data group

  ============================================================================*/

/*============================================================================

  INCLUDE FILES

  ============================================================================*/
#include "comdef.h"
#include "sensor1.h"
#include "sns_common.h"
#include "sns_reg.h"
#include "sns_reg_common.h"
#include "sns_reg_api_v01.h"
#include "sns_reg_api_v02.h"
#include "sns_smgr_api_v01.h"
#include "sns_debug_str.h"
#include "sns_memmgr.h"
#include "sns_pwr.h"
#include "sns_reg_platform.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/
#define MAX_GROUP_SIZE SNS_REG_MAX_GROUP_BYTE_COUNT_V01

#ifdef SNS_PCSIM
#define SNS_PRINTF_STRING_ERROR_1(level, msg, p1) printf(msg, p1)
#endif

/*============================================================================
  Static Variable Definitions
  ============================================================================*/

const sns_reg_item_info_s sns_reg_item_info[] =
{
  /***************************************************************************/
  /* Group:  SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V01                                              */
  /* Item Type              Offset from start of file     Version    Item ID                         */
  { SNS_REG_TYPE_INT32,     0,                            2,1,       SNS_REG_ITEM_ACC_X_BIAS_V01     },
  { SNS_REG_TYPE_INT32,     4,                            2,1,       SNS_REG_ITEM_ACC_Y_BIAS_V01     },
  { SNS_REG_TYPE_INT32,     8,                            2,1,       SNS_REG_ITEM_ACC_Z_BIAS_V01     },
  { SNS_REG_TYPE_UINT32,    12,                           2,1,       SNS_REG_ITEM_ACC_X_SCALE_V01    },
  { SNS_REG_TYPE_UINT32,    16,                           2,1,       SNS_REG_ITEM_ACC_Y_SCALE_V01    },
  { SNS_REG_TYPE_UINT32,    20,                           2,1,       SNS_REG_ITEM_ACC_Z_SCALE_V01    },
  /* Max offset for this group should be MAX_GROUP_SIZE - 1} */

  /* Group: SNS_REG_DRIVER_GROUP_PROX_LIGHT_V01                                                           */
  /* Item Type            Offset from start of file       Version    Item ID                                         */
  { SNS_REG_TYPE_UINT8,   (MAX_GROUP_SIZE),               2,1,       SNS_REG_ITEM_ALP_VISIBLE_LIGHT_TRANS_RATIO_V02  },  /* SNS_REG_ITEM_ALP_VISIBLE_LIGHT_TRANS_RATIO_V01 */
  { SNS_REG_TYPE_UINT8,   (MAX_GROUP_SIZE) + 1,           2,1,       SNS_REG_ITEM_ALP_IR_LIGHT_TRANS_RATIO_V02       },  /* SNS_REG_ITEM_ALP_IR_LIGHT_TRANS_RATIO_V01      */
  { SNS_REG_TYPE_UINT16,  (MAX_GROUP_SIZE) + 2,           2,1,       SNS_REG_ITEM_ALP_DC_OFFSET_V02                  },  /* SNS_REG_ITEM_ALP_DC_OFFSET_V01                 */
  { SNS_REG_TYPE_UINT16,  (MAX_GROUP_SIZE) + 4,           2,1,       SNS_REG_ITEM_ALP_NEAR_THRESHOLD_V02             },  /* SNS_REG_ITEM_ALP_NEAR_THRESHOLD_V01            */
  { SNS_REG_TYPE_UINT16,  (MAX_GROUP_SIZE) + 6,           2,1,       SNS_REG_ITEM_ALP_FAR_THRESHOLD_V02              },  /* SNS_REG_ITEM_ALP_FAR_THRESHOLD_V01             */
  { SNS_REG_TYPE_UINT16,  (MAX_GROUP_SIZE) + 8,           2,1,       SNS_REG_ITEM_ALP_PRX_FACTOR_V02                 },  /* SNS_REG_ITEM_ALP_PRX_FACTOR_V01             */
  { SNS_REG_TYPE_UINT16,  (MAX_GROUP_SIZE) + 10,          2,1,       SNS_REG_ITEM_ALP_ALS_FACTOR_V02                 },  /* SNS_REG_ITEM_ALP_ALS_FACTOR_V01             */
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 12,          2,1,       SNS_REG_ITEM_ALP_DS1_V02                 },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 16,          2,1,       SNS_REG_ITEM_ALP_DS2_V02                 },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 20,          2,1,       SNS_REG_ITEM_ALP_DS3_V02                 },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 24,          2,1,       SNS_REG_ITEM_ALP_DS4_V02                 },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 28,          2,1,       SNS_REG_ITEM_ALP_DS5_V02                 },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 32,          2,1,       SNS_REG_ITEM_ALP_DS6_V02                 },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 36,          2,1,       SNS_REG_ITEM_ALP_DS7_V02                 },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 40,          2,1,       SNS_REG_ITEM_ALP_DS8_V02                 },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 44,          2,1,       SNS_REG_ITEM_ALP_DS9_V02                 },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 48,          2,1,       SNS_REG_ITEM_ALP_DS10_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 52,          2,1,       SNS_REG_ITEM_ALP_DS11_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 56,          2,1,       SNS_REG_ITEM_ALP_DS12_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 60,          2,1,       SNS_REG_ITEM_ALP_DS13_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 64,          2,1,       SNS_REG_ITEM_ALP_DS14_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 68,          2,1,       SNS_REG_ITEM_ALP_DS15_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 72,          2,1,       SNS_REG_ITEM_ALP_DS16_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 76,          2,1,       SNS_REG_ITEM_ALP_DS17_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 80,          2,1,       SNS_REG_ITEM_ALP_DS18_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 84,          2,1,       SNS_REG_ITEM_ALP_DS19_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 88,          2,1,       SNS_REG_ITEM_ALP_DS20_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 92,          2,1,       SNS_REG_ITEM_ALP_DS21_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 96,          2,1,       SNS_REG_ITEM_ALP_DS22_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 100,         2,1,       SNS_REG_ITEM_ALP_DS23_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 104,         2,1,       SNS_REG_ITEM_ALP_DS24_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 108,         2,1,       SNS_REG_ITEM_ALP_DS25_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 112,         2,1,       SNS_REG_ITEM_ALP_DS26_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 116,         2,1,       SNS_REG_ITEM_ALP_DS27_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 120,         2,1,       SNS_REG_ITEM_ALP_DS28_V02                },
  { SNS_REG_TYPE_UINT32,  (MAX_GROUP_SIZE) + 124,         2,1,       SNS_REG_ITEM_ALP_DS29_V02                },
  /* Max offset for this group should be (MAX_GROUP_SIZE * 2) - 1*/

  /* Group: SNS_REG_SAM_GROUP_AMD_V01                                                            */
  /* Item Type              Offset from start of file    Version    Item ID                                */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 2),        2,1,       SNS_REG_ITEM_AMD_DEF_ACC_SAMP_RATE_V01 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 2) + 4,    2,1,       SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V01    },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 2) + 8,    2,1,       SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V01    },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 2) + 12,   2,11,      SNS_REG_ITEM_AMD_DEF_SENSOR_REPORT_RATE_V02 },
  /* Max offset for this group should be (MAX_GROUP_SIZE * 3) - 1 */

  /* Group: SNS_REG_SAM_GROUP_VMD_V01                                                            */
  /* Item Type              Offset from start of file    Version    Item ID                                */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 3),        2,1,       SNS_REG_ITEM_VMD_DEF_ACC_SAMP_RATE_V01 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 3) + 4,    2,1,       SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V01    },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 3) + 8,    2,1,       SNS_REG_ITEM_VMD_INT_CFG_PARAM2_V01    },
  /* Max offset for this group should be (MAX_GROUP_SIZE * 4) - 1 */

  /* Group: SNS_REG_SAM_GROUP_RMD_V01                                                            */
  /* Item Type              Offset from start of file    Version    Item ID                                */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 4),        2,1,       SNS_REG_ITEM_RMD_DEF_ACC_SAMP_RATE_V01 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 4) + 4,    2,1,       SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V01    },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 4) + 8,    2,1,       SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V01    },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 4) + 12,   2,1,       SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V01    },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 4) + 16,   2,1,       SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V01    },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 4) + 20,   2,11,      SNS_REG_ITEM_RMD_DEF_SENSOR_REPORT_RATE_V02 },
  /* Max offset for this group should be (MAX_GROUP_SIZE * 5) - 1 */

  /* Group: SNS_REG_SCM_GROUP_GYRO_CAL_ALGO_V01                                                  */
  /* Item Type              Offset from start of file  Version    Item ID                                */
  { SNS_REG_TYPE_INT64,     (MAX_GROUP_SIZE * 5),      2,1,       SNS_REG_ITEM_GYRO_CAL_DEF_VAR_THOLD_V01   },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 5) + 8,  2,1,       SNS_REG_ITEM_GYRO_CAL_DEF_SAMP_RATE_V01   },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 5) + 12, 2,1,       SNS_REG_ITEM_GYRO_CAL_DEF_NUM_SAMP_V01    },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 5) + 16, 2,1,       SNS_REG_ITEM_GYRO_CAL_DEF_ENABLE_ALGO_V01 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 5) + 17, 2,11,      SNS_REG_ITEM_GYRO_CAL_DEF_SENSOR_REPORT_RATE_V02 },
  /* Max offset for this group should be (MAX_GROUP_SIZE * 6) - 1 */

  /* Group: SNS_REG_SCM_GROUP_ACCEL_DYN_CAL_PARAMS_V01                                                       */
  /* Item Type              Offset from start of file     Version    Item ID                                            */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6),         2,1,       SNS_REG_ITEM_ACC_X_DYN_BIAS_V01     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 4,     2,1,       SNS_REG_ITEM_ACC_Y_DYN_BIAS_V01     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 8,     2,1,       SNS_REG_ITEM_ACC_Z_DYN_BIAS_V01     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 12,    2,1,       SNS_REG_ITEM_ACC_X_DYN_SCALE_V01    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 16,    2,1,       SNS_REG_ITEM_ACC_Y_DYN_SCALE_V01    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 20,    2,1,       SNS_REG_ITEM_ACC_Z_DYN_SCALE_V01    },

  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 6) + 24,    2,3,       SNS_REG_ITEM_ACC_DYN_CAL_HEADER_V02              },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 6) + 25,    2,3,       SNS_REG_ITEM_ACC_DYN_CAL_TEMP_BIN_SIZE_V02       },

  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 6) + 26,    2,3,       SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP1_V02   },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 6) + 27,    2,3,       SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP1_V02     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 28,    2,3,       SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP1_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 32,    2,3,       SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP1_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 36,    2,3,       SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP1_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 40,    2,3,       SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP1_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 44,    2,3,       SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP1_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 48,    2,3,       SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP1_V02          },

  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 6) + 52,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP2_V02   },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 6) + 53,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP2_V02     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 54,     2,3,       SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP2_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 58,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP2_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 62,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP2_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 66,     2,3,       SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP2_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 70,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP2_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 74,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP2_V02          },

  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 6) + 78,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP3_V02   },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 6) + 79,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP3_V02     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 80,     2,3,       SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP3_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 84,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP3_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 88,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP3_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 92,     2,3,       SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP3_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 96,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP3_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 100,    2,3,       SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP3_V02          },

  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 6) + 104,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP4_V02   },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 6) + 105,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP4_V02     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 106,     2,3,       SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP4_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 110,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP4_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 114,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP4_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 118,     2,3,       SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP4_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 122,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP4_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 126,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP4_V02          },

  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 6) + 130,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP5_V02   },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 6) + 131,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP5_V02     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 132,     2,3,       SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP5_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 136,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP5_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 140,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP5_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 144,     2,3,       SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP5_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 148,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP5_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 152,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP5_V02          },

  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 6) + 156,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP6_V02   },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 6) + 157,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP6_V02     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 158,     2,3,       SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP6_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 162,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP6_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 166,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP6_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 170,     2,3,       SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP6_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 174,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP6_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 178,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP6_V02          },

  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 6) + 182,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP7_V02   },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 6) + 183,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP7_V02     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 184,     2,3,       SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP7_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 188,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP7_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 192,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP7_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 196,     2,3,       SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP7_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 200,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP7_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 204,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP7_V02          },

  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 6) + 208,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP8_V02   },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 6) + 209,     2,3,       SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP8_V02     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 210,     2,3,       SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP8_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 214,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP8_V02           },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 6) + 218,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP8_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 222,     2,3,       SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP8_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 226,     2,3,       SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP8_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 6) + 230,     2,3,       SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP8_V02          },


  /* Max offset for this group should be MAX_GROUP_SIZE * 7 - 1} */

  /* Group: SNS_REG_SCM_GROUP_GYRO_DYN_CAL_PARAMS_V01                                                        */
  /* Item Type              Offset from start of file     Version    Item ID                                            */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 7),         2,1,       SNS_REG_ITEM_GYRO_X_DYN_BIAS_V01     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 7) + 4,     2,1,       SNS_REG_ITEM_GYRO_Y_DYN_BIAS_V01     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 7) + 8,     2,1,       SNS_REG_ITEM_GYRO_Z_DYN_BIAS_V01     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 7) + 12,    2,1,       SNS_REG_ITEM_GYRO_X_DYN_SCALE_V01    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 7) + 16,    2,1,       SNS_REG_ITEM_GYRO_Y_DYN_SCALE_V01    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 7) + 20,    2,1,       SNS_REG_ITEM_GYRO_Z_DYN_SCALE_V01    },
  /* Max offset for this group should be MAX_GROUP_SIZE * 8 - 1} */

  /* Group: SNS_REG_SCM_GROUP_GYRO_FAC_CAL_PARAMS_V01                                                        */
  /* Item Type              Offset from start of file     Version    Item ID                                            */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 8),         2,1,       SNS_REG_ITEM_GYRO_X_BIAS_V01     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 8) + 4,     2,1,       SNS_REG_ITEM_GYRO_Y_BIAS_V01     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 8) + 8,     2,1,       SNS_REG_ITEM_GYRO_Z_BIAS_V01     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 8) + 12,    2,1,       SNS_REG_ITEM_GYRO_X_SCALE_V01    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 8) + 16,    2,1,       SNS_REG_ITEM_GYRO_Y_SCALE_V01    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 8) + 20,    2,1,       SNS_REG_ITEM_GYRO_Z_SCALE_V01    },
  /* Max offset for this group should be MAX_GROUP_SIZE * 9 - 1} */

  /* Group: SNS_REG_GROUP_SENSOR_TEST_V02                                                                    */
  /* Item Type              Offset from start of file    Version    Item ID                                            */
  { SNS_REG_TYPE_UINT8,    (MAX_GROUP_SIZE * 9),         2,1,       SNS_REG_ITEM_RAW_DATA_MODE_V02     }, /* SNS_REG_ITEM_RAW_DATA_MODE_V02 */
  { SNS_REG_TYPE_UINT32,   (MAX_GROUP_SIZE * 9) + 1,     2,1,       SNS_REG_ITEM_TEST_EN_FLAGS_V02     }, /* SNS_REG_ITEM_TEST_EN_FLAGS_V02 */
  /* Max offset for this group should be MAX_GROUP_SIZE * 10 - 1} */

  /* Group: SNS_REG_DRIVER_GROUP_ACCEL_V02                                                    */
  /* Item Type              Offset from start of file     Version    Item ID                             */
  { SNS_REG_TYPE_INT8,    (MAX_GROUP_SIZE * 10),          2,1,       SNS_REG_DRIVER_ACCEL_X_ORIENT_V02 },
  { SNS_REG_TYPE_INT8,    (MAX_GROUP_SIZE * 10) + 1,      2,1,       SNS_REG_DRIVER_ACCEL_Y_ORIENT_V02 },
  { SNS_REG_TYPE_INT8,    (MAX_GROUP_SIZE * 10) + 2,      2,1,       SNS_REG_DRIVER_ACCEL_Z_ORIENT_V02 },

  /* Group: SNS_REG_DRIVER_GROUP_ACCEL_2_V02                                                 */
  /* Item Type              Offset from start of file     Version    Item ID                            */
  { SNS_REG_TYPE_INT8,     (MAX_GROUP_SIZE * 11),         2,1,       SNS_REG_DRIVER_ACCEL2_X_ORIENT_V02 },
  { SNS_REG_TYPE_INT8,     (MAX_GROUP_SIZE * 11) + 1,     2,1,       SNS_REG_DRIVER_ACCEL2_Y_ORIENT_V02 },
  { SNS_REG_TYPE_INT8,     (MAX_GROUP_SIZE * 11) + 2,     2,1,       SNS_REG_DRIVER_ACCEL2_Z_ORIENT_V02 },

  /* Group: SNS_REG_DRIVER_GROUP_GYRO_V02                                                  */
  /* Item Type              Offset from start of file     Version    Item ID                          */
  { SNS_REG_TYPE_INT8,     (MAX_GROUP_SIZE * 12),         2,1,       SNS_REG_DRIVER_GYRO_X_ORIENT_V02 },
  { SNS_REG_TYPE_INT8,     (MAX_GROUP_SIZE * 12) + 1,     2,1,       SNS_REG_DRIVER_GYRO_Y_ORIENT_V02 },
  { SNS_REG_TYPE_INT8,     (MAX_GROUP_SIZE * 12) + 2,     2,1,       SNS_REG_DRIVER_GYRO_Z_ORIENT_V02 },

  /* Group: SNS_REG_DRIVER_GROUP_MAG_V02                                                  */
  /* Item Type              Offset from start of file     Version    Item ID                         */
  { SNS_REG_TYPE_INT8,     (MAX_GROUP_SIZE * 13),         2,1,       SNS_REG_DRIVER_MAG_X_ORIENT_V02 },
  { SNS_REG_TYPE_INT8,     (MAX_GROUP_SIZE * 13) + 1,     2,1,       SNS_REG_DRIVER_MAG_Y_ORIENT_V02 },
  { SNS_REG_TYPE_INT8,     (MAX_GROUP_SIZE * 13) + 2,     2,1,       SNS_REG_DRIVER_MAG_Z_ORIENT_V02 },
  /* Max offset for this group should be MAX_GROUP_SIZE * 14 - 1} */

  /* Group: SNS_REG_SAM_GROUP_FMV_PARAMS_V02                                                     */
  /* Item Type              Offset from start of file     Version    Item ID                                            */
  { SNS_REG_TYPE_UINT32,   (MAX_GROUP_SIZE * 14),         2,1,       SNS_REG_ITEM_FMV_TC_ACCURACY_0_V02     },
  { SNS_REG_TYPE_UINT32,   (MAX_GROUP_SIZE * 14) + 4,     2,1,       SNS_REG_ITEM_FMV_TC_ACCURACY_1_V02     },
  { SNS_REG_TYPE_UINT32,   (MAX_GROUP_SIZE * 14) + 8,     2,1,       SNS_REG_ITEM_FMV_TC_ACCURACY_2_V02     },
  { SNS_REG_TYPE_UINT32,   (MAX_GROUP_SIZE * 14) + 12,    2,1,       SNS_REG_ITEM_FMV_TC_ACCURACY_3_V02     },
  { SNS_REG_TYPE_UINT32,   (MAX_GROUP_SIZE * 14) + 16,    2,1,       SNS_REG_ITEM_FMV_GYRO_GAP_THRESH_V02   },
  { SNS_REG_TYPE_FLOAT,    (MAX_GROUP_SIZE * 14) + 20,    2,1,       SNS_REG_ITEM_FMV_MAG_GAP_FACTOR_V02    },
  { SNS_REG_TYPE_FLOAT,    (MAX_GROUP_SIZE * 14) + 24,    2,1,       SNS_REG_ITEM_FMV_ROT_FOR_ZUPT_V02      },
  { SNS_REG_TYPE_FLOAT,    (MAX_GROUP_SIZE * 14) + 28,    2,1,       SNS_REG_ITEM_FMV_MAX_MAG_INNOVATION_V02},
  { SNS_REG_TYPE_INT32,    (MAX_GROUP_SIZE * 14) + 32,    2,11,      SNS_REG_ITEM_FMV_DEF_SENSOR_REPORT_RATE_V02 },
  /* Max offset for this group should be MAX_GROUP_SIZE * 15 - 1} */

  /* Group: SNS_REG_SAM_GROUP_GRAVITY_VECTOR_PARAMS_V02                                                     */
  /* Item Type              Offset from start of file     Version    Item ID                                            */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 15),        2,1,        SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM1_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 15) + 4,    2,1,        SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM2_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 15) + 8,    2,1,        SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM3_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 15) + 12,   2,1,        SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM4_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 15) + 16,   2,1,        SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM5_V02     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 15) + 20,   2,11,       SNS_REG_ITEM_GRAVITY_VECTOR_DEF_SENSOR_REPORT_RATE_V02 },
  /* Max offset for this group should be MAX_GROUP_SIZE * 16 - 1} */

  /* Group: SNS_REG_SAM_GROUP_ORIENTATION_PARAMS_V02                                                     */
  /* Item Type              Offset from start of file     Version    Item ID                                        */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 16),        2,1,       SNS_REG_ITEM_ORIENTATION_INT_CFG_PARAM1_V02     },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 16) + 4,    2,3,       SNS_REG_ITEM_MAG_CAL_LAT_NUM_SAMPLES_V02        },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 16) + 5,    2,3,       SNS_REG_ITEM_FUSION_MIN_SAMP_RATE_HZ_V02        },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 16) + 6,    2,11,      SNS_REG_ITEM_ORIENTATION_DEF_SENSOR_REPORT_RATE_V02 },
  /* Max offset for this group should be MAX_GROUP_SIZE * 17 - 1} */

  /* Group: SNS_REG_SAM_GROUP_GYRO_AMD_V02                                                            */
  /* Item Type              Offset from start of file     Version    Item ID                          */
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 17),        2,1,       SNS_REG_ITEM_GYRO_AMD_INT_CFG_PARAM1_V02    },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 17) + 4,    2,1,       SNS_REG_ITEM_GYRO_AMD_INT_CFG_PARAM2_V02    },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 17) + 8,    2,3,       SNS_REG_ITEM_GYRO_AMD_INT_CFG_PARAM3_V02    },
  /* Max offset for this group should be (MAX_GROUP_SIZE * 18) - 1 */

  /* Group : SNS_REG_SAM_GROUP_GYRO_TAP_V02 2650                                          */
  /* Item Type              Offset from start of file   Version    Item ID                         */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18),      2,1,       SNS_REG_GYRO_TAP_VERSION_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 4,  2,1,       SNS_REG_GYRO_TAP_SCENARIO_0_V02    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 8,  2,1,       SNS_REG_GYRO_TAP_FLAGS_0_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 12, 2,1,       SNS_REG_GYRO_TAP_TIME_SECS_00_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 16, 2,1,       SNS_REG_GYRO_TAP_TIME_SECS_01_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 20, 2,1,       SNS_REG_GYRO_TAP_TIME_SECS_02_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 24, 2,1,       SNS_REG_GYRO_TAP_TIME_SECS_03_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 28, 2,1,       SNS_REG_GYRO_TAP_TIME_SECS_04_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 32, 2,1,       SNS_REG_GYRO_TAP_RATIO_00_V02      },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 36, 2,1,       SNS_REG_GYRO_TAP_RATIO_01_V02      },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 40, 2,1,       SNS_REG_GYRO_TAP_THRESH_00_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 44, 2,1,       SNS_REG_GYRO_TAP_THRESH_01_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 48, 2,1,       SNS_REG_GYRO_TAP_THRESH_02_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 52, 2,1,       SNS_REG_GYRO_TAP_THRESH_03_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 56, 2,1,       SNS_REG_GYRO_TAP_THRESH_04_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 60, 2,1,       SNS_REG_GYRO_TAP_SCENARIO_1_V02    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 64, 2,1,       SNS_REG_GYRO_TAP_FLAGS_1_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 68, 2,1,       SNS_REG_GYRO_TAP_TIME_SECS_10_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 72, 2,1,       SNS_REG_GYRO_TAP_TIME_SECS_11_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 76, 2,1,       SNS_REG_GYRO_TAP_TIME_SECS_12_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 80, 2,1,       SNS_REG_GYRO_TAP_TIME_SECS_13_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 84, 2,1,       SNS_REG_GYRO_TAP_TIME_SECS_14_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 88, 2,1,       SNS_REG_GYRO_TAP_RATIO_10_V02      },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 92, 2,1,       SNS_REG_GYRO_TAP_RATIO_11_V02      },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 96, 2,1,       SNS_REG_GYRO_TAP_THRESH_10_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 100, 2,1,      SNS_REG_GYRO_TAP_THRESH_11_V02    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 104, 2,1,      SNS_REG_GYRO_TAP_THRESH_12_V02    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 108, 2,1,      SNS_REG_GYRO_TAP_THRESH_13_V02    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 112, 2,1,      SNS_REG_GYRO_TAP_THRESH_14_V02    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 116, 2,1,      SNS_REG_GYRO_TAP_SCENARIO_2_V02   },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 120, 2,1,      SNS_REG_GYRO_TAP_FLAGS_2_V02      },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 124, 2,1,      SNS_REG_GYRO_TAP_TIME_SECS_20_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 128, 2,1,      SNS_REG_GYRO_TAP_TIME_SECS_21_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 132, 2,1,      SNS_REG_GYRO_TAP_TIME_SECS_22_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 136, 2,1,      SNS_REG_GYRO_TAP_TIME_SECS_23_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 140, 2,1,      SNS_REG_GYRO_TAP_TIME_SECS_24_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 144, 2,1,      SNS_REG_GYRO_TAP_RATIO_20_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 148, 2,1,      SNS_REG_GYRO_TAP_RATIO_21_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 152, 2,1,      SNS_REG_GYRO_TAP_THRESH_20_V02    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 156, 2,1,      SNS_REG_GYRO_TAP_THRESH_21_V02    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 160, 2,1,      SNS_REG_GYRO_TAP_THRESH_22_V02    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 164, 2,1,      SNS_REG_GYRO_TAP_THRESH_23_V02    },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 18) + 168, 2,1,      SNS_REG_GYRO_TAP_THRESH_24_V02    },

  /* Group : SNS_REG_SETTINGS_V02                                                         */
  /* Item Type              Offset from start of file   Version      Item ID                         */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 19),      2,2,         SNS_REG_SETTINGS_MG_WORD_V02             },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 19) + 8,  2,2,         SNS_REG_SETTINGS_VERSION_MAJOR_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 19) + 12, 2,2,         SNS_REG_SETTINGS_VERSION_MINOR_V02       },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 19) + 16, 2,16,        SNS_REG_SETTINGS_FILE1_NAME_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 19) + 24, 2,16,        SNS_REG_SETTINGS_FILE1_VERSION_V02        },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 19) + 28, 2,16,        SNS_REG_SETTINGS_FILE2_NAME_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 19) + 36, 2,16,        SNS_REG_SETTINGS_FILE2_VERSION_V02        },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 19) + 40, 2,16,        SNS_REG_SETTINGS_FILE3_NAME_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 19) + 48, 2,16,        SNS_REG_SETTINGS_FILE3_VERSION_V02        },

  /* Group: SNS_REG_SCM_GROUP_ACCEL_CAL_ALGO_V02                                                  */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 20),      2,3,         SNS_REG_ITEM_ACCEL_CAL_DEF_SAMP_RATE_V02     },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 20) + 4,  2,3,         SNS_REG_ITEM_ACCEL_CAL_DEF_ENABLE_ALGO_V02   },

  /* Group: SNS_REG_SAM_GROUP_PED_V02                                                  */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 21),      2,4,         SNS_REG_ITEM_PED_DEF_STEP_THRESHOLD_V02      },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 21) + 4,  2,4,         SNS_REG_ITEM_PED_DEF_SWING_THRESHOLD_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 21) + 8,  2,4,         SNS_REG_ITEM_PED_DEF_STEP_PROB_THRESHOLD_V02 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 21) + 12, 2,11,        SNS_REG_ITEM_PED_DEF_SENSOR_REPORT_RATE_V02  },
  /* Max offset for this group should be (MAX_GROUP_SIZE * 22) - 1 */

  /* Group: SNS_REG_SAM_GROUP_PAM_V02                                                            */
  /* Item Type              Offset from start of file     Version    Item ID                          */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 22),        2,5,       SNS_REG_ITEM_PAM_INT_CFG_PARAM1_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 22) + 4,    2,5,       SNS_REG_ITEM_PAM_INT_CFG_PARAM2_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 22) + 8,    2,5,       SNS_REG_ITEM_PAM_INT_CFG_PARAM3_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 22) + 12,   2,5,       SNS_REG_ITEM_PAM_INT_CFG_PARAM4_V02 },
  /* Max offset for this group should be (MAX_GROUP_SIZE * 23) - 1 */

  /* Group: SNS_REG_GROUP_SSI_SMGR_CFG_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23)      ,      2,6, SNS_REG_ITEM_SSI_SMGR_MAJ_VER_NO_V02      },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) +   1,      2,6, SNS_REG_ITEM_SSI_SMGR_MIN_VER_NO_V02      },
  /* 14 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 23) +  16,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 23) +  24,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 23) +  32,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 23) +  36,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 23) +  40,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) +  42,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) +  44,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) +  46,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) +  48,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) +  50,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) +  51,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) +  52,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) +  53,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) +  54,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) +  55,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) +  56,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG0_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) +  57,      2,9, SNS_REG_ITEM_SSI_SMGR_CFG0_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 23) +  64,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 23) +  72,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 23) +  80,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 23) +  84,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 23) +  88,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) +  90,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) +  92,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) +  94,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) +  96,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) +  98,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) +  99,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 100,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 101,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 102,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 103,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 104,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG1_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 105,      2,9, SNS_REG_ITEM_SSI_SMGR_CFG1_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 23) + 112,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 23) + 120,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 23) + 128,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 23) + 132,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 23) + 136,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 138,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 140,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 142,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 144,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 146,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 147,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 148,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 149,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 150,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 151,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 152,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG2_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 153,      2,9, SNS_REG_ITEM_SSI_SMGR_CFG2_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 23) + 160,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 23) + 168,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 23) + 176,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 23) + 180,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 23) + 184,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 186,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 188,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 190,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 192,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 194,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 195,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 196,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 197,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 198,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 199,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 200,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG3_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 201,      2,9, SNS_REG_ITEM_SSI_SMGR_CFG3_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 23) + 208,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 23) + 216,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 23) + 224,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 23) + 228,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 23) + 232,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 234,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 236,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 238,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 23) + 240,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 242,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 243,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 244,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 245,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 246,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 23) + 247,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 248,      2,6, SNS_REG_ITEM_SSI_SMGR_CFG4_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 23) + 249,      2,9, SNS_REG_ITEM_SSI_SMGR_CFG4_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */

  /* Group: SNS_REG_GROUP_SSI_DEVINFO_ACCEL_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 24) +   0,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 24) +   1,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) +   4,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) +  12,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) +  20,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) +  24,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) +  28,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) +  30,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) +  32,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) +  34,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) +  36,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) +  37,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) +  38,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) +  46,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) +  54,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) +  62,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) +  66,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) +  70,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) +  72,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) +  74,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) +  76,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) +  78,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) +  79,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) +  80,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) +  88,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) +  96,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) + 104,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) + 108,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 112,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 114,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 116,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 118,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 120,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 121,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 122,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) + 130,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) + 138,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) + 146,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) + 150,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 154,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 156,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 158,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 160,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 162,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 163,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 164,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) + 172,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) + 180,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) + 188,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) + 192,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 196,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 198,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 200,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 202,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 204,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 205,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 206,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) + 214,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 24) + 222,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) + 230,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 24) + 234,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 238,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 240,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 242,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 24) + 244,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 246,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 247,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 24) + 248,      2,7, SNS_REG_ITEM_SSI_DEVINFO_ACCEL_CFG5_FLAGS_V02               },
  /* 7 reserved bytes */

  /* Group: SNS_REG_GROUP_SSI_DEVINFO_GYRO_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 25) +   0,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 25) +   1,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) +   4,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) +  12,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) +  20,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) +  24,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) +  28,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) +  30,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) +  32,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) +  34,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) +  36,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) +  37,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) +  38,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) +  46,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) +  54,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) +  62,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) +  66,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) +  70,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) +  72,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) +  74,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) +  76,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) +  78,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) +  79,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) +  80,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) +  88,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) +  96,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) + 104,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) + 108,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 112,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 114,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 116,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 118,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 120,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 121,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 122,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) + 130,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) + 138,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) + 146,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) + 150,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 154,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 156,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 158,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 160,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 162,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 163,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 164,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) + 172,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) + 180,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) + 188,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) + 192,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 196,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 198,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 200,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 202,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 204,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 205,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 206,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) + 214,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 25) + 222,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) + 230,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 25) + 234,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 238,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 240,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 242,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 25) + 244,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 246,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 247,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 25) + 248,      2,7, SNS_REG_ITEM_SSI_DEVINFO_GYRO_CFG5_FLAGS_V02               },
  /* 7 reserved bytes */

  /* Group: SNS_REG_GROUP_SSI_DEVINFO_MAG_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 26) +   0,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 26) +   1,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) +   4,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) +  12,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) +  20,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) +  24,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) +  28,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) +  30,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) +  32,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) +  34,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) +  36,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) +  37,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) +  38,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) +  46,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) +  54,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) +  62,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) +  66,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) +  70,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) +  72,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) +  74,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) +  76,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) +  78,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) +  79,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) +  80,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) +  88,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) +  96,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) + 104,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) + 108,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 112,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 114,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 116,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 118,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 120,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 121,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 122,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) + 130,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) + 138,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) + 146,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) + 150,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 154,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 156,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 158,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 160,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 162,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 163,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 164,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) + 172,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) + 180,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) + 188,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) + 192,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 196,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 198,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 200,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 202,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 204,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 205,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 206,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) + 214,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 26) + 222,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) + 230,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 26) + 234,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 238,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 240,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 242,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 26) + 244,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 246,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 247,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 26) + 248,      2,7, SNS_REG_ITEM_SSI_DEVINFO_MAG_CFG5_FLAGS_V02               },
  /* 7 reserved bytes */

  /* Group: SNS_REG_GROUP_SSI_DEVINFO_PROX_LIGHT_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 27) +   0,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 27) +   1,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) +   4,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) +  12,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) +  20,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) +  24,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) +  28,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) +  30,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) +  32,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) +  34,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) +  36,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) +  37,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) +  38,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) +  46,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) +  54,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) +  62,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) +  66,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) +  70,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) +  72,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) +  74,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) +  76,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) +  78,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) +  79,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) +  80,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) +  88,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) +  96,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) + 104,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) + 108,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 112,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 114,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 116,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 118,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 120,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 121,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 122,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) + 130,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) + 138,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) + 146,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) + 150,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 154,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 156,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 158,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 160,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 162,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 163,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 164,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) + 172,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) + 180,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) + 188,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) + 192,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 196,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 198,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 200,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 202,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 204,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 205,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 206,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) + 214,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 27) + 222,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) + 230,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 27) + 234,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 238,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 240,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 242,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 27) + 244,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 246,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 247,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 27) + 248,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PROX_LIGHT_CFG5_FLAGS_V02               },
  /* 7 reserved bytes */

  /* Group: SNS_REG_GROUP_SSI_DEVINFO_PRESSURE_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 28) +   0,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 28) +   1,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) +   4,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) +  12,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) +  20,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) +  24,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) +  28,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) +  30,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) +  32,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) +  34,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) +  36,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) +  37,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) +  38,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) +  46,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) +  54,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) +  62,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) +  66,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) +  70,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) +  72,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) +  74,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) +  76,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) +  78,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) +  79,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) +  80,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) +  88,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) +  96,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) + 104,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) + 108,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 112,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 114,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 116,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 118,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 120,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 121,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 122,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) + 130,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) + 138,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) + 146,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) + 150,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 154,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 156,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 158,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 160,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 162,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 163,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 164,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) + 172,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) + 180,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) + 188,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) + 192,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 196,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 198,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 200,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 202,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 204,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 205,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 206,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) + 214,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 28) + 222,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) + 230,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 28) + 234,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 238,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 240,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 242,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 28) + 244,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 246,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 247,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 28) + 248,      2,7, SNS_REG_ITEM_SSI_DEVINFO_PRESSURE_CFG5_FLAGS_V02               },
  /* 7 reserved bytes */

  /* Group: SNS_REG_GROUP_SSI_SENSOR_DEP_CFG0_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) +   0,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VER_NO_V02        },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) +   1,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SENSOR_TYPE_V02   },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) +   2,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_NEXT_GROUP_ID_V02 },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 29) +   4,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_UUID_HIGH0_V02    },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 29) +  12,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_UUID_LOW0_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) +  20,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID00_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) +  22,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE00_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) +  23,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE00_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) +  27,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID01_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) +  29,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE01_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) +  30,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE01_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) +  34,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID02_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) +  36,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE02_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) +  37,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE02_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) +  41,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID03_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) +  43,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE03_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) +  44,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE03_V02       },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 29) +  48,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_UUID_HIGH1_V02    },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 29) +  56,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_UUID_LOW1_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) +  64,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID10_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) +  66,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE10_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) +  67,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE10_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) +  71,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID11_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) +  73,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE11_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) +  74,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE11_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) +  78,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID12_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) +  80,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE12_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) +  81,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE12_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) +  85,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID13_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) +  87,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE13_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) +  88,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE13_V02       },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 29) +  92,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_UUID_HIGH2_V02    },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 29) + 100,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_UUID_LOW2_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 108,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID20_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 110,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE20_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 111,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE20_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 115,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID21_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 117,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE21_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 118,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE21_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 122,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID22_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 124,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE22_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 125,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE22_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 129,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID23_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 131,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE23_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 132,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE23_V02       },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 29) + 136,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_UUID_HIGH3_V02    },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 29) + 144,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_UUID_LOW3_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 152,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID30_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 154,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE30_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 155,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE30_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 159,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID31_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 161,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE31_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 162,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE31_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 166,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID32_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 168,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE32_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 169,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE32_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 173,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID33_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 175,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE33_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 176,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE33_V02       },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 29) + 180,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_UUID_HIGH4_V02    },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 29) + 188,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_UUID_LOW4_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 196,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID40_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 198,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE40_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 199,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE40_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 203,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID41_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 205,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE41_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 206,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE41_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 210,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID42_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 212,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE42_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 213,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE42_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 29) + 217,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_REG_ITEM_ID43_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 29) + 219,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_SIZE43_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 29) + 220,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG0_VALUE43_V02       },

  /* Group: SNS_REG_GROUP_SSI_SENSOR_DEP_CFG1_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) +   0,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VER_NO_V02        },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) +   1,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SENSOR_TYPE_V02   },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) +   2,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_NEXT_GROUP_ID_V02 },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 30) +   4,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_UUID_HIGH0_V02    },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 30) +  12,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_UUID_LOW0_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) +  20,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID00_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) +  22,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE00_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) +  23,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE00_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) +  27,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID01_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) +  29,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE01_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) +  30,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE01_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) +  34,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID02_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) +  36,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE02_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) +  37,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE02_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) +  41,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID03_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) +  43,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE03_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) +  44,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE03_V02       },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 30) +  48,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_UUID_HIGH1_V02    },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 30) +  56,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_UUID_LOW1_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) +  64,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID10_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) +  66,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE10_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) +  67,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE10_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) +  71,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID11_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) +  73,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE11_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) +  74,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE11_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) +  78,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID12_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) +  80,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE12_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) +  81,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE12_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) +  85,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID13_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) +  87,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE13_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) +  88,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE13_V02       },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 30) +  92,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_UUID_HIGH2_V02    },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 30) + 100,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_UUID_LOW2_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 108,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID20_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 110,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE20_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 111,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE20_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 115,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID21_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 117,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE21_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 118,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE21_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 122,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID22_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 124,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE22_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 125,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE22_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 129,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID23_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 131,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE23_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 132,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE23_V02       },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 30) + 136,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_UUID_HIGH3_V02    },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 30) + 144,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_UUID_LOW3_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 152,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID30_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 154,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE30_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 155,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE30_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 159,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID31_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 161,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE31_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 162,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE31_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 166,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID32_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 168,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE32_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 169,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE32_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 173,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID33_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 175,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE33_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 176,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE33_V02       },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 30) + 180,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_UUID_HIGH4_V02    },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 30) + 188,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_UUID_LOW4_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 196,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID40_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 198,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE40_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 199,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE40_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 203,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID41_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 205,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE41_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 206,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE41_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 210,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID42_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 212,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE42_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 213,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE42_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 30) + 217,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_REG_ITEM_ID43_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 30) + 219,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_SIZE43_V02        },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 30) + 220,      2,8, SNS_REG_ITEM_SSI_SENSOR_DEP_CFG1_VALUE43_V02       },

  /* Group: SNS_REG_GROUP_SSI_GPIO_CFG_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 31) +   0,      2,10, SNS_REG_ITEM_SSI_GPIO_CFG_VERSION_MAJOR_V02   },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 31) +   1,      2,10, SNS_REG_ITEM_SSI_GPIO_CFG_VERSION_MINOR_V02   },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   2,      2,10, SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SDA_1_V02   },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   4,      2,10, SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SCL_1_V02   },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   6,      2,10, SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SDA_2_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   8,      2,10, SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SCL_2_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   10,     2,10, SNS_REG_ITEM_SSI_GPIO_CFG_RESET_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   12,     2,31, SNS_REG_ITEM_SSI_GPIO_CFG_TEST_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   14,     2,31, SNS_REG_ITEM_SSI_GPIO_CFG_ACCEL_MD_INT_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   16,     2,31, SNS_REG_ITEM_SSI_GPIO_CFG_ACCEL_DRDY_INT_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   18,     2,31, SNS_REG_ITEM_SSI_GPIO_CFG_GYRO_DRDY_INT_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   20,     2,31, SNS_REG_ITEM_SSI_GPIO_CFG_MAG_DRDY_INT_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   22,     2,31, SNS_REG_ITEM_SSI_GPIO_CFG_ALSP_INT_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   24,     2,31, SNS_REG_ITEM_SSI_GPIO_CFG_GEST_INT_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   26,     2,31, SNS_REG_ITEM_SSI_GPIO_CFG_PRESS_INT_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   28,     2,31, SNS_REG_ITEM_SSI_GPIO_CFG_SAR_INT_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   30,     2,31, SNS_REG_ITEM_SSI_GPIO_CFG_FP_INT_V02 },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 31) +   32,     2,31, SNS_REG_ITEM_SSI_GPIO_CFG_HALL_INT_V02 },

  /* Group: SNS_REG_SAM_GROUP_BASIC_GESTURES_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 32) +   0,      2,11, SNS_REG_ITEM_BASIC_GESTURES_DEF_SENSOR_REPORT_RATE_V02 },

  /* Group: SNS_REG_SAM_GROUP_FACING_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 33) +   0,      2,11, SNS_REG_ITEM_FACING_DEF_SENSOR_REPORT_RATE_V02 },

  /* Group: SNS_REG_SAM_GROUP_QUATERNION_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 34) +   0,      2,11, SNS_REG_ITEM_QUATERNION_DEF_SENSOR_REPORT_RATE_V02 },

  /* Group: SNS_REG_SAM_GROUP_ROTATION_VECTOR_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 35) +   0,      2,11, SNS_REG_ITEM_ROTATION_VECTOR_DEF_SENSOR_REPORT_RATE_V02 },

  /* Group: SNS_REG_SAM_GROUP_DISTANCE_BOUND_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 36) +   0,      2,13, SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_UNKNOWN_V02 },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 36) +   4,      2,13, SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_STATIONARY_V02 },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 36) +   8,      2,13, SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_INMOTION_V02 },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 36) +  12,      2,13, SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_FIDDLE_V02 },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 36) +  16,      2,13, SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_PEDESTRIAN_V02 },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 36) +  20,      2,13, SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_VEHICLE_V02 },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 36) +  24,      2,13, SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_WALK_V02 },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 36) +  28,      2,13, SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_RUN_V02 },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 36) +  32,      2,32, SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_BIKE_V02 },

  /* Group: SNS_REG_SAM_GROUP_CMC_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 37) +   0,      2,13, SNS_REG_ITEM_CMC_DEF_SENSOR_REPORT_RATE_V02 },

  /* Group: SNS_REG_GROUP_SSI_SMGR_CFG_2_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38)      ,      2,14, SNS_REG_ITEM_SSI_SMGR_2_MAJ_VER_NO_V02      },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) +   1,      2,14, SNS_REG_ITEM_SSI_SMGR_2_MIN_VER_NO_V02      },
  /* 14 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 38) +  16,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 38) +  24,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 38) +  32,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 38) +  36,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 38) +  40,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) +  42,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) +  44,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) +  46,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) +  48,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) +  50,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) +  51,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) +  52,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) +  53,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) +  54,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) +  55,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) +  56,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) +  57,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG0_2_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 38) +  64,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 38) +  72,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 38) +  80,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 38) +  84,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 38) +  88,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) +  90,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) +  92,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) +  94,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) +  96,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) +  98,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) +  99,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 100,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 101,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 102,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 103,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 104,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 105,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG1_2_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 38) + 112,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 38) + 120,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 38) + 128,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 38) + 132,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 38) + 136,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 138,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 140,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 142,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 144,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 146,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 147,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 148,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 149,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 150,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 151,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 152,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 153,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG2_2_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 38) + 160,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 38) + 168,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 38) + 176,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 38) + 180,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 38) + 184,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 186,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 188,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 190,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 192,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 194,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 195,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 196,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 197,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 198,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 199,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 200,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 201,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG3_2_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 38) + 208,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 38) + 216,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 38) + 224,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 38) + 228,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 38) + 232,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 234,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 236,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 238,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 38) + 240,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 242,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 243,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 244,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 245,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 246,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 38) + 247,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 248,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 38) + 249,      2,14, SNS_REG_ITEM_SSI_SMGR_CFG4_2_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */

  /* Group: SNS_REG_GROUP_SSI_DEVINFO_TAP_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 39) +   0,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 39) +   1,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) +   4,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) +  12,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) +  20,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) +  24,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) +  28,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) +  30,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) +  32,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) +  34,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) +  36,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) +  37,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) +  38,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) +  46,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) +  54,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) +  62,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) +  66,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) +  70,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) +  72,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) +  74,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) +  76,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) +  78,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) +  79,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) +  80,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) +  88,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) +  96,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) + 104,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) + 108,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 112,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 114,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 116,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 118,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 120,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 121,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 122,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) + 130,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) + 138,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) + 146,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) + 150,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 154,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 156,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 158,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 160,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 162,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 163,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 164,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) + 172,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) + 180,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) + 188,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) + 192,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 196,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 198,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 200,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 202,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 204,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 205,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 206,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) + 214,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 39) + 222,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) + 230,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 39) + 234,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 238,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 240,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 242,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 39) + 244,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 246,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 247,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 39) + 248,      2,14, SNS_REG_ITEM_SSI_DEVINFO_TAP_CFG5_FLAGS_V02               },
  /* 7 reserved bytes */

  /* Group: SNS_REG_SAM_GROUP_GAME_ROTATION_VECTOR_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 40) +   0,      2,14, SNS_REG_ITEM_GAME_ROT_VEC_DEF_SENSOR_REPORT_RATE_V02 },

  /* Group: SNS_REG_SCM_GROUP_QMAG_CAL_ALGO_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 41) +   0,      2,17,  SNS_REG_ITEM_QMAG_CAL_VERSION_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 41) +   4,      2,17,  SNS_REG_ITEM_QMAG_CAL_ENABLE_ALGO_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 41) +   5,      2,17,  SNS_REG_ITEM_QMAG_CAL_ENABLE_SI_CAL_V02          },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 41) +   6,      2,17,  SNS_REG_ITEM_QMAG_CAL_ENABLE_HI_CAL_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 41) +   7,      2,17,  SNS_REG_ITEM_SAMPLE_RATE_V02                     },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 41) +  11,      2,17,  SNS_REG_ITEM_QMAG_CAL_REPORT_RATE_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 41) +  15,      2,17,  SNS_REG_ITEM_QMAG_CAL_ENABLE_GYRO_ASSIST_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  16,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_X_HI_PUBLISH_V02    },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  20,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Y_HI_PUBLISH_V02    },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  24,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Z_HI_PUBLISH_V02    },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 41) +  28,      2,17,  SNS_REG_ITEM_QMAG_CAL_ACCURACY_HI_PUBLISH_V02    },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  32,      2,17,  SNS_REG_ITEM_QMAG_CAL_RADIUS_HI_BUFF_0_V02       },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  36,      2,17,  SNS_REG_ITEM_QMAG_CAL_RADIUS_HI_BUFF_1_V02       },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  40,      2,17,  SNS_REG_ITEM_QMAG_CAL_RADIUS_HI_BUFF_2_V02       },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  44,      2,17,  SNS_REG_ITEM_QMAG_CAL_RADIUS_HI_BUFF_3_V02       },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  48,      2,17,  SNS_REG_ITEM_QMAG_CAL_RADIUS_HI_BUFF_4_V02       },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 41) +  52,      2,17,  SNS_REG_ITEM_QMAG_CAL_RADIUS_HI_BUFF_COUNT_V02   },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  53,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_X_HI_BUFF_0_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  57,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_X_HI_BUFF_1_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  61,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_X_HI_BUFF_2_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  65,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_X_HI_BUFF_3_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  69,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_X_HI_BUFF_4_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  73,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Y_HI_BUFF_0_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  77,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Y_HI_BUFF_1_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  81,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Y_HI_BUFF_2_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  85,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Y_HI_BUFF_3_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  89,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Y_HI_BUFF_4_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  93,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Z_HI_BUFF_0_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) +  97,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Z_HI_BUFF_1_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) + 101,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Z_HI_BUFF_2_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) + 105,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Z_HI_BUFF_3_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) + 109,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_Z_HI_BUFF_4_V02     },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 41) + 113,      2,17,  SNS_REG_ITEM_QMAG_CAL_OFFSET_HI_BUFF_COUNT_V02   },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) + 114,      2,17,  SNS_REG_ITEM_QMAG_CAL_ACCURACY_HI_BUFF_0_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) + 118,      2,17,  SNS_REG_ITEM_QMAG_CAL_ACCURACY_HI_BUFF_1_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) + 122,      2,17,  SNS_REG_ITEM_QMAG_CAL_ACCURACY_HI_BUFF_2_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) + 126,      2,17,  SNS_REG_ITEM_QMAG_CAL_ACCURACY_HI_BUFF_3_V02     },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 41) + 130,      2,17,  SNS_REG_ITEM_QMAG_CAL_ACCURACY_HI_BUFF_4_V02     },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 41) + 134,      2,17,  SNS_REG_ITEM_QMAG_CAL_ACCURACY_HI_BUFF_COUNT_V02 },

  /* Group: SNS_REG_SCM_GROUP_MAG_DYN_CAL_PARAMS_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                               */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 42) +   0,      2,17,  SNS_REG_ITEM_MAG_DYN_CAL_VERSION_V02                  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 42) +   4,      2,17,  SNS_REG_ITEM_MAG_DYN_CAL_BIAS_VALID_V02               },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 42) +   5,      2,17,  SNS_REG_ITEM_MAG_DYN_CAL_CALIBRATION_MATRIX_VALID_V02 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +   6,      2,17,  SNS_REG_ITEM_MAG_X_DYN_BIAS_V02                       },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  10,      2,17,  SNS_REG_ITEM_MAG_Y_DYN_BIAS_V02                       },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  14,      2,17,  SNS_REG_ITEM_MAG_Z_DYN_BIAS_V02                       },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  18,      2,17,  SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_0_0_V02      },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  22,      2,17,  SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_0_1_V02      },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  26,      2,17,  SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_0_2_V02      },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  30,      2,17,  SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_1_0_V02      },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  34,      2,17,  SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_1_1_V02      },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  38,      2,17,  SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_1_2_V02      },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  42,      2,17,  SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_2_0_V02      },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  46,      2,17,  SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_2_1_V02      },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  50,      2,17,  SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_2_2_V02      },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 42) +  54,      2,17,  SNS_REG_ITEM_MAG_DYN_ACCURACY_V02                     },

  /* Group: SNS_REG_SCM_GROUP_QMAG_CAL_STATE_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                               */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 43) +   0,      2,18,  SNS_REG_ITEM_QMAG_CAL_STATE_VERSION_V02               },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 43) +   4,      2,18,  SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_X_HI_V02   },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 43) +   8,      2,18,  SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_Y_HI_V02   },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 43) +  12,      2,18,  SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_Z_HI_V02   },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 43) +  16,      2,18,  SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_ACCURACY_HI_V02   },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 43) +  17,      2,18,  SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_TIME_V02   },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 43) +  21,      2,18,  SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_VALID_V02  },

  /* Group: SNS_REG_SAM_GROUP_SMD_V02*/
  /* Item Type              Offset from start of file   Version      Item ID                                               */
  { SNS_REG_TYPE_UINT8,    (MAX_GROUP_SIZE * 44) +  0,     2,19,    SNS_REG_ITEM_SMD_VERSION_V02                        },
  { SNS_REG_TYPE_UINT8,    (MAX_GROUP_SIZE * 44) +  1,     2,19,    SNS_REG_ITEM_SMD_VAR_DEC_LAT_V02                    },
  { SNS_REG_TYPE_UINT8,    (MAX_GROUP_SIZE * 44) +  2,     2,19,    SNS_REG_ITEM_SMD_MAX_LAT_V02                        },
  { SNS_REG_TYPE_UINT8,    (MAX_GROUP_SIZE * 44) +  3,     2,21,    SNS_REG_ITEM_SMD_ACC_WIN_TIME_V02                   },
  { SNS_REG_TYPE_UINT8,    (MAX_GROUP_SIZE * 44) +  4,     2,21,    SNS_REG_ITEM_SMD_PCOUNT_THRESH_V02                  },
  { SNS_REG_TYPE_UINT8,    (MAX_GROUP_SIZE * 44) +  5,     2,21,    SNS_REG_ITEM_SMD_PTIME_V02                          },
  { SNS_REG_TYPE_UINT8,    (MAX_GROUP_SIZE * 44) +  6,     2,19,    SNS_REG_ITEM_SMD_DELTA_RESET_TIME_V02               },
  { SNS_REG_TYPE_UINT8,    (MAX_GROUP_SIZE * 44) +  7,     2,19,    SNS_REG_ITEM_SMD_LACC_WIN_TIME_V02                  },
  { SNS_REG_TYPE_INT32,    (MAX_GROUP_SIZE * 44) +  8,     2,19,    SNS_REG_ITEM_SMD_STRANS_PROB_SM_V02                 },
  { SNS_REG_TYPE_INT32,    (MAX_GROUP_SIZE * 44) + 12,     2,19,    SNS_REG_ITEM_SMD_DETECT_THRESH_V02                  },
  { SNS_REG_TYPE_INT32,    (MAX_GROUP_SIZE * 44) + 16,     2,19,    SNS_REG_ITEM_SMD_EIGEN_THRESH_V02                   },
  { SNS_REG_TYPE_INT32,    (MAX_GROUP_SIZE * 44) + 20,     2,19,    SNS_REG_ITEM_SMD_ACC_NORM_SDEV_THRESH_V02           },
  { SNS_REG_TYPE_INT32,    (MAX_GROUP_SIZE * 44) + 24,     2,19,    SNS_REG_ITEM_SMD_SENSOR_REPORT_RATE_V02             },
  { SNS_REG_TYPE_INT32,    (MAX_GROUP_SIZE * 44) + 28,     2,19,    SNS_REG_ITEM_SMD_SAMP_RATE_V02                      },


  /* Group: SNS_REG_GROUP_SSI_DEVINFO_HUMIDITY_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 45) +   0,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 45) +   1,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) +   4,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) +  12,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) +  20,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) +  24,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) +  28,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) +  30,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) +  32,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) +  34,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) +  36,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) +  37,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) +  38,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) +  46,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) +  54,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) +  62,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) +  66,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) +  70,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) +  72,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) +  74,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) +  76,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) +  78,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) +  79,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) +  80,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) +  88,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) +  96,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) + 104,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) + 108,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 112,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 114,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 116,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 118,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 120,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 121,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 122,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) + 130,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) + 138,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) + 146,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) + 150,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 154,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 156,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 158,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 160,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 162,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 163,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 164,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) + 172,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) + 180,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) + 188,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) + 192,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 196,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 198,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 200,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 202,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 204,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 205,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 206,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) + 214,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 45) + 222,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) + 230,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 45) + 234,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 238,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 240,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 242,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 45) + 244,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 246,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 247,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 45) + 248,      2,20, SNS_REG_ITEM_SSI_DEVINFO_HUMIDITY_CFG5_FLAGS_V02               },
  /* 7 reserved bytes */

    /* Group: SNS_REG_GROUP_SSI_DEVINFO_RGB_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 46) +   0,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 46) +   1,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) +   4,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) +  12,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) +  20,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) +  24,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) +  28,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) +  30,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) +  32,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) +  34,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) +  36,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) +  37,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) +  38,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) +  46,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) +  54,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) +  62,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) +  66,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) +  70,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) +  72,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) +  74,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) +  76,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) +  78,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) +  79,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) +  80,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) +  88,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) +  96,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) + 104,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) + 108,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 112,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 114,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 116,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 118,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 120,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 121,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 122,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) + 130,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) + 138,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) + 146,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) + 150,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 154,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 156,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 158,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 160,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 162,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 163,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 164,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) + 172,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) + 180,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) + 188,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) + 192,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 196,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 198,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 200,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 202,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 204,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 205,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 206,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) + 214,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 46) + 222,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) + 230,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 46) + 234,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 238,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 240,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 242,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 46) + 244,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 246,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 247,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 46) + 248,      2,20, SNS_REG_ITEM_SSI_DEVINFO_RGB_CFG5_FLAGS_V02               },

    /* Group: SNS_REG_GROUP_SSI_DEVINFO_IR_GESTURE_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 47) +   0,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 47) +   1,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) +   4,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) +  12,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) +  20,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) +  24,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) +  28,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) +  30,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) +  32,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) +  34,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) +  36,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) +  37,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) +  38,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) +  46,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) +  54,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) +  62,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) +  66,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) +  70,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) +  72,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) +  74,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) +  76,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) +  78,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) +  79,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) +  80,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) +  88,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) +  96,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) + 104,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) + 108,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 112,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 114,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 116,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 118,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 120,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 121,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 122,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) + 130,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) + 138,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) + 146,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) + 150,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 154,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 156,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 158,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 160,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 162,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 163,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 164,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) + 172,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) + 180,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) + 188,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) + 192,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 196,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 198,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 200,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 202,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 204,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 205,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 206,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) + 214,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 47) + 222,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) + 230,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 47) + 234,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 238,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 240,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 242,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 47) + 244,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 246,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 247,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 47) + 248,      2,20, SNS_REG_ITEM_SSI_DEVINFO_IR_GESTURE_CFG5_FLAGS_V02               },
  /* 7 reserved bytes */

  /* Group: SNS_REG_SAM_GROUP_GAME_ROTATION_VECTOR_V02 */
  /* Item Type              Offset from start of file      Version  Item ID                                */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 48) +   0,      2,22, SNS_REG_ITEM_MAG_FAC_CAL_VERSION_V02                             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 48) +   4,      2,22, SNS_REG_ITEM_MAG_FAC_CAL_BIAS_VALID_V02                          },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 48) +   5,      2,22, SNS_REG_ITEM_MAG_FAC_CAL_CALIBRATION_MATRIX_VALID_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +   6,      2,22, SNS_REG_ITEM_MAG_X_FAC_BIAS_V02                                  },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  10,      2,22, SNS_REG_ITEM_MAG_Y_FAC_BIAS_V02                                  },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  14,      2,22, SNS_REG_ITEM_MAG_Z_FAC_BIAS_V02                                  },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  18,      2,22, SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_0_0_V02                 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  22,      2,22, SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_0_1_V02                 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  26,      2,22, SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_0_2_V02                 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  30,      2,22, SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_1_0_V02                 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  34,      2,22, SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_1_1_V02                 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  38,      2,22, SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_1_2_V02                 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  42,      2,22, SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_2_0_V02                 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  46,      2,22, SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_2_1_V02                 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  50,      2,22, SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_2_2_V02                 },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 48) +  54,      2,22, SNS_REG_ITEM_MAG_FAC_ACCURACY_V02                                },

    /* Group: SNS_REG_GROUP_SSI_DEVINFO_SAR_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 49) +   0,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 49) +   1,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) +   4,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) +  12,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) +  20,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) +  24,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) +  28,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) +  30,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) +  32,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) +  34,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) +  36,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) +  37,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) +  38,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) +  46,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) +  54,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) +  62,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) +  66,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) +  70,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) +  72,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) +  74,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) +  76,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) +  78,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) +  79,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) +  80,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) +  88,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) +  96,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) + 104,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) + 108,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 112,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 114,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 116,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 118,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 120,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 121,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 122,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) + 130,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) + 138,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) + 146,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) + 150,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 154,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 156,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 158,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 160,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 162,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 163,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 164,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) + 172,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) + 180,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) + 188,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) + 192,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 196,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 198,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 200,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 202,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 204,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 205,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 206,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) + 214,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 49) + 222,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) + 230,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 49) + 234,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 238,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 240,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 242,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 49) + 244,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 246,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 247,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 49) + 248,      2,23, SNS_REG_ITEM_SSI_DEVINFO_SAR_CFG5_FLAGS_V02               },
  /* 7 reserved bytes */

    /* Group: SNS_REG_DRIVER_GROUP_IR_GESTURE_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +   0,      2,24, SNS_REG_ITEM_IR_GESTURE_DS1_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +   4,      2,24, SNS_REG_ITEM_IR_GESTURE_DS2_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +   8,      2,24, SNS_REG_ITEM_IR_GESTURE_DS3_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  12,      2,24, SNS_REG_ITEM_IR_GESTURE_DS4_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  16,      2,24, SNS_REG_ITEM_IR_GESTURE_DS5_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  20,      2,24, SNS_REG_ITEM_IR_GESTURE_DS6_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  24,      2,24, SNS_REG_ITEM_IR_GESTURE_DS7_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  28,      2,24, SNS_REG_ITEM_IR_GESTURE_DS8_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  32,      2,24, SNS_REG_ITEM_IR_GESTURE_DS9_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  36,      2,24, SNS_REG_ITEM_IR_GESTURE_DS10_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  40,      2,24, SNS_REG_ITEM_IR_GESTURE_DS11_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  44,      2,24, SNS_REG_ITEM_IR_GESTURE_DS12_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  48,      2,24, SNS_REG_ITEM_IR_GESTURE_DS13_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  52,      2,24, SNS_REG_ITEM_IR_GESTURE_DS14_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  56,      2,24, SNS_REG_ITEM_IR_GESTURE_DS15_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  60,      2,24, SNS_REG_ITEM_IR_GESTURE_DS16_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  64,      2,24, SNS_REG_ITEM_IR_GESTURE_DS17_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  68,      2,24, SNS_REG_ITEM_IR_GESTURE_DS18_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  72,      2,24, SNS_REG_ITEM_IR_GESTURE_DS19_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  76,      2,24, SNS_REG_ITEM_IR_GESTURE_DS20_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  80,      2,24, SNS_REG_ITEM_IR_GESTURE_DS21_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  84,      2,24, SNS_REG_ITEM_IR_GESTURE_DS22_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  88,      2,24, SNS_REG_ITEM_IR_GESTURE_DS23_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  92,      2,24, SNS_REG_ITEM_IR_GESTURE_DS24_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) +  96,      2,24, SNS_REG_ITEM_IR_GESTURE_DS25_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 100,      2,24, SNS_REG_ITEM_IR_GESTURE_DS26_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 104,      2,24, SNS_REG_ITEM_IR_GESTURE_DS27_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 108,      2,24, SNS_REG_ITEM_IR_GESTURE_DS28_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 112,      2,24, SNS_REG_ITEM_IR_GESTURE_DS29_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 116,      2,24, SNS_REG_ITEM_IR_GESTURE_DS30_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 120,      2,24, SNS_REG_ITEM_IR_GESTURE_DS31_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 124,      2,24, SNS_REG_ITEM_IR_GESTURE_DS32_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 128,      2,24, SNS_REG_ITEM_IR_GESTURE_DS33_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 132,      2,24, SNS_REG_ITEM_IR_GESTURE_DS34_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 136,      2,24, SNS_REG_ITEM_IR_GESTURE_DS35_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 140,      2,24, SNS_REG_ITEM_IR_GESTURE_DS36_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 144,      2,24, SNS_REG_ITEM_IR_GESTURE_DS37_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 148,      2,24, SNS_REG_ITEM_IR_GESTURE_DS38_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 152,      2,24, SNS_REG_ITEM_IR_GESTURE_DS39_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 156,      2,24, SNS_REG_ITEM_IR_GESTURE_DS40_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 160,      2,24, SNS_REG_ITEM_IR_GESTURE_DS41_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 164,      2,24, SNS_REG_ITEM_IR_GESTURE_DS42_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 168,      2,24, SNS_REG_ITEM_IR_GESTURE_DS43_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 172,      2,24, SNS_REG_ITEM_IR_GESTURE_DS44_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 176,      2,24, SNS_REG_ITEM_IR_GESTURE_DS45_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 180,      2,24, SNS_REG_ITEM_IR_GESTURE_DS46_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 184,      2,24, SNS_REG_ITEM_IR_GESTURE_DS47_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 188,      2,24, SNS_REG_ITEM_IR_GESTURE_DS48_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 192,      2,24, SNS_REG_ITEM_IR_GESTURE_DS49_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 196,      2,24, SNS_REG_ITEM_IR_GESTURE_DS50_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 200,      2,24, SNS_REG_ITEM_IR_GESTURE_DS51_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 204,      2,24, SNS_REG_ITEM_IR_GESTURE_DS52_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 208,      2,24, SNS_REG_ITEM_IR_GESTURE_DS53_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 212,      2,24, SNS_REG_ITEM_IR_GESTURE_DS54_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 216,      2,24, SNS_REG_ITEM_IR_GESTURE_DS55_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 220,      2,24, SNS_REG_ITEM_IR_GESTURE_DS56_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 224,      2,24, SNS_REG_ITEM_IR_GESTURE_DS57_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 228,      2,24, SNS_REG_ITEM_IR_GESTURE_DS58_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 232,      2,24, SNS_REG_ITEM_IR_GESTURE_DS59_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 236,      2,24, SNS_REG_ITEM_IR_GESTURE_DS60_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 240,      2,24, SNS_REG_ITEM_IR_GESTURE_DS61_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 244,      2,24, SNS_REG_ITEM_IR_GESTURE_DS62_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 248,      2,24, SNS_REG_ITEM_IR_GESTURE_DS63_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 50) + 252,      2,24, SNS_REG_ITEM_IR_GESTURE_DS64_V02 },

  /* Group: SNS_REG_DRIVER_GROUP_SAR_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +   0,      2,26, SNS_REG_DRIVER_SAR_NUM_ELECTRODES_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +   4,      2,26, SNS_REG_DRIVER_SAR_E0_BIAS_V02                  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +   8,      2,26, SNS_REG_DRIVER_SAR_E0_THRESHOLD_V02             },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  12,      2,26, SNS_REG_DRIVER_SAR_E1_BIAS_V02                  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  16,      2,26, SNS_REG_DRIVER_SAR_E1_THRESHOLD_V02             },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  20,      2,26, SNS_REG_DRIVER_SAR_E2_BIAS_V02                  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  24,      2,26, SNS_REG_DRIVER_SAR_E2_THRESHOLD_V02             },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  28,      2,26, SNS_REG_DRIVER_SAR_E3_BIAS_V02                  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  32,      2,26, SNS_REG_DRIVER_SAR_E3_THRESHOLD_V02             },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  36,      2,26, SNS_REG_DRIVER_SAR_E4_BIAS_V02                  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  40,      2,26, SNS_REG_DRIVER_SAR_E4_THRESHOLD_V02             },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  44,      2,26, SNS_REG_DRIVER_SAR_E5_BIAS_V02                  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  48,      2,26, SNS_REG_DRIVER_SAR_E5_THRESHOLD_V02             },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  52,      2,26, SNS_REG_DRIVER_SAR_E6_BIAS_V02                  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  56,      2,26, SNS_REG_DRIVER_SAR_E6_THRESHOLD_V02             },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  60,      2,26, SNS_REG_DRIVER_SAR_E7_BIAS_V02                  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  64,      2,26, SNS_REG_DRIVER_SAR_E7_THRESHOLD_V02             },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  68,      2,26, SNS_REG_DRIVER_SAR_E8_BIAS_V02                  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  72,      2,26, SNS_REG_DRIVER_SAR_E8_THRESHOLD_V02             },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  76,      2,26, SNS_REG_DRIVER_SAR_E9_BIAS_V02                  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  80,      2,26, SNS_REG_DRIVER_SAR_E9_THRESHOLD_V02             },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  84,      2,26, SNS_REG_DRIVER_SAR_E10_BIAS_V02                 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  88,      2,26, SNS_REG_DRIVER_SAR_E10_THRESHOLD_V02            },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  92,      2,26, SNS_REG_DRIVER_SAR_E11_BIAS_V02                 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) +  96,      2,26, SNS_REG_DRIVER_SAR_E11_THRESHOLD_V02            },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 100,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_01_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 104,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_02_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 108,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_03_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 112,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_04_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 116,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_05_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 120,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_06_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 124,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_07_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 128,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_08_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 132,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_09_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 136,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_10_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 140,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_11_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 144,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_12_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 148,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_13_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 152,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_14_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 156,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_15_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 160,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_16_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 164,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_17_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 168,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_18_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 172,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_19_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 176,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_20_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 180,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_21_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 184,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_22_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 188,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_23_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 192,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_24_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 196,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_25_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 200,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_26_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 204,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_27_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 208,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_28_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 212,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_29_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 216,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_30_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 220,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_31_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 224,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_32_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 228,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_33_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 232,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_34_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 236,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_35_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 240,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_36_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 244,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_37_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 248,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_38_V02       },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 51) + 252,      2,26, SNS_REG_DRIVER_SAR_DRIVER_SPECIFIC_39_V02       },

  /* Group: SNS_REG_DRIVER_GROUP_SAR_2_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +   0,      2,26, SNS_REG_DRIVER_SAR_2_NUM_ELECTRODES_V02         },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +   4,      2,26, SNS_REG_DRIVER_SAR_2_E0_BIAS_V02                },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +   8,      2,26, SNS_REG_DRIVER_SAR_2_E0_THRESHOLD_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  12,      2,26, SNS_REG_DRIVER_SAR_2_E1_BIAS_V02                },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  16,      2,26, SNS_REG_DRIVER_SAR_2_E1_THRESHOLD_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  20,      2,26, SNS_REG_DRIVER_SAR_2_E2_BIAS_V02                },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  24,      2,26, SNS_REG_DRIVER_SAR_2_E2_THRESHOLD_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  28,      2,26, SNS_REG_DRIVER_SAR_2_E3_BIAS_V02                },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  32,      2,26, SNS_REG_DRIVER_SAR_2_E3_THRESHOLD_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  36,      2,26, SNS_REG_DRIVER_SAR_2_E4_BIAS_V02                },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  40,      2,26, SNS_REG_DRIVER_SAR_2_E4_THRESHOLD_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  44,      2,26, SNS_REG_DRIVER_SAR_2_E5_BIAS_V02                },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  48,      2,26, SNS_REG_DRIVER_SAR_2_E5_THRESHOLD_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  52,      2,26, SNS_REG_DRIVER_SAR_2_E6_BIAS_V02                },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  56,      2,26, SNS_REG_DRIVER_SAR_2_E6_THRESHOLD_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  60,      2,26, SNS_REG_DRIVER_SAR_2_E7_BIAS_V02                },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  64,      2,26, SNS_REG_DRIVER_SAR_2_E7_THRESHOLD_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  68,      2,26, SNS_REG_DRIVER_SAR_2_E8_BIAS_V02                },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  72,      2,26, SNS_REG_DRIVER_SAR_2_E8_THRESHOLD_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  76,      2,26, SNS_REG_DRIVER_SAR_2_E9_BIAS_V02                },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  80,      2,26, SNS_REG_DRIVER_SAR_2_E9_THRESHOLD_V02           },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  84,      2,26, SNS_REG_DRIVER_SAR_2_E10_BIAS_V02               },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  88,      2,26, SNS_REG_DRIVER_SAR_2_E10_THRESHOLD_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  92,      2,26, SNS_REG_DRIVER_SAR_2_E11_BIAS_V02               },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) +  96,      2,26, SNS_REG_DRIVER_SAR_2_E11_THRESHOLD_V02          },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 100,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_01_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 104,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_02_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 108,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_03_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 112,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_04_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 116,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_05_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 120,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_06_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 124,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_07_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 128,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_08_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 132,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_09_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 136,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_10_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 140,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_11_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 144,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_12_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 148,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_13_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 152,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_14_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 156,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_15_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 160,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_16_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 164,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_17_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 168,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_18_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 172,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_19_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 176,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_20_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 180,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_21_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 184,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_22_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 188,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_23_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 192,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_24_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 196,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_25_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 200,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_26_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 204,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_27_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 208,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_28_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 212,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_29_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 216,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_30_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 220,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_31_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 224,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_32_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 228,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_33_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 232,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_34_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 236,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_35_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 240,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_36_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 244,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_37_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 248,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_38_V02     },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 52) + 252,      2,26, SNS_REG_DRIVER_SAR_2_DRIVER_SPECIFIC_39_V02     },

    /* Group: SNS_REG_GROUP_SSI_DEVINFO_HALL_EFFECT_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 53) +   0,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 53) +   1,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) +   4,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) +  12,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) +  20,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) +  24,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) +  28,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) +  30,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) +  32,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) +  34,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) +  36,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) +  37,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) +  38,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) +  46,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) +  54,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) +  62,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) +  66,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) +  70,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) +  72,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) +  74,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) +  76,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) +  78,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) +  79,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) +  80,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) +  88,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) +  96,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) + 104,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) + 108,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 112,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 114,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 116,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 118,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 120,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 121,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 122,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) + 130,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) + 138,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) + 146,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) + 150,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 154,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 156,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 158,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 160,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 162,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 163,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 164,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) + 172,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) + 180,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) + 188,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) + 192,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 196,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 198,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 200,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 202,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 204,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 205,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 206,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) + 214,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 53) + 222,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) + 230,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 53) + 234,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 238,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 240,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 242,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 53) + 244,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 246,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 247,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 53) + 248,      2,25, SNS_REG_ITEM_SSI_DEVINFO_HALL_EFFECT_CFG5_FLAGS_V02               },
  /* 7 reserved bytes */


  /* Group: SNS_REG_DRIVER_GROUP_RGB_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +   0,      2,27, SNS_REG_DRIVER_RGB_R_FACTOR_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +   4,      2,27, SNS_REG_DRIVER_RGB_G_FACTOR_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +   8,      2,27, SNS_REG_DRIVER_RGB_B_FACTOR_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  12,      2,27, SNS_REG_DRIVER_RGB_C_FACTOR_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  16,      2,27, SNS_REG_DRIVER_RGB_IR_FACTOR_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  20,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_01_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  24,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_02_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  28,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_03_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  32,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_04_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  36,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_05_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  40,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_06_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  44,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_07_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  48,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_08_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  52,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_09_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  56,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_10_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  60,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_11_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  64,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_12_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  68,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_13_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  72,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_14_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  76,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_15_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  80,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_16_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  84,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_17_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  88,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_18_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  92,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_19_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) +  96,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_20_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) + 100,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_21_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) + 104,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_22_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) + 108,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_23_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) + 112,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_24_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) + 116,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_25_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) + 120,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_26_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 54) + 124,      2,27, SNS_REG_DRIVER_RGB_DRIVER_SPECIFIC_27_V02 },

  /* Group: SNS_REG_DRIVER_GROUP_RGB_2_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +   0,      2,27, SNS_REG_DRIVER_RGB_2_R_FACTOR_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +   4,      2,27, SNS_REG_DRIVER_RGB_2_G_FACTOR_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +   8,      2,27, SNS_REG_DRIVER_RGB_2_B_FACTOR_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  12,      2,27, SNS_REG_DRIVER_RGB_2_C_FACTOR_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  16,      2,27, SNS_REG_DRIVER_RGB_2_IR_FACTOR_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  20,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_01_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  24,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_02_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  28,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_03_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  32,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_04_V02  },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  36,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_05_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  40,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_06_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  44,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_07_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  48,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_08_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  52,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_09_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  56,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_10_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  60,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_11_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  64,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_12_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  68,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_13_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  72,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_14_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  76,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_15_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  80,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_16_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  84,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_17_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  88,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_18_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  92,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_19_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) +  96,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_20_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) + 100,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_21_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) + 104,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_22_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) + 108,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_23_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) + 112,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_24_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) + 116,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_25_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) + 120,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_26_V02 },
  { SNS_REG_TYPE_UINT32,    (MAX_GROUP_SIZE * 55) + 124,      2,27, SNS_REG_DRIVER_RGB_2_DRIVER_SPECIFIC_27_V02 },

  /* Group: SNS_REG_DRIVER_GROUP_GAME_RV_V02 */
  /* Item Type              Offset from start of file      Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 56) +   0,      2,28, SNS_REG_DRIVER_GAME_RV_X_ORIENT_V02         },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 56) +   1,      2,28, SNS_REG_DRIVER_GAME_RV_Y_ORIENT_V02         },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 56) +   2,      2,28, SNS_REG_DRIVER_GAME_RV_Z_ORIENT_V02         },

  /* Group: SNS_REG_GROUP_SSI_SMGR_CFG_3_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57)      ,      2,30, SNS_REG_ITEM_SSI_SMGR_3_MAJ_VER_NO_V02      },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) +   1,      2,30, SNS_REG_ITEM_SSI_SMGR_3_MIN_VER_NO_V02      },
  /* 14 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 57) +  16,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 57) +  24,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 57) +  32,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 57) +  36,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 57) +  40,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) +  42,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) +  44,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) +  46,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) +  48,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) +  50,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) +  51,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) +  52,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) +  53,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) +  54,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) +  55,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) +  56,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) +  57,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG0_3_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 57) +  64,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 57) +  72,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 57) +  80,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 57) +  84,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 57) +  88,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) +  90,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) +  92,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) +  94,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) +  96,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) +  98,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) +  99,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 100,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 101,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 102,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 103,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 104,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 105,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG1_3_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 57) + 112,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 57) + 120,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 57) + 128,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 57) + 132,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 57) + 136,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 157,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 140,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 142,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 144,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 146,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 147,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 148,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 149,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 150,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 151,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 152,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 153,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG2_3_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 57) + 160,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 57) + 168,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 57) + 176,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 57) + 180,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 57) + 184,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 186,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 188,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 190,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 192,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 194,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 195,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 196,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 197,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 198,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 199,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 200,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 201,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG3_3_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 57) + 208,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_UUID_HIGH_V02            },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 57) + 216,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_UUID_LOW_V02             },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 57) + 224,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_OFF_TO_IDLE_V02          },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 57) + 228,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_IDLE_TO_READY_V02        },
  { SNS_REG_TYPE_INT16,     (MAX_GROUP_SIZE * 57) + 232,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_I2C_BUS_V02              },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 234,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_REG_GROUP_ID_V02         },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 236,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_CAL_PRI_GROUP_ID_V02     },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 238,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_GPIO1_V02                },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 57) + 240,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_GPIO2_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 242,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_SENSOR_ID_V02            },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 243,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_I2C_ADDRESS_V02          },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 244,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_DATA_TYPE1_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 245,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_DATA_TYPE2_V02           },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 246,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_RELATED_SENSOR_INDEX_V02 },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 57) + 247,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_SENSITIVITY_DEFAULT_V02  },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 248,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_FLAGS_V02                },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 57) + 249,      2,30, SNS_REG_ITEM_SSI_SMGR_CFG4_3_DEVICE_SELECT_V02        },
  /* 6 reserved bytes */

  /* Group: SNS_REG_SAM_GROUP_TILT_DETECTOR_V02 */
  /* Item Type              Offset from start of file      Version      Item ID                                */
  { SNS_REG_TYPE_Q16,       (MAX_GROUP_SIZE * 58) +   0,      2,29, SNS_REG_ITEM_TILT_DETECTOR_DEF_SENSOR_REPORT_RATE_V02 },
  { SNS_REG_TYPE_Q16,       (MAX_GROUP_SIZE * 58) +   4,      2,29, SNS_REG_ITEM_TILT_DETECTOR_SAMPLE_RATE_V02            },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 58) +   8,      2,29, SNS_REG_ITEM_TILT_DETECTOR_INIT_ACCEL_WINDOW_TIME_V02 },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 58) +   12,     2,29, SNS_REG_ITEM_TILT_DETECTOR_ACCEL_WINDOW_TIME_V02      },
  { SNS_REG_TYPE_FLOAT,     (MAX_GROUP_SIZE * 58) +   16,     2,29, SNS_REG_ITEM_TILT_DETECTOR_DEF_TILT_ANGLE_THRESH_V02  },

    /* Group: SNS_REG_GROUP_SSI_DEVINFO_HEART_RATE_V02 */
  /* Item Type              Offset from start of file   Version      Item ID                                */
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 59) +   0,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_MIN_VER_NO_V02       },
  { SNS_REG_TYPE_INT8,      (MAX_GROUP_SIZE * 59) +   1,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_NUM_UUID_VALID_V02   },
  /* 2 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) +   4,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG0_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) +  12,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG0_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) +  20,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG0_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) +  24,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG0_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) +  28,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG0_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) +  30,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG0_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) +  32,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG0_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) +  34,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG0_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) +  36,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG0_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) +  37,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG0_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) +  38,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG0_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) +  46,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG1_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) +  54,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG1_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) +  62,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG1_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) +  66,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG1_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) +  70,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG1_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) +  72,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG1_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) +  74,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG1_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) +  76,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG1_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) +  78,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG1_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) +  79,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG1_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) +  80,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG1_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) +  88,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG2_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) +  96,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG2_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) + 104,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG2_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) + 108,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG2_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 112,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG2_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 114,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG2_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 116,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG2_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 118,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG2_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 120,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG2_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 121,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG2_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 122,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG2_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) + 130,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG3_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) + 138,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG3_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) + 146,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG3_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) + 150,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG3_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 154,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG3_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 156,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG3_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 158,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG3_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 160,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG3_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 162,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG3_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 163,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG3_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 164,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG3_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) + 172,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG4_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) + 180,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG4_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) + 188,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG4_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) + 192,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG4_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 196,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG4_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 198,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG4_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 200,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG4_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 202,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG4_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 204,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG4_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 205,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG4_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 206,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG4_FLAGS_V02               },
  /* 7 reserved bytes */
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) + 214,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG5_UUID_HIGH_V02           },
  { SNS_REG_TYPE_UINT64,    (MAX_GROUP_SIZE * 59) + 222,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG5_UUID_LOW_V02            },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) + 230,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG5_OFF_TO_IDLE_V02         },
  { SNS_REG_TYPE_INT32,     (MAX_GROUP_SIZE * 59) + 234,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG5_IDLE_TO_READY_V02       },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 238,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG5_GPIO1_V02               },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 240,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG5_REG_GROUP_ID_V02        },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 242,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG5_CAL_PRI_GROUP_ID_V02    },
  { SNS_REG_TYPE_UINT16,    (MAX_GROUP_SIZE * 59) + 244,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG5_I2C_BUS_V02             },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 246,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG5_I2C_ADDRESS_V02         },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 247,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG5_SENSITIVITY_DEFAULT_V02 },
  { SNS_REG_TYPE_UINT8,     (MAX_GROUP_SIZE * 59) + 248,      2,31, SNS_REG_ITEM_SSI_DEVINFO_HEART_RATE_CFG5_FLAGS_V02               }
  /* 7 reserved bytes */


  /* Max offset for this group should be (MAX_GROUP_SIZE * 60) - 1 */
};

const sns_reg_group_info_s sns_reg_group_info[] =
{
  /***************************************************************************/
  /* Actual Group Size(not the max size)  Offset from start of file   Group ID                                   */
  {  24,                                  0,                         SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V01 },
  { 128,                                  MAX_GROUP_SIZE,            SNS_REG_DRIVER_GROUP_PROX_LIGHT_V01        },
  {  16,                                  (MAX_GROUP_SIZE * 2),      SNS_REG_SAM_GROUP_AMD_V01                  },
  {  12,                                  (MAX_GROUP_SIZE * 3),      SNS_REG_SAM_GROUP_VMD_V01                  },
  {  24,                                  (MAX_GROUP_SIZE * 4),      SNS_REG_SAM_GROUP_RMD_V01                  },
  {  21,                                  (MAX_GROUP_SIZE * 5),      SNS_REG_SCM_GROUP_GYRO_CAL_ALGO_V01        },
  { 234,                                  (MAX_GROUP_SIZE * 6),      SNS_REG_SCM_GROUP_ACCEL_DYN_CAL_PARAMS_V02 },
  {  24,                                  (MAX_GROUP_SIZE * 7),      SNS_REG_SCM_GROUP_GYRO_DYN_CAL_PARAMS_V01  },
  {  24,                                  (MAX_GROUP_SIZE * 8),      SNS_REG_SCM_GROUP_GYRO_FAC_CAL_PARAMS_V01  },
  {   5,                                  (MAX_GROUP_SIZE * 9),      SNS_REG_GROUP_SENSOR_TEST_V02              },
  {   3,                                  (MAX_GROUP_SIZE * 10),     SNS_REG_DRIVER_GROUP_ACCEL_V02             },
  {   3,                                  (MAX_GROUP_SIZE * 11),     SNS_REG_DRIVER_GROUP_ACCEL_2_V02           },
  {   3,                                  (MAX_GROUP_SIZE * 12),     SNS_REG_DRIVER_GROUP_GYRO_V02              },
  {   3,                                  (MAX_GROUP_SIZE * 13),     SNS_REG_DRIVER_GROUP_MAG_V02               },
  {  36,                                  (MAX_GROUP_SIZE * 14),     SNS_REG_SAM_GROUP_FMV_PARAMS_V02           },
  {  24,                                  (MAX_GROUP_SIZE * 15),     SNS_REG_SAM_GROUP_GRAVITY_VECTOR_PARAMS_V02},
  {  10,                                  (MAX_GROUP_SIZE * 16),     SNS_REG_SAM_GROUP_ORIENTATION_PARAMS_V02   },
  {  12,                                  (MAX_GROUP_SIZE * 17),     SNS_REG_SAM_GROUP_GYRO_AMD_V02             },
  { 172,                                  (MAX_GROUP_SIZE * 18),     SNS_REG_SAM_GROUP_GYRO_TAP_V02             },
  {  52,                                  (MAX_GROUP_SIZE * 19),     SNS_REG_GROUP_SETTINGS_V02                 },
  {   5,                                  (MAX_GROUP_SIZE * 20),     SNS_REG_SCM_GROUP_ACCEL_CAL_ALGO_V02       },
  {  16,                                  (MAX_GROUP_SIZE * 21),     SNS_REG_SAM_GROUP_PED_V02                  },
  {  13,                                  (MAX_GROUP_SIZE * 22),     SNS_REG_SAM_GROUP_PAM_V02                  },
  { 256,                                  (MAX_GROUP_SIZE * 23),     SNS_REG_GROUP_SSI_SMGR_CFG_V02             },
  { 256,                                  (MAX_GROUP_SIZE * 24),     SNS_REG_GROUP_SSI_DEVINFO_ACCEL_V02        },
  { 256,                                  (MAX_GROUP_SIZE * 25),     SNS_REG_GROUP_SSI_DEVINFO_GYRO_V02         },
  { 256,                                  (MAX_GROUP_SIZE * 26),     SNS_REG_GROUP_SSI_DEVINFO_MAG_V02          },
  { 256,                                  (MAX_GROUP_SIZE * 27),     SNS_REG_GROUP_SSI_DEVINFO_PROX_LIGHT_V02   },
  { 256,                                  (MAX_GROUP_SIZE * 28),     SNS_REG_GROUP_SSI_DEVINFO_PRESSURE_V02     },
  { 224,                                  (MAX_GROUP_SIZE * 29),     SNS_REG_GROUP_SSI_SENSOR_DEP_CFG0_V02      },
  { 224,                                  (MAX_GROUP_SIZE * 30),     SNS_REG_GROUP_SSI_SENSOR_DEP_CFG1_V02      },
  { 34,                                   (MAX_GROUP_SIZE * 31),     SNS_REG_GROUP_SSI_GPIO_CFG_V02             },
  {  4,                                   (MAX_GROUP_SIZE * 32),     SNS_REG_SAM_GROUP_BASIC_GESTURES_V02       },
  {  4,                                   (MAX_GROUP_SIZE * 33),     SNS_REG_SAM_GROUP_FACING_V02               },
  {  4,                                   (MAX_GROUP_SIZE * 34),     SNS_REG_SAM_GROUP_QUATERNION_V02           },
  {  4,                                   (MAX_GROUP_SIZE * 35),     SNS_REG_SAM_GROUP_ROTATION_VECTOR_V02      },
  { 36,                                   (MAX_GROUP_SIZE * 36),     SNS_REG_SAM_GROUP_DISTANCE_BOUND_V02       },
  {  4,                                   (MAX_GROUP_SIZE * 37),     SNS_REG_SAM_GROUP_CMC_V02                  },
  { 256,                                  (MAX_GROUP_SIZE * 38),     SNS_REG_GROUP_SSI_SMGR_CFG_2_V02           },
  { 256,                                  (MAX_GROUP_SIZE * 39),     SNS_REG_GROUP_SSI_DEVINFO_TAP_V02          },
  {  4,                                   (MAX_GROUP_SIZE * 40),     SNS_REG_SAM_GROUP_GAME_ROTATION_VECTOR_V02 },
  { 135,                                  (MAX_GROUP_SIZE * 41),     SNS_REG_SCM_GROUP_QMAG_CAL_ALGO_V02        },
  {  58,                                  (MAX_GROUP_SIZE * 42),     SNS_REG_SCM_GROUP_MAG_DYN_CAL_PARAMS_V02   },
  {  22,                                  (MAX_GROUP_SIZE * 43),     SNS_REG_SCM_GROUP_QMAG_CAL_STATE_V02       },
  {  32,                                  (MAX_GROUP_SIZE * 44),     SNS_REG_SAM_GROUP_SMD_V02                  },
  { 256,                                  (MAX_GROUP_SIZE * 45),     SNS_REG_GROUP_SSI_DEVINFO_HUMIDITY_V02     },
  { 256,                                  (MAX_GROUP_SIZE * 46),     SNS_REG_GROUP_SSI_DEVINFO_RGB_V02          },
  { 256,                                  (MAX_GROUP_SIZE * 47),     SNS_REG_GROUP_SSI_DEVINFO_IR_GESTURE_V02   },
  {  58,                                  (MAX_GROUP_SIZE * 48),     SNS_REG_SCM_GROUP_MAG_FAC_CAL_PARAMS_V02   },
  { 256,                                  (MAX_GROUP_SIZE * 49),     SNS_REG_GROUP_SSI_DEVINFO_SAR_V02          },
  { 256,                                  (MAX_GROUP_SIZE * 50),     SNS_REG_DRIVER_GROUP_IR_GESTURE_V02        },
  { 256,                                  (MAX_GROUP_SIZE * 51),     SNS_REG_DRIVER_GROUP_SAR_V02               },
  { 256,                                  (MAX_GROUP_SIZE * 52),     SNS_REG_DRIVER_GROUP_SAR_2_V02             },
  { 256,                                  (MAX_GROUP_SIZE * 53),     SNS_REG_GROUP_SSI_DEVINFO_HALL_EFFECT_V02  },
  { 128,                                  (MAX_GROUP_SIZE * 54),     SNS_REG_DRIVER_GROUP_RGB_V02               },
  { 128,                                  (MAX_GROUP_SIZE * 55),     SNS_REG_DRIVER_GROUP_RGB_2_V02             },
  {   3,                                  (MAX_GROUP_SIZE * 56),     SNS_REG_DRIVER_GROUP_GAME_RV_V02           },
  { 256,                                  (MAX_GROUP_SIZE * 57),     SNS_REG_GROUP_SSI_SMGR_CFG_3_V02           },
  {  20,                                  (MAX_GROUP_SIZE * 58),     SNS_REG_SAM_GROUP_TILT_DETECTOR_V02        },
  { 256,                                  (MAX_GROUP_SIZE * 59),     SNS_REG_GROUP_SSI_DEVINFO_HEART_RATE_V02   }
  /***************************************************************************/
};

/*============================================================================
  Static Function Definitions and Documentation
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_reg_get_default

  ===========================================================================*/
/*!
  @brief Retrieve the default value of data items.

  @param[i] index: registry data item index.
  @param[o] data_ptr: pointer to the memory location where default value is to
                      be written.

  @return
  SNS_SUCCESS if group is found, SNS_ERR_FAILED otherwise.
*/
/*=========================================================================*/
sns_err_code_e sns_reg_get_default( uint16_t item_id, void* data_ptr )
{
  switch( item_id )
  {
    case SNS_REG_ITEM_ACC_X_BIAS_V01:
    {
      int32_t data = SNS_REG_ACC_DEF_X_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_BIAS_V01:
    {
      int32_t data = SNS_REG_ACC_DEF_Y_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_BIAS_V01:
    {
      int32_t data = SNS_REG_ACC_DEF_Z_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_SCALE_V01:
    {
      uint32_t data = SNS_REG_ACC_DEF_X_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_SCALE_V01:
    {
      uint32_t data = SNS_REG_ACC_DEF_Y_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_SCALE_V01:
    {
      uint32_t data = SNS_REG_ACC_DEF_Z_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ALP_VISIBLE_LIGHT_TRANS_RATIO_V02:
    {
      uint8_t data = SNS_REG_DEFAULT_ALP_VISIBLE_LIGHT_TRANS_RATIO;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_ALP_IR_LIGHT_TRANS_RATIO_V02:
    {
      uint8_t data = SNS_REG_DEFAULT_ALP_IR_LIGHT_TRANS_RATIO;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_ALP_DC_OFFSET_V02:
    {
      uint16_t data = SNS_REG_DEFAULT_ALP_DC_OFFSET;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint16_t ) );
      break;
    }
    case SNS_REG_ITEM_ALP_NEAR_THRESHOLD_V02:
    {
      uint16_t data = SNS_REG_DEFAULT_ALP_NEAR_THRESHOLD;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint16_t ) );
      break;
    }
    case SNS_REG_ITEM_ALP_FAR_THRESHOLD_V02:
    {
      uint16_t data = SNS_REG_DEFAULT_ALP_FAR_THRESHOLD;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint16_t ) );
      break;
    }
    case SNS_REG_ITEM_ALP_PRX_FACTOR_V02:
    {
      uint16_t data = SNS_REG_DEFAULT_ALP_PRX_FACTOR;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint16_t ) );
      break;
    }
    case SNS_REG_ITEM_ALP_ALS_FACTOR_V01:
    {
      uint16_t data = SNS_REG_DEFAULT_ALP_ALS_FACTOR;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint16_t ) );
      break;
    }
    case SNS_REG_ITEM_ALP_DS1_V02:
    case SNS_REG_ITEM_ALP_DS2_V02:
    case SNS_REG_ITEM_ALP_DS3_V02:
    case SNS_REG_ITEM_ALP_DS4_V02:
    case SNS_REG_ITEM_ALP_DS5_V02:
    case SNS_REG_ITEM_ALP_DS6_V02:
    case SNS_REG_ITEM_ALP_DS7_V02:
    case SNS_REG_ITEM_ALP_DS8_V02:
    case SNS_REG_ITEM_ALP_DS9_V02:
    case SNS_REG_ITEM_ALP_DS10_V02:
    case SNS_REG_ITEM_ALP_DS11_V02:
    case SNS_REG_ITEM_ALP_DS12_V02:
    case SNS_REG_ITEM_ALP_DS13_V02:
    case SNS_REG_ITEM_ALP_DS14_V02:
    case SNS_REG_ITEM_ALP_DS15_V02:
    case SNS_REG_ITEM_ALP_DS16_V02:
    case SNS_REG_ITEM_ALP_DS17_V02:
    case SNS_REG_ITEM_ALP_DS18_V02:
    case SNS_REG_ITEM_ALP_DS19_V02:
    case SNS_REG_ITEM_ALP_DS20_V02:
    case SNS_REG_ITEM_ALP_DS21_V02:
    case SNS_REG_ITEM_ALP_DS22_V02:
    case SNS_REG_ITEM_ALP_DS23_V02:
    case SNS_REG_ITEM_ALP_DS24_V02:
    case SNS_REG_ITEM_ALP_DS25_V02:
    case SNS_REG_ITEM_ALP_DS26_V02:
    case SNS_REG_ITEM_ALP_DS27_V02:
    case SNS_REG_ITEM_ALP_DS28_V02:
    case SNS_REG_ITEM_ALP_DS29_V02:
    {
      uint32_t data = SNS_REG_DEFAULT_ALP_DS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_AMD_DEF_ACC_SAMP_RATE_V01:
    {
      int32_t data = SNS_REG_AMD_DEF_ACC_SAMP_RATE_HZ_Q16;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V01:
    {
      int32_t data = SNS_REG_AMD_DEF_INT_CFG_PARAM1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V01:
    {
      int32_t data = SNS_REG_AMD_DEF_INT_CFG_PARAM2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_AMD_DEF_SENSOR_REPORT_RATE_V02:
    {
      int32_t data = SNS_REG_AMD_DEF_SENSOR_REPORT_RATE_Q16;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_VMD_DEF_ACC_SAMP_RATE_V01:
    {
      int32_t data = SNS_REG_VMD_DEF_ACC_SAMP_RATE_HZ_Q16;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V01:
    {
      int32_t data = SNS_REG_VMD_DEF_INT_CFG_PARAM1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_VMD_INT_CFG_PARAM2_V01:
    {
      int32_t data = SNS_REG_VMD_DEF_INT_CFG_PARAM2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_RMD_DEF_ACC_SAMP_RATE_V01:
    {
      int32_t data = SNS_REG_RMD_DEF_ACC_SAMP_RATE_HZ_Q16;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V01:
    {
      int32_t data = SNS_REG_RMD_DEF_INT_CFG_PARAM1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V01:
    {
      int32_t data = SNS_REG_RMD_DEF_INT_CFG_PARAM2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V01:
    {
      int32_t data = SNS_REG_RMD_DEF_INT_CFG_PARAM3;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V01:
    {
      int32_t data = SNS_REG_RMD_DEF_INT_CFG_PARAM4;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_RMD_DEF_SENSOR_REPORT_RATE_V02:
    {
      int32_t data = SNS_REG_RMD_DEF_SENSOR_REPORT_RATE_Q16;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_CAL_DEF_VAR_THOLD_V01:
    {
      int64_t data = SNS_REG_GYRO_CAL_DEF_VARIANCE_THOLD;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int64_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_CAL_DEF_SAMP_RATE_V01:
    {
      int32_t data = SNS_REG_GYRO_CAL_DEF_SAMP_RATE_HZ_Q16;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_CAL_DEF_NUM_SAMP_V01:
    {
      int32_t data = SNS_REG_GYRO_CAL_DEF_NUM_SAMPLES;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_CAL_DEF_ENABLE_ALGO_V01:
    {
      uint8_t data = SNS_REG_GYRO_CAL_ENABLE_ALGO;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_CAL_DEF_SENSOR_REPORT_RATE_V02:
    {
      int32_t data = SNS_REG_GYRO_CAL_DEF_REPORT_RATE_Q16;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }

    // ACCEL DYN CAL
    case SNS_REG_ITEM_ACC_X_DYN_BIAS_V01:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_X_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_BIAS_V01:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_Y_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_BIAS_V01:
    {
       int32_t data = SNS_REG_ACC_DEF_DYN_Z_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_SCALE_V01:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_X_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_SCALE_V01:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Y_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_SCALE_V01:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Z_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }

    case SNS_REG_ITEM_ACC_DYN_CAL_HEADER_V02:
    {
      uint8_t data = SNS_REG_ACC_DEF_HEADER;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_DYN_CAL_TEMP_BIN_SIZE_V02:
    {
      uint8_t data = SNS_REG_ACC_DEF_TEMP_BIN_SIZE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }

    // ACCEL DYN CAL GROUP 1
    case SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP1_V02:
    {
      uint8_t data = SNS_REG_ACC_DEF_VALID_FLAG_GROUP1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP1_V02:
    {
      int8_t data = SNS_REG_ACC_DEF_TEMP_MIN_GROUP1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP1_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP1_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP1_V02:
    {
       int32_t data = SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP1_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP1_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP1_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }

    // ACCEL DYN CAL GROUP 2
    case SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP2_V02:
    {
      uint8_t data = SNS_REG_ACC_DEF_VALID_FLAG_GROUP2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP2_V02:
    {
      int8_t data = SNS_REG_ACC_DEF_TEMP_MIN_GROUP2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP2_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP2_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP2_V02:
    {
       int32_t data = SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP2_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP2_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP2_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }

    // ACCEL DYN CAL GROUP 3
     case SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP3_V02:
    {
      uint8_t data = SNS_REG_ACC_DEF_VALID_FLAG_GROUP3;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP3_V02:
    {
      int8_t data = SNS_REG_ACC_DEF_TEMP_MIN_GROUP3;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP3_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP3;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP3_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP3;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP3_V02:
    {
       int32_t data = SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP3;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP3_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP3;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP3_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP3;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP3_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP3;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }

    // ACCEL DYN CAL GROUP 4
     case SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP4_V02:
    {
      uint8_t data = SNS_REG_ACC_DEF_VALID_FLAG_GROUP4;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP4_V02:
    {
      int8_t data = SNS_REG_ACC_DEF_TEMP_MIN_GROUP4;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP4_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP4;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP4_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP4;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP4_V02:
    {
       int32_t data = SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP4;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP4_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP4;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP4_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP4;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP4_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP4;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }

    // ACCEL DYN CAL GROUP 5
     case SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP5_V02:
    {
      uint8_t data = SNS_REG_ACC_DEF_VALID_FLAG_GROUP5;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP5_V02:
    {
      int8_t data = SNS_REG_ACC_DEF_TEMP_MIN_GROUP5;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP5_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP5;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP5_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP5;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP5_V02:
    {
       int32_t data = SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP5;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP5_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP5;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP5_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP5;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP5_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP5;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }

    // ACCEL DYN CAL GROUP 6
     case SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP6_V02:
    {
      uint8_t data = SNS_REG_ACC_DEF_VALID_FLAG_GROUP6;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP6_V02:
    {
      int8_t data = SNS_REG_ACC_DEF_TEMP_MIN_GROUP6;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP6_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP6;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP6_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP6;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP6_V02:
    {
       int32_t data = SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP6;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP6_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP6;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP6_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP6;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP6_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP6;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }

    // ACCEL DYN CAL GROUP 7
     case SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP7_V02:
    {
      uint8_t data = SNS_REG_ACC_DEF_VALID_FLAG_GROUP7;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP7_V02:
    {
      int8_t data = SNS_REG_ACC_DEF_TEMP_MIN_GROUP7;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP7_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP7;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP7_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP7;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP7_V02:
    {
       int32_t data = SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP7;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP7_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP7;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP7_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP7;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP7_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP7;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }

    // ACCEL DYN CAL GROUP 8
     case SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP8_V02:
    {
      uint8_t data = SNS_REG_ACC_DEF_VALID_FLAG_GROUP8;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP8_V02:
    {
      int8_t data = SNS_REG_ACC_DEF_TEMP_MIN_GROUP8;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int8_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP8_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_X_BIAS_GROUP8;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP8_V02:
    {
      int32_t data = SNS_REG_ACC_DEF_DYN_Y_BIAS_GROUP8;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP8_V02:
    {
       int32_t data = SNS_REG_ACC_DEF_DYN_Z_BIAS_GROUP8;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP8_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_X_SCALE_GROUP8;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP8_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Y_SCALE_GROUP8;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP8_V02:
    {
      uint32_t data = SNS_REG_ACC_DEF_DYN_Z_SCALE_GROUP8;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }

    case SNS_REG_ITEM_GYRO_X_DYN_BIAS_V01:
    {
      int32_t data = SNS_REG_GYRO_DEF_DYN_X_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_Y_DYN_BIAS_V01:
    {
      int32_t data = SNS_REG_GYRO_DEF_DYN_Y_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_Z_DYN_BIAS_V01:
    {
      int32_t data = SNS_REG_GYRO_DEF_DYN_Z_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_X_DYN_SCALE_V01:
    {
      uint32_t data = SNS_REG_GYRO_DEF_DYN_X_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_Y_DYN_SCALE_V01:
    {
      uint32_t data = SNS_REG_GYRO_DEF_DYN_Y_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_Z_DYN_SCALE_V01:
    {
      uint32_t data = SNS_REG_GYRO_DEF_DYN_Z_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_X_BIAS_V01:
    {
      int32_t data = SNS_REG_GYRO_DEF_X_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_Y_BIAS_V01:
    {
      int32_t data = SNS_REG_GYRO_DEF_Y_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_Z_BIAS_V01:
    {
      int32_t data = SNS_REG_GYRO_DEF_Z_BIAS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_X_SCALE_V01:
    {
      uint32_t data = SNS_REG_GYRO_DEF_X_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_Y_SCALE_V01:
    {
      uint32_t data = SNS_REG_GYRO_DEF_Y_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_Z_SCALE_V01:
    {
      uint32_t data = SNS_REG_GYRO_DEF_Z_SCALE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_ITEM_RAW_DATA_MODE_V02:
    {
      uint8_t data = SNS_REG_DEF_RAW_DATA_MODE;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
      break;
    }
    case SNS_REG_ITEM_TEST_EN_FLAGS_V02:
    {
      uint32_t data = SNS_REG_DEF_TEST_EN_FLAGS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }
    case SNS_REG_DRIVER_ACCEL_X_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_ACCEL_X_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_ACCEL_Y_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_ACCEL_Y_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_ACCEL_Z_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_ACCEL_Z_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_ACCEL2_X_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_ACCEL2_X_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_ACCEL2_Y_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_ACCEL2_Y_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_ACCEL2_Z_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_ACCEL2_Z_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_GYRO_X_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_GYRO_X_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_GYRO_Y_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_GYRO_Y_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_GYRO_Z_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_GYRO_Z_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_MAG_X_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_MAG_X_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_MAG_Y_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_MAG_Y_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_MAG_Z_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_MAG_Z_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_GAME_RV_X_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_GAME_RV_X_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_GAME_RV_Y_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_GAME_RV_Y_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }
    case SNS_REG_DRIVER_GAME_RV_Z_ORIENT_V02:
    {
      int8_t data = SNS_REG_DRIVER_GAME_RV_Z_ORIENT;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof (int8_t) );
      break;
    }

    // FMV
    case SNS_REG_ITEM_FMV_TC_ACCURACY_0_V02:
    {
       uint32_t data = SNS_REG_DEF_FMV_TC_ACCURACY_0;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_FMV_TC_ACCURACY_1_V02:
    {
       uint32_t data = SNS_REG_DEF_FMV_TC_ACCURACY_1;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_FMV_TC_ACCURACY_2_V02:
    {
       uint32_t data = SNS_REG_DEF_FMV_TC_ACCURACY_2;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_FMV_TC_ACCURACY_3_V02:
    {
       uint32_t data = SNS_REG_DEF_FMV_TC_ACCURACY_3;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_FMV_GYRO_GAP_THRESH_V02:
    {
       uint32_t data = SNS_REG_DEF_FMV_GYRO_GAP_THRESH;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_FMV_MAG_GAP_FACTOR_V02:
    {
       float data = SNS_REG_DEF_FMV_MAG_GAP_FACTOR;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_FMV_ROT_FOR_ZUPT_V02:
    {
       float data = SNS_REG_DEF_FMV_ROT_FOR_ZUPT;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_FMV_MAX_MAG_INNOVATION_V02:
    {
       float data = SNS_REG_DEF_FMV_MAX_MAG_INNOV;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_FMV_DEF_SENSOR_REPORT_RATE_V02:
    {
       int32_t data = SNS_REG_FMV_DEF_SENSOR_REPORT_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    // GRAVITY VECTOR
    case SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM1_V02:
    {
       uint32_t data = SNS_REG_DEF_GRAVITY_VECTOR_INT_CFG_PARAM1;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM2_V02:
    {
       uint32_t data = SNS_REG_DEF_GRAVITY_VECTOR_INT_CFG_PARAM2;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM3_V02:
    {
       uint32_t data = SNS_REG_DEF_GRAVITY_VECTOR_INT_CFG_PARAM3;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM4_V02:
    {
       uint32_t data = SNS_REG_DEF_GRAVITY_VECTOR_INT_CFG_PARAM4;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM5_V02:
    {
       float data = SNS_REG_DEF_GRAVITY_VECTOR_INT_CFG_PARAM5;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_GRAVITY_VECTOR_DEF_SENSOR_REPORT_RATE_V02:
    {
       int32_t data = SNS_REG_GRAVITY_VECTOR_DEF_SENSOR_REPORT_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    // ORIENTATION
    case SNS_REG_ITEM_ORIENTATION_INT_CFG_PARAM1_V02:
    {
       uint32_t data = SNS_REG_DEF_ORIENTATION_INT_CFG_PARAM1;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_MAG_CAL_LAT_NUM_SAMPLES_V02:
    {
       uint8_t data = SNS_REG_DEF_MAG_CAL_LAT_NUM_SAMPLES;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }
    case SNS_REG_ITEM_FUSION_MIN_SAMP_RATE_HZ_V02:
    {
       uint8_t data = SNS_REG_DEF_FUSION_MIN_SAMP_RATE_HZ;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }
    case SNS_REG_ITEM_ORIENTATION_DEF_SENSOR_REPORT_RATE_V02:
    {
       int32_t data = SNS_REG_ORIENTATION_DEF_SENSOR_REPORT_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    // GYRO AMD
    case SNS_REG_ITEM_GYRO_AMD_INT_CFG_PARAM1_V02:
    {
      float data = SNS_REG_DEF_GYRO_AMD_INT_CFG_PARAM1;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_AMD_INT_CFG_PARAM2_V02:
    {
      float data = SNS_REG_DEF_GYRO_AMD_INT_CFG_PARAM2;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
      break;
    }
    case SNS_REG_ITEM_GYRO_AMD_INT_CFG_PARAM3_V02:
    {
      float data = SNS_REG_DEF_GYRO_AMD_INT_CFG_PARAM3;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
      break;
    }
    case SNS_REG_GYRO_TAP_VERSION_V02:
    {
      int32_t data = SNS_REG_GYRO_TAP_DEF_VERSION;
      SNS_OS_MEMCOPY(data_ptr, &data, sizeof (int32_t));
      break;
    }

    case SNS_REG_GYRO_TAP_SCENARIO_0_V02:
    {
      int32_t data = SNS_REG_GYRO_INVALID_SCENARIO;
      SNS_OS_MEMCOPY(data_ptr, &data, sizeof (int32_t));
      break;
    }
    case SNS_REG_GYRO_TAP_SCENARIO_1_V02:
    {
      int32_t data = SNS_REG_GYRO_INVALID_SCENARIO;
      SNS_OS_MEMCOPY(data_ptr, &data, sizeof (int32_t));
      break;
    }
    case SNS_REG_GYRO_TAP_SCENARIO_2_V02:
    {
      int32_t data = SNS_REG_GYRO_INVALID_SCENARIO;
      SNS_OS_MEMCOPY(data_ptr, &data, sizeof (int32_t));
      break;
    }
    case SNS_REG_SETTINGS_MG_WORD_V02:
    {
      uint64_t data = SNS_REG_SETTINGS_MG_WORD;
      SNS_OS_MEMCOPY(data_ptr, &data, sizeof (uint64_t));
      break;
    }
    case SNS_REG_SETTINGS_VERSION_MINOR_V02:
    {
      uint32_t data = SNS_REG_SETTINGS_VERSION_MAJOR;
      SNS_OS_MEMCOPY(data_ptr, &data, sizeof (uint32_t));
      break;
    }
    case SNS_REG_SETTINGS_VERSION_MAJOR_V02:
    {
      uint32_t data = SNS_REG_SETTINGS_VERSION_MINOR;
      SNS_OS_MEMCOPY(data_ptr, &data, sizeof (uint32_t));
      break;
    }
    case SNS_REG_SETTINGS_FILE1_NAME_V02:
    case SNS_REG_SETTINGS_FILE2_NAME_V02:
    case SNS_REG_SETTINGS_FILE3_NAME_V02:
    {
      static const uint64_t data = 0;
      SNS_OS_MEMCOPY(data_ptr, &data, sizeof (data));
      break;
    }
    case SNS_REG_SETTINGS_FILE1_VERSION_V02:
    case SNS_REG_SETTINGS_FILE2_VERSION_V02:
    case SNS_REG_SETTINGS_FILE3_VERSION_V02:
    {
      static const uint32_t data = 0;
      SNS_OS_MEMCOPY(data_ptr, &data, sizeof (data));
      break;
    }

    // ACCEL_CAL
    case SNS_REG_ITEM_ACCEL_CAL_DEF_SAMP_RATE_V02:
    {
      int32_t data = SNS_REG_ACCEL_CAL_DEF_SAMP_RATE_HZ_Q16;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }
    case SNS_REG_ITEM_ACCEL_CAL_DEF_ENABLE_ALGO_V02:
    {
       uint8_t data = SNS_REG_ACCEL_CAL_ENABLE_ALGO;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    // Pedometer
    case SNS_REG_ITEM_PED_DEF_STEP_THRESHOLD_V02:
    {
      float data = SNS_REG_PED_DEF_STEP_THRESHOLD;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
      break;
    }
    case SNS_REG_ITEM_PED_DEF_SWING_THRESHOLD_V02:
    {
       float data = SNS_REG_PED_DEF_SWING_THRESHOLD;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_PED_DEF_STEP_PROB_THRESHOLD_V02:
    {
      float data = SNS_REG_PED_DEF_STEP_PROB_THRESHOLD;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
      break;
    }
    case SNS_REG_ITEM_PED_DEF_SENSOR_REPORT_RATE_V02:
    {
      int32_t data = SNS_REG_PED_DEF_SENSOR_REPORT_RATE_Q16;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
      break;
    }

    // PAM
    case SNS_REG_ITEM_PAM_INT_CFG_PARAM1_V02:
    {
       uint32_t data = SNS_REG_DEF_PAM_INT_CFG_PARAM1;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_PAM_INT_CFG_PARAM2_V02:
    {
       uint32_t data = SNS_REG_DEF_PAM_INT_CFG_PARAM2;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_PAM_INT_CFG_PARAM3_V02:
    {
       uint32_t data = SNS_REG_DEF_PAM_INT_CFG_PARAM3;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_PAM_INT_CFG_PARAM4_V02:
    {
       uint8_t data = SNS_REG_DEF_PAM_INT_CFG_PARAM4;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    // Basic gestures algorithm
    case SNS_REG_ITEM_BASIC_GESTURES_DEF_SENSOR_REPORT_RATE_V02:
    {
       int32_t data = SNS_REG_BASIC_GESTURES_DEF_SENSOR_REPORT_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    // Facing algorithm
    case SNS_REG_ITEM_FACING_DEF_SENSOR_REPORT_RATE_V02:
    {
       int32_t data = SNS_REG_FACING_DEF_SENSOR_REPORT_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    // Quaternion algorithm
    case SNS_REG_ITEM_QUATERNION_DEF_SENSOR_REPORT_RATE_V02:
    {
       int32_t data = SNS_REG_QUATERNION_DEF_SENSOR_REPORT_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    // Rotation vector algorithm
    case SNS_REG_ITEM_ROTATION_VECTOR_DEF_SENSOR_REPORT_RATE_V02:
    {
       int32_t data = SNS_REG_ROTATION_VECTOR_DEF_SENSOR_REPORT_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    // Distance Bound algorithm
    case SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_UNKNOWN_V02:
    {
       float data = SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_UNKNOWN;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_STATIONARY_V02:
    {
       float data = SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_STATIONARY;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_INMOTION_V02:
    {
       float data = SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_INMOTION;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_FIDDLE_V02:
    {
       float data = SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_FIDDLE;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_PEDESTRIAN_V02:
    {
       float data = SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_PEDESTRIAN;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_VEHICLE_V02:
    {
       float data = SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_VEHICLE;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_WALK_V02:
    {
       float data = SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_WALK;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_RUN_V02:
    {
       float data = SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_RUN;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_BIKE_V02:
    {
       float data = SNS_REG_DISTANCE_BOUND_DEF_SPEEDBOUNDS_BIKE;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }

    // Coarse Motion Classifier
    case SNS_REG_ITEM_CMC_DEF_SENSOR_REPORT_RATE_V02:
    {
       int32_t data = SNS_REG_CMC_DEF_SENSOR_REPORT_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    // QMAG Calibration
    case SNS_REG_ITEM_QMAG_CAL_VERSION_V02:
    {
       uint32_t data = SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_VERSION;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_QMAG_CAL_ENABLE_ALGO_V02:
    {
       uint8_t data = SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_ENABLE_ALGO;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }
    case SNS_REG_ITEM_QMAG_CAL_ENABLE_SI_CAL_V02:
    {
       uint8_t data = SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_ENABLE_SI_CAL;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }
    case SNS_REG_ITEM_QMAG_CAL_ENABLE_HI_CAL_V02:
    {
       uint8_t data = SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_ENABLE_HI_CAL;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }
    case SNS_REG_ITEM_SAMPLE_RATE_V02:
    {
       int32_t data = SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_SAMPLE_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }
    case SNS_REG_ITEM_QMAG_CAL_REPORT_RATE_V02:
    {
       int32_t data = SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_REPORT_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }
    case SNS_REG_ITEM_QMAG_CAL_ENABLE_GYRO_ASSIST_V02:
    {
       uint8_t data = SNS_REG_GROUP_QMAG_CAL_ALGO_DEF_ENABLE_GYRO_ASSIST;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    // MAG CAL Dynamic parameters
    case SNS_REG_ITEM_MAG_DYN_CAL_VERSION_V02:
    {
       uint32_t data = SNS_REG_MAG_DYN_CAL_DEF_VERSION;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_MAG_DYN_CAL_BIAS_VALID_V02:
    {
       uint8_t data = SNS_REG_MAG_DYN_CAL_DEF_BIAS_VALID;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }
    case SNS_REG_ITEM_MAG_DYN_CAL_CALIBRATION_MATRIX_VALID_V02:
    {
       uint8_t data = SNS_REG_MAG_DYN_CAL_DEF_CAL_MAT_VALID;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }
    case SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_0_0_V02:
    case SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_1_1_V02:
    case SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_2_2_V02:
    {
       int32_t data = SNS_REG_MAG_DYN_CAL_DEF_COMP_MAT_DIAG_ELEMENT_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }
    case SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_0_1_V02:
    case SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_0_2_V02:
    case SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_1_0_V02:
    case SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_1_2_V02:
    case SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_2_0_V02:
    case SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_2_1_V02:
    {
       int32_t data = SNS_REG_MAG_DYN_CAL_DEF_COMP_MAT_NONDIAG_ELEMENT_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    // Game Rotation Vector algorithm
    case SNS_REG_ITEM_GAME_ROT_VEC_DEF_SENSOR_REPORT_RATE_V02:
    {
       int32_t data = SNS_REG_GAME_ROT_VEC_DEF_SENSOR_REPORT_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    // QMag Cal State
    case SNS_REG_ITEM_QMAG_CAL_STATE_VERSION_V02:
    {
       uint32_t data = SNS_REG_QMAG_CAL_STATE_DEF_VERSION;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_X_HI_V02:
    {
       float data = SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_OFFSET_X_HI;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_Y_HI_V02:
    {
       float data = SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_OFFSET_Y_HI;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_Z_HI_V02:
    {
       float data = SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_OFFSET_Z_HI;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }
    case SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_ACCURACY_HI_V02:
    {
       uint8_t data = SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_ACCURACY_HI;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }
    case SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_TIME_V02:
    {
       uint32_t data = SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_OFFSET_TIME;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_VALID_V02:
    {
       uint8_t data = SNS_REG_QMAG_CAL_STATE_DEF_PUBLISH_OFFSET_VALID;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    // SMD
    case SNS_REG_ITEM_SMD_ACC_NORM_SDEV_THRESH_V02:
    {
       int32_t data = SNS_REG_SMD_DEF_ACC_NORM_STD_DEV_THRESH;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_VAR_DEC_LAT_V02:
    {
       uint8_t data = SNS_REG_SMD_DEF_VAR_DECISION_LATENCY;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_MAX_LAT_V02:
    {
       uint8_t data = SNS_REG_SMD_DEF_MAX_LATENCY;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_ACC_WIN_TIME_V02:
    {
       uint8_t data = SNS_REG_SMD_DEF_ACCEL_WINDOW_TIME;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_LACC_WIN_TIME_V02:
    {
       uint8_t data = SNS_REG_SMD_DEF_LONG_ACCEL_WINDOW_TIME;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_STRANS_PROB_SM_V02:
    {
       int32_t data = SNS_REG_SMD_DEF_SELF_TRANS_PROB_SM;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_DETECT_THRESH_V02:
    {
       int32_t data = SNS_REG_SMD_DEF_DETECT_THRESH;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_EIGEN_THRESH_V02:
    {
       int32_t data = SNS_REG_SMD_DEF_EIGEN_THRESH;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_PCOUNT_THRESH_V02:
    {
       uint8_t data = SNS_REG_SMD_DEF_STEP_COUNT_THRESH;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

     case SNS_REG_ITEM_SMD_PTIME_V02:
    {
       uint8_t data = SNS_REG_SMD_DEF_STEP_WINDOW_TIME;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_DELTA_RESET_TIME_V02:
    {
       uint8_t data = SNS_REG_SMD_DEF_DELTA_RESET_TIME;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_SENSOR_REPORT_RATE_V02:
    {
       int32_t data = SNS_REG_SMD_DEF_SENSOR_REPORT_RATE_HZ_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_VERSION_V02:
    {
       uint8_t data = SNS_REG_SMD_DEF_VERSION;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    case SNS_REG_ITEM_SMD_SAMP_RATE_V02:
    {
       int32_t data = SNS_REG_SMD_DEF_SAMPLE_RATE_HZ_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }

    // MAG CAL Factory parameters
    case SNS_REG_ITEM_MAG_FAC_CAL_VERSION_V02:
    {
       uint32_t data = SNS_REG_MAG_FAC_CAL_DEF_VERSION;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
       break;
    }
    case SNS_REG_ITEM_MAG_FAC_CAL_BIAS_VALID_V02:
    {
       uint8_t data = SNS_REG_MAG_FAC_CAL_DEF_BIAS_VALID;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }
    case SNS_REG_ITEM_MAG_FAC_CAL_CALIBRATION_MATRIX_VALID_V02:
    {
       uint8_t data = SNS_REG_MAG_FAC_CAL_DEF_CAL_MAT_VALID;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }
    case SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_0_0_V02:
    case SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_1_1_V02:
    case SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_2_2_V02:
    {
       int32_t data = SNS_REG_MAG_FAC_CAL_DEF_COMP_MAT_DIAG_ELEMENT_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }
    case SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_0_1_V02:
    case SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_0_2_V02:
    case SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_1_0_V02:
    case SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_1_2_V02:
    case SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_2_0_V02:
    case SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_2_1_V02:
    {
       int32_t data = SNS_REG_MAG_FAC_CAL_DEF_COMP_MAT_NONDIAG_ELEMENT_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( int32_t ) );
       break;
    }
    case SNS_REG_ITEM_IR_GESTURE_DS1_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS2_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS3_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS4_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS5_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS6_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS7_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS8_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS9_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS10_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS11_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS12_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS13_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS14_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS15_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS16_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS17_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS18_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS19_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS20_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS21_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS22_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS23_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS24_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS25_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS26_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS27_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS28_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS29_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS30_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS31_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS32_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS33_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS34_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS35_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS36_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS37_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS38_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS39_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS40_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS41_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS42_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS43_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS44_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS45_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS46_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS47_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS48_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS49_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS50_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS51_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS52_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS53_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS54_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS55_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS56_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS57_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS58_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS59_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS60_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS61_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS62_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS63_V02:
    case SNS_REG_ITEM_IR_GESTURE_DS64_V02:
    {
      uint32_t data = SNS_REG_DEFAULT_IR_GESTURE_DS;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }

    case SNS_REG_DRIVER_RGB_R_FACTOR_V02:
    case SNS_REG_DRIVER_RGB_G_FACTOR_V02:
    case SNS_REG_DRIVER_RGB_B_FACTOR_V02:
    case SNS_REG_DRIVER_RGB_C_FACTOR_V02:
    case SNS_REG_DRIVER_RGB_IR_FACTOR_V02:
    case SNS_REG_DRIVER_RGB_2_R_FACTOR_V02:
    case SNS_REG_DRIVER_RGB_2_G_FACTOR_V02:
    case SNS_REG_DRIVER_RGB_2_B_FACTOR_V02:
    case SNS_REG_DRIVER_RGB_2_C_FACTOR_V02:
    case SNS_REG_DRIVER_RGB_2_IR_FACTOR_V02:
    {
      uint32_t data = SNS_REG_DEFAULT_RGB_FACTOR;
      SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint32_t ) );
      break;
    }

    // Tilt Detector algorithm
    case SNS_REG_ITEM_TILT_DETECTOR_DEF_SENSOR_REPORT_RATE_V02:
    {
       q16_t data = SNS_REG_TILT_DETECTOR_DEF_SENSOR_REPORT_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( q16_t ) );
       break;
    }

    case SNS_REG_ITEM_TILT_DETECTOR_SAMPLE_RATE_V02:
    {
       q16_t data = SNS_REG_TILT_DETECTOR_DEF_SAMPLE_RATE_Q16;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( q16_t ) );
       break;
    }

    case SNS_REG_ITEM_TILT_DETECTOR_INIT_ACCEL_WINDOW_TIME_V02:
    {
       float data = SNS_REG_TILT_DETECTOR_DEF_INIT_ACCEL_WINDOW_TIME;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }

    case SNS_REG_ITEM_TILT_DETECTOR_ACCEL_WINDOW_TIME_V02:
    {
       float data = SNS_REG_TILT_DETECTOR_DEF_ACCEL_WINDOW_TIME;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }

    case SNS_REG_ITEM_TILT_DETECTOR_DEF_TILT_ANGLE_THRESH_V02:
    {
       float data = SNS_REG_TILT_DETECTOR_DEF_TILT_ANGLE_THRESH_RAD;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( float ) );
       break;
    }

    case SNS_REG_ITEM_SSI_GPIO_CFG_VERSION_MAJOR_V02:
    {
       uint8_t data = SNS_REG_DEF_ITEM_SSI_GPIO_CFG_MAJ_VER_NO;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    case SNS_REG_ITEM_SSI_GPIO_CFG_VERSION_MINOR_V02:
    {
       uint8_t data = SNS_REG_DEF_ITEM_SSI_GPIO_CFG_MIN_VER_NO;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint8_t ) );
       break;
    }

    case SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SDA_1_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SCL_1_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SDA_2_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SCL_2_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_RESET_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_TEST_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_ACCEL_MD_INT_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_ACCEL_DRDY_INT_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_GYRO_DRDY_INT_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_MAG_DRDY_INT_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_ALSP_INT_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_GEST_INT_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_PRESS_INT_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_SAR_INT_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_FP_INT_V02:
    case SNS_REG_ITEM_SSI_GPIO_CFG_HALL_INT_V02:
    {
       uint16_t data = SNS_REG_DEF_ITEM_SSI_GPIO_CFG_GPIO;
       SNS_OS_MEMCOPY( data_ptr, &data, sizeof( uint16_t ) );
       break;
    }

    default:
    {
      return SNS_ERR_FAILED;
    }
  }
  return SNS_SUCCESS;
}

/*============================================================================
  Externalized Function Definitions and Documentation
  ============================================================================*/
/*===========================================================================

  FUNCTION:   sns_group_data_init

  ===========================================================================*/
/*!
  @brief Retrieve the default values of all data items in data groups.

  @param[i] index: registry data group index.
  @param[i] data_ptr: pointer to the data group structure where default values
                      are to be written.

  @return
  SNS_SUCCESS if group is found, SNS_ERR_FAILED otherwise.
*/
/*=========================================================================*/
sns_err_code_e sns_group_data_init( uint16_t group_id, void* data_ptr )
{
  switch( group_id )
  {
    case SNS_REG_SCM_GROUP_ACCEL_FAC_CAL_PARAMS_V01:
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_BIAS_V01,
        &(((sns_reg_acc_data_group_s*)data_ptr)->accelx_bias) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_BIAS_V01,
        &(((sns_reg_acc_data_group_s*)data_ptr)->accely_bias) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_BIAS_V01,
        &(((sns_reg_acc_data_group_s*)data_ptr)->accelz_bias) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_SCALE_V01,
        &(((sns_reg_acc_data_group_s*)data_ptr)->accelx_scale) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_SCALE_V01,
        &(((sns_reg_acc_data_group_s*)data_ptr)->accely_scale) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_SCALE_V01,
        &(((sns_reg_acc_data_group_s*)data_ptr)->accelz_scale) );
      break;

    case SNS_REG_DRIVER_GROUP_PROX_LIGHT_V02:
      sns_reg_get_default( SNS_REG_ITEM_ALP_VISIBLE_LIGHT_TRANS_RATIO_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->visible_light_trans_ratio) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_IR_LIGHT_TRANS_RATIO_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ir_light_trans_ratio) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DC_OFFSET_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->dc_offset) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_NEAR_THRESHOLD_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->near_threshold) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_FAR_THRESHOLD_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->far_threshold) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_PRX_FACTOR_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->prx_factor) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_ALS_FACTOR_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->als_factor) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS1_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds1) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS2_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds2) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS3_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds3) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS4_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds4) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS5_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds5) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS6_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds6) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS7_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds7) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS8_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds8) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS9_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds9) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS10_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds10) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS11_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds11) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS12_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds12) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS13_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds13) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS14_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds14) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS15_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds15) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS16_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds16) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS17_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds17) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS18_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds18) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS19_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds19) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS20_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds20) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS21_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds21) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS22_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds22) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS23_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds23) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS24_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds24) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS25_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds25) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS26_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds26) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS27_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds27) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS28_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds28) );
      sns_reg_get_default( SNS_REG_ITEM_ALP_DS29_V02,
        &(((sns_reg_alp_data_group_s*)data_ptr)->ds29) );
      break;

    case SNS_REG_SAM_GROUP_AMD_V01:
      sns_reg_get_default( SNS_REG_ITEM_AMD_DEF_ACC_SAMP_RATE_V01,
        &(((sns_reg_amd_data_group_s*)data_ptr)->def_acc_samp_rate));
      sns_reg_get_default( SNS_REG_ITEM_AMD_INT_CFG_PARAM1_V01,
        &(((sns_reg_amd_data_group_s*)data_ptr)->int_cfg_param1));
      sns_reg_get_default( SNS_REG_ITEM_AMD_INT_CFG_PARAM2_V01,
        &(((sns_reg_amd_data_group_s*)data_ptr)->int_cfg_param2));
      sns_reg_get_default( SNS_REG_ITEM_AMD_DEF_SENSOR_REPORT_RATE_V02,
        &(((sns_reg_amd_data_group_s*)data_ptr)->sensor_report_rate));
      break;

    case SNS_REG_SAM_GROUP_VMD_V01:
      sns_reg_get_default( SNS_REG_ITEM_VMD_DEF_ACC_SAMP_RATE_V01,
        &(((sns_reg_vmd_data_group_s*)data_ptr)->def_acc_samp_rate));
      sns_reg_get_default( SNS_REG_ITEM_VMD_INT_CFG_PARAM1_V01,
        &(((sns_reg_vmd_data_group_s*)data_ptr)->int_cfg_param1));
      sns_reg_get_default( SNS_REG_ITEM_VMD_INT_CFG_PARAM2_V01,
        &(((sns_reg_vmd_data_group_s*)data_ptr)->int_cfg_param2));
      break;

    case SNS_REG_SAM_GROUP_RMD_V01:
      sns_reg_get_default( SNS_REG_ITEM_RMD_DEF_ACC_SAMP_RATE_V01,
        &(((sns_reg_rmd_data_group_s*)data_ptr)->def_acc_samp_rate));
      sns_reg_get_default( SNS_REG_ITEM_RMD_INT_CFG_PARAM1_V01,
        &(((sns_reg_rmd_data_group_s*)data_ptr)->int_cfg_param1));
      sns_reg_get_default( SNS_REG_ITEM_RMD_INT_CFG_PARAM2_V01,
        &(((sns_reg_rmd_data_group_s*)data_ptr)->int_cfg_param2));
      sns_reg_get_default( SNS_REG_ITEM_RMD_INT_CFG_PARAM3_V01,
        &(((sns_reg_rmd_data_group_s*)data_ptr)->int_cfg_param3));
      sns_reg_get_default( SNS_REG_ITEM_RMD_INT_CFG_PARAM4_V01,
        &(((sns_reg_rmd_data_group_s*)data_ptr)->int_cfg_param4));
      sns_reg_get_default( SNS_REG_ITEM_RMD_DEF_SENSOR_REPORT_RATE_V02,
        &(((sns_reg_rmd_data_group_s*)data_ptr)->sensor_report_rate));
      break;

    case SNS_REG_SCM_GROUP_GYRO_CAL_ALGO_V01:
      sns_reg_get_default( SNS_REG_ITEM_GYRO_CAL_DEF_VAR_THOLD_V01,
        &(((sns_reg_gyro_cal_data_group_s*)data_ptr)->def_gyro_var_thold) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_CAL_DEF_SAMP_RATE_V01,
        &(((sns_reg_gyro_cal_data_group_s*)data_ptr)->def_gyro_sample_rate) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_CAL_DEF_NUM_SAMP_V01,
        &(((sns_reg_gyro_cal_data_group_s*)data_ptr)->def_gyro_num_samples) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_CAL_DEF_ENABLE_ALGO_V01,
         &(((sns_reg_gyro_cal_data_group_s*)data_ptr)->def_gyro_cal_algo_state) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_CAL_DEF_SENSOR_REPORT_RATE_V02,
         &(((sns_reg_gyro_cal_data_group_s*)data_ptr)->def_sensor_report_rate) );
      break;

     case SNS_REG_SCM_GROUP_ACCEL_DYN_CAL_PARAMS_V01:
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_BIAS_V01,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_bias) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_BIAS_V01,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_bias) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_BIAS_V01,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_bias) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_SCALE_V01,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_scale) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_SCALE_V01,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_scale) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_SCALE_V01,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_scale) );

      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_HEADER_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->header) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_TEMP_BIN_SIZE_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->temp_bin_size) );

      // group1
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP1_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->valid_flag_group1) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP1_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->temp_min_group1) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP1_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_bias_group1) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP1_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_bias_group1) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP1_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_bias_group1) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP1_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_scale_group1) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP1_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_scale_group1) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP1_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_scale_group1) );

      // group2
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP2_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->valid_flag_group2) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP2_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->temp_min_group2) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP2_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_bias_group2) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP2_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_bias_group2) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP2_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_bias_group2) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP2_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_scale_group2) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP2_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_scale_group2) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP2_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_scale_group2) );

      // group3
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP3_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->valid_flag_group3) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP3_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->temp_min_group3) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP3_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_bias_group3) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP3_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_bias_group3) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP3_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_bias_group3) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP3_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_scale_group3) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP3_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_scale_group3) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP3_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_scale_group3) );

      // group4
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP4_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->valid_flag_group4) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP4_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->temp_min_group4) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP4_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_bias_group4) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP4_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_bias_group4) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP4_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_bias_group4) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP4_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_scale_group4) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP4_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_scale_group4) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP4_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_scale_group4) );

      // group5
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP5_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->valid_flag_group5) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP5_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->temp_min_group5) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP5_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_bias_group5) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP5_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_bias_group5) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP5_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_bias_group5) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP5_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_scale_group5) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP5_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_scale_group5) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP5_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_scale_group5) );

      // group6
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP6_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->valid_flag_group6) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP6_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->temp_min_group6) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP6_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_bias_group6) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP6_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_bias_group6) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP6_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_bias_group6) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP6_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_scale_group6) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP6_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_scale_group6) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP6_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_scale_group6) );

      // group7
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP7_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->valid_flag_group7) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP7_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->temp_min_group7) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP7_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_bias_group7) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP7_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_bias_group7) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP7_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_bias_group7) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP7_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_scale_group7) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP7_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_scale_group7) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP7_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_scale_group7) );

      // group8
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_VALID_FLAG_GROUP8_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->valid_flag_group8) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_DYN_CAL_TEMP_MIN_GROUP8_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->temp_min_group8) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_BIAS_GROUP8_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_bias_group8) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_BIAS_GROUP8_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_bias_group8) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_BIAS_GROUP8_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_bias_group8) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_X_DYN_SCALE_GROUP8_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelx_scale_group8) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Y_DYN_SCALE_GROUP8_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accely_scale_group8) );
      sns_reg_get_default( SNS_REG_ITEM_ACC_Z_DYN_SCALE_GROUP8_V02,
          &(((sns_reg_accel_dyn_cal_params_data_group_s*)data_ptr)->accelz_scale_group8) );
      break;

    case SNS_REG_SCM_GROUP_GYRO_DYN_CAL_PARAMS_V01:
      sns_reg_get_default( SNS_REG_ITEM_GYRO_X_DYN_BIAS_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyrox_bias) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_Y_DYN_BIAS_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyroy_bias) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_Z_DYN_BIAS_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyroz_bias) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_X_DYN_SCALE_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyrox_scale) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_Y_DYN_SCALE_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyroy_scale) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_Z_DYN_SCALE_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyroz_scale) );
      break;

    case SNS_REG_SCM_GROUP_GYRO_FAC_CAL_PARAMS_V01:
      sns_reg_get_default( SNS_REG_ITEM_GYRO_X_BIAS_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyrox_bias) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_Y_BIAS_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyroy_bias) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_Z_BIAS_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyroz_bias) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_X_SCALE_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyrox_scale) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_Y_SCALE_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyroy_scale) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_Z_SCALE_V01,
          &(((sns_reg_gyro_data_group_s*)data_ptr)->gyroz_scale) );
       break;
    case SNS_REG_GROUP_SENSOR_TEST_V02:
      sns_reg_get_default( SNS_REG_ITEM_RAW_DATA_MODE_V02,
          &(((sns_reg_sensor_test_data_group_s*)data_ptr)->raw_data_mode) );
      sns_reg_get_default( SNS_REG_ITEM_TEST_EN_FLAGS_V02,
          &(((sns_reg_sensor_test_data_group_s*)data_ptr)->test_en_flags) );
      break;
    case SNS_REG_DRIVER_GROUP_ACCEL_V02:
    {
      sns_reg_acc_driver_group_s* reg_group = data_ptr;
      sns_reg_get_default( SNS_REG_DRIVER_ACCEL_X_ORIENT_V02, &reg_group->accelx_orient );
      sns_reg_get_default( SNS_REG_DRIVER_ACCEL_Y_ORIENT_V02, &reg_group->accely_orient );
      sns_reg_get_default( SNS_REG_DRIVER_ACCEL_Z_ORIENT_V02, &reg_group->accelz_orient );
      break;
    }
    case SNS_REG_DRIVER_GROUP_ACCEL_2_V02:
    {
      sns_reg_acc_driver_group_s* reg_group = data_ptr;
      sns_reg_get_default( SNS_REG_DRIVER_ACCEL2_X_ORIENT_V02, &reg_group->accelx_orient );
      sns_reg_get_default( SNS_REG_DRIVER_ACCEL2_Y_ORIENT_V02, &reg_group->accely_orient );
      sns_reg_get_default( SNS_REG_DRIVER_ACCEL2_Z_ORIENT_V02, &reg_group->accelz_orient );
      break;
    }
    case SNS_REG_DRIVER_GROUP_GYRO_V02:
    {
      sns_reg_gyro_driver_group_s* reg_group = data_ptr;
      sns_reg_get_default( SNS_REG_DRIVER_GYRO_X_ORIENT_V02, &reg_group->gyrox_orient );
      sns_reg_get_default( SNS_REG_DRIVER_GYRO_Y_ORIENT_V02, &reg_group->gyroy_orient );
      sns_reg_get_default( SNS_REG_DRIVER_GYRO_Z_ORIENT_V02, &reg_group->gyroz_orient );
      break;
    }
    case SNS_REG_DRIVER_GROUP_MAG_V02:
    {
      sns_reg_mag_driver_group_s* reg_group = data_ptr;
      sns_reg_get_default( SNS_REG_DRIVER_MAG_X_ORIENT_V02, &reg_group->magx_orient );
      sns_reg_get_default( SNS_REG_DRIVER_MAG_Y_ORIENT_V02, &reg_group->magy_orient );
      sns_reg_get_default( SNS_REG_DRIVER_MAG_Z_ORIENT_V02, &reg_group->magz_orient );
      break;
    }
    case SNS_REG_SAM_GROUP_FMV_PARAMS_V02:
    {
      sns_reg_get_default( SNS_REG_ITEM_FMV_TC_ACCURACY_0_V02,
         &(((sns_reg_fmv_data_group_s*)data_ptr)->accuracies[0]) );
      sns_reg_get_default( SNS_REG_ITEM_FMV_TC_ACCURACY_1_V02,
         &(((sns_reg_fmv_data_group_s*)data_ptr)->accuracies[1]) );
      sns_reg_get_default( SNS_REG_ITEM_FMV_TC_ACCURACY_2_V02,
         &(((sns_reg_fmv_data_group_s*)data_ptr)->accuracies[2]) );
      sns_reg_get_default( SNS_REG_ITEM_FMV_TC_ACCURACY_3_V02,
         &(((sns_reg_fmv_data_group_s*)data_ptr)->accuracies[3]) );
      sns_reg_get_default( SNS_REG_ITEM_FMV_GYRO_GAP_THRESH_V02,
         &(((sns_reg_fmv_data_group_s*)data_ptr)->gyro_gap_thresh) );
      sns_reg_get_default( SNS_REG_ITEM_FMV_MAG_GAP_FACTOR_V02,
         &(((sns_reg_fmv_data_group_s*)data_ptr)->mag_gap_factor) );
      sns_reg_get_default( SNS_REG_ITEM_FMV_ROT_FOR_ZUPT_V02,
         &(((sns_reg_fmv_data_group_s*)data_ptr)->gyro_thresh_for_zupt) );
      sns_reg_get_default( SNS_REG_ITEM_FMV_MAX_MAG_INNOVATION_V02,
         &(((sns_reg_fmv_data_group_s*)data_ptr)->max_mag_innovation) );
      sns_reg_get_default( SNS_REG_ITEM_FMV_DEF_SENSOR_REPORT_RATE_V02,
         &(((sns_reg_fmv_data_group_s*)data_ptr)->sensor_report_rate) );
      break;
    }
    case SNS_REG_SAM_GROUP_GRAVITY_VECTOR_PARAMS_V02:
    {
      sns_reg_get_default( SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM1_V02,
         &(((sns_reg_gravity_data_group_s*)data_ptr)->int_cfg_arr1[0]) );
      sns_reg_get_default( SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM2_V02,
         &(((sns_reg_gravity_data_group_s*)data_ptr)->int_cfg_arr1[1]) );
      sns_reg_get_default( SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM3_V02,
         &(((sns_reg_gravity_data_group_s*)data_ptr)->int_cfg_arr1[2]) );
      sns_reg_get_default( SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM4_V02,
         &(((sns_reg_gravity_data_group_s*)data_ptr)->int_cfg_arr1[3]) );
      sns_reg_get_default( SNS_REG_ITEM_GRAVITY_VECTOR_INT_CFG_PARAM5_V02,
         &(((sns_reg_gravity_data_group_s*)data_ptr)->int_cfg_param1) );
      sns_reg_get_default( SNS_REG_ITEM_GRAVITY_VECTOR_DEF_SENSOR_REPORT_RATE_V02,
         &(((sns_reg_gravity_data_group_s*)data_ptr)->sensor_report_rate) );
      break;
    }
    case SNS_REG_SAM_GROUP_ORIENTATION_PARAMS_V02:
    {
      sns_reg_get_default( SNS_REG_ITEM_ORIENTATION_INT_CFG_PARAM1_V02,
         &(((sns_reg_orientation_data_group_s*)data_ptr)->int_cfg_param1) );
      sns_reg_get_default( SNS_REG_ITEM_MAG_CAL_LAT_NUM_SAMPLES_V02,
         &(((sns_reg_orientation_data_group_s*)data_ptr)->mag_cal_lat_num_samps) );
      sns_reg_get_default( SNS_REG_ITEM_FUSION_MIN_SAMP_RATE_HZ_V02,
         &(((sns_reg_orientation_data_group_s*)data_ptr)->fusion_min_samp_rate_hz) );
      sns_reg_get_default( SNS_REG_ITEM_ORIENTATION_DEF_SENSOR_REPORT_RATE_V02,
         &(((sns_reg_orientation_data_group_s*)data_ptr)->sensor_report_rate) );
      break;
    }
    case SNS_REG_SAM_GROUP_GYRO_AMD_V02:
    {
      sns_reg_get_default( SNS_REG_ITEM_GYRO_AMD_INT_CFG_PARAM1_V02,
         &(((sns_reg_gyro_amd_data_group_s*)data_ptr)->int_cfg_param1) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_AMD_INT_CFG_PARAM2_V02,
         &(((sns_reg_gyro_amd_data_group_s*)data_ptr)->int_cfg_param2) );
      sns_reg_get_default( SNS_REG_ITEM_GYRO_AMD_INT_CFG_PARAM3_V02,
         &(((sns_reg_gyro_amd_data_group_s*)data_ptr)->int_cfg_param3) );
      break;
    }
    case SNS_REG_SAM_GROUP_GYRO_TAP_V02:
    {
      sns_reg_gyro_tap_group_s *reg_group  = data_ptr;
      sns_reg_get_default( SNS_REG_GYRO_TAP_VERSION_V02,    &reg_group->gyro_tap_version);
      sns_reg_get_default( SNS_REG_GYRO_TAP_SCENARIO_0_V02, &reg_group->scn[0].gyro_tap_scenario);
      sns_reg_get_default( SNS_REG_GYRO_TAP_SCENARIO_1_V02, &reg_group->scn[1].gyro_tap_scenario);
      sns_reg_get_default( SNS_REG_GYRO_TAP_SCENARIO_2_V02, &reg_group->scn[2].gyro_tap_scenario);
      break;
    }
    case SNS_REG_GROUP_SETTINGS_V02:
    {
      sns_reg_settings_group_s *reg_group  = data_ptr;
      sns_reg_get_default( SNS_REG_SETTINGS_MG_WORD_V02,       &reg_group->magic_word);
      sns_reg_get_default( SNS_REG_SETTINGS_VERSION_MINOR_V02, &reg_group->version_minor);
      sns_reg_get_default( SNS_REG_SETTINGS_VERSION_MAJOR_V02, &reg_group->version_major);
      sns_reg_get_default( SNS_REG_SETTINGS_FILE1_NAME_V02, &reg_group->conf[0].name);
      sns_reg_get_default( SNS_REG_SETTINGS_FILE1_VERSION_V02, &reg_group->conf[0].version);
      sns_reg_get_default( SNS_REG_SETTINGS_FILE2_NAME_V02, &reg_group->conf[1].name);
      sns_reg_get_default( SNS_REG_SETTINGS_FILE2_VERSION_V02, &reg_group->conf[1].version);
      sns_reg_get_default( SNS_REG_SETTINGS_FILE3_NAME_V02, &reg_group->conf[2].name);
      sns_reg_get_default( SNS_REG_SETTINGS_FILE3_VERSION_V02, &reg_group->conf[1].version);
      break;
    }

   case SNS_REG_SCM_GROUP_ACCEL_CAL_ALGO_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_ACCEL_CAL_DEF_SAMP_RATE_V02,
        &(((sns_reg_accel_cal_algo_data_group_s*)data_ptr)->def_accel_sample_rate) );
      sns_reg_get_default( SNS_REG_ITEM_ACCEL_CAL_DEF_ENABLE_ALGO_V02,
         &(((sns_reg_accel_cal_algo_data_group_s*)data_ptr)->def_accel_cal_algo_state) );
      break;
   }
   case SNS_REG_SAM_GROUP_PED_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_PED_DEF_STEP_THRESHOLD_V02,
        &(((sns_reg_ped_data_group_s*)data_ptr)->step_threshold));
      sns_reg_get_default( SNS_REG_ITEM_PED_DEF_SWING_THRESHOLD_V02,
        &(((sns_reg_ped_data_group_s*)data_ptr)->swing_threshold));
      sns_reg_get_default( SNS_REG_ITEM_PED_DEF_STEP_PROB_THRESHOLD_V02,
        &(((sns_reg_ped_data_group_s*)data_ptr)->step_prob_threshold));
      sns_reg_get_default( SNS_REG_ITEM_PED_DEF_SENSOR_REPORT_RATE_V02,
        &(((sns_reg_ped_data_group_s*)data_ptr)->sensor_report_rate));
      break;
   }

    case SNS_REG_SAM_GROUP_PAM_V02:
    {
      sns_reg_get_default( SNS_REG_ITEM_PAM_INT_CFG_PARAM1_V02,
         &(((sns_reg_pam_data_group_s*)data_ptr)->int_cfg_param1) );
      sns_reg_get_default( SNS_REG_ITEM_PAM_INT_CFG_PARAM2_V02,
         &(((sns_reg_pam_data_group_s*)data_ptr)->int_cfg_param2) );
      sns_reg_get_default( SNS_REG_ITEM_PAM_INT_CFG_PARAM3_V02,
         &(((sns_reg_pam_data_group_s*)data_ptr)->int_cfg_param3) );
      sns_reg_get_default( SNS_REG_ITEM_PAM_INT_CFG_PARAM4_V02,
         &(((sns_reg_pam_data_group_s*)data_ptr)->int_cfg_param4) );
      break;
    }

   case SNS_REG_SAM_GROUP_BASIC_GESTURES_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_BASIC_GESTURES_DEF_SENSOR_REPORT_RATE_V02,
        &(((sns_reg_basic_gestures_data_group_s*)data_ptr)->sensor_report_rate));
      break;
   }

   case SNS_REG_SAM_GROUP_FACING_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_FACING_DEF_SENSOR_REPORT_RATE_V02,
        &(((sns_reg_facing_data_group_s*)data_ptr)->sensor_report_rate));
      break;
   }

   case SNS_REG_SAM_GROUP_QUATERNION_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_QUATERNION_DEF_SENSOR_REPORT_RATE_V02,
        &(((sns_reg_quaternion_data_group_s*)data_ptr)->sensor_report_rate));
      break;
   }

   case SNS_REG_SAM_GROUP_ROTATION_VECTOR_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_ROTATION_VECTOR_DEF_SENSOR_REPORT_RATE_V02,
        &(((sns_reg_rotation_vector_data_group_s*)data_ptr)->sensor_report_rate));
      break;
   }

   case SNS_REG_SAM_GROUP_DISTANCE_BOUND_V02:
   {
     sns_reg_get_default( SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_UNKNOWN_V02,
        &(((sns_reg_distance_bound_data_group_s*)data_ptr)->speedbounds_unknown) );
     sns_reg_get_default( SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_STATIONARY_V02,
        &(((sns_reg_distance_bound_data_group_s*)data_ptr)->speedbounds_stationary) );
     sns_reg_get_default( SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_INMOTION_V02,
        &(((sns_reg_distance_bound_data_group_s*)data_ptr)->speedbounds_inmotion) );
     sns_reg_get_default( SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_FIDDLE_V02,
        &(((sns_reg_distance_bound_data_group_s*)data_ptr)->speedbounds_fiddle) );
     sns_reg_get_default( SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_PEDESTRIAN_V02,
        &(((sns_reg_distance_bound_data_group_s*)data_ptr)->speedbounds_pedestrian) );
     sns_reg_get_default( SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_VEHICLE_V02,
        &(((sns_reg_distance_bound_data_group_s*)data_ptr)->speedbounds_vehicle) );
     sns_reg_get_default( SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_WALK_V02,
        &(((sns_reg_distance_bound_data_group_s*)data_ptr)->speedbounds_walk) );
     sns_reg_get_default( SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_RUN_V02,
        &(((sns_reg_distance_bound_data_group_s*)data_ptr)->speedbounds_run) );
     sns_reg_get_default( SNS_REG_ITEM_DISTANCE_BOUND_SPEEDBOUNDS_BIKE_V02,
        &(((sns_reg_distance_bound_data_group_s*)data_ptr)->speedbounds_bike) );
     break;
   }

   case SNS_REG_SAM_GROUP_CMC_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_CMC_DEF_SENSOR_REPORT_RATE_V02,
        &(((sns_reg_cmc_data_group_s*)data_ptr)->sensor_report_rate));
      break;
   }

   case SNS_REG_SCM_GROUP_QMAG_CAL_ALGO_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_VERSION_V02,
        &(((sns_reg_qmag_cal_algo_data_group_s*)data_ptr)->version_no));
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_ENABLE_ALGO_V02,
        &(((sns_reg_qmag_cal_algo_data_group_s*)data_ptr)->enable_algo));
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_ENABLE_SI_CAL_V02,
        &(((sns_reg_qmag_cal_algo_data_group_s*)data_ptr)->enable_si_cal));
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_ENABLE_HI_CAL_V02,
        &(((sns_reg_qmag_cal_algo_data_group_s*)data_ptr)->enable_hi_cal));
      sns_reg_get_default( SNS_REG_ITEM_SAMPLE_RATE_V02,
        &(((sns_reg_qmag_cal_algo_data_group_s*)data_ptr)->sample_rate));
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_REPORT_RATE_V02,
        &(((sns_reg_qmag_cal_algo_data_group_s*)data_ptr)->report_rate));
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_ENABLE_GYRO_ASSIST_V02,
        &(((sns_reg_qmag_cal_algo_data_group_s*)data_ptr)->enable_gyro_assist));
      break;
   }

   case SNS_REG_SCM_GROUP_MAG_DYN_CAL_PARAMS_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_CAL_VERSION_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->version_no));
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_CAL_BIAS_VALID_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->bias_valid));
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_CAL_CALIBRATION_MATRIX_VALID_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->cal_mat_valid));
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_0_0_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->compensation_matrix[0][0]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_0_1_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->compensation_matrix[0][1]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_0_2_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->compensation_matrix[0][2]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_1_0_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->compensation_matrix[1][0]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_1_1_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->compensation_matrix[1][1]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_1_2_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->compensation_matrix[1][2]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_2_0_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->compensation_matrix[2][0]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_2_1_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->compensation_matrix[2][1]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_DYN_COMPENSATION_MATRIX_2_2_V02,
        &(((sns_reg_mag_dyn_cal_params_data_group_s*)data_ptr)->compensation_matrix[2][2]));
      break;
   }

   case SNS_REG_SAM_GROUP_GAME_ROTATION_VECTOR_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_GAME_ROT_VEC_DEF_SENSOR_REPORT_RATE_V02,
        &(((sns_reg_game_rot_vec_data_group_s*)data_ptr)->sensor_report_rate));
      break;
   }

   case SNS_REG_SCM_GROUP_QMAG_CAL_STATE_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_STATE_VERSION_V02,
        &(((sns_reg_qmag_cal_state_data_group_s*)data_ptr)->version_no));
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_X_HI_V02,
        &(((sns_reg_qmag_cal_state_data_group_s*)data_ptr)->published_offs_HI_x));
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_Y_HI_V02,
        &(((sns_reg_qmag_cal_state_data_group_s*)data_ptr)->published_offs_HI_y));
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_Z_HI_V02,
        &(((sns_reg_qmag_cal_state_data_group_s*)data_ptr)->published_offs_HI_z));
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_ACCURACY_HI_V02,
        &(((sns_reg_qmag_cal_state_data_group_s*)data_ptr)->published_accuracy_HI));
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_TIME_V02,
        &(((sns_reg_qmag_cal_state_data_group_s*)data_ptr)->published_offset_time));
      sns_reg_get_default( SNS_REG_ITEM_QMAG_CAL_STATE_PUBLISH_OFFSET_VALID_V02,
        &(((sns_reg_qmag_cal_state_data_group_s*)data_ptr)->published_offset_valid));
      break;
   }

   case SNS_REG_SAM_GROUP_SMD_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_SMD_ACC_NORM_SDEV_THRESH_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->acc_norm_std_dev_thresh));
      sns_reg_get_default( SNS_REG_ITEM_SMD_VAR_DEC_LAT_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->variable_decision_latency));
      sns_reg_get_default( SNS_REG_ITEM_SMD_MAX_LAT_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->max_latency));
      sns_reg_get_default( SNS_REG_ITEM_SMD_ACC_WIN_TIME_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->accel_window_time));
      sns_reg_get_default( SNS_REG_ITEM_SMD_LACC_WIN_TIME_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->long_accel_window_time));
      sns_reg_get_default( SNS_REG_ITEM_SMD_STRANS_PROB_SM_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->self_transition_prob_sm));
      sns_reg_get_default( SNS_REG_ITEM_SMD_DETECT_THRESH_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->detect_threshold));
      sns_reg_get_default( SNS_REG_ITEM_SMD_EIGEN_THRESH_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->eigen_thr));
      sns_reg_get_default( SNS_REG_ITEM_SMD_PCOUNT_THRESH_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->pedometer_stepcount_thr));
      sns_reg_get_default( SNS_REG_ITEM_SMD_PTIME_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->pedometer_time));
      sns_reg_get_default( SNS_REG_ITEM_SMD_DELTA_RESET_TIME_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->delta_reset_time));
      sns_reg_get_default( SNS_REG_ITEM_SMD_SENSOR_REPORT_RATE_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->sensor_report_rate));
      sns_reg_get_default( SNS_REG_ITEM_SMD_VERSION_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->version));
      sns_reg_get_default( SNS_REG_ITEM_SMD_SAMP_RATE_V02,
        &(((sns_reg_smd_data_group_s*)data_ptr)->sample_rate));
      break;
   }

   case SNS_REG_SCM_GROUP_MAG_FAC_CAL_PARAMS_V02:
   {
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_CAL_VERSION_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->version_no));
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_CAL_BIAS_VALID_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->bias_valid));
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_CAL_CALIBRATION_MATRIX_VALID_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->cal_mat_valid));
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_0_0_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->compensation_matrix[0][0]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_0_1_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->compensation_matrix[0][1]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_0_2_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->compensation_matrix[0][2]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_1_0_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->compensation_matrix[1][0]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_1_1_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->compensation_matrix[1][1]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_1_2_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->compensation_matrix[1][2]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_2_0_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->compensation_matrix[2][0]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_2_1_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->compensation_matrix[2][1]));
      sns_reg_get_default( SNS_REG_ITEM_MAG_FAC_COMPENSATION_MATRIX_2_2_V02,
        &(((sns_reg_mag_fac_cal_params_data_group_s*)data_ptr)->compensation_matrix[2][2]));
      break;
   }

    case SNS_REG_DRIVER_GROUP_IR_GESTURE_V02:
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS1_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds1) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS2_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds2) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS3_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds3) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS4_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds4) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS5_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds5) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS6_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds6) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS7_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds7) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS8_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds8) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS9_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds9) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS10_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds10) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS11_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds11) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS12_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds12) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS13_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds13) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS14_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds14) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS15_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds15) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS16_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds16) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS17_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds17) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS18_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds18) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS19_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds19) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS20_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds20) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS21_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds21) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS22_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds22) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS23_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds23) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS24_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds24) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS25_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds25) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS26_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds26) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS27_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds27) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS28_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds28) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS29_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds29) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS30_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds30) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS31_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds31) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS32_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds32) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS33_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds33) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS34_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds34) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS35_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds35) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS36_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds36) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS37_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds37) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS38_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds38) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS39_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds39) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS40_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds40) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS41_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds41) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS42_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds42) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS43_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds43) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS44_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds44) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS45_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds45) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS46_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds46) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS47_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds47) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS48_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds48) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS49_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds49) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS50_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds50) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS51_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds51) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS52_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds52) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS53_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds53) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS54_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds54) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS55_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds55) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS56_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds56) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS57_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds57) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS58_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds58) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS59_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds59) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS60_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds60) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS61_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds61) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS62_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds62) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS63_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds63) );
      sns_reg_get_default( SNS_REG_ITEM_IR_GESTURE_DS64_V02,
        &(((sns_reg_ir_ges_data_group_s*)data_ptr)->ds64) );
      break;

   /**
    * No good defaults for SAR sensors, since they are highly dependent on the
    * electrode configuration. Just leave the entire SAR group blank.
    */
   case SNS_REG_DRIVER_GROUP_SAR_V02:
   case SNS_REG_DRIVER_GROUP_SAR_2_V02:
      break;

   case SNS_REG_DRIVER_GROUP_RGB_V02:
   {
      sns_reg_get_default( SNS_REG_DRIVER_RGB_R_FACTOR_V02,
        &(((sns_reg_rgb_data_group_s*)data_ptr)->r_factor) );
      sns_reg_get_default( SNS_REG_DRIVER_RGB_G_FACTOR_V02,
        &(((sns_reg_rgb_data_group_s*)data_ptr)->g_factor) );
      sns_reg_get_default( SNS_REG_DRIVER_RGB_B_FACTOR_V02,
        &(((sns_reg_rgb_data_group_s*)data_ptr)->b_factor) );
      sns_reg_get_default( SNS_REG_DRIVER_RGB_C_FACTOR_V02,
        &(((sns_reg_rgb_data_group_s*)data_ptr)->c_factor) );
      sns_reg_get_default( SNS_REG_DRIVER_RGB_IR_FACTOR_V02,
        &(((sns_reg_rgb_data_group_s*)data_ptr)->ir_factor) );
      break;
   }

   case SNS_REG_DRIVER_GROUP_RGB_2_V02:
   {
      sns_reg_get_default( SNS_REG_DRIVER_RGB_2_R_FACTOR_V02,
        &(((sns_reg_rgb_data_group_s*)data_ptr)->r_factor) );
      sns_reg_get_default( SNS_REG_DRIVER_RGB_2_G_FACTOR_V02,
        &(((sns_reg_rgb_data_group_s*)data_ptr)->g_factor) );
      sns_reg_get_default( SNS_REG_DRIVER_RGB_2_B_FACTOR_V02,
        &(((sns_reg_rgb_data_group_s*)data_ptr)->b_factor) );
      sns_reg_get_default( SNS_REG_DRIVER_RGB_2_C_FACTOR_V02,
        &(((sns_reg_rgb_data_group_s*)data_ptr)->c_factor) );
      sns_reg_get_default( SNS_REG_DRIVER_RGB_2_IR_FACTOR_V02,
        &(((sns_reg_rgb_data_group_s*)data_ptr)->ir_factor) );
      break;
   }

   case SNS_REG_DRIVER_GROUP_GAME_RV_V02:
   {
      sns_reg_grv_driver_group_s* reg_group = data_ptr;
      sns_reg_get_default( SNS_REG_DRIVER_GAME_RV_X_ORIENT_V02, &reg_group->grv_x_orient );
      sns_reg_get_default( SNS_REG_DRIVER_GAME_RV_Y_ORIENT_V02, &reg_group->grv_y_orient );
      sns_reg_get_default( SNS_REG_DRIVER_GAME_RV_Z_ORIENT_V02, &reg_group->grv_z_orient );
      break;
   }

   case SNS_REG_SAM_GROUP_TILT_DETECTOR_V02:
   {
     sns_reg_tilt_detector_data_group_s* reg_group = data_ptr;
     sns_reg_get_default( SNS_REG_ITEM_TILT_DETECTOR_DEF_SENSOR_REPORT_RATE_V02,
         &reg_group->sensor_report_rate );
     sns_reg_get_default( SNS_REG_ITEM_TILT_DETECTOR_SAMPLE_RATE_V02,
         &reg_group->sensor_sample_rate );
     sns_reg_get_default( SNS_REG_ITEM_TILT_DETECTOR_INIT_ACCEL_WINDOW_TIME_V02,
         &reg_group->init_accel_window_time );
     sns_reg_get_default( SNS_REG_ITEM_TILT_DETECTOR_ACCEL_WINDOW_TIME_V02,
         &reg_group->accel_window_time );
     sns_reg_get_default( SNS_REG_ITEM_TILT_DETECTOR_DEF_TILT_ANGLE_THRESH_V02,
         &reg_group->def_tilt_angle_thresh );
     break;
   }

   case SNS_REG_GROUP_SSI_GPIO_CFG_V02:
   {
      sns_reg_ssi_gpio_cfg_group_s* reg_group = data_ptr;
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_VERSION_MAJOR_V02,
         &reg_group->maj_ver_no );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_VERSION_MINOR_V02,
         &reg_group->min_ver_no );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SDA_1_V02,
         &reg_group->i2c_sda_1 );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SCL_1_V02,
         &reg_group->i2c_scl_1 );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SDA_2_V02,
         &reg_group->i2c_sda_2 );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_I2C_SCL_2_V02,
         &reg_group->i2c_scl_2 );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_RESET_V02,
         &reg_group->sns_reset );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_TEST_V02,
         &reg_group->sns_test );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_ACCEL_MD_INT_V02,
         &reg_group->sns_accel_md );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_ACCEL_DRDY_INT_V02,
         &reg_group->sns_accel_dri );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_GYRO_DRDY_INT_V02,
         &reg_group->sns_gyro_dri );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_MAG_DRDY_INT_V02,
         &reg_group->sns_mag_dri );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_ALSP_INT_V02,
         &reg_group->sns_alsp_int );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_GEST_INT_V02,
         &reg_group->sns_gest_int );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_PRESS_INT_V02,
         &reg_group->sns_press_int );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_SAR_INT_V02,
         &reg_group->sns_sar_int );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_FP_INT_V02,
         &reg_group->sns_fp_int );
      sns_reg_get_default( SNS_REG_ITEM_SSI_GPIO_CFG_HALL_INT_V02,
         &reg_group->sns_hall_int );
      break;
   }

   default:
   {
      SNS_PRINTF_STRING_LOW_1( SNS_MODULE_APPS_REG, "Invalid group_id: %d",
                               group_id );
      return SNS_ERR_FAILED;
   }
  }
  return SNS_SUCCESS;
}


/*===========================================================================

  FUNCTION:   sns_reg_get_item_count

  ===========================================================================*/
/*!
  @brief Retrieves the number of data items defined

  @param None

  @return
  Number of entries in sns_reg_item_info[]
*/
/*=========================================================================*/
uint16_t sns_reg_get_item_count(void)
{
  return ARR_SIZE(sns_reg_item_info);
}

/*===========================================================================

  FUNCTION:   sns_reg_get_group_count

  ===========================================================================*/
/*!
  @brief Retrieves the number of group items defined

  @param None

  @return
  Number of entries in sns_reg_group_info[]
*/
/*=========================================================================*/
uint16_t sns_reg_get_group_count(void)
{
  return ARR_SIZE(sns_reg_group_info);
}

