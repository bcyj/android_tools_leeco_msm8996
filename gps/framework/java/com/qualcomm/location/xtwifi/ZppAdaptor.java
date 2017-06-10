/*
 * ====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *
 * XTRA-T WiFi Replacement Network Location Provider
 *
 * GENERAL DESCRIPTION This is implementation of adaptor to Zero Power Position.
 *
 * Copyright (c) 2012-2014 Qualcomm Atheros, Inc. All Rights Reserved. Qualcomm
 * Atheros Confidential and Proprietary.
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * =============================================================================
 */
package com.qualcomm.location.xtwifi;

import java.util.concurrent.CountDownLatch;

import android.location.Location;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import com.qualcomm.lib.location.log.*;

public class ZppAdaptor
{
  static
  {
    System.loadLibrary("xtwifi_zpp_adaptor");
  }

  private final LocLog llog = new LocLog(LocLog.LOG_VERBOSE_LEVEL_INFO);

  public static final String TECH_SOURCE_BITMASK = "TECH_SOURCE_BITMASK";

  public static final int TECH_SOURCE_SATELLITE = 0x1;
  public static final int TECH_SOURCE_CELLID = 0x2;
  public static final int TECH_SOURCE_WIFI = 0x4;
  public static final int TECH_SOURCE_SENSORS = 0x8;
  public static final int TECH_SOURCE_REF_LOCATION = 0x10;
  public static final int TECH_SOURCE_INJECTED_COARSE_POSITION = 0x20;

  private static final String TAG = "XTWiFiZpp";

  private static final int MSG_GET_ZPP = 1;
  private static final int MSG_INJECT_FIX = 2;
  private static final int MSG_LOOPER_EXIT_REQUEST = 3;

  // Inject sources as defined in qmiLocInjectPositionReqMsgT_v02
  private static final int INJECT_SRC_CELLID = 1;
  private static final int INJECT_SRC_WIFI = 3;

  private Handler m_handler;
  private Handler m_handlerNlp;
  private int m_msgNlpZppFinished;

  private Thread m_handlerThread;

  // BEGIN: shared among threads, access should be guarded by mutex
  private Object m_mutex = new Object();
  private boolean m_isCleanupCalled = false;
  private boolean m_is_zpp_running = false;
  private boolean m_was_last_zpp_successful = false;
  private boolean m_was_last_zpp_fix_valid = false;
  private Location m_lastKnownLocation = new Location(LocationManager.NETWORK_PROVIDER);
  private Bundle m_locationExtra = new Bundle();
  // END: shared among threads, access should be guarded by mutex

  private int m_log_level;
  private boolean is_zpp_connected = false;

  private native int init(int log_level);

  private native int connect();

  private native int getZppFix(int tx_id);

  private native int sendWiFiFix(boolean is_pos_valid, double lat_deg, double lon_deg, float unc_m,
                                 int mac_array[], int rssi_array [], int channel_array[]);

  private void _tryConnectZpp()
  {
    if (!is_zpp_connected)
    {
      if (0 == init(m_log_level))
      {
        if (0 == connect())
        {
          is_zpp_connected = true;
          llog.i(TAG, "ZPP adaptor connected");
        }
      }
      if (!is_zpp_connected)
      {
        llog.e(TAG, "ZPP adaptor NOT connected");
      }
    }
    else
    {
      // already connected, skip
    }
  }

  public ZppAdaptor(Handler nlpHandler, int msgZppFinished, int log_level)
  {
    llog.w(TAG, "constructor enter");
    m_log_level = log_level;
    llog.setLevel(m_log_level);

    m_handlerNlp = nlpHandler;
    m_msgNlpZppFinished = msgZppFinished;

    is_zpp_connected = false;

    m_handlerThread = new ZppAdaptorThread();
    m_handlerThread.start();
    llog.w(TAG, "constructor exit");
  }

  public boolean isZppRunning()
  {
    synchronized (m_mutex)
    {
      return m_is_zpp_running;
    }
  }

  private boolean _setZppRunning(boolean running)
  {
    synchronized (m_mutex)
    {
      return m_is_zpp_running = running;
    }
  }

  public boolean triggerZppIfNotAlreadyRunning()
  {
    synchronized (m_mutex)
    {
      if (!m_is_zpp_running)
      {
        return m_handler.sendEmptyMessage(MSG_GET_ZPP);
      }
      else
      {
        llog.d(TAG, "ZPP fix is still running. skip triggering");
        return true;
      }
    }
  }

  public void cleanup()
  {
    llog.w(TAG, "cleanup enter");

    try
    {
      boolean isHandlerNull = false;
      synchronized (m_mutex)
      {
        m_isCleanupCalled = true;
        if (m_handler == null)
        {
          isHandlerNull = true;
        }
      }

      if (isHandlerNull == false)
      {
        m_handler.removeCallbacksAndMessages(null);
        m_handler.sendEmptyMessage(MSG_LOOPER_EXIT_REQUEST);
      }
    }
    catch (Exception e)
    {
      llog.w(TAG, "cleanup: " + e.getMessage());
    }

    llog.w(TAG, "cleanup exit");
  }

