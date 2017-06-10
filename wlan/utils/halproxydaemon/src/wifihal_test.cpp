/*
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*
*/

#include "wifihal_test.hpp"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

namespace WIFIHAL_TEST
{
    /* Keep this up-to-date with Feature enums defined in wifi_hal.h */
    feature features[] = {
        {0x0001, "WIFI_FEATURE_INFRA          "},
        {0x0002, "WIFI_FEATURE_INFRA_5G       "},
        {0x0004, "WIFI_FEATURE_HOTSPOT        "},
        {0x0008, "WIFI_FEATURE_P2P            "},
        {0x0010, "WIFI_FEATURE_SOFT_AP        "},
        {0x0020, "WIFI_FEATURE_GSCAN          "},
        {0x0040, "WIFI_FEATURE_NAN            "},
        {0x0080, "WIFI_FEATURE_D2D_RTT        "},
        {0x0100, "WIFI_FEATURE_D2AP_RTT       "},
        {0x0200, "WIFI_FEATURE_BATCH_SCAN     "},
        {0x0400, "WIFI_FEATURE_PNO            "},
        {0x0800, "WIFI_FEATURE_ADDITIONAL_STA "},
        {0x1000, "WIFI_FEATURE_TDLS           "},
        {0x2000, "WIFI_FEATURE_TDLS_OFFCHANNEL"},
        {0x4000, "WIFI_FEATURE_EPR            "},
        {0x8000, "WIFI_FEATURE_AP_STA         "},
        {0x10000, "WIFI_FEATURE_LINK_LAYER_STATS"}
    };
    /* Number of WIFI_FEATURE_* enums defined above */
    #define WIFI_FEATURE_ENUM_SIZE 17

    /* CLI cmd strings */
    const char *WifiHalTestSuite::WIFIHAL_CMD = "wifihal";
    const char *WifiHalTestSuite::WIFIHAL_GET_SUPPORTED_FEATURES =
        "get_supported_features";
    const char *WifiHalTestSuite::WIFIHAL_SET_NO_DFS_FLAG =
        "set_no_dfs_flag";


    /* Constructor */
    WifiHalTestSuite::WifiHalTestSuite(wifi_interface_handle handle)
        :wifiHandle_(handle)
    {
        fprintf(stderr, "WifiHalTestSuite::WifiHalTestSuite: "
            "Created a Wi-Fi HAL Suite.\n");
    }


    /* Process the command line args */
    void WifiHalTestSuite::executeCmd(int cmdIndex,
                                      cmdData data)
    {
        fprintf(stderr, "%s: Enter \n", __func__);
        switch(cmdIndex) {
            case 1:
                wifihalSendGetSupportedFeatures();
                break;
            case 2:
                wifihalSendSetNoDfsFlag(data.no_dfs_flag);
                break;
            case 3:
                wifihalSendGetConcurrencyMatrix(data.set_size_max);
                break;
            case 4:
                wifihalGetIfaces(data.wifiHandle);
                break;
            case 5:
                wifihalSetIfaceEventHandler(data.reqId);
                break;
            case 6:
                wifihalReSetIfaceEventHandler(data.reqId);
                break;
            default:
                fprintf(stderr, "%s: Unknown Cmd ID.\n", __func__);
        }
    }

    void WifiHalTestSuite::wifihalSendGetSupportedFeatures()
    {
        int ret = 0;
        feature_set set = 0;
        ret = wifi_get_supported_feature_set(wifiHandle_, &set);
        fprintf(stderr, "wifi_get_supported_feature_set : 0x%x \n", set);

        if (ret)
        {
            fprintf(stderr, "wifi_get_supported_feature_set failed");
            return;
        }

        for (int i = 0; i < WIFI_FEATURE_ENUM_SIZE; i++)
        {
            if(set & features[i].id)
                fprintf(stderr, "%s: Supported\n", features[i].description);
            else
                fprintf(stderr, "%s: Not Supported\n",
                features[i].description);
        }
    }


