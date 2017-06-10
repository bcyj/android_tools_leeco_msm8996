/*
* Copyright (c) 2011-2012 Qualcomm Atheros Inc. All Rights Reserved.
* Qualcomm Atheros Proprietary and Confidential.
*/

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <time.h>
#include "artagent.h"
#include "libtcmd.h"

static int sid, cid, aid;
static char ar6kifname[32];
unsigned int readLength = 0;
unsigned char  line[LINE_ARRAY_SIZE];
int cmdId,cmdLen;

void callback_rx(void *buf, int len)
{
    readLength = len;
    memcpy(line, buf, len);
}

static int initInterface(char *ifname)
{
    int err = 0;
    err = tcmd_tx_init(ifname, callback_rx);
    return err;
}

static int wmiSend(unsigned char *cmdBuf, unsigned int len, unsigned int totalLen, unsigned char version, bool resp)
{
    TC_CMDS tCmd;
    int err=0;

    memset(&tCmd,0,sizeof(tCmd));

    tCmd.hdr.testCmdId = TC_CMDS_ID;
    tCmd.hdr.u.parm.length = totalLen;
    tCmd.hdr.u.parm.version = version;
    tCmd.hdr.u.parm.bufLen = len;   // less than 256
    memcpy((void*)tCmd.buf, (void*)cmdBuf, len);

    if ((err = tcmd_tx((char*)&tCmd, sizeof(tCmd), resp))) {
        fprintf(stderr, "tcmd_tx had error: %s!\n", strerror(err));
        return 0;
    }

    return 1;
}

static void cleanup(int sig)
{
    if (cid>=0) {
        close(cid);
    }
    if (sid>=0) {
        close(sid);
    }
}

int sock_init(int port)
{
    int                sockid;
    struct sockaddr_in myaddr;
    socklen_t          sinsize;
    int                i, res;

    /* Create socket */
    sockid = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockid == -1) {
        perror(__FUNCTION__);
        printf("Create socket to PC failed\n");
        return -1;
    }

    i = 1;
    res = setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, (char *)&i, sizeof(i));
    if (res == -1) {
        close(sockid);
        return -1;
    }

    i = 1;
    res = setsockopt(sockid, IPPROTO_TCP, TCP_NODELAY, (char *)&i, sizeof(i));
    if (res == -1) {
        close(sockid);
        return -1;
    }

    myaddr.sin_family      = AF_INET;
    myaddr.sin_port        = htons(port);
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    memset(&(myaddr.sin_zero), 0, 8);

    res = bind(sockid, (struct sockaddr *)&myaddr, sizeof(struct sockaddr));

    if (res != 0) {
        perror(__FUNCTION__);
        printf("Bind failed\n");
		close(sockid);
        return -1;
    }

    if (listen(sockid, 4) == -1) {
        perror(__FUNCTION__);
        printf("Listen failed\n");
		close(sockid);
        return -1;
    }

    printf("Waiting for client to connect...\n");

    sinsize = sizeof(struct sockaddr_in);
    if ((cid = accept(sockid, (struct sockaddr *)&myaddr, &sinsize)) == -1) {
        printf("Accept failed\n");
		close(sockid);
        return -1;
    }

    i = 1;
    res = setsockopt(cid, IPPROTO_TCP, TCP_NODELAY, (char *)&i, sizeof(i));
    if (res == -1) {
        printf("cannot set NOdelay for cid\n");
        close(sockid);
        return -1;
    }
    printf("Client connected!\n");

    return sockid;
}

int sock_recv(int sockid, unsigned char *buf, int buflen)
{
    int recvbytes;
    recvbytes = recv(sockid, buf, buflen, 0);
    if (recvbytes == 0) {
        printf("Connection close!? zero bytes received\n");
        return -1;
    } else if (recvbytes > 0) {
        return recvbytes;
    }
    return -1;
}

int sock_send(int sockid, unsigned char *buf, int bytes)
{
    int cnt;
    unsigned char* bufpos = buf;
    while (bytes) {
        cnt = write(sockid, bufpos, bytes);

        if (!cnt) {
            break;
        }
        if (cnt == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                return -1;
            }
        }

        bytes -= cnt;
        bufpos += cnt;
    }
    return (bufpos - buf);
}

static void print_help(char *pname)
{
    printf("An agent program to connect ART host and AR6K device, must be\n");
    printf("started after AR6K device driver is installed.\n\n");
    printf("Usage: %s ifname fragsize\n\n", pname);
    printf("  ifname      AR6K interface name\n");
    printf("  fragsize    Fragment size, must be multiple of 4\n\n");
    printf("Example:\n");
    printf("%s eth1 80\n\n", pname);
}

