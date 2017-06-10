/*
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*
*/

#include "nan_test.hpp"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <netinet/in.h>

#define MAC_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MAC_ADDR_STR "%02x:%02x:%02x:%02x:%02x:%02x"

int gExitflag = 0;

namespace NAN_TEST
{
    /* CLI cmd strings */
    const char *NanTestSuite::NAN_CMD = "nan";
    const char *NanTestSuite::NAN_PUBLISH = "publish";
    const char *NanTestSuite::NAN_SUBSCRIBE = "subscribe";
    const char *NanTestSuite::NAN_ENABLE = "enable";
    const char *NanTestSuite::NAN_DISABLE = "disable";
    const char *NanTestSuite::NAN_PUBLISH_CANCEL = "publish_cancel";
    const char *NanTestSuite::NAN_SUBSCRIBE_CANCEL = "subscribe_cancel";
    const char *NanTestSuite::NAN_CONFIG = "config";
    const char *NanTestSuite::NAN_STATS = "stats";
    const char *NanTestSuite::NAN_TCA = "tca";
    const char *NanTestSuite::NAN_FOLLOWUP = "followup";
    const char *NanTestSuite::NAN_BEACONSDF = "beaconsdf";
    const char *NanTestSuite::NAN_GETSTAPARAMETER = "sta_get_parameter";

    const char *NanTestSuite::DEFAULT_SVC = "GoogleNanCluster";
    const char *NanTestSuite::DEFAULT_SVC_INFO = "GoogleHangout";

    /* Constructor */
    NanTestSuite::NanTestSuite(wifi_handle handle)
        :wifiHandle_(handle)
    {
        NanCallbackHandler callbackHandler =
        {
            .NotifyResponse = NanTestSuite::nanNotifyResponse,
            .EventPublishReplied = NanTestSuite::nanEventPublishReplied,
            .EventPublishTerminated = NanTestSuite::nanEventPublishTerminated,
            .EventMatch = NanTestSuite::nanEventMatch,
            .EventUnMatch = NanTestSuite::nanEventUnMatch,
            .EventSubscribeTerminated = NanTestSuite::nanEventSubscribeTerminated,
            .EventFollowup = NanTestSuite::nanEventFollowup,
            .EventDiscEngEvent = NanTestSuite::nanEventDiscEngEvent,
            .EventDisabled = NanTestSuite::nanEventDisabled,
            .EventTca = NanTestSuite::nanEventTca,
            .EventSdfPayload = NanTestSuite::nanEventSdfPayload
        };

        nan_register_handler(wifiHandle_, callbackHandler);
        gExitflag = 0;
    }

    /* process the command line args */
    void NanTestSuite::processCmd(int argc, char **argv)
    {
        if(argc <3)
        {
            fprintf(stderr, "%s: insufficient NAN args\n", argv[0]);
            return;
        }

        if(strcasecmp(argv[2], NAN_ENABLE) == 0)
            return nanSendEnableRequest(argc, argv);

        if(strcasecmp(argv[2], NAN_DISABLE) == 0)
            return nanSendDisableRequest(argc, argv);

        if(strcasecmp(argv[2], NAN_PUBLISH) == 0)
            return nanSendPublishRequest(argc, argv);

        if(strcasecmp(argv[2], NAN_PUBLISH_CANCEL) == 0)
            return nanSendPublishCancelRequest(argc, argv);

        if(strcasecmp(argv[2], NAN_SUBSCRIBE) == 0)
            return nanSendSubscribeRequest(argc, argv);

        if(strcasecmp(argv[2], NAN_SUBSCRIBE_CANCEL) == 0)
            return nanSendSubscribeCancelRequest(argc, argv);

        if(strcasecmp(argv[2], NAN_CONFIG) == 0)
            return nanSendConfigRequest(argc, argv);

        if(strcasecmp(argv[2], NAN_FOLLOWUP) == 0)
            return nanSendTransmitFollowupRequest(argc, argv);

        if(strcasecmp(argv[2], NAN_STATS) == 0)
            return nanSendStatsRequest(argc, argv);

        if(strcasecmp(argv[2], NAN_TCA) == 0)
            return nanSendTCARequest(argc, argv);

        if(strcasecmp(argv[2], NAN_BEACONSDF) == 0)
            return nanSendBeaconSdfRequest(argc, argv);

        if(strcasecmp(argv[2], NAN_GETSTAPARAMETER) == 0)
            return nanGetStaParameter(argc, argv);

        if(strcasecmp(argv[2], "poll") == 0)
            return nanGetStaParameter(argc, argv);

        fprintf(stderr, "%s: unknown  arg %s\n", argv[0], argv[2]);
    }

    /* NotifyResponse invoked to notify the status of the Request */
    void NanTestSuite::nanNotifyResponse(NanResponseMsg *rsp_data)
    {
        fprintf(stderr, "%s: handle %d status %d value %d response_type %d\n",
          __FUNCTION__,
          rsp_data->header.handle,
          rsp_data->status,
          rsp_data->value,
          rsp_data->response_type
        );

        if(rsp_data->response_type == NAN_RESPONSE_STATS)
        {
           fprintf(stderr, "%s: stats_id %d\n",
             __FUNCTION__,
             rsp_data->body.stats_response.stats_id
           );
           fprintf(stderr, "Exiting...\n");
           exit(0);
        }

        if (rsp_data->response_type == NAN_RESPONSE_ENABLED)
        {
            // Wait for JOINED/STARTED
        }
        else if ((rsp_data->response_type == NAN_RESPONSE_DISABLED) ||
                 (rsp_data->response_type == NAN_RESPONSE_PUBLISH_CANCEL) ||
                 (rsp_data->response_type == NAN_RESPONSE_SUBSCRIBE_CANCEL) ||
                 (rsp_data->response_type == NAN_RESPONSE_TRANSMIT_FOLLOWUP) ||
                 (rsp_data->response_type == NAN_RESPONSE_TCA) ||
                 (rsp_data->response_type == NAN_RESPONSE_BEACON_SDF_PAYLOAD) ||
                 (rsp_data->response_type == NAN_RESPONSE_STATS) ||
                 (rsp_data->response_type == NAN_RESPONSE_CONFIG) ||
                 (rsp_data->response_type == NAN_RESPONSE_BEACON_SDF_PAYLOAD) ||
                 (rsp_data->response_type == NAN_RESPONSE_ERROR) ||
                 (rsp_data->response_type == NAN_RESPONSE_UNKNOWN))
        {
            fprintf(stderr, "Exiting...\n");
            exit(0);
        }
        else if((rsp_data->response_type == NAN_RESPONSE_PUBLISH) ||
                (rsp_data->response_type == NAN_RESPONSE_SUBSCRIBE))
        {
            if (gExitflag)
            {
                fprintf(stderr, "Exiting...\n");
                exit(0);
            }
        }
    }

