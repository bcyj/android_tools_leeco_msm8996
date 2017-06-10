/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.

  Not a Contribution, Apache license notifications and
  license are retained for attribution purposes only.
=============================================================================*/

/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.qualcomm.location.nlp;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Observable;
import java.util.Observer;
import java.util.Properties;
import java.util.List;

import android.content.ComponentName;
import android.content.ContentQueryMap;
import android.content.ContentResolver;
import android.content.Context;
import android.content.ServiceConnection;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageInfo;
import android.database.Cursor;
import android.location.Criteria;
import android.location.ILocationManager;
import android.location.Location;
import android.location.LocationProvider;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;
import android.os.WorkSource;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;
import android.util.Log;
import android.util.Slog;

import com.android.location.provider.LocationProviderBase;
import com.android.location.provider.ProviderPropertiesUnbundled;
import com.android.location.provider.ProviderRequestUnbundled;
import com.android.internal.location.ILocationProvider;
import com.android.internal.location.ProviderRequest;

public class NlpProxyProvider extends LocationProviderBase {
    private static final String TAG = "NlpProxy";
    private static final boolean DEBUG = Log.isLoggable(TAG, Log.DEBUG);

    private static ProviderPropertiesUnbundled PROPERTIES = ProviderPropertiesUnbundled.create(
            true, false, true, false, false, false, false, Criteria.POWER_LOW,
            Criteria.ACCURACY_COARSE);

    private static final int MSG_ENABLE_PRIMARY = 1;
    private static final int MSG_ENABLE_SECONDARY = 2;
    private static final int MSG_DISABLE_PRIMARY = 3;
    private static final int MSG_DISABLE_SECONDARY  = 4;
    private static final int MSG_SET_REQUEST = 5;
    private static final int MSG_LOCATION_UPDATE = 6;
    private static final int MSG_TOLERANCE_TIMEOUT = 7;
    private static final int MSG_RECONNECT_GNP = 8;
    // any new MSG added needs to also update MSG_LAST define below

    private static final int MSG_FIRST = MSG_ENABLE_PRIMARY;
    private static final int MSG_LAST = MSG_RECONNECT_GNP;

    private static final String GPS_CONFIG_FILE = "/etc/gps.conf";
    private static final String IZAT_CONFIG_FILE = "/etc/izat.conf";

    private static final int NLP_MODE_NONE = 0;
    private static final int NLP_MODE_GNP_ONLY = 1;
    private static final int NLP_MODE_QNP_ONLY = 2;
    private static final int NLP_MODE_COMBO = 3;
    private static final int NLP_MODE_LAST = 4;

    // experimental modes
    private static final int NLP_MODE_COMBO_PARALLEL = 5;
    private static final int NLP_MODE_COMBO_GNP_PRIMARY = 6;
    private static final int NLP_MODE_COMBO_QNP_PRIMARY = 7;

    private static final int NLP_TOLERANCE_TIME_FIRST_DEFAULT = 2000;
    private static final int NLP_TOLERANCE_TIME_AFTER_DEFAULT = 20000;
    private static final int NLP_TOLERANCE_TIME_FIRST_MIN = 100;
    private static final int NLP_TOLERANCE_TIME_AFTER_MIN = 100;
    private static final int NLP_THRESHOLD_DEFAULT = 3;
    private static final int NLP_THRESHOLD_MIN = 1;
    private static final int NLP_ACCURACY_MULTIPLE_DEFAULT = 2;
    private static final int NLP_ACCURACY_MULTIPLE_MIN = 1;
    private static final int NLP_TEST_SHOW_POSITION_SOURCE_DEFAULT = 0;
    private static final String[] GNP_PACKAGE_NAMES =
            {"com.google.android.gms",
             "com.google.android.location"};
    private static final String[] GNP_ACTION_NAMES =
            {"com.android.location.service.v3.NetworkLocationProvider",
             "com.android.location.service.v2.NetworkLocationProvider"};
    private static final String[] QNP_PACKAGE_NAMES =
            {SystemProperties.get("persist.loc.nlp_name", "")};
    private static final String[] QNP_ACTION_NAMES =
            {"com.qualcomm.location.service.v2.NetworkLocationProvider"};
    private static final boolean TARGET_LEAN = "true".equals(SystemProperties.get("ro.lean",""));

    private static final int MAX_GNP_RETRIES = 10;
    public static final String ENH_LOCATION_SERVICES_ENABLED = "enhLocationServices_on";

    private final Context mContext;
    private Object mLock = new Object();

    // all members below synchronized on mLock
    private ILocationProvider mNlpServicePrimary;
    private ILocationProvider mNlpServiceSecondary;
    private boolean mEnableOnConnectedPrimary;
    private boolean mEnableOnConnectedSecondary;
    private boolean mSetRequestOnConnected;
    private ProviderRequestUnbundled mProviderRequestOnConnected;
    private WorkSource mWorkSourceOnConnected;
    private boolean mEnabled;
    private boolean mRequestSet;
    private boolean mBindedPrimary;
    private boolean mBindedSecondary;
    // end of members synchronized on mLock

    private ILocationManager mLocationManagerPrivate;
    private LocationManager mLocationManagerPublic;
    private NlpLocationListener mLocationListener;
    private ContentQueryMap mSecureSettings;

    private int mNlpMode;
    private int mNlpToleranceTimeFirst;
    private int mNlpToleranceTimeAfter;
    private int mNlpThreshold;
    private float mNlpAccuracyMultiple;
    private int mNlpTestShowSource;
    private NlpComboState mNlpComboState;
    private boolean mQnpConsent;

    private class NlpComboState {

        private boolean mSwitched;
        private int mSwitchCounter;
        private Location mLocationGNP;
        private Location mLocationQNP;
        private WorkSource mSource;
        private ProviderRequest mRequest;
        private boolean mAwaitingFirstFix;
        private float mGNPReportedAccuracy;
        private float mQNPReportedAccuracy;
        private boolean mToleranceExpired;
        private int mAdaptiveCounter;
        private boolean mGNPPreferred;

        NlpComboState () {
            mRequest = null;
            mSource = null;
            mAwaitingFirstFix = false;
            mSwitched = false;
            mSwitchCounter = 0;
            mGNPReportedAccuracy = 0;
            mQNPReportedAccuracy = 0;
            mAdaptiveCounter = 0;
            mGNPPreferred = true;
            mToleranceExpired = false;
            mLocationGNP = null;
            mLocationQNP = null;
        }

        public void requestStarted(ProviderRequest request, WorkSource source) {
            mRequest = request;
            mSource = source;
            mSwitched = false;
            mToleranceExpired = false;
            mAdaptiveCounter = 0;
            mGNPReportedAccuracy = 0;
            mQNPReportedAccuracy = 0;

            if (!request.reportLocation) {
                mAwaitingFirstFix = false;
                mSwitchCounter = 0;
                mLocationGNP = null;
                mLocationQNP = null;
            } else {
                mAwaitingFirstFix = true;
            }
        }

