/* ==============================================================================
 * WfdEnums.java
 *
 * Data structure for WFD capable device
 *
 * Copyright (c) 2012 - 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ============================================================================== */

package com.qualcomm.wfd;

public class WfdEnums {
    static final int BIT0       = 0x00000001;
    static final int BIT1       = 0x00000002;
    static final int BIT2       = 0x00000004;
    static final int BIT3       = 0x00000008;
    static final int BIT4       = 0x00000010;
    static final int BIT5       = 0x00000020;
    static final int BIT6       = 0x00000040;
    static final int BIT7       = 0x00000080;
    static final int BIT8       = 0x00000100;
    static final int BIT9       = 0x00000200;
    static final int BIT10      = 0x00000400;
    static final int BIT11      = 0x00000800;
    static final int BIT12      = 0x00001000;
    static final int BIT13      = 0x00002000;
    static final int BIT14      = 0x00004000;
    static final int BIT15      = 0x00008000;
    static final int BIT16      = 0x00010000;
    static final int BIT17      = 0x00020000;
    static final int BIT18      = 0x00040000;
    static final int BIT19      = 0x00080000;
    static final int BIT20      = 0x00100000;
    static final int BIT21      = 0x00200000;
    static final int BIT22      = 0x00400000;
    static final int BIT23      = 0x00800000;
    static final int BIT24      = 0x01000000;
    static final int BIT25      = 0x02000000;
    static final int BIT26      = 0x04000000;
    static final int BIT27      = 0x08000000;
    static final int BIT28      = 0x10000000;
    static final int BIT29      = 0x20000000;
    static final int BIT30      = 0x40000000;
    static final int BIT31      = 0x80000000;

    public static final String ACTION_WIFI_DISPLAY_ENABLED = "qualcomm.intent.action.WIFI_DISPLAY_ENABLED";

    public static final String ACTION_WIFI_DISPLAY_DISABLED = "qualcomm.intent.action.WIFI_DISPLAY_DISABLED";

    public static final String ACTION_WIFI_DISPLAY_RESOLUTION = "qualcomm.intent.action.WIFI_DISPLAY_RESOLUTION";

    public static final String ACTION_WIFI_DISPLAY_BITRATE = "qualcomm.intent.action.WIFI_DISPLAY_BITRATE";

    public static final String ACTION_WIFI_DISPLAY_RTP_TRANSPORT = "qualcomm.intent.action.WIFI_DISPLAY_RTP_TRANSPORT";

    public static final String ACTION_WIFI_DISPLAY_TCP_PLAYBACK_CONTROL = "qualcomm.intent.action.WIFI_DISPLAY_TCP_PLAYBACK_CONTROL";

    public static final String ACTION_WIFI_DISPLAY_PLAYBACK_MODE = "qualcomm.intent.action.WIFI_DISPLAY_PLAYBACK_MODE";

    public static final String ACTION_WIFI_DISPLAY_VIDEO = "org.codeaurora.intent.action.WIFI_DISPLAY_VIDEO";

    public static final String CONFIG_BUNDLE_KEY = "wfd_config";

    private static int resWidth = 0, resHeight = 0, resFps = 0;

    /* CEA Resolution/Refresh Rate */
    public static final int     H264_CEA_640x480p60         = BIT0;
    public static final int     H264_CEA_720x480p60         = BIT1;
    public static final int     H264_CEA_720x480i60         = BIT2;
    public static final int     H264_CEA_720x576p50         = BIT3;
    public static final int     H264_CEA_720x576i50         = BIT4;
    public static final int     H264_CEA_1280x720p30        = BIT5;
    public static final int     H264_CEA_1280x720p60        = BIT6;
    public static final int     H264_CEA_1920x1080p30       = BIT7;
    public static final int     H264_CEA_1920x1080p60       = BIT8;
    public static final int     H264_CEA_1920x1080i60       = BIT9;
    public static final int     H264_CEA_1280x720p25        = BIT10;
    public static final int     H264_CEA_1280x720p50        = BIT11;
    public static final int     H264_CEA_1920x1080p25       = BIT12;
    public static final int     H264_CEA_1920x1080p50       = BIT13;
    public static final int     H264_CEA_1920x1080i50       = BIT14;
    public static final int     H264_CEA_1280x720p24        = BIT15;
    public static final int     H264_CEA_1920x1080p24       = BIT16;

