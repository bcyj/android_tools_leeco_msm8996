/* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Not a Contribution. Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "OmaDrmEngine"
// #define LOG_NDEBUG 0

#include "SessionMap.h"
#include "OmaDrmEngine.h"
#include "OmaDrmEngineConst.h"
#include "MimeTypeUtil.h"
#include "objmng/svc_drm.h"
#include "drm_framework_common.h"

#include <utils/Log.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <fcntl.h>
#include <limits.h>
#include <DrmRights.h>
#include <DrmConstraints.h>
#include <DrmMetadata.h>
#include <DrmInfo.h>
#include <DrmInfoStatus.h>
#include <DrmInfoRequest.h>
#include <DrmSupportInfo.h>
#include <DrmConvertedStatus.h>
#include <utils/String8.h>

#define int64_const(s)          (s)
#define int64_add(dst, s1, s2)  ((void)((dst) = (s1) + (s2)))
#define int64_mul(dst, s1, s2)  ((void)((dst) = (int64_t)(s1) * (int64_t)(s2)))

#define MS_PER_SECOND 1000                  /* Milliseconds per second */
#define MS_PER_MINUTE 60 * MS_PER_SECOND    /* Milliseconds per minute */
#define MS_PER_HOUR   60 * MS_PER_MINUTE    /* Milliseconds per hour */
#define MS_PER_DAY    24 * MS_PER_HOUR      /* Milliseconds per day */

#define SECONDS_PER_MINUTE 60                       /* Seconds per minute*/
#define SECONDS_PER_HOUR   60 * SECONDS_PER_MINUTE  /* Seconds per hour */
#define SECONDS_PER_DAY    24 * SECONDS_PER_HOUR    /* Seconds per day */

#define DAY_PER_MONTH 30                    /* Days per month */
#define DAY_PER_YEAR  365                   /* Days per year */

static int readFunc(int handle, unsigned char* buf, int len);
static int seekFunc(int handle, int offset);
static int getLen(int handle);
static int64_t computeTime(int32_t date, int32_t time);
static int64_t computeInterval(int32_t date, int32_t time);
static int64_t mkUTCtime(
        uint32_t year, uint32_t month, uint32_t day,
        uint32_t hour, uint32_t minute, uint32_t second);

using namespace android;

// This extern "C" is mandatory to be managed by TPlugInManager
extern "C" IDrmEngine* create() {
    return new OmaDrmEngine();
}

// This extern "C" is mandatory to be managed by TPlugInManager
extern "C" void destroy(IDrmEngine* plugIn) {
    delete plugIn;
}

OmaDrmEngine::OmaDrmEngine() {
    ALOGV("OmaDrmEngine Construction");
    srand(systemTime());
}

OmaDrmEngine::~OmaDrmEngine() {
    ALOGV("OmaDrmEngine Destruction");
    int size = convertSessionMap.getSize();
    for (int i = 0; i < size; i++) {
        ConvertSession *convSession = (ConvertSession*) convertSessionMap.getValueAt(i);
        delete convSession;
    }
}

const char replaceStrList[] = "/$><?&*`'|\"";
char* OmaDrmEngine::replaceStr(char *old)
{
   int count = 0; 
   int len = (old == NULL)?0:strlen(old);
   if(len == 0) return NULL;

   int len2 = strlen(replaceStrList);
   char *str = (char*)malloc(len*sizeof(char)+1);
   memset(str,'\0',len+1);

   for(int i=0;i<len;i++)
   {    
      bool flag = false;
      for(int j=0;j<len2;j++)
      {    
         if(old[i] == replaceStrList[j])
         {
            flag = true;
            break;
         }
      }
      if(!flag)
        str[count++] = old[i];
   }
   return str;
}


int readFunc(int64_t handle, unsigned char* buf, int len)
{
        FILE * fd = (FILE *)handle;
        if(fd == NULL){
                return -1;
        }
        fread(buf, sizeof(char), len, fd);
        return len;
}

int seekFunc(int64_t handle, int offset)
{
        FILE * fd = (FILE *)handle ;
        if(fd == NULL){
                return -1;
        }
        fseek(fd , offset, SEEK_SET);
        return offset;
}

int getLen(int64_t handle)
{
        int32_t fileLen;
        FILE * fd = (FILE *) handle ;
        if(fd == NULL){
            ALOGE("fd is NULL");
            return -1;
        }
        long prev_pos = ftell(fd);
        fseek(fd , 0, SEEK_END);
        fileLen = ftell(fd);
        fseek(fd, prev_pos, SEEK_SET);
        return fileLen;
}

static int64_t computeTime(int32_t date, int32_t time)
{
    int32_t year, month, day, hour, minute, second;

    year = date / 10000;
    month = (date / 100) % 100;
    day = date % 100;
    hour = time / 10000;
    minute = (time / 100) % 100;
    second = time % 100;

    /* Adjust the invalid parameters. */
    if (year < 1970) year = 1970;
    if (month < 1) month = 1;
    if (month > 12) month = 12;
    if (day < 1) day = 1;
    if (day > 31) day = 31;
    if (hour < 0) hour = 0;
    if (hour > 23) hour = 23;
    if (minute < 0) minute = 0;
    if (minute > 59) minute = 59;
    if (second < 0) second = 0;
    if (second > 59) second = 59;

//    return mkgmtime(year, month, day, hour, minute, second) * 1000;

    return mkUTCtime (year, month, day, hour, minute, second) * 1000;
}

/**
 * Compute the milliseconds by the specified <code>date</code>
 * and <code>time</code>.
 * Note that here we always treat 1 year as 365 days and 1 month as 30 days
 * that is not precise. But it should not be a problem since OMA DRM 2.0
 * already restricts the interval representation to be day-based,
 * i.e. there will not be an interval with year or month any more in the
 * future.
 *
 * @param date - the specified date,
 *               <code>date = year * 10000 + month * 100 + day</code>
 * @param time - the specified time,
 *               <code>time = hour * 10000 + minute * 100 + second</code>
 *
 * @return the related milliseconds
 */
static int64_t computeInterval(int32_t date, int32_t time)
{
    int32_t year, month, day, hour, minute, second;
    int64_t milliseconds;

    year = date / 10000;
    month = (date / 100) % 100;
    day = date % 100;
    hour = time / 10000;
    minute = (time / 100) % 100;
    second = time % 100;

    /* milliseconds = ((((year * 365 + month * 30 + day) * 24
     *                + hour) * 60 + minute) * 60 + second) * 1000;
     */
    int64_mul(milliseconds,
        int64_const(year * DAY_PER_YEAR + month * DAY_PER_MONTH + day),
        int64_const(MS_PER_DAY));
    int64_add(milliseconds, milliseconds,
        int64_const(hour * MS_PER_HOUR + minute * MS_PER_MINUTE +
            second * MS_PER_SECOND));

    return milliseconds;
}

static int64_t mkUTCtime(
        uint32_t year, uint32_t month, uint32_t day,
        uint32_t hour, uint32_t minute, uint32_t second)
{
	struct tm localTime;

	memset (&localTime, 0x0, sizeof (struct tm));
	localTime.tm_year = year - 1900;
	localTime.tm_mon  = month - 1;
	localTime.tm_mday = day;
	localTime.tm_hour = hour;
	localTime.tm_min  = minute;
	localTime.tm_sec  = second;
    return mktime (&localTime);
}

