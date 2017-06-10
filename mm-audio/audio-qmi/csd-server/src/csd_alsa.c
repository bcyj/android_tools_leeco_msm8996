/* Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <sound/asound.h>
#include "alsa_audio.h"
#include "csd_alsa.h"
#include "acdb-loader.h"
#include "csd_alsa_mapping.h"
#include "csd_server.h"
#include "alsa_audio.h"
#include "limits.h"

pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition = PTHREAD_COND_INITIALIZER;

struct csd_common_data csd_common_data = {0};

static struct alsa_usecase_list active_ucList;

static void alsa_clean_ucm_list()
{
    struct alsa_usecase_list *uc_list = &active_ucList;
    struct modifier *modptr = uc_list->mod_head;

    if ((uc_list->verb != NULL) &&
        (strncmp(uc_list->verb, SND_USE_CASE_VERB_INACTIVE,
                strlen(SND_USE_CASE_VERB_INACTIVE)))) {
        LOGE("alsa_clean_ucm_list: Verb is set to inactive");
        strlcpy(uc_list->verb, SND_USE_CASE_VERB_INACTIVE,
                sizeof(SND_USE_CASE_VERB_INACTIVE));
    }

    while (modptr != NULL) {
        LOGD("alsa_clean_ucm_list: Mod is cleared");
        uc_list->mod_head = uc_list->mod_head->next;
        free(modptr->mod);
        free(modptr);
        modptr = uc_list->mod_head;
    }
    uc_list->mod_size = 0;
}

static int alsa_update_ucm_list(char *use_case,
                                char *mod_case, int mod_size)
{
    int i;
    char *mod;
    int rc = 0;
    struct alsa_usecase_list *uc_list = &active_ucList;
    struct modifier *mod_ptr, *prev;

    if (use_case != NULL) {
        strlcpy(uc_list->verb, use_case, strlen(use_case) + 1);
        LOGD("alsa_update_ucm_list: Verb is saved");
    }

    if (mod_size > 0) {
            uc_list->mod_size = mod_size;
            prev = NULL;
        for (i = 0; i < mod_size; i++) {
            mod = (char *)malloc(sizeof(mod_case[i]));
            mod_ptr = (struct modifier *)malloc(sizeof(struct modifier));
            if (mod_ptr != NULL && mod != NULL) {
                strlcpy(mod, mod_case[i], strlen((char *)mod_case[i]));
                mod_ptr->mod = (char *)mod;
                mod_ptr->next = prev;
                prev = mod_ptr;
                uc_list->mod_head = mod_ptr;
                LOGD("alsa_update_ucm_list: Mods are copied\n");
            } else {
                LOGE("alsa_update_ucm_list: Failed to allocate memory");
                if (mod) {
                    free(mod);
                }
                goto ERROR;
            }
        }
    }

EXIT:
    return rc;
ERROR:
    while (mod_ptr != NULL) {
        uc_list->mod_head = uc_list->mod_head->next;
        free(mod_ptr->mod);
        free(mod_ptr);
        mod_ptr = uc_list->mod_head;
    }
    uc_list->mod_size = 0;
    return rc;
}

static int alsa_del_ucm_entry(char *use_case)
{
    struct alsa_usecase_list *uc_list = &active_ucList;
    int i = 0, mod_size = uc_list->mod_size;
    struct modifier *prev = NULL;
    struct modifier *modptr = uc_list->mod_head;
    int rc = 0;

    LOGD("alsa_del_ucm_entry: Use case to delete", use_case);
    if (!(strncmp(uc_list->verb, use_case,
                  strlen(use_case)))) {
        strlcpy(uc_list->verb, SND_USE_CASE_VERB_INACTIVE,
                strlen(SND_USE_CASE_VERB_INACTIVE) + 1);
        LOGD("alsa_del_ucm_entry: Delete Verb Entry", use_case);
        goto EXIT;
    }

    if (mod_size > 0) {
        while (modptr != NULL) {
            if (!(strncmp(modptr->mod, use_case, strlen(use_case)))) {
                if (uc_list->mod_head == modptr)
                    uc_list->mod_head = modptr->next;
                else
                    prev->next = modptr->next;
                uc_list->mod_size--;
                free(modptr->mod);
                free(modptr);
                LOGE("alsa_del_ucm_entry: Delete Mod Entry", use_case);
                break;
            } else {
                prev = modptr;
                modptr = modptr->next;
            }
        }
    }
EXIT:
    return rc;
}

int csd_alsa_disable_device(struct client_data *clnt_data, int devid1,
                            int devid2)
{

    int i, mod_size;
    int rc = 0;
    char *use_case, *mod_case;
    const char **mod_list;
    struct alsa_usecase_list *uc_list = &active_ucList;
    char *dev1, *dev2;
    struct modifier *modptr = uc_list->mod_head;

    rc = get_acdb_device_name_from_acdb_id(devid1, &dev1);
    if (rc != 0) {
        LOGE("Unalbe to get device name from devid1=%d\n", devid1);
        goto EXIT;
    }

    rc = get_acdb_device_name_from_acdb_id(devid2, &dev2);
    if (rc != 0) {
        LOGE("Unalbe to get device name from devid2=%d\n", devid2);
        goto EXIT;
    }

    rc = snd_use_case_get(csd_common_data.ucm_mgr, "_verb",
                     (const char **)&use_case);
    if ((rc != 0) || (use_case == NULL)) {
        LOGE("ERROR snd_use_case_get() failed rc=%d,or use_case is NULL\n", rc);
        goto EXIT;
    }
    LOGD("csd_alsa_disable_device() current verb=%s\n", use_case);

    if (0 != strncmp(use_case, SND_USE_CASE_VERB_INACTIVE,
                     strlen(SND_USE_CASE_VERB_INACTIVE))) {
        rc = alsa_update_ucm_list(use_case, NULL, 0);
        if (rc != 0) {
            LOGE("\nFailed to update the ucm temp list rc=%d\n", rc);
            goto EXIT;
        }
        LOGD("\nDisabling verb=%s\n", use_case);
        rc = snd_use_case_set(csd_common_data.ucm_mgr, "_verb",
                                  SND_USE_CASE_VERB_INACTIVE);
        if (rc != 0) {
            LOGE("\nFailed to Disable UCM verb=%s rc=%d\n",
                 SND_USE_CASE_VERB_INACTIVE, rc);
            goto EXIT;
        }
    }

    mod_size = snd_use_case_get_list(csd_common_data.ucm_mgr,
                                     "_enamods", &mod_list);
    LOGD("csd_alsa_disable_device() mod_size=%d\n", mod_size);

    if (mod_size > 0) {
        while(modptr != NULL) {
            rc = alsa_update_ucm_list(NULL, mod_list, mod_size);
            if (rc != 0) {
                LOGE("\nFailed to update the ucm temp list rc=%d\n", rc);
                goto EXIT;
            }
            rc = snd_use_case_set(csd_common_data.ucm_mgr, "_dismod",
                                    modptr->mod);
            if (rc != 0) {
                LOGE("\n Failed to disable mod=%s rc=%d\n",
                        modptr->mod, rc);
                goto EXIT;
            }
            modptr = modptr->next;
        }
    }

    LOGD("\n\nDisabling device1=%s\n", dev1);
    rc = snd_use_case_set(csd_common_data.ucm_mgr, "_disdev", dev1);
    if (rc != 0) {
        LOGE("Failed to set UCM device=%s\n", dev1);
        goto EXIT;
    }

    LOGD("\n\nDisabling device2=%s\n", dev2);
    rc = snd_use_case_set(csd_common_data.ucm_mgr, "_disdev", dev2);
    if (rc != 0) {
        LOGE("Failed to set UCM device=%s\n", dev2);
        goto EXIT;
    }

EXIT:
    free(use_case);
    return rc;
}

bool is_ttymode_valid_for_devices(int devid1, int devid2, int ttymode)
{
    bool is_valid = false;

    if (devid1 == 0 || devid2 == 0) {
        LOGD("Device enable command not yet recieved\n");
        is_valid = true;
        goto EXIT;
    }

    switch (ttymode) {
            case HCO:
                 if ((devid1 == DEVICE_TTY_HEADSET_MONO_TX_ACDB_ID  &&
                     devid2 == DEVICE_TTY_HCO_HANDSET_RX_ACDB_ID) ||
                     (devid1 == DEVICE_TTY_HCO_HANDSET_RX_ACDB_ID &&
                     devid2 == DEVICE_TTY_HEADSET_MONO_TX_ACDB_ID ))
                     is_valid = true;
                 break;

            case VCO:
                 if ((devid1 ==  DEVICE_TTY_HEADSET_MONO_RX_ACDB_ID &&
                     devid2 == DEVICE_TTY_VCO_HANDSET_TX_ACDB_ID) ||
                     (devid1 == DEVICE_TTY_VCO_HANDSET_TX_ACDB_ID &&
                     devid2 == DEVICE_TTY_HEADSET_MONO_RX_ACDB_ID))
                     is_valid = true;
                 break;

            case FULL:
                 if ((devid1 == DEVICE_TTY_HEADSET_MONO_RX_ACDB_ID &&
                     devid2 == DEVICE_TTY_HEADSET_MONO_TX_ACDB_ID ) ||
                     (devid1 == DEVICE_TTY_HEADSET_MONO_TX_ACDB_ID &&
                     devid2 == DEVICE_TTY_HEADSET_MONO_RX_ACDB_ID ))
                     is_valid = true;
                 break;

            /* If tty mode is OFF or not set, we don't care what devices are chosen */
            case OFF:
            default:
                 LOGD("TTY mode has not yet been set\n");
                 is_valid = true;
                 break;
    }

EXIT:
    return is_valid;
}

