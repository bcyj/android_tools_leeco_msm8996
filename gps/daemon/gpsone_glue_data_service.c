/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <linux/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "gpsone_daemon_dbg.h"
#include "gpsone_glue_data_service.h"
#include "gpsone_ctrl_msg.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/route.h>

/*===========================================================================
FUNCTION    gpsone_write_glue

DESCRIPTION
   This is the glue code to write data to socket

   socket_inet - socket fd
   text - data to write
   size - data size

DEPENDENCIES
   None

RETURN VALUE
   number of bytes that is written or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_write_glue(int socket_inet, const void * text, int size)
{
    int result;
    GPSONE_DMN_DBG("%s:%d] socket_inet = %d, text = 0x%lx, size = %d\n", __func__, __LINE__, socket_inet, (long)text, size);
    result = write(socket_inet, text, size);
    GPSONE_DMN_DBG("%s:%d] socket_inet = %d, result = %d\n", __func__, __LINE__, socket_inet, result);
    return result;
}

/*===========================================================================
FUNCTION    gpsone_read_glue

DESCRIPTION
   This is the glue code to read data fromsocket

   socket_inet - socket fd
   text - buffer to hold data
   size - buffer size

DEPENDENCIES
   None

RETURN VALUE
   number of bytes that is read or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_read_glue(int socket_inet, void * text, int size)
{
    int len;
    GPSONE_DMN_DBG("%s:%d] socket_inet = %d, text = 0x%lx, size = %d\n", __func__, __LINE__, socket_inet, (long)text, size);
    len = read(socket_inet, text, size);
    GPSONE_DMN_DBG("%s:%d] socket_inet = %d, len = %d\n", __func__, __LINE__, socket_inet, len);
    return len;
}

/*===========================================================================
FUNCTION    gpsone_connect_glue

DESCRIPTION
   This functon will connect to a specified server

    cmsg_connect - control message that includes server address and port

DEPENDENCIES
   None

RETURN VALUE
   fd for the socket or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_connect_glue(struct ctrl_msg_connect cmsg_connect)
{
    /*if it's a hostname, gethostbyname, find a correct ip addr
    if it's an ip_addr check for v4 or v6*/
    /* sin_addr and sin_port */
    struct sockaddr_in addr_inet;
    struct sockaddr_in6 addr_inet6;
    int len_inet;
    int result;
    int server_socket_inet = -1;



    if ((int)cmsg_connect.ip_addr.type == GPSONE_BIT_IP_V4) {
        memset(&addr_inet, 0, sizeof(addr_inet));
        addr_inet.sin_family = AF_INET;
#ifndef DEBUG_X86
        addr_inet.sin_port = htons(cmsg_connect.ip_port);
        addr_inet.sin_addr.s_addr = htonl(cmsg_connect.ip_addr.addr.v4_addr);
#else
        addr_inet.sin_port = cmsg_connect.ip_port;
        addr_inet.sin_addr.s_addr = cmsg_connect.ip_addr.addr.v4_addr;
#endif

        len_inet = sizeof(addr_inet);

        GPSONE_DMN_DBG("%s:%d] connect to %s:%d\n", __func__, __LINE__, inet_ntoa(addr_inet.sin_addr), cmsg_connect.ip_port);
        if (addr_inet.sin_addr.s_addr == INADDR_NONE) {
        GPSONE_DMN_PR_ERR("%s:%d] invalid address\n", __func__, __LINE__);
        return server_socket_inet;
        }

    server_socket_inet = socket (AF_INET, SOCK_STREAM, 0);
        if (server_socket_inet < 0) {
            GPSONE_DMN_PR_ERR("%s:%d] socket failed\n", __func__, __LINE__);
            GPSONE_DMN_PR_ERR("socket failed: %s\n", strerror(errno));
            return server_socket_inet;
        }

        GPSONE_DMN_DBG("%s:%d] server_socket_inet = %d, len_inet = %d\n", __func__, __LINE__, server_socket_inet, len_inet);
        result = connect(server_socket_inet, (const struct sockaddr *) &addr_inet, len_inet);
    }
    else if ((int)cmsg_connect.ip_addr.type == GPSONE_BIT_IP_V6) {
        memset(&addr_inet6, 0, sizeof(addr_inet6));
        addr_inet6.sin6_family = AF_INET6;
#ifndef DEBUG_X86
        addr_inet6.sin6_port = htons(cmsg_connect.ip_port);
#else
        addr_inet6.sin6_port = cmsg_connect.ip_port;
#endif
        /* is this already converted to network format?? */
        memcpy((void *)&addr_inet6.sin6_addr.s6_addr,
        (void *)&cmsg_connect.ip_addr.addr.v6_addr,
        V6_ADDR_SIZE*sizeof(unsigned char));

        len_inet = sizeof(addr_inet6);

        //GPSONE_DMN_DBG("%s:%d] connect to %s:%d\n", __func__, __LINE__, inet_ntoa(addr_inet6.sin6_addr), addr_inet6.sin6_port);
        /*
        if (addr_inet6.sin6_addr.s6_addr == INADDR_NONE) {
            GPSONE_DMN_PR_ERR("%s:%d] invalid address\n", __func__, __LINE__);
            return server_socket_inet;
        }
        */

        server_socket_inet = socket (AF_INET6, SOCK_STREAM, 0);
        if (server_socket_inet < 0) {
            GPSONE_DMN_PR_ERR("%s:%d] socket failed\n", __func__, __LINE__);
            GPSONE_DMN_PR_ERR("socket failed: %s\n", strerror(errno));
            return server_socket_inet;
        }

        GPSONE_DMN_DBG("%s:%d] server_socket_inet = %d, len_inet = %d\n", __func__, __LINE__, server_socket_inet, len_inet);
        result = connect(server_socket_inet, (const struct sockaddr *) &addr_inet6, len_inet);
    } else {
        GPSONE_DMN_PR_ERR("%s:%d] invalid ip_addr type %d\n", __func__, __LINE__, cmsg_connect.ip_addr.type);
        close(server_socket_inet);
        return -1;
    }

    if ( result < 0) {
        //        extern int errno;
        GPSONE_DMN_PR_ERR("%s:%d] result = %d.\n", __func__, __LINE__, result);
        GPSONE_DMN_PR_ERR("connect failed: %s\n", strerror(errno));
        close(server_socket_inet);
        server_socket_inet = -1;
    }


    return server_socket_inet;
}

