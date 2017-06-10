/*
* Copyright (c) 2011-2012 Qualcomm Atheros Inc. All Rights Reserved.
* Qualcomm Atheros Proprietary and Confidential.
*/

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/if.h>
#include <linux/wireless.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <assert.h>
#include <ctype.h>
#include <math.h>

#include "testcmd.h"
#include "athtestcmd.h"
#include "sinit_common.h"

#ifdef ATHTESTCMD_LIB
#include <setjmp.h>
extern void testcmd_error(int code, const char *fmt, ...);
#define A_ERR testcmd_error
#else
#define A_ERR err
#endif

// defines

#define strnicmp strncmp

typedef unsigned long A_UINT32;
typedef unsigned char A_UINT8;
typedef int	      A_BOOL;

#ifdef ANDROID
#define _TCMD_PATH "/system/bin/athtestcmd -i wlan0"
#else
#define _TCMD_PATH "./athtestcmd -i wlan0"
#endif

int main(void)
{
    char  cmd[128];
    A_UINT32 i;
    A_UINT32 freq;

#ifdef ANDROID
    system("insmod /system/lib/modules/cfg80211.ko");
    sleep(2);

    system("insmod /system/lib/modules/ath6kl_sdio.ko testmode=2");
    sleep(3);
#else
    system("insmod /lib/modules/`uname -r`/kernel/drivers/net/wireless/ath/ath6kl/ath6kl_sdio.ko testmode=2");
    sleep(3);
#endif

    freq=2412;

    for (i = 0; i < NumEntriesPSTSweepTable; i++)
    {
	    memset(cmd, 0, sizeof(cmd));
	    snprintf(cmd, sizeof(cmd), "%s --psat_char %ld --txfreq %ld", _TCMD_PATH, i, freq);
	    //printf("%s\n", cmd);
	    system(cmd);
	    memset(cmd, 0, sizeof(cmd));
	    snprintf(cmd, sizeof(cmd), "%s --psat_char_result", _TCMD_PATH);
	    //printf("%s\n", cmd);
	    system(cmd);
	    sleep(1);
    }

#ifdef ANDROID
    system("rmmod ath6kl_sdio");
    sleep(1);
    system("rmmod cfg80211");
#else
    system("rmmod ath6kl_sdio.ko");
#endif

    return 0;
}
