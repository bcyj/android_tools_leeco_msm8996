/*
 * EFSKS: A wrapper around the Kickstart utility to perform EFS sync
 *
 * Copyright (C) 2012 Qualcomm Technologies, Inc. All rights reserved.
 *                    Qualcomm Technologies Proprietary/GTDR
 *
 * All data and information contained in or disclosed by this document is
 * confidential and proprietary information of Qualcomm Technologies, Inc. and all
 * rights therein are expressly reserved.  By accepting this material the
 * recipient agrees that this material and the information contained therein
 * is held in confidence and in trust and will not be used, copied, reproduced
 * in whole or in part, nor its contents revealed in any manner to others
 * without the express written permission of Qualcomm Technologies, Inc.
 *
 *
 *  efsks.c : A wrapper around the Kickstart utility to perform EFS sync
 * ==========================================================================================
 *   $Header: //source/qcom/qct/core/storage/tools/kickstart/efsks.c#1 $
 *   $DateTime: 2011/12/15 15:56:08 $
 *   $Author: ahughes $
 *
 *  Edit History:
 *  YYYY-MM-DD		who		why
 *  -----------------------------------------------------------------------------
 *  2011-12-15       ah      Initial creation
 *
 *  Copyright 2012 by Qualcomm Technologies, Inc.  All Rights Reserved.
 *
 *==========================================================================================
 */

#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(LINUXPC)
    #define LOGE printf
    #define LOGI printf
#else
    #define LOG_TAG "kickstart-efsks"
    #include "cutils/log.h"
    #include "common_log.h"
#endif

#define RET_SUCCESS                 0
#define RET_FAILED                  1

int start_sahara(char *ram_dump_path);
int get_ks_path(char* kspath, const char* exec_prog, const char* ksname, int kspath_len);
int TestForFile(char *filename, int attempt_read);

static char ks_path[1024]   = "./";
static char sz[512];
char EFS_TTY[512]           = "/dev/efs_hsic_bridge";
char PathToSaveFiles[1024]  = "./";
int RX_Timeout = -1;

int simplesystem(char* command) {
    pid_t process_pid;
    const char delimiters[] = " \t\n";
    char* token;
    //char *newargv[] = { efsks_path, "-p", "/dev/efs_bridge", "-w", "/dev/block/platform/msm_sdcc.1/by-name/", NULL };
    char **newargv = NULL;
    int num_tokens = 0;
    char *program_name;
    int i;
    char *command_string = NULL;
    int retval = RET_FAILED;
    int process_retval;
    size_t buffer_length;

    if (NULL == command) {
        LOGE("No command to run");
        goto free_simplesystem;
    }

    buffer_length = strlen(command) + 1;
    command_string = (char *) malloc(buffer_length * sizeof(char));
    if (NULL == command_string) {
        LOGE("Could not allocate memory");
        goto free_simplesystem;
    }
    if (strlcpy(command_string, command, buffer_length) >= buffer_length) {
        LOGE("String was truncated while copying");
        goto free_simplesystem;
    }

    token = strtok(command_string, delimiters);
    while (token != NULL) {
        num_tokens++;
        token = strtok(NULL, delimiters);
    }

    if (strlcpy(command_string, command, buffer_length) >= buffer_length) {
        LOGE("String was truncated while copying");
        goto free_simplesystem;
    }

    newargv = (char **) calloc(num_tokens + 1, sizeof(char *));
    if (NULL == newargv) {
        LOGE("Could not allocate memory");
        goto free_simplesystem;
    }

    for (i = 0; i < num_tokens + 1; i++)
        newargv[i] = NULL;

    for (i = 0; i < num_tokens; i++) {
        if (i == 0)
            token = strtok(command_string, delimiters);
        else
            token = strtok(NULL, delimiters);

        if (NULL == token) {
            LOGE("NULL token found after having counted tokens!");
            goto free_simplesystem;
        }

        buffer_length = strlen(token) + 1;
        newargv[i] = (char *) malloc(buffer_length * sizeof(char));
        if (NULL == newargv[i]) {
            LOGE("Could not allocate memory");
            goto free_simplesystem;
        }
        if (strlcpy(newargv[i], token, buffer_length) >= buffer_length) {
            LOGE("String was truncated while copying");
            goto free_simplesystem;
        }

        if (i == 0) {
            buffer_length = strlen(token) + 1;
            program_name = (char *) malloc(buffer_length * sizeof(char));
            if (NULL == program_name) {
                LOGE("Could not allocate memory");
                goto free_simplesystem;
            }
            if (strlcpy(program_name, token, buffer_length) >= buffer_length) {
                LOGE("String was truncated while copying");
                goto free_simplesystem;
            }
        }
    }

    process_pid = fork();
    if (process_pid < 0) {
        LOGE("Forking new process failed");
        goto free_simplesystem;
    }
    else if (process_pid == 0) {
        if (execvp(program_name, newargv) == -1) {
            LOGE("Spawning process using execvp failed");
            _exit(127);
        }
    }
    if (process_pid > 0) {
        wait(&process_retval);
        LOGE("Process return value %d", process_retval);
        process_pid = 0;

        if (process_retval == 0)
            retval = RET_SUCCESS;
    }


free_simplesystem:
    if (NULL != newargv) {
        for (i = 0; i < num_tokens; i++) {
            if (newargv[i] != NULL) {
                free(newargv[i]);
                newargv[i] = NULL;
            }
        }
        free(newargv);
        newargv = NULL;
    }
    if (NULL != command_string) {
        free(command_string);
        command_string = NULL;
    }
    if (NULL != program_name) {
        free(program_name);
        program_name = NULL;
    }

    return retval;
}