int csd_alsa_enable_device(struct client_data *clnt_data, int devid1,
                           int devid2)
{
    struct alsa_usecase_list *uc_list = &active_ucList;
    int i, mod_size = uc_list->mod_size;
    char *useCase;
    int rc = 0;
    bool is_valid = false;
    char *dev1, *dev2;
    struct modifier *modptr = uc_list->mod_head;
    int ttymode = 0;

    /* Search through tty modes from all sessions as only one tty mode from a session is
     * used at a time.  Also all tty modes must be cleared when devices are cleared.
     */
    for (i = 0; i < MAX_SESSIONS_PER_CLIENT; i++)
         if (clnt_data->session[i].voice_stream_manager_data.tty_mode > 0)
             ttymode = clnt_data->session[i].voice_stream_manager_data.tty_mode;

    is_valid = is_ttymode_valid_for_devices(devid1, devid2, ttymode);
    if (!is_valid) {
        LOGE("Device %d, %d combination with tty mode %d is invalid\n",
             devid1, devid2, ttymode);
        rc = 1;
        goto EXIT;
    }

    rc = get_acdb_device_name_from_acdb_id(devid1, &dev1);
    if (rc != 0) {
          LOGE("Unalbe to get device name from devid1=%d\n",
               devid1);
          goto EXIT;
    }

    rc = get_acdb_device_name_from_acdb_id(devid2, &dev2);
    if (rc != 0) {
        LOGE("Unalbe to get device name from devid2=%d\n",
             devid2);
        goto EXIT;
    }

    LOGE("\nEnabling device name dev1=%s\n", dev1);
    rc = snd_use_case_set(csd_common_data.ucm_mgr, "_enadev", dev1);
    if (rc != 0) {
        LOGE("Failed to set UCM device=%s error=%d\n", dev1, rc);
        rc = 0;
    }

    LOGE("Enabling device name dev2=%s\n", dev2);
    rc = snd_use_case_set(csd_common_data.ucm_mgr, "_enadev", dev2);
    if (rc != 0) {
        LOGE("Failed to set UCM device=%s error=%d\n", dev2, rc);
        rc = 0;
    }

    /*
     * Enable the Verb or/and Mod
     */
    LOGD("\nVerb=%s\n", uc_list->verb);
    if ((strlen(uc_list->verb) > 0) &&
        strncmp(uc_list->verb, SND_USE_CASE_VERB_INACTIVE,
                strlen(SND_USE_CASE_VERB_INACTIVE))) {

        rc = snd_use_case_set(csd_common_data.ucm_mgr, "_verb",
                              uc_list->verb);
        if (rc != 0) {
            LOGE("enable_device:Failed to set verb=%s\n", uc_list->verb);
            goto EXIT;
        }
    }

    if (mod_size > 0) {
        while (modptr != NULL) {
            rc = snd_use_case_set(csd_common_data.ucm_mgr, "_enamod",
                                  modptr->mod);
            if (rc != 0) {
                LOGE("Failed to set modifier=%s\n", modptr->mod);
                goto EXIT;
            }
            modptr = modptr->next;
        }
    }

    alsa_clean_ucm_list();
EXIT:
    return rc;
}

static int get_bits_per_sample_for_deviceid(uint32_t device_id,
                                            uint32_t *bit_per_sample)
{
    int rc = 1;
    int i = 0;
    for (i = 0; i < MAX_NO_ACTIVE_DEVICES; i++) {
        if (csd_common_data.device_data.devices[i].dev_id == device_id) {
            *bit_per_sample =
              csd_common_data.device_data.devices[i].dev_attrib.bits_per_sample;
            rc = 0;
        }
    }
    return rc;
}

static int get_sample_rate_for_deviceid(uint32_t device_id,
                                        uint32_t *sample_rate)
{
    int rc = 1;
    int i = 0;
    for (i = 0; i < MAX_NO_ACTIVE_DEVICES; i++) {
        if (csd_common_data.device_data.devices[i].dev_id == device_id) {
            *sample_rate =
                csd_common_data.device_data.devices[i].dev_attrib.sample_rate;
            rc = 0;
        }
    }
    return rc;
}

static int alsa_set_mixer_ctl(char *mixer_ctl_name, void *value)
{
    struct mixer_ctl* ctl=NULL;
    int rc = 0;

    if (mixer_ctl_name == NULL) {
        LOGE("mixer_ctl_name is NULL\n");
        rc = 1;
        goto EXIT;
    }

    if (value == NULL) {
        LOGE("value is NULL\n");
        rc = 1;
        goto EXIT;
    }

    ctl = mixer_get_control(csd_common_data.alsa_mixer, mixer_ctl_name, 0);
    if (ctl == NULL) {
        LOGE("Could not get the mixer control\n");
        rc = 1;
        goto EXIT;
    }

    if (ctl->info->type == SNDRV_CTL_ELEM_TYPE_ENUMERATED) {
        rc = mixer_ctl_select(ctl, (char*)value);
    } else {
        rc = mixer_ctl_set(ctl, *((uint32_t*)value));
    }

    if (rc != 0)
    {
        LOGE("ERROR failed to set ctl:%s value:%d rc:%d\n", mixer_ctl_name,
             value, rc);
        goto EXIT;
    }

EXIT:
    return rc;
}

static int alsa_set_mixer_ctl_ext(char *mixer_ctl_name, int count,
                                  char **setValues)
{
    struct mixer_ctl *ctl = NULL;
    int rc = 0;

    LOGD("%s:: name %s count %d", __func__, mixer_ctl_name, count);

    if (!mixer_ctl_name) {
        LOGE("%s: mixer_ctl_name is NULL\n", __func__);
        rc = 1;
        goto EXIT;
    }

    ctl = mixer_get_control(csd_common_data.alsa_mixer, mixer_ctl_name, 0);
    if(ctl == NULL) {
        LOGE("%s: Could not get the mixer control\n", __func__);
        rc = 1;
        goto EXIT;
    }

    rc = mixer_ctl_set_value(ctl, count, setValues);
    if (rc != 0)
    {
        LOGE("%s: ERROR failed to set ctl:%s rc:%d\n", __func__,
             mixer_ctl_name, rc);
        goto EXIT;
    }

EXIT:
    return rc;
}

static int get_ucm_sound_card_name(char **ucm_sound_card_name)
{
    FILE *fp;
    int rc = 0;
    char soundCardInfo[200];
    int i = 0;
    int size = 0;
    char *kernel_snd_card = NULL;

    if ((fp = fopen("/proc/asound/cards","r")) == NULL) {
        LOGE("Cannot open /proc/asound/cards file to get sound card info");
        rc = 1;
        goto EXIT;
    }

    if ((fgets(soundCardInfo, sizeof(soundCardInfo), fp) != NULL)) {
        LOGD("Kernel sound card=%s\n", soundCardInfo);
        for (i = 0; i < CSD_ALSA_MAX_NUM_CARDS; i++) {
            kernel_snd_card = strstr(soundCardInfo,
                        csd_alsa_card_mapping_list[i].kernel_card_name);
            if (kernel_snd_card &&
                !strncmp(kernel_snd_card,
                         csd_alsa_card_mapping_list[i].kernel_card_name,
                         strlen(csd_alsa_card_mapping_list[i].kernel_card_name)))
            {
                size = sizeof(char *) * (strlen(csd_alsa_card_mapping_list[i].ucm_card_name)+1);
                *ucm_sound_card_name = (char *)malloc(size);
                if (*ucm_sound_card_name == NULL) {
                    rc = ENOMEM;
                    goto EXIT;
                }
                memset(*ucm_sound_card_name, 0, size);
                strlcpy(*ucm_sound_card_name,
                        csd_alsa_card_mapping_list[i].ucm_card_name,
                        size);
                break;
            }
        }
    } else {
        rc = 1;
    }

    if (*ucm_sound_card_name == NULL) {
        LOGE("No mapping b/w Kernel Sound card and UCM sound card");
        rc = 1;
        goto EXIT;
    }
EXIT:
    if (fp)
        fclose(fp);
    return rc;
}

static int csd_alsa_set_usecase(struct client_data *clnt_data, int session_id)
{
    int rc = 0;
    char *cur_ucm_verb = NULL;
    char *use_case_to_set = NULL;

    rc = snd_use_case_get(csd_common_data.ucm_mgr, "_verb",
                          (const char **)&cur_ucm_verb);
    if (rc != 0) {
        LOGE("Failed to get verb from UCM rc=%d\n", rc);
        goto EXIT;
    }

    if (cur_ucm_verb == NULL ||
        !strncmp(cur_ucm_verb, SND_USE_CASE_VERB_INACTIVE,
                 strlen(cur_ucm_verb))) {
            rc = get_verb_name_from_session_id(session_id, &use_case_to_set);
            if (rc != 0) {
                LOGE("Unalbe to get verb from session id=%d\n", session_id);
                goto EXIT;
            }
            rc = snd_use_case_set(csd_common_data.ucm_mgr, "_verb",
                                  use_case_to_set);
            if (rc != 0) {
                LOGE("Failed to set verb=%s\n", use_case_to_set);
                goto EXIT;
            }
    } else {
        rc = get_modifier_name_from_session_id(session_id, &use_case_to_set);
        if (rc != 0) {
            LOGE("Unalbe to get modifier name from session id=%d\n",
                 session_id);
            goto EXIT;
        }
        rc = snd_use_case_set(csd_common_data.ucm_mgr, "_enamod",
                              use_case_to_set);
        if (rc != 0) {
            LOGE("Failed to set modifier=%s\n", use_case_to_set);
            goto EXIT;
        }
     }

    strlcpy(&(clnt_data->session[session_id].alsa_handle.useCase),
            use_case_to_set,
            sizeof(clnt_data->session[session_id].alsa_handle.useCase));

EXIT:
    if (cur_ucm_verb) {
        free(cur_ucm_verb);
    }
    if (use_case_to_set) {
        free(use_case_to_set);
    }

    return rc;

}

static int csd_alsa_vc_set_device_config(void)
{
    int rc = 0;
    qmi_csd_vc_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;

    LOGD("csd_alsa_vc_set_device_config() IN\n");

EXIT:
    LOGD("csd_alsa_vc_set_device_config() rc=%d,\n",rc);
    memset(&resp_msg, 0, sizeof(qmi_csd_vc_cmd_resp_msg));
    resp_msg.u.qmi_csd_set_device_config_resp.qmi_csd_status_code_valid = false;
    resp_msg.u.qmi_csd_set_device_config_resp.handle_valid = true;
    resp_msg.u.qmi_csd_set_device_config_resp.handle =
                                         csd_common_data.command.command_handle;
    resp_msg.u.qmi_csd_set_device_config_resp.cmd_token_valid = true;
    resp_msg.u.qmi_csd_set_device_config_resp.cmd_token =
                                                  csd_common_data.command.token;
    resp_len = sizeof (resp_msg.u.qmi_csd_set_device_config_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);
    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vc_set_device_config() OUT resp_err=%d\n", resp_err);
    return rc;
}

