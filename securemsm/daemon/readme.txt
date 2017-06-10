/*===========================================================================

Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

QSEECOM Daemon
~~~~~~~~~~~~~~

The  purpose of this daemon is to enable applications running on
secure domain to access services on HLOS, services that are not
available on secure domain (trust zone).
The daemon starts service threads to listens for any incoming
requests from a secure application running on secure domain.
Currently two listener services are initiated

- FILE_SERVICE (ID:10)
	listens for file service request like file_open, file_close,
	file_creat, etc.
- TIME_SERVICE (ID:11)
	listens for timer service request like get time  in usec,
	get time in milisec, etc.


Any new service can be added to the existing ones by adding it
to the following 'listeners" array in the format shown below:

static struct qseecom_listener_services listeners[] = {
	{
		.service_name = "<SERVICE_NAME>",
		.id = <SERVICE_ID#>,
		.file_name = "<.so service library to load>",
		.file_start = "<service_start API>",
		.file_stop = "<service_close API>",
		.qhandle = NULL,
	},
};


The daemon is configured to start up after the HLOS (system/kernel) is up.
This is set in the init.target.rc file as shown below:

service qseecomd /system/bin/qseecomd
   class late_start
   user system
   group system

The above configuration/permission/privileges can be changed as needed.