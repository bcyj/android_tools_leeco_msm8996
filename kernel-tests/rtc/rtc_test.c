/******************************************************************************
  @file  rtc_test.c
  @brief Real Time Clock Driver Test Program 

  DESCRIPTION
  Real Time Clock Driver Test Program

  ----------------------------------------------------------------------------
  Copyright (C) 2008,2011 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential
  ----------------------------------------------------------------------------
******************************************************************************/

/* ------------------------------------------------------------------------
 *
 *                 INCLUDES
 *
 * ------------------------------------------------------------------------ */

#include <stdio.h>
#include <getopt.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "stringl.h"


/* ------------------------------------------------------------------------
 *
 *                 DEFINITIONS
 *
 * ------------------------------------------------------------------------ */


/*
 * Masks for which tests to run
 */

#define SET_TIME  0x1
#define READ_TIME 0x2
#define SET_ALARM 0x4
#define TEST_ALL  0x8

/*
 * RTC device path to use by default
 */
#define DEFAULT_RTC_DEV_PATH "/dev/rtc0"

/* RTC device path to use */
static char                   dev_path[50]      = DEFAULT_RTC_DEV_PATH;

/* tests to run */
static int                    op                = 0;

/* File Descriptor for RTC device */
static int                    fd                = 0;

/* String to process date/time input */
static char                   datetime_str[100];

/* Struct to hold time to be written to RTC */
static struct rtc_time        rtc_test_wr_tm;

/* Struct to hold time to be read from RTC */
static struct rtc_time        rtc_test_rd_tm;

/* Struct to hold alarm time to be written to RTC */
static struct rtc_wkalrm      rtc_test_alrm;


/* Debug messages flag - Commented out by default */
//#define MSM_RTC_DEBUG

#ifdef MSM_RTC_DEBUG
	#define MSM_RTC_DBG(msg, args...) do { \
      printf( "[%s] " msg , __func__ , ##args ); \
   } while (0)
#else /* !def MSM_RTC_DEBUG */
	#define MSM_RTC_DBG(msg, args...) do {} while (0)
#endif

/* ------------------------------------------------------------------------
 *
 *                 FUNCTIONS
 *
 * ------------------------------------------------------------------------ */

/**
 * set_time
 *
 * @brief Sets time through the RTC driver via the RTC_SET_TIME ioctl
 * 
 * Sets time through the RTC driver via the RTC_SET_TIME ioctl
 *
 * @param rtc_tm   rtc_time structure to be passed to the RTC driver
 *
 * @return           None
 *
 */

static void set_time(struct rtc_time *rtc_tm)
{
	int retval;

	retval = ioctl(fd,RTC_SET_TIME,rtc_tm);
	if(retval != 0) {
		perror("RTC_SET_TIME ioctl");
		exit(errno);
	}

}

/**
 * set_alarm
 *
 * @brief Sets the alarm through the RTC driver via the RTC_WKALM_SET ioctl
 * 
 * Sets time through the RTC driver via the RTC_WKALM_SET ioctl
 *
 * @param rtc_tm   rtc_wkalrm structure to be passed to the RTC driver
 *
 * @return           None
 *
 */

static void set_alarm(struct rtc_wkalrm *rtc_alarm)
{
	int retval;

	retval = ioctl(fd,RTC_WKALM_SET,rtc_alarm);
	if(retval != 0) {
		perror("RTC_WKALM_SET ioctl");
		exit(errno);
	}

}

/**
 * read_alarm
 *
 * @brief Reads the alarm through the RTC driver via the RTC_WKALM_RD ioctl
 *
 * Reads time through the RTC driver via the RTC_WKALM_RD ioctl
 *
 * @param rtc_tm   rtc_wkalrm structure to be passed to the RTC driver
 *
 * @return           None
 *
 */

static void read_alarm(struct rtc_wkalrm *rtc_alarm)
{
	int retval;

	retval = ioctl(fd,RTC_WKALM_RD,rtc_alarm);
	if(retval != 0) {
		perror("RTC_WKALM_RD ioctl");
		exit(errno);
	}

}

/**
 * read_time
 *
 * @brief Reads time from the RTC driver via the RTC_RD_TIME ioctl
 *
 * Reads time from the RTC driver via the RTC_RD_TIME ioctl
 *
 * @param   rtc_tm - rtc_time structure to be obtained from RTC driver 
 *
 * @return       None
 *
 */

static void read_time(struct rtc_time *rtc_tm)
{

	int retval;

	/* Read the RTC time/date  */
	retval = ioctl(fd, RTC_RD_TIME, rtc_tm);
	if(retval != 0) {
		perror("RTC_RD_TIME ioctl");
		exit(errno);
	}

}

/**
 * rtc_test_str_to_time
 *
 * @brief Converts a datetime string to rtc_time format
 * 
 * Converts a datetime string (in the mm/dd/yy hh:mm:ss format)
 * to rtc_time format
 *
 * @param datetime_str_ptr  Pointer to the datetime string
 * @param rtc_tm            rtc_time equivalent of datetime_str_ptr
 *
 * @return           None
 *
 */

