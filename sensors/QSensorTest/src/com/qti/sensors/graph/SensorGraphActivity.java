/*============================================================================
@file SensorGraphActivity.java

@brief
Activity used to graph sensor data and provide multiple-client scenarios.

Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
package com.qti.sensors.graph;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.qualcomm.sensors.qsensortest.R;

public class SensorGraphActivity extends Fragment{
   private View view;

   /**
    * Called when the activity is first created.
    */
   @Override
   public void onActivityCreated(Bundle savedInstanceState) {
      super.onCreate(savedInstanceState);
   }

   @Override
   public View onCreateView (LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
      if(null == this.view){
         this.view = inflater.inflate(R.layout.activity_graph_layout, null);

         // TODO: Implement Graph tab
         TextView sensorsInfo = (TextView) view.findViewById(R.id.sensor_graph_temp);
         sensorsInfo.setText("TODO");
      }

      return this.view;
   }
}