    /* Events Callback */
    void NanTestSuite::nanEventPublishReplied(NanPublishRepliedInd *event)
    {
        fprintf(stderr, "%s: handle %d " MAC_ADDR_STR " rssi:%d\n",
          __FUNCTION__,
          event->header.handle,
          MAC_ADDR_ARRAY(event->addr),
          event->rssi_value
        );
        /* Print the conn_capability */
        fprintf(stderr, "Printing PostConnectivity Capability \n");
        if (event->is_conn_capability_valid)
        {
            fprintf(stderr, "Wfd supported:%s\n",
                    (event->conn_capability.is_wfd_supported? "yes": "no"));
            fprintf(stderr, "Wfds supported:%s\n",
                    (event->conn_capability.is_wfds_supported? "yes": "no"));
            fprintf(stderr, "TDLS supported:%s\n",
                    (event->conn_capability.is_tdls_supported? "yes": "no"));
            fprintf(stderr, "IBSS supported:%s\n",
                    (event->conn_capability.is_ibss_supported? "yes": "no"));
            fprintf(stderr, "Mesh supported:%s\n",
                    (event->conn_capability.is_mesh_supported? "yes": "no"));
            fprintf(stderr, "Infra Field:%d\n",
                    event->conn_capability.wlan_infra_field);
        }
        else
            fprintf(stderr,"PostConnectivity Capability not present\n");
        /* Print the discovery_attr */
        fprintf(stderr, "Printing PostDiscovery Attribute \n");
        if (event->is_discovery_attr_valid)
        {
            fprintf(stderr,"Conn Type:%d Device Role:%d" MAC_ADDR_STR "\n",
                    event->discovery_attr.type,
                    event->discovery_attr.role,
                    MAC_ADDR_ARRAY(event->discovery_attr.addr));
            fprintf(stderr,"Duration:%d MapId:%d avail_interval_bitmap:%04x \n",
                    event->discovery_attr.duration,
                    event->discovery_attr.mapid,
                    event->discovery_attr.avail_interval_bitmap);
            fprintf(stderr,"Printing Mesh Id:");
            nanhexdump(event->discovery_attr.mesh_id,
                       event->discovery_attr.mesh_id_len);
            fprintf(stderr,"Printing Infrastructure Ssid:");
            nanhexdump(event->discovery_attr.infrastructure_ssid_val,
                       event->discovery_attr.infrastructure_ssid_len);
        }
        else
            fprintf(stderr,"PostDiscovery attribute not present\n");
        /* Print the fam */
        if (event->is_fam_valid)
        {
            nanPrintFurtherAvailabilityMap(&event->fam);
        }
        else
            fprintf(stderr,"Further Availability Map not present\n");
        if(event->cluster_attribute_len)
        {
            fprintf(stderr,"Printing Cluster Attribute:");
            nanhexdump(event->cluster_attribute, event->cluster_attribute_len);
        }
        else
            fprintf(stderr,"Cluster Attribute not present\n");
    }

    /* Events Callback */
    void NanTestSuite::nanEventPublishTerminated(NanPublishTerminatedInd *event)
    {
        fprintf(stderr, "%s: handle %d reason %d\n",
          __FUNCTION__,
         event->header.handle,
         event->reason
        );
    }

    /* Events Callback */
    void NanTestSuite::nanEventMatch(NanMatchInd *event)
    {
        fprintf(stderr, "%s: handle %d match_handle %08x " MAC_ADDR_STR " rssi:%d "
                "Match Occured Flag:%d Out of Resource Flag:%d\n",
          __FUNCTION__,
          event->header.handle,
          event->match_handle,
          MAC_ADDR_ARRAY(event->addr),
          event->rssi_value,
          event->match_occured_flag,
          event->out_of_resource_flag
        );
        /* Print the SSI */
        fprintf(stderr, "Printing SSI:");
        nanhexdump(event->service_specific_info, event->service_specific_info_len);
        /* Print the match filter */
        fprintf(stderr, "Printing sdf match filter:");
        nanhexdump(event->sdf_match_filter, event->sdf_match_filter_len);
        /* Print the conn_capability */
        fprintf(stderr, "Printing PostConnectivity Capability \n");
        if (event->is_conn_capability_valid)
        {
            fprintf(stderr, "Wfd supported:%s\n",
                    (event->conn_capability.is_wfd_supported? "yes": "no"));
            fprintf(stderr, "Wfds supported:%s\n",
                    (event->conn_capability.is_wfds_supported? "yes": "no"));
            fprintf(stderr, "TDLS supported:%s\n",
                    (event->conn_capability.is_tdls_supported? "yes": "no"));
            fprintf(stderr, "IBSS supported:%s\n",
                    (event->conn_capability.is_ibss_supported? "yes": "no"));
            fprintf(stderr, "Mesh supported:%s\n",
                    (event->conn_capability.is_mesh_supported? "yes": "no"));
            fprintf(stderr, "Infra Field:%d\n",
                    event->conn_capability.wlan_infra_field);
        }
        else
            fprintf(stderr,"PostConnectivity Capability not present\n");
        /* Print the discovery_attr */
        fprintf(stderr, "Printing PostDiscovery Attribute \n");
        if (event->is_discovery_attr_valid)
        {
            fprintf(stderr,"Conn Type:%d Device Role:%d" MAC_ADDR_STR "\n",
                    event->discovery_attr.type,
                    event->discovery_attr.role,
                    MAC_ADDR_ARRAY(event->discovery_attr.addr));
            fprintf(stderr,"Duration:%d MapId:%d avail_interval_bitmap:%04x \n",
                    event->discovery_attr.duration,
                    event->discovery_attr.mapid,
                    event->discovery_attr.avail_interval_bitmap);
            fprintf(stderr,"Printing Mesh Id:");
            nanhexdump(event->discovery_attr.mesh_id,
                       event->discovery_attr.mesh_id_len);
            fprintf(stderr,"Printing Infrastructure Ssid:");
            nanhexdump(event->discovery_attr.infrastructure_ssid_val,
                       event->discovery_attr.infrastructure_ssid_len);
        }
        else
            fprintf(stderr,"PostDiscovery attribute not present\n");
        /* Print the fam */
        if (event->is_fam_valid)
        {
            nanPrintFurtherAvailabilityMap(&event->fam);
        }
        else
            fprintf(stderr,"Further Availability Map not present\n");
        if(event->cluster_attribute_len)
        {
            fprintf(stderr,"Printing Cluster Attribute:");
            nanhexdump(event->cluster_attribute, event->cluster_attribute_len);
        }
        else
            fprintf(stderr,"Cluster Attribute not present\n");
    }

    /* Events Callback */
    void NanTestSuite::nanEventUnMatch(NanUnmatchInd *event)
    {
        fprintf(stderr, "%s: handle %d match_handle %08x\n",
          __FUNCTION__,
          event->header.handle,
          event->match_handle
        );
    }

    /* Events Callback */
    void NanTestSuite::nanEventSubscribeTerminated(NanSubscribeTerminatedInd *event)
    {
        fprintf(stderr, "%s: handle %d reason %d\n",
          __FUNCTION__,
         event->header.handle,
         event->reason
        );
    }

    /* Events Callback */
    void NanTestSuite::nanEventFollowup(NanFollowupInd* event)
    {
        fprintf(stderr, "%s: handle %d match_handle %08x dw_or_faw %d "
                MAC_ADDR_STR "\n",
          __FUNCTION__,
          event->header.handle,
          event->match_handle,
          event->dw_or_faw,
          MAC_ADDR_ARRAY(event->addr)
        );

        /* Print the SSI */
        fprintf(stderr, "Printing SSI:");
        nanhexdump(event->service_specific_info, event->service_specific_info_len);
    }

    /* Events Callback */
    void NanTestSuite::nanEventDiscEngEvent(NanDiscEngEventInd *event)
    {
        fprintf(stderr, "%s: handle %d event_id %d\n",
          __FUNCTION__,
         event->header.handle,
         event->event_id
        );

        if(event->event_id == NAN_EVENT_ID_JOINED_CLUSTER)
        {
           fprintf(stderr, "%s: Joined cluster " MAC_ADDR_STR "\n",
             __FUNCTION__,
            MAC_ADDR_ARRAY(event->data.cluster.addr)
           );
        }
        if(event->event_id == NAN_EVENT_ID_STARTED_CLUSTER)
        {
           fprintf(stderr, "%s: Started cluster " MAC_ADDR_STR "\n",
             __FUNCTION__,
            MAC_ADDR_ARRAY(event->data.cluster.addr)
           );
           fprintf(stderr, "Exiting...\n");
           exit(0);
        }
        if(event->event_id == NAN_EVENT_ID_STA_MAC_ADDR)
        {
           fprintf(stderr, "%s: Self STA " MAC_ADDR_STR "\n",
             __FUNCTION__,
            MAC_ADDR_ARRAY(event->data.mac_addr.addr)
           );
        }
    }

