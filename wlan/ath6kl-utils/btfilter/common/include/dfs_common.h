/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */


#ifndef _DFS_COMMON_H_
#define _DFS_COMMON_H_

enum {
	DFS_UNINIT_DOMAIN	= 0,	/* Uninitialized dfs domain */
	DFS_FCC_DOMAIN		= 1,	/* FCC3 dfs domain */
	DFS_ETSI_DOMAIN		= 2,	/* ETSI dfs domain */
	DFS_MKK4_DOMAIN		= 3	/* Japan dfs domain */
};

/* Constants to use for chirping detection, all are unconverted as HW reports them */
#define MIN_BIN5_DUR  63 /* 50 * 1.25*/
#define MIN_BIN5_DUR_MICROSEC 50
#define MAYBE_BIN5_DUR 27 /* 22 * 1.25*/
#define MAYBE_BIN5_DUR_MICROSEC 22
#define MAX_BIN5_DUR 145  /* XXX: Use 145 for Osprey. Does this apply to McKinley too? */
#define MAX_BIN5_DUR_MICROSEC 105

/*Following threshold is not specified but should be okay statistically*/
#define DFS_BIN5_BRI_LOWER_LIMIT	300000  /* us */
#define DFS_BIN5_BRI_UPPER_LIMIT	2000000  /* us */


PREPACK struct ath_dfs_capinfo {
	A_UINT8 enable_radar;
} POSTPACK;

typedef struct ath_dfs_capinfo WMI_DFS_HOST_ATTACH_EVENT;

PREPACK struct ath_dfs_info {
	A_UINT32 dfs_domain;
} POSTPACK;

typedef struct ath_dfs_info WMI_DFS_HOST_INIT_EVENT;


PREPACK struct dfs_event_info {
	A_UINT64  full_ts;    /* 64-bit full timestamp from interrupt time */
	A_UINT32  ts;         /* Original 15 bit recv timestamp */
	A_UINT32  ext_chan_busy; /* Ext chan busy % */
	A_UINT8   rssi;       /* rssi of radar event */
	A_UINT8   dur;        /* duration of radar pulse */
	A_UINT8   chanindex;  /* Channel of event */
	A_UINT8   flags;
#define CH_TYPE_MASK 1
#define PRIMARY_CH 0
#define EXT_CH 1
} POSTPACK;

/* XXX: Replace 256 with WMI_SVC_MAX_BUFFERED_EVENT_SIZE */
#define WMI_DFS_EVENT_MAX_BUFFER_SIZE ((256 - 1)/sizeof(struct dfs_event_info))
/* Fill in event info */
PREPACK struct dfs_ev_buffer {
	A_UINT8 num_events;
	struct dfs_event_info ev_info[WMI_DFS_EVENT_MAX_BUFFER_SIZE];
} POSTPACK;

typedef struct dfs_ev_buffer WMI_DFS_PHYERR_EVENT;


/* This should match the table from if_ath.c */
enum {
	ATH_DEBUG_DFS       = 0x00000100,   /* Minimal DFS debug */
	ATH_DEBUG_DFS1      = 0x00000200,   /* Normal DFS debug */
	ATH_DEBUG_DFS2      = 0x00000400,   /* Maximal DFS debug */
	ATH_DEBUG_DFS3      = 0x00000800,   /* matched filterID display */
};

#define TRAFFIC_DETECTED 1

#endif  /* _DFS_H_ */