    private static void setResParams(int width, int height, int fps) {
        resWidth = width;
        resHeight = height;
        resFps = fps;
    }

    public static int[] getResParams() {
        int[] resParams = new int[3];
        resParams[0] = resWidth;
        resParams[1] = resHeight;
        resParams[2] = resFps;
        return resParams;
    }

    public static final boolean isCeaResolution(int resolution) {
        switch(resolution) {
            case H264_CEA_640x480p60: {
                setResParams(640, 480, 60);
            }
            break;
            case H264_CEA_720x480p60: {
                setResParams(720, 480, 60);
            }
            break;
            case H264_CEA_720x480i60: {
                setResParams(720, 480, 60);
            }
            break;
            case H264_CEA_720x576p50: {
                setResParams(720, 576, 50);
            }
            break;
            case H264_CEA_720x576i50:{
                setResParams(720, 576, 50);
            }
            break;
            case H264_CEA_1280x720p30:{
                setResParams(1280, 720, 30);
            }
            break;
            case H264_CEA_1280x720p60:{
                setResParams(1280, 720, 60);
            }
            break;
            case H264_CEA_1920x1080p30:{
                setResParams(1920, 1080, 30);
            }
            break;
            case H264_CEA_1920x1080p60:{
                setResParams(1920, 1080, 60);
            }
            break;
            case H264_CEA_1920x1080i60:{
                setResParams(1920, 1080, 60);
            }
            break;
            case H264_CEA_1280x720p25:{
                setResParams(1280, 720, 25);
            }
            break;
            case H264_CEA_1280x720p50:{
                setResParams(1280, 720, 50);
            }
            break;
            case H264_CEA_1920x1080p25:{
                setResParams(1920, 1080, 25);
            }
            break;
            case H264_CEA_1920x1080p50:{
                setResParams(1920, 1080, 50);
            }
            break;
            case H264_CEA_1920x1080i50:{
                setResParams(1920, 1080, 50);
            }
            break;
            case H264_CEA_1280x720p24:{
                setResParams(1280, 720, 24);
            }
            break;
            case H264_CEA_1920x1080p24:{
                setResParams(1920, 1080, 24);
            }
            break;
            default:
                return false;
        }
        return true;
    }

    /* VESA Resolution/Refresh Rate */
    public static final int     H264_VESA_800x600p30        = BIT0;
    public static final int     H264_VESA_800x600p60        = BIT1;
    public static final int     H264_VESA_1024x768p30       = BIT2;
    public static final int     H264_VESA_1024x768p60       = BIT3;
    public static final int     H264_VESA_1152x864p30       = BIT4;
    public static final int     H264_VESA_1152x864p60       = BIT5;
    public static final int     H264_VESA_1280x768p30       = BIT6;
    public static final int     H264_VESA_1280x768p60       = BIT7;
    public static final int     H264_VESA_1280x800p30       = BIT8;
    public static final int     H264_VESA_1280x800p60       = BIT9;
    public static final int     H264_VESA_1360x768p30       = BIT10;
    public static final int     H264_VESA_1360x768p60       = BIT11;
    public static final int     H264_VESA_1366x768p30       = BIT12;
    public static final int     H264_VESA_1366x768p60       = BIT13;
    public static final int     H264_VESA_1280x1024p30      = BIT14;
    public static final int     H264_VESA_1280x1024p60      = BIT15;
    public static final int     H264_VESA_1400x1050p30      = BIT16;
    public static final int     H264_VESA_1400x1050p60      = BIT17;
    public static final int     H264_VESA_1440x900p30       = BIT18;
    public static final int     H264_VESA_1440x900p60       = BIT19;
    public static final int     H264_VESA_1600x900p30       = BIT20;
    public static final int     H264_VESA_1600x900p60       = BIT21;
    public static final int     H264_VESA_1600x1200p30      = BIT22;
    public static final int     H264_VESA_1600x1200p60      = BIT23;
    public static final int     H264_VESA_1680x1024p30      = BIT24;
    public static final int     H264_VESA_1680x1024p60      = BIT25;
    public static final int     H264_VESA_1680x1050p30      = BIT26;
    public static final int     H264_VESA_1680x1050p60      = BIT27;
    public static final int     H264_VESA_1920x1200p30      = BIT28;
    public static final int     H264_VESA_1920x1200p60      = BIT29;