  private static final String BUNDLE_LAT_DEG = "BUNDLE_LAT_DEG";
  private static final String BUNDLE_LON_DEG = "BUNDLE_LON_DEG";
  private static final String BUNDLE_UNC_M = "BUNDLE_UNC_M";
  private static final String BUNDLE_AGEOFFIX = "BUNDLE_AGEOFFIX";
  private static final String BUNDLE_IS_POS_VALID = "BUNDLE_IS_POS_VALID";
  private static final String BUNDLE_MAC_ARRAY = "BUNDLE_MAC_ARRAY";
  private static final String BUNDLE_RSSI_ARRAY = "BUNDLE_RSSI_ARRAY";
  private static final String BUNDLE_CHANNEL_ARRAY = "BUNDLE_CHANNEL_ARRAY";
  private static final String BUNDLE_SOURCE_TYPE = "BUNDLE_SOURCE_TYPE";
  private static final String BUNDLE_ALTITUDE_WRT_ELLIPSOID = "BUNDLE_ALTITUDE_WRT_ELLIPSOID";
  private static final String BUNDLE_VERTICAL_UNCERTAINITY = "BUNDLE_VERTICAL_UNCERTAINITY";


// Messages to zpp are already synchronized by the use of ZppAdaptorThread.
// Even if zpp is still running there is no harm in qeueing the next inject message to the ZppAdaptorThread.
// injectFixIfNotAlreadyRunning function was discarding all messages if m_is_zpp_running  was true.
// Want to do away with injectFixIfNotAlreadyRunning and replace it with injectFix function.
public boolean injectFix(int src, boolean is_pos_valid, double lat_deg, double lon_deg, float unc_m,
                         float vertical_uncertainity, long ageOfFix, float altitude_wrt_ellipsoid,
                         int mac_array[], int rssi_array [], int channel_array[])
{
  synchronized (m_mutex)
  {
    Bundle bundle = new Bundle();
    bundle.putInt(BUNDLE_SOURCE_TYPE, src);
    bundle.putBoolean(BUNDLE_IS_POS_VALID, is_pos_valid);
    bundle.putDouble(BUNDLE_LAT_DEG, lat_deg);
    bundle.putDouble(BUNDLE_LON_DEG, lon_deg);
    bundle.putFloat(BUNDLE_UNC_M, unc_m);
    bundle.putFloat(BUNDLE_VERTICAL_UNCERTAINITY, vertical_uncertainity);
    bundle.putLong(BUNDLE_AGEOFFIX, ageOfFix);
    bundle.putFloat(BUNDLE_ALTITUDE_WRT_ELLIPSOID, altitude_wrt_ellipsoid);
    bundle.putIntArray(BUNDLE_MAC_ARRAY, mac_array);
    bundle.putIntArray(BUNDLE_RSSI_ARRAY, rssi_array);
    bundle.putIntArray(BUNDLE_CHANNEL_ARRAY, channel_array);
    Message msg = m_handler.obtainMessage (MSG_INJECT_FIX, 0, 0, bundle);
    return m_handler.sendMessage(msg);
  }
}

  public boolean getLastKnownLocation(Location copy)
  {
    synchronized (m_mutex)
    {
      // duplicate everything, including provider and extras
      copy.set(m_lastKnownLocation);
      return (m_was_last_zpp_successful && m_was_last_zpp_fix_valid);
    }
  }

  public void location_report(boolean is_source_valid, int source_bitmask, boolean is_latitude_valid, double latitude,
      boolean is_longitude_valid, double longitude, boolean is_hor_unc_valid, float hor_unc,
      boolean is_timestamp_valid, long timestamp)
  {
    llog.v(TAG, "location report callback 1: source validity: " + is_source_valid + ", latitude validity: " + is_latitude_valid + ", longitude validity: " + is_longitude_valid + ", unc validity: "
        + is_hor_unc_valid + ", timestamp validity: " + is_timestamp_valid);

    llog.v(TAG, "location report callback 2: source " + source_bitmask + ", (" + latitude + ", " + longitude + "), unc "
        + hor_unc + ", timestamp " + timestamp);

    synchronized (m_mutex)
    {
      // flush everything, including provider and extras
      // latitude and longitude both set to 0
      m_was_last_zpp_fix_valid = false;
      m_lastKnownLocation.reset();
      m_locationExtra.clear();

      m_lastKnownLocation.setProvider(LocationManager.NETWORK_PROVIDER);

      // moving out 'source valid' for current modem doesn't set source properly
      if(is_source_valid)
      {
        m_locationExtra.putInt(TECH_SOURCE_BITMASK, source_bitmask);
      }

      if(!is_timestamp_valid)
      {
        // allow timestamp to be skipped in validity check for now
        timestamp = 0;
      }

      if (is_latitude_valid && is_longitude_valid && is_hor_unc_valid)
      {
        if((latitude == 0) && (longitude == 0))
        {
          // consider this fix invalid
          llog.w(TAG, "ZPP fix contains lat/lon: (0,0), ignore");
        }
        else
        {
          m_lastKnownLocation.setLatitude(latitude);
          m_lastKnownLocation.setLongitude(longitude);
          m_lastKnownLocation.setAccuracy(hor_unc);
          m_lastKnownLocation.setTime(timestamp);
          m_lastKnownLocation.setExtras(m_locationExtra);
          m_was_last_zpp_fix_valid = true;
        }
      }
      else
      {
        llog.v(TAG, "ZPP fix is invalid");
      }
    }
  }

