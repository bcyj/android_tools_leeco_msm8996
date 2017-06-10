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

import android.content.Context;
import android.os.Handler;
import android.os.Message;

public class VTLoopBackConnection implements IVTConnection {

    Context mCtx;
    Handler mHandler;

    public VTLoopBackConnection(Context ctx) {
        // TODO Auto-generated constructor stub
        mCtx = ctx;
    }

    public void acceptCall() {

    }

    public void endSession() {

    }

    public void rejectSession() {

    }

    public void fallBack() {

    }

    public void setHandler(Handler h) {

    }

    public void clear() {
        // TODO Auto-generated method stub

    }

}
