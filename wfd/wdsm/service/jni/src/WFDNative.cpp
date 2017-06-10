/*==============================================================================
*  @file WFDNative.cpp
*
*  @par DESCRIPTION:
*
*
*
*  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/


#define LOG_TAG "WFDNative_CPP"

#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0
#include <utils/Log.h>
#ifndef WFD_ICS
#include "common_log.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <jni.h>
#include <unistd.h>
#ifdef WFD_ICS
#include <surfaceflinger/Surface.h>
#include <ui/Input.h>
#else
#include <Surface.h>
#include <Input.h>
#endif
#include <android_view_MotionEvent.h>
#include <android_view_KeyEvent.h>
#include <android_runtime/android_view_Surface.h>

#include "UIBCDefs.h"
#include "com_qualcomm_wfd_WFDNative.h"
#include "WiFiDisplayClient.h"

#define WFD_NATIVE_STRING_LEN 256
#define WFD_NATIVE_STRING_ARR_SIZE 50
#define UNUSED(x) ((void)x)

#define MM_UPDATE_EVENT         "MMEvent"
#define UIBC_ROTATE_EVENT       "UIBCRotateEvent"
#define WIFIDISPLAY_STRING_SIZE     256

static inline nsecs_t curTime() {
    return systemTime(SYSTEM_TIME_MONOTONIC);
}


#define FIND_CLASS(var, className) \
        var = env->FindClass(className); \
        LOG_FATAL_IF(! var, "Unable to find class " className); \
        var = jclass(env->NewGlobalRef(var));

#define GET_METHOD_ID(var, clazz, methodName, methodDescriptor) \
        var = env->GetMethodID(clazz, methodName, methodDescriptor); \
        LOG_FATAL_IF(! var, "Unable to find method " methodName);

#define GET_FIELD_ID(var, clazz, fieldName, fieldDescriptor) \
         var = env->GetFieldID(clazz, fieldName, fieldDescriptor); \
         if(!var)LOGE("Unable to find field ");

#define DETACH_THREAD(isAttached) do {\
    if (isAttached)\
        gJavaVM->DetachCurrentThread();\
}while(0)

using namespace android;
using namespace std;

static struct {
    jclass clazz;

    jfieldID deviceType;
    jfieldID macAddress;
    jfieldID deviceName;
    jfieldID ipAddress;
    jfieldID rtspPort;
    jfieldID decoderLatency;
} gWfdDeviceClassInfo;

static struct {
    jclass clazz;
    jfieldID surface;
} gSurface;

static struct {
    uint16 height;
    uint16 width;
    uint16 Orientation;
    float aspectRatio;
} gScreenDimensions;

static struct {
    int32_t height;
    int32_t width;
    float aspectRatio;
} gResolution;

typedef struct uibc_Fields_t{
    nsecs_t downTime;
    MotionEvent *prevEv;
    bool multiMode;
    PointerCoords* prevPtrCoords;
    PointerProperties* prevPtrProp;
    uibc_Fields_t() {
        LOGE("uibc_Fields_t ctor()");
        downTime = 0;
        prevEv = new MotionEvent();
        multiMode = false;
        prevPtrCoords = new PointerCoords[UIBC_MAX_TOUCH_INPUTS];
        prevPtrProp = new PointerProperties[UIBC_MAX_TOUCH_INPUTS];
    }
    ~uibc_Fields_t() {
        LOGE("uibc_Fields_t dtor()");
        if(prevEv) {
           delete prevEv;
        }
        if(prevPtrCoords) {
          delete[] prevPtrCoords;
        }
        if(prevPtrProp) {
          delete[] prevPtrProp;
        }
    }
} uibcFields;
static uibcFields* uibcParam;

static void wfdDeviceToNative(JNIEnv* env, jobject wfdDeviceObj, WfdDevice* outRawDevice);
//static void wfdDeviceFromNative(JNIEnv* env, const WfdDevice* rawDevice, jobject outWfdDeviceObj);
static void jni_stringarray_callback(const char* eName, int numObjects, char strArray[][256]);

static JavaVM *gJavaVM;
static jclass gWFDNativeClazz;

struct EventCallbackArgs {
    JNIEnv *jniEnv;
    jobject eventDispatcherObject;
    jclass uibcMgrCls;
    jmethodID HIDeventCbackmethod;
    JavaVM *cached_VM;
    boolean isAttached;
};
//Specially for caching VM and UIBC events
struct EventDispatcherClassInfo {
    jclass clazz;
    jmethodID addEvent;
};

static void setResolutionValues(int16 width, int16 height)
{
    LOGD("set Resolution Values with width %d, height %d", width, height);
    gResolution.height = height;
    gResolution.width = width;
    if(height!=0) {
        gResolution.aspectRatio = width*1.0/height;
        LOGD("Aspect Ratio of WFD session Resolution is %lf",gResolution.aspectRatio);
    } else {
        LOGE("height passed to setResolutionValues is zero!!");
    }
}

struct EventCallbackArgs gEventCallbackArgs;
//Specially for caching VM and UIBC events
struct  EventDispatcherClassInfo gUIBCEventDispatcherClassInfo;
const float X_OFFSET = 0.0f;
const float Y_OFFSET = 0.0f;
const float TOUCH_PRESSURE = 0.8f;
const float TOUCH_SIZE     = 0.04f;
const int MOTION_EVENT_DEVICE_ID = 0;
const int KEY_EVENT_DEVICE_ID = 0;
/* list of native methods */
static JNINativeMethod gMethods[] = {
        // nameOfNativeMethod,           methodSignature,                                      methodPointer
        {   "enableWfd",            "(Lcom/qualcomm/wfd/WfdDevice;)Z",    (void*) Java_com_qualcomm_wfd_WFDNative_enableWfd              },
        {   "disableWfd",           "()Z",                                (void*) Java_com_qualcomm_wfd_WFDNative_disableWfd             },
        {   "startWfdSession",      "(Lcom/qualcomm/wfd/WfdDevice;)V",    (void*) Java_com_qualcomm_wfd_WFDNative_startWfdSession        },
        {   "stopWfdSession",       "(I)V",                               (void*) Java_com_qualcomm_wfd_WFDNative_stopWfdSession         },
        {   "play",                 "(IZ)V",                              (void*) Java_com_qualcomm_wfd_WFDNative_play                   },
        {   "pause",                "(IZ)V",                              (void*) Java_com_qualcomm_wfd_WFDNative_pause                  },
        {   "teardown",             "(IZ)V",                              (void*) Java_com_qualcomm_wfd_WFDNative_teardown               },
        {   "standby",              "(I)Z",                               (void*) Java_com_qualcomm_wfd_WFDNative_standby                },
        {   "enableUIBC",           "(I)Z",                               (void*) Java_com_qualcomm_wfd_WFDNative_enableUIBC             },
        {   "disableUIBC",          "(I)Z",                               (void*) Java_com_qualcomm_wfd_WFDNative_disableUIBC            },
        {   "startUIBC",            "(Ljava/lang/Object;)V",              (void*) Java_com_qualcomm_wfd_WFDNative_startUIBC              },
        {   "stopUIBC",             "()V",                                (void*) Java_com_qualcomm_wfd_WFDNative_stopUIBC               },
        {   "sendUIBCKeyEvent",     "(Landroid/view/KeyEvent;)V",         (void*) Java_com_qualcomm_wfd_WFDNative_sendUIBCKeyEvent       },
        {   "sendUIBCMotionEvent",  "(Landroid/view/MotionEvent;)V",      (void*) Java_com_qualcomm_wfd_WFDNative_sendUIBCMotionEvent    },
        {   "sendUIBCRotateEvent",  "(I)Z",                               (void*) Java_com_qualcomm_wfd_WFDNative_sendUIBCRotateEvent    },
        {   "setSurfaceProp",       "(III)V",                             (void*) Java_com_qualcomm_wfd_WFDNative_setSurfaceProp         },
        {   "setResolution",        "(II[I)Z",                            (void*) Java_com_qualcomm_wfd_WFDNative_setResolution          },
        {   "getConfigItems",       "([I)V",                              (void*) Java_com_qualcomm_wfd_WFDNative_getConfigItems         },
        {   "setUIBC",              "(I)Z",                               (void*) Java_com_qualcomm_wfd_WFDNative_setUIBC                },
        {   "executeRuntimeCommand","(I)Z",                               (void*) Java_com_qualcomm_wfd_WFDNative_executeRuntimeCommand  },
        {   "getCommonRes",         "([I)[I",                             (void*) Java_com_qualcomm_wfd_WFDNative_getCommonRes           },
        {   "getNegotiatedRes",     "([I)[I",                             (void*) Java_com_qualcomm_wfd_WFDNative_getNegotiatedRes       }
};


