/******************************************************************************
  @file    ds_profile_int.c
  @brief   DS PROFILE internal definitions

  DESCRIPTION
  Internal file for ds profile. Depending on tech calls tech specific
  functions

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

  $Header: //source/qcom/qct/modem/data/common/dsprofile/main/latest/src/ds_profilei.c#4 $ $DateTime: 2009/10/26 12:52:40 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#include "ds_profile_plm.h"
#include "ds_profilei.h"
#include "ds_profile_os.h"
#include "ds_util.h"

static plm_store_type plm_tbl[ DS_PROFILE_TECH_MAX ];

/* Handle type */
typedef struct 
{
  ds_profile_num_type      num;     /* profile number */

  ds_profile_tech_etype    tech;    /* technology type */

  uint16                   max_num; /* maximum number of profiles supported */

  void                    *prf_blob;/* pointer to local copy of profile*/

  ds_profile_trn_etype     trn;     /* transaction type */

  boolean                  commit;  /* Commit flag set in set_param function*/

  void                    *self;    /* ptr to self blob for validation */

}dsi_profile_hndl_t;

/* Internal iterator data structure */
typedef struct 
{
  ds_util_itr_hndl_type  hndl;
  ds_profile_tech_etype  tech;
} dsi_profile_itr_type;

/* For internal handle array */
typedef struct 
{
  dsi_profile_hndl_t *ptr;
}dsi_profile_hndl_arr_type;

/* store all open handles */
static dsi_profile_hndl_arr_type hndl_tbl[DS_PROFILE_MAX_NUM_HNDL];

static uint8 valid_tech_mask;

/*---------------------------------------------------------------------------
                            MACROs
---------------------------------------------------------------------------*/
/* Macro to check if handle can be used for write transaction */
#define HNDL_IS_WRITABLE( dsi_hndl ) \
  ( dsi_hndl->trn & DS_PROFILE_TRN_RW )

/* Macro to check if handle is valid */
#define IS_HNDL_VALID( hndl ) \
          ( ( ( hndl ) != NULL ) && ( ( (dsi_profile_hndl_t *)hndl)->self == hndl ) )


/*---------------------------------------------------------------------------
               Implementation of DSI PROFILE module
---------------------------------------------------------------------------*/

/*===========================================================================
FUNCTION DSI_PROFILE_GET_PROFILE_NUM_RANGE 

DESCRIPTION
  This function calls internal tech specific function to get the range of
  profile numbers

PARAMETERS
  tech : technology type
  num  : profile number

DEPENDENCIES 
  
RETURN VALUE 
 
SIDE EFFECTS 
  none 
===========================================================================*/
void dsi_profile_get_profile_num_range (
  ds_profile_tech_etype tech,
  uint16 *min_num,
  uint16 *max_num
)
{
  /* Call tech specific handler */
  if ( plm_tbl[tech].vtbl.get_num_range == NULL )
  {    
    *min_num = 0;
    *max_num = 0;
    return;
  }
  plm_tbl[tech].vtbl.get_num_range(min_num, max_num);
}

