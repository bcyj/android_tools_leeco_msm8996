/* Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef CSD_ALSA_MAPPING_H
#define CSD_ALSA_MAPPING_H

#include <string.h>
#ifndef ANDROID
#define strndup g_strndup
#endif

#define INVALID_ACDB_ID 0xff

#define DEVICE_HANDSET_RX_ACDB_ID           7 // HANDSET_SPKR
#define DEVICE_HANDSET_TX_ACDB_ID           4 // HANDSET_MIC
#define DEVICE_DUALMIC_HANDSET_TX_ENDFIRE_ACDB_ID       41 // HANDSET_MIC_ENDFIRE
#define DEVICE_SPEAKER_RX_ACDB_ID           14// SPKR_PHONE_SPKR_MONO
#define DEVICE_SPEAKER_TX_ACDB_ID           11// SPKR_PHONE_MIC
#define DEVICE_DUALMIC_SPEAKER_TX_ENDFIRE_ACDB_ID       43 // SPEAKER_MIC_ENDFIRE
#define DEVICE_HEADSET_RX_ACDB_ID           10// HEADSET_SPKR_STEREO
#define DEVICE_HEADSET_TX_ACDB_ID           8 // HEADSET_MIC
#define DEVICE_TTY_HEADSET_MONO_RX_ACDB_ID    17// TTY_HEADSET_SPKR
#define DEVICE_TTY_HEADSET_MONO_TX_ACDB_ID    16// TTY_HEADSET_MIC
#define DEVICE_TTY_VCO_HANDSET_TX_ACDB_ID     36// TTY_VCO_HANDSET_MIC
#define DEVICE_TTY_HCO_HANDSET_RX_ACDB_ID     37// TTY_HCO_HANDSET_SPKR

// Use headset calibration
#define DEVICE_SPEAKER_HEADSET_RX_ACDB_ID       DEVICE_HEADSET_RX_ACDB_ID


struct acdb_id_info
{
    char dev_name[80];
    int acdb_id;
};

static struct acdb_id_info mapping_table[] =
{
    {"Voice Earpiece", DEVICE_HANDSET_RX_ACDB_ID},
    {"Handset", DEVICE_HANDSET_TX_ACDB_ID},
    {"DMIC Endfire", DEVICE_DUALMIC_HANDSET_TX_ENDFIRE_ACDB_ID},
    {"Speaker", DEVICE_SPEAKER_RX_ACDB_ID},
    {"Voice Headphones", DEVICE_HEADSET_RX_ACDB_ID},
    {"Headset", DEVICE_HEADSET_TX_ACDB_ID},
    {"Line", DEVICE_SPEAKER_TX_ACDB_ID},
    {"Speaker DMIC Endfire", DEVICE_DUALMIC_SPEAKER_TX_ENDFIRE_ACDB_ID},
    /* Added new CSD TTY devices in UCM such that they do not set tty modes.
     * This is because client sets tty mode in a seperate call from csd-server.
     */
    {"CSD TTY Headset Rx", DEVICE_TTY_HEADSET_MONO_RX_ACDB_ID},
    {"CSD TTY Headset Tx", DEVICE_TTY_HEADSET_MONO_TX_ACDB_ID},
    {"CSD TTY Handset Rx", DEVICE_TTY_HCO_HANDSET_RX_ACDB_ID},
    {"CSD TTY Handset Tx", DEVICE_TTY_VCO_HANDSET_TX_ACDB_ID},
};

char *tty_mode_ctl[] = {"OFF", "HCO", "VCO", "FULL"};

struct session_id_to_usecase_type
{
    uint session_id;
    char verb_name[80];
};

static struct session_id_to_usecase_type session_id_to_verb_table[] =
{
    {VOICE_SESSION_ID, "Voice Call"},
    {VOLTE_SESSION_ID, "VoLTE"},
};

static struct session_id_to_usecase_type session_id_to_mod_table[] =
{
    {VOICE_SESSION_ID, "Play Voice"},
    {VOLTE_SESSION_ID, "Play VoLTE"},
};

struct csd_alsa_card_mapping
{
    char kernel_card_name[50];
    char ucm_card_name[50];
};

static struct csd_alsa_card_mapping csd_alsa_card_mapping_list[] =
{
    {"mdm9615-tabla-snd-card", "snd_soc_msm_I2S"},
    {"mdm9615-tabla-snd-card-i2s", "snd_soc_msm_I2S"},
    {"mdm9625-taiko-i2s-snd-card", "snd_soc_msm_Taiko_I2S"},
    {"mdm9630-taiko-i2s-snd-card", "snd_soc_msm_Taiko_I2S"},
    {"msmzirc-tomtom-i2s-snd-card", "snd_soc_msm_Tomtom_I2S"},
};

#define CSD_ALSA_MAX_NUM_CARDS (sizeof(csd_alsa_card_mapping_list)/sizeof(struct csd_alsa_card_mapping))

int get_acdb_device_name_from_acdb_id(int acdb_id, char **device_name)
{
    unsigned int idx;
    for (idx = 0; idx < (sizeof(mapping_table)/sizeof(struct acdb_id_info));
         idx++) {
        if(acdb_id == mapping_table[idx].acdb_id) {
            *device_name = (char *)strndup(mapping_table[idx].dev_name,
                                           strlen(mapping_table[idx].dev_name));
            return 0;
        }
    }
    return -1;
}

int get_verb_name_from_session_id(uint session_id, char **verb_name)
{
    unsigned int idx;
    for (idx = 0;
         idx < (sizeof(session_id_to_verb_table)/sizeof(struct session_id_to_usecase_type));
         idx++) {
        if(session_id ==  session_id_to_verb_table[idx].session_id) {
            *verb_name = (char *)strndup(session_id_to_verb_table[idx].verb_name,
                               strlen(session_id_to_verb_table[idx].verb_name));
            return 0;
        }
    }
    return -1;
}

int get_modifier_name_from_session_id(uint session_id, char **mod_name)
{
    unsigned int idx;
    for (idx = 0;
         idx < (sizeof(session_id_to_mod_table)/sizeof(struct session_id_to_usecase_type));
         idx++){
        if(session_id ==  session_id_to_mod_table[idx].session_id) {
            *mod_name = (char *)strndup(session_id_to_mod_table[idx].verb_name,
                                strlen(session_id_to_mod_table[idx].verb_name));
            return 0;
        }
    }
    return -1;
}

#endif /* QMI_AUDIO_MAPPER_H */
