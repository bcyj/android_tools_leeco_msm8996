/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 *
 * Copyright (C) 2008 The Android Open Source Project
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

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.TextView;
import com.android.internal.widget.SlidingTab;
import com.android.internal.widget.SlidingTab.OnTriggerListener;

/**
 *
 * Manage sliding card
 *
 */
public class VTSlidingCardManager implements ViewTreeObserver.OnGlobalLayoutListener,OnTriggerListener{

	static private final boolean DBG = true;
	static final String TAG = "VTSlidingCardManager";

	/* VideoCallScreen */
	VideoCallScreen mScreen;
	/* VideoCall Application */
	VideoCallApp mApp;
	/* the view group popup card appending on */
	private VTSlidingCardPanel mMainFrame;
	private View mPopupView;
	private WindowAttachNotifierView wanv;
	/* popup card position */
	private int mPopupUpTop;
	private int mPopupDownTop;
	private int mPopupMidPosTop;
	protected SlidingTab mIncomingSelector;
	// Temporary int array used with various getLocationInWindow() calls.
    private int[] mTempLocation = new int[2]; // Use this only from the main

	/* Y-coordinate of the DOWN event that started */
	private int mTouchDownY;
	private boolean mSlideInProgress = false;

    /* Check if the user has slid the CallCard more than 1/4 of the
       way down. (If the screen is currently off, doing this will
       turn it back on */
	private boolean mPartialSlideEventSent;

	/* Popup Window */
	private PopupWindow mPopup;
	private boolean mPopupRequested = true;

	public VTSlidingCardManager(VideoCallScreen screen, VideoCallApp app) {
		// TODO Auto-generated constructor stub
		mScreen = screen;
		mApp = app;
	}

	void init() {
		log("init...");
		mIncomingSelector = (SlidingTab)mScreen.findViewById(R.id.tab_selector);
		mIncomingSelector.setHoldAfterTrigger(true, true);
		//mIncomingSelector.
		mIncomingSelector.setLeftTabResources(R.drawable.ic_answer_video_call,
				R.drawable.cmcc_phone_target_green,
				R.drawable.jog_tab_bar_left_answer,
				R.drawable.jog_tab_left_answer);
		mIncomingSelector.setRightTabResources(R.drawable.ic_reject_video_call,
				R.drawable.cmcc_phone_target_red,
				R.drawable.jog_tab_bar_right_decline,
				R.drawable.jog_tab_right_decline);
		mIncomingSelector.setOnTriggerListener(this);

	}


    /**
     * Updates the PopupWindow's size and makes it visible onscreen.
     *
     * This needs to be done *after* our main UI gets laid out and attached to
     * its Window, because (1) we base the popup's size on the post-layout size
     * of elements in our main View hierarchy, and (2) we need to have a valid
     * window token for our call to mPopup.showAtLocation().
     */
    void showPopup() {
	log("showPopup...");
	mPopupRequested = true;
        if (mScreen == null) {
            log("showPopup() called, but the InCallScreen activity is gone!");
            return;
        }

        if (mScreen.isFinishing()) {
            log("showPopup: Activity finishing! Bailing out...");
            return;
        }

        if (mPopup == null) {
		log("showPopup: popup window has not been initiated");
		return ;
        }

        mMainFrame.setVisibility(View.VISIBLE);
        mMainFrame.setFocusable(true);
        if (mMainFrame.getWindowToken() == null) {
            log("showPopup: no window token yet! Bailing out...");
            return;
        }
        mPopup.setWidth(mMainFrame.getWidth());
        mPopup.setHeight(mMainFrame.getHeight());

       // mPopup.setWidth(320);
       // mPopup.setHeight(173);

        try {
            // This is a no-op if mPopup is already showing.
		log("##########show popup#################");
            mPopup.showAtLocation(mMainFrame, Gravity.TOP | Gravity.LEFT,
                            0,
					mPopupMidPosTop);
			log("popup is showing:" + mPopup.isShowing());
			// log("call card visibility:" + mCallCard.getVisibility());
			log("popupWindow visible:"
					+ mPopup.getContentView().getVisibility());
			log("popup width:" + mPopup.getWidth() + " height:"
					+ mPopup.getHeight());
			log("popup loc x:" + 0 + " y:" + mPopupMidPosTop);

        } catch (WindowManager.BadTokenException e) {
            // This should never happen; we've already checked that
            // mMainFrame.getWindowToken() is non-null, and we're also
            // careful never to call this method *after* the InCallScreen
            // activity has been destroyed.
            Log.e(TAG, "BadTokenException from mPopup.showAtLocation()", e);
        }
    }

