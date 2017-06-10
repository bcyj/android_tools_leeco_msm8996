/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import java.util.ArrayList;
import java.util.Collection;

public class SyncQueue extends ArrayList<SyncQueue.SyncRequest> {

    private static final long serialVersionUID = 1L;

    public static abstract class SyncRequest {

        private final SyncQueue mSyncQueue;

        public SyncRequest(SyncQueue queue) {
            mSyncQueue = queue;
        }

        protected abstract void start();

        protected final void end() {
            if (!mSyncQueue.contains(this)) {
                return;
            }
            mSyncQueue.remove(this);
            if (!mSyncQueue.isEmpty()) {
                mSyncQueue.get(0).start();
            }
        }

        public void loop() {
            if (!mSyncQueue.contains(this)) {
                mSyncQueue.add(this);
            }
        }
    }

    @Override
    public boolean add(SyncRequest object) {
        boolean result = super.add(object);
        if (size() == 1) {
            object.start();
        }
        return result;
    }

    @Override
    public void add(int index, SyncRequest object) {
        super.add(index, object);
        if (size() == 1) {
            object.start();
        }
    }

    @Override
    public boolean addAll(Collection<? extends SyncRequest> collection) {
        boolean result = super.addAll(collection);
        if (size() > 0 && size() == collection.size()) {
            get(0).start();
        }
        return result;
    }

    @Override
    public boolean addAll(int index, Collection<? extends SyncRequest> collection) {
        boolean result = super.addAll(index, collection);
        if (size() > 0 && size() == collection.size()) {
            get(0).start();
        }
        return result;
    }
}
