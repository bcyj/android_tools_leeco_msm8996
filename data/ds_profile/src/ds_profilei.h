/******************************************************************************
  @file    ds_profile_int.h
  @brief   DS PROFILE internal declarations

  DESCRIPTION
  Internal file for ds profile. Depending on tech calls tech specific
  functions

  All function signatures are techology independent. The routines invoke the 
  technology specific routines

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

  $Header: //source/qcom/qct/modem/data/common/dsprofile/main/latest/src/ds_profilei.h#2 $ $DateTime: 2009/10/23 14:44:55 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#ifndef DS_PROFILEI_H
#define DS_PROFILEI_H

#include "ds_profile.h"
#include "customer.h"

#define TECH_IS_VALID( tech ) \
  ( tech < DS_PROFILE_TECH_MAX )

/*---------------------------------------------------------------------------
                    DSI PROFILE module
---------------------------------------------------------------------------*/
/*===========================================================================
FUNCTION DSI_PROFILE_INIT

DESCRIPTION
  This function initializes the DS profile library. Calls operations and
  access module init functions for all techs

PARAMETERS

DEPENDENCIES 
  
RETURN VALUE 
  mask : ORed value of tech masks
SIDE EFFECTS 
 
===========================================================================*/
uint8 dsi_profile_init( 
  void 
);

/*===========================================================================
FUNCTION DSI_PROFILE_CLOSE_LIB

DESCRIPTION
  This function cleans up open handles if any.

PARAMETERS

DEPENDENCIES 
  
RETURN VALUE 
  
SIDE EFFECTS 
 
===========================================================================*/
void dsi_profile_close_lib (
  void
);

/*===========================================================================
FUNCTION DS_PROFILE_END_TRANSACTION

DESCRIPTION
  This is internal function which calls tech specific function to commits
  the prefetched modified profile to the persistent storage on the modem. 

PARAMETERS
 hndl  : profile handle

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS        : On successful operation
  DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED : Library is not initialized
  DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL : Invalid handle
  DS_PROFILE_REG_RESULT_FAIL           : On general errors.
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype dsi_profile_end_transaction (
  ds_profile_hndl_type hndl
);

/*===========================================================================
FUNCTION DSI_PROFILE_ALLOC_HNDL

DESCRIPTION
  This function calls tech specific function to allocate memory for local
  copy of profile
 

PARAMETERS
  trn  : transaction type requested
  tech : technology type
  num  : profile number
  hndl : pointer to store profile handle returned
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS  : On successful operation
  DS_PROFILE_REG_RESULT_FAIL     : On general errors. This return code 
                                   provides blanket coverage
SIDE EFFECTS 
===========================================================================*/
ds_profile_status_etype dsi_profile_alloc_hndl(
  ds_profile_trn_etype   trn,
  ds_profile_tech_etype  tech, 
  ds_profile_num_type    num,
  ds_profile_hndl_type  *hndl
);

/*===========================================================================
FUNCTION DSI_PROFILE_DEALLOC_HNDL

DESCRIPTION
  This function cleans up the memory used by the local copy of profile.
  Calls tech specific function to clean up
 
PARAMETERS
 hndl : pointer to handle
 
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS        : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL : Profile handle null or not for 
                                         write transaction
  DS_PROFILE_REG_RESULT_FAIL           : On general errors
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype dsi_profile_dealloc_hndl(
  ds_profile_hndl_type *hndl
);

/*===========================================================================
FUNCTION DS_PROFILE_GET_PARAM

DESCRIPTION
  This function is used to get profile data element identified by the 
  identifier from the local copy. Calls tech specific funtion

PARAMETERS
  profile_hndl  : handle to profile to set profile data elements
  identifier    : to identify profile data elements
  info          : pointer to value to which data element is to be set
                  (size of buffer passed can atmost be the max size of 
                  the parameter which needs to be set)
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS            : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL     : Invalid handle 
  DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT    : Invalid identifier
  DS_PROFILE_REG_RESULT_ERR_LEN_INVALID    : Buffer size more than expected
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Invalid tech type 
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM  : Invalid profile number
  DS_PROFILE_REG_RESULT_FAIL               : On general errors 
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype dsi_profile_get_param(
  ds_profile_hndl_type        hndl,
  ds_profile_identifier_type  identifier,
  ds_profile_info_type       *info
);

/*===========================================================================
FUNCTION DS_PROFILE_SET_PARAM

DESCRIPTION
  This function is used to set profile data element identified by the 
  identifier in the local copy. Calls tech specific function

PARAMETERS
  profile_hndl  : handle to profile to set profile data elements
  identifier    : to identify profile data elements
  info          : pointer to value to which data element is to be set
                  (size of buffer passed can atmost be the max size of 
                  the parameter which needs to be set)
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS            : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL     : Invalid handle 
  DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT    : Invalid identifier
  DS_PROFILE_REG_RESULT_ERR_LEN_INVALID    : Buffer size more than expected
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Invalid tech type 
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM  : Invalid profile number
  DS_PROFILE_REG_RESULT_FAIL               : On general errors 
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype dsi_profile_set_param(
  ds_profile_hndl_type        hndl,
  ds_profile_identifier_type  identifier,
  const ds_profile_info_type *info
);

/*===========================================================================
FUNCTION DSI_PROFILE_CREATE

DESCRIPTION
  This function calls tech specific create function

PARAMETERS
  tech  : technology type
  num   : pointer to return profile number of profile created

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS  : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Invalid tech type
  DS_PROFILE_REG_RESULT_ERR_INVAL_OP : Operation not supported for tech type
  DS_PROFILE_REG_RESULT_FAIL     : On general errors. This return code 
                                   provides blanket coverage
SIDE EFFECTS 
===========================================================================*/
ds_profile_status_etype dsi_profile_create( 
  ds_profile_tech_etype   tech,
  ds_profile_num_type    *num
);

