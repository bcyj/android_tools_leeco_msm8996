/* listen_types.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*****************************************************************************
 typedefs, enums, and data structures for Listen C/C++ code
 common to all C++ framework Native and HAL components
******************************************************************************/

#ifndef ANDROID_LISTEN_TYPES_H_
#define ANDROID_LISTEN_TYPES_H_

static const uint16_t MAX_LISTEN_SESSIONS = 8;

/* Used for AudioHAL set MAD enable parameter */
#define AUDIO_PARAMETER_KEY_MAD "mad"
#define AUDIO_PARAMETER_VALUE_MAD_ON "mad_on"
#define AUDIO_PARAMETER_VALUE_MAD_OFF "mad_off"

/* Sound model detection modes */
enum listen_detection_mode_enum{
  LISTEN_MODE_KEYWORD_ONLY_DETECTION = 1,
  LISTEN_MODE_USER_KEYWORD_DETECTION
};
typedef enum listen_detection_mode_enum listen_detection_mode_enum_t;

/* This enum defines the types of event notifications that can be sent
 * by HAL to Listen Service.
 * These enums should correspond to constants in listen_service_event_enum_t
 * in ListenNativeTypes.h
 */
enum listen_event_enum {

  /* generic event to indicate error; should not be sent */
  LISTEN_ERROR = 0,
  /* Keyword detection was successful - minimum keyword and user confidence levels were met */
  LISTEN_EVENT_DETECT_SUCCESS = 5,
  /* Keyword detection would have failed but when Special Detect-All mode this event can be return */
  LISTEN_EVENT_DETECT_FAILED = 6,
  /* Can be sent when all listen sessions are started or stopped during concurency */
  LISTEN_EVENT_STARTED = 8,
  LISTEN_EVENT_STOPPED = 9
};
typedef enum listen_event_enum listen_event_enum_t;

/* Listen algorithm type and associated version */
/* Corresponds to enunm lsm_app_id in kernel\include\soundlsm_params.h  */
enum listen_app_type_enum {
  LISTEN_APP_TYPE_SVA = 1,    /* VoiceWakeup SVA 1.0 */
  LISTEN_APP_TYPE_SVA_V2 = 2, /* VoiceWakeup SVA 2.0 */
};
typedef enum listen_app_type_enum listen_app_type_enum_t;

enum event_type {
    AUDIO_DEVICE_IN_INACTIVE,
    AUDIO_DEVICE_IN_ACTIVE,
    AUDIO_STREAM_OUT_INACTIVE,
    AUDIO_STREAM_OUT_ACTIVE
};
typedef enum event_type event_type_t;

/* SoundModel type: describe format of SM (if known)  */
enum listen_sound_model_enum {
  LISTEN_SM_TYPE_SVA = 1,       /* SVA 1.0 SoundModel format */
  LISTEN_SM_TYPE_SVA_V2 = 2,    /* SVA 2.0 or above SoundModel format */
};
typedef enum listen_sound_model_enum listen_sound_model_enum_t;

struct listen_open_params {
    listen_app_type_enum_t  app_id;  /* algorithm type selected by client */
    uint16_t  num_keywords; /* number of keywords that will be specified in reg'ed SM */
    uint16_t  num_user_pairings;  /* count of user-specific data defined in reg'ed SM */
};
typedef struct listen_open_params listen_open_params_t;

/* Sound model representation */
struct listen_sound_model_data {
    uint8_t   *p_data;           /* block of memory containing Model data */
    uint32_t  size;              /* total size of Model data */
} ;
typedef struct listen_sound_model_data listen_sound_model_data_t;

/* Sound model parameters
 * Parameters for RegisterSoundModel SVA
 *
 * detectionModel
 *    type of detection to perform: Keyword-only, User-specific
 * detect_failure
 *    Indicates the Listen algorithm to ignore minimum Keyword and User
 *    confidence level settingsand return all detections as an event to
 *    the client
 * min_conf_levels
 *    confidence levels for N Keywords and M active User+Keyword pairs
 * buffer_length
 *    in milliseconds; > 0 indicates buffering enabled
 * num_keywords
 *    number of Keywords.
 * num_user_pairings
 *    number of active User+Keyword pairings.
 */
struct listen_sound_model_params {
    listen_sound_model_enum_t    sound_model_type;
    listen_sound_model_data_t*   sound_model_data;
    listen_detection_mode_enum_t detection_mode;
    bool                         detect_failure;
    uint8_t  *                   min_conf_levels;
    bool                         enable_buffering;
    uint16_t                     num_keywords;
    uint16_t                     num_user_pairings;
};
typedef struct listen_sound_model_params listen_sound_model_params_t;

/* Event detect structure
 * status
 *    0-event detection falied, 1-event successfully detected
 * data
 *    event payload
 * size
 *    size of payload
 */
struct listen_event_detect {
    uint16_t status;
    uint16_t size;
    uint8_t  *p_data;
};

union listen_event_data {
   struct listen_event_detect event_detect;
};
typedef union listen_event_data listen_event_data_t;

/* Callback function used for sending event notifications to client
 * event_type
 *    Type of the event returned to client
 * payload
 *    payload of the event
 * priv
 *    handle to listen_session.
 */
typedef void (*listen_callback_t)(listen_event_enum_t event_type,
                                  listen_event_data_t *payload,
                                  void *p_handle);

/* listen_session object to be instantiated by hal */
struct listen_session {
   /* After getting the listen_session handle thru create_listen_session
      this API is called for HAL to setup session with DSP */
   /* Registers the sound model with LSM server */
   int (*register_sound_model)(struct listen_session* p_handle,
                               listen_sound_model_params_t *params);
   /* Deregisters the sound model with LSM server */
   int (*deregister_sound_model)(struct listen_session* p_handle);
   /* Client sets the callback function for listen session with
      its private data*/
   int (*set_session_observer)(struct listen_session* p_handle,
                               listen_callback_t cb_func,
                               void *priv);
   /* Read buffered data after keyword detection. Updates the actual
      number of bytes read in p_bytes */
   int (*read_buffer)(struct listen_session* p_handle,
                      uint8_t *p_buf,
                      uint32_t *p_bytes);
   /* Stop buffering */
   int (*stop_buffering)(struct listen_session* p_handle);

};
typedef struct listen_session listen_session_t;

#endif /* ANDROID_LISTEN_TYPES_H_ */

