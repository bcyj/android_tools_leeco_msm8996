/************************************************************************* */
/**
 * @file HTTPCookieStore.cpp
 * @brief Implementation of RFC6265.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header:
$DateTime:
$Change:

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "HTTPCookieStore.h"

/**
 * @ brief HTTPCookieMgr C'tor
 */
HTTPCookieMgr::HTTPCookieMgr():m_cookiesList(NULL),
                               m_cookieMgrLock(NULL)
{
  MM_CriticalSection_Create(&m_cookieMgrLock);
}

/**
 * @ brief HTTPCookieMgr D'tor
 */
HTTPCookieMgr::~HTTPCookieMgr()
{
  MM_CriticalSection_Enter(m_cookieMgrLock);

  //Free all the cookies
  while ((HTTPCookie *)0 != m_cookiesList)
  {
    DeleteCookie(&m_cookiesList);
  }

  MM_CriticalSection_Leave(m_cookieMgrLock);

  // Release the lock
  if (m_cookieMgrLock)
  {
    MM_CriticalSection_Release(m_cookieMgrLock);
    m_cookieMgrLock = NULL;
  }
}

/**
 * @brief:
 *  Check the new cookie if supersedes older cookie present in
 *  the cookie store as per RFC6265
 * @return:
 *  true if new cookie supersedes
 *  false if new cookie is different.
 */
bool HTTPCookieMgr::DoesCookieSupersedes(HTTPCookie *me, const char *cpszName,
                                 const char *cpcDomain, size_t nDomainLen,
                                 const char *cpcPath, size_t nPathLen)
{
  return !std_stricmp(me->CookieName, cpszName) &&
    (std_strlen(me->domain) == nDomainLen) &&
    !std_strnicmp(me->domain,cpcDomain,nDomainLen) &&
    (std_strlen(me->path) == nPathLen) &&
    !std_strncmp(me->path,cpcPath,nPathLen);
}

/**
 * @brief:
 *  Allocates memory for new cookie element and inserts the
 *  updates the new cookie parameters. Adds the new cookie element
 *  to the head of the cookie list
 * @return:
 *  true if new cookie added succesfully
 *  false if unable to add new cookie.
 */
bool HTTPCookieMgr::CreateCookie(HTTPCookie **ppCkList, NVPair *pwnvp,
                              const char *cpcDomain, size_t nDomainLen,
                              const char *cpcPath, size_t nPathLen,
                              double tExpires, uint32 ulFlags,
                              double creatTime,
                              char *CookieValue)
{
   size_t nNameLen = std_strlen(pwnvp->pcName);
   size_t nValueLen = std_strlen(pwnvp->pcValue);
   size_t nSize = (sizeof(HTTPCookie) + (nNameLen + 1 + nValueLen + 1 + nDomainLen +
                      1 + nPathLen + 1 + std_strlen(CookieValue) + 1))
                      * sizeof(char);

   HTTPCookie *me = (HTTPCookie *)QTV_Malloc(nSize);

   if ((HTTPCookie *)0 == me)
   {
      return false;
   }

   memset(me, 0, nSize);

   me->nFlags = ulFlags;

   if(ulFlags & HTTP_COOKIE_PERSISTANT)
   {
     me->expiryTime = tExpires;
   }

   MM_Time_DateTime currentTime;
   MM_Time_GetUTCTime(&currentTime);
   double currMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(currentTime);

   if(0 == creatTime)
   {
     me->creationTime = currMSeconds;
   }
   else
   {
     me->creationTime = creatTime;
   }

   me->lastAccessTime = currMSeconds;
   me->expiryTime     = tExpires;

   std_memmove(me->CookieName,pwnvp->pcName,nNameLen);

   me->CookieValueString = me->CookieName + nNameLen + 1;
   std_memmove(me->CookieValueString , CookieValue , std_strlen(CookieValue) + 1);

   me->value = me->CookieValueString + std_strlen(CookieValue) + 1;
   std_memmove(me->value,pwnvp->pcValue,nValueLen);

   me->domain = me->value + nValueLen + 1;
   std_memmove(me->domain,cpcDomain,nDomainLen);

   me->path = me->domain + nDomainLen + 1;
   std_memmove(me->path ,cpcPath,nPathLen);

   me->next = *ppCkList;
   *ppCkList = me;

   return true;
}

/**
 * @brief:
 *  Deletes the cookie from the cookie list
 * @return:
 *  None
 */
void HTTPCookieMgr::DeleteCookie(HTTPCookie **pme)
{
   HTTPCookie *me = *pme;

   *pme = me->next;

   QTV_Free(me);
}

/** @brief API Parses and Stores the Cookie String value as per
  *        RFC 6265.
  *
  * @param[in] pCookieString - Input cookie string
  * @param[in] Url - Input http url corresponds to cookie sting
  * @return
  * true  - Cookie is valid and stored in cookie manager
  * false - Ignores cookie (invalid cookie or low memory)
  */
