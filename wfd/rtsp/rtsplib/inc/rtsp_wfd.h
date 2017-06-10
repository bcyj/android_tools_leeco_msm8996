#ifndef _RTSP_WFD_H
#define _RTSP_WFD_H

/***************************************************************************
 *                             rtsp_wfd.h
 * DESCRIPTION
 *  RTSP WFD class for RTSP_LIB module
 *
 * Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header: //source/qcom/qct/multimedia2/Video/wfd/rtsp/main/latest/rtsplib/inc/rtsp_wfd.h#1 $
  $DateTime: 2011/12/14 03:28:24 $
  $Change: 2096652 $
 ***************************************************************************/

#include "rtsp_common.h"
#include "RTSPStringStream.h"

static const char profile[] = "RTP/AVP/UDP;unicast";
static const char profiletcp[] = "RTP/AVP/TCP;unicast";
static const char uibc_input_category[] = "input_category_list";
static const char uibc_generic_capability[] = "generic_cap_list";
static const char uibc_hidc_capability[] = "hidc_cap_list";
static const char tcp_port[] = "port";
static const char uibc_generic[] = "GENERIC";
static const char uibc_hidc[] = "HIDC";
static const char none[] = "none";
static const char enable[] = "enable";
static const char disable[] = "disable";
static const char primary[] = "primary";
static const char secondary[] = "secondary";
static const char supported[] = "supported";
static const char hdcp_spec_2_0[] = "HDCP2.0";
static const char hdcp_spec_2_1[] = "HDCP2.1";

/*
 * WFD Parameters
 */
enum rtspWfdParams {
    wfd_invalid,
    wfd_audio_codecs,
    wfd_video_formats,
    wfd_3d_video_formats,
    wfd_content_protection,
    wfd_display_edid,
    wfd_coupled_sink,
    wfd_trigger_method,
    wfd_presentation_URL,
    wfd_client_rtp_ports,
    wfd_route,
    wfd_I2C,
    wfd_av_format_change_timing,
    wfd_preferred_display_mode,
    wfd_uibc_capability,
    wfd_uibc_setting,
    wfd_standby_resume_capability,
    wfd_standby,
    wfd_resume,
    wfd_connector_type,
    wfd_idr_request,
#ifdef LEGACY_TCP
    wfd_vnd_sec_max_buffer_length,
    wfd_vnd_sec_tcp_window_size,
    wfd_vnd_sec_control_playback
#else
    wd_decoder_latency,
    wd_initial_buffer,
    wd_playback_control
#endif
};
#define WFD_MAX_SIZE  32

struct rtspWfdParamTable {
    rtspWfdParams wfdParam;
    string wfdName;
};

/*
 * Table of valid WFD parameters
 */
static const rtspWfdParamTable wfdTable[] = {
                            {wfd_invalid, ""},
                            {wfd_audio_codecs, "wfd_audio_codecs"},
                            {wfd_video_formats, "wfd_video_formats"},
                            {wfd_3d_video_formats, "wfd_3d_video_formats"},
                            {wfd_content_protection, "wfd_content_protection"},
                            {wfd_display_edid, "wfd_display_edid"},
                            {wfd_coupled_sink, "wfd_coupled_sink"},
                            {wfd_trigger_method, "wfd_trigger_method"},
                            {wfd_presentation_URL, "wfd_presentation_URL"},
                            {wfd_client_rtp_ports, "wfd_client_rtp_ports"},
                            {wfd_route, "wfd_route"},
                            {wfd_I2C, "wfd_I2C"},
                            {wfd_av_format_change_timing,
                             "wfd_av_format_change_timing"},
                            {wfd_preferred_display_mode,
                             "wfd_preferred_display_mode"},
                            {wfd_uibc_capability,
                             "wfd_uibc_capability"},
                            {wfd_uibc_setting,
                             "wfd_uibc_setting"},
                            {wfd_standby_resume_capability,
                             "wfd_standby_resume_capability"},
                            {wfd_standby,
                             "wfd_standby"},
                            {wfd_resume,
                             "wfd_resume"},
                            {wfd_connector_type,
                             "wfd_connector_type"},
                            {wfd_idr_request,
                             "wfd_idr_request"},
#ifdef TCP_LEGACY
                            {wfd_vnd_sec_max_buffer_length,
                                "wfd_vnd_sec_max_buffer_length"},
                            {wfd_vnd_sec_tcp_window_size,
                                "wfd_vnd_sec_tcp_window_size"},
                            {wfd_vnd_sec_control_playback,
                                 "wfd_vnd_sec_control_playback"}
#else
                            {wd_decoder_latency,
                                "wd_decoder_latency"},
                            {wd_initial_buffer,
                                "wd_initial_buffer"},
                            {wd_playback_control,
                                 "wd_playback_control"}
#endif
                           };

static const int numSupportedWfdParams = (int)(sizeof(wfdTable) / sizeof(wfdTable[0]));

#define BITSET_8  32
#define BITSET_4  16
#define BITSET_2  8

#define EDID_BLOCK_SIZE		256
#define EDID_MAX_BLOCKS		256

#define COUPLED				0
#define TEARDOWN_COUPLING	1
#define PRIMARY				1
#define SECONDARY			2

/*
 * WFD related enums/structs
 */
namespace rtsp_wfd {
    enum rtspMode {
        source,
        sink,
        coupledPrimarySink,
        coupledSecondarySink
    };

    enum videoCodecLevel {
        h264_level_3_1,
        h264_level_3_2,
        h264_level_4,
        h264_level_4_1,
        h264_level_4_2,
        h264_level_invalid
    };

