/*
 * QCKS: A wrapper around the Kickstart utility
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
 *  qcks.c : A wrapper around the Kickstart utility
 * ==========================================================================================
 *   $Header: //source/qcom/qct/core/storage/tools/kickstart/qcks.c#1 $
 *   $DateTime: 2011/01/12 11:30:11 $
 *   $Author: ahughes $
 *
 *  Edit History:
 *  YYYY-MM-DD		who		why
 *  -----------------------------------------------------------------------------
 *  2010-12-18       ah      Initial creation
 *  2010-12-28       ab      Added QMI API calls for UART control release
 *
 *  Copyright 2012 by Qualcomm Technologies, Inc.  All Rights Reserved.
 *
 *==========================================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>

#if !defined(LINUXPC)
#include <linux/ioctl.h>
#include <cutils/properties.h>
#define LOG_TAG "kickstart-qcks"
#include "cutils/log.h"
#include "common_log.h"
#include <linux/msm_charm.h>
#else
#define PROPERTY_VALUE_MAX 1024
#endif

#define DELAY_BETWEEN_RETRIES_MS    (500)
#define RET_SUCCESS                 0
#define RET_FAILED                  1
#define COLLECT_RAM_DUMPS           1
#define MAX_STRING_SIZE             1024

#if defined(FUSION2)
#define NUM_RETRIES                 (100)
#define SBL1_IMAGE_ID               -1
#elif defined(FUSION3)
#define NUM_RETRIES                 (10)
#define SBL1_IMAGE_ID               21
#endif

void print_usage(void);
int TestForFile(char *filename, int attempt_read);
int DoPrepend(void);
int LoadDload(char *PATHTOIMAGES);
int LoadSahara(char* MemoryDump, char *PATHTOIMAGES, char *, char *);
int MemoryDump(char *PATHTOIMAGES, char *PATHFORRAMDUMP);
int BringMDM9kOutOfReset(int fd);
int WaitForCOMport(char *DevNode, int attempt_read);
int startswith(const char* haystack, const char* needle);
int get_ks_path(char* kspath, const char* exec_prog, const char* ksname, int kspath_len);

static char COMPORT_DLOAD[MAX_STRING_SIZE]   	="/dev/ttyHSL1";
#if defined(FUSION2)
static char COMPORT_SAHARA[MAX_STRING_SIZE]   	="/dev/tty_sdio_00";
static char EFSRAW1[MAX_STRING_SIZE]      ="/dev/block/mmcblk0p19";
static char EFSRAW2[MAX_STRING_SIZE]      ="/dev/block/mmcblk0p20";
static char EFSRAW3[MAX_STRING_SIZE]      ="/dev/block/mmcblk0p21";
#elif defined(FUSION3)
static char COMPORT_SAHARA[MAX_STRING_SIZE]   	="/dev/ks_hsic_bridge";
static char EFSRAW1[MAX_STRING_SIZE]      ="/dev/block/platform/msm_sdcc.1/by-name/m9kefs1";
static char EFSRAW2[MAX_STRING_SIZE]      ="/dev/block/platform/msm_sdcc.1/by-name/m9kefs2";
static char EFSRAW3[MAX_STRING_SIZE]      ="/dev/block/platform/msm_sdcc.1/by-name/m9kefs3";
static char efsks_path[MAX_STRING_SIZE];
static char HeaderACDB[MAX_STRING_SIZE]="acdb.mbn";
static char ACDBIMAGE[MAX_STRING_SIZE] ="mdm_acdb.img";
static char ACDBRAW[MAX_STRING_SIZE]      ="/system/etc/firmware/mdm_acdb.img";
static pid_t efsks_pid = 0;
#endif
static char PATHTOIMAGES[MAX_STRING_SIZE]  = "/system/etc/firmware/";
static char PATHFORRAMDUMP[MAX_STRING_SIZE]= "/tombstones/mdm/";
static char PATHFOREFS[MAX_STRING_SIZE]    = "/data/qcks/";
static char Header1[MAX_STRING_SIZE]   ="efs1.mbn";
static char Header2[MAX_STRING_SIZE]   ="efs2.mbn";
static char Header3[MAX_STRING_SIZE]   ="efs3.mbn";

static char sz[MAX_STRING_SIZE];
static char ks_path[MAX_STRING_SIZE];

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

int main (int argc, char *argv[])
{
    char Option;
    int RetCode = -1;
    int fd;
    int boot = CHARM_NORMAL_BOOT;
    int first_boot = 1;
    int nRetries;
    int mdm_debug_mode = 0;
    char mdm_debug_mode_property[PROPERTY_VALUE_MAX];

    int    option; /*holds the option from getopt_long*/    
    const char *const short_options = "d:s:i:r:1:2:3:e:";    // possible cmd line short options
    const struct option long_options[] = {               // possible cmd line long options
        { "dloadport",      1, NULL, 'd' },
        { "saharaport",     1, NULL, 's' },
        { "pathimages",     1, NULL, 'i' },
        { "pathramdump",    1, NULL, 'r' },
        { "efsraw1",        1, NULL, '1' },
        { "efsraw2",        1, NULL, '2' },
        { "efsraw3",        1, NULL, '3' },
        { "pathefsprepend", 1, NULL, 'e' },
        {  NULL,            0, NULL,  0  }
    };

    // check argument count, print the usage help and quit
    if(argc < 2) {
        print_usage();
        return EXIT_FAILURE;
    }
    
    do {
        option = getopt_long (argc, argv, short_options, long_options, NULL);

        switch (option) {
        case -1:                /* no more option arguments */
            break;

        case 'd':               /* -h or --help */
            if (strlcpy(COMPORT_DLOAD,optarg,sizeof(COMPORT_DLOAD)) >= sizeof(COMPORT_DLOAD)) {
                LOGE ("String was truncated while copying");
                return EXIT_FAILURE;
            }
            break;

        case 's':               /* -h or --help */
            if (strlcpy(COMPORT_SAHARA,optarg,sizeof(COMPORT_SAHARA)) >= sizeof(COMPORT_SAHARA)) {
                LOGE ("String was truncated while copying");
                return EXIT_FAILURE;
            }
            break;

        case 'i':               /* -h or --help */
            if (strlcpy(PATHTOIMAGES,optarg,sizeof(PATHTOIMAGES)) >= sizeof(PATHTOIMAGES)) {
                LOGE ("String was truncated while copying");
                return EXIT_FAILURE;
            }
            if(PATHTOIMAGES[strlen(PATHTOIMAGES)-1]!='/') {
                LOGE("PATHTOIMAGES is not formed well, must *end* with a '/'");
                return EXIT_FAILURE;
            }
#if defined(FUSION3)
            if (strlcpy(ACDBRAW,optarg,sizeof(ACDBRAW)) >= sizeof(ACDBRAW)) {
                LOGE ("String was truncated while copying");
                return EXIT_FAILURE;
            }
            if (strlcat(ACDBRAW,ACDBIMAGE,sizeof(ACDBRAW)) >= sizeof(ACDBRAW)) {
                LOGE ("String was truncated while concatenating");
                return EXIT_FAILURE;
            }
#endif
            break;

        case 'r':               /* -h or --help */
            if (strlcpy(PATHFORRAMDUMP,optarg,sizeof(PATHFORRAMDUMP)) >= sizeof(PATHFORRAMDUMP)) {
                LOGE ("String was truncated while copying");
                return EXIT_FAILURE;
            }
            if(PATHFORRAMDUMP[strlen(PATHFORRAMDUMP)-1]!='/') {
                LOGE("PATHFORRAMDUMP is not formed well, must *end* with a '/'");
                return EXIT_FAILURE;
            }
            break;

        case '1':               /* -h or --help */
            if (strlcpy(EFSRAW1,optarg,sizeof(EFSRAW1)) >= sizeof(EFSRAW1)) {
                LOGE ("String was truncated while copying");
                return EXIT_FAILURE;
            }
            break;

        case '2':               /* -h or --help */
            if (strlcpy(EFSRAW2,optarg,sizeof(EFSRAW2)) >= sizeof(EFSRAW2)) {
                LOGE ("String was truncated while copying");
                return EXIT_FAILURE;
            }
            break;

        case '3':               /* -h or --help */
            if (strlcpy(EFSRAW3,optarg,sizeof(EFSRAW3)) >= sizeof(EFSRAW3)) {
                LOGE ("String was truncated while copying");
                return EXIT_FAILURE;
            }
            break;

        case 'e':               /* -h or --help */
            if (strlcpy(PATHFOREFS,optarg,sizeof(PATHFOREFS)) >= sizeof(PATHFOREFS)) {
                LOGE ("String was truncated while copying");
                return EXIT_FAILURE;
            }
            if(PATHFOREFS[strlen(PATHFOREFS)-1]!='/') {
                LOGE("PATHFOREFS is not formed well, must *end* with a '/'");
                return EXIT_FAILURE;
            }
            break;

        default:                /* unknown option. */
            LOGE("Unrecognized option");
            print_usage ();
            return EXIT_FAILURE;
        }
    } while (option != -1);

    LOGI("PARAMETER VALUES");
    LOGI("COMPORT_DLOAD=%s",COMPORT_DLOAD);
    LOGI("COMPORT_SAHARA=%s",COMPORT_SAHARA);
    LOGI("PATHTOIMAGES=%s",PATHTOIMAGES);
    LOGI("PATHFORRAMDUMP=%s",PATHFORRAMDUMP);
    LOGI("EFSRAW1=%s",EFSRAW1);
    LOGI("EFSRAW2=%s",EFSRAW2);
    LOGI("EFSRAW3=%s",EFSRAW3);
    LOGI("Header1=%s",Header1);
    LOGI("Header2=%s",Header2);
    LOGI("Header3=%s",Header3);

    get_ks_path(ks_path, argv[0], "ks", sizeof(ks_path));
    if(TestForFile(ks_path, 0)==RET_FAILED)
        return EXIT_FAILURE;

    #if defined(FUSION3)
    get_ks_path(efsks_path, argv[0], "efsks", sizeof(efsks_path));
    if(TestForFile(efsks_path, 0)==RET_FAILED)
        return EXIT_FAILURE;
    #endif

    if(WaitForCOMport("/dev/mdm", 0) == RET_FAILED) {
        LOGE("ERROR: Can't bring MDM9K out of reset");
        return EXIT_FAILURE;
    }
    fd = open("/dev/mdm", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        LOGE("Could not open device %s", "/dev/mdm");
        return EXIT_FAILURE;
	}

    RetCode = property_get("mdm.debug", mdm_debug_mode_property, "false");
    if (RetCode > 0 && !strncmp(mdm_debug_mode_property, "true", RetCode)) {
        mdm_debug_mode = 1;
        LOGE("Property mdm.debug=true. Running in MDM debug mode");
    }

