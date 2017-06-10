/******************************************************************************
  @file    ds_profile_os.h
  @brief   Operating System specific header

  DESCRIPTION
  This header defines API for OS specific logging, locking mechanisms. 

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  ---------------------------------------------------------------------------
  Copyright (C) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/data/common/dsprofile/main/latest/inc/ds_profile_os.h#2 $ $DateTime: 2009/10/23 14:44:55 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#ifndef DS_PROFILE_OS_H
#define DS_PROFILE_OS_H

#ifdef FEATURE_DATA_DS_PROFILE_LINUX
  #include "ds_profile_os_linux.h"
#else
#ifdef FEATURE_DATA_DS_PROFILE_AMSS
  #include "customer.h"
  #include "ds_profile_os_amss.h"
#endif
#endif

#endif /* DS_PROFILE_OS_H */
