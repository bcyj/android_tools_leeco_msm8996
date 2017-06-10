/*!
  @file
  qcril_dsi.h

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

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
12/23/08   asn     Added handling on MO call-end and IP addr issue

===========================================================================*/

#ifndef QCRIL_DSI_H
#define QCRIL_DSI_H

#include "ril.h"
#include "IxErrno.h"
#include "qcrili.h"
#include "dssocket.h"

/*---------------------------------------------------------------------------
                           DECLARATIONS
---------------------------------------------------------------------------*/

/* Error codes */
#define DSI_SUCCESS 0
#define DSI_ERROR  -1

#define CALL_ID_INVALID -1

/* Identifier to get/set params */
#define DATA_CALL_INFO_UMTS_PROFILE_IDX    0x00000001
#define DATA_CALL_INFO_CALL_TYPE           0x00000002
#define DATA_CALL_INFO_APN_NAME            0x00000003
#define DATA_CALL_INFO_USERNAME            0x00000004
#define DATA_CALL_INFO_PASSWORD            0x00000005
#define DATA_CALL_INFO_AUTH_PREF           0x00000006
#define DATA_CALL_INFO_CDMA_PROFILE_IDX    0x00000007

/* returned buf contains unsigned int for IPv4 implementation */
#define DATA_CALL_INFO_IP_ADDR             0x00000007

#define DATA_CALL_INFO_DEVICE_NAME         0x00000008

#define DATA_CALL_INFO_GATEWAY             0x00000009

#define DATA_CALL_INFO_SECO_DNS_ADDR       0x0000000A
#define DATA_CALL_INFO_PRIM_DNS_ADDR       0x0000000B

#define DS_CALL_INFO_USERNAME_MAX_LEN      DSS_STRING_MAX_LEN
#define DS_CALL_INFO_PASSWORD_MAX_LEN      DSS_STRING_MAX_LEN

#define DS_MAX_DATA_CALLS     20

#define DS_CALL_INFO_IP_ADDR_MAX_LEN   16                         /* limit to IPv4 addr */

#define DS_CALL_INFO_DNS_ADDR_MAX_LEN   33

#define QCRIL_IPV4_ADDR 4

/* DSI events */
typedef enum
{
  DSI_EVT_INVALID      = 0x0,
  DSI_EVT_NET_IS_CONN,
  DSI_EVT_NET_NO_NET,
  DSI_EVT_PHSYLINK_DOWN_STATE,
  DSI_EVT_PHSYLINK_UP_STATE,
  DSI_EVT_NET_RECONFIGURED,
  DSI_EVT_MAX
} dsi_net_evt_t;

typedef enum
{
  DSI_IFACE_IOCTL_MIN                         = 0x00,
  DSI_IFACE_IOCTL_GO_DORMANT                  = DSI_IFACE_IOCTL_MIN,
  DSI_IFACE_IOCTL_DORMANCY_INDICATIONS_OFF    = 0x01,
  DSI_IFACE_IOCTL_DORMANCY_INDICATIONS_ON     = 0x02,
  DSI_IFACE_IOCTL_MAX                         = DSI_IFACE_IOCTL_DORMANCY_INDICATIONS_ON
} dsi_iface_ioctl_type;

typedef void * dsi_hndl_t;

typedef struct
{
  
  void         *buf;
  unsigned int  len;

} dsi_param_info_t;

typedef struct 
{
  char            cid[4]; 

#define DS_CALL_INFO_TYPE_MAX_LEN 10
  char            type[ DS_CALL_INFO_TYPE_MAX_LEN + 1 ];       /* X.25, IP, IPV6, etc. */

#define DS_CALL_INFO_APN_MAX_LEN  DSS_UMTS_APN_MAX_LEN
  char            apn[ DS_CALL_INFO_APN_MAX_LEN + 1 ];

  char            address[ DS_CALL_INFO_IP_ADDR_MAX_LEN + 1 ];

#define DS_CALL_INFO_DEV_NAME_MAX_LEN DSS_MAX_DEVICE_NAME_LEN
  char            dev_name[ DS_CALL_INFO_DEV_NAME_MAX_LEN + 1 ];

  char            gateway[ DS_CALL_INFO_IP_ADDR_MAX_LEN + 1 ];

  char            dns_address[ DS_CALL_INFO_DNS_ADDR_MAX_LEN + 1 ];

} dsi_call_info_t;

