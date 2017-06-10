/*!
  @file
  qcril_dsi.c

  @brief

*/

/*===========================================================================

  Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //depot/restricted/linux/android/ril/qcril_dsi.c $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/21/09   sm      Passes auth pref to dss from ril
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
04/05/09   fc      Cleanup log macros.
01/26/08   fc      Logged assertion info.
12/23/08   asn     Various fixes
12/15/08   asn     Various fixes and logging
12/08/08   pg      Added multi-mode data call hook up.
11/14/08   sm      Added temp CDMA data support.


===========================================================================*/

#include "qcril_data.h"
#include "qcril_datai.h"
#include "assert.h"
#include "dsc_main.h"

#define DSI_UMTS_DEFAULT_PDP_PROF 1

/* simple unit test */
#undef DSI_TEST_MODE_FAIL_CALL_CONTINUOUS
//#define DSI_TEST_MODE_FAIL_CALL_CONTINUOUS 1

#define DSI_ASSERT( cond, msg ) \
  if ( !( cond ) ) \
  { \
    QCRIL_LOG_FATAL( "%s", "*************ASSERTION FAILED (soft)***************" ); \
    QCRIL_LOG_FATAL( "File: %s, Line: %d, [%s]", __FILE__, __LINE__, msg );         \
    QCRIL_LOG_FATAL( "%s", "***************************************************" ); \
  }

/* Hard Assert */
#define DSI_ASSERT_H( cond, msg ) \
  if ( !( cond ) ) \
  { \
    QCRIL_LOG_FATAL( "%s", "*************ASSERTION FAILED (soft)***************" ); \
    QCRIL_LOG_FATAL( "File: %s, Line: %d, [%s]", __FILE__, __LINE__, msg );         \
    QCRIL_LOG_FATAL( "%s", "***************************************************" ); \
    assert(0); \
  }

#define DSI_CHECK_ERR_COND_THEN_JUMP( cond, err_msg, errlabel ) \
  if ( (cond) ) \
  { \
    QCRIL_LOG_ERROR( "%s", err_msg ); \
    goto errlabel; \
  }

/*---------------------------------------------------------------------------
                           EXTERNs
---------------------------------------------------------------------------*/
//extern int dsc_main( int, char ** );

/*---------------------------------------------------------------------------
                           DECLARATIONS
---------------------------------------------------------------------------*/
#define CDMA_TECH_TYPE    0x00
#define UMTS_TECH_TYPE    0x01

int technology = UMTS_TECH_TYPE ;


typedef enum
{
  DS_CALL_STATE_INVALID                   =  0,
  DS_CALL_STATE_ACTIVATION_IN_PROGRESS    =  1,
  DS_CALL_STATE_CONNECTED                 =  2,
  DS_CALL_STATE_DEACTIVATION_IN_PROGRESS  =  3,
  DS_CALL_STATE_DISCONNECTED              =  4
} dsi_call_state_enum_t;

typedef struct 
{
  /* Private data for DSI module */
  dsi_call_state_enum_t     call_state;
  short int                 dsnet_hndl;
  dss_net_policy_info_type  dsnet_policy;
  dss_iface_id_type         dsnet_iface_id;
  dss_iface_ioctl_device_name_type 
                            dev_name;
  dss_net_down_reason_type  dsnet_fail_cause;
  dss_umts_apn_type         apn;
  dss_iface_ioctl_ipv4_addr_type
                            ip_addr;
#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
  dss_iface_ioctl_ipv4_addr_type  gateway_addr;
  dss_iface_ioctl_ipv4_addr_type  prim_dns_addr;
  dss_iface_ioctl_ipv4_addr_type  sec_dns_addr;
#endif /* (RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6)*/

  void                     *self;
} dsi_priv_t;

typedef struct 
{

  dsi_vtbl_t      fntbl;
  
  dsi_net_ev_cb       net_ev_cb;

  void (*dsi_net_ev_cb)( 
    dsi_hndl_t     hndl, 
    void          *user_data,
    dsi_net_evt_t  evt
  );
  
  void           *user_data;

  dsi_priv_t      priv;

  void           *self;

} dsi_store_t;

/*! 
  @brief:
  dsi_store_ptr: Pointer to the dsi store.
  dsnet_hndl: dsnet handle stored in this table to 
  make sure we release the dss handle only after 
  the dss iface is in down state(Network is Down)

  @notes: 
  This table entry will be used as a cookie, 
  instead of using the store pointer itself.This will 
  help us deterministically figure 
  out if the store is valid or not. 
*/
typedef struct 
{
  unsigned char      is_valid; 
  short int          dsnet_hndl;
  void               *dsi_store_ptr;
}dsi_store_tbl_t;

static dsi_store_tbl_t  dsi_store_table[DS_MAX_DATA_CALLS];

#ifdef DSI_TEST_MODE_FAIL_CALL_CONTINUOUS
  static dsi_store_t test_store_1;
#endif

/*===========================================================================

                    PRIVATE MACROS

===========================================================================*/
#define DSI_IS_HNDL_VALID( hndl )   \
    ( ( hndl != NULL) && ( hndl->self == hndl ) )
#define DSI_IS_IDENT_VALID( ident ) ( 1 )

/*===========================================================================

                    HELPER FUNCTIONS

===========================================================================*/