DrmConstraints* OmaDrmEngine::onGetConstraints(int uniqueId, const String8* path, int action) {
    DrmConstraints* drmConstraints = new DrmConstraints();
     
    ALOGV("onGetConstraints start");
    if (NULL != path &&
        (RightsStatus::RIGHTS_VALID != onCheckRightsStatus(uniqueId, *path, action))) {
        // Return the empty constraints to show no error condition.
        char validRights[5];
        snprintf(validRights, sizeof(validRights), "%d", 0);
        drmConstraints->put(new String8("valid"),validRights);
        return drmConstraints;
    }
    
    int sessionId  = openDrmSession(String8(path->string()));
    int num;
    int result = SVC_drm_getContentRightsNum(sessionId, &num);
    T_DRM_Constraint_Info constraints;
    bool gotConstraints = false;
    int isUnlimited = 0;
    int64_t startDateTime = 0;
    int64_t endDateTime = 0;
    int count = 0;
    int numofRights = 0;
    int64_t tmp_startDate = 0;
    int64_t tmp_endDate   = 0;
    int64_t tmp_startTime = 0;
    int64_t tmp_endTime   = 0;

    ALOGV("onGetConstraints,result=%d,num=%d,action=%d",result,num,action);
    if(DRM_SUCCESS == result && num > 0) {
        T_DRM_Rights_Info_Node* rightsInfoNode = NULL;        
        SVC_drm_getContentRightsList(sessionId, &rightsInfoNode);      

        T_DRM_Rights_Info_Node *curNode = rightsInfoNode;
        ALOGV("rightsInfoNode=%x",rightsInfoNode);
        while(NULL != curNode) {
            T_DRM_Rights_Info info = curNode->roInfo;
            isUnlimited = info.bIsUnlimited;
            char unlimited[5];
            snprintf(unlimited, sizeof(unlimited), "%d", isUnlimited);
            ALOGV("unlimited = %s,%d", unlimited, isUnlimited);
            drmConstraints->put(new String8("unlimited"),unlimited);
            if(info.bIsUnlimited == 1) {
                drmConstraints->put(new String8("valid"),"1");
                SVC_drm_closeSession(sessionId);
                return drmConstraints;
            }
            ALOGV("isUnlimited =%d", isUnlimited);
            switch(action) {
                case Action::PLAY:
                    constraints = info.playRights;
                    ALOGV("copying play constraints");
                    gotConstraints = true;
                    break;
                case Action::EXECUTE:
                    ALOGV("copying exce constraints");
                    constraints = info.executeRights;
                     gotConstraints = true;
                    break;
                case Action::DISPLAY:
                    ALOGV("copying display constraints,info.displayRights=%x",info.displayRights);
                    constraints = info.displayRights;
                    gotConstraints = true;
                    break;
                case Action::TRANSFER:
                case Action::DEFAULT:
                case Action::RINGTONE:
                case Action::OUTPUT:
                case Action::PREVIEW:
                default:
                    break;
            }

            ALOGV("rights object = %d", ++numofRights);
            ALOGV("count = %d", constraints.count);
            ALOGV("startDate = %d,",constraints.startDate);
            ALOGV("startTime = %d", constraints.startTime);
            ALOGV("endDate = %d",constraints.endDate);
            ALOGV("endTime = %d", constraints.endTime);
            ALOGV("intervalDate = %d", constraints.intervalDate);
            ALOGV("intervalTime = %d", constraints.intervalTime);

            count = count + constraints.count;
            if (constraints.startDate > 0 ) {
                int64_t tempStart = computeTime(constraints.startDate, constraints.startTime);
                if( startDateTime == 0) startDateTime = tempStart;
                else if ( tempStart < startDateTime)  startDateTime = tempStart;

                tmp_startDate = constraints.startDate;
                tmp_startTime = constraints.startTime;
            }
            if (constraints.endDate > 0 ) {
                int64_t tempEnd = computeTime(constraints.endDate, constraints.endTime);
                if ( tempEnd > endDateTime) endDateTime = tempEnd;

                tmp_endDate = constraints.endDate;
                tmp_endTime = constraints.endTime;
            }
            curNode = curNode->next;
        }
    }
    SVC_drm_closeSession(sessionId);
    if(gotConstraints == true && constraints.indicator != 0) {
        char countStr[10];
        char startDateTimeStr[50];
        char endDateTimeStr[50];
        char intervalStr[50];
        char validRights[10];
        char startDate[50];
        char startTime[50];
        char endDate[50];
        char endTime[50];
        char intervalDate[50];
        char intervalTime[50];

        int64_t interval = computeInterval(constraints.intervalDate, constraints.intervalTime);
        if (interval != 0 && numofRights > 1) {
            int64_t temp = endDateTime - startDateTime;
            ALOGV("temp Interval = %lld", temp);
            if(interval < temp) interval = temp;
        }

        snprintf(validRights, sizeof(validRights), "%d", 1);
        drmConstraints->put(new String8("valid"),validRights);
        snprintf(countStr, sizeof(countStr), "%d", count);
        drmConstraints->put(new String8("count"),countStr);
        snprintf(startDateTimeStr, sizeof(startDateTimeStr), "%lld",
                startDateTime);
        drmConstraints->put(new String8("startDateTime"),startDateTimeStr);
        snprintf(endDateTimeStr, sizeof(endDateTimeStr), "%lld", endDateTime);
        drmConstraints->put(new String8("endDateTime"),endDateTimeStr);
        snprintf(intervalStr, sizeof(intervalStr), "%lld", interval);
        drmConstraints->put(new String8("interval"),intervalStr);
        ALOGV("startDateTime = %lld, endDateTime= %lld , \ninterval = %lld", startDateTime, endDateTime, interval);
        snprintf(startDate, sizeof(startDate), "%lld", tmp_startDate);
        drmConstraints->put(new String8("startDate"),startDate);
        snprintf(startTime, sizeof(startTime), "%06d", tmp_startTime);
        drmConstraints->put(new String8("startTime"),startTime);
        snprintf(endDate, sizeof(endDate), "%d", tmp_endDate);
        drmConstraints->put(new String8("endDate"),endDate);
        snprintf(endTime, sizeof(endTime), "%06d", tmp_endTime);
        drmConstraints->put(new String8("endTime"),endTime);
        snprintf(intervalDate, sizeof(intervalDate), "%d",
                constraints.intervalDate);
        drmConstraints->put(new String8("intervalDate"),intervalDate);
        snprintf(intervalTime, sizeof(intervalTime), "%d",
                constraints.intervalTime);
        drmConstraints->put(new String8("intervalTime"),intervalTime);

    } else {
        char valid[10];
        snprintf(valid, sizeof(valid), "%d", 0);
        drmConstraints->put(new String8("valid"),valid);
    }    
    return drmConstraints;
}

