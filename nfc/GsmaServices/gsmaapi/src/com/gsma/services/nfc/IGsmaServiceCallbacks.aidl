/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.gsma.services.nfc;

import java.util.List;
import android.nfc.cardemulation.AidGroup;

interface IGsmaServiceCallbacks
{
    oneway void onGetOffHostService(boolean isLast, String description, String seName, int bannerResId,
                                    in List<String> dynamicAidGroupDescriptions,
                                    in List<android.nfc.cardemulation.AidGroup> dynamicAidGroups);
}

