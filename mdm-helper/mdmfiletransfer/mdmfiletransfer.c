/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  mdmfiletransfer.c : Tool for uploading MSM images using "DMSS-DL" and Sahara protocols.
 *
 */
#include "common.h"
#include "log.h"
#include "dload_protocol.h"
#include "streaming_protocol.h"
#include "comm.h"

#if defined(WINDOWSPC)
#include "windows_utils.h"
#else
#include <getopt.h>
#endif

/* print usage information */
void print_usage ()
{
    printf ("\nUsage: \n");
    printf (
            " -h                     --help                      Display this usage information\n"
            " -p                     --port                      Device name for USB driver\n"
            " -w <path>              --where <path>              Where to save files\n"
            " -u                     --dloadramdump              Collect RAM dumps via Dload protocol\n"
            " -x <hexfile>           --hex <hexfile>             Transfer HEX programmer via Dload protocol\n"
            " -a <file>              --streaming <file>          Transfer file via Streaming protocol\n"
            );
    printf (
             "\n\nExample usage: \n"
             "\tmdmfiletransfer -p /dev/ttyUSB0 -u -w /tmp/ \n"
             "\tmdmfiletransfer -p /dev/ttyUSB0 -x ARMPRG.hex -a amss.mbn\n");
    return;
}

int main (int argc, char *argv[])
{
    int    option;
    int    enable_dload_ram_dump     = 0,
           enable_hex_transfer       = 0,
           enable_streaming_transfer = 0;
    const char *hex_filename       = NULL,
               *streaming_filename = NULL,
               *port_name = NULL;
    boolean retval = FALSE;
    const char *const short_options = "hp:w:ux:a:";             /*possible cmd line short options */
    const struct option long_options[] = {                      /*possible cmd line long options*/
        { "help",    		0, NULL, 'h' },
        { "port",    		1, NULL, 'p' },
        { "where",  		1, NULL, 'w' },
        { "dloadramdump",   0, NULL, 'u' },
        { "hex",            1, NULL, 'x' },
        { "streaming",     	1, NULL, 'a' },
        { NULL,      		0, NULL,  0  }
    };

    /* check argument count, print the usage help and quit */
    if (argc < 2) {
        print_usage ();
        return EXIT_FAILURE;
    }

#if defined(WINDOWSPC)
    optind = 1;
#endif
    /* parse command-line options */
    do {
        option = getopt_long (argc, argv, short_options, long_options, NULL);

        switch (option) {
            case -1:                /* no more option arguments */
                break;

            case 'h':               /* -h or --help */
                print_usage ();
                return 0;

            case 'p':               /* Get the port string name */
                port_name = optarg;
                break;

            case 'w':               /* -w or --where - path for memory dump */
                path_to_save_files = optarg;

                if( path_to_save_files[strlen(path_to_save_files)-1]=='/' )
                    logmsg (LOG_INFO,"PathToSaveFiles=%s", path_to_save_files);
                else
                {
                    logmsg(LOG_ERROR, "Path for memory dump must end with a \"/\"");
                    logmsg(LOG_ERROR, "should be \"-w /path/to/save/memorydump/\"");
                    return EXIT_FAILURE;
                }
                break;

            case 'u':
                enable_dload_ram_dump = 1;
                break;

            case 'x':
                hex_filename = optarg;
                enable_hex_transfer = 1;
                break;

            case 'a':               /* -a or --streaming */
                streaming_filename = optarg;
                enable_streaming_transfer = 1;
                break;

            default:                /* unknown option. */
                logmsg (LOG_ERROR, "unrecognized option");
                print_usage ();
                return EXIT_FAILURE;
        }
    } while (option != -1);

    if (enable_dload_ram_dump || enable_hex_transfer) {
        port_comm.port_name = port_name;
        port_comm.protocol_name = DLOAD_PROTOCOL;
        default_rx_timeout = 0;
        default_rx_usecs = 100000LL;
        if (TRUE == connect_port () && TRUE == configure_port () && TRUE == dload_init_buffers () && TRUE == dload_ping_target ()) {
            if (enable_dload_ram_dump)
                retval = collect_ram_dumps ();
            else if (enable_hex_transfer)
                retval = transfer_hex_file (hex_filename);
        }
        free_buffers_and_disconnect ();
        if (FALSE == retval)
            return EXIT_FAILURE;
    }
    if (enable_streaming_transfer) {
        if (enable_hex_transfer)
            sleep (4);
        port_comm.port_name = port_name;
        port_comm.protocol_name = STREAMING_PROTOCOL;
        default_rx_timeout = 0;
        default_rx_usecs = 500000LL;
        if (TRUE == connect_port () && TRUE == configure_port () && TRUE == streaming_init_buffers () && TRUE == streaming_ping_target ())
            retval = begin_streaming_nor (streaming_filename);
        free_buffers_and_disconnect ();
        if (FALSE == retval)
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
