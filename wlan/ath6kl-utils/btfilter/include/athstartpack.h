/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */
#ifdef VXWORKS
#endif /* VXWORKS */

#if defined(LINUX) || defined(__linux__)
#endif /* LINUX */

#ifdef QNX
#endif /* QNX */

#ifdef INTEGRITY
#include "integrity/athstartpack_integrity.h"
#endif /* INTEGRITY */

#ifdef NUCLEUS
#endif /* NUCLEUS */

#ifdef ATHR_WM_NWF
#include "../os/windows/include/athstartpack.h"
#define PREPACK
#endif

#ifdef ATHR_CE_LEGACY
#include "../os/windows/include/athstartpack.h"
#endif /* WINCE */

#ifdef ATHR_WIN_NWF

#ifndef PREPACK
#define PREPACK __declspec(align(1))
#endif

#include <athstartpack_win.h>
#define __ATTRIB_PACK POSTPACK

#endif

#if __LONG_MAX__ == __INT_MAX__
/* 32-bit compilation */
#define PREPACK64
#define POSTPACK64
#else
/* 64-bit compilation */
#define PREPACK64 PREPACK
#define POSTPACK64 POSTPACK
#endif
