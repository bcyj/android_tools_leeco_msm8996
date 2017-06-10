/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 *
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.borqs.videocall;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.app.WallpaperManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.media.MediaRecorder;
import android.media.ToneGenerator;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.Process;
import android.provider.Settings;
import android.provider.Settings.System;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.text.InputType;
import android.os.SystemProperties;
import android.database.Cursor;
import com.android.internal.telephony.CallerInfo;

import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.HashMap;
//import com.android.phone;
import com.android.internal.widget.SlidingTab;
import android.media.AudioManager;
import android.graphics.SurfaceTexture;
import android.view.TextureView;
import android.content.BroadcastReceiver;
import android.util.Log;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

/**
 * Default launcher application.
 */
public final class VideoCallScreen extends Activity implements TextureView.SurfaceTextureListener {

	static final String TAG = "VT/VideoCallScreen";
	static private int PROGRESS_BAR_MAX_VAL = 100;
	// ids for create dialog
	static private final int SHOW_FALLBACK_ALERT = 1;
	// vt App
	VideoCallApp mApp;
	// vt service
	VTService mVTService;

	// //////UI objects
	private AnswerFragment mAnswerFragment;
	// vtpanel manager
	public VTPanelComponentsMgr mPanelManager;
	// vt sliding panel manager
	private VTSlidingCardManager mSlidingManager;
//	View mSlidingGroupView;
	View mBottomConnectingView;
//	View mBlackSpaceView;
	//ImageView mRingingMuteView;

	// SMS Panel
	private static final String REJECT_SMS_NOT_SET = "reject_sms_not_set";
	private static final String ACTION_CLEAR_DIALER_NUMBER = "com.android.phone.action.CLEAR_TOUCH_DIALER_NUMBER";
	private LinearLayout mSMSRejectPanel;
	private TextView mSMSContent;

	//private CheckBox mCheckBoxSMS;
	private CheckBox mChkBoxSMS;
	//private boolean mIsSMSChked = false;
	private ImageButton mBtnSMSSelect;
	private TextView mTxtSMSSelect;
	private TextView mRecordTimeView;
	private AlertDialog mRejectSmsDialog;
	private int mCurrentView = 0;
	private int mPreviousSelectedView = 0;
	private ArrayList<CharSequence> scv;
	private CharSequence[] mTempScv;
	// camera
	private android.hardware.Camera.Parameters mParameters;

	// action panel and buttons
    // private VTPanelActionButton mActionPanel;

	private ImageView btnRecord;
	private ImageView btnSwitch;
	private TextView mTxtRecord;
	private TextView mTxtSwitch;
	private ImageView btnShowKeyboard;
	private ImageView btnMute;
	private TextView mTxtMute;
	private Button mChkFallBack;
	private boolean isVoiceCall = false;
	// surface
	//private SurfaceView mLocalSurface;
	private TextureView mLocalSurface;
	private SurfaceHolder mLocalHolder;
	//private SurfaceView mRemoteSurface;
	private TextureView mRemoteSurface;
	private EditText mKeypadinput;
	private SurfaceHolder mRemoteHolder;

	// keyboard, TODO
	private VideoCallKeyPad mKeyboard;

	// progress dialog for start audio record
	private ProgressDialog progressDlg = null;
	private boolean isProgressing = false;


	private boolean hasPause = false;

	boolean hasStop = false;
	// Input
	String mInputPhoneURL;
	boolean mDialOrAnswer = true;
	int mLaunchMode = VideoCallApp.CALL_FROM_TTY;
	String mDeviceName;

	//DTMF Tone generator
	private ToneGenerator mToneGenerator;
	private Object mToneGeneratorLock = new Object();
	// indicate if we want to enable the local tone playback.
	private boolean mLocalToneEnabled;
	//Dtmf tone duration
	private static final int DTMF_DURATION_MS = 240;
	/** Hash Map to map a character to a tone*/
	private static final HashMap<Character, Integer> mToneMap =
			new HashMap<Character, Integer>();
	/** Set up the static maps*/
	static {
		// Map the key characters to tones
		mToneMap.put('1', ToneGenerator.TONE_DTMF_1);
		mToneMap.put('2', ToneGenerator.TONE_DTMF_2);
		mToneMap.put('3', ToneGenerator.TONE_DTMF_3);
		mToneMap.put('4', ToneGenerator.TONE_DTMF_4);
		mToneMap.put('5', ToneGenerator.TONE_DTMF_5);
		mToneMap.put('6', ToneGenerator.TONE_DTMF_6);
		mToneMap.put('7', ToneGenerator.TONE_DTMF_7);
		mToneMap.put('8', ToneGenerator.TONE_DTMF_8);
		mToneMap.put('9', ToneGenerator.TONE_DTMF_9);
		mToneMap.put('0', ToneGenerator.TONE_DTMF_0);
		mToneMap.put('#', ToneGenerator.TONE_DTMF_P);
		mToneMap.put('*', ToneGenerator.TONE_DTMF_S);
	}


	// vtstatus
	private static final String vtstatusPath = "/data/data/com.borqs.videocall/files/vtstatus";
	private File vtstatusFile = null;
	private final int NOT_RECORDING = 1;
	private final int IS_RECORDING = 2;

	private boolean mIsLocalSurfaceCreated = false;
	private boolean mIsRemoteSurfaceCreated = false;

	private static final int MSG_BASE = 1000;
	static final int MSG_UPDATE_CALLERINFO = MSG_BASE + 1;
	static final int MSG_SERVICE_CONNECTED = MSG_BASE + 2;
	static final int MSG_SERVICE_DISCONNECTED = MSG_BASE + 3;
	static final int MSG_VT_CONNECTED = MSG_BASE + 4;
	static final int MSG_TICK_UPDATE = MSG_BASE + 5;
	static final int MSG_CAMERA_READY = MSG_BASE + 6;
	static final int MSG_CAMERA_STATE_UPDATE = MSG_BASE + 7;
	static final int MSG_USER_INPUT = MSG_BASE + 8;
	static final int MSG_FDN_POST_INIT = MSG_BASE + 9;
	static final int MSG_SPEAKER_STATUS_UPDATE = MSG_BASE + 10;
	static final int MSG_CLOSE_REP_IMAGE = MSG_BASE + 11;
	static final int MSG_SHOW_REP_IMAGE = MSG_BASE + 12;
	static final int MSG_DISMISS_DIALOG = MSG_BASE + 13;
	static final int MSG_STOP_RECORD = MSG_BASE + 14;

	private static final Uri CONTENT_URI = Uri.parse("content://settings/system");

	static boolean mChangeConfig = false;
	class SwitchCameraTask implements Runnable {
		public void run() {
			if (MyLog.DEBUG) MyLog.d(TAG, "swtich camera in the single thread.");
			onSwitchCamera();

			mScreenHandler.sendEmptyMessage(MSG_DISMISS_DIALOG);
		}
	}

	class StartAudioRecordTask implements Runnable {
		public void run() {
			if (MyLog.DEBUG) MyLog.d(TAG, "Start audio recording in the single thread.");
			try {
				mApp.startAudioRecord();
			} catch (RuntimeException exception) {
				if (MyLog.DEBUG) MyLog.d(TAG, "startAudioRecord throw exception:" + exception.toString());
				mScreenHandler.sendEmptyMessage(MSG_DISMISS_DIALOG);
				return;
			}

			if (MyLog.DEBUG) MyLog.d(TAG, "startAudioRecord complete, dismiss progress dialog");
			mScreenHandler.sendEmptyMessage(MSG_DISMISS_DIALOG);
		}
	}

