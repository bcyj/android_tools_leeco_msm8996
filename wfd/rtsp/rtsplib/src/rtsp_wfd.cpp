/***************************************************************************
 *                             rtsp_wfd.cpp
 * DESCRIPTION
 *  RTSP WFD definitions for RTSP_LIB module
 *
 * Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc. All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/src/rtsp_wfd.cpp#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_wfd.h"
#include "rtsp_session.h"

using namespace rtsp_wfd;

/*
 * Convert string to 32 bit bitset
 */
bitset<BITSET_8> stringToBitSize8(string value)
{
    int num = 0;
    RTSPStringStream ss;

    ss << hex << value;
    ss >> num;

    bitset<BITSET_8> bSet(num);

    return bSet;
}

/*
 * Convert string to 8 bit bitset
 */
bitset<BITSET_2> stringToBitSize2(string value)
{
    int num = 0;
    RTSPStringStream ss;

    ss << hex << value;
    ss >> num;
    bitset<BITSET_2> bSet(num);

    return bSet;
}

unsigned stringToNumHex(string value)
{
    unsigned num = 0;
    RTSPStringStream ss;

    ss << value;
    ss >> hex >> num;

    return num;
}

/*
 * Convert string to unsigned
 */
unsigned stringToNum(string value)
{
    unsigned num = 0;
    RTSPStringStream ss;

    ss << value;
    ss >> dec >> num;

    return num;
}

/*
 * Overloaded audio codec output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, audioCodec &audio)
{
    if (audio.valid) {
        stream << audio.name << " ";
        stream << setfill('0') << setw(8) << hex << audio.modes.to_ulong();
        stream << " " << setfill('0') << setw(2) << hex << audio.latency;
        stream << dec;
    }

    return stream;
}

/*
 * Overloaded audio codec input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, audioCodec &audio)
{
    string modeS, latS;

    stream >> audio.name;
    stream >> modeS;
    audio.modes = stringToBitSize8(modeS);
    stream >> latS;
    audio.latency = static_cast<unsigned short>(stringToNumHex(latS));

    audio.valid = true;

    return stream;
}

/*
 * Overloaded audio codec & operator
 */
audioCodec audioCodec::operator&(const audioCodec &theirs)
{
    bitset<BITSET_8> intersectModes;
    unsigned short intersectLatency = 0;
    audioCodec audio;

    if (valid && theirs.valid) {
        intersectModes = (modes & theirs.modes);
        intersectLatency = max(latency, theirs.latency);
        return audioCodec(name, intersectModes, intersectLatency);
    }

    return audio;
}

// LPCM
void audioCodec::getAudioMode(rtsp_wfd::lpcmModes mode, struct wfdAudioMode *pAudioMode)
{
    static const struct wfdAudioMode wfdAudioModeLPCM[] = {
        { 44100, 16, 2},  // lpcm_441_16_2
        { 48000, 16, 2},  // lpcm_48_16_2
    };

    assert(mode < (sizeof(wfdAudioModeLPCM) / sizeof(struct wfdAudioMode)));

    *pAudioMode = wfdAudioModeLPCM[mode];
}

// AAC
void audioCodec::getAudioMode(rtsp_wfd::aacModes mode, struct wfdAudioMode *pAudioMode)
{
    static const struct wfdAudioMode wfdAudioModeAAC[] = {
        { 48000, 16, 2},  // aac_48_16_2
        { 48000, 16, 4},  // aac_48_16_4
        { 48000, 16, 6},  // aac_48_16_6
        { 48000, 16, 8},  // aac_48_16_8
    };

    assert(mode < (sizeof(wfdAudioModeAAC) / sizeof(struct wfdAudioMode)));

    *pAudioMode = wfdAudioModeAAC[mode];
}

// Eac
void audioCodec::getAudioMode(rtsp_wfd::eacModes mode, struct wfdAudioMode *pAudioMode)
{
    static const struct wfdAudioMode wfdAudioModeEac[] = {
        { 48000, 16, 2},  // eac_48_16_2
        { 48000, 16, 4},  // eac_48_16_4
        { 48000, 16, 6},  // eac_48_16_6
    };

    assert(mode < (sizeof(wfdAudioModeEac) / sizeof(struct wfdAudioMode)));

    *pAudioMode = wfdAudioModeEac[mode];
}

// AC3
void audioCodec::getAudioMode(rtsp_wfd::ac3Modes mode, struct wfdAudioMode *pAudioMode)
{
    static const struct wfdAudioMode wfdAudioModeAC3[] = {
        { 48000, 16, 2},  // eac_48_16_2_ac3
        { 48000, 16, 4},  // eac_48_16_4_ac3
        { 48000, 16, 6},  // eac_48_16_6_ac3
    };

    assert(mode < (sizeof(wfdAudioModeAC3) / sizeof(struct wfdAudioMode)));

    *pAudioMode = wfdAudioModeAC3[mode];
}

/*
 * Overloaded video header output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, videoCodec &video)
{
    if (video.valid) {
        stream << setfill('0') << setw(2) << hex << video.native.to_ulong() << " ";
        stream << setfill('0') << setw(2) << hex << video.prefDispSupp.to_ulong();
        stream << dec;
    }

    return stream;
}

/*
 * Overloaded video header input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, videoCodec &video)
{
    string nativeS, prefDispS;

    stream >> nativeS;
    stream >> prefDispS;

    video.native = stringToBitSize2(nativeS);
    video.prefDispSupp = stringToBitSize2(prefDispS);

    video.valid = true;

    return stream;
}

// Display mode - Resolution/Frame rate information
struct wfdDisplayModeS {
    unsigned short int width;
    unsigned short int height;
};

// CEA
void videoCodec::getDisplayMode(ceaResolution mode, struct wfdDisplayMode *pDisplayMode)
{
    static const struct wfdDisplayMode wfdDisplayModeCea[] = {
        { 640, 480,     60, true    },  // cea_640x480p60
        { 720, 480,     60, true    },  // cea_720x480p60
        { 720, 480,     60, false   },  // cea_720x480i60
        { 720, 576,     50, true    },  // cea_720x576p50
        { 720, 576,     50, false   },  // cea_720x576i50
        { 1280, 720,    30, true    },  // cea_1280x720p30
        { 1280, 720,    60, true    },  // cea_1280x720p60
        { 1920, 1080,   30, true    },  // cea_1920x1080p30
        { 1920, 1080,   60, true    },  // cea_1920x1080p60
        { 1920, 1080,   60, false   },  // cea_1920x1080i60
        { 1280, 720,    25, true    },  // cea_1280x720p25
        { 1280, 720,    50, true    },  // cea_1280x720p50
        { 1920, 1080,   25, true    },  // cea_1920x1080p25
        { 1920, 1080,   50, true    },  // cea_1920x1080p50
        { 1920, 1080,   50, false   },  // cea_1920x1080i50
        { 1280, 720,    24, true    },  // cea_1280x720p24
        { 1920, 1080,   24, true    },  // cea_1920x1080p24
    };

    assert(mode < (sizeof(wfdDisplayModeCea) / sizeof(struct wfdDisplayMode)));

    *pDisplayMode = wfdDisplayModeCea[mode];
}

// VESA
void videoCodec::getDisplayMode(enum vesaResolution mode, struct wfdDisplayMode *pDisplayMode)
{
    static const struct wfdDisplayModeS wfdDisplayModeVesa[] = {
        { 800,  600 },  // vesa_800x600p30
        { 800,  600 },  // vesa_800x600p60
        { 1024, 768 },  // vesa_1024x768p30
        { 1024, 768 },  // vesa_1024x768p60
        { 1152, 864 },  // vesa_1152x864p30
        { 1152, 864 },  // vesa_1152x864p60
        { 1280, 768 },  // vesa_1280x768p30
        { 1280, 768 },  // vesa_1280x768p60
        { 1280, 800 },  // vesa_1280x800p30
        { 1280, 800 },  // vesa_1280x800p60
        { 1360, 768 },  // vesa_1360x768p30
        { 1360, 768 },  // vesa_1360x768p60
        { 1366, 768 },  // vesa_1366x768p30
        { 1366, 768 },  // vesa_1366x768p60
        { 1280, 1024 },  // vesa_1280x1024p30
        { 1280, 1024 },  // vesa_1280x1024p60
        { 1400, 1050 },  // vesa_1400x1050p30
        { 1400, 1050 },  // vesa_1400x1050p60
        { 1440, 900  },  // vesa_1440x900p30
        { 1440, 900  },  // vesa_1440x900p60
        { 1600, 900  },  // vesa_1600x900p30
        { 1600, 900  },  // vesa_1600x900p60
        { 1600, 1200 },  // vesa_1600x1200p30
        { 1600, 1200 },  // vesa_1600x1200p60
        { 1680, 1024 },  // vesa_1680x1024p30
        { 1680, 1024 },  // vesa_1680x1024p60
        { 1680, 1050 },  // vesa_1680x1050p30
        { 1680, 1050 },  // vesa_1680x1050p60
        { 1920, 1200 },  // vesa_1920x1200p30
        { 1920, 1200 },  // vesa_1920x1200p60
    };

    assert(mode < (sizeof(wfdDisplayModeVesa) / sizeof(struct wfdDisplayModeS)));

    const struct wfdDisplayModeS *pVesa = &wfdDisplayModeVesa[mode];
    pDisplayMode->width = pVesa->width;
    pDisplayMode->height = pVesa->height;
    pDisplayMode->frameRate = (mode & 1) ? 60 : 30;
    pDisplayMode->progressive = true;
}

// Hand held (HH)
void videoCodec::getDisplayMode(enum hhResolution mode, struct wfdDisplayMode *pDisplayMode)
{
    static const struct wfdDisplayModeS wfdDisplayModeHh[] = {
        { 800, 480 },   // hh_800x480p30
        { 800, 480 },   // hh_800x480p60
        { 854, 480 },   // hh_854x480p30
        { 854, 480 },   // hh_854x480p60
        { 864, 480 },   // hh_864x480p30
        { 864, 480 },   // hh_864x480p60
        { 640, 360 },   // hh_640x360p30
        { 640, 360 },   // hh_640x360p60
    };

    assert(mode < (sizeof(wfdDisplayModeHh) / sizeof(struct wfdDisplayModeS)));

    const struct wfdDisplayModeS *pHH = &wfdDisplayModeHh[mode];
    pDisplayMode->width = pHH->width;
    pDisplayMode->height = pHH->height;
    pDisplayMode->frameRate = (mode & 1) ? 60 : 30;
    pDisplayMode->progressive = true;
}

/*
 * Overloaded video header & operator
 */
