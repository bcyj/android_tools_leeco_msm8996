/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "QCExtractor"
#include <utils/Log.h>
//#define DUMP_TO_FILE


#include "ExtendedExtractorFuncs.h"
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaDefs.h>
#include <utils/String8.h>
#include <dlfcn.h>  // for dlopen/dlclose
#include "common_log.h"


static const char* MM_PARSER_LIB = "libmmparser.so";
static const char* MM_PARSER_LITE_LIB = "libmmparser_lite.so";

namespace android {

void* MmParserLib() {
    static void* mmParserLib = NULL;
    static bool alreadyTriedToOpenMmParsers = false;

    if(!alreadyTriedToOpenMmParsers) {
        alreadyTriedToOpenMmParsers = true;
        mmParserLib = ::dlopen(MM_PARSER_LIB, RTLD_LAZY);

        if(mmParserLib != NULL) {
            return mmParserLib;
        }

        LOGV("Failed to open MM_PARSER_LIB, dlerror = %s \n", dlerror());

        mmParserLib = ::dlopen(MM_PARSER_LITE_LIB, RTLD_LAZY);

        if(mmParserLib == NULL) {
            LOGV("Failed to open MM_PARSER_LITE_LIB, dlerror = %s \n", dlerror());
        }
    }
    return mmParserLib;
}

MediaExtractorFactory MediaExtractorFactoryFunction() {
    static MediaExtractorFactory mediaFactoryFunction = NULL;
    static bool alreadyTriedToFindFactoryFunction = false;

    if(!alreadyTriedToFindFactoryFunction) {

        void *mmParserLib = MmParserLib();
        if (mmParserLib == NULL) {
            return NULL;
        }

        mediaFactoryFunction = (MediaExtractorFactory) dlsym(mmParserLib, MEDIA_CREATE_EXTRACTOR);
        alreadyTriedToFindFactoryFunction = true;
    }

    if(mediaFactoryFunction==NULL) {
        LOGE(" dlsym for ExtendedExtractor factory function failed, dlerror = %s \n", dlerror());
    }

    return mediaFactoryFunction;
}


extern "C" MediaExtractor* CreateExtractor(const sp<DataSource> &source, const char* mime) {

    MediaExtractorFactory f = MediaExtractorFactoryFunction();
    if(f==NULL) {
        return NULL;
    }

    MediaExtractor* extractor = f(source, mime);
    if(extractor==NULL) {
        LOGE(" ExtendedExtractor failed to instantiate extractor \n");
    }

    return extractor;
}

extern "C" bool SniffExtendedExtractor(const sp<DataSource> &source, String8 *mimeType,
                                       float *confidence,sp<AMessage> *meta) {
    *confidence = 0.0f;
    void *mmParserLib = MmParserLib();
    if (mmParserLib == NULL) {
        return false;
    }

    SnifferArrayFunc snifferArrayFunc = (SnifferArrayFunc) dlsym(mmParserLib, MEDIA_SNIFFER_ARRAY);
    if(snifferArrayFunc==NULL) {
        LOGE(" Unable to init Extended Sniffers, dlerror = %s \n", dlerror());
        return false;
    }

    const DataSource::SnifferFunc *snifferArray = NULL;
    int snifferCount = 0;

    //Invoke function in libmmparser to return its array of sniffers.
    snifferArrayFunc(&snifferArray, &snifferCount);

    if(snifferArray==NULL) {
        LOGE(" snifferArray is NULL \n");
        return false;
    }

    float sniffConfidence = 0.0;
    bool retVal = false;
    for(int i=0; i<snifferCount; i++) {
          sniffConfidence = 0.0;
          bool ret = (snifferArray[i])(source, mimeType, &sniffConfidence, meta);
          if((ret) && (sniffConfidence > *confidence)) {
              *confidence = sniffConfidence;
              retVal = true;
          }
    }

    return retVal;
}

}  // namespace android


