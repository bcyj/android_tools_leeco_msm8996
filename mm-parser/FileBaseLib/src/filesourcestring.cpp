// -*- Mode: C++ -*-
//=============================================================================
// FILE: filesourcestring.cpp
//
// SERVICES: fileparser
//
// DESCRIPTION:
/// Declarations required for file parser implementation
///
/// Copyright (c) 2011 Qualcomm Technologies, Inc.
/// All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

//$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/src/filesourcestring.cpp#8 $
//$DateTime: 2011/07/28 22:37:33 $
//$Change: 1858436 $

//=============================================================================

#include "filesourcestring.h"

void FILESOURCE_STRING::deallocate()
{
  if ( m_buffer != NULL )
  {
    MM_Delete_Array( m_buffer );
  }

  m_buffer = NULL;
  m_size = 0;
  m_capacity = 0;
}

void FILESOURCE_STRING::assign(const OSCL_TCHAR *cp)
{
  int32 size = 0;
  if (cp != NULL)
  {
#ifdef WINCE // TODO: verify
    size = (int32)wcslen((AECHAR*)cp);
#elif defined(USE_PARSER_WCHAR_ROUTINES)
      size = (int32)zrex_wcslen((wchar_t*)cp);
#else
    size = (int32)std_wstrlen((AECHAR*)cp);
#endif
  }
  m_buffer = MM_New_Array( OSCL_TCHAR, size+1 );
  if(cp)
  {
   #ifdef WINCE   //TODO: verify
      memcpy(m_buffer, cp, wcslen((AECHAR*)cp)*sizeof(wchar_t));
    #elif defined(USE_PARSER_WCHAR_ROUTINES)
      memcpy(m_buffer, cp, zrex_wcslen((wchar_t*)cp)*sizeof(wchar_t));
    #else
      memcpy(m_buffer, cp, std_wstrlen((AECHAR*)cp)*sizeof(wchar_t));
    #endif
  }
  if(m_buffer)
  {
    BUFFER[size]='\0';
  }
  m_capacity = size;
  m_size = size;
}
void FILESOURCE_STRING::assign(const char *cp)
{
  int32 size = 0;
  if (cp != NULL)
  {
    size =  (int32)strlen(cp);
  }
  m_buffer = MM_New_Array( OSCL_TCHAR, size+1 );
  if(cp)
  {
   #ifdef WINCE   //TODO: verify
      memcpy(m_buffer, cp, wcslen((AECHAR*)cp)*sizeof(wchar_t));
    #elif defined (USE_PARSER_WCHAR_ROUTINES)
      memcpy(m_buffer, cp, zrex_wcslen((wchar_t*)cp)*sizeof(wchar_t));
    #else
      memcpy(m_buffer, cp, std_wstrlen((AECHAR*)cp)*sizeof(wchar_t));
    #endif
  }
  if(m_buffer)
  {
    BUFFER[size]='\0';
  }
  m_capacity = size;
  m_size = size;
}

void FILESOURCE_STRING::assign(const OSCL_TCHAR *cp, int32 length)
{
  if (( cp == NULL ) || (length <= 0))
  {
    cp = (OSCL_TCHAR*)_T("");
    length = 0;
  }
  m_buffer = MM_New_Array( OSCL_TCHAR, length + 1 );
  if (m_buffer == NULL)
  {
    m_size = 0;
    m_capacity = 0;
    return;
  }
  /* strlcpy automatically null-terminates */
  memcpy(m_buffer, cp, length*sizeof(wchar_t));
  m_buffer[length] = '\0';

  m_size = length;
  m_capacity = length;
}
void FILESOURCE_STRING::assign(const char *cp, int32 length)
{
  if (( cp == NULL ) || (length <= 0))
  {
    cp = "";
    length = 0;
  }
  m_buffer = MM_New_Array( OSCL_TCHAR, length + 1 );
  if (m_buffer == NULL)
  {
    m_size = 0;
    m_capacity = 0;
    return;
  }
  /* strlcpy automatically null-terminates */
  memcpy(m_buffer, cp, length*sizeof(wchar_t));
  m_buffer[length] = '\0';

  m_size = length;
  m_capacity = length;
}