static int csd_alsa_vc_cmd_enable(void)
{
    int rc = 0;
    qmi_csd_vc_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;

    LOGD("csd_alsa_vc_cmd_enable() IN\n");
    memset(&resp_msg, 0, sizeof(qmi_csd_vc_cmd_resp_msg));

    resp_msg.u.qmi_csd_enable_resp.qmi_csd_status_code_valid = false;
    resp_msg.u.qmi_csd_enable_resp.handle_valid = true;
    resp_msg.u.qmi_csd_enable_resp.handle =
                                        csd_common_data.command.command_handle;
    resp_msg.u.qmi_csd_enable_resp.cmd_token_valid = true;
    resp_msg.u.qmi_csd_enable_resp.cmd_token = csd_common_data.command.token;

    resp_len = sizeof (resp_msg.u.qmi_csd_enable_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);

    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vc_cmd_enable() OUT rc=%d\n",rc);
    return rc;
}

static int csd_alsa_vm_attach_context(void)
{
    int rc = 0;
    qmi_csd_vm_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;

    LOGD("csd_alsa_vm_attach_context() IN\n");
    memset(&resp_msg, 0, sizeof(qmi_csd_vm_cmd_resp_msg));

    resp_msg.u.qmi_csd_attach_context_resp.qmi_csd_status_code_valid = false;
    resp_msg.u.qmi_csd_attach_context_resp.handle_valid = true;
    resp_msg.u.qmi_csd_attach_context_resp.handle =
                                        csd_common_data.command.command_handle;
    resp_msg.u.qmi_csd_attach_context_resp.cmd_token_valid = true;
    resp_msg.u.qmi_csd_attach_context_resp.cmd_token =
                                                csd_common_data.command.token;
    resp_len = sizeof (resp_msg.u.qmi_csd_attach_context_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);
    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vm_attach_context() OUT\n");
    return rc;

}

static int csd_alsa_vm_set_widevoice(void)
{
    int rc = 0;
    qmi_csd_vm_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;
    uint32_t vm_handle = 0;
    struct voice_stream_manager *vm_data = NULL;

    LOGD("csd_alsa_vm_set_widevoice() IN\n");
    memset(&resp_msg, 0, sizeof(qmi_csd_vm_cmd_resp_msg));
    vm_handle = csd_common_data.command.command_handle;

    if (!is_valid_voice_manager_handle(csd_common_data.command.command_client_data,
                                       vm_handle)) {
        LOGE("Invalid VM handle=%08X\n", vm_handle);
        rc = 1;
        goto EXIT;
    }

    vm_data = (struct voice_stream_manager *)vm_handle;
    rc = alsa_set_mixer_ctl("Widevoice Enable", (void *) &vm_data->wide_voice);

    if (rc != 0) {
        LOGE("alsa_set_mixer_ctl set Widevoice Failed() rc:%d\n", rc);
        goto EXIT;
    }

EXIT:
    LOGD("csd_alsa_vm_set_widevoice() rc:%d\n", rc);
    if (rc != 0) {
        resp_msg.u.qmi_csd_set_widevoice_resp.resp.result =
                                                        QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_widevoice_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_widevoice_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_widevoice_resp.qmi_csd_status_code =
                                                           QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_widevoice_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_widevoice_resp.cmd_token_valid = false;
    } else {
        resp_msg.u.qmi_csd_set_widevoice_resp.qmi_csd_status_code_valid = false;
        resp_msg.u.qmi_csd_set_widevoice_resp.handle_valid = true;
        resp_msg.u.qmi_csd_set_widevoice_resp.handle =
                                        csd_common_data.command.command_handle;
        resp_msg.u.qmi_csd_set_widevoice_resp.cmd_token_valid = true;
        resp_msg.u.qmi_csd_set_widevoice_resp.cmd_token =
                                                csd_common_data.command.token;
    }

    resp_len = sizeof (resp_msg.u.qmi_csd_set_widevoice_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);

    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vm_set_widevoice() OUT\n");
    return rc;
}

static int csd_alsa_vm_set_tty_mode(void)
{
    int rc = 0;
    bool is_valid = false;
    qmi_csd_vm_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;
    uint32_t vm_handle = 0;
    struct voice_stream_manager *vm_data = NULL;

    LOGD("csd_alsa_vm_set_tty() IN\n");

    vm_handle = csd_common_data.command.command_handle;
    if (!is_valid_voice_manager_handle(csd_common_data.command.command_client_data,
                                       vm_handle)) {
        LOGE("Invalid VM handle=%08X\n", vm_handle);
        rc = 1;
        goto EXIT;
    }

    vm_data = (struct voice_stream_manager *)vm_handle;

    is_valid = is_ttymode_valid_for_devices(csd_common_data.device_data.devices[0].dev_id,
                                            csd_common_data.device_data.devices[1].dev_id,
                                            vm_data->tty_mode);
    if (!is_valid) {
        LOGE("Device %d, %d combination with tty mode %d is invalid\n",
             csd_common_data.device_data.devices[0].dev_id,
             csd_common_data.device_data.devices[1].dev_id, vm_data->tty_mode);
        rc = 1;
        goto EXIT;
    }

    LOGD("csd_alsa.c TTY Mode = %d\n", vm_data->tty_mode);

    if (vm_data->tty_mode <= FULL)
        rc = alsa_set_mixer_ctl("TTY Mode", (void *) tty_mode_ctl[vm_data->tty_mode]);
    else
        rc = 1;

    if (rc != 0) {
        LOGE("alsa_set_mixer_ctl TTY Mode Failed rc:%d\n", rc);
        goto EXIT;
    }

EXIT:
    memset(&resp_msg, 0, sizeof(qmi_csd_vm_cmd_resp_msg));
    LOGD("csd_alsa_vm_set_tty() rc =%d\n", rc);
    if (rc != 0) {
        resp_msg.u.qmi_csd_set_tty_mode_resp.resp.result =
                                                         QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_tty_mode_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_tty_mode_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_tty_mode_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_tty_mode_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_tty_mode_resp.cmd_token_valid = false;
    } else {
        resp_msg.u.qmi_csd_set_tty_mode_resp.qmi_csd_status_code_valid = false;
        resp_msg.u.qmi_csd_set_tty_mode_resp.handle_valid = true;
        resp_msg.u.qmi_csd_set_tty_mode_resp.handle =
                                        csd_common_data.command.command_handle;
        resp_msg.u.qmi_csd_set_tty_mode_resp.cmd_token_valid = true;
        resp_msg.u.qmi_csd_set_tty_mode_resp.cmd_token =
                                                 csd_common_data.command.token;
    }

    resp_len = sizeof (resp_msg.u.qmi_csd_attach_context_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);

    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vm_set_tty() OUT\n");
    return rc;
}

static int csd_alsa_set_slowtalk(uint32_t enable)
{
    int rc = 0, i = 0;
    char** setValues;
    const int num_params = 2;
    LOGD("%s: enable %d", __func__, enable);

    setValues = (char**)malloc((num_params)*sizeof(char*));
    if (!setValues) {
        LOGE("%s: allocation error", __func__);
        return;
    }

    for (i = 0; i < num_params; i++) {
        setValues[i] = (char*)malloc(4*sizeof(char));
        if (!setValues[i]) {
            rc = 1;
            LOGE("%s: allocation error", __func__);
            break;
        }
    }

    if (rc == 0) {
        snprintf(setValues[0], (4*sizeof(char)), "%d", enable);
        snprintf(setValues[1], (4*sizeof(char)), "%d", ALL_SESSION_VSID);
        rc = alsa_set_mixer_ctl_ext("Slowtalk Enable", num_params, setValues);
        LOGV("%s ret=%d enable=%d session_id=%#x \n"
             ,__func__, rc, enable, ALL_SESSION_VSID);
    }

    for(i = 0; i < num_params; i++)
        if (setValues[i])
            free(setValues[i]);
    if (setValues)
        free(setValues);

    return rc;
}

static int csd_alsa_vs_set_ui_property(void)
{
    int rc = 0;
    qmi_csd_vs_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;
    uint32_t vs_handle = 0;
    struct voice_stream *vs_data = NULL;

    LOGD("csd_alsa_vs_set_ui_property() IN\n");

    vs_handle = csd_common_data.command.command_handle;
    if (!is_valid_voice_stream_handle(csd_common_data.command.command_client_data,
                                      vs_handle)) {
        LOGE("Invalid VS Handle=%0x%08X\n", vs_handle);
        rc = 1;
        goto EXIT;
    }

    vs_data = (struct voice_stream *)vs_handle;

    if (vs_data->ui_prop_mask & UI_PROP_FENS) {
        rc = alsa_set_mixer_ctl("FENS Enable", (void *) &vs_data->fens);
        if (rc != 0) {
            LOGE("csd_alsa_vm_set_FENS() Failed rc:%d\n", rc);
        }
    } else {
        rc = csd_alsa_set_slowtalk(vs_data->slow_talk);
        if (rc != 0)
            LOGE("csd_alsa_vm_set_Slowtalk() Failed rc:%d\n", rc);
    }

    vs_data->ui_prop_mask = 0;
EXIT:
    memset(&resp_msg, 0, sizeof(qmi_csd_vs_cmd_resp_msg));

    if (rc != 0) {
        resp_msg.u.qmi_csd_set_ui_prop_resp.resp.result =
                                                        QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_ui_prop_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_ui_prop_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_ui_prop_resp.qmi_csd_status_code =
                                                           QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_ui_prop_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_ui_prop_resp.cmd_token_valid = false;
    } else {
        resp_msg.u.qmi_csd_set_ui_prop_resp.qmi_csd_status_code_valid = false;
        resp_msg.u.qmi_csd_set_ui_prop_resp.handle_valid = true;
        resp_msg.u.qmi_csd_set_ui_prop_resp.handle =
                                        csd_common_data.command.command_handle;
        resp_msg.u.qmi_csd_set_ui_prop_resp.cmd_token_valid = true;
        resp_msg.u.qmi_csd_set_ui_prop_resp.cmd_token =
                                                  csd_common_data.command.token;
    }

    resp_len = sizeof (resp_msg.u.qmi_csd_set_ui_prop_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);

    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vs_set_ui_property() OUT\n");
    return rc;
}

static int deviceName(unsigned flags, char *use_case, char **devName)
{
    int ret = 0;
    char ident[70] = {0};

    if (flags & PCM_IN) {
        strlcpy(ident, "CapturePCM/", sizeof(ident));
    } else {
        strlcpy(ident, "PlaybackPCM/", sizeof(ident));
    }

    strlcat(ident, use_case, sizeof(ident));
    ret = snd_use_case_get(csd_common_data.ucm_mgr, ident,
                           (const char **)devName);

    if (*devName != NULL)
       LOGD("Device value returned is %s\n", (*devName));

    return ret;
}

