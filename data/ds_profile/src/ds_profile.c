/******************************************************************************
  @file    ds_profile.c
  @brief   DS PROFILE API implementation

  DESCRIPTION
  This file contains implementation of DS PROFILE API.

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

  $Header: //source/qcom/qct/modem/data/common/dsprofile/main/latest/src/ds_profile.c#1 $ $DateTime: 2009/09/30 19:03:37 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

/*---------------------------------------------------------------------------
                           INCLUDE FILES
---------------------------------------------------------------------------*/
#include "ds_profile_plm.h"
#include "ds_profile_os.h"
#include "ds_profilei.h"


#define DS_PROFILE_LIB_STATE_INVAL   0x00
#define DS_PROFILE_LIB_STATE_INITED  0x01
#define MAX_LIB_INST_NAME            10
static char           lib_state;

/* Platform specific lock */
static plm_lock_type  lib_lock;
static uint32         instance;

char logging_prefix[MAX_LIB_INST_NAME] = "PRFREG_";

/*---------------------------------------------------------------------------
                               UTILITY MACROS
---------------------------------------------------------------------------*/
/*lint -save -e655*/
/* Macro to check if profile handle is valid   */
#define HNDL_IS_VALID( hndl ) \
          ( ( hndl ) != NULL )

/* Macro to check if lib is inited             */
#define LIB_STATE_IS_VALID( lib_state ) \
  ( lib_state == DS_PROFILE_LIB_STATE_INITED )

/* Macro to check if transation type is valid  */
#define TRN_IS_VALID( trn ) \
  ( trn & DS_PROFILE_TRN_VALID )

/* Macro to check if list_type is valid        */
#define LIST_TYPE_IS_VALID( lst ) \
  ( lst != NULL )
 
/* Macro to check if itr is valid              */
#define ITR_IS_VALID( itr ) \
  ( itr != NULL )

/* TEST_FRAMEWORK for QTF                      */
#ifdef TEST_FRAMEWORK
/* Macro to acquire lock                       */
#define ACQ_LOCK( msg ) \
  if ( ds_profile_lock_acq( lib_lock ) != 0 ) \
  { \
    DS_PROFILE_LOGE( msg , 0 ); \
    DS_PROFILE_LOGE( "FAIL: unable to acquire lock", 0 ); \
    return DS_PROFILE_REG_RESULT_FAIL; \
  } 

/* Macro to release lock                       */
#define REL_LOCK( msg ) \
  if ( ds_profile_lock_rel( lib_lock ) != 0 ) \
  { \
    DS_PROFILE_LOGE( msg , 0 ); \
    DS_PROFILE_LOGE( "FAIL: unable to release lock", 0 ); \
    return DS_PROFILE_REG_RESULT_FAIL; \
  }
#else
/* Macro to acquire lock                       */
#define ACQ_LOCK( msg ) \
  if ( ds_profile_lock_acq( &lib_lock ) != 0 ) \
  { \
    DS_PROFILE_LOGE( msg , 0 ); \
    DS_PROFILE_LOGE( "FAIL: unable to acquire lock", 0 ); \
    return DS_PROFILE_REG_RESULT_FAIL; \
  } 

/* Macro to release lock                       */
#define REL_LOCK( msg ) \
  if ( ds_profile_lock_rel( &lib_lock ) != 0 ) \
  { \
    DS_PROFILE_LOGE( msg , 0 ); \
    DS_PROFILE_LOGE( "FAIL: unable to release lock", 0 ); \
    return DS_PROFILE_REG_RESULT_FAIL; \
  }
#endif 

