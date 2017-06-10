/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.encryptdata;

import java.io.Serializable;

public class FileDescriptor implements Serializable {

    private static final long serialVersionUID = 1L;
    final String magic = "SecProtect";
    final String version = "1.0";
    String fileName;
    int size;

    @Override
    public String toString() {

        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("version=" + version + "\n");
        stringBuilder.append("fileName=" + fileName + "\n");
        stringBuilder.append("size=" + size);
        return stringBuilder.toString();
    }
}