videoCodec videoCodec::operator&(const videoCodec &theirs)
{
    bitset<BITSET_2> intersectNative;
    bitset<BITSET_2> intersectLevel;
    videoCodec video;

    if (valid && theirs.valid) {
        intersectNative = (native & theirs.native);
        intersectLevel = (prefDispSupp & theirs.prefDispSupp);
        return videoCodec(intersectNative, intersectLevel);
    }

    return video;
}

/*
 * Overloaded video codec output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, h264Codec &video)
{
    if (video.valid) {
        stream << setfill('0') << setw(2) << hex << video.profile.to_ulong() << " ";
        stream << setfill('0') << setw(2) << hex << video.level.to_ulong() << " ";
        stream << setfill('0') << setw(8) << hex << video.ceaSupp.to_ulong() << " ";
        stream << setfill('0') << setw(8) << hex << video.vesaSupp.to_ulong() << " ";
        stream << setfill('0') << setw(8) << hex << video.hhSupp.to_ulong() << " ";
        stream << setfill('0') << setw(2) << hex << video.latency << " ";
        stream << setfill('0') << setw(4) << hex << video.minSliceSize << " ";
        stream << setfill('0') << setw(4) << hex << video.sliceEnc.to_ulong() << " ";
        stream << setfill('0') << setw(2) << hex << video.frameRateCtlSupp.to_ulong() << " ";
        if (video.maxHres) {
            stream << setfill('0') << setw(4) << hex << video.maxHres << " ";
        } else {
            stream << "none ";
        }
        if (video.maxVres) {
            stream << setfill('0') << setw(4) << hex << video.maxVres;
        } else {
            stream << none;
		}

        stream << dec;
    }

    return stream;
}

/*
 * Overloaded video codec input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, h264Codec &video)
{
    string profileS, levelS, hResS, vResS, ceaS, vesaS, hhS,
           latencyS, minS, sliceEncS, frameS;

    stream >> profileS;
    stream >> levelS;
    stream >> ceaS;
    stream >> vesaS;
    stream >> hhS;
    stream >> latencyS;
    stream >> minS;
    stream >> sliceEncS;
    stream >> frameS;
    stream >> hResS;
    stream >> vResS;

    video.profile = stringToBitSize2(profileS);
    video.level = stringToBitSize2(levelS);
    if (hResS != none) {
        video.maxHres = static_cast<unsigned short>(stringToNumHex(hResS));
    } else {
        video.maxHres = 0;
    }
    if (vResS != none) {
        video.maxVres = static_cast<unsigned short>(stringToNumHex(vResS));
    } else {
        video.maxVres = 0;
    }

    video.ceaSupp = stringToBitSize8(ceaS);
    video.vesaSupp = stringToBitSize8(vesaS);
    video.hhSupp = stringToBitSize8(hhS);
    video.latency = static_cast<unsigned short>(stringToNumHex(latencyS));
    video.minSliceSize = static_cast<unsigned short>(stringToNumHex(minS));
    video.sliceEnc = stringToBitSize2(sliceEncS);
    video.frameRateCtlSupp = stringToBitSize2(frameS);

    video.valid = true;

    switch(video.profile.to_ulong()) {
    case cbp:
        video.name = "CBP";
    break;
    case chp:
        video.name = "CHP";
    break;
    case chi444p:
        video.name = "CHI444P";
    break;
    default:
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: Did not find video codec");
    break;
    }

    return stream;
}

/*
 * Overloaded video codec & operator
 */
h264Codec h264Codec::operator&(const h264Codec &theirs)
{
    bitset<BITSET_2> intersectProfile;
    bitset<BITSET_2> intersectLevel;
    bitset<BITSET_8> intersectCeaSupp;
    bitset<BITSET_8> intersectVesaSupp;
    bitset<BITSET_8> intersectHhSupp;
    unsigned short intersectLatency;
    unsigned short intersectMinSliceSize;
    unsigned short intersectHres, intersectVres;
    bitset<BITSET_2> intersectFrameRate;
    bitset<BITSET_2> intersectSliceEnc;
    h264Codec codec;

    if (valid & theirs.valid) {
        intersectProfile = (profile & theirs.profile);
        intersectLevel = (level.to_ulong() < theirs.level.to_ulong()) ?
                        level : theirs.level;
        intersectHres = theirs.maxHres;
        intersectVres = theirs.maxVres;
        intersectCeaSupp = (ceaSupp & theirs.ceaSupp);
        intersectVesaSupp = (vesaSupp & theirs.vesaSupp);
        intersectHhSupp = (hhSupp & theirs.hhSupp);
        intersectLatency = max(latency, theirs.latency);
        intersectMinSliceSize = max(minSliceSize, theirs.minSliceSize);
        intersectSliceEnc = (sliceEnc & theirs.sliceEnc);
        intersectFrameRate = (frameRateCtlSupp & theirs.frameRateCtlSupp);

        codec = h264Codec(intersectProfile, intersectLevel, intersectHres,
                          intersectVres, intersectCeaSupp, intersectVesaSupp,
                          intersectHhSupp, intersectLatency,
                          intersectMinSliceSize, intersectSliceEnc,
                          intersectFrameRate);
    }

    return codec;
}

/*
 * Get parsed IP address from presentation URL
 */
string getParsedIp(string uri)
{
    string ip;
    size_t pos;

    if (uri != none) {
        uri.erase(0, strlen(url));

        if ((pos = uri.find('/')) != std::string::npos) {
            uri.erase(pos, uri.length());
            ip = uri;
        }
    }

    return ip;
}