String8 OmaDrmEngine::getConstraintsAsString(int uniqueId, const int sessionId, int action) {
    ALOGV("getConstraintsAsString start");

    DrmConstraints* drmConstraints = new DrmConstraints();
    int num;
    int result = SVC_drm_getContentRightsNum(sessionId, &num);
    T_DRM_Constraint_Info constraints;
    bool gotConstraints = false;
    int isUnlimited = 0;
    int64_t startDateTime = 0;
    int64_t endDateTime = 0;
    int count = 0;
    int numofRights = 0;
    int64_t tmp_startDate = 0;
    int64_t tmp_endDate   = 0;
    int64_t tmp_startTime = 0;
    int64_t tmp_endTime   = 0;

    ALOGV("getConstraintsAsString, result=%d, num=%d, action=%d", result, num, action);
    if(DRM_SUCCESS == result && num > 0) {
        T_DRM_Rights_Info_Node* rightsInfoNode = NULL;
        SVC_drm_getContentRightsList(sessionId, &rightsInfoNode);

        T_DRM_Rights_Info_Node *curNode = rightsInfoNode;
        ALOGV("rightsInfoNode=%x", rightsInfoNode);
        while(NULL != curNode) {
            T_DRM_Rights_Info info = curNode->roInfo;
            isUnlimited = info.bIsUnlimited;
            char unlimited[5];
            snprintf(unlimited, sizeof(unlimited), "%d", isUnlimited);
            ALOGV("unlimited = %s,%d", unlimited, isUnlimited);
            drmConstraints->put(new String8("unlimited"), unlimited);
            if(info.bIsUnlimited == 1) {
                drmConstraints->put(new String8("valid"), "1");
                return convertConstraintsToString(drmConstraints);
            }
            ALOGV("isUnlimited =%d", isUnlimited);
            switch(action) {
                case Action::PLAY:
                    constraints = info.playRights;
                    ALOGV("copying play constraints");
                    gotConstraints = true;
                    break;
                case Action::EXECUTE:
                    ALOGV("copying exce constraints");
                    constraints = info.executeRights;
                     gotConstraints = true;
                    break;
                case Action::DISPLAY:
                    ALOGV("copying display constraints,info.displayRights=%x", info.displayRights);
                    constraints = info.displayRights;
                    gotConstraints = true;
                    break;
                case Action::TRANSFER:
                case Action::DEFAULT:
                case Action::RINGTONE:
                case Action::OUTPUT:
                case Action::PREVIEW:
                default:
                    break;
            }

            ALOGV("rights object = %d", ++numofRights);
            ALOGV("count = %d", constraints.count);
            ALOGV("startDate = %d,",constraints.startDate);
            ALOGV("startTime = %d", constraints.startTime);
            ALOGV("endDate = %d",constraints.endDate);
            ALOGV("endTime = %d", constraints.endTime);
            ALOGV("intervalDate = %d", constraints.intervalDate);
            ALOGV("intervalTime = %d", constraints.intervalTime);

            count = count + constraints.count;
            if (constraints.startDate > 0 ) {
                int64_t tempStart = computeTime(constraints.startDate, constraints.startTime);
                if( startDateTime == 0) startDateTime = tempStart;
                else if ( tempStart < startDateTime)  startDateTime = tempStart;

                tmp_startDate = constraints.startDate;
                tmp_startTime = constraints.startTime;
            }
            if (constraints.endDate > 0 ) {
                int64_t tempEnd = computeTime(constraints.endDate, constraints.endTime);
                if ( tempEnd > endDateTime) endDateTime = tempEnd;

                tmp_endDate = constraints.endDate;
                tmp_endTime = constraints.endTime;
            }
            curNode = curNode->next;
        }
    }
    if(gotConstraints == true && constraints.indicator != 0) {
        char countStr[10];
        char startDateTimeStr[50];
        char endDateTimeStr[50];
        char intervalStr[50];
        char validRights[10];
        char startDate[50];
        char startTime[50];
        char endDate[50];
        char endTime[50];
        char intervalDate[50];
        char intervalTime[50];

        int64_t interval = computeInterval(constraints.intervalDate, constraints.intervalTime);
        if (interval != 0 && numofRights > 1) {
            int64_t temp = endDateTime - startDateTime;
            ALOGV("temp Interval = %lld", temp);
            if(interval < temp) interval = temp;
        }

        snprintf(validRights, sizeof(validRights), "%d", 1);
        drmConstraints->put(new String8("valid"), validRights);
        snprintf(countStr, sizeof(countStr), "%d", count);
        drmConstraints->put(new String8("count"), countStr);
        snprintf(startDateTimeStr, sizeof(startDateTimeStr), "%lld",
                startDateTime);
        drmConstraints->put(new String8("startDateTime"), startDateTimeStr);
        snprintf(endDateTimeStr, sizeof(endDateTimeStr), "%lld", endDateTime);
        drmConstraints->put(new String8("endDateTime"), endDateTimeStr);
        snprintf(intervalStr, sizeof(intervalStr), "%lld", interval);
        drmConstraints->put(new String8("interval"), intervalStr);
        ALOGV("startDateTime = %lld, endDateTime= %lld , \ninterval = %lld", startDateTime, endDateTime, interval);
        snprintf(startDate, sizeof(startDate), "%lld", tmp_startDate);
        drmConstraints->put(new String8("startDate"), startDate);
        snprintf(startTime, sizeof(startTime), "%06d", tmp_startTime);
        drmConstraints->put(new String8("startTime"), startTime);
        snprintf(endDate, sizeof(endDate), "%d", tmp_endDate);
        drmConstraints->put(new String8("endDate"), endDate);
        snprintf(endTime, sizeof(endTime), "%06d", tmp_endTime);
        drmConstraints->put(new String8("endTime"), endTime);
        snprintf(intervalDate, sizeof(intervalDate), "%d",
                constraints.intervalDate);
        drmConstraints->put(new String8("intervalDate"), intervalDate);
        snprintf(intervalTime, sizeof(intervalTime), "%d",
                constraints.intervalTime);
        drmConstraints->put(new String8("intervalTime"), intervalTime);
    } else {
        char valid[10];
        snprintf(valid, sizeof(valid), "%d", 0);
        drmConstraints->put(new String8("valid"), valid);
    }
    return convertConstraintsToString(drmConstraints);;
}

String8 OmaDrmEngine::convertConstraintsToString(DrmConstraints* drmConstraints) {
    String8 constraintsString = String8("");
    DrmConstraints::KeyIterator keyIterator = drmConstraints->keyIterator();
    ALOGI("convertConstraintsToString constraints count = %d", drmConstraints->getCount());
    while(keyIterator.hasNext()){
        String8 key = keyIterator.next();
        String8 value = drmConstraints->get(key);
        ALOGI("convertConstraintsToString: %s = %s", key.string(), value.string());
        // make a string here using key value pair
        constraintsString += key;
        constraintsString += String8("\t");
        constraintsString += value;
        constraintsString += String8("\n");
    }
    ALOGI("Final Constraints string: %s", constraintsString.string());
    return constraintsString;
}

DrmMetadata* OmaDrmEngine::onGetMetadata(int uniqueId, const String8* path) {
    DrmMetadata* drmMetadata = NULL;
    ALOGV("onGetMetadata, %s", path->string());

    if (NULL != path) {
        int result = -1;
        uint8_t rightsIssuer[256] = {0};
        int sessionId = openDrmSession(String8(path->string()));
        ALOGV("onGetMetadata,sessionId= %d",sessionId);
        uint8_t contentID[256] = {0};
        char filepath [300];
        int drmType = 0;
        if(DRM_SUCCESS == SVC_drm_getContentID(sessionId, contentID)) {
            char *newCID = replaceStr((char*)contentID);
            if(strcmp((char*)contentID,"cid:.flockro@localhost") == 0) {
                drmType = FORWARD_LOCK;
            } else {
                snprintf(filepath, sizeof(filepath), "/data/local/.Drm/%s.cd",
                        newCID);
                FILE * f1 = fopen (filepath, "r");
                if(f1 != NULL) {
                    drmType = COMBINED_DELIVERY;
                    fclose(f1);
                } else {
                    ALOGV("%s not found, checking for flsd",filepath);
                    snprintf(filepath, sizeof(filepath),
                            "/data/local/.Drm/%s.flsd", newCID);
                    FILE * f2 = fopen (filepath, "r");
                    if(f2 != NULL) {
                        drmType = SEPARATE_DELIVERY_FL;
                        fclose(f2);
                    } else {
                        ALOGV("%s not found, checking for sd",filepath);
                            drmType = SEPARATE_DELIVERY;
                    }
                }
            }
            if(newCID != NULL) free(newCID);
            newCID = NULL;
        }
        result = SVC_drm_getRightsIssuer(sessionId,rightsIssuer);
        ALOGV("onGetMetadata,sessionId= %s",(char*)rightsIssuer);
        SVC_drm_closeSession(sessionId);
        drmMetadata = new DrmMetadata();
        String8 dtkey = String8("DRM-TYPE");
        char type[3];
        snprintf(type, sizeof(type), "%d", drmType);
        drmMetadata->put(&dtkey,  (const char*)type);
        String8 ri = String8("Rights-Issuer");
        drmMetadata->put(&ri,  (const char*)rightsIssuer);           
    }

    return drmMetadata;
}

String8 OmaDrmEngine::getMetadataAsString(int uniqueId, const int sessionId) {
    DrmMetadata* drmMetadata = NULL;
    String8 metadata = String8("");

    char data[356] ;

    ALOGV("getMetadataAsString, sessionId = %d", sessionId);

    if (sessionId != -1) {
        int result = -1;
        uint8_t rightsIssuer[256] = {0};
        ALOGV("getMetadataAsString, sessionId= %d",sessionId);
        uint8_t contentID[256] = {0};
        char filepath [300];
        int drmType = 0;
        if(DRM_SUCCESS == SVC_drm_getContentID(sessionId, contentID)) {
            ALOGI("Got contentId  = %s", (char *) contentID);
            char *newCID = replaceStr((char*)contentID);
            if(strcmp((char*)contentID,"cid:.flockro@localhost") == 0) {
                ALOGI("Found Forward Lock DRM file");
                drmType = FORWARD_LOCK;
            } else {
                //snprintf(filepath,"/data/local/.Drm/%s.cd",(char*)contentID);
                snprintf(filepath, sizeof(filepath), "/data/local/.Drm/%s.cd",
                        newCID);
                FILE * f1 = fopen (filepath, "r");
                if(f1 != NULL) {
                     ALOGI("Found Combined Delivery DRM file");
                    drmType = COMBINED_DELIVERY;
                    fclose(f1);
                } else {
                    ALOGV("%s not found, checking for flsd", filepath);
                    snprintf(filepath, sizeof(filepath),
                            "/data/local/.Drm/%s.flsd", newCID);
                    FILE * f2 = fopen (filepath, "r");
                    if(f2 != NULL) {
                        ALOGI("Found FLSD DRM file");
                        drmType = SEPARATE_DELIVERY_FL;
                        fclose(f2);
                    } else {
                        ALOGV("%s not found, checking for sd", filepath);
                        ALOGI("Found SD DRM file");
                            drmType = SEPARATE_DELIVERY;
                    }
                }
            }
            if(newCID != NULL) free(newCID);
            newCID = NULL;
        }
        result = SVC_drm_getRightsIssuer(sessionId, rightsIssuer);
        snprintf(data, sizeof(data), "DRM-TYPE\t%d\nRights-Issuer\t%s", drmType,
                (char*) rightsIssuer);
        metadata = String8((char *) data);
        ALOGV("getMetadataAsString, sessionId = %d, metadata = %s", sessionId,metadata.string());
    }

    return metadata;
}