        public Location getBestLocation() {
            Location bestLocation = null;

            if (hasAllLocations()) {
                if (mLocationQNP.getAccuracy() <= mLocationGNP.getAccuracy()) {
                    if (DEBUG) {
                        Log.d(TAG, "Best Location from QNP where accuracy "
                              + mLocationQNP.getAccuracy() + " is better than "
                              + mLocationGNP.getAccuracy());
                    }
                    bestLocation = mLocationQNP;
                } else {
                    if (DEBUG) {
                        Log.d(TAG, "Best Location from GNP where accuracy "
                            + mLocationGNP.getAccuracy() + " is better than "
                            + mLocationQNP.getAccuracy());
                    }
                    bestLocation = mLocationGNP;
                }
            } else if (hasQNPLocation()) {
                bestLocation = mLocationQNP;
                if (DEBUG) Log.d(TAG, "Best Location from QNP "+
                                      "because no location from GNP");
            } else if (hasGNPLocation()) {
                bestLocation = mLocationGNP;
                if (DEBUG) Log.d(TAG, "Best Location from GNP "+
                                      "because no location from QNP");
            }

            return bestLocation;
        }

        public boolean hasGNPReported() { return (mGNPReportedAccuracy > 0); }
        public boolean hasQNPReported() { return (mQNPReportedAccuracy > 0); }
        public boolean isGNPPreferred() { return mGNPPreferred; }
        public void setGNPPreferred(boolean value) {mGNPPreferred = value; mAdaptiveCounter = 0;}
        public boolean hasToleranceExpired() { return mToleranceExpired; }
        public void setToleranceExpired(boolean value) { mToleranceExpired = value; }
        public void intervalStarted() { mGNPReportedAccuracy = 0;
                                        mQNPReportedAccuracy = 0;
                                        mToleranceExpired = false;}
        public void GNPReported(float accuracy) { mGNPReportedAccuracy = accuracy; }
        public void QNPReported(float accuracy) { mQNPReportedAccuracy = accuracy; }
        public Location getGNPLocation() { return mLocationGNP; }
        public Location getQNPLocation() { return mLocationQNP; }
        public float getGNPReportedAccuracy() { return mGNPReportedAccuracy;}
        public float getQNPReportedAccuracy() { return mQNPReportedAccuracy;}
        public void incrementAdaptiveCounter() {
            mAdaptiveCounter++;
            if (mAdaptiveCounter > mNlpThreshold) {
                mAdaptiveCounter = mNlpThreshold;
            }

            if (DEBUG) Log.d(TAG, "+Adaptive Counter " + mAdaptiveCounter);
        }
        public void decrementAdaptiveCounter() {
            mAdaptiveCounter--;
            if (mAdaptiveCounter <= -mNlpThreshold) {
                mGNPPreferred = !mGNPPreferred;
                mAdaptiveCounter = 0;
                if (mGNPPreferred) {
                    if (DEBUG) Log.d(TAG, "Preferred Changed to GNP");
                } else {
                    if (DEBUG) Log.d(TAG, "Preferred Changed to QNP");
                }
            }

            if (DEBUG) Log.d(TAG, "-Adaptive Counter " + mAdaptiveCounter);
        }
        public void dump() {
            if (DEBUG) {
                Log.d(TAG, "GNPPreferred=" + mGNPPreferred +
                           " GNPReportedAccuracy=" + mGNPReportedAccuracy +
                           " QNPReportedAccuracy=" + mQNPReportedAccuracy +
                           " AdaptiveCounter=" + mAdaptiveCounter +
                           " ToleranceExpired=" + mToleranceExpired);
            }
        }

        public void switched() { mSwitched = !mSwitched;
                                 mSwitchCounter++; }
        public void incrementSwitchCounter() { mSwitchCounter++; }
        public boolean isSwitched() { return mSwitched; }
        public int getSwitchCounter() { return mSwitchCounter+1; }

        public ProviderRequest getRequest() { return mRequest; }
        public WorkSource getSource() { return mSource; }
        public long getInterval() { return mRequest.interval; }
        public boolean isAwaitingFirstFix() { return (mAwaitingFirstFix); }
        public boolean hasGNPLocation() { return (mLocationGNP != null); }
        public boolean hasQNPLocation() { return (mLocationQNP != null); }
        public boolean hasAllLocations() { return (mLocationGNP != null
                                           && mLocationQNP != null); }
        public void saveGNPLocation(Location location) { mLocationGNP = location; }
        public void saveQNPLocation(Location location) { mLocationQNP = location; }
        public void locationUpdated() { mAwaitingFirstFix = false;
                                        mSwitchCounter = 0; }
        public void locationReported() { mLocationGNP = null;
                                         mLocationQNP = null;
                                         mAwaitingFirstFix = false; }

    }

    private static class RequestWrapper {
        public ProviderRequestUnbundled request;
        public WorkSource source;
        public RequestWrapper(ProviderRequestUnbundled request, WorkSource source) {
            this.request = request;
            this.source = source;
        }
    }

    public NlpProxyProvider(Context context) {
        super(TAG, PROPERTIES);
        if (DEBUG) Log.d(TAG, "Constructor");
        mContext = context;
        mNlpComboState = new NlpComboState();
        synchronized(mLock) {
            mEnableOnConnectedPrimary = false;
            mEnableOnConnectedSecondary = false;
            mSetRequestOnConnected = false;
            mBindedPrimary = false;
            mBindedSecondary = false;
            mEnabled = false;
            mRequestSet = false;
            bind();
        }
    }

    protected void cleanUp() {
        if (DEBUG) Log.d(TAG, "cleanUp");

        synchronized(mLock) {
            if (mBindedPrimary) {
                mContext.unbindService(mNlpServiceConnectionPrimary);
                mBindedPrimary = false;
                mNlpServicePrimary = null;
            }
            if (mBindedSecondary) {
                mContext.unbindService(mNlpServiceConnectionSecondary);
                mBindedSecondary = false;
                mNlpServiceSecondary = null;
            }
        }
        // removes any pending messages not handled yet
        for (int i = MSG_FIRST; i <= MSG_LAST; i++) {
            mHandler.removeMessages(i);
        }
    }