void FILESOURCE_STRING::assign(const FILESOURCE_STRING &src)
{
  if (src.m_buffer == NULL)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                 "assign: src buffer is NULL!");
    return;
  }

  int32 size = src.size();

  if (size < 0)
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL,
                  "assign: src size = %ld < 0!", size);
    return;
  }
  m_buffer = MM_New_Array( OSCL_TCHAR, size + 1 );

  if (m_buffer == NULL)
  {
    m_size = 0;
    m_capacity = 0;
    return;
  }
  memcpy(m_buffer, src.m_buffer, size*sizeof(wchar_t));
  m_buffer[size] ='\0';
  m_size = size;
  m_capacity = size;
}

FILESOURCE_STRING::FILESOURCE_STRING()
{
  m_buffer = NULL;
  assign((OSCL_TCHAR*)_T(""), 0);
}

FILESOURCE_STRING::FILESOURCE_STRING(const OSCL_TCHAR *cp)
{
  assign(cp);
}

FILESOURCE_STRING::FILESOURCE_STRING(const char* cp, uint32 length)
{
  assign(cp, length);
}
FILESOURCE_STRING& FILESOURCE_STRING::operator=(const uint8* src)
{
  if(src)
  {
    deallocate();
    assign((const char*)src,(int32)strlen((char*)src));
  }
  return *this;
}

FILESOURCE_STRING::FILESOURCE_STRING(const FILESOURCE_STRING &src )
{
  assign(src);
}

FILESOURCE_STRING::~FILESOURCE_STRING()
{
  deallocate();
}
const OSCL_TCHAR* FILESOURCE_STRING::get_cstr() const
{
  if ( BUFFER == NULL )
  {
    return FILESOURCE_STRING((OSCL_TCHAR*)"");
  }
  return BUFFER;
}

int32 FILESOURCE_STRING::get_size() const
{
  return m_size;
}

// Ensure this buffer has enough room for size characters without reallocating.
bool FILESOURCE_STRING::ensure_capacity(int32 capacity)
{
  if (m_capacity < capacity)
  {
    OSCL_TCHAR *buffer_to_dealloc = m_buffer;

    m_capacity = FILESOURCE_MAX((capacity << 1) - 1, 15);

    m_buffer = MM_New_Array( OSCL_TCHAR, m_capacity + 1 );

    if (m_buffer == NULL)
    {
      m_capacity = capacity;
      m_buffer = buffer_to_dealloc;
      return false;
    }
    memcpy(m_buffer, buffer_to_dealloc, m_capacity);
    m_buffer[m_capacity]='\0';

    MM_Delete_Array( buffer_to_dealloc );
  }
  return true;
}
FILESOURCE_STRING& FILESOURCE_STRING::operator=(const FILESOURCE_STRING &src)
{
  deallocate();
  assign(src);
  return *this;
}
FILESOURCE_STRING& FILESOURCE_STRING::operator+=(const FILESOURCE_STRING& src)
{
  return append(src);
}
FILESOURCE_STRING& FILESOURCE_STRING::operator+=(const uint8* src)
{
  return append(src,(int32)strlen((const char*)src) );
}

FILESOURCE_STRING& FILESOURCE_STRING::append(const wchar_t*src, int32 length)
{
  if (!ensure_capacity(length + m_size) || (m_buffer == NULL))
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,"Insufficient memory to append!");
    return *this;
  }
  memcpy(m_buffer + m_size, src, length*sizeof(wchar_t));
  BUFFER[m_size + length]='\0';
  m_size += length;
  return *this;
}
FILESOURCE_STRING& FILESOURCE_STRING::append(const FILESOURCE_STRING& suffix)
{
  return append(suffix.get_cstr(), suffix.size());
}
FILESOURCE_STRING& FILESOURCE_STRING::append(const uint8*src, int32 length)
{
  if(src)
  {
    if (!ensure_capacity(length + m_size) || (m_buffer == NULL))
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,"Insufficient memory to append!");
      return *this;
    }
    memcpy(m_buffer + m_size, src, length*sizeof(wchar_t));
    BUFFER[m_size + length]='\0';
    m_size += length;
  }
  return *this;
}
