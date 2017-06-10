/*============================================================================
  @file sns_sam_algo.c

  @brief
  This here is the only file that may be changes while adding new SAM
  algorithms into the SAM Framework.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include "sns_sam.h"
#include "sns_sam_algo_api.h"

/*============================================================================
  Algorithm Registration Function Declarations
  ===========================================================================*/

sns_sam_err egs_sam_register(
      sns_sam_algo_api **algoAPI,
      sns_sam_algo_msg_api **msgAPI,
      qmi_idl_service_object_type *serviceObj,
      sns_sam_sensor_uid *sensorUID );

/*============================================================================
  Global Data
  ===========================================================================*/

const sns_sam_algo_register samAlgoRegisterFuncs[] =
{
};

const uint32_t samAlgoRegisterFuncsSize =
  sizeof(samAlgoRegisterFuncs) / sizeof(samAlgoRegisterFuncs[0]);


const sns_sam_sensor_uid samUImageAlgoSUIDs[] =
{
};

const uint32_t samUImageAlgoSUIDsSize =
  sizeof(samUImageAlgoSUIDs) / sizeof(samUImageAlgoSUIDs[0]);

