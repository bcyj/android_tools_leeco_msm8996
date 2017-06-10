/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cutils/sockets.h>

#define  BUF_LENGTH_SIMPLE_DATA (128)
#define  BUF_LENGTH_CONFIG_DATA (256)
#define  MAX_FILE_NAME_LEN      (32)
#define  CMD_QDCM_PREFIX "qdcm"
#define  CMD_QDCM_PA_PREFIX "qdcm:pa"
#define  CMD_SET_COLORTEMP "qdcm:set:colortemp"
#define  CMD_GET_COLORTEMP "qdcm:get:colortemp"
#define  CMD_ENUMERATE_MODES "qdcm:get:modes"
#define  CMD_ADD_MODE        "qdcm:add:mode"
#define  CMD_SAVE_SETTING    "qdcm:save"
#define  CMD_GET_UI_ATTRIBUTE "qdcm:get:attribute"
#define  CMD_DELETE_MODE      "qdcm:delete:mode"
#define  CMD_GET_ACTIVEMODE   "qdcm:get:activemode"
#define  CMD_APPLY_THISMODE   "qdcm:apply:mode"
#define  CMD_GET_PROFILES_NUM "qdcm:get:profilenum"
#define  CMD_GET_DEFAULTMODE   "qdcm:get:defaultmode"
#define  CMD_SET_DEFAULTMODE   "qdcm:set:defaultmode"
#define  CMD_GET_PA_GLOBAL_RANGE   "qdcm:pa:glb:get:range"
#define  CMD_GET_PA_GLOBAL_CONFIG  "qdcm:pa:glb:get:config"
#define  CMD_SET_PA_GLOBAL_CONFIG  "qdcm:pa:glb:set:config"
#define  CMD_GET_PA_MEMORY_RANGE   "qdcm:pa:mem:get:range"
#define  CMD_GET_PA_MEMORY_CONFIG  "qdcm:pa:mem:get:config"
#define  CMD_SET_PA_MEMORY_CONFIG  "qdcm:pa:mem:set:config"
#define  RESPONSE_SUCCESS     "Success"
#define  RESPONSE_FAILURE     "Failure"
#define FORMAT_LENGTH (64)

struct disp_profile_priv {
    int id;
    char name[MAX_FILE_NAME_LEN];
    int type;
};

enum blend_mask {
    CT_BIT0 = 0x01, /* Color Temperature */
    GA_HSVC = 0x02, /* Global Adjustment */
    MC_SKIN = 0x04, /* Memory Color Skin */
    MC_SKY  = 0x08, /* Memory Color Sky */
    MC_FOL  = 0x10, /* Memory Color Foliage */
};

static inline int socketWrite_string(const int32_t* fd, const char* buf) {
    size_t size = strlen(buf);
    int32_t ret = write(*fd, buf, size);
    if (ret < 0 || (size_t)ret < size) {
        LOGE("Failed to write socket");
        ret = -EFAULT;
    }
    return 0;
}

static inline int socketWrite_binary(const int32_t* fd,
                                        const char* buf, size_t len) {
    int32_t ret = write(*fd, buf, len);
    if (ret < 0) {
        LOGE("Failed to write socket");
        ret = -EFAULT;
    }
    return 0;
}

static inline int socketRead(const int32_t* fd, char* sink, size_t len) {
    int32_t ret = 0;

    ret = read(*fd, sink, len);
    if (ret < 0) {
        LOGE("Failed to read socket");
        ret = -EFAULT;
    } else {
        if (!strncmp(sink, RESPONSE_FAILURE, strlen(RESPONSE_FAILURE)))
            ret = -EFAULT;
    }

    //if good, return actual bytes read back from socket.
    return ret;
}

static inline int unmarshall_prefix_onedata(const char* src,
                                    const char* prefix, int32_t* value) {
    char format[FORMAT_LENGTH] = {0, };
    int ret = 0;

    *value = 0;
    strlcat(format, prefix, FORMAT_LENGTH);
    ret = strlcat(format, ";%d", FORMAT_LENGTH);
    if (ret >= FORMAT_LENGTH)
        return -EINVAL;

    sscanf(src, format, value);
    return 0;
}

