/******************************************************************************
  @file  fbtestUtils.c
  @brief This file contains test code to verify all functionalities of msm_fb

  DESCRIPTION
  fbtest is msm framebuffer test program.  It opens all frambuffers (/dev/fb*)
  and executes the msm specific fb ioctls as well as the standard linux fb
  ioctls.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

 -----------------------------------------------------------------------------
 Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential
 -----------------------------------------------------------------------------

******************************************************************************/
#include "fbtest.h"
#include "fbtestUtils.h"
#ifdef MDP3_FLAG
#include "mdp3.h"
#else
#ifdef MDP4_FLAG
#include "mdp4.h"
#else /* Default Case */
int getMaxUpScale(int mdp_version) {
	return 1;
}

int getMaxDownScale(int mdp_version) {
	return 1;
}
#endif
#endif
#include <media/msm_media_info.h>
extern int fbtest_msg_debug;	/* set non-zero for verbose debugging messages */
extern struct lookup_string_t testNameLUT[MAX_TESTS];
struct inputFileParams ImgFileParams[4];
unsigned int istage;

#ifndef USE_ION
struct memDev * PMEM = &(pmem[PMEM_NUM_DEFAULT]);
#endif
const char *errorLUT[] = {
	"No Error",
	"Error: FBDEV not found",
	"Error: PMEMDEV not found",
	"Error: FBDEV open failed",
	"Error: PMEMDEV open failed",
	"Error: IONDEV open failed",
	"Error: Could not get VSCREEN information",
	"Error: Could not get FSCREEN information",
	"Error: MMAP failed",
	"Error: Invalid input color format in filename",
	"Error: Please Check directory or file path",
	"Error: The filename is not according to naming convention",
	"Error: Unable to open the Directory",
	"Error: Unable to close the Directory",
	"Error: Input Parameter not supported",
	"Error: Could not open input file",
	"Error: Input file not found (NULL)",
	"Error: Wrong video file size calculation",
	"Error: PMEM Allocation failed",
	"Error: Malloc failed",
	"Error: System Command execution failed",
	"Error: Resolution (wxh) out of bound",
	"Error: fbiopan display failed",
	"Error: overlay set ioctl failed",
	"Error: overlay play ioctl failed",
	"Error: overlay unset ioctl failed",
	"Error: Invalid crop co-ordinates. Please check crop parameter values",
	"Error: At least one input file format should match framebuffer format",
	"Error: Could not open Rotator",
	"Error: Could not start Rotator",
	"Error: Could not rotate",
	"Error: Could not finish Rotator",
	"Error: Could not Rotate for 0 or 180 degree:WxH out of Screen Resolution",
	"Error: Could not Rotate for 90 or 270 degree:WxH out of Screen Resolution",
	"Error: Offset (startX, startY or endX, endY) out of bound",
	"Error: Invalid Offset (startX, startY or endX, endY)",
	"Error: Image Format not supported for BG, Drawing Default BG",
	"Error: No free YUV pipe, Ignoring extra YUV input file.",
	"Error: File Read Fail",
	"Error: FB cursor ioctl failed",
	"Error: display commit ioctl failed",
	"Error: blit ioctl failed",
	"Error: Invalid Overlay ID",
	"Error: CRC Entry not found in DB file",
	"Error: CRC Entry not matching with DB file",
	"Error: CRC DB File not found",
	"Info: CRC Entry found in DB file",
	"Info: Add New CRC Entry in DB file",

};

struct lookup_string_t format_name_table[] =  {
	{ "MDP_RGB_565",	MDP_RGB_565, 1 },		/* RGB 565 planer */
	{ "MDP_XRGB_8888",	MDP_XRGB_8888, 1 },	/* RGB 888 padded */
	{ "MDP_Y_CBCR_H2V2",	MDP_Y_CBCR_H2V2, 0 },	/* Y and CbCr, pseudo planer w/ Cb is in MSB */
	{ "MDP_Y_CBCR_H2V2_ADRENO",MDP_Y_CBCR_H2V2_ADRENO,0},
	{ "MDP_ARGB_8888",	MDP_ARGB_8888, 1 },	/* ARGB 888 */
	{ "MDP_RGB_888",	MDP_RGB_888, 1 },		/* RGB 888 planer */
	{ "MDP_Y_CRCB_H2V2",	MDP_Y_CRCB_H2V2, 0 },	/* Y and CrCb, pseudo planer w/ Cr is in MSB */
	{ "MDP_YCRYCB_H2V1",	MDP_YCRYCB_H2V1, 0 },	/* YCrYCb interleave */
	{ "MDP_YCBYCR_H2V1",	MDP_YCBYCR_H2V1, 0 },	/* YCbYCr interleave */
	{ "MDP_CBYCRY_H2V1",MDP_CBYCRY_H2V1,0},  /* CbYCrY interleave */
	{ "MDP_Y_CRCB_H2V1",	MDP_Y_CRCB_H2V1, 0 },	/* Y and CrCb, pseduo planer w/ Cr is in MSB */
	{ "MDP_Y_CBCR_H2V1",	MDP_Y_CBCR_H2V1, 0 },	/* Y and CrCb, pseduo planer w/ Cr is in MSB */
	{ "MDP_Y_CRCB_H1V2",MDP_Y_CRCB_H1V2,0},
	{ "MDP_Y_CBCR_H1V2",MDP_Y_CBCR_H1V2,0},
	{ "MDP_RGBA_8888",	MDP_RGBA_8888, 1 },	/* ARGB 888 */
	{ "MDP_BGRA_8888",	MDP_BGRA_8888, 1 },	/* ABGR 888 */
	{ "MDP_RGBX_8888",	MDP_RGBX_8888, 1 },	/* RGBX 888 */
	{ "MDP_Y_CRCB_H2V2_TILE",MDP_Y_CRCB_H2V2_TILE, 0 },/* Y and CrCb, pseudo planer tile */
	{ "MDP_Y_CBCR_H2V2_TILE",MDP_Y_CBCR_H2V2_TILE, 0 },/* Y and CbCr, pseudo planer tile */
	{ "MDP_Y_CR_CB_H2V2",MDP_Y_CR_CB_H2V2, 0},  /* Y, Cr and Cb, planar */
	{ "MDP_Y_CR_CB_GH2V2",MDP_Y_CR_CB_GH2V2,0},  /* Y, Cr and Cb, planar aligned to Android YV12 */
	{ "MDP_Y_CB_CR_H2V2",MDP_Y_CB_CR_H2V2, 0},  /* Y, Cb and Cr, planar */
	{ "MDP_Y_CRCB_H1V1",MDP_Y_CRCB_H1V1, 0},  /* Y and CrCb, pseduo planer w/ Cr is in MSB */
	{ "MDP_Y_CBCR_H1V1",MDP_Y_CBCR_H1V1, 0},  /* Y and CbCr, pseduo planer w/ Cb is in MSB */
	{ "MDP_YCRCB_H1V1",MDP_YCRCB_H1V1,0},   /* YCrCb interleave */
	{ "MDP_YCBCR_H1V1",MDP_YCBCR_H1V1,0},  /* YCbCr interleave */
	{ "MDP_BGR_565",	MDP_BGR_565, 1 },		/* BGR 565 planer */
	{ "MDP_BGR_888",MDP_BGR_888,1},      /* BGR 888 */
	{ "MDP_Y_CBCR_H2V2_VENUS",MDP_Y_CBCR_H2V2_VENUS,0},
	{ "MDP_BGRX_8888",MDP_BGRX_8888,1},  /* BGRX 8888 */
	{ "MDP_IMGTYPE_LIMIT",MDP_IMGTYPE_LIMIT,1},
	{ "MDP_RGB_BORDERFILL",MDP_RGB_BORDERFILL,1},	/* border fill pipe */
	{ "MDP_FB_FORMAT",	MDP_FB_FORMAT, 0 },	/* framebuffer format */
	{ "MDP_RGBA_8888_TILE", MDP_RGBA_8888_TILE, 1},	/* GPU Macro tile format */
	{ "MDP_ARGB_8888_TILE", MDP_ARGB_8888_TILE, 1},	/* GPU Macro tile format */
	{ "MDP_BGRA_8888_TILE", MDP_BGRA_8888_TILE, 1},	/* GPU Macro tile format */
	{ "MDP_ABGR_8888_TILE", MDP_ABGR_8888_TILE, 1},	/* GPU Macro tile format */
	{ "MDP_RGBX_8888_TILE", MDP_RGBX_8888_TILE, 1},	/* GPU Macro tile format */
	{ "MDP_XRGB_8888_TILE", MDP_XRGB_8888_TILE, 1},	/* GPU Macro tile format */
	{ "MDP_BGRX_8888_TILE", MDP_BGRX_8888_TILE, 1},	/* GPU Macro tile format */
	{ "MDP_XBGR_8888_TILE", MDP_XBGR_8888_TILE, 1},	/* GPU Macro tile format */
	{ "MDP_IMGTYPE_LIMIT2",	MDP_IMGTYPE_LIMIT2, 0},	/* Non valid image type after this enum */
	{ "MDP_ARGB_1555", MDP_ARGB_1555},	/* ARGB 1555 */
	{ "MDP_RGBA_5551", MDP_RGBA_5551},	/* RGBA 5551 */
	{ "MDP_ARGB_4444", MDP_ARGB_4444},	/* ARGB 4444 */
	{ "MDP_RGBA_4444", MDP_RGBA_4444},	/* RGBA 4444 */
	{ NULL, 		-1, -1},
};

