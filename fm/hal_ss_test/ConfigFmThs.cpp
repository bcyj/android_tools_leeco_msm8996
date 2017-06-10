/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 **/

#include <cstdlib>
#include <cstring>
#include "ConfigFmThs.h"
#include "FmPerformanceParams.h"
#include <utils/Log.h>

static int compare_name
(
   const void *name1, const void *name2
)
{
    char *first = (char *)name1;
    struct NAME_MAP *second = (struct NAME_MAP *)name2;

    return(strcmp(first, second->name));
}

ConfigFmThs :: ConfigFmThs
(
)
{
    keyfile = NULL;
}

ConfigFmThs :: ~ConfigFmThs
(
)
{
   free_key_file(keyfile);
}

void ConfigFmThs :: set_af_ths
(
   UINT fd
)
{
    signed char ret = FM_SUCCESS;
    char **keys;
    char **keys_cpy;
    char *key_value;
    int value;
    FmPerformanceParams perf_params;
    struct NAME_MAP *found;

    if(keyfile != NULL) {
       keys_cpy = keys = get_keys(keyfile, GRPS_MAP[0].name);
       if(keys != NULL) {
          while(*keys != NULL) {
              ALOGE("key found is: %s\n", *keys);
              found = (NAME_MAP *)bsearch(*keys, AF_PARAMS_MAP,
                          MAX_AF_PARAMS, sizeof(NAME_MAP), compare_name);
              if(found != NULL) {
                 key_value = get_value(keyfile,
                                     GRPS_MAP[0].name, found->name);
                 if((key_value != NULL) && strcmp(key_value, "")) {
                    value = atoi(key_value);
                    switch(found->num) {
                    case AF_RMSSI_TH:
                         if((value >= AF_RMSSI_TH_MIN)
                             && (value <= AF_RMSSI_TH_MAX)) {
                             ALOGE("Set af rmssi th: %d\n", value);
                             ret = perf_params.SetAfRmssiTh(fd, value);
                             if(ret == FM_FAILURE) {
                                ALOGE("Error in setting Af Rmssi th\n");
                                break;
                             }
                             unsigned short th;
                             ret = perf_params.GetAfRmssiTh(fd, th);
                             if(ret == FM_SUCCESS) {
                                ALOGE("Read af rmssith: %hd\n", th);
                             }else {
                                ALOGE("Error in reading Af Rmssi th\n");
                             }
                         }
                         break;
                    case AF_RMSSI_SAMPLES:
                         if((value >= AF_RMSSI_SAMPLES_MIN)
                             && (value <= AF_RMSSI_SAMPLES_MAX)) {
                             ALOGE("Set af rmssi samples cnt: %d\n", value);
                             ret = perf_params.SetAfRmssiSamplesCnt(fd, value);
                             if(ret == FM_FAILURE) {
                                ALOGE("Error in setting af rmssi samples\n");
                                break;
                             }
                             unsigned char cnt;
                             ret = perf_params.GetAfRmssiSamplesCnt(fd, cnt);
                             if(ret == FM_SUCCESS) {
                                ALOGE("Read af rmssi samples cnt: %hhd\n", cnt);
                             }else {
                                 ALOGE("Error in reading rmssi samples\n");
                             }
                         }
                         break;
                    case GOOD_CH_RMSSI_TH:
                         if((value >= GOOD_CH_RMSSI_TH_MIN)
                             && (value <= GOOD_CH_RMSSI_TH_MAX)) {
                             ALOGE("Set Good channle rmssi th: %d\n", value);
                             ret = perf_params.SetGoodChannelRmssiTh(fd, value);
                             if(ret == FM_FAILURE) {
                                ALOGE("Error in setting Good channle rmssi th\n");
                                break;
                             }
                             signed char th;
                             ret = perf_params.GetGoodChannelRmssiTh(fd, th);
                             if(ret == FM_SUCCESS) {
                                ALOGE("Read good channel rmssi th: %d\n", th);
                             }else {
                                ALOGE("Error in reading Good channle rmssi th\n");
                             }
                         }
                         break;
                   }
                 }else {
                   ALOGE("key_val for key: %s is empty\n",
                             *keys);
                 }
                 free(key_value);
              }
              keys++;
          }
       }else {
          ALOGE("No of keys found is zero\n");
       }
       free_strs(keys_cpy);
    }else {
       ALOGE("key file is null\n");
    }
}