	void onSwitchCamera() {
		if (MyLog.DEBUG) MyLog.d(TAG, "on switch camera");

		if( mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
			if (MyLog.DEBUG) MyLog.d(TAG, "App in idle status, just return");
			return;
		}
		if ( mApp.mCurrentVideoSource == VTManager.VideoSource.CAMERA_MAIN) {
			try {
				if (MyLog.DEBUG) MyLog.d(TAG, "switch from main camera");
				mApp.mLastVideoSource = mApp.mCurrentVideoSource;
				mApp.mCurrentVideoSource = VTManager.VideoSource.CAMERA_SECONDARY;
				mApp.mCameraSetting.resetValues(mApp.mCurrentVideoSource);
				mApp.mVTService.setVideoSource(VTManager.VideoSource.CAMERA_SECONDARY, null,mApp.mCameraSetting);
				//mSkBarContast.setMax( mApp.mCameraSetting.getSettingDegree( CameraSetting.CONSTRAST_SETTING));
				//mSkBarBright.setMax( mApp.mCameraSetting.getSettingDegree( CameraSetting.BRIGHT_SETTING));
				//mVTService.sendUserInput( mApp.MSG_SWITCH_TO_SECONDARY_CAMERA);
				if (MyLog.DEBUG) MyLog.d(TAG, "switch to secondary camera");
			} catch (Exception e) {
				// TODO: handle exception
				Log.e(TAG, e.toString());
				return;
			}
		} else if (mApp.mCurrentVideoSource == VTManager.VideoSource.CAMERA_SECONDARY) {
			try {
				if (MyLog.DEBUG) MyLog.d(TAG, "switch from secondary");
				mApp.mLastVideoSource = mApp.mCurrentVideoSource;
				mApp.mCurrentVideoSource = VTManager.VideoSource.CAMERA_MAIN;
				mApp.mCameraSetting.resetValues(mApp.mCurrentVideoSource);
				mApp.mVTService.setVideoSource(VTManager.VideoSource.CAMERA_MAIN,
						null,mApp.mCameraSetting);
				//mSkBarContast.setMax( mApp.mCameraSetting.getSettingDegree( CameraSetting.CONSTRAST_SETTING));
				//mSkBarBright.setMax( mApp.mCameraSetting.getSettingDegree( CameraSetting.BRIGHT_SETTING));
				//mVTService.sendUserInput( mApp.MSG_SWITCH_TO_MAIN_CAMERA);
				if (MyLog.DEBUG) MyLog.d(TAG, "switch to main camera");
			} catch (Exception e) {
				// TODO: handle exception
				Log.e(TAG, e.toString());
			}
		}
	}
	Handler mScreenHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			//if (MyLog.DEBUG) MyLog.d(TAG, "HandleMessage : " + msg.what);
			switch (msg.what) {
			case MSG_UPDATE_CALLERINFO:
				if (MyLog.DEBUG) MyLog.d(TAG, "update caller infor");
				updateCallInfo();
				break;
			case MSG_SERVICE_CONNECTED:
			{
				if (MyLog.DEBUG) MyLog.d(TAG, "receive Service connected message");

				mVTService = mApp.mVTService;
			}
			break;
			case MSG_SERVICE_DISCONNECTED:
			{
				mVTService = null;

			}
			break;
			case MSG_VT_CONNECTED:
				if (MyLog.DEBUG) MyLog.d(TAG, "received vt connected msg, to update UI");
				UpdateUIAfterConnecting();
				// show control buttons as a tip
				mPanelManager.switchPanelMode(VTPanel.VTPANEL_MODE_REMOTE, VTPanel.ANIMATION_DURATION_NONE);
				// update caller info for control changed
				updateCallInfo();
				break;
			case MSG_TICK_UPDATE:
				//if (MyLog.DEBUG) MyLog.d(TAG, "tick update");
				mPanelManager.setCallCardTitle(mApp.mCallTimeElapsed);
				break;
			case MSG_CAMERA_READY:
				if (MyLog.DEBUG) MyLog.d(TAG, "get camera ready notification");
				break;
			case MSG_CAMERA_STATE_UPDATE:
				if (MyLog.DEBUG) MyLog.d(TAG, "Camera state update...");
				updateCameraUI();
				break;
			case MSG_USER_INPUT:
				if (MyLog.DEBUG) MyLog.d(TAG, "Get User input notify");
				mPanelManager.setUserInput(mApp.getUserInput());
				break;
			case MSG_FDN_POST_INIT:
				if (MyLog.DEBUG) MyLog.d(TAG, "FDN post init");
				if( postOnCreateInitProcess()){
					postOnCreateCommonProcess();
				}
				break;
			case MSG_SPEAKER_STATUS_UPDATE:
				if (MyLog.DEBUG) MyLog.d(TAG, "speaker status update");
				if (mPanelManager != null)
					mPanelManager.updateSpeakerStatus();
				break;
			case MSG_CLOSE_REP_IMAGE:
				if (MyLog.DEBUG) MyLog.d(TAG, "close rep image");
				mPanelManager.closeRepImage();
				//borqs b089: in case lock screen mode
				//local surface will hide rep image
				mLocalSurface.setVisibility(View.VISIBLE);
				break;
			case MSG_SHOW_REP_IMAGE:
				if (MyLog.DEBUG) MyLog.d(TAG, "show rep image");
				mLocalSurface.setVisibility(View.INVISIBLE);
				mPanelManager.showRepImage();
				break;
			case MSG_DISMISS_DIALOG:
				/*if (mApp.mAudioRecordStatus == VideoCallApp.AUDIO_RECORD_STATE.IDLE) {
				btnRecord = (ImageView) findViewById(R.id.btn_record);
                            btnRecord.setBackgroundResource(R.drawable.cmcc_phone_vtcall_btn_icon_record);
                        }*/

				if (progressDlg != null) {
					progressDlg.dismiss();
					progressDlg = null;
				}
				isProgressing = false;

				break;
			case MSG_STOP_RECORD:
				// btnRecord = (ImageView) findViewById(R.id.btn_record);
				//	btnRecord.setBackgroundResource(R.drawable.cmcc_phone_vtcall_btn_icon_record);
				break;
			} //end of swith
		}
	};  //end of handler definition

	public void updateRecordTime(String text)
	{
		if(mRecordTimeView != null)
		{
			mRecordTimeView.setText(text);
		}
	}

	private void updateCallInfo(){
		if (MyLog.DEBUG) MyLog.v(TAG, "updateCallInfo...");
		/* set name view */
		String strName = mApp.getCallerName();
		String strNumber = mApp.getCallerNumber();
		CallerInfo info =  mApp.getCallerInfo();
		String cityName = "";
        if(info != null){
		    info.updateGeoDescription(this, strNumber);
		    cityName = info.geoDescription;
        }
		// name and number can be null
		if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING
				&& mSlidingManager!=null)
			mSlidingManager.updateCallerInfo((String)getText(R.string.answerin_popup_call_card_title),
			        strNumber, cityName, strName);
		if(VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING
				&& mSlidingManager!=null)
			mSlidingManager.updateCallerInfo((String)getText(R.string.dailout_popup_call_card_title),
			        strNumber, cityName, strName);
		else
			mPanelManager.updateCallInfoUI(strName, strNumber);
	}

	protected void onShowKeyboard() {
		if (MyLog.DEBUG) MyLog.d(TAG, "onShowKeypad...");
		// TODO
		// hide action buttons panel
		// View mainView = findViewById(R.id.vt_plate);

		if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_LOCAL) {
			mPanelManager.switchPanelMode(VTPanel.VTPANEL_MODE_KEYPAD_LOCAL);
		} else if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_REMOTE) {
			mPanelManager.switchPanelMode(VTPanel.VTPANEL_MODE_KEYPAD_REMOTE);
		} else if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_REMOTE_ONLY) {
			mPanelManager.switchPanelMode(VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY);
		}

		mKeyboard.setVisibility(View.VISIBLE);
		//mActionPanel.setVisibility(View.GONE);
		// mainView.setBackgroundResource(R.drawable.cmcc_vtcall_plate_bg_small);
	}

	void onMute() {
		if (MyLog.DEBUG) MyLog.d(TAG, "onMute, mIsMute: " + mApp.mIsMute);

		if( mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
			if (MyLog.DEBUG) MyLog.d(TAG, "App in idle status, just return");
			return;
		}
		btnMute = (ImageView) findViewById(R.id.btn_mute);
		mTxtMute = (TextView) findViewById(R.id.textMute);
		// update button UI
		if( (mVTService == null) || (mApp.mEngineState < VideoCallApp.ENGINE_STATE_CONNECTED))
		{
			if (!"1".equals(SystemProperties.get("ro.kernel.qemu")))
				return;
		}
		else
		{
			int flipstate = mApp.GetFlipState();
			if((flipstate==1||flipstate==2)&&((mApp.mIsHardwareH324MStack&&!VTCallUtils.checkHeadsetStatus() && !mApp.mAudioManager.isBluetoothScoOn())||
					(!mApp.mIsHardwareH324MStack&&!VTCallUtils.checkHeadsetStatus())))
			{	if(mApp.isMute())
				return;
			}

			if (MyLog.DEBUG) MyLog.d(TAG, "mIsMute: " + mApp.mIsMute +"  mApp.mIsLandscape:"+mApp.mIsLandscape);
			if( false == mApp.getVTServiceInterface().setMute( !mApp.isMute(),mVTService))
				return;
		}
		if (!mApp.isMute()) {
			// bkg turn blue
			btnMute.setImageResource(R.drawable.ic_micro_phone_presses);
			mTxtMute.setText(R.string.button_unmute);
			mApp.mIsMute = true;
		} else {
			// bkg turn black
			btnMute.setImageResource(R.drawable.ic_micro_phone);
			mTxtMute.setText(R.string.button_mute);
			mApp.mIsMute = false;
		}

	}

	void onRecordVideo() {
		// TODO
	}

	void onStopRecordVideo() {
		// TODO
	}

	boolean onCapturePhoto() {
		if (MyLog.DEBUG) MyLog.d(TAG, "onCapturePhoto");

		String pictures_folder = "/Pictures/VideoCall";
		// tmp path for test
		String path = Environment.getExternalStorageDirectory().getAbsolutePath() + pictures_folder;

		int sdcardStatus = VTCallUtils.checkStorage();

		if(VTCallUtils.SDCARD_OK != sdcardStatus){
			VTCallUtils.showStorageToast(sdcardStatus, this);
			return false;
		}
		if( (mVTService == null) || (mApp.mEngineState < VideoCallApp.ENGINE_STATE_CONNECTED) )
		{
			if ("1".equals(SystemProperties.get("ro.kernel.qemu")))
			{
				try
				{
					if (MyLog.DEBUG) MyLog.d(TAG, "onCapturePhoto on emulator");
					Bitmap bm =Bitmap.createBitmap(1,1,Bitmap.Config.ALPHA_8);
					VTServiceCallUtils.storeImageToFile(this, bm, path);
				}
				catch( Exception e)
				{
					Log.e(TAG,"onCapturePhoto on emulator Exception:"+e.getMessage());
				}
				return true;
			}

			return false;
		}
		mApp.getVTServiceInterface().captureScreen(0/*TODO no setting*/, path,mVTService);
		if (MyLog.DEBUG) MyLog.d(TAG, "finish capturePhoto");

		return true;
	}

	private void UpdateUIAfterConnecting() {

		if (MyLog.DEBUG) MyLog.d(TAG, "UpdateUIAfterConnecting...");

		// Update Call card: remove all except for the texts in call card
		// View mainView = findViewById(R.id.vt_plate);
		//mainView.setBackgroundResource(R.drawable.cmcc_vtcall_plate_bg);

		View call_card = mPanelManager.getCallCard();
		call_card.setBackgroundDrawable(null);

		if (mSlidingManager != null) {
			Log.e(TAG,"mSlidingManager.dismissPopup();");
			mSlidingManager.dismissPopup();
		}
		/*if(mSlidingGroupView!=null)
		{
			mSlidingGroupView.setVisibility(View.GONE);
		}
		if(mBottomConnectingView!=null)
		{
			mBottomConnectingView.setVisibility(View.GONE);
		}
		if(mBlackSpaceView!=null)
		{
			mBlackSpaceView.setVisibility(View.GONE);
		}*/
		if (mAnswerFragment != null)
		{
			mAnswerFragment.getView().setVisibility(View.GONE);
		}
		// Update elements in call screen
		mSMSRejectPanel.setVisibility(View.GONE);

	}

	public void onAccept() {
		if (MyLog.DEBUG) MyLog.d(TAG, "OnAccept...");
		if (mDialOrAnswer) // not an answer
			return;

		if( mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
			if (MyLog.DEBUG) MyLog.d(TAG, "App in idle status, just return");
			return;
		}

		if ( VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING &&
				getFallBackChkState() == true) {
			// fall back in lock mode
			onFallBackClicked();
			return;
		}
		int flipState = mApp.GetFlipState();
		//        	 if((flipState==1 || flipState ==2)&& ((mApp.mIsHardwareH324MStack&&!VTCallUtils.checkHeadsetStatus() && !mApp.mAudioManager.isBluetoothScoOn())||
		//             		(!mApp.mIsHardwareH324MStack&&!VTCallUtils.checkHeadsetStatus())))
		//        	 {
		//        		 if(mApp.flipAlertDialog ==null || !mApp.flipAlertDialog.isShowing())
		//	            	mApp.flipAlertDialog =VTCallUtils.Alerter.doAlert(this,(String)getText(R.string.incall_error_swivel_callin));
		//            	return;
		//        	 }

		mApp.answerCall();
	}

	public void onEndCall() {
		if (MyLog.DEBUG) MyLog.d(TAG, "onEndCall()");

		if( mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
			if (MyLog.DEBUG) MyLog.d(TAG, "App in idle status, just return");
			return;
		}
		if (mApp.mAudioRecordStatus == VideoCallApp.AUDIO_RECORD_STATE.STARTING)
		{
			if (MyLog.DEBUG) MyLog.d(TAG, "Audio recorder is starting, ignore this end call click.");
			return;
		}
		mApp.doEndCall();
	}

	public void onRejectCall() {
		if (MyLog.DEBUG) MyLog.d(TAG, "onRejectCall()");

		if( mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
			if (MyLog.DEBUG) MyLog.d(TAG, "App in idle status, just return");
			return;
		}
		if (mApp.mAudioRecordStatus == VideoCallApp.AUDIO_RECORD_STATE.STARTING)
		{
			if (MyLog.DEBUG) MyLog.d(TAG, "Audio recorder is starting, ignore this end call click.");
			return;
		}
		mApp.doRejectCall();
	}

	void onCloseKeyboard() {
		if (MyLog.DEBUG) MyLog.d(TAG, "on close key board: mode: " + VideoCallApp.sPanelMode);
		if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_LOCAL) {
			mPanelManager.switchPanelMode(VTPanel.VTPANEL_MODE_LOCAL);
		} else if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_REMOTE) {
			mPanelManager.switchPanelMode(VTPanel.VTPANEL_MODE_REMOTE);
		} else if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY) {
			mPanelManager.switchPanelMode(VTPanel.VTPANEL_MODE_REMOTE_ONLY);
		}
		//View mainView = findViewById(R.id.vt_plate);
		//mainView.setBackgroundResource(R.drawable.cmcc_vtcall_plate_bg);
	};

	//FIXME: move below to panel manager
	// VTPanle.IVTPanelListener
	public void onVTPanelNotify(int nEvent) {
		if (MyLog.DEBUG) MyLog.d(TAG, "onVTPanelNotify... event: " + nEvent);
		switch (nEvent) {
		case VTPanelComponentsMgr.VTPANEL_EVENT_INFLATE_DONE:
			break;
		case VTPanelComponentsMgr.VTPANEL_EVENT_ANIMATION_START:
			break;
		case VTPanelComponentsMgr.VTPANEL_EVENT_ANIMATION_DONE:
			break;
		case VTPanelComponentsMgr.VTPANEL_EVENT_SHOW_ACTION_BUTTON:
			mKeyboard.setVisibility(View.GONE);
			//mActionPanel.setVisibility(View.VISIBLE);
			break;
		case VTPanelComponentsMgr.VTPANEL_EVENT_SHOW_KEYPAD:
			break;
		default:
			break;
		}
	}


	@Override
	public void onBackPressed() {
		return;
	}
	void updateUI( int status){
		if (MyLog.DEBUG) MyLog.d(TAG, "updateUI...sPanelMode: " + VideoCallApp.sPanelMode);
		//TODO:
		//View spaceView = findViewById(R.id.connecting_space_view);
		View fallBackView = this.findViewById(R.id.fall_back_parent);
		if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING
				|| VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING) {

			if(SystemProperties.get("apps.videocall.SlidingToast").contentEquals("true"))
			{
				if(!mApp.mIsLandscape)
					Toast.makeText(this, R.string.slidcard_prompt, Toast.LENGTH_LONG).show();
			}
			//if(VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING)
			//{
				if( fallBackView != null)
					fallBackView.setVisibility(View.GONE);
				//View chkFallback = this.findViewById(R.id.chk_fall_back);
				//TextView txtFallback = (TextView)this.findViewById(R.id.txt_Fall_Back);
//				if(chkFallback!=null)
//					chkFallback.setVisibility(View.GONE);
//				if(txtFallback!=null)
//					txtFallback.setText(R.string.local_viedeo_open)	;
			//}
		} else {
			UpdateUIAfterConnecting();
		}

		if (MyLog.DEBUG) MyLog.d(TAG, "VideoCallApp.sPanelMode: " + VideoCallApp.sPanelMode);
		//nicki mark temporarily
		if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING) {
			VideoCallApp.isLockPanel = true;
		}else{
			VideoCallApp.isLockPanel = false;
		}

		boolean isChangeMode = VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY
				|| VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_REMOTE_ONLY;
		mPanelManager.switchPanelMode(isChangeMode ? VTPanel.VTPANEL_MODE_REMOTE
				: VideoCallApp.sPanelMode, VTPanel.ANIMATION_DURATION_NONE);

		if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_REMOTE
				|| VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_LOCAL) {
			if (MyLog.DEBUG) MyLog.d(TAG, "show key board");
			mKeyboard.setVisibility(View.VISIBLE);
			//mActionPanel.setVisibility(View.GONE);
			// View mainView = findViewById(R.id.vt_plate);
			//mainView.setBackgroundResource(R.drawable.cmcc_vtcall_plate_bg_small);
		}

		//   btnRecord = (ImageView) findViewById(R.id.btn_record);
		//   mTxtRecord = (TextView) findViewById(R.id.txtRecord);
		btnSwitch = (ImageView) findViewById(R.id.btn_switch);
		mTxtSwitch = (TextView) findViewById(R.id.txtSwitch);
		//  if(SystemProperties.get("vt.record").contentEquals("disabled"))
		//  {
		// 	 btnRecord.setVisibility(View.GONE);
		//          mTxtRecord.setVisibility(View.GONE);
		btnSwitch.setVisibility(View.VISIBLE);
		mTxtSwitch.setVisibility(View.VISIBLE);
		//  }
		if( mApp.mAudioRecordStatus == VideoCallApp.AUDIO_RECORD_STATE.STARTED){
			mPanelManager.showStartRecordAudioState();
			//   btnRecord.setBackgroundResource(R.drawable.cmcc_phone_vtcall_btn_icon_record_hl);
		}

		// update Mute UI
		btnMute = (ImageView) findViewById(R.id.btn_mute);
		mTxtMute = (TextView) findViewById(R.id.textMute);
		if (MyLog.DEBUG) MyLog.d(TAG, " updateUI mIsMute: " + mApp.mIsMute +"  mApp.mIsLandscape:"+mApp.mIsLandscape);
		if (!mApp.isMute()) {
			btnMute.setImageResource(R.drawable.ic_micro_phone);
			mTxtMute.setText(R.string.button_mute);
		} else {
			btnMute.setImageResource(R.drawable.ic_micro_phone_presses);
			mTxtMute.setText(R.string.button_unmute);
		}

		updateCameraUI();

	}
	boolean getFallBackChkState() {

		return isVoiceCall;
	}

	private void updateCameraUI(){

		if (MyLog.DEBUG) MyLog.d(TAG, "updateCameraUI..");

		ImageView closeImg = (ImageView)findViewById(R.id.img_close_camera);
		TextView closeImgTxt = (TextView)findViewById(R.id.txtClose);
		//        ImageView switchImg = (ImageView)findViewById(R.id.img_switch_camera);
		ImageView displayImg = (ImageView)findViewById(R.id.img_set_camera);
		ImageView switchImg1 = (ImageView)findViewById(R.id.btn_switch);
		FrameLayout  mBtnSwitch = (FrameLayout)findViewById(R.id.btnSwitch);
		if(closeImg == null)
			return;
		if( mApp.mCurrentVideoSource == VTManager.VideoSource.CAMERA_NONE){
			// change to open camera
			if (MyLog.DEBUG) MyLog.d(TAG, "to close camera,update UI");
			closeImg.setImageResource(R.drawable.ic_block_camera_presses);
			closeImgTxt.setText(R.string.open_camera_option);
			//  switchImg.setImageResource(R.drawable.ic_switch_camera);
			switchImg1.setImageResource(R.drawable.ic_switch_camera);
			displayImg.setImageResource(R.drawable.ic_settings_diss);
			mPanelManager.disableControlCamera();
			mBtnSwitch.setEnabled(false);
		} else {
			// change to close camera
			if (MyLog.DEBUG) MyLog.d(TAG, "to open camera,update UI");
			closeImg.setImageResource(R.drawable.ic_block_camera);
			closeImgTxt.setText(R.string.close_camera_option);
			// switchImg.setImageResource(R.drawable.ic_switch_camera);
			switchImg1.setImageResource(R.drawable.ic_switch_camera);
			displayImg.setImageResource(R.drawable.ic_settings);
			mPanelManager.enableControlCamera();
			mBtnSwitch.setEnabled(true);
		}
	}



	private boolean initFromIntentInfo(){
		Intent intent = getIntent();
		if (MyLog.DEBUG) MyLog.d(TAG, "init from intent infro, launch by action," + intent.getAction());

		if(intent.getAction().equals( VideoCallApp.TMP_INTENT_ACTION_LAUNCH_VIDEOCALLSCREEN ) ||
				intent.getAction().equals( VideoCallApp.INTENT_ACTION_LAUNCH_VIDEOCALLSCREEN )){
			if (MyLog.DEBUG) MyLog.d(TAG, "to get extra data");
			// Titank: get the input parameters by intent
			mDialOrAnswer = intent.getBooleanExtra(
					VideoCallApp.INTENT_EXTRA_CALL_OR_ANSWER, false);
			mInputPhoneURL = intent.getStringExtra(
					VideoCallApp.INTENT_EXTRA_PHONE_URL);
			mLaunchMode = intent.getIntExtra(
					VideoCallApp.INTENT_EXTRA_LAUNCH_MODE, 1);

			if (mInputPhoneURL == null || TextUtils.isEmpty(mInputPhoneURL)){
				mInputPhoneURL = this.getString(R.string.unknown_number);
			}

			if (MyLog.DEBUG) MyLog.d(TAG, "launch mode: " + mLaunchMode + " mDialOrAnswer: " + mDialOrAnswer);
			if (MyLog.DEBUG) MyLog.v(TAG, "mInputPhoneURL: " + mInputPhoneURL + ", launchmode"
					+ mLaunchMode + ",iscall," + mDialOrAnswer);

			//Special deal with the case that fake activity launched by system after a crash
			if( mDialOrAnswer == false && mApp.mConnection == null){
				if (MyLog.DEBUG) MyLog.d(TAG, "FAKE activity launching, just finish my self.");
				return false;
			}

			// The startVTService design has been changed, start VTService before launch VideoCallScreen in VTCallReceiver if it is incoming call.
			// So the VideoCallScreen is not created when the VTService connected, VideoCallApp won't send the MSG_SERVICE_CONNECTED.
			// Here assign the VTService manually.
			if (mDialOrAnswer == false) {
				mVTService = mApp.mVTService;
			}
		} else {
			if (MyLog.DEBUG) MyLog.d(TAG, "to get URI data.");
			mInputPhoneURL = null;
			Uri uri = getIntent().getData();
			if (uri != null) {
				String prefix = new String ("tel");
				if (prefix.equals(uri.getScheme())) {
					mInputPhoneURL = uri.getSchemeSpecificPart();
				}
			}
			mLaunchMode = 1;  //always tty under this case
			mDialOrAnswer = true; //always dial out under this case
		}

		if( mDialOrAnswer){

			//back door for test only, FIXME
			if (mInputPhoneURL != null) {
				if(mInputPhoneURL.compareTo("#00800#") == 0){
					mLaunchMode = VideoCallApp.CALL_FROM_LOOPBACK;
					mDialOrAnswer = false;
					return true;
				}
			}

			int initStatus = VTCallUtils.checkMOInitStatus(getContentResolver(), mInputPhoneURL);
			if (initStatus != 0 && initStatus != R.string.not_3G_network) {
				// set current ui to blank
				if (MyLog.DEBUG)
					MyLog.v(TAG, "invalid network, set activity ui to blank");
				Intent i = new Intent(this, FlyingModeAlertDialog.class);
				i.putExtra(FlyingModeAlertDialog.PROMPT_RES_ID, initStatus);
				i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
						| Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
				startActivity(i);
				return false;
			} else if (initStatus == R.string.not_3G_network) {
				mApp.mInputPhoneURL = mInputPhoneURL;
				mApp.doFallback(this.getText(R.string.not_3G_network1));
				return false;
			} else if (initStatus == R.string.not_3G_network_sub_error) {
				mApp.mInputPhoneURL = mInputPhoneURL;
				mApp.doFallback(this.getText(R.string.not_3G_network_sub_error));
				return false;
			}
		}

		return true;
	}

	private boolean postOnCreateInitProcess(){
		mDeviceName = VideoCallApp.STR_CALLURL_PREFIX[mLaunchMode];

		// special deal with IP answer, assign it 0.0.0.0 as server flag
		if (mDialOrAnswer == false && mLaunchMode == VideoCallApp.CALL_FROM_IP) {
			mDeviceName += "0.0.0.0";
		} else if (mLaunchMode == VideoCallApp.CALL_FROM_LOOPBACK){
			//FIXME just for test
			mInputPhoneURL = "Local Loop Back";
		} else {
			mDeviceName += mInputPhoneURL;
		}

		if (mLaunchMode == VideoCallApp.CALL_FROM_IP) {
			mDeviceName += ":60000";
		}

		if (MyLog.DEBUG) MyLog.d(TAG, "!!!@@@@THTHTHTHT Device Name " + mDeviceName + " mInputPhoneURL: " + mInputPhoneURL);
		if (mLaunchMode == VideoCallApp.CALL_FROM_LOOPBACK) {
			mApp.setLoopBackConnection();
		}

		if( mApp.initApp() == false){
			Log.e(TAG, "init App failed, just return");
			finish();
			return false;
		}

		// start call
		if (mDialOrAnswer) {
			if (MyLog.DEBUG) MyLog.v(TAG, "Dial out.");
			int flipstate = mApp.GetFlipState();
			if((flipstate==1 || flipstate==2 )&& ((mApp.mIsHardwareH324MStack&&!VTCallUtils.checkHeadsetStatus() && !mApp.mAudioManager.isBluetoothScoOn())||
					(!mApp.mIsHardwareH324MStack&&!VTCallUtils.checkHeadsetStatus())))
			{
				Intent i = new Intent(this, GeneralModeAlertDialog.class);
				i.putExtra( GeneralModeAlertDialog.PROMPT_RES_ID, R.string.incall_error_swivel_callout);
				i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
				this.startActivity(i);
				finish();
				mApp.ResetAll();
				return false;
			}
			if( mApp.dialOut( mLaunchMode) == false){
				Log.e(TAG, "failed to dial out");
				//Toast.makeText(this, this.getText(R.string.dial_video_call_failure), Toast.LENGTH_LONG).show();
				finish();
				overridePendingTransition(0, 0);
				mApp.ResetAll();
				Intent intent1 = new Intent("com.borqs.videocall.action.LaunchVideoCallScreen");
				intent1.addCategory(Intent.CATEGORY_DEFAULT);
				intent1.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
				intent1.putExtra("IsCallOrAnswer", mDialOrAnswer); // true as a
				// call,
				// while
				// false as
				// answer
				intent1.putExtra("LaunchMode", mLaunchMode); // nLaunchMode: 1 as
				// telephony, while
				// 0 as socket
				intent1.putExtra("call_number_key", mInputPhoneURL);
				startActivity(intent1);
				return false;
			}

			mApp.startIncomingCallQuery(mInputPhoneURL, true);
			// finish dialer UI
			//Intent intent = new Intent("android.intent.action.exit_dialer");
			//this.sendBroadcast(intent);
			VideoCallApp.sPanelMode = VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING;
		}
		else
		{
			VideoCallApp.sPanelMode = VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING;
		}
		/*       String keyguardShow = SystemProperties.get("sys.keyguard.showing");
       // if(keyguardShow.equals("1") && !mDialOrAnswer&&!mApp.mIsLandscape) {
        if(!mDialOrAnswer) {
            VideoCallApp.sPanelMode = VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING;
        } else {
		VideoCallApp.sPanelMode = VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING;
        }*/
		return true;
	}

	private void postOnCreateCommonProcess(){
		//	getWindow().addFlags(WindowManager.LayoutParams.FLAG_SHOW_WALLPAPER);
		setContentView(R.layout.videocall_screen);

		// Construct the UI
		setupViews();

		updateUI( mApp.getStatus());

	}
	BroadcastReceiver receiver;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
        requestWindowFeature(Window.FEATURE_NO_TITLE);
		overridePendingTransition(0, 0);
		super.onCreate(savedInstanceState);

		if (MyLog.DEBUG) MyLog.d(TAG, "onCreate");
		//requestWindowFeature(Window.FEATURE_NO_TITLE);

		mApp = VideoCallApp.getInstance();
		// set this flag so this activity will stay in front of the keyguard
		int flags = WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED;
		//flags |= WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD;
		getWindow().addFlags(flags);

		mApp.setCallScreen(this);
		receiver = new VTBtHeadsetReceiver();
		registerReceiver(receiver, new IntentFilter("android.bluetooth.headset.profile.action.CONNECTION_STATE_CHANGED"));
		if( mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
			if( initFromIntentInfo() == false){
				finish();
				return;
			}
			if( /*VTCallUtils.isFdnEnabled()*/false && mDialOrAnswer){
				mApp.setStatus( VideoCallApp.APP_STATUS_FDN_CHECKING);
				FDNChecker.getInstance().startFDNCheck( mApp.mHandler,
						VideoCallApp.MSG_FDN_ALLOWED, VideoCallApp.MSG_FDN_FORBIDDEN, mInputPhoneURL);
				return;
			} else {
				if( postOnCreateInitProcess() == false)	return;
			}

		}//end of if IDLE status process
		else{
			if (MyLog.DEBUG) MyLog.d(TAG, "&&&&&&&&@@@@@&&&&&&VideoCallScreen restored!!! Restore some values from VTApp");
			mDialOrAnswer = mApp.mDialOrAnswer;
			mLaunchMode = mApp.mLaunchMode;
			mInputPhoneURL = mApp.getCallerNumber();
			if (MyLog.DEBUG) MyLog.v(TAG, "Restore Log: mInputPhoneURL: " + mInputPhoneURL + ", launchmode"
					+ mLaunchMode + ",iscall," + mDialOrAnswer);

			mVTService = mApp.mVTService;
		}

		// Work-around for Beihai: Create a tmp file to record the audio recording status.
		vtstatusFile = new File(vtstatusPath);
		updateVTStatusFile(NOT_RECORDING);
		try {
			String[] chmod = { "chmod", "666", vtstatusPath};
			Runtime.getRuntime().exec(chmod);
		} catch ( Exception e) {
			if (MyLog.DEBUG) MyLog.v(TAG,"Exception: "+e);
		}
		postOnCreateCommonProcess();
	}

	@Override
	public void onDestroy() {
		unregisterReceiver(receiver);
		// mDestroyed = true;
		if (MyLog.DEBUG) MyLog.d(TAG, "onDestroy");
		if (mSlidingManager != null) {
			mSlidingManager.dismissPopup();
		}
		if (mPanelManager != null) {
			mPanelManager.cancelSwitchPanel();
			ProgressDialog progressdlg = mPanelManager.getProgressDialog();
			if(progressdlg !=null)
			{
				progressdlg.dismiss();
			}
		}
		super.onDestroy();
		if(mApp.mVideoCallScreen == this)
		{
			mApp.setCallScreen(null);
		}

		if (progressDlg != null) {
			progressDlg = null;
		}

		// Remove tmp file
		if (vtstatusFile != null) {
			vtstatusFile.delete();
		}
	}

	@Override
	protected void onNewIntent(Intent intent) {
		if (MyLog.DEBUG) Log.e(TAG,"onNewIntent: intent = " + intent);
		if (intent.getAction().equals(VideoCallApp.INTENT_ACTION_LAUNCH_VIDEOCALLSCREEN)) {
			if (MyLog.DEBUG) MyLog.d(TAG, "intent received");
			if( mApp.getStatus() != VideoCallApp.APP_STATUS_IDLE){
				if (MyLog.DEBUG) MyLog.d(TAG, "App status is not idle");
				if (MyLog.DEBUG) MyLog.d(TAG, "to get extra data");
				// Titank: get the input parameters by intent
				mDialOrAnswer = intent.getBooleanExtra(
						VideoCallApp.INTENT_EXTRA_CALL_OR_ANSWER, false);
				mInputPhoneURL = intent.getStringExtra(
						VideoCallApp.INTENT_EXTRA_PHONE_URL);
				mLaunchMode = intent.getIntExtra(
						VideoCallApp.INTENT_EXTRA_LAUNCH_MODE, 1);


				if (mInputPhoneURL == null || TextUtils.isEmpty(mInputPhoneURL)){
					mInputPhoneURL = this.getString(R.string.unknown_number);
				}
				if(true == mDialOrAnswer)
					return;

				if (MyLog.DEBUG) MyLog.d(TAG, "launch mode: " + mLaunchMode + " mDialOrAnswer: " + mDialOrAnswer);
				if (MyLog.DEBUG) MyLog.v(TAG, "mInputPhoneURL: " + mInputPhoneURL + ", launchmode"
						+ mLaunchMode + ",iscall," + mDialOrAnswer);

				//Special deal with the case that fake activity launched by system after a crash
				if( mDialOrAnswer == false && mApp.mConnection == null){
					if (MyLog.DEBUG) MyLog.d(TAG, "FAKE activity launching, just finish my self.");
					return;
				}

				// The startVTService design has been changed, start VTService before launch VideoCallScreen in VTCallReceiver if it is incoming call.
				// So the VideoCallScreen is not created when the VTService connected, VideoCallApp won't send the MSG_SERVICE_CONNECTED.
				// Here assign the VTService manually.
				if (mDialOrAnswer == false) {
					mVTService = mApp.mVTService;
				}

				if( postOnCreateInitProcess() == false)
					return;

				postOnCreateCommonProcess();
			}

		}
	}


	@Override
	protected void onResume() {
		super.onResume();

		if(mDialOrAnswer)
		{
			if (MyLog.DEBUG) Log.e(TAG,"for Phone dialer: ACTION_CLEAR_DIALER_NUMBER");
			Intent dialerIntent = new Intent(ACTION_CLEAR_DIALER_NUMBER);
			this.sendBroadcast(dialerIntent);
		}

		if (MyLog.DEBUG) MyLog.v(TAG, "onResume: mWakeLock: " +mApp.mWakeLock);
		if( mApp.mWakeLock != null)
		{
			if (mApp.mWakeLock.isHeld()) {
				mApp.mWakeLock.release();
			}
			mApp.mWakeLock.acquire();
		}

		VTNotificationManager nm = VTNotificationManager.getDefault();
		if (nm == null) {
			VTNotificationManager.init(this);
			nm = VTNotificationManager.getDefault();
		}
		nm.getStatusBarMgr().enableExpandedView(false);

		if (!hasPause) {
			// Set system brightness and confirm it is whether in the VTCall
			// brightness allowable range.
			try {
				mApp.mScreenBrightness = Settings.System.getInt(getContentResolver(),
						Settings.System.SCREEN_BRIGHTNESS) - VTCallUtils.MIN_BACKLIGHT;
			} catch (Exception e) {
				mApp.mScreenBrightness = VTCallUtils.MAX_BACKLIGHT;
			}
			//to set the screen brightness
			VTCallUtils.setScreenBrightness(mApp.mScreenBrightness + VTCallUtils.MIN_BACKLIGHT);
		} else {
			// Don't update the VTCall screen brightness after the VTCall screen has been paused.
			hasPause = false;
		}

		//click home button and back to videocall , local surfaceview is overlap  other component, so set view state GONE
		if(mApp.sPanelMode==VTPanel.VTPANEL_MODE_REMOTE_ONLY || mApp.sPanelMode==VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY)
		{
			if (mLocalSurface != null) {
				mLocalSurface.setVisibility(View.GONE);
			}
		}

		if (mApp.mCurrentVideoSource == VTManager.VideoSource.CAMERA_NONE)
		{
			if (mVTService != null)
			{
				mVTService.setVideoSource(mApp.mCurrentVideoSource, mApp.mStrReplaceImagePath,null);
				if (mApp.mStrReplaceImagePath != null){
					mLocalSurface.setVisibility(View.INVISIBLE);
					mPanelManager.showRepImage();
				}
			}
		}

		if (MyLog.DEBUG) MyLog.v(TAG, "OnResume");
	}

	@Override
	protected void onPause() {
		super.onPause();
		if (MyLog.DEBUG) MyLog.v(TAG, "OnPause()...");
		mApp.postWakeLockTimeoutMsg();

		VTNotificationManager nm = VTNotificationManager.getDefault();
		if (nm == null) {
			VTNotificationManager.init(this);
			nm = VTNotificationManager.getDefault();
		}
		nm.getStatusBarMgr().enableExpandedView(true);
		hasPause = true;
		if (MyLog.DEBUG) MyLog.v(TAG, "OnPause");

	}

	@Override
	protected void onStart() {
		super.onStart();
		hasStop = false;
		if (MyLog.DEBUG) MyLog.v(TAG, "OnStart");
	}

	@Override
	protected void onStop() {
		if (MyLog.DEBUG) MyLog.d(TAG, "onStop()...");
		super.onStop();
		if (!mApp.getVTServiceInterface().isUseVTCamera()) {
			if(mApp.getStatus() != VideoCallApp.APP_STATUS_IDLE && mApp.mCurrentVideoSource != VTManager.VideoSource.CAMERA_NONE) {
				if (mVTService != null) {
					mVTService.setVideoSource(VTManager.VideoSource.CAMERA_NONE, mApp.mStrReplaceImagePath,null);
					if(mApp.mStrReplaceImagePath == null) {
						mVTService.sendUserInput( VideoCallApp.MSG_CLOSE_CAMERA);
					}
				}
			}
		}
		else {
			// Work-around: If we don't do below operation, when VT move to forground and open camera again, the local preview will display black.
			if (mPanelManager != null) {
				mPanelManager.closeRepImage();
			}
			if (mLocalSurface != null) {
				mLocalSurface.setVisibility(View.VISIBLE);
			}
		}

		//restore the screen setting
		hasStop = true;
		if (MyLog.DEBUG) MyLog.v(TAG, "OnStop");
	}
	/*
           @Override
           protected void onActivityResult(int requestCode, int resultCode, Intent data) {
           super.onActivityResult(requestCode, resultCode, data);
           if (requestCode == 123) {
           if (MyLog.DEBUG) MyLog.d(TAG, "finish VideoCallScreen");
           this.finish();
           }

           }
	 */

	/*        private void setWallpaperBackground()
        {
		  View backgroundView = findViewById(R.id.vtapp);
              if (backgroundView != null) {
		 final WallpaperManager wallpaperManager = WallpaperManager.getInstance(this);
                   final Drawable wallpaperDrawable = wallpaperManager.getDrawable();
		backgroundView.setBackgroundDrawable(wallpaperDrawable);
             }
        }*/
	/**
	 * Finds all the views we need and configure them properly.
	 */
	private void setupViews() {
		if (MyLog.DEBUG) MyLog.v(TAG, "setupViews...");


		//FIXME:
	//	mSlidingGroupView = findViewById(R.id.sliding_group);
	//	mBottomConnectingView = findViewById(R.id.vt_bottom_connecting);
	//	mBlackSpaceView = findViewById(R.id.black_space);
		if (mAnswerFragment == null) {
			mAnswerFragment = (AnswerFragment) getFragmentManager()
						  .findFragmentById(R.id.answerFragment);
		}

		mPanelManager = new VTPanelComponentsMgr(this, mApp);
		if (VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING){
			mSlidingManager = new VTSlidingCardManager(this, mApp);

	//		mBottomConnectingView.setVisibility(View.GONE);
	//		mBlackSpaceView.setVisibility(View.VISIBLE);
	//		mSlidingGroupView.setVisibility(View.VISIBLE);

        mAnswerFragment.getView().setVisibility(View.GONE);
        mPanelManager.cameraImg.setVisibility(View.GONE);
		}


		if( VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING) {
			mSlidingManager = new VTSlidingCardManager(this, mApp);

			mAnswerFragment.getView().setVisibility(View.VISIBLE);
			mAnswerFragment.getUi().showAnswerUi(true);

			if (((AudioManager) getSystemService(Context.AUDIO_SERVICE)).getRingerMode()
					== AudioManager.RINGER_MODE_SILENT) {
				if (MyLog.DEBUG) MyLog.d(TAG, "Silent mode!");
			}

		//	mBottomConnectingView.setVisibility(View.VISIBLE);
		//	mBlackSpaceView.setVisibility(View.GONE);
		//	mSlidingGroupView.setVisibility(View.VISIBLE);
			if (mSlidingManager != null) {
				mSlidingManager.init();
			}

			// setWallpaperBackground();
		}
		final FrameLayout  mBtnSwitch = (FrameLayout)findViewById(R.id.btnSwitch);
		btnSwitch = (ImageView) findViewById(R.id.btn_switch);
		mBtnSwitch.setOnTouchListener(new FrameLayout.OnTouchListener() {
        public boolean onTouch(View view, MotionEvent event) {
		if(VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING){
			switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    break;
                case MotionEvent.ACTION_CANCEL:
                    break;
                case MotionEvent.ACTION_UP:
		    break;
                default:
                    break;
			}
		}
		else{
		switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    mBtnSwitch.setBackgroundResource(R.drawable.cmcc_phone_dialer_btn_bg_dis);
                    break;
                case MotionEvent.ACTION_CANCEL:
                    mBtnSwitch.setBackgroundResource(0);
                    break;
                case MotionEvent.ACTION_UP:
                    mBtnSwitch.setBackgroundResource(0);
		  if(1 == android.hardware.Camera.getNumberOfCameras()){
			Toast.makeText(VideoCallScreen.this,VideoCallScreen.this.getText(R.string.only_one_camera_not_switch),Toast.LENGTH_SHORT).show();
		    }
		    else{
			if( mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
				if (MyLog.DEBUG) MyLog.d(TAG, "App in idle status, just return");
			//	return;
				return true;
			}
			if (mApp.mEngineState >= VideoCallApp.ENGINE_STATE_CONNECTED)
			{
				if (!isProgressing) {
					if (MyLog.DEBUG) MyLog.v(TAG, "mBtnSwitchCamera event:" );
					isProgressing = true;
					progressDlg = ProgressDialog.show(VideoCallScreen.this,"",VideoCallScreen.this.getText(R.string.please_wait_msg), true, false);
					//hideSurfaceControlButtons();
					Thread task = new Thread(new SwitchCameraTask());
					task.start();}
			}}
			break;
            default:
                break;
            }}
            return true;
        }
	});

		final FrameLayout  mbtnMute = (FrameLayout)findViewById(R.id.btnmute);
		mbtnMute.setOnTouchListener(new View.OnTouchListener() {
		    public boolean onTouch(View view,MotionEvent event){
			if(VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING)
			{
			switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    break;
                case MotionEvent.ACTION_CANCEL:
                    break;
                case MotionEvent.ACTION_UP:
					break;
                default:
                    break;
				}
			}
            else{
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    mbtnMute.setBackgroundResource(R.drawable.cmcc_phone_dialer_btn_bg_dis);
                    break;
                case MotionEvent.ACTION_CANCEL:
                    mbtnMute.setBackgroundResource(0);
                    break;
                case MotionEvent.ACTION_UP:
                    mbtnMute.setBackgroundResource(0);
				if (MyLog.DEBUG) MyLog.d(TAG, "mute button clicked");
				onMute();
				break;
			default:
				break;
				}
			}
			return true;
		}
	});

		updateCallInfo();

		// SMS Reject correspondings
		mSMSRejectPanel = (LinearLayout) findViewById(R.id.sms_refuse_group);
		if (mDialOrAnswer == true) {
		   mSMSRejectPanel.setVisibility(View.GONE);
		}
		// mSMSContent = (TextView) findViewById(R.id.txt_view_sms_refuse_content);
		mBtnSMSSelect = (ImageButton) findViewById(R.id.btn_sms_choose_refuse_reason);
		mChkBoxSMS = (CheckBox) findViewById(R.id.chk_view_sms_refuse_content);
		mChkBoxSMS.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener () {
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				if( mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
					if (MyLog.DEBUG) MyLog.d(TAG, "App in idle status, just return");
					return;
				}
				mApp.mIsSmsRfsChecked = isChecked;
				if(isChecked)
				{
					mBtnSMSSelect.setBackgroundResource(R.drawable.cmcc_phone_vtcall_icon_reject_selected);

				}
				else
				{
					mBtnSMSSelect.setBackgroundResource(R.drawable.spinner_bkg_selector);

				}

			}
		});
		mTxtSMSSelect = (TextView) findViewById(R.id.txt_sms_choose_refuse_reason);
		setRejectSMSText();
		View.OnClickListener rejectSMSImage = new View.OnClickListener() {
			public void onClick(View v) {
				setRejectSMSText();
				selectRejectSMSView();
			}
		};
		mBtnSMSSelect.setOnClickListener(rejectSMSImage);


		if( mInputPhoneURL == null || TextUtils.isEmpty(mInputPhoneURL) || getString(R.string.unknown_number).equals(mInputPhoneURL)){

			mBtnSMSSelect.setEnabled(false);
			mChkBoxSMS.setEnabled(false);

		}else{

			mBtnSMSSelect.setEnabled(true);
			mChkBoxSMS.setEnabled(true);
		}
		// retrieve related controllers in VTPanel
		// Surface setup
		mLocalSurface = (TextureView) findViewById(R.id._local_surface);
		mLocalSurface.setSurfaceTextureListener(this);
		//  mLocalSurface.setZOrderOnTop(true);
		// mLocalHolder = mLocalSurface.getHolder();

		//mLocalSurface.setLongClickable(true);

		// mLocalHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		// mLocalHolder.addCallback(mLocalSurfaceCB);
		mRemoteSurface = (TextureView) findViewById(R.id._remote_surface);
		mRemoteSurface.setSurfaceTextureListener(this);
		mKeypadinput = (EditText) findViewById(R.id.txt_view_userinput);
		mKeypadinput.setVisibility(View.GONE);
		// mRemoteSurface.setZOrderOnTop(true);
		// mRemoteHolder = mRemoteSurface.getHolder();

		//mRemoteSurface.setLongClickable(true);

		// mRemoteHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		// mRemoteHolder.addCallback(mRemoteSurfaceCB);

		// TODO: Keyboard
		mKeyboard = (VideoCallKeyPad) findViewById(R.id.touch_keypad);
		if (mKeyboard != null) {
			// keypad.registerListener(this);

			mKeyboard.setKeypadClickListener(mKeypadListener);
			mKeyboard.setKeypadLongClickListener(mKeypadListener);

		}
	}

	public void onModeSwitchDone(int nCurrentMode) {
		if (MyLog.DEBUG) MyLog.d(TAG, "onModeSwidthDone, current Mode" + nCurrentMode);

	}

	public void onModeSwitchStart(int nCurrentMode) {
		if (MyLog.DEBUG) MyLog.d(TAG, "onModeSwidthStart, current Mode" + nCurrentMode);
	}

	/* Implementation of listeners */

	@Override
	public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
		if (surface.equals(mLocalSurface.getSurfaceTexture())) {
			if (MyLog.DEBUG) MyLog.v(TAG,"&&&&&&&&&&&&Local surface " );
			mIsLocalSurfaceCreated = true;
			mApp.mHandler.obtainMessage( VideoCallApp.MSG_LOCAL_SURFACE_CREATED, surface)
			.sendToTarget();

		} else if (surface.equals(mRemoteSurface.getSurfaceTexture())) {
			if (MyLog.DEBUG) MyLog.v(TAG, "Remote surface created.");
			mIsRemoteSurfaceCreated = true;

			mApp.mHandler.obtainMessage( VideoCallApp.MSG_REMOTE_SURFACE_CREATED, surface)
			.sendToTarget();
		}
	}

	@Override
	public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
		if (surface.equals(mLocalSurface.getSurfaceTexture())) {
			if (MyLog.DEBUG) MyLog.v(TAG, "@@@@@@@@@@Local surface  destroyed" );
			mIsLocalSurfaceCreated = false;
			mApp.mHandler.obtainMessage( VideoCallApp.MSG_LOCAL_SURFACE_DESTROYED)
			.sendToTarget();
		} else if (surface.equals(mRemoteSurface.getSurfaceTexture())) {
			if (MyLog.DEBUG) MyLog.v(TAG, "Remote surface destroyed.");
			mIsRemoteSurfaceCreated = false;
			mApp.mHandler.obtainMessage( VideoCallApp.MSG_REMOTE_SURFACE_DESTROYED)
			.sendToTarget();
		}
		return true;
	}

	@Override
	public void onSurfaceTextureUpdated(SurfaceTexture surface) {
		// Invoked every time there's a new Camera preview frame
	}

	@Override
	public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
		// Ignored camera does all the work for us
	}
	// SurfaceHolder.Callback
	/* private SurfaceHolder.Callback mRemoteSurfaceCB = new SurfaceHolder.Callback() {
            public void surfaceChanged(SurfaceHolder surfaceholder, int i, int j,
                    int k) {
                if (MyLog.DEBUG) MyLog.d(TAG, "Remote surfaceChanged called, new size " + i + " ; "
                        + j + " ; " + k);
            }

            public void surfaceDestroyed(SurfaceHolder surfaceholder) {
                if (MyLog.DEBUG) MyLog.d(TAG, "remote surfaceDestroyed called.");

                mIsRemoteSurfaceCreated = false;
                mApp.mHandler.obtainMessage( VideoCallApp.MSG_REMOTE_SURFACE_DESTROYED)
                    .sendToTarget();
            }

            public void surfaceCreated(SurfaceHolder holder) {
                if (MyLog.DEBUG) MyLog.v(TAG, "Remote surface created.");

                // FIXME:!!!!, below size might be changed in future
                //Borqs-india
                holder.setFixedSize(224, 183);

                if (MyLog.DEBUG) MyLog.d(TAG, "@@@@@@@@@@Remote surface " + holder.getSurface());

                mIsRemoteSurfaceCreated = true;

                mApp.mHandler.obtainMessage( VideoCallApp.MSG_REMOTE_SURFACE_CREATED, holder.getSurface())
                    .sendToTarget();
            }
        };

        private SurfaceHolder.Callback mLocalSurfaceCB = new SurfaceHolder.Callback() {
            public void surfaceChanged(SurfaceHolder surfaceholder, int i, int j,
                    int k) {
                if (MyLog.DEBUG) MyLog.d(TAG, "Local surfaceChanged called, new size " + i + " ; " + j
                        + " ; " + k);
            }

            public void surfaceDestroyed(SurfaceHolder surfaceholder) {
                if (MyLog.DEBUG) MyLog.d(TAG, "Local surfaceDestroyed called.");
                mIsLocalSurfaceCreated = false;
                mApp.mHandler.obtainMessage( VideoCallApp.MSG_LOCAL_SURFACE_DESTROYED)
                    .sendToTarget();
            }

            public void surfaceCreated(SurfaceHolder holder) {
                if (MyLog.DEBUG) MyLog.v(TAG, "@@@@@@@@@@Local surface " + holder.getSurface());

                if (MyLog.DEBUG) MyLog.d(TAG, "Local Surface call back, usevtcamera is " + mApp.getVTServiceInterface().isUseVTCamera());
                if (mApp.getVTServiceInterface().isUseVTCamera()) {
                    holder.setFixedSize(176, 144);
                }
                mIsLocalSurfaceCreated = true;
                mApp.mHandler.obtainMessage( VideoCallApp.MSG_LOCAL_SURFACE_CREATED, holder.getSurface())
                    .sendToTarget();
            }
        };*/

	@Override
	public boolean dispatchKeyEvent(KeyEvent event) {
		if(VideoCallApp.sPanelMode==VTPanel.VTPANEL_MODE_LOCAL
				||VideoCallApp.sPanelMode==VTPanel.VTPANEL_MODE_REMOTE || VideoCallApp.sPanelMode==VTPanel.VTPANEL_MODE_REMOTE_ONLY){
			if(mApp.mEngineState >= VideoCallApp.ENGINE_STATE_CONNECTED){
				char keyNumber = event.getNumber();
				switch(keyNumber){
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case '*':
				case '#':
				{
					onShowKeyboard();
					if (MyLog.DEBUG) MyLog.d(TAG, "clicked by hardkey:" + keyNumber);
					mApp.mUserInputBuffer.append(keyNumber);
					mPanelManager.appendUserInput(String.valueOf(keyNumber));
					break;
				}
				default:
					break;
				}
			}
		}
		return super.dispatchKeyEvent(event);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		switch (keyCode) {
		case KeyEvent.KEYCODE_CAMERA:
			//                case KeyEvent.KEYCODE_CUSTOMER:
			//               case KeyEvent.KEYCODE_CUSTOMER1:
			//                case KeyEvent.KEYCODE_CUSTOMER2:
			// for venus and TD2, Disable the customer keys since can not launch camera while in-vtcall
			return true;
		case KeyEvent.KEYCODE_VOLUME_UP:
		case KeyEvent.KEYCODE_VOLUME_DOWN:
			if( mApp.getStatus() == VideoCallApp.APP_STATUS_RINGING && !mApp.mDialOrAnswer){
				//  mRingingMuteView.setImageResource(R.drawable.cmcc_videocall_dialer_btn_icon_mute);
				mApp.stopRing();
				return true;
			}
			break;
		case KeyEvent.KEYCODE_CALL:
			onAccept();
			return true;
		case KeyEvent.KEYCODE_ENDCALL:
			onEndCall();
			return true;
		case KeyEvent.KEYCODE_HEADSETHOOK:
			if (!event.isLongPress()) {
				if (mApp.getStatus() == VideoCallApp.APP_STATUS_RINGING) {
					onAccept();
				} else if (mApp.getStatus() == VideoCallApp.APP_STATUS_CONNECTED) {
					onMute();
				}
			}
			return true;
		default:
			break;
		}
		return super.onKeyUp(keyCode, event);
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		switch (keyCode) {
		case KeyEvent.KEYCODE_CAMERA:
			if (MyLog.DEBUG) MyLog.v(TAG, "camera key pressed.");
			VTCallUtils.notifyCameraKeyDisable(this,
					getText(R.string.camera_key_event));
			return true;
			//              case KeyEvent.KEYCODE_CUSTOMER:
			//              case KeyEvent.KEYCODE_CUSTOMER1:
			//              case KeyEvent.KEYCODE_CUSTOMER2:
			//                        if (MyLog.DEBUG) MyLog.v(TAG, "CUSTOM key pressed.");
			//                        VTCallUtils.notifyCameraKeyDisable(this,getText(R.string.custom_key_event));
			//                        return true;
		case KeyEvent.KEYCODE_HEADSETHOOK:
			if (event.isLongPress()) {
				if (mApp.getStatus() == VideoCallApp.APP_STATUS_RINGING) {
					onRefuseClicked();
				} else if (mApp.getStatus() == VideoCallApp.APP_STATUS_CONNECTED) {
					onEndCall();
				}
			}
			return true;
		case KeyEvent.KEYCODE_VOLUME_UP:
		case KeyEvent.KEYCODE_VOLUME_DOWN:
			if( mApp.getStatus() == VideoCallApp.APP_STATUS_RINGING && !mApp.mDialOrAnswer)
			{
				mApp.stopRing();
				//                            if(mRingingMuteView!=null)
					//                            {mRingingMuteView.setImageResource(R.drawable.cmcc_videocall_dialer_btn_icon_mute);}
				return true;
			}
			break;
		case KeyEvent.KEYCODE_BACK:
			if (mApp.getStatus() == VideoCallApp.APP_STATUS_RINGING) {
				return true;
			}
			break;

		default:
			break;
		}
		return super.onKeyDown(keyCode, event);
	}

	public void SendRejectSms()
	{
		if( mApp.mIsSmsRfsChecked &&mLaunchMode != VideoCallApp.CALL_FROM_LOOPBACK &&
				(mInputPhoneURL != null && !mInputPhoneURL.equals(this.getString(R.string.unknown_number))) ){
			RejectSmsSender rss = new RejectSmsSender();
			if(scv.size()>mCurrentView)
			{
				rss.doSend( mInputPhoneURL, scv.get(mCurrentView).toString());
			}
		}
	}


	/**
	 * Brings up the standard SMS compose UI.
	 */
	private void launchSmsCompose(String phoneNumber, int subscription) {
		if (MyLog.DEBUG) MyLog.d(TAG, "launchSmsCompose: number" + phoneNumber);

		//mApp.getKeyguardManager().newKeyguardLock(TAG).disableKeyguard();

		Uri uri = Uri.fromParts("sms", phoneNumber, null);
		Intent intent = new Intent(Intent.ACTION_VIEW, uri);
		intent.putExtra("send_subscription", subscription);
		if (MyLog.DEBUG) MyLog.d(TAG, "- Launching SMS compose UI:" + intent);
		this.startActivity(intent);

		onRejectCall();

		// TODO: One open issue here: if the user selects "Custom message"
		// for an incoming call while the device was locked, and the user
		// does *not* have a secure keyguard set, we bring up the
		// non-secure keyguard at this point :-(
		// Instead, we should immediately go to the SMS compose UI.
		//
		// I *believe* the fix is for the SMS compose activity to set the
		// FLAG_DISMISS_KEYGUARD window flag (which will cause the
		// keyguard to be dismissed *only* if it is not a secure lock
		// keyguard.)
		//
		// But it there an equivalent way for me to accomplish that here,
		// without needing to change the SMS app?
		//
		// In any case, I'm pretty sure the SMS UI should *not* to set
		// FLAG_SHOW_WHEN_LOCKED, since we do want the force the user to
		// enter their lock pattern or PIN at this point if they have a
		// secure keyguard set.
	}


	private class KeypadListener implements
	VideoCallKeyPad.KeypadClickListener,
	VideoCallKeyPad.KeypadLongClickListener {

		private Context mContext;

		public KeypadListener(Context context) {
			mContext = context;
		}

		public void onClick(char c) {
			if (MyLog.DEBUG) MyLog.d(TAG, "clicked:" + c);
			/*
                       if( mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
                       if (MyLog.DEBUG) MyLog.d(TAG, "App in idle status, just return");
                       return;
                       }
			 */
			if( mApp.mEngineState < VideoCallApp.ENGINE_STATE_CONNECTED){
				if (MyLog.DEBUG) MyLog.d(TAG, "engine not connected yet, just return");
				return;
			}

			stopLocalToneIfNeeded();

			startLocalToneIfNeeded(c);

			KeyEvent event = VideoCallKeyPad.generateKeyEvent(c);
			//Save
			mApp.mUserInputBuffer.append(c);
			mPanelManager.appendUserInput(Character.toString(c));

			/*try {
                        StringBuffer buf = new StringBuffer();
                        buf.append(c);
                        if (MyLog.DEBUG) MyLog.d(TAG, "Screen: sendUserInput: " + buf);
                        mVTService.sendUserInput(buf.toString());
                    } catch (IllegalStateException e) {
                        // TODO: handle exception
                        Log.e(TAG, "IllegalStateException: " + e);
                    } catch (Exception e) {
                        Log.e(TAG, "error: " + e);
                    }*/
		}

		public boolean onLongClick(char c) {
			// TODO: no operation defined
			return true;

		}
	}

	private KeypadListener mKeypadListener = new KeypadListener(this);

	private String[] getCameraParameterValue(String key) {
		String value = mParameters.get(key);
		if (value != null ) {
			String [] values = value.split(",");
			return values;
		}
		return null;
	}

	public void onFallBackClicked() {
		if(mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
			if (MyLog.DEBUG) MyLog.d(TAG, "App in idle status, just return");
			return;
		}
		int flipState = mApp.GetFlipState();
		//       	 if((flipState==1 || flipState ==2) &&  ((mApp.mIsHardwareH324MStack&&!VTCallUtils.checkHeadsetStatus() && !mApp.mAudioManager.isBluetoothScoOn())||
		//          		(!mApp.mIsHardwareH324MStack&&!VTCallUtils.checkHeadsetStatus())))
		//       	 {
		//       		   if(mApp.flipAlertDialog ==null || !mApp.flipAlertDialog.isShowing())
		//	            	mApp.flipAlertDialog =VTCallUtils.Alerter.doAlert(this,(String)getText(R.string.incall_error_swivel_callin));
		//           	return;
		//       	 }
		mApp.doFallback( null);
	}

	public  void onRefuseClicked() {
		if( mApp.getStatus() == VideoCallApp.APP_STATUS_IDLE){
			if (MyLog.DEBUG) MyLog.d(TAG, "App in idle status, just return");
			return;
		}
		SendRejectSms();
		onRejectCall();
	}

	/**
	 * to surface
	 */

	void setRemoteSurfaceVisibility(int visibility) {
		if (MyLog.DEBUG) MyLog.v(TAG, "setRemoteSurfaceVisibility: " + visibility);
		if (mRemoteSurface != null) {
			mRemoteSurface.setVisibility(visibility);
		}
	}

	void setLocalSurfaceVisibility(int visibility) {
		if (MyLog.DEBUG) MyLog.v(TAG, "setLocalSurfaceVisibility: " + visibility);
		if (mLocalSurface != null) {
			if (MyLog.DEBUG) MyLog.v(TAG, "set local surface to: " + visibility + ", mLocalSurface.getVisibility(): " + mLocalSurface.getVisibility());
			mLocalSurface.setVisibility(visibility);
		}
	}

	private void setRejectSMSText() {
		if(MyLog.DEBUG)MyLog.d(TAG, "setRejectSMSText");
		scv = new ArrayList<CharSequence>();
		Context phoneContext = null;
		try {
			phoneContext = createPackageContext("com.android.phone",
					Context.CONTEXT_IGNORE_SECURITY);
		} catch (NameNotFoundException ex) {
			Log.e(TAG, "setRejectSMSText error :" + ex.toString());
			return;
		}

		SharedPreferences rejectSMSsettings = phoneContext
				.getSharedPreferences("respond_via_sms_prefs",
						Context.MODE_WORLD_READABLE
						| Context.MODE_MULTI_PROCESS);
		if (rejectSMSsettings == null)
			return;
		String defaultValue = getDefaultRejectSMS();
		String sms1 = rejectSMSsettings.getString("canned_response_pref_1", this.getString(R.string.respond_via_sms_canned_response_1));
		if (sms1 != null && !sms1.isEmpty()) {
			scv.add(sms1);
		}
		String sms2 = rejectSMSsettings.getString("canned_response_pref_2", this.getString(R.string.respond_via_sms_canned_response_2));
		if (sms2 != null && !sms2.isEmpty()) {
			scv.add(sms2);
		}
		String sms3 = rejectSMSsettings.getString("canned_response_pref_3", this.getString(R.string.respond_via_sms_canned_response_3));
		if (sms3 != null && !sms3.isEmpty()) {
			scv.add(sms3);
		}
		String sms4 = rejectSMSsettings.getString("canned_response_pref_4", this.getString(R.string.respond_via_sms_canned_response_4));
		if (sms4 != null && !sms4.isEmpty()) {
			scv.add(sms4);
		}

		scv.add(this.getString(R.string.respond_via_sms_custom_message));

		if (scv.size() == 0) {
			scv.add(this.getString(R.string.rejectWithSMSContent));
			scv.add(this.getString(R.string.rejectWithSMSContent1));
			scv.add(this.getString(R.string.respond_via_sms_custom_message));
		}
		if (!defaultValue.isEmpty()) {
			mCurrentView = scv.indexOf(defaultValue);
			if (mCurrentView < 0 || mCurrentView == scv.size()-1) {
				mCurrentView = 0;
			}
		}
		if (scv.size() > mCurrentView) {
			mTxtSMSSelect.setText(scv.get(mCurrentView));
		}
	}

	private void updateRejectSMSText() {
		if (scv.size() > mCurrentView) {
			if(mCurrentView == scv.size()-1){
				// Take the user to the standard SMS compose UI.
				if(mLaunchMode != VideoCallApp.CALL_FROM_LOOPBACK){
					launchSmsCompose(mInputPhoneURL, 0);
				}
			}else{
				mTxtSMSSelect.setText(scv.get(mCurrentView).toString().trim());
				saveDefaultRejectSMS(scv.get(mCurrentView).toString().trim());
			}
		}
	}

	private void saveDefaultRejectSMS(String defaultRejectSMS) {
		SharedPreferences sharedPreferences = getSharedPreferences(
				"default_RejectSMS_Value", Context.MODE_PRIVATE);
		Editor editor = sharedPreferences.edit();
		editor.putString("defaultValue", defaultRejectSMS);
		editor.commit();
	}

	private String getDefaultRejectSMS() {
		SharedPreferences sharedPreferences = getSharedPreferences(
				"default_RejectSMS_Value", Context.MODE_PRIVATE);
		if (sharedPreferences != null) {
			return sharedPreferences.getString("defaultValue", "");
		}
		return "";
	}
	private void updateVTStatusFile(int vtStatus) {
		try {
			if (MyLog.DEBUG) MyLog.v(TAG, "change the vtstatus mode:" + vtStatus);
			DataOutputStream os = new DataOutputStream(new FileOutputStream(vtstatusFile));
			os.write("pid:".getBytes());
			os.write(String.valueOf(Process.myPid()).getBytes());
			os.write("\nmode:".getBytes());
			os.write(String.valueOf(vtStatus).getBytes());
			os.write("\n".getBytes());
			os.close();
		} catch (Exception e) {
			if (MyLog.DEBUG) MyLog.v(TAG,"Exception: "+e);
		}
	}

	private void selectRejectSMSView() {
		if (MyLog.DEBUG) MyLog.d(TAG, "selectRejectSMSView..");
		mCurrentView = mPreviousSelectedView;
		mRejectSmsDialog = new AlertDialog.Builder(this)
		.setTitle(R.string.choose_sms_refuse_prompt)
		//           .setIcon(R.drawable.cmcc_dialog_question)
		.setSingleChoiceItems(scv.toArray(new CharSequence[]{}), mCurrentView, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {
				if (MyLog.DEBUG) MyLog.v(TAG, "onClick: whichButton is "+whichButton);
				mCurrentView= whichButton;
				return;
			}
		})
		.setPositiveButton(R.string.dialog_select, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {
				mPreviousSelectedView = mCurrentView;
				updateRejectSMSText();
				mChkBoxSMS.setChecked(true);
				mApp.mIsSmsRfsChecked = true;
				mBtnSMSSelect.setBackgroundResource(R.drawable.cmcc_phone_vtcall_icon_reject_selected);
				return;
			}
		})
		.setNegativeButton(R.string.cancel_fallback_setting, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int whichButton) {
				//mBtnSMSSelect.setBackgroundResource(R.drawable.cmcc_vtcall_reject_bar_field_normal);
				return;
			}
		})
		.show();

	}

	/**
	 * Plays the local tone based the phone type.
	 */
	public void startLocalToneIfNeeded(char c) {
		// if local tone playback is enabled, start it.
		// Only play the tone if it exists.
		if (!mToneMap.containsKey(c)) {
			return;
		}

		//Need to know if there is any settings attribute...
		mLocalToneEnabled = Settings.System.getInt(getContentResolver(),
				Settings.System.DTMF_TONE_WHEN_DIALING, 1) == 1;


		// create the tone generator
		// if the mToneGenerator creation fails, just continue without it.  It is
		// a local audio signal, and is not as important as the dtmf tone itself.
		if (mLocalToneEnabled) {
			synchronized (mToneGeneratorLock) {

				if (mToneGenerator == null) {
					try {
						mToneGenerator = new ToneGenerator(AudioManager.STREAM_DTMF, 80);
					} catch (RuntimeException e) {
						Log.d("DTMF_Tone","Exception caught while creating local tone generator: " + e);
						mToneGenerator = null;
					}
				}

				if (mToneGenerator != null) {
					Log.d("DTMF_Tone","starting local tone " + c);
					int toneDuration = -1;
					toneDuration = DTMF_DURATION_MS;
					mToneGenerator.startTone(mToneMap.get(c), toneDuration);
				}
			}
		}
	}

	/**
	 * Stops the local tone based on the phone type.
	 */
	public void stopLocalToneIfNeeded() {
		// if local tone playback is enabled, stop it.
		Log.d("DTMF_Tone","trying to stop local tone...");
		if (mLocalToneEnabled) {
			synchronized (mToneGeneratorLock) {
				if (mToneGenerator == null) {
					Log.d("DTMF_Tone","stopLocalTone: mToneGenerator == null");
				} else {
					Log.d("DTMF_Tone","stopping local tone.");
					mToneGenerator.stopTone();
					mToneGenerator.release();
					mToneGenerator = null;
				}
			}
		}
	}

	/*	public void onConfigurationChanged(Configuration newConfig) {
		if (MyLog.DEBUG) MyLog.d(TAG,"into onConfigurationChanged mIsLandscape"+(newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE));
		super.onConfigurationChanged(newConfig);
		mChangeConfig = true;
        //if change from portrait to landscape,the mode should be changed here
        if(newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE){
		mApp.mIsLandscape = true;
            if((VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_LOCAL)){
                VideoCallApp.sPanelMode = VTPanel.VTPANEL_MODE_LOCAL;
            }
            if((VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_REMOTE)){
                VideoCallApp.sPanelMode = VTPanel.VTPANEL_MODE_REMOTE;
            }
            if((VideoCallApp.sPanelMode == VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY)){
                VideoCallApp.sPanelMode = VTPanel.VTPANEL_MODE_REMOTE_ONLY;
            }
        }
        else
        {
		mApp.mIsLandscape = false;
        }
         // Construct the UI
			setContentView(R.layout.videocall_screen);
			setupViews();
			updateUI( mApp.getStatus());
	}
	 */
	 public static class VTIncomingCallReceiver extends BroadcastReceiver{
        @Override
        public void onReceive(Context context, Intent intent) {
        if(intent.getAction().equals("qualcomm.intent.action.NEW_CSVT_RINGING_CONNECTION")){
            Intent i = new Intent("borqs.intent.action.NEW_CSVT_RINGING_CONNECTION");
            String strAddr = intent.getStringExtra("connectionAddress");
            boolean bLockMode = intent.getBooleanExtra(VideoCallApp.INTENT_EXTRA_IS_LOCKED_MODE, false);
            i.putExtra("connectionAddress", strAddr);
            i.putExtra(VideoCallApp.INTENT_EXTRA_IS_LOCKED_MODE, bLockMode);
            context.sendBroadcast(i);
            }
        }
    }
}
