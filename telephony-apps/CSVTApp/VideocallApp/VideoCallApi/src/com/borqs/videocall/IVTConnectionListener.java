/* BORQS Software Solutions Pvt Ltd. CONFIDENTIAL
 * Copyright (c) 2012 All rights reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by BORQS Software
 * Solutions Pvt Ltd. No part of the Material may be used,copied,
 * reproduced, modified, published, uploaded,posted, transmitted,
 * distributed, or disclosed in any way without BORQS Software
 * Solutions Pvt Ltd. prior written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by BORQS Software Solutions Pvt Ltd. in writing.
 *
 */

package com.borqs.videocall;

interface IVTConnectionListener {
    // BELOW constant should be provided at where IVideoTelehony defined
    // Call Notify Category
    public final static int VTCALL_CONNECT_CONNECTED = 0;
    public final static int VTCALL_CONNECT_REJECTED = 1;
    public final static int VTCALL_CONNECT_FALLBACK = 2;
    public final static int VTCALL_CONNECT_FAILED_UNKNOWN = 3;
    public final static int VTCALL_CONNECT_END = 4;

    public final static int VTCALL_STATE_OFFHOOK = 5;

    public void OnVTStateChanged(int nResult);

    public void OnVTConnectResult(int nResult);
}
