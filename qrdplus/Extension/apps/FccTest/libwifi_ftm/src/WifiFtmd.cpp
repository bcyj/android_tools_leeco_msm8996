/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "WifiFtmController.h"

#define LOG_TAG "WifiFtm"
#include <cutils/log.h>

int main(int argc, char** argv) {
    WifiFtmController controller("wifi_ftmd");

    controller.start();

    return 0;
}
