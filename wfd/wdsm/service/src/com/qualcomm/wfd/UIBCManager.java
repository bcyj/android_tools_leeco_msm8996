/*==============================================================================
*        @file UIBCManager.java
*
*
*  Copyright (c) 2012 - 2013 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qualcomm.wfd;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

import com.qualcomm.compat.VersionedInputManager;
import com.qualcomm.wfd.WFDNative;
import com.qualcomm.wfd.WfdEnums.HIDDataType;
import com.qualcomm.wfd.WfdEnums.WFDDeviceType;

import android.os.RemoteException;
import android.util.Log;
import android.view.InputDevice;
import android.view.InputEvent;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class UIBCManager {
	private static final String TAG = "UIBCManager";
	private int deviceType;
	private VersionedInputManager inputManager;
	private EventReader eventReader;
	private EventDispatcher eventDispatcher;
	private Thread eventReaderThread, eventDispatcherThread;
    public static HIDCEventListener HIDCListener;
    /* constructor */
	public UIBCManager(int devType) throws InstantiationException, IllegalAccessException, ClassNotFoundException {
	    deviceType = devType;
		if (deviceType == WFDDeviceType.SOURCE.getCode()) {
			inputManager = VersionedInputManager.newInstance();
		}
	}

    static {
        HIDEventCallback(null, 0);//To prevent Progaurd stripping
    }
	/**
	 * Enable UIBC on control path.  This function is only called on WFD source to trigger RTSP request.
	 * When RTSP response is received, start() is used to start the data path.
	 * @return
	 */
	public boolean enable(int sessionId) {
               Log.d(TAG, "UIBCManager::enable " + sessionId);
               if (WFDNative.enableUIBC(sessionId)) {
                  return true;
               }
               Log.e(TAG,"Failed to enable UIBC");
               return false;
        }


	/**
	 * Disable UIBC on control path.  This function is only called on WFD source to trigger RTSP request.
	 * When RTSP response is received, stop() is used to stop the data path.
	 * @return
	 */
	public boolean disable(int sessionId) {
               Log.d(TAG, "UIBCManager::disable " + sessionId);
               if (WFDNative.disableUIBC(sessionId)) {
                  return true;
               }
               Log.e(TAG,"Failed to disable UIBC");
               return false;
        }

	/**
	 * Function to start UIBC data path.
	 * This function is called upon a successful RTSP message exchange to enable UIBC.
	 * @return
	 */
	public boolean start() {
		if (deviceType == WFDDeviceType.SOURCE.getCode()) {
			/* For source device, 1) start thread. 2) enable UIBCAdaptor. */
			eventDispatcher = new EventDispatcher();
			eventDispatcher.eventQueueStatus = true;
			// start EventDispatcher thread
			eventDispatcherThread = new Thread(eventDispatcher);
			eventDispatcher.running = true;
			eventDispatcherThread.start();
			// start UIBC in UIBCAdaptor
			WFDNative.startUIBC(eventDispatcher);
		} else {
			Log.d(TAG, "Create eventReader");
			/* For sink device, 1) enable UIBCAdaptor. 2) start thread. */
			eventReader = new EventReader();
			eventReader.eventQueueStatus = true;
			Log.d(TAG, "calling WFDNative.startUIBC()");
			// start UIBC in UIBCAdaptor
			WFDNative.startUIBC(eventReader);
			// start EventReader thread
			eventReaderThread = new Thread(eventReader);
			eventReader.running = true;
			eventReaderThread.start();
		}
		return true;
	}

	public void addUIBCEvent(InputEvent ev) {
		eventReader.addEvent(ev);
	}

	/**
	 * Function to stop UIBC data path
	 * This function is called upon a successful RTSP message exchange to disable UIBC.
	 * @return
	 */
	public boolean stop() {
		if (deviceType == WFDDeviceType.SOURCE.getCode()) {
			/* For source device, 1) disable UIBCAdaptor. 2) stop thread. */
			WFDNative.stopUIBC();								// (1) stop UIBC in UIBCAdaptor
			eventDispatcher.running = false;					// (2) stop EventDispatcher thread
			eventDispatcher.eventQueueStatus = false;						// (3) stop eventQueue loop
			try {
				eventDispatcherThread.join();					// (3) wait for EventDispatcher thread to finish
			} catch (InterruptedException e) {
				Log.e(TAG, "Error joining event dispatcher thread", e);
			}
		} else {
			/* For sink device, 1) stop thread. 2) disable UIBCAdaptor. */
			eventReader.running = false;						// (1) stop EventReader thread
			eventReader.eventQueueStatus = false;						    // (2) stop eventQueue loop
			try {
				eventReaderThread.join();						// (2) wait for EventReader thread to finish
			} catch (InterruptedException e) {
				Log.e(TAG, "Error joining reader thread", e);
			}
			WFDNative.stopUIBC();								// (4) stop UIBC in UIBCAdaptor
		}
		return true;
	}
    /**
     * EventReader maintains a queue, which stores the UI events generated locally from UI surface.When
     * event is available in queue, an internal thread consumes the event and sends the event to peer
     * device through UIBCAdaptor.
     * @author muhuanc
     *
     */
    class EventReader extends EventQueue implements Runnable {
        public boolean running = true;
        @Override
		public void run() {
            while (running) {
	            InputEvent ev = getNextEvent();
	            if (ev != null) {
	                if (ev instanceof MotionEvent) {
	                	/* send MotionEvent to UIBCAdaptor */
	                    WFDNative.sendUIBCMotionEvent((MotionEvent)ev);
	                } else if (ev instanceof KeyEvent) {
	                	WFDNative.sendUIBCKeyEvent((KeyEvent) ev);
	                } else {
	                	Log.e(TAG, "***** Unknown event received");
	                }
	            } /*else {
	            	Log.w(TAG, "*** event is null");
	            }*/
	        }

		}
    }

    /**
     * EventDispatcher maintains a queue, which stores events received from peer device.  The events are
     * pushed in by UIBCAdaptor.  When event is available in queue, an internal thread consumes the event
     * and injects the UI event through IWindowManager.
     * @author muhuanc
     *
     */
    class EventDispatcher extends EventQueue implements Runnable {
        public boolean running = true;
		@Override
		public void run() {
			while (running) {
	            InputEvent ev = getNextEvent();
	            if (ev != null) {
	                boolean injectSuccess = false;
	                if (ev instanceof MotionEvent) {
	                	/* inject MotionEvent */
	                    MotionEvent me = (MotionEvent) ev;
	                    try {
	                        if (me.getSource() == InputDevice.SOURCE_TOUCHSCREEN) {
	                            injectSuccess = inputManager.injectPointerEvent(me, false);
	                        } else if (me.getSource() == InputDevice.SOURCE_TRACKBALL) {
	                            injectSuccess = inputManager.injectTrackballEvent(me, false);
	                        } else {
	                            Log.e(TAG, "Unknown motion event");
	                        }
	                    } catch (RemoteException e) {
	                        Log.e(TAG, "Exception happened when injecting event", e);
	                    }
	                }else if(ev instanceof KeyEvent)
	                {
	                	/*inject key event*/
	                	KeyEvent ke = (KeyEvent)ev;
	                	Log.e(TAG, "Injecting key event" + ke.getKeyCode());
	                	try{
	                		injectSuccess = inputManager.injectKeyEvent(ke, false);
	                	}
	                	catch(RemoteException e)
	                	{
	                		Log.e(TAG, "Exception happened when injecting key event", e);
	                	}
	                }
	                if (!injectSuccess) {
	                    Log.e(TAG, "Inject failed for event type with contents: " + ev);
	                }
	            }
	        }
		}
    }

    /**
     * Non-blocking queue for event storage
     * @author muhuanc
     *
     */
    private class EventQueue {
        /* event queue */
        private BlockingQueue<InputEvent> queuedEvents = new LinkedBlockingQueue<InputEvent>();
        private final int timeOut =500;
	 protected boolean eventQueueStatus = true;
        public InputEvent getNextEvent() {
            while (eventQueueStatus) {
                InputEvent queuedEvent;
                try {
                    queuedEvent = queuedEvents.poll(timeOut, TimeUnit.MILLISECONDS);
                    if (queuedEvent != null) {
                        // dispatch the event
                        return queuedEvent;
                    } else {
                        return null;
                    }
                } catch (InterruptedException e) {
                    Log.e(TAG, "Interrupted when waiting to read from queue", e);
                    return null;
                }
            }
	    return null;
        }

        /*For EventDispatcher, this gets called from native code*/
        public void addEvent(InputEvent ev) {
            try {
                if(ev != null) {
                  queuedEvents.offer(ev, timeOut, TimeUnit.MILLISECONDS );
                }
            } catch (InterruptedException e) {
                Log.e(TAG, "Interrupted when waiting to insert to queue", e);
            } catch (IllegalArgumentException e) {
                Log.e(TAG, "Illegal Argument Exception ",e);
            }
        }
    }

    public static void HIDEventCallback(byte[] packet, int hidDataType) {
        if (packet == null) {
            Log.e(TAG, "HID payload is null, ignore callback");
        }
        if (hidDataType == HIDDataType.HID_REPORT.ordinal()) {
            HIDCListener.HIDReportRecvd(packet);
        } else if (hidDataType == HIDDataType.HID_REPORT_DESCRIPTOR.ordinal()) {
            HIDCListener.HIDReportDescRecvd(packet);
        }
    }

    public interface HIDCEventListener {

        void HIDReportDescRecvd(byte[] packet);

        void HIDReportRecvd(byte[] packet);
    }
}
