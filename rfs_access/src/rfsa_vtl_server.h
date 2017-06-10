#ifndef __RFSA_VTL_SERVER_H__
#define __RFSA_VTL_SERVER_H__

/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

int32_t rfsa_vtl_server_init();
int32_t rfsa_vtl_server_deinit();
int32_t rfsa_vtl_server_response(rfsa_server_work_item_t *item);

#endif /* __RFSA_VTL_SERVER_H__ */
