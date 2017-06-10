/******************************************************************************
 * @file    VoicePostProcessingTest.java
 * @brief   Provides the preferences for various latencies during playback and 
 *	    recording of audio.User can select show latencies or clear latencies. 
 *          
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2011 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *  ******************************************************************************/

package com.android.AudioLatencyTest;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import android.widget.EditText;
import android.widget.TextView;
import android.app.Activity;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.os.Process;

public class AudioLatencyTest extends Activity {
    private AudioTrack track;
    private AudioRecord record;
    private int buffer_size_record;
    private int buffer_size_play;
    byte[] buffer;
    long time;
    byte[] buffer_warm_play;
    byte[] buffer_cont_play;
    private FileReader fr_out_latency;
    private FileReader fr_in_latency;
    private FileWriter fw_enable_latency;
    private char[] final_buf;
    private String[] final_string;
    private long cold_play_app;
    private long warm_play_app;
    private long cont_play_app;
    private long cold_rec_app;
    private long cont_rec_app;
    private long cold_play_driver_sec=0;
    private long warm_play_driver_sec=0;
    private long cont_play_driver_sec=0;
    private long cold_play_driver_usec=0;
    private long warm_play_driver_usec =0;
    private long cont_play_driver_usec=0;
    private long cont_rec_driver_sec=0;
    private long cont_rec_driver_usec=0;
    private TextView cold_play_tv;
    private TextView warm_play_tv;
    private TextView cont_play_tv;
    private TextView cold_rec_tv;
    private TextView cont_rec_tv;
    private EditText dsp_latency_et;
    private int error=0;
    private long dsp_latency;
    /** Called when the activity is first created.
    @Override
    */
    public void onCreate(Bundle savedInstanceState) {
      	super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        buffer_size_play=AudioTrack.getMinBufferSize(44100,AudioFormat.CHANNEL_OUT_STEREO,AudioFormat.ENCODING_PCM_16BIT );
        Log.v(this.toString(), "MinBufferSize_44.1k_STEREO for track:" + buffer_size_play);
        buffer = new byte[buffer_size_play];
        buffer_warm_play = new byte[buffer_size_play];
        buffer_cont_play = new byte[buffer_size_play];
        
        for (int i=0 ; i< buffer_size_play ; i=i+4) {
            buffer[i] = (byte) 0x00;
            buffer[i+1]= (byte) 0x00;
            buffer[i+2]= (byte) 0x00;
            buffer[i+3]= (byte) 0x00;
        }    
        buffer_size_record = AudioRecord.getMinBufferSize(8000, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
        Log.v(this.toString(), "MinBufferSize_8k_MONO for record:" + buffer_size_record);
        this.track = new AudioTrack(AudioManager.STREAM_MUSIC, 44100, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT,buffer_size_play, AudioTrack.MODE_STREAM);
        this.record = new AudioRecord(MediaRecorder.AudioSource.MIC, 8000, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, buffer_size_record);
       
    }
    
    
    public void onStart() {
        super.onStart();

        final_buf=new char[100];
        final_string=new String[7];
        cold_play_tv=(TextView)findViewById(R.id.tv1);
        warm_play_tv=(TextView)findViewById(R.id.tv2);
        cont_play_tv=(TextView)findViewById(R.id.tv3);
        cold_rec_tv=(TextView)findViewById(R.id.tv4);
        cont_rec_tv=(TextView)findViewById(R.id.tv5);
        dsp_latency_et= (EditText)findViewById(R.id.et1);
        try {
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_out_latency_measurement_node");
            fw_enable_latency.write("1");
            fw_enable_latency.flush();
            fw_enable_latency.close();
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_in_latency_measurement_node");
            fw_enable_latency.write("1");
            fw_enable_latency.flush();
            fw_enable_latency.close();
        }
        catch(Exception e) {
            Log.e(this.toString(),"Exception in file writer");
            e.printStackTrace();
        }
    }
    
    public void coldOutputLatency(View view) {
        time = System.currentTimeMillis();
        cold_play_app = time;
        Log.v(this.toString(), "Cold output latency - Track.play time:"+ time);
        track.play();
        int priority = Process.getThreadPriority(Process.myTid());
        Process.setThreadPriority(Process.myTid(), Process.THREAD_PRIORITY_AUDIO);
        Log.v(this.toString(), " tid = " + Process.myTid() + ",priority =" + priority + " ,new priority = " + Process.getThreadPriority(Process.myTid()));
        for (int i=0;i<25;i++) {
            track.write(buffer, 0, buffer_size_play);
        }
        try {
            fr_out_latency = new FileReader("/sys/kernel/debug/audio_out_latency_measurement_node");
            error=fr_out_latency.read(final_buf,0,100);
            if(error==-1)
                Log.e(this.toString(),"error in reading from debugfs file");
            fr_out_latency.close();
        }
        catch(Exception e) {
            Log.e(this.toString(),"Exception ColdOutputLatency File Operations");
	}
  		
	final_string=new String(final_buf,0,error).trim().split(",");
        cold_play_driver_sec = Long.parseLong(final_string[0]);
        cold_play_driver_usec = Long.parseLong(final_string[1]);
        Process.setThreadPriority(Process.myTid(), Process.THREAD_PRIORITY_DEFAULT);
        track.stop();
    }
    
    public void warmOutputLatency(View view) throws InterruptedException {
        track.play();
        int priority = Process.getThreadPriority(Process.myTid());
        Process.setThreadPriority(Process.myTid(), Process.THREAD_PRIORITY_AUDIO);
        Log.v(this.toString(), " tid = " + Process.myTid() + ",priority =" + priority + " ,new priority = " + Process.getThreadPriority(Process.myTid()));
        for (int i=0;i<25;i++) {
            if(i==16) {
                time = System.currentTimeMillis();
                warm_play_app = time;
                Log.v(this.toString(), "Warm output latency time:"+ time);
                track.write(buffer_warm_play, 0, buffer_size_play);
            }
            else {
                track.write(buffer, 0, buffer_size_play);
            }

            if (i == 15) {
                Thread.sleep(1000);
                buffer_warm_play[0]=(byte)0x57;
                buffer_warm_play[1]=(byte)0x41;
                buffer_warm_play[2]= (byte)0x00;
                buffer_warm_play[3]=(byte)0x00;
                System.arraycopy(buffer,4,buffer_warm_play,4,(buffer_size_play-4));
            }
        }
        try {
            fr_out_latency = new FileReader("/sys/kernel/debug/audio_out_latency_measurement_node");
            error=fr_out_latency.read(final_buf,0,100);
            if(error==-1)
                Log.e(this.toString(),"error in reading from debugfs file");
            fr_out_latency.close();
        }
        catch(Exception e) {
            Log.e(this.toString(),"exception WarmOutputLatency File Opearations");
        }
        final_string=new String(final_buf,0,error).trim().split(",");
        warm_play_driver_sec = Long.parseLong(final_string[2]);
        warm_play_driver_usec = Long.parseLong(final_string[3]);
        Process.setThreadPriority(Process.myTid(), Process.THREAD_PRIORITY_DEFAULT);
        track.stop();
    }
    
    public void continuousOutputLatency(View view) throws InterruptedException{
        track.play();
        int priority = Process.getThreadPriority(Process.myTid());
        Process.setThreadPriority(Process.myTid(), Process.THREAD_PRIORITY_AUDIO);
        Log.v(this.toString(), " tid = " + Process.myTid() + ",priority =" + priority + " ,new priority = " + Process.getThreadPriority(Process.myTid()));
        for (int i=0;i<25;i++) {
            if(i==16) {
                time = System.currentTimeMillis();
                cont_play_app = time;
                Log.v(this.toString(), "Continuous output latency - time:"+ time);
                track.write(buffer_cont_play, 0, buffer_size_play);
            }
            else {
                track.write(buffer, 0, buffer_size_play);
            }
            if (i == 15) {
                buffer_cont_play[0]=(byte)0x00;
                buffer_cont_play[1]=(byte)0x00;
                buffer_cont_play[2]= (byte)0x4E;
                buffer_cont_play[3]=(byte)0x54;
                System.arraycopy(buffer,4,buffer_cont_play,4,(buffer_size_play-4));
            }
        }
        try {
            fr_out_latency = new FileReader("/sys/kernel/debug/audio_out_latency_measurement_node");
            error=fr_out_latency.read(final_buf,0,100);
            if(error==-1)
                Log.e(this.toString(),"error in reading from debugfs file");
            fr_out_latency.close();
        }
        catch(Exception e) {
            Log.e(this.toString(),"Exception Continuous Output Latency File Operations");
        }
        final_string=new String(final_buf,0,error).trim().split(",");
        cont_play_driver_sec = Long.parseLong(final_string[4]);
        cont_play_driver_usec = Long.parseLong(final_string[5]);
        Process.setThreadPriority(Process.myTid(), Process.THREAD_PRIORITY_DEFAULT);
        track.stop();
    }
   
    public void coldInputLatency (View view) {
        time = System.currentTimeMillis();
        cold_rec_app = time;
        Log.v(this.toString(), "Cold input latency - record.startRecord time:"+ time);
        record.startRecording();
        int priority = Process.getThreadPriority(Process.myTid());
        Process.setThreadPriority(Process.myTid(), Process.THREAD_PRIORITY_AUDIO);
        Log.v(this.toString(), " tid = " + Process.myTid() + ",priority =" + priority + " ,new priority = " + Process.getThreadPriority(Process.myTid()));

        for (int i=0;i<10;i++) {
            int size = record.read(buffer, 0, buffer_size_record);
            if(i==0) {
                time = System.currentTimeMillis();
                cold_rec_app = time - cold_rec_app;
                Log.v(this.toString(), "Cold input latency - time:"+ time+" size:"+size );
            }
        }
        Process.setThreadPriority(Process.myTid(), Process.THREAD_PRIORITY_DEFAULT);
        record.stop();
    }
   
    public void continuousInputLatency(View view) throws Exception, IOException {
        record.startRecording();
        int priority = Process.getThreadPriority(Process.myTid());
        Process.setThreadPriority(Process.myTid(), Process.THREAD_PRIORITY_AUDIO);
        Log.v(this.toString(), " tid = " + Process.myTid() + ",priority =" + priority + " ,new priority = " + Process.getThreadPriority(Process.myTid()));

        for (int i=0;i<10;i++) {
        int size = record.read(buffer, 0, buffer_size_record);
        if(i==4) {
                time = System.currentTimeMillis();
                cont_rec_app = time;
                Log.v(this.toString(), "Continuous input latency - time:"+ time+" size:"+size );
            }
            else {
                time = System.currentTimeMillis();
            }
        }
        try {
            fr_in_latency = new FileReader("/sys/kernel/debug/audio_in_latency_measurement_node");
            error=fr_in_latency.read(final_buf,0,20);
            if(error==-1)
                Log.e(this.toString(),"error in reading from debugfs file");
            fr_in_latency.close();
        }
        catch(Exception e) {
            Log.e(this.toString(),"Exception Continuous Input Latency File Operations");
        }
        final_string=new String(final_buf,0,error).trim().split(",");
        cont_rec_driver_sec = Long.parseLong(final_string[0]);
        cont_rec_driver_usec = Long.parseLong(final_string[1]);
        Process.setThreadPriority(Process.myTid(), Process.THREAD_PRIORITY_DEFAULT);
        record.stop();
    }
    
    public void showLatencies(View v) {
        double cold_play_driver = 0.0;
        double warm_play_driver = 0.0;
        double cont_play_driver = 0.0 ;
        double cold_rec_driver = 0.0;
        double cont_rec_driver =0.0;

        dsp_latency=Long.parseLong(dsp_latency_et.getText().toString());

        cold_play_driver = (cold_play_driver_sec*1000) +( cold_play_driver_usec/1000);
        Log.v(this.toString(),"value of cold play driver  is"+ cold_play_driver);
        cold_play_driver= (cold_play_driver-cold_play_app)+dsp_latency;
        Log.v(this.toString(),"value of cold output latency is"+ cold_play_driver);

        warm_play_driver = (warm_play_driver_sec*1000) +( warm_play_driver_usec/1000);
        Log.v(this.toString(),"value of warm play driver  is"+ warm_play_driver);
        warm_play_driver= warm_play_driver-warm_play_app;
        Log.v(this.toString(),"value of warm output latency is"+ warm_play_driver);

        cont_play_driver = (cont_play_driver_sec*1000) +( cont_play_driver_usec/1000);
        Log.v(this.toString(),"value of cont play driver  is"+ cont_play_driver);
        cont_play_driver= cont_play_driver-cont_play_app;
        Log.v(this.toString(),"value of cont output latency is"+ cont_play_driver);

        cold_rec_driver = cold_rec_app;
        Log.v(this.toString(),"value of cold input latency is"+ cold_rec_driver);

        cont_rec_driver = (cont_rec_driver_sec*1000) +( cont_rec_driver_usec/1000);
        cont_rec_driver =cont_rec_app-cont_rec_driver;
        Log.v(this.toString(),"value of cont input latency is"+ cont_rec_driver);

        cold_play_tv.setText("Cold Output Latency :"+Double.toString(cold_play_driver));
        warm_play_tv.setText("Warm Output Latency :"+Double.toString(warm_play_driver));
        cont_play_tv.setText("Cont Output Latency :"+Double.toString(cont_play_driver));
        cold_rec_tv.setText("Cold Input Latency :"+Double.toString(cold_rec_driver));
        cont_rec_tv.setText("Cont Input Latency :"+Double.toString(cont_rec_driver));
    }
    public void clearLatencies(View v) {
        cold_play_tv.setText("Cold Output Latency :"+0);
        warm_play_tv.setText("Warm Output Latency :"+0);
        cont_play_tv.setText("Cont Output Latency :"+0);
        cold_rec_tv.setText("Cold Input Latency :"+0);
        cont_rec_tv.setText("Cont Input Latency :"+0);
    }
    public void onStop() {
        Log.e(this.toString(), "onStop");
        super.onStop();
        try {
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_out_latency_measurement_node");
            fw_enable_latency.write("0");
            fw_enable_latency.flush();
            fw_enable_latency.close();
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_in_latency_measurement_node");
            fw_enable_latency.write("0");
            fw_enable_latency.flush();
            fw_enable_latency.close();
        }
        catch(Exception e) {
            Log.e(this.toString(),"Exception in file writer operations");
        }
    }
    
    public void onDestory() {
        Log.e(this.toString(), "onDestory");
    }

    }
