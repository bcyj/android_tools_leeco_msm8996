/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 **/

#ifndef __CONFIG_FM_THS_H__
#define __CONFIG_FM_THS_H__

#include <cstring>
#include "Qualcomm_FM_Const.h"
#include "ConfFileParser.h"

#define MAX_GRPS 3
#define MAX_SRCH_PARAMS 8
#define MAX_AF_PARAMS 3

#define SINR_SAMPLES_CNT_MIN 0
#define SINR_SAMPLES_CNT_MAX 255
#define SINR_FIRST_STAGE_MIN -128
#define SINR_FIRST_STAGE_MAX 127
#define RMSSI_FIRST_STAGE_MIN -128
#define RMSSI_FIRST_STAGE_MAX 127
#define INTF_LOW_TH_MIN 0
#define INTF_LOW_TH_MAX  255
#define INTF_HIGH_TH_MIN 0
#define INTF_HIGH_TH_MAX 255
#define SRCH_ALGO_TYPE_MIN 0
#define SRCH_ALGO_TYPE_MAX 1
#define SINR_FINAL_STAGE_MIN -128
#define SINR_FINAL_STAGE_MAX 127

#define AF_RMSSI_TH_MIN 0
#define AF_RMSSI_TH_MAX 65535
#define AF_RMSSI_SAMPLES_MIN 0
#define AF_RMSSI_SAMPLES_MAX 255
#define GOOD_CH_RMSSI_TH_MIN -128
#define GOOD_CH_RMSSI_TH_MAX 127

const unsigned char MAX_HYBRID_SRCH_PARAMS = 2;

struct NAME_MAP
{
   const char name[50];
   const int num;
};

enum PERFORMANCE_GRPS
{
    AF_THS,
    SRCH_THS,
    HYBRD_SRCH_LIST,
};

enum PERFORMANCE_SRCH_PARAMS
{
    SRCH_ALGO_TYPE,
    CF0_TH,
    SINR_FIRST_STAGE,
    SINR,
    RMSSI_FIRST_STAGE,
    INTF_LOW_TH,
    INTF_HIGH_TH,
    SINR_SAMPLES,
};

enum PERFORMANCE_AF_PARAMS
{
    AF_RMSSI_TH,
    AF_RMSSI_SAMPLES,
    GOOD_CH_RMSSI_TH,
};

enum HYBRID_SRCH_PARAMS
{
    FREQ_LIST,
    SINR_LIST,
};

//Keep this list in sorted order (ascending order in terms of "name")
//Don't change the name of GRPS, if changed please also change accordingly
//file: fm_srch_af_th.conf
static struct NAME_MAP GRPS_MAP[] =
{
   {"AFTHRESHOLDS", AF_THS},
   {"HYBRIDSEARCHLIST", HYBRD_SRCH_LIST},
   {"SEARCHTHRESHOLDS", SRCH_THS},
};

//Keep this list in sorted order (ascending order in terms of "name")
//Don't change the name of SEARCH thresholds,
//if changed please also change accordingly
//file: fm_srch_af_th.conf
static struct NAME_MAP SEACH_PARAMS_MAP[] =
{
   {"Cf0Th12", CF0_TH},
   {"IntfHighTh", INTF_HIGH_TH},
   {"IntfLowTh", INTF_LOW_TH},
   {"RmssiFirstStage", RMSSI_FIRST_STAGE},
   {"SearchAlgoType", SRCH_ALGO_TYPE},
   {"Sinr", SINR},
   {"SinrFirstStage", SINR_FIRST_STAGE},
   {"SinrSamplesCnt", SINR_SAMPLES},
};

//Keep this list in sorted order (ascending order in terms of "name")
//Don't change the name of SEARCH thresholds,
//if changed please also change accordingly
//file: fm_srch_af_th.conf
static struct NAME_MAP AF_PARAMS_MAP[] =
{
   {"AfRmssiSamplesCnt", AF_RMSSI_SAMPLES},
   {"AfRmssiTh", AF_RMSSI_TH},
   {"GoodChRmssiTh", GOOD_CH_RMSSI_TH},
};

static struct NAME_MAP HYBRD_SRCH_MAP[] =
{
   {"Freqs", FREQ_LIST},
   {"Sinrs", SINR_LIST},
};

class ConfigFmThs {
   private:
          group_table *keyfile;
          void set_srch_ths(UINT fd);
          void set_af_ths(UINT fd);
          unsigned int extract_comma_sep_freqs(char *freqs, unsigned int **freqs_arr, const char *str);
          unsigned int extract_comma_sep_sinrs(char *sinrs, signed char **sinrs_arr, const char *str);
          void set_hybrd_list(UINT fd);
   public:
          ConfigFmThs();
          ~ConfigFmThs();
          void SetRxSearchAfThs(const char *file, UINT fd);
};

#endif //__CONFIG_FM_THS_H__
