/*
 * MDM_IOCTLS: An ioctl function for Kickstart utility
 *
 * Copyright (C) 2011 Qualcomm Technologies, Inc. All rights reserved.
 *                    Qualcomm Technologies Proprietary/GTDR
 *
 * All data and information contained in or disclosed by this document is
 * confidential and proprietary information of Qualcomm Technologies, Inc. and all
 * rights therein are expressly reserved.  By accepting this material the
 * recipient agrees that this material and the information contained therein
 * is held in confidence and in trust and will not be used, copied, reproduced
 * in whole or in part, nor its contents revealed in any manner to others
 * without the express written permission of Qualcomm Technologies, Inc.
 *
 *
 * MDM_IOCTLS: An ioctl function for Kickstart utility
 * ==========================================================================================
 *   $Header: //source/qcom/qct/core/storage/tools/kickstart/mdm_ioctls.h#1 $
 *   $DateTime: 2011/01/12 11:30:11 $
 *   $Author: ahughes $
 *
 *  Edit History:
 *  YYYY-MM-DD          who             why
 *  -----------------------------------------------------------------------------
 *  2011-01-12          ah      Initial creation
 *
 *  Copyright 2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
 *
 *==========================================================================================
 */


#ifndef MDM_IOCTLS_H
#define MDM_IOCTLS_H


#define CHARM_CODE      0xCC
#define WAKE_CHARM      _IO(CHARM_CODE, 1)
#define RESET_CHARM     _IO(CHARM_CODE, 2)
#define CHECK_FOR_BOOT  _IOR(CHARM_CODE, 3, int)
#define WAIT_FOR_BOOT   _IO(CHARM_CODE, 4)

#endif
