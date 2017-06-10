/* 
 * Copyright (c) 2008 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "testsuite.h"

STRING_TO_ID string_to_id_table[] =
{
    {"TS_SERVER_EXIT",            TS_SERVER_EXIT},
    {"TS_CLIENT_EXIT",            TS_CLIENT_EXIT},
    {"TS_CMD_FILENAME",           TS_CMD_FILENAME},
    {"TS_MSLEEP",                 TS_MSLEEP},
    {"TS_SLEEP",                  TS_SLEEP},
    {"TS_COUNT",                  TS_COUNT},
    {"TS_COUNT_RESET",            TS_COUNT_RESET},
    {"TS_TIME",                   TS_TIME},
    {"TS_TIME_MARKER",            TS_TIME_MARKER},
    {"TS_TIME_SINCE_MARKER",      TS_TIME_SINCE_MARKER},
    {"TS_LOOP_START",             TS_LOOP_START},
    {"TS_LOOP_END",               TS_LOOP_END},
    {"TS_SYSTEM_INIT",            TS_SYSTEM_INIT},
    {"TS_SYSTEM_DESTROY",         TS_SYSTEM_DESTROY},
    {"TS_PREVIEW_START",          TS_PREVIEW_START},
    {"TS_PREVIEW_STOP",           TS_PREVIEW_STOP},
    {"TS_SNAPSHOT_YUV_PICTURE",   TS_SNAPSHOT_YUV_PICTURE},
    {"TS_SNAPSHOT_JPEG_PICTURE",  TS_SNAPSHOT_JPEG_PICTURE},
    {"TS_SNAPSHOT_RAW_PICTURE",   TS_SNAPSHOT_RAW_PICTURE},
    {"TS_SNAPSHOT_STOP",          TS_SNAPSHOT_STOP},

    {"TS_PRINT_MAXZOOM",          TS_PRINT_MAXZOOM},
    {"TS_PRINT_ZOOMRATIOS",       TS_PRINT_ZOOMRATIOS},
    {"TS_ZOOM_INCREASE",          TS_ZOOM_INCREASE},
    {"TS_ZOOM_DECREASE",          TS_ZOOM_DECREASE},
    {"TS_ZOOM_STEP_INCREASE",     TS_ZOOM_STEP_INCREASE},
    {"TS_ZOOM_STEP_DECREASE",     TS_ZOOM_STEP_DECREASE},
    {"TS_CONTRAST_INCREASE",      TS_CONTRAST_INCREASE},
    {"TS_CONTRAST_DECREASE",      TS_CONTRAST_DECREASE},
    {"TS_SATURATION_INCREASE",    TS_SATURATION_INCREASE},
    {"TS_SATURATION_DECREASE",    TS_SATURATION_DECREASE},
    {"TS_SPECIAL_EFFECT",         TS_SPECIAL_EFFECT},
    {"TS_BRIGHTNESS_INCREASE",    TS_BRIGHTNESS_INCREASE},
    {"TS_BRIGHTNESS_DECREASE",    TS_BRIGHTNESS_DECREASE},
    {"TS_EV_INCREASE",            TS_EV_INCREASE},
    {"TS_EV_DECREASE",            TS_EV_DECREASE},
    {"TS_ANTI_BANDING",           TS_ANTI_BANDING},
    {"TS_SET_WHITE_BALANCE",      TS_SET_WHITE_BALANCE},
    {"TS_AEC_MODE",               TS_AEC_MODE},
    {"TS_ISO_INCREASE",           TS_ISO_INCREASE},
    {"TS_ISO_DECREASE",           TS_ISO_DECREASE},
    {"TS_SHARPNESS_INCREASE",     TS_SHARPNESS_INCREASE},
    {"TS_SHARPNESS_DECREASE",     TS_SHARPNESS_DECREASE},
    {"TS_SET_AUTO_FOCUS",         TS_SET_AUTO_FOCUS},
    {"TS_SET_HJR",                TS_SET_HJR},
    {"TS_SET_LENS_SHADING",       TS_SET_LENS_SHADING},
    {"TS_SET_LED_MODE",           TS_SET_LED_MODE},
    {"TS_GET_SHARPNESS_AF",       TS_GET_SHARPNESS_AF},
    {"TS_SNAPSHOT_RESOLUTION",    TS_SNAPSHOT_RESOLUTION},
    {"TS_PREVIEW_RESOLUTION",     TS_PREVIEW_RESOLUTION},
    {"TS_MOTION_ISO",             TS_MOTION_ISO},
    {"TS_TOGGLE_HUE",             TS_TOGGLE_HUE},
    {"TS_CANCEL_AUTO_FOCUS",      TS_CANCEL_AUTO_FOCUS},
    {"TS_GET_AF_STEP",            TS_GET_AF_STEP},
    {"TS_SET_AF_STEP",            TS_SET_AF_STEP},
    {"TS_ENABLE_AFD",             TS_ENABLE_AFD},
	{"TS_VIDEO_START",            TS_VIDEO_START},
    {"TS_VIDEO_STOP",             TS_VIDEO_STOP},

    {"TEST_VIDIOC_QUERYMENU",       TEST_VIDIOC_QUERYMENU},
    {"TEST_VIDIOC_QUERYCTRL",       TEST_VIDIOC_QUERYCTRL},
    {"TEST_VIDIOC_S_CTRL",          TEST_VIDIOC_S_CTRL},
    {"TEST_VIDIOC_G_CTRL",          TEST_VIDIOC_G_CTRL},
    {"TEST_VIDIOC_CROPCAP",         TEST_VIDIOC_CROPCAP},
    {"TEST_VIDIOC_G_CROP",          TEST_VIDIOC_G_CROP},
    {"TEST_VIDIOC_S_CROP",          TEST_VIDIOC_S_CROP},
    {"TEST_VIDIOC_G_FMT",           TEST_VIDIOC_G_FMT},
    {"TEST_VIDIOC_S_FMT",           TEST_VIDIOC_S_FMT},
    {"TEST_V4L2_CID_FOCUS_ABSOLUTE", TEST_V4L2_CID_FOCUS_ABSOLUTE},
    {"TEST_V4L2_CID_FOCUS_RELATIVE", TEST_V4L2_CID_FOCUS_RELATIVE},
    {"TEST_V4L2_CID_FOCUS_AUTO",    TEST_V4L2_CID_FOCUS_AUTO},
    {"TEST_V4L2_CID_CONTRAST",      TEST_V4L2_CID_CONTRAST},
    {"TEST_V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_SPATIAL_FILTER_TYPE", TEST_V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_SPATIAL_FILTER_TYPE},
    {"TEST_V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_BOTTOM", TEST_V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_BOTTOM},
    {"TEST_V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_TOP", TEST_V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_TOP},
    {"TEST_V4L2_CID_BRIGHTNESS",    TEST_V4L2_CID_BRIGHTNESS},
    {"TEST_V4L2_CID_EXPOSURE",      TEST_V4L2_CID_EXPOSURE},
    {"TEST_V4L2_CID_EXPOSURE_AUTO", TEST_V4L2_CID_EXPOSURE_AUTO},
    {"TEST_V4L2_CID_EXPOSURE_ABSOLUTE", TEST_V4L2_CID_EXPOSURE_ABSOLUTE},
    {"TEST_V4L2_CID_EXPOSURE_AUTO_PRIORITY", TEST_V4L2_CID_EXPOSURE_AUTO_PRIORITY},
    {"TEST_V4L2_CID_SHARPNESS",     TEST_V4L2_CID_SHARPNESS},
    {"TEST_V4L2_CID_HUE_AUTO",      TEST_V4L2_CID_HUE_AUTO},
    {"TEST_V4L2_CID_SATURATION",    TEST_V4L2_CID_SATURATION},
    {"TEST_V4L2_CID_AUTO_WHITE_BALANCE_AUTO", TEST_V4L2_CID_AUTO_WHITE_BALANCE_AUTO},
    {"TEST_V4L2_CID_DO_WHITE_BALANCE", TEST_V4L2_CID_DO_WHITE_BALANCE},
    {"TEST_V4L2_CID_WHITE_BALANCE_TEMPERATURE", TEST_V4L2_CID_WHITE_BALANCE_TEMPERATURE},
    {"TEST_VIDIOC_ENUM_FRAMEINTERVALS", TEST_VIDIOC_ENUM_FRAMEINTERVALS},
    {"TEST_VIDIOC_STREAMON",        TEST_VIDIOC_STREAMON},
    {"TEST_VIDIOC_STREAMOFF",       TEST_VIDIOC_STREAMOFF},
    {"TEST_VIDIOC_ENCODER_CMD",     TEST_VIDIOC_ENCODER_CMD},
    {"TEST_VIDIOC_TRY_ENCODER_CMD", TEST_VIDIOC_TRY_ENCODER_CMD},
    {"TEST_VIDIOC_G_PARAM",         TEST_VIDIOC_G_PARAM},
    {"TEST_VIDIOC_S_PARAM",         TEST_VIDIOC_S_PARAM},
    {"TEST_V4L2_CID_START_SNAPSHOT", TEST_V4L2_CID_START_SNAPSHOT},
    {"TEST_V4L2_CID_STOP_SNAPSHOT", TEST_V4L2_CID_STOP_SNAPSHOT},

    /* From here down, are V4L2 defined commands */
    {"V4L2_CID_BRIGHTNESS",         V4L2_CID_BRIGHTNESS},
    {"V4L2_CID_CONTRAST",           V4L2_CID_CONTRAST},
    {"V4L2_CID_SATURATION",         V4L2_CID_SATURATION},
    {"V4L2_CID_HUE",                V4L2_CID_HUE},
    {"V4L2_CID_AUDIO_VOLUME",       V4L2_CID_AUDIO_VOLUME},
    {"V4L2_CID_AUDIO_BALANCE",      V4L2_CID_AUDIO_BALANCE},
    {"V4L2_CID_AUDIO_BASS",         V4L2_CID_AUDIO_BASS},
    {"V4L2_CID_AUDIO_TREBLE",       V4L2_CID_AUDIO_TREBLE},
    {"V4L2_CID_AUDIO_MUTE",         V4L2_CID_AUDIO_MUTE},
    {"V4L2_CID_AUDIO_LOUDNESS",     V4L2_CID_AUDIO_LOUDNESS},
    {"V4L2_CID_BLACK_LEVEL",        V4L2_CID_BLACK_LEVEL},
    {"V4L2_CID_AUTO_WHITE_BALANCE", V4L2_CID_AUTO_WHITE_BALANCE},
    {"V4L2_CID_DO_WHITE_BALANCE",   V4L2_CID_DO_WHITE_BALANCE},
    {"V4L2_CID_RED_BALANCE",        V4L2_CID_RED_BALANCE},
    {"V4L2_CID_BLUE_BALANCE",       V4L2_CID_BLUE_BALANCE},
    {"V4L2_CID_GAMMA",              V4L2_CID_GAMMA},
    {"V4L2_CID_WHITENESS",          V4L2_CID_WHITENESS},
    {"V4L2_CID_EXPOSURE",           V4L2_CID_EXPOSURE},
    {"V4L2_CID_AUTOGAIN",           V4L2_CID_AUTOGAIN},
    {"V4L2_CID_GAIN",               V4L2_CID_GAIN},
    {"V4L2_CID_HFLIP",              V4L2_CID_HFLIP},
    {"V4L2_CID_VFLIP",              V4L2_CID_VFLIP},
    {"V4L2_CID_HCENTER",            V4L2_CID_HCENTER},
    {"V4L2_CID_VCENTER",            V4L2_CID_VCENTER},
    {"V4L2_CID_LASTP1",             V4L2_CID_LASTP1},
    {"TS_CMD_ID_INVALID",           TS_CMD_ID_INVALID},
};

