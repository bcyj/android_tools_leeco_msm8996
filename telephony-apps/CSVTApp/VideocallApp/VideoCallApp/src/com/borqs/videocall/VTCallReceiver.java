/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.provider.CallLog;
import android.provider.CallLog.Calls;
import android.util.Log;
import android.widget.Toast;
import android.text.TextUtils;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.CallerInfo;
import com.android.internal.telephony.PhoneConstants;
import android.telecom.PhoneAccountHandle;

public class VTCallReceiver extends BroadcastReceiver {

	static final String INTENT_CALL = "borqs.intent.action.NEW_CSVT_RINGING_CONNECTION";
	static final String ACTION_CLEAR_MISSED_VTCALLS =
			"com.borqs.videocall.action.clearMissedVTCalls";
	static final String TAG = "VTCallReceiver";

	static final String CALLER_NUMBER_KEY = "call_number_key";
	static final String CALLER_IS_LOCK_MODE = "call_is_lock_mode";
	private Context mCtx = null;
	static final int VT_CALLLOG_CALLTYPE = 1;
	public synchronized void onReceive(Context context, Intent intent) {
		if (MyLog.DEBUG) MyLog.d(TAG, "Receive calling " + intent.getAction());
                VideoCallApp mApp = VideoCallApp.getInstance();
		if( intent.getAction().equals(INTENT_CALL)){

			String strAddr;
			boolean bLockMode;

			mCtx = context;

			if(VTCallUtils.isDmLocked())
			{
				new VTPhoneConnection(mCtx, VTCallUtils.getCsvtService()).fallBack();
				return;
			}

			//close waiting fallback dialog
			Intent broadcastIntent = new Intent(FallBackAlertDialog.ACTION_DISMISS_WAITING_FALLBACK_DIALOG);
			mApp.sendBroadcast(broadcastIntent);

			/* get caller number for intent launcher */
			//TODO: make sure underlayer follow this way
			strAddr = intent.getStringExtra("connectionAddress");
			bLockMode = intent.getBooleanExtra(VideoCallApp.INTENT_EXTRA_IS_LOCKED_MODE, false);
            if (!VTCallUtils.isVoiceIdle()) {
                Log.d("Video*****", "voice call is not idle");
                new VTPhoneConnection(mCtx, VTCallUtils.getCsvtService()).fallBack();
                AddBlockNumberToCallLog(strAddr);
                Toast.makeText(mCtx, R.string.voice_ongoing_not_answer_vt, Toast.LENGTH_LONG)
                        .show();
                return;
            } else if (VTCallUtils.isVTActive()) {
                Log.d("Video*****", "voice call is active");
                new VTPhoneConnection(mCtx, VTCallUtils.getCsvtService()).rejectSession();
                AddBlockNumberToCallLog(strAddr);
                Toast.makeText(mCtx, R.string.vt_ongoing_reject_vt, Toast.LENGTH_LONG).show();
                return;
            }

			//check if current call-in number is blocked in black list
			if (!VTCallUtils.isBlockedNumber( mCtx, strAddr)) {

		                // Start VTService before launch VideoCallScreen to increate the process priority.
		                // Avoid the system kill the com.borqs.videocall process when the system resource is low.
				if (mApp.mVTService == null) {
		                    mApp.startVTService();
		                }

				if (MyLog.DEBUG) MyLog.d(TAG, "launch screen: strAddr: " + strAddr);
				mApp.launchVTScreen( false,
						bLockMode, strAddr, VideoCallApp.CALL_FROM_TTY,
						new VTPhoneConnection( mCtx, VTCallUtils.getCsvtService()));

			} else {

				Object args[] = new String[] { strAddr };
				String text = mCtx.getResources().getString(
						R.string.numberBlocked, args);

				//Toast.makeText(mCtx, text,	Toast.LENGTH_SHORT).show();
				new VTPhoneConnection(mCtx, VTCallUtils.getCsvtService()).endSession();
				AddBlockNumberToCallLog(strAddr);
				return;
			}
                        mApp.startIncomingCallQuery(strAddr, false);

			/* modified end */
		}else if( intent.getAction().equals( VideoCallApp.INTENT_ACTION_STOP_VTCALL)){
			if (MyLog.DEBUG) MyLog.d(TAG, "received end call intent, to end call");
			mApp.mHandler.obtainMessage( VideoCallApp.MSG_END_CALL).sendToTarget();
		}
		else if( intent.getAction().equals( VideoCallApp.INTENT_ACTION_ANSWER_VTCALL)){
			if (MyLog.DEBUG) MyLog.d(TAG, "received answer call intent, to answer a call");
			mApp.mHandler.obtainMessage( VideoCallApp.MSG_ANSWER_CALL).sendToTarget();
		}
		else if( intent.getAction().equals( VideoCallApp.INTENT_ACTION_SILENCERING_VTCALL)){
			if (MyLog.DEBUG) MyLog.d(TAG, "received silence ringer intent, to silence the ringer");
			mApp.mHandler.obtainMessage( VideoCallApp.MSG_SILENCERING_VTCALL).sendToTarget();
		}
		else if (intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)){
			//VTCallUtils.createCsvtService(mApp);
		}
		else if (intent.getAction().equals(VideoCallApp.INTENT_ACTION_CLEAR_MISSED_VTCALL)) {
			if (MyLog.DEBUG)
				MyLog.d(TAG, "received clear missed vtcalls intent, to clear notification");
			mApp.mHandler.obtainMessage(VideoCallApp.MSG_CLEAR_MISSED_VTCALL,
					intent.getBooleanExtra("update_calllog", true)).sendToTarget();
		}
	}

	void AddBlockNumberToCallLog(String strAddr)
	{
		if (MyLog.DEBUG) MyLog.d(TAG, "Add blocknum to calllog:"+ strAddr);
		CallerInfo ci = new CallerInfo();
		ci.phoneNumber = strAddr;
		ci.name = "";

		final String _number   = strAddr;
		final int  _presentation ;


		if(TextUtils.isEmpty(_number)){
			_presentation = PhoneConstants.PRESENTATION_UNKNOWN ;

		}else{

			_presentation = PhoneConstants.PRESENTATION_ALLOWED;
		}
             final int _type = CallLog.Calls.MISSED_TYPE;
             final long _start = System.currentTimeMillis();
             final int _duration =  0 ;// in seconds
             final int features = 1; //video
             final Long dataUsage = null;
             final PhoneAccountHandle accountHandle = null ;//call.getAccountHandle();

             try
	      {
	          Calls.addCall( ci, mCtx, _number, _presentation , _type, features,
                         accountHandle,_start,_duration,dataUsage);
             } catch (Exception e) {
                Log.e(TAG, "Exception occurs while add block Number to CallLog."
                        + e.getMessage());
             }
    }
}