/*Clean up the static store table entry*/
static void dsi_cleanup_store_tbl(short int index)
{
  QCRIL_LOG_DEBUG( "%s", "dsi_cleanup_store_tbl: ENTRY" );
  if (index >= 0 && index < DS_MAX_DATA_CALLS)
  {
    if (dsi_store_table[index].dsi_store_ptr != NULL)
    {
      QCRIL_LOG_DEBUG( "%s", "dsi_cleanup_store_tbl: Freeing up the store pointer" );
      free(dsi_store_table[index].dsi_store_ptr);
    }

    dsi_store_table[index].dsi_store_ptr = NULL;
    dsi_store_table[index].dsnet_hndl = -1;
    dsi_store_table[index].is_valid = FALSE;
  }
  else
  {
    QCRIL_LOG_ERROR( "%s", "dsi_cleanup_store_tbl: Invalid index sent" );
  }
  return;
}
/*===========================================================================

                    PRIVATE FUNCTIONS

===========================================================================*/
static int start_data_call(
  dsi_hndl_t hndl
)
{
  dsi_store_t       *st_hndl;
  signed short       dss_errno = 0, rc;
  
  QCRIL_LOG_DEBUG( "%s", "start_data_call: ENTRY" );

  st_hndl      = ( (dsi_store_t *) hndl );
  
  if ( !( DSI_IS_HNDL_VALID( st_hndl ) ) )
  {
    QCRIL_LOG_ERROR( "cannot set, inval arg, st_hndl [%x]", (unsigned int)st_hndl );
    goto err_label;
  }
  
#ifdef DSI_TEST_MODE_FAIL_CALL_CONTINUOUS
  QCRIL_LOG_VERBOSE( "%s", "test mode, fail data call" );
  goto err_label;
#endif

  QCRIL_LOG_VERBOSE( "bringup data call, dsnet_hndl [%d]", 
                     st_hndl->priv.dsnet_hndl );

  /* Call DSNET Start */
  if ( (  ( rc = dsnet_start( st_hndl->priv.dsnet_hndl, &dss_errno ) ) != DSS_SUCCESS ) &&
          ( dss_errno != DS_EWOULDBLOCK ) ) 
  {
    QCRIL_LOG_ERROR( "dsnet ret err, st_hndl [%#x], dss_errno [%d]",
                     (unsigned int)st_hndl, dss_errno );
    goto err_label;
  }

  /* Data connection is already established */
  else if ( rc == DSS_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s", "Data call already active, call user cb" );
    DSI_ASSERT( st_hndl->net_ev_cb != NULL, "validate dss net cb" );
    /* don't call user callback, dss already does */
    /* st_hndl->net_ev_cb( st_hndl, st_hndl->user_data, DSI_EVT_NET_IS_CONN ); */
  }
  else if ( ( rc == DSS_ERROR ) && ( dss_errno == DS_EWOULDBLOCK ) )
  {
    QCRIL_LOG_DEBUG( "%s", "Data call setup has started..." );
  }
  else
  {
    QCRIL_LOG_DEBUG( "unable to hndl dsnet ret code [%d], dss_err [%d]", rc, dss_errno );
    DSI_ASSERT(0, "start_data_call: ASSERT on dsnet_start() ret_code/dss_errno" );
  }

  QCRIL_LOG_DEBUG( "%s", "start_data_call: EXIT with suc" );
  return DSI_SUCCESS;

err_label:
  QCRIL_LOG_DEBUG( "%s", "start_data_call: EXIT with err" );
  return DSI_ERROR;
}

static int stop_data_call(
  dsi_hndl_t hndl
)
{
  dsi_store_t       *st_hndl;
  signed short       dss_errno = 0, rc;

  QCRIL_LOG_DEBUG( "%s", "stop_data_call: ENTRY" );

  st_hndl      = ( (dsi_store_t *) hndl );
  
  if ( !( DSI_IS_HNDL_VALID( st_hndl ) ) )
  {
    QCRIL_LOG_ERROR( "cannot stop, inval arg, st_hndl [%#x]",
                     (unsigned int)st_hndl );
    goto err_label;
  }

  QCRIL_LOG_DEBUG( "call_state [%d]", st_hndl->priv.call_state );

  switch ( st_hndl->priv.call_state ) 
  {

  case DS_CALL_STATE_ACTIVATION_IN_PROGRESS:
    QCRIL_LOG_ERROR( "call_state ACT IN PROGRESS, initiate teardown, dsnet_hndl [%d]", 
                     st_hndl->priv.dsnet_hndl );
    break;

  case DS_CALL_STATE_DISCONNECTED:
    st_hndl->priv.call_state = DS_CALL_STATE_INVALID;
    QCRIL_LOG_INFO( "call_state marked invalid, dsnet_hndl [%d]", 
                    st_hndl->priv.dsnet_hndl );
    break;

  case DS_CALL_STATE_DEACTIVATION_IN_PROGRESS:
    QCRIL_LOG_INFO( "deactivation in progress, dsnet_hndl [%d]", 
                    st_hndl->priv.dsnet_hndl );
    break;

  case DS_CALL_STATE_CONNECTED:
    QCRIL_LOG_INFO( "call in CONNECTED, initiate teardown, dsnet_hndl [%d]", 
                    st_hndl->priv.dsnet_hndl );
    break;

  default:
    QCRIL_LOG_ERROR( "invalid call state [%d], dsnet_hndl [%d]", 
                     st_hndl->priv.call_state, st_hndl->priv.dsnet_hndl );
    DSI_ASSERT( 0, "validate call_state" );
    goto err_label;
    break;
  }/* switch */

  /* Call DSNET Stop */
  if ( (  ( rc = dsnet_stop( st_hndl->priv.dsnet_hndl, &dss_errno ) ) != DSS_SUCCESS ) &&
          ( dss_errno == DS_EBADAPP ) ) 
  {
    QCRIL_LOG_ERROR( "dsnet ret err, st_hndl [%#x], dss_errno [%d]",
                     (unsigned int)st_hndl, dss_errno );
    goto err_label;
  }

  /* Data connection has been terminated */
  else if ( rc == DSS_SUCCESS )
  {
    QCRIL_LOG_DEBUG( "%s", "Data call already inactive, call user cb" );
    if ( st_hndl->net_ev_cb != NULL )
    {
      st_hndl->net_ev_cb( st_hndl, st_hndl->user_data, DSI_EVT_NET_NO_NET ); 
    }
    else
    {
      QCRIL_LOG_ERROR("%s", "st_hndl->net_ev_cb is set to NULL. "
                       "Cannot call callback.");
      goto err_label;
    }
  }
  /* Data Call teardown already started */
  else if ( ( rc == DSS_ERROR ) && ( dss_errno == DS_EWOULDBLOCK ) )
  {
    QCRIL_LOG_DEBUG( "%s", "Data call teardown has started..." );
    st_hndl->priv.call_state = DS_CALL_STATE_DEACTIVATION_IN_PROGRESS;
  }
  /* Data Call teardown was already issued */
  else if ( ( rc == DSS_ERROR ) && ( dss_errno == DS_ENETCLOSEINPROGRESS ) )
  {
    QCRIL_LOG_DEBUG( "%s", "Data call teardown was already issued" );
  }
  else 
  {
    QCRIL_LOG_DEBUG( "unable to handle dsnet ret code [%d], dss_err [%d]", rc, dss_errno );
    DSI_ASSERT(0, "stop_data_call: ASSERT on dsnet_stop() ret_code/dss_errno" );
  }

ret_label:
  QCRIL_LOG_DEBUG( "%s", "stop_data_call: EXIT with succ" );
  return DSI_SUCCESS;

err_label:
  QCRIL_LOG_DEBUG( "%s", "stop_data_call: EXIT with err" );
  return DSI_ERROR;
}

