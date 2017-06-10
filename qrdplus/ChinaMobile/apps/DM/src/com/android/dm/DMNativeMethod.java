/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.dm;

import com.android.dm.DmAlertDialog;
import com.android.dm.DmService;
import com.android.dm.DmService;
import com.android.dm.transaction.DMTransaction;

public class DMNativeMethod {
    public DMNativeMethod() {
    }

    static {
        // The runtime will add "lib" on the front and ".o" on the end of
        // the name supplied to loadLibrary.
        System.loadLibrary("dmjni");
    }

    // DM start
    public static native boolean JMMIDM_IsDmRun();

    public static native boolean JMMIDM_ExitDM();

    public static native int JMMIDM_StartVDM(int type, byte[] msg_body, int msg_size);

    public static native void JsaveDmJniInterfaceObject(DmJniInterface dmsevice);

    public static native void Jhs_dm_mmi_confirmationQuerycb(boolean iscontinue);

    public static native short JVDM_notifyNIASessionProceed();

    public static native short JcopySourceDataToBuffer(short id, byte[] source, long size);

    public static native byte[] JcopyDataForSending(short id);

    public static native void JstepDataReceive();

    public static native void JnotifyCommBroken();

    public static native int JgetWorkSpaceId();

    public static native byte[] JgetReplaceServerAddress();

    public static native void JsaveDmTransactionObject(DMTransaction transaction);

    public static native void Jparse_x_syncml_hmac(byte[] headbody);

    // DM end

}
