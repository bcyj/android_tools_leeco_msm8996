/******************************************************************************
 * @file  msm_uart_test.c
 * @brief
 *
 * User-space unit test application for the msm uart driver.
 *
 * -----------------------------------------------------------------------------
 * Copyright (c) 2008, 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * -----------------------------------------------------------------------------
 ******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>

#define NUM_CMDS 4
#define NOMINAL_T 0
#define ECHO_T 1
#define LOOPBACK_T 2
#define HELP 3

enum device_name {
      DEVICE_HSL,
      DEVICE_HS,
      DEVICE_MSM,
};

static int cmd_switch[NUM_CMDS];
static int echo_flag = 1;
char *port = NULL;
speed_t speed;
int transfer_count=4096;
enum device_name devname;
int hw_flow_ctrl;

void sig_handler_exit (int status)
{
	printf("Stopping \n");
	echo_flag = 0;
}

void usage(void)
{
  printf("Usage: msm_uart_test [-n] [-e] [-l] [-d  <device_name>] [-b <baud_rate>] [-s <data size (in bytes)>] [-f <hw_flow_ctrl(0 or 1)>]\n");
  printf("-n Nominal Test\n");
  printf("-e Echo Test\n");
  printf("-l Loopback Test\n");
  printf("Example usage:msm_uart_test -l -d /dev/ttyHS0 -b 115200 -s 400 -f 1\n");
}

void baud_mapping(int baud)
{
 if(devname == DEVICE_HSL){
     if(baud <300 ||  baud >460800){
                    printf("Not supported baud rate\n");
                    exit(-1);
      }
 }
 else if(devname == DEVICE_HS){
      if(baud <300 || baud >4000000){
                    printf("Not supported baud rate\n");
                    exit(-1);
      }
 }
 else if(devname == DEVICE_MSM){
      if(baud <300 || baud >115200){
                    printf("Not supported baud rate\n");
                    exit(-1);
      }
 }

 switch (baud){
   case 300:
           speed=B300;
           break;
   case 600:
           speed=B600;
           break;
   case 1200:
           speed=B1200;
           break;
   case 2400:
           speed=B2400;
           break;
   case 4800:
           speed=B4800;
           break;
   case 9600:
           speed=B9600;
           break;
   case 19200:
           speed=B19200;
           break;
   case 38400:
           speed=B38400;
           break;
   case 57600:
           speed=B57600;
           break;
   case 115200:
           speed=B115200;
           break;
   case 230400:
           speed=B230400;
           break;
   case 460800:
           speed=B460800;
           break;
   case 921600:
           speed=B921600;
           break;
   case 1000000:
           speed=B1000000;
           break;
   case 1152000:
           speed=B1152000;
           break;
   case 1500000:
           speed=B1500000;
           break;
   case 2500000:
           speed=B2500000;
           break;
   case 3000000:
           speed=B3000000;
           break;
   case 3200000:
           speed=B200;
           break;
   case 3500000:
           speed=B3500000;
           break;
   case 4000000:
           speed=B4000000;
           break;
   default:
           printf("Invalid baud rate entered/n");
           exit(-1);
   }
}

void validate_device(char *argv){
 if (strncmp(argv,"/dev/ttyHSL",11)==0){
	    port = argv;
            devname = DEVICE_HSL;
	    printf("device name=%s\n", port);
       }
       else if (strncmp(argv,"/dev/ttyHS",10)==0){
	    port = argv;
            devname = DEVICE_HS;
	    printf("device name=%s\n", port);
       }
       else if (strncmp(argv,"/dev/ttyMSM",11)==0){
	    port = argv;
            devname = DEVICE_MSM;
	    printf("device name=%s\n", port);
       }
       else{
           printf("Unknown device name\n");
           usage();
           exit(-1);
       }
}

void parse_options(int argc, char**argv){
  int i;
  int br;
  int size;
  for (i = 0; i < NUM_CMDS; i++){
    cmd_switch[i] = 0;
  }

  for (i = 1; i < argc; i++){
    if (strcmp(argv[i], "-n") == 0)
      cmd_switch[NOMINAL_T] = 1;
    else if (strcmp(argv[i], "-e") == 0)
      cmd_switch[ECHO_T] = 1;
    else if (strcmp(argv[i], "-l") == 0)
      cmd_switch[LOOPBACK_T] = 1;
    else if (strcmp(argv[i], "-h") == 0)
      cmd_switch[HELP] = 1;
    else if (strcmp(argv[i], "-d") == 0){
      validate_device(argv[++i]);
    }
    else if (strcmp(argv[i], "-f") == 0){
           hw_flow_ctrl = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-b") == 0){
           br = atoi(argv[++i]);
           baud_mapping(br);
    }
    else if (strcmp(argv[i], "-s") == 0){
           size = atoi(argv[++i]);
           transfer_count = size;
    }
    else{
      printf("Unknown command argument\n");
      usage();
      exit(-1);
    }
  }

  if (!(cmd_switch[NOMINAL_T] | cmd_switch[ECHO_T] | cmd_switch[LOOPBACK_T]))
    cmd_switch[NOMINAL_T] = 1;
}

static int echo_test(char * portid) {
	int file_ptr;
	int rnum;
	int wnum;
	struct sigaction echo_sig;

	struct termios options;
	static char buf[1024];

	echo_flag = 1;

	echo_sig.sa_handler = sig_handler_exit;
	echo_sig.sa_restorer = NULL;
	sigaction(SIGINT,&echo_sig,NULL);

	
	file_ptr = open(portid, O_RDWR | O_NOCTTY | O_NDELAY);
	if (-1 == file_ptr) {
		printf("Could not open uart port \n");
		return(-1);
	}
	if (tcflush(file_ptr, TCIOFLUSH) <0) {
		printf("Could not flush uart port");
		return(-1);
	}


	if (tcgetattr(file_ptr, &options) < 0) {
		printf("Can't get port settings \n");
		return(-1);
	}

	options.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	options.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

	options.c_cflag &= ~CSIZE;
	options.c_cflag |= (CS8 | CLOCAL | CREAD | CRTSCTS);
	options.c_iflag = IGNPAR;
        options.c_oflag = 0;
	options.c_lflag = 0;
	cfsetospeed(&options, speed);
	cfsetispeed(&options, speed);
	if (tcsetattr(file_ptr, TCSANOW, &options) < 0) {
		printf("Can't set port setting \n");
		return(-1);
	}
	/* Blocking behavior */
	fcntl(file_ptr, F_SETFL, 0);

	printf("Press Control-C to exit  \n");
	while (echo_flag == 1) {
		rnum = read(file_ptr, buf, 512);
		if (rnum > 0) {
			wnum = write(file_ptr, buf, rnum);
			if (wnum < 0) {
				printf("Write Failed. Error=%d\n", wnum);
				close(file_ptr);
				return wnum;
			}
		}
	}

	close(file_ptr);
	return(0);
}
static int simple_test(char * portid) { 
	int file_ptr;
	int wnum;
        int i;
	struct termios c_opt;

	char *buf = NULL;
	buf = (char *)malloc(transfer_count*sizeof(char));
	if (buf == NULL) {
		printf("Could not allocate buffer using malloc\n");
		return -1;
	}

	for (i=0; i<transfer_count; i++) {
		buf[i]='a' + (i%26);
	}

	file_ptr = open(portid, O_RDWR | O_NOCTTY | O_NDELAY);
	if (-1 == file_ptr) {
		printf("Could not open uart port \n");
		return(-1);
	}
	if (tcflush(file_ptr, TCIOFLUSH) <0) {
		printf("Could not flush uart port");
		return(-1);
	}

	/* Blocking behavior */
	fcntl(file_ptr, F_SETFL, 0);

	ioctl(file_ptr, TCGETS, &c_opt);

	c_opt.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	c_opt.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

	c_opt.c_cflag &= ~CSIZE;
        c_opt.c_cflag |= (CS8 | CLOCAL | CREAD);
	if (hw_flow_ctrl)
		c_opt.c_cflag |= CRTSCTS;
	c_opt.c_iflag = IGNPAR;
        c_opt.c_oflag = 0;
	c_opt.c_lflag = 0;	

	cfsetospeed(&c_opt, speed);
	cfsetispeed(&c_opt, speed);
	ioctl(file_ptr, TCSETS, &c_opt);

	wnum = write(file_ptr, buf, transfer_count);
	if (wnum < 0) {
		printf("Write Failed. Error=%d\n", wnum);
		close(file_ptr);
		free(buf);
		return wnum;
	}

	close(file_ptr);
	free(buf);
	return(0);
}


