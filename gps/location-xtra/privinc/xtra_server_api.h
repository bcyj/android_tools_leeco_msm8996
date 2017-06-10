#ifndef XTRA_SERVER_API_H_
#define XTRA_SERVER_API_H_

/******************************************************************************
  @file:  xtra_servers_api.h
  @brief: XTRA and SNTP servers usage tracking

  DESCRIPTION

  XTRA and SNTP servers usage tracking

  -----------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technology Incoporated.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

//usage tracking

void Xtra_LoadServerStats(xtra_path_t szPath, server_stats_t *pStats);
void Xtra_SaveServerStats(xtra_path_t szPpath, server_stats_t *pStats);
void Xtra_NextServerNumber(int *pServer_number, int nMaxNumber);
void Xtra_SetLastSntpServer(int nServerNumber);
void Xtra_SetLastXtraServer(int nServerNumber);
int Xtra_GetLastSntpServer();
int Xtra_GetLastXtraServer();

//Server Access

int Xtra_DownloadSntpTime(xtra_assist_data_time_s_type *assist_time, xtra_url_t xtra_server_addr);
unsigned char* Xtra_HttpGet(const char* aUrl, const char *user_agent_string, DWORD *data_len);

#endif