    public static final boolean isVesaResolution(int resolution) {
        switch(resolution) {
            case H264_VESA_800x600p30: {
                setResParams(800, 600, 30);
            }
            break;
            case H264_VESA_800x600p60: {
                setResParams(800, 600, 60);
            }
            break;
            case H264_VESA_1024x768p30: {
                setResParams(1024, 768, 30);
            }
            break;
            case H264_VESA_1024x768p60: {
                setResParams(1024, 768, 60);
            }
            break;
            case H264_VESA_1152x864p30: {
                setResParams(1152, 864, 30);
            }
            break;
            case H264_VESA_1152x864p60: {
                setResParams(1152, 864, 60);
            }
            break;
            case H264_VESA_1280x768p30: {
                setResParams(1280, 768, 30);
            }
            break;
            case H264_VESA_1280x768p60: {
                setResParams(1280, 768, 60);
            }
            break;
            case H264_VESA_1280x800p30: {
                setResParams(1280, 800, 30);
            }
            break;
            case H264_VESA_1280x800p60: {
                setResParams(1280, 800, 60);
            }
            break;
            case H264_VESA_1360x768p30: {
                setResParams(1360, 768, 30);
            }
            break;
            case H264_VESA_1360x768p60: {
                setResParams(1360, 768, 60);
            }
            break;
            case H264_VESA_1366x768p30: {
                setResParams(1366, 768, 30);
            }
            break;
            case H264_VESA_1366x768p60: {
                setResParams(1366, 768, 60);
            }
            break;
            case H264_VESA_1280x1024p30: {
                setResParams(1280, 1024, 30);
            }
            break;
            case H264_VESA_1280x1024p60: {
                setResParams(1280, 1024, 60);
            }
            break;
            case H264_VESA_1400x1050p30: {
                setResParams(1400, 1050, 30);
            }
            break;
            case H264_VESA_1400x1050p60: {
                setResParams(1400, 1050, 60);
            }
            break;
            case H264_VESA_1440x900p30: {
                setResParams(1440, 900, 30);
            }
            break;
            case H264_VESA_1440x900p60: {
                setResParams(1440, 900, 60);
            }
            break;
            case H264_VESA_1600x900p30: {
                setResParams(1600, 900, 30);
            }
            break;
            case H264_VESA_1600x900p60: {
                setResParams(1600, 900, 60);
            }
            break;
            case H264_VESA_1600x1200p30: {
                setResParams(1600, 1200, 30);
            }
            break;
            case H264_VESA_1600x1200p60: {
                setResParams(1600, 1200, 60);
            }
            break;
            case H264_VESA_1680x1024p30: {
                setResParams(1680, 1024, 30);
            }
            break;
            case H264_VESA_1680x1024p60: {
                setResParams(1680, 1024, 60);
            }
            break;
            case H264_VESA_1680x1050p30: {
                setResParams(1680, 1050, 30);
            }
            break;
            case H264_VESA_1680x1050p60: {
                setResParams(1680, 1050, 60);
            }
            break;
            case H264_VESA_1920x1200p30: {
                setResParams(1920, 1200, 30);
            }
            break;
            case H264_VESA_1920x1200p60: {
                setResParams(1920, 1200, 60);
            }
            break;
            default:
                return false;
        }
        return true;
    }

    /* HH Resolution/Refresh Rate */
    public static final int     H264_HH_800x480p30          = BIT0;
    public static final int     H264_HH_800x480p60          = BIT1;
    public static final int     H264_HH_854x480p30          = BIT2; //not supported
    public static final int     H264_HH_854x480p60          = BIT3; //not supported
    public static final int     H264_HH_864x480p30          = BIT4;
    public static final int     H264_HH_864x480p60          = BIT5;
    public static final int     H264_HH_640x360p30          = BIT6;
    public static final int     H264_HH_640x360p60          = BIT7;
    public static final int     H264_HH_960x540p30          = BIT8;
    public static final int     H264_HH_848x480p30          = BIT10;
    public static final int     H264_HH_848x480p60          = BIT11;

