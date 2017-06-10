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

import com.android.nfc.Debug;
import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.nfc.cardemulation.ApduServiceInfo;
import android.nfc.cardemulation.CardEmulation;
import com.android.nfc.QSecureElementManager;
import android.util.Log;

import com.google.android.collect.Maps;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;
import java.util.PriorityQueue;
import java.util.TreeMap;

public class RegisteredAidCache {
    static final String TAG = "RegisteredAidCache";

    static final boolean DBG = Debug.RegisteredAidCache;
    static final String PPSE_AID = "325041592E5359532E4444463031";

    // mAidServices maps AIDs to services that have registered them.
    // It's a TreeMap in order to be able to quickly select subsets
    // of AIDs that conflict with each other.
    final TreeMap<String, ArrayList<ServiceAidInfo>> mAidServices =
            new TreeMap<String, ArrayList<ServiceAidInfo>>();

    // mAidCache is a lookup table for quickly mapping an exact or prefix AID to one or
    // more handling services. It differs from mAidServices in the sense that it
    // has already accounted for defaults, and hence its return value
    // is authoritative for the current set of services and defaults.
    // It is only valid for the current user.
    final TreeMap<String, AidResolveInfo> mAidCache = new TreeMap<String, AidResolveInfo>();

    // Represents a single AID registration of a service
    final class ServiceAidInfo {
        ApduServiceInfo service;
        String aid;
        String category;

        @Override
        public String toString() {
            return "ServiceAidInfo{" +
                    "service=" + service.getComponent() +
                    ", aid='" + aid + '\'' +
                    ", category='" + category + '\'' +
                    '}';
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;

            ServiceAidInfo that = (ServiceAidInfo) o;

            if (!aid.equals(that.aid)) return false;
            if (!category.equals(that.category)) return false;
            if (!service.equals(that.service)) return false;

            return true;
        }

        @Override
        public int hashCode() {
            int result = service.hashCode();
            result = 31 * result + aid.hashCode();
            result = 31 * result + category.hashCode();
            return result;
        }
    }

    // Represents a list of services, an optional default and a category that
    // an AID was resolved to.
    final class AidResolveInfo {
        List<ApduServiceInfo> services = new ArrayList<ApduServiceInfo>();
        ApduServiceInfo defaultService = null;
        String category = null;
        boolean mustRoute = true; // Whether this AID should be routed at all

        @Override
        public String toString() {
            return "AidResolveInfo{" +
                    "services=" + services +
                    ", defaultService=" + defaultService +
                    ", category='" + category + '\'' +
                    ", mustRoute=" + mustRoute +
                    '}';
        }
    }

    final AidResolveInfo EMPTY_RESOLVE_INFO = new AidResolveInfo();

    final Context mContext;
    final AidRoutingManager mRoutingManager;
    final RegisteredServicesCache mServiceCache;

    final Object mLock = new Object();

    ComponentName mPreferredPaymentService;
    ComponentName mPreferredForegroundService;

    boolean mNfcEnabled = false;
    boolean mSupportsPrefixes = false;

    public RegisteredAidCache(Context context, RegisteredServicesCache serviceCache) {
        mContext = context;
        mRoutingManager = new AidRoutingManager();
        mPreferredPaymentService = null;
        mPreferredForegroundService = null;
        mSupportsPrefixes = mRoutingManager.supportsAidPrefixRouting();
        if (mSupportsPrefixes) {
            if (DBG) Log.d(TAG, "Controller supports AID prefix routing");
        }
        mServiceCache = serviceCache;
    }

    public int getNumSecureElement() {
        return mRoutingManager.getNumSecureElement();
    }