/** =======================================================================
**               JNI OnLoad and OnUnload
** ======================================================================= */
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    UNUSED(reserved);
    JNIEnv *env;

    //store java virtual machine in global variable
    gJavaVM = vm;
    gEventCallbackArgs.cached_VM = vm;
    gEventCallbackArgs.eventDispatcherObject = NULL;
    gEventCallbackArgs.jniEnv = NULL;

    LOGE("JNI_OnLoad called");

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("Failed to get the environment using GetEnv()");
        return JNI_FALSE;
    }

    FIND_CLASS(gUIBCEventDispatcherClassInfo.clazz, "com/qualcomm/wfd/UIBCManager$EventDispatcher");
    GET_METHOD_ID(gUIBCEventDispatcherClassInfo.addEvent, gUIBCEventDispatcherClassInfo.clazz,
                  "addEvent", "(Landroid/view/InputEvent;)V");
    /*Populate WfdDevice class info*/
    FIND_CLASS(gWfdDeviceClassInfo.clazz, "com/qualcomm/wfd/WfdDevice");
    gWfdDeviceClassInfo.clazz = jclass(env->NewGlobalRef(gWfdDeviceClassInfo.clazz));

    GET_FIELD_ID(gWfdDeviceClassInfo.deviceName, gWfdDeviceClassInfo.clazz,
                 "deviceName", "Ljava/lang/String;");
    GET_FIELD_ID(gWfdDeviceClassInfo.deviceType, gWfdDeviceClassInfo.clazz,
                 "deviceType", "I");
    GET_FIELD_ID(gWfdDeviceClassInfo.ipAddress, gWfdDeviceClassInfo.clazz,
                 "ipAddress", "Ljava/lang/String;");
    GET_FIELD_ID(gWfdDeviceClassInfo.macAddress, gWfdDeviceClassInfo.clazz,
                 "macAddress", "Ljava/lang/String;");
    GET_FIELD_ID(gWfdDeviceClassInfo.rtspPort, gWfdDeviceClassInfo.clazz,
                 "rtspPort", "I");
    GET_FIELD_ID(gWfdDeviceClassInfo.decoderLatency, gWfdDeviceClassInfo.clazz,
                 "decoderLatency", "I");

    /* Native function registration */
    FIND_CLASS(gWFDNativeClazz, "com/qualcomm/wfd/WFDNative");
    if (env->RegisterNatives(gWFDNativeClazz, gMethods, sizeof(gMethods)/sizeof(gMethods[0])) < 0) {
        LOGE("RegisterNatives failed for '%s'", "com/qualcomm/wfd/WFDNative");
        return JNI_FALSE;
    }
    FIND_CLASS(gSurface.clazz, "android/view/Surface");
    GET_FIELD_ID(gSurface.surface, gSurface.clazz, "mNativeSurface", "I");

    if(gSurface.surface == NULL) {
      env->ExceptionClear();
      GET_FIELD_ID(gSurface.surface, gSurface.clazz, "mNativeObject", "I");
    }

    if(gSurface.surface == NULL) {
      env->ExceptionClear();
      GET_FIELD_ID(gSurface.surface, gSurface.clazz, "mNativeObject", "J");
    }

    FIND_CLASS(gEventCallbackArgs.uibcMgrCls,"com/qualcomm/wfd/UIBCManager");
    gEventCallbackArgs.HIDeventCbackmethod= env->GetStaticMethodID(gEventCallbackArgs.uibcMgrCls,"HIDEventCallback", "([BI)V");
    gScreenDimensions.height = 0;
    gScreenDimensions.width  = 0;
    gScreenDimensions.Orientation = 0;
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved)
{
    UNUSED(vm);
    UNUSED(reserved);
    gJavaVM = 0;
}

/*Converting android key code to WFD specific format*/
static int getWFDFormatKeyCode(int androidKeyCode, int metaState)
{
   int wfdKeyCode = 0;

   /*Based on specification  Used reference:http://tools.ietf.org/html/rfc20*/

   if(androidKeyCode >= AKEYCODE_0 && androidKeyCode <= AKEYCODE_9)
   {
      LOGE("getWFDFormatKeyCode: Number key %d", androidKeyCode);
      if(((metaState & AMETA_SHIFT_ON) == AMETA_SHIFT_ON)) {
         switch (androidKeyCode)
         {
            case AKEYCODE_1              : wfdKeyCode = 0x21;  break; //!
            case AKEYCODE_2              : wfdKeyCode = 0x40;  break; //@
            case AKEYCODE_3              : wfdKeyCode = 0x23;  break; //#
            case AKEYCODE_4              : wfdKeyCode = 0x24;  break; //$
            case AKEYCODE_5              : wfdKeyCode = 0x25;  break; //%
            case AKEYCODE_6              : wfdKeyCode = 0x5E;  break; //^
            case AKEYCODE_7              : wfdKeyCode = 0x26;  break; //&
            case AKEYCODE_8              : wfdKeyCode = 0x2A;  break; //*
            case AKEYCODE_9              : wfdKeyCode = 0x28;  break; //(
            case AKEYCODE_0              : wfdKeyCode = 0x29;  break; //)
         }
      } else {
         //Values from 0x30 to 0x39
         int map[]={0x30,0x31,0x32,0x33,
                    0x34,0x35,0x36,0x37,
                    0x38,0x39};
         wfdKeyCode = map[androidKeyCode - AKEYCODE_0];
      }
   }
   else if(androidKeyCode >= AKEYCODE_A && androidKeyCode <= AKEYCODE_Z)
   {
      LOGE("getWFDFormatKeyCode: Alphabet key %d", androidKeyCode);
      int map[]={0x61,0x62,0x63,0x64,0x65,
                 0x66,0x67,0x68,0x69,0x6A,
                 0x6B,0x6C,0x6D,0x6E,0x6F,
                 0x70,0x71,0x72,0x73,0x74,
                 0x75,0x76,0x77,0x78,0x79,
                 0x7A};
      wfdKeyCode = map[androidKeyCode - AKEYCODE_A];
      if(((metaState & AMETA_SHIFT_ON) == AMETA_SHIFT_ON) ||
         ((metaState & AMETA_CAPS_LOCK_ON) == AMETA_CAPS_LOCK_ON)) {
          wfdKeyCode-= 0x20;
      }
    }
    else
    {
       switch(androidKeyCode)
       {
          #define check(meta) (((meta & AMETA_SHIFT_ON) == AMETA_SHIFT_ON))
          case AKEYCODE_DEL  :           wfdKeyCode = 0x0008;  break;
          case AKEYCODE_TAB  :           wfdKeyCode = 0x0009;  break;
          case AKEYCODE_ENTER:           wfdKeyCode = 0x000D;  break;
          case AKEYCODE_ESCAPE:          wfdKeyCode = 0x001B;  break;
          case AKEYCODE_SPACE:           wfdKeyCode = 0x0020;  break;
          case AKEYCODE_POUND:           wfdKeyCode = 0x0023;  break;
          case AKEYCODE_STAR:            wfdKeyCode = 0x002A;  break;
          case AKEYCODE_COMMA:           wfdKeyCode = check(metaState)?0x003C:0x002C;  break;
          case AKEYCODE_PERIOD:          wfdKeyCode = check(metaState)?0x003E:0x002E;  break;
          case AKEYCODE_SLASH:           wfdKeyCode = check(metaState)?0x003F:0x002F;  break;
          case AKEYCODE_APOSTROPHE:      wfdKeyCode = check(metaState)?0x0022:0x0027;  break;
          case AKEYCODE_PLUS:            wfdKeyCode = 0x002B;  break;
          case AKEYCODE_EQUALS:          wfdKeyCode = check(metaState)?0x002B:0x003D;  break;
          case AKEYCODE_MINUS:           wfdKeyCode = check(metaState)?0x005F:0x002D;  break;
          case AKEYCODE_SEMICOLON:       wfdKeyCode = check(metaState)?0x003A:0x003B;  break;
          case AKEYCODE_LEFT_BRACKET:    wfdKeyCode = check(metaState)?0x007B:0x005B;  break;
          case AKEYCODE_RIGHT_BRACKET:   wfdKeyCode = check(metaState)?0x007D:0x005D;  break;
          case AKEYCODE_BACKSLASH:       wfdKeyCode = check(metaState)?0x007C:0x005C;  break;
          case AKEYCODE_GRAVE:           wfdKeyCode = check(metaState)?0x007E:0x0060;  break;
          case AKEYCODE_FORWARD_DEL:     wfdKeyCode = 0x007F;  break;
        }
        #undef check
    }

   LOGE("getWFDFormatKeyCode: Android code %d,wfd code %d", androidKeyCode, wfdKeyCode);
   return wfdKeyCode;
}