void ConfigFmThs :: set_srch_ths
(
    UINT fd
)
{
    signed char ret = FM_SUCCESS;
    char **keys = NULL;
    char **keys_cpy = NULL;
    char *key_value = NULL;
    int value;
    FmPerformanceParams perf_params;
    struct NAME_MAP *found = NULL;

    if(keyfile != NULL) {
       keys_cpy = keys = get_keys(keyfile, GRPS_MAP[2].name);
       if(keys != NULL) {
          while(*keys != NULL) {
              found = (NAME_MAP *)bsearch(*keys, SEACH_PARAMS_MAP,
                           MAX_SRCH_PARAMS, sizeof(NAME_MAP), compare_name);
              if(found != NULL) {
                 key_value = get_value(keyfile, GRPS_MAP[2].name, found->name);
                 ALOGE("found srch ths: %s: %s\n", found->name, key_value);
                 if((key_value != NULL) && strcmp(key_value, "")) {
                    value = atoi(key_value);
                    switch(found->num) {
                    case SINR_FIRST_STAGE:
                         if((value >= SINR_FIRST_STAGE_MIN)
                             && (value <= SINR_FIRST_STAGE_MAX)) {
                             ALOGE("Set sinr first stage: %d\n", value);
                             ret = perf_params.SetSinrFirstStage(fd, value);
                             if(ret == FM_FAILURE) {
                                ALOGE("Error in setting sinr first stage\n");
                                break;
                             }
                             signed char th;
                             ret = perf_params.GetSinrFirstStage(fd, th);
                             if(ret == FM_SUCCESS) {
                                ALOGE("Read sinr first stage: %d\n", th);
                             }else {
                                ALOGE("Error in reading sinr first stage\n");
                             }
                         }
                         break;
                    case RMSSI_FIRST_STAGE:
                         if((value >= RMSSI_FIRST_STAGE_MIN)
                             && (value <= RMSSI_FIRST_STAGE_MAX)) {
                             ALOGE("Set rmssi first stage: %d\n", value);
                             ret = perf_params.SetRmssiFirstStage(fd, value);
                             if(ret == FM_FAILURE) {
                                ALOGE("Error in setting rmssi first stage\n");
                                break;
                             }
                             signed char th;
                             ret = perf_params.GetRmssiFirstStage(fd, th);
                             if(ret == FM_SUCCESS) {
                                ALOGE("Read rmssi first stage: %d\n", th);
                             }else {
                                ALOGE("Error in reading rmssi first stage\n");
                             }
                         }
                         break;
                    case INTF_LOW_TH:
                         if((value >= INTF_LOW_TH_MIN)
                             && (value <= INTF_LOW_TH_MAX)) {
                            ALOGE("Set intf low th: %d\n", value);
                            ret = perf_params.SetIntfLowTh(fd, value);
                            if(ret == FM_FAILURE) {
                                ALOGE("Error in setting intf low th\n");
                                break;
                            }
                            unsigned char th;
                            ret = perf_params.GetIntfLowTh(fd, th);
                            if(ret == FM_SUCCESS) {
                               ALOGE("Read intf low th: %u\n", th);
                            }else {
                               ALOGE("Error in reading intf low th\n");
                            }
                         }
                         break;
                    case INTF_HIGH_TH:
                         if((value >= INTF_HIGH_TH_MIN)
                             && (value <= INTF_HIGH_TH_MAX)) {
                            ALOGE("Set intf high th: %d\n", value);
                            ret = perf_params.SetIntfHighTh(fd, value);
                            if(ret == FM_FAILURE) {
                                ALOGE("Error in setting intf high th\n");
                                break;
                            }
                            unsigned char th;
                            ret = perf_params.GetIntfHighTh(fd, th);
                            if(ret == FM_SUCCESS) {
                               ALOGE("Read intf high th: %u\n", th);
                            }else {
                               ALOGE("Error in reading intf high th\n");
                            }
                         }
                         break;
                    case CF0_TH:
                         ALOGE("Set cf0 th: %d\n", value);
                         ret = perf_params.SetCf0Th12(fd, value);
                         if(ret == FM_FAILURE) {
                            ALOGE("Error in setting cf0 th\n");
                            break;
                         }
                         int th;
                         ret = perf_params.GetCf0Th12(fd, th);
                         if(ret == FM_SUCCESS) {
                            ALOGE("Read CF012 th: %d\n", th);
                         }else {
                            ALOGE("Error in reading cf0 th\n");
                         }
                         break;
                    case SRCH_ALGO_TYPE:
                         if((value >= SRCH_ALGO_TYPE_MIN)
                             && (value <= SRCH_ALGO_TYPE_MAX)) {
                             ALOGE("Set search algo type: %d\n", value);
                             ret = perf_params.SetSrchAlgoType(fd, value);
                             if(ret == FM_FAILURE) {
                                ALOGE("Error in setting search algo type\n");
                                break;
                             }
                             unsigned char algo;
                             ret = perf_params.GetSrchAlgoType(fd, algo);
                             if(ret == FM_SUCCESS) {
                                ALOGE("Read algo type: %u\n", algo);
                             }else {
                                ALOGE("Error in reading search algo type\n");
                             }
                         }
                         break;
                    case SINR_SAMPLES:
                         if((value >= SINR_SAMPLES_CNT_MIN)
                             && (value <= SINR_SAMPLES_CNT_MAX)) {
                             ALOGE("Set sinr samples count: %d\n", value);
                             ret = perf_params.SetSinrSamplesCnt(fd, value);
                             if(ret == FM_FAILURE) {
                                ALOGE("Error in setting sinr samples count\n");
                                break;
                             }
                             unsigned char cnt;
                             ret = perf_params.GetSinrSamplesCnt(fd, cnt);
                             if(ret == FM_SUCCESS) {
                                ALOGE("Read sinr samples cnt: %u\n", cnt);
                             }else {
                                ALOGE("Error in reading sinr samples count\n");
                             }
                         }
                         break;
                    case SINR:
                         if((value >= SINR_FINAL_STAGE_MIN)
                             && (value <= SINR_FINAL_STAGE_MAX)) {
                             ALOGE("Set final stage sinr: %d\n", value);
                             ret = perf_params.SetSinrFinalStage(fd, value);
                             if(ret == FM_FAILURE) {
                                ALOGE("Error in setting final stage sinr\n");
                                break;
                             }
                             signed char th;
                             ret = perf_params.GetSinrFinalStage(fd, th);
                             if(ret == FM_SUCCESS) {
                                ALOGE("Read final stage sinr: %d\n", th);
                             }else {
                                ALOGE("Error in reading final stage sinr\n");
                             }
                         }
                         break;
                    }
                 }else {
                    ALOGE("key_value for key: %s is empty\n",
                                  *keys);
                 }
                 free(key_value);
              }
              keys++;
          }
       }else {
          ALOGE("No of keys found is zero\n");
       }
       free_strs(keys_cpy);
    }else {
       ALOGE("key file is null\n");
    }
}