    public AidResolveInfo resolveAid(String aid) {
        synchronized (mLock) {
            if (DBG) Log.d(TAG, "resolveAid: resolving AID " + aid);
            if (aid.length() < 10) {
                Log.e(TAG, "AID selected with fewer than 5 bytes.");
                return EMPTY_RESOLVE_INFO;
            }
            AidResolveInfo resolveInfo = new AidResolveInfo();
            if (mSupportsPrefixes) {
                // Our AID cache may contain prefixes which also match this AID,
                // so we must find all potential prefixes and merge the ResolveInfo
                // of those prefixes plus any exact match in a single result.
                String shortestAidMatch = aid.substring(0, 10); // Minimum AID length is 5 bytes
                String longestAidMatch = aid + "*"; // Longest potential matching AID

                if (DBG) Log.d(TAG, "Finding AID registrations in range [" + shortestAidMatch +
                        " - " + longestAidMatch + "]");
                NavigableMap<String, AidResolveInfo> matchingAids =
                        mAidCache.subMap(shortestAidMatch, true, longestAidMatch, true);

                resolveInfo.category = CardEmulation.CATEGORY_OTHER;
                for (Map.Entry<String, AidResolveInfo> entry : matchingAids.entrySet()) {
                    boolean isPrefix = isPrefix(entry.getKey());
                    String entryAid = isPrefix ? entry.getKey().substring(0,
                            entry.getKey().length() - 1) : entry.getKey(); // Cut off '*' if prefix
                    if (entryAid.equalsIgnoreCase(aid) || (isPrefix && aid.startsWith(entryAid))) {
                        if (DBG) Log.d(TAG, "resolveAid: AID " + entryAid + " matches.");
                        AidResolveInfo entryResolveInfo = entry.getValue();
                        if (entryResolveInfo.defaultService != null) {
                            if (resolveInfo.defaultService != null) {
                                // This shouldn't happen; for every prefix we have only one
                                // default service.
                                Log.e(TAG, "Different defaults for conflicting AIDs!");
                            }
                            resolveInfo.defaultService = entryResolveInfo.defaultService;
                            resolveInfo.category = entryResolveInfo.category;
                        }
                        for (ApduServiceInfo serviceInfo : entryResolveInfo.services) {
                            if (!resolveInfo.services.contains(serviceInfo)) {
                                resolveInfo.services.add(serviceInfo);
                            }
                        }
                    }
                }
            } else {
                resolveInfo = mAidCache.get(aid);
            }
            if (DBG) Log.d(TAG, "Resolved to: " + resolveInfo);
            return resolveInfo;
        }
    }

    public boolean supportsAidPrefixRegistration() {
        return mSupportsPrefixes;
    }

    public boolean isDefaultServiceForAid(int userId, ComponentName service, String aid) {
        AidResolveInfo resolveInfo = resolveAid(aid);
        if (resolveInfo == null || resolveInfo.services == null ||
                resolveInfo.services.size() == 0) {
            return false;
        }

        if (resolveInfo.defaultService != null) {
            return service.equals(resolveInfo.defaultService.getComponent());
        } else if (resolveInfo.services.size() == 1) {
            return service.equals(resolveInfo.services.get(0).getComponent());
        } else {
            // More than one service, not the default
            return false;
        }
    }

