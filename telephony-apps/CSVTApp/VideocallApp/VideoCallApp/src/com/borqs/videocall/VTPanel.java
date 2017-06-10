/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import java.util.ArrayList;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.Log;
import android.view.GestureDetector;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.widget.FrameLayout;

public class VTPanel extends ViewGroup {
	static final String TAG = "VT/VTPanel";
	private final boolean DBG = true;

	public static final int ORIENTATION_HORIZONTAL = 0;
	public static final int ORIENTATION_VERTICAL = 1;

	/*
	 * Modes
	 */
	public static final int VTPANEL_MODE_DAILOUT_CONNECTING = 0;
	public static final int VTPANEL_MODE_ANSWERIN_CONNECTING = 1;
	public static final int VTPANEL_MODE_LOCAL = 2;
	public static final int VTPANEL_MODE_REMOTE = 3;
	public static final int VTPANEL_MODE_KEYPAD_LOCAL = 4;
	public static final int VTPANEL_MODE_KEYPAD_REMOTE = 5;
	public static final int VTPANEL_MODE_REMOTE_ONLY = 6;
	public static final int VTPANEL_MODE_KEYPAD_REMOTE_ONLY = 7;
	public static final int VTPANEL_MODE_COUNT = 8;

	static final int [] VTPANEL_MODES  = {
		VTPANEL_MODE_DAILOUT_CONNECTING,
		VTPANEL_MODE_ANSWERIN_CONNECTING,
		VTPANEL_MODE_LOCAL,
		VTPANEL_MODE_REMOTE,
		VTPANEL_MODE_KEYPAD_LOCAL,
		VTPANEL_MODE_KEYPAD_REMOTE,
		VTPANEL_MODE_REMOTE_ONLY,
		VTPANEL_MODE_KEYPAD_REMOTE_ONLY,
	};

	private int mDelayedTargetState;

	private int[] mBottomState = new int[VTPANEL_MODE_COUNT];
    private int[] mRightState = new int[VTPANEL_MODE_COUNT];
	/* animate parameters */
	private static final int MSG_ANIMATE = 1000;
	static final int ANIMATION_DURATION = 0;
	static final int ANIMATION_DURATION_NONE = 0;

	static final int ANIMATION_FRAME_DURATION = 800 / 16; // 30frame
	static final int DELAY_SWITCH_MODE_TIME = 2500;		//milliseconds
	static final int DELAY_SWITCH_MODE_SHORT_TIME = 500;

	private int mAnimationDuration = ANIMATION_DURATION;


	private boolean mIsAnimating = false;
	private long mAnimationStartTime;

	private final Handler mHandler = new AnimHandler();

	public static final int VTPANEL_SHOW_ACTION_BUTTONS = 2;

	private IPanelManager mPanelManager;
	private Context mContext;

	// FIXME: no use in current, TODO
	private int mBottomOffset;
	private int mTopOffset;
	private int mInternalGapSize;

	public VTPanel(Context context, AttributeSet attrs) {
		this(context, attrs, 0);
	}

	public VTPanel(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);

		TypedArray a = context.obtainStyledAttributes(attrs,
				R.styleable.VTPanel);

		mContext = context;

		mBottomOffset = (int) a.getDimension(R.styleable.VTPanel_bottomOffset,
				0.0f);
		mTopOffset = (int) a.getDimension(R.styleable.VTPanel_topOffset, 0.0f);
		// mAnimateOnClick = a.getBoolean(R.styleable.VTPanel_animateOnClick,
		// true);

		//a.recycle();

		setAlwaysDrawnWithCacheEnabled(false);

