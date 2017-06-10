/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */


package com.qualcomm.wfd.client;
import java.io.IOException;
import android.util.Log;

/**
 * A class representing Wireless Display Service Capability information about a
 * Wi-Fi p2p device. Information available is a part of WiFiP2pWfdInfo class
 * IE information is present in mDeviceInfo member
 */
public class P2pWfdDeviceInfo {

    private static final String TAG = "P2pWfdDeviceInfo";
    public static final int SERVICE_DISCOVERY_SUPPORTED      = 0x40;
    public static final int PREFERRED_CONNECTIVITY_TDLS      = 0x80;
    public static final int CP_SUPPORTED                     = 0x100;
    public static final int TIME_SYNC_SUPPORTED              = 0x200;
    public static final int AUDIO_NOT_SUPPORTED_AT_PSINK     = 0x400;
    public static final int AUDIO_ONLY_SUPPORTED_AT_SOURCE   = 0x800;
    public static final int TDLS_PERSISTENT_GROUP            = 0x1000;
    public static final int TDLS_PERSISTENT_GROUP_REINVOKE   = 0x2000;
    // Following value is set based on 802.11n rates
    public static final int WIFI_MAX_THROUGHPUT              = 54;//In multiples of 1Mbps

    public P2pWfdDeviceInfo() {
        deviceType = DEVICETYPE_INVALID;
        preferredConnectivity = PC_P2P;
        coupledSinkStatus = NOT_COUPLED_AVAILABLE;
        sessionMgmtCtrlPort = DEFAULT_SESSION_MGMT_CTRL_PORT;
    };

    /**
     * @param String input with WFD parameters Note: The events formats can be
     *            looked up in the wpa_supplicant code
     * @hide
     */
    public P2pWfdDeviceInfo(String wfdSubelems) throws IllegalArgumentException {

        if (wfdSubelems == null) {
            throw new IllegalArgumentException("Malformed string");
        }

        int val = 0;

        if (wfdSubelems.length() > 6) {//first octet is identifier
                                       //next 2 are the size
            if (wfdSubelems.substring(0,2).equals("00") &&
                wfdSubelems.substring(2,4).equals("06")) {
                    //Indicates it was device specific info and the size is correct
                    val = (Integer.parseInt(wfdSubelems.substring(4,8),16)) & 0x03;
                    if (val == 0) {
                        deviceType = DEVICETYPE_SOURCE;
                    } else if (val == 1) {
                        deviceType = DEVICETYPE_PRIMARYSINK;
                    } else if (val == 2) {
                        deviceType = DEVICETYPE_SECONDARYSINK;
                    } else if (val == 3) {
                        deviceType = DEVICETYPE_SOURCE_PRIMARYSINK;
                    } else {
                        deviceType = DEVICETYPE_INVALID;
                    }
            }

            val = Integer.parseInt(wfdSubelems.substring(4,8),16) & 0x04;
            if (val == 0x04) {
                isCoupledSinkSupportedBySource = true;
            } else {
                isCoupledSinkSupportedBySource = false;
            }

            val = Integer.parseInt(wfdSubelems.substring(4,8),16) & 0x08;

            if (val == 0x08) {
                isCoupledSinkSupportedBySink = true;
            } else {
                isCoupledSinkSupportedBySink = false;
            }

            val = (Integer.parseInt(wfdSubelems.substring(4,8),16)) & 0x30;
            if (val == 0x10 ){
                isAvailableForSession = true;
            } else {
                isAvailableForSession = false;
            }

            val = (Integer.parseInt(wfdSubelems.substring(4,8),16)) & 0x40;
            if (val == 0x40) {
                isServiceDiscoverySupported = true;
            } else {
                isServiceDiscoverySupported = false;
            }

            val = (Integer.parseInt(wfdSubelems.substring(4,8),16)) & 0x80;
            if (val == 0x80) {
                preferredConnectivity = PC_TDLS;
            } else {
                preferredConnectivity = PC_P2P;
            }

            val = (Integer.parseInt(wfdSubelems.substring(4,8),16)) & 0x0100;
            if (val == 0x0100) {
                isContentProtectionSupported = true;
            } else {
                isContentProtectionSupported = false;
            }

            val = (Integer.parseInt(wfdSubelems.substring(4,8),16)) & 0x0200;
            if (val == 0x0200) {
                isTimeSynchronizationSupported = true;
            } else {
                isTimeSynchronizationSupported = false;
            }

            val = (Integer.parseInt(wfdSubelems.substring(4,8),16)) & 0x0400;
            if (val == 0x0400) {
                isAudioUnspportedAtPrimarySink = true;
            } else {
                isAudioUnspportedAtPrimarySink = false;
            }

            val = (Integer.parseInt(wfdSubelems.substring(4,8),16)) & 0x0800;
            if (val == 0x0800) {
                isAudioOnlySupportedAtSource = true;
            } else {
                isAudioOnlySupportedAtSource = false;
            }

            val = (Integer.parseInt(wfdSubelems.substring(4,8),16)) & 0x1000;
            if (val == 0x1000) {
                isTDLSPersistentGroupIntended = true;
            } else {
                isTDLSPersistentGroupIntended = false;
            }

            val = (Integer.parseInt(wfdSubelems.substring(4,8),16)) & 0x2000;
            if (val == 0x2000) {
                isTDLSReInvokePersistentGroupReq = true;
            } else {
                isTDLSReInvokePersistentGroupReq = false;
            }

            sessionMgmtCtrlPort = Integer.parseInt(wfdSubelems.substring(8,12),16);

            coupledSinkStatus = NOT_COUPLED_AVAILABLE;
        }

        Log.d(TAG, "Created P2pWfdDeviceInfo \n" + toString());
    };

