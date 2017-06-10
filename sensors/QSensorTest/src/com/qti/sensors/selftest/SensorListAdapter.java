/*============================================================================
@file SensorListAdapter.java

@brief
An adapter that bridges what the user sees on the list of sensors,
and the sensors that actually back the list.

Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

package com.qti.sensors.selftest;
import java.util.List;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

/**
 * An adapter that bridges what the user sees on the list of sensors,
 * and the sensors that actually back the list.
 *
 */
public class SensorListAdapter extends BaseAdapter {
    private List<SensorStatus> sensorList;
    private Context context;

    public SensorListAdapter(Context context, List<SensorStatus> sensorList){
        this.context = context;
        this.sensorList = sensorList;
    }

    @Override
    public int getCount() { return sensorList.size(); }
    @Override
    public Object getItem(int position) { return sensorList.get(position); }
    @Override
    public long getItemId(int position) { return position; }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
       SensorListItemView sv;
       SensorStatus testSensor = this.sensorList.get(position);

       if(convertView == null) {
          sv = new SensorListItemView(this.context, testSensor);
       }
       else {
          sv = (SensorListItemView) convertView;
       }

       return sv;
    }
}
