/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */
#ifndef _A_TYPES_H_
#define _A_TYPES_H_

#if defined(__linux__) && !defined(LINUX_EMULATION)
#include "../os/linux/include/athtypes_linux.h"
#endif

#ifdef ATHR_WM_NWF
#include "../os/windows/include/athtypes.h"
#endif

#ifdef ATHR_CE_LEGACY
#include "../os/windows/include/athtypes.h"
#endif

#ifdef REXOS
#include "../os/rexos/include/common/athtypes_rexos.h"
#endif

#if defined ART_WIN
#include "../os/win_art/include/athtypes_win.h"
#endif

#ifdef ATHR_WIN_NWF
#include <athtypes_win.h>
#endif

#endif /* _ATHTYPES_H_ */
