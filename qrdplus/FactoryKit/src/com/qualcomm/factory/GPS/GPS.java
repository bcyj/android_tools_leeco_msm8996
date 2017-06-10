/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.GPS;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import android.R.drawable;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.location.Criteria;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.factory.R;
import com.qualcomm.factory.Utilities;

public class GPS extends Activity {

    String TAG = "GPS";
    private Context mContext;
    TextView mTextView;
    Button startButton;
    Button stopButton;
    EditText mEditText;
    ListView mListView = null;
    private Location location;
    LayoutInflater mInflater = null;
    LocationManager mLocationManager = null;
    final int OUT_TIME = 150 * 1000;
    final int MIN_SAT_NUM = 1;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        mContext = this;
        mInflater = LayoutInflater.from(mContext);
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.gps);
        getService();
        bindView();
    }

    CountDownTimer mCountDownTimer = new CountDownTimer(OUT_TIME, 3000) {

        @Override
        public void onTick(long arg0) {

        }

        @Override
        public void onFinish() {

            fail(getString(R.string.time_out));
        }
    };

    void startGPS() {

        if (!mLocationManager.isProviderEnabled(android.location.LocationManager.GPS_PROVIDER)) {
            toast(getString(R.string.gps_enable_first));
            Intent intent = new Intent();

            // Entry the "LocationSettingsActivity" to turn
            // on the "GPS" setting.
            intent.setClassName("com.android.settings",
                    "com.android.settings.Settings$LocationSettingsActivity");
            //startActivity(intent);
            //return;
        }
        Criteria criteria;
        criteria = new Criteria();
        criteria.setAccuracy(Criteria.ACCURACY_FINE);
        criteria.setAltitudeRequired(false);
        criteria.setBearingRequired(false);
        criteria.setCostAllowed(true);
        criteria.setPowerRequirement(Criteria.POWER_LOW);

        String provider = mLocationManager.getBestProvider(criteria, true);
        if (provider == null) {
            fail("Fail to get GPS Provider!");
        }
        loge("here");
        mLocationManager.requestLocationUpdates(provider, 500, 0, mLocationListener);
        mLocationManager.addGpsStatusListener(gpsStatusListener);

        location = mLocationManager.getLastKnownLocation(provider);
        setLocationView(location);
    }

    @Override
    protected void onPause() {

        stopGPS();
        super.onPause();
    }

    private void setLocationView(Location location) {

        if (location != null) {
            double latitude = location.getLatitude();
            double longitude = location.getLongitude();
            double speed = location.getSpeed();
            double altitude = location.getAltitude();
            double bearing = location.getBearing();
            mTextView.setText("latitude:" + latitude + '\n' + "longitude:" + longitude + '\n'
                    + "speed:" + speed + "m/s" + '\n' + "altitude:" + altitude + "m" + '\n'
                    + "bearing:" + bearing + '\n');
        } else {
            mTextView.setText("Location unknown or locating...");
        }
    }

    LocationListener mLocationListener = new LocationListener() {

        public void onLocationChanged(Location location) {

            setLocationView(location);
            pass();
        }

        public void onProviderDisabled(String provider) {

            setLocationView(null);
        }

        public void onProviderEnabled(String provider) {

            toast("Provider enabled");

        }

        public void onStatusChanged(String provider, int status, Bundle extras) {

        }
    };

    private GpsStatus mGpsStatus;
    private Iterable<GpsSatellite> mSatellites;
    List<String> satelliteList = new ArrayList<String>();
    GpsStatus.Listener gpsStatusListener = new GpsStatus.Listener() {

        public void onGpsStatusChanged(int arg0) {

            switch (arg0)
            {
            case GpsStatus.GPS_EVENT_STARTED:
                toast("GPS Start");
                setProgressBarIndeterminateVisibility(true);
                mCountDownTimer.start();
                break;
            case GpsStatus.GPS_EVENT_STOPPED:
                // toast("GPS Stop");
                setProgressBarIndeterminateVisibility(false);
                break;
            case GpsStatus.GPS_EVENT_FIRST_FIX:
                toast("Locate sucess");
                pass();
                break;
            case GpsStatus.GPS_EVENT_SATELLITE_STATUS:
                logd("GPS_EVENT_SATELLITE_STATUS");
                mGpsStatus = mLocationManager.getGpsStatus(null);
                mSatellites = mGpsStatus.getSatellites();
                Iterator<GpsSatellite> it = mSatellites.iterator();
                int count = 0;
                satelliteList.clear();
                while (it.hasNext()) {
                    GpsSatellite gpsS = (GpsSatellite) it.next();
                    satelliteList.add(count++, "Prn" + gpsS.getPrn() + " Snr:" + gpsS.getSnr());
                }
                updateAdapter();
                if (count >= MIN_SAT_NUM)
                    pass();
                break;
            default:
                break;
            }

        }

    };

    public void updateAdapter() {

        mAdapter.notifyDataSetChanged();
    }

    void stopGPS() {

        try {
            mLocationManager.removeUpdates(mLocationListener);
            mLocationManager.removeGpsStatusListener(gpsStatusListener);
            setProgressBarIndeterminateVisibility(true);
        } catch (Exception e) {
            loge(e);
        }
    }

    void bindView() {

        mTextView = (TextView) findViewById(R.id.gps_hint);
        startButton = (Button) findViewById(R.id.gps_start);
        startButton.setOnClickListener(new OnClickListener() {

            public void onClick(View arg0) {                
                startGPS();
            }
        });
        stopButton = (Button) findViewById(R.id.gps_stop);
        stopButton.setOnClickListener(new OnClickListener() {

            public void onClick(View arg0) {

                finish();
            }
        });

        mListView = (ListView) findViewById(R.id.gps_list);
        mListView.setAdapter(mAdapter);
        registerForContextMenu(mListView);

    }

    public void finish() {

        stopGPS();
        super.finish();
    };

    void getService() {

        mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
        if (mLocationManager == null) {
            fail("Fail to get LOCATION_SERVICE!");

        }
    }

    @Override
    protected void onDestroy() {

        if (mCountDownTimer != null)
            mCountDownTimer.cancel();
        super.onDestroy();
    }

    protected void onResume() {

        startGPS();
        super.onResume();
    };

    BaseAdapter mAdapter = new BaseAdapter() {

        public Object getItem(int arg0) {

            return null;
        }

        public long getItemId(int arg0) {

            return 0;
        }

        public View getView(int index, View convertView, ViewGroup parent) {

            if (convertView == null)
                convertView = mInflater.inflate(R.layout.gps_item, null);
            TextView mText = (TextView) convertView.findViewById(R.id.gps_text);
            ImageView mImage = (ImageView) convertView.findViewById(R.id.gps_image);
            mText.setText(satelliteList.get(index));
            mImage.setImageResource(drawable.presence_online);
            return convertView;
        }

        public int getCount() {

            if (satelliteList != null)
                return satelliteList.size();
            else
                return 0;
        }

    };

    void fail(Object msg) {

        loge(msg);
        toast(msg);
        setResult(RESULT_CANCELED);
        Utilities.writeCurMessage(this, TAG,"Failed");
        finish();
    }

    void pass() {

        setResult(RESULT_OK);
        Utilities.writeCurMessage(this, TAG,"Pass");
        finish();
    }

    public void toast(Object s) {

        if (s == null)
            return;
        Toast.makeText(this, s + "", Toast.LENGTH_SHORT).show();
    }

    private void loge(Object e) {

        if (e == null)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        e = "[" + mMethodName + "] " + e;
        Log.e(TAG, e + "");
    }

    private void logd(Object s) {

        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }

}
