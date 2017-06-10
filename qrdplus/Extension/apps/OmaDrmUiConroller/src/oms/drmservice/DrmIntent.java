/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package oms.drmservice;

public abstract interface DrmIntent {

    public static final String START_DOWNLOAD = "android.drmservice.intent.action.START_DOWNLOAD";
    public static final String FINISH_DOWNLOAD = "android.drmservice.intent.action.FINISH_DOWNLOAD";
    public static final String UPDATE_DOWNLOAD = "android.drmservice.intent.action.UPDATE_DOWNLOAD";
    public static final String WAP_PUSH = "android.provider.Telephony.WAP_PUSH_RECEIVED";
    public static final String SHOW_PROPERTIES = "android.drmservice.intent.action.SHOW_PROPERTIES";
    public static final String OPEN_FILES_LICENSE =
            "android.drmservice.intent.action.OPEN_FILES_LICENSE";
    public static final String OPEN_FILES_CONTENT =
            "android.drmservice.intent.action.OPEN_FILES_CONTENT";
    public static final String BUY_LICENSE = "android.drmservice.intent.action.BUY_LICENSE";
    public static final String PHONE_STATE = "android.intent.action.PHONE_STATE";
    public static final String FILE_NOT_EXIST = "android.drmservice.intent.action.FILE_NOT_EXIST";
    public static final String CLEAR_NOTIFICATION =
            "android.drmservice.intent.action.CLEAR_NOTIFICATION";

    public static final String EXTRA_FILE_PATH = "DRM_FILE_PATH";
    public static final String EXTRA_DCF_FILEPATH = "DRM_DCF_FILEPATH";
    public static final String EXTRA_MIME_TYPE = "DRM_MIME_TYPE";
    public static final String EXTRA_DRM_TYPE = "DRM_TYPE";
    public static final String EXTRA_DRM_DOWNLOADID = "DRM_DOWNLOADID";

}