bool HTTPCookieMgr::StoreCookie
(
 char *pCookieString,
 const char *Url,
 bool isAPPQuery
)
{
  const char     *cpszPath          = 0;
  const char     *cpszDomain        = 0;
  const char     *cpszMaxAge        = 0;
  char           *pszExpires        = 0;
  double         tExpires           = 0.0;
  double         creationTime       = 0.0;
  uint32         ulFlags            = 0;
  NVPair         wnvpCookie         = {0, 0};
  UrlParts       up                 = {0, 0, 0};
  size_t         nDomainLen         = 0;
  size_t         nHostLen           = 0;
  size_t         nPathLen           = 0;
  bool           result             = true;
  int             maxAge             = -1;
  size_t            hostNameBufLen     = 0;
  size_t            relativePathBufLen = 0;
  unsigned short port               = 0;
  char           *str               = 0;
  char           *url_cpy           = 0;

  // Acquire the Lock before performing any action on cookie manager
  MM_CriticalSection_Enter(m_cookieMgrLock);

  //Validate input parameters
  if((!isAPPQuery && (char *)0 == Url) || ((char *)0 == pCookieString))
  {
    result = false;
  }
  else
  {
    size_t cookieStringSize = std_strlen(pCookieString);
    url_cpy = (char *)QTV_Malloc((cookieStringSize + 1) * sizeof(char));
    if(url_cpy)
    {
      str = url_cpy;
      std_strlcpy(str, pCookieString, cookieStringSize + 1);
      str[cookieStringSize] = '\0';
    }
    else
    {
      result = false;
    }
  }

  if(result)
  {
    // Parse Cookie name value pair
    if ((0 == ParseNVPairs(&str, ";", '=', &wnvpCookie, 1)))
    {
      QTV_MSG_PRIO(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
                 "CookieStore: Discard Cookie without Name Value pair");
      result = false;
    }
  }

  if(result)
  {
    /* no value part, so take name as value */
    if (wnvpCookie.pcValue == wnvpCookie.pcName+ std_strlen(wnvpCookie.pcName))
    {
      char *pc = wnvpCookie.pcValue;
       wnvpCookie.pcValue = wnvpCookie.pcName;
       wnvpCookie.pcName = pc;
    }
  }

   //Parse all the cookie attributes as per RFC6265
   if(result)
   {
     NVPair wnvp;
     while (0 != ParseNVPairs(&str, ";", '=', &wnvp, 1))
     {
       if (((char *)0 == cpszDomain) && !std_stricmp(wnvp.pcName,"Domain"))
       {
         cpszDomain = wnvp.pcValue;
       }
       else if (((char *)0 == cpszPath) && !std_stricmp(wnvp.pcName,"Path"))
       {
         cpszPath = wnvp.pcValue;
       }
       else if (((char *)0 == cpszMaxAge) && !std_stricmp(wnvp.pcName,"Max-Age"))
       {
         cpszMaxAge = wnvp.pcValue;
       }
       else if (((char *)0 == pszExpires) && !std_stricmp(wnvp.pcName,"Expires"))
       {
         pszExpires = wnvp.pcValue;
       }
       else if (!std_strcmp(wnvp.pcName,"Secure"))
       {
         ulFlags |= HTTP_COOKIE_SECUREONTLY;
       }
       else if (!std_strcmp(wnvp.pcName,"HttpOnly"))
       {
         ulFlags |= HTTP_COOKIE_HTTPONLY;
       }
     }
   }

   if ((true == result) && ((char *)0 != cpszMaxAge))
   {
     if (!parseuint32(cpszMaxAge,0,(unsigned int *)&maxAge) || (0 > (long)tExpires))
     {
        tExpires = 0.0;
     }
     else
     {
       MM_Time_DateTime currentTime;
       MM_Time_GetUTCTime(&currentTime);

       double currMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(currentTime);

       tExpires = currMSeconds + (double)(maxAge * (1000));

       //Set the Persistant flag bit
       ulFlags |= HTTP_COOKIE_PERSISTANT;

       /* prefer max-age over expires*/
        pszExpires = 0;
     }
   }

   if ((true == result) &&((char *)0 != pszExpires))
   {
     tExpires = ParseCookieDate(pszExpires);
     if (0.0 == tExpires)
     {
       tExpires = 0.0;
     }
     else
     {
       //Set the Persistant flag bit
       ulFlags |= HTTP_COOKIE_PERSISTANT;
     }
   }

   if((result && !isAPPQuery) || (result && Url && isAPPQuery))
   {
     result = GetUrlHostPortRelativePath(
                                  Url,
                                  std_strlen(Url),
                                  NULL, 0, hostNameBufLen,
                                  port,
                                  NULL, 0, relativePathBufLen);
     if(true == result)
     {
       up.cpcHost = (char *)QTV_Malloc(hostNameBufLen * sizeof(char));
       up.cpcPath = (char *)QTV_Malloc(relativePathBufLen * sizeof(char));
       if((0 == up.cpcHost) || (0 == up.cpcPath))
       {
         QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
           "CookieStore: Discard Cookie %s for allocation faliure", wnvpCookie.pcName);
         result = false;
       }

       if(result)
       {
         result = GetUrlHostPortRelativePath(
                                     Url,
                                     std_strlen(Url),
                                     up.cpcHost, hostNameBufLen, hostNameBufLen,
                                     port,
                                     up.cpcPath, relativePathBufLen, relativePathBufLen);
         if(!result)
         {
           QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
             "CookieStore: Discard Cookie %s for URL Parse", wnvpCookie.pcName);
         }
       }
     }
   }

   char *UrlStr = NULL;
   char *portStr = NULL;

   // Point to start of hostname
   if((result && !isAPPQuery) || (result && Url && isAPPQuery))
   {
     if (Url)
     {
       UrlStr = (char *)std_strstr(Url, "http://");
       if (NULL == UrlStr)
       {
         UrlStr = (char *)std_strstr(Url, "https://");
         if(UrlStr)
         {
           UrlStr += std_strlen("https://");
         }
         else
         {
           UrlStr = (char *)Url;
         }
       }
       else
       {
         UrlStr += std_strlen("http://");
       }
     }

     if (UrlStr)
     {
       portStr = std_strstr(UrlStr, ":");
       if (NULL == portStr)
       {
         portStr = std_strstr(UrlStr, "/");
       }
     }

     if (((char *)0 != cpszPath) && (up.cpcPath != NULL))
     {
       if (!std_strbegins(up.cpcPath, cpszPath))
       {
         QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
           "CookieStore: Discard Cookie %s for Path Mismatch ",
           wnvpCookie.pcName);

         QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
           "url path %s, cookie path %s", up.cpcPath, cpszPath);

         result = false;
       }

       nPathLen = std_strlen(cpszPath);
     }
     else
     {
       char *pcSlash;
       nPathLen = up.cpcPath ? std_strlen(up.cpcPath) : 0;
       cpszPath = up.cpcPath;

       pcSlash = (char *)std_memrchr(cpszPath, '/', nPathLen);
       if ((char *)0 != pcSlash && (pcSlash > cpszPath))
       {
         nPathLen = (pcSlash - cpszPath) + 1;
       }
     }
   }

   if((result && !isAPPQuery) || (result && Url && isAPPQuery))
   {
     if ((char *)0 != cpszDomain)
     {
       if ('.' == cpszDomain[0])
       {
         cpszDomain++;
       }

       if (((char *)0 == std_strchr(cpszDomain,'.')) && std_strcmp(cpszDomain,"local"))
       {
         QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
           "CookieStore: Discard Cookie %s for Domain Mismatch",
           wnvpCookie.pcName);

         QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
           "url domain %s, cookie domain %s",
           up.cpcHost, cpszDomain);

         /* no embedded dot and not ".local" */
         result = false;
       }

       if(result)
       {
         nDomainLen = std_strlen(cpszDomain);
         nHostLen = up.cpcHost ? std_strlen(up.cpcHost) : 0;

         /* domain must be shorter than or the same length as host */
         if ((nDomainLen > nHostLen) ||
            /* domain must match after a '.' in host if it's shorter
             (Port == Host+ HostLen) */
            ((nDomainLen != nHostLen) &&
             (portStr &&('.' != portStr[-(nDomainLen+1)]))) ||
            /* domain must match end of host(Port == Host+nHostLen) */
            std_strnicmp(cpszDomain, portStr-nDomainLen, nDomainLen) ||

           /* domain must match the whole "domain" part of a hostname */
           ((nDomainLen < nHostLen) && (up.cpcHost) &&
            std_memchr(up.cpcHost,'.',nHostLen-nDomainLen-1)))
         {
           QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
             "CookieStore: Discard Cookie %s for Domain Mismatch",
             wnvpCookie.pcName);

           QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
             "url domain %s, cookie domain %s", up.cpcHost, cpszDomain);

           result = false;
         }
       }
     }
     else
     {
       if(up.cpcHost)
       {
         nDomainLen = std_strlen(up.cpcHost);
         cpszDomain = up.cpcHost;

         ulFlags |= HTTP_COOKIE_HOSTONLY;
       }
     }
   }

   if(result)
   {
     HTTPCookie  **lfp;
     HTTPCookie  *pCk;

     for (lfp = &m_cookiesList; (HTTPCookie *)0 != (pCk = *lfp);
          lfp = &(*lfp)->next)
     {
       if (DoesCookieSupersedes(pCk,wnvpCookie.pcName,
                            cpszDomain,nDomainLen,
                            cpszPath,nPathLen))
       {
          /* Superseding a persistent cookie */
          if((*lfp)->nFlags & HTTP_COOKIE_PERSISTANT)
          {
            creationTime = (*lfp)->creationTime;
          }

          QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
           "CookieStore: Superdedes Cookie %s",
           wnvpCookie.pcName);

          DeleteCookie(lfp);
          break;
        }
     }
   }

   if(result)
   {
     if(CreateCookie(&m_cookiesList, &wnvpCookie,
                        cpszDomain, nDomainLen,
                        cpszPath, nPathLen, tExpires,
                        ulFlags, creationTime,
                        pCookieString))
     {
       QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
       "CookieStore: Added Cookie %s , value %s",
       wnvpCookie.pcName, wnvpCookie.pcValue);

       QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
       "Cookie path %s, cookie domain %s", cpszPath, cpszDomain);

     }
     else
     {
       QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
       "CookieStore: Discard Cookie %s for allocation faliure, path %s",
       wnvpCookie.pcName, cpszPath);

       result = false;
     }
   }

   if(up.cpcHost)
   {
     QTV_Free(up.cpcHost);
     up.cpcHost = NULL;
   }

   if(up.cpcPath)
   {
     QTV_Free(up.cpcPath);
     up.cpcPath = NULL;
   }

   if(url_cpy)
   {
     QTV_Free(url_cpy);
     url_cpy = NULL;
   }

   //Leave the critical section
   MM_CriticalSection_Leave(m_cookieMgrLock);
   return result;
}

