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

import android.util.Log;
import android.util.Config;

public class MyLog {
    // public static final boolean DEBUG = Config.DEBUG;
    public static final boolean DEBUG = true;

    public static final void d(String tag, String msg) {
        // if (Log.isLoggable(tag, Log.DEBUG)) Log.d(tag, msg);
        Log.d(tag, msg);
    }

    public static final void v(String tag, String msg) {
        // if (Log.isLoggable(tag, Log.VERBOSE)) Log.v(tag, msg);
        Log.v(tag, msg);
    }
}
