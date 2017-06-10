/*
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */
package com.android.profile;

import static com.android.profile.ProfileConst.ENABLE_TAB_RINGTONE_SETTING;
import static com.android.profile.ProfileUtils.saveStringValue;
import android.content.Context;
import android.content.Intent;
import android.media.RingtoneManager;
import android.net.Uri;
import android.preference.RingtonePreference;
import android.util.AttributeSet;

public class ProfileRingtonePreference extends RingtonePreference {

    private Context mContext=null;
    private int slotId = 0;
    private String ringtone = null;
    private String[] key = {
            "ringtone1", "ringtone2"
    };

    public ProfileRingtonePreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext=context;
    }

    public String getRingtone() {

        return ringtone;
    }

    public void setRingtone(String ringtone) {
    
        this.ringtone = ringtone;
    }

    public int getSlotId() {
    
        return slotId;
    }

    public void setSlotId(int slotId) {
    
        this.slotId = slotId;
    }

    @Override
    protected void onPrepareRingtonePickerIntent(final Intent ringtonePickerIntent) {
        super.onPrepareRingtonePickerIntent(ringtonePickerIntent);
        ringtonePickerIntent.putExtra(RingtoneManager.EXTRA_RINGTONE_SHOW_DEFAULT, false);
    }
    @Override
    protected void onSaveRingtone(Uri ringtoneUri) {

        if (ringtoneUri != null) {
            ringtone = ringtoneUri.toString();
        } else
            ringtone = null;
        if(ENABLE_TAB_RINGTONE_SETTING)
            saveStringValue(mContext, key[slotId], ringtone);

    }

    @Override
    protected Uri onRestoreRingtone() {

        return ringtone != null ? Uri.parse(ringtone) : null;
    }

}
