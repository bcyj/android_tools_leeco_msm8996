/*==============================================================================
*        @file wfd_cfg_parser.cpp
*
*  @par DESCRIPTION:
*       Definition of the wfd xml configuration file parser
*
*
*  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                         EDIT HISTORY FOR FILE

$Header: //source/qcom/qct/multimedia2/Video/wfd/utils/main/latest/src/wfd_cfg_parser.cpp#6 $
$DateTime: 2012/02/24 03:40:43 $
$Change: 2227180 $

==============================================================================*/

/*===============================================================================
                            Header Inclusion
===============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "MMMalloc.h"
#include "MMFile.h"
#include "wfd_cfg_parser.h"
#include <string>
#include <vector>
#include <map>
#include <cutils/properties.h>
using namespace std;
using std::string;
typedef enum DeviceType {
    SOURCE,
    PRIMARY_SINK,
    SECONDARY_SINK,
    UNKNOWN,
}wfdDevType;
wfdDevType wfdDevice;

#ifdef __cplusplus
extern "C" {
#endif
      string getValueForTag(string buffer, string tag);
      void dumpcapabilitiesRead();
      void parseHWID();
      void parseXml(string buffer);

      void parseAudioCodec(string, audioCodecName);
      void parseVideoHeader(string);
      void parseVideoCodec(string, int index);
      void parseStandby(string);
      void parseidrRequest(string buffer);
      void parseFrameSkipping(string buffer);
      void parseHDCPSupport(string buffer);
      void parseRTPPortType(string buffer);
      unsigned getVideoModes(long int macroBlocks, int videoModesCount, int uMaxHRes, int uMaxVRes);
      void updateVar(const char* key, const cfgKeys cfgKey, const string tagValue);
      int getSupportedHDCPVersion(string version);
      void parseRIRSupport(string buffer);
      void parseDynamicEncoderCfg(string buffer);
#ifdef __cplusplus
}
#endif
#define VDEC_LOG_HIGH  1 // MMOSAL Macro for priting HIGH messages

#include "MMDebugMsg.h"

bool bRTPDumpEnable;
int mnHW_ID;
typedef pair<int,int> pair_val;
vector<int> cfgItems(TOTAL_CFG_KEYS,-1);
map<string,pair_val> cfgItemsStr;

/* =============================================================================
                         Macro Definitions
* =========================================================================== */

#define MACRO_BLOCK_SIZE 256
#define MACRO_BLOCKS(w,h,fps)  (((w*h)/MACRO_BLOCK_SIZE)*fps)

#define NUM_CEA_MODES   17
#define NUM_VESA_MODES  30
#define NUM_HH_MODES    12

#define LPCM_MODE_FREQ_1 44100
#define LPCM_MODE_FREQ_2 48000

#define AAC_AC3_MODE_FREQ 48000

#define MAX_SUPPORTED_WIDTH  1920
#define MAX_SUPPORTED_HEIGHT 1080
#define MAX_SUPPORTED_FPS 60
#define MAX_ALLOWED_FPS_720P 30

#define AUDIO_2_CHANNELS 2
#define AUDIO_4_CHANNELS 4
#define AUDIO_6_CHANNELS 6
#define AUDIO_8_CHANNELS 8

#define AUDIO_MIN_CHANNELS AUDIO_2_CHANNELS
#define AUDIO_MAX_CHANNELS AUDIO_8_CHANNELS

/* =============================================================================
                         Constant Data Declarations
* =========================================================================== */

/**********************************************
* Macro Blocks values for each modes of video
* calculated as ((width*height)/(16*16))* fps)
***********************************************/
/* For commertilazation only 720p resolution is supported */
/* Valid bit is set based on 2 factors 1) Height and Width multiple of 16*/
/* Resolution is less than or equal to 1280*720p 30 fps*/


typedef struct {
int ceaModeMB[NUM_CEA_MODES];
int ceaValidRes[NUM_CEA_MODES];
int ceaWidth[NUM_CEA_MODES];
int ceaHeight[NUM_CEA_MODES];
}ceaStruct;

static ceaStruct ceaVideoModeData = {
   {
      72000, // 640x480p60
      81000, // 720x480p60
      81000, // 720x480i60
      81000, // 720x576p50
      81000, // 720x576i50
      108000, // 1280x720p30
      216000, // 1280x720p60
      243000, // 1920x1080p30
      486000, // 1920x1080p60
      486000, // 1920x1080i60
      90000, // 1280x720p25
      180000, // 1280x720p50
      202500, // 1920x1080p25
      405000, // 1920x1080p50
      405000, // 1920x1080i50
      86400, // 1280x720p24
      194400// 1920x1080p24
   },
   {  // CEA VALID MODES
      1, // 640x480p60
      1, // 720x480p60
      0, // 720x480i60
      1, // 720x576p50
      0, // 720x576i50
      1, // 1280x720p30
      1, // 1280x720p60
      1, // 1920x1080p30
      1, // 1920x1080p60
      0, // 1920x1080i60
      1, // 1280x720p25
      1, // 1280x720p50
      1, // 1920x1080p25
      1, // 1920x1080p50
      0, // 1920x1080i50
      1, // 1280x720p24
      1// 1920x1080p24
   },
   { // CEA WIDTH
      640,
      720,
      720,
      720,
      720,
      1280,
      1280,
      1920,
      1920,
      1920,
      1280,
      1280,
      1920,
      1920,
      1920,
      1280,
      1920
   },
   { // CEA HEIGHT
      480,
      480,
      480,
      576,
      576,
      720,
      720,
      1080,
      1080,
      1080,
      720,
      720,
      1080,
      1080,
      1080,
      720,
      1080
   }
};


typedef struct {
   int vesaModeMB[NUM_VESA_MODES];
   int vesaValidRes[NUM_VESA_MODES];
   int vesaWidth[NUM_VESA_MODES];
   int vesaHeight[NUM_VESA_MODES];
} vesaStruct;

static vesaStruct vesaModeData = {
    {
      56250, // 800x600p30
      112500, // 800x600p60
      92160, // 1024x768p30
      184320, // 1024x768p60
      116640, // 1152x864p30
      233280, // 1152x864p60
      115200, // 1280x768p30
      230400, // 1280x768p60
      120000, // 1280x800p30
      240000, // 1280x800p60
      122400, // 1360x768p30
      244800, // 1360x768p60
      122940, // 1366x768p30
      245880, // 1366x768p60
      153600, // 1280x1024p30
      307200, // 1280x1024p60
      172265, // 1400x1050p30
      344531, // 1400x1050p60
      151875, // 1440x900p30
      303750, // 1440x900p60
      168750, // 1600x900p30
      337500, // 1600x900p60
      225000, // 1600x1200p30
      450000, // 1600x1200p60
      201600, // 1680x1024p30
      403200, // 1680x1024p60
      206718, // 1680x1050p30
      413437, // 1680x1050p60
      270000, // 1920x1200p30
      540000  // 1920x1200p60
   },
   { // VESA VALID MODES
      1, // 800x600p30
      1, // 800x600p60
      0, // 1024x768p30
      0, // 1024x768p60
      0, // 1152x864p30
      0, // 1152x864p60
      0, // 1280x768p30
      0, // 1280x768p60
      0, // 1280x800p30
      0, // 1280x800p60
      0, // 1360x768p30
      0, // 1360x768p60
      0, // 1366x768p30
      0, // 1366x768p60
      0, // 1280x1024p30
      0, // 1280x1024p60
      0, // 1400x1050p30
      0, // 1400x1050p60
      0, // 1440x900p30
      0, // 1440x900p60
      0, // 1600x900p30
      0, // 1600x900p60
      0, // 1600x1200p30
      0, // 1600x1200p60
      0, // 1680x1024p30
      0, // 1680x1024p60
      0, // 1680x1050p30
      0, // 1680x1050p60
      0, // 1920x1200p30
      0// 1920x1200p60
   },
   { // VESA WIDTH
      800,
      800,
      1024,
      1024,
      1152,
      1152,
      1280,
      1280,
      1280,
      1280,
      1360,
      1360,
      1366,
      1366,
      1280,
      1280,
      1400,
      1400,
      1440,
      1440,
      1600,
      1600,
      1600,
      1600,
      1680,
      1680,
      1680,
      1680,
      1920,
      1920
   },
   { // VESA HEIGHT
      600,
      600,
      768,
      768,
      864,
      864,
      768,
      768,
      800,
      800,
      768,
      768,
      768,
      768,
      1024,
      1024,
      1050,
      1050,
      900,
      900,
      900,
      900,
      1200,
      1200,
      1024,
      1024,
      1050,
      1050,
      1200,
      1200
   }
};