static int set_data_call_param(
  dsi_hndl_t        hndl,
  unsigned int      identifier,
  dsi_param_info_t *info
)
{
  dsi_store_t *st;
  signed short dss_errno = DSS_ERROR;
  unsigned int          i = 0;
  unsigned int profile_id_num = 0;   
  unsigned int auth_pref = -1;
  unsigned int copy_length = 0;
  QCRIL_LOG_DEBUG( "%s", "set_data_call_param: ENTRY" );

  st = ( (dsi_store_t *) hndl );

  if ( !DSI_IS_HNDL_VALID( st ) || !DSI_IS_IDENT_VALID ( identifier ) || ( info == NULL ) )
  {
    QCRIL_LOG_ERROR( "cannot set, inval arg, st_hndl [%#x], ident [%u], info [%x]",
                     (unsigned int)st, identifier, (unsigned int)info );
    goto err_label;
  }

  QCRIL_LOG_VERBOSE( "set param for dsi_hndl [%x], ident [%d], len [%d]",
                   (unsigned int)st, identifier, info->len );

  QCRIL_LOG_VERBOSE( "dsnet_policy: policy_flag [%d]", 
                     st->priv.dsnet_policy.policy_flag );

  QCRIL_LOG_VERBOSE( "dsnet_policy: pdp_profile_num [%d]", 
                     st->priv.dsnet_policy.umts.pdp_profile_num );

  switch( identifier )
  {
  case DATA_CALL_INFO_CDMA_PROFILE_IDX:
  case DATA_CALL_INFO_UMTS_PROFILE_IDX:

    if ( info->len > QCRIL_PROFILE_IDX_MAX_LEN ) 
    {
      QCRIL_LOG_ERROR( "cannot set profile id, too long, len [%d]",
                       info->len );
      goto err_label;
    }

    /* derive the decimal value of the profile*/
    profile_id_num = atoi(info->buf);

    if (profile_id_num <= 1000)
    {
      if (profile_id_num == 0)
      {
        break;
      }
      QCRIL_LOG_VERBOSE("not processing profile_id_num %d < 1000", profile_id_num);
      QCRIL_LOG_VERBOSE("%s", "default profile will be used");
      break;
    }
    else
    {
      profile_id_num -= 1000;
      QCRIL_LOG_VERBOSE("profile_id_num %d will be used (android provided %d)",
                        profile_id_num,
                        profile_id_num + 1000);
    }

    switch( identifier )
    {
    case DATA_CALL_INFO_CDMA_PROFILE_IDX:
      st->priv.dsnet_policy.cdma.data_session_profile_id = profile_id_num;
      break;
    case DATA_CALL_INFO_UMTS_PROFILE_IDX:
      st->priv.dsnet_policy.umts.pdp_profile_num = profile_id_num;
      break;
    default:
      DSI_ASSERT(0, "memory corruption");
    }

    QCRIL_LOG_VERBOSE("dsnet_policy profile_id [%d]", profile_id_num); 
    break;
  case DATA_CALL_INFO_APN_NAME:

    if ( info->len > DS_CALL_INFO_APN_MAX_LEN ) 
    {
      QCRIL_LOG_ERROR( "cannot set APN, too long, len [%d]",
                       info->len );
      goto err_label;
    }

    copy_length = MIN(info->len, 
                      sizeof(st->priv.dsnet_policy.umts.apn.name));
    memcpy( st->priv.dsnet_policy.umts.apn.name, info->buf, copy_length );
    st->priv.dsnet_policy.umts.apn.length          = copy_length;

    for ( i = 0; i < copy_length; i++ ) 
    {
      QCRIL_LOG_VERBOSE( "dsnet_policy APN[%d]=%c", 
                         i, st->priv.dsnet_policy.umts.apn.name[ i ] );
    }

    QCRIL_LOG_VERBOSE( "dsnet_policy APN len [%d]",
                        copy_length );
    break;
    case  DATA_CALL_INFO_USERNAME:

      if ( info->len > DS_CALL_INFO_USERNAME_MAX_LEN ) 
      {
        QCRIL_LOG_ERROR( "cannot set UserName, too long, len [%d]",
                         info->len );
        goto err_label;
      }
      copy_length = MIN(info->len, sizeof(st->priv.dsnet_policy.username.value));
      memcpy( st->priv.dsnet_policy.username.value, info->buf, copy_length );
      st->priv.dsnet_policy.username.length           = copy_length;

      for ( i = 0; i < copy_length; i++ ) 
      {
        QCRIL_LOG_VERBOSE( "dsnet_policy UserName[%d]=%c", 
                           i, st->priv.dsnet_policy.username.value[ i ] );
      }

      QCRIL_LOG_VERBOSE( "dsnet_policy UserName len [%d]",
                         copy_length);
      break;

    case  DATA_CALL_INFO_PASSWORD:

      if ( info->len > DS_CALL_INFO_PASSWORD_MAX_LEN ) 
      {
        QCRIL_LOG_ERROR( "cannot set Password, too long, len [%d]",
                         info->len );
        goto err_label;
      }
      copy_length = MIN(info->len, sizeof(st->priv.dsnet_policy.password.value));
      memcpy( st->priv.dsnet_policy.password.value, info->buf, copy_length );
      st->priv.dsnet_policy.password.length           = copy_length;

      for ( i = 0; i < copy_length; i++ ) 
      {
        QCRIL_LOG_VERBOSE( "dsnet_policy Password[%d]=%c", 
                           i, st->priv.dsnet_policy.password.value[ i ] );
      }

      QCRIL_LOG_VERBOSE( "dsnet_policy  Password len [%d]",
                          copy_length);
      break;
    case  DATA_CALL_INFO_AUTH_PREF:

      QCRIL_LOG_DEBUG( "%s","Setting the Authentication Preference. \n");

      if (info->len > 1)
      {
        QCRIL_LOG_ERROR( "%s","Invalid Authentication type string provided \n"); 
        goto err_label;
      }

      /* derive the decimal value of the authentication preference*/
      auth_pref = atoi ((char *)info->buf);

      switch(auth_pref)
      {
        case 0:
          st->priv.dsnet_policy.auth_pref = DSS_AUTH_PREF_PAP_CHAP_NOT_ALLOWED;
        break;
        case 1:
          st->priv.dsnet_policy.auth_pref = DSS_AUTH_PREF_PAP_ONLY_ALLOWED;
        break;
        case 2:
          st->priv.dsnet_policy.auth_pref = DSS_AUTH_PREF_CHAP_ONLY_ALLOWED;
        break;
        case 3:
          st->priv.dsnet_policy.auth_pref = DSS_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED;
        break;
        default:
          QCRIL_LOG_ERROR( "%s","Invalid Authentication type specified \n"); 
          goto err_label;
        break;
      }
   break;
  default:
      QCRIL_LOG_ERROR( "cannot set, unknown ident [%d]", identifier );
    goto err_label;
  }/* switch() */

  QCRIL_LOG_VERBOSE( "set net policy dsnet_hndl [%d]", st->priv.dsnet_hndl );

  if ( dsnet_set_policy( st->priv.dsnet_hndl, &st->priv.dsnet_policy, &dss_errno ) == DSS_ERROR )
  {
    QCRIL_LOG_ERROR( "failed to set net policy dsnet_hndl [%d], dss_errno [%d]", 
                     st->priv.dsnet_hndl, dss_errno );
    goto err_label;
  }

  QCRIL_LOG_VERBOSE( "dsnet_policy flag [%d]", 
                     st->priv.dsnet_policy.policy_flag );
  QCRIL_LOG_VERBOSE( "dsnet_policy pdp_profile_num [%d]", 
                     st->priv.dsnet_policy.umts.pdp_profile_num );
  QCRIL_LOG_VERBOSE( "dsnet_policy famiy [%d]", st->priv.dsnet_policy.family );

  QCRIL_LOG_DEBUG( "%s", "set_data_call_param: EXIT with suc" );
  return DSI_SUCCESS;

err_label:
  QCRIL_LOG_DEBUG( "%s", "set_data_call_param: EXIT with err" );
  return DSI_ERROR;
}/* set_data_call_param() */