android::status_t OmaDrmEngine::onInitialize(int uniqueId) {
    ALOGV("onInitialize");

    return DRM_NO_ERROR;
}

android::status_t
OmaDrmEngine::onSetOnInfoListener(int uniqueId, const IDrmEngine::OnInfoListener* infoListener) {
    // Not used
    ALOGV("onSetOnInfoListener");

    return DRM_NO_ERROR;
}

android::status_t OmaDrmEngine::onTerminate(int uniqueId) {
    ALOGV("onTerminate");

    return DRM_NO_ERROR;
}

DrmSupportInfo* OmaDrmEngine::onGetSupportInfo(int uniqueId) {
    DrmSupportInfo* pSupportInfo = new DrmSupportInfo();

    ALOGV("onGetSupportInfo");

    // fill all Forward Lock mimetypes and extensions
    if (NULL != pSupportInfo) {
        pSupportInfo->addFileSuffix(String8(DRM_DOTEXTENSION_FL));
        pSupportInfo->addFileSuffix(String8(DRM_DOTEXTENSION_DCF));        
        pSupportInfo->addFileSuffix(String8(DRM_DOTEXTENSION_DM));

        pSupportInfo->addMimeType(String8(DRM_MIMETYPE_MESSAGE));
        pSupportInfo->addMimeType(String8(DRM_MIMETYPE_CONTENT));
        pSupportInfo->addMimeType(String8(DRM_MIMETYPE_RIGHTS_XML));
        pSupportInfo->addMimeType(String8(DRM_MIMETYPE_RIGHTS_WXML));

        pSupportInfo->setDescription(String8(DRM_DESCRIPTION));
    }

    return pSupportInfo;
}

bool OmaDrmEngine::onCanHandle(int uniqueId, const String8& path) {
    bool result = false;
    String8 extString = path.getPathExtension();
    extString.toLower();

    if ((extString == String8(DRM_DOTEXTENSION_DCF)) ||
        (extString == String8(DRM_DOTEXTENSION_DM))  ||
        (extString == String8(DRM_DOTEXTENSION_FL))) {
        result = true;
    }
    return result;
}

DrmInfoStatus* OmaDrmEngine::onProcessDrmInfo(int uniqueId, const DrmInfo* drmInfo) {
    DrmInfoStatus *drmInfoStatus = NULL;

    // Nothing to process

    drmInfoStatus = new DrmInfoStatus((int)DrmInfoStatus::STATUS_OK, 0, NULL, String8(""));

    ALOGV("onProcessDrmInfo");

    return drmInfoStatus;
}

status_t OmaDrmEngine::onSaveRights(
            int uniqueId,
            const DrmRights& drmRights,
            const String8& rightsPath,
            const String8& contentPath) {
    // install the rights.
    ALOGV("onSaveRights with rightsPath= %s", rightsPath.string());

    DrmBuffer buff = drmRights.getData();
    ALOGI("Rights data = %s", buff.data);
    char tempPath[50];
    memset(&tempPath[0], '\0', sizeof(tempPath));
    snprintf(tempPath, sizeof(tempPath), "/data/local/.Drm/drmrights.temp");

    FILE * fileRO= fopen(tempPath,"w");
    fwrite(buff.data, sizeof(char), buff.length, fileRO);
    fclose(fileRO);

    int32_t id;
    T_DRM_Input_Data inData;
    T_DRM_Rights_Info rightsInfo;
    FILE * infile = fopen(tempPath, "rb");
    if (infile == NULL) {
        ALOGE("can't open %s", tempPath);
        remove(tempPath);
        return DRM_ERROR_UNKNOWN;
    }

    String8 mimetype = drmRights.getMimeType();
    memset(&inData, 0, sizeof(inData));
    inData.inputHandle = (int64_t)infile;
    ALOGV("saverights= mime=%s",mimetype.string());
    if(mimetype.compare(String8("application/vnd.oma.drm.rights+xml")) == 0) {
        inData.mimeType = TYPE_DRM_RIGHTS_XML;
        ALOGV("installing TYPE_DRM_RIGHTS_XML");
    } else {
        inData.mimeType = TYPE_DRM_RIGHTS_WBXML;
         ALOGV("installing TYPE_DRM_RIGHTS_WBXML");
    }
    inData.getInputDataLength = getLen;
    inData.readInputData = readFunc;

    memset(&rightsInfo, 0, sizeof(T_DRM_Rights_Info));

    if (DRM_FAILURE == SVC_drm_installRights(inData, &rightsInfo)){
        ALOGE("Failed to SVC_drm_installRights with result\n");
        fclose(infile);
        remove(tempPath);
        return DRM_ERROR_UNKNOWN;
    }
    fclose(infile); //0024370 -- Resource leak
    remove(tempPath);
    ALOGV("onSaveRights success!!!!!!!!!!!!!");
    return DRM_NO_ERROR;
}

DrmInfo* OmaDrmEngine::onAcquireDrmInfo(int uniqueId, const DrmInfoRequest* drmInfoRequest) {
    DrmInfo* drmInfo = NULL;

    // Nothing to be done for Forward Lock file
    ALOGV("onAcquireDrmInfo");

    return drmInfo;
}

int OmaDrmEngine::onCheckRightsStatus(int uniqueId,
                                       const String8& path,
                                       int action) {
    int result = RightsStatus::RIGHTS_INVALID;

    ALOGV("onCheckRightsStatus for path =%s &action=%d", path.string(), action);

    // Only Transfer action is not allowed for forward Lock files.
    if (onCanHandle(uniqueId, path)) {
        switch(action) {
            case Action::DEFAULT:
            case Action::PLAY:
            case Action::RINGTONE:
            case Action::OUTPUT:
            case Action::PREVIEW:
            case Action::EXECUTE:
            case Action::DISPLAY:
                result = RightsStatus::RIGHTS_VALID;
                break;

            case Action::TRANSFER:
            default:
                result = RightsStatus::RIGHTS_INVALID;
                break;
        }
    }

    return result;
}

int OmaDrmEngine::getRightsStatus(int uniqueId, const int sessionId,
                                       int action) {
    int result = RightsStatus::RIGHTS_INVALID;

    ALOGV("getRightsStatusAsString for sessionId =%d &action=%d", sessionId, action);
    int drm_action;
    switch(action) {
        case Action::PLAY:
            drm_action = DRM_PERMISSION_PLAY;
            break;
        case Action::EXECUTE:
            drm_action = DRM_PERMISSION_EXECUTE;
            break;
        case Action::DISPLAY:
            drm_action = DRM_PERMISSION_DISPLAY;
            break;
        case Action::TRANSFER:
            drm_action = DRM_PERMISSION_FORWARD;
            break;
        case Action::DEFAULT:
        case Action::RINGTONE:
        case Action::OUTPUT:
        case Action::PREVIEW:
        default:
            result = RightsStatus::RIGHTS_INVALID;
            return result;
    }
    int status = SVC_drm_checkRights(sessionId, drm_action);
    if(DRM_SUCCESS == status) {
        ALOGV("RightsStatus::RIGHTS_VALID");
        result = RightsStatus::RIGHTS_VALID;
    } else {
        ALOGV("RightsStatus::RIGHTS_EXPIRED");
        result = RightsStatus::RIGHTS_EXPIRED;
    }
    return result;
}