    /**
     * Resolves a conflict between multiple services handling the same
     * AIDs. Note that the AID itself is not an input to the decision
     * process - the algorithm just looks at the competing services
     * and what preferences the user has indicated. In short, it works like
     * this:
     *
     * 1) If there is a preferred foreground service, that service wins
     * 2) Else, if there is a preferred payment service, that service wins
     * 3) Else, if there is no winner, and all conflicting services will be
     *    in the list of resolved services.
     */
     AidResolveInfo resolveAidConflictLocked(Collection<ServiceAidInfo> conflictingServices,
                                             boolean makeSingleServiceDefault) {
        if (conflictingServices == null || conflictingServices.size() == 0) {
            Log.e(TAG, "resolveAidConflict: No services passed in.");
            return null;
        }
        AidResolveInfo resolveInfo = new AidResolveInfo();
        resolveInfo.category = CardEmulation.CATEGORY_OTHER;

        ApduServiceInfo matchedForeground = null;
        ApduServiceInfo matchedPayment = null;
        for (ServiceAidInfo serviceAidInfo : conflictingServices) {
            boolean serviceClaimsPaymentAid =
                    CardEmulation.CATEGORY_PAYMENT.equals(serviceAidInfo.category);
            if (serviceAidInfo.service.getComponent().equals(mPreferredForegroundService)) {
                resolveInfo.services.add(serviceAidInfo.service);
                if (serviceClaimsPaymentAid) {
                    resolveInfo.category = CardEmulation.CATEGORY_PAYMENT;
                }
                matchedForeground = serviceAidInfo.service;
            } else if (serviceAidInfo.service.getComponent().equals(mPreferredPaymentService) &&
                    serviceClaimsPaymentAid) {
                resolveInfo.services.add(serviceAidInfo.service);
                resolveInfo.category = CardEmulation.CATEGORY_PAYMENT;
                matchedPayment = serviceAidInfo.service;
            } else {
                if (serviceClaimsPaymentAid) {
                    // if no preferred service is selected and more than one service registered PPSE AID
                    // then let user select one of services
                    if ((mPreferredForegroundService == null) &&
                        (mPreferredPaymentService == null) &&
                        (conflictingServices.size() > 1) &&
                        (serviceAidInfo.aid.equals(PPSE_AID))) {
                        if (DBG) Log.d(TAG, "No preferred service but it's PPSE, add service to ask user");
                        resolveInfo.services.add(serviceAidInfo.service);
                    } else {
                        // If this service claims it's a payment AID, don't route it,
                        // because it's not the default. Otherwise, add it to the list
                        // but not as default.
                        if (DBG) Log.d(TAG, "resolveAidLocked: (Ignoring handling service " +
                                serviceAidInfo.service.getComponent() +
                                " because it's not the payment default.)");
                    }
                } else {
                    resolveInfo.services.add(serviceAidInfo.service);
                }
            }
        }
        if (matchedForeground != null) {
            // 1st priority: if the foreground app prefers a service,
            // and that service asks for the AID, that service gets it
            if (DBG) Log.d(TAG, "resolveAidLocked: DECISION: routing to foreground preferred " +
                    matchedForeground);
            resolveInfo.defaultService = matchedForeground;
        } else if (matchedPayment != null) {
            // 2nd priority: if there is a preferred payment service,
            // and that service claims this as a payment AID, that service gets it
            if (DBG) Log.d(TAG, "resolveAidLocked: DECISION: routing to payment default " +
                    "default " + matchedPayment);
            resolveInfo.defaultService = matchedPayment;
        } else {
            if (resolveInfo.services.size() == 1 && makeSingleServiceDefault) {
                if (DBG) Log.d(TAG, "resolveAidLocked: DECISION: making single handling service " +
                        resolveInfo.services.get(0).getComponent() + " default.");
                resolveInfo.defaultService = resolveInfo.services.get(0);
            } else {
                // Nothing to do, all services already in list
                if (DBG) Log.d(TAG, "resolveAidLocked: DECISION: routing to all matching services");
            }
        }
        return resolveInfo;
    }

    class DefaultServiceInfo {
        ServiceAidInfo paymentDefault;
        ServiceAidInfo foregroundDefault;
    }

    DefaultServiceInfo findDefaultServices(ArrayList<ServiceAidInfo> serviceAidInfos) {
        DefaultServiceInfo defaultServiceInfo = new DefaultServiceInfo();

        for (ServiceAidInfo serviceAidInfo : serviceAidInfos) {
            boolean serviceClaimsPaymentAid =
                    CardEmulation.CATEGORY_PAYMENT.equals(serviceAidInfo.category);
            if (serviceAidInfo.service.getComponent().equals(mPreferredForegroundService)) {
                defaultServiceInfo.foregroundDefault = serviceAidInfo;
            } else if (serviceAidInfo.service.getComponent().equals(mPreferredPaymentService) &&
                    serviceClaimsPaymentAid) {
                defaultServiceInfo.paymentDefault = serviceAidInfo;
            }
        }
        return defaultServiceInfo;
    }