static int alsa_set_hardware_params(struct alsa_handle *alsa_handle,
                                    struct pcm *pcm_handle,
                                    uint32_t bitspersample,
                                    uint32_t sample_rate)
{
    struct snd_pcm_hw_params *params;
    unsigned long bufferSize, reqBuffSize;
    unsigned int periodTime, bufferTime;
    int channels;
    snd_pcm_format_t format = SNDRV_PCM_FORMAT_S16_LE;
    int status = 0, rc = 0;

    LOGD("alsa_set_hardware_params() IN BIPS=%d, sample_rate=%d\n",
         bitspersample, sample_rate);
    channels = alsa_handle->channels;
    params = (struct snd_pcm_hw_params *)
                                   calloc(1, sizeof(struct snd_pcm_hw_params));
    if (!params) {
        LOGE("Failed to allocate ALSA hardware parameters!\n");
        rc = -ENOMEM;
        goto EXIT;
    }

    reqBuffSize = alsa_handle->bufferSize;
    param_init(params);
    param_set_mask(params, SNDRV_PCM_HW_PARAM_ACCESS,
                   SNDRV_PCM_ACCESS_RW_INTERLEAVED);
    format = SNDRV_PCM_FORMAT_S16_LE;
    param_set_mask(params, SNDRV_PCM_HW_PARAM_FORMAT,
                   format);
    param_set_mask(params, SNDRV_PCM_HW_PARAM_SUBFORMAT,
                   SNDRV_PCM_SUBFORMAT_STD);
    param_set_min(params, SNDRV_PCM_HW_PARAM_PERIOD_BYTES, reqBuffSize);
    param_set_int(params, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, bitspersample);
    param_set_int(params, SNDRV_PCM_HW_PARAM_FRAME_BITS,
                   channels * 16);
    param_set_int(params, SNDRV_PCM_HW_PARAM_CHANNELS,
                  channels);
    param_set_int(params, SNDRV_PCM_HW_PARAM_RATE, sample_rate);
    param_set_hw_refine(pcm_handle, params);

    if (param_set_hw_params(pcm_handle, params)) {
        LOGE("cannot set hw params\n");
        rc = -EIO;
        goto EXIT;
    }
    param_dump(params);
    pcm_handle->buffer_size = pcm_buffer_size(params);
    pcm_handle->period_size = pcm_period_size(params);
    pcm_handle->period_cnt = pcm_handle->buffer_size/pcm_handle->period_size;
    pcm_handle->rate = sample_rate;
    pcm_handle->channels = alsa_handle->channels;
    alsa_handle->periodSize = pcm_handle->period_size;
    LOGD("setHardwareParams: buffer_size %d, period_size %d, period_cnt %d\n",
         pcm_handle->buffer_size, pcm_handle->period_size,
         pcm_handle->period_cnt);
EXIT:
    LOGD("setHardwareParams OUT rc:%d\n",rc);
    return rc;
}

static int alsa_set_software_params(struct alsa_handle *alsa_handle,
                                    struct pcm *pcm_handle)
{
    struct snd_pcm_sw_params *params;
    unsigned long periodSize;
    int channels = alsa_handle->channels, rc = 0;

    if (pcm_handle == NULL) {
        LOGE("param pcm_handle passed to setSoftwareParams() is NULL\n");
        rc = -EINVAL;
        goto EXIT;
    }

    periodSize = pcm_handle->period_size;
    params = (struct snd_pcm_sw_params*)
             calloc(1, sizeof(struct snd_pcm_sw_params));
    if (!params) {
        LOGE("Failed to allocate ALSA software parameters!\n");
        rc = -ENOMEM;
        goto EXIT;
    }

    params->tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
    params->period_step = 1;
    params->avail_min = periodSize/2;
    params->start_threshold = channels * (periodSize/4);
    params->stop_threshold = INT_MAX;
    params->silence_threshold = 0;
    params->silence_size = 0;
    if (param_set_sw_params(pcm_handle, params)) {
        LOGE("cannot set sw params\n");
        rc = -EIO;
        goto EXIT;
    }
EXIT:
    return rc;
}

static int alsa_get_prepared_pcm_handle(unsigned flags, char *devName,
                                        struct alsa_handle *alsa_handle,
                                        struct pcm **pcm_h,
                                        uint32_t bitspersample,
                                        uint32_t sample_rate)
{
    struct pcm* pcm_handle = NULL;
    int rc = 0;

    if (devName == NULL) {
        rc = -EINVAL;
        goto EXIT;
    }

    pcm_handle = pcm_open(flags, (char *)devName);
    if (!pcm_handle || pcm_handle->fd == -1) {
        LOGE("start call: could not open PCM device\n");
        rc = -EBADFD;
        goto EXIT;
    }
    pcm_handle->flags = flags;
    alsa_handle->channels = VOICE_CHANNELS_MONO;

    rc = alsa_set_hardware_params(alsa_handle, pcm_handle, bitspersample,
                                  sample_rate);
    if (rc != 0) {
        LOGE("\ns_start_voice_call: setHardwareParams failed\n");
        goto EXIT;
    }
    rc = alsa_set_software_params(alsa_handle, pcm_handle);
    if (rc != 0) {
        LOGE("\ns_start_voice_call: setSoftwareParams failed\n");
        goto EXIT;
    }

    rc = pcm_prepare(pcm_handle);
    if (rc != 0) {
        LOGE("\ns_start_voice_call: pcm_prepare failed\n");
        goto EXIT;
    }

    rc = ioctl(pcm_handle->fd, SNDRV_PCM_IOCTL_START);
    if (rc) {
        LOGE("s_start_voice_call:SNDRV_PCM_IOCTL_START failed\n");
    }

    *pcm_h = pcm_handle;
EXIT:
    LOGD("\n alsa_pcm_open() OUT param pcm_handle=0x%08X\n",*pcm_h);
    return rc;
}

static int alsa_start_voice_session(struct session_data_t *session)
{
    int rc = 0;
    char *devName;
    unsigned flags = 0;
    struct alsa_handle *alsa_handle = &session->alsa_handle;
    struct pcm *pcm_handle = NULL;
    unsigned int bitspersample;
    unsigned int sample_rate;
    uint32_t rx_dev = session->voice_context_data.rx_dev_id;
    uint32_t tx_dev = session->voice_context_data.tx_dev_id;

    LOGD("alsa_start_voice_session()\n");
    /* Open PCM play device */
    flags = ALSA_PCM_OUT_MONO;
    if (deviceName(flags, alsa_handle->useCase, &devName) < 0) {
        LOGE("Failed to get pcm device node\n");
        rc = -EINVAL;
        goto EXIT;
    }

    rc = get_bits_per_sample_for_deviceid(rx_dev, &bitspersample);
    if (rc != 0) {
        LOGE("Could not get bit rate for rxdev=%d error=%d\n", rx_dev, rc);
        goto EXIT;
    }

    rc = get_sample_rate_for_deviceid(rx_dev, &sample_rate);
    if (rc != 0) {
        LOGE("Could not get sample rate for rxdev=%d error=%d\n", rx_dev, rc);
        goto EXIT;
    }

    rc = alsa_get_prepared_pcm_handle(flags, devName, alsa_handle, &pcm_handle,
                                      bitspersample, sample_rate);
    if (rc != 0) {
        LOGE("csd_alsa_pcm_ctrl PCM OUT rc= %d\n", rc);
        goto EXIT;
    }

    alsa_handle->rxHandle = pcm_handle;
    LOGD("RX pcm open done rxHandle=%08X\n", alsa_handle->rxHandle);
    pcm_handle = NULL;
    /* Open PCM capture device */
    flags = ALSA_PCM_IN_MONO;
    if (deviceName(flags, alsa_handle->useCase, &devName) < 0) {
        LOGE("Failed to get pcm device node\n");
        rc = -EINVAL;
        goto EXIT;
    }

    rc = get_bits_per_sample_for_deviceid(tx_dev, &bitspersample);
    if (rc != 0) {
        LOGE("Could not get bit rate for tx_dev=%d error=%d\n", tx_dev, rc);
        goto EXIT;
    }

    rc = get_sample_rate_for_deviceid(tx_dev, &sample_rate);
    if (rc != 0) {
        LOGE("Could not get sample rate for tx_dev=%d error=%d\n", tx_dev, rc);
        goto EXIT;
    }

    rc = alsa_get_prepared_pcm_handle(flags, devName, alsa_handle, &pcm_handle,
                                      bitspersample, sample_rate);
    if (rc != 0) {
        LOGE("csd_alsa_pcm_ctrl PCM OUT rc= %d\n",rc);
        goto EXIT;
    }

    alsa_handle->txHandle = pcm_handle;
    LOGD("TX pcm open done txHandle=%08X\n", alsa_handle->txHandle);
EXIT:
    if (devName) {
        free(devName);
    }
    if (rc != 0) {
        if (alsa_handle->txHandle) {
            pcm_close(alsa_handle->txHandle);
        }
        if (alsa_handle->txHandle) {
            pcm_close(alsa_handle->txHandle);
        }
    }
    return rc;
}

static int alsa_pause_session(struct alsa_handle *alsa_handle, int pause_state)
{
    int rc = 0;

    rc = ioctl((int)alsa_handle->txHandle->fd,
               SNDRV_PCM_IOCTL_PAUSE,
               pause_state);
    if (rc != 0) {
        LOGE("Call Hold/Resume call failed %d\n", pause_state);
    }
    return rc;
}

