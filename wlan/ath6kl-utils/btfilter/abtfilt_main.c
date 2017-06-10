/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */

#include "nl80211_utils.h"
#include "abtfilt_int.h"
#include <cutils/properties.h>

const char *progname;
A_CHAR wifname[IFNAMSIZ] = "wlan0";
static ATH_BT_FILTER_INSTANCE g_AthBtFilterInstance;
A_FILE_HANDLE gConfigFile;
static volatile sig_atomic_t terminated;
ATHBT_FILTER_INFO *GpInfo=NULL;
static void
usage(void)
{
    fprintf(stderr, "usage:\n%s [options] \n", progname);
    fprintf(stderr, "  -n   : do not run as a daemon \n");
    fprintf(stderr, "  -d   : enable debug logging \n");
    fprintf(stderr, "  -c   : output debug logs to the console \n");
    fprintf(stderr, "  -a   : issue AFH channel classification when WLAN connects \n");
    fprintf(stderr, "  -f <config file>  : specify configuration file with overrides \n");
    fprintf(stderr, "  -z   : use HCI filtering for headset profile state notifications (Android-Only) \n");
    fprintf(stderr, "  -x   : co-located bt is Atheros \n");
    fprintf(stderr, "  -w <wlan interface>  : wlan adapter name (wlan0/eth1, etc.)\n");
    fprintf(stderr, "  -b   : Use only d-bus filtering (on interfaces which doesnt support hciX\n");
    fprintf(stderr, "  -s   : Front End is single antenna (if not specified, its assumed to be dual antenna with atleast 25 dB of isolation)\n");
    fprintf(stderr, "  -q   : col-located BT is Qualcomm \n");
    fprintf(stderr, "  -t   : Front End is 2x2 dual antenna with shared BT on chain 0. Isolation between antennas is < 20dB\n");
    fprintf(stderr, "  -u   : Front End is un-shared BT and 2x2 WLAN\n");
    fprintf(stderr, "  -m   : WLAN chip is mckinley\n");
}

void
Abf_ShutDown(void)
{
    A_INFO("Shutting Down\n");

    /* Clean up all the resources */
    Abf_BtStackNotificationDeInit(&g_AthBtFilterInstance);
    Abf_WlanStackNotificationDeInit(&g_AthBtFilterInstance);
    AthBtFilter_Detach(&g_AthBtFilterInstance);

    A_INFO("Shutting Down Complete\n");
}

static void
Abf_SigTerm(int sig)
{
        /* unblock main thread */
    terminated = 1;
}

void Abf_BToff()
{
	/* unblock main thread */
	A_INFO("BT turned off\n");
	terminated = 1;
}

A_BOOL BT_terminated(void)
{
        A_CHAR BT_status[PROPERTY_VALUE_MAX] = "";
        int bt_off=0;

        property_get("bluetooth.isEnabled", BT_status, NULL);
        if(strcmp(BT_status, "false") == 0)
                bt_off = 1;

        if (terminated || bt_off)
                return TRUE;
        else
                return FALSE;
}

