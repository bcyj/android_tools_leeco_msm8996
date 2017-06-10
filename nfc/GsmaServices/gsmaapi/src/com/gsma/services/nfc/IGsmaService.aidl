/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.gsma.services.nfc;

import android.os.Messenger;
import java.util.List;
import android.nfc.cardemulation.AidGroup;
import com.gsma.services.nfc.IGsmaServiceCallbacks;

/**
 * GSMA service interface.
 */
interface IGsmaService {

    boolean isNfccEnabled();
    boolean enableNfcc(in Messenger clientMessenger);
    boolean isCardEmulationEnabled();
    boolean enableCardEmulationMode(in Messenger clientMessenger);
    boolean disableCardEmulationMode(in Messenger clientMessenger);
    String getActiveSecureElement();
    void setActiveSecureElement(String SEName);
    void mgetPname(String packageN);
    void enableMultiReception(String SEName);
    boolean commitOffHostService(String packageName, String seName, String description,
                                 int bannerResId, int uid, in List<String> aidGroupDescriptions,
                                 in List<AidGroup> aidGroups);
    boolean deleteOffHostService(String packageName, String seName);
    boolean getOffHostServices(String packageName, IGsmaServiceCallbacks callbacks);

}