typedef struct {
   int hhModeMB[NUM_HH_MODES];
   int hhValidRes[NUM_HH_MODES];
   int hhWidth[NUM_HH_MODES];
   int hhHeight[NUM_HH_MODES];
}hhStruct;

static hhStruct hhModeData = {
   {
      45000,  // 800x480p30
      90000,  // 800x480p60
      48037,  // 854x480p30
      96075,  // 854x480p60
      48600,  // 864x480p30
      97200,  // 864x480p60
      27000,  // 640x360p30
      54000,  // 640x360p60
      60750,  // 960x540p30
      121500, // 960x540p60
      47700,  // 848x480p30
      95400   // 848x480p60
   },
   { //HH Valid Bit
      1,// 800x480p30
      1,// 800x480p60
      0,// 854x480p30
      0,// 854x480p60
      1,// 864x480p30
      1,// 864x480p60
      1,// 640x360p30
      1,// 640x360p60
      1,// 960x540p30
      1,// 960x540p60
      1,// 848x480p30
      1// 848x480p60
   },
   { //HH Width
      800,
      800,
      854,
      854,
      864,
      864,
      640,
      640,
      960,
      960,
      848,
      848
   },
   { //HH Height
      480,
      480,
      480,
      480,
      480,
      480,
      360,
      360,
      540,
      540,
      480,
      480
   }
};



static readConfigFile *pReadCfgCapabilities;
static int *mbData;
static int *videoModeHeight;
static int *videoModeWidth;
static int *videoModeValidRes;


#ifdef __cplusplus
extern "C" {
#endif



/* =======================================================================
                Function Definitions
* ======================================================================= */


/*==============================================================================

         FUNCTION:         getVideoModes

         DESCRIPTION:
         @brief           All bits set to 1 whose MB count is less than the
                          calculated MB count
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - macroBlocks - calculated from user entered resolution
                                       - ((Vres*Hres*Fps)/(16*16))
                        - mbData - MB values for each resolution - pre calculated
                        - modeCount - number of resolutions supported count

*//*     RETURN VALUE:
*//**       @return
                        - uBitMap - video bitmap


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

unsigned  getVideoModes(long int macroBlocks, int videoModesCount, int uMaxHRes, int uMaxVRes )
{
   int uLoopCounter = 0;
   unsigned uBitMap = 0;

   if( mbData && videoModesCount )
   {
      for( uLoopCounter = (videoModesCount-1) ;uLoopCounter >= 0; uLoopCounter-- )
      {
         uBitMap <<= 1;
         /* Check if the valid bit is set and MB value less than user specified value */
         if( ( videoModeValidRes[uLoopCounter] ) && ( *( mbData + uLoopCounter ) <= macroBlocks )  &&
            (videoModeHeight[uLoopCounter] <= uMaxVRes) && (videoModeWidth[uLoopCounter] <= uMaxHRes) )
         {
            uBitMap |= 1;
         }
      }
   }
   return uBitMap;
}