int allocMEM(unsigned int size)
{
	int result = -ENOMEM;

	if (MEM == NULL)
		return result;
#ifdef USE_ION
        struct ion_fd_data fd_data;
        struct ion_allocation_data ionAllocData;
        fd_data.fd = 0;

        if ((unsigned int)MEM->mem_size >= size && MEM->mem_buf) {
                memset(MEM->mem_buf, 0x00, MEM->mem_size);
                FBTEST_MSG_DEBUG("MEM reuse successful (%d bytes at %p)\n", MEM->mem_size, MEM->mem_buf);
                return 0;
        }

        // If unable to reuse current MEM, unmap and close the current MEM
        // and create a clean one
        if(MEM->mem_size > 0){
                FBTEST_MSG_DEBUG("MEM->mem_size>0\n");
                munmap(MEM->mem_buf, MEM->mem_size);
		ioctl(MEM->fd, ION_IOC_FREE, &handle_data);
		close(MEM->mem_fd);
                close(MEM->fd);

                result = chooseMEMDev();
                if(result < 0)
                        showError(result, 1);

                result = openMEMDev();
                if(result < 0)
                        showError(result, 1);
        }

        // First Allocate MEM
        MEM->mem_page_size = sysconf(_SC_PAGESIZE);
        MEM->mem_size = size;
        MEM->mem_size = (MEM->mem_size + MEM->mem_page_size - 1) & (~(MEM->mem_page_size - 1));
        ionAllocData.len = MEM->mem_size;
        ionAllocData.align = sysconf(_SC_PAGESIZE);
        ionAllocData.heap_id_mask =
                ION_HEAP(ION_IOMMU_HEAP_ID) |
                ION_HEAP(ION_CP_MM_HEAP_ID) |
                ION_HEAP(ION_CP_WB_HEAP_ID) |
                ION_HEAP(ION_SF_HEAP_ID);
        ionAllocData.flags = 0;

        result = ioctl(MEM->fd, ION_IOC_ALLOC,  &ionAllocData);
        if(result){

                FBTEST_MSG_DEBUG("ERROR! MEM_ALLOCATE failed.\n");
                close_devices();        //UTF: added for cleanup code addition.
                // Close MEM Dev <----------------------------------------------
                return -MALLOC_FAILED;

        } else {
                fd_data.handle = ionAllocData.handle;
                handle_data.handle = ionAllocData.handle;

                if(ioctl(MEM->fd, ION_IOC_MAP, &fd_data)){
                        FBTEST_MSG_DEBUG("ERROR! ION_IOC_MAP failed.\n");
                        close_devices();
                }else
                {
                        MEM->mem_buf = mmap(NULL, MEM->mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_data.fd, 0);
                        MEM->mem_fd = fd_data.fd;
                        if (MEM->mem_buf == MAP_FAILED) {
                                FBTEST_MSG_DEBUG("ERROR: MEM MMAP failed!\n");
                                close_devices();        //UTF: added for cleanup code addition.
                                // Deallocate <------------------------------------------
                                return -MMAP_FAILED;
                        }

                        memset(MEM->mem_buf, 0x00, MEM->mem_size);
                        FBTEST_MSG_DEBUG("MEM Allocation successful (%d bytes at %p)\n", MEM->mem_size, MEM->mem_buf);
		}
	}
#else
        if ((unsigned int)MEM->mem_size >= size && MEM->mem_buf) {
                memset(MEM->mem_buf, 0x00, MEM->mem_size);
                FBTEST_MSG_DEBUG("MEM reuse successful (%d bytes at %p)\n", MEM->mem_size, MEM->mem_buf);
                return 0;
        }

        // If unable to reuse current MEM, unmap and close the current MEM
        // and create a clean one
        if(MEM->mem_size > 0){
                FBTEST_MSG_DEBUG("MEM->mem_size>0\n");
                munmap(MEM->mem_buf, MEM->mem_size);
                close(MEM->fd);

                result = chooseMEMDev();
                if(result < 0)
                        showError(result, 1);

                result = openMEMDev();
                if(result < 0)
                        showError(result, 1);
        }

        // First Allocate MEM
        MEM->mem_page_size = sysconf(_SC_PAGESIZE);
        MEM->mem_size = size;
        MEM->mem_size = (MEM->mem_size + MEM->mem_page_size - 1) & (~(MEM->mem_page_size - 1));

        result = ioctl(MEM->fd, PMEM_ALLOCATE, MEM->mem_size);

        if(result){

                FBTEST_MSG_DEBUG("ERROR! MEM_ALLOCATE failed.\n");
                close_devices();        //UTF: added for cleanup code addition.
                // Close MEM Dev <----------------------------------------------
                return -PMEMALLOC_FAILED;

        } else {

			MEM->mem_buf = mmap(NULL, MEM->mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, MEM->fd, 0);
			MEM->mem_fd = MEM->fd;
			if (MEM->mem_buf == MAP_FAILED) {
				FBTEST_MSG_DEBUG("ERROR: MEM MMAP failed!\n");
				close_devices();        //UTF: added for cleanup code addition.
				// Deallocate <------------------------------------------
				return -MMAP_FAILED;
		}

		memset(MEM->mem_buf, 0x00, MEM->mem_size);
		FBTEST_MSG_DEBUG("MEM Allocation successful (%d bytes at %p)\n", MEM->mem_size, MEM->mem_buf);
        }
#endif
        return 0;
}

void memset16(void *pDst, uint16_t Value, int Count)
{
	uint16_t *ptr = pDst;
	while (Count--)
		*ptr++ = Value;
}

void memset24(void *pDst, uint32_t Value, int Count)
{
	uint8_t *ptr = pDst, *endptr;
	uint8_t x,y,z;

	endptr = ptr+3*Count;
	x = Value & 0xFF; y = (Value >> 8) & 0xFF; z = (Value >> 16) & 0xFF;

	while(ptr < endptr){
		*ptr++ = x;
		*ptr++ = y;
		*ptr++ = z;
	}
}

void memset32(void *pDst, uint32_t Value, int Count)
{
	uint8_t *ptr = pDst, *endptr;
	uint8_t w,x,y,z;

	endptr = ptr+4*Count;
	w = Value & 0xFF; x = (Value >> 8) & 0xFF; y = (Value >> 16) & 0xFF; z = (Value >> 24) & 0xFF;

	while(ptr < endptr){
		*ptr++ = w;
		*ptr++ = x;
		*ptr++ = y;
		*ptr++ = z;
	}
}

float getFormatBpp(int colorFmt)
{
	switch (colorFmt) {
	case MDP_Y_CBCR_H2V2:
	case MDP_Y_CRCB_H2V2:
	case MDP_Y_CR_CB_H2V2:
	case MDP_Y_CB_CR_H2V2:
		return 1.5;
	case MDP_Y_CBCR_H2V1:
	case MDP_Y_CRCB_H2V1:
	case MDP_YCRYCB_H2V1:
	case MDP_YCBYCR_H2V1:
	case MDP_CBYCRY_H2V1:
	case MDP_Y_CRCB_H1V2:
	case MDP_Y_CBCR_H1V2:
	case MDP_RGB_565:
	case MDP_BGR_565:
	case MDP_ARGB_1555:
	case MDP_RGBA_5551:
	case MDP_ARGB_4444:
	case MDP_RGBA_4444:
		return 2.0;
	case MDP_RGB_888:
	case MDP_BGR_888:
	case MDP_Y_CRCB_H1V1:
	case MDP_Y_CBCR_H1V1:
	case MDP_YCRCB_H1V1:
	case MDP_YCBCR_H1V1:
		return 3.0;
	case MDP_ARGB_8888:
	case MDP_RGBX_8888:
	case MDP_RGBA_8888:
	case MDP_BGRA_8888:
	case MDP_XRGB_8888:
	case MDP_BGRX_8888:
	case MDP_ARGB_8888_TILE:
	case MDP_RGBA_8888_TILE:
	case MDP_ABGR_8888_TILE:
	case MDP_BGRA_8888_TILE:
	case MDP_XRGB_8888_TILE:
	case MDP_RGBX_8888_TILE:
	case MDP_XBGR_8888_TILE:
	case MDP_BGRX_8888_TILE:
		return 4.0;
	default:
		printf("Skip: Unsupported Color format (%d)\n", colorFmt);
		return -1;
	}
}

// Tests
int paintBuffer(unsigned char *buffer, int color, int format,
			unsigned int width, unsigned int height, unsigned int line_length)
{
	unsigned int row;
	uint16_t shortColorVal;
	uint32_t longColorVal;
	uint8_t	 yVal;
	uint16_t cbcrVal;

	if(buffer == NULL) {
		printf("Skip: Buffer is NULL");
		return TEST_RESULT_SKIP;
	}

	switch (format) {
	case MDP_RGB_565:
		switch (color) {
		case WHITE:
			shortColorVal = COLOR_WHITE_RGB_565;
			break;
		case GREY:
			shortColorVal = COLOR_GREY_RGB_565;
			break;
		case YELLOW:
			shortColorVal = COLOR_YELLOW_RGB_565;
			break;
		case BLACK:
		default:
			shortColorVal = COLOR_BLACK_RGB_565;
			break;
		}

		for (row = 0; row < height; row++)
			memset16(buffer+line_length*row,
			  shortColorVal, width);
		break;

	case MDP_BGR_565:
		switch (color) {
		case YELLOW:
		default:
			shortColorVal = COLOR_YELLOW_BGR_565;
			break;
		}

		for (row = 0; row < height; row++)
			memset16(buffer+line_length*row,
			  shortColorVal, width);
		break;

	case MDP_RGB_888:
		switch (color) {
		case WHITE:
			longColorVal = COLOR_WHITE_RGB_888;
			break;
		case GREY:
			longColorVal = COLOR_GREY_RGB_888;
			break;
		case YELLOW:
			longColorVal = COLOR_YELLOW_RGB_888;
			break;
		case BLACK:
		default:
			longColorVal = COLOR_BLACK_RGB_888;
			break;
		}

		for (row = 0; row < height; row++)
			memset24(buffer+line_length*row,
			   longColorVal, width);
		break;

	case MDP_BGR_888:
		switch (color) {
		case YELLOW:
		default:
			longColorVal = COLOR_YELLOW_BGR_888;
			break;
		}

		for (row = 0; row < height; row++)
			memset24(buffer+line_length*row,
			   longColorVal, width);
		break;

	case MDP_XRGB_8888:
		switch (color) {
		case WHITE:
			longColorVal = COLOR_WHITE_XRGB_8888;
			break;
		case BLACK:
			longColorVal = COLOR_BLACK_XRGB_8888;
			break;
		case YELLOW:
		default:
			longColorVal = COLOR_YELLOW_XRGB_8888;
			break;
		}
		for (row = 0; row < height; row++) {
			memset32(buffer+line_length*row,
				longColorVal, width);
		}
		break;

	case MDP_ARGB_8888:
	case MDP_BGRA_8888:
		switch (color) {
		case WHITE:
			longColorVal = COLOR_WHITE_ARGB_8888;
			break;
		case BLACK:
			longColorVal = COLOR_BLACK_ARGB_8888;
			break;
		case YELLOW:
		default:
			longColorVal = COLOR_YELLOW_ARGB_8888;
			break;
		}
		for (row = 0; row < height; row++)
			memset32(buffer+line_length*row,
				longColorVal, width);
		break;

	case MDP_RGBX_8888:
	case MDP_RGBA_8888:
		if (color == BLEND) {
			longColorVal = 0x00000000;
			for (row = 0; row < height; row++) {
				memset32(buffer+line_length*row,
					longColorVal, width);
				//Increment the Alpha byte by 1
				//Reset to 0 every 256 rows
				if (row % 255)
					longColorVal += (1 << 24);
				else
					longColorVal = 0x00000000;
			}
		} else {
			switch (color) {
			case WHITE:
				longColorVal = COLOR_WHITE_RGBA_8888;
				break;
			case BLACK:
				longColorVal = COLOR_BLACK_RGBA_8888;
				break;
			case YELLOW:
			default:
				longColorVal = COLOR_YELLOW_RGBA_8888;
				break;
			}
			for (row = 0; row < height; row++)
				memset32(buffer+line_length*row,
					longColorVal, width);
		}
        break;

	case MDP_YCRYCB_H2V1:
		switch (color) {
		case WHITE:
			longColorVal = COLOR_WHITE_YCBYCR_H2V1;
			break;
		case GREY:
			longColorVal = COLOR_GREY_YCBYCR_H2V1;
			break;
		case YELLOW:
			longColorVal = COLOR_YELLOW_YCBYCR_H2V1;
			break;
		case BLACK:
		default:
			longColorVal = COLOR_BLACK_YCBYCR_H2V1;
			break;
		}
		for (row = 0; row < height; row++)
			memset32(buffer+line_length*row,
				longColorVal, width/2);
		break;

	case MDP_Y_CBCR_H2V2:
		switch (color) {
		case YELLOW:
		default:
			yVal = COLOR_YELLOW_Y;
			cbcrVal = COLOR_YELLOW_CBCR;
			break;
		}
		memset(buffer, yVal, width*height);
		memset16(buffer+(width*height),
			cbcrVal, (width*height)/4);
		break;

	case MDP_Y_CRCB_H2V2:
		switch (color) {
		case YELLOW:
		default:
			yVal = COLOR_YELLOW_Y;
			cbcrVal = COLOR_YELLOW_CRCB;
			break;
		}
		memset(buffer, yVal, width*height);
		memset16(buffer+(width*height),
			cbcrVal, (width*height)/4);
		break;

	case MDP_Y_CRCB_H2V1:
		switch (color) {
		case YELLOW:
		default:
			yVal = COLOR_YELLOW_Y;
			cbcrVal = COLOR_YELLOW_CRCB;
			break;
		}
		memset(buffer, yVal, width*height);
		memset16(buffer+(width*height),
			cbcrVal, (width*height)/2);
		break;

	case MDP_Y_CBCR_H2V1:
		switch (color) {
		case YELLOW:
		default:
			yVal = COLOR_YELLOW_Y;
			cbcrVal = COLOR_YELLOW_CBCR;
			break;
		}
		memset(buffer, yVal, width*height);
		memset16(buffer+(width*height),
			cbcrVal, (width*height)/2);
		break;

	default:
		printf("Skip: Unsupported FB Format %s (%d)\n",
			lookup_string(format_name_table, format, "unknown"),
			format);
		return TEST_RESULT_SKIP;
	}

	return TEST_RESULT_PASS;
}

