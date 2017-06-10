/*===========================================================================
                           GlobalSettingsPersister.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.digitalpen;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import com.qti.snapdragon.digitalpen.DigitalPenConfigMirror.CopyDirection;
import com.qti.snapdragon.digitalpen.util.DigitalPenConfig;
import android.util.AtomicFile;
import android.util.Log;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class GlobalSettingsPersister {
    private final String settingsFilename;

    private String TAG = "GlobalSettingsPersister";

    private String getFileLocationFromLinkFile(String linkFile) {
        try {
            File targetFile = new File(linkFile);
            File canon;

            if (null == targetFile.getParent()) {
                canon = targetFile;
            } else {
                File canonDir = targetFile.getParentFile().getCanonicalFile();
                canon = new File(canonDir, targetFile.getName());
            }
            boolean isSymbolicLink = !canon.getCanonicalFile().equals(canon.getAbsoluteFile());

            if (!isSymbolicLink) {
                Log.w(TAG, "Link File: " + targetFile + " is not a symbolic link");
                return linkFile;
            }
            return targetFile.getCanonicalPath();
        } catch (IOException e) {
            Log.e(TAG, "Error: " + e.getMessage());
            return linkFile;
        }
    }

    public GlobalSettingsPersister(String settingsFilename) {
        this.settingsFilename = settingsFilename;
    }

    private AtomicFile getFileFromSymlink(String settingsFilename) {
        String settingsFile = getFileLocationFromLinkFile(settingsFilename);
        AtomicFile atomicSettingsFile = new AtomicFile(new File(settingsFile));
        return atomicSettingsFile;
    }

    public DigitalPenConfig deserialize() {
        Serializer serializer = new Persister();
        DigitalPenConfig digitalPenConfig = null;
        InputStream is = null;
        AtomicFile globalSettingsFile = getFileFromSymlink(settingsFilename);
        try {
            is = globalSettingsFile.openRead();
            DigitalPenConfigMirror mirror = serializer.read(DigitalPenConfigMirror.class, is);
            if (mirror != null) {
                digitalPenConfig = new DigitalPenConfig();
                mirror.copyPersistedFields(digitalPenConfig, CopyDirection.MIRROR_TO_TRUE);
                Log.i(TAG, "Successfully deserialized xml");
            }
        } catch (FileNotFoundException e) {
            Log.e(TAG, "FileNotFoundException while opening xml");
            e.printStackTrace();
        } catch (Exception e) {
            Log.e(TAG, "Exception while deserializing xml");
            e.printStackTrace();
        } finally {
            try {
                if (is != null) {
                    is.close();
                }
            } catch (IOException e) {
                Log.e(TAG, "IOException while closing xml");
                e.printStackTrace();
            }
        }
        if (null == digitalPenConfig) {
            Log.e(TAG, "Error deserializing XML: " + settingsFilename
                    + " getting default DigitalPenConfig");
            digitalPenConfig = new DigitalPenConfig();
        }
        return digitalPenConfig;
    }

    public void serialize(DigitalPenConfig config) {
        final FileOutputStream os;
        Serializer serializer = new Persister();
        AtomicFile globalSettingsFile = getFileFromSymlink(settingsFilename);
        try {
            os = globalSettingsFile.startWrite();
            boolean success = false;
            try {
                DigitalPenConfigMirror mirror = new DigitalPenConfigMirror();
                mirror.copyPersistedFields(config, CopyDirection.TRUE_TO_MIRROR);
                serializer.write(mirror, os);
                success = true;
            } catch (Exception e) {
                Log.e(TAG, "Exception while serializing xml");
                e.printStackTrace();
            } finally {
                if (success) {
                    globalSettingsFile.finishWrite(os);
                    Log.i(TAG, "Successfully serialized xml");
                } else {
                    globalSettingsFile.failWrite(os);
                }
            }
        } catch (IOException ex) {
            Log.w(TAG, "Failed to save xml.", ex);
        }
    }
}