/*
 * Overloaded presentation URL input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, presentationUrl &presentation)
{
    string url0, url1;
    string ip0, ip1;

    stream >> url0;
    stream >> url1;

    ip0 = getParsedIp(url0);
    ip1 = getParsedIp(url1);

    presentation.ipAddr0 = ip0;
    presentation.ipAddr1 = ip1;

    presentation.valid = true;

    return stream;
}

/*
 * Overloaded presentation URL output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, presentationUrl &presentation)
{
    if (presentation.valid) {
        stream << presentation.getUri0();
        stream << " ";
        stream << presentation.getUri1();
    }

    return stream;
}

/*
 * Overloaded client rtp ports input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, clientPorts &client)
{
    string port0, port1;

    stream >> client.profile;

    if(strstr(client.profile.c_str(), "TCP"))
    {
        client.setTCP(true);
    }

    stream >> port0;
    client.rtpPort0 = stringToNum(port0);

    stream >> port1;
    client.rtpPort1 = stringToNum(port1);

    client.valid = true;

    return stream;
}

/*
 * Overloaded client rtp ports output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, clientPorts &client)
{
    if (client.valid) {
        if(!client.getTCP()) {
            stream << profile << " " << client.rtpPort0;
        }
        else
        {
            stream << profiletcp << " " << client.rtpPort0;
        }
        stream << " " << client.rtpPort1 << " " << "mode=play";
    }

    return stream;
}

/*
 * Overloaded trigger input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, triggerParam &trig)
{
    stream >> trig.command;

    trig.valid = true;

    return stream;
}

/*
 * Overloaded trigger output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, triggerParam &trig)
{
    if (trig.valid)
        stream << trig.command;

    return stream;
}

/*
 * Overloaded wfd content protection input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, hdcp_cp &cp_spec)
{
  string params;
  stream >> params;
  int port = 0;
  char *pTempPtr = NULL;
  if (!strncasecmp(params.c_str(), none, strlen(none)))
  {
    cp_spec.valid = false;
  }

  if (!strncasecmp(params.c_str(), hdcp_spec_2_0, strlen(hdcp_spec_2_0) - 1))
  {
    int hdcp_version = atoi((char *)&params.c_str()[strlen(hdcp_spec_2_0) - 1]);
    cp_spec.valid = true;
    cp_spec.version = hdcp_version;
  }

  if (cp_spec.valid == true)
  {
    stream >> params;
    if (!strncasecmp(params.c_str(), tcp_port, strlen(tcp_port)))
    {
      params.erase(0, strlen(tcp_port));
      char *CtrlPort = (strtok_r((char *)params.c_str(), "=",&pTempPtr));
      if(CtrlPort!= NULL)
      cp_spec.setCtrlPort((atoi(CtrlPort)));
    }
  }
  return stream;
}

/*
 * Overloaded wfd content protection output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, hdcp_cp &hdpc_set)
{
  if ( hdpc_set.valid )
  {
    if (hdpc_set.version >= 0)
    {
       stream << "HDCP2.";
       stream << hdpc_set.version;
    }
    stream << " ";
    stream << tcp_port << "=";
    if (hdpc_set.ctrl_port)
    {
      stream << hdpc_set.ctrl_port;
    }
  }
  else
  {
    stream << " " << none;
  }

  return stream;
}

/*
 * Overloaded UIBC capability input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, uibcCapability &cap)
{
    string input, params;
    char *pTempPtr = NULL;

    stream >> input;

    if (input == uibc_input_category) {
        while (stream >> params) {
            replace(params.begin(), params.end(), ',', ' ');
            replace(params.begin(), params.end(), '=', ' ');
            if (!strncasecmp(params.c_str(), none, strlen(none))) {
                cap.valid = false;
                break;
            } else if (!strncasecmp(params.c_str(), uibc_generic, strlen(uibc_generic))) {
                cap.setCategory(general);
            } else if (!strncasecmp(params.c_str(), uibc_hidc, strlen(uibc_hidc))) {
                cap.setCategory(hidc);
            }
            cap.valid = true;
        }
    } else if (input == uibc_generic_capability) {
        while (stream >> params) {
            replace(params.begin(), params.end(), ',', ' ');
            replace(params.begin(), params.end(), '=', ' ');
            if (params == none) { break; }

            for (int i = 0; i < numSupportedInputs; i++) {
                if (!strncasecmp(params.c_str(), inputTable[i].name.c_str(), inputTable[i].name.length())) {
                    cap.setGenericCap(inputTable[i].type);
                }
            }
        }
    } else if (input == uibc_hidc_capability) {
        while (stream >> params) {
            replace(params.begin(), params.end(), ',', ' ');
            replace(params.begin(), params.end(), '=', ' ');
            if (params == none) { break; }

            if (char *in = (strtok_r((char *)params.c_str(), "/", &pTempPtr))) {
                char *path = strtok_r(NULL, "/", &pTempPtr);
                for (int i = 0; i < numSupportedInputs; i++) {
                    if (in != NULL && !strncasecmp(in, inputTable[i].name.c_str(), inputTable[i].name.length())) {
                        for (int j = 0; j < numSupportedPaths; j++) {
                            if (path != NULL && !strncasecmp(path, pathTable[j].name.c_str(), pathTable[j].name.length())) {
                                cap.setHidcMap(inputTable[i].type, pathTable[j].type);
                            }
                        }
                    }
                }
            }
        }
    } else if (input == tcp_port) {
        while (stream >> params) {
            replace(params.begin(), params.end(), '=', ' ');
            if (params == "none") { break; }
            cap.setPort(atoi(params.c_str()));
        }
    }
    return stream;
}

/*
 * Overloaded UIBC capability output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, uibcCapability &cap)
{
    bool prev = false, found = false;

    if (cap.valid) {
        stream << uibc_input_category << "=";
        if (cap.category[hidc]) {
            stream << uibc_hidc;
            prev = true;
            found = true;
        }
        if (cap.category[general]) {
            if (prev) { stream << ", "; }
            stream << uibc_generic;
            found = true;
        }
        if (false == found) {
            stream << none;
        }
        stream << ";";

        prev = false;
        found = false;

        stream << uibc_generic_capability << "=";
        for (int i = 0; i < numSupportedInputs; i++) {
            if (cap.genericCaps[inputTable[i].type]) {
                if (prev) { stream << ", "; }
                stream << inputTable[i].name;
                prev = true;
                found = true;
            }
        }
        if (false == found) {
            stream << none;
        }
        stream << ";";

        prev = false;
        found = false;

        stream << uibc_hidc_capability << "=";
        for (int i = 0; i < numSupportedInputs; i++) {
            if ((cap.getHidcCap()).any()) {
                if (prev) { stream << ", "; }
                stream << inputTable[i].name << "/";
                stream << pathTable[cap.getHidcMap(inputTable[i].type)].name;
                prev = true;
                found = true;
            }
        }
        if (false == found) {
            stream << none;
        }
        stream << ";";

        stream << tcp_port << "=";
        if (cap.port) {
            stream << cap.port;
        } else {
            stream << none;
        }
    } else {
            stream << none;
    }

    return stream;
}

/*
 * Overloaded UIBC capability & operator
 */
uibcCapability uibcCapability::operator&(const uibcCapability &theirs)
{
    uibcCapability caps;

    if (valid && theirs.valid) {
        caps.category = (category & theirs.category);
        if (caps.category.any()) {
            caps.genericCaps = theirs.genericCaps;
            caps.hidcCaps = theirs.hidcCaps;
            memcpy(caps.hidcMap, theirs.hidcMap, sizeof(caps.hidcMap));
            caps.port = port;
            caps.valid = true;
        }
    }

    return caps;
}

uibcCapability& uibcCapability::operator=(const uibcCapability &theirs)
{
    if(this != &theirs) {
       category = theirs.category;
       genericCaps = theirs.genericCaps;
       hidcCaps = theirs.hidcCaps;
       for(int i= 0;i < UIBC_MAX_INPUT_SIZE; i++) {
           hidcMap[i] = theirs.hidcMap[i];
       }
       port = theirs.port;
       valid = theirs.valid;
    }
    return *this;
}

/*
 * Overloaded UIBC setting input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, uibcSetting &set)
{
    string setting;

    stream >> setting;

    if (setting == disable)
        set.setting = false;
    else
        set.setting = true;

    set.valid = true;

    return stream;
}

/*
 * Overloaded UIBC setting output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, uibcSetting &set)
{
    if (set.valid) {
        if (set.setting)
            stream << enable;
        else
            stream << disable;
    }

    return stream;
}

/*
 * Overloaded UIBC setting & operator
 */
uibcSetting uibcSetting::operator&(const uibcSetting &theirs)
{
    uibcSetting set;

    if (valid && theirs.valid) {
        set.setting = setting && theirs.setting;
        set.valid = true;
    }

    return set;
}

/*
 * Overloaded standby_resume_capability input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, standby_resume_cap &cap )
{
    string setting;
    stream >> setting;

    if ( setting == none )
      cap.setting = false;
    else
    {
      cap.setting = true;
      cap.valid = true;
    }


    return stream;
}

/*
 * Overloaded standby_resume_capability output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, standby_resume_cap &cap )
{
    if(cap.valid) {
       if (cap.setting )
           stream << supported;
       else
           stream << none;
    }

    return stream;
}

/*
 * Overloaded standby_resume_capability & operator
 */
standby_resume_cap standby_resume_cap::operator&(const standby_resume_cap &theirs)
{
    standby_resume_cap cap;

    if (valid && theirs.valid ){
        cap.setting = setting && theirs.setting;
        cap.valid = true;
    }
    return cap;
}

/*
 * Overloaded standby input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, standby &stop)
{
    stop.valid = true;

    return stream;
}

/*
 * Overloaded standby output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, standby &stop)
{
    UNUSED(stop);
    return stream;
}

/*
 * Overloaded resume input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, resume &start)
{
    start.valid = true;

    return stream;
}

/*
 * Overloaded resume output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, resume &start)
{
    UNUSED(start);
    return stream;
}

/*
 * Overloaded coupled input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, coupled &coup)
{
    string mac, input;

    stream >> input;

    if (input != none) {
        bitset<BITSET_2> status(input);
        stream >> mac;

        coup.status = status;

        if (mac != none)
            coup.macAddr = mac;

        coup.valid = true;
    }

    return stream;
}

/*
 * Overloaded coupled output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, coupled &coup)
{
    if (coup.valid) {
        stream << setfill('0') << setw(2) << hex << coup.status.to_ulong() << " ";
        stream << dec;
        if (coup.status[COUPLED]) {
            stream << coup.macAddr;
        } else
            stream << none;
    } else {
        stream << none;
    }

    return stream;
}

/*
 * Overloaded route input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, route &rte)
{
    string input;

    stream >> input;

    if (input == primary)
        rte.destination = PRIMARY;
    else if (input == secondary)
        rte.destination = SECONDARY;

    rte.valid = true;

    return stream;
}

/*
 * Overloaded route output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, route &rte)
{
    if (rte.valid) {
        if (rte.destination == PRIMARY)
            stream << primary;
        else if (rte.destination == SECONDARY)
            stream << secondary;
        else
            assert(0);
    }

    return stream;
}

/*
 * Overloaded av timing change input stream operator
 */
RTSPStringStream &operator>>(RTSPStringStream &stream, timingChange &tmg)
{
    string pts, dts;

    stream >> tmg.pts;
    stream >> tmg.dts;

    tmg.valid = true;

    return stream;
}

/*
 * Overloaded av timing change output stream operator
 */
