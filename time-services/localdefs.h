/*
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2011, 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  --------------------------------------------------------------------------
 *
 */


/*
 * Various time bases.
 */
#include <pthread.h>

typedef enum time_bases
{
	ATS_RTC = 0, /**< Real time clock timebase.*/
	ATS_TOD,     /**< Proxy base for number of bases.*/
	ATS_USER,    /**< User timebase. */
	ATS_SECURE,  /**< Secure timebase. */
	ATS_DRM,     /**< Digital rights management timebase. */
	ATS_USER_UTC,   /**< Universal Time Coordinated user
			  timebase. */
	ATS_USER_TZ_DL, /**< Global time zone user timebase. */
	ATS_GPS,    /**< Base for GPS time. \n
		      @note1hang When
		      ATS_GSTK is modified,
		      changes are also
		      reflected
		      on
		      ATS_TOD.
		      */
	ATS_1X,     /**< Base for 1X time. \n
		      @note1hang When
		      ATS_1X is modified,
		      changes are also
		      reflected
		      on
		      ATS_TOD.
		      */
	ATS_HDR,    /**< Base for HDR time. \n
		      @note1hang When
		      ATS_HDR is
		      modified, changes
		      are also
		      reflected
		      on
		      ATS_TOD.
		      */
	ATS_WCDMA,  /**< Base for WCDMA time. \n
		      @note1hang When
		      ATS_WCDMA is
		      modified,
		      changes are
		      also
		      reflected
		      on
		      ATS_TOD.
		      */
	ATS_SNTP,   /**< Base for SNTP time. \n
		      @Only for APPS.
		      */
	ATS_UTC,    /**< Base for Multimedia. \n
		      @Only for APPS.
		      */
	ATS_MODEM,    /**< Base for Modem time. \n
                      @Only for APPS.
                      */
	ATS_MFLO,   /**< Base for MediaFLO time. \n
		      @note1hang
		      When ATS_MFLO
		      is modified,
		      changes are
		      also
		      reflected
		      on
		      ATS_TOD.
		      */
	ATS_INVALID = 0x10000000
} time_bases_type;

/*
 * Time unit type.
 */
typedef enum time_unit
{
	TIME_STAMP,      /**< Time is in timestamp format. */
	TIME_MSEC,       /**< Time is in millisecond format. */
	TIME_SECS,       /**< Time is in second format. */
	TIME_JULIAN,     /**< Time is in Julian calendar format. */
	TIME_20MS_FRAME, /**< Time is in 20-millisecond frames
			   format. */
	TIME_NONE        /**< Time format is undefined. */
} time_unit_type ;


typedef enum time_genoff_opr
{
	T_SET, /**< Genoff_Opr sets the time. */
	T_GET, /**< Genoff_Opr gets the time. */
	T_IS_SET, /**< Genoff_Opr checks offset set. */
	T_DISABLE, /**< Disable Logging. */
	T_ENABLE, /**< Enable Logging. */
	T_MAX  /**< Placeholder for maximum enumerator value. */
} time_genoff_opr_type;

typedef struct ind_offset {
	int offset;		/* Offset number */
	int ind_received;	/* Flag for indication status */
	pthread_mutex_t lock;   /* Lock to protect structure */
} cb_indication_t;
