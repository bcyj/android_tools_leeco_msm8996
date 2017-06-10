#ifndef CNE_PERMISSIONS_H
#define CNE_PERMISSIONS_H
/*============================================================================
  @file CnePermissions.h
  This header file provides permission information for connecting cne applications.

               Copyright (c) 2009,2010,2011,2012 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Confidential and Proprietary
============================================================================*/

/*============================================================================
  EDIT HISTORY FOR MODULE

  when        who       what, where, why
  ----------  ---       -------------------------------------------------------
  2011-11-22  sanello   First revision.

============================================================================*/

#include <cutils/properties.h>
#include <pwd.h>
#include <list>

#define COMM_PID_PATH_LEN 32

using namespace std;


struct PermissionInfo
{
  union
  {
    char ApprovedApp[ COMM_PID_PATH_LEN ];
    int  groupId;
  };
  bool flag;  //true indicates appname, false groupI;

  PermissionInfo(int gid)
  {
    flag = false;
    groupId = gid;
  }
  PermissionInfo(char* str)
  {
    flag = true;
    memset(ApprovedApp,0x0,COMM_PID_PATH_LEN);
    memcpy(ApprovedApp,str,strlen(str));
  }
  bool operator==(int gid)
  {
    if (!flag)
    {
      return (gid==groupId?true:false);
    }
    return false;
  }
  bool operator==(char* str)
  {
    if (flag)
    {
      return (strncmp(ApprovedApp,str,strlen(ApprovedApp))==0?true:false);
    }
    return false;
  }
};



/*------------------------------------------------------------------------------
 * CLASS         CnePermission
 *
 * DESCRIPTION   Determine if new socket connections should be allowed
 *               by compairing group id or application name
 *----------------------------------------------------------------------------*/

class CnePermissions
{

private:
  list<PermissionInfo> ApprovedClientList;
  bool isGidAllowed(int gid);
  bool isNameAllowed(char* appName);

public:
  CnePermissions();
  static int setProcessCapabilities();
  bool isAppAllowed (int pid, int gid);
};




#endif /* #ifndef CNE_MSG_ANDROID_H */




