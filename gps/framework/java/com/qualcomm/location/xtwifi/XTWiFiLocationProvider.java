/*
 * ====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *
 * XTRA-T WiFi Replacement Network Location Provider
 *
 * GENERAL DESCRIPTION This is implementation of Location Provider interface and
 * can be loaded by Location Manager Service if so configured.
 *
 * Copyright (c) 2012-2014 Qualcomm Atheros, Inc. All Rights Reserved. Qualcomm
 * Atheros Confidential and Proprietary.
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * =============================================================================
 */
package com.qualcomm.location.xtwifi;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.NoSuchElementException;
import java.util.Properties;

import com.qualcomm.lib.location.mq_client.*;
import com.qualcomm.lib.location.log.*;

import android.app.Service;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.location.Criteria;
import android.location.ILocationManager;
import com.android.internal.location.ILocationProvider;
import com.android.internal.location.ProviderProperties;
import com.android.internal.location.ProviderRequest;
import android.location.Location;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.location.LocationListener;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.WorkSource;
import android.util.Log;
import android.util.SparseIntArray;

public class XTWiFiLocationProvider extends Service
{
  private static final String TAG = "XTWiFiLP";
  public static final String CONFIG_FILE = "/system/etc/xtwifi.conf";
  public static final String IZAT_CONFIG_FILE = "/etc/izat.conf";
  public static final String MQ_SOCKET = "/data/misc/location/mq/location-mq-s";

  private final LocLog llog = new LocLog(LocLog.LOG_VERBOSE_LEVEL_VERBOSE);

  // Bundle extra location parameters for XTWWAN
  private static final String BUNDLE_VERTICAL_UNCERTAINITY = "BUNDLE_VERTICAL_UNCERTAINITY";
  private static final String BUNDLE_ALTITUDE_WRT_ELLIPSOID = "BUNDLE_ALTITUDE_WRT_ELLIPSOID";

  private static final String NLP_ENABLED_CHANGE_ACTION = "android.location.NLP_ENABLED_CHANGE";
  private static final String NLP_FIX_CHANGE_ACTION = "android.location.NLP_FIX_CHANGE";
  private static final String EXTRA_NLP_ENABLED = "enabled";

  private static final int MSG_ENABLE = 1;
  private static final int MSG_ENABLE_TRACKING = 2;
  //private static final int MSG_UPDATE_NETWORK_STATE = 3;
  //private static final int MSG_UPDATE_LOCATION = 4;
  //private static final int MSG_ADD_LISTENER = 5;
  //private static final int MSG_REMOVE_LISTENER = 6;
  private static final int MSG_SET_MIN_TIME = 7;
  private static final int MSG_ZPP_FINISHED = 8;
  private static final int MSG_WIFI_SCAN_RESULT_AVAILABLE = 9;

  private static final int MSG_TRY_CONNECT         = 100;
  private static final int MSG_NEW_IPC_MESSAGE     = 101;
  private static final int MSG_IPC_EXCEPTION       = 102;
  private static final int MSG_LOOPER_EXIT_REQUEST = 103;

  //must match the same values in NlpProxyProvider.java
  private static final int NLP_MODE_COMBO = 3;

  public static final int CONNECT_RETRY_LIMIT = 20;
  public static final int INVALID_TX_ID = -1;
  public static final int MIN_VALID_TX_ID = 1;
  public static final int MAX_VALID_TX_ID = 1000000;
  public static final int MIN_VALID_TX_ID_ON_FREE_WIFI_SCAN = 1000001;
  public static final int MAX_VALID_TX_ID_ON_FREE_WIFI_SCAN = 2000000;

  private int m_log_level = LocLog.LOG_VERBOSE_LEVEL_WARNING;
  private Handler m_handler = null;
  private boolean m_isOnDestroyCalled = false;
  private Object m_lock = new Object();
  private Thread m_handlerThread;
  private ILocationManager m_LocationManager;

  private AlarmManager m_alarmManager;
  private PendingIntent m_PendingIntentTBFTimeout;

  private PowerManager m_powerManager;
  private PowerManager.WakeLock m_wakeLock;

  private boolean m_isFixSessionGoingOn = false;

  private int m_connectRetryCounter = 0;
  private MessageQueueClient m_ipcClient = null;

  // true: enable ZPP. Try to load ZPP adaptor and trigger ZPP fixes
  // false: disable ZPP. Do not try to load ZPP adaptor.
  private boolean m_isZppEnabled = true;

  private boolean m_isXTWWANEnabled = false;
  private boolean m_isXTWIFIEnabled = true;
  private ZppAdaptor m_zppAdaptor;

  private long m_currentTimeBetweenFix = 3000;

  // Minimum TBF, Time Between Fixes for NLP
  private long m_minTimeBetweenFix = 2000;

  // Maximum age for ZPP transaction (request-response) for the ZPP fix report to be used
  // NLP would not try to trigger a new ZPP request if the previous ZPP transaction is still within this age limit
  // set it to zero so we'd query ZPP for each and every XTWiFi fix request
  private long m_maxAgeForZppTransaction = 0;

  // private long m_maxAgeForZppFix = 5000;

  // Timeout for ZPP fix when triggered by NLP
  private long m_timeoutZppFix = 1800;

  // Timeout for XT WWAN fix when triggered by NLP
  private long m_timeoutWWanFix = 8000;

  // Timeout for XTWiFi Final Fix when triggered by NLP
  // This value need to be larger than: TIMEOUT_SEC_SERVER_POSITION_REQUEST_OVER_LOWSPEED_CONN
  private long m_timeoutFinalFix = 35000;

  // Timeout for XTWiFi Final Fix when triggered by free WiFi scan
  private long m_timeoutFreeXtWiFiFix = 3000;

  // Fix on free WiFi scan result
  private boolean m_isEnabledProcessFreeWiFiScan = false;
  private long m_minTimeBetweenFixOnFreeWiFiScan = 20000;
  private long m_timestamp_last_fix_request_on_free_wifi_scan = 0;
  private boolean m_isFreeRideListenerInstalled = false;
  private long m_timestamp_last_wifi_scan_result = 0;
  private int m_current_xtwifi_on_free_scan_req_tx_id = INVALID_TX_ID;
  private XtwifiFixState m_xtwifi_on_free_scan_tx_state;
  private long m_minTimeBetweenWiFiPositionRequestAndTrackingTimeout = 2000;
  private boolean m_isGTPRequestPendingOnFreeWiFiScanPositioning = false;
  private boolean m_isTracking = false;

  // Inject sources as defined in qmiLocInjectPositionReqMsgT_v02
  // we need to inject wifi position on free wifi scan
  private static final int INJECT_SRC_WIFI = 3;

  // Assuming an increase in PUNC by 56 miles per hour
  private static final float PUNC_INCR_METERS_PER_SECOND = 25;
  // PUNC scaling from from 99% confidence to 68% confidence is (1.07/2.14) = 0.5
  // Used for ZPP PUNC growth and consistency check
  private static final float SCALE_FACTOR_PUNC_CONF_99_TO_68 = (float) 0.5;

  private float m_hepe_margin_meter_report_zpp_location = 10;

  //private boolean m_testIgnoreTimestampInZppFix = true;

  private enum InternalState
  {
    IS_DISABLED,
    IS_IDLE,
    IS_TRACKING_WAIT_FOR_TBF,
    IS_TRACKING_WAIT_FOR_ZPP_FIX,
    IS_TRACKING_WAIT_FOR_FINAL_FIX,
    IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX
  };

  private enum XtwifiFixState
  {
    XFS_IDLE_OR_WAITING,
    XFS_PRELIMINARY,
    XFS_FINAL
  }

  // global state - state of the whole NLP
  private InternalState m_internalState = InternalState.IS_IDLE;
  // global state - not reset across sessions
  // It will be incremented first before every usage
  private int m_next_gtp_req_tx_id = MIN_VALID_TX_ID - 1;
  private long m_timestamp_last_fix_request_in_current_tracking_session = -1;

  // session status for GTP transactions
  private int m_current_gtp_req_tx_id = INVALID_TX_ID;
  private XtwifiFixState m_xtwifi_tx_state;
  private boolean m_is_xtwifi_fix_with_lat_lon = false;
  private boolean m_is_xtwwan_fix_with_lat_lon = false;
  private boolean m_got_xtwwan_response = false;
  private boolean m_has_reported_anything_in_current_fix_session = false;
  private Location m_lastKnownXtwifiFix = new Location(LocationManager.NETWORK_PROVIDER);
  private int m_lastKnownXtwifiFix_mac_array [] = null;
  private int m_lastKnownXtwifiFix_rssi_array [] = null;
  private int m_lastKnownXtwifiFix_channel_array [] = null;

  // global threshold HEPE value for comparison before allowing fix to report - 25km
  private static long m_large_accuracy_threshold_to_filter_nlp_position = 25000;

  // Added for 1AP case scenario
  // m_num_AP_cache - its used for checking num of cache hit for AP
  private int m_num_AP_cache = 0;

  // session status for ZPP adaptor transaction
  private Location m_lastKnownZppFix = new Location(LocationManager.NETWORK_PROVIDER);
  private long m_timestampLastZppReport = -1;
  private Location m_lastKnownWwanFix = new Location(LocationManager.NETWORK_PROVIDER);

  // stats collection related
  private long m_timestamp_stats_collection_start = System.currentTimeMillis();
  private int m_time_sec_min_age_reporting_stats_collected = 3600;
  private int m_stats_wifi_fix_inconsistent_with_zpp = 0;
  private int m_stats_fix_dropped_too_large_radius = 0;
  private int m_stats_wifi_fix_dropped_calculated_with_single_AP = 0;

  private NlpPublicStatus m_publicStatus = new NlpPublicStatus();

  @Override
  public IBinder onBind(Intent intent)
  {
    llog.w (TAG, "onBind");
    return m_stubBinder;
  }

  public void onCreate()
  {
    llog.w(TAG, "onCreate enter");
    registerForListenersAndMiscSetup ();
    llog.w(TAG, "onCreate exit");
  }

  public void onDestroy()
  {
    llog.w(TAG, "onDestroy enter");

    try
    {
      // exception is caught at this function
      unregisterForListenersAndMiscCleanup ();

      boolean isHandlerNull = false;
      synchronized (m_lock)
      {
        m_isOnDestroyCalled = true;
        if (m_handler == null)
        {
          isHandlerNull = true;
        }
      }
      llog.w(TAG, "onDestroy enter: isHandlerNull = "+ isHandlerNull);
      if (isHandlerNull == false)
      {
        // send message to request handler thread to exit
        m_handler.removeCallbacksAndMessages(null);
        m_handler.sendEmptyMessage(MSG_LOOPER_EXIT_REQUEST);
      }
    }
    catch (Exception e)
    {
      llog.w (TAG, "onDestroy: "+ e.getMessage ());
    }

    llog.w(TAG, "onDestroy exit");
  }