  private final class ZppAdaptorThread extends Thread
  {
    @Override
    public void run()
    {
      try
      {
        super.run();
        Looper.prepare();

        llog.v(TAG, "ZppAdaptorThread enter: " + this.toString());
        m_handler = new Handler()
        {
          @Override
          public void handleMessage(Message msg)
          {
            try
            {
              boolean isCleanupCalled;
              synchronized (m_mutex)
              {
                isCleanupCalled = m_isCleanupCalled;
              }

              if (isCleanupCalled == false)
              {
                super.handleMessage(msg);
                int message = msg.what;
                switch (message)
                {
                case MSG_INJECT_FIX:
                  {
                    boolean is_pos_valid = false;
                    double lat_deg;
                    double lon_deg;
                    float unc_m;
                    float vertical_uncertainity;
                    float altitude_wrt_ellipsoid;
                    long ageOfFix;
                    int srctype;
                    int mac_array [];
                    int rssi_array [];
                    int channel_array [];

                    Bundle bundle = (Bundle)msg.obj;
                    if(bundle != null)
                    {
                      srctype = bundle.getInt(BUNDLE_SOURCE_TYPE);
                      is_pos_valid = bundle.getBoolean(BUNDLE_IS_POS_VALID);
                      lat_deg = bundle.getDouble(BUNDLE_LAT_DEG);
                      lon_deg = bundle.getDouble(BUNDLE_LON_DEG);
                      unc_m = bundle.getFloat(BUNDLE_UNC_M);
                      vertical_uncertainity = bundle.getFloat(BUNDLE_VERTICAL_UNCERTAINITY);
                      mac_array = bundle.getIntArray(BUNDLE_MAC_ARRAY);
                      rssi_array = bundle.getIntArray(BUNDLE_RSSI_ARRAY);
                      channel_array = bundle.getIntArray(BUNDLE_CHANNEL_ARRAY);
                      altitude_wrt_ellipsoid = bundle.getFloat(BUNDLE_ALTITUDE_WRT_ELLIPSOID);
                      ageOfFix = bundle.getLong(BUNDLE_AGEOFFIX);

                      _setZppRunning(true);
                      // it is possible that QMI is not ready at boot up time yet
                      // so let's try to reconnect if we failed earlier
                      llog.v(TAG, "calling injection");
                      _tryConnectZpp();
                      if (srctype == INJECT_SRC_WIFI)
                      {
                        sendWiFiFix(is_pos_valid, lat_deg, lon_deg, unc_m, mac_array, rssi_array, channel_array);
                      }
                      _setZppRunning(false);
                    }
                    else
                    {
                      llog.e(TAG, "Invalid injection parameters");
                    }
                  }
                  break;

                case MSG_GET_ZPP:
                  _setZppRunning(true);
                  // it is possible that QMI is not ready at boot up time yet
                  // so let's try to reconnect if we failed earlier
                  llog.v(TAG, "calling ZPP");
                  _tryConnectZpp();
                  if (0 == getZppFix(1))
                  {
                    m_was_last_zpp_successful = true;
                  }
                  else
                  {
                    m_was_last_zpp_successful = false;
                  }
                  _setZppRunning(false);

                  // inform NLP that ZPP operation has completed
                  m_handlerNlp.sendEmptyMessage(m_msgNlpZppFinished);
                  break;
                }
              }
              else
              {
                llog.w (TAG, "cleanup called on m_handler: " + m_handler.toString());
                // remove all messages
                m_handler.removeCallbacksAndMessages (null);
                m_handler.getLooper().quitSafely();
                llog.w (TAG, "Looper quit due to cleanup called");
              }
            }
            catch (Exception e)
            {
              e.printStackTrace ();
            }
          }
        };
        Looper.loop();
      }
      catch (Exception e)
      {
        e.printStackTrace ();
      }

      llog.v(TAG, "ZppAdaptorThread exit: " + this.toString());
    }
  }
}
