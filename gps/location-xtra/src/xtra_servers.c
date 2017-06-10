/******************************************************************************
  @file:  xtra_servers.c
  @brief: record the latest used server index

  DESCRIPTION

  XTRA Daemon

  -----------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technology Incoporated.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

#ifdef __cplusplus
extern "C"{
#endif

#include <xtra.h>
#include "log_util.h"
#include "platform_lib_includes.h"


//======Helper functions for server index handling 00000000000000000000000000

/*======
FUNCTION _validatedServerNumber

DESCRIPTION
    Check if server number is in range and return proper value always.

RETURN VALUE
    Valid server index
DEPENDENCIES

======*/

static int _validatedServerNumber(int nNumber, int nMax )
{
    if( nNumber>=0 && nNumber<nMax )
        return nNumber;

    return 0;
}

/*======
FUNCTION Xtra_NextServerNumber

DESCRIPTION
    Increase server index and do roll over if needed

RETURN VALUE
    Valid server index
DEPENDENCIES

======*/

void Xtra_NextServerNumber(int *pServerNumber, int nMaxNumber)
{
     *pServerNumber=_validatedServerNumber((*pServerNumber)+1, nMaxNumber);

     return;
}

/*======
FUNCTION Xtra_SetLastSntpServer

DESCRIPTION
    Set last used SNTP server index

RETURN VALUE
    None
DEPENDENCIES

======*/
void Xtra_SetLastSntpServer(int nServerNumber)
{
   globals.server_statistics.last_used_sntp_server=nServerNumber;
}

/*======
FUNCTION Xtra_GetLastSntpServer

DESCRIPTION
    Get last used SNTP server index

RETURN VALUE
    Valid server index
DEPENDENCIES

======*/
int Xtra_GetLastSntpServer()
{
   return _validatedServerNumber(globals.server_statistics.last_used_sntp_server,AMOUNT_NTP_SERVERS);
}

/*======
FUNCTION Xtra_SetLastXtraServer

DESCRIPTION
    Set last used XTRA server index

RETURN VALUE
    None
DEPENDENCIES

======*/
void Xtra_SetLastXtraServer(int nServerNumber)
{
   globals.server_statistics.last_used_xtra_server=nServerNumber;
}

/*======
FUNCTION Xtra_GetLastXtraServer

DESCRIPTION
    Get last used XTRA server index

RETURN VALUE
    Valid server index
DEPENDENCIES

======*/

int Xtra_GetLastXtraServer()
{
   return _validatedServerNumber(globals.server_statistics.last_used_xtra_server,AMOUNT_XTRA_SERVERS);
}

#ifdef __cplusplus
}
#endif
