/*
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*
*/

#ifndef __WIFIHAL_TEST_HPP__
#define __WIFIHAL_TEST_HPP__

#include "wifi_hal.h"
#include <getopt.h>
#include <netlink/genl/family.h>

namespace WIFIHAL_TEST
{
    typedef struct{
        u32 id;
        const char *description;
    } feature;

    typedef struct{
        u32 no_dfs_flag;
        int set_size_max;
        wifi_request_id reqId;
        wifi_handle wifiHandle;
    } cmdData;

    typedef struct {
        wifi_handle handle;
        char name[IFNAMSIZ+1];
        int  id;
    } interface_info;

    class WifiHalTestSuite
    {
    public:
        /* CLI cmd strings */
        static const char *WIFIHAL_CMD;
        static const char *WIFIHAL_GET_SUPPORTED_FEATURES;
        static const char *WIFIHAL_SET_NO_DFS_FLAG;

        WifiHalTestSuite(wifi_interface_handle handle);

        /* Execute the command line args */
        void executeCmd(int cmdIndex, cmdData data);

    private:
        wifi_interface_handle wifiHandle_;

        /* Send the command to Android HAL */
        void wifihalSendGetSupportedFeatures();
        void wifihalSendSetNoDfsFlag(u32 no_dfs);
        void wifihalSendGetConcurrencyMatrix(int set_size_max);
        void wifihalGetIfaces(wifi_handle handle);
        void wifihalSetIfaceEventHandler(wifi_request_id reqId);
        void wifihalReSetIfaceEventHandler(wifi_request_id reqId);
    };
}

#endif
