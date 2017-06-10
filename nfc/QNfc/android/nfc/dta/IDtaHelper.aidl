/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package android.nfc.dta;

import android.nfc.NdefMessage;

/**
 * @hide
 */
interface IDtaHelper
{
    boolean in_dta_mode();
    void dta_set_pattern_number(int pattern);
    int dta_get_pattern_number();
    boolean nfcDeactivate(int deactivationType);
    void startLlcpCoEchoServer(String serviceNameIn, String serviceNameOut);
    void stopLlcpCoEchoServer();
    void startLlcpClEchoServer(String serviceNameIn, String serviceNameOut);
    void stopLlcpClEchoServer();

    String get_text_from_ndef(in NdefMessage ndefMessage);
    int snep_client_create(in String serviceName);
    boolean snep_client_connect(int handle);
    boolean snep_client_put(int handle, in NdefMessage ndefMessage);
    NdefMessage snep_client_get(int handle, in NdefMessage ndefMessage);
    void snep_client_close(int handle);
    int snep_server_create(in String serviceName, boolean enableExtendedDTAServer);
    void snep_server_close(int handle);
}
