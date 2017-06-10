/*
 * Copyright (c) 2012 - 2013 QUALCOMM Technologies, Inc.  All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 */

package com.qualcomm.wfd.client;

import com.qualcomm.wfd.WfdEnums;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.os.RemoteException;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import com.qualcomm.wfd.WfdStatus;
import com.qualcomm.wfd.WfdEnums;
import android.view.OrientationEventListener;
import android.content.res.Configuration;

//public class WfdSurface extends SurfaceView implements SurfaceHolder.Callback, Runnable {
public class WfdSurface extends SurfaceView implements SurfaceHolder.Callback {

	private static final String TAG = "Client.WfdSurface";
	SurfaceHolder surfaceHolder;
	Thread thread = null;
	volatile boolean running = false;
	private boolean bCaptureEvents = false;
        private boolean bDisablePauseOnSurfaceDestroyed = false;
	private ScaleGestureDetector mScaleDetector;
        private int mHeight=0;
        private int mWidth=0;
        private int mOrientation =0;
        private WfdOrientationEventListener mOrientationListener;
        private WfdStatus beforeDestroy, currentStatus;
        private WiFiUtil wifiUtil;
	private class WfdOrientationEventListener extends OrientationEventListener {
	public WfdOrientationEventListener(Context acontext) {
		super(acontext);
	}

	@Override
	public void onOrientationChanged(int orientation) {
		if (orientation == ORIENTATION_UNKNOWN)
			return;
		//  determine our orientation based on sensor response
                if(!ServiceUtil.getmServiceAlreadyBound()) {
                  return;
                }
		mOrientation = orientation;
		try {
		int ret = ServiceUtil.getInstance().setSurfaceProp(mWidth,mHeight,mOrientation);
		if (ret != 0) {
		    Log.e(TAG, "Error: " + WfdEnums.ErrorType.getValue(ret));
		}
		} catch (RemoteException e) {
			Log.e(TAG, "RemoteException", e);
		}
	}
	};

	public WfdSurface(Context context, AttributeSet attributeSet) {
		super(context, attributeSet);
		Log.d(TAG, "constructor called");
		surfaceHolder = getHolder();
		getHolder().addCallback(this);
		mOrientationListener = new WfdOrientationEventListener(context);
		mOrientationListener.enable();
                beforeDestroy = new WfdStatus();
                currentStatus = new WfdStatus();
                beforeDestroy.state = WfdEnums.SessionState.INVALID.ordinal();
                currentStatus.state = WfdEnums.SessionState.INVALID.ordinal();
                wifiUtil = WiFiUtil.getInstance(null,null);
	}

