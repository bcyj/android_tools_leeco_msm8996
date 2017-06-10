#ifndef SWIM_SYNC_WRAPPER_DEFS_H
#define SWIM_SYNC_WRAPPER_DEFS_H

/*==============================================================================
  FILE:         SwimSyncWrapperdefs.h

  OVERVIEW:     Contains definitions for wrapping socket calls.

  DEPENDENCIES: None

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/


/*==============================================================================
  EDIT HISTORY FOR MODULE

  when        who     what, where, why
  ----------  ---     ----------------------------------------------------------
  06-27-2013  mts     First revision.
==============================================================================*/

/*------------------------------------------------------------------------------
 * Include Files
 * ---------------------------------------------------------------------------*/

#include <sys/socket.h>

#ifdef LIBNIMS_WRAPPER
    #define EXTERN
#else
    #define EXTERN extern
#endif

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

enum NsrmSocketClassType {
  NSRM_SOCKET_CLASS_CONNECT,
  NSRM_SOCKET_CLASS_WRITE,
  NSRM_SOCKET_CLASS_CLOSE,
  NSRM_SOCKET_CLASS_BIND,
  NSRM_SOCKET_CLASS_LISTEN,
  NSRM_SOCKET_CLASS_MAX
};

enum NsrmTimerClassType {
  NSRM_TIMER_CLASS_CONNECT,
  NSRM_TIMER_CLASS_WRITE,
  NSRM_TIMER_CLASS_MAX
};
#ifndef UT

EXTERN int ( *bionic_bind )( int, const struct sockaddr *, socklen_t );
EXTERN int ( *bionic_android_getaddrinfofornet) (const char *hostname,
                const char *servname,
                const struct addrinfo *hints,
                unsigned netid,
                unsigned mark,
                struct addrinfo **res );
EXTERN ssize_t ( *bionic_write ) ( int, const void *, size_t);
EXTERN ssize_t ( *bionic_send ) (int, const void *, size_t, unsigned int);
EXTERN ssize_t ( *bionic_sendto) (int, const void *, size_t, int, const struct sockaddr *, socklen_t);
EXTERN int ( *bionic_close) ( int );
EXTERN int ( *bionic_writev ) (int fd, const struct iovec *iov, int iovcnt);
EXTERN int ( *bionic_sendmsg ) (int sockfd, const struct msghdr *msg, unsigned int flags);
EXTERN int ( *bionic_shutdown ) (int fd, int how);
EXTERN char *( *bionic_getenv )( const char *name );
EXTERN struct hostent* ( *bionic_gethostbyname) (const char *name);
EXTERN int ( *bionic_gethostbyname_r) (const char *name,
        struct hostent *ret, char *buf, size_t buflen,
        struct hostent **result, int *h_errnop);
EXTERN struct hostent* ( *bionic_gethostbyaddr) (const void *addr,
        socklen_t len, int type);
EXTERN int (*bionic_listen)(int fd, int backlog);
EXTERN ssize_t(*bionic_sendmmsg)(int fd, struct mmsghdr *msgvec, unsigned int vlen,
                                 unsigned int flags);
EXTERN int ( *system_qtaguid_tagSocket) (int sockfd, int tag, uid_t uid);
EXTERN int (*system_qtaguid_untagSocket)(int sockfd);
EXTERN int (*bionic_dup)(int sockfd);
EXTERN int (*bionic_dup2)(int oldfd, int newfd);
EXTERN int (*bionic_dup3)(int oldfd, int newfd, int flags);
#endif
#endif /* SWIM_SYNC_WRAPPER_DEFS_H */