int keysym2keycode(unsigned int key, int* metaState)
{
    int keycode = 0;
    int code = (int)key;

    if (code>='0' && code<='9') {
        keycode = (code & 0xF);
        keycode += AKEYCODE_0;
    }
    else if ((code>='A' && code<='Z') || (code>='a' && code<='z')) {
      static const uint16_t map[] = {
          AKEYCODE_A, AKEYCODE_B, AKEYCODE_C, AKEYCODE_D, AKEYCODE_E ,
          AKEYCODE_F, AKEYCODE_G, AKEYCODE_H, AKEYCODE_I, AKEYCODE_J,
          AKEYCODE_K, AKEYCODE_L, AKEYCODE_M, AKEYCODE_N, AKEYCODE_O,
          AKEYCODE_P, AKEYCODE_Q, AKEYCODE_R, AKEYCODE_S, AKEYCODE_T,
          AKEYCODE_U, AKEYCODE_V, AKEYCODE_W, AKEYCODE_X, AKEYCODE_Y,
          AKEYCODE_Z };
      int index = static_cast<int>((code|0x20) - 'a');
      if(index >=0 && index < sizeof(map)/sizeof(map[0])) {
         keycode = map[index];
      }
      if (code>='A' && code<='Z') {
         (*metaState)|= AMETA_SHIFT_ON;
      }
    }
    else {
      switch (code) {
        case 0x0008:  keycode = AKEYCODE_DEL;                break;
        case 0x0009:  keycode = AKEYCODE_TAB;                break;
        case 0x000D:  keycode = AKEYCODE_ENTER;              break;
        case 0x001B:  keycode = AKEYCODE_ESCAPE;             break;
        case 0x0020:  keycode = AKEYCODE_SPACE;              break;
        case 0x0021:  keycode = AKEYCODE_1;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x0022:  keycode = AKEYCODE_APOSTROPHE ;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x0023:  keycode = AKEYCODE_POUND ;             break;
        case 0x0024:  keycode = AKEYCODE_4;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x0025:  keycode = AKEYCODE_5;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x0026:  keycode = AKEYCODE_7;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x0027:  keycode = AKEYCODE_APOSTROPHE;         break;
        case 0x0028:  keycode = AKEYCODE_NUMPAD_LEFT_PAREN;  break;
        case 0x0029:  keycode = AKEYCODE_NUMPAD_RIGHT_PAREN; break;
        case 0x002B:  keycode = AKEYCODE_PLUS;               break;
        case 0x002C:  keycode = AKEYCODE_COMMA;              break;
        case 0x002D:  keycode = AKEYCODE_MINUS;              break;
        case 0x002E:  keycode = AKEYCODE_PERIOD;             break;
        case 0x002F:  keycode = AKEYCODE_SLASH ;             break;
        case 0x003B:  keycode = AKEYCODE_SEMICOLON ;         break;
        case 0x003A:  keycode = AKEYCODE_SEMICOLON ;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x003C:  keycode = AKEYCODE_COMMA ;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x002A:  keycode = AKEYCODE_STAR;               break;
        case 0x003D:  keycode = AKEYCODE_EQUALS;             break;
        case 0x003E:  keycode = AKEYCODE_PERIOD;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x003F:  keycode = AKEYCODE_SLASH;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x0040:  keycode = AKEYCODE_AT;                 break;
        case 0x005B:  keycode = AKEYCODE_LEFT_BRACKET;       break;
        case 0x005C:  keycode = AKEYCODE_BACKSLASH;          break;
        case 0x005D:  keycode = AKEYCODE_RIGHT_BRACKET;      break;
        case 0x005E:  keycode = AKEYCODE_6;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x005F:  keycode = AKEYCODE_MINUS;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x0060:  keycode = AKEYCODE_GRAVE;              break;
        case 0x007B:  keycode = AKEYCODE_LEFT_BRACKET;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x007C:  keycode = AKEYCODE_BACKSLASH;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x007D:  keycode = AKEYCODE_RIGHT_BRACKET;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x007E:  keycode = AKEYCODE_GRAVE;
                      (*metaState)|= AMETA_SHIFT_ON;         break;
        case 0x007F:  keycode = AKEYCODE_FORWARD_DEL;        break;
        default    :  keycode = 0;
      }
    }

    LOGE("keysym2keycode key: %d, keycode: %d meta state%d", key, keycode, (*metaState));
    return keycode;
}

static boolean AttachVM(boolean val, void *pClientData)
{
  UNUSED(pClientData);
  if (val == TRUE)
  {
    //Attach it to the virtual machine
    gEventCallbackArgs.cached_VM->AttachCurrentThread(&gEventCallbackArgs.jniEnv,NULL);
    gEventCallbackArgs.isAttached = true;
  }
  else
  {
    //Detach from the virtual machine
    gEventCallbackArgs.cached_VM->DetachCurrentThread();
    gEventCallbackArgs.jniEnv = NULL;
    gEventCallbackArgs.isAttached = false;
  }
  return TRUE;
}

/** =======================================================
*@brief Function for adjusting difference between WFD
        session resolution and device screen resolution
*
*@params xin normalized x co-ord captured from sink device
         yin normalized y co-ord captured from sink device

Logic:

If the device screen aspect ratio is not matching with that
of the WFD session then either x or y while injecting needs
to be modified accordingly since one of them will remain
invariant while transforming the co-ordiantes.Essentially,
the adjustment can be derived by reconstructing sink side
screen and calculating where the source side screen edges
will be at the sink capturing surface.

** =======================================================*/
void adjustForAspectRatio(double& xin, double& yin)
{
    LOGE("%lf resAspectRatio screenAspectRatio %lf original xin = %lf yin = %lf",
        gResolution.aspectRatio, gScreenDimensions.aspectRatio, xin, yin);
    if(gScreenDimensions.aspectRatio < gResolution.aspectRatio)
    {
        //Pillar boxes on screen so adjust for x
        if(gResolution.aspectRatio != 0)
        {
            xin = (xin - 0.5)/(gScreenDimensions.aspectRatio/(gResolution.aspectRatio*2));
            //This will yield values in the range of [-1,1]
            //Need to transform it to the range [0,1]
            xin = 0.5*(1 +  xin);
            if (!(xin>=0 && xin <= 1))
            {
                xin = yin = -1 ;//Do not inject rogue touches
            }
            LOGD("Pillar boxes x = %lf",xin);
        }
        else
        {
            LOGE("gResolution.aspectRatio is zero");
        }
    }
    else if (gScreenDimensions.aspectRatio > gResolution.aspectRatio)
    {
        //Letter boxes on screen so adjust for y
        if(gScreenDimensions.aspectRatio!=0)
        {
            yin = (yin - 0.5)/(gResolution.aspectRatio/(gScreenDimensions.aspectRatio*2));
            //This will yield values in the range of [-1,1]
            //Need to transform it to the range [0,1]
            yin = 0.5*(1 + yin);
            if (!(yin>=0 && yin <= 1))
            {
                xin = yin = -1 ;//Do not inject rogue touches
            }
            LOGD("Letter boxes y = %lf",yin);
        }
        else
        {
            LOGE("gScreenDimensions.aspectRatio is zero");
        }
    }
    //No need to make adjustments if both the aspect ratios are equal
}
/**
 * Function to convert UIBC event data structure defined in MM
 * lib to Android event data structure
 * @param ev
 *
 * @return jobject
 */
