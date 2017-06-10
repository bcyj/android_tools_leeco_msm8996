/******************************************************************************
  @file    VzwHalCriteria.java
  @brief   location report

  DESCRIPTION

  VZW extention of the standard Location

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.location.vzw_library;

import android.location.Location;
import android.os.Bundle;


import com.qualcomm.location.vzw_library.IVzwHalGpsLocationProvider;

public class VzwHalLocation extends android.location.Location {

    public static final int GPS_VALID_LATITUDE  =                                   0x00000001;
    public static final int GPS_VALID_LONGITUDE  =                                  0x00000002;
    public static final int GPS_VALID_ALTITUDE_WRT_SEA_LEVEL =                      0x00000004;
    public static final int GPS_VALID_POSITION_DILUTION_OF_PRECISION =              0x00000008;
    public static final int GPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION =            0x00000010;
    public static final int GPS_VALID_VERTICAL_DILUTION_OF_PRECISION =              0x00000020;
    public static final int GPS_VALID_ELLIPTICAL_ACCURACY  =                        0x00000040;
    public static final int GPS_VALID_VERTICAL_ACCURACY  =                          0x00000080;
    public static final int GPS_VALID_HORIZONTAL_CONFIDENCE  =                      0x00000100;
    public static final int GPS_VALID_SATELLITES_USED_PRNS =                        0x00000200;

    public static final int GPS_VALID_SATELLITES_IN_VIEW_PRNS =                     0x00000400;
    public static final int GPS_VALID_SATELLITES_IN_VIEW_ELEVATION =                0x00000800;
    public static final int GPS_VALID_SATELLITES_IN_VIEW_AZIMUTH =                  0x00001000;
    public static final int GPS_VALID_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO =    0x00002000;

    public static final int GPS_VALID_FIX_MODE =                                    0x00004000;
    public static final int GPS_VALID_MAGNETIC_VARIATION =                          0x00008000;
    public static final int GPS_VALID_TIME =                                        0x00010000;

    public VzwHalLocation() {
        super(IVzwHalGpsLocationProvider.VZW_GPS_LOCATION_PROVIDER_NAME);
        if(getExtras() == null)
        {
            setExtras(new Bundle());
        }
    }

    public VzwHalLocation(Location l) {
        super(l);
        if(getExtras() == null)
        {
            setExtras(new Bundle());
        }
    }

    private static final String keyValidFieldMask = "VZW_VALID_FIELD_MASK";
    private static final String keyPosDop = "VZW_POS_DOP";
    private static final String keyPosDopH = "VZW_POS_DOP_H";
    private static final String keyPosDopV = "VZW_POS_DOP_V";
    private static final String keyMajorAxis = "VZW_MAJOR_AXIS";
    private static final String keyMajorAxisAngle = "VZW_MAJOR_AXIS_ANGLE";
    private static final String keyMinorAxis = "VZW_MINOR_AXIS";
    private static final String keyConfidenceH = "VZW_CONFIDENCE_H";
    private static final String keySessionId = "VZW_SESSION_ID";
    private static final String keyAltitudeSeaLevel = "VZW_ALTITUDE_SEA_LEVEL";
    private static final String keyVerticalAccuracy = "VZW_VERTICAL_ACCURACY";
    private static final String keySatellitesUsedPRN = "VZW_SATELLITES_USED_PRN";

    private static final String keySatellitesInViewPRN = "VZW_SATELLITES_IN_VIEW_PRN";
    private static final String keySatellitesInViewElevation = "VZW_SATELLITES_IN_VIEW_ELEVEVATION";
    private static final String keySatellitesInViewAzimuth = "VZW_SATELLITES_IN_VIEW_AZIMUTH";
    private static final String keySatellitesInViewSno = "VZW_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO";

    private static final String keyFixMode = "VZW_FIX_MODE";
    private static final String keyMagneticVariation = "VZW_MAGNETIC_VARIATION";

    public int      getValidFieldMask() {
        return getExtras().getInt(keyValidFieldMask, 0);
    }
    public void     setValidFieldMask(int mask) {
        getExtras().putInt(keyValidFieldMask, mask);
    }

    public float    getPositionDilutionOfPrecision() {
        return getExtras().getFloat(keyPosDop, 0);
    }
    public void     setPositionDilutionOfPrecision(float dop) {
        getExtras().putFloat(keyPosDop, dop);
    }

    public float    getHorizontalDilutionOfPrecision() {
        return getExtras().getFloat(keyPosDopH, 0);
    }
    public void     setHorizontalDilutionOfPrecision(float dop) {
        getExtras().putFloat(keyPosDopH, dop);
    }

    public float    getVerticalDilutionOfPrecision() {
        return getExtras().getFloat(keyPosDopV, 0);
    }
    public void     setVerticalDilutionOfPrecision(float dop) {
        getExtras().putFloat(keyPosDopV, dop);
    }

    public float    getMajorAxis() {
        return getExtras().getFloat(keyMajorAxis, 0);
    }
    public void     setMajorAxis(float length) {
        getExtras().putFloat(keyMajorAxis, length);
    }

    public float    getMajorAxisAngle() {
        return getExtras().getFloat(keyMajorAxisAngle, 0);
    }
    public void     setMajorAxisAngle(float angle) {
        getExtras().putFloat(keyMajorAxisAngle, angle);
    }

    public float    getMinorAxis() {
        return getExtras().getFloat(keyMinorAxis, 0);
    }
    public void     setMinorAxis(float length) {
        getExtras().putFloat(keyMinorAxis, length);
    }

    public float    getHorizontalConfidence() {
        return getExtras().getFloat(keyConfidenceH, 0);
    }
    public void     setHorizontalConfidence(float percentage) {
        getExtras().putFloat(keyConfidenceH, percentage);
    }

    public double    getAltitudeWrtSeaLevel() {
        return getExtras().getDouble(keyAltitudeSeaLevel, 0);
    }
    public void     setAltitudeWrtSeaLevel(double altitude_sea_level) {
        getExtras().putDouble(keyAltitudeSeaLevel, altitude_sea_level);
    }

    public double    getVerticalAccuracy() {
        return getExtras().getDouble(keyVerticalAccuracy, 0);
    }
    public void     setVerticalAccuracy(float accuracyVertical) {
        getExtras().putDouble(keyVerticalAccuracy, accuracyVertical);
    }

    public int[] getSatellitesUsedPRN() {
        return getExtras().getIntArray(keySatellitesUsedPRN);
    }
    public void setSatellitesUsedPRN(int[] satelliteUsedPRN) {
        getExtras().putIntArray(keySatellitesUsedPRN, satelliteUsedPRN);
    }

    public float[] getSatellitesInViewAzimuth() {
        return getExtras().getFloatArray(keySatellitesInViewAzimuth);
    }

    public void setSatellitesInViewAzimuth(float[] satellitesInViewAzimuth) {
        getExtras().putFloatArray(keySatellitesInViewAzimuth, satellitesInViewAzimuth);
    }

    public float[] getSatellitesInViewElevation() {
        return getExtras().getFloatArray(keySatellitesInViewElevation);
    }

    public void setSatellitesInViewElevation(float[] satellitesInViewElevation) {
        getExtras().putFloatArray(keySatellitesInViewElevation, satellitesInViewElevation);
    }

    public int[] getSatellitesInViewPRNs() {
        return getExtras().getIntArray(keySatellitesInViewPRN);
    }

    public void setSatellitesInViewPRNs(int[] satellitesInViewPRN) {
        getExtras().putIntArray(keySatellitesInViewPRN, satellitesInViewPRN);
    }

    public float[] getSatellitesInViewSignalToNoiseRatio() {
        return getExtras().getFloatArray(keySatellitesInViewSno);
    }

    public void setSatellitesInViewSignalToNoiseRatio(
            float[] satellitesInViewSignalToNoiseRatio) {
        getExtras().putFloatArray(keySatellitesInViewSno, satellitesInViewSignalToNoiseRatio);
    }

    public int getFixMode() {
        return getExtras().getInt(keyFixMode, -1);
    }
    public void setFixMode(int mode) {
        getExtras().putInt(keyFixMode, mode);
    }

  /* Difference between the bearing to true north and the bearing shown on a magnetic compass.
     The declination is positive when the magnetic north is east of true north. */
    public float getMagneticVariation() {
        return getExtras().getFloat(keyMagneticVariation, 0);
    }
    public void setMagneticVariation(float variation) {
        getExtras().putFloat(keyMagneticVariation, variation);
    }

    public int      getSessionId() {
        return getExtras().getInt(keySessionId, 0);
    }
    public void     setSessionId(int id) {
        getExtras().putInt(keySessionId, id);
    }

    public int getHybridMode () {
        // not supported
        return 0;
    }

    public void setHybridMode (int mode) {
        // do nothing
    }
}