    /* Events Callback */
    void NanTestSuite::nanEventDisabled(NanDisabledInd *event)
    {
        fprintf(stderr, "%s: handle %d reason %d\n",
          __FUNCTION__,
         event->header.handle,
         event->reason
        );
    }

    /* Events Callback */
    void NanTestSuite::nanEventTca(NanTCAInd *event)
    {
        fprintf(stderr, "%s: handle %d tca_id %d "
                "threshold risen %d threshold fallen %d\n",
          __FUNCTION__,
         event->header.handle,
         event->tca_id,
         event->rising_direction_evt_flag,
         event->falling_direction_evt_flag
        );

        if(event->tca_id == NAN_TCA_ID_CLUSTER_SIZE)
        {
           fprintf(stderr, "%s: cluster_size %d\n",
             __FUNCTION__,
             event->data.cluster.cluster_size
           );
        }
    }

    void NanTestSuite::nanEventSdfPayload(NanBeaconSdfPayloadInd* event)
    {
        fprintf(stderr, "%s: Received BeaconSdfPayloadIndication \n",
                __FUNCTION__);
        fprintf(stderr, "%s: handle %d addr " MAC_ADDR_STR
                "isVsa %d isBeaconSdfPayload %d",
          __FUNCTION__,
         event->header.handle,
         MAC_ADDR_ARRAY(event->addr),
         event->is_vsa_received,
         event->is_beacon_sdf_payload_received
        );

        if(event->is_vsa_received)
        {
            fprintf(stderr,"%s: Printing VSA************\n", __FUNCTION__);
            fprintf(stderr,"vsa_received_on:%d\n", event->vsa.vsa_received_on);
            fprintf(stderr,"vendor_oui:0x%08x\n", event->vsa.vendor_oui);
            fprintf(stderr,"vsa_len:%d\n", event->vsa.attr_len);
            nanhexdump(event->vsa.vsa, event->vsa.attr_len);
            fprintf(stderr,"%s: Done VSA************\n", __FUNCTION__);
        }

        if(event->is_beacon_sdf_payload_received)
        {
            fprintf(stderr,"%s: Printing BeaconSdfPayloadReceive**********\n",
                    __FUNCTION__);
            fprintf(stderr,"Frame_len:%d\n", event->data.frame_len);
            nanhexdump(event->data.frame_data, event->data.frame_len);
            fprintf(stderr,"%s: Done BeaconSdfPayloadReceive**********\n",
                    __FUNCTION__);
        }
    }

    /* Helper routine to print usage */
    void NanTestSuite::nanPrintCmdUsage(char **argv, const char *cmd,
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

    void NanTestSuite::nanSendEnableRequest(int argc, char **argv)
    {
        /* A string listing valid short options letters.  */
        const char* const short_options = "hc:l:r:s:o:p:g:a:b:d:e:f:i:j:k:m:" \
            "n:q:t:u:v:w:x:y:z:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",                   0,   NULL, 'h' },
            { "cluster_low",            1,   NULL, 'c' },
            { "cluster_high",           1,   NULL, 'l' },
            { "rssi_close",             1,   NULL, 'r' },
            { "rssi_middle",            1,   NULL, 's' },
            { "hop_count_limit",        1,   NULL, 'o' },
            { "master_pref",            1,   NULL, 'p' },
            { "5G_Support",             1,   NULL, 'g' },
            { "2.4G_Support",           1,   NULL, 'a' },
            { "2.4G_Beacon_use",        1,   NULL, 'b' },
            { "2.4G_Disc_use",          1,   NULL, 'd' },
            { "5G_Beacon_use",          1,   NULL, 'e' },
            { "5G_Disc_use",            1,   NULL, 'f' },
            { "5G_rssi_close",          1,   NULL, 'i' },
            { "5G_rssi_middle",         1,   NULL, 'j' },
            { "2.4G_rssi_proximity",    1,   NULL, 'k' },
            { "5G_rssi_proximity",      1,   NULL, 'm' },
            { "rssi_window_val",        1,   NULL, 'n' },
            { "oui_val",                1,   NULL, 'q' },
            { "nan_interface_addr",     1,   NULL, 't' },
            { "nan_cluster_attr_val",   1,   NULL, 'u' },
            { "nan_socialchannel_param",1,   NULL, 'v' },
            { "nan_debugflags_param",   1,   NULL, 'w' },
            { "random_factor_force",    1,   NULL, 'x' },
            { "hop_count_force",        1,   NULL, 'y' },
            { "sid_beacon",             1,   NULL, 'z' },
            { NULL,                     0,   NULL,  0  }/* Required at end of array.  */
        };