  private final ILocationProvider.Stub m_stubBinder = new ILocationProvider.Stub()
  {
    private final ProviderProperties m_properties =
      new ProviderProperties(true,false,false,false,false,false,false,
                             Criteria.POWER_LOW,Criteria.ACCURACY_COARSE);

    public void enable()
    {

      llog.w(TAG, "enable");
      // Catch any exception: eg: m_handler exists and the binder method is still invoked
      try
      {
        if (m_publicStatus.getStatus() != LocationProvider.OUT_OF_SERVICE)
        {
          llog.w(TAG, "Request to enable on " + this.toString());
          Message nlpMsg = Message.obtain(m_handler, MSG_ENABLE);
          nlpMsg.arg1 = 1;
          m_handler.sendMessage(nlpMsg);
        }
      }
      catch (Exception e)
      {
        e.printStackTrace();
      }
    }

    public void disable()
    {
      // Catch any exception: eg: m_handler exists and the binder method is still invoked
      try
      {
        if (m_publicStatus.getStatus() != LocationProvider.OUT_OF_SERVICE)
        {
          llog.w(TAG, "Request to disable on " + this.toString());
          Message nlpMsg = Message.obtain(m_handler, MSG_ENABLE);
          nlpMsg.arg1 = 0;
          m_handler.sendMessage(nlpMsg);
        }
      }
      catch (Exception e)
      {
        e.printStackTrace();
      }
    }

    public void setRequest(ProviderRequest request, WorkSource ws)
    {
      // Catch any exception: eg: m_handler exists and the binder method is still invoked
      try
      {
        if (m_publicStatus.getStatus() != LocationProvider.OUT_OF_SERVICE)
        {
          llog.w(TAG, "setRequest interval : " + request.interval +
                      "reportLocation: " + request.reportLocation + ", " +
                      this.toString());
          Message msgSetTime = Message.obtain(m_handler, MSG_SET_MIN_TIME);
          msgSetTime.obj = new Long(request.interval);
          m_handler.sendMessage(msgSetTime);

          Message msgEnableTracking = Message.obtain(m_handler, MSG_ENABLE_TRACKING);
          msgEnableTracking.arg1 = (request.reportLocation ? 1 : 0);
          m_handler.sendMessage(msgEnableTracking);
        }
      }
      catch (Exception e)
      {
        e.printStackTrace();
      }
    }

    public ProviderProperties getProperties()
    {
      return m_properties;
    }

    public int getStatus(Bundle extras)
    {
      try
      {
        int status = m_publicStatus.getStatus();
        llog.v(TAG, "getStatus: " + status);
        return status;
      }
      catch (Exception e)
      {
        e.printStackTrace();
        return LocationProvider.OUT_OF_SERVICE;
      }
    }

    public long getStatusUpdateTime()
    {
      try
      {
        return m_publicStatus.getStatusUpdateTime();
      }
      catch (Exception e)
      {
        e.printStackTrace ();
        return SystemClock.elapsedRealtime();
      }
    }

    public boolean sendExtraCommand(String command, Bundle extras)
    {
      // we don't support any extra command for now
      return false;
    }
  };

  public void handleEnableNlp()
  {
    try
    {
      llog.w(TAG, "handleEnable");
      if (m_publicStatus.getStatus() != LocationProvider.OUT_OF_SERVICE)
      {
        if (m_internalState == InternalState.IS_DISABLED)
        {
          // move to IDLE if we're DISABLED
          m_internalState = InternalState.IS_IDLE;
        }
      }
    }
    catch (Exception e)
    {
      e.printStackTrace ();
    }
  }

  public void handleDisableNlp()
  {
    try
    {
      llog.w(TAG, "handleDisable");

      if (m_publicStatus.getStatus() != LocationProvider.OUT_OF_SERVICE)
      {
        handleEnableLocationTracking(false);
        // move to DISABLED and flush current transaction ID used for XTWiFi requests
        m_internalState = InternalState.IS_DISABLED;
      }
    }
    catch (Exception e)
    {
      e.printStackTrace ();
    }
  }

  private void stopCurrentTrackingSession()
  {
    // stop the current running tracking session
    // remove all timeout call backs in the queue.
    // removing these call backs might avoid unnecessary wake ups
    m_handler.removeCallbacks(timeoutZppFix);
    m_handler.removeCallbacks(timeoutFixSession);
    m_alarmManager.cancel(m_PendingIntentTBFTimeout);

    // invalidate GTP transaction ID and session states
    m_current_gtp_req_tx_id = INVALID_TX_ID;
    m_xtwifi_tx_state = XtwifiFixState.XFS_IDLE_OR_WAITING;
    m_is_xtwifi_fix_with_lat_lon = false;
    m_is_xtwwan_fix_with_lat_lon = false;
    m_got_xtwwan_response = false;
    m_has_reported_anything_in_current_fix_session = false;

    // reset this timestamp for last fix in current tracking session
    // this is used to control TBF within the same tracking session
    // with multiple setMinTime
    // before we sleep for TBF, we should check this timestamp
    m_timestamp_last_fix_request_in_current_tracking_session = -1;

    if(m_wakeLock.isHeld())
    {
      m_wakeLock.release();
      llog.v(TAG, "Wakelock release");
    }

    if(m_isFixSessionGoingOn)
    {
      m_isFixSessionGoingOn = false;
      broadcastNLPIntent(NLP_ENABLED_CHANGE_ACTION, m_isFixSessionGoingOn);
    }
  }

  public void handleEnableLocationTracking(boolean enable)
  {
    try
    {
      llog.w(TAG, "handleEnableLocationTracking [" + enable + "]");

      if (m_internalState == InternalState.IS_DISABLED)
      {
        // ignore as we're already disabled
        llog.w(TAG, "handleEnableLocationTracking already in disabled state. ignore request");
      }
      else
      {
        if (enable)
        {
          if(LocationProvider.AVAILABLE != m_publicStatus.getStatus())
          {
            stopCurrentTrackingSession();
            // move state machine back to idle, so we're going to ignore any further
            // events
            m_internalState = InternalState.IS_IDLE;
            llog.w(TAG, "handleEnableLocationTracking not in available state. ignore request");
          }
          else if((InternalState.IS_TRACKING_WAIT_FOR_ZPP_FIX == m_internalState) ||
                  (InternalState.IS_TRACKING_WAIT_FOR_FINAL_FIX == m_internalState))
          {
            llog.v(TAG, "In the middle of a position request.Wait for the request to complete" +
                        " and report the position if available.");
          }
          else if(m_internalState == InternalState.IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX)
          {
            // if we are waiting on a xtwifi response on free wifi scan, continue waiting
            // and keep the GTP request pending
            llog.d(TAG, "EnableLocationTracking triggered " +
                        "while waiting for WiFi position on free scan." +
                        "Keep request pending");
            m_isGTPRequestPendingOnFreeWiFiScanPositioning = true;
          }
          else
          {
            stopCurrentTrackingSession();
            // start a tracking session using current TBF settings
            m_internalState = InternalState.IS_TRACKING_WAIT_FOR_TBF;

            // The reason behind calling this function directly instead of setting an alarm
            // which fires now - At boot up a sevice (InitAlaramService) reschedules/ delays
            // all alarms. This service is started pretty late. User is able to start a position
            // request before the service start. which is causing 1st response to users tracking
            // session to be delayed around 30 sec.
            llog.v(TAG, "handleEnableLocationTracking: Next Session Start at time(msec): " +
                   SystemClock.elapsedRealtime());
            m_alarmManager.cancel(m_PendingIntentTBFTimeout);
            fnTimeoutNextFixSessionDue();
            m_isTracking = true;
          }
        }
        else
        {
          stopCurrentTrackingSession();
          if(m_internalState != InternalState.IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX)
          {
            // move state machine back to idle, so we're going to ignore any further
            // events
            m_internalState = InternalState.IS_IDLE;
          }
          m_isTracking = false;
        }
      }
    }
    catch (Exception e)
    {
      e.printStackTrace ();
    }
  }

  public XTWiFiLocationProvider()
  {
    boolean successful = false;

    try
    {
      do
      {
        llog.w (TAG, "Constructor enter");

        // this is the private interface to underlying location manager service
        IBinder locManBinder = ServiceManager.getService(Context.LOCATION_SERVICE);
        m_LocationManager = ILocationManager.Stub.asInterface(locManBinder);
        if (null == m_LocationManager)
        {
          llog.e(TAG, "Unable to get private LOCATION_SERVICE");
          break;
        }

        readIzatConfig();

        if ((m_isXTWIFIEnabled == false) && (m_isXTWWANEnabled == false))
        {
          llog.e(TAG, "GTP WiFi and GTP AP cell both disabled");
          break;
        }

        readConfigurationItems ();

        m_handlerThread = new NetworkLocationProviderThread();
        m_handlerThread.start();
        successful = true;
        llog.w (TAG, "Constructor exit");
      } while (false);
    }
    catch (Exception e)
    {
      e.printStackTrace ();
    }

    if (successful == true)
    {
      m_publicStatus.setStatus(LocationProvider.TEMPORARILY_UNAVAILABLE);
      llog.w (TAG, "Constructor exit successful");
    }
    else
    {
      m_publicStatus.setStatus(LocationProvider.OUT_OF_SERVICE);
      llog.w (TAG, "Constructor exit failed");
    }
  }

