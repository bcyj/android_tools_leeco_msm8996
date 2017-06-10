/**
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */

package com.qualcomm.update;

import java.io.Serializable;

public class UpdateInfo implements Serializable {

    static class Delta implements Serializable {
        private static final long serialVersionUID = 1L;
        int from;
        int to;
    }

    private static final long serialVersionUID = 1L;

    public static final String QNAME_UPDATE = "update";

    public static final String QNAME_VERSION = "version";

    public static final String QNAME_FILE = "file";

    public static final String QNAME_DES = "description";

    public static final String QNAME_DELTA = "delta";

    private String version;

    private String fileName;

    private String description;

    private long size;

    private Delta delta;

    public Delta getDelta() {
        return delta;
    }

    public void setDelta(Delta delta) {
        this.delta = delta;
    }

    public long getSize() {
        return size;
    }

    public void setSize(long size) {
        this.size = size;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }

    public String getFileName() {
        return fileName;
    }

    public void setFileName(String fileName) {
        this.fileName = fileName;
    }

    public String toString() {
        return "version:" + version + "\tfileName:" + fileName;
    }

}