RTSPStringStream &operator<<(RTSPStringStream &stream, timingChange &tmg)
{
    if (tmg.valid) {
        stream << setfill('0') << setw(10) << hex << tmg.pts;
        stream << " ";
        stream << setfill('0') << setw(10) << hex << tmg.dts;
        stream << dec;
    }

    return stream;
}

RTSPStringStream &operator>>(RTSPStringStream &stream, displayEdid &edid)
{
    string input;
    unsigned count = 0;

    stream >> input;

    if (input == none)
        return stream;

    RTSPStringStream ss(input);
    ss >> count;

    if (count > EDID_MAX_BLOCKS) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Block count is greater than 128");
        count = EDID_MAX_BLOCKS;
    }

    if (count) {
        string data;
        stream >> data;
        edid.setPayload(count, (const unsigned char *)data.c_str());
        edid.valid = true;
    }

    return stream;
}

RTSPStringStream &operator<<(RTSPStringStream &stream, displayEdid &edid)
{
    if (edid.valid) {
        stream << setfill('0') << setw(4) << hex << edid.blockCount;
        stream << " ";
        if (edid.blockCount && edid.payload) {
            stream << edid.payload;
        } else {
            stream << none;
        }
        stream << dec;
    } else {
        stream << none;
    }

    return stream;
}

/*
 * Overloaded rtspWfd & operator
 */
rtspWfd& rtspWfd::operator&(const rtspWfd &rhs)
{
    audioLpcm = audioLpcm & rhs.audioLpcm;
    audioAac = audioAac & rhs.audioAac;
    audioEac = audioEac & rhs.audioEac;
    audioAc3 = audioAc3 & rhs.audioAc3;
    audioDts = audioDts & rhs.audioDts;
    //videoHeader = videoHeader & rhs.videoHeader;
    videoHeader = rhs.videoHeader;
    h264Cbp = h264Cbp & rhs.h264Cbp;
    h264Chp = h264Chp & rhs.h264Chp;
    h264Chi444p = h264Chi444p & rhs.h264Chi444p;
    client = rhs.client;
    uibcCap = uibcCap & rhs.uibcCap;
    uibcSet = rhs.uibcSet;
    standbyCap = standbyCap & rhs.standbyCap;
    halt = rhs.halt;
    start = rhs.start;
    coupledSink = rhs.coupledSink;
    audioRoute = rhs.audioRoute;
    timing = rhs.timing;
    edid = rhs.edid;
    contentProtection = rhs.contentProtection;
    if (rhs.tcpWindowSize.getValid() == false) {
        buffLen.setValid(false);
    }
    tcpWindowSize = rhs.tcpWindowSize;
    return *this;
}

/*
 * Used for assignment of rtspWfd
 * Selectively disables audio/video settings based on which
 * modes deliver the best quality.
 */
void rtspWfd::assign(const rtspWfd &theirs)
{
    if (theirs.audioAac.getValid()) {
        audioAac = theirs.audioAac;
        audioLpcm.setValid(false);
        audioAc3.setValid(false);
        audioEac.setValid(false);
        audioDts.setValid(false);
    } else if (theirs.audioEac.getValid()) {
        audioEac = theirs.audioEac;
        audioAac.setValid(false);
        audioLpcm.setValid(false);
        audioDts.setValid(false);
        audioAc3.setValid(false);
    } else if (theirs.audioAc3.getValid()) {
        audioAc3 = theirs.audioAc3;
        audioAac.setValid(false);
        audioLpcm.setValid(false);
        audioDts.setValid(false);
        audioEac.setValid(false);
    } else if (theirs.audioDts.getValid()) {
        audioDts = theirs.audioDts;
        audioAac.setValid(false);
        audioEac.setValid(false);
        audioLpcm.setValid(false);
        audioAc3.setValid(false);
    } else if (theirs.audioLpcm.getValid()) {
        audioLpcm = theirs.audioLpcm;
        audioAac.setValid(false);
        audioEac.setValid(false);
        audioDts.setValid(false);
        audioAc3.setValid(false);
    }

    if (theirs.videoHeader.getValid()) {
        videoHeader = theirs.videoHeader;
    }

    if (theirs.h264Chp.getValid()) {
        h264Chp = theirs.h264Chp;
        h264Cbp.setValid(false);
        h264Chi444p.setValid(false);
	} else if (theirs.h264Cbp.getValid()) {
        h264Cbp = theirs.h264Cbp;
        h264Chp.setValid(false);
        h264Chi444p.setValid(false);
    } else if (theirs.h264Chi444p.getValid()) {
        h264Chi444p = theirs.h264Chi444p;
        h264Chp.setValid(false);
        h264Cbp.setValid(false);
    }
    if (theirs.client.getValid())
        client = theirs.client;
    else
        client.setRenegotiated(false);

    if (theirs.uri.getValid())
        uri = theirs.uri;
    if (theirs.uibcCap.getValid())
        uibcCap = theirs.uibcCap;
    if (theirs.uibcSet.getValid())
        uibcSet = theirs.uibcSet;
    if (theirs.standbyCap.getValid())
        standbyCap = theirs.standbyCap;
    if (theirs.halt.getValid()) {
        halt = theirs.halt;
        start.setValid(false);
    } else if (theirs.start.getValid()) {
        start = theirs.start;
        halt.setValid(false);
    }
    if (theirs.coupledSink.getValid()) {
        coupledSink = theirs.coupledSink;
    }
    if (theirs.audioRoute.getValid()) {
        audioRoute = theirs.audioRoute;
    }
    if (theirs.timing.getValid()) {
        timing = theirs.timing;
    }
    if (theirs.edid.getValid()) {
        edid = theirs.edid;
    }
   if ( theirs.contentProtection.getValid()){
        contentProtection = theirs.contentProtection;
    }
    if (theirs.idrReq.getValid()) {
        idrReq = theirs.idrReq;
    }

    if(theirs.buffLen.getValid()) {
        buffLen = theirs.buffLen;
    }

    if(theirs.tcpWindowSize.getValid()) {
        tcpWindowSize = theirs.tcpWindowSize;
    }

    if(theirs.tcpStreamControl.getValid()) {
        tcpStreamControl = theirs.tcpStreamControl;
    }

    if(theirs.server.getValid()) {
        server = theirs.server;
    }
}

/*
 * Overloaded rtspWfd += operator
 * Add valid classes from the rhs to the lhs rtspWfd class
 */
rtspWfd& rtspWfd::operator+=(const rtspWfd &rhs)
{
    if (rhs.audioLpcm.getValid()) {
        audioLpcm = rhs.audioLpcm;
    }
    if (rhs.audioAac.getValid()) {
        audioAac = rhs.audioAac;
    }
    if (rhs.audioEac.getValid()) {
        audioEac = rhs.audioEac;
    }
    if (rhs.audioAc3.getValid()) {
        audioAc3 = rhs.audioAc3;
    }
    if (rhs.audioDts.getValid()) {
        audioDts = rhs.audioDts;
    }
    if (rhs.videoHeader.getValid()) {
        videoHeader = rhs.videoHeader;
    }
    if (rhs.h264Cbp.getValid()) {
        h264Cbp = rhs.h264Cbp;
    }
    if (rhs.h264Chp.getValid()) {
        h264Chp = rhs.h264Chp;
    }
    if (rhs.h264Chi444p.getValid()) {
        h264Chi444p = rhs.h264Chi444p;
    }
    if (rhs.client.getValid()) {
        client = rhs.client;
    }
    if (rhs.uri.getValid()) {
        uri = rhs.uri;
    }
    if (rhs.uibcCap.getValid()) {
        uibcCap = rhs.uibcCap;
    }
    if (rhs.uibcSet.getValid()) {
        uibcSet = rhs.uibcSet;
    }
    if (rhs.standbyCap.getValid()) {
        standbyCap = rhs.standbyCap;
    }
    if (rhs.halt.getValid()) {
        halt = rhs.halt;
        start.setValid(false);
    } else if (rhs.start.getValid()) {
        start = rhs.start;
        halt.setValid(false);
    } else {
        start.setValid(false);
    }
    if (rhs.coupledSink.getValid()) {
        coupledSink = rhs.coupledSink;
    }
    if (rhs.audioRoute.getValid()) {
        audioRoute = rhs.audioRoute;
    }
    if (rhs.timing.getValid()) {
        timing = rhs.timing;
    }
    if (rhs.edid.getValid()) {
        edid = rhs.edid;
    }
    if (rhs.contentProtection.getValid()){
        contentProtection = rhs.contentProtection;
    }

    buffLen = rhs.buffLen;

    tcpWindowSize = rhs.tcpWindowSize;

    return *this;
}

/*
 * Calculate the working set of parameters
 */