/*===========================================================================
FUNCTION DSI_PROFILE_DELETE

DESCRIPTION
  This function calls tech specific delete function

PARAMETERS
  tech : technology type
  num  : profile number

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS  : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Invalid tech type
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM  : Invalid profile number
  DS_PROFILE_REG_RESULT_ERR_INVAL_OP : Operation not supported for tech type
  DS_PROFILE_REG_RESULT_FAIL     : On general errors. This return code 
                                   provides blanket coverage
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype dsi_profile_delete( 
  ds_profile_tech_etype  profile_type,
  ds_profile_num_type    profile_number 
);

/*===========================================================================
FUNCTION DSI_PROFILE_RESET_PARAM

DESCRIPTION
  This function calls internal tech specific function to reset parameter value

PARAMETERS
  tech  : technology type
  num   : profile number
  ident : to identify the profile parameter to be set to default

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS   : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Profile type is invalid
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM  : Invalid profile number
  DS_PROFILE_REG_RESULT_ERR_INVAL_OP : Operation not supported for tech type
  DS_PROFILE_REG_RESULT_FAIL      : On general errors. This return 
                                    code provides blanket coverage
SIDE EFFECTS 
  
===========================================================================*/
ds_profile_status_etype dsi_profile_reset_param (  
  ds_profile_tech_etype       tech,
  ds_profile_num_type         num,
  ds_profile_identifier_type  ident
);

/*===========================================================================
FUNCTION DSI_PROFILE_RESET_PROFILE_TO_DEFAULT

DESCRIPTION
  This function calls internal tech specific function to reset all the
  parameters of the profile to default. 

PARAMETERS
  tech  : technology type
  num   : profile number

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS   : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Profile type is invalid
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM  : Invalid profile number
  DS_PROFILE_REG_RESULT_ERR_INVAL_OP : Operation not supported for tech type
  DS_PROFILE_REG_RESULT_FAIL      : On general errors. This return 
                                    code provides blanket coverage
SIDE EFFECTS 
 
===========================================================================*/
ds_profile_status_etype dsi_profile_reset_profile_to_default (  
  ds_profile_tech_etype  tech,
  ds_profile_num_type    num
);

/*===========================================================================
FUNCTION DSI_PROFILE_SET_DEFAULT_PROFILE_NUM

DESCRIPTION
  This function calls internal tech specific function to set the given
  profile number as default profile

PARAMETERS
  tech   : technology type
  family : profile family
  num    : profile number

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS   : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Profile type is invalid
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM  : Invalid profile number
  DS_PROFILE_REG_RESULT_ERR_INVAL_OP : Operation not supported for tech type
  DS_PROFILE_REG_RESULT_FAIL      : On general errors. This return 
                                    code provides blanket coverage
SIDE EFFECTS 
 
===========================================================================*/
ds_profile_status_etype dsi_profile_set_default_profile (  
  ds_profile_tech_etype  tech,
  uint32                 family, 
  ds_profile_num_type    num
);