/**
 * @brief:
 *  Check the cookie if it is valid for the url as per RFC6265
 * @return:
 *  true if new cookie supersedes
 *  false if new cookie is different.
 */
bool HTTPCookieMgr::IsCookieValidForURL(HTTPCookie *me, char *url)
{
  size_t nDomainLen         = 0;
  size_t hostNameBufLen     = 0;
  size_t relativePathBufLen = 0;
  unsigned short port    = 0;
  bool result            = true;
  UrlParts pup           = {0,0,0};

  if((NULL == me) || (NULL == url))
  {
    result = false;
  }

  nDomainLen = (me ? std_strlen(me->domain) : 0);

  if(result)
  {
    result = GetUrlHostPortRelativePath(url,
                                        std_strlen(url),
                                        NULL, 0, hostNameBufLen,
                                        port,
                                        NULL, 0, relativePathBufLen);
    if(result)
    {
      pup.cpcHost = (char *)QTV_Malloc(hostNameBufLen * sizeof(char));
      pup.cpcPath = (char *)QTV_Malloc(relativePathBufLen * sizeof(char));
      if((0 == pup.cpcHost) || (0 == pup.cpcPath))
      {
        result = false;
      }

      if(result)
      {
        result = GetUrlHostPortRelativePath(
                                    url,
                                    std_strlen(url),
                                    pup.cpcHost, hostNameBufLen, hostNameBufLen,
                                    port,
                                    pup.cpcPath, relativePathBufLen, relativePathBufLen);
      }
    }
  }

  char *UrlStr = NULL;
  char *portStr = NULL;

   // Point to start of hostname
   if ((result) && (url))
   {
     UrlStr = std_strstr(url, "http://");
     if (NULL == UrlStr)
     {
       UrlStr = std_strstr(url, "https://");
       if(UrlStr)
       {
         UrlStr += std_strlen("https://");
       }
       else
       {
         UrlStr = url;

         // if httpOnly flag is true then cookie can only
         // go with http urls only
         if(me->nFlags & HTTP_COOKIE_HTTPONLY)
         {
           result = false;
         }
       }
     }
     else
     {
       UrlStr += std_strlen("http://");
     }
   }

   if ((result) && (UrlStr))
   {
     portStr = std_strstr(UrlStr, ":");
     if (NULL == portStr)
     {
       portStr = std_strstr(UrlStr, "/");//pup.cpcPath;
     }
   }

   /* my domain must be shorter than or the same length as host */
   if((result) && (nDomainLen > std_strlen(pup.cpcHost)))
   {
     result = false;
   }

   /* my domain must match after a '.' in host if it's shorter
   (Port == Host+nHostLen) */
   if((result)&& (nDomainLen != std_strlen(pup.cpcHost)) &&
      (portStr && ('.' != portStr[-(nDomainLen+1)])))
   {
      result = false;
   }

   char *UrlDomain = portStr - nDomainLen;

   /* my domain must match end of host(Port == Host + nHostLen) */
   if((result) && (UrlDomain != NULL) && (std_strnicmp(me->domain, UrlDomain, nDomainLen)))
   {
     result = false;
   }

   /* if cookie is secure, the URL should be https (only understand https) */
   if((result) && (me->nFlags & HTTP_COOKIE_SECUREONTLY))
   {
     if(!std_strbegins(url, "https://"))
     {
       result = false;
     }
   }

   if(result)
   {
     if(!std_strbegins(pup.cpcPath, me->path))
     {
       result = false;
     }
   }

   if(pup.cpcHost)
   {
     QTV_Free(pup.cpcHost);
     pup.cpcHost = 0;
   }

   if(pup.cpcPath)
   {
     QTV_Free(pup.cpcPath);
     pup.cpcPath = 0;
   }

   return result;
}

