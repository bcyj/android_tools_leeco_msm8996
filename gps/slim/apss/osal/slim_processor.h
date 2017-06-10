#ifndef SLIM_PROCESSOR_H
#define SLIM_PROCESSOR_H
/*============================================================================
  @file slim_internal.h

  SLIM processor specific utils declarations.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include <stdio.h>
#include <log_util.h>


/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define SLIM_MSG0 SLIM_MSG
#define SLIM_MSG1 SLIM_MSG
#define SLIM_MSG2 SLIM_MSG

#define SLIM_MSG(LEVEL, ...) \
    switch (LEVEL)\
    {\
    case MSG_LEGACY_HIGH: \
        SLIM_DMN_PRINT(LOC_LOGI, __VA_ARGS__) \
        break; \
    case MSG_LEGACY_ERROR: \
        SLIM_DMN_PRINT(LOC_LOGE,  __VA_ARGS__) \
        break; \
    default: \
        SLIM_DMN_PRINT(LOC_LOGV,  __VA_ARGS__) \
        break; \
    }

#define SLIM_DMN_PRINT(FUNC, ...) FUNC(__VA_ARGS__)

/* Can be removed when included in log_codes.h */
#ifndef LOG_GNSS_SLIM_EVENT_C
#define LOG_GNSS_SLIM_EVENT_C 0x1885
#endif /* !LOG_GNSS_SLIM_EVENT_C */

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
/* Typedef for double */
typedef double DBL;

/*mutex for synchronization*/
typedef struct ap_MutexBlockType {
    pthread_mutex_t d_mutex;  /* mutex lock for sensor1 callback */
} ap_MutexBlockType;


typedef ap_MutexBlockType slim_MutexStructType;

//TODO:SET MAX COUNT TO 16 so that clients can be added dynamically
//without updating the list? Currently used by SLIM CORE
#define SLIM_CLIENT_MAX_COUNT SLIM_CLIENT_LAST

/** Enum for SLIM clients in modem. */
typedef enum
{
  SLIM_CLIENT_MPSS  = 1, /**< Client id for Modem clients*/
  SLIM_CLIENT_PIP,       /**< Client id for PIP */
  SLIM_CLIENT_LAST = SLIM_CLIENT_PIP,

  SLIM_CLIENT_MAX  = 2147483647 /**< To force a 32 bit signed enum. Do not change or use */
} slim_ClientEnumType;

/** Enum for SLIM Sensor Providers
    Values should correspond to array indexes in providers array in provider
    registry.
*/
typedef enum
{
  SLIM_PROVIDER_NONE = -1,     /**< Invalid */
  SLIM_PROVIDER_SENSOR1 = 0,   /**< Sensor1 API */
  SLIM_PROVIDER_NDK,           /**< Android NDK */
  SLIM_PROVIDER_COUNT,
  SLIM_PROVIDER_MAX     = 2147483647 /**< To force a 32 bit signed enum. Do not change or use */
} slim_ProviderEnumType;


#endif /* #ifndef SLIM_PROCESSOR_H */
