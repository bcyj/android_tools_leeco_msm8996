/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.qti.telephony.extcarrierpack;

import android.os.Handler;
import android.os.SystemProperties;

import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccCardConstants.State;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.os.AsyncResult;
import android.text.TextUtils;
import android.util.Log;



public class CarrierpackApplication extends Application{

    public static final String TAG = CarrierpackApplication.class.getSimpleName();
    private static final boolean DBG = Log.isLoggable(CarrierpackApplication.TAG, Log.DEBUG);

    //handler constants
    private final int EVENT_PERSO_LOCKED = 1;
    private Phone phone;
    private static CarrierpackApplication mInstance = null;

    Handler mHandler = new Handler(){
        public void handleMessage(android.os.Message msg) {
            switch(msg.what) {
            case EVENT_PERSO_LOCKED:
                log("EVENT_PERSO_LOCKED");
                Intent intent = new Intent("org.codeaurora.carrier.ACTION_DEPERSO_PANEL");
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.putExtra("PersoSubtype", (Integer)((AsyncResult)msg.obj).result);
                startActivity(intent);
                break;
            }
        };
    };

    public static CarrierpackApplication getInstance() {
        return mInstance;
    }

    @Override
    public void onCreate() {
        log("application oncreate");
        mInstance = this;
        phone = PhoneFactory.getDefaultPhone();

        if(!isDePersonalizationEnabled()) {
            registerNpLock();
        }
    }

    public boolean isPersoLocked() {
        IccCard sim = phone.getIccCard();
        return sim.getState() == State.PERSO_LOCKED;
    }

    private void registerNpLock() {
        IccCard sim = phone.getIccCard();
        if (sim != null) {
            log( "register for ICC status");
            sim.registerForPersoLocked(mHandler, EVENT_PERSO_LOCKED, null);
        }
    }

    public boolean isDePersonalizationEnabled() {
        Resources res = null;
        try{
            res = getPackageManager().getResourcesForApplication("com.android.phone");
        }catch(NameNotFoundException e) {
            e.printStackTrace();
        }

        int resourceId = 0;
        if(res != null) {
            resourceId = res.getIdentifier("com.android.phone:"
                    + "bool/icc_depersonalizationPanelEnabled", null, null);
        }

        boolean depersoEnabled = true;
        if(0 != resourceId) {
            CharSequence s = getPackageManager().getText("com.android.phone", resourceId, null);
            log("resource=" + s);
            depersoEnabled = Boolean.valueOf(s.toString());
        } else {
             log("icc_depersonalizationPanelEnabled bool config is not present in "
                     + "com.android.phone. Plz include config and set to false to disable "
                     + " showing NP lock as dialog" );
        }
        return depersoEnabled;
    }

    public void log(String str) {
        if(DBG) {
            Log.d(TAG, str);
        }
    }
}
