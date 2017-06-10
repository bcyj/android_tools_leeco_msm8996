/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2011, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 */
#ifndef TIME_GENOFF_I_H
#define TIME_GENOFF_I_H

#define OFFSET_LOCATION "/data/time"
#define MAX_CB_FUNC              10
#define TIME_GENOFF_REMOTE_UPDATE_MS  10
#define TIME_GENOFF_UPDATE_THRESHOLD_MS 10
#define GENOFF_SOCKET_NAME	"#time_genoff"
#define GENOFF_MAX_CONCURRENT_CONN	4

#define LOG_TAG "QC-time-services"
#define TIME_LOGE(...) { ALOGE(__VA_ARGS__); }
#define TIME_LOGD(...) { ALOGD(__VA_ARGS__); }
#define TIME_LOGV(...) { ALOGV(__VA_ARGS__); }

#include <cutils/log.h>

#define SEC_TO_MSEC(s)	((s) * 1000ULL)
#define MSEC_TO_SEC(s)	((s) / 1000ULL)
#define USEC_TO_MSEC(s)	((s) / 1000ULL)
#define NSEC_TO_MSEC(s)	((s) / 1000000ULL)
#define	MODEM_EPOCH_DIFFERENCE	 315964800
#define ATS_MAX		ATS_MFLO + 1
#define FILE_NAME_MAX	100
#define RTC_MIN_VALUE_TO_RESET_OFFSET 60000

typedef enum time_persistant_operation {
	TIME_READ_MEMORY,
	TIME_WRITE_MEMORY
}time_persistant_opr_type;

/* Time Generic Offset 'type 1' callback function */
typedef int (*time_genoff_t1_cb_type)();

typedef struct {
	/* Mark as TRUE if persistent storage is needed */
	uint8_t initialized;
	/* file name for efs item file */
	char f_name[FILE_NAME_MAX + 1];
	/* Threshold in ms for writing back in efs item file */
	int64_t threshold;
} time_genoff_per_storage_type;

/* Time Generic Offset pointer type */
typedef struct time_genoff_struct *time_genoff_ptr;

typedef struct time_genoff_struct{
	/* Generic Offset, always stored in ms */
	int64_t generic_offset;
	/* Flag to indicate if time-of-day has ever been set before */
	uint8_t initialized;
	/* Time base Type */
	time_bases_type bases_type;
	/* Initialization Sequence */
	time_genoff_t1_cb_type init_func;
	/* Mark if subsys based on another subsys */
	uint8_t reqd_base_genoff;
	/* Base Subsys */
	time_bases_type subsys_base;
	/* Specification for persistent storage */
	time_genoff_per_storage_type    per_storage_spec;
} time_genoff_struct_type;

struct send_recv_struct {
	uint32_t base;
	uint32_t unit;
	uint32_t operation;
	uint64_t value;
	int result;
};

typedef struct time_genoff_info {
	time_bases_type base;		/* Genoff in consideration */
	void *ts_val;			/* Time to be set/get */
	time_unit_type unit;		/* Time unit */
	time_genoff_opr_type operation; /* Time operation to be done */
}time_genoff_info_type;


static int genoff_opr(time_genoff_info_type *pargs);

#endif /* TIME_GENOFF_I_H */