static int get_data_call_param(
  dsi_hndl_t        hndl,
  unsigned int      identifier,
  dsi_param_info_t *info
)
{

  short int         dss_errno = DSS_SUCCESS;
  dsi_store_t      *st;
  int               len = 0;

  QCRIL_LOG_DEBUG( "%s", "get_data_call_param: ENTRY" );

  st = ( (dsi_store_t *) hndl );

  if ( !( DSI_IS_HNDL_VALID( st ) ) || 
       !( DSI_IS_IDENT_VALID ( identifier ) ) || 
        ( info == NULL ) )
  {
    QCRIL_LOG_ERROR( "cannot get, inval arg, dsi_hndl [%x], ident [%d]",
                     (unsigned int)hndl, identifier );
    goto err_label;
  }

  if ( st->priv.call_state != DS_CALL_STATE_CONNECTED )
  {
    QCRIL_LOG_ERROR( "cannot get call attr, inval state [%d], dsi_hndl [%x]",
                     st->priv.call_state, (unsigned int)hndl );
    goto err_label;
  }

  switch( identifier )
  {
  case DATA_CALL_INFO_DEVICE_NAME:

    /* Call IOCTL to get device name */
    if ( ( dss_iface_ioctl( st->priv.dsnet_iface_id, 
                            DSS_IFACE_IOCTL_GET_DEVICE_NAME, 
                            (void *) &(st->priv.dev_name), 
                           &dss_errno) ) == DSS_ERROR )
    {
      QCRIL_LOG_ERROR( "iface ioctl err, errno [%d]", dss_errno );
      goto err_label;
    }

    /* Force length to be within bounds */
    st->priv.dev_name.device_name[ DS_CALL_INFO_DEV_NAME_MAX_LEN - 1 ] = '\0';
    QCRIL_LOG_DEBUG( "dev name is [%s]", st->priv.dev_name.device_name );

    /* prepare output param */
    info->len = strlen( st->priv.dev_name.device_name );
    memcpy( info->buf, st->priv.dev_name.device_name, info->len );
       
    break;

  case DATA_CALL_INFO_APN_NAME:

    QCRIL_LOG_DEBUG( "stored apn len [%d]", st->priv.apn.length );
    DSI_ASSERT( st->priv.apn.length <= DS_CALL_INFO_APN_MAX_LEN, "APN too long" );
    info->len = MINIMUM( st->priv.apn.length, DS_CALL_INFO_APN_MAX_LEN ) ;
    strncpy( info->buf, st->priv.apn.name, info->len );

    break;

  case DATA_CALL_INFO_IP_ADDR:

    /* Call IOCTL to get IP addr */
    if ( ( dss_iface_ioctl( st->priv.dsnet_iface_id,
                            DSS_IFACE_IOCTL_GET_IPV4_ADDR,
                           &st->priv.ip_addr,
                           &dss_errno) ) == DSS_ERROR )
    {
      QCRIL_LOG_ERROR( "iface ioctl err, errno [%d]", dss_errno );
      goto err_label;
    }

    /* Support IPv4 only */
    info->len = sizeof st->priv.ip_addr.addr.v4;
    QCRIL_LOG_DEBUG( "got ip addr type [%u], ip addr [%#lx], size [%u]",
                     st->priv.ip_addr.type, st->priv.ip_addr.addr.v4, info->len );
    memcpy( info->buf, &st->priv.ip_addr.addr.v4, info->len );
    break;

  case DATA_CALL_INFO_CALL_TYPE:

    /* Support IP type only */
    strncpy( info->buf, "IP", 2 );
    info->len = 2;
    ( (char *) info->buf )[ info->len ] = '\0';
    break;

#if ((RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6))
  case DATA_CALL_INFO_GATEWAY:

    /* Call IOCTL to get gateway addr */
    if ( ( dss_iface_ioctl( st->priv.dsnet_iface_id,
                            DSS_IFACE_IOCTL_GET_IPV4_GATEWAY_ADDR,
                           &st->priv.gateway_addr,
                           &dss_errno) ) == DSS_ERROR )
    {
      QCRIL_LOG_ERROR( "iface ioctl err, errno [%d]", dss_errno );
      goto err_label;
    }

    /* Support IPv4 only */
    info->len = sizeof st->priv.gateway_addr.addr.v4;
    QCRIL_LOG_DEBUG( "got gateway type [%u], gateway addr [0x%lx], size [%u]",
                     st->priv.gateway_addr.type, st->priv.gateway_addr.addr.v4, info->len );
    memcpy( info->buf, &st->priv.gateway_addr.addr.v4, info->len );
    break;

  case DATA_CALL_INFO_PRIM_DNS_ADDR:
    /* Call IOCTL to get primary dns addr */
    if ( ( dss_iface_ioctl( st->priv.dsnet_iface_id,
                            DSS_IFACE_IOCTL_GET_IPV4_PRIM_DNS_ADDR,
                           &st->priv.prim_dns_addr,
                           &dss_errno) ) == DSS_ERROR )
    {
      QCRIL_LOG_ERROR( "iface ioctl err, errno [%d]", dss_errno );
      goto err_label;
    }

    info->len = sizeof st->priv.prim_dns_addr.addr.v4;
    QCRIL_LOG_DEBUG( "got primary dns addr type [%u], dns addr [0x%lx], size [%u]",
                     st->priv.prim_dns_addr.type, st->priv.prim_dns_addr.addr.v4, info->len );
    memcpy( info->buf, &st->priv.prim_dns_addr.addr.v4, info->len );
    break;

  case DATA_CALL_INFO_SECO_DNS_ADDR:
    /* Call IOCTL to get secondary dns addr */
    if ( ( dss_iface_ioctl( st->priv.dsnet_iface_id,
                            DSS_IFACE_IOCTL_GET_IPV4_SECO_DNS_ADDR,
                           &st->priv.sec_dns_addr,
                           &dss_errno) ) == DSS_ERROR )
    {
      QCRIL_LOG_ERROR( "iface ioctl err, errno [%d]", dss_errno );
      goto err_label;
    }

    info->len = sizeof st->priv.sec_dns_addr.addr.v4;
    QCRIL_LOG_DEBUG( "got secondary dns addr type [%u], dns addr [0x%lx], size [%u]",
                     st->priv.sec_dns_addr.type, st->priv.sec_dns_addr.addr.v4, info->len );
    memcpy( info->buf, &st->priv.sec_dns_addr.addr.v4, info->len );
    break;
#endif /* (RIL_QCOM_VERSION >= 1) || (RIL_VERSION >= 6) */

  default:
    QCRIL_LOG_ERROR( "cannot get call param, unknown identifier [%d]", identifier );
    goto err_label;
  }

  QCRIL_LOG_DEBUG( "%s", "get_data_call_attr: EXIT with suc" );
  return DSI_SUCCESS;

err_label:
  QCRIL_LOG_DEBUG( "%s", "get_data_call_attr: EXIT with err" );
  return DSI_ERROR;
}

