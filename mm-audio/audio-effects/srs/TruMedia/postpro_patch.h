/*======================================================================*
 DTS, Inc.
 5220 Las Virgenes Road
 Calabasas, CA 91302  USA

 CONFIDENTIAL: CONTAINS CONFIDENTIAL PROPRIETARY INFORMATION OWNED BY
 DTS, INC. AND/OR ITS AFFILIATES ("DTS"), INCLUDING BUT NOT LIMITED TO
 TRADE SECRETS, KNOW-HOW, TECHNICAL AND BUSINESS INFORMATION. USE,
 DISCLOSURE OR DISTRIBUTION OF THE SOFTWARE IN ANY FORM IS LIMITED TO
 SPECIFICALLY AUTHORIZED LICENSEES OF DTS.  ANY UNAUTHORIZED
 DISCLOSURE IS A VIOLATION OF STATE, FEDERAL, AND INTERNATIONAL LAWS.
 BOTH CIVIL AND CRIMINAL PENALTIES APPLY.

 DO NOT DUPLICATE. COPYRIGHT 2014, DTS, INC. ALL RIGHTS RESERVED.
 UNAUTHORIZED DUPLICATION IS A VIOLATION OF STATE, FEDERAL AND
 INTERNATIONAL LAWS.

 ALGORITHMS, DATA STRUCTURES AND METHODS CONTAINED IN THIS SOFTWARE
 MAY BE PROTECTED BY ONE OR MORE PATENTS OR PATENT APPLICATIONS.
 UNLESS OTHERWISE PROVIDED UNDER THE TERMS OF A FULLY-EXECUTED WRITTEN
 AGREEMENT BY AND BETWEEN THE RECIPIENT HEREOF AND DTS, THE FOLLOWING
 TERMS SHALL APPLY TO ANY USE OF THE SOFTWARE (THE "PRODUCT") AND, AS
 APPLICABLE, ANY RELATED DOCUMENTATION:  (i) ANY USE OF THE PRODUCT
 AND ANY RELATED DOCUMENTATION IS AT THE RECIPIENT’S SOLE RISK:
 (ii) THE PRODUCT AND ANY RELATED DOCUMENTATION ARE PROVIDED "AS IS"
 AND WITHOUT WARRANTY OF ANY KIND AND DTS EXPRESSLY DISCLAIMS ALL
 WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE, REGARDLESS OF WHETHER DTS KNOWS OR HAS REASON TO KNOW OF THE
 USER'S PARTICULAR NEEDS; (iii) DTS DOES NOT WARRANT THAT THE PRODUCT
 OR ANY RELATED DOCUMENTATION WILL MEET USER'S REQUIREMENTS, OR THAT
 DEFECTS IN THE PRODUCT OR ANY RELATED DOCUMENTATION WILL BE
 CORRECTED; (iv) DTS DOES NOT WARRANT THAT THE OPERATION OF ANY
 HARDWARE OR SOFTWARE ASSOCIATED WITH THIS DOCUMENT WILL BE
 UNINTERRUPTED OR ERROR-FREE; AND (v) UNDER NO CIRCUMSTANCES,
 INCLUDING NEGLIGENCE, SHALL DTS OR THE DIRECTORS, OFFICERS, EMPLOYEES,
 OR AGENTS OF DTS, BE LIABLE TO USER FOR ANY INCIDENTAL, INDIRECT,
 SPECIAL, OR CONSEQUENTIAL DAMAGES (INCLUDING BUT NOT LIMITED TO
 DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, AND LOSS
 OF BUSINESS INFORMATION) ARISING OUT OF THE USE, MISUSE, OR INABILITY
 TO USE THE PRODUCT OR ANY RELATED DOCUMENTATION.
*======================================================================*/



#ifndef ANDROID_SRS_AUDIOFLINGER_PATCH
#define ANDROID_SRS_AUDIOFLINGER_PATCH

// DEFINE if detailed SRS-related logs are required...
//#define SRS_VERBOSE

#include "srs_processing.h"

#define SRS_PROCESSING_ACTIVE