    void dismissPopup() {
        log("**************dismissPopup()...******************");
        mPopupRequested = false;
        if (mPopup != null)
            mPopup.dismiss(); // This is a no-op if mPopup is not showing
        if (mMainFrame != null) {
	        if(wanv!= null)
	        {
			wanv.getViewTreeObserver().removeGlobalOnLayoutListener(this);
			log("Add by nicki: mMainFrame.removeView");
			mMainFrame.removeView(wanv);
			wanv = null;
	        }
		mMainFrame.setVisibility(View.GONE);
        }
    }



    /**
     * Bail out of an in-progress slide *without* activating whatever action the
     * slide was supposed to trigger.
     */
    private void abortSlide() {
        log("abortSlide()...");
        mSlideInProgress = false;
        // This slide had no effect. Nothing about the state of the
        // UI has changed, so no need to updateCardPreferredPosition() or
        // updateCardSlideHints(). But we *do* need to reposition the
        // PopupWindow back in its correct position.

        // TODO: smoothly animate back to the preferred position.
        mPopup.update(0, mPopupMidPosTop, -1, -1);
    }


    public void handleCallCardRightButtonPress() {
	 log("==> Successful right to left slide!");
         mScreen.onRefuseClicked();
    }
	public void handleCallCardLeftButtonPress() {
		log("==> Successful left to right slide!");
         mScreen.onAccept();
	}

    public void onGlobalLayout() {
		// TODO Auto-generated method stub
        log("============onGlobalLayout() get invoked" );
        log("popup requested:" + mPopupRequested + "is showing:"
                        + mPopup.isShowing());
	if (mPopupRequested && !mPopup.isShowing()) {
            showPopup();
        }
	}

    /**
     * Trivial subclass of View that notifies the InCallScreen when its
     * onAttachedToWindow() method gets called.
     *
     * This is needed because we can't call showPopup() directly from onCreate()
     * or onResume(), because we're not allowed to call mPopup.showAtLocation()
     * until after we have a window token (ie. after the main View hierarchy
     * first gets attached to a Window.)
     *
     * It's easy for a View to notice when that happens, since all Views in the
     * hierarchy get their onAttachedToWindow() methods called. But
     * unfortunately there's no easy way for the *Activity* to notice this has
     * happened.
     *
     * So we add an instance of this class to the main InCallScreen View
     * hierarchy, purely to get notified when the View hierarchy first gets
     * attached to its Window, at which point we're finally able to show the
     * sliding CallCard PopupWindow.
     *
     * TODO: This is cumbersome. Could we instead have a standard Activity
     * callback for this?
     */
     static class WindowAttachNotifierView extends View {
        private VTSlidingCardManager mSlidingManager;

        WindowAttachNotifierView(Context c) {
            super(c);
        }

        public void setSlidingCardManager(VTSlidingCardManager slidingCardManager) {
		mSlidingManager = slidingCardManager;
        }