/*===========================================================================
FUNCTION DSI_PROFILE_VALIDATE_PROFILE_NUM 

DESCRIPTION
  This function calls internal tech specific function to validate the profile
  number

PARAMETERS
  tech : technology type
  num  : profile number

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS   : On successful operation
  DS_PROFILE_REG_RESULT_FAIL      : On general errors. This return code 
                                    provides blanket coverage
 DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : invalid tech type
SIDE EFFECTS 
  none
===========================================================================*/ 
ds_profile_status_etype dsi_profile_validate_profile_num( 
  ds_profile_tech_etype tech, 
  ds_profile_num_type   num 
)
{
  /* Call tech specific handler */
  if ( plm_tbl[tech].vtbl.validate_profile_num == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  return plm_tbl[tech].vtbl.validate_profile_num( num );
}

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
)
{
  uint8 tech_mask = 0;
  /* Call PLM API to init tech modules */
  valid_tech_mask = plm_tech_ops_init(plm_tbl);
  tech_mask = plm_tech_access_init(plm_tbl);
  if ( tech_mask != valid_tech_mask)
  {
    return 0;
  }
  return valid_tech_mask;
}

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
)
{
  uint32 i = 0;
  /* Check if handles are destroyed */
  for (i = 0; i < DS_PROFILE_MAX_NUM_HNDL; i++) 
  {
    if ( IS_HNDL_VALID( hndl_tbl[i].ptr ) )
    {
      if ( plm_tbl[hndl_tbl[i].ptr->tech].vtbl.dealloc( hndl_tbl[i].ptr->prf_blob )
       !=  DSI_SUCCESS )
      {
        DS_PROFILE_LOGE( "_close_lib: FAIL dealloc", 0 );
        DS_PROFILE_LOGD( "_close_lib: EXIT with ERR", 0 );
        return;
      }
      DS_PROFILE_MEM_FREE( (void *)hndl_tbl[i].ptr,
                      MODEM_MEM_CLIENT_DATA );
    }
    hndl_tbl[i].ptr = NULL;
  }
}

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
ds_profile_status_etype dsi_profile_alloc_hndl (
  ds_profile_trn_etype   trn,
  ds_profile_tech_etype  tech,
  ds_profile_num_type    num,
  ds_profile_hndl_type  *hndl
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  dsi_profile_hndl_t  *tmp_hndl = NULL;
  uint32 i = 0;
  uint16 min_num = 0;
  DS_PROFILE_LOGD("_alloc_hndl: ENTRY",0);

  /* Check if handle can be allocated */
  for (i = 0; i < DS_PROFILE_MAX_NUM_HNDL; i++) 
  {
    if( hndl_tbl[i].ptr == NULL )
      break;
  }

  if ( i == DS_PROFILE_MAX_NUM_HNDL ) {
    DS_PROFILE_LOGE( "_alloc_hndl: FAIL max handles allocated", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  /* Allocate handle */
  tmp_hndl = ( dsi_profile_hndl_t * ) DS_PROFILE_MEM_ALLOC( 
      sizeof( dsi_profile_hndl_t ),
      MODEM_MEM_CLIENT_DATA );
  
  if( tmp_hndl == NULL )
  {
    DS_PROFILE_LOGE( "_alloc_hndl: FAIL DS_PROFILE_MEM_ALLOC", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  memset( tmp_hndl, 0, sizeof( dsi_profile_hndl_t ) );

  tmp_hndl->num  = num;
  tmp_hndl->tech = tech;
  tmp_hndl->trn  = trn;
  tmp_hndl->commit = 0;

  /* Validate tech type */
  if ( !TECH_IS_VALID( tech ) ) 
  {
    DS_PROFILE_LOGE( "dsi_profile_alloc_hndl: FAIL invalid tech type ", 0 );
    goto return_clean;
  }
  dsi_profile_get_profile_num_range(tech, &min_num, &(tmp_hndl->max_num) );

  if ( plm_tbl[tech].vtbl.alloc == NULL )
    goto return_clean;

  if ( ( tmp_hndl->prf_blob = plm_tbl[tech].vtbl.alloc() ) == NULL ) 
  {
    DS_PROFILE_LOGE( "_alloc_hndl: FAIL internal alloc function ", 0 );
    goto return_clean;
  }

  if ( (return_status = ( plm_tbl[tmp_hndl->tech].vtbl.profile_read(num,
                                                   tmp_hndl->prf_blob) ) ) !=  
       DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE("_alloc_hndl: FAIL read",0);
    goto return_clean;
  }

  /* Mark blob as valid */
  tmp_hndl->self = (void *)tmp_hndl;
  *hndl = (void *)tmp_hndl;

  /* Store handle in hndl_tbl */
  hndl_tbl[i].ptr = tmp_hndl;

  DS_PROFILE_LOGD("_alloc_hndl: EXIT with SUCCESS",0);
  return DS_PROFILE_REG_RESULT_SUCCESS;

return_clean:
  DS_PROFILE_MEM_FREE( (void *)tmp_hndl,
                  MODEM_MEM_CLIENT_DATA );

  DS_PROFILE_LOGD( "_alloc_hndl: EXIT with ERR", 0 );
  return return_status;
}

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
ds_profile_status_etype dsi_profile_dealloc_hndl (
  ds_profile_hndl_type *hndl
)
{  
  dsi_profile_hndl_t *dsi_hndl = ( dsi_profile_hndl_t *)(*hndl);
  uint32 i = 0;
  DS_PROFILE_LOGD( "_dealloc_hndl: ENTRY", 0 );
  if ( !IS_HNDL_VALID( dsi_hndl) ) 
  {
    DS_PROFILE_LOGE("_dealloc_hndl: INVAL hndl",0);
    return DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL;
  }

  /* set handle to NULL in hndl_tbl */
  for (i = 0; i < DS_PROFILE_MAX_NUM_HNDL; i++) 
  {
    if( hndl_tbl[i].ptr == dsi_hndl )
    {
      hndl_tbl[i].ptr = NULL;
      break;
    }
  }

  if ( i == DS_PROFILE_MAX_NUM_HNDL ) {
    DS_PROFILE_LOGE( "_alloc_hndl: FAIL max handles allocated", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  if ( plm_tbl[dsi_hndl->tech].vtbl.dealloc == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  /* dealloc profile blob */
  if ( plm_tbl[dsi_hndl->tech].vtbl.dealloc( dsi_hndl->prf_blob )
       !=  DSI_SUCCESS )
  {
    DS_PROFILE_MEM_FREE( (void *)dsi_hndl,
                    MODEM_MEM_CLIENT_DATA );

    DS_PROFILE_LOGE( "_dealloc_hndl: FAIL dealloc", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  DS_PROFILE_MEM_FREE( (void *)dsi_hndl, 
                  MODEM_MEM_CLIENT_DATA );

  DS_PROFILE_LOGD( "_dealloc_hndl: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;  
}

/*===========================================================================
FUNCTION DSI_PROFILE_GET_PARAM

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
  ds_profile_identifier_type  ident,
  ds_profile_info_type       *info
)
{  
  dsi_profile_hndl_t *dsi_hndl = ( dsi_profile_hndl_t * )hndl;
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_get_param: ENTRY",0);

  /* Validate handle */
  if ( !IS_HNDL_VALID( dsi_hndl ) ) 
  {
    DS_PROFILE_LOGE( "_get_param: INVAL hndl",0);
    return DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL; 
  }

  /* Validate tech type */
  if ( !TECH_IS_VALID( dsi_hndl->tech ) ) 
  {
    DS_PROFILE_LOGE( "_get_param: FAIL invalid tech type ", 0 );
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  }

  /* Validate profile number */
  if( ( return_status = dsi_profile_validate_profile_num( dsi_hndl->tech, dsi_hndl->num ) ) !=
      DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_get_param: FAIL invalid profile number ", 0 );
    return return_status;
  }

  if ( plm_tbl[ dsi_hndl->tech ].vtbl.get_param == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
 
  /* Call tech specific get_param function*/
  return_status = plm_tbl[ dsi_hndl->tech ].vtbl.get_param(dsi_hndl->prf_blob,
                                                ident, 
                                                info );
  if ( return_status !=  DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_get_param: EXIT with ERR",0);
    return return_status;
  }
  DS_PROFILE_LOGD( "_get_param: EXIT with SUCCESS",0);
  return return_status;
}

/*===========================================================================
FUNCTION DSI_PROFILE_SET_PARAM

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
  ds_profile_identifier_type  ident,
  const ds_profile_info_type *info
)
{
  dsi_profile_hndl_t *dsi_hndl = ( dsi_profile_hndl_t * )hndl;
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_set_param: ENTRY",0);

/*lint -save -e655*/
  if ( !IS_HNDL_VALID( dsi_hndl ) || ( !HNDL_IS_WRITABLE( dsi_hndl ) ) ) 
  {
    DS_PROFILE_LOGE( "_set_param: INVAL hndl",0);
    return DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL; 
  }
/*lint -restore Restore lint error 655*/

  /* Validate tech type */
  if ( !TECH_IS_VALID( dsi_hndl->tech ) ) 
  {
    DS_PROFILE_LOGE( "_set_param: FAIL invalid tech type ", 0 );
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  }

  /* Validate profile number */
  if( ( return_status = dsi_profile_validate_profile_num( dsi_hndl->tech, dsi_hndl->num ) ) !=
      DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_set_param: FAIL invalid profile number ", 0 );
    return return_status;
  }

  if ( plm_tbl[ dsi_hndl->tech ].vtbl.set_param == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;

  /* Call tech specific set_param function*/
  return_status = plm_tbl[ dsi_hndl->tech ].vtbl.set_param( dsi_hndl->prf_blob, 
                                                 ident, 
                                                 info );
  if ( return_status !=  DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_set_param: EXIT with ERR",0);
    return return_status;
  }
  dsi_hndl->commit = TRUE;
  DS_PROFILE_LOGD( "_set_param: EXIT with SUCCESS",0);
  return return_status;
}

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
)
{
  dsi_profile_hndl_t *dsi_hndl = ( dsi_profile_hndl_t * )hndl;
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_end_transaction: ENTRY",0);

  /* Validate handle */
  if ( !IS_HNDL_VALID( dsi_hndl ) ) 
  {
    DS_PROFILE_LOGE( "_end_transaction: INVAL hndl", 0 );
    return DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL; 
  }

  /* Check if commit flag is set, only then call write function */
  if ( dsi_hndl->commit == FALSE )
  {
    goto ret_succ;
  }

  /* Validate tech type */
  if ( !TECH_IS_VALID( dsi_hndl->tech ) ) 
  {
    DS_PROFILE_LOGE( "_end_transaction: FAIL invalid tech type ", 0 );
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  }

  /* Validate profile number */
  if( ( return_status = dsi_profile_validate_profile_num( dsi_hndl->tech, dsi_hndl->num ) ) !=
      DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_end_transaction: FAIL invalid profile number ", 0 );
    return return_status;
  }

  if ( plm_tbl[dsi_hndl->tech].vtbl.profile_write == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;

  /* Call tech specific write function */
  if ( (return_status = plm_tbl[dsi_hndl->tech].vtbl.profile_write(dsi_hndl->num,
                                                  dsi_hndl->prf_blob) ) 
       !=  DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_end_transaction: FAIL write", 0 );
    return return_status;
  }

ret_succ:
  DS_PROFILE_LOGD( "_end_transaction: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

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
  ds_profile_tech_etype  tech,
  ds_profile_num_type   *num
)
{
  if ( plm_tbl[ tech ].vtbl.create == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  /* Call tech specific handler */
  return plm_tbl[ tech ].vtbl.create( num );
}

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
  ds_profile_tech_etype tech,
  ds_profile_num_type   num 
)
{
  if ( plm_tbl[tech].vtbl.del == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  /* Call tech specific handler */
  return plm_tbl[tech].vtbl.del( num );
}

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
)
{
  if ( plm_tbl[tech].vtbl.reset_param == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  /* Call tech specific handler */
  return plm_tbl[tech].vtbl.reset_param( num, ident ); 
}

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
)
{
  if ( plm_tbl[tech].vtbl.reset_profile_to_default == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  /* Call tech specific handler */
  return plm_tbl[tech].vtbl.reset_profile_to_default( num );   
}

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
)
{
  if ( plm_tbl[tech].vtbl.set_default_profile == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  /* Call tech specific handler */
  return plm_tbl[tech].vtbl.set_default_profile( family, num );   
}

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
)
{
  if ( plm_tbl[tech].vtbl.get_default_profile == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  /* Call tech specific handler */
  return plm_tbl[tech].vtbl.get_default_profile( family, num );   
}

/*===========================================================================
FUNCTION DSI_PROFILE_GET_SUPPORTED_TYPE
===========================================================================*/
ds_profile_status_etype dsi_profile_get_supported_type (  
  uint32                 *num,
  ds_profile_tech_etype  *tech
)
{
  uint8 tmp_mask = valid_tech_mask;
  uint8 mask = 0;
  uint8 index = 0;
  uint8 tmp_tech = 0;
  *num = 0;
    
  if ( valid_tech_mask == 0 )
  {  
    DS_PROFILE_LOGD("_get_supported_type: EXIT with ERR",0);
    return DS_PROFILE_REG_RESULT_FAIL;  
  }

  while ( tmp_mask != 0 )
  {
    mask = 0x01 << tmp_tech;
    if ( valid_tech_mask & mask )
    {
      tech[index] = (ds_profile_tech_etype)tmp_tech;
      index++;
    }      
    tmp_mask = tmp_mask >> 1;
    tmp_tech++;
  }  

  *num = (uint32)index;
  DS_PROFILE_LOGD("_get_supported_type: EXIT with SUCCESS",0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

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
)
{
  ds_util_list_hndl_type hndl;
  ds_util_itr_hndl_type itr_hndl;
  dsi_profile_itr_type *dsi_itr = NULL;
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  
  if ( ds_util_list_get_hndl( &hndl ) != DS_SUCCESS )
  {
    DS_PROFILE_LOGE("_get_list_itr: unable to get list handle EXIT with ERR ", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  if ( plm_tbl[ tech ].vtbl.get_list == NULL )
  {
    ds_util_list_rel_hndl(hndl);
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
  }

  return_status = plm_tbl[ tech ].vtbl.get_list( hndl, lst );
  if ( return_status != DS_PROFILE_REG_RESULT_SUCCESS )
  {
    ds_util_list_rel_hndl(hndl);
    DS_PROFILE_LOGE("_get_list_itr: Err / Empty list", 0 );
    return return_status;
  }
  
  if ( ds_util_itr_get_hndl( (ds_util_iterable_type *)hndl, &itr_hndl ) != DS_SUCCESS )
  {
    ds_util_list_rel_hndl(hndl);
    DS_PROFILE_LOGE("_get_list_itr: unable to get itr handle EXIT with ERR ", 0 );
    return DS_PROFILE_REG_RESULT_FAIL; 
  }

  dsi_itr = (dsi_profile_itr_type *)DS_PROFILE_MEM_ALLOC(
      sizeof(dsi_profile_itr_type),
      MODEM_MEM_CLIENT_DATA);

  if ( dsi_itr == NULL )
  {
    ds_util_list_rel_hndl(hndl);
    DS_PROFILE_LOGE("_get_list_itr: FAILED DS_PROFILE_MEM_ALLOC", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  dsi_itr->hndl = (ds_profile_itr_type)itr_hndl;
  dsi_itr->tech = tech;

  *itr = (ds_profile_itr_type)dsi_itr;

  DS_PROFILE_LOGD("_get_list_itr: EXIT with SUCCESS ", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

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
)
{
  dsi_profile_itr_type *dsi_itr = (dsi_profile_itr_type *)itr;
  DS_PROFILE_LOGD( "_itr_next: ENTRY", 0 );

  if ( ds_util_itr_next( dsi_itr->hndl ) != DS_SUCCESS )
  {
    DS_PROFILE_LOGE("_itr_next: EXIT with ERR ", 0 );
    return DS_PROFILE_REG_RESULT_LIST_END;
  }

  DS_PROFILE_LOGD( "_itr_next: EXIT with SUCCESS ",0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

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
)
{
  dsi_profile_itr_type *dsi_itr = (dsi_profile_itr_type *)itr;
  DS_PROFILE_LOGD( "_itr_first: ENTRY",0);

  if ( ds_util_itr_first( dsi_itr->hndl ) != DS_SUCCESS )
  {
    DS_PROFILE_LOGE("_itr_first: EXIT with ERR ", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }
  
  DS_PROFILE_LOGD( "_itr_first: EXIT with SUCCESS ",0);
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

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
)
{
  dsi_profile_itr_type *dsi_itr = (dsi_profile_itr_type *)itr;
  DS_PROFILE_LOGD( "_get_info_by_itr: ENTRY", 0 );

  if ( plm_tbl[ dsi_itr->tech ].vtbl.get_list_node == NULL )
    return DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;

  if ( plm_tbl[ dsi_itr->tech ].vtbl.get_list_node( dsi_itr->hndl, list_info ) 
       != DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE("_get_list_node: EXIT with ERR ", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  DS_PROFILE_LOGD( "_get_info_by_itr: EXIT with SUCCESS ", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

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
)
{
  dsi_profile_itr_type *dsi_itr = (dsi_profile_itr_type *)itr;
  DS_PROFILE_LOGD( "_itr_destroy: ENTRY", 0 );

  if ( ds_util_itr_rel_hndl( dsi_itr->hndl ) != DS_SUCCESS)
  {
    DS_PROFILE_LOGD( "_itr_destroy: EXIT with ERR ", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  if ( ds_util_list_rel_hndl( (ds_util_list_hndl_type)dsi_itr->hndl ) 
       != DS_SUCCESS)
  {
    DS_PROFILE_LOGD( "_itr_destroy: EXIT with ERR ", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  DS_PROFILE_MEM_FREE( (void *)dsi_itr,
                  MODEM_MEM_CLIENT_DATA );
 
  DS_PROFILE_LOGD( "_itr_destroy: EXIT with SUCCESS ", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}

