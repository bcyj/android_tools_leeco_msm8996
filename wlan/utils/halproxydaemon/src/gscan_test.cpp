/*
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*
*/

#include "gscan_test.hpp"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include<sys/socket.h>

extern struct sockaddr_in si;
extern int app_sock, slen;

#define MAC_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MAC_ADDR_STR "%02x:%02x:%02x:%02x:%02x:%02x"

wifi_interface_handle wifi_get_iface_handle(wifi_handle handle, char *name);

static void gscan_on_hotlist_ap_found(wifi_request_id req_id,
        unsigned num_results, wifi_scan_result *results)
{
    int i;
    wifi_scan_result *result = results;
    fprintf(stderr, "%s: request_id:%d, num_results:%d\n",
        __func__, req_id, num_results);

    for(i = 0; i < num_results && result; i++)
    {
        fprintf(stderr, "Hotlist AP : %d\n", i+1);
        fprintf(stderr, "ts  %llu \n", result->ts);
        fprintf(stderr, "SSID  %s \n", result->ssid) ;
        fprintf(stderr, "BSSID: %02x:%02x:%02x:%02x:%02x:%02x \n",
                result->bssid[0], result->bssid[1], result->bssid[2],
                result->bssid[3], result->bssid[4], result->bssid[5]);
        fprintf(stderr, "channel %u \n", result->channel);
        fprintf(stderr, "rssi  %d \n", (signed char) result->rssi);
        fprintf(stderr, "rtt  %llu \n", result->rtt);
        fprintf(stderr, "rtt_sd  %llu \n", result->rtt_sd);
        fprintf(stderr, "beacon period  %d \n", result->beacon_period);
        fprintf(stderr, "capability  %d \n", result->capability);
        fprintf(stderr, "IE length  %d\n", result->ie_length);
        if(result->ie_length > 0)
        {
            fprintf(stderr, "IE Data : ");
            for(int j=0; j < result->ie_length; j++)
                fprintf(stderr, "%2x ", result->ie_data[j]);
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
        result = (wifi_scan_result *)((u8 *)result + sizeof(wifi_scan_result)
                     +  result->ie_length);
    }
}

static void gscan_on_significant_change(wifi_request_id req_id,
            unsigned num_results,
            wifi_significant_change_result **results)
{
    fprintf(stderr, "gscan_on_significant_change: request_id:%d, "
        "num_results:%d\n", req_id, num_results);
    wifi_significant_change_result *result = NULL;
    if (results)
        result = results[0];
    for(u32 i = 0; i < num_results && result; i++, result = results[i])
    {
        fprintf(stderr, "AP : %d\n", i+1);
        fprintf(stderr, "BSSID: %02x:%02x:%02x:%02x:%02x:%02x \n",
                result->bssid[0], result->bssid[1], result->bssid[2],
                result->bssid[3], result->bssid[4], result->bssid[5]);
        fprintf(stderr, "Channel %u \n", result->channel);
        fprintf(stderr, "Num_rssi : %d \n", result->num_rssi);
        fprintf(stderr, "Rssi list : ");
        for(int j=0; j < result->num_rssi; j++)
            fprintf(stderr, "%d ", (signed char) result->rssi[j]);
        fprintf(stderr, "\n\n");
#if 0
        result = (wifi_significant_change_result *)
        ((u8*)results + sizeof(wifi_significant_change_result) +
        result->num_rssi*sizeof(wifi_rssi)) ;
        */
#endif
    }
}

/* reported when report_threshold is reached in scan cache */
static void gscan_on_scan_results_available(wifi_request_id id,
                                unsigned num_results_available)
{
    fprintf(stderr, "%s: request_id:%d, num_results_available:%d\n",
        __func__, id, num_results_available);
}

/* reported when each probe response is received, if report_events
 * enabled in wifi_scan_cmd_params */
static void gscan_on_full_scan_result(wifi_request_id id,
                                        wifi_scan_result *result)
{
    fprintf(stderr, "Full Scan Result: request_id:%d", id);
    if (result) {
        fprintf(stderr, "ts  %llu ", result->ts);
        fprintf(stderr, "SSID  %s ", result->ssid) ;
        fprintf(stderr, "BSSID: %02x:%02x:%02x:%02x:%02x:%02x ",
                 result->bssid[0], result->bssid[1], result->bssid[2],
                 result->bssid[3], result->bssid[4], result->bssid[5]);
        fprintf(stderr, "channel %u ", result->channel);
        fprintf(stderr, "rssi  %d ", (signed char) result->rssi);
        fprintf(stderr, "rtt  %llu ", result->rtt);
        fprintf(stderr, "rtt_sd  %llu ", result->rtt_sd);
        fprintf(stderr, "beacon period  %d ", result->beacon_period);
        fprintf(stderr, "capability  %d ", result->capability);
        fprintf(stderr, "IE length  %d\n", result->ie_length);
    }
}

/* optional event - indicates progress of scanning statemachine */
static void gscan_on_scan_event(wifi_scan_event event, unsigned status)
{
    fprintf(stderr, "%s: scan_event:%d, status:%d\n",
            __func__, event, status);
#if 0
    char *message = "HI!! Trying to send message to newly created thread";
    if(sendto(app_sock, message, strlen(message) , 0 , (struct sockaddr *) &si, slen)==-1)
    {
        printf("Failed to send msg\n");
    }
#endif
}

namespace GSCAN_TEST
{
    /* CLI cmd strings */
    const char *GScanTestSuite::GSCAN_CMD = "gscan";
    const char *GScanTestSuite::GSCAN_START = "start";
    const char *GScanTestSuite::GSCAN_STOP = "stop";
    const char *GScanTestSuite::GSCAN_GET_VALID_CHANNELS =
        "get_valid_channels";
    const char *GScanTestSuite::GSCAN_GET_CAPABILITIES = "get_capabilities";
    const char *GScanTestSuite::GSCAN_SET_BSSID_HOTLIST = "set_bssid_hotlist";
    const char *GScanTestSuite::GSCAN_RESET_BSSID_HOTLIST =
        "reset_bssid_hotlist";
    const char *GScanTestSuite::GSCAN_SET_SIGNIFICANT_CHANGE =
        "set_significant_change";
    const char *GScanTestSuite::GSCAN_RESET_SIGNIFICANT_CHANGE =
        "reset_significant_change";
    const char *GScanTestSuite::GSCAN_GET_CACHED_RESULTS =
        "get_cached_results";

    /* Constructor */
    GScanTestSuite::GScanTestSuite(wifi_handle handle, wifi_request_id request_id)
        :wifiHandle_(handle)
    {
        fprintf(stderr, "GScanTestSuite::GScanTestSuite: Created a GScan Test "
            "Suite with request_id:%d\n", request_id);
        id = request_id;
    }

    void GScanTestSuite::setRequestId(int reqId)
    {
        id = reqId;
    }

    int GScanTestSuite::getRequestId()
    {
        return id;
    }

    /* process the command line args */
    void GScanTestSuite::executeCmd(int argc, char **argv, int cmdIndex,
                                        int max, u32 flush,
                                        int band, oui scan_oui)
    {
        fprintf(stderr, "%s: Enter \n", __func__);

        if(argc < 3)
        {
            fprintf(stderr, "%s: insufficient GSCAN args\n", argv[0]);
            fprintf(stderr, "Usage : hal_proxy_daemon gscan interface_name");
            return;
        }

        ifaceHandle = wifi_get_iface_handle(wifiHandle_, argv[2]);
        if(!ifaceHandle)
        {
            fprintf(stderr, "Interface %s is not up, exiting.\n", argv[2]);
            fprintf(stderr, "Please restart hal_proxy_daemon with a valid"
                " initialized interface\n");
            return;
        }

        switch(cmdIndex) {
            case 1:
                gscanSendStartRequest(argc, argv);
                break;
            case 2:
                gscanSendStopRequest(argc, argv);
                break;
            case 3:
                gscanSendGetValidChannelsRequest(argc, argv, max, band);
                break;
            case 4:
                gscanSendGetCapabilitiesRequest(argc, argv);
                break;
            case 5:
                gscanSendGetCachedResultsRequest(argc, argv, max, flush);
                break;
            case 6:
                gscanSendSetBssidHotlistRequest(argc, argv);
                break;
            case 7:
                gscanSendResetBssidHotlistRequest(argc, argv);
                break;
            case 8:
                gscanSendSetSignificantChangeRequest(argc, argv);
                break;
            case 9:
                gscanSendResetSignificantChangeRequest(argc, argv);
                break;
            case 10:
                gscanSendSetMacOui(argc, argv, scan_oui);
                break;
            default:
                fprintf(stderr, "%s: Unknown Cmd ID.\n", __func__);
        }
    }

    /* process the command line args */
    void GScanTestSuite::processCmd(int argc, char **argv)
    {
        if(argc < 3)
        {
            fprintf(stderr, "%s: insufficient GSCAN args\n", argv[0]);
            return;
        }

        if(strcasecmp(argv[2], GSCAN_START) == 0)
            return gscanSendStartRequest(argc, argv);

        if(strcasecmp(argv[2], GSCAN_STOP) == 0)
            return gscanSendStopRequest(argc, argv);

        if(strcasecmp(argv[2], GSCAN_GET_VALID_CHANNELS) == 0)
            return gscanSendGetValidChannelsRequest(argc, argv, 10, 1);

        if(strcasecmp(argv[2], GSCAN_GET_CAPABILITIES) == 0)
            return gscanSendGetCapabilitiesRequest(argc, argv);

        if(strcasecmp(argv[2], GSCAN_SET_BSSID_HOTLIST) == 0)
            return gscanSendSetBssidHotlistRequest(argc, argv);

        if(strcasecmp(argv[2], GSCAN_RESET_BSSID_HOTLIST) == 0)
            return gscanSendResetBssidHotlistRequest(argc, argv);

        if(strcasecmp(argv[2], GSCAN_SET_SIGNIFICANT_CHANGE) == 0)
            return gscanSendSetSignificantChangeRequest(argc, argv);

        if(strcasecmp(argv[2], GSCAN_RESET_SIGNIFICANT_CHANGE) == 0)
            return gscanSendResetSignificantChangeRequest(argc, argv);

        if(strcasecmp(argv[2], GSCAN_GET_CACHED_RESULTS) == 0)
            return gscanSendGetCachedResultsRequest(argc, argv, 1000, 1);

        fprintf(stderr, "%s: unknown  arg %s\n", argv[0], argv[2]);
    }

    /* Helper routine to print usage */
    void GScanTestSuite::gscanPrintCmdUsage(char **argv, const char *cmd,
       const char *sub_cmd, const struct option long_options[], int size)
    {
        fprintf(stderr, "Usage: %s %s %s\n", argv[0], cmd, sub_cmd);
        for(int i = 1; i <= size-2; i++)
        {
            if(long_options[i].has_arg)
                fprintf(stderr, "\t[--%s arg]\n", long_options[i].name);
            else
                fprintf(stderr, "\t[--%s]\n", long_options[i].name);
        }
        return;
    }

    void GScanTestSuite::gscanSendGetValidChannelsRequest(
                                                           int argc,
                                                           char **argv,
                                                           int max_channels,
                                                           int band)
    {
        int num_channels;
        wifi_channel *channels = NULL;
        int ret = 0;

        /* A string listing valid short options letters.  */
        const char* const short_options = "hb:m:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { "band",         1,   NULL, 'b' },
            { "max_channels", 1,   NULL, 'm' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };

        /* Override with command line arguements */
        int long_index = 0, opt = 0, i;

        fprintf(stderr, "GScanTestSuite::gscanSendGetValidChannelsRequest: "
            "Entry - Sending GSCAN Get Valid Channels Request.\n");
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'b' :
                    band = atoi(optarg);
                    break;
                case 'm' :
                    max_channels = atoi(optarg);
                    break;
                case 'h':
                default:
                    gscanPrintCmdUsage(argv, GSCAN_CMD,
                        GSCAN_GET_VALID_CHANNELS, long_options,
                       sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        if (band < 0) {
            fprintf(stderr, "Invalid band provided:%d\n",
                band);
            goto cleanup;
        }

        if (max_channels <= 0) {
            fprintf(stderr, "Invalid max_channels provided:%d\n",
                max_channels);
            goto cleanup;
        }

        channels =
            (wifi_channel *) malloc (max_channels * sizeof(wifi_channel));
        if (channels)
            memset(channels, 0, max_channels * sizeof(wifi_channel));
        fprintf(stderr, "GScanTestSuite::gscanSendGetValidChannelsRequest: "
            "Sending Get Valid Channels Request. band: %d, "
            "max_channels: %d.\n", band, max_channels);

        ret = wifi_get_valid_channels(ifaceHandle, band, max_channels,
                channels, &num_channels);
        if (ret)
            goto cleanup;
        fprintf(stderr, "GScanTestSuite::gscanSendGetValidChannelsRequest:"
            "Get valid channels event received\n");
        fprintf(stderr, "GScanTestSuite::gscanSendGetValidChannelsRequest:"
            "Num channels : %d \n", num_channels);
        fprintf(stderr, "channels : ");
        for(i = 0; i < num_channels; i++)
        {
            fprintf(stderr, "%u,  ", *(channels + i));
        }
        fprintf(stderr, "\n");
    cleanup:
        if(channels)
            free(channels);
        channels = NULL;
    }

    void GScanTestSuite::gscanSendGetCapabilitiesRequest(int argc, char **argv)
    {
        int ret = 0;
        /* A string listing valid short options letters.  */
        const char* const short_options = "h:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };
        wifi_gscan_capabilities capa;
        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'h':
                default:
                    gscanPrintCmdUsage(argv, GSCAN_CMD, GSCAN_GET_CAPABILITIES,
                        long_options,
                        sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        fprintf(stderr, "%s: Sending Get Capabilities Request. \n",
            __FUNCTION__);
        ret = wifi_get_gscan_capabilities(ifaceHandle, &capa);
        fprintf(stderr, "%s: Received GSCAN Capabilities with value:%d. \n",
            __FUNCTION__, ret);
        if (ret)
            return;
        fprintf(stderr, "%s: Capabilities:\n    max_ap_cache_per_scan:%d,\n "
            "   max_bssid_history_entries:%d,\n    max_hotlist_aps:%d,\n "
            "   max_rssi_sample_size:%d,\n    max_scan_buckets:%d,\n "
            "   max_scan_cache_size:%d,\n    max_scan_reporting_threshold:%d,\n "
            "   max_significant_wifi_change_aps:%d.\n",
            __FUNCTION__, capa.max_ap_cache_per_scan,
            capa.max_bssid_history_entries, capa.max_hotlist_aps,
            capa.max_rssi_sample_size, capa.max_scan_buckets,
            capa.max_scan_cache_size, capa.max_scan_reporting_threshold,
            capa.max_significant_wifi_change_aps);
    }

    void GScanTestSuite::gscanSendStartRequest(int argc, char **argv)
    {
        fprintf(stderr, "%s: Sending GSCAN Start Request. "
            "\n", __FUNCTION__);
        wifi_scan_result_handler nHandler;
        int i, j, ret = 0;
        /* A string listing valid short options letters.  */
        const char* const short_options = "h:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };
        wifi_gscan_capabilities capa;
        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'h':
                default:
                    gscanPrintCmdUsage(argv, GSCAN_CMD,
                        GSCAN_SET_BSSID_HOTLIST,
                        long_options,
                        sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        wifi_scan_result_handler handler;
        wifi_scan_cmd_params params;

        FILE *f_read = fopen("/etc/wifi/gscan_start_params.txt", "r");
        fprintf(stderr, "\n");
        if (f_read) {
            if ( (fscanf(f_read, "%d", &params.num_buckets) &&
                fscanf(f_read, "%d", &params.base_period) &&
                fscanf(f_read, "%d", &params.max_ap_per_scan) &&
                fscanf(f_read, "%d", &params.report_threshold)) == 0 ) {
                fprintf(stderr, "%s: Failed parsing GSCAN Start "
                    "params file. Exit\n", __func__);
                return;
            }
            fprintf(stderr, "Number of buckets:%d\n base_period:%d\n "
                "max_ap_per_scan:%d\n report_threshold:%d\n\n",
                    params.num_buckets,
                    params.base_period,
                    params.max_ap_per_scan,
                    params.report_threshold);

            /* Outer loop for parsing scan buckets */
            for ( i =0; i < params.num_buckets; i++ )
            {
                if ( (fscanf(f_read, "%d", &params.buckets[i].bucket) &&
                    fscanf(f_read, "%d", &params.buckets[i].band) &&
                    fscanf(f_read, "%d", &params.buckets[i].period) &&
                    fscanf(f_read, "%d", &params.buckets[i].report_events) &&
                    fscanf(f_read, "%d", &params.buckets[i].num_channels)) == 0 ) {
                    fprintf(stderr, "%s: Failed parsing GSCAN Start "
                        "params file. Exit\n", __func__);
                    return;
                }

                fprintf(stderr, "params.buckets[%d].index:%d\n",
                        i, params.buckets[i].bucket);

                fprintf(stderr, "params.buckets[%d].band:%d\n params.buckets[%d].period:%d\n"
                            "params.buckets[%d].report_events:%d\n"
                            "params.buckets[%d].num_channels:%d\n\n",
                            i, params.buckets[i].band,
                            i, params.buckets[i].period,
                            i, params.buckets[i].report_events,
                            i, params.buckets[i].num_channels);
                /* Inner loop for parsing channel buckets */
                for ( j = 0; j < params.buckets[i].num_channels; j++ )
                {
                    if ( (fscanf(f_read, "%d", &params.buckets[i].channels[j].channel) &&
                        fscanf(f_read, "%d", &params.buckets[i].channels[j].dwellTimeMs) &&
                        fscanf(f_read, "%d", &params.buckets[i].channels[j].passive)) == 0) {
                        fprintf(stderr, "%s: Failed parsing GSCAN Start "
                            "params file. Exit\n", __func__);
                        return;
                    }

                    fprintf(stderr, "Channel buckets of Scan Bucket[%d]\n", i);

                    fprintf(stderr, "   buckets[%d].channels[%d].channel:%dMHz\n "
                        "   buckets[%d].channels[%d].dwellTimeMs:%d\n"
                            "   buckets[%d].channels[%d].passive:%d \n\n",
                            i, j, params.buckets[i].channels[j].channel,
                            i, j, params.buckets[i].channels[j].dwellTimeMs,
                            i, j, params.buckets[i].channels[j].passive);
                }
            }
            fclose(f_read);
        } else {
            fprintf(stderr, "gscanSendStartRequest: Failed to "
            "open file /etc/wifi/gscan_start_params.txt - "
            "use hard-coded defaults\n");

            params.base_period = 20; /*20ms*/
            params.max_ap_per_scan = 3;
            params.num_buckets = 1;
            params.report_threshold = 50;

            params.buckets[0].bucket = 0;
            params.buckets[0].band = WIFI_BAND_BG;
            params.buckets[0].period = 20;
            params.buckets[0].report_events = 0;
            /* Hardcode info for 1 scan bucket. */
            params.buckets[0].num_channels = 1;

            params.buckets[0].channels[0].channel = 2437;
            params.buckets[0].channels[0].dwellTimeMs = 20;
            params.buckets[0].channels[0].passive = 1;

        }

        /* Set the callback handler functions for related events. */
        handler.on_scan_results_available =
                            gscan_on_scan_results_available;
        handler.on_full_scan_result = gscan_on_full_scan_result;
        handler.on_scan_event = gscan_on_scan_event;

        //ret = wifi_start_gscan(id, wifiHandle_,
        ret = wifi_start_gscan(id, ifaceHandle,
                                params,
                                handler);
        fprintf(stderr, "%s: Sending GSCAN Start request"
            "completed. Returned value: %d.\n", __func__, ret);

        if (ret)
            return;
    }

    void GScanTestSuite::gscanSendStopRequest(int argc, char **argv)
    {
        int ret = 0;
        /* A string listing valid short options letters.  */
        const char* const short_options = "hr:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { "request_id",   1,   NULL, 'r' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };

        /* Override with command line arguements */
        int long_index = 0, opt = 0, i;

        fprintf(stderr, "GScanTestSuite::gscanSendStopRequest: Entry - Sending "
            "GSCAN Stop Request.\n");
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'r' :
                    id = atoi(optarg);
                    break;
                case 'h':
                default:
                    gscanPrintCmdUsage(argv, GSCAN_CMD, GSCAN_STOP,
                        long_options,
                        sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        fprintf(stderr, "GScanTestSuite::gscanSendStopRequest: Stop GSCAN for "
            "request_id:%d.\n", id);
        ret = wifi_stop_gscan(id, ifaceHandle);
        fprintf(stderr, "%s: Sending GSCAN Stop Request completed. "
            "Returned value: %d.\n", __func__, ret);
        if (ret == WIFI_ERROR_NOT_AVAILABLE)
            fprintf(stderr, "GSCAN isn't running or already stopped. ");
    }

    void GScanTestSuite::gscanSendSetBssidHotlistRequest(int argc, char **argv)
    {
        fprintf(stderr, "gscanSendSetBssidHotlistRequest: Sending GSCAN "
            "SetBssidHotlist Request. \n");
        int i, ret = 0;
        /* A string listing valid short options letters.  */
        const char* const short_options = "h:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };
        wifi_gscan_capabilities capa;
        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'h':
                default:
                    gscanPrintCmdUsage(argv, GSCAN_CMD, GSCAN_SET_BSSID_HOTLIST,
                        long_options,
                        sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        fprintf(stderr, "Parse params from "
            "/etc/wifi/gscan_set_hotlist_params.txt \n");
        wifi_hotlist_ap_found_handler handler;
        wifi_bssid_hotlist_params params;

        FILE *f_read = fopen("/etc/wifi/gscan_set_hotlist_params.txt", "r");
        fprintf(stderr, "\n");
        if (f_read) {
            if (fscanf(f_read, "%d", &params.num_ap)) {
                fprintf(stderr, "gscanSendSetBssidHotlistRequest: Parsed "
                    "number of BSSID hotlist:%d\n", params.num_ap);
            }
            for ( i =0; i < params.num_ap; i++ )
            {
                if ( (fscanf(f_read, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &params.ap[i].bssid[0], &params.ap[i].bssid[1],
                        &params.ap[i].bssid[2], &params.ap[i].bssid[3],
                        &params.ap[i].bssid[4], &params.ap[i].bssid[5]) &&
                    fscanf(f_read, "%d", &params.ap[i].low) &&
                    fscanf(f_read, "%d", &params.ap[i].high) &&
                    fscanf(f_read, "%d", &params.ap[i].channel)) == 0 ) {
                    fprintf(stderr, "%s: Failed parsing GSCAN BSSID hotlist "
                        "params file. Exit\n", __func__);
                    return;
                }

                fprintf(stderr, "ap[%d].bssid:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx\n",
                        i,
                        params.ap[i].bssid[0], params.ap[i].bssid[1],
                        params.ap[i].bssid[2],params.ap[i].bssid[3],
                        params.ap[i].bssid[4], params.ap[i].bssid[5]);

                fprintf(stderr, "ap[%d].low:%d\n ap[%d].high:%d\n"
                            "ap[%d].channel:%dMHz\n\n",
                            i, params.ap[i].low,
                            i, params.ap[i].high,
                            i, params.ap[i].channel);
            }
            fclose(f_read);
        } else {
            fprintf(stderr, "gscanSendSetBssidHotlistRequest: Failed to open "
            "file /etc/wifi/gscan_set_hotlist_params.txt  - "
            "use hard-coded defaults\n");

            params.num_ap = 3;
            memcpy(params.ap[0].bssid, "012345", sizeof(mac_addr));
            params.ap[0].low = -80;
            params.ap[0].high = -20;
            params.ap[0].channel = 1;

            memcpy(params.ap[1].bssid, "678901", sizeof(mac_addr));
            params.ap[1].low = -70;
            params.ap[1].high = -30;
            params.ap[1].channel = 11;

            memcpy(params.ap[2].bssid, "333333", sizeof(mac_addr));
            params.ap[2].low = -50;
            params.ap[2].high = -60;
            params.ap[2].channel = 36;
        }

        handler.on_hotlist_ap_found = gscan_on_hotlist_ap_found;
        ret = wifi_set_bssid_hotlist(id, ifaceHandle,
                                    params,
                                    handler);
        fprintf(stderr, "gscanSendSetBssidHotlistRequest: Sending GSCAN Set "
            "Bssid Hotlist request completed. Returned value: %d.\n", ret);

        if (ret)
            return;
    }

    void GScanTestSuite::gscanSendResetBssidHotlistRequest(int argc,
                                                                char **argv)
    {
        int ret = 0;
        /* A string listing valid short options letters.  */
        const char* const short_options = "hr:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { "request_id",   1,   NULL, 'r' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };

        /* Override with command line arguements */
        int long_index = 0, opt = 0, i;

        fprintf(stderr, "%s: Entry - Sending GSCAN Reset Bssid Hotlist "
            "Request.\n", __func__);

        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'r' :
                    id = atoi(optarg);
                    break;
                case 'h':
                default:
                    gscanPrintCmdUsage(argv, GSCAN_CMD,
                        GSCAN_RESET_BSSID_HOTLIST, long_options,
                       sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        fprintf(stderr, "%s: Reset BSSID Hotlist for request_id:%d.\n",
            __FUNCTION__, id);
        ret = wifi_reset_bssid_hotlist(id, ifaceHandle);
        fprintf(stderr, "%s: Sending GSCAN Reset Bssid Hotlist request"
            "completed. Returned value: %d.\n", __FUNCTION__, ret);
    }

    void GScanTestSuite::gscanSendSetSignificantChangeRequest(int argc,
                                                                char **argv)
    {
        fprintf(stderr, "%s: Sending GSCAN SetSignificantChange Request. "
            "\n", __func__);
        int i, ret = 0;
        /* A string listing valid short options letters.  */
        const char* const short_options = "h:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };
        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'h':
                default:
                    gscanPrintCmdUsage(argv, GSCAN_CMD,
                       GSCAN_SET_SIGNIFICANT_CHANGE, long_options,
                       sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        wifi_significant_change_handler handler;
        wifi_significant_change_params params;

        FILE *f_read =
            fopen("/etc/wifi/gscan_set_significant_change_params.txt", "r");
        fprintf(stderr, "\n");
        if (f_read) {
            if ( (fscanf(f_read, "%d", &params.num_ap) &&
                fscanf(f_read, "%d", &params.rssi_sample_size) &&
                fscanf(f_read, "%d", &params.lost_ap_sample_size) &&
                fscanf(f_read, "%d", &params.min_breaching)) == 0 ) {
                fprintf(stderr, "%s: Failed parsing GSCAN Significant "
                    "change params file. Exit\n", __func__);
                return;
            }
            fprintf(stderr, "Number of AP params:%d\n Rssi_sample_size:%d\n "
                "lost_ap_sample_size:%d\n min_breaching:%d\n\n",
                    params.num_ap,
                    params.rssi_sample_size,
                    params.lost_ap_sample_size,
                    params.min_breaching);
            for ( i =0; i < params.num_ap; i++ )
            {
                if ( (fscanf(f_read, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &params.ap[i].bssid[0], &params.ap[i].bssid[1],
                        &params.ap[i].bssid[2], &params.ap[i].bssid[3],
                        &params.ap[i].bssid[4], &params.ap[i].bssid[5]) &&
                    fscanf(f_read, "%d", &params.ap[i].low) &&
                    fscanf(f_read, "%d", &params.ap[i].high) &&
                    fscanf(f_read, "%d", &params.ap[i].channel)) == 0 ) {
                    fprintf(stderr, "%s: Failed parsing GSCAN Significant "
                        "change params file. Exit\n", __func__);
                    return;
                }

                fprintf(stderr, "ap[%d].bssid:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx\n",
                        i,
                        params.ap[i].bssid[0], params.ap[i].bssid[1],
                        params.ap[i].bssid[2],params.ap[i].bssid[3],
                        params.ap[i].bssid[4], params.ap[i].bssid[5]);

                fprintf(stderr, "ap[%d].low:%d\n ap[%d].high:%d\n"
                            "ap[%d].channel:%dMHz\n\n",
                            i, params.ap[i].low,
                            i, params.ap[i].high,
                            i, params.ap[i].channel);
            }
            fclose(f_read);
        } else {
            fprintf(stderr, "gscanSendSetSignificantChangeRequest: Failed to "
            "open file /etc/wifi/gscan_set_significant_change_params.txt  - "
            "use hard-coded defaults\n");

            params.num_ap = 3;
            params.rssi_sample_size = 20;
            params.lost_ap_sample_size = 10;
            params.min_breaching = 5;
            memcpy(params.ap[0].bssid, "543210", sizeof(mac_addr));
            params.ap[0].low = -60;
            params.ap[0].high = -50;
            params.ap[0].channel = 36;

            memcpy(params.ap[1].bssid, "678901", sizeof(mac_addr));
            params.ap[1].low = -70;
            params.ap[1].high = -30;
            params.ap[1].channel = 11;

            memcpy(params.ap[2].bssid, "333333", sizeof(mac_addr));
            params.ap[2].low = -80;
            params.ap[2].high = -20;
            params.ap[2].channel = 1;
        }
        handler.on_significant_change = gscan_on_significant_change;
        ret = wifi_set_significant_change_handler(id, ifaceHandle,
                                                params,
                                                handler);
        fprintf(stderr, "gscanSendSetSignificantChangeRequest: Sending GSCAN "
            "Set Significant Change request completed. Returned value: %d.\n",
            ret);

        if (ret)
            return;
    }

    void GScanTestSuite::gscanSendGetCachedResultsRequest(int argc,
                                                                    char **argv,
                                                                    int max,
                                                                    int flush)
    {
        wifi_scan_result_handler nHandler;
        int ret = 0;
        int num = 0;
        /* A string listing valid short options letters.  */
        const char* const short_options = "h:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };
        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'h':
                default:
                    gscanPrintCmdUsage(argv, GSCAN_CMD,
                        GSCAN_GET_CACHED_RESULTS, long_options,
                       sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        /* if an invalid value is procvided, set to some working default. */
        if (flush < 0) {
            fprintf(stderr, "Invalid flush provided:%d, set to 1\n",
                flush);
            flush = 1;
        }
        /* if an invalid value is procvided, set to some working default. */
        if (max <= 0) {
            fprintf(stderr, "Invalid max provided:%d, set to 1000\n",
                max);
            max = 1000;
        }


        fprintf(stderr, "GScanTestSuite::gscanSendGetCachedResultsRequest: "
            "Sending Get Cached Results Request with i/p values: "
            "flush:%d, max:%d \n", flush, max);
        wifi_scan_result *results = (wifi_scan_result *)
            malloc (max * sizeof(wifi_scan_result));
        if (results)
        {
            memset(results, 0, max * sizeof(wifi_scan_result));
            ret = wifi_get_cached_gscan_results(ifaceHandle,
                    flush, max,
                    results,
                    &num);
            fprintf(stderr, "gscanSendGetCachedResultsRequest: "
                    "Sending Get Cached Results request"
                    "completed. Returned value: %d.\n", ret);

            if (ret) {
                free(results);
                return;
            }
            fprintf(stderr, "Number of cached results returned: %d\n",
                    num);
            wifi_scan_result *result = results;
            int i=0, j=0;
            for(i=0; i< num; i++)
            {
                fprintf(stderr, "TEST-PROXY:  Result : %d\n", i+1);
                fprintf(stderr, "TEST-PROXY:  ts  %lld \n", result->ts);
                fprintf(stderr, "TEST-PROXY:  SSID  %s \n", result->ssid) ;
                fprintf(stderr, "TEST-PROXY:  BSSID: "
                        "%02x:%02x:%02x:%02x:%02x:%02x \n",
                        result->bssid[0], result->bssid[1], result->bssid[2],
                        result->bssid[3], result->bssid[4], result->bssid[5]);
                fprintf(stderr, "TEST-PROXY:  channel %d \n", result->channel);
                fprintf(stderr, "TEST-PROXY:  rssi  %d \n",
                        (signed char) result->rssi);
                fprintf(stderr, "TEST-PROXY:  rtt  %lld \n", result->rtt);
                fprintf(stderr, "TEST-PROXY:  rtt_sd  %lld \n",
                        result->rtt_sd);
                fprintf(stderr, "TEST-PROXY:  beacon period  %d \n",
                        result->beacon_period);
                fprintf(stderr, "TEST-PROXY:  capability  %d \n",
                        result->capability);
                fprintf(stderr, "TEST-PROXY:  IE length  %d \n",
                        result->ie_length);
                fprintf(stderr, "TEST-PROXY:  IE Data \n");
                for(j=0; j<result->ie_length; j++)
                    fprintf(stderr, "%2x \n", result->ie_data[j]);
                /* An extra empty line to made o/p log more readable. */
                fprintf(stderr, "\n");
                result = (wifi_scan_result *)
                    ((u8 *)&results[i] + sizeof(wifi_scan_result) +
                     result->ie_length);
            }
            free(results);
        }
        else
        {
            fprintf(stderr, "TEST-PROXY:  Failed to alloc: \n");
        }
   }

    void GScanTestSuite::gscanSendResetSignificantChangeRequest(int argc,
                                                                char **argv)
    {
        int ret = 0;
        /* A string listing valid short options letters.  */
        const char* const short_options = "hr:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { "request_id",   1,   NULL, 'r' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
            };

        /* Override with command line arguements */
        int long_index = 0, opt = 0, i;

        fprintf(stderr, "%s: Sending GSCAN Reset Significant Change Request. ",
            __FUNCTION__);

        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'r' :
                    id = atoi(optarg);
                    break;
                case 'h':
                default:
                    gscanPrintCmdUsage(argv, GSCAN_CMD,
                        GSCAN_RESET_SIGNIFICANT_CHANGE, long_options,
                           sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }

        fprintf(stderr, "%s: Reset Significant Change for request_id:%d.\n",
            __FUNCTION__, id);

        ret = wifi_reset_significant_change_handler(id, ifaceHandle);
        fprintf(stderr, "%s: Sending GSCAN Reset Significant Change request"
            "completed. Returned value: %d.\n", __FUNCTION__, ret);
    }

    void GScanTestSuite::gscanSendSetMacOui(int argc, char **argv, oui scan_oui)
    {
        int ret = 0;
        fprintf(stderr, "%s: Sending MAC OUI to the wifiHAL. "
            "\n", __FUNCTION__);

        ret = wifi_set_scanning_mac_oui(ifaceHandle, scan_oui);
        if (ret)
        {
            fprintf(stderr, "%s: Set MAC OUI failed.\n", __func__);
            return;
        }
    }

    int GScanTestSuite::gscanParseHex(unsigned char c)
    {
       if (c >= '0' && c <= '9')
          return c-'0';
       if (c >= 'a' && c <= 'f')
          return c-'a'+10;
       if (c >= 'A' && c <= 'F')
          return c-'A'+10;
       return 0;
    }

    int GScanTestSuite::gscanParseMacAddress(const char* arg, u8* addr)
    {
       if (strlen(arg) != 17)
       {
          fprintf(stderr, "Invalid mac address %s\n", arg);
          fprintf(stderr, "expected format xx:xx:xx:xx:xx:xx\n");
          return -1;
       }

       addr[0] = gscanParseHex(arg[0]) << 4 | gscanParseHex(arg[1]);
       addr[1] = gscanParseHex(arg[3]) << 4 | gscanParseHex(arg[4]);
       addr[2] = gscanParseHex(arg[6]) << 4 | gscanParseHex(arg[7]);
       addr[3] = gscanParseHex(arg[9]) << 4 | gscanParseHex(arg[10]);
       addr[4] = gscanParseHex(arg[12]) << 4 | gscanParseHex(arg[13]);
       addr[5] = gscanParseHex(arg[15]) << 4 | gscanParseHex(arg[16]);
       return 0;
    }

}
