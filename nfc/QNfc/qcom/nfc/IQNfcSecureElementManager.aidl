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

package qcom.nfc;

import java.util.List;
import android.os.Bundle;
import android.content.Intent;
import android.content.ComponentName;

import java.util.List;
import android.nfc.cardemulation.AidGroup;

import qcom.nfc.IQNfcSecureElementManagerCallbacks;

/**
 * {@hide}
 */
interface IQNfcSecureElementManager {
    void enableNfcController(in String pkg);
    boolean updateCardEmulationRoute(in String pkg, int route);
    boolean isCardEmulationEnabled(in String pkg);
    boolean enableCardEmulationMode(in String pkg);
    boolean disableCardEmulationMode(in String pkg);
    byte[] getLMRT(in String pkg);
    String getActiveSecureElement(in String pkg);
    void setActiveSecureElement(in String pkg, in String SEName);
    void enableMultiReception(in String pkg, in String SEName);
    boolean isSeEnabled(in String pkg, in String SEName);
    void deliverSeIntent(in String pkg, in Intent seIntent);
    void notifyCheckCertResult(in String pkg, in boolean success);
    void selectSEToOpenApduGate(String pkg, String seName);
    Bundle open(in String pkg, IBinder b);
    Bundle close(in String pkg, IBinder b);
    Bundle transceive(in String pkg, in byte[] data_in);
    boolean setClfAidFilterList(in byte[] filterList);
    boolean enableClfAidFilterCondition(in byte filterConditionTag);
    boolean disableClfAidFilterCondition(in byte filterConditionTag);
    boolean multiSeRegisterAid(in List<String> aid, in ComponentName paymentService, in List<String> seName,in List<String> priority, in List<String> powerState);
    boolean commitOffHostService(String packageName, String seName, String description,
                                 int bannerResId, int uid, in List<String> aidGroupDescriptions,
                                 in List<AidGroup> aidGroups);
    boolean deleteOffHostService(String packageName, String seName);
    boolean getOffHostServices(String packageName, IQNfcSecureElementManagerCallbacks callbacks);
}