    void WifiHalTestSuite::wifihalSendSetNoDfsFlag(u32 no_dfs)
    {
        int ret = 0;
        fprintf(stderr, "%s: Sending No DFS Flag:%d to the wifi HAL. "
            "\n", __func__, no_dfs);

        ret = wifi_set_nodfs_flag(wifiHandle_, no_dfs);
        if (ret)
        {
            fprintf(stderr, "%s: Set No_DFS Flag failed, returned: %d.\n",
                __func__, ret);
            return;
        }
    }

    void WifiHalTestSuite::wifihalSendGetConcurrencyMatrix(int set_size_max)
    {
        int ret = 0;
        int setSize = 0;
        int i = 0;
        int j = 0;
        feature_set *concurrencySet;
        feature_set concurrencyRecord = 0;

        concurrencySet =
            (feature_set *) malloc (set_size_max * sizeof(feature_set));

        if (concurrencySet)
        {
            memset(concurrencySet, 0, set_size_max * sizeof(feature_set));
            fprintf(stderr, "WifiHalTestSuite::wifihalSendGetConcurrencyMatrix"
                ": Sending Get Concurrency MAtrix Request. "
                "max_set_size: %d.\n", set_size_max);

            ret = wifi_get_concurrency_matrix(wifiHandle_,
                                              set_size_max,
                                              concurrencySet, &setSize);
            if (ret)
            {
                fprintf(stderr, "wifi_get_concurrency_matrix failed, "
                    "ret code:%d\n", ret);
                goto cleanup;
            }

            fprintf(stderr, "No. of supported concurrency combinations:%d\n",
                setSize);

            for (i = 0; i < setSize; i++)
            {
                fprintf(stderr, "Supported features combination[%d]: ", i+1);
                concurrencyRecord = concurrencySet[i];
                for (j = 0; j < WIFI_FEATURE_ENUM_SIZE; j++)
                {
                    if (concurrencyRecord & features[j].id)
                        fprintf(stderr, "%s ", features[j].description);
                }
                fprintf(stderr, ".\n");
            }
        }
    cleanup:
        if (concurrencySet) {
            free(concurrencySet);
            concurrencySet = NULL;
        }
    }

    void WifiHalTestSuite::wifihalGetIfaces(wifi_handle handle)
    {
        int i = 0, num=0;
        wifi_interface_handle *interfaces;

        fprintf(stderr, "Get the interfaces available:\n");
        wifi_get_ifaces(handle, &num, &interfaces);

        interface_info **iface = (interface_info **)interfaces;
        char name[IFNAMSIZ+1];

        fprintf(stderr, "Number of interfaces available: %d\n", num);

        for(i = 0; i< num; i++) {
            if(wifi_get_iface_name((wifi_interface_handle)iface[i],
                        &name[0], IFNAMSIZ) == WIFI_SUCCESS) {
                fprintf(stderr, "Interface %d : %s\n", i, name);
            } else {
                fprintf(stderr, "Failed to get the iface name");
            }
        }
        fprintf(stderr, "########## Done #############\n");
    }

    void onCountryCodeChanged(char code[2])
    {
        fprintf(stderr, "New Country Code : \"%c%c\"\n", code[0], code[1]);
    }

    void WifiHalTestSuite::wifihalSetIfaceEventHandler(wifi_request_id reqId)
    {
        int ret = 0;
        wifi_event_handler eh;
        eh.on_country_code_changed = onCountryCodeChanged;
        fprintf(stderr, "%s: request Id : %d\n", __func__, reqId);

        ret = wifi_set_iface_event_handler(reqId, wifiHandle_, eh);
        if (ret)
        {
            fprintf(stderr, "wifi_set_iface_event_handler failed, "
                    "ret code:%d\n", ret);
        }
        fprintf(stderr, "########## Done #############\n");
    }

    void WifiHalTestSuite::wifihalReSetIfaceEventHandler(wifi_request_id reqId)
    {
        int ret = 0;
        wifi_event_handler eh;
        eh.on_country_code_changed = onCountryCodeChanged;
        fprintf(stderr, "%s: request Id : %d\n", __func__, reqId);

        ret = wifi_reset_iface_event_handler(reqId, wifiHandle_);
        if (ret)
        {
            fprintf(stderr, "wifi_reset_iface_event_handler failed, "
                    "ret code:%d\n", ret);
        }
        fprintf(stderr, "########## Done #############\n");
    }
}

