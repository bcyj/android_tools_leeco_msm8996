/*******************************************************************************
    Copyright (c) 2010,2011 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

import android.content.Context;
import android.os.Handler;

public abstract class AtCmdBaseHandler extends Handler implements AtCmdHandler {

    protected Context mContext;

    public AtCmdBaseHandler(Context c) {
        mContext = c;
    }

    protected Context getContext() {
        return mContext;
    }
}
