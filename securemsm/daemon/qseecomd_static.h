/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        HLOS Static QSEECom Daemon functions

GENERAL DESCRIPTION
	Contains the listener library functions used by static qseecom daemon

EXTERNALIZED FUNCTIONS
	None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef __QSEECOMD_STATIC_H_
#define __QSEECOMD_STATIC_H_

/*--------------------------------------------------------------------------
 * Include Files
* -------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

int rpmb_init_service();
int ssd_init_service();
int rpmb_start(int index);
int ssd_start(int index);

#endif