typedef void (*dsi_net_ev_cb)( dsi_hndl_t     hndl, 
                               void          *user_data, 
                               dsi_net_evt_t  evt );
typedef struct
{

  int (*start_data_call)( 
    dsi_hndl_t  hndl
  );

  int (*stop_data_call)( 
    dsi_hndl_t  hndl
  );

  int (*set_data_call_param)(
    dsi_hndl_t        hndl,
    unsigned int      identifier,
    dsi_param_info_t *info
  );

  int (*get_data_call_param)(
    dsi_hndl_t        hndl,
    unsigned int      identifier,
    dsi_param_info_t *info
  );

  int (*get_last_call_fail_cause)(
    dsi_hndl_t  hndl,
    int             *fail_cause
  );

  void (*rel_data_srvc_hndl)(
    dsi_hndl_t           *hndl
  );

}dsi_vtbl_t;


static __inline int dsi_req_start_data_call(
  dsi_hndl_t hndl
)
{
  //LOG_DEBUG( "dsi_req_setup_data_call: ENTRY" );
  if (hndl == NULL)
  {
    return DSI_ERROR;
  }
  else
  {
   return ( (dsi_vtbl_t *)(hndl))->start_data_call( hndl );
  }
}

static __inline int dsi_req_stop_data_call(
  dsi_hndl_t hndl
)
{
  //LOG_DEBUG( "dsi_req_stop_data_call: ENTRY" );
  if (hndl == NULL)
  {
    return DSI_ERROR;
  }
  else
  {
    return ( (dsi_vtbl_t *)(hndl))->stop_data_call( hndl );
  }
}

static __inline int dsi_set_data_call_param(
  dsi_hndl_t   hndl,
  unsigned int      iden,
  dsi_param_info_t *info
)
{
  if (hndl == NULL || info == NULL)
  {
    return DSI_ERROR;
  }
  else
  {
    return ( (dsi_vtbl_t *)(hndl))->set_data_call_param( hndl,
                                                         iden,
                                                         info );
  }
}

static __inline int dsi_get_data_call_param(
  dsi_hndl_t   hndl,
  unsigned int      iden,
  dsi_param_info_t *info
)
{
  if (hndl == NULL || info == NULL)
  {
    return DSI_ERROR;
  }
  else
  {
    return ( (dsi_vtbl_t *)(hndl))->get_data_call_param( hndl,
                                                         iden,
                                                         info );
  }
}

static __inline int dsi_get_last_call_fail_cause(
  dsi_hndl_t  hndl,
  int        *fail_cause
)
{
  if (hndl == NULL || fail_cause == NULL)
  {
    return DSI_ERROR;
  }
  else
  {
    return ( (dsi_vtbl_t *)(hndl))->get_last_call_fail_cause( hndl,
                                                              fail_cause );
  }
}

static __inline void dsi_rel_data_srvc_hndl(
  dsi_hndl_t hndl
)
{
  if (hndl != NULL) 
  {
    return ( (dsi_vtbl_t *)(hndl))->rel_data_srvc_hndl( hndl );
  }
}

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/
extern void dsi_init();
extern dsi_hndl_t dsi_get_data_srvc_hndl( dsi_net_ev_cb cb_fn, void *user_data );
/*-------------------------------------------------------------------------
    This routine registers physlink events ACTIVE/DORMANT
    on the specified iface.
-------------------------------------------------------------------------*/

int dsi_reg_physlink_up_down_events(
  dsi_hndl_t            *handle
);
/*-------------------------------------------------------------------------
    This routine is a generic iface ioctl handler for qcril.
    Currently this routine wraps the dss iface ioctl.
-------------------------------------------------------------------------*/

int dsi_iface_ioctl(
  dsi_hndl_t            *handle,
  dsi_iface_ioctl_type  ioctl
);

/*-------------------------------------------------------------------------
    This routine is used to get the current data bearer technology.
-------------------------------------------------------------------------*/

void dsi_req_get_data_bearer_tech(
  dsi_hndl_t     hndl,
  unsigned int   *tech
);

#endif /* QCRIL_DSI_H */