int drawBG(void)
{
	int result;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	if (thisFBTEST->fBuffer == FB0) {
		result = paintBuffer(FB->fb_buf, WHITE, FB->fb_fmt,
			FB->fb_vinfo.xres, FB->fb_vinfo.yres, FB->fb_finfo.line_length);

		if (result != TEST_RESULT_PASS)
			return result;

		if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
			printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n", __LINE__);
			return TEST_RESULT_FAIL;
		}
	}
	return TEST_RESULT_PASS;
}

int getFrameSize(int width,int height,int format)
{
	int size = 0;
	int alignedh = 0, alignedw = 0;

	if (format == MDP_Y_CR_CB_GH2V2) {
		alignedw = ALIGN(width, 16);
		size = (alignedw * height +
		       ((ALIGN(alignedw/2, 16) * height/2)*2));
		size = ALIGN(size, 4096);
	} else if ((format == MDP_Y_CRCB_H2V2_TILE) ||
		(format == MDP_Y_CBCR_H2V2_TILE)) {
		alignedh = ALIGN(height, 32);
		alignedw = ALIGN(width, 128);
		size = ALIGN( alignedw * alignedh, 8192);
		size += ALIGN( alignedw * ALIGN(height/2, 32), 8192);
	} else if (format == MDP_Y_CBCR_H2V2_ADRENO) {
		alignedh = ALIGN(height, 32);
		alignedw = ALIGN(width, 32);
		size  = ALIGN(alignedw*alignedh, 4096);
                size += ALIGN(2 * ALIGN(width/2, 32) * ALIGN(height/2, 32), 4096);
	}  else if (format == MDP_Y_CBCR_H2V2_VENUS) {
		alignedh = VENUS_Y_SCANLINES(COLOR_FMT_NV12, height);
		size = VENUS_BUFFER_SIZE(COLOR_FMT_NV12, width, alignedh);
	} else
		size = width * height * getFormatBpp(format);

	return size;
}

int lookup_key(lookup_string_t const *ptable, char const *string, int default_result)
{
	while (ptable->string) {
		if (strcmp(ptable->string, string) == 0)
			return ptable->key;

		++ptable;
	}

	/* return the 'not found' value at the end of the table */
	return default_result;
}

char const *lookup_string(lookup_string_t const *ptable, int key, char const *default_result)
{
	while (ptable->string) {
		if (ptable->key == key)
			return ptable->string;

		++ptable;
	}

	return default_result;
}

//UTF: Below code added for fbtest enhancement

int lookup_val(lookup_string_t const *ptable, int key, int default_result)
{
	while (ptable->string) {
		if (ptable->key == key)
			return ptable->val;

		++ptable;
	}

	return default_result;
}

int lookup_key_case_insensitive(lookup_string_t const *ptable, char const *string, int default_result)
{
	while (ptable->string) {
		if (strcasecmp(ptable->string, string) == 0)
			return ptable->key;

		++ptable;
	}

	/* return the 'not found' value at the end of the table */
	return default_result;
}
int rand_range(int min_n, int max_n)
{
    return rand() % (max_n - min_n + 1) + min_n;
}

/* a & b are the numbers whose GCD is to be found */
int gcd(int a,int b)
{
    int c = 0;
    while(1)
    {
	if(b!=0)
		c = a%b;
	else
		return a;

	if(c==0){
		return b;
	}
	a = b;
	b = c;
    }
}

int brightness_change(enum brightness_type type)
{
	char brightness_value_str[4] = {0};
	int len;
	int brightness_fd;

	brightness_fd =
		open("/sys/class/leds/lcd-backlight/brightness", O_RDWR);
	if (brightness_fd >= 0) {
		if (type == BRIGHTNESS_STORE_NEW_LEVEL){
			if (read(brightness_fd, brightness_value_str,
						sizeof(brightness_value_str) - 1)) {
				prev_brightness =
					(unsigned char)atoi(brightness_value_str);
			} else {
				printf("Error in reading brightness value\n");
				return -1;
			}

			if (255 - prev_brightness > 50)
				curr_brightness = 250;
			else
				curr_brightness = prev_brightness - 50;
			FBTEST_MSG_DEBUG("curr_brightness = %u\n", curr_brightness);
			snprintf(brightness_value_str, sizeof(brightness_value_str), "%u", curr_brightness);
		} else {
			FBTEST_MSG_DEBUG("prev_brightness = %u\n", prev_brightness);
			snprintf(brightness_value_str, sizeof(brightness_value_str), "%u", prev_brightness);
		}

		len = strlen(brightness_value_str);
		if (write(brightness_fd, brightness_value_str,
					len) != len) {
			printf("Error in writing brightness value\n");
			powerCycleDisplay();
        }
        close(brightness_fd);
    } else {
		printf("ERROR: Can't open brightness file\n");
		return -1;
	}
	return 0;
}

void parse_config_file(char config_path[MAX_FILE_PATH], int* ptr_num_line, char* argv_array[][MAX_FILE_PATH], int* argc_array)
{
	int line_c = 0;
	FILE *config_file = fopen (config_path, "r");
	if (config_file != NULL) {
		char line [512];
		while (fgets (line, sizeof line, config_file) != NULL) {
			if (line[0] == '\r' || line[0] == '\n')
				continue;
			int len = 0;
			int argc = 0;
			char *argvs[128];
			char *token = strtok (line," ");
			while (token != NULL) {
				len = strlen(token) + 1;
				if (len > 0 && token[len - 1] < 32) {
					argvs[argc] = malloc(strlen(token) + 1);
				} else {
					argvs[argc] = malloc(strlen(token) + 1);
				}
				unsigned int token_c;
				for (token_c = 0; token_c < strlen(token) + 1; token_c++) {
					if (token[token_c] >= 32)
						argvs[argc][token_c] = token[token_c];
				}
				argc++;
				token = strtok (NULL, " ");
			}
			int a;
			for (a = 0; a < argc; a++) {
				char * str = argvs[a];
				argv_array[line_c][a] = malloc((strlen(str) + 1));
				strlcpy(argv_array[line_c][a], str, sizeof(argv_array[line_c][a]));
			}
			argc_array[line_c] = argc;
			line_c++;
		}
		fclose (config_file);
	}else{
		printf("Config File not found.\n");
		exit(0);
	}
	*ptr_num_line = line_c;
}

void date_timestamp_string(char *name)
{
	struct tm* tm;
	time_t now;

	time(&now);
	tm = localtime(&now);
	if (tm)
		strftime(name,16,"_%y%m%d_%H%M%S",tm);
}

//UTF: Below code addded for cleanup.
// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void signal_callback_handler(int signum)
{
	printf("\n%d Called: Aborting fbtest.\n",signum);

	//UNSET the overlay fb.
	cleanup_overlay();

	//Close fb and pmem
	close_devices();

	//Terminate program
	exit(signum);
}

void showError(int errorno, int terminate)
{
	errorno = errorno * -1;
	if (errorno == 0)
		printf("\nInfo No.# %d, %s\n\n",errorno, errorLUT[errorno]);
	else
		printf("\nErro No.# %d, %s\n\n",errorno, errorLUT[errorno]);
	testflag = FALSE;
	if(terminate){
		VPRINT(verbose, "Cleaning and Closing device...\n\n");
		cleanup_overlay();
		close_devices();
		exit(1);
	}
}

int cleanup_overlay()
{
	int i = 0;
	int ret = TEST_RESULT_PASS;
	// Cleanup and close up stuff here

	for (i = 0; i < 4; i++) {
		//check if UNSET call is pending, if so call it.
		if(curr_overlay_id[i] != MSMFB_NEW_REQUEST){
			if (ioctl(FB->fb_fd, MSMFB_OVERLAY_UNSET,
					&curr_overlay_id[i])) {
				printf("\nERROR! OVERLAY_UNSET failed! Id=%d\n",
						curr_overlay_id[i]);
				ret = TEST_RESULT_FAIL;
			}
			curr_overlay_id[i] = MSMFB_NEW_REQUEST;
		}
	}
	if (ioctl(FB->fb_fd, FBIOPAN_DISPLAY, &(FB->fb_vinfo)) < 0) {
		printf("ERROR: FBIOPAN_DISPLAY failed! line=%d\n", __LINE__);
		return TEST_RESULT_FAIL;
	}

	return ret;

}

