/* Copyright (c) 2010-2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 */

#ifndef ACDB_LOADER_H

int acdb_loader_init_v2(char *snd_card_name, char *cvd_version, int metaInfoKey);
int acdb_loader_init_ACDB(void);
int acdb_loader_get_default_app_type(void);
void acdb_loader_deallocate_ACDB(void);
void acdb_loader_send_voice_cal_v2(int rxacdb_id, int txacdb_id, int feature_set);
void acdb_loader_send_voice_cal(int rxacdb_id, int txacdb_id);
int acdb_loader_reload_vocvoltable(int feature_set);
void acdb_loader_send_audio_cal(int acdb_id, int capability);
void acdb_loader_send_audio_cal_v2(int acdb_id, int capability, int app_id, int sample_rate);
void acdb_loader_send_listen_cal(int acdb_id, int app_id);
int acdb_loader_send_anc_cal(int acdb_id);
void send_tabla_anc_data(void);
int acdb_loader_get_remote_acdb_id(unsigned int native_acdb_id);
int acdb_loader_get_ecrx_device(int acdb_id);
int acdb_loader_get_calibration(char *attr, int size, void *data);

#endif /* ACDB_LOADER_H */