void ConfigFmThs :: set_hybrd_list
(
    UINT fd
)
{
    signed char ret = FM_SUCCESS;
    char **keys = NULL;
    char **keys_cpy = NULL;
    char *key_value = NULL;
    char *freqs = NULL;
    unsigned int *freqs_array = NULL;
    signed char *sinrs_array = NULL;
    char *sinrs = NULL;
    int value;
    unsigned int freq_cnt = 0;
    unsigned int sinr_cnt = 0;
    FmPerformanceParams perf_params;
    struct NAME_MAP *found;

    ALOGE("Inside hybrid srch list\n");
    if(keyfile != NULL) {
       keys_cpy = keys = get_keys(keyfile, GRPS_MAP[1].name);
       if(keys != NULL) {
          while(*keys != NULL) {
              found = (NAME_MAP *)bsearch(*keys, HYBRD_SRCH_MAP,
                           MAX_HYBRID_SRCH_PARAMS, sizeof(NAME_MAP), compare_name);
              if(found != NULL) {
                 key_value = get_value(keyfile, GRPS_MAP[1].name, found->name);
                 if((key_value != NULL) && strcmp(key_value, "")) {
                     switch(found->num) {
                     case FREQ_LIST:
                          freqs = key_value;
                          break;
                     case SINR_LIST:
                          sinrs = key_value;
                          break;
                     default:
                          free(key_value);
                          break;
                     }
                 }
              }
              keys++;
          }
          free_strs(keys_cpy);
       }else {
          ALOGE("No of keys found is zero\n");
       }
    }else {
       ALOGE("key file is null\n");
    }

    freq_cnt = extract_comma_sep_freqs(freqs, &freqs_array, ",");
    sinr_cnt = extract_comma_sep_sinrs(sinrs, &sinrs_array, ",");

    if((freq_cnt == sinr_cnt) && (sinr_cnt > 0)) {
       perf_params.SetHybridSrchList(fd, freqs_array, sinrs_array, freq_cnt);
    }

    free(freqs);
    free(sinrs);
    free(freqs_array);
    free(sinrs_array);
}

