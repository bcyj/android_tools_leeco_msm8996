/*
 * Kickstart: A utility for uploading MSM images using the "DMSS-DL"
 * and Sahara protocols
 *
 * Copyright (C) 2012 Qualcomm Technologies, Inc. All rights reserved.
 *                    Qualcomm Technologies Proprietary/GTDR
 *
 * All data and information contained in or disclosed by this document is
 * confidential and proprietary information of Qualcomm Technologies, Inc. and all
 * rights therein are expressly reserved.  By accepting this material the
 * recipient agrees that this material and the information contained therein
 * is held in confidence and in trust and will not be used, copied, reproduced
 * in whole or in part, nor its contents revealed in any manner to others
 * without the express written permission of Qualcomm Technologies, Inc.
 *
 *
 *  kickstart.c : Tool for uploading MSM images using "DMSS-DL" and Sahara protocols.
 * ==========================================================================================
 *   $Header: //source/qcom/qct/core/storage/tools/kickstart/common/kickstart/kickstart/kickstart.c#2 $
 *   $DateTime: 2013/10/23 16:42:51 $
 *   $Author: abrahma $
 *
 *  Edit History:
 *  YYYY-MM-DD       who     why
 *  -----------------------------------------------------------------------------
 *  2010-09-28       ng      Added command mode support
 *  2010-10-18       ab      Added memory debug mode support
 *
 *  Copyright 2012 by Qualcomm Technologies, Inc.  All Rights Reserved.
 *
 *==========================================================================================
 */


#include "common_protocol_defs.h"
#include "comm.h"
#include "sahara_protocol.h"
#include "kickstart_utils.h"
#include "kickstart_log.h"

#ifdef WINDOWSPC

    #include "windows_utils.h"

#else
    /* For capabilities */
    #include <sys/prctl.h>
    #include <sys/capability.h>
    #include <private/android_filesystem_config.h>

    /* System properties (set using setprop and read by get prop on shell) */
    #include <cutils/properties.h>

    /* This *MUST* be of size PROPERTY_VALUE_MAX */
    static char prop_ks_verbose_value[PROPERTY_VALUE_MAX];
#endif

// Unfortunately, VC2010 doesn't support member-wise initialization
// unlike some compilers
kickstart_options_t kickstart_options = {
    "",   // path_to_save_files
    "",   // saved_file_prefix
    10,   // port_connect_retries
    0     // verbose
};

void usage()
{
	printf("Built %s %s\n",__DATE__,__TIME__);
	show_sahara_mapping_list();
	show_search_paths();
    printf("Usage options:\n");
    printf(
             " -v                     --verbose                   Print verbose messages\n"
             " -m                     --memdump                   Force Sahara memory debug mode\n"
             " -n                     --noreset                   Disable sending the sahara reset PKT\n"
             " -q <img_id>            --quitafter                 Force kickstart to exit after transmitting img_id\n"
			 " -c <command_id>        --command                   Force Sahara command mode\n"
             " -p                     --port                      COM Port / TTY device to use\n"
             " -r                     --ramdumpimage              Image ID which must be transferred before forcing Sahara memory dump mode\n"
			 " -w                     --where                     Path to store files received from target\n"
             " -b                     --searchpath                Where to look for files\n"
			 " -j <size_in_kb>        --maxtransfersize			  Specify size in KB\n"
             " -s <img_id:file_name>  --sahara <img_id:file_name> Specify Sahara protocol file mapping\n");
    printf(
             "\n\nNOTE: Only use -s option if you need to overwrite the default file mappings above"
             "\n\nExample usage: \n"
             "\tkickstart.exe -p \\\\.\\COM19 -s 13:ENPRG9x25.mbn -q 13\n"
             "\tsudo kickstart -p /dev/ttyUSB0 -s 11:my_osbl.mbn -s 2:testamss.mbn\n");

    printf(
"\n\nCALLING: kickstart.exe to retrieve serial number"
"\nkickstart.exe -r 21 -c 1 -w c:\\temp\\PATHFORMDMUPLOADS\\ -p \\\\.\\COM19 -s 16:efs1.bin -s 17:efs2.bin -s 20:efs3.bin -b c:\\temp\\PATHFORMDMUPLOADS\\ -b c:\\fusiononpc\\9x15_images\\ "

"\n\nCALLING: kickstart.exe for image transfer"
"\nkickstart.exe -r 21 -w c:\\temp\\PATHFORMDMUPLOADS\\2d966200\\ -p \\\\.\\COM19 -s 16:m9kefs1 -s 17:m9kefs2 -s 20:efs3.bin -b c:\\temp\\PATHFORMDMUPLOADS\\2d966200\\ -b c:\\fusiononpc\\9x15_images\\ "

"\n\nCALLING: kickstart.exe for EFS sync"
"\nkickstart.exe -l -a 32 -m -v -w c:\\temp\\PATHFORMDMUPLOADS\\2d966200\\ -p \\\\.\\COM21 -t -1 -g m9k1_ > efs_sync_messages.txt\n\n");

    return;
}

