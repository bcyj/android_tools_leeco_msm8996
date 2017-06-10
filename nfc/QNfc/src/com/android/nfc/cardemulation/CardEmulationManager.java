/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2014 The Android Open Source Project
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
package com.android.nfc.cardemulation;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.List;

import com.android.nfc.Debug;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.nfc.INfcCardEmulation;
import android.nfc.cardemulation.AidGroup;
import android.nfc.cardemulation.ApduServiceInfo;
import android.nfc.cardemulation.CardEmulation;
import android.os.Binder;
import android.os.RemoteException;
import android.os.UserHandle;
import android.provider.Settings;
import android.util.Log;

import com.android.nfc.NfcPermissions;
import com.android.nfc.cardemulation.RegisteredServicesCache;

/**
 * CardEmulationManager is the central entity
 * responsible for delegating to individual components
 * implementing card emulation:
 * - RegisteredServicesCache keeping track of HCE and SE services on the device
 * - RegisteredAidCache keeping track of AIDs registered by those services and manages
 *   the routing table in the NFCC.
 * - HostEmulationManager handles incoming APDUs for the host and forwards to HCE
 *   services as necessary.
 */
public class CardEmulationManager implements RegisteredServicesCache.Callback,
        PreferredServices.Callback {
    static final String TAG = "CardEmulationManager";
    static final boolean DBG = Debug.CardEmulationManager;

    final RegisteredAidCache mAidCache;
    final RegisteredServicesCache mServiceCache;
    final HostEmulationManager mHostEmulationManager;
    final PreferredServices mPreferredServices;
    final Context mContext;
    final CardEmulationInterface mCardEmulationInterface;

// QNCI - BEGIN
    final MultiSeManager mMultiSeManager;
// QNCI - END

    public CardEmulationManager(Context context) {
        mContext = context;
        mCardEmulationInterface = new CardEmulationInterface();
        mServiceCache = new RegisteredServicesCache(context, this);
        mAidCache = new RegisteredAidCache(context, mServiceCache);
        mHostEmulationManager = new HostEmulationManager(context, mAidCache);
        mPreferredServices = new PreferredServices(context, mServiceCache, mAidCache, this);
// QNCI - BEGIN
        mMultiSeManager = new MultiSeManager(context, this);
// QNCI - END
        mServiceCache.initialize();
    }

    public INfcCardEmulation getNfcCardEmulationInterface() {
        return mCardEmulationInterface;
    }

    public void onHostCardEmulationActivated() {
        mHostEmulationManager.onHostEmulationActivated();
        mPreferredServices.onHostEmulationActivated();
    }

    public void onHostCardEmulationData(byte[] data) {
        mHostEmulationManager.onHostEmulationData(data);
    }

    public void onHostCardEmulationDeactivated() {
        mHostEmulationManager.onHostEmulationDeactivated();
        mPreferredServices.onHostEmulationDeactivated();
    }

    public void onOffHostAidSelected() {
        mHostEmulationManager.onOffHostAidSelected();
    }

    public void onUserSwitched(int userId) {
        mServiceCache.invalidateCache(userId);
        mPreferredServices.onUserSwitched(userId);
    }

    public void onNfcEnabled() {
        mAidCache.onNfcEnabled();
    }

    public void onNfcDisabled() {
        mAidCache.onNfcDisabled();
    }

    public void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        mServiceCache.dump(fd, pw, args);
        mPreferredServices.dump(fd, pw, args);
        mAidCache.dump(fd, pw, args);
        mHostEmulationManager.dump(fd, pw, args);
    }

    public int getNumSecureElement() {
        return mAidCache.getNumSecureElement();
    }

    public boolean hasAnyServices() {
        List<ApduServiceInfo> services;
        services = mServiceCache.getServices(UserHandle.getCallingUserId());

        if ((services != null)&&(!services.isEmpty())) {
            Log.d(TAG, "hasAnyServices return true" );
            return true;
        }

        Log.d(TAG, "hasAnyServices return false" );
        return false;
    }

    // Update ISO-DEP protocol routing by API calling
    public void setDefaultRoute (String seName) {
        mAidCache.setDefaultRoute(seName);
    }

    @Override
    public void onServicesUpdated(int userId, List<ApduServiceInfo> services) {
        // Verify defaults are still sane
        verifyDefaults(userId, services);
        // Update the AID cache
        mAidCache.onServicesUpdated(userId, services);
        // Update the preferred services list
        mPreferredServices.onServicesUpdated();
    }

    void verifyDefaults(int userId, List<ApduServiceInfo> services) {
        ComponentName defaultPaymentService =
                getDefaultServiceForCategory(userId, CardEmulation.CATEGORY_PAYMENT, false);
        if (DBG) Log.d(TAG, "Current default: " + defaultPaymentService);
        if (defaultPaymentService != null) {
            // Validate the default is still installed and handling payment
            ApduServiceInfo serviceInfo = mServiceCache.getService(userId, defaultPaymentService);
            if (serviceInfo == null || !serviceInfo.hasCategory(CardEmulation.CATEGORY_PAYMENT)) {
                if (serviceInfo == null) {
                    Log.e(TAG, "Default payment service unexpectedly removed.");
                } else if (!serviceInfo.hasCategory(CardEmulation.CATEGORY_PAYMENT)) {
                    if (DBG) Log.d(TAG, "Default payment service had payment category removed");
                }
                int numPaymentServices = 0;
                ComponentName lastFoundPaymentService = null;
                for (ApduServiceInfo service : services) {
                    if (service.hasCategory(CardEmulation.CATEGORY_PAYMENT))  {
                        numPaymentServices++;
                        lastFoundPaymentService = service.getComponent();
                    }
                }
                if (DBG) Log.d(TAG, "Number of payment services is " +
                        Integer.toString(numPaymentServices));
                if (numPaymentServices == 0) {
                    if (DBG) Log.d(TAG, "Default removed, no services left.");
                    // No payment services left, unset default and don't ask the user
                    setDefaultServiceForCategoryChecked(userId, null, CardEmulation.CATEGORY_PAYMENT);
                } else if (numPaymentServices == 1) {
                    // Only one left, automatically make it the default
                    if (DBG) Log.d(TAG, "Default removed, making remaining service default.");
                    setDefaultServiceForCategoryChecked(userId, lastFoundPaymentService,
                            CardEmulation.CATEGORY_PAYMENT);
                } else if (numPaymentServices > 1) {
                    // More than one left, unset default and ask the user if he wants
                    // to set a new one
                    if (DBG) Log.d(TAG, "Default removed, asking user to pick.");
                    setDefaultServiceForCategoryChecked(userId, null,
                            CardEmulation.CATEGORY_PAYMENT);
                    Intent intent = new Intent(mContext, DefaultRemovedActivity.class);
                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mContext.startActivityAsUser(intent, UserHandle.CURRENT);
                }
            } else {
                // Default still exists and handles the category, nothing do
                if (DBG) Log.d(TAG, "Default payment service still ok.");
            }
        } else {
            // A payment service may have been removed, leaving only one;
            // in that case, automatically set that app as default.
            int numPaymentServices = 0;
            ComponentName lastFoundPaymentService = null;
            for (ApduServiceInfo service : services) {
                if (service.hasCategory(CardEmulation.CATEGORY_PAYMENT))  {
                    numPaymentServices++;
                    lastFoundPaymentService = service.getComponent();
                }
            }
            if (numPaymentServices > 1) {
                // More than one service left, leave default unset
                if (DBG) Log.d(TAG, "No default set, more than one service left.");
            } else if (numPaymentServices == 1) {
                // Make single found payment service the default
                if (DBG) Log.d(TAG, "No default set, making single service default.");
                setDefaultServiceForCategoryChecked(userId, lastFoundPaymentService,
                        CardEmulation.CATEGORY_PAYMENT);
            } else {
                // No payment services left, leave default at null
                if (DBG) Log.d(TAG, "No default set, last payment service removed.");
            }
        }
    }

    ComponentName getDefaultServiceForCategory(int userId, String category,
             boolean validateInstalled) {
        if (!CardEmulation.CATEGORY_PAYMENT.equals(category)) {
            Log.e(TAG, "Not allowing defaults for category " + category);
            return null;
        }
        // Load current payment default from settings
        String name = Settings.Secure.getStringForUser(
                mContext.getContentResolver(), Settings.Secure.NFC_PAYMENT_DEFAULT_COMPONENT,
                userId);
        if (name != null) {
            ComponentName service = ComponentName.unflattenFromString(name);
            if (!validateInstalled || service == null) {
                return service;
            } else {
                return mServiceCache.hasService(userId, service) ? service : null;
             }
        } else {
            return null;
        }
    }

    boolean setDefaultServiceForCategoryChecked(int userId, ComponentName service,
            String category) {
        if (!CardEmulation.CATEGORY_PAYMENT.equals(category)) {
            Log.e(TAG, "Not allowing defaults for category " + category);
            return false;
        }
        // TODO Not really nice to be writing to Settings.Secure here...
        // ideally we overlay our local changes over whatever is in
        // Settings.Secure
        if (service == null || mServiceCache.hasService(userId, service)) {
            Settings.Secure.putStringForUser(mContext.getContentResolver(),
                    Settings.Secure.NFC_PAYMENT_DEFAULT_COMPONENT,
                    service != null ? service.flattenToString() : null, userId);
        } else {
            Log.e(TAG, "Could not find default service to make default: " + service);
        }
        return true;
    }

    boolean isServiceRegistered(int userId, ComponentName service) {
        boolean serviceFound = mServiceCache.hasService(userId, service);
        if (!serviceFound) {
            // If we don't know about this service yet, it may have just been enabled
            // using PackageManager.setComponentEnabledSetting(). The PackageManager
            // broadcasts are delayed by 10 seconds in that scenario, which causes
            // calls to our APIs referencing that service to fail.
            // Hence, update the cache in case we don't know about the service.
            if (DBG) Log.d(TAG, "Didn't find passed in service, invalidating cache.");
            mServiceCache.invalidateCache(userId);
        }
        return mServiceCache.hasService(userId, service);
    }

    /**
     * This class implements the application-facing APIs
     * and are called from binder. All calls must be
     * permission-checked.
     *
     */
    final class CardEmulationInterface extends INfcCardEmulation.Stub {
        @Override
        public boolean isDefaultServiceForCategory(int userId, ComponentName service,
                String category) {
            NfcPermissions.enforceUserPermissions(mContext);
            NfcPermissions.validateUserId(userId);
            if (!isServiceRegistered(userId, service)) {
                return false;
            }
            ComponentName defaultService =
                    getDefaultServiceForCategory(userId, category, true);
            return (defaultService != null && defaultService.equals(service));
        }

        @Override
        public boolean isDefaultServiceForAid(int userId,
                ComponentName service, String aid) throws RemoteException {
            NfcPermissions.validateUserId(userId);
            NfcPermissions.enforceUserPermissions(mContext);
            if (!isServiceRegistered(userId, service)) {
                return false;
            }
            return mAidCache.isDefaultServiceForAid(userId, service, aid);
        }

        @Override
        public boolean setDefaultServiceForCategory(int userId,
                ComponentName service, String category) throws RemoteException {
            NfcPermissions.validateUserId(userId);
            NfcPermissions.enforceAdminPermissions(mContext);
            if (!isServiceRegistered(userId, service)) {
                return false;
            }
            return setDefaultServiceForCategoryChecked(userId, service, category);
        }

        @Override
        public boolean setDefaultForNextTap(int userId, ComponentName service)
                throws RemoteException {
            NfcPermissions.validateUserId(userId);
            NfcPermissions.enforceAdminPermissions(mContext);
            if (!isServiceRegistered(userId, service)) {
                return false;
            }
            return mPreferredServices.setDefaultForNextTap(service);
        }

        @Override
        public boolean registerAidGroupForService(int userId,
                ComponentName service, AidGroup aidGroup) throws RemoteException {
            NfcPermissions.validateUserId(userId);
            NfcPermissions.enforceUserPermissions(mContext);
            if (!isServiceRegistered(userId, service)) {
                return false;
            }
            return mServiceCache.registerAidGroupForService(userId, Binder.getCallingUid(), service,
                    aidGroup);
        }

        @Override
        public AidGroup getAidGroupForService(int userId,
                ComponentName service, String category) throws RemoteException {
            NfcPermissions.validateUserId(userId);
            NfcPermissions.enforceUserPermissions(mContext);
            if (!isServiceRegistered(userId, service)) {
                return null;
            }
            return mServiceCache.getAidGroupForService(userId, Binder.getCallingUid(), service,
                    category);
        }

        @Override
        public boolean removeAidGroupForService(int userId,
                ComponentName service, String category) throws RemoteException {
            NfcPermissions.validateUserId(userId);
            NfcPermissions.enforceUserPermissions(mContext);
            if (!isServiceRegistered(userId, service)) {
                return false;
            }
            return mServiceCache.removeAidGroupForService(userId, Binder.getCallingUid(), service,
                    category);
        }

        @Override
        public List<ApduServiceInfo> getServices(int userId, String category)
                throws RemoteException {
            NfcPermissions.validateUserId(userId);
            NfcPermissions.enforceAdminPermissions(mContext);
            return mServiceCache.getServicesForCategory(userId, category);
        }

        @Override
        public boolean setPreferredService(ComponentName service)
                throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);
            if (!isServiceRegistered(UserHandle.getCallingUserId(), service)) {
                Log.e(TAG, "setPreferredService: unknown component.");
                return false;
            }
            return mPreferredServices.registerPreferredForegroundService(service,
                    Binder.getCallingUid());
        }

        @Override
        public boolean unsetPreferredService() throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);
            return mPreferredServices.unregisteredPreferredForegroundService(
                    Binder.getCallingUid());

        }

        @Override
        public boolean supportsAidPrefixRegistration() throws RemoteException {
            return mAidCache.supportsAidPrefixRegistration();
        }
    }

    @Override
    public void onPreferredPaymentServiceChanged(ComponentName service) {
        mAidCache.onPreferredPaymentServiceChanged(service);
        mHostEmulationManager.onPreferredPaymentServiceChanged(service);
    }

    @Override
    public void onPreferredForegroundServiceChanged(ComponentName service) {
        mAidCache.onPreferredForegroundServiceChanged(service);
        mHostEmulationManager.onPreferredForegroundServiceChanged(service);
    };
}
