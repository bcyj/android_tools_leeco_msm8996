/******************************************************************************
  @file:  xtra_config.c
  @brief: implementation of xtra configuration

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
#include "xtra_system_interface.h"

extern globals_t globals;


/*======
FUNCTION _isNotAtRange

DESCRIPTION
    This function takes a min, max and value and checks if the value is not
    in at range.

RETURN VALUE
    0: On range
    1: Not At Range

DEPENDENCIES

======*/

static unsigned long _isNotAtRange(unsigned long ulMin, unsigned long ulMax, unsigned long ulValue)
{
      if ( (ulValue>=ulMin)&&(ulValue<=ulMax))
        return 0;

    return 1;
}

/*======
FUNCTION _checkUrl

DESCRIPTION
    This function takes a pointer to a zero terminated URL string and maximum
    length and does sanity check for URL format.

RETURN VALUE
    0: OK
    1: error found

DEPENDENCIES

======*/
static unsigned long _checkUrl(const char* szUrl, unsigned short usMaxlen)
{
    int i=0;
    int nChecklen;

    if (NULL==szUrl)
    {
        XTRA_TRACE_ERROR("XTRA URL Check failure: NULL pointer \n");
        return 1;
    }

    nChecklen = strnlen(szUrl,usMaxlen);

    if (nChecklen >= usMaxlen)
    {
        XTRA_TRACE_ERROR("XTRA URL Check failure: too long URL \n");
        return 1;
    }

    for(i=0;i<nChecklen;i++)
    {
        if(_isNotAtRange(32,126,szUrl[i]))
        {
            XTRA_TRACE_ERROR("XTRA URL Check failure: ASCII non-Printable Characters found %x\n", szUrl[i]);
            return 1;
        }
    }
    return 0;
}



/*======
FUNCTION _checkConfigData

DESCRIPTION
    This function takes a pointer to a config data structure and performs
    sanity check for the data.

RETURN VALUE
    OK, FAIL

DEPENDENCIES

======*/

static unsigned long _checkConfigData(xtra_cfgdata_t *pConfig)
{
#define UPDATE_MASK() (failmask|=1<<shl++)
    unsigned long failmask = 0;
    int shl = 0;

    if(_isNotAtRange(0,1,pConfig->xtra_time_info_enabled))
       UPDATE_MASK() ;

    if( _isNotAtRange(0,4,pConfig->xtra_log_level))
       UPDATE_MASK() ;
    if(_isNotAtRange(0,1,pConfig->xtra_downloading_enabled))
       UPDATE_MASK() ;

    if(_checkUrl(pConfig->xtra_sntp_server_url[0],XTRA_URL_MAX_LEN))
       UPDATE_MASK() ;
    if(_checkUrl(pConfig->xtra_server_url[0],XTRA_URL_MAX_LEN))
       UPDATE_MASK() ;
    if(_checkUrl(pConfig->xtra_server_url[1],XTRA_URL_MAX_LEN))
       UPDATE_MASK() ;
    if(_checkUrl(pConfig->xtra_server_url[2],XTRA_URL_MAX_LEN))
       UPDATE_MASK() ;

    if (_isNotAtRange(0,1,pConfig->xtra_log_server_stats))
    {
       UPDATE_MASK() ;

    }else
    {
       if(pConfig->xtra_log_server_stats)
       {
          if (_checkUrl(pConfig->xtra_server_stats_path,XTRA_MAX_FILE_PATH_LEN))
          {
             UPDATE_MASK() ;
             pConfig->xtra_log_server_stats=0;
          }
       }
    }


    //todo: check pConfig->xtra_log_server_stats

    if(failmask)
    {
        XTRA_TRACE_ERROR("XTRA: configuration error: (%lx)\n", failmask);
    }
#undef UPDATE_MASK
    return (failmask == 0)? OK : FAIL;
}

/*======
FUNCTION _checkConfigData

DESCRIPTION
    This function takes a pointer to a config data and sets default values
    sanity check for the data.

RETURN VALUE
    none

DEPENDENCIES

======*/
void Xtra_ConfigSetDefaults()
{
    globals.config.data.xtra_time_info_enabled=1;

    globals.config.data.xtra_downloading_enabled=1;

    snprintf(globals.config.data.xtra_sntp_server_url[0],XTRA_URL_MAX_LEN,XTRA_NTP_DEFAULT_SERVER1);

    snprintf(globals.config.data.xtra_server_url[0],XTRA_URL_MAX_LEN,XTRA_DEFAULT_SERVER1);
    snprintf(globals.config.data.xtra_server_url[1],XTRA_URL_MAX_LEN,XTRA_DEFAULT_SERVER2);
    snprintf(globals.config.data.xtra_server_url[2],XTRA_URL_MAX_LEN,XTRA_DEFAULT_SERVER3);

    //disabled by default
    globals.config.data.xtra_log_server_stats=0;
    globals.config.data.xtra_server_stats_path[0]=(char)'\0';
}


/*======
FUNCTION xtra_set_config

DESCRIPTION
    This function sets configuration (xtra and sntp server urls)


RETURN VALUE
    None

DEPENDENCIES

======*/
void xtra_set_config(struct Xtra_System_Config *pConfig)
{
   int i = 0;
   if(pConfig != NULL) {

       globals.config.data.xtra_time_info_enabled=1;
       globals.config.data.xtra_downloading_enabled=1;
       globals.config.data.xtra_log_server_stats=0;
       globals.config.data.xtra_server_stats_path[0]=(char)'\0';

       /* Copy sntp server urls */
       for(i=0;i<NUMBER_OF_SNTP_SERVERS;++i) {
           strlcpy(globals.config.data.xtra_sntp_server_url[i], pConfig->xtra_sntp_server_url[i],XTRA_URL_MAX_LEN);
       }
       /* Copy xtra server urls */
       for(i=0;i<NUMBER_OF_XTRA_SERVERS;++i) {
           strlcpy(globals.config.data.xtra_server_url[i], pConfig->xtra_server_url[i],XTRA_URL_MAX_LEN);
       }

       /* Copy user agent information */
       strlcpy(globals.config.data.user_agent_string, pConfig->user_agent_string,USER_AGENT_STRING_LEN);

       /* Check config */
   /*_   _checkConfigData(&(globals.config.data)); */
   }
   else {
       XTRA_TRACE("Bad or No Configuration Supplied ; Loading default configuration \n");
       Xtra_ConfigSetDefaults();
   }
}


#ifdef __cplusplus
}
#endif
