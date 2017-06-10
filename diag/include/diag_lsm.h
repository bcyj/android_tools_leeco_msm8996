#ifndef DIAG_LSM_H
#define DIAG_LSM_H

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================

                   Diag Mapping Layer DLL declarations

DESCRIPTION
  Function declarations for Diag Service Mapping Layer


# Copyright (c) 2007-2012, 2014 by Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:

when       who     what, where, why
--------   ---     ----------------------------------------------------------
02/04/08   mad     Added declarations for Diag_LSM_Init and DeInit functions
                   to enable diag clients to call these directly.
                   Moved IDiagPkt handle declaration to an internal header
                   file, Diag_LSMi.h
11/29/07   mad     Created File
===========================================================================*/

#define MSG_MASKS_TYPE		0x00000001
#define LOG_MASKS_TYPE		0x00000002
#define EVENT_MASKS_TYPE	0x00000004
#define PKT_TYPE		0x00000008
#define DEINIT_TYPE		0x00000010
#define USER_SPACE_DATA_TYPE	0x00000020
#define DCI_DATA_TYPE		0x00000040
#define CALLBACK_DATA_TYPE	0x00000080
#define DCI_LOG_MASKS_TYPE	0x00000100
#define DCI_EVENT_MASKS_TYPE	0x00000200
#define DCI_PKT_TYPE		0x00000400

#define USB_MODE		1
#define MEMORY_DEVICE_MODE	2
#define NO_LOGGING_MODE		3
#define UART_MODE		4
#define SOCKET_MODE		5
#define CALLBACK_MODE		6

#define MAX_NUM_FILES_ON_DEVICE 2000 /* If user wants to stop logging on SD after reaching a max file limit */
#define CONTROL_CHAR 0x7E
#define FILE_NAME_LEN 100
#define NUM_PROC 10
/* Token to identify MDM log */
#define MDM_TOKEN      -1
/* Token to identify QSC log */
#define QSC_TOKEN      -5
#define MSM	0
#define MDM	1
#define QSC	2

#define MODE_NONREALTIME	0
#define MODE_REALTIME		1
#define MODE_UNKNOWN		2

#define DIAG_PROC_DCI		1
#define DIAG_PROC_MEMORY_DEVICE	2


#ifdef ANDROID
	#define LOG_TAG "Diag_Lib"
	#define DIAG_LOGE(...)  { \
		ALOGE(__VA_ARGS__); \
		if (!diag_disable_console) \
			printf(__VA_ARGS__); \
	}
	#include <cutils/log.h>
        #include "common_log.h"
#else
	#define DIAG_LOGE(...) printf (__VA_ARGS__)
#endif

extern int logging_mode;
extern char mask_file[FILE_NAME_LEN];
extern char mask_file_mdm[FILE_NAME_LEN];
extern char output_dir[NUM_PROC][FILE_NAME_LEN];
extern int diag_disable_console;
extern char dir_name[FILE_NAME_LEN];

/* This is used for book keeping of each client. */
/* NOTE: Please always add new member at the end of the structure */
struct diag_client_callback_table {

	/* Current logging proc */
	int proc_type;
	int pid;

	/* Callback function format has to be followed. The input parameters will be
	   buffer pointer, length and context data */
	int (*cb_func_ptr)(unsigned char *, int len, void *context_data);
	void *context_data;
};
/*===========================================================================
FUNCTION   Diag_LSM_Init

DESCRIPTION
  Initializes the Diag Legacy Mapping Layer. This should be called
  only once per process.

DEPENDENCIES
  Successful initialization requires Diag CS component files to be present
  and accessible in the file system.

RETURN VALUE
  FALSE = failure, else TRUE

SIDE EFFECTS
  None

===========================================================================*/
boolean Diag_LSM_Init (byte* pIEnv);

