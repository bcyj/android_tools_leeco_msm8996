/*!
  @file
  dsi_netctrl_mni.h

  @brief
  Exports dsi_netctrl modem network interface functions

*/

/*===========================================================================

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary and Confidential.

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
04/19/10   js      created

===========================================================================*/
#ifndef _DSI_NETCTRL_MNI_
#define _DSI_NETCTRL_MNI_

/* inits modem network interface(s) */
extern int dsi_mni_init(void);

extern void dsi_mni_cleanup(void);

/* init mni client */
int dsi_mni_init_client(int conn_id);

/* starts modem network interface */
extern int dsi_mni_start(int i, dsi_store_t * st);

/* stops modem network interface */
extern int dsi_mni_stop(int i, dsi_store_t * st);

/* aborts previous start request */
extern int dsi_mni_abort_start(int i, dsi_store_t * st);

/* looks up the appropriate modem network interface */
extern int dsi_mni_look_up(dsi_store_t * st_hndl, int * pi);

/* register for handoff indications */
extern int dsi_mni_reg_unreg_handoff_ind(int pi, dsi_store_t * st_hndl, boolean reg_unreg);

/* register WDS indication */
extern int dsi_mni_register_embms_ind(int pi, dsi_store_t * st_hndl);

/* activate EMBMS TMGI */
extern int dsi_mni_embms_tmgi_activate(int pi, dsi_store_t *st_hndl);

/* deactivate EMBMS TMGI */
extern int dsi_mni_embms_tmgi_deactivate(int pi, dsi_store_t *st_hndl);

/* activate and deactivate EMBMS TMGI at the same time */
extern int dsi_mni_embms_tmgi_activate_deactivate(int pi, dsi_store_t *st_hndl);

/* query active/available EMBMS TMGI list */
extern int dsi_mni_embms_tmgi_list_query(int pi, dsi_store_t *st_hndl);

/* Content desc update EMBMS TMGI */
extern int dsi_mni_embms_tmgi_content_desc_update(int pi, dsi_store_t *st_hndl);

/* can be called to release qmi client */
extern void dsi_mni_release_client(int conn_id);

#endif /* _DSI_NETCTRL_MNI_ */