    /** Device Type is WFD Source */
    public static final int DEVICETYPE_SOURCE = 0;

    /** Device Type is Primary Sink */
    public static final int DEVICETYPE_PRIMARYSINK = 1;

    /** Device Type is Secondary Sink */
    public static final int DEVICETYPE_SECONDARYSINK = 2;

    /** Device Type is Source and Primary Sink */
    public static final int DEVICETYPE_SOURCE_PRIMARYSINK = 3;

    /** Device Type Invalid */
    public static final int DEVICETYPE_INVALID = 4;

    /** Preferred Connectivity (PC) is P2P */
    public static final int PC_P2P = 0;

    /** Preferred Connectivity (PC) is TDLS */
    public static final int PC_TDLS = 1;

    /** Device is not coupled and available */
    public static final int NOT_COUPLED_AVAILABLE = 0;

    /** Device is coupled */
    public static final int COUPLED = 1;

    /** Teardown coupling */
    public static final int TEARDOWN_COUPLING = 2;

    /** Default RTSP Control Port */
    public static final int DEFAULT_SESSION_MGMT_CTRL_PORT = 7236;

    /**
     * RTSP Control Port. Valid values are between 1-65535. Default value is
     * DEFAULT_SESSION_MGMT_CTRL_PORT
     */
    public int sessionMgmtCtrlPort;

    /**
     * Maximum average throughput capability of the WFDDevice represented in
     * multiples of 1Mbps
     */
    public int maxThroughput;

    /**
     * DEVICETYPE_SOURCE: WFD source, DEVICETYPE_PRIMARYSINK: Primary sink
     * DEVICETYPE_SECONDARYSINK: Secondary sink, DEVICETYPE_SOURCE_PRIMARYSINK:
     * WFD source/Primary sink Default value is DEVICETYPE_INVALID
     */
    public int deviceType;

    /**
     * PC_P2P: Preferred Connectivity: P2P PC_TDLS: Preferred Connectivity: TDLS
     * Default value is PC_INVALID
     */
    public int preferredConnectivity;

    /**
     * NOT_COUPLED_AVAILABLE: Not Coupled, Available for coupling COUPLED:
     * Coupled TEARDOWN_COUPLING: Teardown coupling Default value is
     * NOT_COUPLED_AVAILABLE
     */
    public int coupledSinkStatus;

    /** Mac Address of coupled sink */
    public String coupledDeviceAdress;

 /** Mac Address of intended Address */
    public String intendedAddress;

    /**
     * false: coupled sink operation not supported by WFD source. true : coupled
     * sink operation supported by WFD source This bit is valid when WFD Device
     * Type is either DEVICETYPE_SOURCE or DEVICETYPE_SOURCE_PRIMARYSINK
     */
    public boolean isCoupledSinkSupportedBySource;

    /**
     * false: coupled sink operation not supported by WFD Sink. true : coupled
     * sink operation supported by WFD Sink This bit is valid when WFD Device
     * Type is either DEVICETYPE_PRIMARYSINK, DEVICETYPE_SECONDARYSINK or
     * DEVICETYPE_SOURCE_PRIMARYSINK
     */
    public boolean isCoupledSinkSupportedBySink;

    /**
     * false: Not available for WFD Session true: Available for WFD Session
     */
    public boolean isAvailableForSession;

    /**
     * false: Service Discovery not supported true: Service Discovery supported
     */
    public boolean isServiceDiscoverySupported;