/*===========================================================================
FUNCTION   diag_switch_logging

DESCRIPTION
  This swtiches the logging mode from default USB to memory device logging

DEPENDENCIES
  valid data type to be passed in:
  In case of ODL second argument is to specifying directory location.
  In case of UART mode second argument is specify PROC type.

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/

void diag_switch_logging(int requested_mode, char *dir_location);
/*===========================================================================
FUNCTION   diag_read_mask_file

DESCRIPTION
  This reads the mask file

DEPENDENCIES
  valid data type to be passed in

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/

int diag_read_mask_file(void);
/*===========================================================================
FUNCTION   diag_register_callback

DESCRIPTION
  This allows diag client to register a callback function with LSM library.
  If the library receives data from kernel space, it will invoke this call
  back function, thus passing the data to the client through this function.

DEPENDENCIES
  valid data type to be passed in

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void diag_register_callback(int (*client_cb_func_ptr)(unsigned char *ptr,
				int len, void *context_data), void *context_data);

/*===========================================================================
FUNCTION   diag_register_remote_callback

DESCRIPTION
  This allows diag client to register a callback function with LSM library.
  If the library receives data from kernel space originating from the remote
  processor, it will invoke this call back function, thus passing the data
  to the client through this function.

DEPENDENCIES
  valid data type to be passed in

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
void diag_register_remote_callback(int (*client_rmt_cb_func_ptr)(unsigned char *ptr,
					int len, void *context_data), int proc,
					void *context_data);

/*===========================================================================

FUNCTION    diag_send_data

DESCRIPTION
  Inject data into diag kernel driver

DEPENDENCIES
  None.

RETURN VALUE
  FALSE = failure, else TRUE.

SIDE EFFECTS
  None

===========================================================================*/
int diag_send_data(unsigned char *, int);

/*===========================================================================

FUNCTION    diag_callback_send_data

DESCRIPTION
  Inject data into diag kernel driver for a specific processor in
  callback mode

DEPENDENCIES
  None.

RETURN VALUE
  FALSE = failure, else TRUE.

SIDE EFFECTS
  None

===========================================================================*/
int diag_callback_send_data(int proc, unsigned char * buf, int len);

/*===========================================================================

FUNCTION    diag_vote_md_real_time

DESCRIPTION
  Votes the on device logging process for real/non-real time
  mode

DEPENDENCIES
  None.

RETURN VALUE
  0 = success, -1 = failure

SIDE EFFECTS
  Puts the entire diag in the mode specified if the process wins
  the vote

===========================================================================*/
int diag_vote_md_real_time(int real_time);

/*===========================================================================

FUNCTION    diag_get_real_time_status

DESCRIPTION
  Gets the mode (real time or non real time) in which Diag is in

DEPENDENCIES
  None.

RETURN VALUE
  0 = success, -1 = failure

SIDE EFFECTS
  None

===========================================================================*/
int diag_get_real_time_status(int *real_time);

/*===========================================================================

FUNCTION    diag_get_real_time_status_proc

DESCRIPTION
  Gets the mode (real time or non real time) in which Diag is
  in, in a particular processor

DEPENDENCIES
  None.

RETURN VALUE
  0 = success, -1 = failure

SIDE EFFECTS
  None

===========================================================================*/
int diag_get_real_time_status_proc(int proc, int *real_time);

/*===========================================================================

FUNCTION    Diag_LSM_DeInit

DESCRIPTION
  De-Initialize the Diag service.

DEPENDENCIES
  None.

RETURN VALUE
  FALSE = failure, else TRUE.
  Currently all the internal boolean return functions called by
  this function just returns TRUE w/o doing anything.

SIDE EFFECTS
  None

===========================================================================*/
boolean Diag_LSM_DeInit(void);

/*===========================================================================

FUNCTION    diag_configure_peripheral_buffering_tx_mode

DESCRIPTION
  Configure the  peripheral Diag's TX mode to Streaming, Circular, or
  Threshold buffering mode  and set high and low watermark threshold limits.
  Streaming Mode is a default TX mode for peripheral Diag.
  Switching to Threshold or Circular buffering mode puts the  peripheral
  Diag to Non-Real Time mode (NRT).
  Switching to streaming mode will put the peripheral to Real Time (RT) mode.

DEPENDENCIES
  None.

RETURN VALUE
  1 = success, else failure

SIDE EFFECTS
  Clients cannot vote for real/non-real time when the Tx mode is set
  to Circular, or Threshold buffering mode.

===========================================================================*/
int diag_configure_peripheral_buffering_tx_mode(uint8 peripheral, uint8 tx_mode,
						uint8 low_wm_val, uint8 high_wm_val);

/*===========================================================================

FUNCTION    diag_peripheral_buffering_drain_immediate

DESCRIPTION

Request  the peripheral to drain its Tx buffering pool immediately.
If peripheral Diag receives this request in
Streaming mode - No action is taken since Diag is already streaming.
Threshold or Circular buffering modes - Diag will drain its Tx buffering
pool until the low watermark threshold is reached, and then resume
buffering in the tx mode it was set


DEPENDENCIES
  None.

RETURN VALUE
  1 = success, else failure

SIDE EFFECTS
  None

===========================================================================*/

int diag_peripheral_buffering_drain_immediate(uint8 peripheral);

#ifdef __cplusplus
}
#endif

#endif /* DIAG_LSM_H */