static int get_last_call_fail_cause(
  dsi_hndl_t       hndl,
  int             *fail_cause
)
{
  dsi_store_t *st;
  short int    dss_errno = DSS_SUCCESS;

  QCRIL_LOG_DEBUG( "%s", "get_last_call_fail_cause: ENTRY" );
  
  st = ( (dsi_store_t *) hndl );

  if ( !( DSI_IS_HNDL_VALID( st ) ) || ( fail_cause == NULL ) )
  {
    QCRIL_LOG_ERROR( "inval arg, dsi_hndl [%#x], fail_cause input param is NULL", 
                     (unsigned int)st);
    *fail_cause = DSS_NET_DOWN_REASON_NOT_SPECIFIED;
    goto err_label;
  }

  if ( ( dss_last_netdownreason( st->priv.dsnet_hndl, 
                                &st->priv.dsnet_fail_cause, 
                                &dss_errno ) ) != DSS_SUCCESS )
  {
    QCRIL_LOG_ERROR( "could not get fail cause, dss_errno [%d]", dss_errno );
    st->priv.dsnet_fail_cause = DSS_NET_DOWN_REASON_NOT_SPECIFIED;
    goto err_label;
  }

  *fail_cause = st->priv.dsnet_fail_cause;

ret_label:
  QCRIL_LOG_DEBUG( "last call fail cause is [%u]", *fail_cause );
  QCRIL_LOG_DEBUG( "%s", "get_last_call_fail_cause: EXIT with suc" );
  return DSI_SUCCESS;

err_label:
  QCRIL_LOG_DEBUG( "last call fail cause is [%u]", *fail_cause );
  QCRIL_LOG_DEBUG( "%s", "get_last_call_fail_cause: EXIT with err" );
  return DSI_ERROR;
  
}/* get_last_call_fail_cause() */


void rel_data_srvc_hndl(
  dsi_hndl_t     *hndl
)
{
  dsi_store_t *st_hndl;
  short int    dss_errno = DSS_SUCCESS;
  int index = 0;

  QCRIL_LOG_DEBUG( "%s", "rel_data_srvc_hndl: ENTRY" );

  st_hndl = ( (dsi_store_t *) (hndl) );

  if ( !( DSI_IS_HNDL_VALID( st_hndl ) ) )
  {
    QCRIL_LOG_ERROR( "inval arg, store hndl [%#x]", (unsigned int)st_hndl );
    goto err_label;
  }

  for (index=0; index < DS_MAX_DATA_CALLS; index ++)
  {
    if (dsi_store_table[index].dsi_store_ptr == hndl)
    {
      break;
    }
  }
  if (index == DS_MAX_DATA_CALLS)
  {
    /*This should never happen*/
    QCRIL_LOG_ERROR("%s", "rel_data_srvc_hndl: PANIC:Could not find the entry in the store table");
    QCRIL_LOG_ERROR("%s", "rel_data_srvc_hndl: PANIC:Qcril_data could be in bad state.");
    return;
  } 
  QCRIL_LOG_DEBUG("rel_data_srvc_hndl: Found the index containing the store handle, %d", index );
  /*-------------------------------------------------------------
    Close the network library. 
  -------------------------------------------------------------*/
  if ( ( dsnet_release_handle( st_hndl->priv.dsnet_hndl,
                               &dss_errno  ) != DSS_SUCCESS ) )
  {
    QCRIL_LOG_ERROR( "could not rel dsnet_hndl [%d], err [%d]", 
                     st_hndl->priv.dsnet_hndl, dss_errno ); 
    goto err_label;
  }
  
  QCRIL_LOG_DEBUG( "rel dsnet_hndl [%d] completed", st_hndl->priv.dsnet_hndl );

  QCRIL_LOG_DEBUG( "%s", "try to dealloc dsi obj");
  dsi_cleanup_store_tbl( index );
  
  QCRIL_LOG_DEBUG("%s", "rel_data_srvc_hndl: EXIT with suc");
  
  return ;

err_label:

  QCRIL_LOG_DEBUG( "%s", "FAILED to free up dsnet_hndl, DSS lib may be in bad state");
  free( dsi_store_table[index].dsi_store_ptr );
  dsi_store_table[index].dsi_store_ptr =  NULL;
  st_hndl = NULL;
  QCRIL_LOG_DEBUG("%s", "rel_data_srvc_hndl: EXIT with err");
  return;

}/* rel_data_srvc_hndl() */


void dsi_req_get_data_bearer_tech(
  dsi_hndl_t     hndl,
  unsigned int   *tech
)
{
  dsi_store_t *st_hndl;
  sint15 dss_errno = DSS_SUCCESS;
  dss_iface_id_type iface;

  QCRIL_LOG_DEBUG( "%s", "dsi_req_get_data_bearer_tech: ENTRY" );

  st_hndl = ( (dsi_store_t *) (hndl) );

  if ( !( DSI_IS_HNDL_VALID( st_hndl ) ) )
  {
    QCRIL_LOG_ERROR( "inval arg, store hndl [%#x]", (unsigned int)st_hndl );
    goto err_label;
  }

  if (tech == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Input param tech passed as NULL");
    goto err_label;
  }

  *tech = DATA_BEARER_TECH_UNKNOWN;

  /* Get the iface Id based on the Net Policy that was obtained */
  iface = dss_get_iface_id_by_policy(st_hndl->priv.dsnet_policy, &dss_errno);

  if (iface == DSS_IFACE_INVALID_ID)
  {
    QCRIL_LOG_ERROR("%s", "Invalid Iface returned \n");
    goto err_label;
  }
  /*Now get the current data bearer technology*/

  if ((dss_iface_ioctl(iface, DSS_IFACE_IOCTL_GET_CURRENT_DATA_BEARER, tech, &dss_errno)) < 0)
  {
    QCRIL_LOG_ERROR("%s", "DSS_IFACE_IOCTL_GET_CURRENT_DATA_BEARER not successful \n");
    goto err_label;
  }

  QCRIL_LOG_DEBUG("technology returned is %d",*tech);
  return;

err_label:
  st_hndl = NULL;
  QCRIL_LOG_ERROR("%s", "dsi_req_get_data_bearer_tech: EXIT with err");
  return;

}

