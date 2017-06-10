/******************************************************************************
  @file:  loc_gui_test.h
  @brief:

  DESCRIPTION

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2007 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

#ifndef LOC_API_GUI_H
#define LOC_API_GUI_H

#define LOC_LB_WIDTH                       100
#define LOC_LB_HEIGHT                      100

#define LOC_FB_DEV                         "/dev/graphics/fb0"

/*=============================================================================
 *
 *                        MODULE TYPE DECLARATION
 *
 *============================================================================*/

typedef struct 
{
   int x0, y0;
   int r;
   rpc_loc_gnss_info_s_type gnss;
} loc_sky_s_type;

/*=============================================================================
 *
 *                          MODULE EXTERNAL DATA
 *
 *============================================================================*/
extern int loc_fb_fd;

/*=============================================================================
 *
 *                       MODULE EXPORTED FUNCTIONS
 *
 *============================================================================*/
extern int loc_gui_open(void);
extern void loc_gui_close(void);
extern int loc_gui_test(void);

extern int loc_enable_sky_plot(int enable);
extern int loc_update_sky_plot(const rpc_loc_gnss_info_s_type *gnss);

#endif /* LOC_API_INI_H */