int string_to_id( char * str)
{
    int i;

    for ( i = 0; i < (int) (sizeof(string_to_id_table)/sizeof(STRING_TO_ID)); i++ )
    {
        if (strcmp(str, string_to_id_table[i].string) == 0) {
            return string_to_id_table[i].id;
        }
    }

    return -1;
}

void print_help(void)
{
    int i;

    printf("Here are all test commands:\n");
    printf("--------------------------------\n");

    for ( i = 0; i < (int) (sizeof(string_to_id_table)/sizeof(STRING_TO_ID)); i++ )
    {
        printf(" %2d: %s\n", string_to_id_table[i].id, string_to_id_table[i].string);
//        if (string_to_id_table[i].id == TS_CMD_ID_INVALID) break;
    }
    printf("--------------------------------\n");
}

int read_a_line(char * test_command_filename, char * test_command_buf)
{
    int len;

    static FILE * fhandle = NULL;

    if (test_command_filename == NULL) {
        rewind(fhandle);
        return 0;
    }

    if (fhandle == NULL) {
        fhandle = fopen(test_command_filename, "r");
        if (fhandle == NULL) {
            perror("failed to open test script file");
            exit(-1);
        }
    }

    if ( fgets( test_command_buf, TS_CMD_STR_MAX + 1, fhandle) == NULL) {
        // end of file
        fclose(fhandle);
        fhandle = NULL;
        return -1;
    }

    len = strlen(test_command_buf);
    for (; len >= 0; len --)
    {
        if (test_command_buf[len-1] == '\n' || test_command_buf[len-1] == '\r') {
            test_command_buf[len-1] = '\0';
        }
        else {
            break;
        }
    }

    return 0;
}