/** @brief API Writes Cookie (header/value) Corresponds to the input
  *        http URL into the input (header/value)buffers as per
  *        RFC6265. If the header/value buffers are null, this API
  *        Updates the length required for cookie header and value.
  *        API also removes the expired cookie form the cookie manager
  *
  * @param[in] url - Input http url
  * @param[in/out] pHeader - Input buffer to store cookie header
  * @param[in/out] pHeaderLen - Size of header buffer
  * @param[in/out] pValue - Input buffer to store cookie value
  * @param[in/out] pValueLen - Size of cookie buffer
  * @return
  * true  - Cookie persent in the cookie mgr for input URL
  * false - No cookie present for the URL
  */
bool HTTPCookieMgr::GetCookies
(
 char   *url,
 char   *pHeader,
 size_t &pHeaderLen,
 char   *pValue,
 size_t &pValueLen,
 bool isAppQuery
)
{
  HTTPCookie *pCk;
  HTTPCookie **lfp;
  uint32     nCookies = 0;
  uint32     ulNow = 0;
  bool       result = false;

  //Enter critical section
  MM_CriticalSection_Enter(m_cookieMgrLock);

  MM_Time_DateTime currentTime;
  MM_Time_GetUTCTime(&currentTime);

  double currMSeconds = StreamSourceTimeUtils::ConvertSysTimeToMSec(currentTime);

  if(NULL == pHeader)
  {
    pHeaderLen = std_strlen("Cookie") + 1;
  }
  else
  {
    std_strlprintf(pHeader,pHeaderLen,"Cookie");
  }

  nCookies = 0, lfp = &m_cookiesList;

  if(0 == pValueLen)
  {
    while ((HTTPCookie *)0 != (pCk = *lfp))
    {
      if ((pCk->nFlags & HTTP_COOKIE_PERSISTANT) && (pCk->expiryTime < currMSeconds))
      {
        QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
        "CookieStore: Delete Expired Cookie %s, Value %s", pCk->CookieName, pCk->value);
        DeleteCookie(lfp);
        continue;
      }

      //Check if the cookie is valid for the url
      if (!isAppQuery && IsCookieValidForURL(pCk, url))
      {
        if (0 != nCookies)
        {
          pValueLen += std_strlen("; ");
        }

        if ('\0' != pCk->CookieName[0])
        {
          pValueLen += std_strlen(pCk->CookieName);
          pValueLen += std_strlen("=");
        }

        pValueLen += std_strlen(pCk->value);
        ++nCookies;

        result = true;
      }

      else if (isAppQuery && (!url || IsCookieValidForURL(pCk, url)))
      {
        if (0 != nCookies)
        {
          pValueLen += std_strlen("; ");
        }

        if (pCk->CookieValueString)
        {
          pValueLen += std_strlen(pCk->CookieValueString);
        }
        ++nCookies;

        result = true;
      }

      lfp = &(pCk)->next;
    }

    pValueLen += 1; //for null termination character
  }
  else
  {
    size_t num = 0;
    while ((HTTPCookie *)0 != (pCk = *lfp))
    {
      if ((pCk->nFlags & HTTP_COOKIE_PERSISTANT) && (pCk->expiryTime < currMSeconds))
      {
        QTV_MSG_SPRINTF_PRIO_2(QTVDIAG_GENERAL, QTVDIAG_PRIO_HIGH,
        "CookieStore: Delete Expired Cookie %s, Value %s", pCk->CookieName, pCk->value);
        DeleteCookie(lfp);
        continue;
      }

      // Check if cookie id valid and write cookie to buffer
      if (!isAppQuery && IsCookieValidForURL(pCk, url) && pValue)
      {
        //Update the cookie last acces time to current time
        pCk->lastAccessTime = currMSeconds;

        if (0 != nCookies)
        {
          num = std_strlprintf(pValue,pValueLen,"; ");
          pValue += num;
        }

        if ('\0' != pCk->CookieName[0])
        {
          num = std_strlprintf(pValue,std_strlen(pCk->CookieName)+1,"%s",pCk->CookieName);
          pValue += num;

          num = std_strlprintf(pValue,pValueLen, "%s", "=");
          pValue += num;
        }

        num = std_strlprintf(pValue,std_strlen(pCk->value)+1,"%s",pCk->value);
        pValue += num;

        ++nCookies;

        result = true;
      }

      else if (isAppQuery && (!url || IsCookieValidForURL(pCk, url)) && pValue)
      {
        if (num < pValueLen)
        {
          if (0 != nCookies)
          {
            num += std_strlcat(pValue, "; ", std_strlen("; ")+ 1);
            //pValueLen += std_strlen("; ");

            if (pCk->CookieValueString)
            {
              num += std_strlcat(pValue, pCk->CookieValueString, std_strlen(pCk->CookieValueString)+ 1);
              //pValueLen += num;
            }
          }
          else
          {
            if (pCk->CookieValueString)
            {
              num += std_strlcpy(pValue, pCk->CookieValueString, std_strlen(pCk->CookieValueString)+ 1);
              //pValueLen += num;
            }
          }
        }

        ++nCookies;

        result = true;
      }
      lfp = &(pCk)->next;
    }
  }

  //Leave critical section
  MM_CriticalSection_Leave(m_cookieMgrLock);
  return result;
}

