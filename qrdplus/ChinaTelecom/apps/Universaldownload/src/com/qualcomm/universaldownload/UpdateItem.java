/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

public class UpdateItem {
    private String mCapabilityName;
    private String mVersion;
    private boolean checked;

    public UpdateItem(String capabilityName, String version) {
        this.mCapabilityName = capabilityName;
        this.mVersion = version;
        this.checked = true;
    }

    public String getCapabilityName() {
        return mCapabilityName;
    }

    public String getVersion() {
        return mVersion;
    }

    public void setVersion(String version) {
        mVersion = version;
    }

    public boolean isChecked() {
        return checked;
    }

    public void check(boolean checked) {
        this.checked = checked;
    }
}
