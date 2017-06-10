/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.nfc;

public interface QHostListener
{
    public void onCardEmulationAidSelected(byte[] dataBuf);
    public void onRfInterfaceDeactivated();
}