    private void readConfigSettings() {
        Properties p = new Properties();
        mNlpMode = NLP_MODE_NONE;
        mNlpToleranceTimeFirst = NLP_TOLERANCE_TIME_FIRST_DEFAULT;
        mNlpToleranceTimeAfter = NLP_TOLERANCE_TIME_AFTER_DEFAULT;
        mNlpThreshold = NLP_THRESHOLD_DEFAULT;
        mNlpTestShowSource = NLP_TEST_SHOW_POSITION_SOURCE_DEFAULT;
        mNlpAccuracyMultiple = NLP_ACCURACY_MULTIPLE_DEFAULT;

        try {
            File file = new File(IZAT_CONFIG_FILE);
            FileInputStream stream = new FileInputStream(file);
            p.load(stream);
            stream.close();

        } catch (IOException e) {
            Log.w(TAG, "Could not open IZAT configuration file " + IZAT_CONFIG_FILE);
        }

        String nlpModeString = p.getProperty("NLP_MODE");
        if (nlpModeString != null) {
            try {
                mNlpMode = Integer.parseInt(nlpModeString.trim());
            } catch (NumberFormatException e) {
                Log.w(TAG, "unable to parse NLP Mode: " + nlpModeString);
            }
            if (mNlpMode >= NLP_MODE_LAST || mNlpMode < NLP_MODE_NONE) {
                mNlpMode = NLP_MODE_NONE;
            }
        }

        String toleranceTimeFirstString = p.getProperty("NLP_TOLERANCE_TIME_FIRST");
        if (toleranceTimeFirstString != null) {
            try {
                mNlpToleranceTimeFirst =
                    Integer.parseInt(toleranceTimeFirstString.trim());
            } catch (NumberFormatException e) {
                Log.w(TAG, "unable to parse NLP Tolerance Time First: "
                      + toleranceTimeFirstString);
            }
            if (mNlpToleranceTimeFirst < NLP_TOLERANCE_TIME_FIRST_MIN) {
                mNlpToleranceTimeFirst = NLP_TOLERANCE_TIME_FIRST_MIN;
            }
        }

        String toleranceTimeAfterString = p.getProperty("NLP_TOLERANCE_TIME_AFTER");
        if (toleranceTimeAfterString != null) {
            try {
                mNlpToleranceTimeAfter =
                    Integer.parseInt(toleranceTimeAfterString.trim());
            } catch (NumberFormatException e) {
                Log.w(TAG, "unable to parse NLP Tolerance Time After: "
                      + toleranceTimeAfterString);
            }
            if (mNlpToleranceTimeAfter < NLP_TOLERANCE_TIME_AFTER_MIN) {
                mNlpToleranceTimeAfter = NLP_TOLERANCE_TIME_AFTER_MIN;
            }
        }

        String thresholdString = p.getProperty("NLP_THRESHOLD");
        if (thresholdString != null) {
            try {
                mNlpThreshold =
                    Integer.parseInt(thresholdString.trim());
            } catch (NumberFormatException e) {
                Log.w(TAG, "unable to parse NLP Threshold: "
                      + thresholdString);
            }
            if (mNlpThreshold < NLP_THRESHOLD_MIN) {
                mNlpThreshold = NLP_THRESHOLD_MIN;
            }
        }

        String accuracyMultipleString = p.getProperty("NLP_ACCURACY_MULTIPLE");
        if (accuracyMultipleString != null) {
            try {
                mNlpAccuracyMultiple =
                    Float.parseFloat(accuracyMultipleString.trim());
            } catch (NumberFormatException e) {
                Log.w(TAG, "unable to parse NLP Accuracy Multiple: "
                      + accuracyMultipleString);
            }
            if (mNlpAccuracyMultiple < NLP_ACCURACY_MULTIPLE_MIN) {
                mNlpAccuracyMultiple = NLP_ACCURACY_MULTIPLE_MIN;
            }
        }

        String showSourceString = p.getProperty("NLP_TEST_SHOW_POSITION_SOURCE");
        if (showSourceString != null) {
            try {
                mNlpTestShowSource =
                    Integer.parseInt(showSourceString.trim());
            } catch (NumberFormatException e) {
                Log.w(TAG, "unable to parse NLP Test Show Position Source: "
                      + showSourceString);
            }
            if (mNlpTestShowSource < 0 || mNlpTestShowSource > 1) {
                mNlpTestShowSource = NLP_TEST_SHOW_POSITION_SOURCE_DEFAULT;
            }
        }

        //find packages that are installed
        boolean gnpExists = false;
        boolean qnpExists = false;
        List<ApplicationInfo> packages;
        PackageManager pm = mContext.getPackageManager();
        packages = pm.getInstalledApplications(0);
        for (ApplicationInfo packageInfo : packages) {
            if (!gnpExists) {
                for (String packageName : GNP_PACKAGE_NAMES) {
                    if (packageInfo.packageName.equals(packageName)) {
                        gnpExists = true;
                        break;
                    }
                }
            }
            if (!qnpExists) {
                for (String packageName : QNP_PACKAGE_NAMES) {
                    if (packageInfo.packageName.equals(packageName)) {
                        qnpExists = true;
                        break;
                    }
                }
            }
            if (gnpExists && qnpExists) {
                break;
            }
        }

        // modify nlp selection based on what packages exist
        if (mNlpMode == NLP_MODE_NONE) {
            if (qnpExists && gnpExists) {
                mNlpMode = NLP_MODE_COMBO;
            } else if (qnpExists) {
                mNlpMode = NLP_MODE_QNP_ONLY;
            } else if (gnpExists) {
                mNlpMode = NLP_MODE_GNP_ONLY;
            }
        } else if (mNlpMode == NLP_MODE_GNP_ONLY) {
            if (!gnpExists && qnpExists) {
                mNlpMode = NLP_MODE_QNP_ONLY;
            } else if (!gnpExists) {
                mNlpMode = NLP_MODE_NONE;
            }
        } else if (mNlpMode == NLP_MODE_QNP_ONLY) {
            if (!qnpExists && gnpExists) {
                mNlpMode = NLP_MODE_GNP_ONLY;
            } else if (!qnpExists) {
                mNlpMode = NLP_MODE_NONE;
            }
        } else if (mNlpMode >= NLP_MODE_COMBO) {
            if (qnpExists && !gnpExists) {
                mNlpMode = NLP_MODE_QNP_ONLY;
            } else if (!qnpExists && gnpExists) {
                mNlpMode = NLP_MODE_GNP_ONLY;
            } else if (!qnpExists && !gnpExists) {
                mNlpMode = NLP_MODE_NONE;
            }
        }

        // QNP is not be supported on lean targets
        if (TARGET_LEAN) {
            if (mNlpMode == NLP_MODE_QNP_ONLY) {
                mNlpMode = NLP_MODE_NONE;
            } else if (mNlpMode >= NLP_MODE_COMBO) {
                mNlpMode = NLP_MODE_GNP_ONLY;
            }
        }

        if (DEBUG) Log.d(TAG, "NLP mode = " + mNlpMode +
                              " QNP exists = " + qnpExists +
                              " GNP exists = " + gnpExists +
                              " Tolerance first = " + mNlpToleranceTimeFirst +
                              " Tolerance after = " + mNlpToleranceTimeAfter +
                              " Threshold = " + mNlpThreshold +
                              " Accuracy multiple = " + mNlpAccuracyMultiple +
                              " Target Lean = " + TARGET_LEAN);


    }

    private void bind() {

        readConfigSettings();

        switch (mNlpMode) {
            case NLP_MODE_GNP_ONLY:
                mBindedPrimary = bindPackage(GNP_PACKAGE_NAMES, GNP_ACTION_NAMES,
                                             mNlpServiceConnectionPrimary);
                break;
            case NLP_MODE_QNP_ONLY:
                mBindedPrimary = bindPackage(QNP_PACKAGE_NAMES, QNP_ACTION_NAMES,
                                             mNlpServiceConnectionPrimary);
                break;
            case NLP_MODE_COMBO:
            case NLP_MODE_COMBO_PARALLEL:
            case NLP_MODE_COMBO_GNP_PRIMARY:
                mBindedPrimary = bindPackage(GNP_PACKAGE_NAMES, GNP_ACTION_NAMES,
                                             mNlpServiceConnectionPrimary);
                mBindedSecondary = bindPackage(QNP_PACKAGE_NAMES, QNP_ACTION_NAMES,
                                               mNlpServiceConnectionSecondary);
                break;
            case NLP_MODE_COMBO_QNP_PRIMARY:
                mBindedPrimary = bindPackage(QNP_PACKAGE_NAMES, QNP_ACTION_NAMES,
                                             mNlpServiceConnectionPrimary);
                mBindedSecondary = bindPackage(GNP_PACKAGE_NAMES, GNP_ACTION_NAMES,
                                               mNlpServiceConnectionSecondary);
                break;
            default:
                Log.e(TAG, "No NLPs exist to bind to!");
                break;
        }
    }

