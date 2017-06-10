/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "gpsone_conn_client.h"
#include "gpsone_glue_data_service.h"
#include "gpsone_daemon_dbg.h"
#include "gpsone_ctrl_msg.h"

/*===========================================================================
FUNCTION    gpsone_check_routing

DESCRIPTION
   This function will check the routing table

    sin_addr - the addr

DEPENDENCIES
   None

RETURN VALUE
   ip address

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_check_routing(struct in_addr sin_addr)
{
    int result = 0;
    static unsigned char is_routing_set = 1; /* @todo enable this */
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    //Check to see if the route is added for this IP connection
    // Will handle only one connection
    if(!is_routing_set) {
        result = gpsone_set_ip_route(sin_addr);
        if(result != 0) {
            return result;
        }
        is_routing_set = 1;
    }

    return 0;
}

/*===========================================================================
FUNCTION    gpsone_client_connect

DESCRIPTION
   This function will connect to the specified server

    ctrl_msg_connect - IP v4/v6 union

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_client_connect(struct ctrl_msg_connect cmsg_connect)
{
    int server_soc;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    /* Do this through java connectivity manager
    if(gpsone_check_routing(sin_addr) != 0) {
        return -1;
    }
    */

    server_soc = gpsone_connect_glue(cmsg_connect);

    return server_soc;
}

/*===========================================================================
FUNCTION    gpsone_client_disconnect

DESCRIPTION
   This function will disconnect from the server

    server_socket_inet - socket fd of the connection

DEPENDENCIES
   None

RETURN VALUE
   0 for success or any other value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_client_disconnect(int server_socket_inet)
{
    int result;
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    result = gpsone_disconnect_glue(server_socket_inet);
    if (result < 0) {
        GPSONE_DMN_DBG("         close server_socket_inet=%d failed\n", server_socket_inet);
    } else {
        GPSONE_DMN_DBG("         close server_socket_inet=%d success\n", server_socket_inet);
    }
    return result;
}

/*===========================================================================
FUNCTION    sock_name

DESCRIPTION
   This function returns the ip address associated with the socket

   server_sock - server socket fd

DEPENDENCIES
   None

RETURN VALUE
   ip address

SIDE EFFECTS
   N/A

===========================================================================*/
unsigned int sock_name(int server_sock)
{
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    return get_sock_name(server_sock);
}

/*===========================================================================
FUNCTION    peer_name

DESCRIPTION
   This function will return the peer ip address associated with the socket

    newclient_soc - socket fd

DEPENDENCIES
   None

RETURN VALUE
   ip address

SIDE EFFECTS
   N/A

===========================================================================*/
unsigned int peer_name(int newclient_soc)
{
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    return get_peer_name(newclient_soc);
}