  private void readIzatConfig()
  {
    Properties p = new Properties();
    int mode = 0;
    FileInputStream stream = null;

    try
    {
      stream = new FileInputStream(IZAT_CONFIG_FILE);
      p.load(stream);
      stream.close();
      String gtp_wwan_proc = p.getProperty("GTP_CELL_PROC");
      llog.v(TAG, "gtp wwan proc: " + gtp_wwan_proc);
      if((gtp_wwan_proc != null) && (gtp_wwan_proc.equals("AP")))
      {
        String gtp_wwan_mode = p.getProperty("GTP_CELL");
        llog.v(TAG, "gtp wwan mode: " + gtp_wwan_mode);
        if((gtp_wwan_mode != null) && (gtp_wwan_mode.equals("BASIC")))
        {
          llog.v(TAG, "Setting GTP WWAN mode to BASIC");
          m_isXTWWANEnabled = true;
        }
        else
        {
          llog.v(TAG, "Setting GTP WWAN mode to DISABLED");
          m_isXTWWANEnabled = false;
        }
      }
      else
      {
        m_isXTWWANEnabled = false;
      }

      String gtp_wifi_mode = p.getProperty("GTP_WIFI");
      llog.v(TAG, "gtp wifi mode: " + gtp_wifi_mode);
      if((gtp_wifi_mode != null) && (gtp_wifi_mode.equals("DISABLED") == true))
      {
        llog.v(TAG, "Setting GTP WIFI mode to DISABLED");
        m_isXTWIFIEnabled = false;
      }
      else
      {
        llog.v(TAG, "Setting GTP WIFI mode to BASIC");
        m_isXTWIFIEnabled = true;
      }
    }
    catch (FileNotFoundException e)
    {
      Log.w(TAG, "Could not find: IZAT configuration file " + IZAT_CONFIG_FILE);
      e.printStackTrace();
    }
    catch (IOException e)
    {
      Log.w(TAG, "Could not open IZAT configuration file " + IZAT_CONFIG_FILE);
      e.printStackTrace();
    }
    catch (Exception e)
    {
      e.printStackTrace();
    }
    finally
    {
      try
      {
        if (stream != null)
        {
          stream.close();
        }
      }
      catch (Exception e)
      {
        e.printStackTrace();
      }
    }
  }

  private Location markLocationSource(Location location, String source)
  {
    Bundle extras = location.getExtras();
    if (extras == null)
    {
      extras = new Bundle();
    }
    extras.putBoolean("com.qualcomm.location.nlp:source-qnp", true);
    extras.putString("com.qualcomm.location.nlp:source-technology", source);
    location.setExtras(extras);
    return location;
  }

  private void handleTryConnect()
  {
    if (m_ipcClient == null)
    {
      m_ipcClient = new MessageQueueClient(MQ_SOCKET);
      m_connectRetryCounter = 0;
    }

    if (!m_ipcClient.connect())
    {
      ++m_connectRetryCounter;
      if (m_connectRetryCounter < CONNECT_RETRY_LIMIT)
      {
        llog.w(TAG, "connection failure. count " + m_connectRetryCounter);
        m_handler.sendEmptyMessageDelayed(MSG_TRY_CONNECT, 5000);
      }
      else
      {
        llog.e(TAG, "connection failure. abort");
        m_publicStatus.setStatus(LocationProvider.OUT_OF_SERVICE);
      }
    }
    else
    {
      if ((m_zppAdaptor == null) && m_isZppEnabled)
      {
        try
        {
          m_zppAdaptor = new ZppAdaptor(m_handler, MSG_ZPP_FINISHED, m_log_level);
        }
        catch (LinkageError e)
        {
          e.printStackTrace();
          llog.e(TAG, "ZPP adaptor not found. disabling ZPP.");
          m_isZppEnabled = false;
        }
        catch (Exception e)
        {
          e.printStackTrace();
          m_isZppEnabled = false;
        }
      }

      try
      {
        m_ipcClient.startReceiverThread(m_handler, MSG_NEW_IPC_MESSAGE, MSG_IPC_EXCEPTION);

        // register
        OutputStream os;
        os = m_ipcClient.getOutputStream();
        OutPostcard out = new OutPostcard();
        out.init();
        out.addString("TO", "SERVER");
        out.addString("FROM", "XTWiFiLP");
        out.addString("REQ", "REGISTER");
        out.seal();
        os.write(out.getEncodedBuffer());
        os.flush();

        m_publicStatus.setStatus(LocationProvider.AVAILABLE);

        if ((m_internalState != InternalState.IS_DISABLED) &&
            (m_internalState != InternalState.IS_IDLE) &&
            (m_isTracking))
        {
          // This will happen in following cases
          // 1. if state is any of IS_TRACKING_WAIT_FOR_TBF, IS_TRACKING_WAIT_FOR_ZPP_FIX, or
          //    IS_TRACKING_WAIT_FOR_FINAL_FIX,
          // 2. if we are waiting on position request due to free wifi scan while being in
          //    a tracking session
          // we were in tracking session before re-connection
          // restart the tracking session!
          llog.w(TAG, "Restart tracking session after re-connection");
          handleEnableLocationTracking(false);
          handleEnableLocationTracking(true);
        }
      }
      catch (Exception e)
      {
        m_publicStatus.setStatus(LocationProvider.OUT_OF_SERVICE);
        llog.w(TAG, "cannot send register message");
        m_handler.sendEmptyMessage(MSG_IPC_EXCEPTION);
        // IPC exception
        e.printStackTrace ();
      }
    }
  }

  private void fnTimeoutNextFixSessionDue()
  {
    // time for a new fix session
    if ((m_internalState != InternalState.IS_TRACKING_WAIT_FOR_TBF) &&
        (m_internalState != InternalState.IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX))
    {
      // this is not supposed to happen, for we
      // 1) only inject alarm when we've been moved into this state
      // 2) remove any redundant callback when we're disabled
      llog.w(TAG, "fnTimeoutNextFixSessionDue triggered at wrong state " + m_internalState);
      return;
    }

    if(m_internalState == InternalState.IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX)
    {
      llog.w(TAG, "fnTimeoutNextFixSessionDue triggered " +
                  "while waiting for WiFi position on free scan." +
                  "Keep request pending");

      // we are waiting on a free wifi scan positioning request to xtwifi, do
      // 1) keep the new fix session pending to address later
      // 2) continue waiting for wifi position
      m_isGTPRequestPendingOnFreeWiFiScanPositioning = true;

      // set up alarm for reattempting after TBF
      // the alarm may be cancelled when free WiFi scan finishes
      m_alarmManager.setExact(AlarmManager.ELAPSED_REALTIME,
                              SystemClock.elapsedRealtime() + m_currentTimeBetweenFix,
                              m_PendingIntentTBFTimeout);

      llog.v(TAG, "alarm set time of next fix(msec): " +
             (SystemClock.elapsedRealtime() + m_currentTimeBetweenFix));
      return;
    }

    // We are addessing a GTP request. Clear pending flags
    m_isGTPRequestPendingOnFreeWiFiScanPositioning = false;

    if(false == m_wakeLock.isHeld())
    {
      m_wakeLock.acquire();
      llog.v(TAG, "Wakelock Acquired");
    }

    m_isFixSessionGoingOn = true;
    broadcastNLPIntent(NLP_ENABLED_CHANGE_ACTION, m_isFixSessionGoingOn);

    m_timestamp_last_fix_request_in_current_tracking_session = System.currentTimeMillis();

    // default is set to No ZPP support
    m_internalState = InternalState.IS_TRACKING_WAIT_FOR_FINAL_FIX;

    // inject exactly one timer for this fix session
    m_handler.removeCallbacks(timeoutFixSession);
    m_handler.postDelayed(timeoutFixSession, m_timeoutFinalFix);

    // check if we need to fire a ZPP request
    // if XTWWan is disabled only then fire a ZPP fix request
    if ((!m_isXTWWANEnabled) && m_isZppEnabled && (m_zppAdaptor != null) && (!isZppRequestSentRecently()))
    {
      if (m_zppAdaptor.triggerZppIfNotAlreadyRunning())
      {
        m_internalState = InternalState.IS_TRACKING_WAIT_FOR_ZPP_FIX;

        // inject exactly one timer for this ZPP session
        m_handler.removeCallbacks(timeoutZppFix);
        m_handler.postDelayed(timeoutZppFix, m_timeoutZppFix);
      }
      else
      {
        // something is wrong
        m_isZppEnabled = false;
        llog.e(TAG, "fnTimeoutNextFixSessionDue cannot trigger ZPP fix. disable ZPP");
      }
    }
    else
    {
      // we don't have ZPP at all
      // or we just got a report from ZPP recently. skip
      // or XTWWAN may be enabled
    }

    // prepare transaction ID for GTP
    ++m_next_gtp_req_tx_id;
    if (m_next_gtp_req_tx_id > MAX_VALID_TX_ID)
    {
      m_next_gtp_req_tx_id = MIN_VALID_TX_ID;
    }
    m_current_gtp_req_tx_id = m_next_gtp_req_tx_id;
    // Automation -log - Please do not touch
    llog.d(TAG, "fireGTPFixSession: Fire a position request to GTP-AP " + m_next_gtp_req_tx_id);

    if (m_isXTWWANEnabled)
    {
      m_internalState = InternalState.IS_TRACKING_WAIT_FOR_ZPP_FIX;

      // fire XTWWAN fix request here
      try
      {
        // send out position request
        // if IPC connection is broken, m_ipcClient could be null, also os could be null
        OutputStream os = m_ipcClient.getOutputStream();
        OutPostcard out = new OutPostcard();
        out.init();
        out.addString("TO", "XTWWAN-PE");
        out.addString("FROM", "XTWiFiLP");
        out.addString("REQ", "POSITION");
        out.addInt32("TX-ID", m_current_gtp_req_tx_id);
        out.seal();
        os.write(out.getEncodedBuffer());
        os.flush();
      }
      catch (Exception e)
      {
        m_publicStatus.setStatus(LocationProvider.OUT_OF_SERVICE);
        llog.e(TAG, "cannot send POSITION request message to XTWWAN-PE");
        // We dont want to release wakelock here. even if we did we need to acquire it
        // again when we try to send request to wifi position engine
        m_handler.sendEmptyMessage(MSG_IPC_EXCEPTION);
        // IPC exception
        e.printStackTrace ();
      }

      // inject exactly one timer for this XTWWAN session
      m_handler.removeCallbacks(timeoutZppFix);
      m_handler.postDelayed(timeoutZppFix, m_timeoutZppFix);
    }
    else
    {
     // XTWWan is not enabled , may be ZPP fix is enabled
    }

    // fire XTWiFi fix request here
    try
    {
      // send out position request
      // if IPC connection is broken, m_ipcClient could be null, also os could be null
      OutputStream os = m_ipcClient.getOutputStream();
      OutPostcard out = new OutPostcard();
      out.init();
      out.addString("TO", "XTWiFi-PE");
      out.addString("FROM", "XTWiFiLP");
      out.addString("REQ", "POSITION");
      out.addInt32("TX-ID", m_current_gtp_req_tx_id);
      out.addBool("SERVER-ACCESS-ALLOWED", true);
      long now = System.currentTimeMillis();
      long stat_collection_age = now - m_timestamp_stats_collection_start;
      if((m_time_sec_min_age_reporting_stats_collected <= (stat_collection_age/1000)) &&
         ((m_stats_wifi_fix_inconsistent_with_zpp > 0) ||
          (m_stats_fix_dropped_too_large_radius > 0) ||
          (m_stats_wifi_fix_dropped_calculated_with_single_AP > 0)))
      {
        llog.i("metrics.lbs","qclbs:data:wifi_fix_inconsistent_with_zpp=" +
                             m_stats_wifi_fix_inconsistent_with_zpp + ";CT;1," +
                             "fix_dropped_too_large_radius=" + m_stats_fix_dropped_too_large_radius +
                             ";CT;1,dropped_wifi_fix_due_to_single_AP=" +
                             m_stats_wifi_fix_dropped_calculated_with_single_AP + ";CT;1:NR");
        m_timestamp_stats_collection_start = now;
        m_stats_wifi_fix_inconsistent_with_zpp = 0;
        m_stats_fix_dropped_too_large_radius = 0;
        m_stats_wifi_fix_dropped_calculated_with_single_AP = 0;
      }
      out.seal();
      os.write(out.getEncodedBuffer());
      os.flush();
    }
    catch (Exception e)
    {
      // IPC exception
      e.printStackTrace();
      m_publicStatus.setStatus(LocationProvider.OUT_OF_SERVICE);
      llog.e(TAG, "cannot send POSITION request message to XTWiFi-PE");

      // We dont want to release wakelock here. We may be waiting on a ZPP fix. better to wait
      // for timeout to happen

      m_handler.sendEmptyMessage(MSG_IPC_EXCEPTION);
    }
  }