    enum videoDisplay {
        cea,
        vesa,
        hh,
        displayInvalid
    };

    enum videoCodecProfile {
        cbp,
        chp,
        chi444p,
        profileInvalid
    };

    enum audioCodecName {
        lpcm,
        aac,
        eac,
        ac3,
        dts
    };

    enum uibcCategory {
        general,
        hidc
    };

    enum uibcInput {
        keyboard,
        mouse,
        singleTouch,
        multiTouch,
        joystick,
        camera,
        gesture,
        remoteControl
    };

    enum uibcPath {
        noSp,
        infrared,
        usb,
        bt,
        zigbee,
        wifi
    };

    struct inputTypes {
        uibcInput type;
        string name;
    };

    struct pathTypes {
        uibcPath type;
        string name;
    };

    enum ceaResolution {
        cea_640x480p60,
        cea_720x480p60,
        cea_720x480i60,
        cea_720x576p50,
        cea_720x576i50,
        cea_1280x720p30,
        cea_1280x720p60,
        cea_1920x1080p30,
        cea_1920x1080p60,
        cea_1920x1080i60,
        cea_1280x720p25,
        cea_1280x720p50,
        cea_1920x1080p25,
        cea_1920x1080p50,
        cea_1920x1080i50,
        cea_1280x720p24,
        cea_1920x1080p24
    };

    enum vesaResolution {
        vesa_800x600p30,
        vesa_800x600p60,
        vesa_1024x768p30,
        vesa_1024x768p60,
        vesa_1152x864p30,
        vesa_1152x864p60,
        vesa_1280x768p30,
        vesa_1280x768p60,
        vesa_1280x800p30,
        vesa_1280x800p60,
        vesa_1360x768p30,
        vesa_1360x768p60,
        vesa_1366x768p30,
        vesa_1366x768p60,
        vesa_1280x1024p30,
        vesa_1280x1024p60,
        vesa_1400x1050p30,
        vesa_1400x1050p60,
        vesa_1440x900p30,
        vesa_1440x900p60,
        vesa_1600x900p30,
        vesa_1600x900p60,
        vesa_1600x1200p30,
        vesa_1600x1200p60,
        vesa_1680x1024p30,
        vesa_1680x1024p60,
        vesa_1680x1050p30,
        vesa_1680x1050p60,
        vesa_1920x1200p30,
        vesa_1920x1200p60
    };

    enum hhResolution {
        hh_800x480p30,
        hh_800x480p60,
        hh_854x480p30,
        hh_854x480p60,
        hh_864x480p30,
        hh_864x480p60,
        hh_640x360p30,
        hh_640x360p60
    };

    enum lpcmModes {
        lpcm_441_16_2,
        lpcm_48_16_2,
    };

    enum aacModes {
        aac_48_16_2,
        aac_48_16_4,
        aac_48_16_6,
        aac_48_16_8
    };

    enum eacModes {
        eac_48_16_2,
        eac_48_16_4,
        eac_48_16_6,
    };

    enum ac3Modes {
        ac3_48_16_2,
        ac3_48_16_4,
        ac3_48_16_6,
    };

    enum dtsModes {
        dts_48_16_2,
        dts_48_16_4,
        dts_48_16_6
    };

    enum hdcp2_spec {
      hdcp_cp_2_0,
      hdcp_cp_2_1
    };

    enum tcp_control_command {
        cmdNone,
        flush,
        play,
        pause,
        status
    };
}

/*
 * UIBC Input types
 */
static const rtsp_wfd::inputTypes inputTable[] =
                                       {
                                         {rtsp_wfd::keyboard, "Keyboard"},
                                         {rtsp_wfd::mouse, "Mouse"},
                                         {rtsp_wfd::singleTouch, "SingleTouch"},
                                         {rtsp_wfd::multiTouch, "MultiTouch"},
                                         {rtsp_wfd::joystick, "Joystick"},
                                         {rtsp_wfd::camera, "Camera"},
                                         {rtsp_wfd::gesture, "Gesture"},
                                         {rtsp_wfd::remoteControl, "RemoteControl"},
                                       };

/*
 * UIBC Input paths
 */
static const rtsp_wfd::pathTypes pathTable[] =
                                     {
                                       {rtsp_wfd::noSp, "No-SP"},
                                       {rtsp_wfd::infrared, "Infrared"},
                                       {rtsp_wfd::usb, "USB"},
                                       {rtsp_wfd::bt, "BT"},
                                       {rtsp_wfd::zigbee, "Zigbee"},
                                       {rtsp_wfd::wifi, "Wi-Fi"},
                                     };

#define UIBC_MAX_INPUT_SIZE 8
#define UIBC_NUM_FIELDS		4

static const int numSupportedInputs = (int)(sizeof(inputTable) / sizeof(inputTable[0]));
static const int numSupportedPaths = (int)(sizeof(pathTable) / sizeof(pathTable[0]));

struct wfdAudioMode {
    unsigned int        sampleFrequency;   /* Hz */
    unsigned short int  bitsPerSample;
    unsigned short int  numChannels;
};

/*
 * wfd_audio_codecs
 */
class audioCodec {
public:
    audioCodec() : name(""), modes(0), latency(0), valid(false) {}
    audioCodec(string codec, bitset<BITSET_8> supported, unsigned short latency) :
               name(codec), modes(supported), latency(latency), valid(true) { }
    audioCodec(string codec, bitset<BITSET_8> supported, unsigned short latency, bool valid) :
               name(codec), modes(supported), latency(latency), valid(valid) {}