/*
errno:
DS_ENETISCONN           200
DS_ENETINPROGRESS       201
DS_ENETNONET            202
DS_ENETCLOSEINPROGRESS  203
*/
static void dsi_net_cb(
  signed short       nethndl,
  dss_iface_id_type  iface_id,
  signed short       errno,
  void              *user_data
)
{
  dsi_store_tbl_t  *st_tbl;
  dsi_store_t *st = NULL;
  
  QCRIL_LOG_DEBUG( "%s", "dsi_net_cb: ENTRY" );

  QCRIL_LOG_DEBUG("dsi_net_cb: user data is %x",  user_data );

  st_tbl = (dsi_store_tbl_t *) user_data;
  /*
   Use the cookie to index into the static table to get the dsi 
   store pointer. Make sure that the table entry is still valid
   (not freed up by qcril) dsi  store pointer is itself not NULL. 
   If the dsi store pointer is NULL and the dss err no is ENETNONET 
   then the qcril state must have already been cleaned up. 
   Qcril at this stage only needs to release the dss net handle.
  */
  if (st_tbl->is_valid && st_tbl->dsi_store_ptr != NULL)
  {
    st = (dsi_store_t *)( st_tbl->dsi_store_ptr );
  }
  else if (!st_tbl->is_valid || (st_tbl->dsi_store_ptr == NULL && errno != DS_ENETNONET) )
  {
     QCRIL_LOG_DEBUG( "%s", "dsi_net_cb: Bad cookie/store pointer is NULL" );
     goto err_label;
  }

  QCRIL_LOG_DEBUG("dsi_net_cb: store pointer is %x",  st );

  if ((st != NULL) && 
      !( DSI_IS_HNDL_VALID(st)) && 
      (st->net_ev_cb != NULL) && 
      ( st->self == st))
  {
    QCRIL_LOG_ERROR( "inval arg, st_hndl [%#x], dsnet_hndl [%d], dss_errno [%d]",
                     (unsigned int)st, nethndl, errno );
    goto err_label;
  }

  QCRIL_LOG_DEBUG( "dsnet_hndl [%d], iface_id [%ld], dss_errno [%d], user_data [%#lx]",
                   nethndl, (unsigned long int)iface_id, errno, *( (unsigned long int*) user_data) );

  switch ( errno )
  {
    case DS_ENETISCONN:

    QCRIL_LOG_DEBUG( "rcvd dsnet event DS_ENETISCONN, curr call state [%d]", st->priv.call_state );

    /* Populate module private Data Structures */
    DSI_ASSERT( nethndl == st->priv.dsnet_hndl, "validate dsnet hndl" );
    st->priv.call_state     = DS_CALL_STATE_CONNECTED;
    st->priv.dsnet_iface_id = iface_id;
    QCRIL_LOG_INFO( "new call_state CONNECTED, iface_id [%ld]", 
                    (unsigned long int)iface_id );

    /* Call user cb */
    QCRIL_LOG_DEBUG( "%s", "dsi_net_cb: call client callback fn" );
    st->net_ev_cb( (dsi_hndl_t)st, 
                       st->user_data, 
                       DSI_EVT_NET_IS_CONN );
    
    break;

    case DS_ENETINPROGRESS:
    
    QCRIL_LOG_DEBUG( "rcvd dsnet event DS_ENETINPROGRESS, curr call_state [%d]",
                     st->priv.call_state );

    DSI_ASSERT( nethndl == st->priv.dsnet_hndl, "validate dsnet hndl" );
    st->priv.call_state = DS_CALL_STATE_ACTIVATION_IN_PROGRESS;
    QCRIL_LOG_INFO( "new call state ACTIVATION_IN_PROGRESS, iface_id [%ld]", 
                  (unsigned long int)iface_id );


    break;

    case DS_ENETNONET:
      /*
      dsi store is already NULL and the static table entry is valid 
      which tells us that qcril state is cleaned up but not dss state, 
      hence clean up dss state at this point
      */
      if (st == NULL && st_tbl->is_valid)
      {
        short int    dss_errno = DSS_SUCCESS;
       /*release the dss net handle*/
        if ( ( dsnet_release_handle( st_tbl->dsnet_hndl,
                                     &dss_errno  ) != DSS_SUCCESS ) )
        {
          QCRIL_LOG_ERROR( "could not rel dsnet_hndl [%d], err [%d]", 
                           st_tbl->dsnet_hndl, dss_errno ); 
          goto err_label;
        }
        st_tbl->is_valid = FALSE;
        st_tbl->dsnet_hndl = -1;
      }
      else if (st != NULL)
      {
        QCRIL_LOG_DEBUG( "rcvd dsnet event DS_ENETNONET, curr call_state [%d]",
                       st->priv.call_state );

        DSI_ASSERT( nethndl == st->priv.dsnet_hndl, "validate dsnet hndl" );
        st->priv.call_state = DS_CALL_STATE_DISCONNECTED;
        QCRIL_LOG_INFO( "new call state DS_CALL_STATE_DISCONNECTED, iface_id [%ld]",
                    (unsigned long int) iface_id );

        /* Call user cb */
        st->net_ev_cb(( dsi_hndl_t *)st->self, 
                       st->user_data, 
                       DSI_EVT_NET_NO_NET );
      }
    
    break;
  
    case DS_ENETCLOSEINPROGRESS:

    QCRIL_LOG_DEBUG( "rcvd dsnet event ev DS_ENETCLOSEINPROGRESS, curr call_state [%d]",
                     st->priv.call_state );

    DSI_ASSERT( nethndl == st->priv.dsnet_hndl, "validate dsnet hndl" );
    st->priv.call_state = DS_CALL_STATE_DEACTIVATION_IN_PROGRESS;
    QCRIL_LOG_INFO( "new call state CLOSEINPROGRESS, iface_id [%ld]", 
                    (unsigned long int) iface_id );

    break;

    case DS_ENETRECONFIGURED:

    QCRIL_LOG_DEBUG( "rcvd dsnet event DS_ENETRECONFIGURED, curr call state [%d]",
                     st->priv.call_state );

    /* Populate module private Data Structures */
    DSI_ASSERT( nethndl == st->priv.dsnet_hndl, "validate dsnet hndl" );
    DSI_ASSERT( st->priv.call_state == DS_CALL_STATE_CONNECTED,
               "validate call state");
    QCRIL_LOG_INFO( "call RECONFIGURED, iface_id [%ld]",
                    (unsigned long int)iface_id );

    /* Call user cb */
    QCRIL_LOG_DEBUG( "%s", "dsi_net_cb: call client callback fn" );
    st->net_ev_cb( (dsi_hndl_t)st,
                    st->user_data,
                    DSI_EVT_NET_RECONFIGURED );

    break;

    default:
    QCRIL_LOG_ERROR( "rcvd dsnet unknown event [%d], curr call_state [%d]",
                     errno, st->priv.call_state );
    goto err_label;

  }/* switch( errno )*/

  QCRIL_LOG_DEBUG( "%s", "dsi_net_cb: EXIT with suc" );
  return;

err_label:
  QCRIL_LOG_ERROR( "%s", "dsi_net_cb: err occured which cannot be handled!" );
  QCRIL_LOG_DEBUG( "%s", "dsi_net_cb: EXIT with err" );
  return;

} /* dsi_net_cb() */ 