namespace android {

// MACROS to help create very minimal deltas at the audioflinger level

#ifdef SRS_VERBOSE
    #define SRS_LOG(...) ((void)ALOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#else
    #define SRS_LOG(...) ((void)0)
#endif

#define POSTPRO_PATCH_PARAMS_SET(a) \
    if (SRS_DoAny()){ \
        SRS_Processing::ParamsSet(SRS_Processing::AUTO, a); \
    }

#define POSTPRO_PATCH_PARAMS_GET(a, b) \
    if (SRS_DoAny()){ \
        String8 srs_params = SRS_Processing::ParamsGet(SRS_Processing::AUTO, a); \
        if (srs_params != "") b += srs_params+";"; \
    }

#define POSTPRO_PATCH_OUTPROC_PLAY_INIT(a, b) \
    if (SRS_DoOutput()){ \
        SRS_LOG("SRS_Processing - OutNotify_Init: %s\n", b.string()); \
        SRS_Processing::ProcessOutNotify(SRS_Processing::AUTO, a, true); \
    }

#define POSTPRO_PATCH_OUTPROC_PLAY_SAMPLES(a, fmt, buf, bsize, rate, count) \
    if (((fmt == AUDIO_FORMAT_PCM_16_BIT) || (fmt == AUDIO_FORMAT_PCM_32_BIT))    && SRS_DoOutput()){ \
        SRS_Processing::ProcessOut(SRS_Processing::AUTO, a, buf, bsize, rate, count); \
    }

#define POSTPRO_PATCH_OUTPROC_PLAY_EXIT(a, b) \
    if (SRS_DoOutput()){ \
        SRS_LOG("SRS_Processing - OutNotify_Exit: %s\n", b.string()); \
        SRS_Processing::ProcessOutNotify(SRS_Processing::AUTO, a, false); \
    }

#define POSTPRO_PATCH_OUTPROC_PLAY_ROUTE_BY_VALUE(a, val) \
    if (SRS_DoOutput()){ \
        SRS_Processing::ProcessOutRoute(SRS_Processing::AUTO, a, val); \
    }

#define POSTPRO_PATCH_OUTPROC_PLAY_ROUTE(a, para, val) \
    if (SRS_DoOutput()){ \
        if (para.getInt(String8(AudioParameter::keyRouting), val) == NO_ERROR){ \
            SRS_Processing::ProcessOutRoute(SRS_Processing::AUTO, a, val); \
        } \
    }

#define POSTPRO_PATCH_OUTPROC_DIRECT_INIT(a, b) \
    if (SRS_DoOutput()){ \
        SRS_LOG("SRS_Processing - DirectOutputThread - OutNotify_Init: %s\n", b.string()); \
        SRS_Processing::ProcessOutNotify(SRS_Processing::AUTO, a, true); \
    }

#define POSTPRO_PATCH_OUTPROC_DIRECT_SAMPLES(a, fmt, buf, bsize, rate, count) \
    if ((fmt == AUDIO_FORMAT_PCM_16_BIT) && SRS_DoOutput()){ \
        SRS_Processing::ProcessOut(SRS_Processing::AUTO, a, buf, bsize, rate, count); \
    }

#define POSTPRO_PATCH_OUTPROC_DIRECT_EXIT(a, b) \
    if (SRS_DoOutput()){ \
        SRS_LOG("SRS_Processing - DirectOutputThread - OutNotify_Exit: %s\n", b.string()); \
        SRS_Processing::ProcessOutNotify(SRS_Processing::AUTO, a, false); \
    }

#define POSTPRO_PATCH_INPROC_INIT(a, b, fmt) \
    if ((fmt == AUDIO_FORMAT_PCM_16_BIT) && SRS_DoInput()){ \
        SRS_LOG("SRS_Processing - RecordThread - InNotify_Init: %p TID %d\n", a, b); \
        SRS_Processing::ProcessInNotify(SRS_Processing::AUTO, a, true); \
    }

#define POSTPRO_PATCH_INPROC_SAMPLES(a, fmt, buf, bsize, rate, count) \
    if ((bsize > 0) && (fmt == AUDIO_FORMAT_PCM_16_BIT) && SRS_DoInput()){ \
        SRS_Processing::ProcessIn(SRS_Processing::AUTO, a, buf, bsize, rate, count); \
    }

#define POSTPRO_PATCH_INPROC_EXIT(a, b, fmt) \
    if ((fmt == AUDIO_FORMAT_PCM_16_BIT) && SRS_DoInput()){ \
        SRS_LOG("SRS_Processing - RecordThread - InNotify_Exit: %p TID %d\n", a, b); \
        SRS_Processing::ProcessInNotify(SRS_Processing::AUTO, a, false); \
    }

#define POSTPRO_PATCH_INPROC_ROUTE(a, para, val) \
    if (SRS_DoInput()){ \
        if (para.getInt(String8(AudioParameter::keyRouting), val) == NO_ERROR){ \
            SRS_Processing::ProcessInRoute(SRS_Processing::AUTO, a, val); \
        } \
    }


// FUNCTIONS to help direct execution based on build.prop settings

#ifdef POSTPRO_PROPGATE
bool SRS_DoOutput();
bool SRS_DoInput();
bool SRS_DoAny();
#else
static bool SRS_DoOutput(){ return true; }
static bool SRS_DoInput(){ return true; }
static bool SRS_DoAny(){ return true; }
#endif

};	// namespace android

#endif // ANDROID_SRS_AUDIOFLINGER_PATCH