void * write_data(void * arg) {

	int i;
	int wnum;
	unsigned long file_ptr = (unsigned long) arg;
	char *buf = NULL;
	buf = (char *)malloc(transfer_count*sizeof(char));
	if (buf == NULL) {
		printf("Could not allocate buffer using malloc\n");
		return NULL;
        }

	for(i=0; i<transfer_count; i++) {
		buf[i] = 'a' + (i % 26);
	}
	wnum = write(file_ptr, buf, transfer_count);
	if (wnum < 0)
		printf("Write Failed. Error=%d\n", wnum);

	free(buf);
	return (NULL);

}


int loopback_test(char * portid) { 
	unsigned long file_ptr;
	int rnum;
	int tnum;
	int i;
	int timeout;
	int err;
	struct termios options;
	pthread_t thread;


	char *buf_read = NULL;
	buf_read = (char *)malloc(transfer_count*sizeof(char));
	if (buf_read == NULL) {
		printf("Could not allocate buffer using malloc\n");
		return -1;
	}

	file_ptr = open(portid, O_RDWR | O_NOCTTY | O_NDELAY);
	if (-1 == file_ptr) {
		printf("Could not open uart port \n");
		return(-1);
	}

	/* Blocking behavior */
	fcntl(file_ptr, F_SETFL, 0);
	ioctl(file_ptr, TCGETS, &options);
	options.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	options.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */

	options.c_cflag &= ~CSIZE;
	options.c_cflag |= (CS8 | CLOCAL | CREAD);
	if (hw_flow_ctrl)
		options.c_cflag|= CRTSCTS;
	options.c_iflag = IGNPAR;
        options.c_oflag = 0;
	options.c_lflag = 0;

	cfsetospeed(&options, speed);
	cfsetispeed(&options, speed);
	ioctl(file_ptr, TCSETS, &options);


	printf("Loopback connection required! \n");

	err = pthread_create (&thread, NULL, write_data, (void *) file_ptr);
	if (err) {
		printf("Fail to create write thread \n");
		free(buf_read);
		return(-1);
	}


	rnum = 0;
	tnum = 0;
	timeout = 0;
	
	while((rnum <transfer_count) && (timeout < 10000) ) {

		tnum = read(file_ptr, &buf_read[rnum], transfer_count);

		if(tnum >= 0) {
                        rnum += tnum;
		} else {
			timeout++;
		}
	}

	if(timeout >= 1000) {
			printf("FAIL! \n");
			free(buf_read);
			return(1);
	}
	
	for(i=0; i<transfer_count; i++) {
		if (buf_read[i] != ('a' + (i % 26))) {
			printf("FAIL %c! \n", buf_read[i]);
			free(buf_read);
			return(1);
		}
	}

	pthread_join (thread, NULL);
	close(file_ptr);
	free(buf_read);
	return(0);
}



int main(int argc, char **argv)
{
  int result = 0;
  parse_options(argc, argv);

  if (cmd_switch[HELP]){
    usage();
    exit(0);
  }

 if (cmd_switch[NOMINAL_T]){
    printf("running nominal test with port %s \n", port);
    result |= simple_test(port);
  }
  if (cmd_switch[LOOPBACK_T]){
    printf("running loopback test with port %s \n", port);
    result |= loopback_test(port);
  }
  if (cmd_switch[ECHO_T]){
    printf("running echo test with port %s \n", port);
    result |= echo_test(port);
  }
  if (result == 0)
    printf("Test Passed\n");
  else
    printf("Test Failed\n");

  return result;
}