    /* Get member functions */
    string getName() const { return name; }
    bitset<BITSET_8> getModes() const { return modes; }
    unsigned short getLatency() const { return latency; }
    bool getValid() const { return valid; }
    int getMaxMode() const
    {
        if (!valid)
            return -1;

        for (int i = (BITSET_8-1); i >= 0; i--) {
            if (modes[i]) {
                return i;
            }
        }
        return -1;
    }
    void getAudioMode(rtsp_wfd::lpcmModes mode, struct wfdAudioMode *pAudioMode);
    void getAudioMode(rtsp_wfd::aacModes mode, struct wfdAudioMode *pAudioMode);
    void getAudioMode(rtsp_wfd::eacModes mode, struct wfdAudioMode *pAudioMode);
    void getAudioMode(rtsp_wfd::ac3Modes mode, struct wfdAudioMode *pAudioMode);

    void setModes(bitset<BITSET_8> m) { modes = m; }
    void resetModes() { modes.reset(); }
    void setModesBit(unsigned b)
    {
        assert(b < BITSET_8);
        modes.set(b);
    }
    void setLatency(unsigned short l) { latency = l; }
    void setValid(bool v) { valid = v; }
    void setName(string n) { name = n; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, audioCodec&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, audioCodec&);
    audioCodec operator&(const audioCodec&);

private:
    string name;
    bitset<BITSET_8> modes;
    unsigned short latency;
    bool valid;
};

/*
 * wfd_video_formats: H.264 Codec
 */
class h264Codec {
public:
    h264Codec() : profile(0), level(0), maxHres(0), maxVres(0), ceaSupp(0), vesaSupp(0), hhSupp(0), latency(0), minSliceSize(0), sliceEnc(0), frameRateCtlSupp(0), valid(false), name("") {}
    h264Codec(bitset<BITSET_2> profile,
              bitset<BITSET_2> level,
              unsigned short maxHres,
              unsigned short maxVres,
              bitset<BITSET_8> ceaSupp,
              bitset<BITSET_8> vesaSupp,
              bitset<BITSET_8> hhSupp,
              unsigned short latency,
              unsigned short minSliceSize,
              bitset<BITSET_2> sliceEnc,
              bitset<BITSET_2> frameRateCtlSupp) :
	          profile(profile), level(level), maxHres(maxHres), maxVres(maxVres),
              ceaSupp(ceaSupp), vesaSupp(vesaSupp), hhSupp(hhSupp),
              latency(latency), minSliceSize(minSliceSize),
              sliceEnc(sliceEnc), frameRateCtlSupp(frameRateCtlSupp), valid(true), name("")
    {}
    h264Codec(bitset<BITSET_2> profile,
              bitset<BITSET_2> level,
              unsigned short maxHres,
              unsigned short maxVres,
              bitset<BITSET_8> ceaSupp,
              bitset<BITSET_8> vesaSupp,
              bitset<BITSET_8> hhSupp,
              unsigned short latency,
              unsigned short minSliceSize,
              bitset<BITSET_2> sliceEnc,
              bitset<BITSET_2> frameRateCtlSupp,
              bool valid) :
	          profile(profile), level(level),  maxHres(maxHres), maxVres(maxVres),
              ceaSupp(ceaSupp), vesaSupp(vesaSupp), hhSupp(hhSupp), latency(latency),
              minSliceSize(minSliceSize), sliceEnc(sliceEnc), frameRateCtlSupp(frameRateCtlSupp),
              valid(valid), name("")
    {}

    /* Get member functions */
    rtsp_wfd::videoCodecProfile getProfile() const {
        if (profile[0])
            return rtsp_wfd::cbp;
        else if (profile[1])
            return rtsp_wfd::chp;
        else
            return rtsp_wfd::profileInvalid;
    }
    rtsp_wfd::videoCodecLevel getLevel() const {
        if (profile[0])
            return rtsp_wfd::h264_level_3_1;
        else if (profile[1])
            return rtsp_wfd::h264_level_3_2;
        else if (profile[2])
            return rtsp_wfd::h264_level_4;
        else if (profile[3])
            return rtsp_wfd::h264_level_4_1;
        else if (profile[4])
            return rtsp_wfd::h264_level_4_2;
        else
            return rtsp_wfd::h264_level_invalid;
    }
    unsigned short getMaxHres() const { return maxHres; }
    unsigned short getMaxVres() const { return maxVres; }
    bitset<BITSET_8> getCeaSupp() const { return ceaSupp; }
    bitset<BITSET_8> getVesaSupp() const { return vesaSupp; }
    bitset<BITSET_8> getHhSupp() const { return hhSupp; }
    unsigned short getLatency() const { return latency; }
    unsigned short getMinSliceSize() const { return minSliceSize; }
    bitset<BITSET_2> getSliceEnc() const { return sliceEnc; }
    bitset<BITSET_2> getFrameCtlSupp() const { return frameRateCtlSupp; }
    bool getValid() const { return valid; }
    int getMaxCeaMode() const
    {
        if (!valid)
            return -1;

        for (int i = (BITSET_8-1); i >= 0; i--) {
            if (ceaSupp[i]) {
                return i;
            }
        }
        return -1;
    }
    int getMaxVesaMode() const
    {
        if (!valid)
            return -1;

        for (int i = (BITSET_8-1); i >= 0; i--) {
            if (vesaSupp[i]) {
                return i;
            }
        }
        return -1;
    }
    int getMaxHhMode() const
    {
        if (!valid)
            return -1;

        for (int i = (BITSET_8-1); i >= 0; i--) {
            if (hhSupp[i]) {
                return i;
            }
        }
        return -1;
    }