		// VT difference height values under different modes
		mBottomState[VTPANEL_MODE_ANSWERIN_CONNECTING] = (int) a.getDimension(R.styleable.VTPanel_panelHeightLockConnecting, 0.0f);
		mBottomState[VTPANEL_MODE_DAILOUT_CONNECTING] = (int) a.getDimension(R.styleable.VTPanel_panelHeightUnlockConnecting, 0.0f);
		mBottomState[VTPANEL_MODE_LOCAL]
		             = mBottomState[VTPANEL_MODE_REMOTE]
		             = mBottomState[VTPANEL_MODE_REMOTE_ONLY]
		             = (int) a.getDimension(R.styleable.VTPanel_panelHeightInCall, 0.0f);
		mBottomState[VTPANEL_MODE_KEYPAD_LOCAL]
		             = mBottomState[VTPANEL_MODE_KEYPAD_REMOTE]
		             = mBottomState[VTPANEL_MODE_KEYPAD_REMOTE_ONLY]
		             = (int) a.getDimension(R.styleable.VTPanel_panelHeightKeypad, 0.0f);
        mRightState[VTPANEL_MODE_ANSWERIN_CONNECTING]
                     = mRightState[VTPANEL_MODE_DAILOUT_CONNECTING]
                     =(int) a.getDimension(R.styleable.VTPanel_panelWidthConnecting, 0.0f);
        mRightState[VTPANEL_MODE_LOCAL]
                     = mRightState[VTPANEL_MODE_REMOTE]
                     = mRightState[VTPANEL_MODE_REMOTE_ONLY]
                     = (int) a.getDimension(R.styleable.VTPanel_panelWidthInCall, 0.0f);
         mRightState[VTPANEL_MODE_KEYPAD_LOCAL]
                     = mRightState[VTPANEL_MODE_KEYPAD_REMOTE]
                     = mRightState[VTPANEL_MODE_KEYPAD_REMOTE_ONLY]
                     = (int) a.getDimension(R.styleable.VTPanel_panelWidthKeypad, 0.0f);

	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		log("onmeasure...");
		int widthSpecMode = MeasureSpec.getMode(widthMeasureSpec);
		int widthSpecSize = MeasureSpec.getSize(widthMeasureSpec);

		int heightSpecMode = MeasureSpec.getMode(heightMeasureSpec);
		int heightSpecSize = MeasureSpec.getSize(heightMeasureSpec);

		if (widthSpecMode == MeasureSpec.UNSPECIFIED
				|| heightSpecMode == MeasureSpec.UNSPECIFIED) {
			throw new RuntimeException(
					"VTPanel cannot have UNSPECIFIED dimensions");
		}

		VTPanelComponent child;
		for (int i = 0; i < this.getChildCount(); i++) {
			child = (VTPanelComponent) this.getChildAt(i);
			if (child != null
					&& child.mVisibilities[VideoCallApp.sPanelMode] != View.GONE
					&& child.mPositions[VideoCallApp.sPanelMode] != null) {

				child.measure(
						MeasureSpec.makeMeasureSpec(
								child.mAnimPos.right - child.mAnimPos.left,
								MeasureSpec.EXACTLY),
						MeasureSpec.makeMeasureSpec(
								child.mAnimPos.bottom - child.mAnimPos.top,
								MeasureSpec.EXACTLY)
						);
			}
		}