/*===========================================================================
FUNCTION    gpsone_disconnect_glue

DESCRIPTION
   This function disconnect the socket from server

   server_socket_inet - fd of the socket

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_disconnect_glue(int server_socket_inet)
{
    return close(server_socket_inet);
}

/*===========================================================================
FUNCTION    get_sock_name

DESCRIPTION
   returns the socket server ip address

   server_sock - socket fd

DEPENDENCIES
   None

RETURN VALUE
   ip address

SIDE EFFECTS
   N/A

===========================================================================*/
unsigned int get_sock_name(int server_sock)
{
    socklen_t addrlen;
    int rc;
    struct sockaddr_in server_addr;
    addrlen = sizeof(server_addr);
    rc = getsockname(server_sock,(struct sockaddr *)&server_addr, &addrlen);

    if(0 != rc)
    {
        GPSONE_DMN_DBG("         %s get sock name error\n", __func__);
    }

    return server_addr.sin_addr.s_addr;
}

/*===========================================================================
FUNCTION    get_peer_name

DESCRIPTION
   This function returns the ip address of the peer

   newclient_sock - client socket fd

DEPENDENCIES
   None

RETURN VALUE
   ip address

SIDE EFFECTS
   N/A

===========================================================================*/
unsigned int get_peer_name(int newclient_sock)
{
    socklen_t addrlen;
    int rc;
    struct sockaddr_in client_addr;
    addrlen = sizeof(client_addr);
    rc = getpeername(newclient_sock, (struct sockaddr *)&client_addr, &addrlen);

    if(0 != rc)
    {
        GPSONE_DMN_DBG("         %s get peer name error\n", __func__);
    }

    return client_addr.sin_addr.s_addr;
}

/*===========================================================================
FUNCTION    set_address

DESCRIPTION
   This is an utility function to convert the address format

   address - input address format in string
   sa - out address format in sockaddr

DEPENDENCIES
   None

RETURN VALUE

SIDE EFFECTS
   N/A

===========================================================================*/
static inline int set_address(const char *address, struct sockaddr *sa) {
    return inet_aton(address, &((struct sockaddr_in *)sa)->sin_addr);
}

/*===========================================================================
FUNCTION    gpsone_get_wifi_ip

DESCRIPTION
   This function get IP address of wifi interface

DEPENDENCIES
   None

RETURN VALUE
   ip address

SIDE EFFECTS
   N/A

===========================================================================*/
/*
static struct in_addr get_wifi_ip(void)
{
    struct in_addr sin_addr;
    struct ifreq ifr;
    int s;

    sin_addr.s_addr = -1;

    if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        GPSONE_DMN_PR_ERR("cannot open control socket\n");
        return sin_addr;
    }

    if (ioctl(s, SIOCGIFADDR, &ifr) < 0) {
        perror(ifr.ifr_name);
    } else
        sin_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;

    return sin_addr;
}
*/

/*===========================================================================
FUNCTION    gpsone_set_ip_route

DESCRIPTION
   This function sets IP route

   sin_addr - server ip address

DEPENDENCIES
   None

RETURN VALUE
   0 for success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_set_ip_route(struct in_addr sin_addr)
{
    int ret = 0;
    struct rtentry rt = {
        .rt_dst     = {.sa_family = AF_INET},
        .rt_genmask = {.sa_family = AF_INET},
        .rt_gateway = {.sa_family = AF_INET},
    };
    rt.rt_flags = RTF_UP | RTF_GATEWAY;
    ((struct sockaddr_in *) &rt.rt_dst)->sin_addr = sin_addr;
    /* ((struct sockaddr_in *) &rt.rt_gateway)->sin_addr = get_wifi_ip(); */
    rt.rt_dev = "rmnet0";

    int s = socket(AF_INET, SOCK_STREAM, 0);

    if (s == -1) {
        ret = -1;
    }
    else if(ioctl(s, SIOCADDRT, &rt) == -1 ) {
        close(s);
        ret = -1;
    }

    return ret;
}
