/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.accesscache;

interface ICarrierAccessCacheService{
    boolean writeActionFile(String content);
    boolean writeCommandFile(String filePath, String locale);
}
