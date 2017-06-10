/**
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 **/

package com.qualcomm.qti.modemtestmode;

import android.content.Context;
import java.util.Arrays;

public class MbnMetaInfo {
    //Expected like xxx-3CSFB-DSDS-CMCC
    private final int META_MIN_DASH_SIZE = 3;
    private Context mContext;
    private String mMbnId;
    private String mMetaInfo;
    private String mDeviceType = "Unknown";
    private String mCarrier = "Unknown";
    private String mMultiMode = "Unknown";
    private byte[] mQcVersion;
    private byte[] mOemVersion;

    //default 0->can't get multisim configure, 1->ssss, 2->dsdx
    private int mMultiModeNumber = 0;
    private int mSource = MbnTestUtils.MBN_FROM_UNKNOWN;

    public MbnMetaInfo(Context context, String mbnId, String metaInfo) {
        this.mContext = context;
        this.mMbnId = mbnId;
        this.mMetaInfo = metaInfo;
        setSource(mbnId);
        // get all info from meta instead of MbnID
        setDeviceCarrierMultiMode(metaInfo);
    }

    private void setMultiModeNumber(String mode) {
        if (mode == null) {
            return;
        }
        if (mode.toLowerCase().contains("ss") || mode.toLowerCase().contains("singlesim")) {
            this.mMultiModeNumber = 1;
            this.mMultiMode = "ssss";
        } else if (mode.toLowerCase().contains("da")) {
            this.mMultiModeNumber = 2;
            this.mMultiMode = "dsda";
        } else if (mode.toLowerCase().contains("ds")) {
            this.mMultiModeNumber = 2;
            this.mMultiMode = "dsds";
        }
    }

    public void setQcVersion(byte[] version) {
        this.mQcVersion = version;
    }

    public void setOemVersion(byte[] version) {
        this.mOemVersion = version;
    }

    public void setDeviceCarrierMultiMode(String meta) {
        if (meta == null) {
            return;
        }

        String[] tempName;
        tempName = meta.split("-");
        int len = tempName.length;
        this.mCarrier = tempName[len-1];
        if (len >= META_MIN_DASH_SIZE) {
            this.mDeviceType = tempName[len-3];
            setMultiModeNumber(tempName[len-2].toLowerCase());
        }
        return;
    }

    private void setSource(String meta) {
        if (meta == null) {
            return;
        }
        if (meta.startsWith(MbnTestUtils.GOLDEN_MBN_ID_PREFIX)) {
            this.mSource = MbnTestUtils.MBN_FROM_GOLDEN;
        } else if (meta.startsWith(MbnTestUtils.APP_MBN_ID_PREFIX)) {
            this.mSource = MbnTestUtils.MBN_FROM_APP;
        } else {
            this.mSource = MbnTestUtils.MBN_FROM_PC;
        }
    }

    public String getCarrier() {
        return mCarrier;
    }

    public int getMultiModeNumber() {
        return mMultiModeNumber;
    }

    public String getDeviceType() {
        return mDeviceType;
    }

    public String getMultiMode() {
        return mMultiMode;
    }

    public byte[] getQcVersion() {
        return mQcVersion;
    }

    public byte[] getOemVersion() {
        return mOemVersion;
    }

    public boolean isQcMbn() {
        return Arrays.equals(mOemVersion, mQcVersion);
    }

    public String getMbnId() {
        return mMbnId;
    }

    public String getMetaInfo() {
        return mMetaInfo;
    }

    public int getSource() {
        return mSource;
    }

    public String getSourceString() {
        switch (mSource) {
        case MbnTestUtils.MBN_FROM_GOLDEN:
            return mContext.getString(R.string.source_golden);
        case MbnTestUtils.MBN_FROM_APP:
            return mContext.getString(R.string.source_application);
        case MbnTestUtils.MBN_FROM_PC:
            return mContext.getString(R.string.source_pc);
        default:
            return "Unknown";
        }
    }

    @Override
    public String toString() {
        return "Meta Info:" + mMetaInfo + " Mbn ID:" + mMbnId + " Source:" + getSourceString();
    }

}
