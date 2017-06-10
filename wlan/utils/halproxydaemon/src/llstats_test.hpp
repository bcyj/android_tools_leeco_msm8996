/*
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*
*/

#ifndef __WIFI_HAL_LLSTATS_TEST_HPP__
#define __WIFI_HAL_LLSTATS_TEST_HPP__

#include "wifi_hal.h"
#include <getopt.h>
namespace LLStats
{
    class LLStatsTestSuite
    {
    public:

        /* CLI cmd strings */
        static const char *LL_CMD;
        static const char *LL_SET;
        static const char *LL_GET;
        static const char *LL_CLEAR;

        LLStatsTestSuite(wifi_handle handle);
        void processCmd(int argc, char **argv);
        void  wifi_set_link_stats_test(wifi_interface_handle iface,
                                       u32 mpduSizeThreshold,
                                       u32 aggressiveStatisticsGathering);
        void wifi_get_link_stats_test(wifi_interface_handle iface);
        void wifi_clr_link_stats_test(wifi_interface_handle iface,
                                      u32 stats_clear_req_mask,
                                      u8 stop_req);

        /* process the command line args */
    private:
        wifi_handle wifiHandle_;
        wifi_interface_handle ifaceHandle;

        /* Send a LLStats command to Android HAL */
    };
}
#endif
