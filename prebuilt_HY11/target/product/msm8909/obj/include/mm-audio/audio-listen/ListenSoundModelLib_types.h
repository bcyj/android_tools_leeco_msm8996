/*
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*======================================================================
DESCRIPTION : ListenSoundModelLibrary enums, structures, constants
====================================================================*/

#ifndef __LISTEN_SOUND_MODEL_LIB_TYPES_H__
#define __LISTEN_SOUND_MODEL_LIB_TYPES_H__

#include <stdint.h>

#define MAX_KEYWORD 50

#ifndef __LISTEN_SOUND_MODEL_LIB_V2_H__
typedef struct {
	int16_t *data;				/* Audio samples ( in Raw PCM format: 16kHz, 16bit, mono ) */
	uint32_t n_samples;			/* number of audio samples */
} listen_user_recording;

typedef struct {
	uint8_t *data;				/* block of memory containing Model data */
	uint32_t size;				/* size of memory allocated for Model data */
} listen_model_type;

typedef enum {
	kKeywordModel = 1,			/* Keyword model */
	kUserKeywordModel = 2,      /* Userkeyword model */
	kTargetSoundModel = 3
} listen_model_enum;

typedef struct {
	listen_model_enum type;		/* model type: Keyword, User, TargetSound */
	uint32_t          version;	/* model version */
	uint32_t          size;		/* total size of the model: header + payload size */
} listen_sound_model_info;

typedef struct {
	uint8_t *data;				// block of memory containing data payload
	uint32_t size;				// size in bytes of payload data
} listen_event_payload;

typedef struct {
	uint16_t keywordConfidenceLevel;
	uint16_t userConfidenceLevel;
} listen_detection_event_v1;

typedef struct {
	char keyword[MAX_KEYWORD];
	uint16_t version;

	union {
		listen_detection_event_v1 event_v1;
	} event;

} listen_detection_event_type;

typedef enum {
	kSucess = 0,
	kFailed = 1,
} listen_status_enum;
#endif // __LISTEN_SOUND_MODEL_LIB_V2_H__

#endif /* __LISTEN_SOUND_MODEL_LIB_TYPES_H__ */