/**
  || Function
  || --------
  || int ParseNVPairs(char **ppszIn, const char *pszDelims, char cSep,
  ||                  NVPair *awavp, int nMaxPairs)
  ||
  || Description
  || -----------
  || Parses a string of attr-value pairs with the pseudo-BNF:
  ||
  ||   av-pairs      =   av-pair *(<delim char> av-pair)
  ||   av-pair       =   attr [<sep char> value]
  ||   attr          =   token
  ||   value         =   token | quoted-string
  ||   token         =   1*<any CHAR except CTLs, <sep char> or <delim char> >
  ||   quoted-string =   ( <"> *(qdtext | quoted-pair ) <"> )
  ||   qdtext        =   <any TEXT except <">>
  ||   quoted-pair   =   "\" CHAR
  ||
  || Parameters
  || ----------
  || char **ppszIn: [in/out] pointer to pointer to string to parse,
  ||                         on return, is set to the next attr val pair
  || const char *cpszDelims: null-terminated list of characters to use as
  ||                          av-pair delimiters (list of "delim chars")
  ||                          above
  || char cSep: character to use to separate name from value
  || NVPair *awavp: [out] array of NVPairs to fill
  || int nNumFields: number of elements in the array awavp
  ||
  || Returns
  || -------
  || the number of successfully parsed av-pairs fields, always less than or
  ||  equal to nMaxPairs
  ||
  || Remarks
  || -------
  || the sep char must not be a member of the list of delim chars,
  ||   the behaviour of this function is undefined if this constraint
  ||   is violated
  ||
  */