void close_devices()
{
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	/* close fb and pmem devs */
	if (MEM && MEM->fd >= 0) {
		if ((MEM->mem_buf != NULL) && (MEM->mem_buf != MAP_FAILED)) {
			munmap(MEM->mem_buf, MEM->mem_size);
#ifdef USE_ION
			ioctl(MEM->fd, ION_IOC_FREE, &handle_data);
			close(MEM->mem_fd);
#ifdef MDP3_FLAG
			ioctl(MEM->fd, ION_IOC_FREE, &handle_data_src);
			close(MEM->mem_fd_src);
#endif
#endif
		}
		close(MEM->fd);
	}

	if (FB->fb_fd >= 0) {
		if (thisFBTEST->fBuffer == FB0) {
			/* unmap */
			if ((FB->fb_buf != NULL) && (FB->fb_buf != MAP_FAILED)) {
				munmap(FB->fb_buf, FB->fb_finfo.smem_len);
			}
		}
		closeFBDev(FB);
	}
	close(thisFBTEST->rot_fd);
}
//UTF: Above code addded for cleanup.
void get_rand4(int *pois_rv, int rv_max1, int rv_max2, int seed)
{
	double	lambda1, lambda2, lambda3, lambda4;  // Mean rate
	int		t, i, j, k, l;

	pois_rv[0] = pois_rv[1] = pois_rv[2] = pois_rv[3] = 0;
	rand_val((int) time(NULL));
	for (i = 1, j = 1, k = 1, l = 1; ;
		(i += 7) % rv_max1, (j += 13) % rv_max2, (k += 17) % rv_max1,
		(l+=23)%rv_max2)
	{
		lambda1 = (int)time(NULL) % i;
		pois_rv[0] *= (poisson(1.0 / lambda1));
		t = (poisson(1.0 / lambda1));
		pois_rv[0] = (pois_rv[0]+t) % rv_max1;
		lambda2 = (int)time(NULL) % j;
		pois_rv[1] *= (poisson(1.0 / lambda2));
		t = (poisson(1.0 / lambda2));
		pois_rv[1] = (pois_rv[1] + t) % rv_max2;
		lambda3 = (int)time(NULL) % k;
		pois_rv[2] *= (poisson(1.0 / lambda3));
		t = (poisson(1.0 / lambda3));
		pois_rv[2] = (pois_rv[2] + t) % rv_max1;
		lambda4 = (int)time(NULL) % l;
		pois_rv[3] *= (poisson(1.0 / lambda4));
		t = (poisson(1.0 / lambda4));
		pois_rv[3] = (pois_rv[3] + t) % rv_max2;

	if ((pois_rv[0] + pois_rv[2] <= rv_max1) &&
		(pois_rv[1] + pois_rv[3] <= rv_max2) &&
		(pois_rv[2] > 0) && (pois_rv[3] > 0)) {
		if(seed-- == 0)
			break;
	}
  }
}

void dump_img(unsigned char *buf, int size, const char *filename) {
	FILE *fp;

	fp = fopen(filename, "wb+");
	if (fp == NULL) {
		fprintf(stderr, "Could not open output file: %s\n", filename);
		return;
	}

	fwrite(buf, 1, size, fp);
	fclose(fp);
}
int poisson(double x)
{
	int    poi_value;
	double t_sum;
	poi_value = 0;
	t_sum = 0.0;
	while (1) {
		t_sum = t_sum + expon(x);
		if (t_sum >= 1.0) break;
		poi_value++;
	}
	return poi_value;
}

double expon(double x)
{
	double z;
	double exp_value;
	do {
		z = rand_val(0);
	} while ((z == 0) || (z == 1));
	exp_value = -x * log(z);
	return exp_value;
}
double rand_val(int seed)
{
	const long  a =      16807;  // Multiplier
	const long  m = 2147483647;  // Modulus
	const long  q =     127773;  // m div a
	const long  r =       2836;  // m mod a
	static long x;               // Random int value
	long        x_div_q;         // x divided by q
	long        x_mod_q;         // x modulo q
	long        x_new;           // New x value

	if (seed > 0) {
		x = seed;
		return(0.0);
	}
	x_div_q = x / q;
	x_mod_q = x % q;
	x_new = (a * x_mod_q) - (r * x_div_q);
	if (x_new > 0)
		x = x_new;
	else
		x = x_new + m;

	return ((double) x / m);
}

int unsetOverlay(struct fbDev* ptr_fb, uint32_t* overlay_id) {
    uint32_t before = overlay_id ? *overlay_id : 0;
    if (ioctl(ptr_fb->fb_fd, MSMFB_OVERLAY_UNSET, overlay_id)) {
        FBTEST_MSG_DEBUG("ERROR! MSMFB_OVERLAY_UNSET failed(before = %d, "
        "after = %d)! %s (%d)\n", before, overlay_id ? *overlay_id : 0,
        strerror(errno), errno);
        return 0;
    }
    return 0;
}

/* Check ROI against scaling limits */
int is_valid_overlay (struct mdp_overlay *overlay) {
	int max_downscale, max_upscale;
	if((FBTEST.decimation) &&
		((strcmp(FBTEST.testname, "scale")) == 0)) {
		FBTEST_MSG_DEBUG("INFO: Decimation enabled");
		return VALID_OVERLAY ;
	}

	max_upscale = getMaxUpScale(mdp_version);
	max_downscale = getMaxDownScale(mdp_version);

	if (overlay->dst_rect.h > (overlay->src_rect.h * max_upscale)) {
		VPRINT(verbose, "Dst Height is greater than Src Height*%d\n",
			max_upscale);
		return -INVALID_OVERLAY;
	}
	if (overlay->dst_rect.w > (overlay->src_rect.w * max_upscale)) {
		VPRINT(verbose, "Dst Width is greater than Src Width*%d\n",
			max_upscale);
		return -INVALID_OVERLAY;
	}
	if (overlay->src_rect.h > (overlay->dst_rect.h * max_downscale)) {
		VPRINT(verbose, "Src Height is greater than Dst Height*%d\n",
			max_downscale);
		return -INVALID_OVERLAY;
	}
	if (overlay->src_rect.w > (overlay->dst_rect.w * max_downscale)) {
		VPRINT(verbose, "Src Width is greater than Dst Width*%d\n",
			max_downscale);
		return -INVALID_OVERLAY;
	}

	return VALID_OVERLAY ;
}

int setOverlay(struct mdp_overlay* ptr_overlay, struct fbDev* ptr_fb,
                        uint32_t format, int32_t src_w, int32_t src_h,
                        int32_t src_x, int32_t src_y, int32_t dst_w,
                        int32_t dst_h, int32_t dst_x, int32_t dst_y,
                        int32_t z_order, uint32_t transp_mask, uint32_t flags,
						int32_t id, uint32_t offset) {
	ptr_overlay->src.width  = ptr_fb->fb_vinfo.xres;
	ptr_overlay->src.height = ptr_fb->fb_vinfo.yres;
	ptr_overlay->src.format = format;
	ptr_overlay->src_rect.x = src_x;
	ptr_overlay->src_rect.y = src_y;
	ptr_overlay->src_rect.w = src_w;
	ptr_overlay->src_rect.h = src_h;
	ptr_overlay->dst_rect.x = dst_x;
	ptr_overlay->dst_rect.y = dst_y;
	ptr_overlay->dst_rect.w = dst_w;
	ptr_overlay->dst_rect.h = dst_h;
	ptr_overlay->z_order = z_order;
	ptr_overlay->transp_mask = transp_mask;
	ptr_overlay->flags = flags;
	ptr_overlay->id = id;
	//If the framebuffer (i.e. base layer) format is 32 bits,
	//we use it as the background, and do not use fg for blending.
	if (ptr_fb->fb_vinfo.bits_per_pixel == 32) {
		ptr_overlay->is_fg = 0;
		ptr_overlay->alpha = 0;
	}
	//Otherwise, we set the alpha mask of the background to 0xFF
	//transparent, and set the is_fg flag to 1 to use the foreground's
	//alpha value when blending.
	else {
		ptr_overlay->is_fg = 1;
		ptr_overlay->alpha = 0xFF;
	}

	/* On MDP3 Scaling is done in Blit, So this check is not valid. */
#ifndef MDP3_FLAG
	if (is_valid_overlay(ptr_overlay)) {
        FBTEST_MSG_DEBUG("Warning: Invalid Overlay [src_w src_h dst_w dst_h dst_x dst_y]=\
                [%d %d %d %d %d %d]\n",
				src_w, src_h, dst_w, dst_h, dst_x, dst_y);
		return INVALID_OVERLAY;
	}
#endif

	if (ioctl(ptr_fb->fb_fd, MSMFB_OVERLAY_SET, ptr_overlay) < 0) {
		FBTEST_MSG_DEBUG("ERROR: MSMFB_OVERLAY_SET failed! Line=%d, %s",
				__LINE__, strerror(errno));
		FBTEST_MSG_DEBUG("[src_w src_h dst_w dst_h dst_x dst_y]=\
				[%d %d %d %d %d %d]\n",
				src_w, src_h, dst_w, dst_h, dst_x, dst_y);
		return -OVERLAY_SET_FAILED;
	}
	VPRINT(verbose, "set overlay successful\n");
	return 0;
}

int doCommit(struct fbDev* hdl, struct mdp_display_commit* commit_info) {
	if (ioctl(hdl->fb_fd, MSMFB_DISPLAY_COMMIT, commit_info)) {
		FBTEST_MSG_DEBUG("[FORMAT] MSMFB_DISPLAY_COMMIT for primary failed\
				line=%d\n", __LINE__);
		return -MSMFB_DISPLAY_COMMIT_FAILED;
	}
	return 0;
}

int doPlay(struct fbDev* hdl, struct msmfb_overlay_data* ptr_ovdata) {
	if (ioctl(FB->fb_fd, MSMFB_OVERLAY_PLAY, ptr_ovdata)) {
		FBTEST_MSG_DEBUG("ERROR! MSMFB_OVERLAY_PLAY failed! Line %d %s\n",
				__LINE__, strerror(errno));
		return -OVERLAY_PLAY_FAILED;
	}
	return 0;
}

void showCRCError(int errorno, char* crcfb_file) {
	if ((CRC_ENTRY_FOUND == errorno) || (CRC_ENTRY_ADD == errorno)) {
		printf("\n%s %s\n\n", errorLUT[errorno], crcfb_file);
	} else {
		errorno = errorno * -1;
		printf("\nErro No.# %d, %s %s\n\n", errorno, errorLUT[errorno], crcfb_file);
	}
}

int setMisr(int misrid) {
	int ret = 0;
	int result = TEST_RESULT_PASS;
	struct msmfb_metadata metadata;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	if (thisFBTEST->misr == 1) {
		memset(&metadata, 0x00, sizeof(metadata));
		metadata.op    = metadata_op_crc;
		metadata.data.misr_request.block_id    = misrid;
		metadata.data.misr_request.frame_count = 1;
		metadata.data.misr_request.crc_op_mode = MISR_OP_SFM;
		printf("setMisr START\n");
		result = ioctl(FB->fb_fd, MSMFB_METADATA_SET, &metadata);
		if (result < 0) {
			printf("SET MISR FAILED! Error = %d\n", result);
			result = TEST_RESULT_FAIL;
		} else
			printf("setMisr END\n");
	}
	return result;
}

