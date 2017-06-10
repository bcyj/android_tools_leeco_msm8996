/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.os.Handler;
import android.util.Log;
//TEMP Commented due to the compile reason
//import android.bluetooth.BluetoothIntent;
import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothProfile;
public class VTAppReceiver extends BroadcastReceiver{

	private static String TAG = "VT/VTHeadSetReceiver";

	@Override
	public void onReceive(Context context, Intent intent) {

		final VideoCallApp mApp = VideoCallApp.getInstance();
		final Handler mAppHandler = VideoCallApp.getInstance().mHandler;

		// TODO Auto-generated method stub
		String action = intent.getAction();
		if ( action.equals(Intent.ACTION_HEADSET_PLUG)) {

			if (MyLog.DEBUG) MyLog.d( TAG, "mReceiver: ACTION_HEADSET_PLUG");
			if (MyLog.DEBUG) MyLog.d( TAG, "==> intent: " + intent);
			if (MyLog.DEBUG) MyLog.d( TAG, "    state: " + intent.getIntExtra("state", 0));
			if (MyLog.DEBUG) MyLog.d( TAG, "    name: " + intent.getStringExtra("name"));

			int headsetState = intent.getIntExtra("state",0);

			mAppHandler.sendMessage(mAppHandler.obtainMessage(VideoCallApp.MSG_WIRED_HEADSET_PLUG, headsetState, 0));

		}
		else if (action.equals(BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED)) {
            int state = intent.getIntExtra(BluetoothProfile.EXTRA_STATE,0);

            if (MyLog.DEBUG) MyLog.d( TAG, "New BT headset state: " + state );

            if (state == BluetoothHeadset.STATE_CONNECTED) {
		mAppHandler.sendMessage(
				mAppHandler.obtainMessage( VideoCallApp.MSG_BLUETOOTH_PLUG, 1, 0));
            } else if (state == BluetoothHeadset.STATE_DISCONNECTED) {
                mAppHandler.sendMessage(
                        mAppHandler.obtainMessage(VideoCallApp.MSG_BLUETOOTH_PLUG, 0));
            }
        } //end of if BluetoothIntent.Headset_state_changed_action
		else if(action.equals( Intent.ACTION_SCREEN_OFF)){
			mApp.mIsScreenOn = false;
			if (MyLog.DEBUG) MyLog.d( TAG, "mReceiver: ACTION_SCREEN_OFF screenLockStatus off ");
        } else if(action.equals(Intent.ACTION_SCREEN_ON)){
		mApp.mIsScreenOn = true;
            if (MyLog.DEBUG) MyLog.d( TAG, "mReceiver:ACTION_SCREEN_ON  screenLockStatus on");
//        } else if(action.equals(Intent.ACTION_FLIP_CHANGED)){
//	        	if (!intent.hasExtra("flipStatus")) {
//					Log.e(TAG,"receive the CLI intent with wrong extra");
//					return;
//				}
//				int flip = intent.getIntExtra("flipStatus", 0);
//				if (MyLog.DEBUG) MyLog.d(TAG,"receive the flip change message flip = " + flip);
//				mAppHandler.sendMessage(mAppHandler.obtainMessage(VideoCallApp.MSG_FLIP_CHANGE,flip,0));
			}
        }

	}