int HTTPCookieMgr::ParseNVPairs(char **ppszIn, const char *cpszDelims, char cSep,
                                NVPair *awavp, int nMaxPairs)
{
   char  *psz;
   int   nNumPairs;
   bool  bTerminate = true;
   bool  bTrim      = true;

   for (psz = *ppszIn, nNumPairs = 0;
        (0 != *psz) && (nNumPairs < nMaxPairs); ) {
      NVPair wavp;
      ptrdiff_t nNameLen, nValueLen;

      /* find the name, value, and end of the field */
      wavp.pcName  = psz;
      psz          = std_strchrsend(psz,cpszDelims);
      wavp.pcValue = (char *)std_memchr(wavp.pcName,cSep,(size_t)(psz - wavp.pcName));
      if(NULL == wavp.pcValue)
      {
        wavp.pcValue = wavp.pcName + (psz - wavp.pcName);
      }

      /* get namelen and valuelen before poking termination */
      nValueLen = psz - wavp.pcValue;
      nNameLen  = wavp.pcValue - wavp.pcName;

      /* poke in null termination */
      if ('\0' != *psz) {
         if (bTerminate) {
            *psz = 0;
         }
         psz++;
      }
      if (cSep == *wavp.pcValue) {
         if (bTerminate) {
            *wavp.pcValue = 0;
         }
         wavp.pcValue++;
         nValueLen--;
      }

      /* decode in place, null terminate again... */
      if (bTrim) {
         nNameLen = std_trim(wavp.pcName,nNameLen);
         nValueLen = std_trim(wavp.pcValue,nValueLen);
      }

      if (bTerminate) {
         wavp.pcName[nNameLen] = 0;
         wavp.pcValue[nValueLen] = 0;
      }

      /* check to see if we found a real field (not just a separator) */
      if ((0 != nValueLen) ||
          (0 != nNameLen) ||
          (wavp.pcValue != wavp.pcName)) {

         if ((NVPair *)0 != awavp) {
            memmove(awavp,&wavp,sizeof(wavp));
            awavp++;
         }

         nNumPairs++;
      }
   }

   *ppszIn = psz;

   return nNumPairs;
}

/**
 * @ brief Returns true if the month is valid and updated the
 *   month index
 */
bool HTTPCookieMgr::parsemonth(const char *cpsz, unsigned int *pnMonth)
{
   const char *cpszMonths =
      "Jan"
      "Feb"
      "Mar"
      "Apr"
      "May"
      "Jun"
      "Jul"
      "Aug"
      "Sep"
      "Oct"
      "Nov"
      "Dec";

   if (3 != std_strlen(cpsz)) {
      return false;
   }

   return strmxnstr(cpszMonths, cpsz, 3, pnMonth);
}

/**
  || Function
  || --------
  || double HTTPCookieMgr::CookieDateParse(char *pszIn);
  ||
  || Description
  || -----------
  || All HTTP date/time stamps MUST be represented in Greenwich Mean Time
  || (GMT), without exception. For the purposes of HTTP, GMT is exactly
  || equal to UTC (Coordinated Universal Time). This is indicated in the
  || first two formats by the inclusion of "GMT" as the three-letter
  || abbreviation for time zone, and MUST be assumed when reading the
  || asctime format. HTTP-date is case sensitive and MUST NOT include
  || additional LWS beyond that specifically included as SP in the
  || grammar.
  ||
  ||        HTTP-date    = rfc1123-date | rfc850-date | asctime-date
  ||        rfc1123-date = wkday "," SP date1 SP time SP "GMT"
  ||        rfc850-date  = weekday "," SP date2 SP time SP "GMT"
  ||        asctime-date = wkday SP date3 SP time SP 4DIGIT
  ||        date1        = 2DIGIT SP month SP 4DIGIT
  ||                       ; day month year (e.g., 02 Jun 1982)
  ||        date2        = 2DIGIT "-" month "-" 2DIGIT
  ||                       ; day-month-year (e.g., 02-Jun-82)
  ||        date3        = month SP ( 2DIGIT | ( SP 1DIGIT ))
  ||                       ; month day (e.g., Jun  2)
  ||        time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
  ||                       ; 00:00:00 - 23:59:59
  ||        wkday        = "Mon" | "Tue" | "Wed"
  ||                     | "Thu" | "Fri" | "Sat" | "Sun"
  ||        weekday      = "Monday" | "Tuesday" | "Wednesday"
  ||                     | "Thursday" | "Friday" | "Saturday" | "Sunday"
  ||        month        = "Jan" | "Feb" | "Mar" | "Apr"
  ||                     | "May" | "Jun" | "Jul" | "Aug"
  ||                     | "Sep" | "Oct" | "Nov" | "Dec"
  ||
  || Note: HTTP requirements for the date/time stamp format apply only
  || to their usage within the protocol stream. Clients and servers are
  || not required to use these formats for user presentation, request
  || logging, etc.
  ||
  ||
  || Parameters
  || ----------
  || char *psz: Date string
  ||
  || Returns
  || -------
  || a mill seconds time, or 0.0 on error
  ||
  || Remarks
  || -------
  || this function destroys the string
  ||
  */