  private void handleZppFix()
  {
    m_handler.removeCallbacks(timeoutZppFix);
    if (m_internalState == InternalState.IS_TRACKING_WAIT_FOR_ZPP_FIX)
    {
      m_internalState = InternalState.IS_TRACKING_WAIT_FOR_FINAL_FIX;

      if ((m_xtwifi_tx_state == XtwifiFixState.XFS_PRELIMINARY) ||
          (m_xtwifi_tx_state == XtwifiFixState.XFS_FINAL))
      {
        // it seems we already have some fix report from xtwifi.
        // let's process it as now we know there will be no ZPP fix coming
        handleFixReportFromXtwifi();

        // clean up xtwifi transaction after we get the final fix
        if (m_xtwifi_tx_state == XtwifiFixState.XFS_FINAL)
        {
          // reset session status
          reInstallTbfTimer();
        }
      }
      else
      {
        // we don't have any fix from XTWiFi-PE yet.
        // Check if we should report ZPP fix without waiting for XTWiFi fix
        // We can report the ZPP fix if its hepe is considerably low and
        // technology source is GPS
        llog.v(TAG, "handleZppFix check if we can report ZPP fix " +
                    "without waiting for XTWiFi final fix");
        if(isZppFixDirectlyReportable())
        {
          m_internalState = InternalState.IS_TRACKING_WAIT_FOR_FINAL_FIX;

          // remove callback this fix session timeout
          m_handler.removeCallbacks(timeoutFixSession);
          m_xtwifi_tx_state = XtwifiFixState.XFS_FINAL;
          // handleFixReportFromXtwifi also reports the ZPP fix
          handleFixReportFromXtwifi();
          // reset session status
          reInstallTbfTimer();
        }
        else
        {
          // we don't want to directly report ZPP fix. keep waiting for XTWiFi final fix
          llog.v(TAG, "handleZppFix keep waiting for XTWiFi final fix");
        }
      }
    }
    else
    {
      // something is wrong
      llog.w(TAG, "handleZppFix fired in wrong state " + m_internalState);
    }
  }

  private Runnable timeoutZppFix = new Runnable()
  {
    @Override
    public void run()
    {
      try
      {
        // ZPP fix timeout
        llog.w(TAG, "timeoutZppFix ZPP session timed out");

        if (m_internalState == InternalState.IS_TRACKING_WAIT_FOR_ZPP_FIX)
        {
          handleZppFix();
        }
      }
      catch (Exception e)
      {
        e.printStackTrace ();
      }
    }
  };

  private Runnable timeOutFreeXtWiFiFixSession = new Runnable()
  {
    @Override
    public void run()
    {
      try
      {
        // Fix session timeout
        llog.w(TAG, "timeOutFreeXtWiFiFixSession fix session timed out");

        switch (m_internalState)
        {
        case IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX:
          // we didn't get the final fix report from XTWiFi-PE
          m_xtwifi_on_free_scan_tx_state = XtwifiFixState.XFS_FINAL;
          handleFixReportFromXtwifi();
          break;
        default:
          // ignore
          llog.w(TAG, "timeOutFreeXtWiFiFixSession fired at wrong state " + m_internalState);
          return;
        }
        restoreTrackingSessionAfterFreeWiFiScan();
      }
      catch (Exception e)
      {
        e.printStackTrace ();
      }
    }
  };

  private Runnable timeoutFixSession = new Runnable()
  {
    @Override
    public void run()
    {
      try
      {
        // Fix session timeout
        llog.w(TAG, "timeoutFixSession fix session timed out");

        switch (m_internalState)
        {
        case IS_TRACKING_WAIT_FOR_ZPP_FIX:
          llog.w(TAG, "timeoutFixSession fired before ZPP timeout?");
          break;
        case IS_TRACKING_WAIT_FOR_FINAL_FIX:
          // we didn't get the final fix report from XTWiFi-PE
          // assume we just got one final report, without altering the validity bit
          // so if the validity bit would be kept if we have received any
          // preliminary reports
          m_xtwifi_tx_state = XtwifiFixState.XFS_FINAL;
          handleFixReportFromXtwifi();
          break;
        default:
          // ignore
          llog.w(TAG, "timeoutFixSession fired at wrong state " + m_internalState);
          return;
        }

        // reset session status no matter what
        reInstallTbfTimer();
      }
      catch (Exception e)
      {
        e.printStackTrace ();
      }
    }
  };

  private void reInstallTbfTimer()
  {
    m_internalState = InternalState.IS_TRACKING_WAIT_FOR_TBF;

    // invalidate GTP transaction ID and session state
    m_current_gtp_req_tx_id = INVALID_TX_ID;
    m_xtwifi_tx_state = XtwifiFixState.XFS_IDLE_OR_WAITING;
    m_is_xtwifi_fix_with_lat_lon = false;
    m_is_xtwwan_fix_with_lat_lon = false;
    m_got_xtwwan_response = false;

    long now = System.currentTimeMillis();
    long time_to_next_fix = 0;

    if ((m_timestamp_last_fix_request_in_current_tracking_session < 0)
        || (m_timestamp_last_fix_request_in_current_tracking_session > now))
    {
      // < 0: we have not started any fix in this tracking session
      // > now: system time adjustment?
      // retry immediately
      time_to_next_fix = 0;
    }
    else if(m_has_reported_anything_in_current_fix_session)
    {
      time_to_next_fix = m_currentTimeBetweenFix;
    }
    else
    {
      // we haven't reported anything in the last fix session, delay for awhile and restart
      // we do not want to squeeze the system too much by setting shorter TBF here
      // as it contributes to harmless race condition with coarse position injection
      // and potentially too fast wifi scan rate
      time_to_next_fix = m_currentTimeBetweenFix;
    }

    // reset the remaining state variable here
    m_has_reported_anything_in_current_fix_session = false;

    // flush all timeout call backs and inject one for TBF
    m_handler.removeCallbacks(timeoutZppFix);
    m_handler.removeCallbacks(timeoutFixSession);
    m_alarmManager.cancel(m_PendingIntentTBFTimeout);
    m_alarmManager.setExact(AlarmManager.ELAPSED_REALTIME,
                            SystemClock.elapsedRealtime() + time_to_next_fix,
                            m_PendingIntentTBFTimeout);

    llog.w(TAG, "reInstallTbfTimer set timer to next fix: " + time_to_next_fix);
    llog.v(TAG, "reInstallTbfTimer set time of next fix((msec)): " +
           (SystemClock.elapsedRealtime() + time_to_next_fix));

    if(m_wakeLock.isHeld())
    {
      m_wakeLock.release();
      llog.v(TAG, "Wakelock release");
    }

    if(m_isFixSessionGoingOn)
    {
      m_isFixSessionGoingOn = false;
      broadcastNLPIntent(NLP_ENABLED_CHANGE_ACTION, m_isFixSessionGoingOn);
    }
  }

  private boolean isZppRequestSentRecently()
  {
    if (!m_isZppEnabled)
    {
      // ZPP is not enabled!
      return false;
    }

    if(m_maxAgeForZppTransaction == 0)
    {
      return false;
    }

    long now = System.currentTimeMillis();
    long age_ZPP_tx = -1;

    if ((m_timestampLastZppReport > 0) && (m_timestampLastZppReport < now))
    {
      // timer seems to be valid
      age_ZPP_tx = now - m_timestampLastZppReport;
    }

    if ((age_ZPP_tx > 0) && (age_ZPP_tx < m_maxAgeForZppTransaction))
    {
      // current ZPP fix is acquired within the configured time limit
      return true;
    }
    else
    {
      return false;
    }
  }

  private boolean isZppFixDirectlyReportable()
  {
    boolean shouldReport = true;
    if(!(isZppFixGood()))
    {
      shouldReport = false;
    }
    else
    {
      Bundle zppLocationExtra = m_lastKnownZppFix.getExtras();
      if (zppLocationExtra == null)
      {
        shouldReport = false;
      }
      else if(0 == ((ZppAdaptor.TECH_SOURCE_SATELLITE | ZppAdaptor.TECH_SOURCE_SENSORS) &
                    zppLocationExtra.getInt(ZppAdaptor.TECH_SOURCE_BITMASK)))
      {
        shouldReport = false;
      }
      else if(m_lastKnownZppFix.getAccuracy() >= m_hepe_margin_meter_report_zpp_location)
      {
        shouldReport = false;
      }
      else
      {
        // immediately report the fix
      }
    }

    return shouldReport;
  }

