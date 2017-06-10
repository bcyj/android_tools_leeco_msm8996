/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

 /*
 * Copyright (C) 2013 The Android Open Source Project
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
 * limitations under the License
 */

package com.borqs.videocall;

//import com.android.services.telephony.common.Call;

import java.util.ArrayList;



/**
 * Presenter for the Incoming call widget.
 */
public class AnswerPresenter extends Presenter<AnswerPresenter.AnswerUi> {

	/* VideoCallScreen */
	VideoCallScreen mScreen;
	/* VideoCall Application */
	VideoCallApp mApp;


	public void onAnswer() {
		Log.d(this,"OnAccept...");
		final VideoCallApp mApp = VideoCallApp.getInstance();
		if( mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
		    Log.d(this, "App in idle status, just return");
		    return;
		}

		int flipState = mApp.getInstance().GetFlipState();
		mApp.answerCall();
	}

	public void onDecline() {
		Log.d(this, "onRejectCall()");
                final VideoCallApp mApp = VideoCallApp.getInstance();
		if( mApp.getInstance().getStatus() == VideoCallApp.APP_STATUS_IDLE){
		    Log.d(this, "App in idle status, just return");
		    return;
		}
		if (mApp.mAudioRecordStatus == VideoCallApp.AUDIO_RECORD_STATE.STARTING)
		{
		    Log.d(this, "Audio recorder is starting, ignore this end call click.");
		    return;
		}

		mApp.doRejectCall();
	}

	public void onText() {
	    Log.d(this, "onText...");
	    if (getUi() != null) {
		getUi().showMessageDialog();
	    }
	}

        public void rejectCallWithMessage(String message) {
	    Log.d(this, " rejectCallWithMessage sendTextToDefaultActivity()...");
	    final VideoCallApp mApp = VideoCallApp.getInstance();
	    onDismissDialog();
        }

	public void onDismissDialog() {
        }

    interface AnswerUi extends Ui {
        public void showAnswerUi(boolean show);
        public void showVideoButtons();
        public void showTextButton(boolean show);
        public void showMessageDialog();
        public void configureMessageDialog(ArrayList<String> textResponses);
    }
}
