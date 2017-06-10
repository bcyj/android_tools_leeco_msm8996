/*!
  @file
  qcril_sms_api_map.h

  @brief

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

#ifndef QCRIL_SMS_API_MAP_H
#define QCRIL_SMS_API_MAP_H

#include "comdef.h"
#include "oncrpc.h"
#include "wms.h"
#ifdef FEATURE_QCRIL_FUSION
#include "wms_fusion.h"
#endif /* FEATURE_QCRIL_FUSION */
#include "qcrili.h"


/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

typedef wms_client_err_e_type (wms_client_release_f_type) (

  wms_client_id_type       client_id

);

typedef wms_client_err_e_type (wms_client_activate_f_type) (

  wms_client_id_type       client_id

);

#ifdef FEATURE_QCRIL_WMS_PM
typedef wms_client_err_e_type (wms_client_init_for_pm_f_type) (

  wms_client_type_e_type       client_type,
  wms_client_id_type           *client_id_ptr,
  wms_client_processor_e_type  processor
);

typedef wms_client_err_e_type (wms_client_reg_cfg_cb_for_pm_f_type) (

  wms_client_id_type       client_id,
  wms_pm_notify_e_type     cfg_notify_type,
  uint32                   cfg_event_mask,
  wms_cfg_event_cb_type    cfg_event_cb
);

typedef wms_client_err_e_type (wms_client_reg_msg_cb_for_pm_f_type) (

  wms_client_id_type       client_id,
  wms_pm_notify_e_type     msg_notify_type,
  uint32                   msg_event_mask,
  wms_msg_event_cb_type    msg_event_cb
);

typedef wms_client_err_e_type (wms_client_reg_bc_mm_cb_for_pm_f_type) (

  wms_client_id_type          client_id,
  wms_pm_notify_e_type     bc_mm_notify_type,
  uint32                   bc_mm_event_mask,
  wms_bc_mm_event_cb_type     bc_event_cb
);
#else
typedef wms_client_err_e_type (wms_client_init_f_type) (

  wms_client_type_e_type   client_type,
  wms_client_id_type       *client_id_ptr

);

typedef wms_client_err_e_type (wms_client_reg_cfg_cb_f_type) (

  wms_client_id_type       client_id,
  wms_cfg_event_cb_type    cfg_event_cb

);

typedef wms_client_err_e_type (wms_client_reg_msg_cb_f_type) (

  wms_client_id_type       client_id,
  wms_msg_event_cb_type    msg_event_cb

);

typedef wms_client_err_e_type (wms_client_reg_bc_mm_cb_f_type) (

  wms_client_id_type          client_id,
  wms_bc_mm_event_cb_type     bc_event_cb

);
#endif /* FEATURE_QCRIL_WMS_PM */

typedef wms_status_e_type (wms_bc_mm_set_pref_f_type) (

  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode,
  wms_bc_pref_e_type              pref

);

typedef wms_status_e_type (wms_bc_mm_get_table_f_type) (

  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode

);

typedef wms_status_e_type (wms_bc_mm_add_services_f_type) (

  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode,
  uint8                           num_entries,
  const wms_bc_mm_service_info_s_type   *entries

);

typedef wms_status_e_type (wms_bc_mm_delete_all_services_f_type) (
  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode

);

#ifdef FEATURE_QCRIL_DSDS
typedef wms_status_e_type (wms_cfg_ms_set_primary_client_f_type) (

  wms_client_id_type     client_id,
  sys_modem_as_id_e_type as_id,
  wms_cmd_cb_type        cmd_cb,
  const void             *user_data,
  boolean                set_primary,
  boolean                use_client_memory

);

typedef wms_status_e_type (wms_cfg_ms_set_routes_f_type) (

  wms_client_id_type               client_id,
  sys_modem_as_id_e_type           as_id,
  wms_cmd_cb_type                  cmd_cb,
  const void*                      user_data,
  const wms_routes_s_type*         routes_ptr
);

typedef wms_status_e_type (wms_cfg_ms_set_link_control_f_type) (

  wms_client_id_type               client_id,
  sys_modem_as_id_e_type           as_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       *user_data,
  wms_cfg_link_control_mode_e_type control_option,
  uint8                            idle_timer /* in seconds */

);

typedef wms_status_e_type (wms_cfg_ms_set_memory_full_f_type) (

  wms_client_id_type             client_id,
  sys_modem_as_id_e_type         as_id,
  wms_cmd_cb_type                cmd_cb,
  const void                     *user_data,
  boolean                        memory_full

);

typedef wms_status_e_type (wms_cfg_ms_get_message_list_f_type) (

  wms_client_id_type               client_id,
  sys_modem_as_id_e_type           as_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       *user_data,
  wms_memory_store_e_type          mem_store,
  wms_message_tag_e_type           tag

);

/* Message Group */