unsigned int ConfigFmThs :: extract_comma_sep_freqs
(
    char *freqs,
    unsigned int **freqs_arr,
    const char *str
)
{
    char *next_freq;
    unsigned int freq;
    unsigned int *freqs_new_arr;
    unsigned int size = 0;
    unsigned int len = 0;

    next_freq = strtok(freqs, str);
    while(next_freq != NULL) {
          freq = atoi(next_freq);
          ALOGD("HYBRID_SRCH freq: %u\n", freq);
          if(size == len) {
             size <<= 1;
             if(size == 0)
                size = 1;
             freqs_new_arr = (unsigned int *)realloc(*freqs_arr,
                                              size * sizeof(unsigned int));
             if(freqs_new_arr == NULL) {
                free(*freqs_arr);
                *freqs_arr = NULL;
                break;
             }
             *freqs_arr = freqs_new_arr;
          }
          (*freqs_arr)[len] = freq;
          len++;
          next_freq = strtok(NULL, str);
    }
    return len;
}

unsigned int ConfigFmThs :: extract_comma_sep_sinrs
(
    char *sinrs,
    signed char **sinrs_arr,
    const char *str
)
{
    char *next_sinr;
    signed char *sinrs_new_arr;
    unsigned int size = 0;
    unsigned int len = 0;
    signed char sinr;

    next_sinr = strtok(sinrs, str);
    while(next_sinr != NULL) {
          sinr = atoi(next_sinr);
          ALOGD("HYBRID_SRCH sinr: %d\n", sinr);
          if(size == len) {
             size <<= 1;
             if(size == 0)
                size = 1;
             sinrs_new_arr = (signed char *)realloc(*sinrs_arr,
                                               size * sizeof(signed char));
             if(sinrs_new_arr == NULL) {
                free(*sinrs_arr);
                *sinrs_arr = NULL;
                break;
             }
             *sinrs_arr = sinrs_new_arr;
          }
          (*sinrs_arr)[len] = sinr;
          len++;
          next_sinr = strtok(NULL, str);
    }
    return len;
}

void  ConfigFmThs :: SetRxSearchAfThs
(
    const char *file, UINT fd
)
{
    int index;
    struct NAME_MAP *found;
    char **grps = NULL;
    char **grps_cpy = NULL;

    keyfile = get_key_file();

    ALOGE("file name is: %s\n", file);
    if(!parse_load_file(keyfile, file)) {
       ALOGE("Error in loading threshold file\n");
    }else {
       grps_cpy = grps = get_grps(keyfile);
       if(grps != NULL) {
          while(*grps != NULL) {
              ALOGE("Search grp: %s\n", *grps);
              found = (NAME_MAP *)bsearch(*grps, GRPS_MAP, MAX_GRPS,
                             sizeof(NAME_MAP), compare_name);
              if(found != NULL) {
                 ALOGE("Found group: %s\n", found->name);
                 switch(found->num) {
                 case AF_THS:
                      set_af_ths(fd);
                      break;
                 case SRCH_THS:
                      set_srch_ths(fd);
                      break;
                 case HYBRD_SRCH_LIST:
                      set_hybrd_list(fd);
                      break;
                 }
              }
              grps++;
          }
       }else {
          ALOGE("No of groups found is zero\n");
       }
       free_strs(grps_cpy);
    }
    free_key_file(keyfile);
    keyfile = NULL;
}