static boolean SendEvent(WFD_uibc_event_t* ev, void * pClientData )
{
    UNUSED(pClientData);
    JNIEnv *jniEnv ;
    nsecs_t eventTime = 0;
    int angle = 0;
    bool isAttached = false;
    bool injectEvent = true;
    jint status = gJavaVM->GetEnv((void **) &jniEnv, JNI_VERSION_1_4);
    if (status == JNI_EDETACHED) {
        if (gJavaVM->AttachCurrentThread(&jniEnv, NULL) != 0) {
            LOGE("Failed to attach thread to JVM!!!");
            return false;
        }
        isAttached = true;
    } else if (status == JNI_OK) {
        //The thread is already attached, JVM will take care of GC'ng it
    } else if (status == JNI_EVERSION) {
        LOGE("GetEnv: version JNI_VERSION_1_4 not supported");
        return false;
    }
    jobject eventDispatcherObject = gEventCallbackArgs.eventDispatcherObject;
    if (!ev || !jniEnv || !eventDispatcherObject) {
        LOGE("Invalid argument");
        return FALSE;
    }
    jobject eventObj = NULL;
    LOGE("Event type %d", ev->type);
    switch (ev->type)
    {
    case WFD_UIBC_TOUCH:
     {
        MotionEvent *newEv = new MotionEvent();
        if (newEv == NULL){
          LOGE ("Failed to obtain a native MotionEvent object");
          DETACH_THREAD(isAttached);
          return FALSE;
        }
        int eventActionType = -1;
        if(!uibcParam->multiMode && (ev->parms.touch_event.num_pointers > 1)) {
            //This is the first time a multi touch event is received after a
            //stream of single touch events. Set the multi mode flag to true
            uibcParam->multiMode = true;
            LOGE("Multi touch mode is ON");
        }

        //For multi touch events its the Action type that needs to be taken care of

        if(uibcParam->multiMode) {
            if(uibcParam->prevEv->getPointerCount() < ev->parms.touch_event.num_pointers) {
                //The pointer count of the incoming event is > last event
                //Hence this is definitely a POINTER_DOWN event
                LOGE("AMOTION_EVENT_ACTION_POINTER_DOWN");
                //1. Set the LSB to the correct action
                eventActionType = AMOTION_EVENT_ACTION_POINTER_DOWN;
                //2. Clear everything except LSB
                eventActionType &= AMOTION_EVENT_ACTION_MASK;
                //3. Now search for which pointer index actually went down
                for(int i = 0; i< ev->parms.touch_event.num_pointers ;i++) {
                   if(uibcParam->prevEv->findPointerIndex(ev->parms.touch_event.pointer_id[i])== -1){
                    //Previous touch event did not have this pointer ID
                    //This pointer ID's index needs to go down.
                    //4. Set the correct index in action in the correct byte
                     eventActionType|= i<<AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                     break;//break out of loop since only 1 id can differ
                   }
                }
            } else if(uibcParam->prevEv->getPointerCount() > ev->parms.touch_event.num_pointers) {
                //The pointer count of the incoming event is < last event
                //So the sink has had one of its pointers lifted.
                //This is very Android specific. At the sink side, POINTER_UP
                //(with the same pointer count as the previous event) will be
                //delivered to the source. The source has no way of discerning
                //between a POINTER_MOVE and POINTER_UP as such since
                //these events are not supported in WFD specification. So when
                //actually the pointer count decreases, the source can inject a
                //filler event mimicking a POINTER_UP
                LOGE("AMOTION_EVENT_ACTION_POINTER_UP");
                //Construct a dummy event from the previous event and fake it
                //to be a pointer up with the correct index. Then continue ahead
                //with the processing of the event
                MotionEvent *fillerEv = new MotionEvent();
                int fillerAction = -1;
                list<int> pointerID;
                for(size_t i = 0; i< uibcParam->prevEv->getPointerCount();i++) {
                   pointerID.push_back(uibcParam->prevEv->getPointerId(i));
                }
                for(int i = 0; i< ev->parms.touch_event.num_pointers ;i++) {
                   pointerID.remove(ev->parms.touch_event.pointer_id[i]);
                }
                for(list<int>::iterator beg = pointerID.begin();beg != pointerID.end();beg++)
                   LOGE("Remaining Pointer ID = %d",*beg);
                if(pointerID.size() != 1 ) {
                   LOGE("Abrupt change in pointer co-ords");
                   //reset the multi-touch mode
                   uibcParam->multiMode = false;
                   LOGE("Multi touch mode is OFF");
                   DETACH_THREAD(isAttached);
                   return FALSE;
                }

               //This list now contains the pointer ID that differs between the
               //previously injected event and the incoming event, hence this
               //yields the pointer up index that need to be filled in

               //1. Set the LSB to the correct action
               fillerAction= AMOTION_EVENT_ACTION_POINTER_UP;
               //2. Clear everything except LSB
               fillerAction&=AMOTION_EVENT_ACTION_MASK;
               //3. Now search for which pointer index actually went up
               int upIndex = uibcParam->prevEv->findPointerIndex(pointerID.back());
               fillerAction|= (upIndex)<<AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
               //The entire initialize rigmarole is necessary because simply
               //using copyFrom to copy from prevEv won't help. The correct
               //timestamp needs to be supplied for the event and unfortunately
               //there is no API to set the event time of a motion event
               nsecs_t fillerEvTime = curTime();
               int fillerEvPtrCount = uibcParam->prevEv->getPointerCount();
               fillerEv->initialize(MOTION_EVENT_DEVICE_ID,
                            AINPUT_SOURCE_TOUCHSCREEN, fillerAction,
                            AMOTION_EVENT_FLAG_WINDOW_IS_OBSCURED,
                            AMOTION_EVENT_EDGE_FLAG_NONE,
                            AMETA_NONE,AMOTION_EVENT_BUTTON_PRIMARY,
                            X_OFFSET, Y_OFFSET, 2.0f, 2.1f,
                            (uibcParam->prevEv)->getDownTime(), fillerEvTime,
                            fillerEvPtrCount, uibcParam->prevPtrProp,
                            uibcParam->prevPtrCoords);
               jobject fillerEvent = android_view_MotionEvent_obtainAsCopy(jniEnv, (const MotionEvent*) fillerEv);
               LOGD("Injecting filler event with action %d",fillerAction);
               jniEnv->CallVoidMethod(eventDispatcherObject, gUIBCEventDispatcherClassInfo.addEvent, fillerEvent);
               jniEnv->DeleteLocalRef(fillerEvent);
               delete fillerEv;
               if(ev->parms.touch_event.num_pointers == 1) {
                 //Multi-mode is still on and the incoming event has only one
                 //pointer, so the stream of multi-touch has ended, turn off
                 // multi mode
                 uibcParam->multiMode = false;
                 LOGE("Multi touch mode is OFF");
                 //Set the event action type as that of the incoming event
                 if (ev->parms.touch_event.type == WFD_UIBC_TOUCH_DOWN) {
                     eventActionType = AMOTION_EVENT_ACTION_DOWN;
                 } else if (ev->parms.touch_event.type == WFD_UIBC_TOUCH_UP) {
                     eventActionType = AMOTION_EVENT_ACTION_UP;
                 } else {
                     eventActionType = AMOTION_EVENT_ACTION_MOVE;
                 }
               } else {
                 //The pointer count is still > 1, hence continue injecting
                 //with MOVE action as in normal multi mode
                 eventActionType = AMOTION_EVENT_ACTION_MOVE;
               }
          } else {
             //no change in number of pointers in multi-touch events
             //since multi mode is on, inject it simply as a MOVE
             eventActionType = AMOTION_EVENT_ACTION_MOVE;
          }
        } else {//take the normal single touch route
            if (ev->parms.touch_event.type == WFD_UIBC_TOUCH_DOWN) {
                eventActionType = AMOTION_EVENT_ACTION_DOWN;
            } else if (ev->parms.touch_event.type == WFD_UIBC_TOUCH_UP) {
                eventActionType = AMOTION_EVENT_ACTION_UP;
            } else {
                eventActionType = AMOTION_EVENT_ACTION_MOVE;
            }
        }

        if (ev->parms.touch_event.num_pointers > 0 && ev->parms.touch_event.num_pointers <= UIBC_MAX_TOUCH_INPUTS) {
            PointerCoords pointerCoords[UIBC_MAX_TOUCH_INPUTS];
//            int32_t pointerIds[UIBC_MAX_TOUCH_INPUTS];

            PointerProperties pointerProps[UIBC_MAX_TOUCH_INPUTS];
            double point_x=0,point_y=0;
            for (int i = 0, j=0; i < ev->parms.touch_event.num_pointers; i++) {
                point_x = ev->parms.touch_event.coordinate_x[i];
                point_y = ev->parms.touch_event.coordinate_y[i];
                if((point_x>=0)&&(point_x<=1)&&(point_y>=0)&&(point_x<=1)) {
                   pointerProps[j].clear();
                   pointerProps[j].id = ev->parms.touch_event.pointer_id[i];
                   pointerCoords[j].clear();
                   adjustForAspectRatio(point_x, point_y);
                   if(!(point_x >= 0 && point_x <=1 && point_y >= 0 && point_y <= 1)) {
                        injectEvent = false;
                        LOGD("Not injecting rogue touches");
                   }
                   ev->parms.touch_event.coordinate_x[j] =(int)(point_x * gScreenDimensions.width);
                   ev->parms.touch_event.coordinate_y[j] =(int ) (point_y * gScreenDimensions.height);
                   pointerCoords[j].setAxisValue(AMOTION_EVENT_AXIS_X,\
                                ev->parms.touch_event.coordinate_x[i]);
                   pointerCoords[j].setAxisValue(AMOTION_EVENT_AXIS_Y,\
                             ev->parms.touch_event.coordinate_y[i]);
                   j++;
                }
            }
            eventTime = curTime();
            if(eventActionType == AMOTION_EVENT_ACTION_DOWN) {
                uibcParam->downTime= eventTime;
            }
            LOGD(" Event time = %lld",eventTime);
            newEv->initialize(MOTION_EVENT_DEVICE_ID, AINPUT_SOURCE_TOUCHSCREEN, eventActionType,
                              AMOTION_EVENT_FLAG_WINDOW_IS_OBSCURED,
                              AMOTION_EVENT_EDGE_FLAG_NONE, AMETA_NONE,AMOTION_EVENT_BUTTON_PRIMARY,
                              X_OFFSET, Y_OFFSET, 2.0f, 2.1f, uibcParam->downTime, eventTime,
                              ev->parms.touch_event.num_pointers, pointerProps, pointerCoords);
            uibcParam->prevEv->copyFrom(newEv,true);
            for(int i = 0; i < ev->parms.touch_event.num_pointers; i++) {
                uibcParam->prevPtrCoords[i].copyFrom(pointerCoords[i]);
                uibcParam->prevPtrProp[i].copyFrom(pointerProps[i]);
            }
        } else {
            LOGE("Invalid number of inputs [%d] or mouse events are NULL", ev->parms.touch_event.num_pointers);
              delete newEv;
            break;
        }
        eventObj = android_view_MotionEvent_obtainAsCopy(jniEnv, (const MotionEvent*) newEv);
        delete newEv;
        break;
     }
    case WFD_UIBC_KEY:
      {
        KeyEvent *newEvent = new KeyEvent();
        if (newEvent == NULL) {
          LOGE("Failed to get a native KeyEvent object");
          DETACH_THREAD(isAttached);
          return FALSE;
        }
        int eventActionType=0;
        if (ev->parms.key_event.type == WFD_UIBC_KEY_DOWN) {
            eventActionType = AKEY_EVENT_ACTION_DOWN;
        } else if (ev->parms.key_event.type == WFD_UIBC_KEY_UP) {
            eventActionType = AKEY_EVENT_ACTION_UP;
        }
        //Ignoring Key Code 2 for now
        eventTime = curTime();
        int metaState = 0;
        unsigned int keyCode = keysym2keycode(ev->parms.key_event.key_code_1, &metaState);
        newEvent->initialize(KEY_EVENT_DEVICE_ID, AINPUT_SOURCE_KEYBOARD,
                             eventActionType,0,keyCode,0,metaState,0,eventTime,eventTime - 1 );
        eventObj = android_view_KeyEvent_fromNative(jniEnv,(const KeyEvent*)newEvent);
        delete newEvent;
        break;
     }
    case WFD_UIBC_ROTATE:
    {
      // Convert into degrees and then rotate to the nearest angle
      angle = (int) (((float)ev->parms.rotate_event.num_rotate_int +
      (float)ev->parms.rotate_event.num_rotate_fraction/256.0) * (180/3.141));
      LOGE("Angle extracted from sink = %d", angle);
      if (angle > 0 && angle < 45)
          angle = 0;
      else if (angle >=45 && angle < 135)
          angle = 90;
      else if (angle >=135 && angle < 225)
          angle = 180;
      else if (angle >=225 && angle < 315)
          angle = 270;
      else
          angle = 0;
      const int numObjects = 1;
      char strarray[numObjects][WIFIDISPLAY_STRING_SIZE];
      snprintf(strarray[0],256,"%d", angle);
      LOGE("Sending Rotate event");
      jni_stringarray_callback(UIBC_ROTATE_EVENT, numObjects, strarray);
      break;
    }
    default: //do nothing
        LOGE("Unsupported event type: %d", ev->type);
    }
    if (!eventObj && WFD_UIBC_ROTATE != ev->type) {
        LOGE("Failed to create event object");
    }
    else
    {
        if(injectEvent) {
          jniEnv->CallVoidMethod(eventDispatcherObject,
            gUIBCEventDispatcherClassInfo.addEvent, eventObj);
        }
        //Don't forget to delete the local reference
        jniEnv->DeleteLocalRef(eventObj);
    }
    DETACH_THREAD(isAttached);
    return TRUE;

}