/*==============================================================================

         FUNCTION:    getValueForTag

         DESCRIPTION:
         @brief       Get value corresponding to string "tag" in string "buffer"
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - buffer  - buffer to parser for tag
                        - tag - the tag to find in the buffer

*//*     RETURN VALUE:
*//**       @return
                        - tag  - identified tag

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

string  getValueForTag(string buffer, string tag)
{
   string tagStart = "<" + tag + ">";
   string tagEnd = "</" + tag + ">";
   string retString;
   size_t begin, end;

   if ((begin = buffer.find(tagStart)) != string::npos)
   {
      if ((end = buffer.find(tagEnd)) != string::npos)
      {
         begin += tagStart.length();
         size_t len = end - begin;
         char *resStr = (char *)MM_Malloc(static_cast<int>(sizeof(char)*(len+1)));
         if(!resStr)
         {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"getValueForTag: Memory Allocation Failure");
            return "";
         }
         size_t stop = buffer.copy(resStr, len, begin);
         resStr[stop] = '\0';
         retString = string(resStr);
         MM_Free(resStr);
      }
   }
   return retString;
}

/*==============================================================================

         FUNCTION:    parseUIBCSupport

         DESCRIPTION:
         @brief       Parse XML for UIBC Support capabilities
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - buffer - buffer to parser for tag

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void  parseUIBCSupport(string uibcBuffer)
{
   string tagValue;
   bool bValid = false, bGeneric = false, bHID = false;

   if(!uibcBuffer.c_str())
   {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"parseUIBCSupport: buffer is null");
      return;
   }
   tagValue = getValueForTag(uibcBuffer, UIBC_VALID_KEY);
   bValid = atoi(tagValue.c_str()) ? true : false;
   if(tagValue.length())
   {
     updateVar(UIBC_VALID_KEY,UIBC_VALID,tagValue);
   }
   if(bValid)
   {
      pReadCfgCapabilities->uibcCapability.port_id =
               ( uint16 ) atoi((getValueForTag(uibcBuffer, "TcpPort")).c_str()) ;

      tagValue = getValueForTag(uibcBuffer,"InputCategory");
      bGeneric = atoi((getValueForTag(uibcBuffer, "Generic")).c_str()) ? true : false;
      bHID = atoi((getValueForTag(uibcBuffer, "HID")).c_str()) ? true : false;
      tagValue = getValueForTag(uibcBuffer,UIBC_M14_KEY);
      if(tagValue.length())
      {
        updateVar(UIBC_M14_KEY,UIBC_M14,tagValue);
      }
      if(bGeneric)
      {
         pReadCfgCapabilities->uibcCapability.config.category |= GENERIC;
         tagValue = getValueForTag(uibcBuffer,"GenericInputEvents");

         pReadCfgCapabilities->uibcCapability.config.generic_input_type = 0;

         if(atoi((getValueForTag(tagValue, "Keyboard")).c_str()) ? true : false)
         {
            pReadCfgCapabilities->uibcCapability.config.generic_input_type |= KEYBOARD;
         }
         if(atoi((getValueForTag(tagValue, "Mouse")).c_str()) ? true : false)
         {
            pReadCfgCapabilities->uibcCapability.config.generic_input_type |= MOUSE;
         }
         if(atoi((getValueForTag(tagValue, "SingleTouch")).c_str()) ? true : false)
         {
            pReadCfgCapabilities->uibcCapability.config.generic_input_type |= SINGLETOUCH;
         }
         if(atoi((getValueForTag(tagValue, "MultiTouch")).c_str()) ? true : false)
         {
            pReadCfgCapabilities->uibcCapability.config.generic_input_type |= MULTITOUCH;
         }
         if(atoi((getValueForTag(tagValue, "JoyStick")).c_str()) ? true : false)
         {
            pReadCfgCapabilities->uibcCapability.config.generic_input_type |= JOYSTICK;
         }
         if(atoi((getValueForTag(tagValue, "Gesture")).c_str()) ? true : false)
         {
            pReadCfgCapabilities->uibcCapability.config.generic_input_type |= GESTURE;
         }
         if(atoi((getValueForTag(tagValue, "Camera")).c_str()) ? true : false)
         {
            pReadCfgCapabilities->uibcCapability.config.generic_input_type |= CAMERA;
         }
         if(atoi((getValueForTag(tagValue, "RemoteControl")).c_str()) ? true : false)
         {
            pReadCfgCapabilities->uibcCapability.config.generic_input_type |= REMOTECONTROL;
         }
         MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"parseUIBCSupport: generic input = %d",pReadCfgCapabilities->uibcCapability.config.generic_input_type);
      }
      if(bHID)
      {
        pReadCfgCapabilities->uibcCapability.config.category |= HIDC;
        string sHIDSup = getValueForTag(uibcBuffer,"HIDInputPaths");
        int nHIDInpPath = 0;
        uint8 uHIDBitset = 0;
        for(int i= 0; i< UIBC_NUM_INPUT_PATHS; i++)
        {
            pReadCfgCapabilities->uibcCapability.config.hid_input_type_path[i]= 0;
            switch(i)
            {
                case 0:
                    nHIDInpPath = INFRARED;
                    uHIDBitset = (uint8)atoi((getValueForTag(sHIDSup,"Infrared")).c_str());
                    break;
                case 1:
                    nHIDInpPath = USB;
                    uHIDBitset = (uint8)atoi((getValueForTag(sHIDSup,"USB")).c_str());
                    break;
                case 2:
                    nHIDInpPath = BT;
                    uHIDBitset = (uint8)atoi((getValueForTag(sHIDSup,"BT")).c_str());
                    break;
                case 3:
                    nHIDInpPath = ZIGBEE;
                    uHIDBitset = (uint8)atoi((getValueForTag(sHIDSup,"Zigbee")).c_str());
                    break;
                case 4:
                    nHIDInpPath = WIFI;
                    uHIDBitset = (uint8)atoi((getValueForTag(sHIDSup,"Wifi")).c_str());
                    break;
                case 5:
                    nHIDInpPath = NOSP;
                    uHIDBitset = (uint8)atoi((getValueForTag(sHIDSup,"NoSP")).c_str());
                    break;
                default:
                    nHIDInpPath = NOSP;
                    uHIDBitset = (uint8)atoi((getValueForTag(sHIDSup,"NoSP")).c_str());
            }
            pReadCfgCapabilities->uibcCapability.config.hid_input_type_path[nHIDInpPath]=uHIDBitset;
        }
      }
      if(!bGeneric && !bHID)
      {
         pReadCfgCapabilities->uibcCapability.port_id = 0;
         pReadCfgCapabilities->uibcCapability.config.category= 0;
         pReadCfgCapabilities->uibcCapability.config.generic_input_type = 0;
         return;
      }
   }
   else
   {
      pReadCfgCapabilities->uibcCapability.port_id = 0;
      pReadCfgCapabilities->uibcCapability.config.category= 0;
      pReadCfgCapabilities->uibcCapability.config.generic_input_type = 0;
      return;
   }
}

/*==============================================================================

         FUNCTION:    parseStandby

         DESCRIPTION:
         @brief       Parse XML for stand by resume capabilities
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - buffer - buffer to parser for tag
                        - tag - the tag to find in the buffer

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void  parseStandby(string standByBuffer)
{
   bool valid = false;
   valid = atoi((getValueForTag(standByBuffer, "Valid")).c_str()) ? true : false;
   pReadCfgCapabilities->pCfgCapabilities->standby_resume_support = valid;
}

/*==============================================================================

         FUNCTION:    parseidrRequest

         DESCRIPTION:
         @brief       Parse XML for IDR Request validity and interval
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - buffer - buffer to parser for tag
                        - tag - the tag to find in the buffer

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void  parseidrRequest(string idrReqBuffer)
{
   pReadCfgCapabilities->idrRequestCapability.idrReqValid
      = atoi((getValueForTag(idrReqBuffer, "Valid")).c_str()) ? true : false;
   pReadCfgCapabilities->idrRequestCapability.idrIntvl
      = (unsigned long)atoi((getValueForTag(idrReqBuffer, "IDRInterval")).c_str());
}

/*==============================================================================

         FUNCTION:    parseRIRSupport

         DESCRIPTION:
         @brief       Parse XML for Random IR validity and interval
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - buffer - buffer to parser for tag

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void  parseRIRSupport(string buffer)
{
    if(!buffer.c_str())
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid param to parseRIRSupport");
        return;
    }
    string tagvalue = getValueForTag(buffer,PERIODIC_IDR_INTERVAL_VALID_KEY);
    if(tagvalue.length())
    {
        updateVar(PERIODIC_IDR_INTERVAL_VALID_KEY,PERIODIC_IDR_INTERVAL_VALID,tagvalue);
    }
    tagvalue = getValueForTag(buffer,PERIODIC_IDR_INTERVAL_KEY);
    if(tagvalue.length())
    {
        updateVar(PERIODIC_IDR_INTERVAL_KEY,PERIODIC_IDR_INTERVAL,tagvalue);
    }
}

/*==============================================================================

         FUNCTION:    parseDynamicEncoderCfg

         DESCRIPTION:
         @brief       Parse XML for configurable encoder settings
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - buffer - buffer to parser for tag

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void  parseDynamicEncoderCfg(string buffer)
{
    if(!buffer.c_str())
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid param to parseDynamicEncoderCfg");
        return;
    }
    string tagvalue = getValueForTag(buffer,NARROW_SEARCH_RANGE_KEY);
    if(tagvalue.length())
    {
        int value = atoi(tagvalue.c_str());
        if((value == 0) &&
           ((mnHW_ID == 206)|| (mnHW_ID>=239 && mnHW_ID <= 243)||
            (mnHW_ID>=268 && mnHW_ID <= 271)))
        {
          tagvalue = string("1");
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"wfd_cfg_parser : Updating NARROW_SEARCH_RANGE_KEY to 1");
        }
        updateVar(NARROW_SEARCH_RANGE_KEY,NARROW_SEARCH_RANGE,tagvalue);
    }
}

/*==============================================================================

         FUNCTION:    parseFrameSkipping

         DESCRIPTION:
         @brief        Parse XML for Frame Skipping validity and interval
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - buffer - buffer to parser for tag
                        - tag - the tag to find in the buffer

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void  parseFrameSkipping(string fsBuffer)
{
   pReadCfgCapabilities->frameSkippingCapability.frameSkipValid
      = atoi((getValueForTag(fsBuffer, "Valid")).c_str()) ? true : false;
   pReadCfgCapabilities->frameSkippingCapability.frameSkipIntvl
      = (unsigned short)atoi((getValueForTag(fsBuffer, "FrameInterval")).c_str());
   if(pReadCfgCapabilities->frameSkippingCapability.frameSkipValid)
   {
     for(int index=0; index < pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.num_h264_profiles; index++)
     {
        ( pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).frame_rate_control_support |= 0X01;
        ( pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).frame_rate_control_support |=static_cast<uint8>((pReadCfgCapabilities->frameSkippingCapability.frameSkipIntvl << 1));
     }
   }
}

/*==============================================================================

         FUNCTION:    parseHDCPSupport

         DESCRIPTION:
         @brief        Parse XML for Content Protection validity
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - buffer - buffer to parser for tag
                        - tag - the tag to find in the buffer

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void  parseHDCPSupport(string hdcpBuffer)
{
   bool bValid = false;
   bValid = atoi((getValueForTag(hdcpBuffer, "Valid")).c_str()) ? true : false;
   if(bValid)
   {
      string version((getValueForTag(hdcpBuffer,"Version")).c_str()) ;

      pReadCfgCapabilities->pCfgCapabilities->content_protection_config.content_protection_capability
               = static_cast<uint8>(getSupportedHDCPVersion(version));
      pReadCfgCapabilities->pCfgCapabilities->content_protection_config.content_protection_ake_port
               = ( uint16 ) atoi((getValueForTag(hdcpBuffer, "CPPort")).c_str()) ;
   }
   else
   {
      /* 0 in HDCP version indicates no support for HDCP */
      pReadCfgCapabilities->pCfgCapabilities->content_protection_config.content_protection_capability = 0;
      pReadCfgCapabilities->pCfgCapabilities->content_protection_config.content_protection_ake_port = 0;
   }
}

