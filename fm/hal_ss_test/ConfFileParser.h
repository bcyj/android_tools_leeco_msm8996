/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 **/

#ifndef __CONF_FILE_PARSER_H__
#define __CONF_FILE_PARSER_H__

#define MAX_LINE_LEN 512
#define MAX_UNIQ_KEYS 5
#define MAX_UNIQ_GRPS 10
#define TRUE 1
#define FALSE 0

struct key_value_pair_list
{
   char *key;
   char *value;
   key_value_pair_list *next;
};

struct group
{
    char *grp_name;
    unsigned int num_of_keys;
    unsigned int keys_hash_size;
    key_value_pair_list **list;
    group *grp_next;
};

struct group_table
{
    unsigned int grps_hash_size;
    unsigned int num_of_grps;
    group **grps_hash;
};

enum CONF_PARSE_ERRO_CODE
{
  PARSE_SUCCESS,
  INVALID_FILE_NAME,
  FILE_OPEN_FAILED,
  FILE_NOT_PROPER,
  MEMORY_ALLOC_FAILED,
  PARSE_FAILED,
};

unsigned int get_hash_code(const char *str);
group_table *get_key_file();
void free_strs(char **str_array);
void free_key_file(group_table *key_file);
char parse_load_file(group_table *key_file, const char *file);
char **get_grps(const group_table *key_file);
char **get_keys(const group_table *key_file, const char *grp);
char *get_value(const group_table *key_file, const char *grp,
                 const char *key);

#endif //__CONF_FILE_PARSER_H__