#if defined(FUSION2)
    while(1) {
        if (BringMDM9kOutOfReset(fd) == RET_FAILED)
            continue;

        if (boot == CHARM_NORMAL_BOOT) {
            if ((RetCode = LoadDload(PATHTOIMAGES)) != RET_FAILED)
            {
                if ((RetCode = DoPrepend()) != RET_FAILED)
                {
                    if (first_boot) {
                        RetCode = LoadSahara("", PATHTOIMAGES, PATHFORRAMDUMP, PATHFOREFS);
                        first_boot = 0;
                    }
                    else
                        RetCode = LoadSahara("-i ", PATHTOIMAGES, PATHFORRAMDUMP, PATHFOREFS);
                }
            }
            if (ioctl(fd, NORMAL_BOOT_DONE, &RetCode) < 0) {
                LOGE("Could not issue ioctl NORMAL_BOOT_DONE");
                continue;
            }
        }
        else if (boot == CHARM_RAM_DUMPS) {
            if ((RetCode = LoadDload(PATHTOIMAGES)) != RET_FAILED)
            {
                if ((RetCode = DoPrepend()) != RET_FAILED)
                {
                    RetCode = LoadSahara("-m ", PATHTOIMAGES, PATHFORRAMDUMP, PATHFOREFS);
                }
            }
            if (ioctl(fd, RAM_DUMP_DONE, &RetCode) < 0) {
                LOGE("Could not issue ioctl RAM_DUMP_DONE");
                continue;
            }
        }
        if (ioctl(fd, WAIT_FOR_RESTART, &boot) < 0) {
            LOGE("Could not issue ioctl WAIT_FOR_RESTART");
            continue;
        }
    }