/*==============================================================================

         FUNCTION:    parseRTPPortType

         DESCRIPTION:
         @brief        Parse XML for RTP port connection type
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - buffer - buffer to parser for tag

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void  parseRTPPortType(string rtpBuffer)
{
   pReadCfgCapabilities->pCfgCapabilities->transport_capability_config.eRtpPortType =
      (WFD_rtp_port_type )atoi((getValueForTag(rtpBuffer, "ConnectionType")).c_str()) ;
   MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"RTP Port Connection Type = %d",
   pReadCfgCapabilities->pCfgCapabilities->transport_capability_config.eRtpPortType);
}

/*==============================================================================

         FUNCTION:    parseRTPDumpEnable

         DESCRIPTION:
         @brief        Parse XML for RTP DumpEnable boolean flag
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - buffer - buffer to parser for tag

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void  parseRTPDumpEnable(string buffer)
{
   bRTPDumpEnable = atoi((getValueForTag(buffer, "Valid")).c_str()) ? true : false;
   cfgItems.at(RTP_DUMP_ENABLE)= atoi((getValueForTag(buffer, "Valid")).c_str());
   MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_LOW,"RTP Dump Enable = %d",bRTPDumpEnable);
}

/*==============================================================================

         FUNCTION:    parseVideoHeader

         DESCRIPTION:
         @brief        Parse XML for Video Header Parameters
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - buffer - videoBuffer to parser for tag
                        - tag - the tag to find in the buffer

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void  parseVideoHeader(string vidheadbuffer)
{

   pReadCfgCapabilities->pCfgCapabilities->video_method
      = (WFD_video_type )atoi((getValueForTag(vidheadbuffer, "VideoMethod")).c_str());
   pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.preferred_display_mode_supported
      = (uint8)atoi((getValueForTag(vidheadbuffer, "PreferredDisplaySupport")).c_str());
   pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.num_h264_profiles
      = (uint8)atoi((getValueForTag(vidheadbuffer, "H264Profiles")).c_str());
}

/*==============================================================================

         FUNCTION:    parseVideoCodec

         DESCRIPTION:
         @brief        Parse XML for Video Codec Parameters
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param      - videoBuffer - buffer to parser for tag
                        - tag - the tag to find in the videoBuffer

*//*     RETURN VALUE:
*//**       @return
                           None

@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

void  parseVideoCodec(string videoBuffer, int index)
{
   int fpsSupported;
   uint8 nativeModes = 0;
   uint16 uMaxHRes,uMaxVRes;
   long int uMacroBlocks = 0;
   int ceaModeCount = NUM_CEA_MODES;
   int vesaModeCount = NUM_VESA_MODES;
   int hhModeCount = NUM_HH_MODES;
   uint32  hhBitMap = 0, vesaBitMap = 0, ceaBitMap = 0;

   /* Profile Values : CBP or CHP */
   string  profile((getValueForTag(videoBuffer, "Profile")).c_str());
   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).h264_profile = (uint8)atoi((getValueForTag(videoBuffer, "Profile")).c_str());
/*    = profile.compare("CBP")?
            ( profile.compare("CHP") ? H264_SUPPORTED_PROFILE_CBP : H264_SUPPORTED_PROFILE_CHP )
        : BIT0; */

   MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"wfd_cfg_parser : user enterted profile %d for index %d",
   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).h264_profile, index);

   /* Level values : 3.1/3.2/4/4.1/4.2*/
   string  level((getValueForTag(videoBuffer, "Level")).c_str());

   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).h264_level
   = level.compare("3.1") ?
      (level.compare("3.2")?
         (level.compare("4")?
            (level.compare("4.1")?
               (level.compare("4.2")? BIT0   // Level 3.1, BIT0 by default
                                    : BIT4 )
            : BIT3 )
         : BIT2 )
         : BIT1 )
     : BIT0;

    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"wfd_cfg_parser: user enterted level %d ",
   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).h264_level);

   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).max_hres = uMaxHRes
      = (uint16)atoi((getValueForTag(videoBuffer, "HorizontalResolution")).c_str());
   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).max_vres
      = uMaxVRes = (uint16)atoi((getValueForTag(videoBuffer, "VerticalResolution")).c_str());
   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).decoder_latency
      = (uint8)atoi((getValueForTag(videoBuffer, "Latency")).c_str());
   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).min_slice_size
      = (uint16)atoi((getValueForTag(videoBuffer, "MinimumSliceSize")).c_str());
   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).slice_enc_params
      = (uint8)atoi((getValueForTag(videoBuffer, "SliceEncodingParams")).c_str());
   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).frame_rate_control_support
      = (uint8)atoi((getValueForTag(videoBuffer, "FrameRateControlSupport")).c_str());

   fpsSupported = (unsigned int )atoi((getValueForTag(videoBuffer, "VideoFps")).c_str());

   MM_MSG_PRIO3(MM_GENERAL,MM_PRIO_HIGH,"wfd_cfg_parser : user enterted reolution %d x %d %d fps",uMaxHRes,uMaxVRes,fpsSupported);

   switch(mnHW_ID)
   {
     case 206:
     case 268:
     case 269:
     case 270:
     case 271:/*SOC IDs which will support max 720p WFD */
        if(wfdDevice == SOURCE && (uMaxHRes > 1280 || uMaxVRes > 720))
        {
            uMaxHRes = 1280;
            (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).max_hres = 1280;
            uMaxVRes = 720;
            (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).max_vres = 720;
            MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"wfd_cfg_parser : Updated resolution %d x %d ",uMaxHRes,uMaxVRes);
        }
        break;

      case 239:
      case 240:
      case 241:
      case 242:
      case 243: /* Place Holder if something is needed. */
      default:
        break;
   }
   if((uMaxHRes <= MAX_SUPPORTED_WIDTH)&& (uMaxVRes <= MAX_SUPPORTED_HEIGHT)
                                       && (fpsSupported <= MAX_SUPPORTED_FPS))
   {
      uMacroBlocks = MACRO_BLOCKS(uMaxHRes, uMaxVRes, fpsSupported);
   }
   else
   {
      /* if user specified resolution is greater than 1280*720 30 fps default it to 1280*720 30 fps*/
      uMacroBlocks = MACRO_BLOCKS(MAX_SUPPORTED_WIDTH, MAX_SUPPORTED_HEIGHT, MAX_ALLOWED_FPS_720P);
   }

   mbData = ceaVideoModeData.ceaModeMB;
   videoModeHeight = ceaVideoModeData.ceaHeight;
   videoModeWidth = ceaVideoModeData.ceaWidth;
   videoModeValidRes = ceaVideoModeData.ceaValidRes;

   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).supported_cea_mode
      = ceaBitMap = getVideoModes(uMacroBlocks, ceaModeCount, uMaxHRes, uMaxVRes );
   mbData = vesaModeData.vesaModeMB;
   videoModeHeight = vesaModeData.vesaHeight;
   videoModeWidth = vesaModeData.vesaWidth;
   videoModeValidRes = vesaModeData.vesaValidRes;

   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).supported_vesa_mode
      = vesaBitMap = getVideoModes(uMacroBlocks, vesaModeCount, uMaxHRes, uMaxVRes );

   mbData = hhModeData.hhModeMB;
   videoModeHeight = hhModeData.hhHeight;
   videoModeWidth = hhModeData.hhWidth;
   videoModeValidRes = hhModeData.hhValidRes;

   (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).supported_hh_mode
      = hhBitMap = getVideoModes(uMacroBlocks, hhModeCount, uMaxHRes, uMaxVRes );

    nativeModes = vesaBitMap ? 1 : 0;
    nativeModes = hhBitMap ? (1 << 1) : 0 ;

   // Default bit map CEA BIT0, nativeModes = 0;
    if(!nativeModes && !ceaBitMap && !vesaBitMap && !hhBitMap)
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Defaulting to VGA");
      (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).supported_cea_mode = H264_CEA_640x480p60;
      (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).max_hres = 640;
      (pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).max_vres = 480;
       nativeModes = 0;
    }
    pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.native_bitmap = nativeModes;
}


/*==============================================================================

      FUNCTION:    parseAudioCodec

      DESCRIPTION:
      @brief        Parse XML for Audio Codec Entries
*//**

@par     DEPENDENCIES:
                        None
*//*
      PARAMETERS:
*//**       @param      - buffer - buffer to parser for tag
                     - tag - the tag to find in the buffer

*//*     RETURN VALUE:
*//**       @return
                        None

@par     SIDE EFFECTS:
                        None

*//*==========================================================================*/

