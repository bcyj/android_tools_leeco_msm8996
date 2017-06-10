/*
 * Copyright (c) 2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 *
 */

#ifndef MCS_ROUTE_CTRL_H
#define MCS_ROUTE_CTRL_H

struct mcs_route_ctrl_info {
	FILE *file_hdl;
	int sndcard_num;

};

struct mcs_route_ctrl_info * mcs_route_ctrl_init(const char* config_filename, int sndcard_num);
int mcs_route_ctrl_set_path(struct mcs_route_ctrl_info * route_hdl, int acdb_id, int ena_flag, int * pcm_device);

#endif