    AidResolveInfo resolvePrefixAidConflictLocked(ArrayList<ServiceAidInfo> prefixServices,
                                                  ArrayList<ServiceAidInfo> conflictingServices) {
        // Find defaults among the prefix services themselves
        DefaultServiceInfo prefixDefaultInfo = findDefaultServices(prefixServices);

        // Find any defaults among the children
        DefaultServiceInfo conflictingDefaultInfo = findDefaultServices(conflictingServices);

        // Three conditions under which the prefix root AID gets to be the default
        // 1. A service registering the prefix root AID is the current foreground preferred
        // 2. A service registering the prefix root AID is the current tap & pay default AND
        //    no child is the current foreground preferred
        // 3. There is only one service for the prefix root AID, and there are no children
        if (prefixDefaultInfo.foregroundDefault != null) {
            if (DBG) Log.d(TAG, "Prefix AID service " +
                    prefixDefaultInfo.foregroundDefault.service.getComponent() + " has foreground" +
                    " preference, ignoring conflicting AIDs.");
            // Foreground default trumps any conflicting services, treat as normal AID conflict
            // and ignore children
            return resolveAidConflictLocked(prefixServices, true);
        } else if (prefixDefaultInfo.paymentDefault != null) {
            // Check if any of the conflicting services is foreground default
            if (conflictingDefaultInfo.foregroundDefault != null) {
                // Conflicting AID registration is in foreground, trumps prefix tap&pay default
                if (DBG) Log.d(TAG, "One of the conflicting AID registrations is foreground " +
                        "preferred, ignoring prefix.");
                return EMPTY_RESOLVE_INFO;
            } else {
                // Prefix service is tap&pay default, treat as normal AID conflict for just prefix
                if (DBG) Log.d(TAG, "Prefix AID service " +
                        prefixDefaultInfo.paymentDefault.service.getComponent() + " is payment" +
                        " default, ignoring conflicting AIDs.");
                return resolveAidConflictLocked(prefixServices, true);
            }
        } else {
            if (conflictingDefaultInfo.foregroundDefault != null ||
                    conflictingDefaultInfo.paymentDefault != null) {
                if (DBG) Log.d(TAG, "One of the conflicting AID registrations is either payment " +
                        "default or foreground preferred, ignoring prefix.");
                return EMPTY_RESOLVE_INFO;
            } else {
                // No children that are preferred; add all services of the root
                // make single service default if no children are present
                if (DBG) Log.d(TAG, "No service has preference, adding all.");
                return resolveAidConflictLocked(prefixServices, conflictingServices.isEmpty());
            }
        }
    }

    void generateServiceMapLocked(List<ApduServiceInfo> services) {
        // Easiest is to just build the entire tree again
        mAidServices.clear();
        for (ApduServiceInfo service : services) {
            if (DBG) Log.d(TAG, "generateServiceMap component: " + service.getComponent());
            for (String aid : service.getAids()) {
                if (!CardEmulation.isValidAid(aid)) {
                    Log.e(TAG, "Aid " + aid + " is not valid.");
                    continue;
                }
                if (aid.endsWith("*") && !supportsAidPrefixRegistration()) {
                    Log.e(TAG, "Prefix AID " + aid + " ignored on device that doesn't support it.");
                    continue;
                }
                ServiceAidInfo serviceAidInfo = new ServiceAidInfo();
                serviceAidInfo.aid = aid.toUpperCase();
                serviceAidInfo.service = service;
                serviceAidInfo.category = service.getCategoryForAid(aid);

                if (mAidServices.containsKey(serviceAidInfo.aid)) {
                    final ArrayList<ServiceAidInfo> serviceAidInfos =
                            mAidServices.get(serviceAidInfo.aid);
                    serviceAidInfos.add(serviceAidInfo);
                } else {
                    final ArrayList<ServiceAidInfo> serviceAidInfos =
                            new ArrayList<ServiceAidInfo>();
                    serviceAidInfos.add(serviceAidInfo);
                    mAidServices.put(serviceAidInfo.aid, serviceAidInfos);
                }
            }
        }
    }

    static boolean isPrefix(String aid) {
        return aid.endsWith("*");
    }

    final class PrefixConflicts {
        NavigableMap<String, ArrayList<ServiceAidInfo>> conflictMap;
        final ArrayList<ServiceAidInfo> services = new ArrayList<ServiceAidInfo>();
        final HashSet<String> aids = new HashSet<String>();
    }