    void setProfile(rtsp_wfd::videoCodecProfile p)
    {
        assert(p < rtsp_wfd::profileInvalid);
        profile.reset();
        profile.set(p);
    }
    void setLevel(rtsp_wfd::videoCodecLevel l)
    {
        assert(l < rtsp_wfd::h264_level_invalid);
        level.reset();
        level.set(l);
    }
    void setMaxHres(unsigned short h) { maxHres = h; }
    void setMaxVres(unsigned short v) { maxVres = v; }
    void setCeaSupp(bitset<BITSET_8> c) { ceaSupp = c; }
    void resetCea() { ceaSupp.reset(); }
    void setCeaBit(unsigned b)
    {
        assert(b < BITSET_8);
        ceaSupp.set(b);
    }
    void setVesaSupp(bitset<BITSET_8> v) { vesaSupp = v; }
    void resetVesa() { vesaSupp.reset(); }
    void setVesaBit(unsigned b)
    {
        assert(b < BITSET_8);
        vesaSupp.set(b);
    }
    void setHhSupp(bitset<BITSET_8> h) { hhSupp = h; }
    void resetHh() { hhSupp.reset(); }
    void setHhBit(unsigned b)
    {
        assert(b < BITSET_8);
        hhSupp.set(b);
    }
    void setLatency(unsigned short l) { latency = l; }
    void setMinSliceSize(unsigned short s) { minSliceSize = s; }
    void setSliceEnc(unsigned maxNumSlices, unsigned ratioMaxSliceMinSize)  {
        bitset<BITSET_2> val(maxNumSlices | (ratioMaxSliceMinSize << 10));
        sliceEnc = val;
    }
    void setFrameCtlSupp(bitset<BITSET_2> f) { frameRateCtlSupp = f; }
    void setValid(bool v) { valid = v; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, h264Codec&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, h264Codec&);
    h264Codec operator&(const h264Codec&);

private:
    bitset<BITSET_2> profile;
    bitset<BITSET_2> level;
    unsigned short maxHres;
    unsigned short maxVres;
    bitset<BITSET_8> ceaSupp;
    bitset<BITSET_8> vesaSupp;
    bitset<BITSET_8> hhSupp;
    unsigned short latency;
    unsigned short minSliceSize;
    bitset<BITSET_2> sliceEnc;
    bitset<BITSET_2> frameRateCtlSupp;
    bool valid;
    string name;
};

struct wfdDisplayMode {
    unsigned short int width;
    unsigned short int height;
    unsigned short int frameRate;
    bool               progressive;       // true = progressive, false = interlaced
};

/*
 * wfd_video_formats: header
 */
class videoCodec {
public:
    videoCodec() : native(0), prefDispSupp(0), valid(false), name("VideoHeader") {}
    videoCodec(bitset<BITSET_2> native,
               bitset<BITSET_2> pref) :
	           native(native), prefDispSupp(pref), valid(true), name("VideoHeader") {}
    videoCodec(bitset<BITSET_2> native,
               bitset<BITSET_2> pref,
               bool valid) :
	           native(native), prefDispSupp(pref), valid(valid), name("VideoHeader") {}

    bitset<BITSET_2> getNative() const { return native; }
    bitset<BITSET_2> getPrefDispSupp() const { return prefDispSupp; }
    bool getValid() const { return valid; }
    rtsp_wfd::videoDisplay getNativeDisplay() const
    {
        if (native[0])
            return rtsp_wfd::cea;
        else if (native[1])
            return rtsp_wfd::vesa;
        else if (native[2])
            return rtsp_wfd::hh;
        else
            return rtsp_wfd::displayInvalid;
    }
    unsigned long getNativeRes() const
    {
        unsigned long res = native.to_ulong();

        res >>= 3;

        return res;
    }

    void getDisplayMode(rtsp_wfd::ceaResolution mode, struct wfdDisplayMode *pDisplayMode);
    void getDisplayMode(rtsp_wfd::vesaResolution mode, struct wfdDisplayMode *pDisplayMode);
    void getDisplayMode(rtsp_wfd::hhResolution mode, struct wfdDisplayMode *pDisplayMode);

    void setPrefDispSupp(bitset<BITSET_2> d) { prefDispSupp = d; }
    void setNative(rtsp_wfd::videoDisplay display, unsigned offset) {
        assert(display < rtsp_wfd::displayInvalid);
        offset <<= 3;
        bitset<BITSET_2> val(offset);
        val.set(display);
        native = val;
    }
    void setValid(bool v) { valid = v; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, videoCodec&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, videoCodec&);
    videoCodec operator&(const videoCodec&);

private:
    bitset<BITSET_2> native;
    bitset<BITSET_2> prefDispSupp;
    bool valid;
    string name;
};

/*
 * wfd_presentation_URL
 */
class presentationUrl {
public:
    presentationUrl() : ipAddr0(""), ipAddr1(""), valid(false), name("PresentationUrl") {}
    presentationUrl(string ip0) : ipAddr0(ip0), ipAddr1(""), valid(true), name("PresentationUrl") {}
    presentationUrl(string ip0, string ip1) : ipAddr0(ip0), ipAddr1(ip1), valid(true), name("PresentationUrl") {}

    string getIpAddr0() const { return ipAddr0; }
    string getIpAddr1() const { return ipAddr1; }
    bool getValid() const { return valid; }
    string getUri0() const
    {
        string uri0;

        if (ipAddr0.length()) {
            uri0 += "rtsp://";
            uri0 += ipAddr0;
            uri0 += "/wfd1.0/streamid=0";
        } else {
            uri0 = none;
        }
        return uri0;
    }

