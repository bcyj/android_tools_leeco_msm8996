/* Copyright (c) 2010-2013 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 */

#ifndef ACDB_LOADER_H

int acdb_loader_init_ACDB(void);
void acdb_loader_deallocate_ACDB(void);
void acdb_loader_send_voice_cal(int rxacdb_id, int txacdb_id);
void acdb_loader_send_audio_cal(int acdb_id, int capability);
int acdb_loader_send_anc_cal(int acdb_id);
void send_tabla_anc_data(void);
#ifdef ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID
int acdb_loader_get_remote_acdb_id(unsigned int native_acdb_id);
#else
static int acdb_loader_get_remote_acdb_id(unsigned int native_acdb_id)
{
	return -1;
}
#endif /* ACDB_CMD_GET_COMPATIBLE_REMOTE_DEVICE_ID */
int acdb_loader_get_ecrx_device(int acdb_id);

#endif /* ACDB_LOADER_H */