    PrefixConflicts findConflictsForPrefixLocked(String prefixAid) {
        PrefixConflicts prefixConflicts = new PrefixConflicts();
        String plainAid = prefixAid.substring(0, prefixAid.length() - 1); // Cut off "*"
        String lastAidWithPrefix = String.format("%-32s", plainAid).replace(' ', 'F');
        if (DBG) Log.d(TAG, "Finding AIDs in range [" + plainAid + " - " +
                lastAidWithPrefix + "]");
        prefixConflicts.conflictMap =
                mAidServices.subMap(plainAid, true, lastAidWithPrefix, true);
        for (Map.Entry<String, ArrayList<ServiceAidInfo>> entry :
                prefixConflicts.conflictMap.entrySet()) {
            if (!entry.getKey().equalsIgnoreCase(prefixAid)) {
                if (DBG)
                    Log.d(TAG, "AID " + entry.getKey() + " conflicts with prefix; " +
                            " adding handling services for conflict resolution.");
                prefixConflicts.services.addAll(entry.getValue());
                prefixConflicts.aids.add(entry.getKey());
            }
        }
        return prefixConflicts;
    }

    void generateAidCacheLocked() {
        mAidCache.clear();
        // Get all exact and prefix AIDs in an ordered list
        PriorityQueue<String> aidsToResolve = new PriorityQueue<String>(mAidServices.keySet());

        while (!aidsToResolve.isEmpty()) {
            final ArrayList<String> resolvedAids = new ArrayList<String>();

            String aidToResolve = aidsToResolve.peek();
            // Because of the lexicographical ordering, all following AIDs either start with the
            // same bytes and are longer, or start with different bytes.

            // A special case is if another service registered the same AID as a prefix, in
            // which case we want to start with that AID, since it conflicts with this one
            if (aidsToResolve.contains(aidToResolve + "*")) {
                aidToResolve = aidToResolve + "*";
            }
            if (DBG) Log.d(TAG, "generateAidCacheLocked: starting with aid " + aidToResolve);

            if (isPrefix(aidToResolve)) {
                // This AID itself is a prefix; let's consider this prefix as the "root",
                // and all conflicting AIDs as its children.
                // For example, if "A000000003*" is the prefix root,
                // "A000000003", "A00000000301*", "A0000000030102" are all conflicting children AIDs
                final ArrayList<ServiceAidInfo> prefixServices = new ArrayList<ServiceAidInfo>(
                        mAidServices.get(aidToResolve));

                // Find all conflicting children services
                PrefixConflicts prefixConflicts = findConflictsForPrefixLocked(aidToResolve);

                // Resolve conflicts
                AidResolveInfo resolveInfo = resolvePrefixAidConflictLocked(prefixServices,
                        prefixConflicts.services);
                mAidCache.put(aidToResolve, resolveInfo);
                resolvedAids.add(aidToResolve);
                if (resolveInfo.defaultService != null) {
                    // This prefix is the default; therefore, AIDs of all conflicting children
                    // will no longer be evaluated.
                    resolvedAids.addAll(prefixConflicts.aids);
                } else if (resolveInfo.services.size() > 0) {
                    // This means we don't have a default for this prefix and all its
                    // conflicting children. So, for all conflicting AIDs, just add
                    // all handling services without setting a default
                    boolean foundChildService = false;
                    for (Map.Entry<String, ArrayList<ServiceAidInfo>> entry :
                            prefixConflicts.conflictMap.entrySet()) {
                        if (!entry.getKey().equalsIgnoreCase(aidToResolve)) {
                            if (DBG)
                                Log.d(TAG, "AID " + entry.getKey() + " shared with prefix; " +
                                        " adding all handling services.");
                            AidResolveInfo childResolveInfo = resolveAidConflictLocked(
                                    entry.getValue(), false);
                            // Special case: in this case all children AIDs must be routed to the
                            // host, so we can ask the user which service is preferred.
                            // Since these are all "children" of the prefix, they don't need
                            // to be routed, since the prefix will already get routed to the host
                            childResolveInfo.mustRoute = false;
                            mAidCache.put(entry.getKey(),childResolveInfo);
                            resolvedAids.add(entry.getKey());
                            foundChildService |= !childResolveInfo.services.isEmpty();
                        }
                    }
                    // Special case: if in the end we didn't add any children services,
                    // and the prefix has only one service, make that default
                    if (!foundChildService && resolveInfo.services.size() == 1) {
                        resolveInfo.defaultService = resolveInfo.services.get(0);
                    }
                } else {
                    // This prefix is not handled at all; we will evaluate
                    // the children separately in next passes.
                }
            } else {
                // Exact AID and no other conflicting AID registrations present
                // This is true because aidsToResolve is lexicographically ordered, and
                // so by necessity all other AIDs are different than this AID or longer.
                if (DBG) Log.d(TAG, "Exact AID, resolving.");
                final ArrayList<ServiceAidInfo> conflictingServiceInfos =
                        new ArrayList<ServiceAidInfo>(mAidServices.get(aidToResolve));
                mAidCache.put(aidToResolve, resolveAidConflictLocked(conflictingServiceInfos, true));
                resolvedAids.add(aidToResolve);
            }

            // Remove the AIDs we resolved from the list of AIDs to resolve
            if (DBG) Log.d(TAG, "AIDs: " + resolvedAids + " were resolved.");
            aidsToResolve.removeAll(resolvedAids);
            resolvedAids.clear();
        }

        // QNCI - BEGIN
        //updateRoutingLocked();
        updateRoutingWithSeNameLocked();
        // QNCI - END
    }