int parse_command( char * command_str, char ** p_command_args, char ** p_command_result)
{
    int command_id;
    char * command_args;
    char * command_result;
    char * prev_command;

//    printf("command_str is: {%s}\n", command_str);

    strtok_r(command_str, "()\n\r", &prev_command);
    command_args = strtok_r(NULL, "()\n\r", &prev_command);
    command_result = strtok_r(NULL, "()\n\r", &prev_command);

    command_id = string_to_id( command_str);
    if ( p_command_result != NULL ) * p_command_result = command_result;
    if ( p_command_args != NULL ) * p_command_args = command_args;

//    printf("command_id = %d, ", command_id);
//    printf("p_command_args = {%s}, ", command_args);
//    printf("p_command_result = {%s}\n", command_result);

    return command_id;
}

int testsuite_send(int socket_inet, const char * text)
{
    int length = strlen (text) + 1;
    if (write (socket_inet, &length, sizeof (length)) == 0) {
//        printf("%s: fail on the connection.\n", __func__);
        return -1;
    }
    /* Write the string. */
    if (write (socket_inet, text, length) == 0) {
//        printf("%s: fail on the connection.\n", __func__);
        return -1;
    }
    return length;
}

int testsuite_receive(int socket_inet, char ** text)
{
    int length;
    if (read (socket_inet, &length, sizeof (length)) == 0) {
//        printf("%s: fail on the connection.\n", __func__);
        return -1;
    }
    * text = (char *) malloc (length);
    if (read (socket_inet, * text, length) == 0) {
//        printf("%s: fail on the connection.\n", __func__);
        free(* text);
        * text = NULL;
        return -1;
    }
    return length;
}
