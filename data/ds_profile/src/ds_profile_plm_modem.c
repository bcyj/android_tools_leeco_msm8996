/******************************************************************************
  @file    ds_profile_plm_modem.c
  @brief   

  DESCRIPTION
  This file implements the modem (AMSS) specific routines

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  ---------------------------------------------------------------------------
  Copyright (C) 2008 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/data/common/dsprofile/main/latest/src/ds_profile_plm_modem.c#1 $ $DateTime: 2009/09/30 19:03:37 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#include "ds_profile_plm.h"

/*---------------------------------------------------------------------------
                     PUBLIC FUNCTION IMPLEMENTATIONS 
---------------------------------------------------------------------------*/
#ifdef FEATURE_DATA_DS_PROFILE_3GPP2
#include "ds_profile_3gpp2i.h"
extern uint8 ds_profile_3gpp2_qmi_init ( tech_fntbl_type *fntbl );
#endif

#ifdef FEATURE_DATA_DS_PROFILE_3GPP
#include "ds_profile_3gppi.h"
extern uint8 ds_profile_3gpp_qmi_init ( tech_fntbl_type *fntbl );
#endif

/*===========================================================================
FUNCTION PLM_TECH_OPS_INIT

DESCRIPTION
  This function calls the tech init function for operations module

PARAMETERS
  store :  pointer to table for all techs, having table of function
           pointers to be initialized in the tech specific init functions
DEPENDENCIES 
  
RETURN VALUE 
  mask : ORed value of tech masks 
SIDE EFFECTS 
 
===========================================================================*/
uint8 plm_tech_ops_init( 
  plm_store_type *store 
)
{
  uint8 mask = 0;

#ifdef FEATURE_DATA_DS_PROFILE_3GPP
  mask |= ds_profile_3gpp_init( &(store[DS_PROFILE_TECH_3GPP].vtbl) ); 
#endif

#ifdef FEATURE_DATA_DS_PROFILE_3GPP2
  mask |= ds_profile_3gpp2_init ( &(store[DS_PROFILE_TECH_3GPP2].vtbl) );
#endif

  return mask;
}

/*===========================================================================
FUNCTION PLM_TECH_ACCESS_INIT

DESCRIPTION
  This function calls the tech init function for access module

PARAMETERS
  store :  pointer to table for all techs, having table of function
           pointers to be initialized in the tech specific init functions
DEPENDENCIES 
  
RETURN VALUE 
  mask : ORed value of tech masks 
SIDE EFFECTS 
 
===========================================================================*/
uint8 plm_tech_access_init( 
  plm_store_type *store 
)
{
  uint8 mask = 0;

#ifdef FEATURE_DATA_DS_PROFILE_3GPP
  mask |= ds_profile_3gpp_qmi_init( &(store[DS_PROFILE_TECH_3GPP].vtbl) );
#endif

#ifdef FEATURE_DATA_DS_PROFILE_3GPP2
  mask |= ds_profile_3gpp2_qmi_init ( &(store[DS_PROFILE_TECH_3GPP2].vtbl) );
#endif

  return mask;
}


