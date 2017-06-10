/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.nfc.dhimpl;

import android.content.Context;
import android.content.SharedPreferences;
import android.nfc.ErrorCodes;
import android.nfc.tech.Ndef;
import android.nfc.dta.tech.TagTechnology;
import android.util.Log;

import com.android.nfc.QHostListener;
import com.android.nfc.DeviceHost;
import com.android.nfc.LlcpException;
import com.android.nfc.NfcDiscoveryParameters;

import java.util.TimerTask;
import java.util.Timer;

/**
 * Native interface to the NFC Manager functions
 */
public class NativeNfcManager implements DeviceHost {
    private static final String TAG = "NativeNfcManager";
    static final String PREF = "NciDeviceHost";

    static final String PREF_RESET_COUNT = "pref_reset_count";

    static final int DEFAULT_LLCP_MIU = 1980;
    static final int DEFAULT_LLCP_RWSIZE = 2;

    static final String DRIVER_NAME = "android-nci";

    static {
        System.loadLibrary("qnfc_nci_jni");
    }


    /* Native structure */
    private long mNative;

    private final DeviceHostListener mListener;
    private static QHostListener mQHostListener;
    private final Context mContext;

    public NativeNfcManager(Context context, DeviceHostListener listener) {
        mListener = listener;
        initializeNativeStructure();
        mContext = context;
    }

    public static void registerQHostListener ( QHostListener q){
        mQHostListener = q;
    }

    public native boolean initializeNativeStructure();

    private native boolean doDownload();

    public native int doGetLastError();

    @Override
    public void checkFirmware() {
        doDownload();
    }

    private native void doInitNfceeIdSeMap();

    @Override
    public void initNfceeIdSeMap(){
        doInitNfceeIdSeMap();
    }

    private native boolean doInitialize();

    @Override
    public boolean initialize() {
        return doInitialize();
    }

    private native void doReportReason(int reason);

    @Override
    public void nfcShutdownReason(int reason) {
          doReportReason(reason);
    }

    private native boolean doIsUiStateSupported();

    @Override
    public boolean isUiStateSupported()
    {
          return doIsUiStateSupported();
    }

    private native boolean doDeinitialize();

    @Override
    public boolean deinitialize() {
        return doDeinitialize();
    }

    @Override
    public native void updateHostPresence(int hciUiState, int nfccUiState);

    @Override
    public native boolean updateLockScreenPollingMode(boolean enable);

    @Override
    public String getName() {
        return DRIVER_NAME;
    }

    @Override
    public native boolean PrbsOn(int tech, int rate, boolean init);

    @Override
    public native boolean PrbsOff();

    @Override
    public native boolean sendRawFrame(byte[] data);

    @Override
    public native boolean multiSeControlCmd(byte[] ppse_rsp, int op_code);
    @Override
    public native boolean isMultiSeEnabled();
    @Override
    public native boolean routeAid(byte[] aid, int route, boolean isSubSet, boolean isSuperSet);

    @Override
    public native boolean unrouteAid(byte[] aid);

    public native boolean commitRouting();


    private native boolean doEnableDiscovery(int techMask,
                                          boolean enableLowPowerPolling,
                                          boolean enableReaderMode,
                                          boolean enableHostRouting,
                                          boolean enableP2p,
                                          int offHostRouting,
                                          boolean restart);

    @Override
    public boolean enableDiscovery(NfcDiscoveryParameters params, boolean restart) {
        String offHostRouting = params.getOffHostRouting();
        int nfceeId = -1;
        if (offHostRouting != null) {
            nfceeId = doGetNfceeId(offHostRouting);
        }
        return doEnableDiscovery(params.getTechMask(), params.shouldEnableLowPowerDiscovery(),
                params.shouldEnableReaderMode(), params.shouldEnableHostRouting(),
                params.shouldEnableP2p(), nfceeId, restart);
    }

    @Override
    public native boolean disableDiscovery();

    @Override
    public native void deactivateRfInterface();

    /*deprecated?*/
    @Override
    public native void enableRoutingToHost();

    /*deprecated?*/
    @Override
    public native void disableRoutingToHost();

    /*deprecated?*/
    @Override
    public native boolean doActivateSwp(byte slot_id);

    /*deprecated?*/
    @Override
    public native boolean doSelectSecureElement(String activeSE);

    /*deprecated?*/
    @Override
    public native void doDeselectSecureElement();


    private native NativeLlcpConnectionlessSocket doCreateLlcpConnectionlessSocket(int nSap,
            String sn);

