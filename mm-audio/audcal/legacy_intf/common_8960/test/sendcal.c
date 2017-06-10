/*
 * Copyright (c) 2010-2013 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 */


#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

#include "acdb-loader.h"

const char *help_text =
"\nmm-audio-send-cal: This app is used to send calibration from the ACDB\n\
to the kernel. Once calibration is sent to the kernel it should be then\n\
passed to the Q6 when a voice call, playback or record is made using\n\
another test app. If the UI (Audio HAL) is used to make a voice call,\n\
playback or record the calibration sent using this app will most likely\n\
be overwritten by the calibration currently in the ACDB for the device.\n\
currently selected in the AHAL. Source for this test app can be found\n\
under: vendor/qcom/proprietary/mm-audio/audcal/test/sendcal.c\n\
\n\
Usage:	Start process:\n\
	mm-audio-send-cal &\n\
\n\
	Commands once process has started:\n\
	echo \"-audio <acdb_id> <path_id>\" > /data/sendcal\n\
	echo \"-voice <rxacdb_id> <txacdb_id>\" > /data/sendcal\n\
\n\
	End process:\n\
	echo \"exit\" > /data/sendcal\n\
Examples:\n\
	mm-audio-send-cal &\n\
	echo \"-audio 4 2\" > /data/sendcal \n\
	echo \"-voice 7 4\" > /data/sendcal\n\
\n\
Note:\n\
	(i) path_id 1:RX, 2:TX, this needs to correspond with the acdb_id used\n\
	(ii) The ACDB ID can be found in the QACT tool by selecting 'device\n\
	     design' and clicking on the appropriate device.\n";


void process_audio(int acdb_id, int path_id)
{
	acdb_loader_send_audio_cal(acdb_id, path_id);
}


void process_voice(int rxacdb_id, int txacdb_id)
{
	acdb_loader_send_voice_cal(rxacdb_id, txacdb_id);
}

void execute_command(char *token, char *p_cmd)
{
	char *arg1;
	char *arg2;

	/* parse aguments for voice or audio cal */
	arg1 = strtok_r(NULL, " ", &p_cmd);
	if (arg1 == NULL)
		goto done;

	arg2 = strtok_r(NULL, " ", &p_cmd);
	if (arg2 == NULL)
		goto done;

	if (!strcmp(token, "-audio")) {
		process_audio(atoi(arg1), atoi(arg2));
	} else if (!strcmp(token, "-voice")) {
		process_voice(atoi(arg1), atoi(arg2));
	}
done:
	return;
}

void send_cal_process_loop(int fd)
{
	ssize_t		read_count;
	char		*token;
	char		cmdstr[256];
	char            *p_cmd;

	while (1) {
		cmdstr[0] = '\0';
		read_count = read(fd, cmdstr, 255);

		if (read_count == 0) {
			sleep(2);
		} else if (read_count < 0) {
			printf("send_cal: error reading cmd, exiting process...\n");
			break;
		} else {
			cmdstr[read_count-1] = ' ';
			cmdstr[read_count] = '\0';

			token = strtok_r(cmdstr, " ", &p_cmd);
                        if (token != NULL ) {
			    if (!strcmp(token, "exit")) {
				    break;
			    }
			    execute_command(token, p_cmd);
                        }
		}
	}
}

static void *send_cal_thread(void* arg)
{
	int result = 0;;
	int fd = 0;

	if (mknod("/data/sendcal", S_IFIFO | 0666, 0) < 0) {
		printf("Could not create /data/sendcal node\n");
		goto done;
	}

	fd = open("/data/sendcal", O_RDONLY);
	if (fd < 0) {
		printf("Could not open /data/sendcal file\n");
		goto remove_node;
	}

	if (acdb_loader_init_ACDB() < 0) {
		printf("Could not initialize ACDB!\n");
		goto close_fd;
	}

	send_cal_process_loop(fd);

close_fd:
	close(fd);
remove_node:
	remove("/data/sendcal");
done:
	pthread_exit((void*) result);
	return NULL;
}

int main(int argc, char **argv)
{
	int		result;
	pthread_t	process_thread;

	argc--;
	argv++;
	if (argc > 0) {
		printf("%s", help_text);
		goto done;
	}

	result = pthread_create(&process_thread, NULL, send_cal_thread, NULL);
	if (result < 0) {
		printf("Could not create send_cal thread!\n");
		goto done;
	}

	pthread_join(process_thread, NULL);
done:
	printf("Exiting program!\n");
	return 0;
}