void  parseAudioCodec(string audioBuffer, audioCodecName type)
{
   switch(type)
   {
      case LPCM:
      {
         unsigned long samplingFreq;
         string name = getValueForTag(audioBuffer, "Name");
         bool valid = atoi((getValueForTag(audioBuffer, "Valid")).c_str()) ? true : false;
         int channels = atoi((getValueForTag(audioBuffer, "Channels")).c_str());
         samplingFreq = (unsigned long)atoi((getValueForTag(audioBuffer, "SamplingFreq")).c_str());

         if( valid && ( samplingFreq == LPCM_MODE_FREQ_1 || samplingFreq == LPCM_MODE_FREQ_2 ) && ( channels == AUDIO_MIN_CHANNELS ) )
         {
            if( LPCM_MODE_FREQ_1 == samplingFreq )
            {
               pReadCfgCapabilities->pCfgCapabilities->audio_config.lpcm_codec.supported_modes_bitmap = LPCM_441_16_2 ;
               MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Currently this LPCM Freq is not supported");
            }
            else if(LPCM_MODE_FREQ_2 == samplingFreq )
            {
               pReadCfgCapabilities->pCfgCapabilities->audio_config.lpcm_codec.supported_modes_bitmap = LPCM_48_16_2 ;// BIT1 only ,44.1 is not supoprted right now
            }
            pReadCfgCapabilities->pCfgCapabilities->audio_config.lpcm_codec.decoder_latency = (uint8)atoi((getValueForTag(audioBuffer, "Latency")).c_str());
         }
         else
         {
            // Debug Msg - Invalid data entered
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid data entered for LPCM");
         }
      }
      break;
      case AAC:
      {
         unsigned long samplingFreq;
         string name = getValueForTag(audioBuffer, "Name");
         bool valid = atoi((getValueForTag(audioBuffer, "Valid")).c_str()) ? true : false;
         int channels = atoi((getValueForTag(audioBuffer, "Channels")).c_str());
         samplingFreq = (unsigned long)atoi((getValueForTag(audioBuffer, "SamplingFreq")).c_str());

         if( valid && ( samplingFreq == AAC_AC3_MODE_FREQ ) && ( ( channels >= AUDIO_MIN_CHANNELS )
                         && ( channels <= AUDIO_MAX_CHANNELS )
                         && ( ( channels%2 ) == 0 ) ) )
         {
            switch(channels)
            {
             case AUDIO_MIN_CHANNELS:
             case AUDIO_4_CHANNELS:
                  pReadCfgCapabilities->pCfgCapabilities->audio_config.aac_codec.supported_modes_bitmap = AAC_48_16_2;
             break;
             case AUDIO_6_CHANNELS:
                  pReadCfgCapabilities->pCfgCapabilities->audio_config.aac_codec.supported_modes_bitmap = (AAC_48_16_2 | AAC_48_16_6);
             break;
             case AUDIO_MAX_CHANNELS:
                  pReadCfgCapabilities->pCfgCapabilities->audio_config.aac_codec.supported_modes_bitmap = (AAC_48_16_2 | AAC_48_16_6 | AAC_48_16_8);
             break;
            }
            pReadCfgCapabilities->pCfgCapabilities->audio_config.aac_codec.decoder_latency =
            (uint8)atoi((getValueForTag(audioBuffer, "Latency")).c_str());
         }
         else
         {
            // Invalid AAC Values
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid data entered for AAC");
         }
      }
      break;
      case AC3:
      {
         unsigned long samplingFreq;
         string name = getValueForTag(audioBuffer, "Name");
         bool valid = atoi((getValueForTag(audioBuffer, "Valid")).c_str()) ? true : false;
         int channels = atoi((getValueForTag(audioBuffer, "Channels")).c_str());
         samplingFreq = (unsigned long)atoi((getValueForTag(audioBuffer, "SamplingFreq")).c_str());

         if( valid && ( samplingFreq == AAC_AC3_MODE_FREQ ) && ( ( channels >= AUDIO_MIN_CHANNELS )
         && ( channels <= AUDIO_MAX_CHANNELS )
         && ( ( channels%2 ) == 0 ) ) )
         {
            switch(channels)
            {
               case AUDIO_MIN_CHANNELS:
                  pReadCfgCapabilities->pCfgCapabilities->audio_config.dolby_digital_codec.supported_modes_bitmap = DOLBY_DIGITAL_48_16_2_AC3;
               break;
               case AUDIO_4_CHANNELS:
                  pReadCfgCapabilities->pCfgCapabilities->audio_config.dolby_digital_codec.supported_modes_bitmap = (DOLBY_DIGITAL_48_16_2_AC3 | DOLBY_DIGITAL_48_16_4_AC3);
               break;
               case AUDIO_6_CHANNELS:
                  pReadCfgCapabilities->pCfgCapabilities->audio_config.dolby_digital_codec.supported_modes_bitmap = (DOLBY_DIGITAL_48_16_2_AC3 | DOLBY_DIGITAL_48_16_4_AC3 | DOLBY_DIGITAL_48_16_6_AC3);
               break;
               case AUDIO_8_CHANNELS:
                  pReadCfgCapabilities->pCfgCapabilities->audio_config.dolby_digital_codec.supported_modes_bitmap = (DOLBY_DIGITAL_48_16_2_AC3 | DOLBY_DIGITAL_48_16_4_AC3 | DOLBY_DIGITAL_48_16_6_AC3 | DOLBY_DIGITAL_48_16_8_EAC3);
               break;
            }
            pReadCfgCapabilities->pCfgCapabilities->audio_config.dolby_digital_codec.decoder_latency =
            (uint8)atoi((getValueForTag(audioBuffer, "Latency")).c_str());
         }
         else
         {
            // Invalid AC3 Values
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid data entered for AC3");
         }
      }
      break;
      default:
         MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Invalid Audio type");
      break;
   }
   if(pReadCfgCapabilities->pCfgCapabilities->audio_config.lpcm_codec.supported_modes_bitmap ||
      pReadCfgCapabilities->pCfgCapabilities->audio_config.aac_codec.supported_modes_bitmap ||
      pReadCfgCapabilities->pCfgCapabilities->audio_config.dolby_digital_codec.supported_modes_bitmap)
   {
      // AC3 is of priority check if it is set and assign the audio method to it
      if(pReadCfgCapabilities->pCfgCapabilities->audio_config.dolby_digital_codec.supported_modes_bitmap)
         pReadCfgCapabilities->pCfgCapabilities->audio_method = WFD_AUDIO_DOLBY_DIGITAL;
      // AAC is 2nd priority
      else if(pReadCfgCapabilities->pCfgCapabilities->audio_config.aac_codec.supported_modes_bitmap)
         pReadCfgCapabilities->pCfgCapabilities->audio_method = WFD_AUDIO_AAC;
      // LPCM last option
      else
         pReadCfgCapabilities->pCfgCapabilities->audio_method = WFD_AUDIO_LPCM;
   }
   else
   {
      //Default LPCM BIT0
      pReadCfgCapabilities->pCfgCapabilities->audio_method = WFD_AUDIO_LPCM;
      pReadCfgCapabilities->pCfgCapabilities->audio_config.lpcm_codec.supported_modes_bitmap = LPCM_48_16_2;
   }
}

/*==============================================================================

   FUNCTION:    parseHWID

   DESCRIPTION:
   @brief        Extracts target ID information
                 to properly update properties.

   DEPENDENCIES: None

   PARAMETERS:
       @param -  None

   RETURN VALUE:
       @return - None

   SIDE EFFECTS: None

*//*==========================================================================*/

void parseHWID()
{
    mnHW_ID = -1;
    int result = -1;
    char buffer[10];
    FILE *device = NULL;
    device = fopen("/sys/devices/soc0/soc_id", "r");
    if(device)
    {
      /* 4 = 3 (MAX_SOC_ID_LENGTH) + 1 */
      result = fread(buffer, 1, 4, device);
      fclose(device);
    }
    else
    {
      device = fopen("/sys/devices/system/soc/soc0/id", "r");
      if(device)
      {
         result = fread(buffer, 1, 4, device);
         fclose(device);
      }
    }
    if(result > 0)
    {
       mnHW_ID = atoi(buffer);
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"Got HW_ID = %d",mnHW_ID);
}

