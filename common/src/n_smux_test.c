/******************************************************************************
 * @file  n_smux_test.c
 * @brief
 *
 * User-space application to set the line discipline of a tty to n_smux
 *
 * -----------------------------------------------------------------------------
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * -----------------------------------------------------------------------------
 ******************************************************************************/

/*
 * Based upon code from:
 * vendor/qcom/proprietary/kernel-tests/msm_uart/msm_uart_test.c
 * Protocol documentaion: Documentation/arm/msm/serial_mux.txt
 */

#include <sys/ioctl.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>


#define N_SMUX 25
static const int ldisc = N_SMUX;

/* B200 is remapped in the HS UART as 3.2Mbps */
#define B3355443 B200
int main(int argc, char **argv)
{
	int fd;
	struct termios options;
	const char *dev_name;
	int tmp;

	if (argc <= 1) {
		printf("Usage: %s TTY_Device\n", argv[0]);
		return -1;
	}

	dev_name = argv[1];
	fd = open(dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		printf("Cannot open fd '%s'\n", dev_name);
		return -1;
	}

	/* Configure Baud Rate */
	printf("Configuring UART\n");
	ioctl(fd, TCGETS, &options);
	fcntl(fd, F_SETFL, 0);
	ioctl(fd, TCGETS, &options);
	options.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	options.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */

	options.c_cflag &= ~CSIZE;
	options.c_cflag |= (CS8 | CLOCAL | CREAD | CRTSCTS);
	options.c_iflag = IGNPAR;
        options.c_oflag = 0;
	options.c_lflag = 0;

	printf("Baud Rate = 3.2Mbps\n");
	cfsetospeed(&options, B3355443);
	cfsetispeed(&options, B3355443);
	ioctl(fd, TCSETS, &options);

	/* Load SMUX as Line discipline */
	printf("Loading SMUX as line discipline for '%s'\n", dev_name);
	tmp = ioctl(fd, TIOCSETD, &ldisc);
	if (tmp) {
		perror("Line Discipline IOCTL failed: ");
		return -1;
	}

	/* Enter daemon mode to keep the line discipline loaded */
	daemon(0,0);
	pause();

	return 0;
}