#ifndef WINDOWSPC
int read_verbosity_property (int default_value)
{
	int result, property_val;

    result = property_get("persist.kickstart.verbose",
                          prop_ks_verbose_value, "");

    if ((result < 0) || (prop_ks_verbose_value[0] == '\0'))
        property_val = default_value;
    else
        property_val = atoi(prop_ks_verbose_value);

    return property_val;
}
#endif

int main (int argc, char *argv[])
{
    int    option;                                            // holds the option from getopt_long
    const char *const short_options = "hp:v:c:mis:g:w:r:lt:j:b:n";  // possible cmd line short options
    const struct option long_options[] = {                    // possible cmd line long options
        { "help",           0, NULL, 'h' },
        { "port",           1, NULL, 'p' },
        { "verbose",        1, NULL, 'v' },
        { "command",        1, NULL, 'c' },
        { "memdump",        0, NULL, 'm' },
        { "image",          0, NULL, 'i' },
        { "sahara",         1, NULL, 's' },
        { "prefix",         1, NULL, 'g' },
        { "where",          1, NULL, 'w' },
        { "ramdumpimage",   1, NULL, 'r' },
        { "efssyncloop",    0, NULL, 'l' },
        { "rxtimeout",      1, NULL, 't' },
        { "maxwrite",       1, NULL, 'j' },
        { "addsearchpath",  1, NULL, 'b' },
        { "noreset",        0, NULL, 'n' },
        { NULL,             0, NULL,  0  }
    };

    bool efs_sync = false;
    unsigned int i;
    bool   enable_sahara_transfer = false;

#ifndef WINDOWSPC
    unsigned long cap;
    int err;
    int rc;
    struct __user_cap_header_struct capheader;
    struct __user_cap_data_struct capdata[2];


    if (prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0) != 0) {
        dbg(LOG_WARN, "set keepcaps failed!");
    }
    for (cap = 0; prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0; cap++) {
        if ((cap == CAP_SETUID) || (cap == CAP_SETGID) || (cap == CAP_BLOCK_SUSPEND)) {
            continue;
        }
        err = prctl(PR_CAPBSET_DROP, cap, 0, 0, 0);
        if ((err < 0) && (errno != EINVAL)) {
            dbg(LOG_WARN, "Drop capability %d failed\n", cap);
        }
    }

    if (setgid(AID_SYSTEM) != 0) {
      dbg(LOG_WARN, "setgid failed");
    }
    else {
      if (setuid(AID_SYSTEM) != 0) {
        dbg(LOG_WARN, "setuid failed");
      }
      else {
          memset(&capheader, 0, sizeof(capheader));
          memset(&capdata, 0, sizeof(capdata));
          capheader.version = _LINUX_CAPABILITY_VERSION_3;
          capheader.pid = 0;
          capdata[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].permitted |= CAP_TO_MASK(CAP_BLOCK_SUSPEND);
          capdata[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].effective |= CAP_TO_MASK(CAP_BLOCK_SUSPEND);

          if ((rc = capset(&capheader, capdata)) < 0) {
              dbg(LOG_WARN, "capset failed: %s, rc = %d\n", strerror(errno), rc);
          }
      }
    }