	@Override
	public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) {
		Log.d(TAG, "surfaceChanged() called");
		mWidth  = arg2;
		mHeight = arg3;
                Configuration config = getResources().getConfiguration();
                mOrientation = config.orientation;
		try {
		int ret = ServiceUtil.getInstance().setSurfaceProp(mWidth,mHeight,mOrientation);
		if (ret != 0) {
		    Log.e(TAG, "Error: " + WfdEnums.ErrorType.getValue(ret));
		}
		} catch (RemoteException e) {
			Log.e(TAG, "RemoteException", e);
		}
	}

	@Override
	public void surfaceCreated(SurfaceHolder arg0) {
		Log.d(TAG, "surfaceCreated() called");
		Log.d(TAG, "surfaceHolder: " + getHolder());
		Log.d(TAG, "surface: " + getHolder().getSurface());
		setFocusable(true);
		setFocusableInTouchMode(true);
		boolean focusStatus = requestFocus();
		Log.d(TAG, "focus status:" + focusStatus);
                if (wifiUtil.manager != null) {
                     Log.d(TAG, "MIRACAST_SINK");
                     wifiUtil.manager.setMiracastMode(wifiUtil.manager.MIRACAST_SINK);
                }
		if (surfaceHolder.getSurface().isValid()) {
			Log.d(TAG, "surfaceHolder.getSurface().isValid() is true");
			/*Canvas canvas = surfaceHolder.lockCanvas();
			Bitmap myBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.quic_device);
			canvas.drawBitmap(myBitmap, 0, 0, null);
			surfaceHolder.unlockCanvasAndPost(canvas);*/
		} else {
			Log.d(TAG, "surfaceHolder.getSurface().isValid() is false");
		}

        try {
            Log.d(TAG, "setSurface() called");
            ServiceUtil.getInstance().setSurface(getHolder().getSurface());
        } catch (RemoteException e) {
            Log.e(TAG, "setSurface()- Remote exception: ", e);
        }

        try {
            currentStatus = ServiceUtil.getInstance().getStatus();
            Log.d(TAG, "Session state during surface creation is "+ currentStatus.state);
        } catch (RemoteException e) {
            Log.e(TAG, "Remote Exception ",e);
        }

        if(!bDisablePauseOnSurfaceDestroyed &&
           currentStatus.state != WfdEnums.SessionState.STANDING_BY.ordinal()
           && currentStatus.state != WfdEnums.SessionState.STANDBY.ordinal()){
          currentStatus.state = WfdEnums.SessionState.INVALID.ordinal();
          if (beforeDestroy.state == WfdEnums.SessionState.PLAY.ordinal()) {
             Log.d(TAG, "Calling play on surface creation");
             beforeDestroy.state = WfdEnums.SessionState.INVALID.ordinal();
             try {
                 ServiceUtil.getInstance().play();
             } catch (RemoteException e) {
                 Log.d(TAG, "Remote Exception ", e);
             }
          }
        }
        }

	@Override
	public void surfaceDestroyed(SurfaceHolder arg0) {
           Log.d(TAG, "surfaceDestroyed() called");
           try {
               if(ServiceUtil.getmServiceAlreadyBound() != false) {
                   beforeDestroy = ServiceUtil.getInstance().getStatus();
                   Log.d(TAG, "Session state is " + beforeDestroy.state);
               } else {
                   Log.d(TAG, "WFDSession Instance not available");
                   return;
               }
           } catch (RemoteException e) {
               Log.e(TAG, "setSurface()- Remote exception: ", e);
           }

           if (wifiUtil.manager != null) {
               Log.d(TAG, "MIRACAST_DISABLED");
               wifiUtil.manager.setMiracastMode(wifiUtil.manager.MIRACAST_DISABLED);
           }

           try {
               Log.d(TAG, "setSurface() called");
               ServiceUtil.getInstance().setSurface(null);
               //using null as a poison pointer
           } catch (RemoteException e) {
               Log.e(TAG, "setSurface()- Remote exception: ", e);
           }

           if (!bDisablePauseOnSurfaceDestroyed  && beforeDestroy.state == WfdEnums.SessionState.PLAY.ordinal()) {
               Log.d(TAG, "Calling pause on surface destruction");
               try {
                   ServiceUtil.getInstance().pause();
               } catch (RemoteException e) {
                   Log.d(TAG, "Remote Exception ", e);
               }
           }
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
	    Log.d(TAG, "onTouchEvent() called");
	    if (bCaptureEvents) {
	        Log.d(TAG, "onTouchEvent()- capturing events");
	        // Use ScaleGestureDetector to find scale gesture
	        try {
	            int ret = ServiceUtil.getInstance().sendEvent(event);
	            if (ret != 0) {
	                Log.e(TAG, "Error: " + WfdEnums.ErrorType.getValue(ret));
	                return false;
	            }
	        } catch (RemoteException e) {
	            Log.e(TAG, "RemoteException", e);
	            return false;
	        }
	        return true;
	    }
	    return super.onTouchEvent(event);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
	    Log.d(TAG, "onKeyUp() called with keycode: " + event);
	    if (bCaptureEvents) {
	        Log.d(TAG, "onKeyUp() called while capturing events with keycode: " + keyCode);
	        try {
	            int ret = ServiceUtil.getInstance().sendEvent(event);
	            if (ret != 0) {
	                Log.e(TAG, "Error: " + WfdEnums.ErrorType.getValue(ret));
	            }
	        } catch (RemoteException e) {
	            Log.e(TAG, "RemoteException", e);
	        }
	    }
	    return false; //allow other key receivers to handle the event
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
	    Log.e(TAG, "onKeyDown() called with keycode: " + event);
	    if (bCaptureEvents) {
	        try {
	            int ret = ServiceUtil.getInstance().sendEvent(event);
	            if (ret != 0) {
	                Log.e(TAG, "Error: " + WfdEnums.ErrorType.getValue(ret));
	            }
	        } catch (RemoteException e) {
	            Log.e(TAG, "RemoteException", e);
	        }
	    }
	    return false; //allow other key receivers to handle the event
	}

	/**
	 * Function to start capturing UIBC event.  This function can only be called on WFD sink.
	 * @return
	 */
	public boolean startUIBCEventCapture() {
	    bCaptureEvents = true;
	    return true;
	}

	/**
	 * Function to stop capturing UIBC event.  This function can only be called on WFD sink.
	 * @return
	 */
	public boolean stopUIBCEventCapture() {
	    bCaptureEvents = false;
	    return true;
	}

        /**
         * Function to set whether or not to disable pause when Surface
         * is destroyed
         * @return
         */
        public void disablePauseOnSurfaceDestroyed() {
            bDisablePauseOnSurfaceDestroyed = true;
        }

	/*private class ScaleGesterListener extends ScaleGestureDetector.SimpleOnScaleGestureListener {
	    @Override
	    public boolean onScale(ScaleGestureDetector detector) {
	    	// TODO: send UIBC ZOOM event data from sink to source
	    	float zf = detector.getScaleFactor();
	        int iPart = (int)zf;
	        //int fPart = zf-iPart;		// how to represent fractional part as an integer?
	        detector.getFocusX();
	        detector.getFocusY();
	        return true;
	    }
	}*/
}