int getMisr(int misrid) {
	int ret = 0;
	int result = TEST_RESULT_PASS;
	struct msmfb_metadata metadata;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	if (thisFBTEST->misr == 1) {
		memset(&metadata, 0x00, sizeof(metadata));
		printf("getMisr START\n");
		metadata.op    = metadata_op_crc;
		metadata.data.misr_request.block_id    = misrid;
		metadata.data.misr_request.crc_op_mode = MISR_OP_SFM;
		result = ioctl(FB->fb_fd, MSMFB_METADATA_GET, &metadata);
		if (result < 0) {
			printf("getMisr FAILED\n");
			result = TEST_RESULT_FAIL;
		}
		if (mdp_version <= MDP_V4_0)
			result = powerCycleDisplay();
		if (result < 0) {
			printf("power cycle display FAILED\n");
			result = TEST_RESULT_FAIL;
		}
		printf("getMisr END\n");
		if (thisFBTEST->crcmode == 0)
			ret = crcGen(metadata.data.misr_request.crc_value[0]);
		else if (thisFBTEST->crcmode == 1)
			ret = crcValidation(metadata.data.misr_request.crc_value[0]);

		showCRCError(ret, thisFBTEST->crcPath);
		if (!((ret == 0) || (ret == CRC_ENTRY_FOUND) || (ret == CRC_ENTRY_ADD))) {
			result = TEST_RESULT_FAIL;
		} else {
			printf("CRC Val = 0x%x\n", metadata.data.misr_request.crc_value[0]);
			result = TEST_RESULT_PASS;
		}
	}
	return result;
}

/* Post Processing CRC capture*/
int postProcCRC (void) {
	int result = TEST_RESULT_PASS;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;

	if (thisFBTEST->fBuffer == FB0) {
		if (thisFBTEST->misr == 1)
			result = misr(DISPLAY_MISR_DSI0);
	} else if (thisFBTEST->fBuffer == FB1) {
		if (thisFBTEST->misr == 1)
			result = misr(DISPLAY_MISR_HDMI);
	}

	if (TEST_RESULT_PASS != result)
		printf("ERROR: CRC API failed (%d)!\n", result);
	return result;
}

/* Display suspend/resute operation */
int powerCycleDisplay(void) {
	int result = TEST_RESULT_PASS;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	VPRINT(verbose, "FB_BLANK_POWERDOWN\n");
	result = ioctl(FB->fb_fd, FBIOBLANK, FB_BLANK_POWERDOWN);
	if (result < 0) {
		VPRINT(verbose, "[BLANK] FB_BLANK_POWERDOWN failed! line=%d err=%d\n",
				__LINE__, errno);
		return TEST_RESULT_FAIL;
	}
	sleep(1);
	VPRINT(verbose, "FB_BLANK_UNBLANK\n");
	result = ioctl(FB->fb_fd, FBIOBLANK, FB_BLANK_UNBLANK);
	if (result < 0) {
		VPRINT(verbose, "[BLANK] FB_BLANK_UNBLANK failed! line=%d err=%d\n",
				__LINE__, errno);
		return TEST_RESULT_FAIL;
	}
	return result;
}

#define HDMI_PANEL_NAME "hdmi panel"
#define LVDS_PANEL_NAME "lvds panel"
#define DTV_PANEL_NAME "dtv panel"
#define MIPI_DSI_VIDEO_PANEL_NAME "mipi dsi video panel"
#define MIPI_DSI_CMD_PANEL_NAME "mipi dsi cmd panel"
#define WB_PANEL_NAME "writeback panel"
#define EDP_PANEL_NAME "edp panel"

/* CRC Capture */
int misr(int misrid) {
	int result = TEST_RESULT_PASS;
	char fb_type_str[MAX_FILE_PATH];
	char fb_type_path[MAX_FILE_PATH];
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int fb_type_fd;

	snprintf(fb_type_path, sizeof(fb_type_path), "%s%d",
			"/sys/class/graphics/fb", thisFBTEST->fBuffer);
	strlcat(fb_type_path, "/msm_fb_type", sizeof(fb_type_path));
	fb_type_fd = open(fb_type_path, O_RDONLY);
	read(fb_type_fd, fb_type_str, sizeof(fb_type_str));
	close(fb_type_fd);

	if (strncmp(MIPI_DSI_VIDEO_PANEL_NAME, fb_type_str, strlen(MIPI_DSI_VIDEO_PANEL_NAME)) == 0) {
		printf("DSI Video Interface MISR Selected\n");
		misrid = DISPLAY_MISR_DSI0;
	}
	if (strncmp(MIPI_DSI_CMD_PANEL_NAME, fb_type_str, strlen(MIPI_DSI_CMD_PANEL_NAME)) == 0) {
		printf("DSI Command Interface MISR Selected\n");
		misrid = DISPLAY_MISR_DSI_CMD;
	}
	if (strncmp(LVDS_PANEL_NAME, fb_type_str, strlen(LVDS_PANEL_NAME)) == 0) {
		printf("LCDC/LVD Interface MISR Selected\n");
		misrid = DISPLAY_MISR_LCDC;
	}
	if (strncmp(DTV_PANEL_NAME, fb_type_str, strlen(DTV_PANEL_NAME)) == 0) {
		printf("HDMI Interface MISR Selected\n");
		misrid = DISPLAY_MISR_HDMI;
	}
	// if (strncmp(WB_PANEL_NAME, fb_type_str, strlen(WB_PANEL_NAME)) == 0) {
		// printf("MDP Layer Mixer MISR Selected for WB\n");
		// misrid = DISPLAY_MISR_MDP;
	// }
	if (strncmp(EDP_PANEL_NAME, fb_type_str, strlen(EDP_PANEL_NAME)) == 0) {
		printf("EDP Interface MISR Selected\n");
		misrid = DISPLAY_MISR_EDP;
	}
	setMisr(misrid);
	result = getMisr(misrid);
#ifdef MDP3_FLAG
	powerCycleDisplay();
#endif
	return result;
}

static int crcGenPostProc(char *fbname, long int crc) {
	FILE *fptr;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = CRC_ENTRY_ADD;
	int fb = -1;
	char *crcpath, path[256], fname[100];
	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;

	if (NULL != crcpath) {
		if (access(crcpath, F_OK) != -1)
			fptr = fopen(crcpath, "r");
		else
			fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
	}

	while (fscanf(fptr, "%s %s %s 0x%08lx\n", (char *)&fbname_db,
		(char *)&path, (char *)&fname, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr && (strcmp(fname, thisFBTEST->testname) == 0) &&
			(strcmp(fbname_db, fbname_ref) == 0)    &&
			(strcmp(path, ptr) == 0)) {
			if (fcrc != crc) {
				flag = CRC_ENTRY_ADD;
			} else {
				flag = CRC_ENTRY_FOUND;
			}
			break;
		}
	}
	if (flag == CRC_ENTRY_ADD) {
		fclose(fptr);
		fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr) {
			if (strcmp(thisFBTEST->testname, "format") == 0)
				fprintf(fptr, "%s %s %s 0x%08lx\n", fbname_ref, ptr,
					"format", crc);
			if (strcmp(thisFBTEST->testname, "postproc") == 0)
				fprintf(fptr, "%s %s %s 0x%08lx\n", fbname_ref, ptr,
					"postproc", crc);
			if (strcmp(thisFBTEST->testname, "deinterlace") == 0)
				fprintf(fptr, "%s %s %s 0x%08lx\n", fbname_ref, ptr,
					"deinterlace", crc);
			if (strcmp(thisFBTEST->testname, "dither") == 0)
				fprintf(fptr, "%s %s %s 0x%08lx\n", fbname_ref, ptr,
					"dither", crc);
			if (strcmp(thisFBTEST->testname, "multiop") == 0)
				fprintf(fptr, "%s %s %s 0x%08lx\n", fbname_ref, ptr,
					"multiop", crc);
		}
	}
	fclose(fptr);
	return flag;
}

static int crcGenScale(char *fbname, long int crc) {
	FILE *fptr;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = CRC_ENTRY_ADD;
	int fb = -1;
	char *crcpath, path[256], fname[100];
	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;

	if (NULL != crcpath) {
		if (access(crcpath, F_OK) != -1)
			fptr = fopen(crcpath, "r");
		else
			fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
	}
	unsigned int h, w;
	int s;
	while (fscanf(fptr, "%s %s %d %d %d %s  0x%08lx\n", (char *)&fbname_db,
		(char *)&path, &h, &w, &s, (char *)&fname, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr && (h == thisFBTEST->outResolution.height)   &&
			(w == thisFBTEST->outResolution.width)    &&
			(s == thisFBTEST->isStepScale)            &&
			(strcmp(fbname_db, fbname_ref) == 0)    &&
			(strcmp(fname, thisFBTEST->testname) == 0) &&
			(strcmp(path, ptr) == 0)) {
			if (fcrc != crc) {
				flag = CRC_ENTRY_ADD;
			} else {
				flag = CRC_ENTRY_FOUND;
			}
			break;
		}
	}
	if (flag == CRC_ENTRY_ADD) {
		fclose(fptr);
		fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr) {
			if (strcmp(thisFBTEST->testname, "scale") == 0)
				fprintf(fptr, "%s %s %d %d %d %s 0x%08lx\n", fbname_ref, ptr,
					thisFBTEST->outResolution.height, thisFBTEST->outResolution.width,
					thisFBTEST->isStepScale, "scale", crc);
			if (strcmp(thisFBTEST->testname, "overlayscale") == 0)
				fprintf(fptr, "%s %s %d %d %d %s 0x%08lx\n", fbname_ref, ptr,
					thisFBTEST->outResolution.height, thisFBTEST->outResolution.width,
					thisFBTEST->isStepScale, "overlayscale", crc);
		}
	}
	fclose(fptr);
	return flag;
}

static int crcGenVideoplay(char *fbname, long int crc) {
	FILE *fptr;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = CRC_ENTRY_ADD;
	int fb = -1, frmNum = 0;
	char *crcpath, path[256], fname[100];
	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	int fps;
	crcpath = thisFBTEST->crcPath;

	if (NULL != crcpath) {
		if (access(crcpath, F_OK) != -1)
			fptr = fopen(crcpath, "r");
		else
			fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
	}
	while (fscanf(fptr, "%s %s %d %s frmNum %d 0x%08lx\n", (char *)&fbname_db, (char *)&path, &fps,
		(char *)&fname, &frmNum, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr && (fps == thisFBTEST->fps) &&
			(strcmp(fbname_db, fbname_ref) == 0)    &&
			(strcmp(fname, thisFBTEST->testname) == 0) &&
			(strcmp(path, ptr) == 0) && (frmNum == thisFBTEST->frameNum)) {
			if (fcrc != crc) {
				flag = CRC_ENTRY_ADD;
			} else {
				flag = CRC_ENTRY_FOUND;
			}
			break;
		}
	}
	if (flag == CRC_ENTRY_ADD) {
		fclose(fptr);
		fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr) {
			if (strcmp(thisFBTEST->testname, "videoplay") == 0)
			fprintf(fptr, "%s %s %d %s frmNum %d 0x%08lx\n", fbname_ref, ptr,
				thisFBTEST->fps, "videoplay", thisFBTEST->frameNum, crc);
			if (strcmp(thisFBTEST->testname, "fpsvsync") == 0)
			fprintf(fptr, "%s %s %d %s frmNum %d 0x%08lx\n", fbname_ref, ptr,
				thisFBTEST->fps, "fpsvsync", thisFBTEST->frameNum, crc);
		}
	}
	fclose(fptr);
	return flag;
}

