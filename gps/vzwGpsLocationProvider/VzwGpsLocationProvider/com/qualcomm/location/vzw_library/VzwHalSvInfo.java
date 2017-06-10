/******************************************************************************
  @file    VzwHalSvInfo.java
  @brief   satellite information for the location fix

  DESCRIPTION

  VZW requires more detailed satellite information

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.location.vzw_library;

public class VzwHalSvInfo {

    public static final int GPS_VALID_SATELLITES_IN_VIEW_COUNT =                    0x00000001;
    public static final int GPS_VALID_SATELLITES_IN_VIEW_PRNS =                     0x00000002;
    public static final int GPS_VALID_SATELLITES_IN_VIEW_ELEVATION =                0x00000004;
    public static final int GPS_VALID_SATELLITES_IN_VIEW_AZIMUTH =                  0x00000008;
    public static final int GPS_VALID_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO =    0x00000010;
    public static final int GPS_VALID_SATELLITES_WITH_EPHEMERIS =                   0x00000020;
    public static final int GPS_VALID_SATELLITES_WITH_ALMANAC =                     0x00000040;

    private int mValidFlagMask = 0;
    private int NumSatellitesInView;
    private float[] SatellitesInViewAzimuth;
    private float[] SatellitesInViewElevation;
    private int[] SatellitesInViewPRN;
    private float[] SatellitesInViewSignalToNoiseRatio;
    private int[] SatellitesWithEphemeris;
    private int[] SatellitesWithAlmanac;

    private static final int MAX_SV_COUNT = 32;

    /**
     * get the maximum number of satellites in this report
     *
     * @return
     */
    public int getMaximumPossibleSatelliteCount() {
        return MAX_SV_COUNT;
    }

    /**
     * get the valid field mask in this report
     *
     * @return
     */
    public int getValidFieldMask() {
        return mValidFlagMask;
    }
    /*
     * set the valid field mask in this report
     *
     * @param mask
     */
    public void setValidFieldMask(int mask) {
        mValidFlagMask = mask;
    }

    public int getNumSatellitesInView() {
        return NumSatellitesInView;
    }

    public void setNumSatellitesInView(int numSatellitesInView) {
        if((numSatellitesInView < 0) || (numSatellitesInView > MAX_SV_COUNT))
        {
            throw new IllegalArgumentException("Num of SV must be [0..MAX_SV_COUNT]");
        }

        NumSatellitesInView = numSatellitesInView;
    }

    public float[] getSatellitesInViewAzimuth() {
        return SatellitesInViewAzimuth;
    }

    public void setSatellitesInViewAzimuth(float[] satellitesInViewAzimuth) {
        SatellitesInViewAzimuth = satellitesInViewAzimuth;
    }

    public float[] getSatellitesInViewElevation() {
        return SatellitesInViewElevation;
    }

    public void setSatellitesInViewElevation(float[] satellitesInViewElevation) {
        SatellitesInViewElevation = satellitesInViewElevation;
    }

    public int[] getSatellitesInViewPRNs() {
        return SatellitesInViewPRN;
    }

    public void setSatellitesInViewPRNs(int[] satellitesInViewPRN) {
        SatellitesInViewPRN = satellitesInViewPRN;
    }

    public float[] getSatellitesInViewSignalToNoiseRatio() {
        return SatellitesInViewSignalToNoiseRatio;
    }

    public void setSatellitesInViewSignalToNoiseRatio(
            float[] satellitesInViewSignalToNoiseRatio) {
        SatellitesInViewSignalToNoiseRatio = satellitesInViewSignalToNoiseRatio;
    }

    public int[] getSatellitesWithEphemeris() {
        return SatellitesWithEphemeris;
    }

    public void setSatellitesWithEphemeris(int[] satellitesWithEphemeris) {
        SatellitesWithEphemeris = satellitesWithEphemeris;
    }

    public int[] getSatellitesWithAlmanac() {
        return SatellitesWithAlmanac;
    }

    public void setSatellitesWithAlmanac(int[] satellitesWithAlmanac) {
        SatellitesWithAlmanac = satellitesWithAlmanac;
    }
}