void rtspWfd::calcWorkingSet()
{
    unsigned long res = videoHeader.getNativeRes();
    videoDisplay display = videoHeader.getNativeDisplay();
    int lpcmMaxMode = audioLpcm.getMaxMode(),
        aacMaxMode = audioAac.getMaxMode(),
        eacMaxMode = audioEac.getMaxMode(),
        Ac3MaxMode = audioAc3.getMaxMode(),
        dtsMaxMode = audioDts.getMaxMode();
    bitset<BITSET_8> ceaRes, vesaRes, hhRes;

    if (audioAac.getValid() && aacMaxMode >= 0) {
        audioAac.resetModes();
        audioAac.setModesBit(aacMaxMode);
        audioDts.setValid(false);
        audioEac.setValid(false);
        audioAc3.setValid(false);
        audioLpcm.setValid(false);
    } else if (audioDts.getValid() && dtsMaxMode >= 0) {
        audioDts.resetModes();
        audioDts.setModesBit(dtsMaxMode);
        audioAac.setValid(false);
        audioEac.setValid(false);
        audioAc3.setValid(false);
        audioLpcm.setValid(false);
    } else if (audioEac.getValid() && eacMaxMode >= 0) {
        audioEac.resetModes();
        audioEac.setModesBit(eacMaxMode);
        audioDts.setValid(false);
        audioAac.setValid(false);
        audioAc3.setValid(false);
        audioLpcm.setValid(false);
    } else if (audioLpcm.getValid() && lpcmMaxMode >= 0) {
        audioLpcm.resetModes();
        audioLpcm.setModesBit(lpcmMaxMode);
        audioDts.setValid(false);
        audioEac.setValid(false);
        audioAc3.setValid(false);
        audioAac.setValid(false);
    }
    else if (audioAc3.getValid() && Ac3MaxMode >= 0) {
        audioAc3.resetModes();
        audioAc3.setModesBit(Ac3MaxMode);
        audioDts.setValid(false);
        audioEac.setValid(false);
        audioLpcm.setValid(false);
        audioAac.setValid(false);
    }

    if (h264Chp.getValid()) {
        switch((unsigned int)display) {
        case cea:
            ceaRes = h264Chp.getCeaSupp();
            if (ceaRes[res]) {
                ceaRes.reset();
                ceaRes.set(res);
                h264Chp.setCeaSupp(ceaRes);
            } else {
                int idx = h264Chp.getMaxCeaMode();
                h264Chp.resetCea();
                h264Chp.setCeaBit((idx >= 0) ? idx : cea_640x480p60);
            }
            h264Chp.setVesaSupp(0);
            h264Chp.setHhSupp(0);
        break;
        case vesa:
            vesaRes = h264Chp.getVesaSupp();
            if (vesaRes[res]) {
                vesaRes.reset();
                vesaRes.set(res);
                h264Chp.setVesaSupp(vesaRes);
            } else {
                int idx = h264Chp.getMaxVesaMode();
                h264Chp.resetVesa();
                h264Chp.setVesaBit((idx >= 0) ? idx : vesa_800x600p30);
            }
            h264Chp.setCeaSupp(0);
            h264Chp.setHhSupp(0);
        break;
        case hh:
            hhRes = h264Chp.getHhSupp();
            if (hhRes[res]) {
                hhRes.reset();
                hhRes.set(res);
                h264Chp.setHhSupp(hhRes);
            } else {
                int idx = h264Chp.getMaxHhMode();
                h264Chp.resetHh();
                h264Chp.setHhBit((idx >= 0) ? idx : hh_800x480p30);
            }
            h264Chp.setVesaSupp(0);
            h264Chp.setCeaSupp(0);
        break;
        }
        h264Cbp.setValid(false);
        if (h264Chp.getProfile() == profileInvalid) {
            h264Chp.setProfile(chp);
        }
        if (h264Chp.getLevel() == h264_level_invalid) {
            h264Chp.setLevel(h264_level_3_1);
        }
    } else if (h264Cbp.getValid()) {
        switch((unsigned int)display) {
        case cea:
            ceaRes = h264Cbp.getCeaSupp();
            if (ceaRes[res]) {
                ceaRes.reset();
                ceaRes.set(res);
                h264Cbp.setCeaSupp(ceaRes);
            } else {
                int idx = h264Cbp.getMaxCeaMode();
                h264Cbp.resetCea();
                h264Cbp.setCeaBit((idx >= 0) ? idx : cea_640x480p60);
            }
            h264Cbp.setVesaSupp(0);
            h264Cbp.setHhSupp(0);
        break;
        case vesa:
            vesaRes = h264Cbp.getVesaSupp();
            if (vesaRes[res]) {
                vesaRes.reset();
                vesaRes.set(res);
                h264Cbp.setVesaSupp(vesaRes);
            } else {
                int idx = h264Cbp.getMaxVesaMode();
                h264Cbp.resetVesa();
                h264Cbp.setVesaBit((idx >= 0) ? idx : vesa_800x600p30);
            }
            h264Cbp.setCeaSupp(0);
            h264Cbp.setHhSupp(0);
        break;
        case hh:
            hhRes = h264Cbp.getHhSupp();
            if (hhRes[res]) {
                hhRes.reset();
                hhRes.set(res);
                h264Cbp.setHhSupp(hhRes);
            } else {
                int idx = h264Cbp.getMaxHhMode();
                h264Cbp.resetHh();
                h264Cbp.setHhBit((idx >= 0) ? idx : hh_800x480p30);
            }
            h264Cbp.setVesaSupp(0);
            h264Cbp.setCeaSupp(0);
        break;
        }
        h264Chp.setValid(false);
        if (h264Cbp.getProfile() == profileInvalid) {
            h264Cbp.setProfile(cbp);
        }
        if (h264Cbp.getLevel() == h264_level_invalid) {
            h264Cbp.setLevel(h264_level_3_1);
        }
    }

#if 0
    if ((uibcCap.getCategory())[hidc]) {
        uibcCap.unsetCategory(general);
        uibcCap.resetGenericCap();
    } else if ((uibcCap.getCategory())[general]) {
        uibcCap.unsetCategory(hidc);
        uibcCap.resetHidcCap();
        uibcCap.resetHidcMap();
    }

    else {
        h264Cbp.setValid(true);
        h264Cbp.setCeaBit(cea_640x480p60);
        h264Cbp.resetVesa();
        h264Cbp.resetHh();
        h264Cbp.setProfile(h264_level_3_1);
        h264Cbp.setLevel(cbp);
        h264Cbp.setMaxVres(0);
        h264Cbp.setMaxHres(0);
        h264Cbp.setLatency(0);
        h264Cbp.setMinSliceSize(0);
        h264Cbp.setSliceEnc((bitset<BITSET_2>) 0);
        h264Cbp.setFrameCtlSupp((bitset<BITSET_2>) 0);
    }
#endif
}

/*
 * Overloaded rtspWfd == operator
 */
bool rtspWfd::operator==(const rtspWfd &theirs)
{
    audioCodec lpcm = (audioLpcm & theirs.audioLpcm),
                aac = (audioAac & theirs.audioAac),
                eac = (audioEac & theirs.audioEac),
                ac3 = (audioAc3 & theirs.audioAc3),
                dts = (audioDts & theirs.audioDts);
    bitset<BITSET_2> cat = (uibcCap.getCategory() & theirs.uibcCap.getCategory()),
                     gen = (uibcCap.getGenericCap() & theirs.uibcCap.getGenericCap()),
                      hi = (uibcCap.getHidcCap() & theirs.uibcCap.getHidcCap());

    h264Codec cbp = (h264Cbp & theirs.h264Cbp),
              chp = (h264Chp & theirs.h264Chp);

    if (lpcm.getValid() && lpcm.getMaxMode() < 0) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: LPCM invalid");
        return false;
    } else if (aac.getValid() && aac.getMaxMode() < 0) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: AAC invalid");
        return false;
    } else if (eac.getValid() && eac.getMaxMode() < 0) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: EAC invalid");
        return false;
    } else if (ac3.getValid() && ac3.getMaxMode() < 0) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: AC3 invalid");
        return false;
    } else if (dts.getValid() && dts.getMaxMode() < 0) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: DTS invalid");
        return false;
    }

    if (cbp.getValid() &&
        ((cbp.getMaxCeaMode() < 0) &&
         (cbp.getMaxVesaMode() < 0) &&
         (cbp.getMaxHhMode() < 0))) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: CBP invalid");
        return false;
    }

    if (chp.getValid() &&
        ((chp.getMaxCeaMode() < 0) &&
         (chp.getMaxVesaMode() < 0) &&
         (chp.getMaxHhMode() < 0))) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: CHP invalid");
        return false;
    }

    if (uibcCap.getValid() && theirs.uibcCap.getValid()) {
        if (cat[hidc]) {
            if (!hi.any()) {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: HIDC caps invalid");
                return false;
            }
            for (int i = 0; i < UIBC_MAX_INPUT_SIZE; i++) {
                if (hi[i]) {
                    if (uibcCap.getHidcMap((uibcInput)i) != theirs.uibcCap.getHidcMap((uibcInput)i)) {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: HIDC input invalid");
                        return false;
                    }
                }
            }
        } else if (cat[general]) {
            if (!gen.any()) {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Generic cap invalid");
                return false;
            }
        } else {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: UIBC invalid");
            return false;
        }
    }

    return true;
}

/*
 * Erase up to first occurrence of character 'c' in string "input"
 */
string findStart(char *input, char c)
{
    size_t pos;

    string str(input);

    if ((pos = str.find(c)) != std::string::npos) {
        str.erase(0, pos+1);
    }

    //fflush(stdout);

    return str;
}

/*
 * Erase up to first occurrence of character 'c' in string "input"
 */
string findStart(string input, char c)
{
    size_t pos;

    if ((pos = input.find(c)) != std::string::npos) {
        input.erase(0, pos+1);
    }

    return input;
}