  private boolean isZppFixGood()
  {
    if ((!m_isZppEnabled) && (!m_isXTWWANEnabled))
    {
      return false;
    }

    llog.v(TAG, "isZppFixGood ZPP Enabled = " + m_isZppEnabled + " XTWWAN Enabled = " +
           m_isXTWWANEnabled);

    m_lastKnownZppFix.reset();
    boolean is_zpp_fix_valid = false;

    if (m_isXTWWANEnabled)
    {
      is_zpp_fix_valid = m_is_xtwwan_fix_with_lat_lon;
      if (is_zpp_fix_valid == true)
      {
        m_lastKnownZppFix.set (m_lastKnownWwanFix);
      }
    }
    else // m_isZppEnabled == true
    {
      if(m_zppAdaptor != null)
      {
        is_zpp_fix_valid = m_zppAdaptor.getLastKnownLocation(m_lastKnownZppFix);
      }
      else
      {
        llog.w(TAG, "isZppFixGood: ZPP adaptor null");
      }
    }

    if (!is_zpp_fix_valid)
    {
      // last ZPP transaction must has failed. probably ZPP couldn't tell us where we are
      llog.w(TAG, "isZppFixGood: ZPP fix is not valid");
    }
    else
    {
      // JB MR1 change
      m_lastKnownZppFix.setProvider(LocationManager.NETWORK_PROVIDER);
      m_lastKnownZppFix.setElapsedRealtimeNanos(SystemClock.elapsedRealtimeNanos());

      // Adjust PUNC of zpp or GTP AP cell fix based on time propogation
      adjustZPPFixPUNC();
    }

    return is_zpp_fix_valid;
  }

  // This function will increment ZPP fix or GTP AP cell PUNC based on
  // the time difference from the time fix was obtained and current time
  private void adjustZPPFixPUNC()
  {
    if(m_timestampLastZppReport < 0)
    {
      // No ZPP fix received so far
    }
    else
    {
      long now = System.currentTimeMillis();
      long age_ZPP_fix = now - m_timestampLastZppReport;

      if (age_ZPP_fix <= 0)
      {
        // it seems the ZPP fix is still fresh
      }
      else
      {
        // fix is stale, adjust Punc
        float age_ZPP_fix_sec = age_ZPP_fix/((float)1000);
        float punc_incr_m = age_ZPP_fix_sec * PUNC_INCR_METERS_PER_SECOND;
        llog.v(TAG, "adjustZPPFixPUNC increase PUNC by " + punc_incr_m + " for "
               + age_ZPP_fix_sec + "seconds");
        // PUNC growth model need to be applied to 99% confidence level
        float punc_99 = punc_incr_m +
                        m_lastKnownZppFix.getAccuracy() / SCALE_FACTOR_PUNC_CONF_99_TO_68;
        float punc_68 = punc_99 * SCALE_FACTOR_PUNC_CONF_99_TO_68;
        llog.v(TAG, "adjustZPPFixPUNC, previous PUNC (68%) " + m_lastKnownZppFix.getAccuracy() +
               ", adjusted for PUNC growth to (68%):" + punc_68 + "m");
        // Scale 99% PUNC back to 68% for reporting
        m_lastKnownZppFix.setAccuracy(punc_68);
      }
    }
  }

  private void verifyWithZppAndReportXtwifiFix()
  {
    boolean should_report = true;

    if (!m_is_xtwifi_fix_with_lat_lon)
    {
      should_report = false;
    }
    else
    {
      if (isZppFixGood())
      {
        // since we already have the ZPP fix, let's validate the XTWiFi fix
        // (no matter it's preliminary or final) and
        // send out the fix if it's good
        float distance = m_lastKnownZppFix.distanceTo(m_lastKnownXtwifiFix);
        float zpp_accuracy_99 = m_lastKnownZppFix.getAccuracy() / SCALE_FACTOR_PUNC_CONF_99_TO_68;
        // Automation -log - Please do not touch
        llog.v(TAG, "Distance between ZPP and XTWiFi fixes: " + distance +
                    " m, while accuracy of ZPP fix is " + zpp_accuracy_99 + " m (99%), " +
                    m_lastKnownZppFix.getAccuracy() + " m (68%)");
        if (distance > zpp_accuracy_99)
        {
          should_report = false;
          m_stats_wifi_fix_inconsistent_with_zpp += 1;
          llog.w(TAG, "Dropping XTwiFi fix because of ZPP inconsistency");
        }
      }
    }

    if (should_report)
    {
      try
      {
        // defining location report varibale if its true we report position
        boolean report_location = true;

        // This is 1-AP case we check which HEPE is greater and swap the HEPE and check against threshold

        if (m_num_AP_cache == 1)
        {
          if (!isZppFixGood())
          {
            llog.w(TAG, "Dropping XTwiFi fix because cache hit is 1 and "+
                        "no ZPP fix available for comparison");
            m_stats_wifi_fix_dropped_calculated_with_single_AP += 1;
            report_location = false;
          }
          else
          {
            if (m_lastKnownXtwifiFix.getAccuracy()< m_lastKnownZppFix.getAccuracy())
            {
              llog.v(TAG, "WiFi position calculated only with one AP. Swapping HEPE value with ZPP" +
                          "Previous hepe value: " + m_lastKnownXtwifiFix.getAccuracy() +
                          "hepe value: " + m_lastKnownZppFix.getAccuracy());
              m_lastKnownXtwifiFix.setAccuracy(m_lastKnownZppFix.getAccuracy());
            }
          }
        }

        // Compare the swapped HEPE with pre-defined threshold
        if (m_lastKnownXtwifiFix.getAccuracy() > m_large_accuracy_threshold_to_filter_nlp_position)
        {
          llog.w(TAG, "Dropping XTwiFi fix - because value of HEPE is greater than threshold "+
                      "-- XTWiFi HEPE(" + m_lastKnownXtwifiFix.getAccuracy() + " HEPE_Thres " +
                 m_large_accuracy_threshold_to_filter_nlp_position);
          report_location = false;
          m_stats_fix_dropped_too_large_radius += 1;
        }

        // Report_location only if above conditions are met
        if (report_location)
        {
          // Automation -log - Please do not touch
          llog.v(TAG, "Reporting XTWiFi fix (" + m_lastKnownXtwifiFix.getLatitude() + ", " +
                      m_lastKnownXtwifiFix.getLongitude() + "), unc " +
                      m_lastKnownXtwifiFix.getAccuracy() + " time (ms) " +
                      m_lastKnownXtwifiFix.getElapsedRealtimeNanos() / 1000000);

          // JB MR1 change
          Location fuzzifiedLocation = new Location(m_lastKnownXtwifiFix);
          m_lastKnownXtwifiFix.setExtraLocation(Location.EXTRA_NO_GPS_LOCATION, fuzzifiedLocation);

          m_lastKnownXtwifiFix = markLocationSource(m_lastKnownXtwifiFix, "GTP-AP-WIFI");

          m_LocationManager.reportLocation(new Location(m_lastKnownXtwifiFix), false);
          // set to true whenever we report anything in this fix session
          m_has_reported_anything_in_current_fix_session = true;
          broadcastNLPIntent(NLP_FIX_CHANGE_ACTION, m_has_reported_anything_in_current_fix_session);
        }

      }
      catch (RemoteException e)
      {
        llog.w(TAG, "Exception in reporting location to LocationManagerService (2)");
        e.printStackTrace();
      }
    }
  }

  private void handleFixReportFromXtwifi()
  {
    // Note we do not call reInstallTbfTimer here because that routine needs to be called
    // with all possible states in timeoutFixSession, after calling this routine

    if ((m_internalState != InternalState.IS_TRACKING_WAIT_FOR_FINAL_FIX) &&
        (m_internalState != InternalState.IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX))
    {
      llog.e(TAG, "handleFixReportFromXtwifi called in wrong state " + m_internalState);
      return;
    }

    if(m_internalState == InternalState.IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX)
    {
      if((m_xtwifi_on_free_scan_tx_state == XtwifiFixState.XFS_FINAL) &&
         (m_is_xtwifi_fix_with_lat_lon) &&
         (m_zppAdaptor != null) &&
         (m_num_AP_cache > 1))
      {
        // we have a wifi fix for CPI
        // Inject fix
        // If we don't have a fix we dont't want to inject
        // as the injecion without position can not really serve the purpose

        // If this fix is calculated with only 1 AP found in cache don't inject
        llog.v(TAG, "Injecting XTWiFi fix on free WiFi scan");
        float hepe = m_lastKnownXtwifiFix.getAccuracy();
        m_zppAdaptor.injectFix(INJECT_SRC_WIFI, m_is_xtwifi_fix_with_lat_lon,
                               m_lastKnownXtwifiFix.getLatitude(), m_lastKnownXtwifiFix.getLongitude(),
                               hepe, -1, 0, -1, m_lastKnownXtwifiFix_mac_array,
                               m_lastKnownXtwifiFix_rssi_array, m_lastKnownXtwifiFix_channel_array);
      }
      return;
    }

    if(!(isZppFixDirectlyReportable()))
    {
      verifyWithZppAndReportXtwifiFix();
    }

    if ((!m_has_reported_anything_in_current_fix_session) &&
        (m_xtwifi_tx_state == XtwifiFixState.XFS_FINAL) && (isZppFixGood()))
    {
      // since XTWiFi-PE failed to provide any good fix, let's see if we can
      // report ZPP instead
      try
      {
        // Filtering out ZPP fix if it doesnt pass HEPE criteria
        if (m_lastKnownZppFix.getAccuracy() > m_large_accuracy_threshold_to_filter_nlp_position)
        {
          llog.w(TAG, "Dropping ZPP fix because value of HEPE is greater than set threshold "+
                      "-- ZPP HEPE(" + m_lastKnownZppFix.getAccuracy() + " HEPE_Thres " +
                      m_large_accuracy_threshold_to_filter_nlp_position);
          m_stats_fix_dropped_too_large_radius += 1;
        }
        else
        {
          // Automation -log - Please do not touch
          llog.v(TAG, "Reporting ZPP fix (" + m_lastKnownZppFix.getLatitude() + ", " +
                      m_lastKnownZppFix.getLongitude() + "), unc " + m_lastKnownZppFix.getAccuracy() +
                      " time (ms) " + m_lastKnownZppFix.getElapsedRealtimeNanos() / 1000000);

          // JB MR1 change
          Location fuzzifiedLocation = new Location(m_lastKnownXtwifiFix);
          m_lastKnownZppFix.setExtraLocation(Location.EXTRA_NO_GPS_LOCATION, fuzzifiedLocation);


          if (m_isXTWWANEnabled)
          {
            m_lastKnownZppFix = markLocationSource(m_lastKnownZppFix, "GTP-AP-WWAN");
          }
          else
          {
            m_lastKnownZppFix = markLocationSource(m_lastKnownZppFix, "ZPP");
          }

          m_LocationManager.reportLocation(new Location(m_lastKnownZppFix), false);
          // set to true whenever we report anything in this fix session
          m_has_reported_anything_in_current_fix_session = true;
          broadcastNLPIntent(NLP_FIX_CHANGE_ACTION, m_has_reported_anything_in_current_fix_session);
        }
      }
      catch (RemoteException e)
      {
        llog.w(TAG, "Exception in reporting location to LocationManagerService (1)");
        e.printStackTrace ();
      }
    }

    if(!m_has_reported_anything_in_current_fix_session)
    {
      llog.v(TAG, "Reporting Nothing" );
      broadcastNLPIntent(NLP_FIX_CHANGE_ACTION, m_has_reported_anything_in_current_fix_session);
    }
  }