    string getUri1() const
    {
        string uri1;

        if (ipAddr1.length()) {
            uri1 += "rtsp://";
            uri1 += ipAddr1;
            uri1 += "/wfd1.0/streamid=1";
        } else {
            uri1 = none;
        }
        return uri1;
    }

    void setIpAddr0(string addr) { ipAddr0 = addr; valid = true; }
    void setIpAddr1(string addr) { ipAddr1 = addr; valid = true; }
    void setValid(bool v) { valid = v; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, presentationUrl&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, presentationUrl&);

private:
    string ipAddr0;
    string ipAddr1;
    bool valid;
    string name;
};

/*
 * wfd_client_rtp_ports
 */
class clientPorts {
public:
    clientPorts() : profile(""), rtpPort0(0), rtpPort1(0), rtcpPort0(0), valid(false), renegotiated(false), tcp(false), name("ClientRtpPorts")   {}
    clientPorts(unsigned port0, unsigned port1) : profile(""), rtpPort0(port0), rtpPort1(port1), rtcpPort0(0), valid(true), renegotiated(false), tcp (false), name("ClientRtpPorts") {}

    unsigned getRtpPort0() const { return rtpPort0; }
    unsigned getRtpPort1() const { return rtpPort1; }
    unsigned getRtcpPort0() const { return rtcpPort0; }
    void setRtcpPort0(unsigned port) {rtcpPort0 = port; valid = true;}

    bool getValid() const { return valid; }

    void setRtpPort0(unsigned port) { rtpPort0 = port; valid = true; }
    void setRtpPort1(unsigned port) { rtpPort1 = port; valid = true; }
    void setValid(bool v) { valid = v; }
    void setTCP(bool v) {tcp = v;}
    bool getTCP() {return tcp;}
    friend RTSPStringStream &operator>>(RTSPStringStream &stream, clientPorts&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, clientPorts&);
    bool getRenegotiated(){return renegotiated;}
    void setRenegotiated(bool yes){renegotiated = yes;}

private:
    string profile;
    unsigned rtpPort0;
    unsigned rtpPort1;
    unsigned rtcpPort0;
    bool valid;
    bool renegotiated;
    bool tcp;
    string name;
};

/*
 * wfd_server_rtp_ports
 */
class serverPorts {
public:
    serverPorts() : rtpPort0(0), rtpPort1(0), rtcpPort0(0), valid(false), name("ServerRtpPorts") {}
    serverPorts(unsigned port0, unsigned port1) : rtpPort0(port0), rtpPort1(port1), rtcpPort0(0), valid(true),  name("ServerRtpPorts") {}

    unsigned getRtpPort0() const { return rtpPort0; }
    unsigned getRtpPort1() const { return rtpPort1; }
    unsigned getRtcpPort0() const { return rtcpPort0; }

    bool getValid() const { return valid; }

    void setRtpPort0(unsigned port) { rtpPort0 = port; valid = true; }
    void setRtpPort1(unsigned port) { rtpPort1 = port; valid = true; }
    void setRtcpPort0(unsigned port) {rtcpPort0 = port; valid = true; }

    void setValid(bool v) { valid = v; }
    friend RTSPStringStream &operator>>(RTSPStringStream &stream, clientPorts&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, clientPorts&);

private:
    unsigned rtpPort0;
    unsigned rtpPort1;
    unsigned rtcpPort0;
    bool valid;
    string name;
};


/*
 * wfd_trigger_method
 */
class triggerParam {
public:
    triggerParam() : command(""), valid(false) {}

    bool getValid() { return valid; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, triggerParam&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, triggerParam&);

private:
    string command;
    bool valid;
};

/*
 * wfd_uibc_capability
 */
class uibcCapability {
public:
    uibcCapability() : category(0), genericCaps(0), hidcCaps(0), port(0), valid(false)
    {
        memset(hidcMap, 0, sizeof(hidcMap));
    }
    uibcCapability(bitset<BITSET_2> c, bitset<BITSET_2> g, bitset<BITSET_2> h, rtsp_wfd::uibcPath m[], unsigned p, bool valid) :
                   category(c), genericCaps(g), hidcCaps(h), port(p), valid(true)
    {
        UNUSED(valid);
        memcpy(hidcMap, m, sizeof(hidcMap));
    }

    bool getValid() const { return valid; }
    bitset<BITSET_2> getCategory() const { return category; }
    rtsp_wfd::uibcPath getHidcMap(rtsp_wfd::uibcInput i) const { return hidcMap[i]; }
    bitset<BITSET_2> getGenericCap() const { return genericCaps; }
    bitset<BITSET_2> getHidcCap() const { return hidcCaps; }
    unsigned getPort() const { return port; }

    void setValid(bool v) { valid = v; }
    void setCategory(rtsp_wfd::uibcCategory c) { category.set(c); valid = true; }
    void unsetCategory(rtsp_wfd::uibcCategory c) { category[c] = 0; }
    void setHidcMap(rtsp_wfd::uibcInput i, rtsp_wfd::uibcPath p) { hidcCaps.set(i); hidcMap[i] = p; valid = true; }
    void setGenericCap(rtsp_wfd::uibcInput i) { genericCaps.set(i); valid = true; }
    void setHidcCap(rtsp_wfd::uibcInput i) { hidcCaps.set(i); valid = true; }
    void setPort(unsigned p) { port = p; valid = true; }

