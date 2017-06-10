/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __TEST_DBG_H__
#define __TEST_DBG_H__

#include "gpsone_daemon_dbg.h"

#define TEST_DBG(fmt, args...) FPRINTF(stderr, fmt, ##args)

#endif /* __TEST_DBG_H__ */
