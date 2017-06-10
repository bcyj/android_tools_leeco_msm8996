/******************************************************************************
  @file:  loc_gui_test.c
  @brief:

  DESCRIPTION

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2007 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      -------------------------------------------------------
03/17/09   dx       Android version
01/16/09   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_gui.c#4 $
======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

/* GUI */
#include <linux/fb.h>

#include "loc_api_rpc_glue.h"
#include "loc_api_test.h"
#include "loc_api_gui.h"
#include "loc_api_data.h"

/*=============================================================================
 *
 *                          GLOBAL DATA DECLARATION
 *
 *============================================================================*/

#include "gps_one_logo.inc"

int loc_fb_fd = -1;
int loc_fb_size = 0;

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

static unsigned char *loc_fb_mmap = NULL;
static unsigned char *fb_mmap_back = NULL;

loc_sky_s_type sky;

int loc_sky_thread_init = 0;
pthread_t loc_sky_thread;
sem_t loc_sky_sem;

/*=============================================================================
 *
 *                      FRAMEBUFFER SUPPORTING FUNCTIONS
 *
 *============================================================================*/

static int fb_info(void) {
   if (ioctl(loc_fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
   {
      loc_write_error("LCD: can't retrieve vscreenInfo!\n");
      return -1;
   }

   if (ioctl(loc_fb_fd, FBIOGET_FSCREENINFO, &finfo) < 0)
   {
      loc_write_error("LCD: can't retrieve fscreenInfo!\n");
      return -1;
   }
   
   return 0;
}

/* Returns pixel index or -1 if out of range */
static int fb_pix_index(int x, int y)
{  
   /* Check range */
   if (x > (signed) vinfo.xres || y > (signed) vinfo.yres || x < 0 || y < 0)
   {
      return -1;
   }

   int pixel_index = (y * vinfo.xres) + x;  
      
   return pixel_index;
}

/* Returns 1 if successful, 0 if failed */
static void fb_plot(int x, int y, int pixel)
{
   int pixel_index = fb_pix_index(x, y);   
   if (pixel_index >= 0)
   {
      /* 16-bit pixels are assumed */
      ((uint16*) loc_fb_mmap)[pixel_index] = (uint16) pixel;
   }
}

/* Plot vertical line */
static void fb_vline(int x, int y1, int y2, int pixel)
{   
   int tmp; 
   if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }
   while (y1 <= y2) 
   {
      fb_plot(x, y1, pixel);
      y1++;
   }
}

/* Plot horizontal line */
static void fb_hline(int y, int x1, int x2, int pixel)
{
   int tmp; 
   if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
   while (x1 <= x2) 
   {
      fb_plot(x1, y, pixel);
      x1++;
   }
}

/* Plot circle */
static void fb_plot_circle(int x0, int y0, int radius, int pixel)
{
   int x, r2 = radius * radius;
   int prev_y = 0;
   for (x = - radius; x <= radius; x++)
   {
      int y = sqrt(r2 - x*x);
      fb_vline(x0 + x, y0 - y, y0 - prev_y, pixel);
      fb_vline(x0 + x, y0 + y, y0 + prev_y, pixel);     
      prev_y = y;
   }
}

/* Plots random dots */
static void fb_random_dots(int num_dots)
{
   static int pixel = 0xffff;
   srand(time(NULL));

   int i;
   for (i = 0; i < num_dots; i++)
   {
      int x = rand() % (vinfo.xres / 2);
      int y = rand() % (vinfo.yres / 2);
      fb_plot(x, y, pixel);
   }
}

/* Prepares a memory-based bitmap */
static void fb_make_bmp(
      unsigned char *bmp_pixel,           /* destination of the bmp */
      int w,                              /* width */
      int h,                              /* height */
      const unsigned char* bmp_index,     /* source indexed bmp */
      const unsigned short* palette)      /* source palette */
{
   int iPix = 0;
   int i, j;
   for (i = 0; i < h; i++)
   {
     for (j = 0; j < w; j++)
     {
       /* 16-bit pixels are assumed */
       int pixel = palette[bmp_index[iPix]];
       ((uint16*) bmp_pixel)[iPix] = (uint16) pixel;
       iPix++;
     }
   }
}

/* Paint BMP into the buffer */
static void fb_paint_bmp(int dst_x, int dst_y, const unsigned char* srcp, int w, int h)
{
  unsigned char *dstp = loc_fb_mmap;  
  
  /* XXX 16-bit pixels are assumed */
  dstp += (dst_y * vinfo.xres + dst_x) * sizeof(uint16);
  
  int i;
  for (i = 0; i < h; i++)
  {
    memcpy(dstp, srcp, w * sizeof(uint16));
    dstp += vinfo.xres * sizeof(uint16);
    srcp += w * sizeof(uint16);
  }
}

/*===========================================================================
FUNCTION loc_init_gui

DESCRIPTION
   Initialize the LCD

RETURN VALUE
   Returns 0 if successful
     
===========================================================================*/
int loc_gui_open(void)
{   
   int rc = 0; /* return code */
   
   if (sys.lcd_started)
   {
      return rc;  /* don't need to re-init */
   }
   
   /* Open device */
   loc_fb_fd = open(LOC_FB_DEV, O_RDWR);
   if (loc_fb_fd < 0)
   {
      loc_write_error("LCD: can't open framebuffer %s file node!\n", LOC_FB_DEV);    
      rc = -1;
      goto loc_fb_exit_init;
   }

   /* Get framebuffer info */
   if ((rc = fb_info()) != 0)
   {
      goto loc_fb_exit_init;
   }

   /* Check resolution */
   if ((LOC_LB_WIDTH > vinfo.xres) || (LOC_LB_HEIGHT > vinfo.yres))
   {
      loc_write_error("LCD: display resolution too small!\n");
      rc = -1;
      goto loc_fb_exit_init;
   }

   /* Check pixel bits */
   /* XXX Check pixel format, should be 565 */
   if ( vinfo.bits_per_pixel != 16)
   {
     loc_write_error("LCD: this version only supports 16-bit pixels.\n");
     rc = -1;
     goto loc_fb_exit_init;
   }

   /* Init memory */
   vinfo.activate = FB_ACTIVATE_VBL;
   loc_fb_size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

   loc_fb_mmap = fb_mmap_back = mmap (NULL,
         finfo.smem_len,
         PROT_READ|PROT_WRITE,
         MAP_SHARED,
         loc_fb_fd,
         0);
   if (loc_fb_mmap == MAP_FAILED)
   {
      loc_write_error("LCD: map failed!\n");
      rc = -1;
      goto loc_fb_exit_init;  
   }
  
   /* Clear graphics */
   memset(loc_fb_mmap, 0, finfo.smem_len);
   
   if ((signed) finfo.smem_len < (2*loc_fb_size))
   {
      loc_write_error("LCD: no back buffer!\n");
      rc = -1;
      goto loc_fb_exit_init;  
   }
   else {
      fb_mmap_back += loc_fb_size;      
   }
   
   /* label */ loc_fb_exit_init:        
   if (rc != 0) /* release if failed */
   {
      sys.lcd_started = 0;
      if (loc_fb_fd >= 0) { close(loc_fb_fd); }
   }
   else {
      loc_write_msg("LCD screen enabled: %dx%d (%d bits)\n", 
            vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
      
      sys.lcd_started = 1;
   }
   
   return rc;
}

/*===========================================================================
FUNCTION loc_plot_refresh

DESCRIPTION
   Transfers the buffer to LCD

DEPENDENCY
   loc_fb_init
===========================================================================*/
static int loc_plot_refresh(void)
{
   assert(sys.lcd_started);
   if (ioctl(loc_fb_fd, FBIOPAN_DISPLAY, &vinfo) < 0)
   {
      loc_write_error("LCD DISPLAY failed! (%s:%d)\n", __FILE__, __LINE__);
      return 0; /* error */
   }
   
   return 1;
}

/*===========================================================================
FUNCTION loc_plot_sky_circle

DESCRIPTION
   Plots sky on the LCD

DEPENDENCY
   loc_fb_init must be called first   
===========================================================================*/
static void loc_plot_sky_circle(void)
{
   assert(sys.lcd_started);
   
   /* Draw a circle */
   sky.x0 = vinfo.xres / 2;
   sky.y0 = vinfo.yres / 2;
   sky.r = - 20 + (sky.x0 < sky.y0 ? sky.x0 : sky.y0);
   
   fb_plot_circle(sky.x0, sky.y0, sky.r, 0xFF /* blue */);
}

/*===========================================================================
FUNCTION loc_plot_sv

DESCRIPTION
   Plots a SV on the sky

DEPENDENCY
   loc_plot_sky_circle must be called first   
   
===========================================================================*/
static void loc_plot_sv(
      float azimuth,        /* 0 - 360 degrees */ 
      float elevation       /* 0 - 90  degrees */
)
{
   assert(sys.lcd_started);

   int sv_r = sky.r * cos(elevation * 3.1416 / 180);  /* radius of sv's circle */
   int azi_grad = azimuth * 3.1416 / 180;
   int x = sv_r * cos(azi_grad);
   int y = - sv_r * sin(azi_grad);
   
   int pixel = 0xFFFFFF;
   int sv_size = 10;
   
   fb_hline(sky.y0 + y, sky.x0 + x - sv_size, sky.x0 + x + sv_size, pixel);
   fb_vline(sky.x0 + x, sky.y0 + y - sv_size, sky.y0 + y + sv_size, pixel);
}

/*===========================================================================
FUNCTION loc_plot_clear

DESCRIPTION
   Clears the buffer.

DEPENDENCY
   loc_fb_init      
===========================================================================*/
static void loc_plot_clear(void)
{
   assert(sys.lcd_started);
   
   memset(loc_fb_mmap, 0, loc_fb_size);
}

/*===========================================================================
FUNCTION loc_plot_logo

DESCRIPTION
   Plots the gpsOne logo at bottom-left corner

DEPENDENCY
   loc_fb_init      
===========================================================================*/
static void loc_plot_logo(void)
{
   assert(sys.lcd_started);
   
   int img_size = gpsLogoWidth * gpsLogoHeight; /* in pixels */
   short *logo_bmp = (short*) malloc(img_size * 2);
   if (!logo_bmp) { return; } /* out of memory */
   fb_make_bmp((unsigned char*) logo_bmp, gpsLogoWidth, gpsLogoHeight, (unsigned char*) gpsLogo, gpsLogoPalette);
   fb_paint_bmp(0, vinfo.yres - gpsLogoHeight - 5, (unsigned char*) logo_bmp, gpsLogoWidth, gpsLogoHeight);
   if (logo_bmp) { free(logo_bmp); }
}

/*===========================================================================
FUNCTION loc_plot_clear

DESCRIPTION
   Clears the buffer.

DEPENDENCY
   loc_fb_init
===========================================================================*/
static void loc_plot_sky_view(void)
{
   assert(sys.lcd_started);

   loc_plot_clear();
   loc_plot_logo();
   loc_plot_sky_circle();
   int i;
   for (i = 0; i < sky.gnss.sv_count; i++)
   {
      if ((sky.gnss.sv_list.sv_list_val[i].valid_mask & RPC_LOC_SV_INFO_VALID_AZIMUTH) &&
          (sky.gnss.sv_list.sv_list_val[i].valid_mask & RPC_LOC_SV_INFO_VALID_ELEVATION) )
      {
         float azi  = sky.gnss.sv_list.sv_list_val[i].azimuth;
         float elev = sky.gnss.sv_list.sv_list_val[i].elevation;
         loc_plot_sv(azi, elev);
      }
   }
   loc_plot_refresh();
}

/*===========================================================================
FUNCTION loc_sky_thread_proc

DESCRIPTION
   Sky plot working thread

DEPENDENCY
   loc_fb_init
===========================================================================*/
static void* loc_sky_thread_proc(void *arg)
{
   while (1) 
   {
      sem_wait(&loc_sky_sem);
      loc_plot_sky_view();
   }
   return NULL;
}

/*===========================================================================
FUNCTION loc_enable_sky_plot

DESCRIPTION
   Turns on/off sky plot
   
RETURN VALUE
   0   if successful
  !0   if failed
===========================================================================*/
int loc_enable_sky_plot(int enable)
{
   int rc = 0;

   if (enable) 
   {
      sys.sky_plot = 0;

      /* Initialize the first time */
      if (loc_gui_open())
      {
         return -1;
      }
      
      loc_plot_clear();
      loc_plot_logo();
      loc_plot_refresh();      /* draw to LCD */
      
      /* init thread */
      if (!loc_sky_thread_init)
      {
         if (sem_init(&loc_sky_sem, 0, 0) == -1) 
         {
            loc_write_error("GUI: cannot start sky view semaphore.\n");
            return -1;
         }

         if (pthread_create(&loc_sky_thread, NULL, loc_sky_thread_proc, NULL) != 0)
         {            
            loc_write_error("GUI: cannot start sky view thread.\n");
            return -1;
         }
         
         loc_sky_thread_init = 1;
      }
      
      sys.sky_plot = 1;
   }
   else {
      sys.sky_plot = 0;
      loc_plot_clear();
      rc = !loc_plot_refresh();
   }

   return rc;
}

/*===========================================================================
FUNCTION loc_update_sky_plot

DESCRIPTION
   Updates the LCD plot of the sky
   
RETURN VALUE
   0   if successful
  !0   if failed
===========================================================================*/
int loc_update_sky_plot(const rpc_loc_gnss_info_s_type *gnss)
{   
   if (!sys.lcd_started || !sys.sky_plot || !loc_sky_thread_init)
   {
     return -1;
   }

   if (gnss->valid_mask & RPC_LOC_GNSS_INFO_VALID_SV_LIST)
   {
      memcpy(&sky.gnss, gnss, sizeof sky.gnss);
      sem_post(&loc_sky_sem); /* simply wakes up the working thread */
   }
   return 0;
}

/*===========================================================================
FUNCTION loc_update_sky_plot

DESCRIPTION
   Closes all resources used by GUI
===========================================================================*/
void loc_gui_close()
{   
   if (sys.lcd_started)
   {
      loc_plot_clear();
      loc_plot_refresh();         

      if (loc_fb_fd >= 0)
      {
         close(loc_fb_fd);
         loc_fb_fd = -1;
      }
      
      sys.lcd_started = 0;
   }
}

/*===========================================================================
FUNCTION loc_gui_test

DESCRIPTION
   Simple GUI test
   
RETURN VALUE
   0   if successful
   1   if failed
===========================================================================*/
int loc_gui_test()
{
   /* Initialize the first time */
   if (loc_gui_open() != 0)
   {
      return -1;
   }
   
   /* Test random dots */
   fb_random_dots(2000);    /* 0 dot, just keeps the function called */
   
   loc_plot_sky_circle();
   
   return !loc_plot_refresh(); /* draw to LCD */
}