    private boolean bindPackage(String[] packageNames, String[] actionNames,
                                ServiceConnection serviceConnection) {
        boolean success = false;
        for (int i = 0; i < packageNames.length && i < actionNames.length; i++) {
            Intent intent = new Intent(actionNames[i]);
            intent.setPackage(packageNames[i]);
            if (DEBUG) Log.d(TAG, "binding " + packageNames[i] +
                    " with action " + actionNames[i]);
            success = mContext.bindServiceAsUser(intent, serviceConnection,
                                                 Context.BIND_AUTO_CREATE |
                                                 Context.BIND_NOT_FOREGROUND |
                                                 Context.BIND_ALLOW_OOM_MANAGEMENT |
                                                 Context.BIND_NOT_VISIBLE, UserHandle.OWNER);
            if (success) {
                if (DEBUG) Log.d(TAG, "bind success");
                break;
            } else {
                if (DEBUG) Log.d(TAG, "bind failed");
            }
        }

        if (!success) Log.e(TAG, "Failed to bind to a package");

        return success;
    }

    private boolean LocationNeedsScreening(Location location) {
        Bundle extras = location.getExtras();
        if (extras != null) {
            if (extras.containsKey("com.qualcomm.location.nlp:screen")) {
                return true;
            }
        }
        return false;
    }

    private Location ClearScreeningMarker(Location location) {
        Bundle extras = location.getExtras();
        if (extras != null) {
            extras.remove("com.qualcomm.location.nlp:screen");
            location.setExtras(extras);
        }
        return location;
    }

    private boolean IsLocationSourceQnp(Location location) {
        Bundle extras = location.getExtras();
        if (extras != null) {
            if (extras.containsKey("com.qualcomm.location.nlp:source-qnp")) {
                return true;
            }
        }
        return false;
    }

    private Location ClearSourceQnpMarker(Location location) {
        Bundle extras = location.getExtras();
        if (extras != null) {
            extras.remove("com.qualcomm.location.nlp:source-qnp");
            location.setExtras(extras);
        }
        return location;
    }

    private Location MarkLocationReady(Location location) {
        Bundle extras = location.getExtras();
        if (extras == null) {
            extras = new Bundle();
        }
        extras.putBoolean("com.qualcomm.location.nlp:ready", true);
        location.setExtras(extras);
        return location;
    }

    private void ReportLocation(Location location) {
        if (location != null) {
            location = MarkLocationReady(location);
            if (DEBUG) Log.d(TAG, "Location ready for broadcast "
                             + location.toString());
            try {
                if (mLocationManagerPrivate != null) {
                    mLocationManagerPrivate.reportLocation(location, false);
                }
            } catch (RemoteException e) {
                Log.w(TAG, e);
            } catch (Exception e) {
                Log.w(TAG, "Exception ", e);
            }
        } else {
            Log.w(TAG, "No location to report");
        }
        mNlpComboState.locationReported();
    }

    private void UpdateQnpConsent() {
        ContentResolver resolver = mContext.getContentResolver();
        boolean newQnpConsent = false;
        String enhancedLocationSevices =
            Settings.Secure.getString(resolver, ENH_LOCATION_SERVICES_ENABLED);
        if (enhancedLocationSevices != null && enhancedLocationSevices.equals("1")) {
            newQnpConsent = true;
        } else {
            newQnpConsent = false;
        }
        if (DEBUG) Log.d(TAG, "QNP Consent: " + mQnpConsent);

        if (newQnpConsent != mQnpConsent) {
            if (DEBUG) Log.d(TAG, "QNP Consent changed from : "
                             + mQnpConsent + " to " + newQnpConsent);
            mQnpConsent = newQnpConsent;

            if (mNlpMode == NLP_MODE_COMBO && !mQnpConsent &&
                !mNlpComboState.isGNPPreferred()) {
                mNlpComboState.setGNPPreferred(true);
            }
        }

    }

    private void EnableNlpCombo() {
        UpdateQnpConsent();

        // listen for settings changes to know if user has consented to QNP
        if (mSecureSettings == null) {
            ContentResolver resolver = mContext.getContentResolver();
            Cursor secureSettingsCursor =
                resolver.query(Settings.Secure.CONTENT_URI,
                               new String[] { Settings.System.NAME, Settings.System.VALUE },
                               "(" + Settings.System.NAME + "=?)",
                               new String[] { ENH_LOCATION_SERVICES_ENABLED },
                               null);
            mSecureSettings = new ContentQueryMap(secureSettingsCursor,
                                                  Settings.System.NAME,
                                                  true,
                                                  mHandler);
            QnpConsentObserver qnpConsentObserver = new QnpConsentObserver();
            mSecureSettings.addObserver(qnpConsentObserver);
        }
    }