static boolean sendHIDEvent(uint8* HIDPack, uint8 len, HIDDataType type)
{
    LOGD("sendHIDEvent");
    JNIEnv *env;
    boolean isAttached = false;
    jint status = gJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
    if (status == JNI_EDETACHED) {
        LOGE("GetEnv: not attached Attaching thread");
        if (gJavaVM->AttachCurrentThread(&env, NULL) != 0) {
            LOGE("Failed to attach thread to JVM!!!");
            return false;
        }
        isAttached = true;
    } else if (status == JNI_OK) {
        //The thread is already attached, JVM will take care of GC'ng it
    } else if (status == JNI_EVERSION) {
        LOGE("GetEnv: version JNI_VERSION_1_4 not supported");
        return false;
    }

    jsize length = len;
    //create a native array of jbytes before sending it across
    jbyte* tempArr;
    tempArr = new jbyte[length];
    for(jint i = 0; i<length;i++)
    {
        tempArr[i] = HIDPack[i];
    }
    jbyteArray HIDPacket = env->NewByteArray(length);
    if(HIDPacket)
    {
        env->SetByteArrayRegion(HIDPacket,0,length,tempArr);
    }
    else
    {
        LOGE("Failed to get an array");
        delete[] tempArr;
        DETACH_THREAD(isAttached);
        return false;
    }
    jint hidDataType = (jint)type;
    LOGD("Callback for HID event for type %d", hidDataType);
    env->CallStaticVoidMethod(gEventCallbackArgs.uibcMgrCls, gEventCallbackArgs.HIDeventCbackmethod, HIDPacket,hidDataType);
    env->DeleteLocalRef(HIDPacket);
    delete[] tempArr;
    DETACH_THREAD(isAttached);
    return true;
}
static void  convertToWFDFormat (InputEvent *ie, WFD_uibc_event_t *ev)
{
  int eventtype = ie->getType();
  switch (eventtype)
  {
    case AINPUT_EVENT_TYPE_MOTION:
     {
       MotionEvent* me = (MotionEvent*)ie;
	   double point_x =0, point_y=0;
       ev->type = WFD_UIBC_TOUCH;
       ev->parms.touch_event.num_pointers = me->getPointerCount();
       if((gScreenDimensions.width<=0)||(gScreenDimensions.height<=0))
                return;
       for (int i=0,j=0; i < ev->parms.touch_event.num_pointers; i++) {
                point_x = (double)(((double) me->getX(i))/gScreenDimensions.width);
                point_y = (double)(((double) me->getY(i))/gScreenDimensions.height);
                if((point_x>=0)&&(point_x<=1)&&(point_y>=0)&&(point_x<=1)) {
                        ev->parms.touch_event.coordinate_x[j] = point_x;
                        ev->parms.touch_event.coordinate_y[j] = point_y;
                        ev->parms.touch_event.pointer_id[j] = me->getPointerId(i);
                        j++;
                }
       }
       switch (me->getActionMasked())
       {
        LOGV("me->getActionMasked() = %d",me->getActionMasked());
        case AMOTION_EVENT_ACTION_DOWN:
        case AMOTION_EVENT_ACTION_POINTER_DOWN:
           ev->parms.touch_event.type = WFD_UIBC_TOUCH_DOWN;
           break;
        case AMOTION_EVENT_ACTION_UP:
        case AMOTION_EVENT_ACTION_POINTER_UP:
           ev->parms.touch_event.type = WFD_UIBC_TOUCH_UP;
           break;
        case AMOTION_EVENT_ACTION_MOVE:
           ev->parms.touch_event.type = WFD_UIBC_TOUCH_MOVE;
           break;
        default:
           LOGE("Unknown Motion Action");
       }
       break;
     }
    case AINPUT_EVENT_TYPE_KEY:
    {
      KeyEvent* ke = (KeyEvent*)ie;
      ev->type = WFD_UIBC_KEY;
      ev->parms.key_event.key_code_1 = getWFDFormatKeyCode(ke->getKeyCode(),ke->getMetaState());
      ev->parms.key_event.key_code_2 = 0;
      switch(ke->getAction())
      {
        case AKEY_EVENT_ACTION_DOWN:
           ev->parms.key_event.type = WFD_UIBC_KEY_DOWN;
           break;
        case AKEY_EVENT_ACTION_UP:
           ev->parms.key_event.type = WFD_UIBC_KEY_UP;
           break;
        default:
          LOGE("Unknown Key Action");
      }
      break;
    }
    default :
    LOGE("Unknown Event type");
  }

}

/*
 * WfdDevice (jobject) <-> Device (native) conversion
 */