static int crcGenRotate(char *fbname, long int crc) {
	FILE *fptr;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = CRC_ENTRY_ADD;
	int fb = -1;
	char *crcpath, path[256], fname[100];
	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;

	if (NULL != crcpath) {
		if (access(crcpath, F_OK) != -1)
			fptr = fopen(crcpath, "r");
		else
			fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
	}
	int d, flip;
	while (fscanf(fptr, "%s %s %d %d %s 0x%08lx\n", (char *)&fbname_db, (char *)&path, &d, &flip,
				(char *)&fname, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');

		if (ptr && (flip == thisFBTEST->rotateFlip) &&
				(strcmp(fbname_db, fbname_ref) == 0)    &&
				(d == thisFBTEST->rotateDegree) &&
				(strcmp(fname, thisFBTEST->testname) == 0) &&
				(strcmp(path, ptr) == 0)) {
			if (fcrc != crc) {
				flag = CRC_ENTRY_ADD;
			} else {
				flag = CRC_ENTRY_FOUND;
			}
			break;
		}
	}
	if (flag == CRC_ENTRY_ADD) {
		fclose(fptr);
		fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr)
			fprintf(fptr, "%s %s %d %d %s 0x%08lx\n", fbname_ref, ptr,
				thisFBTEST->rotateDegree, thisFBTEST->rotateFlip, "rotate", crc);
	}
	fclose(fptr);
	return flag;
}

static int crcGenMove(char *fbname, long int crc) {
	FILE *fptr;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = CRC_ENTRY_ADD;
	int fb = -1;
	char *crcpath, path[256], fname[100];
	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;

	if (NULL != crcpath) {
		if (access(crcpath, F_OK) != -1)
			fptr = fopen(crcpath, "r");
		else
			fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
	}
	int x1, x2, y1, y2;
	while (fscanf(fptr, "%s %s %d %d %d %d %s 0x%08lx\n", (char *)&fbname_db,
		(char *)&path, &x1, &y1, &x2, &y2, (char *)&fname, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr && (x1 == thisFBTEST->imgOffset.startX) &&
			(y1 == thisFBTEST->imgOffset.startY) &&
			(x2 == thisFBTEST->imgOffset.endX) &&
			(y2 == thisFBTEST->imgOffset.endY) &&
			(strcmp(fbname_db, fbname_ref) == 0)    &&
			(strcmp(fname, thisFBTEST->testname) == 0) &&
			(strcmp(path, ptr) == 0)) {
			if (fcrc != crc) {
				flag = CRC_ENTRY_ADD;
			} else {
				flag = CRC_ENTRY_FOUND;
			}
			break;
		}
	}
	if (flag == CRC_ENTRY_ADD) {
		fclose(fptr);
		fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr) {
			if (strcmp(thisFBTEST->testname, "move") == 0)
				fprintf(fptr, "%s %s %d %d %d %d %s 0x%08lx\n", fbname_ref, ptr,
				thisFBTEST->imgOffset.startX, thisFBTEST->imgOffset.startY,
				thisFBTEST->imgOffset.endX, thisFBTEST->imgOffset.endY,
				"move", crc);
			if (strcmp(thisFBTEST->testname, "hwcursor") == 0)
				fprintf(fptr, "%s %s %d %d %d %d %s 0x%08lx\n", fbname_ref, ptr,
				thisFBTEST->imgOffset.startX, thisFBTEST->imgOffset.startY,
				thisFBTEST->imgOffset.endX, thisFBTEST->imgOffset.endY,
				"hwcursor", crc);
		}
	}
	fclose(fptr);
	return flag;
}

static int crcGenCrop(char *fbname, long int crc) {
	FILE *fptr;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int flag = CRC_ENTRY_ADD;
	int fb = -1;
	char *crcpath, path[256], fname[100];
	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;

	if (NULL != crcpath) {
		if (access(crcpath, F_OK) != -1)
			fptr = fopen(crcpath, "r");
		else
			fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
	}
	unsigned int h, w;
	int c, i, j, x, y;
	struct cropParams *thisCrop;
	thisCrop = &CROPTEST;
	while (fscanf(fptr, "%s %s %d %d %d %d %d %d %d %s  0x%08lx\n",
		(char *)&fbname_db, (char *)&path, &c, &x, &y, &h, &w, &i, &j,
		(char *)&fname, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr && (c == thisFBTEST->crop) &&
			(x == thisCrop->src_rect.x) &&
			(y == thisCrop->src_rect.y) &&
			(i == thisCrop->dst_rect.x) &&
			(j == thisCrop->dst_rect.y) &&
			(w == thisCrop->resolution.width) &&
			(h == thisCrop->resolution.height) &&
			(strcmp(fbname_db, fbname_ref) == 0)    &&
			(strcmp(fname, thisFBTEST->testname) == 0)  &&
			(strcmp(path, ptr) == 0)) {
			if (fcrc != crc) {
				flag = CRC_ENTRY_ADD;
			} else {
				flag = CRC_ENTRY_FOUND;
			}
		}
	}
	if (flag == CRC_ENTRY_ADD) {
		fclose(fptr);
		fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr)
			fprintf(fptr, "%s %s %d %d %d %d %d %d %d %s 0x%08lx\n", fbname_ref, ptr,
			thisFBTEST->crop, thisCrop->src_rect.x, thisCrop->src_rect.y,
			thisCrop->resolution.height, thisCrop->resolution.width,
			thisCrop->dst_rect.x, thisCrop->dst_rect.y, "crop", crc);
	}
	fclose(fptr);
	return flag;
}

static int crcGenBlend(char *fbname, long int crc) {
	FILE *fptr;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = CRC_ENTRY_ADD;
	int fb = -1;
	char *crcpath, path[256], fname[100];
	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;

	if (NULL != crcpath) {
		if (access(crcpath, F_OK) != -1)
			fptr = fopen(crcpath, "r");
		else
			fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
	}
	printf("\n%s  %s \n", ImgFileParams[0].filenamePath,
			ImgFileParams[1].filenamePath);
	unsigned int ispixblend, ispremul, stage;
	int alpha0, alpha1, alpha2, alpha3;
	char path1[MAX_FILE_PATH], path2[MAX_FILE_PATH], path3[MAX_FILE_PATH] = "none", path4[MAX_FILE_PATH] = "none";
	char *ptr0 = strrchr(ImgFileParams[0].filenamePath, '/');
	if (ptr0 == NULL)
		ptr0 =  ImgFileParams[0].filenamePath;
	char *ptr1 = strrchr(ImgFileParams[1].filenamePath, '/');
	if (ptr1 == NULL)
		ptr1 =  ImgFileParams[1].filenamePath;
	char *ptr2, *ptr3;
	if (strcmp(ImgFileParams[2].filenamePath, "") != 0) {
		ptr2 = strrchr(ImgFileParams[2].filenamePath, '/');
		if (ptr2 == NULL)
			ptr2 =  ImgFileParams[2].filenamePath;
	} else {
		ptr2 = "none";
	}
	if (strcmp(ImgFileParams[3].filenamePath, "") != 0) {
		ptr3 = strrchr(ImgFileParams[3].filenamePath, '/');
		if (ptr3 == NULL)
			ptr3 =  ImgFileParams[3].filenamePath;
	} else {
		ptr3 = "none";
	}
	while (fscanf(fptr, "%s %s %s %s %s %d %d %s %d %d %d %d %d 0x%08lx\n",
		(char *)&fbname_db, (char *)&path1, (char *)&path2,
		(char *)&path3, (char *)&path4,	&ispixblend,
		&ispremul, (char *)&fname, &alpha0, &alpha1, &alpha2, &alpha3, &stage, &fcrc) != -1) {
		if ((strcmp(fbname_db, fbname_ref) == 0)    &&
			(ispixblend == thisFBTEST->blend.isPixBlend) &&
			(ispremul == thisFBTEST->blend.ispremul) &&
			(alpha0 == thisFBTEST->blend.ImgAlphaVal[0]) &&
			(alpha1 == thisFBTEST->blend.ImgAlphaVal[1]) &&
			(alpha2 == thisFBTEST->blend.ImgAlphaVal[2]) &&
			(alpha3 == thisFBTEST->blend.ImgAlphaVal[3]) &&
			(stage == istage) &&
			(strcmp(fname, thisFBTEST->testname) == 0) &&
			(strcmp(path1, ptr0) == 0) &&
			(strcmp(path2, ptr1) == 0) &&
			(strcmp(path3, ptr2) == 0) &&
			(strcmp(path4, ptr3) == 0)) {
			if (fcrc != crc) {
				flag = CRC_ENTRY_ADD;
			} else {
				flag = CRC_ENTRY_FOUND;
			}
			break;
		}
	}
	if (flag == CRC_ENTRY_ADD) {
		fclose(fptr);
		fptr = fopen(crcpath, "a");
		if (fptr == NULL) {
			printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
			return -CRC_DB_NOT_FOUND;
		}
		printf("%d\n", fprintf(fptr, "%s %s %s %s %s %d %d %s %d %d %d %d %d 0x%08lx\n",
			fbname_ref, ptr0, ptr1, ptr2, ptr3, thisFBTEST->blend.isPixBlend,
			thisFBTEST->blend.ispremul, "blend", thisFBTEST->blend.ImgAlphaVal[0],
			thisFBTEST->blend.ImgAlphaVal[1], thisFBTEST->blend.ImgAlphaVal[2],
			thisFBTEST->blend.ImgAlphaVal[3], istage, crc));
	}
	fclose(fptr);
	return flag;
}

/* CRC data base creation */
int crcGen(long int crc) {
	struct fbtest_params *thisFBTEST;
	int test_num = 0;
	int ret = 0;
	char fbname_ref[MAX_FB_NAME];
	thisFBTEST = &FBTEST;
	snprintf(fbname_ref, sizeof(fbname_ref), "fb%d", thisFBTEST->fBuffer);
	test_num = lookup_key_case_insensitive (testNameLUT,
			thisFBTEST->testname, -1);
	switch (test_num) {
	case 1:		/* format */
	case 6:		/* deinterlace */
	case 7:		/* dither */
	case 11:	/* colorkey */
	case 12:	/* multiop */
	case 21:	/* csc */
	case 22:	/* postproc */
		ret = crcGenPostProc((char *)&fbname_ref, crc);
		break;
	case 2:		/* scale */
	case 15:	/* overlayscale */
		ret = crcGenScale((char *)&fbname_ref, crc);
		break;
	case 3:		/* rotate */
		ret = crcGenRotate((char *)&fbname_ref, crc);
		break;
	case 4:		/* crop */
		ret = crcGenCrop((char *)&fbname_ref, crc);
		break;
	case 5:		/* move */
	case 14:	/* hwcursor */
		ret = crcGenMove((char *)&fbname_ref, crc);
		break;
	case 8:		/* videoplay */
	case 9:		/* fpsvsync */
		ret = crcGenVideoplay((char *)&fbname_ref, crc);
		break;
	case 10:	 /* blend */
		ret = crcGenBlend((char *)&fbname_ref, crc);
		break;
	default:
		printf("CRC Generation is Not supported for %s\n",
				thisFBTEST->testname);
		break;
	}
	return ret;
}