    public static final boolean isHhResolution(int resolution) {
        switch(resolution) {
            case H264_HH_800x480p30: {
                setResParams(800, 480, 30);
            }
            break;
            case H264_HH_800x480p60: {
                setResParams(800, 480, 60);
            }
            break;
            case H264_HH_864x480p30: {
                setResParams(864, 480, 30);
            }
            break;
            case H264_HH_864x480p60: {
                setResParams(864, 480, 60);
            }
            break;
            case H264_HH_640x360p30: {
                setResParams(640, 360, 30);
            }
            break;
            case H264_HH_640x360p60: {
                setResParams(640, 360, 60);
            }
            break;
            case H264_HH_960x540p30: {
                setResParams(960, 540, 30);
            }
            break;
            case H264_HH_848x480p30: {
                setResParams(848, 480, 30);
            }
            break;
            case H264_HH_848x480p60: {
                setResParams(848, 480, 60);
            }
            break;
            default:
                return false;
        }
        return true;
    }

    //Should match with DeviceType enum in Device.h
    public static enum WFDDeviceType {
        SOURCE(0), PRIMARY_SINK(1), SECONDARY_SINK(2), SOURCE_PRIMARY_SINK(3), UNKNOWN(4);

        private final int code;

        private WFDDeviceType(int c) {
            code = c;
        }

        public int getCode() {
            return code;
        }

        public static WFDDeviceType getValue(int c) {
            for (WFDDeviceType e:values()) {
                if (e.code == c)
                    return e;
            }
            return null;
        }

    }

    //Enum name string is used as key in WfdDevice.capabilities
    //Order should match with the CapabilityType enum in WFDDefs.h
    public static enum CapabilityType {
        WFD_AUDIO_CODECS, // values from AudioCodec
        WFD_VIDEO_FORMATS, // values from VideoFormat
        WFD_3D_VIDEO_FORMATS, // values from VideoFormat
        WFD_CEA_RESOLUTIONS_BITMAP, // values from H264_CEA_* constants
        WFD_VESA_RESOLUTIONS_BITMAP, // values from H264_VESA_* constants
        WFD_HH_RESOLUTIONS_BITMAP, // values form H264_HH_* constants
        WFD_DISPLAY_EDID,
        WFD_COUPLED_SINK,
        WFD_I2C,
        WFD_UIBC_SUPPORTED, // boolean
        WFD_STANDBY_RESUME_CAPABILITY, // boolean
        WFD_COUPLED_SINK_SUPPORTED_BY_SOURCE, // boolean
        WFD_COUPLED_SINK_SUPPORTED_BY_SINK, // boolean
        WFD_SERVICE_DISCOVERY_SUPPORTED, // boolean
        WFD_CONTENT_PROTECTION_SUPPORTED, // boolean
        WFD_TIME_SYNC_SUPPORTED, // boolean
    }

    // Order should match with AVPlaybackModeType enum in WFDDefs.h
    public static enum AVPlaybackMode {
        NO_AUDIO_VIDEO,
        AUDIO_ONLY,
        VIDEO_ONLY,
        AUDIO_VIDEO
    }

    public static enum AudioCodec {
        WFD_AUDIO_UNK, /* unknown audio */
        WFD_AUDIO_LPCM, /* LPCM audio */
        WFD_AUDIO_AAC, /* AAC audio */
        WFD_AUDIO_DOLBY_DIGITAL, /* Dolby Digital audio (AC3) */
        WFD_AUDIO_INVALID
    }

    public static enum VideoFormat {
        WFD_VIDEO_UNK, /* unknown video codec type */
        WFD_VIDEO_H264, /* H264 video */
        WFD_VIDEO_3D, /* 3D video */
        WFD_VIDEO_INVALID
    }

    public static enum SessionState {
        INVALID,
        INITIALIZED, /* Session is initalized and ready to connect */
        IDLE, /* Ready to PLAY */
        PLAY,
        PAUSE,
        ESTABLISHED,
        TEARDOWN,
        PLAYING,
        PAUSING,
        STANDBY,
        STANDING_BY,
        TEARING_DOWN
    }

