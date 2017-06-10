/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package qcom.nfc;

import java.util.List;
import android.nfc.cardemulation.AidGroup;

interface IQNfcSecureElementManagerCallbacks
{
    oneway void onGetOffHostService(boolean isLast, String description, String seName, int bannerResId,
                                    in List<String> dynamicAidGroupDescriptions,
                                    in List<android.nfc.cardemulation.AidGroup> dynamicAidGroups);
}