/*
 * Erase from the beginning of string "input" to the first occurrence
 * of character 'c'
 */
string findEnd(char *input, char c)
{
    size_t pos;

    string str(input);
    if ((pos = str.find(c)) != std::string::npos) {
        str.erase(pos+1, str[str.size()-1]);
    }

    return str;
}

/*
 * Return WFD parameter found in string "line"
 */
rtspWfdParams rtspWfd::wfdType(string line, bool &set)
{
    set = false;

    for (int i = 1; i < numSupportedWfdParams; i++) {
        if (line.find(wfdTable[i].wfdName) != std::string::npos) {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: Found  %s",  wfdTable[i].wfdName.c_str());
            if ((line.find(wfdTable[i].wfdName + ":") != std::string::npos) ||
                (wfdTable[i].wfdParam == wfd_standby) ||
                (wfdTable[i].wfdParam == wfd_resume) ||
                (wfdTable[i].wfdParam == wfd_idr_request))
                set = true;
            return wfdTable[i].wfdParam;
        }
    }

    return wfd_invalid;
}

/*
 * Parse WFD parameters contained in the string "line"
 */
void rtspWfd::wfdParse(rtspWfdParams param, string line)
{
    char *listStr = (char *)line.c_str();
    char *fieldStr;
    char *pTempPtr = NULL;

    switch(param) {
    case wfd_audio_codecs:
        if ((fieldStr = strtok_r(listStr, ",", &pTempPtr))) {
            string begin = findStart(fieldStr, ' ');
            fieldStr = (char *)begin.c_str();

            do {
                audioCodec aCodec;
                string data(fieldStr);
                if (!data.length())
                    break;
                RTSPStringStream in(data);
                in >> aCodec;

                if (!strcmp(aCodec.getName().c_str(), "LPCM")) {
                    audioLpcm = aCodec;
                    //cout << audioLpcm << endl;
                } else if (!strcmp(aCodec.getName().c_str(), "AAC")) {
                    audioAac = aCodec;
                    //cout << audioAac << endl;
                } else if (!strcmp(aCodec.getName().c_str(), "E-AC")) {
                    audioEac = aCodec;
                    //cout << audioEac << endl;
                } else if (!strcmp(aCodec.getName().c_str(), "AC3")) {
                    audioAc3 = aCodec;
                    //cout << audioAc3 << endl;
                } else if (!strcmp(aCodec.getName().c_str(), "DTS")) {
                    audioDts = aCodec;
                    //cout << audioDts << endl;
                }
            } while ((fieldStr = strtok_r(NULL, ",", &pTempPtr)));
        }
    break;
    case wfd_video_formats:
        pTempPtr = NULL;
        if ((fieldStr = strtok_r(listStr, ",", &pTempPtr))) {
            string begin = findStart(fieldStr, ' ');
            fieldStr = (char *)begin.c_str();

            string data(fieldStr);
            if (!data.length())
                break;

            RTSPStringStream in(data);
            in >> videoHeader;
            //cout << videoHeader << endl;
            data.erase(0, 6);
            fieldStr = (char *)data.c_str();

            do {
                h264Codec hCodec;
                string data(fieldStr);
                RTSPStringStream in(data);
                in >> hCodec;

                if (hCodec.getProfile() == cbp) {
                    h264Cbp = hCodec;
                    //cout << h264Cbp << endl;
                } else if (hCodec.getProfile() == chp) {
                    h264Chp = hCodec;
                    //cout << h264Chp << endl;
                } else if (hCodec.getProfile() == chi444p) {
                    h264Chi444p = hCodec;
                    //cout << h264Chi444p << endl;
                }
            } while ((fieldStr = strtok_r(NULL, ",", &pTempPtr)));
        }
    break;
    case wfd_trigger_method:
        pTempPtr = NULL;
        if ((fieldStr = strtok_r(listStr, " ", &pTempPtr))) {
            if ((fieldStr = strtok_r(NULL, " ", &pTempPtr))) {
                string data(fieldStr);
                if (!data.length())
                    break;
                RTSPStringStream in(data);
                in >> trigger;
                //cout << trigger << endl;
            }
        }
    break;
    case wfd_presentation_URL:
    {
        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        in >> uri;
    }
    break;
    case wfd_client_rtp_ports:
    {
        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        in >> client;
        //cout << client << endl;
    }
    break;
    case wfd_content_protection:
    {
      string data = findStart(listStr, ' ');
      if(!data.length())
        break;

      RTSPStringStream in(data);
      in >> contentProtection;
      //cout << contentProtection << endl;
    }
    break;
    case wfd_uibc_capability:
    {
        string strArr[UIBC_NUM_FIELDS];
        int count = 0;
        pTempPtr = NULL;

        if ((fieldStr = strtok_r(listStr, ";", &pTempPtr))) {
            string begin = findStart(fieldStr, ' ');
            fieldStr = (char *)begin.c_str();
            uibcCapability uCap;
            do {
                string data(fieldStr);

                if (!data.length())
                    break;

                replace(data.begin(), data.end(), '=', ' ');

                strArr[count++] = data;
            } while ((fieldStr = strtok_r(NULL, ";", &pTempPtr)));

            for (int i = 0; i < count; i++) {
                RTSPStringStream in(strArr[i]);
                in >> uCap;
            }
            uibcCap = uCap;
        }
    }
    break;
    case wfd_uibc_setting:
    {
        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        in >> uibcSet;
        //cout << uibcSet << endl;
    }
    break;
    case wfd_standby_resume_capability:
    {
        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        in >> standbyCap;
        //cout << standbyCap << endl;
    }
    break;
    case wfd_standby:
        halt.setValid(true);
        start.setValid(false);
    break;
    case wfd_resume:
        start.setValid(true);
        halt.setValid(false);
    break;
    case wfd_route:
    {
        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        in >> audioRoute;
        //cout << audioRoute << endl;
    }
    break;
    case wfd_av_format_change_timing:
    {
        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        in >> timing;
        //cout << timing << endl;
    }
    break;
#ifdef LEGACY_TCP
    case wfd_vnd_sec_max_buffer_length:
    {
        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        string len;
        in >> len;
        buffLen.setBufferLen(stringToNum(len));
        buffLen.setValid(true);
        //cout << listStr << endl;
    }
    break;

    case wfd_vnd_sec_tcp_window_size:
    {
        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        string size;
        in >> size;
        tcpWindowSize.setWindowSize(stringToNum(size));
        //cout << listStr << endl;
    }
    break;

    case wfd_vnd_sec_control_playback:
    {
        string data = findStart(listStr, '=');

        if(!data.length()) {
            break;
        }

        RTSPStringStream in(data);
        string control;
        in >> control;

        if(control == "play") {
            tcpStreamControl.setCommand(play);
        }else if (control == "pause") {
            tcpStreamControl.setCommand(rtsp_wfd::pause);
        }else if (control == "flush") {
            tcpStreamControl.setCommand(rtsp_wfd::flush);
        }else if (control == "status"){
            tcpStreamControl.setCommand(status);
        }else
            break;
    }
    break;
#else
    case wd_initial_buffer:
    {
        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        string size;
        in >> size;
        tcpWindowSize.setWindowSize(stringToNum(size));
    }
    break;
    case wd_playback_control:
    {
        string data = findStart(listStr, '=');
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: Inside wd_playback_control");

        if(!data.length()) {
            break;
        }

        RTSPStringStream in(data);
        string control;
        in >> control;

        if(control == "play") {
            tcpStreamControl.setCommand(play);
        }else if (control == "pause") {
            tcpStreamControl.setCommand(rtsp_wfd::pause);
        }else if ((control.substr(0,5)) == "flush") {
            tcpStreamControl.setCommand(rtsp_wfd::flush);
            string timing = findStart(data,'=');
            RTSPStringStream duration(timing);
            unsigned long long flush_timing;
            duration>>flush_timing;
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: wd_playback_control flush_timing found = %llu",flush_timing);
            tcpStreamControl.setDuration(flush_timing);
        }else if (control == "status"){
            tcpStreamControl.setCommand(status);
        }else
            break;
    }
    break;
    case wd_decoder_latency:
    {

        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        string len;
        in >> len;
        buffLen.setBufferLen(stringToNum(len));
        buffLen.setValid(true);
    }

    break;
#endif
    case wfd_I2C:
    break;
    case wfd_preferred_display_mode:
    break;
    case wfd_3d_video_formats:
    break;
    case wfd_display_edid:
    {
        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        in >> edid;
        //cout << edid << endl;
    }
    break;
    case wfd_coupled_sink:
    {
        string data = findStart(listStr, ' ');

        if (!data.length())
            break;

        RTSPStringStream in(data);
        in >> coupledSink;
        //cout << coupledSink << endl;
    }
    break;
    case wfd_connector_type:
    {
       string data = findStart(listStr,' ');
       if (!data.length())
         break;

       RTSPStringStream in(data);
       string ctVal;
       in >> ctVal;
       connectorType = stringToNum(ctVal);
    }
    break;
    case wfd_idr_request:
        idrReq.setValid(true);
    break;
    case wfd_invalid:
    default:
    break;
    }
}

/*
 * Get value corresponding to string "tag" in string "buffer"
 */
