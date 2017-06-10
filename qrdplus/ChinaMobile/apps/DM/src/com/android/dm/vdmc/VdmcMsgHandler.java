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

package com.android.dm.vdmc;

import android.os.Handler;
import android.os.Message;
import android.widget.Toast;
import com.android.dm.DmService;
import com.android.dm.R;

public class VdmcMsgHandler extends Handler {

    /*
     * (non-Javadoc)
     * @see android.os.Handler#handleMessage(android.os.Message) The
     * VdmcMsgHandler enables us to deliver messages and to handle them by the
     * Handler's handleMessage (VdmcMsgHandler#handleMessage) function.
     * MySessionObserver is sending messages to VdmcMsgHandler which handles
     * them.
     */
    public void handleMessage(Message msg)
    {
        super.handleMessage(msg);

        String statusStr = new String();

        switch (msg.what) {
        /*
         * case(MSG_TYPE_DOWNLOADING): // msg#arg1 is the current number of
         * bytes of the update package // msg#arg2 is the size update package
         * (in bytes) if(msg.arg1 == msg.arg2) {
         * Vdmc.getInstance()._progressDialog.show();
         * Vdmc.getInstance()._progressDialog.dismiss(); } else {
         * Vdmc.getInstance()._progressDialog.display(msg.arg1, msg.arg2); }
         * break; case (MSG_TYPE_DL_STARTED): statusStr = "DL Session Started";
         * break; case (MSG_TYPE_DL_ENDED)://on Session State = 'Completed'
         * Vdmc.getFumoHandler().onDownloadComplete(); statusStr =
         * "DL Session Ended"; break; case (MSG_TYPE_DL_ABORTED): statusStr =
         * "DL Session Aborted"; break;
         */
            case (MSG_TYPE_DM_STARTED):
                statusStr = Vdmc.getAppContext().getString(R.string.dm_session_started_tip);
                break;
            case (MSG_TYPE_DM_ENDED):
                statusStr = Vdmc.getAppContext().getString(R.string.dm_session_ended_tip);
                // exit vDM task
                Vdmc.getInstance().stopVDM();
                break;
            case (MSG_TYPE_DM_ABORTED):
                statusStr = Vdmc.getAppContext().getString(R.string.dm_session_aborted_tip);
                // exit vDM task
                Vdmc.getInstance().stopVDM();
                break;
        }

        if (msg.what != MSG_TYPE_DOWNLOADING) {
            // Vdmc.getInstance().displayMsg(statusStr);
            if (DmService.getInstance().isDebugMode())
            {
                ShowMessage(statusStr);
            }
        }
    }

    private Toast mToast;

    private void ShowMessage(CharSequence msg)
    {
        if (mToast == null)
            mToast = Toast.makeText(Vdmc.getAppContext(), msg, Toast.LENGTH_LONG);
        mToast.setText(msg);
        mToast.show();
    }

    // //////////////////////////
    // Data members
    // /////////////////////////
    // status types
    // DL
    public static final int MSG_TYPE_DOWNLOADING = 0;
    public static final int MSG_TYPE_DL_STARTED = 1;
    public static final int MSG_TYPE_DL_ENDED = 2;
    public static final int MSG_TYPE_DL_ABORTED = 3;

    // DM
    public static final int MSG_TYPE_DM_STARTED = 4;
    public static final int MSG_TYPE_DM_ENDED = 5;
    public static final int MSG_TYPE_DM_ABORTED = 6;
}
