#ifndef DIAG_LSMI_H
#define DIAG_LSMI_H

/*===========================================================================

                   Diag Mapping Layer DLL , internal declarations

DESCRIPTION
  Internal declarations for Diag Service Mapping Layer.


Copyright (c) 2007-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/01/08   sj      Added featurization for WM specific code & CBSP2.0
02/04/08   mad     Created File
===========================================================================*/

#define DIAG_HANDLE fd
#define DIAG_INVALID_HANDLE -1
#define DIAG_MDLOG_DIR		"/sdcard/diag_logs/"
#define DIAG_MDLOG_PID_FILE	"/sdcard/diag_logs/diag_mdlog_pid"
#define DIAG_MDLOG_PID_FILE_SZ  34
#define NUM_PROC 10
extern int fd;
extern int fd_md[NUM_PROC];
extern int gdwClientID;
void log_to_device(unsigned char *ptr, int logging_mode, int size, int type);
void send_mask_modem(unsigned char mask_buf[], int count_mask_bytes);
int diag_has_remote_device(uint16 *remote_mask);
int diag_register_socket_cb(int (*callback_ptr)(void *data_ptr, int socket_id), void *data_ptr);
int diag_set_socket_fd(int socket_id, int socket_fd);
int diag_send_socket_data(int id, unsigned char buf[], int num_bytes);
int diag_get_max_channels(void);
int diag_read_mask_file_list(char *mask_list_file);
uint8 get_qshrink4_error(int error);
void set_qshrink4_dir(const char *dir);
const char *get_qshrink4_dir(void);

/* === Functions dealing with diag wakelocks === */

/* Returns 1 if a wakelock is initialized for this process,
   0 otherwise. */
int diag_is_wakelock_init(void);

/* Opens the wakelock files and initializes the wakelock for
   the current process. It doesn't hold any wakelock. To hold
   a wakelock, call diag_wakelock_acquire. */
void diag_wakelock_init(char *wakelock_name);

/* Closes the wakelock files. It doesn't release the wakelock
   for the current process if held. */
void diag_wakelock_destroy(void);

/* Acquires a wakelock for the current process under the name
   given by diag_wakelock_init. */
void diag_wakelock_acquire(void);

/* Releases the wakelock held by the current process. */
void diag_wakelock_release(void);

extern boolean gbRemote;
#define DIAG_LSM_PKT_EVENT_PREFIX "DIAG_SYNC_EVENT_PKT_"
#define DIAG_LSM_MASK_EVENT_PREFIX "DIAG_SYNC_EVENT_MASK_"
#endif /* DIAG_LSMI_H */

