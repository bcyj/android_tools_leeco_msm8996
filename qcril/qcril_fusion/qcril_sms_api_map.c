/*!
  @file
  qcril_sms_api_map.c

  @brief
  Handles the mapping to modems' WMS APIs.

*/

/*===========================================================================

  Copyright (c) 2009 - 2010 Qualcomm Technologies, Inc. All Rights Reserved

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

when       who     what, where, why
--------   ---     ---------------------------------------------------------- 
04/06/10   pg      First cut.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "qcril_sms_api_map.h"


/*===========================================================================

                       EXTERN DEFINITIONS 

===========================================================================*/

extern void qcril_sms_cfg_event_callback( wms_cfg_event_e_type event, wms_cfg_event_info_s_type *event_ptr );
extern void qcril_sms_msg_event_callback( wms_msg_event_e_type event, wms_msg_event_info_s_type *event_ptr, boolean *shared );
extern void qcril_sms_bc_mm_event_callback( wms_bc_mm_event_e_type event, wms_bc_mm_event_info_s_type *info_ptr );

extern void qcril_sms_cfg_event_callback_fusion( wms_cfg_event_e_type event, wms_cfg_event_info_s_type *event_ptr );
extern void qcril_sms_msg_event_callback_fusion( wms_msg_event_e_type event, wms_msg_event_info_s_type *event_ptr, boolean *shared );
extern void qcril_sms_bc_mm_event_callback_fusion( wms_bc_mm_event_e_type event, wms_bc_mm_event_info_s_type *info_ptr );


/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/*===========================================================================

                               API MAP 

===========================================================================*/

qcril_wms_callback_funcs_type qcril_sms_api_callbacks[ QCRIL_MAX_MODEM_ID ] =
{
  {
    qcril_sms_cfg_event_callback,
    qcril_sms_msg_event_callback,
    qcril_sms_bc_mm_event_callback
  }
  #ifdef FEATURE_QCRIL_FUSION
  ,
  {
    qcril_sms_cfg_event_callback_fusion,
    qcril_sms_msg_event_callback_fusion,
    qcril_sms_bc_mm_event_callback_fusion
  }
  #endif /* FEATURE_QCRIL_FUSION */
};

qcril_wms_funcs_type qcril_sms_api_funcs[ QCRIL_MAX_MODEM_ID ] =
{
  {
    wms_client_release,
    wms_client_activate,
    #ifdef FEATURE_QCRIL_WMS_PM
    wms_client_init_for_pm,
    wms_client_reg_cfg_cb_for_pm,
    wms_client_reg_msg_cb_for_pm,
    wms_client_reg_bc_mm_cb_for_pm,
    #else
    wms_client_init, 
    wms_client_reg_cfg_cb,
    wms_client_reg_msg_cb,
    wms_client_reg_bc_mm_cb,
    #endif /* FEATURE_QCRIL_WMS_PM */

    #ifdef FEATURE_QCRIL_DSDS
    wms_cfg_ms_set_primary_client,
    wms_cfg_ms_set_routes,
    wms_cfg_ms_set_link_control,
    wms_cfg_ms_set_memory_full,
    wms_cfg_ms_get_message_list,
    
    wms_msg_ms_send,
    wms_msg_ms_ack,
    wms_msg_ms_write,
    wms_msg_ms_delete,
    wms_msg_ms_read_template,
    wms_msg_ms_write_template,

    wms_bc_ms_set_pref,
    wms_bc_ms_get_table,
    wms_bc_ms_add_services,
    wms_bc_ms_delete_all_services
    #else
    wms_cfg_set_primary_client,
    wms_cfg_set_routes,
    wms_cfg_set_link_control,
    wms_cfg_set_memory_full,
    wms_cfg_get_message_list,
    
    wms_msg_send,
    wms_msg_ack,
    wms_msg_write,
    wms_msg_delete,
    wms_msg_read_template,
    wms_msg_write_template,

    wms_bc_mm_set_pref,
    wms_bc_mm_get_table,
    wms_bc_mm_add_services,
    wms_bc_mm_delete_all_services
    #endif /* FEATURE_QCRIL_DSDS */

    #ifdef FEATURE_QCRIL_IMS_EXT
    , wms_msg_resend
    #endif /* FEATURE_QCRIL_IMS_EXT */
  }

  #if !defined( FEATURE_QCRIL_DSDS) && defined(FEATURE_QCRIL_FUSION)
  ,
  {
    wms_client_release_fusion,
    wms_client_activate_fusion,
    wms_client_init_fusion,
    wms_client_reg_cfg_cb_fusion,
    wms_client_reg_msg_cb_fusion,
    wms_client_reg_bc_mm_cb_fusion,
    
    wms_cfg_set_primary_client_fusion,
    wms_cfg_set_routes_fusion,
    wms_cfg_set_link_control_fusion,
    wms_cfg_set_memory_full_fusion,
    wms_cfg_get_message_list_fusion,
    
    wms_msg_send_fusion,
    wms_msg_ack_fusion,
    wms_msg_write_fusion,
    wms_msg_delete_fusion,
    wms_msg_read_template_fusion,
    wms_msg_write_template_fusion,
    
    wms_bc_mm_set_pref_fusion,
    wms_bc_mm_get_table_fusion,
    wms_bc_mm_add_services_fusion,
    wms_bc_mm_delete_all_services_fusion

    #ifdef FEATURE_QCRIL_IMS_EXT
    , wms_msg_resend_fusion
    #endif /* FEATURE_QCRIL_IMS_EXT */
  }
  #endif /* !FEATURE_QCRIL_DSDS && FEATURE_QCRIL_FUSION */
};
