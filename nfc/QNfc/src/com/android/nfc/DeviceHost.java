/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.nfc;

import android.annotation.Nullable;
import android.nfc.NdefMessage;
import android.os.Bundle;

import java.io.IOException;

public interface DeviceHost {
    public interface DeviceHostListener {
        public void onRemoteEndpointDiscovered(TagEndpoint tag);

        /**
         * Notifies transaction
         */
        public void onCardEmulationDeselected();

        /**
         * Notifies transaction
         */
        public void onCardEmulationAidSelected(byte[] dataBuff);

        /**
         * Notifies HCI Event Connectivity (slot 0:SIM1, slot 1:SIM2)
         */
        public void onCardEmulationHciEvtConnectivity(int slotId);

        /**
         */
        public void onHostCardEmulationActivated();
        public void onHostCardEmulationData(byte[] data);
        public void onHostCardEmulationDeactivated();

        /**
         * Notifies P2P Device detected, to activate LLCP link
         */
        public void onLlcpLinkActivated(NfcDepEndpoint device);

        /**
         * Notifies P2P Device detected, to activate LLCP link
         */
        public void onLlcpLinkDeactivated(NfcDepEndpoint device);

        public void onLlcpFirstPacketReceived(NfcDepEndpoint device);

        public void onRemoteFieldActivated();

        public void onRemoteFieldDeactivated();

        /**
         * Notify NFCC info to nfcservice
         */
        public void onNfccInit(byte[] nfccinfo);

        public void noRequestRestartNfc();

        /**
         * Update Host Presence Callback
         */
        public void onUpdateHostCallBack();
    }

    public interface TagEndpoint {
        boolean connect(int technology);
        boolean reconnect();
        boolean disconnect();

        boolean presenceCheck();
        boolean isPresent();
        void startPresenceChecking(int presenceCheckDelay,
                                   @Nullable TagDisconnectedCallback callback);
        void setIsPresent(boolean isPresent); //<DTA>
        int[] getTechList();
        void removeTechnology(int tech); // TODO remove this one
        Bundle[] getTechExtras();
        byte[] getUid();
        int getHandle();

        byte[] transceive(byte[] data, boolean raw, int[] returnCode);

        boolean checkNdef(int[] out);
        byte[] readNdef();
        boolean writeNdef(byte[] data);
        NdefMessage findAndReadNdef();
        boolean formatNdef(byte[] key);
        boolean isNdefFormatable();
        boolean makeReadOnly();

        int getConnectedTechnology();
        boolean hasTech(int tech);
    }

    public interface TagDisconnectedCallback {
        void onTagDisconnected(long handle);
    }

    public interface NfceeEndpoint {
        // TODO flesh out multi-EE and use this
    }

    public interface NfcDepEndpoint {

        /**
         * Peer-to-Peer Target
         */
        public static final short MODE_P2P_TARGET = 0x00;
        /**
         * Peer-to-Peer Initiator
         */
        public static final short MODE_P2P_INITIATOR = 0x01;
        /**
         * Invalid target mode
         */
        public static final short MODE_INVALID = 0xff;

        public byte[] receive();

        public boolean send(byte[] data);

        public boolean connect();

        public boolean disconnect();

        public byte[] transceive(byte[] data);

        public int getHandle();

        public int getMode();

        public byte[] getGeneralBytes();
    }

    public interface LlcpSocket {
        public void connectToSap(int sap) throws IOException;

        public void connectToService(String serviceName) throws IOException;

        public void close() throws IOException;

        public void send(byte[] data) throws IOException;

        public int receive(byte[] recvBuff) throws IOException;

        public int getRemoteMiu();

        public int getRemoteRw();

        public int getLocalSap();

        public int getLocalMiu();

        public int getLocalRw();
    }

    public interface LlcpServerSocket {
        public LlcpSocket accept() throws IOException, LlcpException;

        public void close() throws IOException;
    }

    public interface LlcpConnectionlessSocket {
        public int getLinkMiu();

        public int getSap();

        public void send(int sap, byte[] data) throws IOException;

        public LlcpPacket receive() throws IOException;