        @Override
        protected void onAttachedToWindow() {
            // This is called when the view is attached to a window.
            // At this point it has a Surface and will start drawing.
            //if (DBG)
		mSlidingManager.log("WindowAttachNotifierView: onAttachedToWindow!");
            super.onAttachedToWindow();

            // The code in showPopup() needs to know the sizes and
            // positions of some views in the mMainFrame view hierarchy,
            // in order to set the popup window's size and position. That
            // means that showPopup() needs to be called *after* the whole
            // in-call UI has been measured and laid out. At this point
            // that hasn't happened yet, so we can't directly call
            // mSlidingCardManager.showPopup() from here.
            //
            // Also, to reduce flicker onscreen, we'd like the PopupWindow
            // to appear *before* any of the main view hierarchy becomes
            // visible. So we use the main view hierarchy's
            // ViewTreeObserver to get notified *after* the layout
            // happens, but before anything gets drawn.
            //
            // TODO: This still isn't perfect, though. Even though I call
            // PopupWindow.showAtLocation() before the View hierarchy
            // underneath even gets drawn, there's still a very short
            // interval where the main View hierarchy is shown to the user
            // *before* the PopupWindow actually becomes visible on top.
            // To fix this, we'd need a synchronous way to tell the
            // WindowManager to make a given window visible *right now*,
            // or at least a way to find out when a new window actually
            // does become visible onscreen. See bug 1152287 for more
            // info.

            // Get the ViewTreeObserver for the main InCallScreen view
            // hierarchy. (You can only call getViewTreeObserver() after
            // the view tree gets attached to a Window, which is why we do
            // this here rather than in InCallScreen.onCreate().)
            final ViewTreeObserver viewTreeObserver = getViewTreeObserver();

            // Arrange for the SlidingCardManager to get called after
            // the main view tree has been laid out.
            // (addOnPreDrawListener() would also be basically equivalent here.)
            viewTreeObserver.addOnGlobalLayoutListener(mSlidingManager);

            // See SlidingCardManager.onGlobalLayout() for the next step.
        }

        @Override
        protected void onDetachedFromWindow() {
            // This is called when the view is detached from a window.
            // At this point it no longer has a surface for drawing.
            //if (DBG)
		mSlidingManager.log("WindowAttachNotifierView: onDetachedFromWindow!");
            super.onDetachedFromWindow();

            // Nothing necessary here (yet) since we already
            // dismiss the popup from onDestroy().
        }
    }

    /**
     *
     * Update caller info on PopupCard
     */
    void updateCallerInfo(String title,String number, String cityName, String name) {
	log("updateCallerInfo: number: " + number + " name: " + name);
	TextView titleView = (TextView) mScreen.findViewById(R.id.popup_call_title);
	TextView numView = (TextView) mScreen.findViewById(R.id.popup_caller_name);
	TextView cityView = (TextView) mScreen.findViewById(R.id.popup_caller_location);
	TextView nameView = (TextView) mScreen.findViewById(R.id.popup_caller_number);
	ImageView picView = (ImageView)mScreen.findViewById(R.id.call_card_picture);
	if(name != null)
	{
		if(numView != null)
		{
			titleView.setText(title);
			nameView.setText(name);
			numView.setText(number);
		}
	}
	else
	{
		if(numView != null)
		{
			titleView.setText(title);
			nameView.setText(number);
			numView.setText("");
		}
	}
	if (cityView != null) {
		cityView.setText(cityName);
	}
		Bitmap  bm = mApp.getCallerPicture();
		if(bm!=null)
		picView.setImageBitmap(bm);
    }

	private void log(String info) {
		if (DBG) {
			if (MyLog.DEBUG) MyLog.d(TAG, info);
		}
	}
	public void onGrabbedStateChange(View v, int grabbedState) {
		// TODO Auto-generated method stub

	}
	public void onTrigger(View v, int whichHandle) {
		// TODO Auto-generated method stub
		if (whichHandle == SlidingTab.OnTriggerListener.LEFT_HANDLE) {
			handleCallCardLeftButtonPress();
		} else if (whichHandle == SlidingTab.OnTriggerListener.RIGHT_HANDLE) {
			handleCallCardRightButtonPress();
		}
	}
}