    void updateRoutingLocked() {
        if (!mNfcEnabled) {
            if (DBG) Log.d(TAG, "Not updating routing table because NFC is off.");
            return;
        }
        final HashMap<String, Boolean> routingEntries = Maps.newHashMap();
        // For each AID, find interested services
        for (Map.Entry<String, AidResolveInfo> aidEntry:
                mAidCache.entrySet()) {
            String aid = aidEntry.getKey();
            AidResolveInfo resolveInfo = aidEntry.getValue();
            if (!resolveInfo.mustRoute) {
                if (DBG) Log.d(TAG, "Not routing AID " + aid + " on request.");
                continue;
            }
            if (resolveInfo.services.size() == 0) {
                // No interested services
            } else if (resolveInfo.defaultService != null) {
                // There is a default service set, route to where that service resides -
                // either on the host (HCE) or on an SE.
                routingEntries.put(aid, resolveInfo.defaultService.isOnHost());
            } else if (resolveInfo.services.size() == 1) {
                // Only one service, but not the default, must route to host
                // to ask the user to choose one.
                routingEntries.put(aid, true);
            } else if (resolveInfo.services.size() > 1) {
                // Multiple services, need to route to host to ask
                routingEntries.put(aid, true);
            }
        }
        mRoutingManager.configureRouting(routingEntries);
    }

    // QNCI - BEGIN
    void updateRoutingWithSeNameLocked() {
        if (DBG) Log.d(TAG, "updateRoutingWithSeNameLocked()");
        if (!mNfcEnabled) {
            if (DBG) Log.d(TAG, "Not updating routing table because NFC is off.");
            return;
        }
        String aidSeName = null;
        // AID, SeName
        final HashMap<String, String> routingEntries = Maps.newHashMap();
        // For each AID, find interested services
        for (Map.Entry<String, AidResolveInfo> aidEntry:
                mAidCache.entrySet()) {
            String aid = aidEntry.getKey();
            AidResolveInfo resolveInfo = aidEntry.getValue();
            if (!resolveInfo.mustRoute) {
                if (DBG) Log.d(TAG, "Not routing AID " + aid + " on request.");
                continue;
            }
            if (resolveInfo.services.size() == 0) {
                // No interested services
            } else if (resolveInfo.defaultService != null) {
                // There is a default service set, route to where that service resides -
                // either on the host (HCE) or on an SE.
                //routingEntries.put(aid, resolveInfo.defaultService.isOnHost());
                aidSeName = resolveInfo.defaultService.getSeName();
                if (aidSeName.equals("MULTISE")) {
                    if (DBG) Log.d(TAG, "MULTISE: find the seName from AID = " + aid);
                    aidSeName = MultiSeManager.getAidMappedSe(aid);
                    if (aidSeName == null) {
                        if (DBG) Log.d(TAG, "MULTISE: Did not find seName");
                        continue;
                    }
                    if (DBG) Log.d(TAG, "MULTISE: found seName = "+ aidSeName);
                }
                routingEntries.put(aid, aidSeName);

            } else if (resolveInfo.services.size() == 1) {
                // Only one service, but not the default, must route to host
                // to ask the user to choose one.
                routingEntries.put(aid, "DH");
            } else if (resolveInfo.services.size() > 1) {
                // Multiple services, need to route to host to ask
                routingEntries.put(aid, "DH");
            }
        }
        mRoutingManager.configureRoutingWithSeName(routingEntries);
    }