int main (int argc, char **argv)
{
    int recvbytes=0,bytesRcvd=0;
    int chunkLen = 0;
    unsigned char *bufpos;
    int reducedARTPacket = 1;
    int frag_size = 200;
    //int i=0;
    int port = ART_PORT;
    bool resp = false;
    bool firstpacket = true;

    struct sigaction sa;
    int cmdIndx, statusIndx;
    int continuous = 0;
    A_CHAR wifname[IFNAMSIZ] = "wlan0";

    printf("setup signal\n");
    memset(&sa, 0, sizeof(struct sigaction));

    sa.sa_flags = SA_NOCLDSTOP;
    sa.sa_handler = cleanup;

    printf("before call sigaction\n");
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);

    printf("setup ifname\n");
    memset(ar6kifname, '\0', sizeof(ar6kifname));

    if (argc == 1 ) {
        print_help(argv[0]);
        return -1;
    }

    if (argc > 1 ) {
        strlcpy(ar6kifname, &argv[1], strlen(argv[1]));
    }
    else {
        strlcpy(ar6kifname, &wifname[0], strlen("wlan0"));
    }

    if (argc > 2) {
        frag_size = atoi(argv[2]);
    }

    if (argc > 3) {
        port = atoi(argv[3]);
    }
    if (argc > 4) {
        continuous = atoi(argv[4]);
    }

    if (port == 0)
	port = ART_PORT;
    else if (port < 0 || port >65534) {
	printf("Invalid port number\n");
	goto main_exit;
    }

    //NOTE: issue with bigger size on ath6kl driver..
    if ( ((frag_size == 0) || ((frag_size % 4) != 0)) || (frag_size > 200) ) {
        printf("Invalid fragsize, should be multiple of 4 and frag size less than 200\n");
        goto main_exit;
    }

    if ( initInterface(ar6kifname) ) {
        printf("Init interface cfg80211 failed\n");
        cleanup(0);
        return -1;
    }
    do{
    cid = sid = aid = -1;
    if (continuous) {
        system("rmmod ath6kl_sdio");
        system("insmod /lib/modules/3.2.0-rc2-wl-ar6/kernel/net/wireless/ath6kl_sdio.ko testmode=2");
    }
    printf("open sock\n");
    sid = sock_init(port);
    if (sid < 0) {
        printf("Create socket to ART failed\n");
        cleanup(0);
        return -1;
    }

    if ((recvbytes=sock_recv(cid, line, LINE_ARRAY_SIZE)) < 0) {
        printf("Cannot nego packet size\n");
        cleanup(0);
        return -1;
    }

    printf("Get nego bytes %d\n", recvbytes);

    if (1 == (*(unsigned int *)line)) {
	reducedARTPacket = 1;
        printf("Not supporting reducedARTPacket\n");
        goto main_exit;
    }
    else {
	reducedARTPacket = 0;
    }

    sock_send(cid, &(line[0]), 1);

    printf("Ready to loop for art packet reduce %d\n", reducedARTPacket);

    while (1) {
        //printf("wait for tcp socket\n");
        if ((recvbytes = sock_recv(cid, line, LINE_ARRAY_SIZE)) < 0) {
            printf("Cannot recv packet size %d\n", recvbytes);
            cleanup(0);
            if (continuous) {
                break;
            }else{
                return -1;
            }
        }

        bytesRcvd = recvbytes;
        readLength = 0;
        resp = false;

        if ( firstpacket == true )
        {
            cmdLen = *(unsigned short *)&(line[0]);
            cmdId = *(unsigned char *)&(line[2]);

            printf("->FW len %d Command %d recvbytes %d\n",cmdLen,cmdId,recvbytes);
            firstpacket = false;
        }

        if (cmdId == DISCONNECT_PIPE_CMD_ID)
        {
            cleanup(0);
            if (!continuous) {
                system("rmmod wlan.ko");
                system("rmmod cfg80211.ko");
                exit(1);
            }
        }

        if (!reducedARTPacket) {
            //printf("Recived bytes from NART %d frag size %d\n",recvbytes,frag_size);
            bufpos = line;

            while (recvbytes) {
                if (recvbytes > frag_size) {
                    chunkLen = frag_size;
                } else {
                    chunkLen = recvbytes;
                }

                //we have to find out whether we need a resp or not for the last packet..
                recvbytes-=chunkLen;

                if ( recvbytes <=0 )
                {
                    resp = true;
                    firstpacket = true; //look for first packet again..
                }

                //printf("Chunk Len %d total size %d respNeeded %d\n",chunkLen,bytesRcvd,resp);
                wmiSend(bufpos, chunkLen, bytesRcvd, 1, resp);

                bufpos+=chunkLen;
            }
        }

        //line and readLength is populated in the callback
	cmdIndx = sizeof(TC_CMDS_HDR) + 4;
        statusIndx = cmdIndx + 4;
/*        if ((REG_WRITE_CMD_ID != line[cmdIndx]) && (MEM_WRITE_CMD_ID != line[cmdIndx]) &&
            (M_PCI_WRITE_CMD_ID != line[cmdIndx]) && (M_PLL_PROGRAM_CMD_ID != line[cmdIndx]) &&
            (M_CREATE_DESC_CMD_ID != line[cmdIndx])) { */
            printf("<- N/ART len %d Command %d status %d\n", readLength,(int)line[cmdIndx],(int)line[statusIndx]);
            sock_send(cid, line, readLength);
/*        } else {
            printf("<- N/ART ACK Command %d\n", (int)line[cmdIndx]);
            sock_send(cid, line, statusIndx);
        }*/
    }
}while(continuous);

main_exit:
    printf("Normal exit\n");
    cleanup(0);
    return 0;
}
