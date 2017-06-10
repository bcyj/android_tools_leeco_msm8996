/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.android.stoptimer;

import java.util.ArrayList;
import java.util.HashMap;
import android.app.Activity;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.view.Menu;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;

public class StopTimer extends Activity implements OnClickListener {
    private final int HOUR_BASE = 3600000;    // 3600s
    private final int MINUTE_BASE = 60000;    // 60s
    private final int SECOND_BASE = 1000;
    private final int TIME_INTERVAL = 10;    // in the unit of millisecond
    private final int MSG = 1;
    private final int MAX_RECORD_COUNT = 999;
    private final int MSG_DELAY_TIME = 500;
    private long mStartTime;
    private long mPauseTime;
    private long mLoopTimePre;
    
    enum RunningState {PENDING, RUNNING, PAUSE};
    private RunningState state;

    TextView tvStopWatch;
    TextView tvMilliSecond;
    ListView lvRecords;
    Button btnStart;
    Button btnPause;
    Button btnReset;
    
    private int sequence;
    ArrayList<HashMap<String, Object>> listItem = new ArrayList<HashMap<String, Object>>();
    
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            synchronized(StopTimer.this) {
                updateTimer(SystemClock.elapsedRealtime() - mStartTime);    // update every TIME_INTERVAL ms
                sendMessageDelayed(obtainMessage(MSG), TIME_INTERVAL);
            }
        }
    };
    
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.main);
        tvStopWatch = (TextView) findViewById(R.id.tvStopWatch);
        tvStopWatch.setText("00:00:00");
        
        tvMilliSecond = (TextView) findViewById(R.id.tvMilliSecond);
        tvMilliSecond.setText("00");
        btnStart = (Button) findViewById(R.id.btnStart);
        btnStart.setOnClickListener(this);
        
        btnPause = (Button) findViewById(R.id.btnPause);
        btnPause.setOnClickListener(this);
        
        btnReset = (Button) findViewById(R.id.btnReset);
        btnReset.setOnClickListener(this);
        
        lvRecords = (ListView) findViewById(R.id.lvRecords);
        
        state = RunningState.PENDING;
        sequence = 1;
        mLoopTimePre = 0;
    }
    
   

    public void Start() {
        mStartTime = SystemClock.elapsedRealtime();
        state = RunningState.RUNNING;
        sequence = 1;
        mLoopTimePre = 0;
        mHandler.sendMessage(mHandler.obtainMessage(MSG));
        btnStart.setVisibility(View.GONE);
        
        btnPause.setVisibility(View.VISIBLE);
        btnPause.setText(R.string.pause);
        
        btnReset.setText(R.string.loop);
        btnReset.setVisibility(View.VISIBLE);
    }
    
    public void Pause() {
        if(RunningState.PAUSE != state) {
            mPauseTime = SystemClock.elapsedRealtime();
            mHandler.removeMessages(MSG);
            btnPause.setText(R.string.resume);
            state = RunningState.PAUSE;
            btnReset.setText(R.string.reset);
        } else {
            mStartTime = SystemClock.elapsedRealtime() - mPauseTime + mStartTime;
            mHandler.sendMessage(mHandler.obtainMessage(MSG));
            state = RunningState.RUNNING;
            btnPause.setText(R.string.pause);
            btnReset.setText(R.string.loop);
        }
    }
    
    public void Reset() {
        if(RunningState.PAUSE == state) {
            mHandler.removeMessages(MSG);
            tvStopWatch.setText("00:00:00");
            tvMilliSecond.setText("00");
            listItem.clear();
            
            btnPause.setVisibility(View.GONE);
            btnReset.setVisibility(View.GONE);        
            btnStart.setVisibility(View.VISIBLE);

            //reset the state to pending.because onConfigurationChanged will use this state.
            state = RunningState.PENDING;
        } else if(RunningState.RUNNING == state) {
            // update Loop records ListView
            long timeDiff = SystemClock.elapsedRealtime() / 10 - mStartTime / 10;
            timeDiff *= 10;
            UpdateLoopList(timeDiff);
            mLoopTimePre = timeDiff;
            ++sequence;
        }
    }
    
    public void UpdateLoopList(long milliSecond) {
        int intervalHour = (int)milliSecond / HOUR_BASE % 24;
        int intervalMinute = (int)milliSecond % HOUR_BASE / MINUTE_BASE;
        int intervalSecond = (int)milliSecond % MINUTE_BASE / SECOND_BASE;
        int intervalMS = (int)milliSecond % SECOND_BASE /10;
        StringBuffer sb = new StringBuffer();
        if (tvStopWatch.isLayoutRtl()) {
            displayTimeString(intervalSecond, intervalMinute, intervalHour, sb);
        } else {
            displayTimeString(intervalHour, intervalMinute, intervalSecond, sb);
        }
        String strTime = sb.toString();     // HH:MM:SS
        sb.delete(0, sb.length());          // clear sb data
        String strMs = (intervalMS < 10) ? ("0" + intervalMS) : (String.valueOf(intervalMS)); 
        
        long diff = milliSecond - mLoopTimePre;
        intervalHour = (int)diff / HOUR_BASE % 24;
        intervalMinute = (int)diff % HOUR_BASE / MINUTE_BASE;
        intervalSecond = (int)diff % MINUTE_BASE / SECOND_BASE;
        intervalMS = (int)diff % SECOND_BASE /10;
        if (tvStopWatch.isLayoutRtl()) {
            if (intervalMS < 10) {
                sb.append('0');
            }
            sb.append(intervalMS).append(':'); // MS
            displayTimeString(intervalHour, intervalMinute, intervalSecond, sb);
                } else {
            displayTimeString(intervalHour, intervalMinute, intervalSecond, sb);
            sb.append(':');
            if (intervalMS < 10) {
                sb.append('0');
            }
            sb.append(intervalMS); // MS
        }

        String strDiff = sb.toString();     // HH:MM:SS:MS
        
        HashMap<String, Object> map = new HashMap<String, Object>();
        map.put("sequence", String.valueOf(sequence));
        map.put("time", strTime);
        map.put("timeMs", strMs);
        map.put("diff", strDiff);
        listItem.add(0, map);
        
        if(listItem.size() > MAX_RECORD_COUNT) {
            listItem.remove(listItem.size() - 1);   // remove the oldest record
        }
        
        SimpleAdapter listItemAdapter = new SimpleAdapter(this, listItem, R.layout.record, 
                new String[] {"sequence", "time", "timeMs", "diff"},    
                new int[] {R.id.sequence, R.id.time, R.id.timeMs, R.id.diff}
                );
        lvRecords.setAdapter(listItemAdapter);
    }

    public String updateTimer(long milliSecond) {
        int intervalHour = (int)milliSecond / HOUR_BASE % 24;
        int intervalMinute = (int)milliSecond % HOUR_BASE / MINUTE_BASE;
        int intervalSecond = (int)milliSecond % MINUTE_BASE / SECOND_BASE;
        int intervalMS = (int)milliSecond % SECOND_BASE /10;
        StringBuffer sb = new StringBuffer();
        if (tvStopWatch.isLayoutRtl()) {
            displayTimeString(intervalSecond, intervalMinute, intervalHour, sb);
        } else {
            displayTimeString(intervalHour, intervalMinute, intervalSecond, sb);
        }

        tvStopWatch.setText(sb);
        if(intervalMS < 10) {
            tvMilliSecond.setText("0" + intervalMS);
        } else {
            tvMilliSecond.setText(String.valueOf(intervalMS));
        }
        return sb.toString();
    }

    public void onClick(View v) {
        switch(v.getId()) {
            case R.id.btnStart:
                Start();
                break;
            case R.id.btnPause:
                Pause();
                break;
            case R.id.btnReset:
                Reset();
                break;
        }
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        mHandler.removeMessages(MSG);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (state == RunningState.RUNNING) {
            updateTimer(SystemClock.elapsedRealtime() - mStartTime);
            // Delay 500ms to avoid the screen twitter.
            mHandler.sendEmptyMessageDelayed(MSG, MSG_DELAY_TIME);
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (state == RunningState.RUNNING) {
            // No need to setTextView.
            mHandler.removeMessages(MSG);
        }
    }

    /**
     * when system configuration are changed.this method will
     * be invoked.so when language changed.will flush words source
     * in this method.
     *
     * @param newConfig the new configuration of device
     */
    @Override
    public void onConfigurationChanged (Configuration newConfig) {
        super.onConfigurationChanged (newConfig);

        //handle locale changed event.
        handleLocaleChanged();

    }

    /**
     * handle the locale change event.mainly when the language changed,
     * adaption the current language.
     */
    private void handleLocaleChanged() {

      if (RunningState.RUNNING == state) {

          if (null != btnPause) {
              btnPause.setText(R.string.pause);
          }
         if (null != btnReset) {
              btnReset.setText(R.string.loop);
         }
      } else if (RunningState.PAUSE == state) {

          if (null != btnPause) {
              btnPause.setText(R.string.resume);
          }
          if (null != btnReset) {
              btnReset.setText(R.string.reset);
          }

      }

      if (null != btnStart) {
          btnStart.setText(R.string.start);
      }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Let the system ignore the menu key when the activity is foreground.
        return false;
    }

    /*
     * Structure the string buffer to display the time.
     */
    private StringBuffer displayTimeString (int nFirstAppendTime, int nSecoundAppendTime,
            int nThreeAppendTime, StringBuffer sb) {
        if (nFirstAppendTime < 10) {
            sb.append('0');
        }
        sb.append(nFirstAppendTime).append(':');

        if (nSecoundAppendTime < 10) {
            sb.append('0');
        }
        sb.append(nSecoundAppendTime).append(':');

        if (nThreeAppendTime < 10) {
            sb.append('0');
        }
        sb.append(nThreeAppendTime);

        return sb;
    }
}
