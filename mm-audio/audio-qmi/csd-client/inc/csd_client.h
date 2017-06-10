/* Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef CSD_CLIENT_H
#define CSD_CLIENT_H

#define VOICE_SESSION_VSID  0x10C01000
#define VOICE2_SESSION_VSID 0x10DC1000
#define VOLTE_SESSION_VSID  0x10C02000
#define VOWLAN_SESSION_VSID 0x10002000
#define ALL_SESSION_VSID    0xFFFFFFFF

int csd_client_init(bool i2s_ext_modem);
int csd_client_deinit(void);
int csd_client_start_voice(uint32_t vsid);
int csd_client_stop_voice(uint32_t vsid);
int csd_client_standby_voice(uint32_t vsid);
int csd_client_resume_voice(uint32_t vsid);
int csd_client_start_playback(uint32_t vsid);
int csd_client_stop_playback(uint32_t vsid);
int csd_client_start_record(uint32_t vsid, int rec_mode);
int csd_client_stop_record(uint32_t vsid);
int csd_client_disable_device(void);
int csd_client_enable_device_config(int rx_dev_id, int tx_dev_id);
int csd_client_enable_device(int rx_dev_id, int tx_dev_id, uint32_t flags);
int csd_client_volume(uint32_t vsid, int volume, uint16_t ramp_duration);
int csd_client_mic_mute(uint32_t vsid, int mute, uint16_t ramp_duration);
int csd_client_slow_talk(uint32_t vsid, uint8_t flag);
int csd_client_set_rx_mute(uint32_t vsid, int mute, uint16_t ramp_duration);
int csd_client_get_sample_rate(int *sample_rate);
int csd_client_set_lch(uint32_t vsid, enum voice_lch_mode lch_mode);
#endif /* CSD_CLIENT_H */