status_t OmaDrmEngine::onConsumeRights(int uniqueId,
                                        DecryptHandle* decryptHandle,
                                        int action,
                                        bool reserve) {
    ALOGV("onConsumeRights start called");
    if(decryptHandle == NULL) {
        ALOGV("Could not consume rights as decryptHandle is NULL");
        return DRM_ERROR_DECRYPT_UNIT_NOT_INITIALIZED;
    }

    int drm_action;
    status_t result = DRM_ERROR_UNKNOWN;
    switch(action) {
        case Action::PLAY:
            drm_action = DRM_PERMISSION_PLAY;
            break;        
        case Action::EXECUTE:
            drm_action = DRM_PERMISSION_EXECUTE;
            break;
        case Action::DISPLAY:
            drm_action = DRM_PERMISSION_DISPLAY;
            break;
        case Action::TRANSFER:
            drm_action = DRM_PERMISSION_FORWARD;
            break;
        case Action::DEFAULT:
        case Action::RINGTONE:
        case Action::OUTPUT:
        case Action::PREVIEW:
        default:
            ALOGV("Could not find valide action. action = %d ", action);
            return DRM_ERROR_CANNOT_HANDLE;
    }

    int count = decryptHandle->extendedData.size();
    ALOGV("extendedData size = %d", count);
    if(count < 1) {
        ALOGV("Could not consume rights. extendedData should not be empty.");
        return DRM_ERROR_SESSION_NOT_OPENED;
    }
    String8 fdstr = decryptHandle->extendedData.valueFor(String8("fd"));
    int fileDesc = atoi(fdstr.string());
    ALOGV("onConsumeRights %d -> ####%s####", fileDesc,fdstr.string());

     if (fileDesc < 0) {
        ALOGE("onConsumeRights->invalid file decsriptor");
        return result;
    }

    FILE * infile;
    int ret;
    int id = 0;
    char *buf;
    int readBytes = 0;
    int buflen = 0;
    uint8_t contentType[64] = {0};
    uint8_t mime[64] = {0};

    infile = fdopen(fileDesc, "r");   
    if (infile == NULL) {
        ALOGE("Error opening file: %s.\n", strerror(errno));
        if(fileDesc > -1) {::close(fileDesc);}
        return result;
    }
    fseek (infile , 0 , SEEK_SET);

    T_DRM_Input_Data inData;
    memset(&inData, 0, sizeof(inData));

    inData.inputHandle = (int64_t)infile;
    inData.mimeType = TYPE_DRM_CONTENT;
    inData.getInputDataLength = getLen;
    inData.readInputData = readFunc;
    inData.seekInputData = seekFunc;
    id = SVC_drm_openSession(inData);
    ALOGV("Session id in ods= %d", id);
    if(id < 0) {
        fclose(infile);
        return result;
    }
	int status = SVC_drm_consumeRights(id, drm_action);
    if(DRM_SUCCESS == status) {
        ALOGV("y:onConsumeRights success");
        result = DRM_NO_ERROR;
    } else {
        ALOGE("y:onConsumeRights Error");
        result = DRM_ERROR_LICENSE_EXPIRED;
    }
    SVC_drm_closeSession(id);
    fclose(infile);
    return result;
}

bool OmaDrmEngine::onValidateAction(int uniqueId,
                                     const String8& path,
                                     int action,
                                     const ActionDescription& description) {
    ALOGV("onValidateAction");

    // For the forwardlock engine checkRights and ValidateAction are the same.
    return (onCheckRightsStatus(uniqueId, path, action) == RightsStatus::RIGHTS_VALID);
}

String8 OmaDrmEngine::onGetOriginalMimeType(int uniqueId, const String8& path, int fd) {
    ALOGV("onGetOriginalMimeType,path=%s", path.string());

    char* str = (char*) path.string();
    char  actiontype[15];
    int size = -1;
    char  actionString[2];
    int action = -1;

    memset(actiontype, '\0', sizeof(actiontype));
    memset(actionString, '\0', 2);

    char* pch = strchr(str, ':');
    if(pch != NULL){
        size = pch - str;
    }

    if (size > 0){
        ALOGI("Found action string on the path! parsing for actiontype and action");
        strncpy(actiontype, str, size - 1);
        if(strcmp(actiontype,"rights") == 0 ||
            strcmp(actiontype,"constraints") == 0 ){
            strncpy(actionString, str + (size - 1), 1);
            action = atoi(actionString);
        }

        ALOGV(" actiontype = %s, action = %d", actiontype, action);
    }

    String8 mimeString = String8("");
    FILE * infile;
    int ret;
    int id = 0;
    uint8_t contentType[64] = {0};
    uint8_t mime[64] = {0};

    if( fd < 0) {
        ALOGI("Opening file using file path, path = %s", path.string());
        infile = fopen(path, "rb");
        if (infile == NULL) {
            ALOGE("Error opening file: %s.\n", strerror(errno));
        	ALOGE("Can't open file in read mode, errno=%d", errno);
             return mimeString;
        }
    } else {
        ALOGI("Opening file using file Descripter, fd = %d", fd);
        infile = fdopen(fd, "r");
        if (infile == NULL) {
            ALOGE("Error opening file: %s.\n", strerror(errno));
            ALOGE("Unable  to open file in read mode, fd is %d", fd);
            return mimeString; 
        }
        fseek (infile , 0 , SEEK_SET);      
    }

    if (infile != NULL) {
        ALOGI("Opening the file successful.");
    }

    T_DRM_Input_Data inData;
    memset(&inData, 0, sizeof(inData));
    inData.inputHandle = (int64_t)infile;
    inData.mimeType = TYPE_DRM_CONTENT;
    inData.getInputDataLength = getLen;
    inData.readInputData = readFunc;
    inData.seekInputData = seekFunc;
    id = SVC_drm_openSession(inData);
    if (id < 0){
        ALOGV("id is < 0 ");
        return String8("");
    }

    // Hack to get metadata right status and constrants
    if(strcmp(actiontype, "metadata") == 0) {
        ALOGI("call getMetadataAsString with session id = %d", id);
        String8 metadata = getMetadataAsString(uniqueId, id);
        SVC_drm_closeSession(id);
        fclose(infile);
        return metadata;
    }

    if(strcmp(actiontype, "rights") == 0 && action > 0) {
        ALOGI("call getRightsStatus of action = %d", action);
        int status = getRightsStatus(uniqueId, id, action);
        SVC_drm_closeSession(id);
        fclose(infile);
        char rights[10];
        snprintf(rights, sizeof(rights), "%d", status);
        return String8(rights);
    }

    if(strcmp(actiontype, "constraints") == 0 && action > 0) {
        ALOGI("call getConstraints of action = %d", action);
        int status = getRightsStatus(uniqueId, id, action);
        String8 constraints = getConstraintsAsString(uniqueId, id, action);
        SVC_drm_closeSession(id);
        fclose(infile);
        return constraints;
    }

    int32_t status = SVC_drm_getContentType(id, contentType);
    if(status < 0){
        ALOGV("onGetOriginalMimeType , failed to get the conent type");
    }
    strcpy((char *)mime, (char *)contentType);
    mimeString = String8((char *)mime);

    ALOGV("onGetOriginalMimeType----------%s", mimeString.string());

    SVC_drm_closeSession(id);
    fclose(infile);
    return mimeString;
}

int OmaDrmEngine::onGetDrmObjectType(int uniqueId,
                                      const String8& path,
                                      const String8& mimeType) {
    String8 mimeStr = String8(mimeType);

    ALOGV("onGetDrmObjectType");

    mimeStr.toLower();

    /* Checks whether
    * 1. path and mime type both are not empty strings (meaning unavailable) else content is unknown
    * 2. if one of them is empty string and if other is known then its a DRM Content Object.
    * 3. if both of them are available, then both may be of known type
    *    (regardless of the relation between them to make it compatible with other DRM Engines)
    */
    if (((0 == path.length()) || onCanHandle(uniqueId, path)) &&
        ((0 == mimeType.length()) ||
        (mimeStr == String8(DRM_MIMETYPE_MESSAGE))) && (mimeType != path) ) {
            return DrmObjectType::CONTENT;
    }

    return DrmObjectType::UNKNOWN;
}

status_t OmaDrmEngine::onRemoveRights(int uniqueId, const String8& path) {
    // No Rights to remove
    ALOGV("onRemoveRights");
    return DRM_NO_ERROR;
}

