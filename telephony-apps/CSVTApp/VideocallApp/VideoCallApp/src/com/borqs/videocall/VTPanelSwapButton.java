/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;

public class VTPanelSwapButton extends VTPanelComponent{

	static final String TAG = "VT/VTPanelSwapButton";

	public VTPanelSwapButton(Context context, AttributeSet attrs) {
		super(context, attrs);
		if (MyLog.DEBUG) MyLog.d(TAG, "Construct!");
		initLayoutParam(context, attrs);
	}

	@Override
	void InitVisibility() {
		// TODO Auto-generated method stub
		mVisibilities[VTPanel.VTPANEL_MODE_KEYPAD_LOCAL] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_LOCAL] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING] = View.GONE;
		mVisibilities[VTPanel.VTPANEL_MODE_REMOTE] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING] = View.GONE;
		mVisibilities[VTPanel.VTPANEL_MODE_REMOTE_ONLY] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY] = View.VISIBLE;
	}

	private void log(String info) {
		if (DBG) {
			if (MyLog.DEBUG) MyLog.d(TAG, info);
		}
	}
}