static void wfdDeviceToNative(JNIEnv* env, jobject wfdDeviceObj, WfdDevice* outRawDevice){
    const char *str;
    jstring jstr;

    //memset(outRawDevice, 0, sizeof(*outRawDevice));
    jstr = jstring(env->GetObjectField(wfdDeviceObj, gWfdDeviceClassInfo.ipAddress));

    if (jstr) {
        str = env->GetStringUTFChars(jstr, NULL);
        LOG_FATAL_IF(! str, "Out of memory when retrieving jstring chars");
        strlcpy(outRawDevice->ipaddress, str, sizeof(outRawDevice->ipaddress));
        env->ReleaseStringUTFChars(jstr, str);
        env->DeleteLocalRef(jstr);
    } else {
    	strlcpy(outRawDevice->ipaddress, "", sizeof(outRawDevice->ipaddress));
    }
    jstr = jstring(env->GetObjectField(wfdDeviceObj, gWfdDeviceClassInfo.macAddress));
    if (jstr) {
        str = env->GetStringUTFChars(jstr, NULL);
        LOG_FATAL_IF(! str, "Out of memory when retrieving jstring chars");
        strlcpy(outRawDevice->macaddress, str, sizeof(outRawDevice->macaddress));
        env->ReleaseStringUTFChars(jstr, str);
        env->DeleteLocalRef(jstr);
    } else {
    	strlcpy(outRawDevice->macaddress, "", sizeof(outRawDevice->macaddress));
    }

    outRawDevice->SMControlPort = env->GetIntField(wfdDeviceObj, gWfdDeviceClassInfo.rtspPort);
    outRawDevice->decoderLatency = env->GetIntField(wfdDeviceObj, gWfdDeviceClassInfo.decoderLatency);
    outRawDevice->deviceType = env->GetIntField(wfdDeviceObj, gWfdDeviceClassInfo.deviceType);

}


/*static void wfdDeviceFromNative(JNIEnv* env, const WfdDevice* rawDevice, jobject outWfdDeviceObj){
    jstring jstr;
    env->SetIntField(outWfdDeviceObj, gWfdDeviceClassInfo.deviceType, rawDevice->deviceType);
    env->SetIntField(outWfdDeviceObj, gWfdDeviceClassInfo.rtspPort, rawDevice->SMControlPort);
    jstr = env->NewStringUTF(rawDevice->ipaddress);
    LOG_FATAL_IF(! jstr, "Out of memory when creating jstring");
    env->SetObjectField(outWfdDeviceObj, gWfdDeviceClassInfo.ipAddress, jstr);
    jstr = env->NewStringUTF(rawDevice->macaddress);
    LOG_FATAL_IF(! jstr, "Out of memory when creating jstring");
    env->SetObjectField(outWfdDeviceObj, gWfdDeviceClassInfo.macAddress, jstr);
}*/


/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    setAvPlaybackMode
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_qualcomm_wfd_WFDNative_setAvPlaybackMode
  (JNIEnv *env, jclass cls, jint mode)
{
    UNUSED(env);
    UNUSED(cls);
    LOGD("setAvPlaybackMode: %d", mode);
    int ret = WiFiDisplayClient::setAVPlaybackMode(mode);
    LOGE("Calling WiFiDisplayClient::setAVPlaybackMode = %d",ret);
    return (ret == 1) ? JNI_TRUE : JNI_FALSE;
}

