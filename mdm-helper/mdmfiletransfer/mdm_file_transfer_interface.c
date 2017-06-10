/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2009-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 *  mdm_file_transfer_interface.c : Interface to dload ram dump collection and
 *  streaming download
 *
 */
#include "common.h"
#include "log.h"
#include "dload_protocol.h"
#include "streaming_protocol.h"
#include "comm.h"
#include "mdm_file_transfer_interface.h"
#if defined(WINDOWSPC)
#include "windows_utils.h"
#else
#include <getopt.h>
#endif

int get_dload_status(char *port)
{
    port_comm.port_name = port;
    port_comm.protocol_name = DLOAD_PROTOCOL;
    if( connect_port() != TRUE){
        logmsg(LOG_STATUS,"Connect port failed");
        return -1;
    }
    if( configure_port() != TRUE ){
        logmsg(LOG_STATUS,"Configure port failed");
        disconnect_port();
        return -1;
    }
    if(dload_init_buffers() != TRUE){
        logmsg(LOG_STATUS, "dload init buffers failed");
        disconnect_port();
        return -1;
    }
    if(dload_ping_target() != TRUE){
        logmsg(LOG_STATUS, "dload ping target is FALSE,Not in DLOAD mode");
        free_buffers();
        disconnect_port();
        return 0;
    }
    logmsg(LOG_STATUS, "Device is IN DLOAD MODE");
    free_buffers();
    disconnect_port();
    return 1;
}

int save_ram_dumps(char *port, char *destination_path)
{
    int retval = FALSE;
    default_rx_timeout = 0;
    default_rx_usecs = 100000LL;
    if(!*port | !*destination_path){
        logmsg(LOG_STATUS, "Invalid port/destination path");
        return -1;
    }
    port_comm.port_name = port;
    port_comm.protocol_name = DLOAD_PROTOCOL;
    if( connect_port() != TRUE){
        logmsg(LOG_STATUS,"Connect port failed");
        return -1;
    }
    if( configure_port() != TRUE ){
        logmsg(LOG_STATUS,"Configure port failed");
        disconnect_port();
        return -1;
    }
    if(dload_init_buffers() != TRUE){
        logmsg(LOG_STATUS, "dload init buffers failed");
        disconnect_port();
        return -1;
    }
    if(dload_ping_target() != TRUE){
        logmsg(LOG_STATUS, "dload ping target is FALSE,Not in DLOAD mode");
        free_buffers();
        disconnect_port();
        return -1;
    }
    logmsg(LOG_STATUS, "Starting Collecting Ram dumps");
    path_to_save_files = destination_path;
    retval = collect_ram_dumps();
    if (retval == FALSE){
        logmsg(LOG_STATUS, "Failed to collect dumps");
        free_buffers();
        disconnect_port();
        return -1;
    }
    logmsg(LOG_STATUS, "Dump collection successful");
    free_buffers();
    disconnect_port();
    return 0;
}

int program_nor_image(char *port, char *hex_filename, char *image_path)
{
    int retval = FALSE;
    default_rx_timeout = 0;
    default_rx_usecs = 500000LL;
    if(!*port | !*image_path){
        logmsg(LOG_STATUS, "Invalid port/destination path");
        return -1;
    }

    port_comm.port_name = port;
    port_comm.protocol_name = DLOAD_PROTOCOL;
    if( connect_port() != TRUE){
        logmsg(LOG_STATUS,"Connect port failed");
        return -1;
    }
    if( configure_port() != TRUE ){
        logmsg(LOG_STATUS,"Configure port failed");
        disconnect_port();
        return -1;
    }
    if(dload_init_buffers() != TRUE){
        logmsg(LOG_STATUS, "dload init buffers failed");
        disconnect_port();
        return -1;
    }
    if(dload_ping_target() != TRUE){
        logmsg(LOG_STATUS, "dload ping target is FALSE,Not in DLOAD mode");
        free_buffers();
        disconnect_port();
        return -1;
    }
    logmsg(LOG_STATUS, "Starting HEX file transfer");
    retval = transfer_hex_file(hex_filename);
    if (retval == FALSE){
        logmsg(LOG_STATUS, "Failed to transfer HEX file");
        free_buffers();
        disconnect_port();
        return -1;
    }
    logmsg(LOG_STATUS, "HEX file transfer successful");
    free_buffers();
    disconnect_port();

    usleep(4000 * 1000);

    port_comm.port_name = port;
    port_comm.protocol_name = STREAMING_PROTOCOL;
    if( connect_port() != TRUE){
        logmsg(LOG_STATUS,"Connect port failed");
        return -1;
    }
    if( configure_port() != TRUE ){
        logmsg(LOG_STATUS,"Configure port failed");
        disconnect_port();
        return -1;
    }
    if(streaming_init_buffers() != TRUE){
        logmsg(LOG_STATUS, "streaming init buffers failed");
        disconnect_port();
        return -1;
    }
    if(streaming_ping_target() != TRUE){
        logmsg(LOG_STATUS, "streaming ping target is FALSE,Not in STREAMING mode");
        free_buffers();
        disconnect_port();
        return -1;
    }
    logmsg(LOG_STATUS, "Starting NOR image programming");
    retval = begin_streaming_nor(image_path);
    if (retval == FALSE){
        logmsg(LOG_STATUS, "Failed to program NOR image");
        free_buffers();
        disconnect_port();
        return -1;
    }
    logmsg(LOG_STATUS, "NOR image programming successful");
    free_buffers();
    disconnect_port();
    return 0;
}
