/*
 * Copyright (c) 2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cutils/atomic.h>
#include <cutils/properties.h> // for property_get

#include <hardware/sound_trigger.h>

#define OK 0

#define SOUNDTRIGGER_TEST_USAGE \
    "sound_trigger_test usage"  \
    "sound_trigger_test -sm <soundmodel> -nk <number of keywords>"

static void eventCallback(struct sound_trigger_recognition_event *event, void *sessionHndl __unused)
{
    printf("Callback event received: %d", event->status);
}

int main(int argc, char *argv[])
{
    char sound_model_file[128] = "";
    bool exit_loop = false;
    sound_model_handle_t sm_handle = 0;
    int sm_data_size  = 0;
    int sound_model_size = 0;
    unsigned int num_kws = 0;
    unsigned int i;
    struct sound_trigger_phrase_sound_model *sound_model = NULL;
    struct sound_trigger_recognition_config *rc_config = NULL;

    if (argc < 2) {
        printf(SOUNDTRIGGER_TEST_USAGE);
        return 0;
    }
    if(argc > 1) {
       if((strcmp(argv[1], "-sm") == 0) && (argc > 2)) {
           strlcpy(sound_model_file, argv[2],sizeof(sound_model_file));
       }
       if((strcmp(argv[3], "-nk") == 0) && (argc > 2)) {
           num_kws = atoi(argv[4]);
       }
    }

    int status = 0;
    const hw_module_t *mod;
    sound_trigger_hw_device_t *st_hw_device;
    char command[128];

    status = hw_get_module_by_class(SOUND_TRIGGER_HARDWARE_MODULE_ID,
                                    SOUND_TRIGGER_HARDWARE_MODULE_ID_PRIMARY, &mod);
    if (OK != status) {
        printf("hw_get_module_by_class() failed with %d\n", status);
        return status;
    }

    status = sound_trigger_hw_device_open(mod, &st_hw_device);
    if (OK != status || NULL == st_hw_device) {
        printf("sound_trigger_hw_device_open() failed with %d\n",status);
        return status;
    }

    FILE *fp = fopen(sound_model_file, "rb");
    if(fp == NULL) {
        printf("Could not open sound mode file : %s\n", sound_model_file);
        return -1;
    }

    /* Get the sound mode size i.e. file size */
    fseek( fp, 0, SEEK_END);
    sm_data_size  = ftell(fp);
    fseek( fp, 0, SEEK_SET);

    sound_model_size = sizeof(struct sound_trigger_phrase_sound_model) + sm_data_size;
    sound_model = (struct sound_trigger_phrase_sound_model *)calloc(1, sound_model_size);
    if (sound_model == NULL) {
        printf("Could not allocate memory for sound model");
        goto error_sm_mem;
    }
    sound_model->common.type = SOUND_MODEL_TYPE_KEYPHRASE;
    sound_model->common.data_size = sm_data_size;
    sound_model->common.data_offset = sizeof(*sound_model);
    sound_model->num_phrases = num_kws;
    for (i = 0; i < num_kws; i++) {
        sound_model->phrases[i].recognition_mode = RECOGNITION_MODE_VOICE_TRIGGER;
    }
    int bytes_read = fread((char*)sound_model+sound_model->common.data_offset , 1, sm_data_size , fp);
    printf("bytes from the file %d\n", bytes_read);
    if(bytes_read != sm_data_size) {
        printf("Something wrong while reading data from file: bytes_read %d file_size %d", bytes_read, sm_data_size);
        return -1;
    }

    printf("sound model data_size %d data_offset %d\n", sm_data_size, sound_model->common.data_offset);
    status = st_hw_device->load_sound_model(st_hw_device, &sound_model->common, NULL, NULL, &sm_handle);
    if (OK != status) {
       printf("load_sound_model failed\n");
       return status;
    }

    rc_config = (struct sound_trigger_recognition_config *)calloc(1, sizeof(struct sound_trigger_recognition_config));
    if (rc_config == NULL) {
        printf("Could not allocate memory for recognition config");
        goto error_rc_config_mem;
    }
    rc_config->capture_handle = AUDIO_IO_HANDLE_NONE;
    rc_config->capture_device = AUDIO_DEVICE_NONE;
    rc_config->capture_requested = 0;
    rc_config->num_phrases = num_kws;
    
    for (i = 0; i < num_kws; i++) {
       rc_config->phrases[i].id = i;
       rc_config->phrases[i].recognition_modes = RECOGNITION_MODE_VOICE_TRIGGER;
       rc_config->phrases[i].confidence_level = 60;
       // no user confidence levels sent
    }

    do {
        printf("Enter command <start/stop/exit>: ");
        fgets(command, 128, stdin);
        printf("Received the command: %s\n", command);

        if(!strncmp(command, "exit", 4)){
            printf("exiting the loop ..\n");
            exit_loop = true;
        } else if(!strncmp(command, "start", 5)) {
            status = st_hw_device->start_recognition(st_hw_device, sm_handle,
                                                     rc_config, eventCallback, NULL);
            if (OK != status) {
               printf("start_recognition failed\n");
               exit_loop = true;
            }
        } else if(!strncmp(command, "stop", 4)) {
            status = st_hw_device->stop_recognition(st_hw_device, sm_handle);
            if (OK != status) {
                printf("stop_recognition failed\n");
                exit_loop = true;
            }
        }
    } while(!exit_loop);

error_rc_config_mem:
    status = st_hw_device->unload_sound_model(st_hw_device, sm_handle);
    if (OK != status) {
       printf("unload_sound_model failed\n");
       return status;
    }
    if (rc_config)
       free(rc_config);
error_sm_mem:
    status = sound_trigger_hw_device_close(st_hw_device);
    if (OK != status) {
       printf("sound_trigger_hw_device_close() failed, status %d\n", status);
    }
    if (sound_model)
       free(sound_model);
    fclose(fp);
    return status;
}
