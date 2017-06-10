/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 * Modifications to this file Developed by {group/entity that performed the modifications}.
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.nativeaudiolatency;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.util.Log;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import android.widget.EditText;
import android.widget.TextView;

public class NativeAudioLatency extends Activity {

    static final int CLIP_NONE = 0;
    static final int COLD_OUTPUT_LATENCY = 1;
    static final int WARM_OUTPUT_LATENCY = 2;
    static final int CONTINUOUS_OUTPUT_LATENCY = 3;
    static final int COLD_INPUT_LATENCY = 4;
    static final int CONTINUOUS_INPUT_LATENCY = 5;
    private FileWriter fw_enable_latency;
    private FileReader fr_out_latency;
    private FileReader fr_in_latency;
    private FileReader fr_out_breakdown_latency;
    private FileReader fr_in_breakdown_latency;
    private FileReader fr_adm_out_latency;
    private FileReader fr_adm_in_latency;

    private TextView cold_play_tv;
    private TextView warm_play_tv;
    private TextView cont_play_tv;
    private TextView cold_rec_tv;
    private TextView cont_rec_tv;
    private EditText dsp_latency_et;
    private long cold_play_app=0;
    private long warm_play_app=0;
    private long cont_play_app=0;
    private long cold_rec_app=0;
    private long cont_rec_app=0;
    private long cold_play_driver_sec=0;
    private long warm_play_driver_sec=0;
    private long cont_play_driver_sec=0;
    private long cold_play_driver_usec=0;
    private long warm_play_driver_usec =0;
    private long cont_play_driver_usec=0;
    private long cont_rec_driver_sec=0;
    private long cont_rec_driver_usec=0;
    private long cold_play_latency=0;
    private long warm_play_latency=0;
    private long cont_play_latency=0;
    private long cold_rec_latency=0;
    private long cont_rec_latency=0;

    /*private long in_open_start_driver_sec =0;
    private long in_open_start_driver_usec =0;
    private long in_open_end_driver_sec =0;
    private long in_open_end_driver_usec =0;
    private long out_open_start_driver_sec =0;
    private long out_open_start_driver_usec =0;
    private long out_open_end_driver_sec =0;
    private long out_open_end_driver_usec =0;
    private long in_mmap_start_driver_usec =0;
    private long in_mmap_start_driver_sec =0;
    private long in_mmap_end_driver_sec =0;
    private long in_mmap_end_driver_usec =0;
    private long out_mmap_start_driver_sec =0;
    private long out_mmap_start_driver_usec =0;
    private long out_mmap_end_driver_sec =0;
    private long out_mmap_end_driver_usec =0;
    private long in_open_start_driver;
    private long in_open_end_driver;
    private long out_open_start_driver;
    private long out_open_end_driver;
    private long out_mmap_start_driver;
    private long out_mmap_end_driver;
    private long in_mmap_end_driver;
    private long in_mmap_start_driver;
    private long in_adm_start_driver_sec =0;
    private long in_adm_start_driver_usec =0;
    private long in_adm_end_driver_sec =0;
    private long in_adm_end_driver_usec =0;
    private long out_adm_start_driver_sec =0;
    private long out_adm_start_driver_usec =0;
    private long out_adm_end_driver_sec =0;
    private long out_adm_end_driver_usec =0;
    private long in_adm_start_driver;
    private long in_adm_end_driver;
    private long out_adm_start_driver;
    private long out_adm_end_driver;*/

    private char[] final_buf;
    private String[] final_string;
    private int error = 0;
    private long dsp_latency = 0;
    static boolean created = false;

