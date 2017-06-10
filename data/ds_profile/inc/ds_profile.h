/******************************************************************************
  @file    ds_profile.h
  @brief   Data Services Profile Registry API definitions

  DESCRIPTION
  This file contains common, external header file definitions for Data Services 
  Profile Registry API. This library is thread-safe

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  -----------------------------------------------------------------------------
  Copyright (C) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
******************************************************************************/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/api/data/main/latest/ds_profile.h#1 $ $DateTime: 2009/09/30 19:08:30 $ $Author: vsheth $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#ifndef DS_PROFILE_H
#define DS_PROFILE_H

#include "comdef.h"

/* Maximum number of handles supported */
#define DS_PROFILE_MAX_NUM_HNDL 32

/* Profile parameter identifier type   */
typedef uint32  ds_profile_identifier_type;    

/* Profile number */
typedef uint16  ds_profile_num_type;           

/* Profile handle */
typedef void*   ds_profile_hndl_type;          

/* Iterator used in list functions     */
typedef void*   ds_profile_itr_type; 

/* status return values, success/error */
typedef enum   
{
  DS_PROFILE_REG_RESULT_SUCCESS    =       0,  /* Successful operation       */
  DS_PROFILE_REG_RESULT_FAIL,                  /* General failure in the lib */
  
  DS_PROFILE_REG_RESULT_ERR_INVAL_HNDL,        /* Invalid profile handle     */ 
  DS_PROFILE_REG_RESULT_ERR_INVAL_OP,          /* Operation not supported    */
  
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_TYPE,/* Invalid tech type          */
  DS_PROFILE_REG_RESULT_ERR_INVAL_PROFILE_NUM, /* Invalid profile number     */
  DS_PROFILE_REG_RESULT_ERR_INVAL_IDENT,       /* Invalid identifier         */
  DS_PROFILE_REG_RESULT_ERR_INVAL,             /* other invalid arg          */

  DS_PROFILE_REG_RESULT_ERR_LIB_NOT_INITED,    /* lib not initialized        */

  DS_PROFILE_REG_RESULT_ERR_LEN_INVALID,       /* for get_param, buff size 
                                                  cannot be less than max
                                                  for set_param buff size 
                                                  cannot be greater than max */
  
  DS_PROFILE_REG_RESULT_LIST_END,              /* End of list reached, return 
                                                  value for _itr_next        */
  
  DS_PROFILE_REG_3GPP_SPEC_MIN = 0x1000,       /* Offset for UMTS Tech 
                                                  specific errors            */
  DS_PROFILE_REG_3GPP_INVAL_PROFILE_FAMILY,
  DS_PROFILE_REG_3GPP_ACCESS_ERR,
  DS_PROFILE_REG_3GPP_CONTEXT_NOT_DEFINED,
  DS_PROFILE_REG_3GPP_VALID_FLAG_NOT_SET,
  DS_PROFILE_REG_3GPP_READ_ONLY_FLAG_SET,
  DS_PROFILE_REG_3GPP_ERR_OUT_OF_PROFILES,
  DS_PROFILE_REG_3GPP_SPEC_MAX = 0x10FF,
  
  DS_PROFILE_REG_3GPP2_SPEC_MIN = 0x1100,       /* Offset for CDMA Tech 
                                                  specific errors            */
  DS_PROFILE_REG_3GPP2_ERR_INVALID_IDENT_FOR_PROFILE, /* To specify that 
                                                  identifier is not valid for 
                                                  the profile                */
  DS_PROFILE_REG_3GPP2_SPEC_MAX = 0x11FF,
  
  DS_PROFILE_REG_RESULT_MAX    = 0xFFFF
} ds_profile_status_etype;

/* Profile technology type values */
typedef enum
{
  DS_PROFILE_TECH_MIN     = 0x00,
  DS_PROFILE_TECH_3GPP    = DS_PROFILE_TECH_MIN,
  DS_PROFILE_TECH_3GPP2   = 0x01,
  DS_PROFILE_TECH_MAX     = 0x02,
  DS_PROFILE_TECH_INVALID = 0xFF
}ds_profile_tech_etype;

/* Transaction type values (read/write)     */
/* DS_PROFILE_TRN_VALID is for internal use */
typedef enum
{
  DS_PROFILE_TRN_READ   = 0x01,
  DS_PROFILE_TRN_RW     = 0x03,
  DS_PROFILE_TRN_VALID  = DS_PROFILE_TRN_READ | DS_PROFILE_TRN_RW
}ds_profile_trn_etype;

/* Action on end_transaction, commit or cancel */
typedef enum 
{
  DS_PROFILE_ACTION_MIN    = 0x0,
  DS_PROFILE_ACTION_COMMIT = 0x1,
  DS_PROFILE_ACTION_CANCEL = 0x2,
  DS_PROFILE_ACTION_MAX    = 0xFF
}ds_profile_action_etype;

/* definition for list, either all profiles or search on <key, value> */
typedef enum
{
  DS_PROFILE_LIST_DFN_MIN           = 0,
  DS_PROFILE_LIST_ALL_PROFILES      = 1,
  DS_PROFILE_LIST_SEARCH_PROFILES   = 2,
  DS_PROFILE_LIST_DFN_MAX           = 0xFF
} ds_profile_list_dfn_etype;

/* structure to store parameter value for get/set operations */
typedef struct 
{
  void     *buf;
  uint16   len;
} ds_profile_info_type;

/* Node for list */
/* for name, allocate memory as specified by profile_name_max_len
   in corresponding tech header */
typedef struct
{
  ds_profile_num_type   num;
  ds_profile_info_type *name;
} ds_profile_list_info_type;

/* type of list to be returned, depending on dfn */
typedef struct
{
  ds_profile_list_dfn_etype  dfn;
  ds_profile_identifier_type ident;
  ds_profile_info_type       info;
} ds_profile_list_type;

/*---------------------------------------------------------------------------
                       PUBLIC ROUTINES
---------------------------------------------------------------------------*/
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
);

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
);

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
  ds_profile_hndl_type    hndl,
  ds_profile_action_etype act
);

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
);

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
);

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
  ds_profile_hndl_type        profile_hndl,
  ds_profile_identifier_type  identifier,
  ds_profile_info_type       *info
);

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
  ds_profile_hndl_type         profile_hndl,
  ds_profile_identifier_type   identifier,
  const ds_profile_info_type  *info
);

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
);

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
);

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
);

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
);

/*===========================================================================
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
ds_profile_status_etype ds_profile_get_max_num (  
  ds_profile_tech_etype   tech,
  uint32                 *max_num
);

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
);

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
);

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
);

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
);

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
);

#endif /* DS_PROFILE_H */