    private void DisableNlpCombo() {

        mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);

    }

    private void SetRequest(ILocationProvider provider, ProviderRequest request, WorkSource source) {
        if (DEBUG) Log.d(TAG, "set request: " + request.toString());

        synchronized (mLock) {
            try {
                provider.setRequest(request, source);
            } catch (RemoteException e) {
                Log.w(TAG, e);
            } catch (Exception e) {
                Log.w(TAG, "Exception ", e);
            }
        }
    }

    private ServiceConnection mNlpServiceConnectionPrimary = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            if (DEBUG) Log.d(TAG, "Service Primary Connected");
            synchronized (mLock) {
                mNlpServicePrimary = ILocationProvider.Stub.asInterface(binder);

                if (mEnableOnConnectedPrimary) {
                    if (DEBUG) Log.d(TAG, "Enabling after connected");
                    mHandler.sendEmptyMessage(MSG_ENABLE_PRIMARY);
                    mEnableOnConnectedPrimary = false;
                }

                if (mSetRequestOnConnected && !mEnableOnConnectedSecondary) {
                    if (DEBUG) Log.d(TAG, "SetRequest after connected");
                    mHandler.obtainMessage(MSG_SET_REQUEST,
                            new RequestWrapper(mProviderRequestOnConnected,
                                               mWorkSourceOnConnected)).sendToTarget();
                    mSetRequestOnConnected = false;
                }
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.e(TAG, "Service Primary Disconnected");

            synchronized (mLock) {
                mNlpServicePrimary = null;
                if (mEnabled) {
                    if (DEBUG) Log.d(TAG, "Will enable primary after connected");
                    mEnableOnConnectedPrimary = true;
                    if (mRequestSet) {
                        if (DEBUG) Log.d(TAG, "Will set request after connected");
                        mSetRequestOnConnected = true;
                    }
                }

                if (mNlpMode == NLP_MODE_GNP_ONLY || mNlpMode >= NLP_MODE_COMBO) {
                    Log.w(TAG, "Remote Service GNP will be reconnected");
                    mContext.unbindService(mNlpServiceConnectionPrimary);
                    mBindedPrimary = false;
                    Message message = Message.obtain(mHandler, MSG_RECONNECT_GNP, 1);
                    mHandler.sendMessageDelayed(message, 100);
                } else {
                    Log.e(TAG, "Will wait for Local Service QNP to reconnected");
                }
            }
        }
     };

     private ServiceConnection mNlpServiceConnectionSecondary = new ServiceConnection() {

         @Override
         public void onServiceConnected(ComponentName name, IBinder binder) {
             if (DEBUG) Log.d(TAG, "Service Secondary Connected");
             synchronized (mLock) {
                 mNlpServiceSecondary = ILocationProvider.Stub.asInterface(binder);

                 if (mEnableOnConnectedSecondary) {
                     if (DEBUG) Log.d(TAG, "Enabling Secondary after connected");
                     mHandler.sendEmptyMessage(MSG_ENABLE_SECONDARY);
                     mEnableOnConnectedSecondary = false;
                 }

                 if (mSetRequestOnConnected && !mEnableOnConnectedPrimary) {
                     if (DEBUG) Log.d(TAG, "SetRequest after connected");
                     mHandler.obtainMessage(MSG_SET_REQUEST,
                             new RequestWrapper(mProviderRequestOnConnected,
                                                mWorkSourceOnConnected)).sendToTarget();
                     mSetRequestOnConnected = false;
                 }

             }
         }

         @Override
         public void onServiceDisconnected(ComponentName name) {
             Log.e(TAG, "Service Secondary Disconnected");

            synchronized (mLock) {
                mNlpServiceSecondary = null;
                if (mEnabled) {
                    if (DEBUG) Log.d(TAG, "Will enable secondary after connected");
                    mEnableOnConnectedSecondary = true;
                    if (mRequestSet) {
                        if (DEBUG) Log.d(TAG, "Will set request after connected");
                        mSetRequestOnConnected = true;
                    }
                }
            }

         }
      };

    /**
     * For serializing requests to mNlpService.
     */
    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {

            switch (msg.what) {
                case MSG_ENABLE_PRIMARY:
                {
                    if (DEBUG) Log.d(TAG, "MSG_ENABLE_PRIMARY");
                    synchronized (mLock) {
                        try {
                            mNlpServicePrimary.enable();
                        } catch (RemoteException e) {
                            Log.w(TAG, e);
                        } catch (Exception e) {
                            Log.w(TAG, "Exception ", e);
                        }
                    }

                    if (mLocationManagerPrivate == null) {
                        IBinder binderLocationManager =
                            ServiceManager.getService(Context.LOCATION_SERVICE);
                        mLocationManagerPrivate =
                            ILocationManager.Stub.asInterface(binderLocationManager);
                        if (mLocationManagerPrivate == null) {
                            Log.e(TAG, "Failed to get private LOCATION_SERVICE Instance");
                        }
                    }

                    if (mLocationManagerPublic == null) {
                        mLocationManagerPublic =
                            (LocationManager)mContext.getSystemService(Context.LOCATION_SERVICE);
                        if (mLocationManagerPublic == null) {
                            Log.e(TAG, "Failed to get public LOCATION_SERVICE Instance");
                        }
                    }

                    if (mLocationManagerPublic != null) {
                        if (mLocationListener == null) {
                            mLocationListener = new NlpLocationListener();
                        }
                        try {
                            LocationRequest request =
                                    LocationRequest.createFromDeprecatedProvider(
                                            LocationManager.PASSIVE_PROVIDER,
                                            0, 0, false);
                            request.setHideFromAppOps(true);
                            mLocationManagerPublic.requestLocationUpdates(
                                    request, mLocationListener, mHandler.getLooper());
                        }
                        catch(RuntimeException e) {
                            Log.e(TAG, "Cannot request for passive location updates");
                        }
                    }

                    if (mNlpMode >= NLP_MODE_COMBO) {
                        EnableNlpCombo();
                    }

                    break;
                }
                case MSG_ENABLE_SECONDARY:
                {
                    if (DEBUG) Log.d(TAG, "MSG_ENABLE_SECONDARY");
                    synchronized (mLock) {
                        try {
                            mNlpServiceSecondary.enable();
                        } catch (RemoteException e) {
                            Log.w(TAG, e);
                        } catch (Exception e) {
                            Log.w(TAG, "Exception ", e);
                        }
                    }
                    break;
                }
                case MSG_DISABLE_PRIMARY:
                {
                    if (DEBUG) Log.d(TAG, "MSG_DISABLE_PRIMARY");
                    synchronized (mLock) {
                        try {
                            mNlpServicePrimary.disable();
                        } catch (RemoteException e) {
                            Log.w(TAG, e);
                        } catch (Exception e) {
                            Log.w(TAG, "Exception ", e);
                        }
                    }

                    if (mNlpMode >= NLP_MODE_COMBO) {
                        DisableNlpCombo();
                    }

                    break;
                }
                case MSG_DISABLE_SECONDARY:
                {
                    if (DEBUG) Log.d(TAG, "MSG_DISABLE_SECONDARY");
                    synchronized (mLock) {
                        try {
                            mNlpServiceSecondary.disable();
                        } catch (RemoteException e) {
                            Log.w(TAG, e);
                        } catch (Exception e) {
                            Log.w(TAG, "Exception ", e);
                        }
                    }
                    break;
                }
                case MSG_SET_REQUEST:
                {
                    if (DEBUG) Log.d(TAG, "MSG_SET_REQUEST");

                    RequestWrapper wrapper = (RequestWrapper) msg.obj;
                    ProviderRequest providerRequest = null;
                    try {
                        Field providerRequestField =
                            ProviderRequestUnbundled.class.getDeclaredField("mRequest");
                        providerRequestField.setAccessible(true);
                        providerRequest =
                            (ProviderRequest)providerRequestField.get(wrapper.request);
                    } catch (Exception e) {
                        Log.w(TAG, "Exception ", e);
                    }

                    if (providerRequest == null) {
                        Log.e(TAG, "provider request is null!");
                        break;
                    } else {
                        if (DEBUG) Log.d(TAG, "providerRequest " + providerRequest.toString());
                    }


                    if (mNlpMode == NLP_MODE_COMBO) {
                        SetRequest(mNlpServicePrimary, providerRequest, wrapper.source);
                        SetRequest(mNlpServiceSecondary, providerRequest, wrapper.source);

                        if (DEBUG) Log.d(TAG, "New Tolerance Timer for First Fix "
                                         + mNlpToleranceTimeFirst);
                        mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);
                        mHandler.sendEmptyMessageDelayed(MSG_TOLERANCE_TIMEOUT,
                                                         mNlpToleranceTimeFirst);
                        mNlpComboState.requestStarted(providerRequest, wrapper.source);
                        mNlpComboState.dump();
                    } else if (mNlpMode == NLP_MODE_COMBO_PARALLEL) {
                        SetRequest(mNlpServicePrimary, providerRequest, wrapper.source);
                        SetRequest(mNlpServiceSecondary, providerRequest, wrapper.source);

                        mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);
                        mNlpComboState.requestStarted(providerRequest, wrapper.source);
                    } else if (mNlpMode == NLP_MODE_COMBO_GNP_PRIMARY ||
                               mNlpMode == NLP_MODE_COMBO_QNP_PRIMARY) {
                        if (mNlpComboState.isSwitched()) {
                            ProviderRequest stopRequest = new ProviderRequest();
                            stopRequest.reportLocation = false;
                            SetRequest(mNlpServiceSecondary, stopRequest, wrapper.source);
                        }

                        if (mNlpMode == NLP_MODE_COMBO_QNP_PRIMARY && !mQnpConsent
                            && providerRequest.interval > 0) {
                            // no qnp consent, so immediate switch to gnp
                            SetRequest(mNlpServiceSecondary, providerRequest, wrapper.source);
                            mNlpComboState.switched();
                        } else {
                            SetRequest(mNlpServicePrimary, providerRequest, wrapper.source);
                        }

                        mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);

                        if (providerRequest.interval == 0) {
                            if (DEBUG) Log.d(TAG, "New Interval 0. location reporting stop");
                        } else {
                            if (DEBUG) Log.d(TAG, "New Tolerance Timer " + mNlpToleranceTimeFirst);
                            long time = mNlpToleranceTimeFirst * mNlpComboState.getSwitchCounter();
                            if (time < 0) { //check for overflow
                                time = Long.MAX_VALUE;
                            }
                            mHandler.sendEmptyMessageDelayed(MSG_TOLERANCE_TIMEOUT, time);
                        }

                        mNlpComboState.requestStarted(providerRequest, wrapper.source);
                    } else {
                        SetRequest(mNlpServicePrimary, providerRequest, wrapper.source);
                    }

                    break;
                }
                case MSG_LOCATION_UPDATE:
                {
                    if (DEBUG) Log.d(TAG, "MSG_LOCATION_UPDATE");
                    Location location = (Location)msg.obj;

                     if (mNlpMode < NLP_MODE_COMBO) {
                         if (DEBUG) Log.d(TAG, "Reporting location, as not in Combo Mode");
                          ReportLocation(location);
                     } else if (mNlpMode == NLP_MODE_COMBO) {
                        mNlpComboState.dump();
                        if (IsLocationSourceQnp(location)) {
                            mNlpComboState.QNPReported(location.getAccuracy());
                            if (mNlpTestShowSource != 1) {
                                location = ClearSourceQnpMarker(location);
                            }
                            if (DEBUG) Log.d(TAG, "QNP Reported");
                            if (mNlpComboState.isAwaitingFirstFix()) {
                                if (mNlpComboState.hasGNPReported()) {
                                    float qnpAccuracy = mNlpComboState.getQNPReportedAccuracy();
                                    float gnpAccuracy = mNlpComboState.getGNPReportedAccuracy();
                                    if (qnpAccuracy < gnpAccuracy) {
                                        if (DEBUG) Log.d(TAG, "Reporting QNP location, as " +
                                                         qnpAccuracy + " < " + gnpAccuracy);
                                        ReportLocation(location);
                                        mNlpComboState.setGNPPreferred(false);
                                        if (DEBUG) Log.d(TAG, "QNP Preferred");
                                    } else {
                                        if (DEBUG) Log.d(TAG, "Reporting GNP location as " +
                                                         gnpAccuracy + " <= " + qnpAccuracy);
                                        ReportLocation(mNlpComboState.getGNPLocation());
                                        mNlpComboState.setGNPPreferred(true);
                                        if (DEBUG) Log.d(TAG, "GNP now Preferred");
                                    }
                                } else if (mNlpComboState.hasToleranceExpired()) {
                                    if (DEBUG) {
                                        Log.d(TAG, "Reporting QNP location, as GNP did not " +
                                                "report within " + mNlpToleranceTimeFirst + "ms");
                                    }
                                    ReportLocation(location);
                                    mNlpComboState.setGNPPreferred(false);
                                    if (DEBUG) Log.d(TAG, "QNP now Preferred");
                                } else {
                                    if (DEBUG) Log.d(TAG, "Saving QNP location " +
                                            location.toString());
                                    mNlpComboState.saveQNPLocation(location);
                                    break;
                                }
                            } else if (!mNlpComboState.isGNPPreferred()) {
                                if (DEBUG) Log.d(TAG, "Reporting QNP location");
                                ReportLocation(location);
                                if (mNlpComboState.hasToleranceExpired()) {
                                    mNlpComboState.incrementAdaptiveCounter();
                                }
                            } else {
                                if (DEBUG) Log.d(TAG, "Dropping non-preferred QNP location "
                                                 + location.toString());
                                if (mNlpComboState.hasToleranceExpired()) {
                                    mNlpComboState.decrementAdaptiveCounter();
                                }
                            }
                        } else {
                            mNlpComboState.GNPReported(location.getAccuracy());
                            if (DEBUG) Log.d(TAG, "GNP Reported");
                            if (mNlpComboState.isAwaitingFirstFix()) {
                                if (mNlpComboState.hasQNPReported()) {
                                    float qnpAccuracy = mNlpComboState.getQNPReportedAccuracy();
                                    float gnpAccuracy = mNlpComboState.getGNPReportedAccuracy();
                                    if (gnpAccuracy < qnpAccuracy) {
                                        if (DEBUG) Log.d(TAG, "Reporting GNP location, as " +
                                                         gnpAccuracy + " < " + qnpAccuracy);
                                        ReportLocation(location);
                                        mNlpComboState.setGNPPreferred(true);
                                        if (DEBUG) Log.d(TAG, "GNP Preferred");
                                    } else {
                                        if (DEBUG) Log.d(TAG, "Reporting QNP location as " +
                                                         qnpAccuracy + " <= " + gnpAccuracy);
                                        ReportLocation(mNlpComboState.getQNPLocation());
                                        mNlpComboState.setGNPPreferred(false);
                                        if (DEBUG) Log.d(TAG, "QNP now Preferred");
                                    }
                                } else if (mNlpComboState.hasToleranceExpired()) {
                                    if (DEBUG) {
                                        Log.d(TAG, "Reporting GNP location, as QNP did not " +
                                                "report within " + mNlpToleranceTimeFirst + "ms");
                                    }
                                    ReportLocation(location);
                                    mNlpComboState.setGNPPreferred(true);
                                    if (DEBUG) Log.d(TAG, "GNP now Preferred");
                                } else {
                                    if (DEBUG) Log.d(TAG, "Saving GNP location " +
                                            location.toString());
                                    mNlpComboState.saveGNPLocation(location);
                                    break;
                                }
                            } else if (mNlpComboState.isGNPPreferred()) {
                                if (DEBUG) Log.d(TAG, "Reporting GNP location");
                                ReportLocation(location);
                                if (mNlpComboState.hasToleranceExpired()) {
                                    mNlpComboState.incrementAdaptiveCounter();
                                }
                            } else {
                                if (DEBUG) Log.d(TAG, "Dropping non-preferred GNP location "
                                                 + location.toString());
                                if (mNlpComboState.hasToleranceExpired()) {
                                    mNlpComboState.decrementAdaptiveCounter();
                                }
                            }
                        }
                        if (mNlpComboState.hasGNPReported() && mNlpComboState.hasQNPReported()) {
                            if (mNlpComboState.getGNPReportedAccuracy() >=
                                (mNlpComboState.getQNPReportedAccuracy() * mNlpAccuracyMultiple)) {
                                if (DEBUG) Log.d(TAG, "GNP has much worse Accuracy than QNP (" +
                                        mNlpComboState.getGNPReportedAccuracy() + " vs " +
                                        mNlpComboState.getQNPReportedAccuracy() + ")");
                                if (!mNlpComboState.isGNPPreferred()) {
                                    mNlpComboState.incrementAdaptiveCounter();
                                } else {
                                    mNlpComboState.decrementAdaptiveCounter();
                                }
                            } else if (mNlpComboState.getQNPReportedAccuracy() >=
                                (mNlpComboState.getGNPReportedAccuracy() * mNlpAccuracyMultiple)) {
                                if (DEBUG) Log.d(TAG, "QNP has much worse Accuracy than GNP (" +
                                        mNlpComboState.getQNPReportedAccuracy() + " vs " +
                                        mNlpComboState.getGNPReportedAccuracy() + ")");
                                if (mNlpComboState.isGNPPreferred()) {
                                    mNlpComboState.incrementAdaptiveCounter();
                                } else {
                                    mNlpComboState.decrementAdaptiveCounter();
                                }
                            }
                        }
                        if ((mNlpComboState.hasGNPReported() && mNlpComboState.hasQNPReported())
                            || mNlpComboState.hasToleranceExpired()) {
                            if (DEBUG) Log.d(TAG, "New Tolerance Timer for Interval");
                            mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);
                            mHandler.sendEmptyMessageDelayed(MSG_TOLERANCE_TIMEOUT,
                                    mNlpToleranceTimeAfter + mNlpComboState.getInterval());
                            mNlpComboState.intervalStarted();
                        }
                        mNlpComboState.dump();
                    } else if (mNlpMode == NLP_MODE_COMBO_PARALLEL) {
                        if (IsLocationSourceQnp(location)) {
                            if (mNlpTestShowSource != 1) {
                                location = ClearSourceQnpMarker(location);
                            }
                            if (DEBUG) Log.d(TAG, "Saving QNP location " + location.toString());
                            mNlpComboState.saveQNPLocation(location);
                        } else {
                            if (DEBUG) Log.d(TAG, "Saving GNP location " + location.toString());
                            mNlpComboState.saveGNPLocation(location);
                        }

                        if ((mNlpComboState.hasAllLocations()) ||
                            (mNlpComboState.hasGNPLocation() && mQnpConsent == false)) {
                             ReportLocation(mNlpComboState.getBestLocation());
                             mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);
                        } else if (mNlpComboState.isAwaitingFirstFix()) {
                            if (DEBUG) Log.d(TAG, "New Tolerance Timer for First Fix "
                                             + mNlpToleranceTimeFirst);
                            mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);
                            mHandler.sendEmptyMessageDelayed(MSG_TOLERANCE_TIMEOUT, mNlpToleranceTimeFirst);
                        } else {
                            if (DEBUG) Log.d(TAG, "New Tolerance Timer for After First Fix "
                                             + mNlpToleranceTimeAfter);
                            mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);
                            mHandler.sendEmptyMessageDelayed(MSG_TOLERANCE_TIMEOUT, mNlpToleranceTimeAfter);
                        }
                    } else if (mNlpMode == NLP_MODE_COMBO_GNP_PRIMARY ||
                               mNlpMode == NLP_MODE_COMBO_QNP_PRIMARY) {
                        if (IsLocationSourceQnp(location)) {
                            if (DEBUG) Log.d(TAG, "Got QNP location " + location.toString());
                            if (mNlpTestShowSource != 1) {
                                location = ClearSourceQnpMarker(location);
                            }
                        } else {
                            if (DEBUG) Log.d(TAG, "Got GNP location " + location.toString());
                        }
                        ReportLocation(location);
                        mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);

                        long time = mNlpToleranceTimeAfter + mNlpComboState.getInterval();
                        if (time < 0) { //check for overflow
                            time = Long.MAX_VALUE;
                        }

                        if (DEBUG) Log.d(TAG, "New Tolerance Timer for After First Fix " + time);
                        mHandler.sendEmptyMessageDelayed(MSG_TOLERANCE_TIMEOUT, time);
                    }

                    mNlpComboState.locationUpdated();
                    break;
                }
                case MSG_TOLERANCE_TIMEOUT:
                {
                    if (DEBUG) Log.d(TAG, "MSG_TOLERANCE_TIMEOUT");

                    if (mNlpMode == NLP_MODE_COMBO) {
                        mNlpComboState.dump();
                        if (!mNlpComboState.hasQNPReported() && !mNlpComboState.hasGNPReported()) {
                            mNlpComboState.setToleranceExpired(true);
                        } else {
                            if (mNlpComboState.isAwaitingFirstFix()) {
                                if (mNlpComboState.hasQNPReported()) {
                                    if (DEBUG) Log.d(TAG, "Reporting QNP location, as GNP did " +
                                                     "not report within " + mNlpToleranceTimeFirst);
                                    ReportLocation(mNlpComboState.getQNPLocation());
                                    mNlpComboState.setGNPPreferred(false);
                                    if (DEBUG) Log.d(TAG, "QNP now Preferred");
                                } else if (mNlpComboState.hasGNPReported()) {
                                    if (DEBUG) Log.d(TAG, "Reporting GNP location, as QNP did " +
                                                     "not report within " + mNlpToleranceTimeFirst);
                                    ReportLocation(mNlpComboState.getGNPLocation());
                                    mNlpComboState.setGNPPreferred(true);
                                    if (DEBUG) Log.d(TAG, "GNP now Preferred");
                                }
                                mNlpComboState.incrementAdaptiveCounter();
                            } else if (mNlpComboState.isGNPPreferred()) {
                                if (mNlpComboState.hasGNPReported() &&
                                    !mNlpComboState.hasQNPReported()) {
                                    mNlpComboState.incrementAdaptiveCounter();
                                } else if (mNlpComboState.hasQNPReported() &&
                                           !mNlpComboState.hasGNPReported()) {
                                    mNlpComboState.decrementAdaptiveCounter();
                                }
                            } else {
                                if (mNlpComboState.hasQNPReported() &&
                                    !mNlpComboState.hasGNPReported()) {
                                    mNlpComboState.incrementAdaptiveCounter();
                                } else if (mNlpComboState.hasGNPReported() &&
                                           !mNlpComboState.hasQNPReported()) {
                                    mNlpComboState.decrementAdaptiveCounter();
                                }
                            }

                            if (DEBUG) Log.d(TAG, "New Tolerance Timer for Interval");
                            mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);
                            mHandler.sendEmptyMessageDelayed(MSG_TOLERANCE_TIMEOUT,
                                    mNlpToleranceTimeAfter + mNlpComboState.getInterval());
                            mNlpComboState.intervalStarted();
                        }
                        mNlpComboState.dump();
                    } else if (mNlpMode == NLP_MODE_COMBO_PARALLEL) {
                        ReportLocation(mNlpComboState.getBestLocation());
                        mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);
                    } else if (mNlpMode == NLP_MODE_COMBO_GNP_PRIMARY ||
                               mNlpMode == NLP_MODE_COMBO_QNP_PRIMARY) {

                        mHandler.removeMessages(MSG_TOLERANCE_TIMEOUT);
                        if (mNlpComboState.isAwaitingFirstFix()) {
                            long time = mNlpToleranceTimeFirst *
                                        mNlpComboState.getSwitchCounter();
                            if (time < 0) { //check for overflow
                                time = Long.MAX_VALUE;
                            }
                            if (DEBUG) Log.d(TAG, "New Tolerance Timer for First Fix " + time);
                            mHandler.sendEmptyMessageDelayed(MSG_TOLERANCE_TIMEOUT, time);
                        } else {
                            long time = mNlpToleranceTimeAfter *
                                        mNlpComboState.getSwitchCounter() +
                                        mNlpComboState.getInterval();
                            if (time < 0) { //check for overflow
                                time = Long.MAX_VALUE;
                            }
                            if (DEBUG) Log.d(TAG, "New Tolerance Timer for After First Fix " + time);
                            mHandler.sendEmptyMessageDelayed(MSG_TOLERANCE_TIMEOUT, time);
                        }

                        // if no qnp consent and we are currently using gnp, don't switch
                        if (!mQnpConsent &&
                            ((mNlpMode == NLP_MODE_COMBO_GNP_PRIMARY &&
                              !mNlpComboState.isSwitched()) ||
                             (mNlpMode == NLP_MODE_COMBO_QNP_PRIMARY &&
                              mNlpComboState.isSwitched()))) {
                            // increment counter even without switching so that timer increases
                            mNlpComboState.incrementSwitchCounter();
                        } else {
                            ProviderRequest stopRequest = new ProviderRequest();
                            stopRequest.reportLocation = false;
                            stopRequest.interval = 0;
                            if (mNlpComboState.isSwitched()) {
                                if (DEBUG) Log.d(TAG, "Switching to Primary NLP ");
                                SetRequest(mNlpServiceSecondary,
                                           stopRequest,
                                           mNlpComboState.getSource());
                                SetRequest(mNlpServicePrimary,
                                           mNlpComboState.getRequest(),
                                           mNlpComboState.getSource());
                            } else {
                                if (DEBUG) Log.d(TAG, "Switching to Secondary NLP ");
                                SetRequest(mNlpServicePrimary,
                                           stopRequest,
                                           mNlpComboState.getSource());
                                SetRequest(mNlpServiceSecondary,
                                           mNlpComboState.getRequest(),
                                           mNlpComboState.getSource());
                            }

                            mNlpComboState.switched();
                        }

                    }
                    break;
                }
                case MSG_RECONNECT_GNP:
                {
                    int retryNumber = (Integer)msg.obj;
                    Log.w(TAG, "Reconnect GNP retry #" + retryNumber);
                    synchronized(mLock) {
                        mBindedPrimary = bindPackage(GNP_PACKAGE_NAMES, GNP_ACTION_NAMES,
                                                     mNlpServiceConnectionPrimary);
                        if (!mBindedPrimary) {
                            if (retryNumber < MAX_GNP_RETRIES) {
                                Message message = Message.obtain(mHandler,
                                                                 MSG_RECONNECT_GNP, retryNumber+1);
                                mHandler.sendMessageDelayed(message, retryNumber*1000);
                            } else {
                                Log.e(TAG, "Give up on GNP after " + MAX_GNP_RETRIES + " retries!");
                            }
                        }
                    }
                    break;
                }
                default:
                {
                    Log.w(TAG, "Unhandled Message " + msg.what);
                }
            }
        }
    };

    @Override
    public void onEnable() {
        if (DEBUG) Log.d(TAG, "onEnable");
        synchronized (mLock) {
            mEnabled = true;
            if (mNlpServicePrimary == null) {
                if (DEBUG) Log.d(TAG, "Will enable primary after connected");
                mEnableOnConnectedPrimary = true;
            } else {
                mHandler.sendEmptyMessage(MSG_ENABLE_PRIMARY);
            }

            if (mNlpMode >= NLP_MODE_COMBO) {
                if (mNlpServiceSecondary == null) {
                    if (DEBUG) Log.d(TAG, "Will enable secondary after connected");
                    mEnableOnConnectedSecondary = true;
                } else {
                    mHandler.sendEmptyMessage(MSG_ENABLE_SECONDARY);
                }
            }
        }
    }

    @Override
    public void onDisable() {
        if (DEBUG) Log.d(TAG, "onDisable");
        synchronized (mLock) {
            mEnabled = false;
            if (mNlpServicePrimary == null) {
                if (DEBUG) Log.d(TAG, "Will Not enable primary after connected");
                mEnableOnConnectedPrimary = false;
            } else {
                mHandler.sendEmptyMessage(MSG_DISABLE_PRIMARY);
            }

            if (mNlpMode >= NLP_MODE_COMBO) {
                if (mNlpServiceSecondary == null) {
                    if (DEBUG) Log.d(TAG, "Will not enable secondary after connected");
                    mEnableOnConnectedSecondary = false;
                } else {
                    mHandler.sendEmptyMessage(MSG_DISABLE_SECONDARY);
                }
            }
        }
    }

    @Override
    public void onSetRequest(ProviderRequestUnbundled request, WorkSource source) {
        if (DEBUG) Log.d(TAG, "onSetRequest");
        synchronized (mLock) {
            if (mNlpMode == NLP_MODE_NONE) {
                Log.e(TAG, "NLP request ignored, as no existing NLPs");
                return;
            }
            mRequestSet = true;
            mProviderRequestOnConnected = request;
            mWorkSourceOnConnected = source;
            if (mNlpServicePrimary == null ||
                (mNlpMode >= NLP_MODE_COMBO && mNlpServiceSecondary == null)) {
                if (DEBUG) Log.d(TAG, "Will set request after connected");
                mSetRequestOnConnected = true;
                return;
            }

            mHandler.obtainMessage(MSG_SET_REQUEST,
                                   new RequestWrapper(request, source)).sendToTarget();
        }
    }

    @Override
    public int onGetStatus(Bundle extras) {
        return LocationProvider.AVAILABLE;
    }

    @Override
    public long onGetStatusUpdateTime() {
        return 0;
    }

    private final class NlpLocationListener implements LocationListener {

        public void onLocationChanged(Location location) {
            if (DEBUG) Log.d(TAG, "onLocationChanged: " + location.getProvider());

            if (LocationManager.NETWORK_PROVIDER.equals(location.getProvider())) {

                if (LocationNeedsScreening(location)) {
                    location = ClearScreeningMarker(location);
                    mHandler.obtainMessage(MSG_LOCATION_UPDATE, location).sendToTarget();
                }
            }
        }

        public void onStatusChanged(String provider, int status, Bundle extras) {
            if (DEBUG) Log.d(TAG, "status: " + status);
        }

        public void onProviderDisabled(String provider) {
            if (DEBUG) Log.d(TAG, "provider disabled: " + provider);
        }

        public void onProviderEnabled(String provider) {
            if (DEBUG) Log.d(TAG, "provider re-enabled: " + provider);
        }
    };

  private final class QnpConsentObserver implements Observer
  {
      public void update(Observable o, Object arg)
      {
          UpdateQnpConsent();
      }
  }

}
