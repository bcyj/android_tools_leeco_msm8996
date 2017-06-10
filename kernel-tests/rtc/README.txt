/******************************************************************************
 *   @file README.txt
 *   @brief Real Time Clock Driver Test Description
 *
 *   DESCRIPTION
 *         Notes to describe the Real Time Clock Driver Test Program
 *
 *-----------------------------------------------------------------------------
 *   Copyright (C) 2008,2011 Qualcomm Technologies, Inc.
 *   All Rights Reserved. Qualcomm Technologies Proprietary and Confidential
 *-----------------------------------------------------------------------------
 *******************************************************************************/

  Usage:
             ./rtc_test <Operations>

       Set and/or get RTC Time and alarm time. Setting an alarm will
       trigger an interrupt in case of (8660/8960) target. This is
       primarily used for waking the system from suspend state.

  Mandatory arguments to long options are mandatory for short options too.
       -r, --read_time                      Reads the RTC time
       -s, --set_time mm/dd/yyyy hh:mm:ss   Sets the RTC time.
       -a, --alarm mm/dd/yyyy hh:mm:ss      Sets the Alarm time.
       -d, --device <Path to RTC Device>    RTC device to be tested
                                            (Default : /dev/rtc0)
       -u, --auto			    Perform all rtc operations
					    (Read/Write/Alarm) based on target.
       -h, --help                           This message will be displayed

  Example:
       ./rtc_test --set_time 04/05/1980 09:30:25 --read_time --device /dev/rtc0
       ./rtc_test -s 04/05/1980 09:30:25 -r -d /dev/rtc0
       ./rtc_test -s 04/05/1980 09:30:25 -r -a 04/05/1980 09:31:00 -d /dev/rtc0
       ./rtc_test -u

  Target Support: 7x00, 7x27, 8x50, 7x30, 8660, 8960

  Note:
       (1)For more info on the capabilities of the RTC driver type:

             cat /proc/driver/rtc

          in the command shell

       (2)Limitation on 8660/8960 (Android): The ioctl RTC_SET_TIME is not supported by the RTC
          driver. So, test utility returns success for RTC_SET_TIME without performing the actual
	  write operation. Only operations of read time and set the alarm time work.

       (3)RTC_SET_TIME is supported on all the targets (7x00, 7x27, 8x50, 7x30) prior to 8660.