status_t OmaDrmEngine::onRemoveAllRights(int uniqueId) {
    // No rights to remove
    ALOGV("onRemoveAllRights");
    return DRM_NO_ERROR;
}

#ifdef USE_64BIT_DRM_API
status_t OmaDrmEngine::onSetPlaybackStatus(int uniqueId, DecryptHandle* decryptHandle,
                                            int playbackStatus, int64_t position) {
#else
status_t OmaDrmEngine::onSetPlaybackStatus(int uniqueId, DecryptHandle* decryptHandle,
                                            int playbackStatus, int position) {
#endif
    ALOGV("onSetPlaybackStatus : status = %d", playbackStatus);
    mPlaybackStatus = playbackStatus;
    return DRM_NO_ERROR;
}

status_t OmaDrmEngine::onOpenConvertSession(int uniqueId,
                                         int convertId) {
    status_t result = DRM_ERROR_UNKNOWN;
    ALOGV("onOpenConvertSession");

    if (!convertSessionMap.isCreated(convertId)) {
        ConvertSession *newSession = new ConvertSession();
        int rnum = rand();
        ALOGV("onOpenConvertSession: rnum= %d", rnum);
        char tempPath[50];
        memset(&tempPath[0], '\0', sizeof(tempPath));
        snprintf(tempPath, sizeof(tempPath), "/data/local/.Drm/cs%d.temp", rnum);
        FILE* file = fopen(tempPath, "w");
        if (file != NULL) {
            newSession->setPath(tempPath);
            ALOGE("onOpenConvertSession, filePath=%s, convertId=%d, tempPath=%s",newSession->filePath,convertId,tempPath);
            convertSessionMap.addValue(convertId, newSession);
            fclose(file);
            result = DRM_NO_ERROR;
            if (!convertSessionMap.isCreated(convertId)) {
                ALOGE("error-----session not created");
            } else {
                ALOGE("----session creation sucess----");
                int smapSize = convertSessionMap.getSize();
                ALOGE("onOpenConvertSession: convertSessionMap size = %d", smapSize);
            }
        } else {
            ALOGE("onOpenConvertSession failed.");
            delete newSession;
        }
    }
    return result;
}

DrmConvertedStatus* OmaDrmEngine::onConvertData(int uniqueId,
                                                 int convertId,
                                                 const DrmBuffer* inputData) {
    DrmBuffer* convResult = new DrmBuffer(NULL, 0);
    int offset = 0;
    FILE* file = NULL;
    ConvertSession *convSession = NULL;

    if (convertSessionMap.isCreated(convertId)) {
        convSession = convertSessionMap.getValue(convertId);        
    }
    if(convSession == NULL) {
        ALOGE("session not created for %d", convertId);
        return new DrmConvertedStatus(DrmConvertedStatus::STATUS_ERROR, convResult, offset);
    }

    const char *tempfilepath = convSession->filePath;
    if(tempfilepath != NULL) {
        file = fopen(tempfilepath ,"a");
    }
    if(tempfilepath == NULL || file == NULL){
        ALOGE("file is null");
        return new DrmConvertedStatus(DrmConvertedStatus::STATUS_ERROR, convResult, offset);
    }
    fwrite(inputData->data, sizeof(char), inputData->length, file);
    fclose(file);

    return new DrmConvertedStatus(DrmConvertedStatus::STATUS_OK, convResult, offset);
}

DrmConvertedStatus* OmaDrmEngine::onCloseConvertSession(int uniqueId,int convertId) {
    ALOGV("onCloseConvertSession start");
    DrmBuffer *convResult = new DrmBuffer(NULL, 0);
    const char *tempfilepath = NULL;
    int offset = 0;
    int ret = -1;

    if (convertSessionMap.isCreated(convertId)) {
        ConvertSession *convSession = convertSessionMap.getValue(convertId);
        tempfilepath = convSession->filePath;
    }
    else {
        ALOGE("Invalide convertId");
        return new DrmConvertedStatus(DrmConvertedStatus::STATUS_ERROR, convResult, offset);
    }

    const char  *mime = "application/vnd.oma.drm.message";
    if(tempfilepath != NULL) {
        ALOGV("Install drm object,  tempfilepath = %s", tempfilepath);
        ret = SVC_drm_installDRMObject((const uint8_t*)tempfilepath ,(const uint8_t*)mime);
    }

    ALOGV("SVC_drm_installDRMObject returned %d", ret);
    
    if(tempfilepath == NULL || ret < 0) {
        ALOGE("Unable to install drm file, ret = %d", ret);
        return new DrmConvertedStatus(DrmConvertedStatus::STATUS_ERROR, convResult, offset);
    }

    FILE* infile = fopen ( tempfilepath, "rb" );
    T_DRM_Input_Data inData;
    memset(&inData, 0, sizeof(inData));
    inData.inputHandle = (int64_t)infile;
    inData.mimeType = TYPE_DRM_CONTENT;
    inData.getInputDataLength = getLen;
    inData.readInputData = readFunc;
    inData.seekInputData = seekFunc;
    int id = SVC_drm_openSession(inData);
    ALOGV("SVC drm open session afterSVC_drm_installDRMObject =%d", id);
    uint8_t contentID[256] = {0};
    char filepath [300];
    bool createFile = false;
    int err = SVC_drm_getContentID(id, contentID);
    SVC_drm_closeSession(id);
    fclose (infile); //0024370 -- Resource Leak
    if (DRM_SUCCESS == err) {
        char *newCID = replaceStr((char*)contentID);
        switch(ret) {
            case COMBINED_DELIVERY:                   
                    //snprintf(filepath,"/data/local/.Drm/%s.cd",(char*)contentID);
                    snprintf(filepath, sizeof(filepath), "/data/local/.Drm/%s.cd",
                            newCID);
                    createFile = true;
                    ALOGV("COMBINED_DELIVERY :cid+.cd=%s", filepath);
                break;
            case SEPARATE_DELIVERY:
                    //snprintf(filepath,"/data/local/.Drm/%s.sd",(char*)contentID);
                    snprintf(filepath, sizeof(filepath), "/data/local/.Drm/%s.sd",newCID);
                    createFile = true;
                    ALOGV("SEPARATE_DELIVERY :cid+.sd=%s", filepath);
                break;
            case SEPARATE_DELIVERY_FL:
                    //snprintf(filepath,"/data/local/.Drm/%s.flsd",(char*)contentID);
                    snprintf(filepath, sizeof(filepath), "/data/local/.Drm/%s.flsd",newCID);
                    createFile = true;
                    ALOGV("SEPARATE_DELIVERY_FL:cid+.flsd=%s", filepath);
                break;
            default:
                break;
        }
        if(newCID) free(newCID);
        newCID = NULL;
        if(createFile) {
             FILE* f = fopen (filepath, "w");
             if(f != NULL) fclose(f);
        }
    }
    // now write the encrypted data from temp to convResult
    FILE * pFile;
    long lSize;
    //char * buffer;
    size_t result;
    pFile = fopen ( tempfilepath, "r" );
    if (pFile==NULL) {
       ALOGE("File error");
    }

    // obtain file size:
    long prev_pos = ftell(pFile);

    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    fseek(pFile, prev_pos, SEEK_SET);

    ALOGV("--------file size ==%d",lSize); 

    //copy from buffer to convResult  
    int len = strlen(tempfilepath);
    convResult->data = (char*) malloc (sizeof(char)*len);    
    //memcpy(convResult->data, (char *)buffer, result);
    convResult->length = len;
    strcpy(convResult->data, tempfilepath);
    ALOGV("convResult->data=%s, filePath=%s",convResult->data, tempfilepath);
    fclose (pFile);
    //free(buffer);
    ALOGV("--------before returning from close convertion------------"); 

    //Temporary, but not against the security breach 
    char cmdStr[100];
    snprintf(cmdStr, sizeof(cmdStr), "chmod 777 %s", tempfilepath);
    ALOGV("--------cmd=(%s)------------",cmdStr);
    system(cmdStr); 

    if (convertSessionMap.isCreated(convertId)) {
        convertSessionMap.removeValue(convertId);
    }

    return new DrmConvertedStatus(DrmConvertedStatus::STATUS_OK, convResult, offset);
}

#ifdef USE_64BIT_DRM_API
status_t OmaDrmEngine::onOpenDecryptSession(int uniqueId,
                                             DecryptHandle* decryptHandle,
                                             int fd,
                                             off64_t offset,
                                             off64_t length,
                                             const char* mime) {
#else
status_t OmaDrmEngine::onOpenDecryptSession(int uniqueId,
                                             DecryptHandle* decryptHandle,
                                             int fd,
                                             int offset,
                                             int length,
                                             const char* mime) {
#endif
    return onOpenDecryptSession(uniqueId, decryptHandle, fd, offset, length);
}    

#ifdef USE_64BIT_DRM_API
status_t OmaDrmEngine::onOpenDecryptSession(int uniqueId,
                                             DecryptHandle* decryptHandle,
                                             int fd,
                                             off64_t offset,
                                             off64_t length) {
#else
status_t OmaDrmEngine::onOpenDecryptSession(int uniqueId,
                                             DecryptHandle* decryptHandle,
                                             int fd,
                                             int offset,
                                             int length) {
#endif
    ALOGV("onOpenDecryptSession with fd=%d",fd); 
    mPlaybackStatus = -1;  // reset the playback status
    status_t result = DRM_ERROR_CANNOT_HANDLE;
    int fileDesc = -1;
    int newFd = -1;
    if ((-1 < fd) && (NULL != decryptHandle)) {
        fileDesc = dup(fd);
        ALOGV("fileDesc =%d,fd=%d", fileDesc, fd);
    } else {
        ALOGE("onOpenDecryptSession parameter error");
        return result;
    }

    FILE * infile = NULL;
    int err = -1;
    int id = 0;
    char *buf = NULL;    
    int readBytes = 0;
    int buflen = 0;
    uint8_t contentType[64] = {0};

    infile = fdopen(fileDesc, "r");
    if (infile == NULL) {
        ALOGE("Error opening file: %s.\n", strerror(errno));
        ALOGE("unable  to open file in read mode, fd is %d",fileDesc);
        if(-1 < fileDesc) {::close(fileDesc);}
        return result;
    }

    fseek (infile , 0 , SEEK_SET);    

    T_DRM_Input_Data inData;
    memset(&inData, 0, sizeof(inData));
    inData.inputHandle = (int64_t)infile;
    inData.mimeType = TYPE_DRM_CONTENT;
    inData.getInputDataLength = getLen;
    inData.readInputData = readFunc;
    inData.seekInputData = seekFunc;

    id = SVC_drm_openSession(inData);
    ALOGV("SVC_drm_openSession returned id = %d",id);

    if(id < 0 ) {
        ALOGE("SVC_drm_openSession error, leaving....");
        fclose(infile);
         if(-1 < fileDesc) {::close(fileDesc);}
        return result;
    }

    buflen = SVC_drm_getContentLength(id);
    buf = (char *) malloc(buflen * sizeof(char));
    ALOGV("SVC_drm_getContentLength returned! buflen=%d, buf = %s", buflen, buf);
    readBytes = SVC_drm_getContent(id, 0, (uint8_t*) buf, buflen);
    ALOGV(" SVC_drm_getContent returned readBytes = %d",readBytes);
    if(readBytes < 0) {
        SVC_drm_closeSession(id);
        fclose(infile);
        ALOGE("Opendecrypt session: unable to decrypt");
         if(-1 < fileDesc) {::close(fileDesc);}
        return DRM_ERROR_DECRYPT;
    }
    err = SVC_drm_getContentType(id, contentType);
    if(err < 0) {
        ALOGE("failed to get the content type");
    }
    String8 mimeString = String8((char *)contentType);
    decryptHandle->mimeType = mimeString;
    decryptHandle->decryptApiType = DecryptApiType::CONTAINER_BASED;
    int status = DRM_FAILURE;
    if ((strncmp((char *)contentType, "audio/", 6) == 0) || (strncmp((char *)contentType, "video/", 6) == 0)
        || (strncmp((char *)contentType, "application/ogg", 15) == 0)  ) {
        status = SVC_drm_checkRights(id, DRM_PERMISSION_PLAY);
    }
    else if (strncmp((char *)contentType, "image/", 6) == 0) {
        status = SVC_drm_checkRights(id, DRM_PERMISSION_DISPLAY);
    }
    ALOGV("onOpenDecryptSession with mimetype = %s", mimeString.string()); 
    if(DRM_SUCCESS == status) {
        decryptHandle->status = RightsStatus::RIGHTS_VALID;
         ALOGV("onOpenDecryptSession RightsStatus::RIGHTS_VALID"); 
    } else {
        decryptHandle->status = RightsStatus::RIGHTS_EXPIRED;
        ALOGV("onOpenDecryptSession RightsStatus::RIGHTS_EXPIRED"); 
    }
    SVC_drm_closeSession(id);
    fclose(infile); 
    //write the decrypted data to file
    nsecs_t time = systemTime();
    int rnum = rand();
    ALOGV("random num= %d", rnum);
    FILE * decryptedData;
    char tempPath[50];
    //snprintf(tempPath,"/data/drm/%d%d.temp",rnum,fd);
    snprintf(tempPath, sizeof(tempPath), "/data/drm/%lld.temp",time);
    ALOGV("tempPath = %s", tempPath);
    decryptedData = fopen(tempPath,"w");
    if(decryptedData == NULL) {
        if(-1 < fileDesc) {::close(fileDesc);}
        ALOGE("unable to open temp file for writing");
        return result;
    }
    fwrite(buf, sizeof(char), readBytes, decryptedData);
    ALOGV("temp data write complete");
    fclose(decryptedData);

    //Temporary, but not against the security breach 
    char command[64];
    memset(command,'\0',64);
    snprintf(command, sizeof(command), "chmod 777 %s", tempPath);
    system(command);

    if(-1 < fileDesc) {::close(fileDesc);}
    if(buf) free(buf);
    buf = NULL;

    result = DRM_NO_ERROR;
    decryptHandle->decryptInfo = new DecryptInfo();
    decryptHandle->decryptInfo->decryptBufferLength = readBytes;
    ALOGV("calculated decrypted buffer length");
    //dup fd for use in onconsumerights. 
    //This should be closed in onconsumerights or in closedecrypt session
    newFd = dup(fd);
    ALOGV("newFd = %d", newFd);
    ALOGV("fd = %d", fd);
    char buffer [10];    
    snprintf(buffer, sizeof(buffer), "%d", newFd);
    //newFd -- Can't be closed here. It'll get closed in closeDecryptSession.
    decryptHandle->extendedData.add(String8("fd"),String8(buffer));
    decryptHandle->extendedData.add(String8("path"),String8(tempPath));
    ALOGV("Opendecrypt complete");
    return result;
}

status_t OmaDrmEngine::onOpenDecryptSession(int uniqueId,
                                             DecryptHandle* decryptHandle,
                                             const char* uri) {
    status_t result = DRM_ERROR_CANNOT_HANDLE;
    const char fileTag [] = "file://";
    mPlaybackStatus = -1;  // reset the playback status
    ALOGV("onOpenDecryptSession with URI = %s",uri); 

    FILE * infile;
    int err = -1;
    int id = 0;
    char *buf = NULL;  
    int readBytes = 0;
    int buflen = 0;
    uint8_t contentType[64] = {0};

    infile = fopen(uri, "r");
    if (infile == NULL) {
        ALOGE("Error opening file: %s.\n", strerror(errno));
         ALOGE("unable to open %s in read mode",uri);
         return result;
    }

    fseek (infile , 0 , SEEK_SET);

    T_DRM_Input_Data inData;
    memset(&inData, 0, sizeof(inData));

    inData.inputHandle = (int64_t)infile;
    inData.mimeType = TYPE_DRM_CONTENT;
    inData.getInputDataLength = getLen;
    inData.readInputData = readFunc;
    inData.seekInputData = seekFunc;
    id = SVC_drm_openSession(inData);

    ALOGV("SVC_drm_openSession returned id = %d",id);
    if(id < 0) 
    {
        fclose(infile);
        return result;
    }

    buflen = SVC_drm_getContentLength(id);
    buf = (char *) malloc(buflen*sizeof(char));
    readBytes = SVC_drm_getContent(id, 0, (uint8_t*)buf, buflen);
    ALOGV(" SVC_drm_getContent returned readBytes = %d",readBytes);    

    if(readBytes <= 0){
        ALOGE("readBytes <=0");        
        SVC_drm_closeSession(id);
        fclose(infile);
        return result;    
    }

    err = SVC_drm_getContentType(id, contentType);
    if(err < 0) {
        ALOGE("failed to get the content type");
    }
    String8 mimeString = String8((char *)contentType);
    decryptHandle->mimeType = mimeString;
    decryptHandle->decryptApiType = DecryptApiType::CONTAINER_BASED;
    int status = DRM_FAILURE;
    if ((strncmp((char *)contentType, "audio/", 6) == 0) || (strncmp((char *)contentType, "video/", 6) == 0)) {
        status = SVC_drm_checkRights(id, DRM_PERMISSION_PLAY);
    }
    else if (strncmp((char *)contentType, "image/", 6) == 0) {
        status = SVC_drm_checkRights(id, DRM_PERMISSION_DISPLAY);
    }
    ALOGV("onOpenDecryptSession with mime = %s", mimeString.string()); 
    if(DRM_SUCCESS == status) {
        decryptHandle->status = RightsStatus::RIGHTS_VALID;
         ALOGV("onOpenDecryptSession RightsStatus::RIGHTS_VALID"); 
    } else {
        decryptHandle->status = RightsStatus::RIGHTS_EXPIRED;
        ALOGV("onOpenDecryptSession RightsStatus::RIGHTS_EXPIRED"); 
    }
    SVC_drm_closeSession(id);
    fclose(infile);
    
    //write the decrypted data to file
    FILE * decryptedData;
    decryptedData = fopen("/data/drm/tempDecryptedData","w");
    if(decryptedData == NULL){
        ALOGE("unable to open temp file for writing");
        return result;
    }
    fwrite(buf, sizeof(char), readBytes, decryptedData);
    fclose(decryptedData);
    if(buf) free(buf);
    buf = NULL;
    result = DRM_NO_ERROR;
    decryptHandle->decryptInfo = new DecryptInfo();
    decryptHandle->decryptInfo->decryptBufferLength = readBytes;

    ALOGV("onOpenDecryptSession Exit. result = %d", result);
    return result;
}

status_t OmaDrmEngine::onCloseDecryptSession(int uniqueId,
                                              DecryptHandle* decryptHandle) {
    ALOGV("onCloseDecryptSession start");

    if (NULL != decryptHandle) {
        int count = decryptHandle->extendedData.size();
        ALOGV("onCloseDecryptSession: count = %d and mPlaybackStatus = %d, mimeType = %s", count, mPlaybackStatus, decryptHandle->mimeType.string());
        if(count >=1) {
            if(mPlaybackStatus == Playback::STOP) {
                if ((strncmp(decryptHandle->mimeType, "audio/", 6) == 0)
                        || (strncmp(decryptHandle->mimeType, "video/", 6) == 0)) {
                    onConsumeRights(uniqueId, decryptHandle, Action::PLAY, true);
                    ALOGV("consumed PLAY license");
                } else if (strncmp(decryptHandle->mimeType, "image/", 6) == 0) {
                    onConsumeRights(uniqueId, decryptHandle, Action::DISPLAY, true);
                    ALOGV("consumed DISPLAY license");
                }
            }
        }

        if (NULL != decryptHandle->decryptInfo) {
            delete decryptHandle->decryptInfo;
            decryptHandle->decryptInfo = NULL;
        }

        decryptHandle->copyControlVector.clear();

        if(count >=1) {
            String8 fdstr = decryptHandle->extendedData.valueFor(String8("fd"));
            //This is newFd as opened in onOpenDecryptSession
            int fileDesc = atoi(fdstr.string());

            ALOGV("onCloseDecryptSession%d <- ####%s####",fileDesc,fdstr.string());

             if (fileDesc > -1) {
                ::close(fileDesc);
            }

            String8 tempPath = decryptHandle->extendedData.valueFor(String8("path"));
            ALOGV("onCloseDecryptSession: tempPath=%s",tempPath.string());
            if(tempPath.string() != NULL) remove(tempPath.string());
        }
        decryptHandle->extendedData.clear();

        delete decryptHandle;
        decryptHandle = NULL;
    }

    ALOGV("onCloseDecryptSession Exit");
    return DRM_NO_ERROR;
}

status_t OmaDrmEngine::onInitializeDecryptUnit(int uniqueId,
                                                DecryptHandle* decryptHandle,
                                                int decryptUnitId,
                                                const DrmBuffer* headerInfo) {
    ALOGV("onInitializeDecryptUnit is not supported for this DRM scheme");
    return DRM_ERROR_UNKNOWN;
}

status_t OmaDrmEngine::onDecrypt(int uniqueId, DecryptHandle* decryptHandle, int decryptUnitId,
            const DrmBuffer* encBuffer, DrmBuffer** decBuffer, DrmBuffer* IV) {
    ALOGV("onDecrypt is not supported for this DRM scheme");
    return DRM_ERROR_UNKNOWN;
}

status_t OmaDrmEngine::onDecrypt(int uniqueId,
                                  DecryptHandle* decryptHandle,
                                  int decryptUnitId,
                                  const DrmBuffer* encBuffer,
                                  DrmBuffer** decBuffer) {
    ALOGV("onDecrypt is not supported for this DRM scheme");
    return DRM_ERROR_UNKNOWN;
}

status_t OmaDrmEngine::onFinalizeDecryptUnit(int uniqueId,
                                              DecryptHandle* decryptHandle,
                                              int decryptUnitId) {
    ALOGV("onFinalizeDecryptUnit is not supported for this DRM scheme");
    return DRM_ERROR_UNKNOWN;
}

ssize_t OmaDrmEngine::onRead(int uniqueId,
                              DecryptHandle* decryptHandle,
                              void* buffer,
                              int numBytes) { 
    ssize_t size = -1;
    return size;
}

#ifdef USE_64BIT_DRM_API
off64_t OmaDrmEngine::onLseek(int uniqueId, DecryptHandle* decryptHandle,
                               off64_t offset, int whence) {
#else
off_t OmaDrmEngine::onLseek(int uniqueId, DecryptHandle* decryptHandle,
                             off_t offset, int whence) {
#endif
    off_t offval = -1;

    if (NULL != decryptHandle) {
        //TODO:seek with offset
    }

    return offval;
}

#ifdef USE_64BIT_DRM_API
ssize_t OmaDrmEngine::onPread(int uniqueId,
                               DecryptHandle* decryptHandle,
                               void* buffer,
                               ssize_t numBytes,
                               off64_t offset) {
#else
ssize_t OmaDrmEngine::onPread(int uniqueId,
                               DecryptHandle* decryptHandle,
                               void* buffer,
                               ssize_t numBytes,
                               off_t offset) {
#endif 
    FILE * pFile;
    long lSize;
    char * buff;
    size_t result = -1;
    if(decryptHandle->status != RightsStatus::RIGHTS_VALID) {
        return DRM_ERROR_LICENSE_EXPIRED;
    }

    int count = decryptHandle->extendedData.size();
    if(count < 1) { 
        return DRM_ERROR_SESSION_NOT_OPENED;
    }
    String8 tempPath = decryptHandle->extendedData.valueFor(String8("path"));
    if (tempPath.string() ==  NULL) {
        ALOGE("error in decrypt session info");
        return DRM_ERROR_SESSION_NOT_OPENED;
    }
    pFile = fopen( tempPath.string(), "r" );
    if (pFile==NULL) {
        ALOGE("File error");
        return result;
    }
    
    int err = fseek (pFile , offset , SEEK_SET);
    // copy the file into the buffer:
    result = fread (buffer,1,numBytes,pFile);
    fclose(pFile);
    return result;
}

int OmaDrmEngine::openDrmSession(const String8& path) {
    FILE * infile;
	int id = -1;
    infile = fopen(path, "rb"); // Unix ignores 'b', windows needs it.
	if (infile == NULL) {
        ALOGE("Error opening file: %s.\n", strerror(errno));
        return id;
	}

	T_DRM_Input_Data inData;
    memset(&inData, 0, sizeof(inData));
	inData.inputHandle = (int64_t)infile;
    inData.mimeType = TYPE_DRM_CONTENT;
    inData.getInputDataLength = getLen;
	inData.readInputData = readFunc;
    inData.seekInputData = seekFunc;
    id = SVC_drm_openSession(inData);
    fclose(infile); //0024370 -- Resource leak
    return id;
}
