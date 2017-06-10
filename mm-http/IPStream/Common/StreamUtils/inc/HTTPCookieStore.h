#ifndef _HTTP_COOKIE_STORE_H_
#define _HTTP_COOKIE_STORE_H_

/************************************************************************* */
/**
 * HTTPCookieStore.h
 * @brief Interface for Cookie Store Manager.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
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
#include "AEEstd.h"
#include "AEEStdDef.h"
#include "MMCriticalSection.h"
#include "qtv_msg.h"
#include "MMTime.h"
#include "StreamSourceTimeUtils.h"
#include "SourceMemDebug.h"
#include "oscl_string_utils.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

//Default http port
#define DEFAULT_PORT 80

//Persistant cookie flag
#define HTTP_COOKIE_PERSISTANT  0x01

//Host only cookie flag
#define HTTP_COOKIE_HOSTONLY    0x02

//secure only cookie flag
#define HTTP_COOKIE_SECUREONTLY 0x04

//http only cookie flag
#define HTTP_COOKIE_HTTPONLY    0x08

/* -----------------------------------------------------------------------
** Class / Structure Declarations
** ----------------------------------------------------------------------- */

//Cookie Manager Class
class HTTPCookieMgr {
public:
  HTTPCookieMgr();
  ~HTTPCookieMgr();

  // Public API to store Cookie
  bool StoreCookie(char *SetCookieValue, const char *Url, bool isAPPQuery = false);

  // Public API to get Cookie from cookie manager
  bool GetCookies(char *url, char *pHeader, size_t& pHeaderLen,
                           char *pValue, size_t& pValueLen,
                           bool isAppQuery = false);
private:

  // Structure for http URL parts
  typedef struct UrlParts {
           char *cpcHost; /* at "host:port/..." */
     const char *cpcPort; /* at ":port/..." */
           char *cpcPath; /* at '/', if present  */
  } UrlParts;

  // Structure for Name-Value pair pointers
  typedef struct NVPair {
    char *pcName;
    char *pcValue;
  } NVPair;

  // Structure for Cookie element as per RFC6265
  typedef struct HTTPCookie
  {
    HTTPCookie *next;      /* Points to next element in the list */
    char *CookieValueString;  /*comple cookie string*/
    char *value;           /* Cookie value string*/
    char *domain;          /* Cookie domain string*/
    char *path;            /* Cookie path string*/
    double expiryTime;     /* Cookie expiry time valid for persistant cookies only */
    double lastAccessTime; /* Cookie last Access time */
    double creationTime;   /* Cookie Creation time */
    uint32 nFlags;         /* Contains OR values for
                            *  HTTP_COOKIE_PERSISTANT,
                            *  HTTP_COOKIE_HOSTONLY,
                            *  HTTP_COOKIE_SECUREONTLY,
                            *  HTTP_COOKIE_HTTPONLY
                            */
    char CookieName[1];    /* Cookie Name  string */
  } HTTPCookie;

  // Delete Cookie from cookie list
  void DeleteCookie(HTTPCookie **pCookie);

  // Checks if the new cookie Supersedes older cookie
  bool DoesCookieSupersedes(HTTPCookie *me, const char *cpszName,
                        const char *cpcDomain, size_t nDomainLen,
                        const char *cpcPath, size_t nPathLen);

  // Creates and stores new cookie into cookie list
  bool CreateCookie(HTTPCookie **ppCkList, NVPair *pwnvp,
                 const char *cpcDomain, size_t nDomainLen,
                 const char *cpcPath, size_t nPathLen,
                 double tExpires, uint32 ulFlags,
                 double creatTime,
                 char *CookieValue);

  // Parses cookie date and returns the Milisecond value
  double ParseCookieDate(char *dateString);

  // Checks if the cookie is valid for the http URL
  bool IsCookieValidForURL(HTTPCookie *me, char *url);

  //Helper methods

  int   ParseNVPairs(char **ppszIn, const char *cpszDelims, char cSep,
                     NVPair *awavp, int nMaxPairs);

  bool  parsemonth(const char *cpsz, unsigned int *pnMonth);

  void  FillRelativePath(char *relativePath, size_t relativePathBufSize,
                             size_t& relativePathBufSizeRequested,
                             const char *ptrRelativeUrl);
  void  FillPortNumber(unsigned short& portNumber, const char *ptrPort);
  void  FillHostName(char *hostName, size_t hostNameBufSize,
                         size_t& hostNameBufSizeRequested, const char *ptrHost);
  void  FillHostPortPath(char *uri, char *hostName, size_t hostNameBufSize,
                             size_t& hostNameBufSizeRequested, unsigned short & portNumber,
                             char *relativePath, size_t relativePathBufSize,
                             size_t& relativePathBufSizeRequested);

  bool GetUrlHostPortRelativePath(const char *url, size_t urlLen,
                                  char *hostName, size_t hostNameBufSize,
                                  size_t& hostNameBufSizeRequested,
                                  unsigned short & portNumber,
                                  char *relativePath, size_t relativePathBufSize,
                                  size_t& relativePathBufSizeRequested);

  // the data unit elements
  HTTPCookie *m_cookiesList;

  // lock to control the access for cookies
  MM_HANDLE  m_cookieMgrLock;
};

#endif /* _HTTP_COOKIE_STORE_H_ */