typedef wms_status_e_type (wms_msg_ms_send_f_type) (

  wms_client_id_type               client_id,
  sys_modem_as_id_e_type           as_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       *user_data,
  wms_send_mode_e_type             send_mode,
  const wms_client_message_s_type  *message_ptr

);

typedef wms_status_e_type (wms_msg_ms_ack_f_type) (

  wms_client_id_type              client_id,
  sys_modem_as_id_e_type          as_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  const wms_ack_info_s_type       *ack_info_ptr

);

typedef wms_status_e_type (wms_msg_ms_write_f_type) (

  wms_client_id_type               client_id,
  sys_modem_as_id_e_type           as_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       *user_data,
  wms_write_mode_e_type            write_mode,
  const wms_client_message_s_type  *message_ptr

);

typedef wms_status_e_type (wms_msg_ms_delete_f_type) (

  wms_client_id_type              client_id,
  sys_modem_as_id_e_type          as_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index

);

typedef wms_status_e_type (wms_msg_ms_read_template_f_type) (

  wms_client_id_type              client_id,
  sys_modem_as_id_e_type          as_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index

);

typedef wms_status_e_type (wms_msg_ms_write_template_f_type) (

  wms_client_id_type               client_id,
  sys_modem_as_id_e_type           as_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       *user_data,
  wms_write_mode_e_type            write_mode,
  const wms_client_message_s_type  *message_ptr

);

typedef wms_status_e_type (wms_bc_ms_set_pref_f_type) (
  wms_client_id_type              client_id,
  sys_modem_as_id_e_type          as_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode,
  wms_bc_pref_e_type              pref
  );

typedef wms_status_e_type (wms_bc_ms_get_table_f_type) (
  wms_client_id_type              client_id,
  sys_modem_as_id_e_type          as_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode
);

typedef wms_status_e_type (wms_bc_ms_add_services_f_type) (
  wms_client_id_type              client_id,
  sys_modem_as_id_e_type          as_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode,
  uint8                           num_entries,
  const wms_bc_mm_service_info_s_type   *entries
);

typedef wms_status_e_type (wms_bc_ms_delete_all_services_f_type) (
  wms_client_id_type              client_id,
  sys_modem_as_id_e_type          as_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_message_mode_e_type         message_mode
);
#else
typedef wms_status_e_type (wms_cfg_set_primary_client_f_type) (

  wms_client_id_type    client_id,
  wms_cmd_cb_type       cmd_cb,
  const void            *user_data,
  boolean               set_primary,
  boolean               use_client_memory

);

typedef wms_status_e_type (wms_cfg_set_routes_f_type) (

  wms_client_id_type         client_id,
  wms_cmd_cb_type            cmd_cb,
  const void                 *user_data,
  const wms_routes_s_type    *routes_ptr

);

typedef wms_status_e_type (wms_cfg_set_link_control_f_type) (

  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       *user_data,
  wms_cfg_link_control_mode_e_type control_option,
  uint8                            idle_timer /* in seconds */

);

typedef wms_status_e_type (wms_cfg_set_memory_full_f_type) (

  wms_client_id_type             client_id,
  wms_cmd_cb_type                cmd_cb,
  const void                     *user_data,
  boolean                        memory_full

);

typedef wms_status_e_type (wms_cfg_get_message_list_f_type) (

  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       *user_data,
  wms_memory_store_e_type          mem_store,
  wms_message_tag_e_type           tag

);

/* Message Group */

typedef wms_status_e_type (wms_msg_send_f_type) (

  wms_client_id_type                 client_id,
  wms_cmd_cb_type                    cmd_cb,
  const void                         *user_data,
  wms_send_mode_e_type               send_mode,
  const wms_client_message_s_type    *message_ptr

);

typedef wms_status_e_type (wms_msg_ack_f_type) (

  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  const wms_ack_info_s_type       *ack_info_ptr

);

typedef wms_status_e_type (wms_msg_write_f_type) (

  wms_client_id_type                 client_id,
  wms_cmd_cb_type                    cmd_cb,
  const void                         *user_data,
  wms_write_mode_e_type              write_mode,
  const wms_client_message_s_type    *message_ptr

);

typedef wms_status_e_type (wms_msg_delete_f_type) (

  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index

);

typedef wms_status_e_type (wms_msg_read_template_f_type) (

  wms_client_id_type              client_id,
  wms_cmd_cb_type                 cmd_cb,
  const void                      *user_data,
  wms_memory_store_e_type         mem_store,
  wms_message_index_type          index

);

typedef wms_status_e_type (wms_msg_write_template_f_type) (

  wms_client_id_type               client_id,
  wms_cmd_cb_type                  cmd_cb,
  const void                       *user_data,
  wms_write_mode_e_type            write_mode,
  const wms_client_message_s_type  *message_ptr

);
#endif /* FEATURE_QCRIL_DSDS */

