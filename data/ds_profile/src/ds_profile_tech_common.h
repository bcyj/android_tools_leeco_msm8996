/******************************************************************************
  @file    ds_profile_tech_common.h
  @brief   

  DESCRIPTION
  

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

  $Header: //source/qcom/qct/modem/data/common/dsprofile/main/latest/inc/ds_profile_tech_common.h#2 $ $DateTime: 2009/10/23 14:44:55 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/
#ifndef DS_PROFILE_TECH_COMMON_H
#define DS_PROFILE_TECH_COMMON_H

#include "ds_profile.h"
#include "ds_util.h"
#include "customer.h"
/*---------------------------------------------------------------------------
                           DECLARATIONS
---------------------------------------------------------------------------*/
#define DSI_SUCCESS 0
#define DSI_FAILURE 1

/* Macro to convert identifier to mask */
#define CONVERT_IDENT_TO_MASK( a, ident ) { \
  a = 1 << ident; \
}

/*--------------------------------------------------------------------------- 
   Each tech should have concrete implementations of below virtual
   functions and return the structure as part of tech init
---------------------------------------------------------------------------*/
typedef struct
{
  /* 
	 Create Profile
   	 num - output parameter, profile number
  */
  ds_profile_status_etype (*create)(
    ds_profile_num_type  *num          
  );

  /* 
	 Delete a Profile
   	 num - profile number to be deleted
  */
  ds_profile_status_etype (*del)(
    ds_profile_num_type  num 
  );

  /* 
	 Allocate blob memory on 
   	 begin_transaction
  */
  void * (*alloc)();

  /* 
   	 Dealloc blob memory on	end_transaction
   	 ptr - input parameter, ptr to blob
  */
  int (*dealloc)(
    void *ptr
  );

  /* 
   	 Set identified params in the blob
   	 blob - input parameter, pointer to profile blob
   	 ident - input parameter, identifier for parameter to be set
     info - input parameter, value to be set
  */
  ds_profile_status_etype (*set_param)( 
    void                        *blob,
    ds_profile_identifier_type   ident,
    const ds_profile_info_type  *info
  );

  /* 
   	 Get identified params from the blob
	 blob - input parameter, pointer to profile blob
   	 ident - input parameter, identifier for parameter to be fetched
     info - output parameter, to store value of parameter fetched
  */
  ds_profile_status_etype (*get_param)( 
    void                        *blob,
    ds_profile_identifier_type   ident,
    ds_profile_info_type        *info
  );

  /* 
   	 Read profile into memory
   	 num - input parameter, profile to be read
   	 blob - output parameter, to store profile 
  */
  ds_profile_status_etype (*profile_read)(
    ds_profile_num_type   num,
    void                 *blob
  );

  /* 
   	 Write profile to persistant storage
   	 num - input parameter, profile to be written
   	 blob - input parameter
  */
  ds_profile_status_etype (*profile_write)(
    ds_profile_num_type   num,
    void                 *blob
  );

  /* 
   	 Reset value of param to default
  */
  ds_profile_status_etype (*reset_param)(
    ds_profile_num_type         num,
    ds_profile_identifier_type  ident
  );

  /* 
   	 Reset all parameters to default values
  */
  ds_profile_status_etype (*reset_profile_to_default)(  
    ds_profile_num_type    num
  );

  /* Set a profile as default */
  ds_profile_status_etype (*set_default_profile)(  
    uint32                 family, 
    ds_profile_num_type    num
  );

  /* Get the default profile number */
  ds_profile_status_etype (*get_default_profile)(
    uint32                 family, 
    ds_profile_num_type   *num
  );

  /* Validate profile number */
  ds_profile_status_etype (*validate_profile_num)(
    ds_profile_num_type num
  );

  /* Get profiles number range */
  void ( *get_num_range)(
    uint16 *min, 
    uint16 *max
  );

  /* Get list of profiles */
  ds_profile_status_etype (*get_list)(
    ds_util_list_hndl_type hndl,
    ds_profile_list_type  *lst
  );

  ds_profile_status_etype (*get_list_node)(
    ds_util_itr_hndl_type  hndl,
    ds_profile_list_info_type  *list_info
  );

}tech_fntbl_type;


/* get / set function pointers */
typedef struct 
{
  ds_profile_identifier_type  ident;

  ds_profile_status_etype (*set_fn)(
    void                  *blob,
    uint32               mask,
    const ds_profile_info_type  *info    /* input parameter */
  );

  ds_profile_status_etype (*get_fn)(
    const void            *blob,
    ds_profile_info_type  *info    /* output parameter */
  );
}dsi_profile_acc_mut_fn_type;

/* Identifier description */
typedef struct
{
  ds_profile_identifier_type  uid;  /* unique id to identify Profile Param */
  uint16  len;  /* length of identified Profile Param  */
}dsi_profile_params_desc_type;

#endif /* DS_PROFILE_TECH_COMMON_H */
