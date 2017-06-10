/*==============================================================================
*        @file WFDNative.java
*
*
*  Copyright (c) 2012 -2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qualcomm.wfd;


import com.qualcomm.wfd.WfdEnums.SessionState;
import com.qualcomm.wfd.WfdEnums.WfdEvent;

import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;

public class WFDNative {

	private static final String TAG = "WFDNative";

	static {
		Log.d(TAG, "try to load libwfdnative.so");
		System.loadLibrary("wfdnative");
		Log.d(TAG, "libwfdnative.so loaded.");
		eventCallback(null, null); //So that JB packages this method in APK
	}

	public static WfdActionListener listener;


	/**
	 *  Supported native functions
	 */
	public static native boolean enableWfd(WfdDevice thisDevice);
	public static native boolean disableWfd();
	public static native void startWfdSession(WfdDevice device);
	public static native void stopWfdSession(int sessionId);
	public static native void play(int sessionId, boolean secureFlag);
	public static native void pause(int sessionId, boolean secureFlag);
	public static native void teardown(int sessionId, boolean isTriggered);

        public static native boolean standby(int sessionId);
	//public static native void resume(int sessionId);

    public static native boolean enableUIBC(int sessionId);
    public static native boolean disableUIBC(int sessionId);
	public static native void startUIBC(Object obj);
	public static native void stopUIBC();

	public static native void setVideoSurface(Surface surface);
	public static native void sendUIBCKeyEvent(KeyEvent ev);
	public static native void sendUIBCMotionEvent(MotionEvent ev);
        public static native boolean sendUIBCRotateEvent(int angle);
        public static native boolean setResolution(int formatType, int value,
            int[] resParams);
	public static native void setBitrate(int bitrate);
	public static native void queryTCPTransportSupport();
	public static native void negotiateRtpTransport(int transportType, int bufferLengthMs, int port);
	public static native void setRtpTransport(int transportType);
	public static native void tcpPlaybackControl(int cmdType, int cmdVal);
    public static native void setDecoderLatency(int latency);
    public static native void flush(int sessionId);

	public static native void setSurfaceProp(int width,int height, int orientation);
        public static native boolean setAvPlaybackMode(int mode);

        public static native void getConfigItems(int[] configItems);
    public static native boolean setUIBC(int sessionId);

    public static native boolean executeRuntimeCommand(int CmdType);
    public static native int[] getCommonRes(int[] result);
    public static native int[] getNegotiatedRes(int[] result);

	/**
	 * Static fields for native thread access
	 */
	private static String localMacAddress = "00:00:00:00:00:00";
	private static String localIpAddress = "";

	/**
	 *  Callback functions from native thread
	 */

	public static String getLocalIpAddress() {
		if (localIpAddress.length() == 0) {
			localIpAddress = "0.0.0.0"; //FIXME
		}
		return localIpAddress;
	}
	
	public static int getLinkSpeed() {
		//return WLANAdaptor.getLinkSpeed();
		return 0; //FIXME
	}

	public static String getIpAddress(String macAddr) {
		String peerIP = "0.0.0.0"; //FIXME WLANAdaptor.getPeerIP(macAddr);
		int devIndex = 0 ;//FIXME WFDService.getDiscoveredDeviceIndex(macAddr);

		Log.d(TAG, "getIpAddress() called.");
		Log.d(TAG, "peerIP = "+peerIP);
		if (devIndex >=0)
			Log.d(TAG, "ip info in deviceList = "); //WFDService.discoveredDeviceList.elementAt(devIndex).ip_addr);

		if (peerIP.length() > 0 && devIndex >= 0) {
			//update ip_addr in discoveredDeviceList
			//FIXME WFDService.discoveredDeviceList.elementAt(devIndex).ip_addr = peerIP;
		}

		if (peerIP.length() > 0) {
			return peerIP;
		} else if (devIndex >=0) {
			//FIXME return WFDService.discoveredDeviceList.elementAt(devIndex).ip_addr;
			return peerIP;
		}
		return "";
	}

	public static long getWfdDeviceInfoBitmap(String macAddr) {
		//Device peerDev = WFDService.discoveredDeviceList.elementAt(WFDService.getDiscoveredDeviceIndex(macAddr));
		//return WLANAdaptor.getWFDDeviceInfoBitmap(peerDev);
		return 0; //FIXME
	}

	public static int getWfdDeviceRtspPort(String macAddr) {
		//Device peerDev = WFDService.discoveredDeviceList.elementAt(WFDService.getDiscoveredDeviceIndex(macAddr));
		//return peerDev.rtsp_port;
		return 854; //FIXME
	}

	public static int getWfdDeviceMaxThroughput(String macAddr) {
		//Device peerDev = WFDService.discoveredDeviceList.elementAt(WFDService.getDiscoveredDeviceIndex(macAddr));
		//return peerDev.max_throughput;
		return 0; //FIXME
	}

	public static int getWfdDeviceCoupleSinkStatusBitmap(String macAddr) {
		//Device peerDev = WFDService.discoveredDeviceList.elementAt(WFDService.getDiscoveredDeviceIndex(macAddr));
		//return peerDev.wfd_coupled_sink_status.getCode();
		return 0; //FIXME
	}


	public static void eventCallback(String eventName, Object[] objectArray) {
		Log.d(TAG, "eventCallback triggered");
		if (eventName == null || objectArray == null) {
		    Log.d(TAG, "No event info, ignore.");
		    return;
		}
		int array_length = objectArray.length;
		Log.d(TAG, "CallbackEvent \"" + eventName + "\" --- objectArray length=" + array_length);
		for (int i=0;i<array_length;i++) {
            if (objectArray[i] != null) {
                Log.d(TAG,
                        "\tobjectArray[" + i + "] = "
                                + objectArray[i].toString());
            }
		}
		if (listener == null) {
		    Log.d(TAG, "Listener is destroyed, can't notify");
		    return;
		}
		if ("Error".equalsIgnoreCase(eventName)) {
		    if (objectArray.length > 0) {
		        if ("RTSPCloseCallback".equalsIgnoreCase((String)objectArray[0])) {
		            Log.d(TAG, "RTSP close callback, treat as TEARDOWN start");
		            listener.notifyEvent(WfdEvent.TEARDOWN_START, -1);
		        }
                        if ("StartSessionFail".equalsIgnoreCase((String)objectArray[0])) {
                            Log.e(TAG, "Start of WFD Session Failed");
                            listener.notifyEvent(WfdEvent.START_SESSION_FAIL, -1);
                        }
                }

		} else if ("ServiceStateChanged".equalsIgnoreCase(eventName)) {
		    boolean enabled = "enabled".equalsIgnoreCase((String)objectArray[0]);
		    if (enabled) {
		        listener.notifyEvent(WfdEvent.WFD_SERVICE_ENABLED, 0);
		        Log.d(TAG, eventName + " WFD_SERVICE_ENABLED");
		    } else {
		        listener.notifyEvent(WfdEvent.WFD_SERVICE_DISABLED, 0);
		        Log.d(TAG, eventName + " WFD_SERVICE_DISABLED");
		    }

		} else if ("SessionStateChanged".equalsIgnoreCase(eventName)) {
		    //if (objectArray.length == 3 && "ESTABLISHED".compareToIgnoreCase((String)objectArray[0]) == 0) {
		    if (objectArray.length == 3) {
		        String state = (String)objectArray[0];
		        int sessionId = Integer.parseInt((String)objectArray[2]);
		        if ("STANDBY".equalsIgnoreCase(state)) {
		            listener.notifyEvent(WfdEvent.STANDBY_START, sessionId);
                        } else if ("ESTABLISHED".equalsIgnoreCase(state)) {
                            listener.updateState(SessionState.ESTABLISHED, sessionId);
		        } else if ("STOPPED".equalsIgnoreCase(state)){
		            listener.updateState(SessionState.TEARDOWN, sessionId);
		        } else {
		            Log.d(TAG, "No Session state change is required for native state " + state);
		        }
		        Log.d(TAG, "Event: " + eventName + " State: " + (String)objectArray[2]);
		    }
		    //WFDService.wfdServiceState.obtainMessage(WFDState.WFD_SESSION_STATE_CHANGED, objectArray).sendToTarget();

		} else if ("StreamControlCompleted".equalsIgnoreCase(eventName)) {
		    Log.d(TAG, eventName);
		    if (objectArray.length >= 2) {
		        String state = (String)objectArray[1];
		        int sessionId = Integer.parseInt((String)objectArray[0]);
		        if ("PLAY".equalsIgnoreCase(state)) {
		            listener.notifyEvent(WfdEvent.PLAY_START, sessionId);
		        } else if ("PLAY_DONE".equalsIgnoreCase(state)) {
		            listener.updateState(SessionState.PLAY, sessionId);
		        } else if ("PAUSE".equalsIgnoreCase(state)) {
		            listener.notifyEvent(WfdEvent.PAUSE_START, sessionId);
		        } else if ("PAUSE_DONE".equalsIgnoreCase(state)) {
		            listener.updateState(SessionState.PAUSE, sessionId);
		        } else if ("TEARDOWN".equalsIgnoreCase(state)) {
		            listener.notifyEvent(WfdEvent.TEARDOWN_START, sessionId);
		        }
		    }
		} else if ("UIBCControlCompleted".equalsIgnoreCase(eventName)) {
		    if (objectArray.length >= 2) {
		        int sessionId = Integer.parseInt((String)objectArray[0]);
		        String state = (String) objectArray[1];
		        if ("ENABLED".equalsIgnoreCase(state)) {
		            listener.notifyEvent(WfdEvent.UIBC_ENABLED, sessionId);
		            Log.d(TAG, eventName + " ENABLED");
		        } else {
		            listener.notifyEvent(WfdEvent.UIBC_DISABLED, sessionId);
		            Log.d(TAG, eventName + " DISABLED");
		        }
		    }
        } else if ("UIBCRotateEvent".equalsIgnoreCase(eventName)) {
            if (objectArray.length >= 1) {
                int angle = -1;
                try {
                    angle = Integer.parseInt((String) objectArray[0]);
                } catch (NumberFormatException e) {
                    Log.e(TAG, "Number Format Exceptionwhile parsing value");
                    e.printStackTrace();
                }
                Bundle b = new Bundle(2);
                b.putString("event", eventName);
                b.putInt("rot_angle", angle);
                listener.notify(b, -1);
                Log.e(TAG, eventName + "Angle = " + angle);
            }
		} else if ("MMEvent".equalsIgnoreCase(eventName)) {
		    if (objectArray.length >= 2) {
		        String var = (String) objectArray[0];
		        String value = (String) objectArray[1];
		        if ("HDCP_CONNECT".equalsIgnoreCase(var)) {
		            if ("SUCCESS".equalsIgnoreCase(value)) {
		                listener.notifyEvent(WfdEvent.HDCP_CONNECT_SUCCESS, -1);
			    } else if ("UNSUPPORTEDBYPEER".equalsIgnoreCase(value)) {
		                listener.notifyEvent(WfdEvent.HDCP_ENFORCE_FAIL, -1);
		            } else {
		                listener.notifyEvent(WfdEvent.HDCP_CONNECT_FAIL, -1);
		            }
                } else if("MMStreamStarted".equalsIgnoreCase(var)) {
                    int width = Integer.parseInt((String) objectArray[1]);
                    int height = Integer.parseInt((String) objectArray[2]);
                    int hdcp = 0;
                    if (objectArray.length > 3) {
                        hdcp = Integer.parseInt((String) objectArray[3]);
                    }
                    Surface surface = null;
                    if (array_length > 4) {
                        // Surface argument is there in the callback as well
                        try {
                            if (objectArray[4] != null) {
                                Log.e(TAG, objectArray[4].getClass().getName());
                                surface = (Surface) objectArray[4];
                            }
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                        // Sanity check
                        if (surface != null) {
                            if (surface.isValid()) {
                                Log.d(TAG, "Received a valid surface");
                            }
                        }
                    }
                    Log.d(TAG, "MMStreamStarted " + width + " " + height + " "
                            + hdcp);
                    Bundle b = new Bundle();
                    b.putString("event", var);
                    b.putInt("width", width);
                    b.putInt("height", height);
                    b.putInt("hdcp", hdcp);
                    b.putParcelable("surface", surface);
                    listener.notify(b, -1);
                }
		    }
		} else if ("VideoEvent".equalsIgnoreCase(eventName)) {
		    if (objectArray.length >= 1) {
		        String desc = (String) objectArray[0];
		        if ("RuntimeError".equalsIgnoreCase(desc)) {
		            listener.notifyEvent(WfdEvent.VIDEO_RUNTIME_ERROR, -1);
		        } else if ("ConfigureFailure".equalsIgnoreCase(desc)) {
		            listener.notifyEvent(WfdEvent.VIDEO_CONFIGURE_FAILURE, -1);
		        } else {
		            Log.d(TAG, "Unknown description:" + desc);
		        }
		    } else {
		        Log.e(TAG, "No description for VideoEvent");
		    }
		} else if ("AudioEvent".equalsIgnoreCase(eventName)) {
		    if (objectArray.length >= 1) {
		        String desc = (String) objectArray[0];
		        if ("RuntimeError".equalsIgnoreCase(desc)) {
                            listener.notifyEvent(WfdEvent.AUDIO_RUNTIME_ERROR, -1);
                        } else if ("ConfigureFailure".equalsIgnoreCase(desc)) {
                            listener.notifyEvent(WfdEvent.AUDIO_CONFIGURE_FAILURE, -1);
                        } else if ("AudioProxyOpened".equalsIgnoreCase(desc)) {
                            listener.notifyEvent(WfdEvent.AUDIOPROXY_OPENED, -1);
                        } else if ("AudioProxyClosed".equalsIgnoreCase(desc)) {
                            listener.notifyEvent(WfdEvent.AUDIOPROXY_CLOSED, -1);
                        } else if ("AudioOnlySession".equalsIgnoreCase(desc)) {
                            listener.notifyEvent(WfdEvent.AUDIO_ONLY_SESSION, -1);
                        } else {
                            Log.e(TAG, "Unknown description:" + desc);
                        }
		    } else {
		        Log.e(TAG, "No description for AudioEvent");
		    }
		}else if ("HdcpEvent".equalsIgnoreCase(eventName)) {
		    if (objectArray.length >= 1) {
		        String desc = (String) objectArray[0];
		        if ("RuntimeError".equalsIgnoreCase(desc)) {
                    listener.notifyEvent(WfdEvent.HDCP_RUNTIME_ERROR, -1);
                } else {
                    Log.e(TAG, "Unknown description:" + desc);
                }
		    } else {
		        Log.e(TAG, "No description for AudioEvent");
		    }
		}
		else if ("NetworkEvent".equalsIgnoreCase(eventName)) {
                    if (objectArray.length >= 1) {
                        String desc = (String) objectArray[0];
                        if ("RuntimeError".equalsIgnoreCase(desc)) {
                            listener.notifyEvent(WfdEvent.NETWORK_RUNTIME_ERROR, -1);
                        } else if ("ConfigureFailure".equalsIgnoreCase(desc)) {
                            listener.notifyEvent(WfdEvent.NETWORK_CONFIGURE_FAILURE, -1);
                        } else if ("RtpTransportNegotiationSuccess".equalsIgnoreCase(desc)) {
                            listener.notifyEvent(WfdEvent.RTP_TRANSPORT_NEGOTIATED, -1);
                            Bundle b = new Bundle(4);
                            b.putString("event", desc);
                            b.putString("status", (String)"0");
                            if(objectArray.length > 1) {
                                b.putString("prevMode", (String) objectArray[1]);
                                b.putString("newMode", (String) objectArray[2]);
                            }
                            listener.notify(b, -1);
                        } else if ("BufferingUpdate".equalsIgnoreCase(desc) && objectArray.length >= 3) {
                            Bundle b = new Bundle(3);
                            b.putString("event", desc);
                            b.putString("bufferLength", (String) objectArray[1]);
                            b.putString("windowSize", (String) objectArray[2]);
                            listener.notify(b, -1);
                        } else if ("setDecoderLatency".equalsIgnoreCase(desc)) {
                            Bundle b = new Bundle(3);
                            b.putString("event", desc);
                            b.putString("status", (String) objectArray[1]);
                            listener.notify(b, -1);
                        }else if ("TCPTransportSupport".equalsIgnoreCase(desc)) {
                            Bundle b = new Bundle(3);
                            b.putString("event", desc);
                            b.putString("status", (String) objectArray[1]);
                            listener.notify(b, -1);
                        }else if ("TCPPlaybackControl".equalsIgnoreCase(desc)) {
                            Bundle b = new Bundle(3);
                            b.putString("event", desc);
                            b.putString("cmd", (String) objectArray[1]);
                            b.putString("status", (String) objectArray[2]);
                            if (objectArray.length >= 5) {
                                b.putString("bufferLength", (String) objectArray[3]);
                                b.putString("windowSize", (String) objectArray[4]);
                            }
                            listener.notify(b, -1);
			    listener.notifyEvent(WfdEvent.TCP_PLAYBACK_CONTROL, -1);
               } else if ("RTCPRRMessage".equalsIgnoreCase(desc)) {
                    // The RTCPRR callback should have a minimum of 3 objects in
                    // the array [0] will have the description name [1] will
                    // have the length of the message and following that there
                    // will be the RTCP message itself. Hence don't accept
                    // callback without a minimum length of 3
                    if (array_length >= 3) {
                        Bundle b = new Bundle(3);
                        b.putString("event", desc);
                        try {
                            b.putInt("length",
                                    Integer.parseInt((String) objectArray[1]));
                            StringBuilder sb = new StringBuilder();
                            for (int i = 2; i < array_length; i++) {
                                sb.append((String) objectArray[i]);
                            }
                            b.putString("mesg", sb.toString());
                            listener.notify(b, -1);
                        } catch (Exception e) {
                            Log.e(TAG, "Exception while parsing RTCP message"
                                    + e);
                        }
                    } else {
                        Log.e(TAG, "Too few params in RTCPRR callback");
                    }
                }

            } else {
                Log.e(TAG, "No description for NetworkEvent");
            }
        } else {
		    Log.e(TAG, "Receive unrecognized event from WFDNative.cpp: "+eventName);
		}
	}


	/*
	public static boolean setTxPower(int txPower) {
		return true;
	}

	public static boolean setWakeLock(boolean lockEnabled) {
		return true;
	}
	*/
	public interface WfdActionListener {
	    void updateState(SessionState state, int sessionId);

	    void notifyEvent(WfdEvent event, int sessionId);

	    void notify(Bundle b, int sessionId);
	}

}