    @Override
    public LlcpConnectionlessSocket createLlcpConnectionlessSocket(int nSap, String sn)
            throws LlcpException {
        LlcpConnectionlessSocket socket = doCreateLlcpConnectionlessSocket(nSap, sn);
        if (socket != null) {
            return socket;
        } else {
            /* Get Error Status */
            int error = doGetLastError();

            Log.d(TAG, "failed to create llcp socket: " + ErrorCodes.asString(error));

            switch (error) {
                case ErrorCodes.ERROR_BUFFER_TO_SMALL:
                case ErrorCodes.ERROR_INSUFFICIENT_RESOURCES:
                    throw new LlcpException(error);
                default:
                    throw new LlcpException(ErrorCodes.ERROR_SOCKET_CREATION);
            }
        }
    }

    private native NativeLlcpServiceSocket doCreateLlcpServiceSocket(int nSap, String sn, int miu,
            int rw, int linearBufferLength);
    @Override
    public LlcpServerSocket createLlcpServerSocket(int nSap, String sn, int miu,
            int rw, int linearBufferLength) throws LlcpException {
        LlcpServerSocket socket = doCreateLlcpServiceSocket(nSap, sn, miu, rw, linearBufferLength);
        if (socket != null) {
            return socket;
        } else {
            /* Get Error Status */
            int error = doGetLastError();

            Log.d(TAG, "failed to create llcp socket: " + ErrorCodes.asString(error));

            switch (error) {
                case ErrorCodes.ERROR_BUFFER_TO_SMALL:
                case ErrorCodes.ERROR_INSUFFICIENT_RESOURCES:
                    throw new LlcpException(error);
                default:
                    throw new LlcpException(ErrorCodes.ERROR_SOCKET_CREATION);
            }
        }
    }

    private native NativeLlcpSocket doCreateLlcpSocket(int sap, int miu, int rw,
            int linearBufferLength);
    @Override
    public LlcpSocket createLlcpSocket(int sap, int miu, int rw,
            int linearBufferLength) throws LlcpException {
        LlcpSocket socket = doCreateLlcpSocket(sap, miu, rw, linearBufferLength);
        if (socket != null) {
            return socket;
        } else {
            /* Get Error Status */
            int error = doGetLastError();

            Log.d(TAG, "failed to create llcp socket: " + ErrorCodes.asString(error));

            switch (error) {
                case ErrorCodes.ERROR_BUFFER_TO_SMALL:
                case ErrorCodes.ERROR_INSUFFICIENT_RESOURCES:
                    throw new LlcpException(error);
                default:
                    throw new LlcpException(ErrorCodes.ERROR_SOCKET_CREATION);
            }
        }
    }

    @Override
    public native boolean doCheckLlcp();

    @Override
    public native boolean doActivateLlcp();

    private native void doResetTimeouts();

    @Override
    public void resetTimeouts() {
        doResetTimeouts();
    }

    public native void doAbortCount(int count);
    @Override
    public void doAbort() {
        SharedPreferences prefs = mContext.getSharedPreferences(PREF, Context.MODE_PRIVATE);
        int count = prefs.getInt(PREF_RESET_COUNT, 0);
        doAbortCount(count);
    }


    private native boolean doSetTimeout(int tech, int timeout);
    @Override
    public boolean setTimeout(int tech, int timeout) {
        return doSetTimeout(tech, timeout);
    }

    private native int doGetTimeout(int tech);
    @Override
    public int getTimeout(int tech) {
        return doGetTimeout(tech);
    }

    private native byte[] doGetRamDump(int addr, int len);

    @Override
    public byte[] GetRamDump(int addr, int len) {
        byte[] result = doGetRamDump(addr, len);
        return result;
    }

    // <DTA>
    private native void do_dta_set_pattern_number(int pattern);
    @Override
    public void dta_set_pattern_number(int pattern) {
        do_dta_set_pattern_number(pattern);
    }

    private native int do_dta_get_pattern_number();
    @Override
    public int dta_get_pattern_number() {
        int receivedPattern = do_dta_get_pattern_number();
        return receivedPattern;
    }

    private native boolean doNfcDeactivate(int deactivationType);
    @Override
    public boolean nfcDeactivate(int deactivationType) {
        return doNfcDeactivate(deactivationType);
    }

    @Override
    public boolean in_dta_mode() {
        return dta_get_pattern_number() >= 0;
    }
    // </DTA>

    @Override
    public boolean canMakeReadOnly(int ndefType) {
        return (ndefType == Ndef.TYPE_1 || ndefType == Ndef.TYPE_2);
    }

