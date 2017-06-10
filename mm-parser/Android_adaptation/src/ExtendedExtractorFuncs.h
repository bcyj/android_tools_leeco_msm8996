/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef EXTENDED_EXTRACTOR_FUNCS_
#define EXTENDED_EXTRACTOR_FUNCS_

#include <media/stagefright/DataSource.h>

namespace android {

class MediaExtractor;

//Prototype for factory function - extended media extractor must export a function with this prototype to
//instantiate MediaExtractor objects.
typedef MediaExtractor* (*MediaExtractorFactory)(const sp<DataSource> &source, const char* mime);

//Function name for extractor factory function. Extended extractor must export a function with this name.
static const char* MEDIA_CREATE_EXTRACTOR = "createExtractor";

//Prototype for function to return sniffers - extended media extractor must export a function with this prototype to
//set a pointer to an array of sniffers and set the count value.
typedef void (*SnifferArrayFunc)(const DataSource::SnifferFunc* snifferArray[], int *count);

//Function name for function to return sniffer array. Extended extractor must export a function with this name.
static const char* MEDIA_SNIFFER_ARRAY = "snifferArray";

}   //namespace android

#endif  //EXTENDED_EXTRACTOR_FUNCS_