/*==============================================================================

   FUNCTION:    parseXml

   DESCRIPTION:
   @brief        Top level XML Configuration parsing
*//**

@par     DEPENDENCIES:
                     None
*//*
   PARAMETERS:
*//**       @param - cfgBuffer - xml data

*//*     RETURN VALUE:
*//**       @return
                     None

@par     SIDE EFFECTS:
                     None

*//*==========================================================================*/
void  parseXml(string cfgBuffer)
{
   cfgItems.clear();
   cfgItems.assign(TOTAL_CFG_KEYS,-1);
   cfgItemsStr.clear();
   string tagValue;
   parseHWID();

   tagValue = getValueForTag(cfgBuffer, "AudioLPCM");
   if (tagValue.length())
      parseAudioCodec(tagValue, LPCM);

   tagValue = getValueForTag(cfgBuffer, "AudioAAC");
   if (tagValue.length())
      parseAudioCodec(tagValue, AAC);

   tagValue = getValueForTag(cfgBuffer, "AudioAC3");
   if (tagValue.length())
      parseAudioCodec(tagValue, AC3);

   tagValue = getValueForTag(cfgBuffer, "VideoHeader");
   if (tagValue.length())
      parseVideoHeader(tagValue);

   {
      string sVideoCodec = getValueForTag(cfgBuffer, "CBP");
      if(sVideoCodec.length())
      {
         tagValue = getValueForTag(sVideoCodec, "VideoCodec");
      }
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"PARSE CBP");
      if (tagValue.length())
         parseVideoCodec(tagValue, 0);

      if(pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.num_h264_profiles > 0)
      {
         string sVideoCodec = getValueForTag(cfgBuffer, "CHP");
         if(sVideoCodec.length())
         {
            tagValue = getValueForTag(sVideoCodec, "VideoCodec");
         }
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"PARSE CHP");
         if (tagValue.length())
            parseVideoCodec(tagValue, 1);
      }
   }

   tagValue = getValueForTag(cfgBuffer, "StandbyResumeCapability");
   if (tagValue.length())
      parseStandby(tagValue);

   tagValue = getValueForTag(cfgBuffer, "IDRRequest");
   if (tagValue.length())
      parseidrRequest(tagValue);

   tagValue = getValueForTag(cfgBuffer, "FrameSkipping");
   if (tagValue.length())
      parseFrameSkipping(tagValue);

   tagValue = getValueForTag(cfgBuffer, "ContentProtection");
   if (tagValue.length())
      parseHDCPSupport(tagValue);

   tagValue = getValueForTag(cfgBuffer, "RTPPortType");
   if (tagValue.length())
      parseRTPPortType(tagValue);

   tagValue = getValueForTag(cfgBuffer, "RTPDumpEnable");
   if (tagValue.length())
      parseRTPDumpEnable(tagValue);

   tagValue = getValueForTag(cfgBuffer,"UIBC");
   if (tagValue.length())
      parseUIBCSupport(tagValue);

   tagValue = getValueForTag(cfgBuffer,"PeriodicIDRSettings");
   if(tagValue.length())
   {
       parseRIRSupport(tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,"DynamicEncoderConfig");
   if(tagValue.length())
   {
       parseDynamicEncoderCfg(tagValue);
   }

//Parse for configurable items here itself in order to cache for later use

   tagValue = getValueForTag(cfgBuffer,DISABLE_AVSYNC_MODE_KEY);
   if(tagValue.length())
   {
     updateVar(DISABLE_AVSYNC_MODE_KEY,DISABLE_AVSYNC_MODE,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,ENABLE_AUDIO_TRACK_LATENCY_MODE_KEY);
   if(tagValue.length())
   {
     updateVar(ENABLE_AUDIO_TRACK_LATENCY_MODE_KEY,ENABLE_AUDIO_TRACK_LATENCY_MODE,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,AUDIO_AVSYNC_DROP_WINDOW_KEY);
   if(tagValue.length())
   {
     updateVar(AUDIO_AVSYNC_DROP_WINDOW_KEY,AUDIO_AVSYNC_DROP_WINDOW,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,VIDEO_AVSYNC_DROP_WINDOW_KEY);
   if(tagValue.length())
   {
     updateVar(VIDEO_AVSYNC_DROP_WINDOW_KEY,VIDEO_AVSYNC_DROP_WINDOW,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,AUDIO_AV_SYNC_DEL_KEY);
   if(tagValue.length())
   {
     updateVar(AUDIO_AV_SYNC_DEL_KEY,AUDIO_AV_SYNC_DEL,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,AUDIO_IN_SUSPEND_KEY);
   if(tagValue.length())
   {
     updateVar(AUDIO_IN_SUSPEND_KEY,AUDIO_IN_SUSPEND,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,CYCLIC_IR_KEY);
   if(tagValue.length())
   {
     updateVar(CYCLIC_IR_KEY,CYCLIC_IR,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,CYCLIC_IR_NUM_MACRO_BLK_KEY);
   if(tagValue.length())
   {
     updateVar(CYCLIC_IR_NUM_MACRO_BLK_KEY,CYCLIC_IR_NUM_MACRO_BLK,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,DISABLE_NALU_FILLER_KEY);
   if(tagValue.length())
   {
     updateVar(DISABLE_NALU_FILLER_KEY,DISABLE_NALU_FILLER,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,DYN_BIT_ADAP_KEY);
   if(tagValue.length())
   {
     updateVar(DYN_BIT_ADAP_KEY,DYN_BIT_ADAP,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,ENCRYPT_AUDIO_DECISION_KEY);
   if(tagValue.length())
   {
     updateVar(ENCRYPT_AUDIO_DECISION_KEY,ENCRYPT_AUDIO_DECISION,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,ENCRYPT_NON_SECURE_KEY);
   if(tagValue.length())
   {
     updateVar(ENCRYPT_NON_SECURE_KEY,ENCRYPT_NON_SECURE,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,HDCP_ENFORCED_KEY);
   if(tagValue.length())
   {
     updateVar(HDCP_ENFORCED_KEY,HDCP_ENFORCED,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,VIDEO_PKTLOSS_FRAME_DROP_MODE_KEY);
   if(tagValue.length())
   {
     updateVar(VIDEO_PKTLOSS_FRAME_DROP_MODE_KEY,VIDEO_PKTLOSS_FRAME_DROP_MODE,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,MAX_FPS_SUPPORTED_KEY);
   if(tagValue.length())
   {
     if(((mnHW_ID == 206)|| (mnHW_ID>=239 && mnHW_ID <= 243) ||
          (mnHW_ID>=268 && mnHW_ID <= 271)))
     {
        /* For Lower End Target */
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"Setting MAX_FPS_SUPPORTED to 30");
        tagValue = string("30");
     }
     updateVar(MAX_FPS_SUPPORTED_KEY,MAX_FPS_SUPPORTED,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,PERF_LEVEL_PERF_MODE_KEY);
   if(tagValue.length())
   {
     updateVar(PERF_LEVEL_PERF_MODE_KEY,PERF_LEVEL_PERF_MODE,tagValue);
   }

   tagValue = getValueForTag(cfgBuffer,PERF_LEVEL_TURBO_MODE_KEY);
   if(tagValue.length())
   {
     if(mnHW_ID>=268 && mnHW_ID <= 271)
     {
       /* For target where venus does not have turbo clock */
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"Do not request turbo clock to encoder");
       tagValue = string("0");
     }
     updateVar(PERF_LEVEL_TURBO_MODE_KEY,PERF_LEVEL_TURBO_MODE,tagValue);
   }

}

/*==============================================================================

   FUNCTION:    parseCfg

   DESCRIPTION:
   @brief      Parse XML File presence, size and call the actual parsing routine
*//**

@par     DEPENDENCIES:
                     None
*//*
   PARAMETERS:
*//**       @param  - filename - xfg file name
                    - readCfgCaps - capabilities structure

*//*     RETURN VALUE:
*//**       @return
                     None

@par     SIDE EFFECTS:
                     None

*//*==========================================================================*/
void  parseCfg(const char *filename, readConfigFile *readCfgCaps)
{
   char *buffer;
   ssize_t bytesRead = 0;
   unsigned long length;
   MM_HANDLE pFileHandle;

   if(readCfgCaps)
   {
      pReadCfgCapabilities = readCfgCaps;
   }
   else
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parserCfg:readCfgCaps is NULL ");
      return;
   }

   if(!filename)
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"filename  is NULL");
      return;
   }

   if(!MM_File_Create(filename, MM_FILE_CREATE_R, &pFileHandle))
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"parserCfg:open config file successful");
   }
   else
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parserCfg:open config file failed");
      pFileHandle = NULL;
      return;
   }

   if(filename && strstr(filename,"sink"))
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parserCfg: parsing sink config file");
      wfdDevice = PRIMARY_SINK;
   }

   MM_File_Seek(pFileHandle, 0, MM_FILE_SEEK_END);
   MM_File_GetSize(pFileHandle, &length);
   MM_File_Seek(pFileHandle, 0, MM_FILE_SEEK_BEG);

   buffer = (char *) MM_Malloc(static_cast<int>(sizeof(char)*length));
   if(!buffer)
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parserCfg:Memory Allocation Failure");
      MM_File_Release(pFileHandle);
      pFileHandle = NULL;
      return;
   }

   MM_File_Read(pFileHandle, buffer, length, &bytesRead);

   if(length == (unsigned long) bytesRead)
   {
      string xmlbuffer(buffer);
      parseXml(xmlbuffer);
   }
   else
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parserCfg:File read failed");
      MM_File_Release(pFileHandle);
      pFileHandle = NULL;
      MM_Free(buffer);
      return;
   }

#if( VDEC_LOG_HIGH )
   dumpcapabilitiesRead();
#endif

   if(MM_File_Release(pFileHandle))
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parserCfg:File release failed");
   }

   MM_Free(buffer);
   return;
}

/*==============================================================================

   FUNCTION:    ParseCfgForBitrate

   DESCRIPTION:
   @brief      Parse min and max bitrates from cfg file mentioned.
*//**

@par     DEPENDENCIES:
                     None
*//*
   PARAMETERS:
*//**       @param  - mode - CEA VESA or HH
                    - bit  - bit which is set for the mode
                    - filename - cfg file name
                    - minBitrate - minimum Bitrate
                    - maxBitrate - maximum Bitrate


*//*     RETURN VALUE:
*//**       @return
                     None

@par     SIDE EFFECTS:
                     None

*//*==========================================================================*/
void ParseCfgForBitrate(int mode, int bit, char *pFilename, int *minBitrate, int *maxBitrate)
{
    #define SEARCHKEYLEN 6

    if(NULL == minBitrate || NULL == maxBitrate || bit > 31 || !pFilename)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "ParseBitrateTable: Invalid args");
        return;
    }
    *minBitrate = 0;
    *maxBitrate = 0;
    if(mode != WFD_MM_VESA_MODE && mode != WFD_MM_CEA_MODE
       && mode != WFD_MM_HH_MODE && mode != WFD_MM_AAC_MODE
       && mode != WFD_MM_DOLBY_DIGITAL_MODE)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "ParseBitrateTable: Invalid mode");
        return;
    }


    MM_HANDLE hFile = NULL;
    unsigned long nSize = 0;
    char  *pFileBuf = NULL;
    char aModeString[SEARCHKEYLEN];
    char aBitString[SEARCHKEYLEN];
    snprintf(aBitString, SEARCHKEYLEN, "%s%d","BIT",bit);

    if(MM_File_Create(pFilename,MM_FILE_CREATE_R, &hFile) != 0)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "ParseBitrateTable: Unable to open file");
        hFile = NULL;
        return;
    }

    MM_File_GetSize(hFile, &nSize);

    if(!nSize || nSize > 128*1024)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "ParseBitrateTable: File of 0 size or too large");
        MM_File_Release(hFile);
        hFile = NULL;
        return;
    }

    pFileBuf = (char*)MM_Malloc(static_cast<int>(nSize + 4));

    if(!pFileBuf)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "ParseBitrateTable: Malloc fail");
        MM_File_Release(hFile);
        hFile = NULL;
        return;
    }

    ssize_t bytesRead = 0;
    MM_File_Read(hFile, pFileBuf, nSize, &bytesRead);

    pFileBuf[nSize] = 0;

    if(mode == WFD_MM_CEA_MODE)
    {
        strlcpy(aModeString, "CEA", SEARCHKEYLEN);
    }
    else if(mode == WFD_MM_VESA_MODE)
    {
       strlcpy(aModeString, "VESA", SEARCHKEYLEN);
    }
    else if(mode == WFD_MM_HH_MODE)
    {
       strlcpy(aModeString, "HH", SEARCHKEYLEN);
    }
    else if(mode == WFD_MM_AAC_MODE)
    {
       strlcpy(aModeString, "AAC", SEARCHKEYLEN);
    }
    else if(mode == WFD_MM_DOLBY_DIGITAL_MODE)
    {
       strlcpy(aModeString, "AC3", SEARCHKEYLEN);
    }
    string sMode = getValueForTag(pFileBuf, aModeString);


    if(sMode.length())
    {
        string sBitrates = getValueForTag(sMode, aBitString);

        if(sBitrates.length())
        {
            *minBitrate = atoi((getValueForTag(sBitrates, "MinBitrate")).c_str());
            *maxBitrate = atoi((getValueForTag(sBitrates, "MaxBitrate")).c_str());
        }
    }

    MM_File_Release(hFile);
    hFile = NULL;
    if(pFileBuf != NULL)
    {
      MM_Free(pFileBuf);
    }
    return;

}