    void resetHidcMap() { memset(hidcMap, 0, sizeof(hidcMap)); }
    void resetGenericCap() { genericCaps = 0; }
    void resetHidcCap() { hidcCaps = 0; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, uibcCapability&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, uibcCapability&);
    uibcCapability operator&(const uibcCapability&);
    //Need to make deep copies of HidcMap hence can't rely on simple assignment
    uibcCapability& operator=(const uibcCapability&);

private:
    bitset<BITSET_2> category;
    bitset<BITSET_2> genericCaps;
    bitset<BITSET_2> hidcCaps;
    rtsp_wfd::uibcPath hidcMap[UIBC_MAX_INPUT_SIZE];
    unsigned port;
    bool valid;
};

/*
 * wfd_uibc_setting
 */
class uibcSetting {
public:
    uibcSetting() : valid(false), setting(false) {}
    uibcSetting(bool valid, bool setting) : valid(valid), setting(setting) {}

    bool getValid() const { return valid; }
    void setValid(bool v) { valid = v; }

    bool getSetting() const { return setting; }
    void setSetting(bool s) { setting = s; valid = true; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, uibcSetting&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, uibcSetting&);
    uibcSetting operator&(const uibcSetting&);

private:
    bool valid;
    bool setting;
};

/*
 * wfd_standby_resume_capability
 */
class standby_resume_cap{
public:
    standby_resume_cap() : valid(false), setting(false){}
    standby_resume_cap(bool valid, bool setting) : valid(valid), setting(setting){}

    bool getValid() const { return valid; }
    void setValid(bool v) { valid = v; }
    bool getSetting() const { return setting; }
    void setSetting(bool s) { setting = s; valid = true; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, standby_resume_cap& );
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, standby_resume_cap& );
    standby_resume_cap operator&(const standby_resume_cap&);

private:
    bool valid;
    bool setting;
};

/*
 * wfd_standby
 */
class standby {
public:
    standby() : valid(false) {}
    standby(bool valid) : valid(valid) {}

    bool getValid() const { return valid; }
    void setValid(bool v) { valid = v; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, standby&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, standby&);

private:
    bool valid;
};

/*
 * wfd_resume
 */
class resume {
public:
    resume() : valid(false) {}
    resume(bool valid) : valid(valid) {}

    bool getValid() const { return valid; }
    void setValid(bool v) { valid = v; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, resume&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, resume&);

private:
    bool valid;
};

/*
 * wfd_coupled_sink
 */
class coupled {
public:
    coupled() : status(0), macAddr(""), valid(false) {}
    coupled(bitset<BITSET_2> mode, string mac) : status(mode), macAddr(mac), valid(true) {}

    bool getValid() const { return valid; }
    string getMacAddr() const { return macAddr; }
    bool isCoupled() const { return (status[COUPLED] && (macAddr.length() == MAC_ADDR_LEN)) ? true : false; }

    void setValid(bool v) { valid = v; }
    void setMacAddr(string mac) { macAddr = mac; status.set(COUPLED); valid = true; }
    void setCoupled() { status.set(COUPLED); }
    void setUnCoupled() { status[COUPLED] = 0; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, coupled&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, coupled&);

private:
    bitset<BITSET_2> status;
    string macAddr;
    bool valid;
};

/*
 * wfd_content_protection
 */

class hdcp_cp{
public:
  hdcp_cp() :ctrl_port(0), valid(false), version(-1) {}
  hdcp_cp(unsigned hdcp_port): ctrl_port(hdcp_port), valid(true), version(-1) {}

  bool getValid() const {return valid;}
  unsigned getCtrlPort() const {return ctrl_port;}
  int getVersion() const { return version; }

  void setValid(bool v) { valid = v; }
  void setCtrlPort(unsigned port) { ctrl_port = port; valid = true; }
  void setVersion(int ver) { version = ver; valid = true; }

  friend RTSPStringStream &operator>>(RTSPStringStream &stream, hdcp_cp&);
  friend RTSPStringStream &operator<<(RTSPStringStream &stream, hdcp_cp&);

private:
  unsigned ctrl_port;
  bool valid;
  int version;

};
/*
 * wfd_route
 */
class route {
public:
    route() : valid(false), destination(0) {}
    route(bool valid, int dest) : valid(valid), destination(dest) {}

    bool getValid() const { return valid; }
    void setValid(bool v) { valid = v; }

    unsigned getDestination() const { return destination; }
    void setSetting(int d) { destination = d; valid = true; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, route&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, route&);

private:
    bool valid;
    unsigned destination;
};

/*
 * wfd_av_format_change_timing
 */
class timingChange {
public:
    timingChange() : valid(false), pts(0), dts(0) {}
    timingChange(bool valid, unsigned long long p, unsigned long long d) : valid(valid), pts(p), dts(d) {}

    bool getValid() const { return valid; }
    void setValid(bool v) { valid = v; }

    unsigned long long getPts() const { return pts; }
    void setPts(unsigned long long p) { pts = p; valid = true; }

    unsigned long long getDts() const { return dts; }
    void setDts(unsigned long long d) { dts = d; valid = true; }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, timingChange&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, timingChange&);

private:
    bool valid;
    unsigned long long pts;
    unsigned long long dts;
};

/*
 * wfd_display_edid
 */
