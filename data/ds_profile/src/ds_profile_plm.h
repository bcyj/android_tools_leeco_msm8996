/******************************************************************************
  @file    ds_profile_plm.h
  @brief   Platform specific header

  DESCRIPTION
  It exposes API to get a tech specific function table, and init function
  which calls tech specific init functions

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

  $Header: //source/qcom/qct/modem/data/common/dsprofile/main/latest/inc/ds_profile_plm.h#1 $ $DateTime: 2009/09/30 19:03:37 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#ifndef DS_PROFILE_PLM_H
#define DS_PROFILE_PLM_H

#include "ds_profile_tech_common.h"

/*===========================================================================
   Profile Operation Object, this is shared between PLM and DSI_INT module
===========================================================================*/
typedef struct
{

  tech_fntbl_type   vtbl;
  void             *priv;
  void             *self;  
} plm_store_type;

/*===========================================================================
   Init functions which internally call all tech inits
===========================================================================*/
uint8 plm_tech_ops_init( plm_store_type *tbl );
uint8 plm_tech_access_init( plm_store_type *store );

#endif /* DS_PROFILE_PLM_H */