#ifdef FEATURE_QCRIL_IMS_EXT
typedef wms_status_e_type (wms_msg_resend_f_type) (

  wms_client_id_type                 client_id,
  wms_cmd_cb_type                    cmd_cb,
  const void                         *user_data,
  wms_send_mode_e_type               send_mode,
  const wms_client_message_s_type    *message_ptr,
  wms_message_number_type            client_msg_id,
  wms_message_number_type            wms_msg_id

);
#endif /* FEATURE_QCRIL_IMS_EXT */

typedef struct {

  wms_cfg_event_cb_type     cfg_event_cb_func;
  wms_msg_event_cb_type     msg_event_cb_func;
  wms_bc_mm_event_cb_type   bc_mm_event_cb_func;

} qcril_wms_callback_funcs_type;

typedef struct {

  wms_client_release_f_type            *wms_client_release_func;
  wms_client_activate_f_type           *wms_client_activate_func;
  #ifdef FEATURE_QCRIL_WMS_PM
  wms_client_init_for_pm_f_type               *wms_client_init_func;
  wms_client_reg_cfg_cb_for_pm_f_type         *wms_client_reg_cfg_cb_func;
  wms_client_reg_msg_cb_for_pm_f_type         *wms_client_reg_msg_cb_func;
  wms_client_reg_bc_mm_cb_for_pm_f_type       *wms_client_reg_bc_mm_cb_func;
  #else
  wms_client_init_f_type               *wms_client_init_func;
  wms_client_reg_cfg_cb_f_type         *wms_client_reg_cfg_cb_func;
  wms_client_reg_msg_cb_f_type         *wms_client_reg_msg_cb_func;
  wms_client_reg_bc_mm_cb_f_type       *wms_client_reg_bc_mm_cb_func;
  #endif /* FEATURE_QCRIL_WMS_PM */

  #ifdef FEATURE_QCRIL_DSDS
  wms_cfg_ms_set_primary_client_f_type    *wms_cfg_set_primary_client_func;
  wms_cfg_ms_set_routes_f_type            *wms_cfg_set_routes_func;
  wms_cfg_ms_set_link_control_f_type      *wms_cfg_set_link_control_func;
  wms_cfg_ms_set_memory_full_f_type       *wms_cfg_set_memory_full_func;
  wms_cfg_ms_get_message_list_f_type      *wms_cfg_get_message_list_func;
                                      
  wms_msg_ms_send_f_type                  *wms_msg_send_func;
  wms_msg_ms_ack_f_type                   *wms_msg_ack_func;
  wms_msg_ms_write_f_type                 *wms_msg_write_func;
  wms_msg_ms_delete_f_type                *wms_msg_delete_func;
  wms_msg_ms_read_template_f_type         *wms_msg_read_template_func;
  wms_msg_ms_write_template_f_type        *wms_msg_write_template_func;

  wms_bc_ms_set_pref_f_type            *wms_bc_mm_set_pref_func;
  wms_bc_ms_get_table_f_type           *wms_bc_mm_get_table_func;
  wms_bc_ms_add_services_f_type        *wms_bc_mm_add_services_func;
  wms_bc_ms_delete_all_services_f_type *wms_bc_mm_delete_all_services_func;
  #else
  wms_cfg_set_primary_client_f_type    *wms_cfg_set_primary_client_func;
  wms_cfg_set_routes_f_type            *wms_cfg_set_routes_func;
  wms_cfg_set_link_control_f_type      *wms_cfg_set_link_control_func;
  wms_cfg_set_memory_full_f_type       *wms_cfg_set_memory_full_func;
  wms_cfg_get_message_list_f_type      *wms_cfg_get_message_list_func;
                                      
  wms_msg_send_f_type                  *wms_msg_send_func;
  wms_msg_ack_f_type                   *wms_msg_ack_func;
  wms_msg_write_f_type                 *wms_msg_write_func;
  wms_msg_delete_f_type                *wms_msg_delete_func;
  wms_msg_read_template_f_type         *wms_msg_read_template_func;
  wms_msg_write_template_f_type        *wms_msg_write_template_func;

  wms_bc_mm_set_pref_f_type            *wms_bc_mm_set_pref_func;
  wms_bc_mm_get_table_f_type           *wms_bc_mm_get_table_func;
  wms_bc_mm_add_services_f_type        *wms_bc_mm_add_services_func;
  wms_bc_mm_delete_all_services_f_type *wms_bc_mm_delete_all_services_func;
  #endif /* FEATURE_QCRIL_DSDS */
                                      
  #ifdef FEATURE_QCRIL_IMS_EXT
  wms_msg_resend_f_type                *wms_msg_resend_func;
  #endif /* FEATURE_QCRIL_IMS_EXT */

} qcril_wms_funcs_type;


/*===========================================================================

                          EXTERN API MAP 

===========================================================================*/

extern qcril_wms_callback_funcs_type qcril_sms_api_callbacks[ QCRIL_MAX_MODEM_ID ];
extern qcril_wms_funcs_type          qcril_sms_api_funcs[ QCRIL_MAX_MODEM_ID ];

#endif /* QCRIL_SMS_API_MAP_H */