    /**
     * false: Content Protection not supported true: Content Protection
     * supported
     */
    public boolean isContentProtectionSupported;

    /**
     * false: Time Synchronization not supported true: Time Synchronization
     * supported
     */
    public boolean isTimeSynchronizationSupported;

    /**
     * false: Audio Supported at Primary Sink true: Audio Unsupported at
     * supported
     */
    public boolean isAudioUnspportedAtPrimarySink;

    /**
     * false: Audio only WFD Session not supported by Source true: Audio only
     * WFD Session supported by Source
     */
    public boolean isAudioOnlySupportedAtSource;

    /**
     * false: TDLS Persistent Group not intended by device true: TDLS Persistent
     * Group intended by device
     */
    public boolean isTDLSPersistentGroupIntended;

    /**
     * false: Not a request to reinvoke a TDLS persistent group true: This is a
     * request to reinvoke TDLS persistent group
     */
    public boolean isTDLSReInvokePersistentGroupReq;

    /** copy constructor */
    public P2pWfdDeviceInfo(P2pWfdDeviceInfo source) {
        if (source != null) {
            sessionMgmtCtrlPort = source.sessionMgmtCtrlPort;
            maxThroughput = source.maxThroughput;
            deviceType = source.deviceType;
            preferredConnectivity = source.preferredConnectivity;
            coupledSinkStatus = source.coupledSinkStatus;
            coupledDeviceAdress = source.coupledDeviceAdress;
            intendedAddress = source.intendedAddress;
            isCoupledSinkSupportedBySource = source.isCoupledSinkSupportedBySource;
            isCoupledSinkSupportedBySink = source.isCoupledSinkSupportedBySink;
            isAvailableForSession = source.isAvailableForSession;
            isCoupledSinkSupportedBySource = source.isCoupledSinkSupportedBySource;
            isContentProtectionSupported = source.isContentProtectionSupported;
            isTimeSynchronizationSupported = source.isTimeSynchronizationSupported;
            isAudioUnspportedAtPrimarySink = source.isAudioUnspportedAtPrimarySink;
            isAudioOnlySupportedAtSource = source.isAudioOnlySupportedAtSource;
            isTDLSPersistentGroupIntended = source.isTDLSPersistentGroupIntended;
            isTDLSReInvokePersistentGroupReq = source.isTDLSReInvokePersistentGroupReq;
        }
    }

    /** Implement the Parcelable interface */
    public int describeContents() {
        return 0;
    }

    public String toString() {
        StringBuffer sbuf = new StringBuffer();
        sbuf.append("\n WFD Info Expanded:");
        sbuf.append("\n DeviceType: ").append(deviceType);
        sbuf.append("\n Control Port: ").append(sessionMgmtCtrlPort);
        sbuf.append("\n MaxThroughput: ").append(maxThroughput);
        sbuf.append("\n PreferredConnectivity: ").append(preferredConnectivity);
        sbuf.append("\n CoupledSinkStatus: ").append(coupledSinkStatus);
        sbuf.append("\n CoupledDeviceAddress: ").append(coupledDeviceAdress);
        sbuf.append("\n IntendedAddress: ").append(intendedAddress);
        sbuf.append("\n IsCoupledSinkSupportedBySource: ").append(isCoupledSinkSupportedBySource);
        sbuf.append("\n IsCoupledSinkSupportedBySink: ").append(isCoupledSinkSupportedBySink);
        sbuf.append("\n IsAvailableForSession: ").append(isAvailableForSession);
        sbuf.append("\n IsCoupledSinkSupportedBySource: ").append(isCoupledSinkSupportedBySource);
        sbuf.append("\n IsContentProtectionSupported: ").append(isContentProtectionSupported);
        sbuf.append("\n IsTimeSynchronizationSupported: ").append(isTimeSynchronizationSupported);
        sbuf.append("\n IsAudioUnspportedAtPrimarySink: ").append(isAudioUnspportedAtPrimarySink);
        sbuf.append("\n IsAudioOnlySupportedAtSource: ").append(isAudioOnlySupportedAtSource);
        sbuf.append("\n IsTDLSPersistentGroupIntended: ").append(isTDLSPersistentGroupIntended);
        sbuf.append("\n IsTDLSReInvokePersistentGroupReq: ").append(
                isTDLSReInvokePersistentGroupReq);
        return sbuf.toString();
    }

    /** Returns true if the device is a group owner */
    public boolean isWFDDevice() {

        return (deviceType != DEVICETYPE_INVALID);
    }

}
