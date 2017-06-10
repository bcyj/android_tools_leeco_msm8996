/*
**
** Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
** Qualcomm Technologies Proprietary and Confidential.
*/

/*****************************************************************************
 typedefs, enums, and data structures for Listen C/C++ code
******************************************************************************/

#ifndef ANDROID_LISTEN_NATIVE_TYPES_H_
#define ANDROID_LISTEN_NATIVE_TYPES_H_

#include <listen_types.h>

namespace android {

//
// forward declaration
//
struct listen_session;

//
// Typedefs
//
typedef uint32_t listen_session_id_t;

/* This structure defines the id type */
typedef uint32_t   listen_id_t;

static const bool     ENABLE  = true;
static const bool     DISABLE = false;

static const uint32_t UNDEFINED = 0xFFFFFFFF;

/* Application Type include 4-byte algorithm type plus 4-byte version number
 *   4-byte version number: 2-bytes major, 2-bytes minor numbers
 */
static const uint16_t  APP_TYPE_UNKNOWN = 0;
static const uint32_t  SVA_APP_TYPE            = 0x01;  // version not yet known

static const uint16_t  ALGO_VERS_UNKNOWN       = 0;
static const uint16_t  ALGO_VERS_0100          = 0x0100;  // version 1.0
static const uint16_t  ALGO_VERS_0200          = 0x0200;  // version 2.0

static const uint32_t  EVENT_DATA_VERSION = 1;
static const uint32_t  EVENT_DATA_VERSION_V2 = 2;

static const uint32_t  UNKNOWN_SOUNDMODEL_TYPE = 0;
// SoundModel is Snapdragon Voice Activation format
static const uint32_t  SVA_SOUNDMODEL_TYPE = 1;

// Version number: 2 MSBytes major number, 2 LSBytes minor number
//    For non-SVA SoundModels version will be unknown
static const uint32_t  VERSION_UNKNOWN    = 0;
static const uint32_t  VERSION_0100       = 0x0100; // Version 1.0
static const uint32_t  VERSION_0200       = 0x0200; // Version 2.0

//
// Enumerators
//

/* This enum is used to return status of Listen API calls */
typedef enum{
  LISTEN_SUCCESS = 0,                 // must be 0 to match ListenSoundModel lib enums
  // Error numbering that match Java API errors in ListenTypes.java
  LISTEN_EFAILURE = -1,               // Failed for some unspecified reason - generic
  LISTEN_EBAD_PARAM = -2,             // Bad input parameter(s)
  LISTEN_ESOUNDMODEL_NOT_REGISTERED = -3,  // SoundModel is not registered
  LISTEN_ESOUNDMODEL_ALREADY_REGISTERED = -4, // SoundModel should be deregistered before re-registering
  LISTEN_EFEATURE_NOT_ENABLED = -5,   // either Listen or VoiceWakeup Feature not enabled
  LISTEN_ERESOURCE_NOT_AVAILABLE = -6,  // requested object can not be instanced
  LISTEN_ECALLBACK_NOT_SET = -7,      // callback must be set for MasterControl or Session object
  LISTEN_EBUFFERING_NOT_ENABLED = -8,  // buffering is not enabled for this session
  LISTEN_ENOT_BUFFERING = -9,  // currently not doing buffering
  LISTEN_EBUFFERING_DATA_INCOMPLETE = -10, // data in buffer is not complete; e.g. overflow occurred
  LISTEN_EKEYWORD_NOT_IN_SOUNDMODEL = -11, // keyword specified as input parameter not in SoundModel
  LISTEN_EUSER_NOT_IN_SOUNDMODEL = -12, // user specified as input parameter not in SoundModel
  LISTEN_EKEYWORD_USER_PAIR_NOT_IN_SOUNDMODEL = -13, // user+keyword pair specified as input parameter not in SoundModel
  LISTEN_ENOT_SUPPORTED_FOR_SOUNDMODEL_VERSION = -14, // feature not supported with given SoundModel type
  LISTEN_EUNSUPPORTED_SOUNDMODEL = -15, // type or version of SoundModel not support by algorithm
  LISTEN_EUSER_NAME_CANNOT_BE_USED = -16, // user specified as input parameter already used or reserved
  LISTEN_ENO_MEMORY = -17, // memory could not be allocated
  LISTEN_EDETECTION_NOT_RUNNING = -18,  // detection not running
  LISTEN_EUSER_KEYWORD_PAIRING_ALREADY_PRESENT = -19, // can't train same KW-user pair if present in SM
  LISTEN_ESOUNDMODELS_WITH_SAME_KEYWORD_CANNOT_BE_MERGED = -20, // merged SMs must have unique KWs
  LISTEN_ESOUNDMODELS_WITH_SAME_USER_KEYWORD_PAIR_CANNOT_BE_MERGED = -21, // merged SMs can't not contain same pair
  LISTEN_EMAX_KEYWORDS_EXCEEDED = -22, // merge would result in SM that exceeds max KW capacity
  LISTEN_EMAX_USERS_EXCEEDED = -23, // merge would result in SM that exceeds max User capacity
  LISTEN_ECANNOT_DELETE_LAST_KEYWORD = -24, // can not delete keyword from soundmodel if it is the only one
  LISTEN_ENO_SPEACH_IN_RECORDING = -25, // no speach detected within recording
  LISTEN_ETOO_MUCH_NOISE_IN_RECORDING = -26, // level of noise in recording to high

  // Error not (yet) exposed to Java
  LISTEN_ENO_GLOBAL_CONTROL = -1001,            // returned if session request to change global param but does not have permission
  LISTEN_ENOT_INITIALIZED = -1002,              // Service object not initialized
  LISTEN_ERECORDINGS_MISMATCH_KEYWORD = -1003,  // Recordings dont match the model
  LISTEN_ESESSION_NOT_ACTIVE = -1004,           // Session was not created/opened successfully
} listen_status_enum_t;

/* This enum defines the types of event notifications sent to the App
 * from ListenService.  This is a superset of event setn*/
typedef enum{
  // generic event to indicate error
  LISTEN_ERROR = 0,

  // MAD HW Disabled or re-Enabled by AudioHAL or ListenServer
  LISTEN_FEATURE_DISABLED = 1,
  LISTEN_FEATURE_ENABLED = 2,
  VOICE_WAKEUP_FEATURE_DISABLED = 3,
  VOICE_WAKEUP_FEATURE_ENABLED = 4,

  // SVA 1.0 Keyword detection was successful - minimum keyword and user confidence levels were met
  LISTEN_DETECT_SUCCEED = 5,

  // SVA 1.0 Keyword detection would have failed but when Special Detect-All mode this event can be return
  LISTEN_DETECT_FAILED = 6,

  // SoundModel de-registered by ListenNativeService because MAD HW Disabled
  SOUNDMODEL_DEREGISTERED = 7,

  // Listen started or stopped.
  // Could be sent due to microphone device concurrency.
  LISTEN_ENGINE_STARTED = 8,
  LISTEN_ENGINE_STOPPED = 9,

  // Catastropic error occurred; Listen Service has been restarted.
  // All previously created Listen objects are stale & must be recreated.
  LISTEN_ENGINE_DIED = 10,

  //
  // Event constants exposed only to C++ Framework code
  //
  // SVA 2.0 Keyword detection was successful.
  // This will be converted to LISTEN_DETECT_SUCCEED before passing to Java
  LISTEN_DETECT_V2_SUCCEED = 105,

  // SVA 2.0 Keyword detection would have failed but when Special Detect-All mode this event can be return
  // This will be converted to LISTEN_DETECT_FAILED before passing to Java
  LISTEN_DETECT_V2_FAILED = 106
}  listen_service_event_enum_t;

/* This enum defines the parameter Types used by Set/GetParam function*/
typedef enum{
  LISTEN_PARAM_LISTEN_FEATURE_ENABLE = 1,      // used to enable or disable Listen Feature
  LISTEN_PARAM_VOICE_WAKEUP_FEATURE_ENABLE  // used to enable or disable VoiceWakeup Feature
}listen_param_enum_t;

/* Reciever types */
typedef enum{
   LISTEN_RECEIVER_UNDEFINED = 0,
	LISTEN_RECEIVER_MASTER_CONTROL = 1,       // has control over global params
	LISTEN_RECEIVER_VOICE_WAKEUP_SESSION = 2,
}listen_receiver_enum_t;

// number should be set to that same int as the last enum in listen_receiver_enum_t
#define NUM_LISTEN_RECEIVER_TYPES 2

/* Session types */
typedef enum{
	LISTEN_VOICE_WAKEUP_SESSION = 1
}listen_session_enum_t;

//
// Structures
//
typedef struct listen_detection_params {
    uint16_t                     status;        // 0 successful detection, else failure
    listen_detection_mode_enum_t detectionMode;
    uint16_t                     numKeywords;   // keywords in registered SoundModel
    uint16_t                     numUsers;      // users in registered SoundModel
    uint16_t                     numActiveKWUserPairs;  // active pairs in registered SoundModel
    char **                      keywords;
    char **                      users;
    uint16_t                     sizeConfLevels;
    uint8_t *                    pMinConfLevels; // minimum confidence levels for keywords and pairs
    uint16_t *                   pUserKeywordPairFlags;  // table containing [user][keyword] active pair flags
} listen_detection_params_t;


typedef struct listen_resources {
    uint16_t                     num_sessions;
    uint16_t                     num_keywords;
    uint16_t                     num_user_kw_pairings;
    uint16_t                     num_kw_plus_user_pairings;
} listen_resources_t;

}; // namespace android

#endif // ANDROID_LISTEN_NATIVE_TYPES_H_
