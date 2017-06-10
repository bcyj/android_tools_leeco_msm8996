/*
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */
package com.android.profile;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.RelativeLayout;

public class ProfileItemView extends RelativeLayout {

    public static final int STATE_CURRENT_PROFILE = -1;
    public static final int STATE_SAVED_PROFILE = 0;
    public static final int ID_CURRENT_PROFILE = -1;

    private int state = STATE_SAVED_PROFILE;
    private int id = ID_CURRENT_PROFILE;

    public ProfileItemView(Context context, AttributeSet attrs) {

        super(context, attrs);
    }
    
    public ProfileItemView(Context context) {

        super(context, null);
    }

    public int getId() {

        return id;
    }

    public void setId(int id) {

        this.id = id;
    }

    public int getState() {

        return state;
    }

    public void setState(int state) {

        this.state = state;
    }

}