		heightSpecSize = getPanelBottom();
        widthSpecSize = getPanelRight();
		setMeasuredDimension(widthSpecSize, heightSpecSize);
	}

	@Override
	protected void dispatchDraw(Canvas canvas) {
		/* draw components */
		VTPanelComponent child;
		final long drawingTime = getDrawingTime();
		for(int i = 0; i < this.getChildCount(); i ++) {
			child = (VTPanelComponent)this.getChildAt(i);
			if (child.mVisibilities[VideoCallApp.sPanelMode] != View.VISIBLE) {
				continue;
			}
			//log("draw child: " + child);
			drawChild(canvas, child, drawingTime);
		}
	}

	@Override
	protected void onLayout(boolean changed, int l, int t, int r, int b) {
		log("onLayout...l: " + l + " t: " + t + " r: " + r + " b: " + b);
		/* layout components */
		VTPanelComponent child;
		for (int i = 0; i < this.getChildCount(); i++) {
			child = (VTPanelComponent) this.getChildAt(i);
			if (child.mVisibilities[VideoCallApp.sPanelMode] != View.VISIBLE) {
				continue;
			}
			log("child(mState): " + child + "(" + VideoCallApp.sPanelMode + ")");
			log("visibility: " + child.mVisibilities[VideoCallApp.sPanelMode]);
			log("child Rect: " + child.mAnimPos);
			/* layout to animate pos */
			child.layout(l + child.mAnimPos.left + child.mMarginLeft,
					t + child.mAnimPos.top + child.mMarginTop,
					l + child.mAnimPos.right + child.mMarginLeft,
					t + child.mAnimPos.bottom + child.mMarginTop);
		}
	}


	/*
	 * set to a certain mode
	 * just for retrieve UI state for VTPanel after returning from home screen
	 */

	@Override
	protected void onFinishInflate() {
		// TODO Auto-generated method stub
		VTPanelComponent child;
		for (int i = 0; i < this.getChildCount(); i ++) {
			child = (VTPanelComponent)this.getChildAt(i);
			child.setPanel(this);
		}
		super.onFinishInflate();
	}

	void switchMode(int nMode, int milliseconds) {
		log("switch mode: " + nMode + " pending mode: " + VideoCallApp.sPendingPanelMode);
		mHandler.removeCallbacks(mDelaySwitchModeRunnable);
		// TODO: check nMode is valid
/*		if(nMode!=VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING && nMode!=VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING)
		{
			View remote_workaround = VideoCallApp.getInstance().mVideoCallScreen.findViewById(R.id.img_remote_workaround);
			View local_workaround = VideoCallApp.getInstance().mVideoCallScreen.findViewById(R.id.img_local_workaround);
			if(local_workaround!=null)
			{local_workaround.setVisibility(View.VISIBLE);}
			if(remote_workaround!=null)
			{remote_workaround.setVisibility(View.VISIBLE);}
		}*/
		VideoCallApp.sPendingPanelMode = VideoCallApp.sPanelMode;
		VideoCallApp.sPanelMode = nMode;
		log("mState: " + VideoCallApp.sPanelMode + " mPendingState: "
				+ VideoCallApp.sPendingPanelMode);
		resetAnimPos(VideoCallApp.sPendingPanelMode);
		doAnimation();
		if (milliseconds != ANIMATION_DURATION_NONE) {
			// no animation
			mAnimationDuration = milliseconds;
			startAnimation();
		} else {
			// do animation
			mAnimationDuration = -1;
			mAnimationStartTime = SystemClock.uptimeMillis();
			doAnimation();
		}
/*		if(nMode!=VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING && nMode!=VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING)
		{
			View remote_workaround = VideoCallApp.getInstance().mVideoCallScreen.findViewById(R.id.img_remote_workaround);
			View local_workaround = VideoCallApp.getInstance().mVideoCallScreen.findViewById(R.id.img_local_workaround);
			if(local_workaround!=null)
			{local_workaround.setVisibility(View.GONE);}
			if(remote_workaround!=null)
			{remote_workaround.setVisibility(View.GONE);}
		}*/
	}

	private Runnable mDelaySwitchModeRunnable = new Runnable() {
		public void run() {
			switchMode(mDelayedTargetState, ANIMATION_DURATION);
		}
	};

	public void delaySwitchMode(int newMode) {
		delaySwitchMode(newMode, DELAY_SWITCH_MODE_TIME);
	}

	public void delaySwitchMode(int newMode, int ms_delay) {
		mHandler.removeCallbacks(mDelaySwitchModeRunnable);
		mDelayedTargetState = newMode;
		mHandler.postDelayed(mDelaySwitchModeRunnable, ms_delay);
	}

	public void cancelDelayedSwitchMode() {
		mHandler.removeCallbacks(mDelaySwitchModeRunnable);
	}




	private void startAnimation() {

		mAnimationStartTime = SystemClock.uptimeMillis();
		mHandler.sendMessageAtTime(mHandler.obtainMessage(MSG_ANIMATE),
				(mAnimationStartTime + ANIMATION_FRAME_DURATION));

		VTPanelComponent child;

		for (int i = 0; i < getChildCount(); i ++) {
			child = (VTPanelComponent) getChildAt(i);
			if (child.mAnims[VideoCallApp.sPendingPanelMode][VideoCallApp.sPanelMode] != null) {
				child.mAnims[VideoCallApp.sPendingPanelMode][VideoCallApp.sPanelMode].start(mAnimationStartTime);
			}
		}
		mPanelManager.onAnimationStart();
		mIsAnimating = true;
		return;
	}

	private void doAnimation() {
		log("doAnimation...");
		long now = SystemClock.uptimeMillis();
		if ((now - mAnimationStartTime) > mAnimationDuration) {
			// Animation finish
			log("Animation End");
			mIsAnimating = false;

			int eventArg = 0;
			if ((VideoCallApp.sPendingPanelMode == VTPANEL_MODE_KEYPAD_LOCAL && VideoCallApp.sPanelMode == VTPANEL_MODE_LOCAL)
					|| (VideoCallApp.sPendingPanelMode == VTPANEL_MODE_KEYPAD_REMOTE && VideoCallApp.sPanelMode == VTPANEL_MODE_REMOTE)) {
				eventArg = VTPANEL_SHOW_ACTION_BUTTONS;
			}
			mPanelManager.onAnimationEnd();

		//requestLayout();
			invalidate();
			resetAnimPos(VideoCallApp.sPanelMode);
			/* modified end */
			requestLayout();
			invalidate();
			return;
		}

		VTPanelComponent child;
		for (int i = 0; i < getChildCount(); i ++) {
			child = (VTPanelComponent) getChildAt(i);
			if (child.mAnims[VideoCallApp.sPendingPanelMode][VideoCallApp.sPanelMode] != null) {
				log("child do animation...child: " + child);
				child.mAnims[VideoCallApp.sPendingPanelMode][VideoCallApp.sPanelMode]
						.getTransformation(now,
								child.mPositions[VideoCallApp.sPendingPanelMode],
								child.mAnimPos);
			}
		}
		requestLayout();
		log("invalidate()");
		invalidate();
		mHandler.sendMessageAtTime(mHandler.obtainMessage(MSG_ANIMATE),
				(now + ANIMATION_FRAME_DURATION));
	}

	private class AnimHandler extends Handler {
		public void handleMessage(Message m) {
			log("handle Message :" + m);
			switch (m.what) {
			case MSG_ANIMATE:
				doAnimation();
				break;
			default:
				break;
			}
		}
	}

	void resetAnimPos(int state) {
		VTPanelComponent child;
		// set animate start pos
		for (int i = 0; i < getChildCount(); i ++ ) {
			child = (VTPanelComponent)getChildAt(i);
			child.setAnimStartPos(state);
		}
	}

	private int getPanelBottom() {
		int bottom = mBottomState[VideoCallApp.sPanelMode];
		if (mIsAnimating) {
			long now = SystemClock.uptimeMillis();
			float normalizedTime = ((float) (now - mAnimationStartTime))
					/ (float) mAnimationDuration;

			boolean expired = normalizedTime >= 1.0f;
			// Pin time to 0.0 to 1.0 range
			normalizedTime = Math.max(Math.min(normalizedTime, 1.0f), 0.0f); //
			AccelerateDecelerateInterpolator interpolator = new AccelerateDecelerateInterpolator();

			if ((normalizedTime >= 0.0f) && (normalizedTime <= 1.0f)) {
				final float interpolatedTime = interpolator
						.getInterpolation(normalizedTime);
				bottom = (int) (mBottomState[VideoCallApp.sPendingPanelMode] + interpolatedTime
						* (mBottomState[VideoCallApp.sPanelMode] - mBottomState[VideoCallApp.sPendingPanelMode]));
			}
		}
		return bottom;
	}
	private int getPanelRight(){
        int right = mRightState[VideoCallApp.sPanelMode];
        return right;
    }
	public boolean addVTPanelComponent(VTPanelComponent child) {
		if (child != null) {
			addView(child);

		}
		return false;
	}

	@Override
	public boolean onInterceptTouchEvent(MotionEvent event) {

		final int action = event.getAction();
		return true;
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {

		// TODO Auto-generated method stub
		if (mIsAnimating == true)
			return false;

		boolean gesture = mGestureDetector.onTouchEvent(event);
		log("onTouchEvent:....event: " + event + " mode: "
				+ VideoCallApp.sPanelMode + " gesture: " + gesture);

		int x = (int)event.getX();
		int y = (int)event.getY();

		int action = event.getAction();

		VTPanelComponent child;
		for (int i = 0; i < this.getChildCount(); i++) {
			child = (VTPanelComponent)this.getChildAt(i);
//			log("onTouch event");
			if (child == null
					|| child.mVisibilities[VideoCallApp.sPanelMode] != View.VISIBLE) {
				continue;
			}

//			log("event: " + event);
//			log("child: " + child + " child.rect: " + child.mPositions[mState]);
			if (gesture == true
					|| !child.mPositions[VideoCallApp.sPanelMode]
							.contains(x, y)) {
				log("child position: " + child.mPositions[VideoCallApp.sPanelMode]);
				log("child.mPositions[VideoCallApp.sPanelMode].contains(x, y)? " + child.mPositions[VideoCallApp.sPanelMode].contains(x, y));
				log("touch down: " + child.mhasTouchDown);
				// cancel down action
				if (child.mhasTouchDown == true) {
					log("set cancel event...child: " + child);
					event.setAction(MotionEvent.ACTION_CANCEL);
					child.mhasTouchDown = false;
					child.dispatchTouchEvent(event);
					event.setAction(action);
				}
			} else {
				// gesture == false && 	child.mPositions[mState].contains(x, y) == true
				log("child: " + child);
				event.setLocation(x	- child.mPositions[VideoCallApp.sPanelMode].left, y - child.mPositions[VideoCallApp.sPanelMode].top);
				child.dispatchTouchEvent(event);

				if (action == MotionEvent.ACTION_DOWN) {
					child.mhasTouchDown = true;
				} else if (action == MotionEvent.ACTION_CANCEL || action == MotionEvent.ACTION_OUTSIDE || action == MotionEvent.ACTION_UP) {
					child.mhasTouchDown = false;
				}
			}
		}
		return true;
	}

	private GestureDetector mGestureDetector = new GestureDetector(mContext,
			new GestureDetector.SimpleOnGestureListener() {

				@Override
				public boolean onFling(MotionEvent e1, MotionEvent e2,
						float velocityX, float velocityY) {
					// TODO Auto-generated method stub
					log("onFling...");
					return mPanelManager.onFlingAction(e1, e2, velocityX, velocityY);
				}

				@Override
				public void onLongPress(MotionEvent e) {
					// TODO Auto-generated method stub
					super.onLongPress(e);
				}

			});


	private void log(String info) {
		if (DBG) {
			if (MyLog.DEBUG) MyLog.d(TAG, info);
		}
	}

	interface IPanelManager {
		boolean onFlingAction(MotionEvent e1, MotionEvent e2,
				float velocityX, float velocityY);
		void onTouchDownEvent(VTPanelComponent target, MotionEvent ev);
		void onClickEvent(VTPanelComponent target);
		void onAnimationStart();
		void onAnimationEnd();
		void onTouchCancelEvent(VTPanelComponent target, MotionEvent ev);
	};

	void setPanelManager(IPanelManager mgr) {
		mPanelManager = mgr;
	}

	IPanelManager getPanelManager() {
		return mPanelManager;
	}
}
