/*===========================================================================
                           DirectoryObserver.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.digitalpen;

import java.util.HashMap;

import android.os.FileObserver;

/*
 * Wraps a single FileObserver of a directory, since Android's FileObserver
 * doesn't allow multiple observers on the same directory.
 * This design is specific to DigitalPenService, so it has shortcomings:
 * - no way to stop watching
 * - only one listener per filename
 */
public class DirectoryObserver {

    public interface FileChangeListener {

        void onEvent(int event, String file);

    }

    private class ListenerEntry {
        public ListenerEntry(FileChangeListener listener, int mask) {
            this.listener = listener;
            this.mask = mask;
        }

        public final FileChangeListener listener;
        public final int mask;
    }

    private HashMap<String, ListenerEntry> listenerMap;
    private FileObserver fileObserver;

    public DirectoryObserver(String path) {
        listenerMap = new HashMap<String, ListenerEntry>();
        fileObserver = new FileObserver(path) {

            @Override
            public void onEvent(int event, String path) {
                handleEvent(event, path);
            }
        };
        fileObserver.startWatching();
    }

    public void registerObserver(String string, int mask, FileChangeListener listener) {
        listenerMap.put(string, new ListenerEntry(listener, mask));
    }

    protected void handleEvent(int event, String path) {
        ListenerEntry entry = listenerMap.get(path);
        if (entry == null) {
            return;
        }
        if ((event & entry.mask) != 0) {
            entry.listener.onEvent(event, path);
        }
    }
}
