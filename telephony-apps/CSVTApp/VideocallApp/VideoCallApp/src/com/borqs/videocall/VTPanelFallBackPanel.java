/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import android.content.Context;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;

public class VTPanelFallBackPanel extends VTPanelComponent{

	public VTPanelFallBackPanel(Context context, AttributeSet attrs) {
		super(context, attrs);
		initLayoutParam(context, attrs);
	}

	@Override
	void InitVisibility() {
		// TODO Auto-generated method stub
		mVisibilities[VTPanel.VTPANEL_MODE_KEYPAD_LOCAL] = View.GONE;
		mVisibilities[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE] = View.GONE;
		mVisibilities[VTPanel.VTPANEL_MODE_LOCAL] = View.GONE;
		mVisibilities[VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_REMOTE] = View.GONE;
		mVisibilities[VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_REMOTE_ONLY] = View.GONE;
		mVisibilities[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY] = View.GONE;
	}
}