static int csd_alsa_vm_start_voice(void)
{
    int rc = 0;
    qmi_csd_vm_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;
    uint32_t vm_handle = 0;
    struct client_data *clnt_data = NULL;
    int session_id = 0;

    LOGD("csd_alsa_vm_start_voice() IN\n");
    memset(&resp_msg, 0, sizeof(qmi_csd_vm_cmd_resp_msg));

    vm_handle = csd_common_data.command.command_handle;
    clnt_data = csd_common_data.command.command_client_data;
    rc = get_session_id_from_vm_handle(vm_handle, clnt_data, &session_id);
    if (rc != 0)
       goto EXIT;

    if (clnt_data->session[session_id].alsa_handle.txHandle &&
        clnt_data->session[session_id].alsa_handle.rxHandle) {
        rc = alsa_pause_session(&clnt_data->session[session_id].alsa_handle,
                                CALL_RESUME);
    } else {
         rc = alsa_start_voice_session(&clnt_data->session[session_id]);
    }

EXIT:
    LOGD("csd_alsa_vm_start_voice() rc=%d\n", rc);
    if (rc != 0) {
        resp_msg.u.qmi_csd_start_voice_resp.resp.result =
                                                        QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_start_voice_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_start_voice_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_start_voice_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_start_voice_resp.handle_valid = false;
        resp_msg.u.qmi_csd_start_voice_resp.cmd_token_valid = false;
    } else {
        resp_msg.u.qmi_csd_start_voice_resp.qmi_csd_status_code_valid = false;
        resp_msg.u.qmi_csd_start_voice_resp.handle_valid = true;
        resp_msg.u.qmi_csd_start_voice_resp.handle =
                                        csd_common_data.command.command_handle;
        resp_msg.u.qmi_csd_start_voice_resp.cmd_token_valid = true;
        resp_msg.u.qmi_csd_start_voice_resp.cmd_token =
                                                csd_common_data.command.token;
    }

    resp_len = sizeof (resp_msg.u.qmi_csd_start_voice_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                  csd_common_data.command.msg_id,
                                  &resp_msg, resp_len);
    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;
    LOGD("csd_alsa_vm_start_voice() OUT\n");
    return rc;
}

static int csd_alsa_vm_standby_voice(void)
{
    int rc = 0;
    qmi_csd_vm_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;
    uint32_t vm_handle = 0;
    struct client_data *clnt_data = NULL;
    int session_id = 0;
    qmi_csd_status_v01 qmi_rc = QMI_CSD_EFAILED_V01;

    LOGD("csd_alsa_vm_standby_voice() IN\n");
    memset(&resp_msg, 0, sizeof(qmi_csd_vm_cmd_resp_msg));

    vm_handle = csd_common_data.command.command_handle;
    clnt_data = csd_common_data.command.command_client_data;
    rc = get_session_id_from_vm_handle(vm_handle, clnt_data, &session_id);

    if (rc != 0) {
        LOGE("Failed to get session id from VM Handle=0x%08X", vm_handle);
        goto EXIT;
    }

    rc = alsa_pause_session(&clnt_data->session[session_id].alsa_handle,
                            CALL_PAUSE);

EXIT:
    LOGD("csd_alsa_vm_standby_voice() rc=%d\n", rc);
    if (rc != 0) {
        resp_msg.u.qmi_csd_standby_voice_resp.resp.result =
                                                        QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_standby_voice_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_standby_voice_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_standby_voice_resp.qmi_csd_status_code = qmi_rc;
        resp_msg.u.qmi_csd_standby_voice_resp.handle_valid = false;
        resp_msg.u.qmi_csd_standby_voice_resp.cmd_token_valid = false;
    } else {
        resp_msg.u.qmi_csd_standby_voice_resp.qmi_csd_status_code_valid = false;
        resp_msg.u.qmi_csd_standby_voice_resp.handle_valid = true;
        resp_msg.u.qmi_csd_standby_voice_resp.handle =
                                         csd_common_data.command.command_handle;
        resp_msg.u.qmi_csd_standby_voice_resp.cmd_token_valid = true;
        resp_msg.u.qmi_csd_standby_voice_resp.cmd_token =
                                                  csd_common_data.command.token;
    }

    resp_len = sizeof (resp_msg.u.qmi_csd_standby_voice_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);

    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vm_standby_voice() OUT\n");
    return rc;
}

static int alsa_stop_voice(struct session_data_t *session)
{
    int rc = 0;
    struct alsa_handle *alsa_handle = &session->alsa_handle;

    LOGD("alsa_stop_voice() IN\n");

    if (alsa_handle->txHandle) {
        rc = pcm_close(alsa_handle->txHandle);
        LOGD("\nalsa_stop_voice() closing tx handle rc=%d\n", rc);
        alsa_handle->txHandle = NULL;
    }

    if (alsa_handle->rxHandle) {
        rc = pcm_close(alsa_handle->rxHandle);
        LOGD("\nalsa_stop_voice() closing rx handle rc=%d\n", rc);
        alsa_handle->rxHandle = NULL;
    }

EXIT:
    LOGD("alsa_stop_voice() OUT=%d\n", rc);
    return rc;
}

static int csd_alsa_vm_stop_voice(void)
{
    int rc = 0;
    qmi_csd_vm_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;
    uint32_t vm_handle = 0;
    struct client_data *clnt_data = NULL;
    int session_id = 0;

    LOGD("csd_alsa_vm_stop_voice() IN\n");
    vm_handle = csd_common_data.command.command_handle;
    clnt_data = csd_common_data.command.command_client_data;
    rc = get_session_id_from_vm_handle(vm_handle, clnt_data, &session_id);
    if (rc != 0) {
        LOGE("Failed to get session id from VM Handle=0x%08X\n", vm_handle);
        goto EXIT;
    }

    rc = alsa_stop_voice(&clnt_data->session[session_id]);
    if (rc != 0) {
        LOGE("Failed to stop voice rc=%d\n", rc);
        goto EXIT;
    }

EXIT:
    LOGD("csd_alsa_vm_stop_voice() rc=%d\n", rc);
    memset(&resp_msg, 0, sizeof(qmi_csd_vm_cmd_resp_msg));
    if (rc != 0) {
        resp_msg.u.qmi_csd_stop_voice_resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_stop_voice_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_stop_voice_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_stop_voice_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_stop_voice_resp.handle_valid = false;
        resp_msg.u.qmi_csd_stop_voice_resp.cmd_token_valid = false;
    } else {
        resp_msg.u.qmi_csd_stop_voice_resp.qmi_csd_status_code_valid = false;
        resp_msg.u.qmi_csd_stop_voice_resp.handle_valid = true;
        resp_msg.u.qmi_csd_stop_voice_resp.handle =
                                         csd_common_data.command.command_handle;
        resp_msg.u.qmi_csd_stop_voice_resp.cmd_token_valid = true;
        resp_msg.u.qmi_csd_stop_voice_resp.cmd_token =
                                                 csd_common_data.command.token;
    }

    resp_len = sizeof (resp_msg.u.qmi_csd_stop_voice_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);
    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vm_stop_voice() OUT\n");
    return rc;
}

static int alsa_set_rx_volume(uint32_t volume)
{
    struct mixer_ctl *ctl = NULL;
    int rc = 0;
    unsigned percent = 0;

    ctl = mixer_get_control(csd_common_data.alsa_mixer, "Voice Rx Volume", 0);
    if (ctl == NULL) {
        LOGE("Could not get the mixer control for Voice Rx Volume\n");
        rc = 1;
        goto EXIT;
    }

    percent = index_to_percent(volume, ctl->info->value.integer.min,
                               ctl->info->value.integer.max);
    LOGD("percent=%d, volume_index=%d\n", percent, volume);
    rc = mixer_ctl_set(ctl, percent);

    if (rc != 0) {
        LOGE("Failed to set rx volume: %d\n", rc);
        goto EXIT;
    }
EXIT:
    return rc;
}

static int alsa_set_rx_soft_volume(uint32_t volume)
{
    struct mixer_ctl *ctl = NULL;
    int rc = 0, i = 0;
    char** setValues;
    const int num_params = 3;
    LOGD("%s: volume %d", __func__, volume);

    setValues = (char**)malloc((num_params)*sizeof(char*));
    if (!setValues) {
        LOGE("%s: alsa_set_rx_soft_volume: allocation error", __func__);
        rc = 1;
        goto EXIT;
    }

    for (i = 0; i < num_params; i++) {
        setValues[i] = (char*)malloc(4*sizeof(char));
        if (!setValues[i]) {
            rc = 1;
            LOGE("%s: allocation error", __func__);
            break;
        }
    }

    if (rc == 0) {
        snprintf(setValues[0], (4*sizeof(char)), "%d", volume);
        snprintf(setValues[1], (4*sizeof(char)), "%d", ALL_SESSION_VSID);
        snprintf(setValues[2], (4*sizeof(char)), "%d", DEFAULT_VOLUME_RAMP_DURATION_MS);

        rc = alsa_set_mixer_ctl_ext("Voice Rx Gain", num_params, setValues);
        LOGV("%s ret=%d volume=%d session_id=%#x ramp_dur=%d\n"
             ,__func__, rc, volume, ALL_SESSION_VSID, DEFAULT_VOLUME_RAMP_DURATION_MS);
    }

    for(i = 0; i < num_params; i++)
        if (setValues[i])
            free(setValues[i]);
    if (setValues)
        free(setValues);

    if (rc != 0) {
        LOGE("Failed to set rx volume: %d\n", rc);
        goto EXIT;
    }
EXIT:
    return rc;
}

static int alsa_enable_hdvoice(uint32_t enable)
{
    struct mixer_ctl *ctl = NULL;
    int rc = 0, i = 0;
    char** setValues;
    const int num_params = 2;

    setValues = (char**)malloc((num_params)*sizeof(char*));
    if (!setValues) {
        LOGE("%s: allocation error\n", __func__);

        rc = 1;
        goto EXIT;
    }

    for (i = 0; i < num_params; i++) {
        setValues[i] = (char*)malloc(4*sizeof(char));
        if (!setValues[i]) {
            LOGE("%s: allocation error\n", __func__);

            rc = 1;
            break;
        }
    }

    if (rc == 0) {
        snprintf(setValues[0], (4*sizeof(char)), "%d", enable);
        snprintf(setValues[1], (4*sizeof(char)), "%d", ALL_SESSION_VSID);

        rc = alsa_set_mixer_ctl_ext("HD Voice Enable", num_params, setValues);
        LOGV("%s: rc=%d enable=%d session_id=%#x\n", __func__, rc, enable,
             ALL_SESSION_VSID);
    }

    for(i = 0; i < num_params; i++) {
        if (setValues[i])
            free(setValues[i]);
    }
    if (setValues)
        free(setValues);

    if (rc != 0) {
        LOGE("%s: Failed to enable HD voice: %d\n", __func__, rc);
    }

EXIT:
    return rc;
}

