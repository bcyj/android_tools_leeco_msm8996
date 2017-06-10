/*
** Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
** Qualcomm Technologies Proprietary and Confidential.
**
** Copyright 2011, The Android Open Source Project
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of The Android Open Source Project nor the names of
**       its contributors may be used to endorse or promote products derived
**       from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY The Android Open Source Project ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL The Android Open Source Project BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
** DAMAGE.
*/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "acph.h"

void sigint_handler(int sig)
{
    acdb_mcs_callback(ACPH_MCS_CMD_STOP, NULL, NULL, NULL, NULL, NULL);
}

int main(int argc, char **argv)
{
    struct ACPH_MCS_CMD_PLAY_REC_req req_param;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.pcm [-t tx_device] [-r rx_device] [-c channels] "
                "[-s sample_rate] [-b bits] [-d duration]\n", argv[0]);
        return 1;
    }

    acdb_mcs_init();
    req_param.rec_session.tx_device_id = 100;
    req_param.rec_session.sample_rate = 48000;
    req_param.rec_session.no_of_channels = 2;
    req_param.rec_session.bit_width = 16;
    req_param.rec_session.rec_dur_in_sec = 10;
    req_param.rec_session.write_to_file = 1;

    req_param.play_session.rx_device_id = 101;
    req_param.play_session.sample_rate = 48000;
    req_param.play_session.no_of_channels = 2;
    req_param.play_session.bit_width = 16;
    req_param.play_session.play_dur_in_sec = 10;
    req_param.play_session.playback_mode = 2;

    req_param.play_session.file_name_len = strlcpy(
                                            req_param.play_session.fileName,
                                            argv[1],
                                            sizeof(req_param.play_session.fileName));

    req_param.rec_session.file_name_len = strlcpy(
                                            &req_param.rec_session.fileName,
                                            argv[1],
                                            sizeof(req_param.rec_session.fileName));
    req_param.rec_session.fileName[req_param.rec_session.file_name_len++] = '_';
    req_param.rec_session.fileName[req_param.rec_session.file_name_len++] = 'r';
    req_param.rec_session.fileName[req_param.rec_session.file_name_len] = '\0';

    /* parse command line arguments */
    argv += 2;
    while (*argv) {
        if (strcmp(*argv, "-t") == 0) {
            argv++;
            if (*argv)
                req_param.rec_session.tx_device_id = atoi(*argv);
        } else if (strcmp(*argv, "-r") == 0) {
            argv++;
            if (*argv)
                req_param.play_session.rx_device_id  = atoi(*argv);
        } else if (strcmp(*argv, "-c") == 0) {
            argv++;
            if (*argv) {
                req_param.play_session.no_of_channels = atoi(*argv);
                req_param.rec_session.no_of_channels  = atoi(*argv);
            }
        } else if (strcmp(*argv, "-s") == 0) {
            argv++;
            if (*argv) {
                req_param.play_session.sample_rate = atoi(*argv);
                req_param.rec_session.sample_rate  = atoi(*argv);
            }
        } else if (strcmp(*argv, "-b") == 0) {
            argv++;
            if (*argv) {
                req_param.play_session.bit_width = atoi(*argv);
                req_param.rec_session.bit_width = atoi(*argv);
            }
        } else if (strcmp(*argv, "-d") == 0) {
            argv++;
            if (*argv) {
                req_param.play_session.play_dur_in_sec = atoi(*argv);
                req_param.rec_session.rec_dur_in_sec = atoi(*argv);
            }
        }

        if (*argv)
            argv++;
    }


    /* install signal handler and begin capturing */
    signal(SIGINT, sigint_handler);

    printf("Send the playback and recording command. \n\r");
    acdb_mcs_callback(ACPH_MCS_CMD_PLAY_REC, (uint8_t * )&req_param,
				sizeof(req_param), NULL, NULL, NULL);

    if (req_param.rec_session.rec_dur_in_sec > 0) {
        printf("Wait for %d sec (+3 sec)\n", req_param.rec_session.rec_dur_in_sec);
        sleep(req_param.rec_session.rec_dur_in_sec+3);
        acdb_mcs_callback(ACPH_MCS_CMD_STOP, NULL, NULL, NULL, NULL, NULL);
    } else {
	printf("contiuous play\n\r");
        while (1);
    }

    return 0;
}