void dsi_sock_cb(
  signed short       nethndl,
  signed short       sockfd,
  unsigned long int  event_mask,
  void              *user_data
)
{
  /*-------------------------------------------------------------------------
    This module cannot handle socket events. Trap this!
  -------------------------------------------------------------------------*/
  DSI_ASSERT( 0, "cannot hndl sock cb, trap this!" );
  return;
}

static void
dsi_ev_cb_func
(
  dss_iface_ioctl_event_enum_type          event,
  dss_iface_ioctl_event_info_union_type    event_info,
  void                                     *user_data,
  sint15                                   dss_nethandle,
  dss_iface_id_type                        iface_id
)
{
  dsi_store_t *st;
  QCRIL_LOG_DEBUG( "%s", "dsi_ev_cb_func: ENTRY \n" );
  QCRIL_LOG_DEBUG( "In dss_test_ev_cb_func: nethandle = %d, iface_id = %ld, user_data = %d\n",
                   dss_nethandle, iface_id, (int) user_data);

  st = (dsi_store_t *) user_data;

  if (event == DSS_IFACE_IOCTL_PHYS_LINK_DOWN_EV) {
    QCRIL_LOG_DEBUG( "%s", "===> PHYSLINK DOWN INDICATION RECEIVED <==== \n ");
    st->net_ev_cb( ( dsi_hndl_t *)st, 
                     st->user_data, 
                     DSI_EVT_PHSYLINK_DOWN_STATE );  
  }
  else if (event == DSS_IFACE_IOCTL_PHYS_LINK_UP_EV) {
      QCRIL_LOG_DEBUG( "%s", "===> PHYSLINK UP INDICATION RECEIVED <==== \n ");
      st->net_ev_cb( ( dsi_hndl_t *)st, 
                         st->user_data, 
                         DSI_EVT_PHSYLINK_UP_STATE );  
  }
  QCRIL_LOG_DEBUG( "%s", "dsi_ev_cb_func: EXIT \n" );
  return;
}

/*===========================================================================

                    PUBLIC FUNCTIONS

===========================================================================*/

/*-------------------------------------------------------------------------
    This routine registers physlink events ACTIVE/DORMANT
    on the specified iface.
-------------------------------------------------------------------------*/

int dsi_reg_physlink_up_down_events(
  dsi_hndl_t            *handle
)
{
  dsi_store_t    *st; 
  int status;
  sint15 dss_errno;
  dss_iface_id_type iface;
  dss_iface_ioctl_ev_cb_type reg_cb, dereg_cb;

  QCRIL_LOG_DEBUG( "%s", "dsi_reg_physlink_up_down_events: ENTRY \n" );

  if (handle == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Input param handle is passed as NULL \n");
    goto err_label;
  }
  st = (dsi_store_t *)handle;

  /* Get the iface Id based on the Net Policy that was obtained */
  iface = dss_get_iface_id_by_policy(st->priv.dsnet_policy, &dss_errno);

  if (iface == DSS_IFACE_INVALID_ID)
  {
    QCRIL_LOG_ERROR("%s", "Invalid Iface returned \n");
    goto err_label;
  }
  /*Now register for PHYSLINK UP and DOWN*/

  reg_cb.event_cb = &dsi_ev_cb_func;
  reg_cb.user_data_ptr = (void *) handle;
  reg_cb.app_id = st->priv.dsnet_hndl;

  reg_cb.event = DSS_IFACE_IOCTL_PHYS_LINK_DOWN_EV;

  if ((status = dss_iface_ioctl(iface, 
                                DSS_IFACE_IOCTL_REG_EVENT_CB, 
                                &reg_cb, 
                                &dss_errno)) < 0)
  {
    QCRIL_LOG_ERROR("%s", "dsi_reg_physlink_up_down_events:"
                          " DSS_IFACE_IOCTL_PHYS_LINK_DOWN_EV "
                          "regisration was not Successful\n");
    goto err_label;
  }

  reg_cb.event = DSS_IFACE_IOCTL_PHYS_LINK_UP_EV;

  if ((status = dss_iface_ioctl(iface, DSS_IFACE_IOCTL_REG_EVENT_CB, &reg_cb, &dss_errno)) < 0)
  {
    QCRIL_LOG_VERBOSE("%s", "dsi_reg_physlink_up_down_events:"
                            "DSS_IFACE_IOCTL_PHYS_LINK_UP_EV "
                            "regisration was not Successful\n");
    goto err_label;
  }
  
  QCRIL_LOG_DEBUG("%s", "dsi_reg_physlink_up_down_events: EXIT \n");

ret_label:
  return DSI_SUCCESS;
err_label:
  return DSI_ERROR;
}

/*-------------------------------------------------------------------------
    This routine is a generic iface ioctl handler for qcril.
    Currently this routine calls the dss iface ioctl.
-------------------------------------------------------------------------*/

int dsi_iface_ioctl(
  dsi_hndl_t                    *handle,
  dsi_iface_ioctl_type          ioctl
)
{
  dsi_store_t    *st; 
  int status;
  sint15 dss_errno;
  dss_iface_id_type iface_id;

  st = (dsi_store_t *)handle;

  if (!DSI_IS_HNDL_VALID(st))
  {
    QCRIL_LOG_ERROR("%s","dsi_iface_ioctl: Bad Input: Invalid Net Handle Provided \n");
    goto err_label;
  }

  iface_id = dss_get_iface_id(st->priv.dsnet_hndl);

  if (iface_id == DSS_IFACE_INVALID_ID)
  {
    QCRIL_LOG_ERROR("dsi_iface_ioctl: No Iface was found bound to the specified net_hndl: %d \n",st->priv.dsnet_hndl);
    goto err_label;
  }

  switch (ioctl)
  {
    case DSI_IFACE_IOCTL_GO_DORMANT:
      {
  if ((status = dss_iface_ioctl(iface_id, DSS_IFACE_IOCTL_GO_DORMANT, NULL, &dss_errno)) < 0)
  {
          QCRIL_LOG_ERROR("dsi_iface_ioctl: DSS_IFACE_IOCTL_GO_DORMANT was not Successful, dss_errno: %d\n",dss_errno);
    goto err_label;
  }
      }
      break;
    case DSI_IFACE_IOCTL_DORMANCY_INDICATIONS_OFF:
      {
        if ((status = dss_iface_ioctl(iface_id, DSS_IFACE_IOCTL_DORMANCY_INDICATIONS_OFF, NULL, &dss_errno)) < 0)
        {
          QCRIL_LOG_ERROR("dsi_iface_ioctl: DSS_IFACE_IOCTL_DORMANCY_INDICATIONS_OFF was not Successful, dss_errno: %d\n",dss_errno);
          goto err_label;
        }
      }
      break;
    case DSI_IFACE_IOCTL_DORMANCY_INDICATIONS_ON:
      {
        if ((status = dss_iface_ioctl(iface_id, DSS_IFACE_IOCTL_DORMANCY_INDICATIONS_ON, NULL, &dss_errno)) < 0)
        {
          QCRIL_LOG_ERROR("dsi_iface_ioctl: DSS_IFACE_IOCTL_DORMANCY_INDICATIONS_ON was not Successful, dss_errno: %d\n",dss_errno);
          goto err_label;
        }
      }
      break;
    default:
      {
        QCRIL_LOG_ERROR("%s","dsi_iface_ioctl: Invalid ioctl received\n");
      }
      break;
  }

  return DSI_SUCCESS;
err_label:
  return DSI_ERROR;
}