    void setDefaultOffHostToPreferredService() {
        if (DBG) Log.d(TAG, "setDefaultOffHostToPreferredService()");

        ComponentName preferredService = null;
        if (mPreferredForegroundService != null) {
            preferredService = mPreferredForegroundService;
        } else if (mPreferredPaymentService != null) {
            preferredService = mPreferredPaymentService;
        } else {
            return;
        }

        for (Map.Entry<String, AidResolveInfo> aidEntry: mAidCache.entrySet()) {
            AidResolveInfo resolveInfo = aidEntry.getValue();
            if (resolveInfo.defaultService != null) {
                if (resolveInfo.defaultService.getComponent().equals(preferredService)) {
                    String seName = resolveInfo.defaultService.getSeName();
                    if ((seName != null) && (!seName.equals("DH"))) {
                        //update default off-host
                        QSecureElementManager.getInstance().updateDefaultOffHostRoute (seName);
                        return;
                    }
                }
            }
        }
        if (DBG) Log.d(TAG, "cannot find SE Name of Preferred Service");
    }

    // QNCI - END

    public void onServicesUpdated(int userId, List<ApduServiceInfo> services) {
        if (DBG) Log.d(TAG, "onServicesUpdated");
        synchronized (mLock) {
            if (ActivityManager.getCurrentUser() == userId) {
                // Rebuild our internal data-structures
                generateServiceMapLocked(services);
                generateAidCacheLocked();
            } else {
                if (DBG) Log.d(TAG, "Ignoring update because it's not for the current user.");
            }
        }
    }

    // update ISO-DEP protocol by API calling
    public void setDefaultRoute (String seName) {
        mRoutingManager.setDefaultRoute(seName);
    }

    public void onPreferredPaymentServiceChanged(ComponentName service) {
        if (DBG) Log.d(TAG, "Preferred payment service changed.");
       synchronized (mLock) {
           mPreferredPaymentService = service;
           //updateDefaultRoute();
           generateAidCacheLocked();
           setDefaultOffHostToPreferredService();
       }
    }

    public void onPreferredForegroundServiceChanged(ComponentName service) {
        if (DBG) Log.d(TAG, "Preferred foreground service changed.");
        synchronized (mLock) {
            mPreferredForegroundService = service;
            //updateDefaultRoute();
            generateAidCacheLocked();
            setDefaultOffHostToPreferredService();
        }
    }

    public void onNfcDisabled() {
        synchronized (mLock) {
            mNfcEnabled = false;
        }
        mRoutingManager.onNfccRoutingTableCleared();
    }

    public void onNfcEnabled() {
        synchronized (mLock) {
            mNfcEnabled = true;
            // QNCI - BEGIN
            //updateRoutingLocked();
            updateRoutingWithSeNameLocked();
            // QNCI - END
        }
    }

    String dumpEntry(Map.Entry<String, AidResolveInfo> entry) {
        StringBuilder sb = new StringBuilder();
        String category = entry.getValue().category;
        ApduServiceInfo defaultServiceInfo = entry.getValue().defaultService;
        sb.append("    \"" + entry.getKey() + "\" (category: " + category + ")\n");
        ComponentName defaultComponent = defaultServiceInfo != null ?
                defaultServiceInfo.getComponent() : null;

        for (ApduServiceInfo serviceInfo : entry.getValue().services) {
            sb.append("        ");
            if (serviceInfo.getComponent().equals(defaultComponent)) {
                sb.append("*DEFAULT* ");
            }
            sb.append(serviceInfo.getComponent() +
                    " (Description: " + serviceInfo.getDescription() + ")\n");
        }
        return sb.toString();
    }

    public void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        pw.println("    AID cache entries: ");
        for (Map.Entry<String, AidResolveInfo> entry : mAidCache.entrySet()) {
            pw.println(dumpEntry(entry));
        }
        pw.println("    Service preferred by foreground app: " + mPreferredForegroundService);
        pw.println("    Preferred payment service: " + mPreferredPaymentService);
        pw.println("");
        mRoutingManager.dump(fd, pw, args);
        pw.println("");
    }
}