static int csd_alsa_vc_set_rx_volume_index(void)
{
    int rc = 0;
    qmi_csd_vc_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;
    struct voice_context_type *vc_handle  = NULL;

    LOGD("csd_alsa_vc_set_rx_volume_index() IN\n");
    vc_handle = (struct voice_context_type *)csd_common_data.command.command_handle;
    rc = alsa_set_rx_soft_volume(vc_handle->volume);

    if (rc != 0) {
        LOGE("Failed to set rx_volume rc=%d\n", rc);
    }

EXIT:
    LOGD("csd_alsa_vc_set_rx_volume_index() rc=%d\n", rc);
    memset(&resp_msg, 0, sizeof(qmi_csd_vc_cmd_resp_msg));
    if (rc != 0) {
        resp_msg.u.qmi_csd_set_rx_volume_resp.resp.result =
                                                        QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_rx_volume_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_rx_volume_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_rx_volume_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_rx_volume_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_rx_volume_resp.cmd_token_valid = false;
    } else {
        resp_msg.u.qmi_csd_set_rx_volume_resp.qmi_csd_status_code_valid = false;
        resp_msg.u.qmi_csd_set_rx_volume_resp.handle_valid = true;
        resp_msg.u.qmi_csd_set_rx_volume_resp.handle =
                                        csd_common_data.command.command_handle;
        resp_msg.u.qmi_csd_set_rx_volume_resp.cmd_token_valid = true;
        resp_msg.u.qmi_csd_set_rx_volume_resp.cmd_token =
                                                  csd_common_data.command.token;
    }

    resp_len = sizeof (resp_msg.u.qmi_csd_set_rx_volume_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);

    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vc_set_rx_volume_index() OUT\n");
    return rc;
}

static int csd_alsa_vm_set_hdvoice_mode(void)
{
    int rc = 0;
    qmi_csd_vm_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;
    uint32_t vm_handle = 0;
    struct voice_stream_manager *vm_data = NULL;

    LOGD("%s: IN\n", __func__);

    memset(&resp_msg, 0, sizeof(qmi_csd_vm_cmd_resp_msg));
    vm_handle = csd_common_data.command.command_handle;

    if (!is_valid_voice_manager_handle(csd_common_data.command.command_client_data,
                                       vm_handle)) {
        LOGE("Invalid VM handle=%08X\n", vm_handle);

        rc = 1;
        goto EXIT;
    }

    vm_data = (struct voice_stream_manager *)vm_handle;
    rc = alsa_enable_hdvoice(vm_data->hd_voice);

    if (rc != 0) {
        LOGE("%s: set hd voice Failed() rc:%d\n", __func__, rc);
    }

EXIT:
    LOGD("%s: rc:%d\n", __func__, rc);

    if (rc != 0) {
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.resp.result =
                                                        QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.qmi_csd_status_code =
                                                           QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.cmd_token_valid = false;
    } else {
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.qmi_csd_status_code_valid = false;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.handle_valid = true;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.handle =
                                        csd_common_data.command.command_handle;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.cmd_token_valid = true;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.cmd_token =
                                                csd_common_data.command.token;
    }

    resp_len = sizeof (resp_msg.u.qmi_csd_set_hdvoice_mode_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);

    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("%s: OUT\n", __func__);

    return rc;
}

static int csd_alsa_set_tx_soft_mute(int state)
{
    int rc = 0, i = 0;
    char** setValues;
    const int num_params = 3;
    LOGD("%s: state %d", __func__, state);

    setValues = (char**)malloc((num_params)*sizeof(char*));
    if (!setValues) {
        LOGE("%s : allocation error", __func__);
        return;
    }

    for (i = 0; i < num_params; i++) {
        setValues[i] = (char*)malloc(4*sizeof(char));
        if (!setValues[i]) {
            rc = 1;
            LOGE("%s: allocation error", __func__);
            break;
        }
    }

    if (rc == 0) {
        snprintf(setValues[0], (4*sizeof(char)), "%d", state);
        snprintf(setValues[1], (4*sizeof(char)), "%d", ALL_SESSION_VSID);
        snprintf(setValues[2], (4*sizeof(char)), "%d", DEFAULT_MUTE_RAMP_DURATION);
        rc = alsa_set_mixer_ctl_ext("Voice Tx Mute", num_params, setValues);
        LOGV("%s ret=%d state=%d session_id=%#x ramp_dur=%d\n"
             ,__func__, rc, state, ALL_SESSION_VSID, DEFAULT_MUTE_RAMP_DURATION);
    }

    for(i = 0; i < num_params; i++)
        if (setValues[i])
            free(setValues[i]);
    if (setValues)
        free(setValues);

    return rc;
}

static int csd_alsa_vs_set_mute(void)
{
    int rc = 0;
    qmi_csd_vs_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;
    struct voice_stream *vs_handle = NULL;

    LOGD("csd_alsa_vs_set_mute() IN\n");

    if (!is_valid_voice_stream_handle(csd_common_data.command.command_client_data,
                                      csd_common_data.command.command_handle)) {
        LOGE("Invalid VS Handle=0x%08X\n",
             csd_common_data.command.command_handle);
        rc = 1;
        goto EXIT;
    }

    vs_handle = (struct voice_stream *)csd_common_data.command.command_handle;
    rc = csd_alsa_set_tx_soft_mute(vs_handle->mute.mute_flag);

    if (rc != 0)
        LOGE("Failed to set mixer ctl TX Mute rc=%dX\n", rc);


EXIT:
    LOGD("csd_alsa_vs_set_mute() rc=%d\n", rc);
    memset(&resp_msg, 0, sizeof(qmi_csd_vs_cmd_resp_msg));

    if (rc != 0) {
        resp_msg.u.qmi_csd_set_mute_resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_mute_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_mute_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_mute_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_mute_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_mute_resp.cmd_token_valid = false;
    } else {
        resp_msg.u.qmi_csd_set_mute_resp.qmi_csd_status_code_valid = false;
        resp_msg.u.qmi_csd_set_mute_resp.handle_valid = true;
        resp_msg.u.qmi_csd_set_mute_resp.handle =
                                         csd_common_data.command.command_handle;
        resp_msg.u.qmi_csd_set_mute_resp.cmd_token_valid = true;
        resp_msg.u.qmi_csd_set_mute_resp.cmd_token =
                                                  csd_common_data.command.token;
    }

    resp_len = sizeof (resp_msg.u.qmi_csd_set_mute_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);

    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vs_set_mute() OUT\n");
    return rc;
}

static int csd_alsa_vm_detach_context(void)
{
    int rc = 0;
    qmi_csd_vm_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;

    LOGD("csd_alsa_vm_detach_context() IN\n");
    memset(&resp_msg, 0, sizeof(qmi_csd_vm_cmd_resp_msg));

    resp_msg.u.qmi_csd_detach_context_resp.qmi_csd_status_code_valid = false;
    resp_msg.u.qmi_csd_detach_context_resp.handle_valid = true;
    resp_msg.u.qmi_csd_detach_context_resp.handle =
                                        csd_common_data.command.command_handle;
    resp_msg.u.qmi_csd_detach_context_resp.cmd_token_valid = true;
    resp_msg.u.qmi_csd_detach_context_resp.cmd_token =
                                                 csd_common_data.command.token;

    resp_len = sizeof (resp_msg.u.qmi_csd_detach_context_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);

    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vm_detach_context() OUT\n");
    return rc;
}

static int csd_alsa_vc_cmd_disable(void)
{
    int rc = 0;
    qmi_csd_vc_cmd_resp_msg resp_msg;
    uint32_t resp_len;
    qmi_csi_error resp_err;

    LOGD("csd_alsa_vc_cmd_disable() IN\n");
    memset(&resp_msg, 0, sizeof(qmi_csd_vc_cmd_resp_msg));

    resp_msg.u.qmi_csd_disable_resp.qmi_csd_status_code_valid = false;
    resp_msg.u.qmi_csd_disable_resp.handle_valid = true;
    resp_msg.u.qmi_csd_disable_resp.handle =
                                        csd_common_data.command.command_handle;
    resp_msg.u.qmi_csd_disable_resp.cmd_token_valid = true;
    resp_msg.u.qmi_csd_disable_resp.cmd_token = csd_common_data.command.token;

    resp_len = sizeof (resp_msg.u.qmi_csd_disable_resp);
    resp_err = qmi_csi_send_resp(csd_common_data.command.req_handle,
                                 csd_common_data.command.msg_id,
                                 &resp_msg, resp_len);

    if (resp_err != QMI_CSI_NO_ERR)
        rc = QMI_CSI_CB_INTERNAL_ERR;

    LOGD("csd_alsa_vc_cmd_disable() OUT\n");
    return rc;
}

static int process_csd_to_alsa_command(void)
{
    int rc = 0;

    switch (csd_common_data.command.msg_id) {
    case QMI_CSD_IOCTL_VC_CMD_SET_DEVICE_CONFIG_REQ_V01:
        csd_alsa_vc_set_device_config();
        break;
    case QMI_CSD_IOCTL_VC_CMD_ENABLE_REQ_V01:
        csd_alsa_vc_cmd_enable();
        break;
    case QMI_CSD_IOCTL_VM_CMD_ATTACH_CONTEXT_REQ_V01:
        csd_alsa_vm_attach_context();
        break;
    case QMI_CSD_IOCTL_VM_CMD_SET_WIDEVOICE_REQ_V01:
        csd_alsa_vm_set_widevoice();
        break;
    case QMI_CSD_IOCTL_VM_CMD_SET_TTY_MODE_REQ_V01:
        csd_alsa_vm_set_tty_mode();
        break;
    case QMI_CSD_IOCTL_VS_CMD_SET_UI_PROPERTY_REQ_V01:
        csd_alsa_vs_set_ui_property();
        break;
    case QMI_CSD_IOCTL_VM_CMD_START_VOICE_REQ_V01:
        csd_alsa_vm_start_voice();
        break;
    case QMI_CSD_IOCTL_VM_CMD_STANDBY_VOICE_REQ_V01:
        csd_alsa_vm_standby_voice();
        break;
    case QMI_CSD_IOCTL_VM_CMD_STOP_VOICE_REQ_V01:
        csd_alsa_vm_stop_voice();
        break;
    case QMI_CSD_IOCTL_VC_CMD_SET_RX_VOLUME_INDEX_REQ_V01:
         csd_alsa_vc_set_rx_volume_index();
        break;
    case QMI_CSD_IOCTL_VS_CMD_SET_MUTE_REQ_V01:
        csd_alsa_vs_set_mute();
        break;
    case QMI_CSD_IOCTL_VM_CMD_DETACH_CONTEXT_REQ_V01:
        csd_alsa_vm_detach_context();
        break;
    case QMI_CSD_IOCTL_VC_CMD_DISABLE_REQ_V01:
        csd_alsa_vc_cmd_disable();
        break;
    case QMI_CSD_IOCTL_VM_CMD_SET_HDVOICE_MODE_REQ_V01:
        csd_alsa_vm_set_hdvoice_mode();
        break;
    default:
        LOGD("process_csd_to_alsa_command() Invalid command:0x%08X\n",
              csd_common_data.command.msg_id);
        break;
    }

    return rc;
}