/*==============================================================================

   FUNCTION:    PargeCfgForIntValueForKey

   DESCRIPTION:
   @brief      Parse XML File for value of a given key
*//**

@par     DEPENDENCIES:
                     None
*//*
   PARAMETERS:
*//**       @param  - filename - cfg file name
                    - pKey - capabilities structure
                    - pVal

*//*     RETURN VALUE:
*//**       @return
                     None

@par     SIDE EFFECTS:
                     None

*//*==========================================================================*/
int PargeCfgForIntValueForKey(char *filename, char *pKey, int *pVal)
{
   ssize_t bytesRead = 0;
   unsigned long length;
   char *buffer;
   MM_HANDLE pFileHandle;

   if(!filename || !pKey || !pVal)
   {
      return -1;
   }

   if(!MM_File_Create(filename, MM_FILE_CREATE_R, &pFileHandle))
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW,"PargeCfgForIntValueForKey:open config file successful");
   }
   else
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"PargeCfgForIntValueForKey:open config file failed");
      pFileHandle = NULL;
      return -1;
   }

   MM_File_Seek(pFileHandle, 0, MM_FILE_SEEK_END);
   MM_File_GetSize(pFileHandle, &length);
   MM_File_Seek(pFileHandle, 0, MM_FILE_SEEK_BEG);

   buffer = (char *) MM_Malloc(static_cast<int>(sizeof(char)*length));
   if(!buffer)
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"PargeCfgForIntValueForKey:Memory Allocation Failure");
      MM_File_Release(pFileHandle);
      pFileHandle = NULL;
      return -1;
   }

   MM_File_Read(pFileHandle, buffer, length, &bytesRead);

   if(length == (unsigned long) bytesRead)
   {
      string tagValue;
      string xmlbuffer(buffer);
      tagValue = getValueForTag(buffer, pKey);

      if(!tagValue.length())
      {
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"PargeCfgForIntValueForKey:Key not present");
         MM_Free(buffer);
         buffer = NULL;
         MM_File_Release(pFileHandle);
         pFileHandle = NULL;
         return -1;
      }

      *pVal =  atoi(tagValue.c_str());
      MM_Free(buffer);
      buffer = NULL;
      MM_File_Release(pFileHandle);
      pFileHandle = NULL;
      return 0;
   }
   else
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"PargeCfgForIntValueForKey:File read failed");
      MM_Free(buffer);
      buffer = NULL;
      MM_File_Release(pFileHandle);
      pFileHandle = NULL;
      return -1;
   }
   if(!MM_File_Release(pFileHandle))
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"PargeCfgForIntValueForKey:File release failed");
   }
   MM_Free(buffer);
   buffer = NULL;
   return -1;

}

/*=========================================================

   FUNCTION:    updateVar

   DESCRIPTION:
   @brief     Helper method to update variables holding config values

*//**
@par     DEPENDENCIES: None

*//*
   PARAMETERS:
*//**
               @param[in] key - a key for adding to cfgItemsStr map

               @param[in] cfgKey - index of config item in cfgItems vector
                        should be as per cfgKeys enum in wfd_cfg_parser.h

               @param[in] keyValue - string buffer containing value for the
               config item which has a value of key

*//*     RETURN VALUE:
*//**       @return none

*//*=====================================================*/

void updateVar(const char* key, const cfgKeys cfgKey, const string keyValue)
{
  if(keyValue.length() && key)
  {
    pair<map<string,pair_val>::iterator,bool> ret;
    cfgItems.at(cfgKey)= atoi(keyValue.c_str());
    ret = cfgItemsStr.insert(pair<string,pair_val>(make_pair(string(key),make_pair(cfgKey,cfgItems.at(cfgKey)))));
    if(ret.second == false)
    {
        MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"updateVar:: Element %s already exists with %d",(ret.first->first).c_str(),ret.first->second.second);
    }
    else
    {
        MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_LOW,"updateVar:: Element %s newly inserted with %d",(ret.first->first).c_str(),ret.first->second.second);
    }
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"updateVar:: Invalid parameters");
  }
}

/*=========================================================

   FUNCTION:    getCfgItem

   DESCRIPTION:
   @brief     Used to get the value of a configurable
                item from config file

*//**
@par     DEPENDENCIES: None

*//*
   PARAMETERS:
*//**       @param[in] key - a string key for querying
            @param[out] pVal returns the value of the config
                                  item in this variable

*//*     RETURN VALUE:
*//**       @return returns 0 on success else -1


*//*=====================================================*/