void rtc_test_str_to_time(char *datetime_str_ptr, struct rtc_time *rtc_tm)
{
	MSM_RTC_DBG("Time in set_time : %s\r\n",datetime_str_ptr);    
	sscanf(datetime_str_ptr,"%d/%d/%d %d:%d:%d",&rtc_tm->tm_mon, 
	       &rtc_tm->tm_mday,
	       &rtc_tm->tm_year,
	       &rtc_tm->tm_hour,
	       &rtc_tm->tm_min,
	       &rtc_tm->tm_sec);


// Adjust the month to start from 0 and the year relative to 1900.
	rtc_tm->tm_mon = rtc_tm->tm_mon - 1;
	rtc_tm->tm_year = rtc_tm->tm_year - 1900;
}

/**
  * auto_test
  *
  * @brief	Perform all RTC operations read/write/alarm.
  *
  * @return	None
  *
  */

static void auto_test(void)
{
	struct rtc_wkalrm  rtc_alarm;

#ifndef DISABLE_WRITE_TEST
	/* Read time */
	read_time(&rtc_alarm.time);
	/* write to RTC */
	rtc_alarm.time.tm_sec += 3;
	set_time(&rtc_alarm.time);
#endif
	read_time(&rtc_alarm.time);
	rtc_alarm.time.tm_sec += 5;
	rtc_alarm.enabled = 1;
	set_alarm(&rtc_alarm);

#ifndef DISABLE_ALARM_READ
	/* Wait for alarm expiration */
	fprintf(stderr,"\nSleeping for 6 seconds for alarm expiration\n");
	sleep(6);
	read_alarm(&rtc_alarm);
	if (rtc_alarm.enabled != 0){
		printf("\n Alarm expiration test failed \n");
		exit (-1);
	}
#endif
}


/**
 * usage
 *
 * @brief Displays the usage info for this app
 *
 * Displays the usage info for this app
 *
 * @param exename  Name of the app as given by argv[0]
 *
 * @return       None
 *
 */

void usage(char *exename)
{
	printf("Usage: %s <Operations>\r\n" \
	       "     Set and/or get RTC Time and alarm time. Setting an alarm will \r\n" \
	       "     trigger an IRQ on the ARM9 side when the alarm expires. This is \r\n" \
	       "     primarily used to power on the system if turned off. \r\n"\
	       " \r\n" \
	       "Mandatory arguments to long options are mandatory for short options too.\r\n" \
	       "   -r, --read_time                      Reads the RTC time\r\n" \
	       "   -s, --set_time mm/dd/yyyy hh:mm:ss   Sets the RTC time. \r\n" \
	       "   -a, --alarm mm/dd/yyyy hh:mm:ss      Sets the Alarm time. \r\n" \
	       "   -u, --auto				Perform all RTC"
               "                                        opearions write/read/alarm \r\n" \

	       "   -d, --device <Path to RTC Device>    RTC device to be tested\r\n" \
	       "                                        (Default : "DEFAULT_RTC_DEV_PATH")\r\n" \
	       "   -h, --help                           This message will be displayed\r\n" \
	       "	\r\n" \
	       "Example:\r\n" \
	       "   %s --set_time 04/05/1980 09:30:25 --read_time --device /dev/rtc0\r\n" \
	       "   %s -s 04/05/1980 09:30:25 -r -d /dev/rtc0\r\n"\
	       "   %s -s 04/05/1980 09:30:25 -r -a 04/05/1980 09:31:00 -d /dev/rtc0\r\n",
	       exename,exename,exename,exename);
	exit(1);              
}

/**
 * parse_args
 *
 * @brief Parse args and identify the operations to be performed
 *
 * Parse args and identify the operations to be performed
 *
 * @param argc   Number of arguments present in the argv  
 *
 * @param argv   Arguments send from the command line
 *
 * @return       None
 *
 */

