/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MMI_GRAPHICS_H__
#define __MMI_GRAPHICS_H__

#define DEFAULT_FONT_PATH "/system/etc/mmi/fonts.ttf"
/**One line in screen, */
#define DEFAULT_UNICODE_STR_LEN 256
typedef void (*gr_text_func) (int x, int y, const char *s, int bold);

void gr_deinit_multi(void);
int gr_init_multi(void);
void gr_text_multi(int x, int y, const char *s, int bold);
void set_font_size(const char *font_size);
void gr_font_size_multi(int *x, int *y);

#endif