void *thread_main(void *data)
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 1;

    while (csd_common_data.thread_state != STATE_EXIT_THREAD) {
        pthread_mutex_lock(&cond_mutex);
        if (csd_common_data.thread_state == STATE_PROCESS_CMD) {
            LOGD("processing cmd in thread\n");
            process_csd_to_alsa_command();
            csd_common_data.thread_state = STATE_WAIT;
        }
        pthread_cond_timedwait(&condition, &cond_mutex, &ts);
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;
        pthread_mutex_unlock(&cond_mutex);
        LOGD("waiting for commands\n");
    }

    pthread_exit(NULL);
    return NULL;
}

int get_new_client_handle_from_pool(qmi_client_handle client_handle,
                                    void **connection_handle)
{
    int i = 0;
    client_handle = 0;
    int rc = 1;

    for(i = 0; i< MAX_CSD_QMI_CLIENTS; i++)
    {
        LOGD("csd_common_data.client_data[%d].index_in_use %d\n", i,
             csd_common_data.client_data[i].index_in_use);
        if (!csd_common_data.client_data[i].index_in_use)
        {
            memset(&csd_common_data.client_data[i], 0,
                   sizeof(csd_common_data.client_data[i]));
            csd_common_data.client_data[i].index_in_use = true;
            csd_common_data.client_data[i].qmi_client_handle_t = client_handle;
            *connection_handle = &csd_common_data.client_data[i];
            LOGD("client %d created\n", i);
            rc = 0;
            break;
        }
    }

    return rc;
}

#define MAX_CARD_RETRY 25
#define CARD_RETRY_DELAY 5

#define CVD_VERSION_MIXER_CTL "CVD Version"
#define MAX_CVD_VERSION_STRING_SIZE    100

static void get_cvd_version(char *cvd_version)
{
    int ret = 0;
    int count;
    struct mixer_ctl *ctl;
    struct mixer *mixer;
    const char* device = "/dev/snd/controlC0";

    mixer = mixer_open(device);
    if (!mixer) {
        LOGE("Fail to open mixer\n");

        return;
    }

    ctl = mixer_get_ctl_by_name(mixer, CVD_VERSION_MIXER_CTL);
    if (!ctl) {
        LOGE("Fail to get mixer ctl\n");

        goto done;
    }
    mixer_ctl_update(ctl);

    count = mixer_ctl_get_num_values(ctl);
    if (count > MAX_CVD_VERSION_STRING_SIZE)
        count = MAX_CVD_VERSION_STRING_SIZE;

    ret = mixer_ctl_get_array(ctl, cvd_version, count);
    if (ret != 0) {
        LOGE("Fail to get mixer_ctl_get_array\n");
    }

done:
    mixer_close(mixer);
    return;
}

int csd_alsa_preinit(void)
{
    int rc = 0;
    char *ucm_sound_card_name = NULL;
    int card_retry = 0;
    bool card_found = false;
    char *cvd_version = NULL;

    while (card_retry <= MAX_CARD_RETRY && card_found == false) {
       rc = get_ucm_sound_card_name(&ucm_sound_card_name);
       if (rc == 0) {
           LOGD("Detected Sound card Successfully");
           card_found = true;
       } else {
           card_retry++;
           LOGD("Card detection retry %d", card_retry);
           sleep(CARD_RETRY_DELAY);
       }
    }
    if (card_found == false || ucm_sound_card_name == NULL) {
        rc = -1;
        goto EXIT;
    }
    LOGD("ucm_sound_card_name: %s\n", ucm_sound_card_name);

    cvd_version = calloc(1, MAX_CVD_VERSION_STRING_SIZE);
    if (!cvd_version)
        LOGE("Failed to allocate cvd_version");
    else
        get_cvd_version(cvd_version);
    rc = acdb_loader_init_v2(ucm_sound_card_name, cvd_version);
    if (cvd_version)
        free(cvd_version);
    if (rc != 0) {
        LOGE("Failed to init ACDB in CSD");
        goto EXIT;
    }

    rc = snd_use_case_mgr_open(&csd_common_data.ucm_mgr, ucm_sound_card_name);
    if (rc != 0) {
        LOGE("\nFailed to open UCM instance\n");
        goto UCM_ERROR;
    }

    goto EXIT;

UCM_ERROR:
    if (csd_common_data.ucm_mgr != NULL) {
        snd_use_case_mgr_close(csd_common_data.ucm_mgr);
        csd_common_data.ucm_mgr == NULL;
    }
    acdb_loader_deallocate_ACDB();

EXIT:
    if (rc != 0) {
        LOGE("Error in csd_alsa_init() rc=%d\n", rc);
    }

    if (ucm_sound_card_name != NULL)
        free(ucm_sound_card_name);

    return rc;
}

int csd_alsa_init(void)
{
    int32_t rc = 0;
    csd_common_data.alsa_mixer = mixer_open("/dev/snd/controlC0");
    if (csd_common_data.alsa_mixer == NULL){
        LOGE("\nERROR opening mixer\n");
        rc = 1;
        goto EXIT;
    } else {
        rc = 0;
    }

    csd_common_data.thread_state = STATE_WAIT;
    rc = pthread_create(&(csd_common_data).thread, 0, thread_main,
                 NULL);

    if (rc != 0) {
        LOGE("Failed to start csd_alsa thread rc=%d\n",rc);
    }

EXIT:
    if (rc != 0) {
        LOGE("Error in csd_alsa_init() rc=%d\n", rc);
        if (csd_common_data.alsa_mixer != NULL)
            mixer_close(csd_common_data.alsa_mixer);
    }

    LOGD("csd_alsa_init() OUT rc=%d\n", rc);
    return rc;
}

int csd_alsa_deinit(void)
{
    int i = 0;

    for (i = 0; i < MAX_CSD_QMI_CLIENTS; i++) {
        csd_alsa_cleanup_client(&csd_common_data.client_data[i]);
    }

    if (csd_common_data.alsa_mixer != NULL)
        mixer_close(csd_common_data.alsa_mixer);

    return 0;
}

bool is_valid_clnt(struct client_data *clnt_data)
{
    int i = 0;

    for (i = 0; i < MAX_CSD_QMI_CLIENTS; i++){
        LOGD("is_valid_clnt() IN clnt_data:%08X\n"
             "&csd_common_data.client_data[i]:%08X\n",
             clnt_data, &csd_common_data.client_data[i]);

        if (clnt_data == &csd_common_data.client_data[i])
            return true;
    }

    return false;
}

int csd_alsa_close_device_handle(struct device_data *device_handle)
{
    int rc = 0;
    memset(device_handle, 0, sizeof(struct device_data));
    return rc;
}

int csd_alsa_close_voice_context_handle(struct voice_context_type *vc_handle)
{
    int rc = 0;
    memset(vc_handle, 0, sizeof(struct voice_context_type));
    return rc;
}

int csd_alsa_close_passive_control_stream_handle(struct voice_stream *vs_handle)
{
    int rc = 0;
    memset(vs_handle, 0, sizeof(struct voice_stream));
    return rc;
}

int csd_alsa_close_voice_stream_manager_handle(struct client_data *clnt_data,
                                               struct voice_stream_manager *vs_handle)
{
    int rc = 0;
    char *ucm_use_case = NULL;
    int session_id = -1;
    struct alsa_handle *alsa_handle = NULL;

    rc = get_session_id_from_vs_handle(clnt_data, vs_handle, &session_id);
    if (rc != 0) {
        LOGE("Could not get session_id from vs_handle=0x[%8x]", vs_handle);
        goto EXIT;
    }

    alsa_handle = &clnt_data->session[session_id].alsa_handle;

    if (csd_common_data.ucm_mgr != NULL) {
        rc = snd_use_case_get(csd_common_data.ucm_mgr, "_verb",
                              (const char **)&ucm_use_case);
        if (rc != 0) {
            LOGE("Failed to get verb from UCM rc=%d\n", rc);
            goto EXIT;
        }
        LOGD("\n alsa_handle_use_case=%s\n", alsa_handle->useCase);

        if ((ucm_use_case != NULL) &&
            (strlen(alsa_handle->useCase) > 0) &&
            strncmp(alsa_handle->useCase, SND_USE_CASE_VERB_INACTIVE,
                    strlen(alsa_handle->useCase))) {
                LOGD("\n UCM use case=%s\n", ucm_use_case);

                if (0 != strncmp(ucm_use_case, alsa_handle->useCase,
                                 MAX_UC_LEN)) {
                    rc = snd_use_case_set(csd_common_data.ucm_mgr, "_dismod",
                                          alsa_handle->useCase);
                    if (rc != 0) {
                        LOGD("\n Failed to disable mod=%s rc=%d\n",
                             alsa_handle->useCase, rc);
                    }
                } else {
                    rc = snd_use_case_set(csd_common_data.ucm_mgr, "_verb",
                                          SND_USE_CASE_VERB_INACTIVE);
                    if (rc != 0) {
                        LOGD("\n Failed to set UCM verb=%s rc=%d\n",
                             SND_USE_CASE_VERB_INACTIVE, rc);
                    }
                }
                alsa_del_ucm_entry(alsa_handle->useCase);
        }
    }

    memset(alsa_handle->useCase, 0, strlen(alsa_handle->useCase));
    memset(vs_handle, 0, sizeof(struct voice_stream_manager));

EXIT:
    if (ucm_use_case)
        free(ucm_use_case);
    return rc;
}