class displayEdid {
public:
    displayEdid() : valid(false), blockCount(0), payload(NULL) {}
    displayEdid(bool valid, unsigned count, unsigned char *data) : valid(valid)
    {
        if (count)
            setPayload(count, data);
    }
    displayEdid(const displayEdid &copy)
    {
        if (copy.blockCount) {
            setPayload(copy.blockCount, copy.payload);
        } else {
            blockCount = 0;
            payload = NULL;
            valid = copy.valid;
        }
    }
    displayEdid& operator=(const displayEdid &copy)
    {
        if (copy.blockCount) {
            setPayload(copy.blockCount, copy.payload);
        } else {
            blockCount = 0;
            payload = NULL;
            valid = copy.valid;
        }
        return *this;
    }
    ~displayEdid() { if (payload) { RTSP_FREEIF(payload); } }

    bool getValid() const { return valid; }
    void setValid(bool v) { valid = v; }

    unsigned char* getPayload(unsigned &count) { count = blockCount; return payload; }
    void setPayload(unsigned count, const unsigned char *data)
    {
        blockCount = count;

        if (blockCount > EDID_MAX_BLOCKS) {
            blockCount = EDID_MAX_BLOCKS;
        }

        if (blockCount) {
            payload = (unsigned char *)MM_Malloc(sizeof(unsigned char)*((blockCount * EDID_BLOCK_SIZE) + 1));
        if(payload == NULL)
        {
            //MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTSP_LIB :: payload memory allocation failed");
            return;
        }
            memcpy(payload, data, blockCount * EDID_BLOCK_SIZE);
            payload[blockCount * EDID_BLOCK_SIZE] = '\0';
        }

        valid = true;
    }

    friend RTSPStringStream &operator>>(RTSPStringStream &stream, displayEdid&);
    friend RTSPStringStream &operator<<(RTSPStringStream &stream, displayEdid&);

private:
    bool valid;
    unsigned blockCount;
    unsigned char *payload;
};

/*
 * wfd_vnd_sec_max_buffer_length
 */
class buffer_len {
public:
    buffer_len() : valid(false), bufferLen(0) {}
    buffer_len(bool valid, int len) : valid(valid) , bufferLen(len) {}

    bool getValid() const { return valid; }
    void setValid(bool v) { valid = v; }
    void setBufferLen(int len) { bufferLen = len; valid = true;}
    int getBufferLen() { return bufferLen; }

private:
    bool valid;
    int  bufferLen;
};

/*
 * wfd_vnd_sec_tcp_window_size
 */
class tcp_window_size {
public:
    tcp_window_size() : valid(false) , windowSize(0) {}
    tcp_window_size(bool valid, int size) : valid(valid) , windowSize(size) {}

    bool getValid() const { return valid; }
    void setValid(bool v) { valid = v; }
    void setWindowSize(int size) { windowSize = size; valid = true;}
    int getWindowSize() { return windowSize; }

private:
    bool valid;
    int windowSize;
};

class tcp_stream_control
{
public:
    tcp_stream_control() : valid(false), command(rtsp_wfd::cmdNone),timestamp(0){}
    tcp_stream_control(bool valid, rtsp_wfd::tcp_control_command c) : valid(valid), command(c), timestamp(0) {}

    bool getValid() const {return valid;}
    void setValid(bool v) {valid = v;}
    void setCommand(rtsp_wfd::tcp_control_command c) {command = c; valid = true;}
    void setDuration(unsigned long long duration){timestamp = duration;}
    unsigned long long getDuration(){return timestamp;}
    rtsp_wfd::tcp_control_command getCommand() {return command;}

private:
    bool valid;
    rtsp_wfd::tcp_control_command command;
    unsigned long long timestamp;
};

/*
 * wfd_idr_request
 */
class idrRequest {
public:
    idrRequest() : valid(false) {}
    idrRequest(bool valid) : valid(valid) {}

    bool getValid() const { return valid; }
    void setValid(bool v) { valid = v; }

private:
    bool valid;
};

/* Forward declarations needed for rtspWfd class below */
class rtspSession;
class rtspParams;
class getParamCommand;
class setParamCommand;

/*
 * Contains classes for all currently supported WFD parameters and
 * related data
 */
class rtspWfd {
public:
    rtspWfd()
    {
        audioLpcm.setName("LPCM");
        audioAac.setName("AAC");
        audioEac.setName("E-AC");
        audioAc3.setName("AC3");
        audioDts.setName("DTS");
        h264Cbp.setProfile(rtsp_wfd::cbp);
        h264Chp.setProfile(rtsp_wfd::chp);
        h264Chi444p.setProfile(rtsp_wfd::chi444p);
        isPrimarySink = isSecondarySink = false;
    }

    rtspWfd(audioCodec lpcm,
            audioCodec aac,
            audioCodec eac,
            audioCodec ac3,
            audioCodec dts,
            videoCodec video,
            h264Codec cbp,
            h264Codec chp,
            h264Codec chi444p,
            presentationUrl uri,
            clientPorts client,
            uibcCapability cap,
            uibcSetting set,
            standby_resume_cap SRCap,
            coupled couple,
            hdcp_cp cp,
            route rte,
            timingChange tmg,
            displayEdid edid,
            idrRequest idr) :
            audioLpcm(lpcm), audioAac(aac), audioEac(eac), audioAc3(ac3), audioDts(dts),
            h264Cbp(cbp), h264Chp(chp), h264Chi444p(chi444p), uri(uri),
            client(client), uibcCap(cap), uibcSet(set),standbyCap(SRCap),
            coupledSink(couple),audioRoute(rte), timing(tmg), edid(edid),
            idrReq(idr),contentProtection(cp), isPrimarySink(false),
            isSecondarySink(false) {UNUSED(video);}

    void init(string);
    rtspWfdParams wfdType(string, bool&);
    void wfdParse(rtspWfdParams, string);
    //void makeXml(string);
    void calcWorkingSet();
    void assign(const rtspWfd&);