string getNvp(string buffer, string tag)
{
    string tagStart = "<" + tag + ">";
    string tagEnd = "</" + tag + ">";
    string ret;
    size_t begin, end;

    if ((begin = buffer.find(tagStart)) != string::npos) {
        if ((end = buffer.find(tagEnd)) != string::npos) {
            begin += tagStart.length();
            size_t len = end - begin;
            char *result = (char*)MM_Malloc((len+1)*sizeof(char));
            if(result == NULL)
            {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: result memory allocation failed");
               return "";
            }
            size_t stop = buffer.copy(result, len, begin);
            result[stop] = '\0';
            ret = string(result);
            RTSP_FREEIF( result);
        }
    }

    return ret;
}

/*
 * Parse XML audio codec entries
 */
void rtspWfd::parseAudioXml(string buffer, audioCodecName type)
{
    switch(type)
    {
    case lpcm:
    {
        string name = getNvp(buffer, "Name");
        bitset<BITSET_8> modes(getNvp(buffer, "Modes"));
        unsigned short latency = (unsigned short)atoi((getNvp(buffer, "Latency")).c_str());
        bool valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

        audioCodec audio(name, modes, latency, valid);
        audioLpcm = audio;
    }
    break;
    case aac:
    {
        string name = getNvp(buffer, "Name");
        bitset<BITSET_8> modes(getNvp(buffer, "Modes"));
        unsigned short latency = (unsigned short)atoi((getNvp(buffer, "Latency")).c_str());
        bool valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

        audioCodec audio(name, modes, latency, valid);
        audioAac = audio;
    }
    break;
    case eac:
    {
        string name = getNvp(buffer, "Name");
        bitset<BITSET_8> modes(getNvp(buffer, "Modes"));
        unsigned short latency = (unsigned short)atoi((getNvp(buffer, "Latency")).c_str());
        bool valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

        audioCodec audio(name, modes, latency, valid);
        audioEac = audio;
    }
    break;

    case ac3:
    {
        string name = getNvp(buffer, "Name");
        bitset<BITSET_8> modes(getNvp(buffer, "Modes"));
        unsigned short latency = (unsigned short)atoi((getNvp(buffer, "Latency")).c_str());
        bool valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

        audioCodec audio(name, modes, latency, valid);
        audioAc3 = audio;
    }
    break;

    case dts:
    {
        string name = getNvp(buffer, "Name");
        bitset<BITSET_8> modes(getNvp(buffer, "Modes"));
        unsigned short latency = (unsigned short)(atoi((getNvp(buffer, "Latency")).c_str()));
        bool valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

        audioCodec audio(name, modes, latency, valid);
        audioDts = audio;
    }
    break;
    }
}

/*
 * Parse XML video header entry
 */
void rtspWfd::parseVideoHeaderXml(string buffer)
{
    bitset<BITSET_2> native(getNvp(buffer, "Native"));
    bitset<BITSET_2> prefDisp(getNvp(buffer, "PreferredDisplaySupport"));
    bool valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

    videoCodec video(native, prefDisp, valid);
    videoHeader = video;
}

/*
 * Parse XML video codec entries
 */
void rtspWfd::parseCodecXml(string buffer, videoCodecProfile type)
{
    switch((unsigned int)type)
    {
    case cbp:
    {
        string name = getNvp(buffer, "Name");
        bitset<BITSET_2> profile(getNvp(buffer, "Profile"));
        bitset<BITSET_2> level(getNvp(buffer, "Level"));
        unsigned short hRes = (unsigned short)atoi((getNvp(buffer, "HorizontalResolution")).c_str());
        unsigned short vRes = (unsigned short)atoi((getNvp(buffer, "VerticalResolution")).c_str());
        bitset<BITSET_8> ceaSupp(getNvp(buffer, "CeaSupport"));
        bitset<BITSET_8> vesaSupp(getNvp(buffer, "VesaSupport"));
        bitset<BITSET_8> hhSupp(getNvp(buffer, "HhSupport"));
        unsigned char latency = (unsigned char)atoi((getNvp(buffer, "Latency")).c_str());
        unsigned short minSliceSize = (unsigned short)atoi((getNvp(buffer, "MinimumSliceSize")).c_str());
        bitset<BITSET_2> sliceEnc(getNvp(buffer, "SliceEncodingParams"));
        bitset<BITSET_2> frameRateCtl(getNvp(buffer, "FrameRateControlSupport"));
        bool valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

        h264Codec video(profile, level, hRes, vRes, ceaSupp, vesaSupp, hhSupp,
                        latency, minSliceSize, sliceEnc, frameRateCtl, valid);
        h264Cbp = video;
    }
    break;
    case chp:
    {
        string name = getNvp(buffer, "Name");
        bitset<BITSET_2> profile(getNvp(buffer, "Profile"));
        bitset<BITSET_2> level(getNvp(buffer, "Level"));
        unsigned short hRes = (unsigned short)atoi((getNvp(buffer, "HorizontalResolution")).c_str());
        unsigned short vRes = (unsigned short)atoi((getNvp(buffer, "VerticalResolution")).c_str());
        bitset<BITSET_8> ceaSupp(getNvp(buffer, "CeaSupport"));
        bitset<BITSET_8> vesaSupp(getNvp(buffer, "VesaSupport"));
        bitset<BITSET_8> hhSupp(getNvp(buffer, "HhSupport"));
        unsigned char latency = (unsigned char)atoi((getNvp(buffer, "Latency")).c_str());
        unsigned short minSliceSize = (unsigned short)atoi((getNvp(buffer, "MinimumSliceSize")).c_str());
        bitset<BITSET_2> sliceEnc(getNvp(buffer, "SliceEncodingParams"));
        bitset<BITSET_2> frameRateCtl(getNvp(buffer, "FrameRateControlSupport"));
        bool valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

        h264Codec video(profile, level, hRes, vRes, ceaSupp, vesaSupp, hhSupp,
                        latency, minSliceSize, sliceEnc, frameRateCtl, valid);
        h264Chp = video;
    }
    break;
    case chi444p:
    {
        string name = getNvp(buffer, "Name");
        bitset<BITSET_2> profile(getNvp(buffer, "Profile"));
        bitset<BITSET_2> level(getNvp(buffer, "Level"));
        unsigned short hRes = (unsigned short)atoi((getNvp(buffer, "HorizontalResolution")).c_str());
        unsigned short vRes = (unsigned short)atoi((getNvp(buffer, "VerticalResolution")).c_str());
        bitset<BITSET_8> ceaSupp(getNvp(buffer, "CeaSupport"));
        bitset<BITSET_8> vesaSupp(getNvp(buffer, "VesaSupport"));
        bitset<BITSET_8> hhSupp(getNvp(buffer, "HhSupport"));
        unsigned char latency = (unsigned char)atoi((getNvp(buffer, "Latency")).c_str());
        unsigned short minSliceSize = (unsigned short)atoi((getNvp(buffer, "MinimumSliceSize")).c_str());
        bitset<BITSET_2> sliceEnc(getNvp(buffer, "SliceEncodingParams"));
        bitset<BITSET_2> frameRateCtl(getNvp(buffer, "FrameRateControlSupport"));
        bool valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

        h264Codec video(profile, level, hRes, vRes, ceaSupp, vesaSupp, hhSupp,
                        latency, minSliceSize, sliceEnc, frameRateCtl, valid);
        h264Chi444p = video;
    }
    break;
    }
}

/*
 * Find UIBC HIDC path for a given input
 */
uibcPath findPath(string input)
{
    if (input.find("infrared") != string::npos)
        return infrared;
    if (input.find("usb") != string::npos)
        return usb;
    if (input.find("bt") != string::npos)
        return bt;
    if (input.find("zigbee") != string::npos)
        return zigbee;
    if (input.find("wi-fi") != string::npos)
        return wifi;

    return noSp;
}

/*
 * Parse XML UIBC capability entry
 */