int
main(int argc, char *argv[])
{
    int ret;
    char *config_file = NULL;
    int opt = 0, daemonize = 1, debug = 0, console_output=0;
    progname = argv[0];
    A_STATUS status;
    struct sigaction sa;
    ATHBT_FILTER_INFO *pInfo;
    A_UINT32 btfiltFlags = 0;

    A_MEMZERO(&g_AthBtFilterInstance, sizeof(ATH_BT_FILTER_INSTANCE));

    /*
     * Keep an option to specify the wireless extension. By default,
     * assume it to be equal to WIRELESS_EXT TODO
     */

    /* Get user specified options */
    while ((opt = getopt(argc, argv, "bsvandczxqtumf:w:")) != EOF) {
        switch (opt) {
        case 'n':
            daemonize = 0;
            break;

        case 'd':
            debug = 1;
            break;

        case 'f':
            if (optarg) {
                config_file = strdup(optarg);
                if (config_file == NULL) {
                    A_ERR("[%s] failed to allocate memory\n", __FUNCTION__);
                    exit(1);
                }
            }
            break;
        case 'c':
            console_output = 1;
            break;
        case 'a':
            btfiltFlags |= ABF_ENABLE_AFH_CHANNEL_CLASSIFICATION;
            break;
        case 'z':
            btfiltFlags |= ABF_USE_HCI_FILTER_FOR_HEADSET_PROFILE;
            break;
        case 'v':
            btfiltFlags |= ABF_WIFI_CHIP_IS_VENUS ;
            A_DEBUG("wifi chip is venus\n");
            break;
        case 'm':
            btfiltFlags |= ABF_WIFI_CHIP_IS_MCKINLEY;
            A_DEBUG("wifi chip is mckinley\n");
            break;
        case 'x':
            btfiltFlags |= ABF_BT_CHIP_IS_ATHEROS ;
            A_DEBUG("bt chip is atheros\n");
            break;
        case 's':
            btfiltFlags |= ABF_FE_ANT_IS_SA ;
            A_DEBUG("Front End Antenna Configuration is single antenna \n");
            break;
        case 'w':
            if (!optarg) {
                A_ERR("[%s] failed to get wlan interface\n", __FUNCTION__);
                exit(1);
            }
            memset(wifname, '\0', IFNAMSIZ);
            strlcpy(wifname, optarg, IFNAMSIZ);
            g_AthBtFilterInstance.pWlanAdapterName = (A_CHAR *)&wifname;
            break;
	case 'b':
	    btfiltFlags |= ABF_USE_ONLY_DBUS_FILTERING;
	    break;
        case 'q':
            btfiltFlags |= ABF_BT_CHIP_IS_QCOM;
            break;
        case 't':
            btfiltFlags |= ABF_FE_ANT_IS_DA_SB_LI;
            break;
        case 'u':
            btfiltFlags |= ABF_FE_ANT_IS_3A;
            break;
        default:
            usage();
            exit(1);
        }
    }

    /* Launch the daemon if desired */
    if (daemonize && daemon(0, console_output ? 1 : 0)) {
        printf("Can't daemonize: %s\n", strerror(errno));
        exit(1);
    }

    /* Initialize the debug infrastructure */
    A_DBG_INIT("ATHBT", "Ath BT Filter Daemon");
    if (debug) {
        if (console_output) {
            A_DBG_SET_OUTPUT_TO_CONSOLE();
        }
       // setlogmask(LOG_INFO | LOG_DEBUG | LOG_ERR);
        A_INFO("Enabling Debug Information\n");
        A_SET_DEBUG(1);
    }

    if (config_file) {
        A_DEBUG("Config file: %s\n", config_file);
        if (!(gConfigFile = fopen(config_file, "r")))
        {
            A_ERR("[%s] fopen failed\n", __FUNCTION__);
        }
    }

    A_MEMZERO(&sa, sizeof(struct sigaction));
    sa.sa_flags = SA_NOCLDSTOP;
    sa.sa_handler = Abf_SigTerm;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);

    Abf_HciLibInit(&btfiltFlags);

    /* Initialize the Filter core */
    do {
        ret = AthBtFilter_Attach(&g_AthBtFilterInstance, btfiltFlags );
        if (ret) {
            A_ERR("Filter initialization failed\n");
            break;
        }

        /* Initialize the WLAN notification mechanism */
        status = Abf_WlanStackNotificationInit(&g_AthBtFilterInstance, btfiltFlags );
        if (A_FAILED(status)) {
            AthBtFilter_Detach(&g_AthBtFilterInstance);
            A_ERR("WLAN stack notification init failed\n");
            break;
        }

        /* Initialize the BT notification mechanism */
        status = Abf_BtStackNotificationInit(&g_AthBtFilterInstance,btfiltFlags);
        if (A_FAILED(status)) {
            Abf_WlanStackNotificationDeInit(&g_AthBtFilterInstance);
            AthBtFilter_Detach(&g_AthBtFilterInstance);
            A_ERR("BT stack notification init failed\n");
            goto deinit_done;
        }

        /* Check for errors on the return value TODO */
        pInfo = g_AthBtFilterInstance.pContext;
        GpInfo = pInfo;

        A_DEBUG("Service running, waiting for termination .... \n");

            /* wait for termination signal */
        while (!terminated) {
            usleep(100000);
        }
    } while(FALSE);

    /* Initiate the shutdown sequence */
    if(GpInfo != NULL) {
        AthBtFilter_State_Off(GpInfo);
    }
    Abf_ShutDown();

deinit_done:
    if(g_AthBtFilterInstance.nlstate)
	btcoex_nl_deinit(g_AthBtFilterInstance.nlstate);

    Abf_HciLibDeInit();
    /* Shutdown */
    if (gConfigFile) {
        fclose(gConfigFile);
    }

    if (config_file) {
        A_FREE(config_file);
    }

    A_DEBUG("Service terminated \n");
    A_MEMZERO(&g_AthBtFilterInstance, sizeof(ATH_BT_FILTER_INSTANCE));
    A_DBG_DEINIT();

    return 0;
}