double HTTPCookieMgr::ParseCookieDate(char *pszIn)
{
   MM_Time_DateTime j;
   char *pszYear;
   char *pszMonth;
   char *pszDay;
   char *pszHour;
   char *pszMinute;
   char *pszSecond;

   memset(&j, 0, sizeof(j));

   /* all the formats have exactly 4 space-separated fields */
   switch (strchop(pszIn," ")) {
   case 4: /* parse rfc850 date */
      {
         /* char *pszWeekday = strchopped_nth(pszIn,0); */
         char *pszDate = strchopped_nth(pszIn,1);
         char *pszTime = strchopped_nth(pszIn,2);

         if (std_strcmp(strchopped_nth(pszIn,3),"GMT") ||
            (3 != strchop(pszDate,"-")) ||
            (3 != strchop(pszTime,":"))) {
            return 0.0;
         }

         pszDay    = strchopped_nth(pszDate,0);
         pszMonth  = strchopped_nth(pszDate,1);
         pszYear   = strchopped_nth(pszDate,2);
         pszHour   = strchopped_nth(pszTime,0);
         pszMinute = strchopped_nth(pszTime,1);
         pszSecond = strchopped_nth(pszTime,2);
      }
      break;

   case 5: /* parse asctime */
      {
         /* char *pszWkday = strchopped_nth(pszIn,0); */
         char *pszTime = strchopped_nth(pszIn,3);

         pszMonth = strchopped_nth(pszIn,1);
         pszDay = strchopped_nth(pszIn,2);
         pszYear = strchopped_nth(pszIn,4);

         if (3 != strchop(pszTime,":")) {
            return 0.0;
         }
         pszHour   = strchopped_nth(pszTime,0);
         pszMinute = strchopped_nth(pszTime,1);
         pszSecond = strchopped_nth(pszTime,2);
      }
      break;

   case 6: /* parse rfc1123 date */
      {
         /* char *pszWkday = strchopped_nth(pszIn,0); */
         char *pszTime = strchopped_nth(pszIn,4);

         pszDay = strchopped_nth(pszIn,1);
         pszMonth = strchopped_nth(pszIn,2);
         pszYear = strchopped_nth(pszIn,3);

         if (std_strcmp(strchopped_nth(pszIn,5),"GMT") ||
            (3 != strchop(pszTime,":"))) {
            return 0.0;
         }

         pszHour   = strchopped_nth(pszTime,0);
         pszMinute = strchopped_nth(pszTime,1);
         pszSecond = strchopped_nth(pszTime,2);
      }
      break;

   default:
      return 0.0;
   }

   if(!parseuint32(pszDay,10,(unsigned int *)&j.m_nDay) ||
     !parsemonth(pszMonth,(unsigned int *)&j.m_nMonth) ||
     !parseuint32(pszYear,10,(unsigned int *)&j.m_nYear) ||
     !parseuint32(pszYear,10,(unsigned int *)&j.m_nYear) ||
     !parseuint32(pszYear,10,(unsigned int *)&j.m_nYear) ||
     !parseuint32(pszYear,10,(unsigned int *)&j.m_nYear) ||
     !parseuint32(pszHour,10,(unsigned int *)&j.m_nHour) ||
     !parseuint32(pszMinute,10,(unsigned int *)&j.m_nMinute) ||
     !parseuint32(pszSecond,10,(unsigned int *)&j.m_nSecond)) {
      return 0.0;
   }

   /* all the above parsing uses 0-based index to month */
   j.m_nMonth += 1;

   /* 2-digit year? */
   if (j.m_nYear < 100) {
     j.m_nYear += (j.m_nYear > 69 ? 1900:2000);
   }

   {
      double availabilityStartTime =
                         StreamSourceTimeUtils::ConvertSysTimeToMSec(j);

      return availabilityStartTime;
   }
}

/**
 * Extract relative path from URI
 *
 * @param relativePath  - Ptr to client owned string.
 * @param relativePathBufSize - Size of client owned string
 *                            buffer.
 * @param relativePathBufSizeRequested - Size necessary for
 *                                     client owned buffer.
 * @param ptrRelativeUrl  - Ptr to start of relative path of
 *                        URI. If it is NULL, then relative path
 *                        is "/".
 */
void HTTPCookieMgr::FillRelativePath
(
  char *relativePath,
  size_t relativePathBufSize,
  size_t& relativePathBufSizeRequested,
  const char *ptrRelativeUrl
)
{
  // Set relativeBufSizeRequested and/or
  // populate relativePath
  if (ptrRelativeUrl)
  {
    relativePathBufSizeRequested = std_strlen(ptrRelativeUrl) + 1;

    if (relativePath)
    {
      std_strlcpy(relativePath, ptrRelativeUrl, relativePathBufSize);
    }
  }
  else
  {
    relativePathBufSizeRequested = std_strlen("/") + 1;

    if (relativePath)
    {
      std_strlcpy(relativePath, "/", relativePathBufSize);
    }
  }
}

/**
 * Extract port number from URI
 *
 * @param portNumber - Client owned ptr to unsigned short
 * @param ptrPort - Ptr to start of Port Number in URI.
 */
void HTTPCookieMgr::FillPortNumber
(
 unsigned short& portNumber,
 const char *ptrPort
)
{
  portNumber = DEFAULT_PORT;
  unsigned int temp_portNumber;
  if (ptrPort)
  {
    temp_portNumber = atoi(ptrPort);
    if (!temp_portNumber)
    {
       // not able to parse
       QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPCookieStore::PopulatePortNumber - Error in  parsing");
      portNumber = DEFAULT_PORT;
    }
    else
    {
      portNumber = (unsigned short)temp_portNumber;
    }
  }
}

