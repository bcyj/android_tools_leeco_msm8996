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
import android.widget.FrameLayout;

public class VTPanelLocalWindow extends VTPanelComponent{

	public VTPanelLocalWindow(Context context, AttributeSet attrs) {
		super(context, attrs);
		initLayoutParam(context, attrs);
	}



	@Override
	void InitVisibility() {
		// TODO Auto-generated method stub
		mVisibilities[VTPanel.VTPANEL_MODE_KEYPAD_LOCAL] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_LOCAL] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_REMOTE] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING] = View.VISIBLE;
		mVisibilities[VTPanel.VTPANEL_MODE_REMOTE_ONLY] = View.GONE;
		mVisibilities[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY] = View.GONE;
	}
	@Override
	public boolean dispatchTouchEvent(MotionEvent ev) {
		log("local window on touch...event: " + ev);
		// TODO Auto-generated method stub

		int action = ev.getAction();
		if (mPanel == null) {
			throw new IllegalArgumentException(
					"vtpanel does not exist");
		}
		VTPanel.IPanelManager manager = mPanel.getPanelManager();
		if (manager == null) {
			// panel is not built yet
			return false;
		}
		if (action == MotionEvent.ACTION_DOWN) {
			manager.onTouchDownEvent(this, ev);
		} else if (action == MotionEvent.ACTION_CANCEL) {
			manager.onTouchCancelEvent(this, ev);
		} else if (mhasTouchDown && action == MotionEvent.ACTION_UP){
			// switch to control button mode
			manager.onTouchCancelEvent(this, ev);
			manager.onClickEvent(this);
		}
		return super.dispatchTouchEvent(ev);
	}

	private void log(String info) {
		if (DBG) {
			if (MyLog.DEBUG) MyLog.d(TAG, info);
		}
	}
}