        public void close() throws IOException;
        /**
         * Sends data to the service corresponding the specified service name.
         *
         * @param remoteServiceName The remote service name.
         * @param data The data to send.
         *
         * @throws IOException if the operation fails.
         */
        public void send(String remoteServiceName, byte[] data) throws IOException;
        // </DTA>
    }

    /**
     * Called at boot if NFC is disabled to give the device host an opportunity
     * to check the firmware version to see if it needs updating. Normally the firmware version
     * is checked during {@link #initialize(boolean enableScreenOffSuspend)},
     * but the firmware may need to be updated after an OTA update.
     *
     * <p>This is called from a thread
     * that may block for long periods of time during the update process.
     */
    public void checkFirmware();

    public void initNfceeIdSeMap();

    public boolean initialize();

    public boolean deinitialize();

    public String getName();

    public boolean enableDiscovery(NfcDiscoveryParameters params, boolean restart);

    public boolean disableDiscovery();

    public void deactivateRfInterface();

    /*deprecated*/
    public void enableRoutingToHost();

    /*deprecated*/
    public void disableRoutingToHost();

    /*deprecated*/
    public boolean doActivateSwp(byte slot_id);

    /*deprecated*/
    //public String doGetSecureElementList();

    /*deprecated*/
    //public String doGetSecureElementName(int seId);

    /*deprecated*/
    //public int doGetNfceeId(String seName);

    /*deprecated*/
    public boolean doSelectSecureElement(String activeSE);

    /*deprecated*/
    public void doDeselectSecureElement();

    public boolean sendRawFrame(byte[] data);

    public boolean routeAid(byte[] aid, int route, boolean isSubSet, boolean isSuperSet);

    public boolean unrouteAid(byte[] aid);

    public LlcpConnectionlessSocket createLlcpConnectionlessSocket(int nSap, String sn)
            throws LlcpException;

    public LlcpServerSocket createLlcpServerSocket(int nSap, String sn, int miu,
            int rw, int linearBufferLength) throws LlcpException;

    public LlcpSocket createLlcpSocket(int sap, int miu, int rw,
            int linearBufferLength) throws LlcpException;

    public boolean doCheckLlcp();

    public boolean doActivateLlcp();

    public void resetTimeouts();

    public boolean setTimeout(int technology, int timeout);

    public int getTimeout(int technology);

    public void doAbort();

    boolean canMakeReadOnly(int technology);

    int getMaxTransceiveLength(int technology);

    void setP2pInitiatorModes(int modes);

    void setP2pTargetModes(int modes);

    boolean getExtendedLengthApdusSupported();

    int getDefaultLlcpMiu();

    int getDefaultLlcpRwSize();

    String dump();

    boolean enableScreenOffSuspend();

    public void nfcShutdownReason(int reason);

    boolean disableScreenOffSuspend();

    public boolean isUiStateSupported();

    public void updateHostPresence(int hciUiState, int nfccUiState);

    public boolean updateLockScreenPollingMode(boolean enable);

    public boolean PrbsOn(int tech, int rate, boolean init);

    public boolean PrbsOff();

    public byte[] GetRamDump(int addr,int len);

    // <DTA>
    void dta_set_pattern_number(int pattern);

    int dta_get_pattern_number();

    public boolean nfcDeactivate(int deactivationType);

    boolean in_dta_mode();
    // </DTA>

    public byte[] getLMRT();
    public boolean getEeRoutingReloadAtReboot();
    public String getDefaultActiveSecureElement();
    public String getSecureElementList();
    public boolean multiSeControlCmd(byte[] ppse_rsp, int op_code);
    public boolean isMultiSeEnabled();
    public String getSecureElementName(int seId);
    public int getNfceeId(String seName);
    public int getEeRoutingState();
    public boolean setDefaultRoute(String seName);
    public String getDefaultRoute();
    public boolean isExchangingApduWithEse();
    public boolean isRfInterfaceActivated();
    public void notifyApduGateRfIntfDeactivated();

    // AID Filter
    public boolean initClfAidFilterList();
    public boolean setClfAidFilterList(byte[] filterList);
    public boolean enableClfAidFilterCondition(byte filterConditionTag);
    public boolean disableClfAidFilterCondition(byte filterConditionTag);
}