    /** Called when the activity is first created. */
    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.main);

        // initialize native audio system
        init();
        createEngine();
        createBufferQueueAudioPlayer();
    }

   protected void onStart() {
        super.onStart();


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
            //To Enable this break down latency we need to port the abandoned gerrit # 97027 to latest kernel version for 8960 and 8974
            /*fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_out_latency_measurement_breakdown_node");
            fw_enable_latency.write("1");
            fw_enable_latency.flush();
            fw_enable_latency.close();
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_in_latency_measurement_breakdown_node");
            fw_enable_latency.write("1");
            fw_enable_latency.flush();
            fw_enable_latency.close();
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_adm_out_latency_measurement_node");
            fw_enable_latency.write("1");
            fw_enable_latency.flush();
            fw_enable_latency.close();
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_adm_in_latency_measurement_node");
            fw_enable_latency.write("1");
            fw_enable_latency.flush();
            fw_enable_latency.close();*/
        }
        catch(Exception e) {
            Log.e(this.toString(),"Exception in file writer");
            e.printStackTrace();
        }

        // initialize button click handlers

        ((Button) findViewById(R.id.cold_output)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                // ignore the return value
                Log.v(this.toString(), "COLD_OUTPUT_LATENCY : " + COLD_OUTPUT_LATENCY);
                selectClip(COLD_OUTPUT_LATENCY, 25);
            }
        });

        ((Button) findViewById(R.id.warm_output)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                // ignore the return value
                Log.v(this.toString(), "WARM_OUTPUT_LATENCY : " + WARM_OUTPUT_LATENCY);
                selectClip(WARM_OUTPUT_LATENCY, 25);
            }
        });

        ((Button) findViewById(R.id.continuous_output)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                Log.v(this.toString(), "CLIP_CONTINUOUS : " + CONTINUOUS_OUTPUT_LATENCY);
                // ignore the return value
                selectClip(CONTINUOUS_OUTPUT_LATENCY, 50);
            }
        });

        ((Button) findViewById(R.id.cold_input)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
            created = isRecorderCreated();
                if (!created) {
                    Log.v(this.toString(),"Create record - Cold");
                    created = createAudioRecorder(COLD_INPUT_LATENCY);
                }
                if (created) {
                    Log.v(this.toString(),"start record - Cold");
                    startRecording(COLD_INPUT_LATENCY);
                }
            }
        });

        ((Button) findViewById(R.id.continuous_input)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
            created = isRecorderCreated();
                if (!created) {
                    Log.v(this.toString(),"Create record - Cont");
                    created = createAudioRecorder(CONTINUOUS_INPUT_LATENCY);
                }
                if (created) {
                    Log.v(this.toString(),"start record - Cont");
                    startRecording(CONTINUOUS_INPUT_LATENCY);
                }
            }
        });

        ((Button) findViewById(R.id.show)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {

                long cold_play_driver=0;
                long warm_play_driver=0;
                long cont_play_driver=0;
                long cold_rec_driver=0;
                long cont_rec_driver=0;
                final_buf=new char[200];
                final_string=new String[200];

                cold_play_app = getColdOutputLatency();
                warm_play_app = getWarmOutputLatency();
                cont_play_app = getContinuousOutputLatency();
                cold_rec_app = getColdInputLatency();
                cont_rec_app= getContinuousInputLatency();
                Log.e(this.toString(),"cold_play_app is : " +cold_play_app);
                Log.e(this.toString(),"warm_play_app is : " +warm_play_app);
                Log.e(this.toString(),"continuous_play_app is : " +cont_play_app);
                Log.e(this.toString(),"cold_rec_app is : " +cold_rec_app);
                Log.e(this.toString(),"continuous_rec_app is : " +cont_rec_app);

                try {
                    fr_out_latency = new FileReader("/sys/kernel/debug/audio_out_latency_measurement_node");
                    error=fr_out_latency.read(final_buf,0,200);
                    if(error==-1)
                        Log.e(this.toString(),"error in reading from debugfs file - out");
                    fr_out_latency.close();
                }
                catch(Exception e) {
                    Log.e(this.toString(),"Exception ColdOutputLatency File Operations - out");
                }

                final_string=new String(final_buf,0,error).trim().split(",");
                if (cold_play_app != 0) {
                    // parser driver latency only when app latency is calculated
                    cold_play_driver_sec = Long.parseLong(final_string[0]);
                    cold_play_driver_usec = Long.parseLong(final_string[1]);
                }
                if (warm_play_app != 0 && cold_play_app != 0) {
                    // parser driver latency only when app latency is calculated
                    warm_play_driver_sec = Long.parseLong(final_string[2]);
                    warm_play_driver_usec = Long.parseLong(final_string[3]);
                }
                if (cont_play_app != 0  && cold_play_app != 0) {
                    // parser driver latency only when app latency is calculated
                    cont_play_driver_sec = Long.parseLong(final_string[4]);
                    cont_play_driver_usec = Long.parseLong(final_string[5]);
                }
                //To Enable this break down latency we need to port the abandoned gerrit # 97027 to latest kernel version for 8960 and 8974
                /* Breakdown Cold Output Playback Latency*/
                /*try {
                    fr_out_breakdown_latency = new FileReader("/sys/kernel/debug/audio_out_latency_measurement_breakdown_node");
                    error=fr_out_breakdown_latency.read(final_buf,0,200);
                    if(error==-1)
                        Log.e(this.toString(),"error in reading from debugfs file - breakdown");
                    fr_out_breakdown_latency.close();
                }
                catch(Exception e) {
                    Log.e(this.toString(),"Exception OutputLatency Breakdown File Operations - breakdown");
                }
                final_string=new String(final_buf,0,error).trim().split(",");
                out_open_start_driver_sec = Long.parseLong(final_string[0]);
                Log.e(this.toString(),"out_open_start_driver_sec:"+out_open_start_driver_sec);
                out_open_start_driver_usec = Long.parseLong(final_string[1]);
                Log.e(this.toString(),"out_open_start_driver_usec:"+out_open_start_driver_usec);
                out_open_start_driver =( out_open_start_driver_sec * 1000) + (out_open_start_driver_usec/1000);
                out_open_end_driver_sec = Long.parseLong(final_string[2]);
                Log.e(this.toString(),"out_open_end_driver_sec:"+out_open_end_driver_sec);
                out_open_end_driver_usec = Long.parseLong(final_string[3]);
                Log.e(this.toString(),"out_open_end_driver_usec:"+out_open_end_driver_usec);
                out_open_end_driver =( out_open_end_driver_sec * 1000) + (out_open_end_driver_usec/1000);
                out_mmap_start_driver_sec = Long.parseLong(final_string[4]);
                Log.e(this.toString(),"out_mmap_start_driver_sec:"+out_mmap_start_driver_sec);
                out_mmap_start_driver_usec = Long.parseLong(final_string[5]);
                Log.e(this.toString(),"out_mmap_start_driver_usec:"+out_mmap_start_driver_usec);
                out_mmap_start_driver =( out_mmap_start_driver_sec * 1000) + (out_mmap_start_driver_usec/1000);
                out_mmap_end_driver_sec = Long.parseLong(final_string[6]);
                Log.e(this.toString(),"out_mmap_end_driver_sec:"+out_mmap_end_driver_sec);
                out_mmap_end_driver_usec = Long.parseLong(final_string[7]);
                Log.e(this.toString(),"out_mmap_end_driver_usec:"+out_mmap_end_driver_usec);
                out_mmap_end_driver =( out_mmap_end_driver_sec * 1000) + (out_mmap_end_driver_usec/1000);

                try {
                    fr_adm_out_latency = new FileReader("/sys/kernel/debug/audio_adm_out_latency_measurement_node");
                    error=fr_adm_out_latency.read(final_buf,0,40);
                    if(error==-1)
                        Log.e(this.toString(),"error in reading from debugfs file - adm");
                    fr_adm_out_latency.close();
                }
                catch(Exception e) {
                    Log.e(this.toString(),"Exception OutputLatency Breakdown File Operations - adm");
                }

                final_string=new String(final_buf,0,error).trim().split(",");
                out_adm_start_driver_sec = Long.parseLong(final_string[0]);
                Log.e(this.toString(),"out_adm_start_driver_sec:"+out_adm_start_driver_sec);
                out_adm_start_driver_usec = Long.parseLong(final_string[1]);
                Log.e(this.toString(),"out_adm_start_driver_usec:"+out_adm_start_driver_usec);
                out_adm_start_driver =(out_adm_start_driver_sec * 1000) + (out_adm_start_driver_usec/1000);
                out_adm_end_driver_sec = Long.parseLong(final_string[2]);
                Log.e(this.toString(),"out_adm_end_driver_sec:"+out_adm_end_driver_sec);
                out_adm_end_driver_usec = Long.parseLong(final_string[3]);
                Log.e(this.toString(),"out_adm_end_driver_usec:"+out_adm_end_driver_usec);
                out_adm_end_driver =(out_adm_end_driver_sec * 1000) + (out_adm_end_driver_usec/1000);*/

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

                /* Breakdown cold recording latency*/
                final_string=new String(final_buf,0,error).trim().split(",");
                if (cont_rec_app != 0 && cold_rec_app != 0) {
                    // parser driver latency only when app latency is calculated
                    cont_rec_driver_sec = Long.parseLong(final_string[0]);
                    cont_rec_driver_usec = Long.parseLong(final_string[1]);
                }
                //To Enable this break down latency we need to port the abandoned gerrit # 97027 to latest kernel version for 8960 and 8974
                /*try {
                    fr_in_breakdown_latency = new FileReader("/sys/kernel/debug/audio_in_latency_measurement_breakdown_node");
                    error=fr_in_breakdown_latency.read(final_buf,0,200);
                    if(error==-1)
                        Log.e(this.toString(),"error in reading from debugfs file");
                    fr_in_breakdown_latency.close();
                }
                catch(Exception e) {
                    Log.e(this.toString(),"Exception Input Breakdown File Operations");
                }
                final_string=new String(final_buf,0,error).trim().split(",");

                in_open_start_driver_sec = Long.parseLong(final_string[0]);
                Log.e(this.toString(),"in_open_start_driver_usec"+in_open_start_driver_sec);
                in_open_start_driver_usec = Long.parseLong(final_string[1]);
                Log.e(this.toString(),"in_open_start_driver_usec"+in_open_start_driver_usec);
                in_open_start_driver =( in_open_start_driver_sec * 1000) + (in_open_start_driver_usec/1000);
                in_open_end_driver_sec = Long.parseLong(final_string[2]);
                Log.e(this.toString(),"in_open_end_driver_sec"+in_open_end_driver_sec);
                in_open_end_driver_usec = Long.parseLong(final_string[3]);
                Log.e(this.toString(),"in_open_end_driver_sec"+in_open_end_driver_usec);

                try {
                    fr_adm_in_latency = new FileReader("/sys/kernel/debug/audio_adm_in_latency_measurement_node");
                    error=fr_adm_in_latency.read(final_buf,0,40);
                    if(error==-1)
                        Log.e(this.toString(),"error in reading from debugfs file - adm");
                    fr_adm_in_latency.close();
                }
                catch(Exception e) {
                    Log.e(this.toString(),"Exception InputLatency Breakdown File Operations - adm");
                }

                in_open_end_driver =( in_open_end_driver_sec * 1000) + (in_open_end_driver_usec/1000);
                in_mmap_start_driver_sec = Long.parseLong(final_string[4]);
                Log.e(this.toString(),"in_mmap_start_driver_sec"+in_mmap_start_driver_sec);
                in_mmap_start_driver_usec = Long.parseLong(final_string[5]);
                Log.e(this.toString(),"in_mmap_start_driver_usec"+in_mmap_start_driver_usec);
                in_mmap_start_driver =( in_mmap_start_driver_sec * 1000) + (in_mmap_start_driver_usec/1000);
                in_mmap_end_driver_sec = Long.parseLong(final_string[6]);
                Log.e(this.toString(),"in_mmap_end_driver_sec"+in_mmap_end_driver_sec);
                in_mmap_end_driver_usec = Long.parseLong(final_string[7]);
                Log.e(this.toString(),"in_mmap_end_driver_usec"+in_mmap_end_driver_usec);
                in_mmap_end_driver =( in_mmap_end_driver_sec * 1000) + (in_mmap_end_driver_usec/1000);

                try {
                    fr_adm_in_latency = new FileReader("/sys/kernel/debug/audio_adm_in_latency_measurement_node");
                    error=fr_adm_in_latency.read(final_buf,0,40);
                    if(error==-1)
                        Log.e(this.toString(),"error in reading from debugfs file - adm");
                    fr_adm_in_latency.close();
                }
                catch(Exception e) {
                    Log.e(this.toString(),"Exception InputLatency Breakdown File Operations - adm");
                }

                final_string=new String(final_buf,0,error).trim().split(",");
                in_adm_start_driver_sec = Long.parseLong(final_string[0]);
                Log.e(this.toString(),"in_adm_start_driver_sec:"+in_adm_start_driver_sec);
                in_adm_start_driver_usec = Long.parseLong(final_string[1]);
                Log.e(this.toString(),"in_adm_start_driver_usec:"+in_adm_start_driver_usec);
                in_adm_start_driver =(in_adm_start_driver_sec * 1000) + (in_adm_start_driver_usec/1000);
                in_adm_end_driver_sec = Long.parseLong(final_string[2]);
                Log.e(this.toString(),"in_adm_end_driver_sec:"+in_adm_end_driver_sec);
                in_adm_end_driver_usec = Long.parseLong(final_string[3]);
                Log.e(this.toString(),"in_adm_end_driver_usec:"+in_adm_end_driver_usec);
                in_adm_end_driver =(in_adm_end_driver_sec * 1000) + (in_adm_end_driver_usec/1000);*/


                if (dsp_latency_et.getText().toString().length() == 0) {
                    dsp_latency = 0;
                    Log.e(this.toString(),"DSP latency not set, use 0 as default");
                } else {
                    dsp_latency = Long.parseLong(dsp_latency_et.getText().toString());
                }
                Log.e(this.toString(),"DSP latency is"+ dsp_latency);

                cold_play_driver = (cold_play_driver_sec*1000) +( cold_play_driver_usec/1000);
                Log.e(this.toString(),"value of cold play driver  is"+ cold_play_driver);
                if (cold_play_driver-cold_play_app>0) {
                    cold_play_latency= (cold_play_driver-cold_play_app)+dsp_latency;
                    Log.e(this.toString(),"value of cold output latency is"+ cold_play_latency);
                }

                warm_play_driver = (warm_play_driver_sec*1000) +( warm_play_driver_usec/1000);
                Log.e(this.toString(),"value of warm play driver  is"+ warm_play_driver);
                if (warm_play_driver-warm_play_app>0) {
                    warm_play_latency= warm_play_driver-warm_play_app+dsp_latency;
                    Log.e(this.toString(),"value of warm output latency is"+ warm_play_latency);
                }

                cont_play_driver = (cont_play_driver_sec*1000) +( cont_play_driver_usec/1000);
                Log.e(this.toString(),"value of cont play driver  is"+ cont_play_driver);
                if (cont_play_driver-cont_play_app>0) {
                    cont_play_latency= cont_play_driver-cont_play_app+dsp_latency;
                    Log.e(this.toString(),"value of cont output latency is"+ cont_play_latency);
                }

                cold_rec_latency = cold_rec_app;
                Log.e(this.toString(),"value of cold input latency is"+ cold_rec_latency);

                cont_rec_driver = (cont_rec_driver_sec*1000) +( cont_rec_driver_usec/1000);
                Log.e(this.toString(),"value of cont_rec_driver is " +cont_rec_driver);
                if (cont_rec_app-cont_rec_driver>0) {
                    cont_rec_latency =cont_rec_app-cont_rec_driver;
                    Log.e(this.toString(),"value of cont input latency is"+ cont_rec_latency);
                }

                cold_play_tv.setText("Cold Output Latency :"+String.valueOf(cold_play_latency));
                warm_play_tv.setText("Warm Output Latency :"+String.valueOf(warm_play_latency));
                cont_play_tv.setText("Cont Output Latency :"+String.valueOf(cont_play_latency));
                cold_rec_tv.setText("Cold Input Latency :"+String.valueOf(cold_rec_latency));
                cont_rec_tv.setText("Cont Input Latency :"+String.valueOf(cont_rec_latency));
            }
        });

        ((Button) findViewById(R.id.clear)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                cold_play_latency=0;
                warm_play_latency=0;
                cont_play_latency=0;
                cold_rec_latency=0;
                cont_rec_latency=0;

                cold_play_tv.setText("Cold Output Latency :"+0);
                warm_play_tv.setText("Warm Output Latency :"+0);
                cont_play_tv.setText("Cont Output Latency :"+0);
                cold_rec_tv.setText("Cold Input Latency :"+0);
                cont_rec_tv.setText("Cont Input Latency :"+0);
            }
        });
    }

    protected void onStop() {
        Log.e(this.toString(), "onStop");
        super.onStop();
        created = false;
        try {
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_out_latency_measurement_node");
            fw_enable_latency.write("0");
            fw_enable_latency.flush();
            fw_enable_latency.close();
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_in_latency_measurement_node");
            fw_enable_latency.write("0");
            fw_enable_latency.flush();
            fw_enable_latency.close();
            //To Enable this break down latency we need to port the abandoned gerrit # 97027 to latest kernel version for 8960 and 8974
            /*fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_out_latency_measurement_breakdown_node");
            fw_enable_latency.write("0");
            fw_enable_latency.flush();
            fw_enable_latency.close();
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_in_latency_measurement_breakdown_node");
            fw_enable_latency.write("0");
            fw_enable_latency.flush();
            fw_enable_latency.close();
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_adm_out_latency_measurement_node");
            fw_enable_latency.write("0");
            fw_enable_latency.flush();
            fw_enable_latency.close();
            fw_enable_latency = new FileWriter("/sys/kernel/debug/audio_adm_in_latency_measurement_node");
            fw_enable_latency.write("0");
            fw_enable_latency.flush();
            fw_enable_latency.close();*/
        }
        catch(Exception e) {
            Log.e(this.toString(),"Exception in file writer operations");
    }
    }


    /** Called when the activity is about to be destroyed. */
    @Override
    protected void onDestroy() {
        shutdown();
        super.onDestroy();
    }

    /** Native methods, implemented in jni folder */
    public static native void createEngine();
    public static native void createBufferQueueAudioPlayer();
    public static native boolean selectClip(int which, int count);
    public static native boolean createAudioRecorder(int latency);
    public static native void startRecording(int latency);
    public static native void shutdown();
    public static native long getColdOutputLatency();
    public static native long getWarmOutputLatency();
    public static native long getContinuousOutputLatency();
    public static native long getColdInputLatency();
    public static native long getContinuousInputLatency();
    public static native boolean isRecorderCreated();
    public static native void init();

    /** Load jni .so on initialization */
    static {
         System.loadLibrary("native_audio_latency_jni");
    }
}