int getCfgItem(const char* key,int *pVal)
{
    if(key && pVal)
    {
        if(cfgItems.size() && cfgItemsStr.size())
        {
            if(cfgItemsStr.count(string(key))> 0 )
            {//Key is present in map
                *pVal = cfgItemsStr[(string(key))].second;
                MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"getCfgItem:: Value of %s is %d",key,*pVal);
                return 0;
            }
            else
            {
                MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"getCfgItem:: %s not in config file",key);
            }
        }
        else
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"getCfgItem:: Failed to get Config item!");
        }
    }
    else
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"getCfgItem:: Invalid param to getConfigItem!!!");
    }
    return -1;
}

/*=========================================================

   FUNCTION:    resetCfgItems

   DESCRIPTION:
   @brief     Used to reset the values of configurable
                 item of config file

*//**
@par     DEPENDENCIES: None

*//*
   PARAMETERS:
*//**       @param none

*//*     RETURN VALUE:
*//**       @return none


*//*=====================================================*/

void resetCfgItems()
{
    cfgItems.clear();
    cfgItemsStr.clear();
    wfdDevice = SOURCE;
    MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_LOW,"Resetting configurable items cfgItems to %d and cfgItemsStr to %d",cfgItems.size(),cfgItemsStr.size());
}

/*==============================================================================

   FUNCTION:    parseCfgforUIBC

   DESCRIPTION:
   @brief      Parse XML File presence, size and call the actual parsing routine
               for UIBC capabilities from XML
*//**

@par     DEPENDENCIES:
                     None
*//*
   PARAMETERS:
*//**       @param  - filename - xfg file name
                    - readCfgCaps - capabilities structure

*//*     RETURN VALUE:
*//**       @return
                     None

@par     SIDE EFFECTS:
                     None

*//*==========================================================================*/
void  parseCfgforUIBC(const char *filename, readConfigFile *readCfgCaps)
{
   ssize_t bytesRead = 0;
   unsigned long length;
   char *buffer;
   MM_HANDLE pFileHandle;

   if(readCfgCaps)
   {
      pReadCfgCapabilities = readCfgCaps;
   }
   else
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parseCfgforUIBC:readCfgCaps is NULL ");
      return;

   }
   if(!MM_File_Create(filename, MM_FILE_CREATE_R, &pFileHandle))
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW,"parseCfgforUIBC:open config file successful");
   }
   else
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parseCfgforUIBC:open config file failed");
      pFileHandle = NULL;
      return;
   }

   MM_File_Seek(pFileHandle, 0, MM_FILE_SEEK_END);
   MM_File_GetSize(pFileHandle, &length);
   MM_File_Seek(pFileHandle, 0, MM_FILE_SEEK_BEG);

   buffer = (char *) MM_Malloc(static_cast<int>(sizeof(char)*length));
   if(!buffer)
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parseCfgforUIBC:Memory Allocation Failure");
      MM_File_Release(pFileHandle);
      pFileHandle = NULL;
      return;
   }

   MM_File_Read(pFileHandle, buffer, length, &bytesRead);

   if(length == (unsigned long) bytesRead)
   {
      string tagValue;
      string xmlbuffer(buffer);
      tagValue = getValueForTag(buffer, "UIBC");
      parseUIBCSupport(tagValue);
   }
   else
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parseCfgforUIBC:File read failed");
      MM_File_Release(pFileHandle);
      pFileHandle = NULL;
      MM_Free(buffer);
      return;
   }
   if(MM_File_Release(pFileHandle))
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"parseCfgforUIBC:File release failed");
   }
   MM_Free(buffer);
   return;
}

/*==============================================================================

   FUNCTION:    dumpcapabilitiesRead

   DESCRIPTION:
   @brief      Dump the capabilities read from the xml cfg file
*//**

@par     DEPENDENCIES:
                     None
*//*
   PARAMETERS:
                     None
*//*     RETURN VALUE:
*//**       @return
                     None

@par     SIDE EFFECTS:
                     None

*//*==========================================================================*/

void  dumpcapabilitiesRead()
{

   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "************************************************");
   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, " Dumping Capabilities Read from XML Config File");
   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "************************************************");
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Audio Method  = %d",  pReadCfgCapabilities->pCfgCapabilities->audio_method);
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "   AAC Bitmap = %u",  pReadCfgCapabilities->pCfgCapabilities->audio_config.aac_codec.supported_modes_bitmap);
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "   AC3 Bitmap = %d",  pReadCfgCapabilities->pCfgCapabilities->audio_config.dolby_digital_codec.supported_modes_bitmap);
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "  LPCM Bitmap = %d",  pReadCfgCapabilities->pCfgCapabilities->audio_config.lpcm_codec.supported_modes_bitmap);
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " Video_method = %d",  pReadCfgCapabilities->pCfgCapabilities->video_method);
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " Num H264 Profiles = %d", pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.num_h264_profiles);

   for(int index=0; index < pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.num_h264_profiles; index++)
   {
     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"Video codec properties for profile %d : ",index);
     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " H264 Level = %d", ( pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).h264_level);
     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " H264 Profile = %d", ( pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).h264_profile);
     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " CEA Bitmap = %d", ( pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).supported_cea_mode);
     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " VESA Bitmap = %d", ( pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).supported_vesa_mode);
     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "  HH Bitmap = %d", ( pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).supported_hh_mode);
     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " Max HRES = %d", ( pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).max_hres);
     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " Max VRES = %d", ( pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).max_vres);
     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " Frame Rate Control Support = %d", ( pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.h264_codec[index]).frame_rate_control_support);
   }

   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " Stand By Resume = %d",  pReadCfgCapabilities->pCfgCapabilities->standby_resume_support);
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, " Preferred Display Mode Supported = %d", pReadCfgCapabilities->pCfgCapabilities->video_config.video_config.preferred_display_mode_supported);

   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "IDR Request Valid = %d",pReadCfgCapabilities->idrRequestCapability.idrReqValid);
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "IDR Request Intvl = %d",pReadCfgCapabilities->idrRequestCapability.idrIntvl);
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Frame Skip Valid  = %d",pReadCfgCapabilities->frameSkippingCapability.frameSkipValid);
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Frame Skip Intvl  = %d",pReadCfgCapabilities->frameSkippingCapability.frameSkipIntvl);

   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Content Protection Version = %d",pReadCfgCapabilities->pCfgCapabilities->content_protection_config.content_protection_capability);
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Content Protection Port = %d",pReadCfgCapabilities->pCfgCapabilities->content_protection_config.content_protection_ake_port);

   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTP Port Type (0-UDP,1-TCP) = %d",pReadCfgCapabilities->pCfgCapabilities->transport_capability_config.eRtpPortType);

   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTP Dump Enable = %d",bRTPDumpEnable);

   MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH, "UIBC Generic = %d, Generic Input = %d, Port ID: = %d",pReadCfgCapabilities->uibcCapability.config.category,
                                                                                                  pReadCfgCapabilities->uibcCapability.config.generic_input_type,
                                                                                                  pReadCfgCapabilities->uibcCapability.port_id);
}

/*==============================================================================

   FUNCTION:    getSupportedHDCPVersion

   DESCRIPTION:
   @brief      gets the supported version from the version string
*//**

@par     DEPENDENCIES:
                     None
*//*
   PARAMETERS:
                     string
*//*     RETURN VALUE:
*//**       @return
                     int

@par     SIDE EFFECTS:
                     none

*//*==========================================================================*/

int getSupportedHDCPVersion(string version)
{
    int nMinorVersion = WFD_HDCP_VERSION_2_1;  // fall back to version 2.1

    nMinorVersion = strstr(version.c_str(), "WFD_HDCP_2_") ?
                      (atoi((char *)&version.c_str()[strlen("WFD_HDCP_2_")]) + 1) :
                      WFD_HDCP_VERSION_2_1;

   if ( ((WFD_HDCP_version_t)nMinorVersion >= WFD_HDCP_VERSION_2_0) &&
        ((WFD_HDCP_version_t)nMinorVersion <= WFD_HDCP_VERSION_MAX))
   {

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "wfd_cfg_parser->getSupportedHDCPVersion() WFD_HDCP_2_%d",
                     (nMinorVersion - 1));
        return nMinorVersion;
   }
   nMinorVersion = WFD_HDCP_VERSION_2_1; // fall back to version 2.1
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "wfd_cfg_parser->getSupportedHDCPVersion() fall back to WFD_HDCP_2_%d",
                (nMinorVersion - 1));
   return nMinorVersion;
}

#ifdef __cplusplus
}
#endif


