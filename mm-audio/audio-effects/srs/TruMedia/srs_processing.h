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

#ifndef ANDROID_SRS_PROCESSING_API
#define ANDROID_SRS_PROCESSING_API

namespace android {

class SRS_Processing {
public:
    static const int AUTO;                                      // A special-case handle is _always_ available SRS_Processing::AUTO

    // Setup/Shutdown
    static int CreateHandle();                                  // Create a unique handle to an instance of SRS_Processing
    static void DestroyHandle(int handle);                      // Destroy a handle

    // Audio to Speaker/Output
    static void ProcessOutNotify(int handle, void* pSource, bool init);     // Buffers from pSource will be processed - (or closed if init=false)
    static void ProcessOutRoute(int handle, void* pSource, int device);     // Called on any Routing parameter changes - device is from AudioSystem::DEVICE_OUT_XXX
    static void ProcessOut(int handle, void* pSource, void* pSamples, int sampleBytes, int sampleRate, int countChans);     // Process the buffer specified

    // Audio from Mic/Input
    static void ProcessInNotify(int handle, void* pSource, bool init);      // Buffers from pSource will be processed - (or closed if init=false)
    static void ProcessInRoute(int handle, void* pSource, int device);      // Called on any Routing parameter changes - device is from AudioSystem::DEVICE_IN_XXX
    static void ProcessIn(int handle, void* pSource, void* pSamples, int sampleBytes, int sampleRate, int countChans);      // Process the buffer specified

    // Parameters via String
    static void ParamsSet(int handle, const String8& keyValues);
    static String8 ParamsGet(int handle, const String8& keys);

    static bool ParamsSet_Notify(int handle, const String8& keyValues);

    // Typeless Data...
    static void RawDataSet(int* pHandle, char* pKey, void* pData, int dataLen);     // Used for side-band configuration.  NULL handle means 'global' or singleton.

    // Parameters via Enum  - param defined in seperate params enum header
    //ENUMS SAVED FOR LATER
    //static void EnumSet(int handle, int param, float value);
    //static float EnumGet(int handle, int param);
};

};	// namespace android

#endif // ANDROID_SRS_PROCESSING_API