  private final BroadcastReceiver m_AlarmReceiver_TBFTimeout = new BroadcastReceiver()
  {
    @Override
    public void onReceive(Context context, Intent intent)
    {
      try
      {
        llog.v(TAG, "TBF timeout Alarm receiver called at time(msec) " +
                    SystemClock.elapsedRealtime());
        m_alarmManager.cancel(m_PendingIntentTBFTimeout);
        fnTimeoutNextFixSessionDue();
      }
      catch (Exception e)
      {
        e.printStackTrace ();
      }
    }
  };

  private boolean mReceiverRegistered = false;
  private final BroadcastReceiver m_WiFiScanResultReceiver = new BroadcastReceiver()
  {
    @Override
    public void onReceive(Context context, Intent intent)
    {
      try
      {
        llog.v(TAG, "Wifi scan result receiver called");
        m_handler.sendEmptyMessage(MSG_WIFI_SCAN_RESULT_AVAILABLE);
      }
      catch (Exception e)
      {
        e.printStackTrace ();
      }
    }
  };

  // We are allowed to send Free Ride Request if
  // 1. We are in either IS_IDLE or IS_TRACKING_WAIT_FOR_TBF state and
  // 2. last fix request on free wifi scan happened atleast m_minTimeBetweenFixOnFreeWiFiSca(default 20sec) away
  // 3. last fix request triggered by NLP happened atleast m_minTimeBetweenFixOnFreeWiFiScan(default 20sec) away
  private boolean shouldSendFreeRideRequest ()
  {
    if((m_internalState != InternalState.IS_IDLE) &&
       (m_internalState != InternalState.IS_TRACKING_WAIT_FOR_TBF))
    {
      // We are in any of the following states
      // IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX - we are laready waiting on a free wifi scan position request
      // IS_TRACKING_WAIT_FOR_FINAL_FIX, IS_TRACKING_WAIT_FOR_ZPP_FIX - already waiting on a position request from NLP
      // we can use the position once those requests come back. ignore
      llog.v(TAG, "shouldSendFreeRideRequest: fix request session is already active, ignore");
      return false;
    }

    long now = System.currentTimeMillis();

    // check this de-bouncing timer
    // we do not want too freq free ride requests
    // Checking the time between last fix request on free wifi scan and now
    if(m_timestamp_last_fix_request_on_free_wifi_scan != 0)
    {
      if(now > m_timestamp_last_fix_request_on_free_wifi_scan)
      {
        if((now - m_timestamp_last_fix_request_on_free_wifi_scan) < m_minTimeBetweenFixOnFreeWiFiScan)
        {
          llog.v(TAG, "shouldSendFreeRideRequest: free fix request was sent out not too long ago, ignore");
          return false;
        }
      }
      else
      {
        // maybe system time has rolled back? ignore the timer check now
      }
    }

    // Checking the time between GTP fix request by NLP trigger and now
    if(m_internalState == InternalState.IS_TRACKING_WAIT_FOR_TBF)
    {
      // We already send out a position request by NLP trigger
      // we do not want too freq free ride requests
      if(m_timestamp_last_fix_request_in_current_tracking_session != 0)
      {
        if(now > m_timestamp_last_fix_request_in_current_tracking_session)
        {
          if((now - m_timestamp_last_fix_request_in_current_tracking_session) < m_minTimeBetweenFixOnFreeWiFiScan)
          {
            llog.v(TAG, "shouldSendFreeRideRequest: fix request was sent out not too long ago, ignore");
            return false;
          }
        }
        else
        {
          // maybe system time has rolled back? ignore the timer check now
        }
      }
    }

    return true;
  }

  private void handleFreeWiFiScanResultForPositioning ()
  {
    long now = System.currentTimeMillis();
    m_timestamp_last_wifi_scan_result = now;

    llog.v(TAG, "handleFreeWiFiScanResultForPositioning: wifi scan result is available");

    if(!m_isEnabledProcessFreeWiFiScan)
    {
      // free ride is not enabled
      return;
    }

    if(false == shouldSendFreeRideRequest())
    {
      llog.v(TAG, "handleFreeWiFiScanResultForPositioning: "+
                  "free fix request is not allowed at this time, ignore");
      return;
    }

    m_timestamp_last_fix_request_on_free_wifi_scan = now;

    // prepare transaction ID
    if ((INVALID_TX_ID == m_current_xtwifi_on_free_scan_req_tx_id) ||
        (m_current_xtwifi_on_free_scan_req_tx_id == MAX_VALID_TX_ID_ON_FREE_WIFI_SCAN))
    {
      m_current_xtwifi_on_free_scan_req_tx_id = MIN_VALID_TX_ID_ON_FREE_WIFI_SCAN;
    }
    else
    {
      ++m_current_xtwifi_on_free_scan_req_tx_id;
    }

    if(false == m_wakeLock.isHeld())
    {
      m_wakeLock.acquire();
      llog.v(TAG, "Wakelock Acquired");
    }

    // Automation -log - Please do not touch
    llog.d(TAG, "fireGTPFixSession: Fire a position request to XTWiFi-PE on free wifi scan " +
           m_current_xtwifi_on_free_scan_req_tx_id);
    try
    {
      // send out position request
      // if IPC connection is broken, m_ipcClient could be null, also os could be null
      OutputStream os = m_ipcClient.getOutputStream();
      OutPostcard out = new OutPostcard();
      out.init();
      out.addString("TO", "XTWiFi-PE");
      out.addString("FROM", "XTWiFiLP");
      out.addString("REQ", "POSITION");
      out.addInt32("TX-ID", m_current_xtwifi_on_free_scan_req_tx_id);
      out.addBool("SERVER-ACCESS-ALLOWED", false);
      out.seal();
      os.write(out.getEncodedBuffer());
      os.flush();
      m_internalState = InternalState.IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX;
      m_xtwifi_on_free_scan_tx_state = XtwifiFixState.XFS_IDLE_OR_WAITING;
      m_handler.removeCallbacks(timeOutFreeXtWiFiFixSession);
      m_handler.postDelayed(timeOutFreeXtWiFiFixSession, m_timeoutFreeXtWiFiFix);
    }
    catch (Exception e)
    {
      m_publicStatus.setStatus(LocationProvider.OUT_OF_SERVICE);
      llog.e(TAG, "cannot send POSITION request message to XTWiFi-PE");
      if(m_wakeLock.isHeld())
      {
        m_wakeLock.release();
        llog.v(TAG, "Wakelock release");
      }
      m_handler.sendEmptyMessage(MSG_IPC_EXCEPTION);
      // IPC exception
      e.printStackTrace ();
    }
  }

  private void restoreTrackingSessionAfterFreeWiFiScan()
  {
    m_handler.removeCallbacks(timeOutFreeXtWiFiFixSession);
    m_xtwifi_on_free_scan_tx_state = XtwifiFixState.XFS_IDLE_OR_WAITING;
    if(m_wakeLock.isHeld())
    {
      m_wakeLock.release();
      llog.v(TAG, "Wakelock release");
    }

    if(!m_isTracking)
    {
      m_internalState = InternalState.IS_IDLE;
    }
    else if(true == m_isGTPRequestPendingOnFreeWiFiScanPositioning)
    {
      m_internalState = InternalState.IS_TRACKING_WAIT_FOR_TBF;
      boolean start_fix_now = false;
      long    timestamp_now = System.currentTimeMillis();
      long    timestamp_next_fix_due = m_timestamp_last_fix_request_in_current_tracking_session + m_currentTimeBetweenFix;

      // m_timestamp_last_fix_request_in_current_tracking_session == -1:
      //    first fix in the tracking session is kept pending
      // m_timestamp_last_fix_request_in_current_tracking_session > now:
      //    time rolls back, to be safe, let us request fix now
      if ((m_timestamp_last_fix_request_in_current_tracking_session == -1) ||
          (m_timestamp_last_fix_request_in_current_tracking_session > timestamp_now))
      {
        start_fix_now = true;
      }
      // Fix was needed in the past but kept pending, let us request now
      else if (timestamp_next_fix_due <= timestamp_now)
      {
        start_fix_now = true;
      }

      llog.v(TAG, "Adressing pending GTP request at time(msec): " + timestamp_now +  "last fix request time (msec):" +
              m_timestamp_last_fix_request_in_current_tracking_session + ", start NLP fix now: " + start_fix_now);

      if(start_fix_now == true)
      {
        m_alarmManager.cancel(m_PendingIntentTBFTimeout);
        fnTimeoutNextFixSessionDue();
      }
    }
    else
    {
      // tracking is enabled, but there was no pending GTP request to address
      m_internalState = InternalState.IS_TRACKING_WAIT_FOR_TBF;
    }
  }

