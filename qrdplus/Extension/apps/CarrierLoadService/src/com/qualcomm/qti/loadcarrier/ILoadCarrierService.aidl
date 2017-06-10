/**
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.loadcarrier;

interface ILoadCarrierService{
    Map getCarrierList();
    String copyToData(String srcFilePath);
    String downloadToData(String url);
}