        NanEnableRequest req;
        memset(&req, 0, sizeof(NanEnableRequest));
        req.header.handle = 0x0;
        req.header.transaction_id = 0;
        req.support_5g = 0;
        req.cluster_low = 0;
        req.cluster_high = 0;
        req.sid_beacon = 0x01;
        req.rssi_close = 60;
        req.rssi_middle = 70;
        req.rssi_proximity = 70;
        req.hop_count_limit = 2;
        req.random_time = 120;
        req.master_pref = 0;
        req.periodic_scan_interval = 20;

        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
               case 'c' :
                   req.cluster_low = (u16) strtoul(optarg, NULL, 0);
                   break;
               case 'l' :
                   req.cluster_high = (u16) strtoul(optarg, NULL, 0);
                   break;
               case 'r' :
                   req.rssi_close  = atoi(optarg);
                   break;
               case 's' :
                   req.rssi_middle = atoi(optarg);
                   break;
               case 'o' :
                   req.hop_count_limit = atoi(optarg);
                   break;
               case 'p' :
                   req.master_pref = atoi(optarg);
                   break;
               case 'g':
                   req.support_5g = atoi(optarg);
                   break;
               case 'a':
                   req.config_2dot4g_support = 1;
                   req.support_2dot4g_val = atoi(optarg);
                   break;
               case 'b':
                   req.config_2dot4g_beacons = 1;
                   req.beacon_2dot4g_val = atoi(optarg);
                   break;
               case 'd':
                   req.config_2dot4g_discovery = 1;
                   req.discovery_2dot4g_val = atoi(optarg);
                   break;
               case 'e':
                   req.config_5g_beacons = 1;
                   req.beacon_5g_val = atoi(optarg);
                   break;
               case 'f':
                   req.config_5g_discovery = 1;
                   req.discovery_5g_val = atoi(optarg);
                   break;
               case 'i':
                   req.config_5g_rssi_close = 1;
                   req.rssi_close_5g_val = atoi(optarg);
                   break;
               case 'j':
                   req.config_5g_rssi_middle = 1;
                   req.rssi_middle_5g_val = atoi(optarg);
                   break;
               case 'k':
                   req.rssi_proximity = atoi(optarg);
                   break;
               case 'm':
                   req.config_5g_rssi_close_proximity = 1;
                   req.rssi_close_proximity_5g_val = atoi(optarg);
                   break;
               case 'n':
                   req.config_rssi_window_size = 1;
                   req.rssi_window_size_val = atoi(optarg);
                   break;
               case 'q':
               {
                   req.config_oui = 1;
                   u32 oui = 0;
                   size_t size = sizeof(oui);
                   nanParseHexString(optarg, (u8*)&oui,
                                     (int*)&size);
                   req.oui_val = ntohl(oui);
                   break;
               }
               case 't':
                   req.config_intf_addr = 1;
                   nanParseMacAddress(optarg, req.intf_addr_val);
                   break;
               case 'u':
                   req.config_cluster_attribute_val = atoi(optarg);
                   break;
               case 'v':
                   req.config_scan_params = 1;
                   nanParseSocialChannelParams(optarg, &req.scan_params_val);
                   break;
               case 'w':
                   req.config_debug_flags = 1;
                   req.debug_flags_val = strtoull(optarg, NULL, 0);
                   break;
               case 'x':
                   req.config_random_factor_force = 1;
                   req.random_factor_force_val = atoi(optarg);
                   break;
               case 'y':
                   req.config_hop_count_force = 1;
                   req.hop_count_force_val = atoi(optarg);
                   break;
               case 'z':
                   req.sid_beacon = atoi(optarg);
                   break;
               case 'h':
               default:
                   nanPrintCmdUsage(argv, NAN_CMD, NAN_ENABLE, long_options,
                                    sizeof(long_options)/sizeof(struct option));
                   return;
            }
        }

        fprintf(stderr, "%s: hop_count_limit %d master_pref %d\n",
            __FUNCTION__, req.hop_count_limit, req.master_pref);
        fprintf(stderr, "%s: cluster_low 0x%04x cluster_high 0x%04x\n",
            __FUNCTION__, req.cluster_low, req.cluster_high);

        nan_enable_request(0, wifiHandle_, &req);
    }

    void NanTestSuite::nanSendPublishRequest(int argc, char **argv)
    {
        /* A string listing valid short options letters.  */
        const char* const short_options = "ht:p:r:u:x:b:s:i:a:c:d:o:e:z:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",               0,   NULL, 'h' },
            { "ttl",                1,   NULL, 't' },
            { "period",             1,   NULL, 'p' },
            { "publish_match",      1,   NULL, 'r' },
            { "publish_type",       1,   NULL, 'u' },
            { "tx_type",            1,   NULL, 'x' },
            { "publish_count",      1,   NULL, 'b' },
            { "service_name",       1,   NULL, 's' },
            { "ssi",                1,   NULL, 'i' },
            { "rx_match_filter",    1,   NULL, 'a' },
            { "tx_match_filter",    1,   NULL, 'c' },
            { "rssi_threshold",     1,   NULL, 'd' },
            { "ota_flag",           1,   NULL, 'o' },
            { "connmap",            1,   NULL, 'e' },
            { "exitflag",           1,   NULL, 'z' },
            { NULL,                 0,   NULL,  0  }/* Required at end of array.  */
        };

        NanPublishRequest req;
        const char* def_svc = DEFAULT_SVC;
        int size= 0;
        memset(&req, 0, sizeof(NanPublishRequest));
        req.header.handle = 0xFFFF;
        req.header.transaction_id = 0;
        req.ttl = 0;
        req.period = 500;
        req.replied_event_flag = 1;
        req.publish_type = NAN_PUBLISH_TYPE_UNSOLICITED;
        req.tx_type = NAN_TX_TYPE_BROADCAST;
        req.publish_count = 0;
        strlcpy((char*)req.service_name, def_svc, strlen(def_svc) + 1);
        req.service_name_len = strlen(def_svc);

        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
               case 't' :
                   req.ttl = atoi(optarg);
                   break;
               case 'p' :
                   req.period = atoi(optarg);
                   break;
               case 'r' :
                   req.publish_match = (NanMatchAlg)atoi(optarg);
                   break;
               case 'u' :
                   req.publish_type  = (NanPublishType)atoi(optarg);
                   break;
               case 'x' :
                   req.tx_type = (NanTxType)atoi(optarg);
                   break;
               case 'b' :
                   req.publish_count = atoi(optarg);
                   break;
               case 's' :
                   strlcpy((char*)req.service_name, optarg, strlen(optarg) + 1);
                   req.service_name_len = strlen(optarg);
                   break;
               case 'i' :
                   strlcpy((char*)req.service_specific_info, optarg, strlen(optarg) + 1);
                   req.service_specific_info_len = strlen(optarg);
                   break;
               case 'a' :
                   size = NAN_MAX_MATCH_FILTER_LEN;
                   nanParseHexString(optarg, &req.rx_match_filter[0],
                                     (int*)&size);
                   req.rx_match_filter_len = size;
                   break;
               case 'c' :
                   size = NAN_MAX_MATCH_FILTER_LEN;
                   nanParseHexString(optarg, &req.tx_match_filter[0],
                                     (int*)&size);
                   req.tx_match_filter_len = size;
                   break;
               case 'd':
                   req.rssi_threshold_flag = atoi(optarg);
                   break;
               case 'o':
                   req.ota_flag = atoi(optarg);
                   break;
               case 'e':
                   size = sizeof(req.connmap);
                   nanParseHexString(optarg, &req.connmap, &size);
                   break;
               case 'z':
                   gExitflag = atoi(optarg);
                   break;
               case 'h':
               default:
                   nanPrintCmdUsage(argv, NAN_CMD, NAN_PUBLISH, long_options,
                                    sizeof(long_options)/sizeof(struct option));
                   return;
            }
        }

        fprintf(stderr, "%s: service_name %s len %d ssi %s ssi_len %d \n",
            __FUNCTION__, req.service_name, req.service_name_len,
            req.service_specific_info, req.service_specific_info_len);

        nan_publish_request(0, wifiHandle_, &req);
    }

    void NanTestSuite::nanSendDisableRequest(int argc, char **argv)
    {
        /* A string listing valid short options letters.  */
        const char* const short_options = "ht:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { "handle",       1,   NULL, 't' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
        };
        NanDisableRequest req;
        memset(&req, 0, sizeof(NanDisableRequest));
        req.header.handle = 0x0;
        req.header.transaction_id = 0;
        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 't' :
                    req.header.handle = atoi(optarg);
                    break;
                case 'h':
                default:
                    nanPrintCmdUsage(argv, NAN_CMD, NAN_DISABLE, long_options,
                       sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }
        nan_disable_request(0, wifiHandle_, &req);
    }

    void NanTestSuite::nanSendPublishCancelRequest(int argc, char **argv)
    {
        /* A string listing valid short options letters.  */
        const char* const short_options = "ht:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { "handle",       1,   NULL, 't' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
        };
        NanPublishCancelRequest req;
        memset(&req, 0, sizeof(NanPublishCancelRequest));
        req.header.handle = 0;
        req.header.transaction_id = 0;

        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 't' :
                    req.header.handle = atoi(optarg);
                    break;
                case 'h':
                default:
                    nanPrintCmdUsage(argv, NAN_CMD, NAN_PUBLISH_CANCEL, long_options,
                       sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }
        nan_publish_cancel_request(0, wifiHandle_, &req);
    }

    void NanTestSuite::nanSendSubscribeRequest(int argc, char **argv)
    {
        /* A string listing valid short options letters.  */
        const char* const short_options = "ht:p:s:f:g:j:k:m:c:n:i:a:b:r:o:d:e:z:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",               0,   NULL, 'h' },
            { "ttl",                1,   NULL, 't' },
            { "period",             1,   NULL, 'p' },
            { "subscribe_type",     1,   NULL, 's' },
            { "srvrspfilter",       1,   NULL, 'f' },
            { "srvrspincbit",       1,   NULL, 'g' },
            { "usesrvrspfilter",    1,   NULL, 'j' },
            { "ssiinfoneeded",      1,   NULL, 'k' },
            { "subscribe_match",    1,   NULL, 'm' },
            { "subscribe_count",    1,   NULL, 'c' },
            { "service_name",       1,   NULL, 'n' },
            { "ssi",                1,   NULL, 'i' },
            { "rx_match_filter",    1,   NULL, 'a' },
            { "tx_match_filter",    1,   NULL, 'b' },
            { "rssi_threshold",     1,   NULL, 'r' },
            { "ota_flag",           1,   NULL, 'o' },
            { "connmap",            1,   NULL, 'd' },
            { "interface_addr_list",1,   NULL, 'e' },
            { "exitflag",           1,   NULL, 'z' },
            { NULL,                 0,   NULL,  0  }   /* Required at end of array.  */
        };

        NanSubscribeRequest req;
        const char* def_svc = DEFAULT_SVC;
        int size = 0;
        memset(&req, 0, sizeof(NanSubscribeRequest));
        req.header.handle = 0xFFFF;
        req.header.transaction_id = 0;
        req.ttl = 0;
        req.period =  1000;
        req.subscribe_type = 1;
        req.serviceResponseFilter = 0;
        req.serviceResponseInclude = 1;
        req.ssiRequiredForMatchIndication = 0;
        req.subscribe_match = NAN_MATCH_ALG_MATCH_CONTINUOUS;
        req.subscribe_count = 0;
        strlcpy((char*)req.service_name, def_svc, strlen(def_svc) + 1);
        req.service_name_len = strlen(def_svc);

        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
               case 't' :
                   req.ttl = atoi(optarg);
                   break;
               case 'p' :
                   req.period = atoi(optarg);
                   break;
               case 's' :
                   req.subscribe_type  = atoi(optarg);
                   break;
               case 'f' :
                   req.serviceResponseFilter  = atoi(optarg);
                   break;
               case 'g' :
                   req.serviceResponseInclude  = atoi(optarg);
                   break;
               case 'j' :
                   req.useServiceResponseFilter  = atoi(optarg);
                   break;
               case 'k' :
                   req.ssiRequiredForMatchIndication  = atoi(optarg);
                   break;
               case 'm' :
                   req.subscribe_match = (NanMatchAlg)atoi(optarg);
                   break;
               case 'c' :
                   req.subscribe_count = atoi(optarg);
                   break;
               case 'n' :
                   strlcpy((char*)req.service_name, optarg, strlen(optarg) + 1);
                   req.service_name_len = strlen(optarg);
                   break;
               case 'i' :
                   strlcpy((char*)req.service_specific_info, optarg, strlen(optarg) + 1);
                   req.service_specific_info_len = strlen(optarg);
                   break;
               case 'a' :
                   size = NAN_MAX_MATCH_FILTER_LEN;
                   nanParseHexString(optarg, &req.rx_match_filter[0],
                                     (int*)&size);
                   req.rx_match_filter_len = size;
                   break;
               case 'b' :
                   size = NAN_MAX_MATCH_FILTER_LEN;
                   nanParseHexString(optarg, &req.tx_match_filter[0],
                                     (int*)&size);
                   req.tx_match_filter_len = size;
                   break;
               case 'r':
                   req.rssi_threshold_flag = atoi(optarg);
                   break;
               case 'o':
                   req.ota_flag = atoi(optarg);
                   break;
               case 'd':
                   size = sizeof(req.connmap);
                   nanParseHexString(optarg, &req.connmap, &size);
                   break;
               case 'e':
                   req.num_intf_addr_present = \
                       nanParseMacAddresslist(optarg, &req.intf_addr[0][0],
                                              NAN_MAX_SUBSCRIBE_MAX_ADDRESS);
                   break;
               case 'z':
                   gExitflag = atoi(optarg);
                   break;
               case 'h':
               default:
                   nanPrintCmdUsage(argv, NAN_CMD, NAN_SUBSCRIBE, long_options,
                                    sizeof(long_options)/sizeof(struct option));
                   return;
            }
        }
        fprintf(stderr, "%s: service_name %s len %d ssi %s ssi_len %d \n",
            __FUNCTION__, req.service_name, req.service_name_len,
            req.service_specific_info, req.service_specific_info_len);

        nan_subscribe_request(0, wifiHandle_, &req);
    }

    void NanTestSuite::nanSendSubscribeCancelRequest(int argc, char **argv)
    {
        /* A string listing valid short options letters.  */
        const char* const short_options = "ht:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { "handle",       1,   NULL, 't' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
        };
        NanSubscribeCancelRequest req;
        memset(&req, 0, sizeof(NanSubscribeCancelRequest));
        req.header.handle = 128;
        req.header.transaction_id = 0;

        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 't' :
                    req.header.handle = atoi(optarg);
                    break;
                case 'h':
                default:
                    nanPrintCmdUsage(argv, NAN_CMD, NAN_SUBSCRIBE_CANCEL, long_options,
                       sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }
        nan_subscribe_cancel_request(0, wifiHandle_, &req);
    }

    void NanTestSuite::nanSendConfigRequest(int argc, char **argv)
    {
        /* A string listing valid short options letters.  */
        const char* const short_options = "hp:k:m:d:n:v:w:x:y:z:u:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",                   0,   NULL, 'h' },
            { "master_pref",            1,   NULL, 'p' },
            { "2.4G_rssi_proximity",    1,   NULL, 'k' },
            { "5G_rssi_proximity",      1,   NULL, 'm' },
            { "dw_slots",               1,   NULL, 'd' },
            { "rssi_window_val",        1,   NULL, 'n' },
            { "nan_socialchannel_param",1,   NULL, 'v' },
            { "nan_debugflags_param",   1,   NULL, 'w' },
            { "random_factor_force",    1,   NULL, 'x' },
            { "hop_count_force",        1,   NULL, 'y' },
            { "tx_conn_capability",     1,   NULL, 'z' },
            { "nan_cluster_attr_val",   1,   NULL, 'u' },
            { "sid_beacon",             1,   NULL, 1000 },
            { "tx_postdiscovery",       1,   NULL, 1001 },
            { "fam",                    1,   NULL, 1002 },
            { NULL,                     0,   NULL,  0   }/* Required at end of array.  */
        };

        NanConfigRequest req;
        memset(&req, 0, sizeof(NanConfigRequest));
        req.header.handle = 0x0;
        req.header.transaction_id = 0;

        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
               case 'p' :
                   req.config_master_pref = 1;
                   req.master_pref = atoi(optarg);
                   break;
               case 'k':
                   req.config_rssi_proximity = 1;
                   req.rssi_proximity = atoi(optarg);
                   break;
               case 'm':
                   req.config_5g_rssi_close_proximity = 1;
                   req.rssi_close_proximity_5g_val = atoi(optarg);
                   break;
               case 'd' :
                   req.additional_disc_window_slots = atoi(optarg);
                   break;
               case 'n':
                   req.config_rssi_window_size = 1;
                   req.rssi_window_size_val = atoi(optarg);
                   break;
               case 'v':
                   req.config_scan_params = 1;
                   nanParseSocialChannelParams(optarg, &req.scan_params_val);
                   break;
               case 'w':
                   req.config_debug_flags = 1;
                   req.debug_flags_val = strtoull(optarg, NULL, 0);
                   break;
               case 'x':
                   req.config_random_factor_force = 1;
                   req.random_factor_force_val = atoi(optarg);
                   break;
               case 'y':
                   req.config_hop_count_force = 1;
                   req.hop_count_force_val = atoi(optarg);
                   break;
               case 'z':
                   req.config_conn_capability = 1;
                   nanParseTransmitPostConnectivityCapability(optarg,
                       &req.conn_capability_val);
                   break;
               case 'u':
                   req.config_cluster_attribute_val = atoi(optarg);
                   break;
               case 1000:
                   req.config_sid_beacon = 1;
                   req.sid_beacon = atoi(optarg);
                   break;
               case 1001:
                   req.config_discovery_attr = 1;
                   nanParseTransmitPostDiscovery(optarg, &req.discovery_attr_val);
                   break;
               case 1002:
                   req.config_fam = 1;
                   nanParseFurtherAvailabilityMap(optarg, &req.fam_val);
                   break;
               case 'h':
               default:
                   nanPrintCmdUsage(argv, NAN_CMD, NAN_CONFIG, long_options,
                                    sizeof(long_options)/sizeof(struct option));
                   return;
            }
        }
        nan_config_request(0, wifiHandle_, &req);
    }

    int NanTestSuite::nanParseHex(unsigned char c)
    {
       if (c >= '0' && c <= '9')
          return c-'0';
       if (c >= 'a' && c <= 'f')
          return c-'a'+10;
       if (c >= 'A' && c <= 'F')
          return c-'A'+10;
       return 0;
    }

    int NanTestSuite::nanParseMacAddress(const char* arg, u8* addr)
    {
       if (strlen(arg) != 17)
       {
          fprintf(stderr, "Invalid mac address %s\n", arg);
          fprintf(stderr, "expected format xx:xx:xx:xx:xx:xx\n");
          return -1;
       }

       addr[0] = nanParseHex(arg[0]) << 4 | nanParseHex(arg[1]);
       addr[1] = nanParseHex(arg[3]) << 4 | nanParseHex(arg[4]);
       addr[2] = nanParseHex(arg[6]) << 4 | nanParseHex(arg[7]);
       addr[3] = nanParseHex(arg[9]) << 4 | nanParseHex(arg[10]);
       addr[4] = nanParseHex(arg[12]) << 4 | nanParseHex(arg[13]);
       addr[5] = nanParseHex(arg[15]) << 4 | nanParseHex(arg[16]);
       return 0;
    }

    void NanTestSuite::nanSendTransmitFollowupRequest(int argc, char **argv)
    {
        /* A string listing valid short options letters.  */
        const char* const short_options = "ht:a:p:d:i:m:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",           0,   NULL, 'h' },
            { "handle",         1,   NULL, 't' },
            { "addr",           1,   NULL, 'a' },
            { "priority",       1,   NULL, 'p' },
            { "dw_or_faw",      1,   NULL, 'd' },
            { "ssi",            1,   NULL, 'i' },
            { "match_handle",   1,   NULL, 'm' },
            { NULL,             0,   NULL,  0  }   /* Required at end of array.  */
        };

        NanTransmitFollowupRequest req;
        memset(&req, 0, sizeof(NanTransmitFollowupRequest));
        req.header.handle = 0x0;
        req.header.transaction_id = 0;
        req.addr[0] = 0xFF;
        req.addr[1] = 0xFF;
        req.addr[2] = 0xFF;
        req.addr[3] = 0xFF;
        req.addr[4] = 0xFF;
        req.addr[5] = 0xFF;
        req.priority = NAN_TX_PRIORITY_NORMAL;
        req.dw_or_faw = 0;

        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 't' :
                    req.header.handle = atoi(optarg);
                    break;
                case 'a' :
                    nanParseMacAddress(optarg, req.addr);
                    break;
                case 'p' :
                    req.priority = (NanTxPriority)atoi(optarg);
                    break;
                case 'd' :
                    req.dw_or_faw = atoi(optarg);
                    break;
                case 'i' :
                    strlcpy((char*)req.service_specific_info, optarg, strlen(optarg) + 1);
                    req.service_specific_info_len = strlen(optarg);
                    break;
                case 'm' :
                {
                    u32 match_handle = 0;
                    size_t size = sizeof(match_handle);
                    nanParseHexString(optarg, (u8*)&match_handle,
                                     (int*)&size);
                    req.match_handle = ntohl(match_handle);
                    break;
                }
                case 'h':
                default:
                    nanPrintCmdUsage(argv, NAN_CMD, NAN_FOLLOWUP, long_options,
                       sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }
        fprintf(stderr, "%s: ssi %s ssi_len %d " MAC_ADDR_STR "\n",
            __FUNCTION__, req.service_specific_info,
            req.service_specific_info_len, MAC_ADDR_ARRAY(req.addr));

        nan_transmit_followup_request(0, wifiHandle_, &req);
    }

    void NanTestSuite::nanSendStatsRequest(int argc, char **argv)
    {
        /* A string listing valid short options letters.  */
        const char* const short_options = "hi:c:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { "stats_id",     1,   NULL, 'i' },
            { "clear",        1,   NULL, 'c' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
        };
        NanStatsRequest req;
        memset(&req, 0, sizeof(NanStatsRequest));
        req.header.handle = 0x0;
        req.header.transaction_id = 0;
        req.stats_id = NAN_STATS_ID_DE_PUBLISH;
        req.clear = 0;

        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'i' :
                    req.stats_id = (NanStatsId)atoi(optarg);
                    break;
                case 'c' :
                    req.clear = atoi(optarg);
                    break;
                case 'h':
                default:
                    nanPrintCmdUsage(argv, NAN_CMD, NAN_STATS, long_options,
                       sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }
        nan_stats_request(0, wifiHandle_, &req);
    }

    void NanTestSuite::nanSendTCARequest(int argc, char **argv)
    {
        /* A string listing valid short options letters.  */
        const char* const short_options = "hi:r:f:c:t:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",         0,   NULL, 'h' },
            { "tca_id",       1,   NULL, 'i' },
            { "rising",       1,   NULL, 'r' },
            { "falling",      1,   NULL, 'f' },
            { "clear",        1,   NULL, 'c' },
            { "threshold",    1,   NULL, 't' },
            { NULL,           0,   NULL,  0  }   /* Required at end of array.  */
        };
        NanTCARequest req;
        memset(&req, 0, sizeof(NanTCARequest));
        req.header.handle = 0x0;
        req.header.transaction_id = 0;
        req.tca_id = NAN_TCA_ID_CLUSTER_SIZE;

        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 'i' :
                    req.tca_id = (NanTcaId)atoi(optarg);
                    break;
                case 'r' :
                    req.rising_direction_evt_flag = atoi(optarg);
                    break;
                case 'f' :
                    req.falling_direction_evt_flag = atoi(optarg);
                    break;
                case 'c' :
                    req.clear = atoi(optarg);
                    break;
                case 't' :
                    req.threshold = atoi(optarg);
                    break;
                case 'h':
                default:
                    nanPrintCmdUsage(argv, NAN_CMD, NAN_TCA, long_options,
                       sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }
        nan_tca_request(0, wifiHandle_, &req);
    }

    void NanTestSuite::nanSendBeaconSdfRequest(int argc, char **argv)
    {
        /* A string listing valid short options letters.  */
        const char* const short_options = "ht:p:f:o:v:";
        /* An array describing valid long options.  */
        const struct option long_options[] = {
            { "help",           0,   NULL, 'h' },
            { "handle",         1,   NULL, 't' },
            { "payload_tx_flag",1,   NULL, 'p' },
            { "tx_in_flag",     1,   NULL, 'f' },
            { "vendor_oui",     1,   NULL, 'o' },
            { "vsa",            1,   NULL, 'v' },
            { NULL,             0,   NULL,  0  }   /* Required at end of array.  */
        };

        NanBeaconSdfPayloadRequest req;
        memset(&req, 0, sizeof(NanBeaconSdfPayloadRequest));
        req.header.handle = 0x0;
        req.header.transaction_id = 0;
        req.vsa.payload_transmit_flag = 1;
        req.vsa.tx_in_discovery_beacon = 1;
        req.vsa.tx_in_service_discovery = 1;
        req.vsa.tx_in_sync_beacon = 1;
        req.vsa.vendor_oui = 0x001374;
        strlcpy((char*)req.vsa.vsa, "Qualcomm", NAN_MAX_VSA_DATA_LEN);
        req.vsa.vsa_len = strlen("Qualcomm");

        /* Override with command line arguements */
        int long_index = 0, opt = 0;
        while ((opt = getopt_long(argc, argv, short_options,
                long_options, &long_index )) != -1)
        {
            switch (opt)
            {
                case 't' :
                    req.header.handle = atoi(optarg);
                    break;
                case 'p' :
                    req.vsa.payload_transmit_flag = atoi(optarg);
                    break;
                case 'f' :
                    req.vsa.tx_in_discovery_beacon = (atoi(optarg) & 0x01);
                    req.vsa.tx_in_service_discovery = (atoi(optarg) & 0x02) >> 1;
                    req.vsa.tx_in_sync_beacon = (atoi(optarg) & 0x04) >> 2;
                    break;
                case 'o' :
                {
                    req.vsa.vendor_oui = atoi(optarg);
                    u32 oui = 0;
                    size_t size = sizeof(oui);
                    nanParseHexString(optarg, (u8*)&oui,
                                      (int*)&size);
                    req.vsa.vendor_oui = ntohl(oui);
                    break;
                }
                case 'v' :
                    strlcpy((char*)req.vsa.vsa, optarg, strlen(optarg) + 1);
                    req.vsa.vsa_len = strlen(optarg);
                    break;
                case 'h':
                default:
                    nanPrintCmdUsage(argv, NAN_CMD, NAN_BEACONSDF, long_options,
                       sizeof(long_options)/sizeof(struct option));
                    return;
            }
        }
        fprintf(stderr, "%s: oui 0x%08x vsa_len %d vsa %s \n",
            __FUNCTION__, req.vsa.vendor_oui,
            req.vsa.vsa_len, req.vsa.vsa);

        nan_beacon_sdf_payload_request(0, wifiHandle_, &req);
    }

    void NanTestSuite::nanGetStaParameter(int argc, char **argv)
    {
        NanStaParameter rsp;
        wifi_error ret;
        memset(&rsp, 0, sizeof(NanStaParameter));
        ret = nan_get_sta_parameter(0, wifiHandle_, &rsp);
        if(ret == WIFI_ERROR_NONE)
        {
            fprintf(stderr, "%s: NanStaparameter Master_pref:0x%02x," \
                " Random_factor:0x%02x, hop_count:0x%02x " \
                " beacon_transmit_time:0x%08x \n", __FUNCTION__,
                rsp.master_pref,
                rsp.random_factor,
                rsp.hop_count,
                rsp.beacon_transmit_time);
        }
        else
        {
            fprintf(stderr, "%s: Error %d", __FUNCTION__,
                    ret);
        }
    }

    void
    NanTestSuite::nanhexdump(
        uint8_t* data,
        size_t len)
    {
        char buf[512];
        uint16_t index;
        uint8_t* ptr;
        int pos;

        memset(buf, 0, sizeof(buf));
        ptr = data;
        pos = 0;
        for (index=0; index<len; index++)
        {
            pos += snprintf(&(buf[pos]), sizeof(buf) - pos, "%02x ", *ptr++);
            if (pos > 508)
            {
                break;
            }
        }

        fprintf(stderr,"HEX DUMP len=[%d]\n", (int)len);
        fprintf(stderr,"buf:%s\n", buf);
    }

    void NanTestSuite::nanPrintFurtherAvailabilityMap(
        NanFurtherAvailabilityMap* fam)
    {
        fprintf(stderr, "*********Printing FurtherAvailabilityMap*******\n");
        fprintf(stderr, "Numchans:%d NanAvailDuration:%d class_val:%02x channel:%d\n",
                fam->numchans, fam->entry_control, fam->class_val, fam->channel);
        fprintf(stderr, "mapid:%d Availability bitmap:%08x\n",
                fam->mapid, fam->avail_interval_bitmap);
        fprintf(stderr, "Additional Vector elements ");
        nanhexdump(fam->vendor_elements, fam->vendor_elements_len);
        fprintf(stderr, "*********************Done**********************\n");
    }

    int NanTestSuite::nanParseSocialChannelParams(
        const char* arg,
        NanSocialChannelScanParams* scan_params)
    {
        /*
          Social Channel param should have 3 consecutive dwell_time
          and 3 consecutive scan period seperated by "," as the
          delimiter
        */
        char* saveptr;
        char* token;
        int i = 0;
        char* input = (char*)arg;
        for(i = 0; i < MAX_SOCIAL_CHANNELS; i++,input=NULL)
        {
            token = strtok_r(input, ",", &saveptr);
            if (token == NULL)
            {
                fprintf(stderr,"Invalid Argument for SocialChannelParam\n");
                fprintf(stderr,"Expecting 6 integers seperated by \",\"\n");
                return -1;
            }
            scan_params->dwell_time[i] = (u8)atoi(token);
        }
        for(i = 0; i < MAX_SOCIAL_CHANNELS; i++,input=NULL)
        {
            token = strtok_r(NULL, ",", &saveptr);
            if (token == NULL)
            {
                fprintf(stderr,"Invalid Argument for SocialChannelParam\n");
                fprintf(stderr,"Expecting 6 integers seperated by \",\"\n");
                return -1;
            }
            scan_params->scan_period[i] = (u16)atoi(token);
        }
        fprintf(stderr,"********Parsed SocialChannelParam*********\n");
        fprintf(stderr,"dwell_time: %d %d %d scan_period: %d %d %d\n",
                scan_params->dwell_time[0], scan_params->dwell_time[1],
                scan_params->dwell_time[2], scan_params->scan_period[0],
                scan_params->scan_period[1], scan_params->scan_period[2]);
        fprintf(stderr,"********Parsed SocialChannelParam*********\n");
        return 0;
    }

    int NanTestSuite::nanParseTransmitPostConnectivityCapability(
        const char* arg,
        NanTransmitPostConnectivityCapability* conn_capability)
    {
        /*
          NanTransmitPostConnectivityCapability is expected as per
          how it is present in the Nan document
        */
        u32 conn_capability_value = 0;
        size_t size = sizeof(conn_capability_value);
        nanParseHexString(arg, (u8*)&conn_capability_value,
                          (int*)&size);
        conn_capability_value = ntohl(conn_capability_value);

        conn_capability->payload_transmit_flag = ((conn_capability_value & (0x01<<16))? 1:0);
        conn_capability->is_mesh_supported = ((conn_capability_value & (0x01<<5))? 1:0);
        conn_capability->is_ibss_supported = ((conn_capability_value & (0x01<<4))? 1:0);
        conn_capability->wlan_infra_field = ((conn_capability_value & (0x01<<3))? 1:0);
        conn_capability->is_tdls_supported = ((conn_capability_value & (0x01<<2))? 1:0);
        conn_capability->is_wfds_supported = ((conn_capability_value & (0x01<<1))? 1:0);
        conn_capability->is_wfd_supported = ((conn_capability_value & (0x01))? 1:0);
        fprintf(stderr,"********Parsed TxConnCapability*********\n");
        fprintf(stderr,"Input param:0x%08x\n",conn_capability_value);
        fprintf(stderr, "Wfd supported:%s\n",
                (conn_capability->is_wfd_supported? "yes": "no"));
        fprintf(stderr, "Wfds supported:%s\n",
                (conn_capability->is_wfds_supported? "yes": "no"));
        fprintf(stderr, "TDLS supported:%s\n",
                (conn_capability->is_tdls_supported? "yes": "no"));
        fprintf(stderr, "IBSS supported:%s\n",
                (conn_capability->is_ibss_supported? "yes": "no"));
        fprintf(stderr, "Mesh supported:%s\n",
                (conn_capability->is_mesh_supported? "yes": "no"));
        fprintf(stderr, "Infra Field:%d\n",
                conn_capability->wlan_infra_field);
        fprintf(stderr, "Payload Transmit Flag:%d\n",
                conn_capability->payload_transmit_flag);
        fprintf(stderr,"********Parsed TxConnCapability*********\n");
        return 0;
    }

    int NanTestSuite::nanParseTransmitPostDiscovery(
        const char* arg,
        NanTransmitPostDiscovery* discovery_attr)
    {
        /*
          1) Post Discovery should have 4 integers representing
            a) type
            b) role
            c) transmit_freq_flag
            d) duration
          2) FurtherAvailabilityMap hex string
          3) Mac address
          4) 32 octet of meshid which is
          required in case of conn type being WLAN_MESH or
          32 octet of ssid which is required in case of
          conn type being WLAN_INFRA
          All of these seperated by "," as the
          delimiter
        */
        char* saveptr;
        char* token;
        char* input = (char*)arg;

        token = strtok_r(input, ",", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr,"Invalid Argument for TxPostDiscovery\n");
            fprintf(stderr,"Expecting 7 values seperated by \",\"\n");
            return -1;
        }
        discovery_attr->type = (NanConnectionType)atoi(token);
        token = strtok_r(NULL, ",", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr,"Invalid Argument for TxPostDiscovery\n");
            fprintf(stderr,"Expecting 7 values seperated by \",\"\n");
            return -1;
        }
        discovery_attr->role =  (NanDeviceRole)atoi(token);
        token = strtok_r(NULL, ",", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr,"Invalid Argument for TxPostDiscovery\n");
            fprintf(stderr,"Expecting 7 values seperated by \",\"\n");
            return -1;
        }
        discovery_attr->transmit_freq =  (u8)atoi(token);
        token = strtok_r(NULL, ",", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr,"Invalid Argument for TxPostDiscovery\n");
            fprintf(stderr,"Expecting 7 values seperated by \",\"\n");
            return -1;
        }
        discovery_attr->duration =  (NanAvailDuration)atoi(token);
        token = strtok_r(NULL, ",", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr,"Invalid Argument for TxPostDiscovery\n");
            fprintf(stderr,"Expecting 7 values seperated by \",\"\n");
            return -1;
        }
        int fam_bitmap = sizeof(discovery_attr->avail_interval_bitmap);
        nanParseHexString(token, (u8*)&discovery_attr->avail_interval_bitmap,
                          &fam_bitmap);
        discovery_attr->avail_interval_bitmap = \
            ntohl(discovery_attr->avail_interval_bitmap);
        token = strtok_r(NULL, ",", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr,"Invalid Argument for TxPostDiscovery\n");
            fprintf(stderr,"Expecting 7 values seperated by \",\"\n");
            return -1;
        }
        nanParseMacAddress(token, discovery_attr->addr);
        if (discovery_attr->type == NAN_CONN_WLAN_MESH)
        {
            /* Parse the MESH ID */
            token = strtok_r(NULL, ",", &saveptr);
            if(token == NULL)
            {
                fprintf(stderr,"Invalid Argument for TxPostDiscovery\n");
                fprintf(stderr,"Expecting MESH ID\n");
                return -1;
            }
            int mesh_id_len =  NAN_MAX_MESH_DATA_LEN;
            nanParseHexString(token,
                              discovery_attr->mesh_id,
                              &mesh_id_len);
            discovery_attr->mesh_id_len = mesh_id_len;
        }
        if (discovery_attr->type == NAN_CONN_WLAN_INFRA)
        {
            /* Parse the INFRA SSID */
            token = strtok_r(NULL, ",", &saveptr);
            if(token == NULL)
            {
                fprintf(stderr,"Invalid Argument for TxPostDiscovery\n");
                fprintf(stderr,"Expecting Infrastructure Id\n");
                return -1;
            }
            int ssid_len =  NAN_MAX_MESH_DATA_LEN;
            nanParseHexString(token,
                              discovery_attr->infrastructure_ssid_val,
                              &ssid_len);
            discovery_attr->infrastructure_ssid_len = ssid_len;
        }
        /* print the parsed values here */
        fprintf(stderr,"********Parsed TxPostDiscovery*********\n");
        fprintf(stderr,"Connection Type:%d Role:%d transmit freq flag :%d\n",
                discovery_attr->type, discovery_attr->role,
                discovery_attr->transmit_freq);
        fprintf(stderr,"Duration:%d Availability_interval_bitmap:%04x\n",
                discovery_attr->duration, discovery_attr->avail_interval_bitmap);
        fprintf(stderr,"Mac address: " MAC_ADDR_STR "\n",
                MAC_ADDR_ARRAY(discovery_attr->addr));
        if (discovery_attr->type == NAN_CONN_WLAN_MESH)
        {
            fprintf(stderr,"MeshId:");
            nanhexdump(discovery_attr->mesh_id,
                       discovery_attr->mesh_id_len);
        }
        if (discovery_attr->type == NAN_CONN_WLAN_INFRA)
        {
            fprintf(stderr,"Infrastructure ssid:");
            nanhexdump(discovery_attr->infrastructure_ssid_val,
                       discovery_attr->infrastructure_ssid_len);
        }
        fprintf(stderr,"********Parsed TxPostDiscovery*********\n");
        return 0;
    }

    int NanTestSuite::nanParseFurtherAvailabilityMap(
        const char* arg,
        NanFurtherAvailabilityMap* fam)
    {
        /*
          Further availability map contains
          a) AvailDuration - integer
          b) class_val - byte value
          c) channel - byte value
          d) mapid - byte value
          e) Availability bitmap - 32bit value in hex
          All of these seperated by ":" as the delimiter
        */
        /* supporting only one numchans parsing */
        char* saveptr;
        char* token;
        char* input = (char*)arg;

        fam->numchans = 1;
        token = strtok_r(input, ":", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr,"Invalid Argument for FurtherAvailabilityMap\n");
            fprintf(stderr,"Expecting 5 values seperated by \":\"\n");
            return -1;
        }
        fam->entry_control = (NanAvailDuration)atoi(token);
        token = strtok_r(NULL, ":", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr,"Invalid Argument for FurtherAvailabilityMap\n");
            fprintf(stderr,"Expecting 5 values seperated by \":\"\n");
            return -1;
        }
        fam->class_val =  (u8)atoi(token);
        token = strtok_r(NULL, ":", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr,"Invalid Argument for FurtherAvailabilityMap\n");
            fprintf(stderr,"Expecting 5 values seperated by \":\"\n");
            return -1;
        }
        fam->channel =  (u8)atoi(token);
        token = strtok_r(NULL, ":", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr,"Invalid Argument for FurtherAvailabilityMap\n");
            fprintf(stderr,"Expecting 5 values seperated by \":\"\n");
            return -1;
        }
        fam->mapid =  (u8)atoi(token);
        token = strtok_r(NULL, ":", &saveptr);
        if(token == NULL)
        {
            fprintf(stderr,"Invalid Argument for FurtherAvailabilityMap\n");
            fprintf(stderr,"Expecting 5 values seperated by \":\"\n");
            return -1;
        }
        int fam_bitmap = sizeof(fam->avail_interval_bitmap);
        nanParseHexString(token, (u8*)&fam->avail_interval_bitmap,
                          &fam_bitmap);
        fam->avail_interval_bitmap = ntohl(fam->avail_interval_bitmap);
        fam->vendor_elements_len = 0;
        memset(&fam->vendor_elements[0], 0,
               sizeof(fam->vendor_elements));
        nanPrintFurtherAvailabilityMap(fam);
        return 0;
    }

    int NanTestSuite::nanParseHexString(
        const char* input,
        u8* output,
        int* outputlen)
    {
        int i = 0;
        int j = 0;
        for(i = 0; ((i < (int)strlen(input)) && (j < *(outputlen))); i+= 2)
        {
            output[j] = nanParseHex(input[i]);
            if((i + 1) < (int)strlen(input))
            {
                output[j] = ((output[j] << 4) | nanParseHex(input[i + 1]));
            }
            j++;
        }
        *outputlen = j;
        fprintf(stderr, "Input:%s inputlen:%d outputlen:%d\n",
                input, strlen(input), (int)*outputlen);
        return 0;
    }

    int NanTestSuite::nanParseMacAddresslist(
        const char* input,
        u8* output,
        u16 max_addr_allowed)
    {
        /*
         Reads a list of mac address seperated
         by ','. Each mac address should have
         the format of aa:bb:cc:dd:ee:ff
        */
        char* saveptr;
        char* token;
        int i = 0;
        for(i = 0; i < max_addr_allowed; i++)
        {
            token = strtok_r((i ==0)? (char*)input: NULL,
                             ",", &saveptr);
            if (token)
                {
                nanParseMacAddress(token, output);
                output += NAN_MAC_ADDR_LEN;
            }
            else
                break;
        }
        fprintf(stderr, "Num MacAddress:%d\n",i);
        return i;
    }
}

