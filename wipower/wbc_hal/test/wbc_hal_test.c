/*=========================================================================
  wbc_hal_test.c
  DESCRIPTION
  Test program for WBC HAL

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  =========================================================================*/

#include <stdio.h>
#include <unistd.h>
#include "../wbc_hal_interface.h"

#define LOG_TAG "wbc_hal_test"
#include <cutils/log.h>

static wbc_hal_module_t * g_wbcModule = NULL;

int main(int argc, char *argv[]) {

    int ret = hw_get_module(WBC_HARDWARE_MODULE_ID, (hw_module_t const **)&g_wbcModule);
    if (ret) {
        ALOGE("hw_get_module failed\n");
        return-1;
    }

    if (g_wbcModule != NULL) {
        g_wbcModule->init(NULL);  // need to add more after init, for now we'll
                                  // just see uevents printed from wbc hal library
    }

    for (;;) {
        sleep(1);
    }

    return 0;
}

/*=========================================================================
  END OF FILE
  =========================================================================*/