#elif defined(FUSION3)
    while(1) {
        if (BringMDM9kOutOfReset(fd) == RET_FAILED)
            continue;
        usleep(2000*1000);

        if (mdm_debug_mode) {
            LOGE("Will not continue with image transfer");
            break;
        }

        if (boot == CHARM_NORMAL_BOOT) {
            if ((RetCode = DoPrepend()) != RET_FAILED)
            {
                for(nRetries = 0; nRetries < NUM_RETRIES; nRetries++) {
                    if (first_boot) {
                        RetCode = LoadSahara("", PATHTOIMAGES, PATHFORRAMDUMP, PATHFOREFS);
                    }
                    else
                        RetCode = LoadSahara("-i ", PATHTOIMAGES, PATHFORRAMDUMP, PATHFOREFS);
                    
                    if(RetCode == RET_SUCCESS)
                        break;
                    else {
                        if (BringMDM9kOutOfReset(fd) == RET_FAILED)
                            continue;
                        usleep(2000*1000);
                    }
                }
                if (first_boot)
                    first_boot = 0;
            }
            if (ioctl(fd, NORMAL_BOOT_DONE, &RetCode) < 0) {
                LOGE("Could not issue ioctl NORMAL_BOOT_DONE");
                continue;
            }

            if(RetCode == RET_SUCCESS && efsks_pid == 0) {
                LOGE("Spawning efsks");
                efsks_pid = fork();
                if (efsks_pid < 0) {
                    LOGE("Forking new process for efsks failed");
                    continue;
                }
                else if (efsks_pid == 0) {
                    char *newargv[] = { efsks_path, "-p", "/dev/efs_hsic_bridge", "-w", "/dev/block/platform/msm_sdcc.1/by-name/", NULL };
                    char *newenviron[] = { NULL };
                    if (execve(efsks_path, newargv, newenviron) == -1) {
                        LOGE("Spawning efsks using execve failed");
                        _exit(127);
                    }
                }
            }
        }
        else if (boot == CHARM_RAM_DUMPS) {
            if ((RetCode = DoPrepend()) != RET_FAILED)
            {
                for(nRetries = 0; nRetries < NUM_RETRIES; nRetries++) {
                    RetCode = MemoryDump(PATHTOIMAGES, PATHFORRAMDUMP);
                    if(RetCode == RET_SUCCESS)
                        break;
                    else {
                        if (BringMDM9kOutOfReset(fd) == RET_FAILED)
                            continue;
                        usleep(2000*1000);
                    }
                }
            }
            if (ioctl(fd, RAM_DUMP_DONE, &RetCode) < 0) {
                LOGE("Could not issue ioctl RAM_DUMP_DONE");
                continue;
            }
        }
        if (ioctl(fd, WAIT_FOR_RESTART, &boot) < 0) {
            LOGE("Could not issue ioctl WAIT_FOR_RESTART");
            continue;
        }
        /*if (efsks_pid>0) {
            LOGE("Issuing kill(%i) for EFSKS\n", efsks_pid);
            kill(efsks_pid, SIGTERM);
            wait(&RetCode);
            LOGE("EFSKS should be dead");
            efsks_pid = 0;
        }*/
    }