  private void handleNewIpcMessage(InPostcard in_card)
  {
    try
    {
      String msg_from = in_card.getString("FROM");
      String msg_resp = in_card.getStringDefault ("RESP", "");
      int msg_tx_id = in_card.getInt32("TX-ID");
      boolean is_preliminary_fix = in_card.getBoolDefault("PRELIMINARY-FIX", false);

      // Automation -log - Please do not touch
      llog.v(TAG, "FROM: " + msg_from + "RESP:" + msg_resp + ", TX-ID:" + msg_tx_id +
                  ", Preliminary: " + is_preliminary_fix);

      if(msg_resp.equalsIgnoreCase(""))
      {
        llog.d(TAG, "Not a response message, ignore");
        return;
      }

      if (!msg_resp.equalsIgnoreCase("POSITION"))
      {
        throw new Exception("unknown response msg: " + msg_resp);
      }

      switch (m_internalState)
      {
      case IS_TRACKING_WAIT_FOR_ZPP_FIX:
      case IS_TRACKING_WAIT_FOR_FINAL_FIX:
        // match transaction id for NLP driven requests
        if (msg_tx_id != m_current_gtp_req_tx_id)
        {
          // transaction mismatch, ignore
          throw new Exception("handleNewIpcMessage TX ID mismatch was expecting:"
                              + m_current_gtp_req_tx_id);
        }
        break;
      case IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX:
        // match transaction id for free wifi scan driven fixes
        if (msg_tx_id != m_current_xtwifi_on_free_scan_req_tx_id)
        {
          // transaction mismatch, ignore
          throw new Exception("handleNewIpcMessage TX ID mismatch was expecting:"
                              + m_current_xtwifi_on_free_scan_req_tx_id);
        }
        break;
      default:
        // ignore this IPC message
        throw new Exception("handleNewIpcMessage fired at wrong state"
                              + m_internalState);
      }

      if (msg_from.equals("XTWiFi-PE"))
      {
        if (is_preliminary_fix)
        {
          // this is one of the preliminary fixes for current XTWiFi-PE
          // transaction
          if((m_internalState != InternalState.IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX) &&
             (m_xtwifi_tx_state == XtwifiFixState.XFS_IDLE_OR_WAITING))
          {
            // this state is needed when we receive ZPP fix or when ZPP timed out
            m_xtwifi_tx_state = XtwifiFixState.XFS_PRELIMINARY;
          }
          else
          {
            // We are not expecting a preliminary fix for free wifi scan requests
            // as we dont allow server access for them
            llog.w(TAG, "handleNewIpcMessage Receive preliminary fix at wrong state " + m_internalState);
          }
        }
        else
        {
          if((m_internalState == InternalState.IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX) &&
             (m_xtwifi_on_free_scan_tx_state == XtwifiFixState.XFS_IDLE_OR_WAITING))
          {
            m_xtwifi_on_free_scan_tx_state = XtwifiFixState.XFS_FINAL;
          }
          else if((m_internalState != InternalState.IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX) &&
            ((m_xtwifi_tx_state == XtwifiFixState.XFS_IDLE_OR_WAITING) ||
             (m_xtwifi_tx_state == XtwifiFixState.XFS_PRELIMINARY)))
          {
            // this state is needed when we receive ZPP fix or when ZPP timed out
            m_xtwifi_tx_state = XtwifiFixState.XFS_FINAL;
          }
          else
          {
            llog.w(TAG, "handleNewIpcMessage Receive final fix at wrong state " + m_internalState);
          }
        }

        try
        {
          m_num_AP_cache = in_card.getInt32("NUM-AP-USED-IN-FIX");
          double lat_deg = in_card.getDouble("LATITUDE_DEG");
          double lon_deg = in_card.getDouble("LONGITUDE_DEG");
          double hor_unc_meter = in_card.getFloat("HOR_UNC_M");

          llog.v(TAG, "Received WiFi Position; Lat: " + lat_deg + " lon: " + lon_deg +
                      " Number of Aps Used for position calculation: " + m_num_AP_cache);
          if(0 == m_num_AP_cache)
          {
            // We are not expectng this to happen.
            // Still continue processing after reporting error
            llog.e(TAG, "Received WiFi Position with Number of Aps Used for position "+
                        "calculation as 0 ");
          }

          m_lastKnownXtwifiFix.reset();
          m_lastKnownXtwifiFix.setProvider(LocationManager.NETWORK_PROVIDER);
          m_lastKnownXtwifiFix.setLatitude(lat_deg);
          m_lastKnownXtwifiFix.setLongitude(lon_deg);
          m_lastKnownXtwifiFix.setTime(System.currentTimeMillis());
          m_lastKnownXtwifiFix.setAccuracy((float) hor_unc_meter);

          // JB MR1 change
          m_lastKnownXtwifiFix.setElapsedRealtimeNanos(SystemClock.elapsedRealtimeNanos());

          // reaching here means we have a valid fix
          // note this flag is set to true every time we receive a response with
          // lat/lon/unc
          // but never cleared within this XTWiFi-PE transaction
          // this way we keep the last good result, which may or may not be the
          // final report
          m_is_xtwifi_fix_with_lat_lon = true;
        }
        catch (NoSuchElementException e)
        {
          // do nothing
          // note we're not resetting m_is_xtwifi_fix_valid to false
          // so if it's set to true by any fix report in this transaction, it
          // stays true
          llog.w(TAG, "No fix from XTWiFi-PE");
        }

        try
        {
          // flush older ones
          m_lastKnownXtwifiFix_mac_array = null;
          m_lastKnownXtwifiFix_rssi_array = null;
          m_lastKnownXtwifiFix_channel_array = null;

          // lookup WiFi scan result from the ipc message
          // note we might get exception if there is no WiFi result attached
          int mac_int_array [] = in_card.getArrayUInt8("SCAN_RESULT_MAC_ADDRESS");
          int rssi_array [] = in_card.getArrayInt16("SCAN_RESULT_RSSI");
          int channel_array [] = in_card.getArrayInt16("SCAN_RESULT_CHANNEL");

          m_lastKnownXtwifiFix_mac_array = mac_int_array;
          m_lastKnownXtwifiFix_rssi_array = rssi_array;
          m_lastKnownXtwifiFix_channel_array = channel_array;
        }
        catch (NoSuchElementException e)
        {
          // do nothing, as WiFi scan list report is considered as optional
          llog.w(TAG, "No WiFi scan result from XTWiFi-PE");

          // flush what we might have gotten
          m_lastKnownXtwifiFix_mac_array = null;
          m_lastKnownXtwifiFix_rssi_array = null;
          m_lastKnownXtwifiFix_channel_array = null;
        }
      }
      else if (msg_from.equals("XTWWAN-PE"))
      {
        m_got_xtwwan_response = true;
        try
        {
          if (m_internalState == InternalState.IS_TRACKING_WAIT_FOR_ZPP_FIX)
          {
            double lat_deg = in_card.getDouble("LATITUDE_DEG");
            double lon_deg = in_card.getDouble("LONGITUDE_DEG");
            float hor_unc_meter = in_card.getFloat("HOR_UNC_M");

            m_timestampLastZppReport = System.currentTimeMillis();
            m_is_xtwwan_fix_with_lat_lon = true;

            // cache wwan fix so the code will be consistent
            // between ZPP and GTP AP WWAN
            m_lastKnownWwanFix.reset();
            m_lastKnownWwanFix.setProvider(LocationManager.NETWORK_PROVIDER);
            m_lastKnownWwanFix.setLatitude(lat_deg);
            m_lastKnownWwanFix.setLongitude(lon_deg);
            m_lastKnownWwanFix.setTime(m_timestampLastZppReport);
            m_lastKnownWwanFix.setAccuracy(hor_unc_meter);
            llog.d(TAG, "got XTWWAN fix hepe: " + hor_unc_meter +
                        "(m), time: " + m_timestampLastZppReport + "(ms)" );
            // JB MR1 change
            m_lastKnownZppFix.setElapsedRealtimeNanos(SystemClock.elapsedRealtimeNanos());
          }
          else
            llog.d(TAG, "got XTWWAN fix in wrong state " + m_internalState);
        }
        catch (NoSuchElementException e)
        {
          llog.d(TAG, "Exception in getting lat/long/accurcy for XTWWAN fix");
        }

        if (m_is_xtwwan_fix_with_lat_lon)
        {
          // vertical_unc and altitude may not be received on every fix
          try
          {
            float vert_unc_meter = in_card.getFloat("VER_UNC_M");
            float altitude_wrt_ellipsoid = in_card.getFloat("ALT_ELP_M");

            Bundle bundle = new Bundle();
            bundle.putFloat(BUNDLE_VERTICAL_UNCERTAINITY, vert_unc_meter);
            bundle.putFloat(BUNDLE_ALTITUDE_WRT_ELLIPSOID, altitude_wrt_ellipsoid);
            m_lastKnownZppFix.setExtras(bundle);
          }
          catch (NoSuchElementException e)
          {
            llog.d(TAG, "Exception in altitude/ vertical uncertainity for XTWWAN fix");
          }
        }
      }

      // check if we already have ZPP fix to validate against
      if (m_internalState == InternalState.IS_TRACKING_WAIT_FOR_ZPP_FIX)
      {
        if (!m_got_xtwwan_response)
        {
          llog.d(TAG, "handleNewIpcMessage Wait for XTWWAN / Zpp fix to come");
        }
        else
        {
          // handleZppFix does the same handling for XTWWAN as well.
          handleZppFix();
        }
      }
      else
      {
        // we must be in IS_TRACKING_WAIT_FOR_FINAL_FIX or IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX state
        handleFixReportFromXtwifi();

        // clean up xtwifi transaction after we get the final fix
        if ((m_internalState == InternalState.IS_TRACKING_WAIT_FOR_FINAL_FIX) &&
            (m_xtwifi_tx_state == XtwifiFixState.XFS_FINAL))
        {
          // reset session status
          reInstallTbfTimer();
        }
        else if((m_internalState == InternalState.IS_FREE_WIFI_SCAN_WAIT_FOR_WIFI_FIX) &&
                (m_xtwifi_on_free_scan_tx_state == XtwifiFixState.XFS_FINAL))
        {
          restoreTrackingSessionAfterFreeWiFiScan();
        }
      }
    }
    catch (IOException e)
    {
      llog.w(TAG, "handleNewIpcMessage cannot receive IPC message");
      e.printStackTrace();

      // IPC exception
      m_handler.sendEmptyMessage(MSG_IPC_EXCEPTION);
    }
    catch (NoSuchElementException e)
    {
      llog.w(TAG, "handleNewIpcMessage missing information in IPC message");
      e.printStackTrace();
    }
    catch (Exception e)
    {
      llog.w(TAG, "handleNewIpcMessage: "+ e.getMessage());
    }
  }

  // Call this function to send appropriate NLP intent
  // action - action of intent
  // status - status of intent
  private void broadcastNLPIntent(String action, boolean status)
  {
    Intent intent = new Intent();
    intent.setAction(action);
    intent.putExtra(EXTRA_NLP_ENABLED, status);
    sendBroadcast(intent);
  }

  private void registerForListenersAndMiscSetup ()
  {
    llog.w(TAG, "registerForListnersAndMisc enter");

    try
    {
      if(m_isEnabledProcessFreeWiFiScan)
      {
        llog.v(TAG, "Installing wifi scan result receiver");

        // Register for WiFi scan result updates
        registerReceiver(m_WiFiScanResultReceiver,
                         new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION),
                         null, m_handler);
        mReceiverRegistered = true;
      }
      else
      {
        llog.v(TAG, "Skip installing wifi scan result receiver");
      }

      m_alarmManager = (AlarmManager) getSystemService(ALARM_SERVICE);

      this.registerReceiver(m_AlarmReceiver_TBFTimeout, new IntentFilter("TBF_TIMEOUT"),
                            null, m_handler);
      m_PendingIntentTBFTimeout = PendingIntent.getBroadcast(this, 0, new Intent("TBF_TIMEOUT"), 0);

      m_powerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
      m_wakeLock = m_powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
    }
    catch (Exception e)
    {
      e.printStackTrace ();
    }