    public static enum WfdEvent {
        WFD_SERVICE_ENABLED,
        WFD_SERVICE_DISABLED,
        UIBC_ENABLED,
        UIBC_DISABLED,
        PLAY_START,
        PAUSE_START,
        AUDIOPROXY_CLOSED,
        AUDIOPROXY_OPENED,
        TEARDOWN_START,
        HDCP_CONNECT_SUCCESS,
        HDCP_CONNECT_FAIL,
        HDCP_ENFORCE_FAIL,
        VIDEO_RUNTIME_ERROR,
        VIDEO_CONFIGURE_FAILURE,
        AUDIO_RUNTIME_ERROR,
        HDCP_RUNTIME_ERROR,
        AUDIO_CONFIGURE_FAILURE,
        NETWORK_RUNTIME_ERROR,
        NETWORK_CONFIGURE_FAILURE,
        RTP_TRANSPORT_NEGOTIATED,
        STANDBY_START,
        START_SESSION_FAIL,
        TCP_PLAYBACK_CONTROL,
        AUDIO_ONLY_SESSION
    }

    public static enum PreferredConnectivity {
        P2P, TDLS
    }

    public static enum RtpTransportType {
        UDP, TCP
    }

    public static enum ControlCmdType {
        FLUSH,
        PLAY,
        PAUSE,
        STATUS
    }

    //Should match with WFD_runtime_cmd_t enum in wdsm_mm_interface.h
    public enum RuntimecmdType {
        UNKNOWN(-1),
        /**
         * This command is a no-op in native code
         */
        @Deprecated
        AUDIOPROXY_OPEN(0),
        /**
         * This command is a no-op in native code
         */
        @Deprecated
        AUDIOPROXY_CLOSE(1),
        ENABLE_BITRATE_ADAPT(2),
        DISABLE_BITRATE_ADAPT(3);

        private final int code;

        private RuntimecmdType(int c) {
            code = c;
        }
        public int getCmdType() {
            return code;
        }
    }

    public static enum CoupledSinkStatus {
        NOT_COUPLED(0),
        COUPLED(1),
        TEARDOWN_COUPLING(2);

        private int code;

        private CoupledSinkStatus(int c) {
            code = c;
        }

        public int getCode() {
            return code;
        }
    }

    public static enum ErrorType {
        UNKNOWN(-1),
        INVALID_ARG(-2),
        HDMI_CABLE_CONNECTED(-3),
        OPERATION_TIMED_OUT(-4),
        ALREADY_INITIALIZED(-10), /*Session specific errors*/
        NOT_INITIALIZED(-11),
        SESSION_IN_PROGRESS(-12),
        INCORRECT_STATE_FOR_OPERATION(-13),
        NOT_SINK_DEVICE(-14),
        NOT_SOURCE_DEVICE(-15),
        UIBC_NOT_ENABLED(-20) /*UIBC related errors*/,
        UIBC_ALREADY_ENABLED(-21);

        private final int code;

        private ErrorType(int c) {
            code = c;
        }

        public int getCode() {
            return code;
        }

        public static ErrorType getValue(int c) {
            for (ErrorType e:values()) {
                if (e.code == c)
                    return e;
            }
            return null;
        }
    }

    /*
     * This should be in sync with definition in wfd_cfg_parser.h
     */
    public static enum ConfigKeys {
        AUDIO_AV_SYNC_DEL,
        AUDIO_IN_SUSPEND,
        CYCLIC_IR,
        CYCLIC_IR_NUM_MACRO_BLK,
        DISABLE_NALU_FILLER,
        DYN_BIT_ADAP,
        ENCRYPT_AUDIO_DECISION,
        ENCRYPT_NON_SECURE,
        HDCP_ENFORCED,
        PERF_LEVEL_PERF_MODE,
        PERF_LEVEL_TURBO_MODE,
        RTP_DUMP_ENABLE,
        UIBC_M14,
        UIBC_VALID,
        VIDEO_PKTLOSS_FRAME_DROP_MODE,
        DISABLE_AVSYNC_MODE,
        MAX_FPS_SUPPORTED,
        PERIODIC_IDR_INTERVAL_VALID,
        PERIODIC_IDR_INTERVAL,
        NARROW_SEARCH_RANGE,
        ENABLE_AUDIO_TRACK_LATENCY_MODE,
        AUDIO_AVSYNC_DROP_WINDOW,
        VIDEO_AVSYNC_DROP_WINDOW,
        TOTAL_CFG_KEYS
    }

    // This enum should be in sync with definition in UIBCDefs.h
    public static enum HIDDataType {
        HID_INVALID_DATA,HID_REPORT, HID_REPORT_DESCRIPTOR
    }

}