static inline int unmarshall_prefix_str_onedata(const char* src,
            const char* prefix, char *arg1, int32_t* value) {
    char format[FORMAT_LENGTH] = {0, };
    int ret = 0;

    strlcat(format, prefix, FORMAT_LENGTH);
    ret = strlcat(format, ":%s%d", FORMAT_LENGTH);
    if (ret >= FORMAT_LENGTH)
        return -EINVAL;

    *value = 0;
    sscanf(src, format, arg1, value);
    return 0;
}

static inline int unmarshall_prefix_name_twodata(const char* src,
            const char* prefix, char *arg1, int32_t* value, int32_t* value1) {
    char format[FORMAT_LENGTH] = {0, };
    int ret = 0;
    int x, y;

    strlcat(format, prefix, FORMAT_LENGTH);
    ret = strlcat(format, ":%d:%d:%s", FORMAT_LENGTH);
    if (ret >= FORMAT_LENGTH)
        return -EINVAL;

    *value = *value1 = 0;
    x = y = 0;
    sscanf(src, format, &x, &y, arg1);
    LOGD("%s format =%s arg1 = %s value = %d value1 = %d", __FUNCTION__, format,
            src, (int)x, (int)y);
    *value = x;
    *value1 = y;
    return 0;
}

static inline int unmarshall_prefix_str_twodata(const char* src,
            const char* prefix, char *arg1, int32_t* value, int32_t* value1) {
    char format[FORMAT_LENGTH] = {0, };
    int ret = 0;
    int x, y;

    strlcat(format, prefix, FORMAT_LENGTH);
    ret = strlcat(format, ";%d;%d:%[^\t\n]", FORMAT_LENGTH);
    if (ret >= FORMAT_LENGTH)
        return -EINVAL;

    *value = *value1 = 0;
    x = y = 0;
    sscanf(src, format, &x, &y, arg1);
    LOGD("%s format = %s arg1 = %s value = %d value1 = %d", __FUNCTION__, format,
            src, (int)x, (int)y);
    *value = x;
    *value1 = y;
    return 0;
}

static inline int unmarshall_prefix_twodata(const char* src,
                            const char* prefix, int32_t* arg1, int32_t* arg2) {
    char format[FORMAT_LENGTH] = {0, };
    int ret = 0;

    strlcat(format, prefix, FORMAT_LENGTH);
    ret = strlcat(format, ";%d;%d", FORMAT_LENGTH);
    if (ret >= FORMAT_LENGTH)
        return -EINVAL;

    *arg1 = *arg2 = 0;
    sscanf(src, format, arg1, arg2);
    return 0;
}

static inline int unmarshall_prefix_sixdata(const char* src,
            const char* prefix, int32_t* arg1, int32_t* arg2, int32_t* arg3,
                            int32_t* arg4, int32_t* arg5, int32_t* arg6) {
    char format[FORMAT_LENGTH] = {0, };
    int ret = 0;

    strlcat(format, prefix, FORMAT_LENGTH);
    ret = strlcat(format, ";%d;%d;%d;%d;%d;%d", FORMAT_LENGTH);
    if (ret >= FORMAT_LENGTH)
        return -EINVAL;

    *arg1 = *arg2 = *arg3 = *arg4 = *arg5 = *arg6 = 0;
    sscanf(src, format, arg1, arg2, arg3, arg4, arg5, arg6);
    return 0;
}

