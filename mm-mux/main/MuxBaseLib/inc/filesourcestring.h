#ifndef FILESOURCE_STRING_H
#define FILESOURCE_STRING_H
/* =======================================================================
                          filesource_string.h
DESCRIPTION
  This is a simple string class without any multithread access protection.
========================================================================== */
/*
  Copyright (c) 2011, 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential. */

/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/filesourcestring.h#7 $
$DateTime: 2011/07/28 22:37:33 $
$Change: 1858436 $
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

//#include "parserdatadef.h"
#include "MMMalloc.h"
#include "MMMemory.h"
#include "MMDebugMsg.h"
#include "zrex_string.h"

/* ==========================================================================
                        DATA DECLARATIONS
========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

#if defined WINCE || defined PLATFORM_LTK
  #define INTF_DLL __declspec( dllexport )
#else
  #define INTF_DLL
#endif

#ifndef _T
  #define _T(x) L ## x
#endif
#define BUFFER m_buffer
typedef wchar_t  OSCL_TCHAR;
#ifndef WCHAR
#define WCHAR wchar_t
#endif

//! filesource_string_status_t defines return codes for filesourcestring APIs
typedef enum
{
  FILESOURCE_STRING_SUCCESS,
  FILESOURCE_STRING_FAIL,
  FILESOURCE_STRING_ETIMEOUT,
  FILESOURCE_STRING_ECONNREF,
  FILESOURCE_STRING_EINVALIDADDRESS,
  FILESOURCE_STRING_ENOPORT,
  FILESOURCE_STRING_EBADSOCK,
  FILESOURCE_STRING_ECANTBIND,
  FILESOURCE_STRING_ECANTLISTEN,
  FILESOURCE_STRING_EINDEXOUTOFBOUND,
  FILESOURCE_STRING_EMEMORY
} filesource_string_status_t;

class INTF_DLL FILESOURCE_STRING
{
public:

  FILESOURCE_STRING();
  FILESOURCE_STRING(const wchar_t* src);
  FILESOURCE_STRING(const char* src, uint32 length);
  FILESOURCE_STRING& operator=(const uint8*);
  FILESOURCE_STRING& operator=(const FILESOURCE_STRING&);
  FILESOURCE_STRING& operator+=(const FILESOURCE_STRING& src);
  FILESOURCE_STRING& operator+=(const uint8* src);
  FILESOURCE_STRING& append(const uint8*cp, int32 length);
  FILESOURCE_STRING& append(const wchar_t*cp, int32 length);
  FILESOURCE_STRING& append(const FILESOURCE_STRING& suffix);

  FILESOURCE_STRING(const FILESOURCE_STRING& src);
  ~FILESOURCE_STRING();
  operator const wchar_t*() const { return get_cstr(); }
  //Size count space for '\0' as well.
  int32 get_size() const;
  const wchar_t* get_cstr() const;

  int32 size() const
  {
    return get_size();
  }
  const wchar_t* c_str() const
  {
    return get_cstr();
  }
protected:
  void deallocate();
  void assign(const wchar_t *cp);
  void assign(const char* cp);
  void assign(const char *cp, int32 length);
  void assign(const wchar_t *cp, int32 length);
  void assign(const FILESOURCE_STRING &src);
  bool ensure_capacity(int32 capacity);
  wchar_t *m_buffer;
  int32 m_size;
  int32 m_capacity;
};
#endif /* FILESOURCE_STRING_H */