/** =======================================================================
*                Supported JNI native functions
** ======================================================================= */

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    enableWfd
 * Signature: (Lcom/qualcomm/wfd/WfdDevice;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_qualcomm_wfd_WFDNative_enableWfd
  (JNIEnv *env, jclass cls, jobject thisDevice)
{
    UNUSED(cls);
    jboolean bWFDenableflag = false;
    LOGD("enableWfd called");

    WiFiDisplayClient::registerUibcCallbacks(&SendEvent,&sendHIDEvent);

    WfdDevice *dev = new WfdDevice;
    if (dev) {
       wfdDeviceToNative(env, thisDevice, dev);
       bWFDenableflag = WiFiDisplayClient::enableWfd(dev,&jni_stringarray_callback);
       LOGE("Calling WiFiDisplayClient::enableWfd returned %d",bWFDenableflag);
       delete dev;
    }
    uibcParam = new uibcFields;
    return bWFDenableflag;
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    disableWfd
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_qualcomm_wfd_WFDNative_disableWfd
  (JNIEnv *env, jclass cls)
{
    UNUSED(env);
    UNUSED(cls);
    LOGD("disableWfd called");
    if(uibcParam) {
       delete uibcParam;
    }
    if (WiFiDisplayClient::disableWfd()) {
        return JNI_TRUE ;
    } else {
        return JNI_FALSE;
    }
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    startWfdSession
 * Signature: (Lcom/qualcomm/wfd/WfdDevice;)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_startWfdSession
  (JNIEnv *env, jclass cls, jobject peerDevice)
{
    UNUSED(cls);
    WfdDevice *dev = new WfdDevice;
    if (dev != NULL) {
        wfdDeviceToNative(env, peerDevice, dev);
        LOGD("start WFD Session with %s", dev->macaddress);
        LOGD("decoder latency %d", dev->decoderLatency);
        int res = WiFiDisplayClient::startWfdSession(dev);
        LOGE("Calling WiFiDisplayClient::startWfdSession returned %d", res);
        delete dev; //dev is copied in startWfdSession
    }
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    stopWfdSession
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_stopWfdSession
  (JNIEnv *env, jclass cls, jint sid)
{
    UNUSED(env);
    UNUSED(cls);
    LOGD("stopWfdSession called");
    int res = WiFiDisplayClient::stopWfdSession(sid);
    LOGE("Calling WiFiDisplayClient::stopWfdSession returned %d", res);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    play
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_play
  (JNIEnv *env, jclass cls, jint sid, jboolean secureFlag)
{
    UNUSED(env);
    UNUSED(cls);
    LOGD("Play RTSP session.  sid=%d", sid);
    int res = WiFiDisplayClient::play_rtsp(sid,secureFlag);
    LOGE("Calling WiFiDisplayClient::play_rtsp returned %d",res);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    pause
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_pause
  (JNIEnv *env, jclass cls, jint sid, jboolean secureFlag)
{
    UNUSED(env);
    UNUSED(cls);
    LOGD("Pause RTSP session.  sid=%d", sid);
    int res = WiFiDisplayClient::pause_rtsp(sid,secureFlag);
    LOGE("Calling WiFiDisplayClient::pause_rtsp returned %d",res);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    teardown
 * Signature: (IZ)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_teardown
  (JNIEnv *env, jclass cls, jint sid, jboolean isTriggered)
{
    UNUSED(env);
    UNUSED(cls);
    LOGD("Teardown RTSP session.  sid=%d", sid);
    int res = WiFiDisplayClient::teardown_rtsp(sid,isTriggered);
    LOGE("Calling WiFiDisplayClient::teardown_rtsp returned %d",res);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    setUIBC
 * Signature: (I)Z
 */

JNIEXPORT jboolean JNICALL Java_com_qualcomm_wfd_WFDNative_setUIBC
  (JNIEnv *env, jclass cls, jint sessionId)

{
    UNUSED(env);
    UNUSED(cls);
    LOGE("setUIBC with session ID %d ", sessionId);
    if (WiFiDisplayClient::setUIBC(sessionId))
        return JNI_TRUE;
    else
        return JNI_FALSE;
}
/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    standby
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_qualcomm_wfd_WFDNative_standby
  (JNIEnv *env, jclass cls, jint sessionId)

{
    UNUSED(env);
    UNUSED(cls);
    LOGD("standby called");
    if (WiFiDisplayClient::standby_rtsp(sessionId)) {
        return JNI_TRUE ;
    } else {
        return JNI_FALSE;
    }
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    enableUIBC
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_qualcomm_wfd_WFDNative_enableUIBC
  (JNIEnv *env, jclass cls, jint sessionId)
{
    UNUSED(env);
    UNUSED(cls);

    // send RTSP request to enable UIBC
    LOGD("Enable UIBC.  RTSP sessionId=%d", sessionId);
    if(WiFiDisplayClient::enableUIBC(sessionId)){
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    disableUIBC
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_qualcomm_wfd_WFDNative_disableUIBC
  (JNIEnv *env, jclass cls, jint sessionId)
{
    UNUSED(env);
    UNUSED(cls);

    // send RTSP request to disable UIBC
    LOGD("Disable UIBC.  RTSP sessionId=%d", sessionId);
    if(WiFiDisplayClient::disableUIBC(sessionId)){
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    startUIBC
 * Signature: (Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_startUIBC
  (JNIEnv *env, jclass cls, jobject jobj)
{
    UNUSED(cls);
    LOGE("WFDNative_startUIBC");
    // initialize event source
    if (WiFiDisplayClient::getResolution(&(gResolution.width),&(gResolution.height)) != 1) {
        LOGE("Unable to query the session resolution");
    } else {
        LOGE("Queried resolution is width = %d, height = %d", gResolution.width,gResolution.height);
        gResolution.aspectRatio = gResolution.width * 1.0/gResolution.height;
    }
    gEventCallbackArgs.eventDispatcherObject = env->NewGlobalRef(jobj);
    // call MM lib to start UIBC
    int ret = WiFiDisplayClient::startUIBC();
    LOGE("Calling WiFiDisplayClient::startUIBC returned %d",ret);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    stopUIBC
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_stopUIBC
  (JNIEnv *env, jclass cls)
{
    UNUSED(env);
    UNUSED(cls);
    // call MM lib to stop UIBC
    int ret = WiFiDisplayClient::stopUIBC();
    LOGE("Calling WiFiDisplayClient::stopUIBC returned %d",ret);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    setVideoSurface
 * Signature: (Landroid/view/Surface;)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_setVideoSurface
  (JNIEnv *env, jclass cls, jobject javaSurface)
{
    UNUSED(cls);
    Surface *nativeSurface = NULL;

    if (javaSurface) {
        // TODO: FIXME to maintain b/w compatibility where surface is an int
        nativeSurface = reinterpret_cast<Surface*>
                        (env->GetLongField(javaSurface, gSurface.surface));
        LOGD("Native surface pointer %p", nativeSurface);
    } else {
        LOGD("Java surface is null");
    }
    int ret = WiFiDisplayClient::setSurface(nativeSurface);
    LOGE("Calling WiFiDisplayClient::setSurface returned %d",ret);
}
/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    sendUIBCKeyEvent
 * Signature: (Landroid/view/InputEvent;)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_sendUIBCKeyEvent
  (JNIEnv *env, jclass cls, jobject keyEventObj)
{
    UNUSED(cls);
    KeyEvent    ke;
    WFD_uibc_event_t ev;
    ev.timestamp = UINT16_MAX;
    android_view_KeyEvent_toNative(env,keyEventObj,&ke);
    convertToWFDFormat(&ke, &ev);
    int ret = WiFiDisplayClient::sendUIBCEvent(ev);
    LOGD("Calling WiFiDisplayClient::sendUIBCEvent returned %d", ret);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    sendUIBCInputEvent
 * Signature: (Landroid/view/InputEvent;)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_sendUIBCMotionEvent
  (JNIEnv *env, jclass cls, jobject motionEventObj)
{
    UNUSED(cls);
    WFD_uibc_event_t ev;
    MotionEvent *me;

    me = android_view_MotionEvent_getNativePtr(env, motionEventObj);
    if (me == NULL)
    {
        LOGE("Failed to get Native Motion Event object");
        return;
    }
    ev.timestamp = UINT16_MAX;
    convertToWFDFormat (me, &ev);
    int ret = WiFiDisplayClient::sendUIBCEvent(ev);
    LOGD("Calling WiFiDisplayClient::sendUIBCEvent returned %d", ret);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    sendUIBCRotateEvent
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_qualcomm_wfd_WFDNative_sendUIBCRotateEvent
  (JNIEnv *env, jclass clazz, jint angle)
{
    UNUSED(env);
    UNUSED(clazz);
    LOGD("sendUIBC Rotate with angle %d ", angle);
    WFD_uibc_event_t rotationEvent;
    rotationEvent.type = WFD_UIBC_ROTATE;
    rotationEvent.parms.rotate_event.num_rotate_int = (int)((angle * 3.141)/180);
    rotationEvent.parms.rotate_event.num_rotate_fraction = (int)(((angle * 3.141)/180)-
                        rotationEvent.parms.rotate_event.num_rotate_int) * 256;
    rotationEvent.timestamp = UINT16_MAX;
    if(1 == WiFiDisplayClient::sendUIBCEvent(rotationEvent)) {
      return JNI_TRUE;
    } else {
      LOGE("sendUIBCEvent failed!!!");
      return JNI_FALSE;
    }
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    setResolution
 * Signature: (II[I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_qualcomm_wfd_WFDNative_setResolution
  (JNIEnv *env, jclass cls, jint formatType, jint value, jintArray resParams)
{
    UNUSED(cls);
    LOGD("setResolution formatType: %d, value: %d", formatType, value);
    jsize len = env->GetArrayLength(resParams);
    jint *nativeResParamArr = env->GetIntArrayElements(resParams, 0);
    bool retStatus = WiFiDisplayClient::setResolution(formatType, value,
        nativeResParamArr,len);
    if (retStatus)
    {
        //This should be matched with what is being passed from JAVA layers
        setResolutionValues(nativeResParamArr[0],nativeResParamArr[1]);
    }
    env->ReleaseIntArrayElements(resParams , nativeResParamArr, 0);
    return (retStatus)?JNI_TRUE: JNI_FALSE;
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    setBitrate
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_setBitrate
  (JNIEnv *env, jclass cls, jint bitrate)
{
    UNUSED(env);
    UNUSED(cls);
    LOGD("setBitrate bitrate: %d", bitrate);
    int res = WiFiDisplayClient::setBitrate(bitrate);
    LOGE("Calling WiFiDisplayClient:::setBitrate returned %d",res);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    negotiateRtpTransport
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_negotiateRtpTransport
  (JNIEnv *env, jclass cls, jint transportType, jint bufferLengthMs, jint port)
{
    UNUSED(env);
    UNUSED(cls);
    LOGD("negotiateRtpTransport[transportType: %d, bufferLengthMs: %d, port: %d]", transportType, bufferLengthMs, port);
    int res = WiFiDisplayClient::negotiateRtpTransport(transportType,bufferLengthMs,port);
    LOGE("Calling WiFiDisplayClient::negotiateRtpTransport returned %d", res);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    setRtpTransport
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_setRtpTransport
  (JNIEnv *env, jclass cls, jint transportType)
{
    UNUSED(env);
    UNUSED(cls);
    LOGD("setRtpTransport: %d", transportType);
    int res = WiFiDisplayClient::setRtpTransport(transportType);
    LOGE("Calling WiFiDisplayClient::setRtpTransport returned %d", res);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    queryTCPTransportSupport
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_queryTCPTransportSupport
  (JNIEnv *, jclass)
{
    LOGD("queryTCPTransportSupport in native");
    int res = WiFiDisplayClient::queryTCPTransportSupport();
    LOGE("Calling WiFiDisplayClient::queryTCPTransportSupport returned %d", res);
}


/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    setDecoderLatency
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_setDecoderLatency
  (JNIEnv *env, jclass cls, jint latency)
{
    UNUSED(env);
    UNUSED(cls);
    LOGD("setDecoderLatency: %d", latency);
    int res = WiFiDisplayClient::setDecoderLatency(latency);
    LOGE("Calling WiFiDisplayClient::setDecoderLatency returned %d", res);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    tcpPlaybackControl
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_tcpPlaybackControl
  (JNIEnv *env, jclass cls, jint cmdType, jint cmdVal)
{
    UNUSED(env);
    UNUSED(cls);
    LOGD("tcpPlaybackControl: %d %d", cmdType, cmdVal);
    int res = WiFiDisplayClient::tcpPlaybackControl(cmdType,cmdVal);
    LOGE("Calling WiFiDisplayClient::tcpPlaybackControl returned %d", res);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    setSurfaceProp
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_setSurfaceProp
  (JNIEnv *env, jclass cls, jint arg1, jint arg2, jint orientation)
{
        UNUSED(env);
        UNUSED(cls);
        gScreenDimensions.width  = arg1;
        gScreenDimensions.height = arg2;
        gScreenDimensions.Orientation = orientation;
        if (arg2!=0) {
        gScreenDimensions.aspectRatio = arg1*1.0/arg2;
        LOGD("setSurfaceProp width %d height %d orientation %d screen aspect ratio is %lf",
             gScreenDimensions.width, gScreenDimensions.height,orientation,
             gScreenDimensions.aspectRatio);
        } else {
            LOGE("height of screen resolution is zero");
        }
}


/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    getConfigItems
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_wfd_WFDNative_getConfigItems
  (JNIEnv *env, jclass cls, jintArray configItems)
{
    UNUSED(cls);
    int* cfgItems = NULL;
    size_t length = 0;
    int ret = WiFiDisplayClient::getConfigItems(&cfgItems,&length);
    LOGE("Calling native WiFiDisplayClient::getConfigItems returned %d", ret);
    if(0 != ret || !cfgItems || 0 == length)
    {
        LOGE("Something horrible transpired!");
        return;
    }
    jsize len = length;
    //Create jint array to tide over platform dependent
    //definition of ints
    jint* tempArr = new jint[len];
    for(jint i=0;i< len;i++) {
        tempArr[i] = cfgItems[i];
        LOGI("cfgItems is %d",tempArr[i]);
    }
    env->SetIntArrayRegion(configItems, 0, len, tempArr);
    //Clear any array out of bound access exceptions if any
    env->ExceptionClear();
    delete[] tempArr;
    delete[] cfgItems;
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    executeRuntimeCommand
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_qualcomm_wfd_WFDNative_executeRuntimeCommand
  (JNIEnv * env, jclass cls, jint cmd)
{
    UNUSED(env);
    UNUSED(cls);
    int ret = WiFiDisplayClient::executeRuntimeCommand(cmd);
    LOGE("Calling WiFiDisplayClient::executeRuntimeCommand returned %d", ret);
    if(ret == 0)
      return JNI_FALSE;
    else
      return JNI_TRUE;
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    getCommonRes
 * Signature: ([I)[I
 */
JNIEXPORT jintArray JNICALL Java_com_qualcomm_wfd_WFDNative_getCommonRes
  (JNIEnv * env, jclass cls, jintArray result)
{
    UNUSED(cls);
    if(!result)
    {
        return NULL;
    }
    jint nativeResult[1] = {0};
    jint numProf = 0;
    uint32_t* comRes = NULL;
    int ret = WiFiDisplayClient::getCommonResolution(&comRes,&numProf);
    LOGD("Calling WiFiDisplayClient::getCommonResolution returned %d", ret);
    if(!comRes)
    {
        nativeResult[0]= -1;
        env->SetIntArrayRegion(result,0,1,nativeResult);
        return NULL;
    }
    jsize len = 4*numProf;
    /*For each of the H264 profiles there will be 3 values to consider CEA,
     *VESA and HH. So create an array with a size of multiple of
      * 4 = [1Profile + its 3 corresponding bitmaps]
     */
    jint* tempComRes = new jint[len*sizeof(jint)];
    //Create an array of jints to tide over platform dependent defn. of ints
    for(jint i = 0 ; i< len ; i++ )
    {
        tempComRes[i] = comRes[i];
    }
    jintArray retArr = env->NewIntArray(len);
    if(retArr)
    {
        env->SetIntArrayRegion(retArr,0,len,tempComRes);
    }
    else
    {
        LOGE("Unable to alloacate memory!");
        env->ExceptionClear();
        //Clear memory exception, WFD shouldn't fail
        nativeResult[0]= -1;
    }
    if (comRes)
    {
        delete[] comRes;
    }
    if(tempComRes)
    {
        delete[] tempComRes;
    }
    env->SetIntArrayRegion(result,0,1,nativeResult);
    if(nativeResult[0] == -1)
    {
        return NULL;
    }
    return retArr;
}


/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    getNegotiatedRes
 * Signature: ([I)[I
 */
JNIEXPORT jintArray JNICALL Java_com_qualcomm_wfd_WFDNative_getNegotiatedRes
  (JNIEnv *env, jclass cls, jintArray result)
{
    UNUSED(cls);
    if(!result)
    {
        return NULL;
    }
    jint nativeResult[1] = {0};
    uint32_t* negRes = NULL;
    int ret = WiFiDisplayClient::getNegotiatedResolution(&negRes);
    LOGD("Calling WiFiDisplayClient::getNegotiatedResolution returned %d", ret);
    if(!negRes)
    {
        nativeResult[0] = -1;
        env->SetIntArrayRegion(result,0,1,nativeResult);
        return NULL;
    }
    //construct an array of size 4 to hold the bitmaps for CEA,VESA and HH
    jsize len = 4;
    jint* tempNegRes = new jint[len*sizeof(jint)];
    //Create an array of jints to tide over platform-dependent defn. of ints
    if(tempNegRes)
    {
        for(jint i = 0 ; i< len ; i++ )
        {
            tempNegRes[i] = negRes[i];
        }
    }
    jintArray retArr = env->NewIntArray(len);
    if(retArr)
    {
        env->SetIntArrayRegion(retArr,0,len,tempNegRes);
    }
    else
    {
        LOGE("Unable to alloacate memory!");
        env->ExceptionClear();
        //Clear memory exception, WFD shouldn't fail
        nativeResult[0]= -1;
    }
    if(negRes)
    {
        delete[] negRes;
    }
    if(tempNegRes)
    {
        delete[] tempNegRes;
    }
    env->SetIntArrayRegion(result,0,1,nativeResult);
    if (nativeResult[0] == -1)
    {
        return NULL;
    }
    return retArr;
}

/** =======================================================================
*                Callback functions from C++ to Java
** ======================================================================= */

/**
 * callback_handler is used as the callback function from .cpp to .java.
 * This function can be used in both Java threads and native threads.
 */
static void callback_handler(jstring eventName, jobjectArray oarr)
{
	int status;
	JNIEnv *env;
	bool isAttached = false;

	status = gJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
	if (status < 0) {
		LOGE("callback_handler: failed to get JNI environment, assuming native thread");
		status = gJavaVM->AttachCurrentThread(&env, NULL);
		if (status < 0) {
			LOGE("callback_handler: failed to attach current thread");
			return;
		}
		isAttached = true;
	}
	/* Find the callback method ID */
	jmethodID method = env->GetStaticMethodID(gWFDNativeClazz, "eventCallback", "(Ljava/lang/String;[Ljava/lang/Object;)V");
	if (!method) {
		LOGE("callback_handler: failed to get method ID");
		if (isAttached)
			gJavaVM->DetachCurrentThread();
		return;
	}
	const char* eName = env->GetStringUTFChars(eventName, NULL);
	LOGD("callback_handler: call Java method....   event=%s  oarr_size=%d", eName, env->GetArrayLength(oarr));
	env->ReleaseStringUTFChars(eventName, eName);
	env->CallStaticVoidMethod(gWFDNativeClazz, method, eventName, oarr);

	if (isAttached)
		gJavaVM->DetachCurrentThread();
}


/**
 * Function to trigger call back function to WFDNative.
 * message is comprised of: eventName, eventObjectArray={specified string array}.
 */
static void jni_stringarray_callback(const char* eName, int numObjects, char strArray[][256])
{
    int status;
    JNIEnv *env;
    bool isAttached = false;
    bool isSurfaceUpdate = false;
    LOGD("stringarray_callback  eName=%s  numObjects=%d", eName, numObjects);
    if(numObjects >= 4 && !strcmp(eName,MM_UPDATE_EVENT)) {
       //Make sure that is a MM_UPDATE_EVENT with the requisite number of objects
       //and then check if its a MMStreamStarted event to handle it a bit differently
       if(!strcmp(strArray[0],"MMStreamStarted")) {
          isSurfaceUpdate = true;
          LOGE("Received MMStreamStarted");
       }
    }
    for (int i=0; i<numObjects; i++) {
        LOGD("\t strArray[%d] = \"%s\"", i, strArray[i]);
    }

    status = gJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
    if (status < 0) {
    LOGE("callback_handler: failed to get JNI environment, assuming native thread");
    status = gJavaVM->AttachCurrentThread(&env, NULL);
    if (status < 0) {
        LOGE("callback_handler: failed to attach current thread");
        return;
    }
    isAttached = true;
    }

    jstring eventName = env->NewStringUTF(eName);
    jclass objArrCls = env->FindClass("java/lang/Object");
    jobjectArray oarr = env->NewObjectArray(numObjects, objArrCls, NULL);
    if(!isSurfaceUpdate) {
       jstring strArr[numObjects];
       for (int i=0;i<numObjects;i++) {
            strArr[i] = env->NewStringUTF((char*)strArray[i]);
            env->SetObjectArrayElement(oarr, i, strArr[i]);
            env->DeleteLocalRef(strArr[i]);
       }
       callback_handler(eventName, oarr);
    } else {
        jstring strArr[numObjects - 1];
        int i = 0;
        for (;i<numObjects-1;i++) {
            strArr[i] = env->NewStringUTF((char*)strArray[i]);
            env->SetObjectArrayElement(oarr, i, strArr[i]);
            env->DeleteLocalRef(strArr[i]);
        }
        long surface = atol((char*)strArray[i]);
        IGraphicBufferProducer* gphBuff = reinterpret_cast<IGraphicBufferProducer*>(surface);
        jobject jSurface = android_view_Surface_createFromIGraphicBufferProducer(env,gphBuff);
        env->SetObjectArrayElement(oarr, i, jSurface);
        callback_handler(eventName, oarr);
        env->DeleteLocalRef(jSurface);
     }
    env->DeleteLocalRef(eventName);
    env->DeleteLocalRef(oarr);
    if (isAttached)
        gJavaVM->DetachCurrentThread();
}


/*
 *
 *
[SM_A -> SM_B]
-	enableWfd(devType)
-	disableWfd()
-	getWfdIeInfo()
-	startWfdSession(peer_mac_addr)
-	startWfdCoupling(peer_mac_addr)
-	play(sessionId)
-	pause(sessionId)
-	teardown()
-	audioReroute()
  (Power management)
-   suspend()
-   resume()
  (Exception handling)
-   eventWiFiStateChanged()

[SM_B -> SM_A]
-	getLocalWiFiInfo()
-	getWfdDeviceInfo(peer_mac_addr)
-	eventServiceStateChanged(serviceState)
-	eventSessionStateChanged(mac_addr, sessionState, sessionId0, sessionId1)
-	eventStreamControlCompleted(controlCmd, sessionId, status)
  (Link info)
-   getWiFiState()
  (set WiFi)
-   setTxPower
  (Power management)
-   setWakeLock()
 *
 */