static inline int unmarshall_prefix_sevendata(const char* src,
            const char* prefix, int32_t* arg1, int32_t* arg2, int32_t* arg3,
            int32_t* arg4, int32_t* arg5, int32_t* arg6, int32_t* arg7) {
    char format[FORMAT_LENGTH] = {0, };
    int ret = 0;

    strlcat(format, prefix, FORMAT_LENGTH);
    ret = strlcat(format, ";%d;%d;%d;%d;%d;%d;%d", FORMAT_LENGTH);
    if (ret >= FORMAT_LENGTH)
        return -EINVAL;

    *arg1 = *arg2 = *arg3 = *arg4 = *arg5 = *arg6 = *arg7 = 0;
    sscanf(src, format, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    return 0;
}

static inline int unmarshall_onedata(const char* src, int32_t* value) {
    *value = 0;
    sscanf(src, "%d", value);
    return 0;
}

static inline int unmarshall_twodata(const char* src,
                                            int32_t* arg1, int32_t* arg2) {
    *arg1 = *arg2 = 0;
    sscanf(src, "%d;%d", arg1, arg2);
    return 0;
}

static inline int marshall_prefix_str_onedata(char* sink,
            const char* prefix, const char *arg1, int32_t value) {
    snprintf(sink, BUF_LENGTH_SIMPLE_DATA, "%s:%s %d", prefix, arg1, value);
    return BUF_LENGTH_SIMPLE_DATA;
}

static inline int marshall_prefix_str_twodata(char* sink,
       const char* prefix, const char *arg1, int32_t value, int32_t value1) {
    snprintf(sink, BUF_LENGTH_SIMPLE_DATA, "%s;%d;%d:%s",
            prefix, value, value1, arg1);
    return BUF_LENGTH_SIMPLE_DATA;
}

static inline int marshall_prefix_onedata(char* sink,
                                    const char* prefix, int32_t value) {
    int ret = 0;
    ret = snprintf(sink, BUF_LENGTH_SIMPLE_DATA, "%s;%d", prefix, value);
    return (ret <= 0)? -EINVAL : BUF_LENGTH_SIMPLE_DATA;
}

static inline int marshall_prefix_twodata(char* sink, const char* prefix,
                                        int32_t arg1, int32_t arg2) {
    snprintf(sink, BUF_LENGTH_SIMPLE_DATA, "%s;%d;%d", prefix, arg1, arg2);
    return BUF_LENGTH_SIMPLE_DATA;
}

static inline int marshall_prefix_fourdata(char* sink, const char* prefix,
                    int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4) {
    snprintf(sink, BUF_LENGTH_CONFIG_DATA, "%s;%d;%d;%d;%d", prefix,
                                                    arg1, arg2, arg3, arg4);
    return BUF_LENGTH_CONFIG_DATA;
}

static inline int marshall_prefix_sixdata(char* sink, const char* prefix,
                    int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4,
                                                int32_t arg5, int32_t arg6) {
    snprintf(sink, BUF_LENGTH_CONFIG_DATA, "%s;%d;%d;%d;%d;%d;%d;", prefix,
                                        arg1, arg2, arg3, arg4, arg5, arg6);
    return BUF_LENGTH_CONFIG_DATA;
}

static inline int marshall_prefix_tendata(char* sink, const char* prefix,
     int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4, int32_t arg5,
        int32_t arg6, int32_t arg7, int32_t arg8, int32_t arg9, int32_t arg10) {
    snprintf(sink, BUF_LENGTH_CONFIG_DATA, "%s;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;",
        prefix, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
    return BUF_LENGTH_CONFIG_DATA;
}

static inline int marshall_prefix_str(char* sink,
                                const char* prefix, const char* cmd) {
    snprintf(sink, BUF_LENGTH_SIMPLE_DATA, "%s:%s", prefix, cmd);
    return BUF_LENGTH_SIMPLE_DATA;
}

static inline int marshall_onedata(char* sink, int32_t value) {
    snprintf(sink, BUF_LENGTH_SIMPLE_DATA, "%d", value);
    return BUF_LENGTH_SIMPLE_DATA;
}

static inline int marshall_twodata(char* sink, int32_t arg1, int32_t arg2) {
    snprintf(sink, BUF_LENGTH_SIMPLE_DATA, "%d;%d", arg1, arg2);
    return BUF_LENGTH_SIMPLE_DATA;
}

static inline int marshall_disp_profiles(char** sink, int32_t count,
                                struct disp_profile_priv* src) {
    //allow performing different encoding methods, currently similar to
    //memcpy via socket.write().
    return sizeof(struct disp_profile_priv) * count + sizeof(int32_t);
}

static inline int unmarshall_disp_profiles(char* src, int32_t* count,
                                struct disp_profile_priv** sink) {
    if (src == NULL)
        return -EINVAL;

    *count = *(int32_t *)src;
    struct disp_profile_priv *profiles =
                (struct disp_profile_priv *)(src + sizeof(int32_t));
    *sink = profiles;
    return 0;
}

#endif

#ifdef __cplusplus
};
#endif
