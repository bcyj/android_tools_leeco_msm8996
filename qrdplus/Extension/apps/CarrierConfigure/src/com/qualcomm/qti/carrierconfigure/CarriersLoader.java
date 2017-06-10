/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.content.AsyncTaskLoader;
import android.content.Context;
import android.os.RemoteException;
import android.util.Log;

import com.qualcomm.qti.carrierconfigure.Carrier.CarriersStorage;
import com.qualcomm.qti.loadcarrier.ILoadCarrierService;

import java.util.HashMap;

public class CarriersLoader extends AsyncTaskLoader<CarriersStorage> {
    // Save the result.
    private CarriersStorage mCarriersStorage = null;

    // Save the state.
    private boolean mParsedPreset = false;
    private boolean mParsedSdCard = false;

    private ILoadCarrierService mService;

    public CarriersLoader(Context context, ILoadCarrierService service) {
        super(context);
        mService = service;
    }

    @Override
    protected void onReset() {
        super.onReset();
        if (mCarriersStorage != null) mCarriersStorage.reset();
        mParsedPreset = false;
        mParsedSdCard = false;
    }

    @Override
    protected void onStartLoading() {
        if (mParsedPreset && (mService == null || mParsedSdCard)) {
            // Have a result available, deliver it immediately.
            deliverResult(mCarriersStorage);
        }

        if (takeContentChanged() || !mParsedPreset || (mService != null && !mParsedSdCard)) {
            forceLoad();
        }
    }

    @SuppressWarnings("unchecked")
    @Override
    public CarriersStorage loadInBackground() {
        if (mCarriersStorage == null) {
            mCarriersStorage = new CarriersStorage();
        }

        // Load the preset carriers, and we will load the carriers on the SD card if we get the
        // service changed notify. Then restart loading.
        if (!mParsedPreset) {
            Carrier.parsePresetCarriers(mCarriersStorage);
            mParsedPreset = true;
        }
        if (!mParsedSdCard && mService != null) {
            // Get the carrier list from SD card maybe expend most time, we will deliver
            // the result first. After get parse finish, it will deliver the result again.
            deliverResult(mCarriersStorage);

            try {
                Carrier.parseCarriers(
                        (HashMap<String, String>) mService.getCarrierList(), mCarriersStorage);
                mParsedSdCard = true;
            } catch (RemoteException e) {
                Log.e("CarriersLoader", "Found the LoadCarrierService, but catch RemoteException: "
                        + e.getMessage());
            }
        }

        return mCarriersStorage;
    }

    public void notifyCarrierLoadServiceChanged(ILoadCarrierService service) {
        // Update the service value.
        mService = service;

        // Start loading process.
        startLoading();
    }
}
