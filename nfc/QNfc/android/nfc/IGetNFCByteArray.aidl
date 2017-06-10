/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package android.nfc;

interface IGetNFCByteArray {
    byte[] GetNfccInfo();
    byte[] CollectRamDump(int addr,int len);
    void doPrbsOn(int tech, int rate);
    void doPrbsOff();
}
