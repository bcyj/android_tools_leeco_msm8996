/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2013 The Android Open Source Project
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
import android.util.Log;
import android.util.SparseArray;

import com.android.nfc.NfcService;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class AidRoutingManager {
    static final String TAG = "AidRoutingManager";

    static final boolean DBG = Debug.AidRoutingManager;

    static final int ROUTE_HOST = 0x00;

    // Every routing table entry is matched exact
    static final int AID_MATCHING_EXACT_ONLY = 0x00;
    // Every routing table entry can be matched either exact or prefix
    static final int AID_MATCHING_EXACT_OR_PREFIX = 0x01;
    // Every routing table entry is matched as a prefix
    static final int AID_MATCHING_PREFIX_ONLY = 0x02;

    // This is the default IsoDep protocol route; it means
    // that for any AID that needs to be routed to this
    // destination, we won't need to add a rule to the routing
    // table, because this destination is already the default route.
    //
    // For Nexus devices, the default route is always 0x00.
    int mDefaultRoute;

    // For Nexus devices, just a static route to the eSE
    // OEMs/Carriers could manually map off-host AIDs
    // to the correct eSE/UICC based on state they keep.
    final int mDefaultOffHostRoute;

    // How the NFC controller can match AIDs in the routing table;
    // see AID_MATCHING constants
    final int mAidMatchingSupport;

    final Object mLock = new Object();

    // mAidRoutingTable contains the current routing table. The index is the route ID.
    // The route can include routes to a eSE/UICC.
    SparseArray<Set<String>> mAidRoutingTable =
            new SparseArray<Set<String>>();

    // Easy look-up what the route is for a certain AID
    HashMap<String, Integer> mRouteForAid = new HashMap<String, Integer>();

    private native int doGetDefaultRouteDestination();
    private native int doGetDefaultOffHostRouteDestination();
    private native int doGetAidMatchingMode();
    private native int doGetNumSecureElement();

    public AidRoutingManager() {
        mDefaultRoute = doGetDefaultRouteDestination();
        if (DBG) Log.d(TAG, "mDefaultRoute=0x" + Integer.toHexString(mDefaultRoute));
        mDefaultOffHostRoute = doGetDefaultOffHostRouteDestination();
        if (DBG) Log.d(TAG, "mDefaultOffHostRoute=0x" + Integer.toHexString(mDefaultOffHostRoute));
        mAidMatchingSupport = doGetAidMatchingMode();
        if (DBG) Log.d(TAG, "mAidMatchingSupport=0x" + Integer.toHexString(mAidMatchingSupport));
    }

    public int getNumSecureElement() {
        return doGetNumSecureElement();
    }

    public boolean supportsAidPrefixRouting() {
        return mAidMatchingSupport == AID_MATCHING_EXACT_OR_PREFIX ||
                mAidMatchingSupport == AID_MATCHING_PREFIX_ONLY;
    }

    void clearNfcRoutingTableLocked() {
        for (Map.Entry<String, Integer> aidEntry : mRouteForAid.entrySet())  {
            String aid = aidEntry.getKey();
            if (aid.endsWith("*")) {
                if (mAidMatchingSupport == AID_MATCHING_EXACT_ONLY) {
                    Log.e(TAG, "Device does not support prefix AIDs but AID [" + aid
                            + "] is registered");
                } else if (mAidMatchingSupport == AID_MATCHING_PREFIX_ONLY) {
                    if (DBG) Log.d(TAG, "Unrouting prefix AID " + aid);
                    // Cut off '*' since controller anyway treats all AIDs as a prefix
                    aid = aid.substring(0, aid.length() - 1);
                } else if (mAidMatchingSupport == AID_MATCHING_EXACT_OR_PREFIX) {
                    if (DBG) Log.d(TAG, "Unrouting prefix AID " + aid);
                }
            } else {
                if (DBG) Log.d(TAG, "Unrouting exact AID " + aid);
            }

            NfcService.getInstance().unrouteAids(aid);
        }
    }

    public boolean configureRouting(HashMap<String, Boolean> aidMap) {
        SparseArray<Set<String>> aidRoutingTable = new SparseArray<Set<String>>(aidMap.size());
        HashMap<String, Integer> routeForAid = new HashMap<String, Integer>(aidMap.size());
        // Then, populate internal data structures first
        for (Map.Entry<String, Boolean> aidEntry : aidMap.entrySet())  {
            int route = aidEntry.getValue() ? ROUTE_HOST : mDefaultOffHostRoute;
            String aid = aidEntry.getKey();
            Set<String> entries = aidRoutingTable.get(route, new HashSet<String>());
            entries.add(aid);
            aidRoutingTable.put(route, entries);
            routeForAid.put(aid, route);
        }

        synchronized (mLock) {
            if (routeForAid.equals(mRouteForAid)) {
                if (DBG) Log.d(TAG, "Routing table unchanged, not updating");
                return false;
            }

            // Otherwise, update internal structures and commit new routing
            clearNfcRoutingTableLocked();
            mRouteForAid = routeForAid;
            mAidRoutingTable = aidRoutingTable;
            if (mAidMatchingSupport == AID_MATCHING_PREFIX_ONLY) {
                /* If a non-default route registers an exact AID which is shorter
                 * than this exact AID, this will create a problem with controllers
                 * that treat every AID in the routing table as a prefix.
                 * For example, if App A registers F0000000041010 as an exact AID,
                 * and App B registers F000000004 as an exact AID, and App B is not
                 * the default route, the following would be added to the routing table:
                 * F000000004 -> non-default destination
                 * However, because in this mode, the controller treats every routing table
                 * entry as a prefix, it means F0000000041010 would suddenly go to the non-default
                 * destination too, whereas it should have gone to the default.
                 *
                 * The only way to prevent this is to add the longer AIDs of the
                 * default route at the top of the table, so they will be matched first.
                 */
                Set<String> defaultRouteAids = mAidRoutingTable.get(mDefaultRoute);
                if (defaultRouteAids != null) {
                    for (String defaultRouteAid : defaultRouteAids) {
                        // Check whether there are any shorted AIDs routed to non-default
                        // TODO this is O(N^2) run-time complexity...
                        for (Map.Entry<String, Integer> aidEntry : mRouteForAid.entrySet()) {
                            String aid = aidEntry.getKey();
                            int route = aidEntry.getValue();
                            if (defaultRouteAid.startsWith(aid) && route != mDefaultRoute) {
                                if (DBG)
                                    Log.d(TAG, "Adding AID " + defaultRouteAid + " for default " +
                                            "route, because a conflicting shorter AID will be " +
                                            "added to the routing table");
                                NfcService.getInstance().routeAids(defaultRouteAid, mDefaultRoute);
                            }
                        }
                    }
                }
            }

            // Add AID entries for all non-default routes
            for (int i = 0; i < mAidRoutingTable.size(); i++) {
                int route = mAidRoutingTable.keyAt(i);
                if (route != mDefaultRoute) {
                    Set<String> aidsForRoute = mAidRoutingTable.get(route);
                    for (String aid : aidsForRoute) {
                        if (aid.endsWith("*")) {
                            if (mAidMatchingSupport == AID_MATCHING_EXACT_ONLY) {
                                Log.e(TAG, "This device does not support prefix AIDs.");
                            } else if (mAidMatchingSupport == AID_MATCHING_PREFIX_ONLY) {
                                if (DBG) Log.d(TAG, "Routing prefix AID " + aid + " to route "
                                        + Integer.toString(route));
                                // Cut off '*' since controller anyway treats all AIDs as a prefix
                                NfcService.getInstance().routeAids(aid.substring(0,
                                                aid.length() - 1), route);
                            } else if (mAidMatchingSupport == AID_MATCHING_EXACT_OR_PREFIX) {
                                if (DBG) Log.d(TAG, "Routing prefix AID " + aid + " to route "
                                        + Integer.toString(route));
                                NfcService.getInstance().routeAids(aid, route);
                            }
                        } else {
                            if (DBG) Log.d(TAG, "Routing exact AID " + aid + " to route "
                                    + Integer.toString(route));
                            NfcService.getInstance().routeAids(aid, route);
                        }
                    }
                }
            }
        }

        // And finally commit the routing
        NfcService.getInstance().commitRouting();

        return true;
    }

    // QNCI - BEGIN
    // Routing AID based on SeName
    public boolean configureRoutingWithSeName(HashMap<String, String> aidMap) {
        if (DBG) Log.d(TAG, "configureRoutingWithSeName()");
        SparseArray<Set<String>> aidRoutingTable = new SparseArray<Set<String>>(aidMap.size());
        HashMap<String, Integer> routeForAid = new HashMap<String, Integer>(aidMap.size());
        // Then, populate internal data structures first
        for (Map.Entry<String, String> aidEntry : aidMap.entrySet())  {
            //int route = aidEntry.getValue() ? ROUTE_HOST : mDefaultOffHostRoute;
            int route = NfcService.getInstance().getNfceeIdOfSecureElement(aidEntry.getValue());
            if (route != -1) {
                String aid = aidEntry.getKey();
                Set<String> entries = aidRoutingTable.get(route, new HashSet<String>());
                entries.add(aid);
                aidRoutingTable.put(route, entries);
                routeForAid.put(aid, route);
                if (DBG) Log.d(TAG, "AID:" + aid + " Dest:" + aidEntry.getValue());
            } else {
                Log.e(TAG, "Couldn't find secure element:" + aidEntry.getValue());
            }
        }

        synchronized (mLock) {
            if (routeForAid.equals(mRouteForAid)) {
                if (DBG) Log.d(TAG, "Routing table unchanged, not updating");
                return false;
            }

            // Otherwise, update internal structures and commit new routing
            clearNfcRoutingTableLocked();
            mRouteForAid = routeForAid;
            mAidRoutingTable = aidRoutingTable;

            if (mAidMatchingSupport == AID_MATCHING_PREFIX_ONLY) {
                /* If a non-default route registers an exact AID which is shorter
                 * than this exact AID, this will create a problem with controllers
                 * that treat every AID in the routing table as a prefix.
                 * For example, if App A registers F0000000041010 as an exact AID,
                 * and App B registers F000000004 as an exact AID, and App B is not
                 * the default route, the following would be added to the routing table:
                 * F000000004 -> non-default destination
                 * However, because in this mode, the controller treats every routing table
                 * entry as a prefix, it means F0000000041010 would suddenly go to the non-default
                 * destination too, whereas it should have gone to the default.
                 *
                 * The only way to prevent this is to add the longer AIDs of the
                 * default route at the top of the table, so they will be matched first.
                 */
                Set<String> defaultRouteAids = mAidRoutingTable.get(mDefaultRoute);
                if (defaultRouteAids != null) {
                    for (String defaultRouteAid : defaultRouteAids) {
                        // Check whether there are any shorted AIDs routed to non-default
                        // TODO this is O(N^2) run-time complexity...
                        for (Map.Entry<String, Integer> aidEntry : mRouteForAid.entrySet()) {
                            String aid = aidEntry.getKey();
                            int route = aidEntry.getValue();
                            if (defaultRouteAid.startsWith(aid) && route != mDefaultRoute) {
                                if (DBG)
                                    Log.d(TAG, "Adding AID " + defaultRouteAid + " for default " +
                                            "route, because a conflicting shorter AID will be " +
                                            "added to the routing table");
                                NfcService.getInstance().routeAids(defaultRouteAid, mDefaultRoute);
                            }
                        }
                    }
                }
            }

            // Add AID entries for all non-default routes
            for (int i = 0; i < mAidRoutingTable.size(); i++) {
                int route = mAidRoutingTable.keyAt(i);
                if (route != mDefaultRoute) {
                    Set<String> aidsForRoute = mAidRoutingTable.get(route);
                    for (String aid : aidsForRoute) {
                        if (aid.endsWith("*")) {
                            if (mAidMatchingSupport == AID_MATCHING_EXACT_ONLY) {
                                Log.e(TAG, "This device does not support prefix AIDs.");
                            } else if (mAidMatchingSupport == AID_MATCHING_PREFIX_ONLY) {
                                if (DBG) Log.d(TAG, "Routing prefix AID " + aid + " to route "
                                        + Integer.toString(route));
                                // Cut off '*' since controller anyway treats all AIDs as a prefix
                                NfcService.getInstance().routeAids(aid.substring(0,
                                                aid.length() - 1), route);
                            } else if (mAidMatchingSupport == AID_MATCHING_EXACT_OR_PREFIX) {
                                if (DBG) Log.d(TAG, "Routing prefix AID " + aid + " to route "
                                        + Integer.toString(route));
                                NfcService.getInstance().routeAids(aid, route);
                            }
                        } else {
                            if (DBG) Log.d(TAG, "Routing exact AID " + aid + " to route "
                                    + Integer.toString(route));
                            NfcService.getInstance().routeAids(aid, route);
                        }
                    }
                }
            }
        }

/* TODO: uncomment to send PPSE response for MultiSE feature
        // process PPSE response
        if (MultiSeManager.getInstance() != null)
            MultiSeManager.getInstance().processPpse();
*/
        // And finally commit the routing
        NfcService.getInstance().commitRouting();

        return true;
    }
    // QNCI - END

/* TODO: fix or delete
    public boolean setOffHostRouteForAid(String aid, String seName) {
        int route;
        synchronized (mLock) {
            int currentRoute = getRouteForAidLocked(aid);
            if (DBG) Log.d(TAG, "Set route for AID: " + aid + ", secure element: " + seName + " , current: 0x" +
                    Integer.toHexString(currentRoute));
            if (seName == null) {
                seName = "SIM1";
            }
            route = NfcService.getInstance().getNfceeIdOfSecureElement(seName);
            if (route == -1) {
                Log.e(TAG, "Couldn't find secure element:" + seName);
                return false;
            }
            if (route == currentRoute) return true;

            if (currentRoute != -1) {
                // Remove current routing
                removeAid(aid);
            }
            Set<String> aids = mAidRoutingTable.get(route);
            if (aids == null) {
               aids = new HashSet<String>();
               mAidRoutingTable.put(route, aids);
            }
            aids.add(aid);
            mRouteForAid.put(aid, route);
            if (route != mDefaultRoute) {
                NfcService.getInstance().routeAids(aid, route);
                mDirty = true;
            }
        }
        return true;
    }
*/
    /**
     * This notifies that the AID routing table in the controller
     * has been cleared (usually due to NFC being turned off).
     */
    public void onNfccRoutingTableCleared() {
        // The routing table in the controller was cleared
        // To stay in sync, clear our own tables.
        synchronized (mLock) {
            mAidRoutingTable.clear();
            mRouteForAid.clear();
        }
    }


    public void setDefaultRoute(String seName) {
        int newDefaultRoute = NfcService.getInstance().getNfceeIdOfSecureElement(seName);

        if (DBG) Log.d(TAG, "setDefaultRoute() from NFCEE " + mDefaultRoute + " to " + newDefaultRoute + ":" + seName);

        if (newDefaultRoute == -1) {
            Log.e(TAG, "setDefaultRoute() invalid SE: " + seName);
            return;
        }

        synchronized (mLock) {
            if (mDefaultRoute != newDefaultRoute) {
                Set<String> aids = mAidRoutingTable.get(mDefaultRoute);
                if (aids != null) {
                    for (String aid : aids) {
                        if (DBG) Log.d(TAG, "add route " + aid + " for " + mDefaultRoute);
                        mRouteForAid.put(aid, mDefaultRoute);
                        NfcService.getInstance().routeAids(aid, mDefaultRoute);
                    }
                }

                aids = mAidRoutingTable.get(newDefaultRoute);
                if (aids != null) {
                    for (String aid : aids) {
                        if (DBG) Log.d(TAG, "remove route " + aid + " of " + newDefaultRoute);
                        mRouteForAid.remove(aid);
                        NfcService.getInstance().unrouteAids(aid);
                    }
                }

                mDefaultRoute = newDefaultRoute;
            } else {
                if (DBG) Log.d(TAG, "Not changing default route because it's not changed.");
            }
        }
    }

    public void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        pw.println("Routing table:");
        pw.println("    Default route: " + ((mDefaultRoute == 0x00) ? "host" : "secure element"));
        synchronized (mLock) {
            for (int i = 0; i < mAidRoutingTable.size(); i++) {
                Set<String> aids = mAidRoutingTable.valueAt(i);
                pw.println("    Routed to 0x" + Integer.toHexString(mAidRoutingTable.keyAt(i)) + ":");
                for (String aid : aids) {
                    pw.println("        \"" + aid + "\"");
                }
            }
        }
    }
}