static int crcValidationPostProc(char *fbname, char *crcReportPath, long int crc) {
	FILE *fptr, *report;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = -CRC_ENTRY_NOT_FOUND;
	int fb = -1;
	char *crcpath, path[MAX_FILE_PATH], fname[100];

	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;

	fptr = fopen(crcpath, "r");
	if (fptr == NULL) {
		printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
		return -CRC_DB_NOT_FOUND;
	}
	report = fopen(crcReportPath, "a");
	if (report == NULL) {
		printf("Cant open file %s @ line %d\n", crcReportPath, __LINE__);
		return -FOPEN_FAILED;
	}

	printf("\n%s", thisFBTEST->testname);
	while (fscanf(fptr, "%s %s %s 0x%08lx\n",  (char *)&fbname_db,
		(char *)&path, (char *)&fname, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr && (strcmp(path, ptr) == 0) && (strcmp(fname, thisFBTEST->testname) == 0) &&
			(strcmp(fbname_db, fbname_ref) == 0)) {
			if (fcrc == crc) {
				fprintf(report, "%s %s %s (Ref CRC 0x%08lx) <<CRC_MATCH>>\n",
						(char *)fbname_db, (char *)path, (char *)fname, fcrc);
				flag = CRC_ENTRY_FOUND;
			} else {
				flag = -CRC_ENTRY_NOT_MATCHING;
			}
			break;
		}
	}
	if(flag == -CRC_ENTRY_NOT_MATCHING) {
		fprintf(report, "%s %s %s (Ref CRC 0x%08lx) (New CRC 0x%08lx)) ## CRC_MISMATCH ##\n",
			(char *)fbname_db, (char *)path, (char *)fname, fcrc, crc);
	}
	if(flag == -CRC_ENTRY_NOT_FOUND) {
		fprintf(report, "%s %s %s (New CRC 0x%08lx)) ## CRC_NOT_FOUND ##\n",
			(char *)fbname_db, (char *)path, (char *)fname, crc);
	}
	fclose(fptr);
	fclose(report);
	return flag;
}

static int crcValidationScale(char *fbname, char *crcReportPath, long int crc) {
	FILE *fptr, *report;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = -CRC_ENTRY_NOT_FOUND;
	int fb = -1;
	char *crcpath, path[MAX_FILE_PATH], fname[100];

	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;
	fptr = fopen(crcpath, "r");
	if (fptr == NULL) {
		printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
		return -CRC_DB_NOT_FOUND;
	}
	report = fopen(crcReportPath, "a");
	if (report == NULL) {
		printf("Cant open file %s @ line %d\n", crcReportPath, __LINE__);
		return -FOPEN_FAILED;
	}

	unsigned int w, h;
	int s;
	while (fscanf(fptr, "%s %s %d %d %d %s 0x%08lx\n", (char *)&fbname_db,
				(char *)&path, &h, &w, &s, (char *)&fname, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr && (h == thisFBTEST->outResolution.height) &&
			(w == thisFBTEST->outResolution.width) &&
			(s == thisFBTEST->isStepScale) &&
			(strcmp(fbname_db, fbname_ref) == 0)    &&
			(strcmp(fname, thisFBTEST->testname) == 0) &&
			(strcmp(path, ptr) == 0)) {
			if (fcrc == crc) {
				fprintf(report, "%s %s %d %d %d %s (Ref CRC 0x%08lx) <<CRC_MATCH>>\n",
					(char *)fbname_db, (char *)path, h, w, s, (char *)fname, fcrc);
				flag = CRC_ENTRY_FOUND;
			} else {
				flag = -CRC_ENTRY_NOT_MATCHING;
			}
			break;
		}
	}
	if(flag == -CRC_ENTRY_NOT_MATCHING) {
		fprintf(report, "%s %s %d %d %d %s (Ref CRC 0x%08lx) (New CRC 0x%08lx)) ## CRC_MISMATCH ##\n",
		(char *)fbname_db, (char *)path, h, w, s, (char *)fname, fcrc, crc);
	}
	if(flag == -CRC_ENTRY_NOT_FOUND) {
		fprintf(report, "%s %s %d %d %d %s (New CRC 0x%08lx)) ## CRC_NOT_FOUND ##\n",
		(char *)fbname_db, (char *)path, h, w, s, (char *)fname, crc);
	}
	fclose(fptr);
	fclose(report);
	return flag;
}

static int crcValidationVideoplay(char *fbname, char *crcReportPath, long int crc) {
	FILE *fptr, *report;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = -CRC_ENTRY_NOT_FOUND;
	int fb = -1, frmNum = 0;
	char *crcpath, path[MAX_FILE_PATH], fname[100];

	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;
	fptr = fopen(crcpath, "r");
	if (fptr == NULL) {
		printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
		return -CRC_DB_NOT_FOUND;
	}
	report = fopen(crcReportPath, "a");
	if (report == NULL) {
		printf("Cant open file %s @ line %d\n", crcReportPath, __LINE__);
		return -FOPEN_FAILED;
	}

	int fps;
	while (fscanf(fptr, "%s %s %d %s  frmNum %d 0x%08lx\n", (char *)&fbname_db,
		(char *)&path, &fps, (char *)&fname, &frmNum, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr && (fps == thisFBTEST->fps) && (strcmp(fname, thisFBTEST->testname) == 0) &&
			(strcmp(fbname_db, fbname_ref) == 0) &&
			(strcmp(path, ptr) == 0) && (frmNum == thisFBTEST->frameNum)) {
			if(fcrc == crc) {
				fprintf(report, "%s %s %d %s  frmNum %d (Ref CRC 0x%08lx) <<CRC_MATCH>>\n",
					(char *)fbname_db, (char *)path, fps,
					(char *)fname, frmNum, fcrc);
				flag = CRC_ENTRY_FOUND;
			} else {
				flag = -CRC_ENTRY_NOT_MATCHING;
			}
			break;
		}
	}
	if(flag == -CRC_ENTRY_NOT_MATCHING) {
		fprintf(report, "%s %s %d %s frmNum %d (Ref CRC 0x%08lx) (New CRC 0x%08lx)) ## CRC_MISMATCH ## \n",
			(char *)fbname_db, (char *)path, fps, (char *)fname, frmNum, fcrc, crc);
	}
	if(flag == -CRC_ENTRY_NOT_FOUND) {
		fprintf(report, "%s %s %d %s frmNum %d (New CRC 0x%08lx)) ## CRC_NOT_FOUND ## \n",
			(char *)fbname_db, (char *)path, fps, (char *)fname, frmNum, crc);
	}
	fclose(fptr);
	fclose(report);
	return flag;
}

static int crcValidationRotate(char *fbname, char *crcReportPath, long int crc) {
	FILE *fptr, *report;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = -CRC_ENTRY_NOT_FOUND;
	int fb = -1;
	char *crcpath, path[MAX_FILE_PATH], fname[MAX_FILE_PATH];

	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;
	fptr = fopen(crcpath, "r");
	if (fptr == NULL) {
		printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
		return -CRC_DB_NOT_FOUND;
	}
	report = fopen(crcReportPath, "a");
	if (report == NULL) {
		printf("Cant open file %s @ line %d\n", crcReportPath, __LINE__);
		return -FOPEN_FAILED;
	}

	int d, flip;
	while (fscanf(fptr, "%s %s %d %d %s 0x%08lx\n", (char *)&fbname_db,
				(char *)&path, &d, &flip, (char *)&fname, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr && (strcmp(path, ptr) == 0) && (flip == thisFBTEST->rotateFlip) &&
				(d == thisFBTEST->rotateDegree) &&
				(strcmp(fname, thisFBTEST->testname) == 0) &&
				(strcmp(fbname_db, fbname_ref) == 0)) {
			if (fcrc == crc) {
				fprintf(report, "%s %s %d %d %s (Ref CRC 0x%08lx) <<CRC_MATCH>>\n",
						(char *)fbname_db, (char *)path, d, flip, (char *)fname, fcrc);
				flag = CRC_ENTRY_FOUND;
			} else {
				flag = -CRC_ENTRY_NOT_MATCHING;
			}
			break;
		}
	}
	if(flag == -CRC_ENTRY_NOT_MATCHING) {
		fprintf(report, "%s %s %d %d %s (Ref CRC 0x%08lx) (New CRC 0x%08lx)) ## CRC_MISMATCH ##\n",
				(char *)fbname_db, (char *)path, d, flip, (char *)fname, fcrc, crc);
	}
	if(flag == -CRC_ENTRY_NOT_FOUND) {
		fprintf(report, "%s %s %d %d %s (New CRC 0x%08lx)) ## CRC_NOT_FOUND ##\n",
				(char *)fbname_db, (char *)path, d, flip, (char *)fname, crc);
	}
	fclose(fptr);
	fclose(report);
	return flag;
}

static int crcValidationMove(char *fbname, char *crcReportPath, long int crc) {
	FILE *fptr, *report;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = -CRC_ENTRY_NOT_FOUND;
	int fb = -1;
	char *crcpath, path[MAX_FILE_PATH], fname[MAX_FILE_PATH];
	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;

	fptr = fopen(crcpath, "r");
	if (fptr == NULL) {
		printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
		return -CRC_DB_NOT_FOUND;
	}
	report = fopen(crcReportPath, "a");
	if (report == NULL) {
		printf("Cant open file %s @ line %d\n", crcReportPath, __LINE__);
		return -FOPEN_FAILED;
	}

	int x1, x2, y1, y2;
	while (fscanf(fptr, "%s %s %d %d %d %d %s  0x%08lx\n",  (char *)&fbname_db,
				(char *)&path, &x1, &y1, &x2, &y2, (char *)&fname, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr && (x1 == thisFBTEST->imgOffset.startX) &&
			(y1 == thisFBTEST->imgOffset.startY) &&
			(x2 == thisFBTEST->imgOffset.endX) &&
			(y2 == thisFBTEST->imgOffset.endY) &&
			(strcmp(fbname_db, fbname_ref) == 0)    &&
			(strcmp(fname, thisFBTEST->testname) == 0) &&
			(strcmp(path, ptr) == 0)) {
			if (fcrc == crc) {
				fprintf(report, "%s %s %d %d %d %d %s   (Ref CRC 0x%08lx) <<CRC_MATCH>>\n",
						(char *)fbname_db, (char *)path, x1, y1, x2, y2, (char *)fname, fcrc);
				flag = CRC_ENTRY_FOUND;
			} else {
				flag = -CRC_ENTRY_NOT_MATCHING;
			}
			break;
		}
	}
	if(flag == -CRC_ENTRY_NOT_MATCHING) {
		fprintf(report, "%s %s %d %d %d %d %s (Ref CRC 0x%08lx) (New CRC 0x%08lx)) ## CRC_MISMATCH ##\n",
				(char *)fbname_db, (char *)path, x1, y1, x2, y2, (char *)fname, fcrc, crc);
	}
	if(flag == -CRC_ENTRY_NOT_FOUND) {
		fprintf(report, "%s %s %d %d %d %d %s (New CRC 0x%08lx)) ## CRC_NOT_FOUND ##\n",
				(char *)fbname_db, (char *)path, x1, y1, x2, y2, (char *)fname, crc);
	}
	fclose(fptr);
	fclose(report);

	return flag;
}