int csd_alsa_close_handle(struct client_data *clnt_data, uint32_t handle)
{
    int rc = 0;
    int i = 0;
    bool invalid_handle = true;

    if (is_valid_clnt(clnt_data)) {
        if ((uint32_t)&csd_common_data.device_data == handle) {
            rc = csd_alsa_close_device_handle(&csd_common_data.device_data);
            invalid_handle = false;
        } else {
            for (i = 0; i < MAX_SESSIONS_PER_CLIENT; i++) {
                if ((uint32_t)&clnt_data->session[i].voice_context_data == handle) {
                    rc = csd_alsa_close_voice_context_handle((struct voice_context_type *)&clnt_data->session[i].voice_context_data);
                    memset(&clnt_data->session[i].voice_context_data, 0,
                           sizeof(struct voice_context_type));
                    invalid_handle = false;
                    break;
                } else if ((uint32_t)&clnt_data->session[i].voice_passive_control_stream_data == handle) {
                    rc = csd_alsa_close_passive_control_stream_handle((struct voice_stream *)&clnt_data->session[i].voice_passive_control_stream_data);
                    memset(&clnt_data->session[i].voice_passive_control_stream_data, 0, sizeof(struct voice_stream));
                    invalid_handle = false;
                    break;
                } else if ((uint32_t)&clnt_data->session[i].voice_stream_manager_data == handle) {
                    rc = csd_alsa_close_voice_stream_manager_handle(clnt_data,
                                                                    (struct voice_stream_manager *)&clnt_data->session[i].voice_stream_manager_data);
                    memset(&clnt_data->session[i].voice_stream_manager_data,
                           0, sizeof(struct voice_stream_manager));
                    invalid_handle = false;
                    break;
                }
            }
        }
    } else {
        rc = 1;
    }

    return rc & invalid_handle;
}

bool is_valid_device_handle(struct client_data *clnt_data,
                            uint32_t device_handle)
{
    bool rc = false;

    if (is_valid_clnt(clnt_data)) {
        if (device_handle == (uint32_t)&csd_common_data.device_data)
            rc = true;
    }

    LOGD("is_valid_device_handle() :%d\n", rc);
    return rc;

}

bool is_valid_voice_context_handle(struct client_data *clnt_data,
                                   uint32_t voice_context_handle)
{
    int i = 0;
    bool rc = false;

    if (is_valid_clnt(clnt_data)) {
        for (i = 0; i < MAX_SESSIONS_PER_CLIENT; i++) {
            if (voice_context_handle ==
                (uint32_t)&clnt_data->session[i].voice_context_data) {
                rc = true;
                break;
            }
        }
    }

    LOGD("is_valid_voice_context_handle() :%d\n", rc);
    return rc;
}

bool is_valid_voice_stream_handle(struct client_data *clnt_data,
                                  uint32_t voice_stream_handle)
{
    int i = 0;
    bool rc = false;

    if (is_valid_clnt(clnt_data)) {
        for (i = 0; i < MAX_SESSIONS_PER_CLIENT; i++) {
            if (voice_stream_handle ==
                (uint32_t)&clnt_data->session[i].voice_passive_control_stream_data) {
                rc = true;
                break;
            }
        }
    }

    LOGD("is_valid_voice_stream_handle() :%d\n", rc);
    return rc;
}

bool is_valid_voice_manager_handle(struct client_data *clnt_data,
                                   uint32_t voice_stream_handle)
{
    int i = 0;
    bool rc = false;

    if (is_valid_clnt(clnt_data)) {
        for (i = 0; i < MAX_SESSIONS_PER_CLIENT; i++) {
            if (voice_stream_handle ==
                (uint32_t)&clnt_data->session[i].voice_stream_manager_data) {
                rc = true;
                break;
            }
        }
    }

    LOGD("is_valid_voice_manager_handle() :%d\n", rc);
    return rc;
}

int csd_create_device_handle(struct client_data *clnt_data,
                             uint32_t *dev_handle)
{
    int rc = 0;

    if (is_valid_clnt(clnt_data)) {
        *dev_handle = (uint32_t)&csd_common_data.device_data;
    } else {
        rc = 1;
    }

    LOGD("csd_get_device_handle() rc:%d dev_handle:%08X\n", rc, *dev_handle);
    return rc;
}

int csd_create_voice_stream_handle(struct client_data *clnt_data,
                                   char *session_name, uint32_t *vs_handle)
{
    int rc = 0;

    if (is_valid_clnt(clnt_data)) {
        if (!strncmp("default modem voice",
                     session_name, strlen("default modem voice"))) {
            *vs_handle = (uint32_t) &clnt_data->session[VOICE_SESSION_ID].voice_passive_control_stream_data;
        } else if (!strncmp("default volte voice",
                            session_name, strlen("default volte voice"))) {
            *vs_handle = (uint32_t)&clnt_data->session[VOLTE_SESSION_ID].voice_passive_control_stream_data;
        } else {
            *vs_handle = 0;
            LOGE("csd_get_voice_stream_handle() Err:Invalid session_name\n");
        }
    } else {
        LOGE("csd_get_voice_stream_handle() Err:Invalid client\n");
        rc = 1;
    }

    LOGD("csd_get_voice_stream_handle() rc:%d vs_handle:%08X\n", rc, *vs_handle);
    return rc;

}

int csd_create_voice_manager_handle(struct client_data *clnt_data,
                                    char *session_name, uint32_t *vm_handle)
{
    int rc = 0;
    int session_id = -1;

    if (is_valid_clnt(clnt_data)) {
        if (!strncmp("default modem voice",
                     session_name, strlen("default modem voice"))) {
            session_id = VOICE_SESSION_ID;
        } else if (!strncmp("default volte voice",
                            session_name, strlen("default volte voice"))) {
            session_id = VOLTE_SESSION_ID;
        } else {
            *vm_handle = 0;
            LOGE("csd_get_voice_manager_handle() Err:Invalid session_name\n");
        }
        if (session_id != -1) {
            *vm_handle =
                (uint32_t)&clnt_data->session[session_id].voice_stream_manager_data;
            rc = csd_alsa_set_usecase(clnt_data, session_id);
            if (rc != 0)
                LOGE("Failed to set UCM usecase=%d\n", rc);
        }
    } else {
        LOGE("csd_get_voice_manager_handle() Err:Invalid client\n");
        rc = 1;
    }

    LOGD("csd_get_voice_manager_handle() rc:%d vm_handle:%08X\n", rc, *vm_handle);
    return rc;

}

int csd_create_voice_context_handle(struct client_data *clnt_data,
                                    char *session_name, uint32_t *vc_handle)
{
    int rc = 0;

    if (is_valid_clnt(clnt_data)) {
        if (!strncmp("default modem voice",
                     session_name, strlen("default modem voice"))) {
            *vc_handle = (uint32_t)&clnt_data->session[VOICE_SESSION_ID].voice_context_data;
        } else if (!strncmp("default volte voice",
                            session_name, strlen("default volte voice"))) {
            *vc_handle = (uint32_t)&clnt_data->session[VOLTE_SESSION_ID].voice_context_data;
        } else {
            *vc_handle = 0;
            LOGE("csd_get_voice_context_handle() Err:Invalid session_name\n");
        }
    } else {
        LOGE("csd_get_voice_context_handle() Err:Invalid client\n");
        rc = 1;
    }
    LOGD("csd_get_voice_context_handle() rc:%d vc_handle:%08X\n", rc, *vc_handle);
    return rc;
}

int get_session_id_from_vc_handle(uint32_t voice_context_handle,
                                  struct client_data *command_client_data,
                                  uint *session_id)
{
    int rc = 0;

    if (voice_context_handle == (uint32_t)&command_client_data->session[VOICE_SESSION_ID].voice_context_data) {
        *session_id = VOICE_SESSION_ID;
    } else if (voice_context_handle == (uint32_t)&command_client_data->session[VOLTE_SESSION_ID].voice_context_data) {
        *session_id = VOLTE_SESSION_ID;
    } else {
        LOGE("get_session_id_from_voice_context_handle() Err:Invalid voice_context_handle\n");
        rc = 1;
    }
    return rc;
}

int get_session_id_from_vm_handle(uint32_t vm_handle,
                                  struct client_data *command_client_data,
                                  uint *session_id)
{
    int rc = 0;
    *session_id = -1;

    if (vm_handle == (uint32_t)&command_client_data->session[VOICE_SESSION_ID].voice_stream_manager_data) {
        *session_id = VOICE_SESSION_ID;
    } else if (vm_handle == (uint32_t)&command_client_data->session[VOLTE_SESSION_ID].voice_stream_manager_data) {
        *session_id = VOLTE_SESSION_ID;
    } else {
        LOGE("get_session_id_from_voice_manager_handle()\n"
             "Err:Invalid voice_manager_handle\n");
        rc = 1;
    }

    LOGD("get_session_id_from_voice_manager_handle()\n"
         "voice_manager_handle:0x%08X sessionid:%d rc:%d\n",
         vm_handle, *session_id, rc);
    return rc;
}


int get_session_id_from_vs_handle(uint32_t voice_stream_handle,
                                  struct client_data *command_client_data,
                                  uint *session_id)
{
    int rc = 0;

    if (voice_stream_handle == (uint32_t)&command_client_data->session[VOICE_SESSION_ID].voice_passive_control_stream_data) {
        *session_id = VOICE_SESSION_ID;
    } else if (voice_stream_handle == (uint32_t)&command_client_data->session[VOLTE_SESSION_ID].voice_passive_control_stream_data) {
        *session_id = VOLTE_SESSION_ID;
    } else {
        LOGE("get_session_id_from_voice_stream_handle() Err:Invalid get_session_id_from_voice_stream_handle\n");
        rc = 1;
    }

    return rc;
}

int csd_alsa_cleanup_client(struct client_data *clnt_data)
{
    int rc = 0;
    int i = 0, j = 0;
    bool session_active = false;

    if (is_valid_clnt(clnt_data)) {
        for (i = 0; i < MAX_SESSIONS_PER_CLIENT; i++) {
            alsa_stop_voice(&clnt_data->session[i]);
            memset(&clnt_data->session[i].alsa_handle, 0, sizeof(struct alsa_handle));
            csd_alsa_close_voice_context_handle(&clnt_data->session[i].voice_context_data);
            memset(&clnt_data->session[i].voice_context_data, 0, sizeof(struct voice_context_type));
            csd_alsa_close_passive_control_stream_handle(&clnt_data->session[i].voice_passive_control_stream_data);
            memset(&clnt_data->session[i].voice_passive_control_stream_data, 0, sizeof(struct voice_stream));
            csd_alsa_close_voice_stream_manager_handle(clnt_data, &clnt_data->session[i].voice_stream_manager_data);
            memset(&clnt_data->session[i].voice_stream_manager_data, 0, sizeof(struct voice_stream_manager));
        }
    } else {
        rc = 1;
    }

    return rc;
}