    void parseXml(string);
    void parseAudioXml(string, rtsp_wfd::audioCodecName);
    void parseVideoHeaderXml(string);
    void parseCodecXml(string, rtsp_wfd::videoCodecProfile);
    void parseUibcXml(string, rtspWfdParams);
    void parseCoupling(string);
    void parseStandbyXml(string);
    void parseContentProtection(string);

    rtspWfd& operator&(const rtspWfd&);
    rtspWfd& operator+=(const rtspWfd&);
    bool operator==(const rtspWfd&);
    bool operator!=(const rtspWfd &theirs)
    {
        return !(*this == theirs);
    }

    void dump();

    /*
     * Return a bitset of all valid members
     */
    bitset<WFD_MAX_SIZE> getValidWfd() const
    {
        bitset<WFD_MAX_SIZE> wfdValid;

        if (audioLpcm.getValid() ||
            audioAac.getValid() ||
            audioEac.getValid() ||
            audioAc3.getValid() ||
            audioDts.getValid()) {
            wfdValid.set(wfd_audio_codecs);
        }
        if (h264Cbp.getValid() ||
            h264Chp.getValid()) {
            wfdValid.set(wfd_video_formats);
        }
        if (uri.getValid()) {
            wfdValid.set(wfd_presentation_URL);
        }
        if (client.getValid()) {
            wfdValid.set(wfd_client_rtp_ports);
        }
        if (uibcCap.getValid()) {
            wfdValid.set(wfd_uibc_capability);
        }
        if (uibcSet.getValid()) {
            wfdValid.set(wfd_uibc_setting);
        }
        if (standbyCap.getValid()) {
            wfdValid.set(wfd_standby_resume_capability);
        }
        if (halt.getValid()) {
            wfdValid.set(wfd_standby);
        }
        if (start.getValid()) {
            wfdValid.set(wfd_resume);
        }
        if (coupledSink.getValid()) {
            wfdValid.set(wfd_coupled_sink);
        }
        if (audioRoute.getValid()) {
            wfdValid.set(wfd_route);
        }
        if (timing.getValid()) {
            wfdValid.set(wfd_av_format_change_timing);
        }
        if (edid.getValid()) {
            wfdValid.set(wfd_display_edid);
        }
        if (idrReq.getValid()) {
            wfdValid.set(wfd_idr_request);
        }
        if (contentProtection.getValid()) {
          wfdValid.set(wfd_content_protection);
        }
#ifdef LEGACY_TCP
        if (tcpWindowSize.getValid()) {
            wfdValid.set(wfd_vnd_sec_tcp_window_size);
        }

        if(buffLen.getValid()) {
            wfdValid.set(wfd_vnd_sec_max_buffer_length);
        }

        if(tcpStreamControl.getValid()) {
            wfdValid.set(wfd_vnd_sec_control_playback);
        }
#else
        if (tcpWindowSize.getValid()) {
            wfdValid.set(wd_initial_buffer);
        }

        if(buffLen.getValid()) {
            wfdValid.set(wd_decoder_latency);
        }

        if(tcpStreamControl.getValid()) {
            wfdValid.set(wd_playback_control);
        }
#endif
        /*if (isKeepAlive == true) {
            wfdValid.set(wfd_keepalive);
        }*/

        return wfdValid;
    }

    /*
     * Invalidate all members
     */
    void reset()
    {
        audioLpcm.setValid(false);
        audioAac.setValid(false);
        audioEac.setValid(false);
        audioAc3.setValid(false);
        audioDts.setValid(false);
        videoHeader.setValid(false);
        h264Cbp.setValid(false);
        h264Chp.setValid(false);
        h264Chi444p.setValid(false);
        uri.setValid(false);
        client.setValid(false);
        halt.setValid(false);
        start.setValid(false);
        uibcCap.setValid(false);
        uibcSet.setValid(false);
        standbyCap.setValid(false);
        coupledSink.setValid(false);
        audioRoute.setValid(false);
        timing.setValid(false);
        edid.setValid(false);
        idrReq.setValid(false);
        contentProtection.setValid(false);
        isPrimarySink = isSecondarySink = false;
        buffLen.setValid(false);
        tcpWindowSize.setValid(false);
        tcpStreamControl.setValid(false);
    }

    friend class rtspSession;
    friend void parseRecv(char **, size_t, rtspParams *);
    friend RTSPStringStream &operator<<(RTSPStringStream &, getParamCommand *);
    friend RTSPStringStream &operator<<(RTSPStringStream &, setParamCommand *);

    triggerParam trigger;
    audioCodec audioLpcm;
    audioCodec audioAac;
    audioCodec audioEac;
    audioCodec audioAc3;
    audioCodec audioDts;
    videoCodec videoHeader;
    h264Codec  h264Cbp;
    h264Codec  h264Chp;
    h264Codec  h264Chi444p;
    presentationUrl uri;
    clientPorts client;
    serverPorts server;
    uibcCapability uibcCap;
    uibcSetting uibcSet;
    standby_resume_cap standbyCap;
    standby halt;
    resume start;
    coupled coupledSink;
    route audioRoute;
    timingChange timing;
    displayEdid edid;
    idrRequest idrReq;
    hdcp_cp contentProtection;
    buffer_len buffLen;
    tcp_window_size tcpWindowSize;
    tcp_stream_control tcpStreamControl;

    bool isPrimarySink;
    bool isSecondarySink;

    int connectorType;
};

#endif /*_RTSP_WFD_H */
