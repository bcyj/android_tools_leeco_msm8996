/******************************************************************************
 * @file    EmInfo.java
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 *******************************************************************************/
package com.qualcomm.ftm.em;

import java.io.Serializable;

public class EmInfo implements Serializable {

    public String mName;
    public long mValue;
    public int mSub;
    public int mIsNotImportant;

    public EmInfo(String name, long value, int sub, int isNotImportant) {
        this.mName = name;
        this.mValue = value;
        this.mSub = sub;
        this.mIsNotImportant = isNotImportant;
    }
    public EmInfo(String name, long value, int sub) {
        this(name, value, sub, 0);
    }
}