dsi_hndl_t dsi_get_data_srvc_hndl(
  dsi_net_ev_cb   user_cb_fn,
  void           *user_data
)
{
  dsi_store_t              *st;
  short int                 dss_errno     = DSS_SUCCESS;
  int                       dsnet_hndl    = 0;
  int                       index  = 0;
  QCRIL_LOG_DEBUG( "%s", "dsi_get_data_srvc_hndl: ENTRY" );

#ifdef DSI_TEST_MODE_FAIL_CALL_CONTINUOUS
  QCRIL_LOG_INFO( "test mode, ret dsi_hndl to test_store_1 [%x]", &test_store_1 );
  test_store_1.fntbl.start_data_call     = start_data_call;
  test_store_1.fntbl.stop_data_call      = stop_data_call;
  test_store_1.fntbl.set_data_call_param = set_data_call_param;
  test_store_1.fntbl.get_data_call_param = get_data_call_param;
  test_store_1.fntbl.get_last_call_fail_cause
                                = get_last_call_fail_cause;
  test_store_1.fntbl.rel_data_srvc_hndl  = rel_data_srvc_hndl; 

  test_store_1.net_ev_cb                 = user_cb_fn;
  test_store_1.user_data                 = user_data;

  test_store_1.priv.dsnet_hndl           = 420;
  test_store_1.self                      = &test_store_1;

  st = &test_store_1;
  goto ret_label;
#endif

  if ( ( st  = ( dsi_store_t * )malloc( sizeof( dsi_store_t ) ) ) == NULL )
  {
    QCRIL_LOG_ERROR( "%s", "alloc dsi obj FAILED" );
    goto err_label;
  }
  memset( st, 0, sizeof( dsi_store_t ) );
  QCRIL_LOG_DEBUG( "%s", "alloc dsi obj successful" );
  /*
    Find a free table entry and store the dsi 
    store pointer and the dsnet_handle(from dss)
    This will be used as a cookie sent to dss.
  */
  for (index = 0; index < DS_MAX_DATA_CALLS; index++)
  {
    if (!dsi_store_table[index].is_valid)
    {
      QCRIL_LOG_VERBOSE("found an un-used index %d, store pointer is %x",index, st );
      break;
    }
  }
  if (index == DS_MAX_DATA_CALLS )
  {
    QCRIL_LOG_ERROR( "%s", "dsi_get_data_srvc_hndl: Couldnt find a free store table slot" );
    free(st);
    goto err_label;
  }
  else
  {
    dsi_store_table[index].dsi_store_ptr = st;
    dsi_store_table[index].is_valid = TRUE;
  }
  /*----------------------------------------------------------------------------
    Initialize the network policy for UMTS
  ----------------------------------------------------------------------------*/
  dss_init_net_policy_info( &(st->priv.dsnet_policy) );

  st->priv.dsnet_policy.iface.kind           = DSS_IFACE_NAME;
  st->priv.dsnet_policy.iface.info.name      = DSS_IFACE_ANY;
  st->priv.dsnet_policy.umts.pdp_profile_num = DSI_UMTS_DEFAULT_PDP_PROF;

  /* Debug info for net_policy */
  QCRIL_LOG_VERBOSE( "dsnet_policy policy_flag [%d]", 
                     st->priv.dsnet_policy.policy_flag );
  QCRIL_LOG_VERBOSE( "dsnet_policy pdp_profile_num [%d]", 
                     st->priv.dsnet_policy.umts.pdp_profile_num );
  QCRIL_LOG_VERBOSE( "dsnet_policy family [%d]", st->priv.dsnet_policy.family );

  /* Call DSS lib API to get DSNET HNDL */
  if ( ( dsnet_hndl = ( dsnet_get_handle( dsi_net_cb, 
                                          (void *)&dsi_store_table[index], 
                                          dsi_sock_cb, 
                                          (void *)&dsi_store_table[index], 
                                          &(st->priv.dsnet_policy), 
                                          &dss_errno) ) ) == DSS_ERROR )
  {
    QCRIL_LOG_ERROR( "could not get dsnet hndl, err [%d]", dss_errno );
    dsi_cleanup_store_tbl(index);
    goto err_label;
  }
  else
  {
    QCRIL_LOG_VERBOSE( "got dsnet hndl = [%d]", dsnet_hndl );
    dsi_store_table[index].dsnet_hndl = dsnet_hndl;
  }

  /* Setup fn tbl */
  st->fntbl.start_data_call     = start_data_call;
  st->fntbl.stop_data_call      = stop_data_call;
  st->fntbl.set_data_call_param = set_data_call_param;
  st->fntbl.get_data_call_param = get_data_call_param;
  st->fntbl.get_last_call_fail_cause
                                = get_last_call_fail_cause;
  st->fntbl.rel_data_srvc_hndl  = rel_data_srvc_hndl; 

  /* Populate */
  st->net_ev_cb                 = user_cb_fn;
  st->user_data                 = user_data;
  st->priv.dsnet_hndl           = dsnet_hndl;

  /* Mark as valid */
  st->self                      = st;

  QCRIL_LOG_VERBOSE( "data store is at [%#x]", (unsigned int)st );

ret_label:
  QCRIL_LOG_DEBUG("%s", "dsi_get_data_srvc_hndl: EXIT with suc");
  return (dsi_hndl_t)st;

err_label:
  QCRIL_LOG_DEBUG("%s", "dsi_get_data_srvc_hndl: EXIT with err");
  return NULL;

}/* dsi_get_data_srvc_hndl() */

void dsi_init()
{
  int  i = 0;      

  //[todo] can we get rid of .sh file ref?
  char *dscmain_input[12] = {

    "QCRIL_DSI",
    "-f",
    "-s",
    "-k",
    "-l",
    "0",
    "-i",
    "rmnet",
    "-u",
    "/opt/qcom/bin/udhcpc.sh",
    "-m",
    "/opt/qcom/bin/qcomdsc-kif.sh",

  };


  QCRIL_LOG_DEBUG( "%s", "dsi_init: ENTRY" );

#ifdef DSI_TEST_MODE_FAIL_CALL_CONTINUOUS
  QCRIL_LOG_INFO( "%s", "dsc_main has been excluded" );
#else
  QCRIL_LOG_INFO( "%s", "dsc_main has been included" );
  dsc_main( 12, dscmain_input );
#endif

  QCRIL_LOG_DEBUG( "%s", "dsi_init: EXIT" );
}/* dsi_init() */