/**
 * Extract host name from URI
 *
 * @param hostName - Ptr to client owned buffer
 * @param hostNameBufSize - Size of client owned buffer
 * @param hostNameBufSizeRequested - Size necessary for client
 *                                 owned buffer
 * @param ptrHost - Ptr to hostname in URI.
 */
void HTTPCookieMgr::FillHostName
(
  char *hostName,
  size_t hostNameBufSize,
  size_t& hostNameBufSizeRequested,
  const char *ptrHost
)
{
  if (ptrHost)
  {
    hostNameBufSizeRequested = std_strlen(ptrHost) + 1;

    if (hostName)
    {
      std_strlcpy(hostName, ptrHost, hostNameBufSize);
    }
  }
}

/**
 * Determine length of hostname and path or
 * populate host, port, path in client buffers.
 *
 * @param localUrl
 * @param hostName  - Ptr to client hostname buffer
 * @param hostNameBufSize - Size of client hostname buffer
 * @param hostNameBufSizeRequested - Size necessary for client
 *                                 hostname buffer
 * @param portNumber - Ptr to client owned unsigend short
 * @param relativePath - Ptr to client relativePath buffer
 * @param relativePathBufSize - Size of client relativePath
 *                            buffer
 * @param relativePathBufSizeRequested - Size necessary for
 *                                     client relativePath
 *                                     buffer.
 */
void HTTPCookieMgr::FillHostPortPath
(
 char *uri,
 char *hostName, size_t hostNameBufSize,
 size_t& hostNameBufSizeRequested,
 unsigned short & portNumber,
 char *relativePath, size_t relativePathBufSize,
 size_t& relativePathBufSizeRequested
)
{
  char *posHost = NULL;

  // Point posHost to start of hostname
  if (uri)
  {
    posHost = std_strstr(uri, "http://");
    if (NULL == posHost)
    {
      posHost = std_strstr(uri, "https://");
      if(NULL == posHost)
      {
        posHost = uri;
      }
      else
      {
        posHost += std_strlen("https://");
      }
    }
    else
    {
      posHost += std_strlen("http://");
    }
  }

  // Point posRelativeUrl to start of relative path.
  // If there is no relative path, then "/" is
  // considered the relative path
  char *posRelativeUrl = NULL;
  if (posHost)
  {
    posRelativeUrl = std_strstr(posHost, "/");
  }

  FillRelativePath(relativePath,
                   relativePathBufSize,
                   relativePathBufSizeRequested,
                   posRelativeUrl);

  if (posRelativeUrl)
  {
    *posRelativeUrl = 0; // No longer needed
  }

  char *posPort = NULL;
  if (posHost)
  {
    posPort = std_strstr(posHost, ":");
  }

  if (posPort)
  {
    *posPort = 0;
    ++posPort;
  }

  FillPortNumber(portNumber, posPort);

  // If valid, posHost points to hostname, posPort points to portNumber,
  // posRelativePath points to relativePath except that "/" is relaced by 0.

  FillHostName(hostName, hostNameBufSize, hostNameBufSizeRequested, posHost);
}

/**
 * @brief:
 *  Extract the hostname, port, relative url.
 *  eg. url: "http://A.B.C.D:pp/somepath
 *  HostName : "A.B.C.D", port: pp, relativePath: "somepath"
 *  Special cases:
 *  a. 'http://' missing: hostName starts at url[0]
 *  b. Relative path missing. Theb relativePath = "/"
 */
bool HTTPCookieMgr::GetUrlHostPortRelativePath(
                          const char *url, size_t urlLen,
                          char *hostName, size_t hostNameBufSize,
                          size_t& hostNameBufSizeRequested,
                          unsigned short & portNumber,
                          char *relativePath, size_t relativePathBufSize,
                          size_t& relativePathBufSizeRequested)
{
  bool result = true;

  hostNameBufSizeRequested = 0;
  portNumber = 0;
  relativePathBufSizeRequested = 0;

  // param 'url' is untrusted and may not be null terminated.
  // localUrl ensures it is null terminated.
  char *localUrl = NULL;

  if (NULL == url)
  {
    // err msg
    result = false;
  }
  else
  {
    localUrl = (char *)QTV_Malloc((urlLen + 1) * sizeof(char));
    if (NULL == localUrl)
    {
      result = false;

      QTV_MSG_PRIO( QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
        "HTTPCookieStore::GetHostAndPortFromUrl - Failed to allocate localUrl");
    }else
    {
      std_strlcpy(localUrl, url, urlLen + 1);
      result = true;
    }
  }

  if (true == result)
  {
    FillHostPortPath(localUrl,
                     hostName, hostNameBufSize,
                     hostNameBufSizeRequested,
                     portNumber,
                     relativePath, relativePathBufSize,
                     relativePathBufSizeRequested);
  }

  if((portNumber == 0) ||
     (hostNameBufSizeRequested == 0) ||
     (relativePathBufSizeRequested == 0))
  {
    QTV_MSG_PRIO3( QTVDIAG_GENERAL, QTVDIAG_PRIO_ERROR,
                 "HTTPStackCommon::GetHostAndPortFromUrl - Port '%d', or  "
                 "hostNameBufSizeRequested '%d' or relativePathBufSizeRequested '%d'"
                 "is zero", portNumber, hostNameBufSizeRequested, relativePathBufSizeRequested);

    result = false;
  }

  if (localUrl)
  {
    QTV_Free(localUrl);
    localUrl = NULL;
  }

  return result;
}