void print_usage (void)
{
    printf("\n\nEFSKS Built on '%s' at time '%s'\n",__DATE__,__TIME__);
    printf ("\nUsage: \n");
    printf ( " -p ttyport   --port       Device name for USB driver, i.e. /dev/ttyUSB0\n"
             " -t value     --timeout   in seconds, how long TTY device should wait\n"
             " -w path      --where     Where files from MDM are temporarily stored\n");
    printf ( "\n\nExample usage: \n"
             "\tsudo ./efsks -p /dev/ttyUSB0 -1 /dev/block/mmcblk0p10/ -2 /dev/block/mmcblk0p11/ -3 /dev/block/mmcblk0p12/ \n"
             "\tsudo ./efsks -p /dev/ttyUSB0 -t 120 -w ./\n");
    return;
}

int main(int argc, char *argv[])
{
    int option;
    const char *const short_options = "p:t:w:";    // possible cmd line short options
    const struct option long_options[] = {               // possible cmd line long options
        { "port",    		1, NULL, 'p' },
        { "timeout",  		1, NULL, 't' },
        { "where",  		1, NULL, 'w' },
        {  NULL,      		0, NULL,  0  }
    };

    if(argc < 2)
    {
        LOGE("Not enough arguments\n");
        print_usage();
        return RET_FAILED;
    }

    do {
        option = getopt_long (argc, argv, short_options, long_options, NULL);

        switch (option) {
        case -1:                // no more option arguments
            break;

        case 'p':               // Get the port string name
            strlcpy(EFS_TTY, optarg, strlen(EFS_TTY)+1);
			LOGI("EFS_TTY='%s'\n",EFS_TTY);
            break;

        case 'w':               // -w or --where - path for memory dump
            strlcpy(PathToSaveFiles, optarg, 1024);

            LOGI("Parsing 'where to save memorydump' options\n");

			if( PathToSaveFiles[strlen(PathToSaveFiles)-1]=='/' )
				LOGI("PathToSaveFiles='%s'\n",PathToSaveFiles);
			else
			{
				LOGI("ERROR: Path for memory dump must end with a \"/\"\n");
				LOGI("       should be \"-w /path/to/save/memorydump/\"\n\n");
               goto clean_and_exit;
			}

            break;

        case 't':               // Get the port string name
            RX_Timeout = atoi(optarg);
			LOGI("RX_Timeout=%i\n",RX_Timeout);
            break;

        default:                // unknown option
            LOGE("unrecognized option '%c'\n",option);
            print_usage ();
            goto clean_and_exit;
        }
    } while (option != -1);

    get_ks_path(ks_path, argv[0], "ks", sizeof(ks_path));
    if(TestForFile(ks_path, 0)==RET_FAILED)
        return EXIT_FAILURE;

    LOGI("EFSKS parameters");
	LOGI("EFS_TTY='%s'\n",EFS_TTY);
	LOGI("ks_path='%s'\n",ks_path);
	LOGI("PathToSaveFiles='%s'\n",PathToSaveFiles);
	LOGI("RX_Timeout=%i\n",RX_Timeout);

    while(1) {
        if(TestForFile(EFS_TTY, 0)==RET_FAILED) {
            LOGE("%s does not exist.", EFS_TTY);
            sleep(2); /* delay before launching ks for efs sync */
            continue;
        }
        if(start_sahara(PathToSaveFiles)!=RET_FAILED)
        {
            LOGE("EFS sync completed successfully\n\n");
        }
        else
        {
            LOGE("Back from KS call, something went wrong, trying again\n\n");
	        sleep(1);
        }
    }
    return RET_SUCCESS;

clean_and_exit:
    LOGE("Cleaning up and exiting");
    return RET_FAILED;
}

int start_sahara(char *ram_dump_path)
{
    int retval;
    char ThisDirectory[] = "./";

    if(strlen(ram_dump_path)==0)
        ram_dump_path = ThisDirectory; // avoid KS from complaining that -w arg is not formatted correctly

    if (snprintf(sz, sizeof(sz),"%s -m -v -w %s -p %s -t %d -l",
        ks_path,ram_dump_path, EFS_TTY, RX_Timeout) >= sizeof(sz)) {
        LOGE("String was truncated in snprintf");
        return RET_FAILED;
    }
    LOGE("RUNNING: '%s'",sz);
    retval = simplesystem(sz);
    LOGI("retval = %u",retval);

    if(retval != 0) {
        LOGE("ERROR: ks return code was %d, something failed", retval);
        return RET_FAILED;
    }
    LOGE("Sahara transfer completed successfully");
    return RET_SUCCESS;
}

int get_ks_path(char* kspath, const char* exec_prog, const char* ksname, int kspath_len)
{
    int i = strlen(exec_prog) - 1;
    int ksname_length = strlen(ksname);

    while(i >= 0 && exec_prog[i] != '/')
        i--;
    if (i >= 0){
        if(i+1 > kspath_len) {
            return 1;
        }
        strlcpy(kspath, exec_prog, i+2);
    }
    i++;
    if(i+ksname_length > kspath_len) {
        return 1;
    }
    strlcpy(kspath+i, ksname, ksname_length+1);
    kspath[i+ksname_length] = '\0';
    return RET_SUCCESS;
}

int TestForFile(char *filename, int attempt_read)
{
    struct stat status_buf;
    if (stat(filename, &status_buf) < 0) {
        LOGE("File '%s' was not found", filename);
        return RET_FAILED;
    }
    if (attempt_read) {
        FILE *fp = fopen(filename,"rb");
        if(fp) {
            LOGI("File:'%s' is available for reading", filename);
            fclose(fp);
            return RET_SUCCESS;
        }
        else {
            LOGE("File: %s could not be opened for reading", filename);
            return RET_FAILED;
        }
    }
    return RET_SUCCESS;
}