#endif

	if (false == init_search_path_list() || false == init_sahara_mapping_list()) {
		dbg(LOG_ERROR, "Could not initialize.");
		return EXIT_FAILURE;
	}

    /* parse command-line options */
    do {
        option = getopt_long (argc, argv, short_options, long_options, NULL);

        switch (option) {
        case -1:                /* no more option arguments */
            break;

        case 'h':               /* -h or --help */
            usage();
            return EXIT_SUCCESS;

        case 'p':               /* Get the port string name */
            com_port.port_name = optarg;
            dbg(LOG_INFO, "Port name '%s'", com_port.port_name);
            break;

        case 's':               /* -s or --sahara */
            /*add the input to <id,file_name> list*/
            if (false == add_input_to_sahara_mapping_list(optarg)) {
               dbg(LOG_ERROR, "Failed to add file to file list");
               return EXIT_FAILURE;
            }
            enable_sahara_transfer = true;
            break;

		case 'b':
            if (false == add_search_path(optarg)) {
               dbg(LOG_ERROR, "Failed to add to search path list");
               return EXIT_FAILURE;
            }
            break;

        case 'i':               /* -i or --image */
            sahara_data.mode = SAHARA_MODE_IMAGE_TX_PENDING;
			enable_sahara_transfer = true;
            break;

        case 'v':               /* -v or --verbose */
            kickstart_options.verbose = atoi(optarg);
            break;

        case 'm':               /* -m or --memdump */
            sahara_data.mode = SAHARA_MODE_MEMORY_DEBUG;
            enable_sahara_transfer = true;
            break;

        case 'r':               /* -r or --ramdumpimage */
            sahara_data.ram_dump_image = atoi(optarg);
			enable_sahara_transfer = true;
            break;

        case 'g':               /* -g or --prefix */
            kickstart_options.saved_file_prefix = optarg;
            break;

        case 'w':               /* -w or --where - path for memory dump */
            kickstart_options.path_to_save_files = optarg;
            break;

        case 'c':               /* -c or --command */
            sahara_data.mode = SAHARA_MODE_COMMAND;
			sahara_data.command = atoi(optarg);
			enable_sahara_transfer = true;
            break;

        case 'l':               /* -l or --loop */
            efs_sync = true;
			sahara_data.mode = SAHARA_MODE_MEMORY_DEBUG;
			com_port.rx_timeout = -1;
			enable_sahara_transfer = true;
            break;

        case 't':
            com_port.rx_timeout = atoi(optarg);
            break;

		case 'j':               /* -c or --command */
			com_port.MAX_TO_WRITE = atoi(optarg);
            break;

        case 'n':               /* -n or --noreset */
			sahara_data.allow_sahara_reset = false;
            break;

        default:                /* unknown option. */
            dbg(LOG_ERROR, "unrecognized option: '%c'", option);
            usage ();
            return EXIT_FAILURE;
        }
    } while (option != -1);

    #ifndef WINDOWSPC
    /* After parsing the command line args try to change the verbosity level of
       the logs if the system property was set. */
       kickstart_options.verbose = read_verbosity_property (kickstart_options.verbose);
    #endif

	if (true == enable_sahara_transfer) {
        if (NULL == com_port.port_name) {
            dbg(LOG_ERROR, "Port device name not specified; use -p option.");
            return EXIT_FAILURE;
        }

        for (i = 0; i < kickstart_options.port_connect_retries; i++) {
            if (true == port_connect(com_port.port_name)) {
                break;
            }
        }
        if (kickstart_options.port_connect_retries == i) {
            dbg(LOG_ERROR, "Could not connect to %s", com_port.port_name);
            return EXIT_FAILURE;
        }

        // This is a little hacky. Ideally the timeout values should be passed in
        // as an argument, but since they don't change too often, hardcoding them
        // here for now
        if (efs_sync) {
          com_port.rx_timeout_sec = 0;
          com_port.rx_timeout_usec = 500000;
          dbg(LOG_STATUS, "Setting timeout to 500ms");
        }
        else {
          com_port.rx_timeout_sec = 2;
          com_port.rx_timeout_usec = 0;
          dbg(LOG_STATUS, "Setting timeout to 2s");
        }

        if (false == sahara_main (efs_sync))
        {
           dbg(LOG_ERROR, "Uploading  Image using Sahara protocol failed");
           use_wakelock(WAKELOCK_RELEASE);
           port_disconnect();
           return EXIT_FAILURE;
        }
    }

    port_disconnect();
    return EXIT_SUCCESS;
}
