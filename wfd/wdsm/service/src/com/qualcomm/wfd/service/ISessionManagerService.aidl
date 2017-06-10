/* ==============================================================================
 * ISessionManagerService.aidl
 *
 * Copyright (c) 2012 - 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ============================================================================== */

package com.qualcomm.wfd.service;

import com.qualcomm.wfd.service.IWfdActionListener;
import com.qualcomm.wfd.WfdStatus;
import com.qualcomm.wfd.WfdDevice;
import android.os.Bundle;
import android.view.Surface;
import android.view.InputEvent;
import com.qualcomm.wfd.service.IHIDEventListener;

/**
 * Session Manager service handles all the Wifi Display specific operations
 * that are not related to P2P connection. Assumption is P2P connection is
 * already made successfully before exercising any API here.
 *
 * Following is the state diagram that represents what operations are allowed in
 * which state and transitions from one state to another.
 *
 *                                        teardown()
 *     |                         +-----------------------------+------------------------+
 *     | deinit()                |                             |                        |
 *     |                         |                             |                        |
 *    \|/                       \|/                            |                        |
 *+----+-----+   init()   +------+------+ startWfdSession() +--+---+   internal  +------+-------+
 *| INVALID  +----------->| INITIALIZED +------------------>| IDLE +------------>| ESTABLISHED  |
 *+----------+            +------+------+/_                 +------+   callback  +------+-------+
 *                              /|\      \ ``--..__                                     |
 *                               |                 ``--..__teardown()                   |
 *                               |                         ``--.._                      | play()
 *                               |                                ``--..__              |
 *                               |                                        ``--..__     \|/
 *                               |  teardown()      +---------+       pause()     ``----+----+
 *                               +------------------+  PAUSE  |<-------------------+  PLAY   |
 *                                                  +----+----+       standby()    +----+----+
 *                                                       |                             /|\
 *                                                       |                              |
 *                                                       +------------------------------+
 *                                                                     play()
 *
 */
interface ISessionManagerService {

	/**
	 * Set device type to source/sink etc. The default value should be read from
	 * a configuration file.
	 *
	 * NOTE: Currently only Source type is supported
	 * @param type
	 *            Type of device
	 * @return 0 on success or negative value on error.
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int setDeviceType(int type);

	/**
	 * Get the supported types by this device. {@link setDeviceType} can only
	 * accept one of the values returned by this.
	 *
	 * @param types This will contain type codes when call returns successfully
	 *              See WfdEnums.WFDDeviceType for different device type codes.
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 */
	int getSupportedTypes(out int[] types);

	/**
	 * Get current status of WFD session manager
	 *
	 * @return WfdStatus object with session id, state and connected device info
	 */
	WfdStatus getStatus();

	/**
	 * Initialize the service. The listener passed as paramater
	 * will receive state update notifications and event
	 * notifications callback. Send qualcomm.intent.action.WIFI_DISPLAY
	 * intent with state=1 to notify interested components.
	 *
	 * @param listener Listener that will be invoked when state changes
	 * @param thisDevice This device info with mac address and WFD info
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int init(IWfdActionListener listener, in WfdDevice thisDevice);

      /**
        * Initialize the service. The listener passed as paramater
        * will receive state update notifications and event
        * notifications callback. Send qualcomm.intent.action.WIFI_DISPLAY
        * intent with state=1 to notify interested components.
        *
        * This is meant to be called from system server context only and
        * will result in the client bearing the onus to create a Virtual
        * Display (normally system server will use WifiDisplayAdapter to
        * create the Virtual Display to be registered with DisplayManager).
        *
        * @param listener Listener that will be invoked when state changes
        * @param thisDevice This device info with mac address and WFD info
        *
        * @return 0 on success or negative value on error
        *         See WfdEnums.ErrorType for different error codes.
        * @throws RemoteException
        *
        * @hide
        */
        int initSys(IWfdActionListener listener, in WfdDevice thisDevice);

	/**
	 * Unregister the listener object that was registered with {@link #init(IWfdActionListener)}
	 *
	 * @param listener Listener to unregister
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int unregisterListener(IWfdActionListener listener);

	/**
	 * Deinit the service and send qualcomm.intent.action.WIFI_DISPLAY
	 * intent with state=1. Service will go to INVALID state.
	 *
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int deinit();


	/**
	 * Start WFD session with given peer device. This is a
	 * blocking call and it will wait until peer device has
	 * also initated the session and connected through RTSP.
	 *
	 * @param device WFD capable device
	 * @return session id or error (negative value)
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int startWfdSession(in WfdDevice device);

	/**
	 * Stop existing WFD session. Same as teardown().
	 *
	 * @param sessionId
	 *            WFD session id
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int stopWfdSession();

	/**
	 * Set resolution by providing format type (CEA/VESA/HH)
	 * and one of the predefined format value in {@link WfdEnums}
	 *
	 * @param formatType One of WFD_CEA_RESOLUTIONS_BITMAP,
	 *        WFD_VESA_RESOLUTIONS_BITMAP or WFD_HH_RESOLUTIONS_BITMAP
	 * @param value Only one of constants with names H264_CEA,
	 *        H264_VESA and H264_HH in {@link WfdEnums}
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int setResolution(int formatType, int value);

	/**
	 * Set bitrate in MBPS
	 *
	 * @param bitrate Bitrate in MBPS
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int setBitrate(int bitrate);

	/**
	 * Set RTP packet transport mechanism and related parameters.
	 *
	 * @param transportType TCP/UDP
	 * @param bufferLengthMs Buffer length in milliseconds
	 * @param port UDP server port / TCP client port
	 * @throws RemoteException
	 */
	int setRtpTransport(int transportType, int bufferLengthMs, int port);