    llog.w(TAG, "registerForListnersAndMisc exit");
  }

  private void unregisterForListenersAndMiscCleanup ()
  {
    llog.w(TAG, "unregisterForListenersAndMiscCleanup");

    try
    {
      // Need special try-catch to work around Android bug:
      // java.lang.IllegalArgumentException: Receiver not registered
      try
      {
        unregisterReceiver(m_AlarmReceiver_TBFTimeout);
      }
      catch (Exception e)
      {
        llog.w (TAG, e.getMessage());
      }

      if(m_wakeLock.isHeld())
      {
        m_wakeLock.release();
        llog.v(TAG, "Wakelock release");
      }

      if(m_isFixSessionGoingOn)
      {
        m_isFixSessionGoingOn = false;
        broadcastNLPIntent(NLP_ENABLED_CHANGE_ACTION, m_isFixSessionGoingOn);
      }
    }
    catch (Exception e)
    {
      e.printStackTrace ();
    }

    llog.w(TAG, "unregisterForListenersAndMiscCleanup exit");
  }

  private void readConfigurationItems ()
  {
    FileInputStream inFileStream = null;

    try
    {
      Properties config = new Properties();
      inFileStream = new FileInputStream(CONFIG_FILE);
      config.load(inFileStream);

      // try to load from the config file, using the declared default value as
      // the default here
      m_log_level = Integer.decode(config.getProperty("DEBUG_GLOBAL_LOG_LEVEL", Integer.toString(m_log_level)).trim());
      llog.setLevel(m_log_level);

      m_isZppEnabled = Integer.decode(config.getProperty("ENABLE_NLP_ZPP_ADAPTOR", (m_isZppEnabled ? "1" : "0")).trim()) == 1;
      m_minTimeBetweenFix = Long.parseLong(config.getProperty("TIME_MSEC_NLP_MIN_TIME_BETWEEN_FIX",
                                                              Long.toString(m_minTimeBetweenFix)).trim());
      m_timeoutZppFix = Long.parseLong(config.getProperty("TIMEOUT_MSEC_NLP_ZPP_FIX", Long.toString(m_timeoutZppFix)).trim());
      //If XT WWAN (AP based) is enabled, then use longer timeout value
      if (m_isXTWWANEnabled == true)
      {
        m_timeoutWWanFix = Long.parseLong(config.getProperty("TIMEOUT_MSEC_NLP_WWAN_FIX", Long.toString(m_timeoutWWanFix)).trim());
        m_timeoutZppFix = m_timeoutWWanFix;
      }

      m_timeoutFinalFix = Long.parseLong(config.getProperty("TIMEOUT_MSEC_NLP_XTWIFI_FINAL_FIX",
                                                            Long.toString(m_timeoutFinalFix)).trim());
      m_maxAgeForZppTransaction = Long.parseLong(config.getProperty("TIME_MSEC_NLP_MAX_AGE_FOR_ZPP_TRANSACTION",
                                                                    Long.toString(m_maxAgeForZppTransaction)).trim());

      m_isEnabledProcessFreeWiFiScan = Integer.decode(
          config.getProperty("ENABLE_NLP_PROCESS_FREE_WIFI_SCAN",
                             (m_isEnabledProcessFreeWiFiScan ? "1" : "0")).trim()) == 1;
      m_minTimeBetweenFixOnFreeWiFiScan = Long.parseLong(
          config.getProperty("TIME_MSEC_NLP_MIN_TIME_BETWEEN_FIX_REQ_ON_FREE_WIFI",
                             Long.toString(m_minTimeBetweenFixOnFreeWiFiScan)).trim());
      m_minTimeBetweenWiFiPositionRequestAndTrackingTimeout = Long.parseLong(
          config.getProperty("TIME_MSEC_MIN_TIME_BETWEEN_WIFI_POSITION_REQ_AND_TRACKING_TIMEOUT",
                             Long.toString(m_minTimeBetweenWiFiPositionRequestAndTrackingTimeout)).trim());
      m_timeoutFreeXtWiFiFix = Long.parseLong(config.getProperty("TIMEOUT_MSEC_FREE_XTWIFI_FIX",
                                                                   Long.toString(m_timeoutFreeXtWiFiFix)).trim());
      m_hepe_margin_meter_report_zpp_location = Float.parseFloat(config.getProperty("HEPE_MARGIN_METER_REPORT_ZPP_LOCATION",
              Float.toString(m_hepe_margin_meter_report_zpp_location)).trim());
      m_large_accuracy_threshold_to_filter_nlp_position = Long.parseLong(config.getProperty("LARGE_ACCURACY_THRESHOLD_TO_FILTER_NLP_POSITION",
                Long.toString(m_large_accuracy_threshold_to_filter_nlp_position)).trim());

      m_time_sec_min_age_reporting_stats_collected = Integer.decode(
          config.getProperty("TIME_SEC_MIN_AGE_REPORTING_STATS_COLLECTED",
                             Integer.toString(m_time_sec_min_age_reporting_stats_collected)).trim());

      llog.w(TAG,"ENABLE_NLP_PROCESS_FREE_WIFI_SCAN: " + m_isEnabledProcessFreeWiFiScan +
               ", HEPE_MARGIN_METER_REPORT_ZPP_LOCATION: " + m_hepe_margin_meter_report_zpp_location);
    }
    catch (FileNotFoundException e)
    {
      e.printStackTrace();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
    catch (Exception e)
    {
      e.printStackTrace();
    }
    finally
    {
      try
      {
        if (inFileStream != null)
        {
          inFileStream.close();
        }
      }
      catch (Exception e)
      {
        e.printStackTrace();
      }
    }
  }


  private final class NetworkLocationProviderThread extends Thread
  {
    @Override
    public void run()
    {
      try
      {
        super.run();

        llog.w (TAG, "NetworkLocationProviderThread enter: " + this.toString());

        Looper.prepare();

        m_handler = new Handler()
        {
          @Override
          public void handleMessage(Message msg)
          {
            try
            {
              boolean isOnDestroyCalled;
              synchronized (m_lock)
              {
                isOnDestroyCalled = m_isOnDestroyCalled;
              }

              if (isOnDestroyCalled == false)
              {
                super.handleMessage(msg);
                int message = msg.what;
                llog.w (TAG, "handleMessage: " + message);
                switch (message)
                {
                case MSG_ENABLE:
                  if (msg.arg1 == 1)
                  {
                    handleEnableNlp();
                  }
                  else
                  {
                    handleDisableNlp();
                  }
                  break;
                case MSG_ENABLE_TRACKING:
                  handleEnableLocationTracking(msg.arg1 == 1);
                  break;
                case MSG_TRY_CONNECT:
                  handleTryConnect();
                  break;
                case MSG_IPC_EXCEPTION:
                  llog.w(TAG, "IPC receiver thread ended, remove local client");
                  if (m_ipcClient != null)
                  {
                    // first disconnect, and then remove reference to this connection
                    // it will be re-allocated and retry counter will be reset in
                    // handler of MSG_TRY_CONNECT
                    m_ipcClient.disconnect();
                    m_ipcClient = null;
                  }
                  // remove redundant trails
                  removeMessages(MSG_IPC_EXCEPTION);
                  removeMessages(MSG_TRY_CONNECT);
                  // retry connection
                  sendEmptyMessage(MSG_TRY_CONNECT);
                  break;
                case MSG_NEW_IPC_MESSAGE:
                  handleNewIpcMessage((InPostcard) msg.obj);
                  break;
                case MSG_SET_MIN_TIME:
                  m_currentTimeBetweenFix = ((Long) msg.obj).longValue();
                  if (m_currentTimeBetweenFix < m_minTimeBetweenFix)
                  {
                    m_currentTimeBetweenFix = m_minTimeBetweenFix;
                  }
                  llog.v(TAG, "handleMessage TBF set to " + m_currentTimeBetweenFix);

                  // reset TBF timer to the new settings, so it takes effect right away
                  if(m_isTracking)
                  {
                    // note everything else has been cleared/reset in reInstallTbfTimer
                    m_alarmManager.cancel(m_PendingIntentTBFTimeout);
                    m_alarmManager.setExact(AlarmManager.ELAPSED_REALTIME,
                                            SystemClock.elapsedRealtime() + m_currentTimeBetweenFix,
                                            m_PendingIntentTBFTimeout);
                    llog.v(TAG, "alarm set time of next fix(msec): " +
                           (SystemClock.elapsedRealtime() + m_currentTimeBetweenFix));
                  }
                  break;
                case MSG_ZPP_FINISHED:
                  // we got notified that ZPP result has arrived
                  m_timestampLastZppReport = System.currentTimeMillis();
                  handleZppFix();
                  break;
                case MSG_WIFI_SCAN_RESULT_AVAILABLE:
                  handleFreeWiFiScanResultForPositioning();
                  break;
                default:
                  break;
                }
              }
              else
              {
                llog.w (TAG, "isOnDestroyCalled: " + isOnDestroyCalled +
                          ", m_handler: " + m_handler.toString());
                // remove all messages
                m_handler.removeCallbacksAndMessages (null);
                m_handler.getLooper().quitSafely();
                llog.w (TAG, "Looper quit due to onDestroy called");
              }
            }
            catch(Exception e)
            {
              llog.e(TAG, "exception in message handler");
              e.printStackTrace();
            }
          }
        };

        // now the handler has been constructed, let's try to connect to
        // location-mq
        m_handler.sendEmptyMessage(MSG_TRY_CONNECT);
        Looper.loop();
      }
      catch (Exception e)
      {
        e.printStackTrace();
      }

      try
      {
        // Cleanup ZPP: including native resource and ZPP thread
        if (m_zppAdaptor != null)
        {
          m_zppAdaptor.cleanup();
          m_zppAdaptor = null;
        }

        // Close the socket with location mq
        if (m_ipcClient != null)
        {
          m_ipcClient.disconnect();
          m_ipcClient = null;
        }
      }
      catch (Exception e)
      {
        e.printStackTrace();
      }

      llog.w (TAG, "NetworkLocationProviderThread exit: " + this.toString());
    }
  }
}