static int crcValidationCrop(char *fbname, char *crcReportPath, long int crc) {
	FILE *fptr, *report;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int flag = -CRC_ENTRY_NOT_FOUND;
	int fb = -1;
	char *crcpath, path[MAX_FILE_PATH], fname[MAX_FILE_PATH];

	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;
	crcpath = thisFBTEST->crcPath;

	fptr = fopen(crcpath, "r");
	if (fptr == NULL) {
		printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
		return -CRC_DB_NOT_FOUND;
	}
	report = fopen(crcReportPath, "a");
	if (report == NULL) {
		printf("Cant open file %s @ line %d\n", crcReportPath, __LINE__);
		return -FOPEN_FAILED;
	}

	struct cropParams *thisCrop;
	thisCrop = &CROPTEST;
	int c, j, i, x, y;
	unsigned int h, w;
	while (fscanf(fptr, "%s %s %d %d %d %d %d %d %d %s 0x%08lx\n",
				(char *)&fbname_db, (char *)&path, &c, &x , &y, &h, &w, &i, &j,
				(char *)&fname, &fcrc) != -1) {
		char *ptr = strrchr(thisFBTEST->fileParams.filenamePath, '/');
		if (ptr && (c == thisFBTEST->crop) &&
			(x == thisCrop->src_rect.x) &&
			(y == thisCrop->src_rect.y) &&
			(i == thisCrop->dst_rect.x) &&
			(j == thisCrop->dst_rect.y) &&
			(w == thisCrop->resolution.width) &&
			(h == thisCrop->resolution.height) &&
			(strcmp(fbname_db, fbname_ref) == 0)    &&
			(strcmp(fname, thisFBTEST->testname) == 0) &&
			(strcmp(path, ptr) == 0)) {
			if (fcrc == crc) {
				fprintf(report, "%s %s %d %d %d %d %d %d %d %s\
					 (Ref CRC 0x%08lx) <<CRC_MATCH>>\n",
					(char *)fbname_db, (char *)path, x ,
					y, c, h, w, i, j, (char *)fname, fcrc);
				flag = CRC_ENTRY_FOUND;
			} else {
				flag = -CRC_ENTRY_NOT_MATCHING;
			}
			break;
		}
	}
	if(flag == -CRC_ENTRY_NOT_MATCHING) {
		fprintf(report, "%s %s %d %d %d %d %d %d %d %s (Ref CRC 0x%08lx)\
			(New CRC 0x%08lx)) ## CRC_MISMATCH ##\n", (char *)fbname_db,
			(char *)path, x , y, c, h, w, i, j, (char *)fname, fcrc, crc);
	}
	if(flag == -CRC_ENTRY_NOT_FOUND) {
		fprintf(report, "%s %s %d %d %d %d %d %d %d %s \
			(New CRC 0x%08lx)) ## CRC_NOT_FOUND ##\n", (char *)fbname_db,
			(char *)path, x , y, c, h, w, i, j, (char *)fname, crc);
	}
	fclose(fptr);
	fclose(report);
	return flag;
}

static int crcValidationBlend(char *fbname, char *crcReportPath, long int crc) {
	FILE *fptr, *report;
	struct fbtest_params *thisFBTEST;
	thisFBTEST = &FBTEST;
	int i = 0, flag = -CRC_ENTRY_NOT_FOUND;
	int fb = -1;
	char *crcpath, path[MAX_FILE_PATH], fname[MAX_FILE_PATH];
	char *fbname_ref = fbname;
	char fbname_db[MAX_FB_NAME] = "BAD";
	long int fcrc;

	crcpath = thisFBTEST->crcPath;
	fptr = fopen(crcpath, "r");
	if (fptr == NULL) {
		printf("Cant open file %s @ line %d\n", crcpath, __LINE__);
		return -CRC_DB_NOT_FOUND;
	}
	report = fopen(crcReportPath, "a");
	if (report == NULL) {
		printf("Cant open file %s @ line %d\n", crcReportPath, __LINE__);
		return -FOPEN_FAILED;
	}

	unsigned int ispixblend, ispremul, stage;
	int alpha0, alpha1, alpha2, alpha3;
	char path1[MAX_FILE_PATH], path2[MAX_FILE_PATH], path3[MAX_FILE_PATH] = "none", path4[MAX_FILE_PATH] = "none";
	char *ptr0 = strrchr(ImgFileParams[0].filenamePath, '/');
	if (ptr0 == NULL)
		ptr0 =  ImgFileParams[0].filenamePath;
	char *ptr1 = strrchr(ImgFileParams[1].filenamePath, '/');
	if (ptr1 == NULL)
		ptr1 =  ImgFileParams[1].filenamePath;
	char *ptr2, *ptr3;
	if (strcmp(ImgFileParams[2].filenamePath, "") != 0) {
		ptr2 = strrchr(ImgFileParams[2].filenamePath, '/');
		if (ptr2 == NULL)
			ptr2 =  ImgFileParams[2].filenamePath;
	} else {
		ptr2 = "none";
	}
	if (strcmp(ImgFileParams[3].filenamePath, "") != 0) {
		ptr3 = strrchr(ImgFileParams[3].filenamePath, '/');
		if (ptr3 == NULL)
			ptr3 =  ImgFileParams[3].filenamePath;
	} else {
		ptr3 = "none";
	}
	while (fscanf(fptr, "%s %s %s %s %s %d %d %s %d %d %d %d %d 0x%08lx\n",  (char *)&fbname_db,
				(char *)&path1, (char *)&path2, (char *)&path3, (char *)&path4, &ispixblend,
				&ispremul, (char *)&fname, &alpha0, &alpha1, &alpha2, &alpha3, &stage, &fcrc) != -1) {
		if ((ispixblend == thisFBTEST->blend.isPixBlend) &&
			(strcmp(fbname_db, fbname_ref) == 0)    &&
			(ispremul == thisFBTEST->blend.ispremul) &&
			(strcmp(fname, thisFBTEST->testname) == 0) &&
			(alpha0 == thisFBTEST->blend.ImgAlphaVal[0]) &&
			(alpha1 == thisFBTEST->blend.ImgAlphaVal[1]) &&
			(alpha2 == thisFBTEST->blend.ImgAlphaVal[2]) &&
			(alpha3 == thisFBTEST->blend.ImgAlphaVal[3]) &&
			(stage == istage) &&
			(strcmp(path1, ptr0) == 0) && (strcmp(path2, ptr1) == 0) &&
			(strcmp(path3, ptr2) == 0) && (strcmp(path4, ptr3) == 0)) {
			if (fcrc == crc) {
				fprintf(report, "%s %s %s %s %s %d %d %s %d %d %d %d %d  (Ref CRC 0x%08lx) <<CRC_MATCH>>\n",
					(char *)fbname_db, (char *)path1, (char *)path2, (char *)path3, (char *)path4,
					ispixblend, ispremul, (char *)fname, alpha0, alpha1, alpha2, alpha3, stage, fcrc);
				flag = CRC_ENTRY_FOUND;
			} else {
				flag = -CRC_ENTRY_NOT_MATCHING;
			}
			break;
		}
	}
	if(flag == -CRC_ENTRY_NOT_MATCHING) {
		fprintf(report, "%s %s %s %s %s %d %d %s %d %d %d %d %d (Ref CRC 0x%08lx)\
			(New CRC 0x%08lx)) ## CRC_MISMATCH ##\n", (char *)fbname_db,
			(char *)path1, (char *)path2, (char *)path3, (char *)path4,
			ispixblend, ispremul, (char *)fname, alpha0, alpha1, alpha2,
				alpha3, stage, fcrc, crc);
	}
	if(flag == -CRC_ENTRY_NOT_FOUND) {
		fprintf(report, "%s %s %s %s %s %d %d %s %d %d %d %d %d \
			(New CRC 0x%08lx)) ## CRC_NOT_FOUND ##\n", (char *)fbname_db,
			(char *)path1, (char *)path2, (char *)path3, (char *)path4,
			ispixblend, ispremul, (char *)fname, alpha0, alpha1, alpha2,
				alpha3, stage, crc);
	}

	fclose(fptr);
	fclose(report);
	return flag;
}

/* CRC validation is performed */
int crcValidation(long int crc) {
	struct fbtest_params *thisFBTEST;
	int test_num = 0;
	int ret = -CRC_ENTRY_NOT_FOUND;
	char fbname_ref[MAX_FB_NAME] = "BAD";
	char crcReportPath[MAX_FILE_PATH];
	char *crcpath;
	thisFBTEST = &FBTEST;
	crcpath = thisFBTEST->crcPath;
	strlcpy(crcReportPath, crcpath, sizeof(crcReportPath));
	strlcat(crcReportPath, ".report", sizeof(crcReportPath));
	printf("\n\ncrcValidation Report file name (%s)\n", crcReportPath);
	snprintf(fbname_ref, sizeof(fbname_ref), "fb%d", thisFBTEST->fBuffer);
	test_num = lookup_key_case_insensitive (testNameLUT,
			thisFBTEST->testname, -1);
	switch (test_num) {
	case 1:		/* format */
	case 6:		/* deinterlace */
	case 7:		/* dither */
	case 11:	/* colorkey */
	case 12:	/* multiop */
	case 21:	/* csc */
	case 22:	/* postproc */
		printf("crcValidationPostProc\n");
		ret = crcValidationPostProc(fbname_ref, crcReportPath, crc);
		break;
	case 2:		/* scale */
	case 15:	/* overlayscale */
		printf("crcValidationScale\n");
		ret = crcValidationScale(fbname_ref, crcReportPath, crc);
		break;
	case 3:		/* rotate */
		printf("crcValidationRotate\n");
		ret = crcValidationRotate(fbname_ref, crcReportPath, crc);
		break;
	case 4:		/* crop */
		printf("crcValidationCrop\n");
		ret = crcValidationCrop(fbname_ref, crcReportPath, crc);
		break;
	case 5:		/* move */
	case 14:	/* hwcursor */
		printf("crcValidationMove\n");
		ret = crcValidationMove(fbname_ref, crcReportPath, crc);
		break;
	case 8:		/* videoplay */
	case 9:		/* fpsvsync */
		printf("crcValidationVideoplay\n");
		ret = crcValidationVideoplay(fbname_ref, crcReportPath, crc);
		break;
	case 10:	/* blend */
		printf("crcValidationBlend\n");
		ret = crcValidationBlend(fbname_ref, crcReportPath, crc);
		break;
	default:
		printf("CRC Validation Not supported for %s\n", thisFBTEST->testname);
		break;
	}
	return ret;
}
/***************************************************************/
