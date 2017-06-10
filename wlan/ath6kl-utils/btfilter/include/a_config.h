/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */
#ifndef _A_CONFIG_H_
#define _A_CONFIG_H_

#ifdef ATHR_WM_NWF
#include "../os/windows/include/config.h"
#endif

#ifdef ATHR_CE_LEGACY
#include "../os/windows/include/config.h"
#endif

#if defined(__linux__) && !defined(LINUX_EMULATION)
#include "../os/linux/include/config_linux.h"
#endif

#ifdef REXOS
#include "../os/rexos/include/common/config_rexos.h"
#endif

#ifdef ATHR_WIN_NWF
#include "../os/windows/include/config.h"
#pragma warning( disable:4242)
#pragma warning( disable:4100)
#pragma warning( disable:4189)
#pragma warning( disable:4244)
#pragma warning( disable:4701)
#pragma warning( disable:4389)
#pragma warning( disable:4057)
#pragma warning( disable:28193)
#endif

#endif