void parse_args (int argc, char **argv)
{
	int command;

	struct option longopts[] = 
	{
		{ "device",       1, NULL, 'd'},
		{ "read_time",    0, NULL, 'r'},
		{ "set_time",     1, NULL, 's'},
		{ "alarm",        1, NULL, 'a'},
		{ "auto",         0, NULL, 'u'},
		{ "help",         0, NULL, 'h'},
		{ NULL,           0, NULL,  0 },
	};

	while(
	     (command = getopt_long( argc, argv, "d:rus:a:h", longopts, NULL))
	     != -1) {
		switch( command ) {
			case 'd':
				strlcpy(dev_path, optarg,sizeof(dev_path));
				MSM_RTC_DBG("dev_path : %s\r\n",dev_path);
				break;

			case 'r':
				op |= READ_TIME;
				break;

			case 's':
				strlcpy(datetime_str,optarg,
					sizeof(datetime_str));
				if(optind >= argc || argv[optind][0] == '-') {
					MSM_RTC_DBG("[Set Time] Only date was "\
						    " provided. Defaulting time"\
						    " to 00:00:00 hrs\r\n");
					strlcat(datetime_str," 00:00:00",
						sizeof(datetime_str));
				} else {
					strlcat(datetime_str," ",
						sizeof(datetime_str));
					strlcat(datetime_str,argv[optind],
						sizeof(datetime_str));
				}
				MSM_RTC_DBG("time string : %s\r\n",datetime_str);
				rtc_test_str_to_time(datetime_str,
						     &rtc_test_wr_tm);
				op |= SET_TIME;
				break;
			case 'a':
				{
					strlcpy(datetime_str,optarg,
						sizeof(datetime_str));
					if(optind >= argc || 
					   argv[optind][0] == '-') {
						MSM_RTC_DBG("[Alarm] Only date "\
							    "was provided. "\
							    "Defaulting time to"\
							    " 00:00:00 hrs\r\n");
						strlcat(datetime_str,
							" 00:00:00",
							sizeof(datetime_str));
					} else {
						strlcat(datetime_str," ",
							sizeof(datetime_str));
						strlcat(datetime_str,
							argv[optind],
							sizeof(datetime_str));
					}
					MSM_RTC_DBG("time string : %s\r\n",
						    datetime_str);
					rtc_test_str_to_time(datetime_str,
							     &rtc_test_alrm.time);
					op |= SET_ALARM;
					break;

				}

			case 'u':
				op |= TEST_ALL;
				break;

			case 'h':
				usage(argv[0]);
				break;

			default:
				fprintf(stderr, "Invalid argument: %c\n", command);
				usage(argv[0]);
		}
	}
	if(op == 0) {
		usage(argv[0]);
	}
} /* parse_args */

/**
 * main
 *
 * @brief Entry point of this test program
 * 
 * Entry point of this test program. Conducts the tests indicated by the user
 * as command line arguments
 *
 * @param argc   Number of arguments present in the argv  
 *
 * @param argv   Arguments send from the command line
 *
 * @return           None
 *
 */


int main(int argc, char **argv)
{
	fprintf(stderr, "\n\t\t\tRTC Driver Test Example.\n\n");

	/*
	 * Parse command line arguments
	 */

	parse_args(argc, argv); 


	/* 
	 * Open the device
	 */

	fd = open(dev_path, O_RDONLY);

	if(fd ==  -1) {
		perror(dev_path);
		exit(errno);
	}

	/* 
	 * Perform the requested operations
	 */

	if(op & SET_TIME) {

#ifdef DISABLE_WRITE_TEST
		fprintf(stderr, "\nWrite operations disabled for this"
				" target: Test PASSED\n");
#else
		fprintf(stderr, "Setting Time to   : %02d/%02d/%04d %02d:%02d:%02d\r\n",
			rtc_test_wr_tm.tm_mon + 1, rtc_test_wr_tm.tm_mday, 
			rtc_test_wr_tm.tm_year + 1900 , rtc_test_wr_tm.tm_hour,
			rtc_test_wr_tm.tm_min, rtc_test_wr_tm.tm_sec);

		set_time(&rtc_test_wr_tm);      
		//Sleep for 2 seconds ..If we are going to read..it would be
		// nice to show some difference!
		if(op & READ_TIME) {
			fprintf(stderr,"Sleeping for 2 seconds..to show liveliness of RTC\r\n");
			sleep(2);
		}
#endif
	}
	if(op & READ_TIME) {
		read_time(&rtc_test_rd_tm);
		fprintf(stderr, "Reading Time back : %02d/%02d/%04d %02d:%02d:%02d\r\n",
			rtc_test_rd_tm.tm_mon + 1, rtc_test_rd_tm.tm_mday, 
			rtc_test_rd_tm.tm_year + 1900, rtc_test_rd_tm.tm_hour, 
			rtc_test_rd_tm.tm_min, rtc_test_rd_tm.tm_sec);
	}
	if(op & SET_ALARM) {
		fprintf(stderr, "Setting Alarm to  : %02d/%02d/%04d %02d:%02d:%02d\r\n",
			rtc_test_alrm.time.tm_mon + 1, rtc_test_alrm.time.tm_mday, 
			rtc_test_alrm.time.tm_year + 1900 , rtc_test_alrm.time.tm_hour,
			rtc_test_alrm.time.tm_min, rtc_test_alrm.time.tm_sec);
		rtc_test_alrm.enabled = 1;
		set_alarm(&rtc_test_alrm);
	}
	if(op & TEST_ALL) {
		fprintf(stderr, "\nPerforming all RTC operations "
				"(READ/WRITE/ALARM) \r\n");
		auto_test();
	}
	fprintf(stderr, "\n\n\t\t\t *** Test completed successfully ***\n");

	/* 
	 * Close the device
	 */

	close(fd);

	return 0;
}