#endif
    close(fd);

    return EXIT_SUCCESS;
}

int DoPrepend(void)
{
    LOGI("EFS Prepend");
    char szPath1[2048];
    char szPath2[2048];
    char szPath3[2048];
#if defined(FUSION3)
    char szPathACDB[2048];
#endif

    if(strlen(PATHTOIMAGES)>0) {
        if (strlcpy(szPath1,PATHTOIMAGES, sizeof(szPath1)) >= sizeof(szPath1)
            || strlcpy(szPath2,PATHTOIMAGES, sizeof(szPath2)) >= sizeof(szPath2)
            || strlcpy(szPath3,PATHTOIMAGES, sizeof(szPath3)) >= sizeof(szPath3)
#if defined(FUSION3)
            || strlcpy(szPathACDB,PATHTOIMAGES, sizeof(szPathACDB)) >= sizeof(szPathACDB)
#endif
        ) {
            LOGE("String was truncated while copying");
            return RET_FAILED;
        }

        if (strlcat(szPath1,Header1, sizeof(szPath1)) >= sizeof(szPath1)
            || strlcat(szPath2,Header2, sizeof(szPath2)) >= sizeof(szPath2)
            || strlcat(szPath3,Header3, sizeof(szPath3)) >= sizeof(szPath3)
#if defined(FUSION3)
            || strlcat(szPathACDB,HeaderACDB, sizeof(szPathACDB)) >= sizeof(szPathACDB)
#endif
        ) {
            LOGE("String was truncated while concatenating");
            return RET_FAILED;
        }
    }
    else {
        if (strlcpy(szPath1,Header1, sizeof(szPath1)) >= sizeof(szPath1)
            || strlcpy(szPath2,Header2, sizeof(szPath2)) >= sizeof(szPath2)
            || strlcpy(szPath3,Header3, sizeof(szPath3)) >= sizeof(szPath3)
#if defined(FUSION3)
            || strlcpy(szPathACDB,HeaderACDB, sizeof(szPathACDB)) >= sizeof(szPathACDB)
#endif
        ) {
            LOGE("String was truncated while copying");
            return RET_FAILED;
        }
    }

    if (TestForFile(szPath1, 1) == RET_FAILED || TestForFile(szPath2, 1) == RET_FAILED /*|| TestForFile(szPath3, 1) == RET_FAILED*/
     || TestForFile(EFSRAW1, 1) == RET_FAILED || TestForFile(EFSRAW2, 1) == RET_FAILED /*|| TestForFile(EFSRAW3, 1) == RET_FAILED*/
#if defined(FUSION3)
     || TestForFile(szPathACDB, 1) == RET_FAILED || TestForFile(ACDBRAW, 1) == RET_FAILED
#endif
    ) {
        LOGE("Prepend failed.");
        return RET_FAILED;
    }

    LOGI("Reading RAW EFS1 partition");
    if (snprintf(sz, sizeof(sz),"dd if=%s of=%stemp.dump bs=1024 count=3072",EFSRAW1,PATHFOREFS) >= sizeof(sz)) {
        LOGE("String was truncated in snprintf");
        return RET_FAILED;
    }
    LOGI("Running %s",sz);
    simplesystem(sz);
    if (snprintf(sz, sizeof(sz), "chmod 666 %stemp.dump",PATHFOREFS) >= sizeof(sz)) {
        LOGE("String was truncated in snprintf");
        return RET_FAILED;
    }
    simplesystem(sz);
    LOGI("Combining Header1 with RAW EFS1 partition");
    if (snprintf(sz, sizeof(sz),"cat %s%s %stemp.dump > %sefs1.bin",PATHTOIMAGES,Header1,PATHFOREFS,PATHFOREFS) >= sizeof(sz)) {
        LOGE("String was truncated in snprintf");
        return RET_FAILED;
    }
    LOGI("Running %s",sz);
    system(sz);

    LOGI("Reading RAW EFS2 partition");
    if (snprintf(sz, sizeof(sz),"dd if=%s of=%stemp.dump bs=1024 count=3072",EFSRAW2,PATHFOREFS) >= sizeof(sz)) {
        LOGE("String was truncated in snprintf");
        return RET_FAILED;
    }
    LOGI("Running %s",sz);
    simplesystem(sz);

    LOGI("Combining Header2 with RAW EFS2 partition");
    if (snprintf(sz, sizeof(sz),"cat %s%s %stemp.dump > %sefs2.bin",PATHTOIMAGES,Header2,PATHFOREFS,PATHFOREFS) >= sizeof(sz)) {
        LOGE("String was truncated in snprintf");
        return RET_FAILED;
    }
    LOGI("Running %s",sz);
    system(sz);

    LOGI("Reading RAW EFS3 partition");
    if (snprintf(sz, sizeof(sz),"dd if=%s of=%stemp.dump bs=1024 count=3072",EFSRAW3,PATHFOREFS) >= sizeof(sz)) {
        LOGE("String was truncated in snprintf");
        return RET_FAILED;
    }
    LOGI("Running %s",sz);
    simplesystem(sz);

    LOGI("Combining Header3 with RAW EFS3 partition");
    if (snprintf(sz, sizeof(sz),"cat %s%s %stemp.dump > %sefs3.bin",PATHTOIMAGES,Header3,PATHFOREFS,PATHFOREFS) >= sizeof(sz)) {
        LOGE("String was truncated in snprintf");
        return RET_FAILED;
    }
    LOGI("Running %s",sz);
    system(sz);

#if defined(FUSION3)
    LOGI("Combining ACDB Header with ACDB binary");
    if (snprintf(sz, sizeof(sz),"cat %s%s %s > %sacdb.bin",PATHTOIMAGES,HeaderACDB,ACDBRAW,PATHFOREFS) >= sizeof(sz)) {
        LOGE("String was truncated in snprintf");
        return RET_FAILED;
    }
    LOGI("Running %s",sz);
    system(sz);
#endif
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

void print_usage(void)
{
    printf("\nBuilt on '%s' at '%s'\n",__DATE__,__TIME__);
    printf("\n\nExample usage:"
           "\n\tsudo qcks m\t\t<-- Memory dump"
           "\n\tsudo qcks p\t\t<-- Prepend, creates efs1.bin and efs2.bin"
           "\n\tsudo qcks l\t\t<-- Prepend + Load ALL Images"
           "\n\tsudo qcks d\t\t<-- Load DLOAD images"
           "\n\tsudo qcks s\t\t<-- Load SAHARA Images (does not do Prepend)"
           "\n\n");
    return;
}

int MemoryDump(char *PATHTOIMAGES, char *PATHFORRAMDUMP)
{
    LOGI("Memory Dump");
#if defined(FUSION2)
    LoadDload(PATHTOIMAGES);
#endif
    return LoadSahara("-m ", PATHTOIMAGES, PATHFORRAMDUMP, PATHFOREFS);	// memory dump
}

int LoadDload(char *PATHTOIMAGES)
{
    LOGI("Loading Dload");
    int RetCode;
    int i;
    int delay_ms = 10;
    if(WaitForCOMport(COMPORT_DLOAD, 0) == RET_FAILED) {
        LOGE("DLoad failed");
        return RET_FAILED;
    }

    if (snprintf(sz, sizeof(sz),"%s -o -v -p %s -d 10:%sdbl.mbn", ks_path, COMPORT_DLOAD,PATHTOIMAGES) >= sizeof(sz)) {
        LOGE("String was truncated in snprintf");
        return RET_FAILED;
    }

    for (i=0; i<NUM_RETRIES; i++) {
        LOGE("RUNNING:\t%s",sz);
        RetCode = simplesystem(sz);

        if(RetCode != 0) {
            LOGE("ERROR: Dload return code was %d, something failed", RetCode);
            usleep(delay_ms*1000);
            delay_ms += 10;
        }
        else {
            LOGE("Dload completed successfully");
            return RET_SUCCESS;
        }
    }
    return RET_FAILED;
}

int LoadSahara(char* MemoryDump, char *PATHTOIMAGES, char *PATHFORRAMDUMP, char *PATHFOREFS)
{
    LOGI("Loading Sahara images");
    int RetCode;
    if(WaitForCOMport(COMPORT_SAHARA, 0) == RET_FAILED) {
        LOGE("LoadSahara failed");
        return RET_FAILED;
    }
    if (snprintf(sz, sizeof(sz),"%s %s -w %s -p %s -r %d"
                                " -s 2:%samss.mbn"
                                " -s 6:%sapps.mbn"
                                " -s 8:%sdsp1.mbn"
                                " -s 11:%sosbl.mbn"
                                " -s 12:%sdsp2.mbn"
                                " -s 21:%ssbl1.mbn"
                                " -s 22:%ssbl2.mbn"
                                " -s 23:%srpm.mbn"
                                " -s 28:%sdsp3.mbn"
                                " -s 16:%sefs1.bin"
                                " -s 17:%sefs2.bin"
                                " -s 20:%sefs3.bin"
                                " -s 29:%sacdb.bin",
                                ks_path, MemoryDump,PATHFORRAMDUMP,COMPORT_SAHARA,SBL1_IMAGE_ID,
                                PATHTOIMAGES,PATHTOIMAGES,PATHTOIMAGES,PATHTOIMAGES,PATHTOIMAGES,PATHTOIMAGES,PATHTOIMAGES,PATHTOIMAGES,PATHTOIMAGES,
                                PATHFOREFS,PATHFOREFS,PATHFOREFS,PATHFOREFS) >= sizeof(sz)) {
        LOGE("String was truncated in snprintf");
        return RET_FAILED;
    }
    LOGE("RUNNING:\t%s",sz);
    RetCode = simplesystem(sz);
    LOGI("RetCode = %u",RetCode);

    if(RetCode != 0) {
        if (RetCode == 1280) {
            LOGE("ERROR: RAM dumps were forced unexpectedly");
        }
        else {
            LOGE("ERROR: ks return code was %d, something failed", RetCode);
        }
        return RET_FAILED;
    }
    LOGE("Sahara transfer completed successfully");
    return RET_SUCCESS;
}

int BringMDM9kOutOfReset(int fd)
{
    LOGI("Trying to bring MDM9K out of reset");
    if (ioctl(fd, WAKE_CHARM) < 0) {
        LOGE("Could not issue ioctl WAKE_CHARM");
        return RET_FAILED;
    }
    return RET_SUCCESS;
}

int WaitForCOMport(char *DevNode, int attempt_read)
{
    LOGI("Testing if port \"%s\" exists", DevNode);
    struct stat status_buf;
    int i;
    for (i=0; i<NUM_RETRIES && stat(DevNode, &status_buf) < 0; i++) {
        LOGE("Couldn't find \"%s\", %i of %i", DevNode, i+1, NUM_RETRIES);
        usleep(DELAY_BETWEEN_RETRIES_MS*1000);
    }
    if (i == NUM_RETRIES) {
        LOGE("'%s' was not found", DevNode);
        return RET_FAILED;
    }
    if (attempt_read) {
        FILE *fd;
        LOGI("Attempting to open port \"%s\" for reading", DevNode);
        for (i=0; i<NUM_RETRIES && (fd = fopen(DevNode,"r"))==NULL; i++) {
            LOGE("Couldn't read \"%s\", %i of %i", DevNode, i+1, NUM_RETRIES);
            usleep(DELAY_BETWEEN_RETRIES_MS*1000);
        }
        if (i == NUM_RETRIES) {
            LOGE("'%s' could not be opened for reading", DevNode);
            return RET_FAILED;
        }
        fclose(fd);
    }
    return RET_SUCCESS;
}

int startswith(const char* haystack, const char* needle)
{
    if(strlen(needle) > strlen(haystack))
        return RET_FAILED;
    int i;
    for(i = 0; needle[i] != '\0' && haystack[i] == needle[i]; i++);
    if (needle[i] == '\0')
        return RET_SUCCESS;
    return RET_FAILED;
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
