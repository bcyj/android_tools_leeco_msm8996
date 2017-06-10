/*
* Copyright (c) 2011-2012 Qualcomm Atheros Inc. All Rights Reserved.
* Qualcomm Atheros Proprietary and Confidential.
*/

#ifndef _TCMD_H_
#define _TCMD_H_

#ifdef __cplusplus
extern "C" {
#endif


enum {
	TESTMODE_CONT_TX = 801,	/* something that doesn't collide with ascii */
	TESTMODE_CONT_RX,
	TESTMODE_PM,
	TESTMODE_SETLPREAMBLE,
	TESTMODE_SETREG,
	TESTMODE_CMDS,
};

enum {
	TCMD_SET_RX_LPL = 501, /* something that doesn't collide with ascii */
	TCMD_EFUSE_START = 502,
	TCMD_EFUSE_END = 503,
	TCMD_EFUSE_DATA = 504,
	TCMD_EFUSE_DUMP = 505,
	TCMD_EFUSE_WRITE = 506,
	TCMD_OTP_DUMP = 507,
	TCMD_OTP_WRITE = 508,
	TCMD_READ_THERMAL = 509,
#ifdef CONFIG_AR6002_REV6
	TCMD_SET_BT_MODE = 510,
#else
	TCMD_PSAT_CAL = 510,
	TCMD_PSAT_CAL_RESULT,
	TCMD_PM_CAL_RESULT,
	TCMD_PM_CAL,
	TCMD_UPDATE_CAL_OTP,
	TCMD_CHAR_PSAT,
	TCMD_CHAR_PSAT_RESULT,
	TCMD_SINIT_WAIT,
#endif
};

#ifdef __cplusplus
}
#endif
#endif				/* _TCMD_H_ */