	/**
	 * RTP over TCP playback control
	 *
	 * @param cmdType Control command from WfdEnums.ControlCmdType
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int tcpPlaybackControl(int cmdType, int cmdVal);

	/**
	 * Sets decoder latency through RTSP to sink device
	 *
	 * @param latency value of the buffering latency for decoder in Sink
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int setDecoderLatency(int latency);

	/**
	 * Queries whether Sink device supports TCP for RTP payload
	 *
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int  queryTCPTransportSupport();

	/**
	 * Set AV playback mode
	 *
	 * @param mode Playback mode from WfdEnums.AVPlaybackMode
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int setAvPlaybackMode(int mode);

	/**
	 * RTSP Session Control API
	 */

	/**
	 * RTSP PLAY
	 */
	int play();

	/**
	 * RTSP PAUSE
	 */
	int pause();

	/**
	 * RTSP TEARDOWN. This is blocking call with a timeout
	 * of 5 secs. It waits for sink to send TEARDOWN request.
	 */
	int teardown();

	/**
	 * RTSP standby. Same as pause()
	 */
	int standby();

	/**
	 * Start UIBC session
	 *
	 * @return UIBC session ID or error (negative value)
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int startUibcSession();

	/**
	 * Stop UIBC session
	 *
	 * @return 0 on success or negative value on error
	 *         See WfdEnums.ErrorType for different error codes.
	 * @throws RemoteException
	 */
	int stopUibcSession();

	/**
	 * NOTE: Currently Unsupported
	 * Retrieve common capabilities between paired source and
	 * sink capabilities.
	 *
	 * @param capabilities Map of common capabilities will be filled by the service
	 */
	int getCommonCapabilities(out Bundle capabilities);

	/**
	 * NOTE: Currently Unsupported
	 * Set negotiated capabilities after retrieving common
	 * capabilities.
	 */
	int setNegotiatedCapabilities(in Bundle capabilities);

        /**
         * This will supply the configurable items to the client
         *
         * @throws RemoteException
         *
         * @hide
         */
        int getConfigItems(out Bundle configItems);

	/**
         * Send RTSP M14 message for setting UIBC parameters
         * [or changing UIBC capabilities after UIBC session establishment]
         *
         * @throws RemoteException
         */
        int setUIBC();

        /**
         * Queries UIBC status to check whether UIBC has got enabled/disabled
         * at Session Manager Level
         */
        boolean getUIBCStatus();

        /**
     * Supplies client with the bitmap of the common resolutions between
     * source and sink in an array. The format of the data supplied in Bundle:
     *
     * The bundle key to retrieve number of profiles (an Integer)is "numProf"
     * the key to retrieve the integer array is "comRes" and the integer
     * array that is retrieved using this key is formatted as follows:
     *
     * [i] the H264 profile
     * [i+1] the CEA resolution bitmap of the
     * [i+2] the VESA resolution bitmap
     * [i+3] the HH resolution bitmap
     *
     * where i is 0,4,...,(numProf-1)*4
     *
     * @return 0 on success else an error code as in WfdEnums.ErrorType,
     *         on error the bundle will not have any data
     *
     * @throws RemoteException
     */
     int getCommonResolution(out Bundle comRes);

    /**
     * Supplies client with the bitmap of the negotiated resolution between
     * source and sink in an array. The format of the data supplied in Bundle:
     *
     * The bundle key to retrieve the integer array is "negRes"and the integer
     * array that is retrieved using this key is formatted as follows:
     *
     * [0] the H264 profile
     * [1] the CEA resolution bitmap
     * [2] the VESA resolution bitmap
     * [3] the HH resolution bitmap
     *
     * @return 0 on success else an error code ,on error the bundle will not
     *         have any data
     *
     * @throws RemoteException
     */
     int getNegotiatedResolution(out Bundle negRes);

     /**
      * Enable Dynamic Bitrate Adaptation at runtime
      *
      * @param flag true to enable/false to disable bitrate adaptation
      *
      * @return 0 on success else an error code as in WfdEnums.ErrorType
      *
      * @throws RemoteException
      */
      int enableDynamicBitrateAdaptation(boolean flag);

      /**
       * Register the HID event listener to receive
       * callbacks when HID events are received.
       */
      int registerHIDEventListener(IHIDEventListener listener);

      /**
       * Unregister the HID event listener previously
       * registered for HID event callbacks.
       */
      int unregisterHIDEventListener(IHIDEventListener listener);

        /**
	 * Sink Specific functions (only applicable on SINK device type)
	 */

	/**
	 * Set the surface where multimedia content is rendered. This
	 * does not have any effect when device type is SOURCE.
	 * NOTE: Make sure the surface is valid before calling this function.
	 *
	 * @param mmSurface If the device type is Sink, then this surface is where multimedia
	 *                  video content should be delivered. Otherwise provide null.
	 * @return 0 on success or negative value on error
	 * @throws RemoteException
	 */
	int setSurface(in Surface mmSurface);

	/**
	 * Send UIBC event in the form of Android native event
	 * from Sink to Source.
	 *
	 * @param event KeyEvent/MotionEvent object
	 * @return 0 on success or negative value on error
	 * @throws RemoteException
	 */
	int sendEvent(in InputEvent event);
	/**
	 * Set surface properties like height, width and orientation
	 * does not have any effect when device type is SOURCE.
	 *
	 * @param int width
	 * @param int height
	 * @param int Orientation range from 0 to 359
	 * @return 0 on success or negative value on error
	 */
	int setSurfaceProp(int width, int height, int orientation);

        /**
         * Send UIBC rotation event
         *
         * @param int angle can only be 0,90,180 or 270
         *
         * @return 0 on success or negative value on error as
         *           defined in WfdEnums.ErrorType
         *
         * @throws RemoteException
         */
         int uibcRotate(int angle);


}
