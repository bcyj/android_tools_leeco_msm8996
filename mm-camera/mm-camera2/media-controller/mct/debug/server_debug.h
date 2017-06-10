/* server_debug.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define   MAX_FD_PER_PROCESS  250
void dump_list_of_daemon_fd();
void reset_fd_dump();
unsigned int dump_done;

void* mct_controller_dump_data_for_sof_freeze();