    @Override
    public int getMaxTransceiveLength(int technology) {
        switch (technology) {
            case (TagTechnology.NFC_A):
            case (TagTechnology.MIFARE_CLASSIC):
            case (TagTechnology.MIFARE_ULTRALIGHT):
                if(in_dta_mode()) {
                    return 1024; //1k (no justification)
                } else {
                    return 253; // PN544 RF buffer = 255 bytes, subtract two for CRC
                }
            case (TagTechnology.NFC_B):
                /////////////////////////////////////////////////////////////////
                // Broadcom: Since BCM2079x supports this, set NfcB max size.
                //return 0; // PN544 does not support transceive of raw NfcB
                return 253; // PN544 does not support transceive of raw NfcB
            case (TagTechnology.NFC_V):
                return 253; // PN544 RF buffer = 255 bytes, subtract two for CRC
            case (TagTechnology.ISO_DEP):
            case (TagTechnology.NFC_DEP_INITIATOR):
            case (TagTechnology.NFC_DEP_TARGET):
                /* The maximum length of a normal IsoDep frame consists of:
                 * CLA, INS, P1, P2, LC, LE + 255 payload bytes = 261 bytes
                 * such a frame is supported. Extended length frames however
                 * are currently not supported in normal mode.
                 */
                if(in_dta_mode()) {
                    /* Extended length frames only for DTA mode
                     * TODO: Can we support them for normal mode
                     * CLA(1)+INS(1)+P1(1)+P2(1)+LC(3)+LE(3)+65535 payload bytes = 65545 bytes
                     */
                    return 65545;
                } else {
                    return 261; // Will be automatically split in two frames on the RF layer
                }
            case (TagTechnology.NFC_F):
                if(in_dta_mode()) {
                    return 1024; //1k (no justification)
                } else {
                    return 252; // PN544 RF buffer = 255 bytes, subtract one for SoD, two for CRC
                }
            default:
                return 0;
        }

    }

    private native void doSetP2pInitiatorModes(int modes);
    @Override
    public void setP2pInitiatorModes(int modes) {
        doSetP2pInitiatorModes(modes);
    }

    private native void doSetP2pTargetModes(int modes);
    @Override
    public void setP2pTargetModes(int modes) {
        doSetP2pTargetModes(modes);
    }

    @Override
    public boolean getExtendedLengthApdusSupported() {
        return false;
    }

    @Override
    public int getDefaultLlcpMiu() {
        return DEFAULT_LLCP_MIU;
    }

    @Override
    public int getDefaultLlcpRwSize() {
        return DEFAULT_LLCP_RWSIZE;
    }


    private native String doDump();
    @Override
    public String dump() {
        return doDump();
    }

    private native void doEnableScreenOffSuspend();
    @Override
    public boolean enableScreenOffSuspend() {
        doEnableScreenOffSuspend();
        return true;
    }

    private native void doDisableScreenOffSuspend();
    @Override
    public boolean disableScreenOffSuspend() {
        doDisableScreenOffSuspend();
        return true;
    }

    /**
     * Notifies Ndef Message (TODO: rename into notifyTargetDiscovered)
     */
    private void notifyNdefMessageListeners(NativeNfcTag tag) {
        mListener.onRemoteEndpointDiscovered(tag);
    }

    /**
     * Notifies transaction
     */
    private void notifyTargetDeselected() {
        mListener.onCardEmulationDeselected();
    }

    /**
     * Notifies transaction
     */
    private void notifyTransactionListeners(byte[] dataBuf) {
        mQHostListener.onCardEmulationAidSelected(dataBuf);
    }

    /**
     * Notifies RF interface deactivation
     */
    public void notifyRfInterfaceDeactivated() {
        mQHostListener.onRfInterfaceDeactivated();
    }
    /**
     * Notifies HCI Event Connectivity
     */
    private void notifyHciEventConnectivity(int nfceeId) {
        String seName = doGetSecureElementName(nfceeId);
        int slotId = -1;
        if (seName.equals("SIM - UICC") ||
            seName.equals("SIM1")) {
           slotId = 0;
        } else if (seName.equals("SIM2")) {
           slotId = 1;
        }
        if (slotId != -1) {
            mListener.onCardEmulationHciEvtConnectivity(slotId);
        }
    }

    /**
     * Notifies P2P Device detected, to activate LLCP link
     */
    private void notifyLlcpLinkActivation(NativeP2pDevice device) {
        mListener.onLlcpLinkActivated(device);
    }

    /**
     * Notifies P2P Device detected, to activate LLCP link
     */
    private void notifyLlcpLinkDeactivated(NativeP2pDevice device) {
        mListener.onLlcpLinkDeactivated(device);
    }

    /**
     * Notifies first packet received from remote LLCP
     */
    private void notifyLlcpLinkFirstPacketReceived(NativeP2pDevice device) {
        mListener.onLlcpFirstPacketReceived(device);
    }

    private void notifyHostEmuActivated() {
        mListener.onHostCardEmulationActivated();
    }

    private void notifyHostEmuData(byte[] data) {
        mListener.onHostCardEmulationData(data);
    }

