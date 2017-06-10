/*
 * Copyright (c) 2011, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 */
package com.android.qworldclock;

public interface TickListener {
    public void updateTime();
    public boolean register();
    public boolean unregister();
}
