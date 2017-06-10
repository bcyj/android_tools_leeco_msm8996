/* Copyright (c) 2009-2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef ALSA_CONTROL_H
#define ALSA_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

struct msm_control {
	int card;
	int fd;
};

extern int msm_mixer_count(void);
extern int msm_mixer_open(const char *name, int card);
extern int msm_mixer_close(void);
int msm_get_device(const char * name);
int msm_en_device(int dev_id, int set);
int msm_route_stream(int dir, int dec_id, int dev_id, int set);
int msm_route_voice (int rx_dev_id, int tx_dev_id, int set);
int msm_set_volume(int dec_id, float volume);
int msm_get_device_class(int device_id);
int msm_get_device_capability(int device_id);
const char **msm_get_device_list(void);
int msm_get_device_count(void);
int msm_start_voice(void);
int msm_end_voice(void);
int msm_set_voice_tx_mute(int mute);
int msm_set_voice_rx_vol(int volume);
int msm_set_device_volume(int dev_id, int volume);
int msm_reset_all_device(void);
int msm_enable_anc(int dev_id, int enable);
int msm_set_dual_mic_config(int enc_session_id, int config);
int msm_snd_dev_loopback (int rx_dev_id, int tx_dev_id, int set);
int msm_device_mute(int dev_id, int mute);
int msm_start_voice_ext(int session_id);
int msm_end_voice_ext(int session_id);
int msm_set_voice_tx_mute_ext(int mute, int session_id);
int msm_set_voice_rx_vol_ext(int volume, int session_id);
/* Valid session names: "Voice session" or "VoIP session" */
#define VOICE_SESSION_NAME "Voice session"
#define VOIP_SESSION_NAME "VoIP session"
int msm_get_voc_session(const char *name);
#ifdef __cplusplus
}
#endif

#endif /* ALSA_CONTROL_H */
