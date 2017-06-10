/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
@file  DummyCalib.cpp
@brief Interface for Display Calibration Manager for non-supported
       platforms.

GENERAL DESCRIPTION

  Interface implementation of Display Calibration Manager.

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

#include "Calib.h"


int DCM::DCMControl(bool flag){
    return -1;
}