/*===========================================================================
FUNCTION DS_PROFILE_INIT_LIB

DESCRIPTION
  This function initializes the DS profile library. On modem, this function is 
  called only once at initialization. This will initialize the library for
  that process domain.

PARAMETERS

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS  : On successful operation
  DS_PROFILE_REG_RESULT_FAIL     : On general errors. This return code 
                                   provides blanket coverage
SIDE EFFECTS 
 
===========================================================================*/
ds_profile_status_etype ds_profile_init_lib (
  void
)
{
  uint8 tech_mask = 0;
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  char lib[MAX_LIB_INST_NAME + 5];

  DS_PROFILE_LOGD( "_init_lib: ENTRY", 0 );

  /*--------------------------------------------------------------------------- 
    multiple calls to init lib allowed from different thread contexts
  ---------------------------------------------------------------------------*/
  instance++;
  instance %= DS_PROFILE_MAX_NUM_HNDL;
  //(void)std_strlprintf( lib, 
  //                     sizeof(logging_prefix) + sizeof(instance), 
  //                      "%s[%d]", logging_prefix, instance );

  /*--------------------------------------------------------------------------- 
    Initialize logging and logging
  ---------------------------------------------------------------------------*/
  if( ds_profile_log_init( lib ) != 0 )
  {
    DS_PROFILE_LOGE( "_init_lib: FAIL, log_init", 0 );
    goto ret_err;
  }

  if ( ds_profile_lock_init( &lib_lock ) != 0 ) 
  {
    DS_PROFILE_LOGE( "_init_lib: FAIL, unable to init lib_lock", 0 );
    goto ret_err;
  }

  /* Acquire lock */
  ACQ_LOCK( "_init_lib" );
  /*--------------------------------------------------------------------------- 
    Allow initialization only if not inited
  ---------------------------------------------------------------------------*/
  if ( LIB_STATE_IS_VALID( lib_state ) )
  {
    REL_LOCK( "_init_lib" );
    DS_PROFILE_LOGE( "_init_lib: FAIL, lib state invalid", 0 );
    goto ret_err;
  }

  /*--------------------------------------------------------------------------- 
    Call internal function for initialization. Returns the ORed value of all
    tech types, if 0 then error 
  ---------------------------------------------------------------------------*/
  if ( (tech_mask = dsi_profile_init()) ==  0 )
  {
    REL_LOCK( "_init_lib" );
    DS_PROFILE_LOGE( "_init_lib: FAIL, dsi_profile_init() failed, tech mask %x", tech_mask );
    goto ret_err;
  }

  DS_PROFILE_LOGD( "_init_lib: tech mask returned %x", tech_mask );

  lib_state = DS_PROFILE_LIB_STATE_INITED;
  REL_LOCK( "_init_lib" );
  DS_PROFILE_LOGD( "_init_lib: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS; 

ret_err:
  DS_PROFILE_LOGD( "_init_lib: EXIT with ERR", 0 );
  return return_status; 
} 

/*===========================================================================
FUNCTION DS_PROFILE_CLOSE_LIB 
 
DESCRIPTION
  This functions cleans up any open handles, closes the library

PARAMETERS

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS  : On successful operation
  DS_PROFILE_REG_RESULT_FAIL     : On general errors. This return code 
                                   provides blanket coverage
SIDE EFFECTS  
===========================================================================*/
ds_profile_status_etype ds_profile_close_lib (
  void
)
{
  DS_PROFILE_LOGD( "_close_lib: ENTRY", 0 );

  /* Check lib was initialized */
  if ( !LIB_STATE_IS_VALID( lib_state ) ) 
  {
    DS_PROFILE_LOGE( "_close_lib: FAIL lib was not initialized", 0 );
    DS_PROFILE_LOGD( "_close_lib: EXIT with ERR", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }

  ACQ_LOCK( "_init_lib" );
  lib_state = DS_PROFILE_LIB_STATE_INVAL;
  /*--------------------------------------------------------------------------- 
    Check handles have been released and iterators are destroyed
  ---------------------------------------------------------------------------*/
  dsi_profile_close_lib();

  REL_LOCK( "_close_lib" );
  DS_PROFILE_LOGD( "_close_lib: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
} 

/*===========================================================================
FUNCTION DS_PROFILE_BEGIN_TRANSACTION

DESCRIPTION
  This returns a Handle that the clients of this software library can use for 
  subsequent Profile operations. The Handle returned is of requested 
  transaction type. All Profile operations using this Handle require that 
  DS_PROFILE_END_TRANSACTION be called at the end. 

PARAMETERS
  trn  : requested transaction type
  tech : technology type
  num  : profile number
  hndl : pointer to return requested handle

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS            : On successful operation
  DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED : Library is not initialized
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Invalid tech type
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM  :  Invalid profile number 
  DS_PROFILE_REG_RESULT_FAIL               : On general errors. This return 
                                             code provides blanket coverage
SIDE EFFECTS 
 
===========================================================================*/
ds_profile_status_etype ds_profile_begin_transaction (
  ds_profile_trn_etype   trn,
  ds_profile_tech_etype  tech,
  ds_profile_num_type    num,
  ds_profile_hndl_type  *hndl
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_begin_transaction: ENTRY", 0 );

  /* Validate lib state */
  if( !LIB_STATE_IS_VALID( lib_state ) )
  {
    DS_PROFILE_LOGE( "_begin_transaction: FAIL lib not inited ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED; 
    goto ret_err;
  }

  /* Validate transaction */
  if ( !TRN_IS_VALID( trn ) )
  {
    DS_PROFILE_LOGE( "_begin_transaction: FAIL invalid transaction type ", 0 );
    return_status = DS_PROFILE_REG_RESULT_FAIL;
    goto ret_err;
  }

  /* Validate tech type */
  if ( !TECH_IS_VALID( tech ) ) 
  {
    DS_PROFILE_LOGE( "_begin_transaction: FAIL invalid tech type ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
    goto ret_err;
  }

  /* Validate profile number */
  if( ( return_status = dsi_profile_validate_profile_num( tech, num ) ) 
      != DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_begin_transaction: FAIL invalid profile number ", 0 );
    goto ret_err;
  }

  ACQ_LOCK( "_begin_transaction" );
  /*-------------------------------------------------------------------------- 
    Call internal function to allocate handle for transaction
  --------------------------------------------------------------------------*/
  if ( ( return_status = dsi_profile_alloc_hndl( trn, tech, num, hndl ) ) 
       !=  DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE("_begin_transaction: FAIL alloc hndl", 0 ); 
    REL_LOCK( "_begin_transaction" );
    goto ret_err;
  }

  REL_LOCK( "_begin_transaction" );
  DS_PROFILE_LOGD( "_begin_transaction: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;

ret_err:
  DS_PROFILE_LOGD( "_begin_transaction: EXIT with ERR", 0 );
  return return_status;
} 

/*===========================================================================
FUNCTION DS_PROFILE_END_TRANSACTION

DESCRIPTION
  This function commits the prefetched modified profile to the persistent 
  storage on the modem. It also invokes cleanup routines for the profile
  handle specified. On return the handle becomes unusable

PARAMETERS
 hndl  : profile handle
 act   : action (commit / cancel)

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS        : On successful operation
  DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED : Library is not initialized
  DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL : Invalid handle
  DS_PROFILE_REG_RESULT_FAIL           : On general errors.
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype ds_profile_end_transaction (
  ds_profile_hndl_type    profile_hndl,
  ds_profile_action_etype act
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_end_transaction: ENTRY", 0 );

  /* Validate lib state */
  if( !LIB_STATE_IS_VALID( lib_state ) )
  {
    DS_PROFILE_LOGE( "_end_transaction: FAIL lib not inited ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED; 
    goto ret_err;
  }

  /* Validate handle */
  if ( !HNDL_IS_VALID( profile_hndl )  ) 
  {
    DS_PROFILE_LOGE( "_end_transaction: INVAL hndl", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL;
    goto ret_err;
  }

  /* Lock here */
  ACQ_LOCK( "_end_transaction" );
  if(act == DS_PROFILE_ACTION_COMMIT)
  {
    /*----------------------------------------------------------------------- 
      Call internal function to end transaction and write back the changes
      made on the profile
    ------------------------------------------------------------------------*/
    if ( ( return_status = dsi_profile_end_transaction( profile_hndl ) ) 
       !=  DS_PROFILE_REG_RESULT_SUCCESS )
    {
      /*-------------------------------------------------------------------- 
        If end transaction fails, call internal function to dealloc handle,
        release memory
      ---------------------------------------------------------------------*/
      (void)dsi_profile_dealloc_hndl(&profile_hndl );
      DS_PROFILE_LOGE( "_end_transaction: FAIL internal function", 0 ); 
      REL_LOCK( "_end_transaction" );
      goto ret_err;
    }
  }
  /*-------------------------------------------------------------------- 
    Call internal function to dealloc handle, release memory
  ---------------------------------------------------------------------*/
  if ( ( return_status = dsi_profile_dealloc_hndl(&profile_hndl ) ) 
       !=  DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_end_transaction: FAIL dealloc hndl", 0 ); 
    REL_LOCK( "_end_transaction" );
    goto ret_err;
  }
  
  REL_LOCK( "_end_transaction" );
  DS_PROFILE_LOGD( "_end_transaction: EXIT with SUCCESS", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;

ret_err:
  DS_PROFILE_LOGD( "_end_transaction: EXIT with ERR", 0 );
  return return_status;
} 

/*===========================================================================
FUNCTION DS_PROFILE_SET_PARAM

DESCRIPTION
  This function is used to set profile data element identified by the 
  identifier. The identifiers are specified in the corresponding tech 
  header file. The prefetched copy is modified. end_transaction will
  modify the profile on modem. 

PARAMETERS
  profile_hndl  : handle to profile to set profile data elements
  identifier    : to identify profile data elements
  info          : pointer to value to which data element is to be set
                  (size of buffer passed can atmost be the max size of 
                  the parameter which needs to be set)
DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS            : On successful operation
  DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED : Library is not initialized
  DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL     : Invalid handle 
  DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT    : Invalid identifier
  DS_PROFILE_REG_RESULT_ERR_LEN_INVALID    : Buffer size more than expected
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Invalid tech type 
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM  : Invalid profile number
  DS_PROFILE_REG_RESULT_FAIL               : On general errors 
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype ds_profile_set_param (  
  ds_profile_hndl_type        hndl,  
  ds_profile_identifier_type  identifier,
  const ds_profile_info_type *info
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_set_param: ENTRY", 0 );

  /* Validate lib state */
  if( !LIB_STATE_IS_VALID( lib_state ) )
  {
    DS_PROFILE_LOGE( "_set_param: FAIL lib not inited ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED; 
    goto ret_err;
  }

  /* Validate handle */
  if ( (!HNDL_IS_VALID( hndl )) ) 
  {
    DS_PROFILE_LOGE( "_set_param: INVAL hndl", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL;
    goto ret_err;
  }

  ACQ_LOCK( "_set_param" );
  /*-------------------------------------------------------------------- 
    Call internal function which depending on tech and identifier
    calls tech-specific set function
  ---------------------------------------------------------------------*/
  if ( ( return_status = dsi_profile_set_param( hndl, identifier, info ) ) 
       !=  DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE("_set_param: FAIL internal set function", 0 );
    REL_LOCK( "_set_param" );
    goto ret_err;
  }

  REL_LOCK( "_set_param" );
  DS_PROFILE_LOGD("_set_param: EXIT with SUCCESS ", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;

ret_err:
  DS_PROFILE_LOGD( "_set_param: EXIT with ERR", 0 );
  return return_status;
} 

/*===========================================================================
FUNCTION DS_PROFILE_GET_PARAM

DESCRIPTION
  This function is used to get Profile data element identified by the 
  identifier. The identifiers are specified in the corresponding tech 
  header file. The data elements are read from the prefetched Profile and
  info is returned with that value and length.

PARAMETERS
  profile_hndl : handle to profile to get profile data element
  identifier   : to identify profile data element
  info         : pointer to store value and length of data element
                 (size of buffer allocated should atleast be the max size of 
                  the parameter which needs to be fetched)

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS            : On successful operation
  DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED : Library is not initialized
  DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL     : Invalid handle 
  DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT    : Invalid identifier
  DS_PROFILE_REG_RESULT_ERR_LEN_INVALID    : Buffer size less than required
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Invalid tech type 
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM  : Invalid profile number
  DS_PROFILE_REG_RESULT_FAIL               : On general errors 
SIDE EFFECTS 
 
===========================================================================*/
ds_profile_status_etype ds_profile_get_param (  
  ds_profile_hndl_type        hndl,
  ds_profile_identifier_type  identifier,
  ds_profile_info_type       *info
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_get_param: ENTRY",0);

  /* Validate lib state */
  if( !LIB_STATE_IS_VALID( lib_state ) )
  {
    DS_PROFILE_LOGE( "_get_param: FAIL lib not inited ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED; 
    goto ret_err;
  }
  /* Validate handle */
  if ( !HNDL_IS_VALID( hndl )  ) 
  {
    DS_PROFILE_LOGE( "_get_param: INVAL hndl",0);
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL;
    goto ret_err;
  }

  ACQ_LOCK( "_get_param" );
  /*-------------------------------------------------------------------- 
    Call internal function, depending on tech and identifier
    calls tech-specific get function
  --------------------------------------------------------------------*/
  if ( ( return_status = dsi_profile_get_param( hndl, identifier, info ) ) 
       !=  DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE("_get_param: FAIL internal get function", 0 );
    REL_LOCK( "_get_param" );
    goto ret_err;
  }

  REL_LOCK( "_get_param" );
  DS_PROFILE_LOGD( "_get_param: EXIT with SUCCESS ", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;

ret_err:
  DS_PROFILE_LOGD( "_get_param: EXIT with ERR", 0 );
  return return_status;
}

/*===========================================================================
FUNCTION DS_PROFILE_DELETE

DESCRIPTION
  This is used to reset a profile to undefined and return to free pool. Not
  all technology types support this operation.

PARAMETERS
  tech : technology type
  num  : profile number

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS  : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Invalid tech type
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM  : Invalid profile number
  DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED : Library not initialized
  DS_PROFILE_REG_RESULT_ERR_INVAL_OP : Operation not supported for tech type
  DS_PROFILE_REG_RESULT_FAIL     : On general errors. This return code 
                                   provides blanket coverage
SIDE EFFECTS 
  none
===========================================================================*/
ds_profile_status_etype ds_profile_delete ( 
  ds_profile_tech_etype  tech,
  ds_profile_num_type    num
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_delete: ENTRY",0);

  /* Validate lib state */
  if( !LIB_STATE_IS_VALID( lib_state ) )
  {
    DS_PROFILE_LOGE( "_delete: FAIL lib not inited ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED; 
    goto ret_err;
  }

  /* Validate tech type */
  if ( !TECH_IS_VALID( tech ) ) 
  {
    DS_PROFILE_LOGE( "_delete: FAIL invalid tech type ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
    goto ret_err;
  }

  /* Validate profile number */
  if( ( return_status = dsi_profile_validate_profile_num( tech, num ) ) !=
      DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_delete: FAIL invalid profile number ", 0 );
    goto ret_err;
  }

  ACQ_LOCK( "_delete" );
  /*-------------------------------------------------------------------- 
    Call internal function which depending on tech calls the
    tech specific delete function
  --------------------------------------------------------------------*/
  if ( ( return_status = dsi_profile_delete( tech, num ) ) !=  
       DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE("_delete: FAIL internal delete function", 0 );
    REL_LOCK( "_delete" );
    goto ret_err;
  }

  REL_LOCK( "_delete" );
  DS_PROFILE_LOGD( "_delete: EXIT with SUCCESS ",0);
  return DS_PROFILE_REG_RESULT_SUCCESS;

ret_err:
  DS_PROFILE_LOGD( "_delete: EXIT with ERR", 0 );
  return return_status;
} 

/*===========================================================================
FUNCTION DS_PROFILE_CREATE

DESCRIPTION
  This function is used to return a profile number from a pool of free
  profiles. Not all technology types support this operation.

PARAMETERS
  tech  : technology type
  num   : pointer to return profile number of profile created

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS  : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Invalid tech type
  DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED : Library not initialized
  DS_PROFILE_REG_RESULT_ERR_INVAL_OP : Operation not supported for tech type
  DS_PROFILE_REG_RESULT_FAIL     : On general errors. This return code 
                                   provides blanket coverage
SIDE EFFECTS 
===========================================================================*/
ds_profile_status_etype ds_profile_create ( 
  ds_profile_tech_etype  tech,
  ds_profile_num_type   *num
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_create: ENTRY",0);

  /* Validate lib state */
  if( !LIB_STATE_IS_VALID( lib_state ) )
  {
    DS_PROFILE_LOGE( "_create: FAIL lib not inited ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED; 
    goto ret_err;
  }

  /* Validate tech type */
  if ( !TECH_IS_VALID( tech ) ) 
  {
    DS_PROFILE_LOGE( "_create: FAIL invalid tech type ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
    goto ret_err;
  }

  if ( num == NULL)
  {
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL;
    goto ret_err;
  }

  ACQ_LOCK( "_create" );
  /*-------------------------------------------------------------------- 
    Call internal function which depending on tech calls the
    tech specific create function
  --------------------------------------------------------------------*/
  if ( ( return_status = dsi_profile_create( tech, num ) ) !=  
       DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE("_create: FAIL internal delete function", 0 );
    REL_LOCK( "_create" );
    goto ret_err;
  }

  REL_LOCK( "_create" );
  DS_PROFILE_LOGD( "_create: EXIT with SUCCESS ", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;

ret_err:
  DS_PROFILE_LOGD( "_create: EXIT with ERR", 0 );
  return return_status;
}

/*===========================================================================
FUNCTION DS_PROFILE_RESET_PARAM_TO_INVALID

DESCRIPTION
  This function resets the value of parameter in a profile to default. It
  directly changes the value on modem, so begin/end transaction need not
  be called before/after this function.

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
ds_profile_status_etype ds_profile_reset_param_to_invalid (  
  ds_profile_tech_etype       tech,
  ds_profile_num_type         num,
  ds_profile_identifier_type  ident
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  /* Validate tech type */
  if ( !TECH_IS_VALID( tech ) ) 
  {
    DS_PROFILE_LOGE( "_reset_param: FAIL invalid tech type ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
    goto ret_err;
  }

  /* Validate profile number */
  if( ( return_status = dsi_profile_validate_profile_num( tech, num ) )
      != DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_reset_param: FAIL invalid profile number ", 0 );
    goto ret_err;
  }

  ACQ_LOCK( "_reset_param" );
  /*-------------------------------------------------------------------- 
    Call internal function which depending on tech calls the
    tech specific reset function
  --------------------------------------------------------------------*/
  if ( ( return_status = dsi_profile_reset_param( tech, num, ident ) ) 
       !=  DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE("_reset_param: FAIL internal reset_param function", 0 );
    REL_LOCK( "_reset_param" );
    goto ret_err;
  }

  REL_LOCK( "_reset_param" );
  DS_PROFILE_LOGD( "_reset_param: EXIT with SUCCESS ", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;

ret_err:
  DS_PROFILE_LOGD( "_reset_param: EXIT with ERR", 0 );
  return return_status;
}

/*===========================================================================
FUNCTION DS_PROFILE_RESET_PROFILE_TO_DEFAULT

DESCRIPTION
  This function resets all the parameters of the profile to default. It
  directly changes the value on modem, so begin/end transaction need not
  be called before/after this function.

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
ds_profile_status_etype ds_profile_reset_profile_to_default (  
  ds_profile_tech_etype  tech,
  ds_profile_num_type    num
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  /* Validate tech type */
  if ( !TECH_IS_VALID( tech ) ) 
  {
    DS_PROFILE_LOGE( "_reset_profile_to_default: FAIL invalid tech type ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
    goto ret_err;
  }

  /* Validate profile number */
  if( ( return_status = dsi_profile_validate_profile_num( tech, num ) )
      != DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_reset_profile_to_default: FAIL invalid profile number ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM;
    goto ret_err;
  }

  ACQ_LOCK( "_reset_profile_to_default" );
  /*-------------------------------------------------------------------- 
    Call internal function which depending on tech calls the
    tech specific reset_profile function
  --------------------------------------------------------------------*/
  if ( ( return_status = dsi_profile_reset_profile_to_default( tech, num ) ) 
       !=  DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE("_reset_profile_to_default: FAIL internal reset_profile function", 0 );
    REL_LOCK( "_reset_profile_to_default" );
    goto ret_err;
  }

  REL_LOCK( "_reset_profile_to_default" );
  DS_PROFILE_LOGD( "_reset_profile_to_default: EXIT with SUCCESS ", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS; 

ret_err:
  DS_PROFILE_LOGD( "_reset_profile_to_default: EXIT with ERR", 0 );
  return return_status;
}

/*===========================================================================
FUNCTION DS_PROFILE_SET_DEFAULT_PROFILE_NUM

DESCRIPTION
  This function sets the given profile number as default profile for the
  family of the specified tech.

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
ds_profile_status_etype ds_profile_set_default_profile_num (  
  ds_profile_tech_etype  tech,
  uint32                 family, 
  ds_profile_num_type    num
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  /* Validate tech type */
  if ( !TECH_IS_VALID( tech ) ) 
  {
    DS_PROFILE_LOGE( "_set_default_profile_num: FAIL invalid tech type ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
    goto ret_err;
  }

  /* Validate profile number */
  if( ( return_status = dsi_profile_validate_profile_num( tech, num ) )
      != DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE( "_set_default_profile_num: FAIL invalid profile number ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM;
    goto ret_err;
  }

  ACQ_LOCK( "_set_default_profile_num" );
  /*-------------------------------------------------------------------- 
    Call internal function which depending on tech calls the
    tech specific function
  --------------------------------------------------------------------*/
  if ( ( return_status = dsi_profile_set_default_profile( tech, family, num ) ) 
       !=  DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE("_set_default_profile_num: FAIL internal reset_profile function", 0 );
    REL_LOCK( "_set_default_profile_num" );
    goto ret_err;
  }

  REL_LOCK( "_set_default_profile_num" );
  DS_PROFILE_LOGD( "_set_default_profile_num: EXIT with SUCCESS ", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;   

ret_err:
  DS_PROFILE_LOGD( "_set_default_profile_num: EXIT with ERR", 0 );
  return return_status;
}

/*===========================================================================
FUNCTION DS_PROFILE_GET_DEFAULT_PROFILE_NUM

DESCRIPTION
  This function gets the default profile number for the family of the
  specified tech.

PARAMETERS
  tech   : technology type
  family : profile family
  num    : pointer to store default profile number

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS   : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE : Profile type is invalid
  DS_PROFILE_REG_RESULT_ERR_INVAL_OP : Operation not supported for tech type
  DS_PROFILE_REG_RESULT_FAIL      : On general errors. This return 
                                    code provides blanket coverage
SIDE EFFECTS 
  
===========================================================================*/
ds_profile_status_etype ds_profile_get_default_profile_num (  
  ds_profile_tech_etype   tech,
  uint32                  family, 
  ds_profile_num_type    *num
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  /* Validate tech type */
  if ( !TECH_IS_VALID( tech ) ) 
  {
    DS_PROFILE_LOGE( "_get_default_profile_num: FAIL invalid tech type ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
    goto ret_err;
  }

  if ( num == NULL )
  {
    DS_PROFILE_LOGE( "_get_default_profile_num: FAIL num ptr NULL", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL;
    goto ret_err;
  }

  ACQ_LOCK( "_get_default_profile_num" );
  /*-------------------------------------------------------------------- 
    Call internal function which depending on tech calls the
    tech specific function
  --------------------------------------------------------------------*/
  if ( ( return_status = dsi_profile_get_default_profile( tech, family, num ) ) 
       !=  DS_PROFILE_REG_RESULT_SUCCESS )
  {
    DS_PROFILE_LOGE("_get_default_profile_num: FAIL internal reset_profile function", 0 );
    REL_LOCK( "_get_default_profile_num" );
    goto ret_err;
  }

  REL_LOCK( "_get_default_profile_num" );
  DS_PROFILE_LOGD( "_get_default_profile_num: EXIT with SUCCESS ", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;   

ret_err:
  DS_PROFILE_LOGD( "_get_default_profile_num: EXIT with ERR", 0 );
  return return_status;
}


/*=========================================================================
FUNCTION DS_PROFILE_GET_MAX_NUM

DESCRIPTION
  This function returns the maximum number of Profiles possible for a 
  given technology type  

PARAMETERS
  tech    : technology type
  max_num : pointer to store maximum number of profiles possible

DEPENDENCIES 
  
RETURN VALUE 
  DS_PROFILE_REG_RESULT_SUCCESS   : On successful operation
  DS_PROFILE_REG_RESULT_ERR_INVALID_PROFILE_TYPE : Profile type is invalid
  DS_PROFILE_REG_RESULT_FAIL      : On general errors. This return 
                                    code provides blanket coverage
SIDE EFFECTS 
  
===========================================================================*/
ds_profile_status_etype ds_profile_get_max_num(
  ds_profile_tech_etype  tech,
  uint32                *max_num
)
{
  uint16 min_num = 0;
  uint16 mx = 0;
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_get_max_num: ENTRY",0);

  if ( max_num == NULL  ) 
  {
    DS_PROFILE_LOGE( "_get_max_num: INVAL max_num ptr", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL;
    goto ret_err;
  }

  /* Validate tech */
  if ( !TECH_IS_VALID( tech ) ) 
  {
    DS_PROFILE_LOGE( "_get_max_num: FAIL invalid tech type ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
    goto ret_err;
  }

  ACQ_LOCK( "_get_max_num" );
  /* Call internal function */
  dsi_profile_get_profile_num_range(tech, &min_num, &mx);
  if ( mx == 0 && min_num == 0 )
  {
    REL_LOCK( "_get_max_num" );  
    DS_PROFILE_LOGD( "_get_max_num: EXIT with ERR ",0);
    goto ret_err;
  }
  *max_num = (uint32)mx;

  REL_LOCK( "_get_max_num" );  
  DS_PROFILE_LOGD( "_get_max_num: EXIT with SUCCESS ",0);
  return DS_PROFILE_REG_RESULT_SUCCESS;

ret_err:
  DS_PROFILE_LOGE( "_get_max_num: EXIT with ERR", 0 );
  return return_status;
}

/*===========================================================================
FUNCTION DS_PROFILE_GET_SUPPORTED_TYPE 
 
---NOT EXPOSED AS AN API 
===========================================================================*/
ds_profile_status_etype ds_profile_get_supported_type (  
  uint32                 *num,
  ds_profile_tech_etype  *tech
)
{
  if ( num == NULL || tech == NULL )
  {
    return DS_PROFILE_REG_RESULT_ERR_INVAL;
  }
  if (dsi_profile_get_supported_type(num, tech) != 
      DS_PROFILE_REG_RESULT_SUCCESS)
  {
    DS_PROFILE_LOGD( "_get_supported_type: EXIT with ERR ", 0 );
    return DS_PROFILE_REG_RESULT_FAIL;
  }
  DS_PROFILE_LOGD( "_get_supported_type: EXIT with SUCCESS ", 0 );
  return DS_PROFILE_REG_RESULT_SUCCESS;
}


/*===========================================================================
FUNCTION DS_PROFILE_GET_LIST_ITR

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
ds_profile_status_etype ds_profile_get_list_itr ( 
  ds_profile_tech_etype  tech,
  ds_profile_list_type  *lst,
  ds_profile_itr_type   *itr
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_get_list_itr: ENTRY", 0 );

  /* Validate tech type */
  if ( !TECH_IS_VALID( tech ) ) 
  {
    DS_PROFILE_LOGE( "_get_list_itr: FAIL invalid tech type ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE;
    goto ret_err;
  }

  /* Validate list_type and itr_type */
  if ( !LIST_TYPE_IS_VALID( lst ) )
  {
    DS_PROFILE_LOGE( "_get_list_itr: list_type NULL ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL; 
    goto ret_err;
  }

  /* Validate itr_type */
  if ( !ITR_IS_VALID( itr ) )
  {
    DS_PROFILE_LOGE( "_get_list_itr: itr NULL ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL; 
    goto ret_err;
  }

  ACQ_LOCK( "_get_list_itr" );
  if ( (return_status = dsi_profile_get_list_itr( tech, lst, itr ) )
      != DS_PROFILE_REG_RESULT_SUCCESS)
  {
    REL_LOCK( "_get_list_itr" );
    DS_PROFILE_LOGE( "_get_list_itr: EXIT with ERR  ", 0 ); 
    goto ret_err;
  }

  REL_LOCK( "_get_list_itr" );
  DS_PROFILE_LOGD( "_get_list_itr: EXIT with SUCCESS ",0);
  return return_status;

ret_err:
  DS_PROFILE_LOGD( "_get_list_itr: EXIT with ERR", 0 );
  return return_status;
}

/*===========================================================================
FUNCTION DS_PROFILE_ITR_NEXT

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
ds_profile_status_etype ds_profile_itr_next (  
  ds_profile_itr_type   itr
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_itr_next: ENTRY", 0 );

  /* Validate itr_type */
  if ( !ITR_IS_VALID( itr ) )
  {
    DS_PROFILE_LOGE( "_itr_next: itr NULL ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL; 
    goto ret_err;
  }

  ACQ_LOCK( "_itr_next" );
  if ( (return_status = dsi_profile_itr_next( itr ) )
      != DS_PROFILE_REG_RESULT_SUCCESS)
  {
    REL_LOCK( "_itr_next" );
    DS_PROFILE_LOGE( "_itr_next: EXIT with ERR ", 0 ); 
    goto ret_err;
  }

  REL_LOCK( "_itr_next" );
  DS_PROFILE_LOGD( "_itr_next: EXIT with SUCCESS ",0);
  return return_status;

ret_err:
  DS_PROFILE_LOGD( "_itr_next: EXIT with ERR", 0 );
  return return_status;
}

/*===========================================================================
FUNCTION DS_PROFILE_ITR_FIRST

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
ds_profile_status_etype ds_profile_itr_first ( 
  ds_profile_itr_type   itr
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_itr_first: ENTRY",0);

  /* Validate itr_type */
  if ( !ITR_IS_VALID( itr ) )
  {
    DS_PROFILE_LOGE( "_itr_first: itr NULL ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL; 
    goto ret_err;
  }

  ACQ_LOCK( "_itr_first" );

  if ( (return_status = dsi_profile_itr_first( itr ) )
      != DS_PROFILE_REG_RESULT_SUCCESS)
  {
    REL_LOCK( "_itr_first" );
    DS_PROFILE_LOGE( "_itr_first: EXIT with ERR", 0 ); 
    goto ret_err;
  }

  REL_LOCK( "_itr_first" );
  DS_PROFILE_LOGD( "_itr_first: EXIT with SUCCESS ",0);
  return return_status;

ret_err:
  DS_PROFILE_LOGD( "_itr_first: EXIT with ERR", 0 );
  return return_status;
}

/*===========================================================================
FUNCTION DS_PROFILE_GET_INFO_BY_ITR

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
ds_profile_status_etype ds_profile_get_info_by_itr ( 
  ds_profile_itr_type         itr,
  ds_profile_list_info_type  *list_info
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_get_info_by_itr: ENTRY", 0 );

  /* Validate itr_type */
  if ( !ITR_IS_VALID( itr ) )
  {
    DS_PROFILE_LOGE( "_get_info_by_itr: itr NULL ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL; 
    goto ret_err;
  }

  if ( list_info == NULL)
  {
    DS_PROFILE_LOGE( "_get_info_by_itr: list_info ptr NULL", 0 ); 
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL;
    goto ret_err;
  }

  ACQ_LOCK( "_get_info_by_itr" );
  if ( (return_status = dsi_profile_get_info_by_itr( itr, list_info ) )
      != DS_PROFILE_REG_RESULT_SUCCESS)
  {
    REL_LOCK( "_get_info_by_itr" );
    goto ret_err;
  }

  REL_LOCK( "_get_info_by_itr" );
  DS_PROFILE_LOGD( "_get_info_by_itr: EXIT with SUCCESS ", 0 );
  return return_status;

ret_err:
  DS_PROFILE_LOGD( "_get_info_by_itr: EXIT with ERR", 0 );
  return return_status;
}

/*===========================================================================
FUNCTION DS_PROFILE_ITR_DESTROY

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
ds_profile_status_etype ds_profile_itr_destroy ( 
  ds_profile_itr_type   itr
)
{
  ds_profile_status_etype return_status = DS_PROFILE_REG_RESULT_FAIL;
  DS_PROFILE_LOGD( "_itr_destroy: ENTRY", 0 );

  /* Validate itr_type */
  if ( !ITR_IS_VALID( itr ) )
  {
    DS_PROFILE_LOGE( "_itr_destroy: itr NULL ", 0 );
    return_status = DS_PROFILE_REG_RESULT_ERR_INVAL; 
    goto ret_err;
  }

  ACQ_LOCK( "_itr_destroy" );
  if ( (return_status = dsi_profile_itr_destroy( itr ) )
      != DS_PROFILE_REG_RESULT_SUCCESS)
  {
    REL_LOCK( "_itr_destroy" );
    goto ret_err;
  }

  REL_LOCK( "_itr_destroy" );
  DS_PROFILE_LOGD( "_itr_destroy: EXIT with SUCCESS ", 0 );
  return return_status;

ret_err:
  DS_PROFILE_LOGD( "_itr_destroy: EXIT with ERR", 0 );
  return return_status;
}

/*lint -restore Restore lint error 655*/