    private void notifyHostEmuDeactivated() {
        mListener.onHostCardEmulationDeactivated();
    }

    final Object sync = new Object();
    Timer rfFieldUpdateTimer = new Timer();
    RfOffTask pendingRfUpdate;

    private class RfOffTask extends TimerTask {
        public void run(){
            synchronized(sync) {
                mListener.onRemoteFieldDeactivated();
                pendingRfUpdate = null;
            }
        }
    }

    private void notifyRfFieldActivated() {
        synchronized(sync) {
            if(pendingRfUpdate == null) {
                mListener.onRemoteFieldActivated();
            } else {
                pendingRfUpdate.cancel();
            }
        }
    }

    private void notifyRfFieldDeactivated() {
        synchronized(sync) {
            if(pendingRfUpdate!=null) pendingRfUpdate.cancel();
            pendingRfUpdate = new RfOffTask();
            rfFieldUpdateTimer.schedule(pendingRfUpdate, 100);
        }
    }

    /**
     * Notify NFCC info to nfcservice
     */
    private void notifyNfccInfo(byte[] nfccinfo) {
        mListener.onNfccInit(nfccinfo);
    }

    private void notifyRequestRestartNfc() {
        mListener.noRequestRestartNfc();
    }

    private native boolean doGetEeRoutingReloadAtReboot();

    @Override
    public boolean getEeRoutingReloadAtReboot()
    {
          return doGetEeRoutingReloadAtReboot();
    }

    private native String doGetDefaultActiveSecureElement();

    @Override
    public String getDefaultActiveSecureElement()
    {
          return doGetDefaultActiveSecureElement();
    }

    private native String doGetSecureElementList();

    @Override
    public String getSecureElementList()
    {
          return doGetSecureElementList();
    }

    private native String doGetSecureElementName(int seId);

    @Override
    public String getSecureElementName(int seId)
    {
          return doGetSecureElementName(seId);
    }

    private native int doGetNfceeId(String seName);

    @Override
    public int getNfceeId(String seName)
    {
        return doGetNfceeId(seName);
    }

    private native int doGetEeRoutingState();

    @Override
    public int getEeRoutingState()
    {
          int num = doGetEeRoutingState();
          return num;
    }

    public native byte[] doGetLMRT();

    @Override
    public byte[] getLMRT()
    {
        return doGetLMRT();
    }

    public native boolean doSetDefaultRoute(int nfceeId);

    @Override
    public boolean setDefaultRoute(String seName)
    {
        int nfceeId = doGetNfceeId(seName);
        return doSetDefaultRoute(nfceeId);
    }

    public native int doGetDefaultRoute();

    @Override
    public String getDefaultRoute()
    {
        int nfceeId = doGetDefaultRoute();
        return doGetSecureElementName(nfceeId);
    }

    public native boolean doIsExchangingApduWithEse();

    @Override
    public boolean isExchangingApduWithEse() {
        return doIsExchangingApduWithEse();
    }

    public native boolean doIsRfInterfaceActivated();

    @Override
    public boolean isRfInterfaceActivated() {
        return doIsRfInterfaceActivated();
    }

    public native void doNotifyApduGateRfIntfDeactivated();

    @Override
    public void notifyApduGateRfIntfDeactivated() {
        doNotifyApduGateRfIntfDeactivated();
    }

    private void updateHostCallBack() {
        mListener.onUpdateHostCallBack();
    }

    /**
     * update Reset Counter
     */
    private void notifyUpdateResetCounter(boolean reset) {
        SharedPreferences prefs = mContext.getSharedPreferences(PREF, Context.MODE_PRIVATE);

        int count = prefs.getInt(PREF_RESET_COUNT, 0);

        if(reset) {
        count = 0;
        } else {
        count++;
        }

        SharedPreferences.Editor editor = prefs.edit();
        editor.putInt(PREF_RESET_COUNT, count);
        editor.commit();
    }

    // AID Filter
    private native boolean doInitClfAidFilterList();

    public boolean initClfAidFilterList() {
        return doInitClfAidFilterList();
    }

    private native boolean doSetClfAidFilterList(byte[] filterList);

    public boolean setClfAidFilterList(byte[] filterList) {
        return doSetClfAidFilterList(filterList);
    }

    private native boolean doEnableDisableClfAidFilterCondition(boolean enable, byte filterConditionTag);

    public boolean enableClfAidFilterCondition(byte filterConditionTag) {
        return doEnableDisableClfAidFilterCondition(true, filterConditionTag);
    }

    public boolean disableClfAidFilterCondition(byte filterConditionTag) {
        return doEnableDisableClfAidFilterCondition(true, filterConditionTag);
    }

}