void rtspWfd::parseUibcXml(string buffer, rtspWfdParams type)
{
    UNUSED(type);
    bitset<BITSET_2> genCaps = 0;
    bitset<BITSET_2> hidcCaps = 0;
    uibcPath hidcMap[UIBC_MAX_INPUT_SIZE];
    string input;
    bitset<BITSET_2> category = 0;
    bitset<BITSET_2> types = 0;
    bool valid = false;
    unsigned port = 0;

    memset(hidcMap, 0, sizeof(hidcMap));

    valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

    if (false == valid)
        return;

    input = getNvp(buffer, "InputCategory");
    transform(input.begin(), input.end(), input.begin(), (int(*)(int))tolower);

    if (!strcmp(input.c_str(), "hidc"))
        category.set(hidc);
    else if (!strcmp(input.c_str(), "generic"))
        category.set(general);
    else if (!strcmp(input.c_str(), "both")) {
        category.set(general);
        category.set(hidc);
    }

    input = getNvp(buffer, "GenericCapability");
    transform(input.begin(), input.end(), input.begin(), (int(*)(int))tolower);

    if (input.find("keyboard") != string::npos)
        genCaps.set(keyboard);
    if (input.find("mouse") != string::npos)
        genCaps.set(mouse);
    if (input.find("singletouch") != string::npos)
        genCaps.set(singleTouch);
    if (input.find("multitouch") != string::npos)
        genCaps.set(multiTouch);
    if (input.find("joystick") != string::npos)
        genCaps.set(joystick);
    if (input.find("camera") != string::npos)
        genCaps.set(camera);
    if (input.find("gesture") != string::npos)
        genCaps.set(gesture);
    if (input.find("remotecontrol") != string::npos)
        genCaps.set(remoteControl);

    string keyboardInput = getNvp(buffer, "HidcKeyboard");
    if (!keyboardInput.empty()) {
        transform(keyboardInput.begin(), keyboardInput.end(), keyboardInput.begin(), (int(*)(int))tolower);
        uibcPath keyboardPath = findPath(keyboardInput);
        hidcMap[keyboard] = keyboardPath;
        hidcCaps.set(keyboard);
    }

    string mouseInput = getNvp(buffer, "HidcMouse");
    if (!mouseInput.empty()) {
        transform(mouseInput.begin(), mouseInput.end(), mouseInput.begin(), (int(*)(int))tolower);
        uibcPath mousePath = findPath(mouseInput);
        hidcMap[mouse] = mousePath;
        hidcCaps.set(mouse);
    }

    string singleTouchInput = getNvp(buffer, "HidcSingleTouch");
    if (!singleTouchInput.empty()) {
        transform(singleTouchInput.begin(), singleTouchInput.end(), singleTouchInput.begin(), (int(*)(int))tolower);
        uibcPath singleTouchPath = findPath(singleTouchInput);
        hidcMap[singleTouch] = singleTouchPath;
        hidcCaps.set(singleTouch);
    }

    string multiTouchInput = getNvp(buffer, "HidcMultiTouch");
    if (!multiTouchInput.empty()) {
        transform(multiTouchInput.begin(), multiTouchInput.end(), multiTouchInput.begin(), (int(*)(int))tolower);
        uibcPath multiTouchPath = findPath(multiTouchInput);
        hidcMap[multiTouch] = multiTouchPath;
        hidcCaps.set(multiTouch);
    }

    string joystickInput = getNvp(buffer, "HidcJoystick");
    if (!joystickInput.empty()) {
        transform(joystickInput.begin(), joystickInput.end(), joystickInput.begin(), (int(*)(int))tolower);
        uibcPath joystickPath = findPath(joystickInput);
        hidcMap[joystick] = joystickPath;
        hidcCaps.set(joystick);
    }

    string cameraInput = getNvp(buffer, "HidcCamera");
    if (!cameraInput.empty()) {
        transform(cameraInput.begin(), cameraInput.end(), cameraInput.begin(), (int(*)(int))tolower);
        uibcPath cameraPath = findPath(cameraInput);
        hidcMap[camera] = cameraPath;
        hidcCaps.set(camera);
    }

    string gestureInput = getNvp(buffer, "HidcGesture");
    if (!gestureInput.empty()) {
        transform(gestureInput.begin(), gestureInput.end(), gestureInput.begin(), (int(*)(int))tolower);
        uibcPath gesturePath = findPath(gestureInput);
        hidcMap[gesture] = gesturePath;
        hidcCaps.set(gesture);
    }

    string remoteInput = getNvp(buffer, "HidcRemoteControl");
    if (!remoteInput.empty()) {
        transform(remoteInput.begin(), remoteInput.end(), remoteInput.begin(), (int(*)(int))tolower);
        uibcPath remoteControlPath = findPath(remoteInput);
        hidcMap[remoteControl] = remoteControlPath;
        hidcCaps.set(remoteControl);
    }

    port = atoi((getNvp(buffer, "Port")).c_str());

    uibcCapability caps(category, genCaps, hidcCaps, hidcMap, port, valid);

    uibcCap = caps;
}

/*
 * Parse XML coupled sink entry
 */
void rtspWfd::parseCoupling(string buffer)
{
    bitset<BITSET_2> status(getNvp(buffer, "Status"));
    string mac = getNvp(buffer, "MacAddress");
    bool valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

    if (false == valid)
        return;

    coupled couple(status, mac);

    coupledSink = couple;
}

void rtspWfd::parseStandbyXml(string buffer)
{
  bool valid = false;
  valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;
  standby_resume_cap cap(valid,valid);
  standbyCap = cap;
}
void rtspWfd::parseContentProtection(string buffer)
{
    int port = 0;
    bool valid = atoi((getNvp(buffer, "Valid")).c_str()) ? true : false;

    port = atoi((getNvp(buffer, "Port")).c_str());
    hdcp_cp contentProtection(port);
}

/*
 * Top level XML parsing
 */
void rtspWfd::parseXml(string buffer)
{
    string nvp;

    nvp = getNvp(buffer, "AudioLPCM");
    if (nvp.length())
        parseAudioXml(nvp, lpcm);

    nvp = getNvp(buffer, "AudioAAC");
    if (nvp.length())
        parseAudioXml(nvp, aac);

    nvp = getNvp(buffer, "AudioEAC");
    if (nvp.length())
        parseAudioXml(nvp, eac);

    nvp = getNvp(buffer, "AudioAC3");
    if (nvp.length())
        parseAudioXml(nvp, ac3);

    nvp = getNvp(buffer, "AudioDTS");
    if (nvp.length())
        parseAudioXml(nvp, dts);

    nvp = getNvp(buffer, "VideoHeader");
    if (nvp.length())
        parseVideoHeaderXml(nvp);

    nvp = getNvp(buffer, "H264CBP");
    if (nvp.length())
        parseCodecXml(nvp, cbp);

    nvp = getNvp(buffer, "H264CHP");
    if (nvp.length())
        parseCodecXml(nvp, chp);

    nvp = getNvp(buffer, "H264Chi444p");
    if (nvp.length())
        parseCodecXml(nvp, chi444p);

    nvp = getNvp(buffer, "UibcCapability");
    if (nvp.length())
        parseUibcXml(nvp, wfd_uibc_capability);

    nvp = getNvp(buffer, "CoupledSink");
    if (nvp.length())
        parseCoupling(nvp);

    nvp = getNvp(buffer, "StandbyResumeCapability");
    if (nvp.length())
      parseStandbyXml(nvp);

    nvp = getNvp(buffer, "ContentProtection");
    if (nvp.length())
        parseContentProtection(nvp);

}

/*
 * Load and parse XML config file
 */
void rtspWfd::init(string filename)
{
    FILE * fp = fopen(filename.c_str(), "r");
    char *buffer;
    size_t length;

    if (!fp)
    {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: Unable to open config file");
       return;
    }

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = (char*)MM_Malloc(length*sizeof(char));
    if(buffer == NULL)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTSP_LIB :: buffer memory allocation failed");
        return;
    }

    fread(buffer, sizeof(char), length, fp);

    parseXml(string(buffer));

    //cout.write(buffer, length);
    dump();

    fclose(fp);

    RTSP_FREEIF(buffer);
}

/*
 * Dump all valid rtspWfd members
 */
void rtspWfd::dump()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB ::**************************");

    if (trigger.getValid()) {
        RTSPStringStream ss;
        ss << trigger;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", ss.str().c_str());
    }
    if (audioLpcm.getValid()) {
        RTSPStringStream ss;
        ss << audioLpcm;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (audioAac.getValid()) {
        RTSPStringStream ss;
        ss << audioAac;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", ss.str().c_str());
    }
    if (audioEac.getValid()) {
        RTSPStringStream ss;
        ss << audioEac;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (audioAc3.getValid()) {
        RTSPStringStream ss;
        ss << audioAc3;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (audioDts.getValid()) {
        RTSPStringStream ss;
        ss << audioDts;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (videoHeader.getValid()) {
        RTSPStringStream ss;
        ss << videoHeader;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (h264Cbp.getValid()) {
        RTSPStringStream ss;
        ss << h264Cbp;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (h264Chp.getValid()) {
        RTSPStringStream ss;
        ss << h264Chp;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (h264Chi444p.getValid()) {
        RTSPStringStream ss;
        ss << h264Chi444p;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (uri.getValid()) {
        RTSPStringStream ss;
        ss << uri;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (client.getValid()) {
        RTSPStringStream ss;
        ss << client;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (uibcCap.getValid()) {
        RTSPStringStream ss;
        ss << uibcCap;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (uibcSet.getValid()) {
        RTSPStringStream ss;
        ss << "UIBC Setting: ";
        ss << uibcSet;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", ss.str().c_str());
    }
    if (standbyCap.getValid()) {
        RTSPStringStream ss;
        ss << standbyCap;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (halt.getValid()) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: Standby");
    }
    if (start.getValid()) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: Resume");
    }
    if (coupledSink.getValid()) {
        RTSPStringStream ss;
        ss << coupledSink;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (edid.getValid()) {
        RTSPStringStream ss;
        ss << edid;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }
    if (contentProtection.getValid()) {
        RTSPStringStream ss;
        ss << contentProtection;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: %s", ss.str().c_str());
    }

    if (timing.getValid()) {
        RTSPStringStream ss;
        ss << timing;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTSP_LIB :: %s", ss.str().c_str());
    }

    if (idrReq.getValid()) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: IDR Request");
    }
    if (isPrimarySink) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: Primary Sink");
    }
    if (isSecondarySink) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: Secondary Sink");
    }

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTSP_LIB :: **************************");
}
