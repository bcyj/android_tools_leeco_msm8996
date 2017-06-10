/*******************************************************************************
 * -----------------------------------------------------------------------------
 * Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * -----------------------------------------------------------------------------
 ******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/rtc.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "header.h"

#define NETMGR_KIF_SYSFILE_OPEN_TIMEOUT "/data/data_test/modem_port_timeout"
#define NETMGR_KIF_SYSFILE_OPEN_STATUS  "/data/data_test/modem_port_status"

char *g_sys_pm = NULL;

int main(int argc, char *argv[])
{

  int modem_timeout, modem_timeout1, modem_status;

	fprintf(stdout, "Check whether power management is up\n");
	if (!file_exists(PM_STATS_NODE)) {
		fprintf(stdout, "power management is not available\n");
		goto modem_test_bail;
	}

	fprintf(stdout, "Determine the power management module\n");
	if (directory_exists(SYS_PM))
		g_sys_pm = SYS_PM;
	else if (directory_exists(SYS_PM2))
		g_sys_pm = SYS_PM2;
	else if (directory_exists(SYS_PM_8x60))
		g_sys_pm = SYS_PM_8x60;
	else if (directory_exists(SYS_PM_8660))
		g_sys_pm = SYS_PM_8660;
	else {
		fprintf(stdout, "power management is not available\n");
		goto modem_test_bail;
	}


	/*
	   first check if the modem files are present or not
	*/
	if (!file_exists(NETMGR_KIF_SYSFILE_OPEN_TIMEOUT) &&
	    !file_exists(NETMGR_KIF_SYSFILE_OPEN_STATUS)) {
		fprintf(stdout, "This is an apq device\n");
		return EXIT_SUCCESS;
	}

	/*
	  determine if the modem ports have been successfully opened by apps
	   or wait for timeout
	*/
	fprintf(stdout, "Check modem port\n");
	modem_timeout = read_unsigned_int_from_file( NULL, NETMGR_KIF_SYSFILE_OPEN_TIMEOUT);
	if (modem_timeout < 0) {
		fprintf(stdout,	"cannot read %s: %s\n",
			NETMGR_KIF_SYSFILE_OPEN_TIMEOUT, strerror(-modem_timeout));
		goto modem_test_bail;
	}

	modem_timeout1 = modem_timeout;

	modem_status = read_unsigned_int_from_file( NULL, NETMGR_KIF_SYSFILE_OPEN_STATUS);
	if (modem_status < 0) {
	  fprintf(stdout,	"cannot read %s: %s\n",
		  NETMGR_KIF_SYSFILE_OPEN_TIMEOUT, strerror(-modem_status));
	}

	while ((modem_timeout > 0) && (modem_status <= 0)) {
		modem_timeout -= sleep(3);
		modem_status = read_unsigned_int_from_file(NULL, NETMGR_KIF_SYSFILE_OPEN_STATUS);
		if (modem_status < 0) {
			fprintf(stdout,	"cannot read %s: %s\n",
				NETMGR_KIF_SYSFILE_OPEN_STATUS, strerror(-modem_status));
		}
	}

	if (modem_status <= 0) {
		fprintf(stdout, "the modem is not up even after max timeout of %d secs \n", modem_timeout1);
		fprintf(stdout, "\n Test failed\n");
		goto modem_test_bail;
	} else {
		fprintf(stdout, "\n Test passed\n");
		return EXIT_SUCCESS;
	}


modem_test_bail:

	return EXIT_FAILURE;

}