/*===========================================================================
FUNCTION DSI_PROFILE_GET_DEFAULT_PROFILE_NUM

DESCRIPTION
  This function calls internal tech specific function to get the default
  profile number for that family

PARAMETERS
  tech   : technology type
  family : profile family
  num    : pointer to store profile number

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS   : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Profile type is invalid
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM  : Invalid profile number
  DS_PROFILE_REG_RESULT_ERR_INVAL_OP : Operation not supported for tech type
  DS_PROFILE_REG_RESULT_FAIL      : On general errors. This return 
                                    code provides blanket coverage
SIDE EFFECTS 
 
===========================================================================*/
ds_profile_status_etype dsi_profile_get_default_profile (  
  ds_profile_tech_etype  tech,
  uint32                 family, 
  ds_profile_num_type   *num
);

/*===========================================================================
FUNCTION DSI_PROFILE_GET_LIST_ITR

DESCRIPTION
  This is used to get the list of Profiles of a particular tech type. This 
  function returns an Iterator. The Iterator is traversed using 
  DS_PROFILE_ITR_NEXT. After traversal is complete, the caller is 
  expected to call DS_PROFILE_ITR_DESTROY.

PARAMETERS
  tech  : technology type
  lst   : type of list, (list with all profiles / depending on some search)
  itr   : iterator to traverse through search result

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS  : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Invalid tech type
  DS_PROFILE_REG_RESULT_FAIL     : On general errors. This return code 
                                   provides blanket coverage
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype dsi_profile_get_list_itr ( 
  ds_profile_tech_etype  tech,
  ds_profile_list_type  *lst,
  ds_profile_itr_type   *itr
);

/*===========================================================================
FUNCTION DSI_PROFILE_ITR_NEXT

DESCRIPTION
  This routine advances the Iterator to the next element.  

PARAMETERS
  itr : iterator

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS   : On successful operation
  DS_PROFILE_REG_RESULT_FAIL      : On general errors. This return code 
                                    provides blanket coverage
  DS_PROFILE_REG_RESULT_ERR_INVAL : Invalid argument (iterator)
  DS_PROFILE_REG_RESULT_LIST_END  : End of list
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype dsi_profile_itr_next (  
  ds_profile_itr_type   itr
);

/*===========================================================================
FUNCTION DSI_PROFILE_ITR_FIRST

DESCRIPTION
  This routine resets the Iterator to the beginning of the list.  

PARAMETERS
  itr : iterator

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS   : On successful operation
  DS_PROFILE_REG_RESULT_FAIL      : On general errors. This return code 
                                    provides blanket coverage
  DS_PROFILE_REG_RESULT_ERR_INVAL : Invalid argument (iterator)
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype dsi_profile_itr_first ( 
  ds_profile_itr_type   itr
);

/*===========================================================================
FUNCTION DSI_PROFILE_GET_INFO_BY_ITR

DESCRIPTION
  This routine gets info stored in that Iterator node.  

PARAMETERS
  itr       : iterator
  list_info : pointer to structure to return profile info

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS   : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL : Invalid argument (iterator)
  DS_PROFILE_REG_RESULT_FAIL      : On general errors. This return code 
                                    provides blanket coverage
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype dsi_profile_get_info_by_itr ( 
  ds_profile_itr_type         itr,
  ds_profile_list_info_type  *list_info
);

/*===========================================================================
FUNCTION DSI_PROFILE_ITR_DESTROY

DESCRIPTION
  This routine destroys the Iterator  

PARAMETERS
  itr : iterator

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS   : On successful operation
  DS_PROFILE_REG_RESULT_FAIL      : On general errors. This return code 
                                    provides blanket coverage
  DS_PROFILE_REG_RESULT_ERR_INVAL : Invalid argument (iterator)
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype dsi_profile_itr_destroy ( 
  ds_profile_itr_type   itr
);

void dsi_profile_get_profile_num_range (
  ds_profile_tech_etype tech,
  uint16               *min_num,
  uint16               *max_num
);

ds_profile_status_etype dsi_profile_validate_profile_num( 
  ds_profile_tech_etype tech, 
  ds_profile_num_type   num 
);

ds_profile_status_etype dsi_profile_get_supported_type(  
  uint32                 *num,
  ds_profile_tech_etype  *tech
);

void dsi_profile_get_max_num( 
  ds_profile_tech_etype  tech,
  uint32                *max_num
);

#endif /* DS_PROFILEI_H */
