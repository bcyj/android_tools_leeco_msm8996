/*==============================================================================
*  @file wfd_netutils.h
*  @par DESCRIPTION:
*       Header file of the wfd network utilities
*
*  Copyright (c) 2012,2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

#ifndef _WFD_NETUTILS_H_
#define _WFD_NETUTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

int getP2pInterfaceName(char* pInterfaceName, int size);

int getLocalIpAddress(char* pLocalIpAddr, int size);

// API to retrieve LOCAL IP from connected socket
// API returns 0 if successful and < 1 otherwise
int getLocalIpSocket (int SocketID, char* IP);

int getLocalMacAddress(char* pMacAddr, int size);

unsigned int getLinkSpeed(char *pMacAddr, unsigned int size, bool& queryIfName);

int socketClose(int nSock);

bool getIPSockPair(bool bPair, int *nSock1, int * nSock2,
                    int *nPort1, int *nPort2, bool tcp);
#ifdef __cplusplus
}
#endif

#endif /* _WFD_NETUTILS_H_ */
