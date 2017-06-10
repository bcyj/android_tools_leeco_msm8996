/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.widget.LinearLayout;

public abstract class VTPanelComponent extends LinearLayout {
	static final String TAG = "VT/VTPanelComponent";
	static final boolean DBG = true;
	boolean mhasTouchDown = false;
	VTPanel mPanel;

	// positions for various of modes
	protected Rect[] mPositions = new Rect[VTPanel.VTPANEL_MODE_COUNT];

	protected int[] mVisibilities = new int[VTPanel.VTPANEL_MODE_COUNT];

	protected int mMarginLeft;
	protected int mMarginTop;

	// Animating position
	protected Rect mAnimPos = new Rect();
	protected AnimAction[][] mAnims = new AnimAction[VTPanel.VTPANEL_MODE_COUNT][VTPanel.VTPANEL_MODE_COUNT];

	public VTPanelComponent(Context context, AttributeSet attrs) {
		super(context, attrs);
		//initLayoutParam(context, attrs);
		InitVisibility();
		//InitAnimParams();
	}

	protected void initLayoutParam(Context context, AttributeSet attrs) {
		TypedArray a = context.obtainStyledAttributes(attrs,
				R.styleable.VTPanelComponent);
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_LOCAL] = new Rect();
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_LOCAL].left = (int) a.getDimension(R.styleable.VTPanelComponent_left_keypad_local, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_LOCAL].top = (int) a.getDimension(R.styleable.VTPanelComponent_top_keypad_local, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_LOCAL].right = mPositions[VTPanel.VTPANEL_MODE_KEYPAD_LOCAL].left
			+ (int) a.getDimension(R.styleable.VTPanelComponent_width_keypad_local, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_LOCAL].bottom = mPositions[VTPanel.VTPANEL_MODE_KEYPAD_LOCAL].top
			+ (int) a.getDimension(R.styleable.VTPanelComponent_height_keypad_local, 0.0f);

		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE] = new Rect();
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE].left = (int) a.getDimension(R.styleable.VTPanelComponent_left_keypad_remote, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE].top = (int) a.getDimension(R.styleable.VTPanelComponent_top_keypad_remote, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE].right = mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE].left
			+ (int) a.getDimension(R.styleable.VTPanelComponent_width_keypad_remote, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE].bottom = mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE].top
			+ (int) a.getDimension(R.styleable.VTPanelComponent_height_keypad_remote, 0.0f);

		mPositions[VTPanel.VTPANEL_MODE_LOCAL] = new Rect();
		mPositions[VTPanel.VTPANEL_MODE_LOCAL].left = (int) a.getDimension(R.styleable.VTPanelComponent_left_local, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_LOCAL].top = (int) a.getDimension(R.styleable.VTPanelComponent_top_local, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_LOCAL].right = mPositions[VTPanel.VTPANEL_MODE_LOCAL].left
			+ (int) a.getDimension(R.styleable.VTPanelComponent_width_local, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_LOCAL].bottom = mPositions[VTPanel.VTPANEL_MODE_LOCAL].top
			+ (int) a.getDimension(R.styleable.VTPanelComponent_height_local, 0.0f);

		mPositions[VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING] = new Rect();
		mPositions[VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING].left = (int) a.getDimension(R.styleable.VTPanelComponent_left_lock_connecting, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING].top = (int) a.getDimension(R.styleable.VTPanelComponent_top_lock_connecting, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING].right = mPositions[VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING].left
			+ (int) a.getDimension(R.styleable.VTPanelComponent_width_lock_connecting, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING].bottom = mPositions[VTPanel.VTPANEL_MODE_ANSWERIN_CONNECTING].top
			+ (int) a.getDimension(R.styleable.VTPanelComponent_height_lock_connecting, 0.0f);

		mPositions[VTPanel.VTPANEL_MODE_REMOTE] = new Rect();
		mPositions[VTPanel.VTPANEL_MODE_REMOTE].left = (int) a.getDimension(R.styleable.VTPanelComponent_left_remote, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_REMOTE].top = (int) a.getDimension(R.styleable.VTPanelComponent_top_remote, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_REMOTE].right = mPositions[VTPanel.VTPANEL_MODE_REMOTE].left
			+ (int) a.getDimension(R.styleable.VTPanelComponent_width_remote, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_REMOTE].bottom = mPositions[VTPanel.VTPANEL_MODE_REMOTE].top
			+ (int) a.getDimension(R.styleable.VTPanelComponent_height_remote, 0.0f);

		mPositions[VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING] = new Rect();
		mPositions[VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING].left = (int) a.getDimension(R.styleable.VTPanelComponent_left_unlock_connecting, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING].top = (int) a.getDimension(R.styleable.VTPanelComponent_top_unlock_connecting, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING].right = mPositions[VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING].left
			+ (int) a.getDimension(R.styleable.VTPanelComponent_width_unlock_connecting, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING].bottom = mPositions[VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING].top
			+ (int) a.getDimension(R.styleable.VTPanelComponent_height_unlock_connecting, 0.0f);

		mPositions[VTPanel.VTPANEL_MODE_REMOTE_ONLY] = new Rect();
		mPositions[VTPanel.VTPANEL_MODE_REMOTE_ONLY].left = (int) a.getDimension(R.styleable.VTPanelComponent_left_remote_only, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_REMOTE_ONLY].top = (int) a.getDimension(R.styleable.VTPanelComponent_top_remote_only, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_REMOTE_ONLY].right = mPositions[VTPanel.VTPANEL_MODE_REMOTE_ONLY].left
			+ (int) a.getDimension(R.styleable.VTPanelComponent_width_remote_only, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_REMOTE_ONLY].bottom = mPositions[VTPanel.VTPANEL_MODE_REMOTE_ONLY].top
			+ (int) a.getDimension(R.styleable.VTPanelComponent_height_remote_only, 0.0f);

		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY] = new Rect();
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY].left = (int) a.getDimension(R.styleable.VTPanelComponent_left_keypad_remote_only, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY].top = (int) a.getDimension(R.styleable.VTPanelComponent_top_keypad_remote_only, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY].right = mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY].left
			+ (int) a.getDimension(R.styleable.VTPanelComponent_width_keypad_remote_only, 0.0f);
		mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY].bottom = mPositions[VTPanel.VTPANEL_MODE_KEYPAD_REMOTE_ONLY].top
			+ (int) a.getDimension(R.styleable.VTPanelComponent_height_keypad_remote_only, 0.0f);

	}


	/*
	 * Manager Animate for component
	 */
	class AnimTrans{
		private float mPivotX;
		private float mPivotY;
		private float mFromX;
		private float mFromY;
		private float mToX;
		private float mToY;
		private float mSX;
		private float mSY;

		private float mDX; /* horizontal offset */
		private float mDY; /* vertical offset */

		AnimTrans(float fFromPivotX, float fFromPivotY,
				float fToPivotX, float fToPivotY, float fFromX, float fToX,
				float fFromY, float fToY) {
			mPivotX = fFromPivotX;
			mPivotY = fFromPivotY;
			mFromX = fFromX;
			mFromY = fFromY;
			mToX = fToX;
			mToY = fToY;

			mDX = fToPivotX - fFromPivotX;
			mDY = fToPivotY - fFromPivotY;

			mSX = 1.0f;
			if (mFromX - mPivotX != 0.0) {
				mSX = (mToX - fToPivotX) / (mFromX - mPivotX);
			}

			mSY = 1.0f;
			if (mFromY - mPivotY != 0.0) {
				mSY = (mToY - fToPivotY) / (mFromY - mPivotY);
			}

		}

		// public void applyTrans( float interpolatedTime, Transformation t){
		public void applyTrans(float interpolatedTime, Rect src, Rect dst) {
			dst.left = (int) (((src.left - mPivotX) * (mSX - 1.0f) + mDX)
					* interpolatedTime + src.left);
			dst.right = (int) (((src.right - mPivotX) * (mSX - 1.0f) + mDX)
					* interpolatedTime + src.right);
			dst.top = (int) (((src.top - mPivotY) * (mSY - 1.0f) + mDY)
					* interpolatedTime + src.top);
			dst.bottom = (int) (((src.bottom - mPivotY) * (mSY - 1.0f) + mDY)
					* interpolatedTime + src.bottom);

			log("interpolatedTime: " + interpolatedTime);
			log("src: " + src);
			log("dst: " + dst);
			log("mDx: " + mDX + " mDy" + mDY + " mSx: " + mSX);
		}

		@Override
		public String toString() {
			return "AnimTrans{" + " mFromX=" + mFromX + " mFromY=" + mFromY
					+ " mToX=" + mToX + " mToY=" + mToY + " mDX=" + mDX
					+ " mDY=" + mDY + "}";
		}
	}

	class AnimAction {
		private long mStartTime = -1;
		private long mDuration;
		private long mFrameDuration;
		private AnimTrans mTrans;
		private AccelerateDecelerateInterpolator mInterpolator = new AccelerateDecelerateInterpolator();
		private boolean mIsStopped = true;

		public boolean getTransformation(long currentTime, Rect rSrc, Rect rDst) {

			log("object: " + VTPanelComponent.this + " getTransformation..mStartTime: " + mStartTime
						+ " mDuration: " + mDuration + " currentTime"
						+ currentTime);

			if (mIsStopped)
				return false;

			float normalizedTime = ((float) (currentTime - (mStartTime)))
					/ (float) mDuration;

			boolean expired = normalizedTime >= 1.0f;
			// Pin time to 0.0 to 1.0 range
			normalizedTime = Math.max(Math.min(normalizedTime, 1.0f), 0.0f);

			if ((normalizedTime >= 0.0f) && (normalizedTime <= 1.0f)) {

				final float interpolatedTime = mInterpolator
						.getInterpolation(normalizedTime);
				mTrans.applyTrans(interpolatedTime, rSrc, rDst);
			}

			if (expired)
				mIsStopped = true;

			return expired;
		}

		public AnimAction(long duration, long frameDuration, AnimTrans trans) {
			mDuration = duration;
			mFrameDuration = frameDuration;
			mTrans = trans;
		}

		public boolean isStopped() {
			return mIsStopped;
		}

		public void start(long startTime) {
			mStartTime = startTime;
			mIsStopped = false;
		}

		@Override
		public String toString() {
			// TODO Auto-generated method stub
			return "Trans: " + this.mTrans;
		}


	}

	/**
	 * Init Animate parameters
	 */
	void InitAnimParams() {
		for (int i = 0; i < VTPanel.VTPANEL_MODE_COUNT; i ++) {
			for (int j = 0; j < VTPanel.VTPANEL_MODE_COUNT; j++) {
				if (i == j || mVisibilities[i] != View.VISIBLE || mVisibilities[j] != View.VISIBLE) {
					mAnims[i][j] = null;
					continue;
				}
				AnimTrans scaleAndTranslate;

				/* local window translate and scale */
				scaleAndTranslate = new AnimTrans(
						mPositions[i].right,
						mPositions[i].top,
						mPositions[j].right,
						mPositions[j].top,
						mPositions[i].left,
						mPositions[j].left,
						mPositions[i].bottom,
						mPositions[j].bottom);
				mAnims[i][j] = new AnimAction(
						VTPanel.ANIMATION_DURATION, VTPanel.ANIMATION_FRAME_DURATION,
						scaleAndTranslate);
				//log("child: " + this + " Anims(i, j): " + "(" + i + "," + j + ") " + scaleAndTranslate);
			}
		}
	}

	/**
	 * set animate start pos
	 */
	void setAnimStartPos (int mode) {
		mAnimPos.left = mPositions [mode].left;
		mAnimPos.top = mPositions [mode].top;
		mAnimPos.right = mPositions [mode].right;
		mAnimPos.bottom = mPositions [mode].bottom;
	}

	/**
	 * Init Visibility for components in various modes
	 * Called in construct function
	 */
	abstract void InitVisibility();

	private void log(String info) {
		if (DBG) {
			if (MyLog.DEBUG) MyLog.d(TAG, info);
		}
	}

	void setPanel(VTPanel panel) {
		mPanel = panel;
	}
}
